#include <FS.h>
#include <WiFiUdp.h>
#include <IntParsing.h>
#include <Settings.h>
#include <MiLightHttpServer.h>
#include <MiLightRadioConfig.h>
#include <string.h>
#include <TokenIterator.h>
#include <AboutHelper.h>
#include <index.html.gz.h>

using namespace std::placeholders;

void MiLightHttpServer::begin() {
  applySettings(settings);

  // set up HTTP end points to serve

  server
    .buildHandler("/")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleServe_P, this, index_html_gz, index_html_gz_len));

  server
    .buildHandler("/settings")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::serveSettings, this))
    .onJsonBody(HTTP_PUT, std::bind(&MiLightHttpServer::handleUpdateSettings, this, _2, _3))
    .onUpload(
      std::bind(&MiLightHttpServer::handleUpdateSettingsPost, this),
      std::bind(&MiLightHttpServer::handleUpdateFile, this, SETTINGS_FILE)
    );

  server
    .buildHandler("/remote_configs")
    .onJson(HTTP_GET, std::bind(&MiLightHttpServer::handleGetRadioConfigs, this, _2));

  server
    .buildHandler("/gateway_traffic")
    .onJson(HTTP_GET, std::bind(&MiLightHttpServer::handleListenGateway, this, nullptr, _2));
  server
    .buildHandler("/gateway_traffic/:type")
    .onJson(HTTP_GET, std::bind(&MiLightHttpServer::handleListenGateway, this, _1, _2));

  server
    .buildHandler("/gateways/:device_id/:type/:group_id")
    .onJsonBody(HTTP_PUT, std::bind(&MiLightHttpServer::handleUpdateGroup, this, _1, _2, _3))
    .onJsonBody(HTTP_POST, std::bind(&MiLightHttpServer::handleUpdateGroup, this, _1, _2, _3))
    .onJson(HTTP_DELETE, std::bind(&MiLightHttpServer::handleDeleteGroup, this, _1, _2))
    .onJson(HTTP_GET, std::bind(&MiLightHttpServer::handleGetGroup, this, _1, _2));

  server
    .buildHandler("/raw_commands/:type")
    .onJsonBody(HTTP_ANY, std::bind(&MiLightHttpServer::handleSendRaw, this, _1, _2, _3));

  server
    .buildHandler("/about")
    .onJson(HTTP_GET, std::bind(&MiLightHttpServer::handleAbout, this, _2));

  server
    .buildHandler("/system")
    .onJsonBody(HTTP_POST, std::bind(&MiLightHttpServer::handleSystemPost, this, _2, _3));

  server
    .buildHandler("/firmware")
    .onUpload(
      std::bind(&MiLightHttpServer::handleFirmwarePost, this),
      std::bind(&MiLightHttpServer::handleFirmwareUpload, this)
    );

  server.clearBuilders();

  // set up web socket server
  wsServer.onEvent(
    [this](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
      handleWsEvent(num, type, payload, length);
    }
  );
  wsServer.begin();

  server.begin();
}

void MiLightHttpServer::handleClient() {
  server.handleClient();
  wsServer.loop();
}

WiFiClient MiLightHttpServer::client() {
  return server.client();
}

void MiLightHttpServer::on(const char* path, HTTPMethod method, ESP8266WebServer::THandlerFunction handler) {
  server.on(path, method, handler);
}

void MiLightHttpServer::handleSystemPost(JsonDocument& requestDoc, RichHttp::Response& response) {
  JsonObject request = requestDoc.as<JsonObject>();

  bool handled = false;

  if (request.containsKey("command")) {
    if (request["command"] == "restart") {
      Serial.println(F("Restarting..."));
      server.send_P(200, TEXT_PLAIN, PSTR("true"));

      delay(100);

      ESP.restart();

      handled = true;
    } else if (request["command"] == "clear_wifi_config") {
        Serial.println(F("Resetting Wifi and then Restarting..."));
        server.send_P(200, TEXT_PLAIN, PSTR("true"));

        delay(100);
        ESP.eraseConfig();
        delay(100);
        ESP.restart();

        handled = true;
      }
  }

  if (handled) {
    response.json["success"] = true;
  } else {
    response.json["success"] = false;
    response.json["error"] = "Unhandled command";
    response.setCode(400);
  }
}

void MiLightHttpServer::serveSettings() {
  // Save first to set defaults
  settings.save();
  serveFile(SETTINGS_FILE, APPLICATION_JSON);
}

void MiLightHttpServer::applySettings(Settings& settings) {
  if (settings.hasAuthSettings()) {
    server.requireAuthentication(settings.adminUsername, settings.adminPassword);
  } else {
    server.disableAuthentication();
  }
}

void MiLightHttpServer::onSettingsSaved(SettingsSavedHandler handler) {
  this->settingsSavedHandler = handler;
}

void MiLightHttpServer::onGroupDeleted(GroupDeletedHandler handler) {
  this->groupDeletedHandler = handler;
}

void MiLightHttpServer::handleAbout(RichHttp::Response& response) {
  AboutHelper::generateAboutObject(response.json);
}

void MiLightHttpServer::handleGetRadioConfigs(RichHttp::Response& response) {
  JsonArray arr = response.json.as<JsonArray>();

  for (size_t i = 0; i < MiLightRemoteConfig::NUM_REMOTES; i++) {
    const MiLightRemoteConfig* config = MiLightRemoteConfig::ALL_REMOTES[i];
    arr.add(config->name);
  }
}

bool MiLightHttpServer::serveFile(const char* file, const char* contentType) {
  if (SPIFFS.exists(file)) {
    File f = SPIFFS.open(file, "r");
    server.streamFile(f, contentType);
    f.close();
    return true;
  }

  return false;
}

void MiLightHttpServer::handleUpdateFile(const char* filename) {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    updateFile = SPIFFS.open(filename, "w");
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if (updateFile.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Serial.println(F("Error updating web file"));
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    updateFile.close();
  }
}

void MiLightHttpServer::handleUpdateSettings(JsonDocument& request, RichHttp::Response& response) {
  JsonObject parsedSettings = request.as<JsonObject>();

  if (! parsedSettings.isNull()) {
    settings.patch(parsedSettings);
    settings.save();

    this->applySettings(settings);

    if (this->settingsSavedHandler) {
      this->settingsSavedHandler();
    }

    response.json["success"] = true;
    Serial.println(F("Settings successfully updated"));
  }
}

void MiLightHttpServer::handleUpdateSettingsPost() {
  Settings::load(settings);

  this->applySettings(settings);
  if (this->settingsSavedHandler) {
    this->settingsSavedHandler();
  }

  server.send_P(200, TEXT_PLAIN, PSTR("success."));
}

void MiLightHttpServer::handleFirmwarePost() {
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");

  if (Update.hasError()) {
    server.send_P(
      500,
      TEXT_PLAIN,
      PSTR("Failed updating firmware. Check serial logs for more information. You may need to re-flash the device.")
    );
  } else {
    server.send_P(
      200,
      TEXT_PLAIN,
      PSTR("Success. Device will now reboot.")
    );
  }

  delay(1000);

  ESP.restart();
}

void MiLightHttpServer::handleFirmwareUpload() {
  HTTPUpload& upload = server.upload();
  if(upload.status == UPLOAD_FILE_START){
    WiFiUDP::stopAll();
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if(!Update.begin(maxSketchSpace)){//start with max available size
      Update.printError(Serial);
    }
  } else if(upload.status == UPLOAD_FILE_WRITE){
    if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
      Update.printError(Serial);
    }
  } else if(upload.status == UPLOAD_FILE_END){
    if(Update.end(true)){ //true to set the size to the current progress
    } else {
      Update.printError(Serial);
    }
  }
  yield();
}


void MiLightHttpServer::handleListenGateway(const UrlTokenBindings* bindings, RichHttp::Response& response) {
  bool listenAll = bindings == NULL;
  size_t configIx = 0;
  std::shared_ptr<MiLightRadio> radio = NULL;
  const MiLightRemoteConfig* remoteConfig = NULL;
  const MiLightRemoteConfig* tmpRemoteConfig = NULL;

  uint8_t packet[MILIGHT_MAX_PACKET_LENGTH];

  if (bindings != NULL) {
    String strType(bindings->get("type"));
    tmpRemoteConfig = MiLightRemoteConfig::fromType(strType);
    milightClient->prepare(tmpRemoteConfig, 0, 0);
  }

  if (tmpRemoteConfig == NULL && !listenAll) {
    response.sendRaw(400, TEXT_PLAIN, "Unknown device type supplied");
    return;
  }

  if (tmpRemoteConfig != NULL) {
    radio = milightClient->switchRadio(tmpRemoteConfig);
  }

  while (remoteConfig == NULL) {
    if (!server.isClientConnected()) {
      return;
    }

    if (listenAll) {
      radio = milightClient->switchRadio(configIx++ % milightClient->getNumRadios());
    } else {
      radio->configure();
    }

    if (milightClient->available()) {
      size_t packetLen = milightClient->read(packet);
      remoteConfig = MiLightRemoteConfig::fromReceivedPacket(
        radio->config(),
        packet,
        packetLen
      );
    }

    yield();
  }

  char responseBody[200];
  char* responseBuffer = responseBody;

  responseBuffer += sprintf_P(
    responseBuffer,
    PSTR("\n%s packet received (%d bytes):\n"),
    remoteConfig->name.c_str(),
    remoteConfig->packetFormatter->getPacketLength()
  );
  remoteConfig->packetFormatter->format(packet, responseBuffer);

  response.json["packet_info"] = responseBody;
}

void MiLightHttpServer::sendGroupState(BulbId& bulbId, GroupState* state, RichHttp::Response& response) {
  JsonObject obj = response.json.to<JsonObject>();

  if (state != NULL) {
    state->applyState(obj, bulbId, settings.groupStateFields);
    state->debugState("test");
  }
}

void MiLightHttpServer::handleGetGroup(const UrlTokenBindings* urlBindings, RichHttp::Response& response) {
  const String _deviceId = urlBindings->get("device_id");
  uint8_t _groupId = atoi(urlBindings->get("group_id"));
  const MiLightRemoteConfig* _remoteType = MiLightRemoteConfig::fromType(urlBindings->get("type"));

  if (_remoteType == NULL) {
    char buffer[40];
    sprintf_P(buffer, PSTR("Unknown device type\n"));
    response.sendRaw(400, TEXT_PLAIN, buffer);
    return;
  }

  BulbId bulbId(parseInt<uint16_t>(_deviceId), _groupId, _remoteType->type);
  sendGroupState(bulbId, stateStore->get(bulbId), response);
}

void MiLightHttpServer::handleDeleteGroup(const UrlTokenBindings* urlBindings, RichHttp::Response& response) {
  const String _deviceId = urlBindings->get("device_id");
  uint8_t _groupId = atoi(urlBindings->get("group_id"));
  const MiLightRemoteConfig* _remoteType = MiLightRemoteConfig::fromType(urlBindings->get("type"));

  if (_remoteType == NULL) {
    char buffer[40];
    sprintf_P(buffer, PSTR("Unknown device type\n"));
    response.sendRaw(400, TEXT_PLAIN, buffer);
    return;
  }

  BulbId bulbId(parseInt<uint16_t>(_deviceId), _groupId, _remoteType->type);
  stateStore->clear(bulbId);

  if (groupDeletedHandler != NULL) {
    this->groupDeletedHandler(bulbId);
  }

  response.json["success"] = true;
}

void MiLightHttpServer::handleUpdateGroup(const UrlTokenBindings* urlBindings, JsonDocument& request, RichHttp::Response& response) {
  JsonObject reqObj = request.as<JsonObject>();

  if (reqObj.isNull()) {
    response.sendRaw(400, TEXT_PLAIN, "Invalid JSON");
    return;
  }

  milightClient->setResendCount(
    settings.httpRepeatFactor * settings.packetRepeats
  );

  String _deviceIds = urlBindings->get("device_id");
  String _groupIds = urlBindings->get("group_id");
  String _remoteTypes = urlBindings->get("type");
  char deviceIds[_deviceIds.length()];
  char groupIds[_groupIds.length()];
  char remoteTypes[_remoteTypes.length()];
  strcpy(remoteTypes, _remoteTypes.c_str());
  strcpy(groupIds, _groupIds.c_str());
  strcpy(deviceIds, _deviceIds.c_str());

  TokenIterator deviceIdItr(deviceIds, _deviceIds.length());
  TokenIterator groupIdItr(groupIds, _groupIds.length());
  TokenIterator remoteTypesItr(remoteTypes, _remoteTypes.length());

  BulbId foundBulbId;
  size_t groupCount = 0;

  while (remoteTypesItr.hasNext()) {
    const char* _remoteType = remoteTypesItr.nextToken();
    const MiLightRemoteConfig* config = MiLightRemoteConfig::fromType(_remoteType);

    if (config == NULL) {
      char buffer[40];
      sprintf_P(buffer, PSTR("Unknown device type: %s"), _remoteType);
      response.sendRaw(400, TEXT_PLAIN, buffer);
      return;
    }

    deviceIdItr.reset();
    while (deviceIdItr.hasNext()) {
      const uint16_t deviceId = parseInt<uint16_t>(deviceIdItr.nextToken());

      groupIdItr.reset();
      while (groupIdItr.hasNext()) {
        const uint8_t groupId = atoi(groupIdItr.nextToken());

        milightClient->prepare(config, deviceId, groupId);
        handleRequest(reqObj);
        foundBulbId = BulbId(deviceId, groupId, config->type);
        groupCount++;
      }
    }
  }

  if (groupCount == 1) {
    sendGroupState(foundBulbId, stateStore->get(foundBulbId), response);
  } else {
    response.json["success"] = true;
  }
}

void MiLightHttpServer::handleRequest(const JsonObject& request) {
  milightClient->update(request);
}

void MiLightHttpServer::handleSendRaw(const UrlTokenBindings* bindings, JsonDocument& requestDoc, RichHttp::Response& response) {
  JsonObject request = requestDoc.as<JsonObject>();
  const MiLightRemoteConfig* config = MiLightRemoteConfig::fromType(bindings->get("type"));

  if (config == NULL) {
    char buffer[50];
    sprintf_P(buffer, PSTR("Unknown device type: %s"), bindings->get("type"));
    response.sendRaw(400, TEXT_PLAIN, buffer);
    return;
  }

  uint8_t packet[MILIGHT_MAX_PACKET_LENGTH];
  const String& hexPacket = request["packet"];
  hexStrToBytes<uint8_t>(hexPacket.c_str(), hexPacket.length(), packet, MILIGHT_MAX_PACKET_LENGTH);

  size_t numRepeats = MILIGHT_DEFAULT_RESEND_COUNT;
  if (request.containsKey("num_repeats")) {
    numRepeats = request["num_repeats"];
  }

  milightClient->prepare(config, 0, 0);

  for (size_t i = 0; i < numRepeats; i++) {
    milightClient->write(packet);
  }

  response.json["success"] = true;
}

void MiLightHttpServer::handleWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      if (numWsClients > 0) {
        numWsClients--;
      }
      break;

    case WStype_CONNECTED:
      numWsClients++;
      break;

    default:
      Serial.printf_P(PSTR("Unhandled websocket event: %d\n"), static_cast<uint8_t>(type));
      break;
  }
}

void MiLightHttpServer::handlePacketSent(uint8_t *packet, const MiLightRemoteConfig& config) {
  if (numWsClients > 0) {
    size_t packetLen = config.packetFormatter->getPacketLength();
    char buffer[packetLen*3];
    IntParsing::bytesToHexStr(packet, packetLen, buffer, packetLen*3);

    char formattedPacket[200];
    config.packetFormatter->format(packet, formattedPacket);

    char responseBuffer[300];
    sprintf_P(
      responseBuffer,
      PSTR("\n%s packet received (%d bytes):\n%s"),
      config.name.c_str(),
      packetLen,
      formattedPacket
    );
    wsServer.broadcastTXT(reinterpret_cast<uint8_t*>(responseBuffer));
  }
}

void MiLightHttpServer::handleServe_P(const char* data, size_t length) {
  server.sendHeader("Content-Encoding", "gzip");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent_P(data, length);
  server.sendContent("");
  server.client().stop();
}

