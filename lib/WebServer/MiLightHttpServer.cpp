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
  // set up HTTP end points to serve

  server
    .buildHandler("/")
    .onSimple(HTTP_GET, std::bind(&MiLightHttpServer::handleServe_P, this, index_html_gz, index_html_gz_len));

  server
    .buildHandler("/settings")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::serveSettings, this))
    .on(HTTP_PUT, std::bind(&MiLightHttpServer::handleUpdateSettings, this, _1))
    .on(
      HTTP_POST,
      std::bind(&MiLightHttpServer::handleUpdateSettingsPost, this, _1),
      std::bind(&MiLightHttpServer::handleUpdateFile, this, SETTINGS_FILE)
    );

  server
    .buildHandler("/remote_configs")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleGetRadioConfigs, this, _1));

  server
    .buildHandler("/gateway_traffic")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleListenGateway, this, _1));
  server
    .buildHandler("/gateway_traffic/:type")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleListenGateway, this, _1));

  server
    .buildHandler("/gateways/:device_id/:type/:group_id")
    .on(HTTP_PUT, std::bind(&MiLightHttpServer::handleUpdateGroup, this, _1))
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleUpdateGroup, this, _1))
    .on(HTTP_DELETE, std::bind(&MiLightHttpServer::handleDeleteGroup, this, _1))
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleGetGroup, this, _1));

  server
    .buildHandler("/gateways/:device_alias")
    .on(HTTP_PUT, std::bind(&MiLightHttpServer::handleUpdateGroupAlias, this, _1))
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleUpdateGroupAlias, this, _1))
    .on(HTTP_DELETE, std::bind(&MiLightHttpServer::handleDeleteGroupAlias, this, _1))
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleGetGroupAlias, this, _1));

  server
    .buildHandler("/transitions/:id")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleGetTransition, this, _1))
    .on(HTTP_DELETE, std::bind(&MiLightHttpServer::handleDeleteTransition, this, _1));

  server
    .buildHandler("/transitions")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleListTransitions, this, _1))
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleCreateTransition, this, _1));

  server
    .buildHandler("/raw_commands/:type")
    .on(HTTP_ANY, std::bind(&MiLightHttpServer::handleSendRaw, this, _1));

  server
    .buildHandler("/about")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleAbout, this, _1));

  server
    .buildHandler("/system")
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleSystemPost, this, _1));

  server
    .buildHandler("/firmware")
    .handleOTA();

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

void MiLightHttpServer::handleSystemPost(RequestContext& request) {
  JsonObject requestBody = request.getJsonBody().as<JsonObject>();

  bool handled = false;

  if (requestBody.containsKey(GroupStateFieldNames::COMMAND)) {
    if (requestBody[GroupStateFieldNames::COMMAND] == "restart") {
      Serial.println(F("Restarting..."));
      server.send_P(200, TEXT_PLAIN, PSTR("true"));

      delay(100);

      ESP.restart();

      handled = true;
    } else if (requestBody[GroupStateFieldNames::COMMAND] == "clear_wifi_config") {
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
    request.response.json["success"] = true;
  } else {
    request.response.json["success"] = false;
    request.response.json["error"] = "Unhandled command";
    request.response.setCode(400);
  }
}

void MiLightHttpServer::serveSettings() {
  // Save first to set defaults
  settings.save();
  serveFile(SETTINGS_FILE, APPLICATION_JSON);
}

void MiLightHttpServer::onSettingsSaved(SettingsSavedHandler handler) {
  this->settingsSavedHandler = handler;
}

void MiLightHttpServer::onGroupDeleted(GroupDeletedHandler handler) {
  this->groupDeletedHandler = handler;
}

void MiLightHttpServer::handleAbout(RequestContext& request) {
  AboutHelper::generateAboutObject(request.response.json);

  JsonObject queueStats = request.response.json.createNestedObject("queue_stats");
  queueStats[F("length")] = packetSender->queueLength();
  queueStats[F("dropped_packets")] = packetSender->droppedPackets();
}

void MiLightHttpServer::handleGetRadioConfigs(RequestContext& request) {
  JsonArray arr = request.response.json.to<JsonArray>();

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

void MiLightHttpServer::handleUpdateSettings(RequestContext& request) {
  JsonObject parsedSettings = request.getJsonBody().as<JsonObject>();

  if (! parsedSettings.isNull()) {
    settings.patch(parsedSettings);
    settings.save();

    if (this->settingsSavedHandler) {
      this->settingsSavedHandler();
    }

    request.response.json["success"] = true;
    Serial.println(F("Settings successfully updated"));
  }
}

void MiLightHttpServer::handleUpdateSettingsPost(RequestContext& request) {
  Settings::load(settings);

  if (this->settingsSavedHandler) {
    this->settingsSavedHandler();
  }

  request.response.json["success"] = true;
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


void MiLightHttpServer::handleListenGateway(RequestContext& request) {
  bool listenAll = !request.pathVariables.hasBinding("type");
  size_t configIx = 0;
  std::shared_ptr<MiLightRadio> radio = NULL;
  const MiLightRemoteConfig* remoteConfig = NULL;
  const MiLightRemoteConfig* tmpRemoteConfig = NULL;

  uint8_t packet[MILIGHT_MAX_PACKET_LENGTH];

  if (!listenAll) {
    String strType(request.pathVariables.get("type"));
    tmpRemoteConfig = MiLightRemoteConfig::fromType(strType);
    milightClient->prepare(tmpRemoteConfig, 0, 0);
  }

  if (tmpRemoteConfig == NULL && !listenAll) {
    request.response.setCode(400);
    request.response.json["error"] = "Unknown device type supplied";
    return;
  }

  if (tmpRemoteConfig != NULL) {
    radio = radios->switchRadio(tmpRemoteConfig);
  }

  while (remoteConfig == NULL) {
    if (!server.client().connected()) {
      return;
    }

    if (listenAll) {
      radio = radios->switchRadio(configIx++ % radios->getNumRadios());
    } else {
      radio->configure();
    }

    if (radios->available()) {
      size_t packetLen = radios->read(packet);
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

  request.response.json["packet_info"] = responseBody;
}

void MiLightHttpServer::sendGroupState(BulbId& bulbId, GroupState* state, RichHttp::Response& response) {
  bool blockOnQueue = server.arg("blockOnQueue").equalsIgnoreCase("true");

  // Wait for packet queue to flush out.  State will not have been updated before that.
  // Bit hacky to call loop outside of main loop, but should be fine.
  while (blockOnQueue && packetSender->isSending()) {
    packetSender->loop();
  }

  JsonObject obj = response.json.to<JsonObject>();

  if (blockOnQueue && state != NULL) {
    state->applyState(obj, bulbId, settings.groupStateFields);
  } else {
    obj[F("success")] = true;
  }
}

void MiLightHttpServer::_handleGetGroup(BulbId bulbId, RequestContext& request) {
  sendGroupState(bulbId, stateStore->get(bulbId), request.response);
}

void MiLightHttpServer::handleGetGroupAlias(RequestContext& request) {
  const String alias = request.pathVariables.get("device_alias");

  std::map<String, BulbId>::iterator it = settings.groupIdAliases.find(alias);

  if (it == settings.groupIdAliases.end()) {
    request.response.setCode(404);
    request.response.json[F("error")] = F("Device alias not found");
    return;
  }

  _handleGetGroup(it->second, request);
}

void MiLightHttpServer::handleGetGroup(RequestContext& request) {
  const String _deviceId = request.pathVariables.get(GroupStateFieldNames::DEVICE_ID);
  uint8_t _groupId = atoi(request.pathVariables.get(GroupStateFieldNames::GROUP_ID));
  const MiLightRemoteConfig* _remoteType = MiLightRemoteConfig::fromType(request.pathVariables.get("type"));

  if (_remoteType == NULL) {
    char buffer[40];
    sprintf_P(buffer, PSTR("Unknown device type\n"));
    request.response.setCode(400);
    request.response.json["error"] = buffer;
    return;
  }

  BulbId bulbId(parseInt<uint16_t>(_deviceId), _groupId, _remoteType->type);
  _handleGetGroup(bulbId, request);
}

void MiLightHttpServer::handleDeleteGroup(RequestContext& request) {
  const String _deviceId = request.pathVariables.get(GroupStateFieldNames::DEVICE_ID);
  uint8_t _groupId = atoi(request.pathVariables.get(GroupStateFieldNames::GROUP_ID));
  const MiLightRemoteConfig* _remoteType = MiLightRemoteConfig::fromType(request.pathVariables.get("type"));

  if (_remoteType == NULL) {
    char buffer[40];
    sprintf_P(buffer, PSTR("Unknown device type\n"));
    request.response.setCode(400);
    request.response.json["error"] = buffer;
    return;
  }

  BulbId bulbId(parseInt<uint16_t>(_deviceId), _groupId, _remoteType->type);
  _handleDeleteGroup(bulbId, request);
}

void MiLightHttpServer::handleDeleteGroupAlias(RequestContext& request) {
  const String alias = request.pathVariables.get("device_alias");

  std::map<String, BulbId>::iterator it = settings.groupIdAliases.find(alias);

  if (it == settings.groupIdAliases.end()) {
    request.response.setCode(404);
    request.response.json[F("error")] = F("Device alias not found");
    return;
  }

  _handleDeleteGroup(it->second, request);
}

void MiLightHttpServer::_handleDeleteGroup(BulbId bulbId, RequestContext& request) {
  stateStore->clear(bulbId);

  if (groupDeletedHandler != NULL) {
    this->groupDeletedHandler(bulbId);
  }

  request.response.json["success"] = true;
}

void MiLightHttpServer::handleUpdateGroupAlias(RequestContext& request) {
  const String alias = request.pathVariables.get("device_alias");

  std::map<String, BulbId>::iterator it = settings.groupIdAliases.find(alias);

  if (it == settings.groupIdAliases.end()) {
    request.response.setCode(404);
    request.response.json[F("error")] = F("Device alias not found");
    return;
  }

  BulbId& bulbId = it->second;
  const MiLightRemoteConfig* config = MiLightRemoteConfig::fromType(bulbId.deviceType);

  if (config == NULL) {
    char buffer[40];
    sprintf_P(buffer, PSTR("Unknown device type: %s"), bulbId.deviceType);
    request.response.setCode(400);
    request.response.json["error"] = buffer;
    return;
  }

  milightClient->prepare(config, bulbId.deviceId, bulbId.groupId);
  handleRequest(request.getJsonBody().as<JsonObject>());
  sendGroupState(bulbId, stateStore->get(bulbId), request.response);
}

void MiLightHttpServer::handleUpdateGroup(RequestContext& request) {
  JsonObject reqObj = request.getJsonBody().as<JsonObject>();

  String _deviceIds = request.pathVariables.get(GroupStateFieldNames::DEVICE_ID);
  String _groupIds = request.pathVariables.get(GroupStateFieldNames::GROUP_ID);
  String _remoteTypes = request.pathVariables.get("type");
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
      request.response.setCode(400);
      request.response.json["error"] = buffer;
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
    sendGroupState(foundBulbId, stateStore->get(foundBulbId), request.response);
  } else {
    request.response.json["success"] = true;
  }
}

void MiLightHttpServer::handleRequest(const JsonObject& request) {
  milightClient->setRepeatsOverride(
    settings.httpRepeatFactor * settings.packetRepeats
  );
  milightClient->update(request);
  milightClient->clearRepeatsOverride();
}

void MiLightHttpServer::handleSendRaw(RequestContext& request) {
  JsonObject requestBody = request.getJsonBody().as<JsonObject>();
  const MiLightRemoteConfig* config = MiLightRemoteConfig::fromType(request.pathVariables.get("type"));

  if (config == NULL) {
    char buffer[50];
    sprintf_P(buffer, PSTR("Unknown device type: %s"), request.pathVariables.get("type"));
    request.response.setCode(400);
    request.response.json["error"] = buffer;
    return;
  }

  uint8_t packet[MILIGHT_MAX_PACKET_LENGTH];
  const String& hexPacket = requestBody["packet"];
  hexStrToBytes<uint8_t>(hexPacket.c_str(), hexPacket.length(), packet, MILIGHT_MAX_PACKET_LENGTH);

  size_t numRepeats = settings.packetRepeats;
  if (requestBody.containsKey("num_repeats")) {
    numRepeats = requestBody["num_repeats"];
  }

  packetSender->enqueue(packet, config, numRepeats);

  // To make this response synchronous, wait for packet to be flushed
  while (packetSender->isSending()) {
    packetSender->loop();
  }

  request.response.json["success"] = true;
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

void MiLightHttpServer::handleGetTransition(RequestContext& request) {
  size_t id = atoi(request.pathVariables.get("id"));
  auto transition = transitions.getTransition(id);

  if (transition == nullptr) {
    request.response.setCode(404);
    request.response.json["error"] = "Not found";
  } else {
    JsonObject response = request.response.json.to<JsonObject>();
    transition->serialize(response);
  }
}

void MiLightHttpServer::handleDeleteTransition(RequestContext& request) {
  size_t id = atoi(request.pathVariables.get("id"));
  bool success = transitions.deleteTransition(id);

  if (success) {
    request.response.json["success"] = true;
  } else {
    request.response.setCode(404);
    request.response.json["error"] = "Not found";
  }
}

void MiLightHttpServer::handleListTransitions(RequestContext& request) {
  auto current = transitions.getTransitions();
  JsonArray transitions = request.response.json.to<JsonObject>().createNestedArray(F("transitions"));

  while (current != nullptr) {
    JsonObject json = transitions.createNestedObject();
    current->data->serialize(json);
    current = current->next;
  }
}

void MiLightHttpServer::handleCreateTransition(RequestContext& request) {
  JsonObject body = request.getJsonBody().as<JsonObject>();

  if (! body.containsKey(GroupStateFieldNames::DEVICE_ID)
    || ! body.containsKey(GroupStateFieldNames::GROUP_ID)
    || ! body.containsKey(F("remote_type"))) {
    char buffer[200];
    sprintf_P(buffer, PSTR("Must specify required keys: device_id, group_id, remote_type"));

    request.response.setCode(400);
    request.response.json[F("error")] = buffer;
    return;
  }

  const String _deviceId = body[GroupStateFieldNames::DEVICE_ID];
  uint8_t _groupId = body[GroupStateFieldNames::GROUP_ID];
  const MiLightRemoteConfig* _remoteType = MiLightRemoteConfig::fromType(body[F("remote_type")].as<const char*>());

  if (_remoteType == nullptr) {
    char buffer[40];
    sprintf_P(buffer, PSTR("Unknown device type\n"));
    request.response.setCode(400);
    request.response.json[F("error")] = buffer;
    return;
  }

  milightClient->prepare(_remoteType, parseInt<uint16_t>(_deviceId), _groupId);

  if (milightClient->handleTransition(request.getJsonBody().as<JsonObject>(), request.response.json)) {
    request.response.json[F("success")] = true;
  } else {
    request.response.setCode(400);
  }
}