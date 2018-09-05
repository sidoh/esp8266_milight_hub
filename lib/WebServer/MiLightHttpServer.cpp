#include <FS.h>
#include <WiFiUdp.h>
#include <IntParsing.h>
#include <Settings.h>
#include <MiLightHttpServer.h>
#include <MiLightRadioConfig.h>
#include <string.h>
#include <TokenIterator.h>
#include <index.html.gz.h>

void MiLightHttpServer::begin() {
  applySettings(settings);

  // set up HTTP end points to serve

  _handleRootPage = handleServe_P(index_html_gz, index_html_gz_len);
  server.onAuthenticated("/", HTTP_GET, [this]() { _handleRootPage(); });
  server.onAuthenticated("/settings", HTTP_GET, [this]() { serveSettings(); });
  server.onAuthenticated("/settings", HTTP_PUT, [this]() { handleUpdateSettings(); });
  server.onAuthenticated("/settings", HTTP_POST, [this]() { handleUpdateSettingsPost(); }, handleUpdateFile(SETTINGS_FILE));
  server.onAuthenticated("/radio_configs", HTTP_GET, [this]() { handleGetRadioConfigs(); });

  server.onAuthenticated("/gateway_traffic", HTTP_GET, [this]() { handleListenGateway(NULL); });
  server.onPatternAuthenticated("/gateway_traffic/:type", HTTP_GET, [this](const UrlTokenBindings* b) { handleListenGateway(b); });

  const char groupPattern[] = "/gateways/:device_id/:type/:group_id";
  server.onPatternAuthenticated(groupPattern, HTTP_PUT, [this](const UrlTokenBindings* b) { handleUpdateGroup(b); });
  server.onPatternAuthenticated(groupPattern, HTTP_POST, [this](const UrlTokenBindings* b) { handleUpdateGroup(b); });
  server.onPatternAuthenticated(groupPattern, HTTP_GET, [this](const UrlTokenBindings* b) { handleGetGroup(b); });

  server.onPatternAuthenticated("/raw_commands/:type", HTTP_ANY, [this](const UrlTokenBindings* b) { handleSendRaw(b); });
  server.onAuthenticated("/web", HTTP_POST, [this]() { server.send_P(200, TEXT_PLAIN, PSTR("success")); }, handleUpdateFile(WEB_INDEX_FILENAME));
  server.on("/about", HTTP_GET, [this]() { handleAbout(); });
  server.onAuthenticated("/system", HTTP_POST, [this]() { handleSystemPost(); });
  server.onAuthenticated("/firmware", HTTP_POST, [this]() { handleFirmwarePost(); }, [this]() { handleFirmwareUpload(); });


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

void MiLightHttpServer::on(const char* path, HTTPMethod method, ESP8266WebServer::THandlerFunction handler) {
  server.on(path, method, handler);
}

WiFiClient MiLightHttpServer::client() {
  return server.client();
}

void MiLightHttpServer::handleSystemPost() {
  DynamicJsonBuffer buffer;
  JsonObject& request = buffer.parse(server.arg("plain"));

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
    server.send_P(200, TEXT_PLAIN, PSTR("true"));
  } else {
    server.send_P(400, TEXT_PLAIN, PSTR("{\"error\":\"Unhandled command\"}"));
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

void MiLightHttpServer::handleAbout() {
  DynamicJsonBuffer buffer;
  JsonObject& response = buffer.createObject();

  response["version"] = QUOTE(MILIGHT_HUB_VERSION);
  response["variant"] = QUOTE(FIRMWARE_VARIANT);
  response["free_heap"] = ESP.getFreeHeap();
  response["arduino_version"] = ESP.getCoreVersion();
  response["reset_reason"] = ESP.getResetReason();

  String body;
  response.printTo(body);

  server.send(200, APPLICATION_JSON, body);
}

void MiLightHttpServer::handleGetRadioConfigs() {
  DynamicJsonBuffer buffer;
  JsonArray& arr = buffer.createArray();

  for (size_t i = 0; i < MiLightRadioConfig::NUM_CONFIGS; i++) {
    const MiLightRemoteConfig* config = MiLightRemoteConfig::ALL_REMOTES[i];
    arr.add(config->name);
  }

  String body;
  arr.printTo(body);

  server.send(200, APPLICATION_JSON, body);
}

ESP8266WebServer::THandlerFunction MiLightHttpServer::handleServeFile(
  const char* filename,
  const char* contentType,
  const char* defaultText) {

  return [this, filename, contentType, defaultText]() {
    if (!serveFile(filename)) {
      if (defaultText) {
        server.send(200, contentType, defaultText);
      } else {
        server.send(404);
      }
    }
  };
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

ESP8266WebServer::THandlerFunction MiLightHttpServer::handleUpdateFile(const char* filename) {
  return [this, filename]() {
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
  };
}

void MiLightHttpServer::handleUpdateSettings() {
  DynamicJsonBuffer buffer;
  const String& rawSettings = server.arg("plain");
  JsonObject& parsedSettings = buffer.parse(rawSettings);

  if (parsedSettings.success()) {
    settings.patch(parsedSettings);
    settings.save();

    this->applySettings(settings);

    if (this->settingsSavedHandler) {
      this->settingsSavedHandler();
    }

    server.send(200, APPLICATION_JSON, "true");
    Serial.println(F("Settings successfully updated"));
  } else {
    server.send(400, APPLICATION_JSON, "\"Invalid JSON\"");
    Serial.println(F("Settings failed to update; invalid JSON"));
  }
}

void MiLightHttpServer::handleUpdateSettingsPost() {
  Settings::load(settings);
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


void MiLightHttpServer::handleListenGateway(const UrlTokenBindings* bindings) {
  bool available = false;
  bool listenAll = bindings == NULL;
  size_t configIx = 0;
  const MiLightRadioConfig* radioConfig = NULL;
  const MiLightRemoteConfig* remoteConfig = NULL;
  uint8_t packet[MILIGHT_MAX_PACKET_LENGTH];

  if (bindings != NULL) {
    String strType(bindings->get("type"));
    const MiLightRemoteConfig* remoteConfig = MiLightRemoteConfig::fromType(strType);
    milightClient->prepare(remoteConfig, 0, 0);
    radioConfig = &remoteConfig->radioConfig;
  }

  if (radioConfig == NULL && !listenAll) {
    server.send_P(400, TEXT_PLAIN, PSTR("Unknown device type supplied."));
    return;
  }

  while (remoteConfig == NULL) {
    if (!server.clientConnected()) {
      return;
    }

    if (listenAll) {
      radioConfig = &milightClient->switchRadio(configIx++ % milightClient->getNumRadios())->config();
    }

    if (milightClient->available()) {
      size_t packetLen = milightClient->read(packet);
      remoteConfig = MiLightRemoteConfig::fromReceivedPacket(
        *radioConfig,
        packet,
        packetLen
      );
    }

    yield();
  }

  char response[200];
  char* responseBuffer = response;

  responseBuffer += sprintf_P(
    responseBuffer,
    PSTR("\n%s packet received (%d bytes):\n"),
    remoteConfig->name.c_str(),
    remoteConfig->packetFormatter->getPacketLength()
  );
  remoteConfig->packetFormatter->format(packet, responseBuffer);

  server.send(200, "text/plain", response);
}

void MiLightHttpServer::sendGroupState(BulbId& bulbId, GroupState* state) {
  String body;
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& obj = jsonBuffer.createObject();

  if (state != NULL) {
    state->applyState(obj, bulbId, settings.groupStateFields, settings.numGroupStateFields);
  }

  obj.printTo(body);

  server.send(200, APPLICATION_JSON, body);
}

void MiLightHttpServer::handleGetGroup(const UrlTokenBindings* urlBindings) {
  const String _deviceId = urlBindings->get("device_id");
  uint8_t _groupId = atoi(urlBindings->get("group_id"));
  const MiLightRemoteConfig* _remoteType = MiLightRemoteConfig::fromType(urlBindings->get("type"));

  if (_remoteType == NULL) {
    char buffer[40];
    sprintf_P(buffer, PSTR("Unknown device type\n"));
    server.send(400, TEXT_PLAIN, buffer);
    return;
  }

  BulbId bulbId(parseInt<uint16_t>(_deviceId), _groupId, _remoteType->type);
  GroupState* state = stateStore->get(bulbId);
  sendGroupState(bulbId, stateStore->get(bulbId));
}

void MiLightHttpServer::handleUpdateGroup(const UrlTokenBindings* urlBindings) {
  DynamicJsonBuffer buffer;
  JsonObject& request = buffer.parse(server.arg("plain"));

  if (!request.success()) {
    server.send_P(400, TEXT_PLAIN, PSTR("Invalid JSON"));
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
      server.send(400, "text/plain", buffer);
      return;
    }

    deviceIdItr.reset();
    while (deviceIdItr.hasNext()) {
      const uint16_t deviceId = parseInt<uint16_t>(deviceIdItr.nextToken());

      groupIdItr.reset();
      while (groupIdItr.hasNext()) {
        const uint8_t groupId = atoi(groupIdItr.nextToken());

        milightClient->prepare(config, deviceId, groupId);
        handleRequest(request);
        foundBulbId = BulbId(deviceId, groupId, config->type);
        groupCount++;
      }
    }
  }

  if (groupCount == 1) {
    sendGroupState(foundBulbId, stateStore->get(foundBulbId));
  } else {
    server.send(200, APPLICATION_JSON, "true");
  }
}

void MiLightHttpServer::handleRequest(const JsonObject& request) {
  milightClient->update(request);
}

void MiLightHttpServer::handleSendRaw(const UrlTokenBindings* bindings) {
  DynamicJsonBuffer buffer;
  JsonObject& request = buffer.parse(server.arg("plain"));
  const MiLightRemoteConfig* config = MiLightRemoteConfig::fromType(bindings->get("type"));

  if (config == NULL) {
    char buffer[50];
    sprintf_P(buffer, PSTR("Unknown device type: %s"), bindings->get("type"));
    server.send(400, "text/plain", buffer);
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

  server.send_P(200, TEXT_PLAIN, PSTR("true"));
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

ESP8266WebServer::THandlerFunction MiLightHttpServer::handleServe_P(const char* data, size_t length) {
  return [this, data, length]() {
    server.sendHeader("Content-Encoding", "gzip");
    server.setContentLength(CONTENT_LENGTH_UNKNOWN);
    server.send(200, "text/html", "");
    server.sendContent_P(data, length);
    server.sendContent("");
    server.client().stop();
  };
}

