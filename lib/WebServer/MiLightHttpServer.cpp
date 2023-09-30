#include <FS.h>
#include <WiFiUdp.h>
#include <IntParsing.h>
#include <Settings.h>
#include <MiLightHttpServer.h>
#include <MiLightRadioConfig.h>
#include <string.h>
#include <TokenIterator.h>
#include <AboutHelper.h>
#include <GroupAlias.h>
#include <ProjectFS.h>
#include <StreamUtils.h>

#include <index.html.gz.h>
#include <BackupManager.h>

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
    .buildHandler("/backup")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleCreateBackup, this, _1))
    .on(
        HTTP_POST,
        std::bind(&MiLightHttpServer::handleRestoreBackup, this, _1),
        std::bind(&MiLightHttpServer::handleUpdateFile, this, BACKUP_FILE));

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
    .buildHandler("/aliases")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleListAliases, this, _1))
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleCreateAlias, this, _1));

  server
    .buildHandler("/aliases.bin")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::serveFile, this, ALIASES_FILE, APPLICATION_OCTET_STREAM))
    .on(HTTP_DELETE, std::bind(&MiLightHttpServer::handleDeleteAliases, this, _1))
    .on(
        HTTP_POST,
        std::bind(&MiLightHttpServer::handleUpdateAliases, this, _1),
        std::bind(&MiLightHttpServer::handleUpdateFile, this, ALIASES_FILE)
    );

  server
    .buildHandler("/aliases/:id")
    .on(HTTP_PUT, std::bind(&MiLightHttpServer::handleUpdateAlias, this, _1))
    .on(HTTP_DELETE, std::bind(&MiLightHttpServer::handleDeleteAlias, this, _1));

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
  if (ProjectFS.exists(file)) {
    File f = ProjectFS.open(file, "r");
    server.streamFile(f, contentType);
    f.close();
    return true;
  }

  return false;
}

void MiLightHttpServer::handleUpdateFile(const char* filename) {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    updateFile = ProjectFS.open(filename, "w");
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
    saveSettings();

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

void MiLightHttpServer::sendGroupState(bool allowAsync, BulbId& bulbId, RichHttp::Response& response) {
  bool blockOnQueue = server.arg("blockOnQueue").equalsIgnoreCase("true");

  // Wait for packet queue to flush out.  State will not have been updated before that.
  // Bit hacky to call loop outside of main loop, but should be fine.
  while (blockOnQueue && packetSender->isSending()) {
    packetSender->loop();
  }

  JsonObject obj = response.json.to<JsonObject>();
  GroupState* state = stateStore->get(bulbId);

  if (blockOnQueue || allowAsync) {
    if (state == nullptr) {
      obj[F("error")] = F("not found");
      response.setCode(404);
    } else {
      state->applyState(obj, bulbId, settings.groupStateFields);
    }
  } else {
    obj[F("success")] = true;
  }
}

void MiLightHttpServer::_handleGetGroup(bool allowAsync, BulbId bulbId, RequestContext& request) {
  sendGroupState(allowAsync, bulbId, request.response);
}

void MiLightHttpServer::handleGetGroupAlias(RequestContext& request) {
  const String alias = request.pathVariables.get("device_alias");

  auto it = settings.groupIdAliases.find(alias);

  if (it == settings.groupIdAliases.end()) {
    request.response.setCode(404);
    request.response.json[F("error")] = F("Device alias not found");
    return;
  }

  _handleGetGroup(true, it->second.bulbId, request);
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
  _handleGetGroup(true, bulbId, request);
}

void MiLightHttpServer::handleDeleteGroup(RequestContext& request) {
  const char* _deviceId = request.pathVariables.get("device_id");
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

  auto it = settings.groupIdAliases.find(alias);

  if (it == settings.groupIdAliases.end()) {
    request.response.setCode(404);
    request.response.json[F("error")] = F("Device alias not found");
    return;
  }

  _handleDeleteGroup(it->second.bulbId, request);
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

  auto it = settings.groupIdAliases.find(alias);

  if (it == settings.groupIdAliases.end()) {
    request.response.setCode(404);
    request.response.json[F("error")] = F("Device alias not found");
    return;
  }

  BulbId& bulbId = it->second.bulbId;
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
  sendGroupState(false, bulbId, request.response);
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
    sendGroupState(false, foundBulbId, request.response);
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
    || (!body.containsKey(F("remote_type")) && !body.containsKey(GroupStateFieldNames::DEVICE_TYPE))) {
    char buffer[200];
    sprintf_P(buffer, PSTR("Must specify required keys: device_id, group_id, device_type"));

    request.response.setCode(400);
    request.response.json[F("error")] = buffer;
    return;
  }

  const String _deviceId = body[GroupStateFieldNames::DEVICE_ID];
  uint8_t _groupId = body[GroupStateFieldNames::GROUP_ID];
  const MiLightRemoteConfig* _remoteType = nullptr;

  if (body.containsKey(GroupStateFieldNames::DEVICE_TYPE)) {
    _remoteType = MiLightRemoteConfig::fromType(body[GroupStateFieldNames::DEVICE_TYPE].as<const char*>());
  } else if (body.containsKey(F("remote_type"))) {
    _remoteType = MiLightRemoteConfig::fromType(body[F("remote_type")].as<const char*>());
  }

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

void MiLightHttpServer::handleListAliases(RequestContext& request) {
  uint8_t page = request.server.hasArg("page") ? request.server.arg("page").toInt() : 1;

  // at least 1 per page
  uint8_t perPage = request.server.hasArg("page_size") ? request.server.arg("page_size").toInt() : DEFAULT_PAGE_SIZE;
  perPage = perPage > 0 ? perPage : 1;

  uint8_t numPages = settings.groupIdAliases.empty() ? 1 : ceil(settings.groupIdAliases.size() / (float) perPage);

  // check bounds
  if (page < 1 || page > numPages) {
    request.response.setCode(404);
    request.response.json[F("error")] = F("Page out of bounds");
    request.response.json[F("page")] = page;
    request.response.json[F("num_pages")] = numPages;
    return;
  } 

  JsonArray aliases = request.response.json.to<JsonObject>().createNestedArray(F("aliases"));
  request.response.json[F("page")] = page;
  request.response.json[F("count")] = settings.groupIdAliases.size();
  request.response.json[F("num_pages")] = numPages;

  // Skip iterator to start of page
  auto it = settings.groupIdAliases.begin();
  std::advance(it, (page - 1) * perPage);

  for (size_t i = 0; i < perPage && it != settings.groupIdAliases.end(); i++, it++) {
    JsonObject alias = aliases.createNestedObject();
    alias[F("alias")] = it->first;
    alias[F("id")] = it->second.id;

    const BulbId& bulbId = it->second.bulbId;
    alias[F("device_id")] = bulbId.deviceId;
    alias[F("group_id")] = bulbId.groupId;
    alias[F("device_type")] = MiLightRemoteTypeHelpers::remoteTypeToString(bulbId.deviceType);

  }
}

void MiLightHttpServer::handleCreateAlias(RequestContext& request) {
  JsonObject body = request.getJsonBody().as<JsonObject>();

  if (! body.containsKey(F("alias"))
    || ! body.containsKey(GroupStateFieldNames::DEVICE_ID)
    || ! body.containsKey(GroupStateFieldNames::GROUP_ID)
    || ! body.containsKey(GroupStateFieldNames::DEVICE_TYPE)) {
    char buffer[200];
    sprintf_P(buffer, PSTR("Must specify required keys: alias, device_id, group_id, device_type"));

    request.response.setCode(400);
    request.response.json[F("error")] = buffer;
    return;
  }

  const String alias = body[F("alias")];
  const uint16_t deviceId = body[GroupStateFieldNames::DEVICE_ID];
  const uint8_t groupId = body[GroupStateFieldNames::GROUP_ID];
  const MiLightRemoteType deviceType = MiLightRemoteTypeHelpers::remoteTypeFromString(body[GroupStateFieldNames::DEVICE_TYPE].as<const char*>());

  if (settings.groupIdAliases.find(alias) != settings.groupIdAliases.end()) {
    char buffer[200];
    sprintf_P(buffer, PSTR("Alias already exists: %s"), alias.c_str());

    request.response.setCode(400);
    request.response.json[F("error")] = buffer;
    return;
  }

  settings.addAlias(alias.c_str(), BulbId(deviceId, groupId, deviceType));
  saveSettings();

  request.response.json[F("success")] = true;
  request.response.json[F("id")] = settings.groupIdAliases[alias].id;
}

void MiLightHttpServer::handleDeleteAlias(RequestContext& request) {
  const size_t id = atoi(request.pathVariables.get("id"));

  if (settings.deleteAlias(id)) {
    saveSettings();
    request.response.json[F("success")] = true;
  } else {
    request.response.setCode(404);
    request.response.json[F("error")] = F("Alias not found");
    return;
  }
}

void MiLightHttpServer::handleUpdateAlias(RequestContext& request) {
  const size_t id = atoi(request.pathVariables.get("id"));
  auto alias = settings.findAliasById(id);

  if (alias == settings.groupIdAliases.end()) {
    request.response.setCode(404);
    request.response.json[F("error")] = F("Alias not found");
    return;
  } else {
    JsonObject body = request.getJsonBody().as<JsonObject>();
    GroupAlias updatedAlias(alias->second);

    if (body.containsKey(F("alias"))) {
      strncpy(updatedAlias.alias, body[F("alias")].as<const char*>(), MAX_ALIAS_LEN);
    }

    if (body.containsKey(GroupStateFieldNames::DEVICE_ID)) {
      updatedAlias.bulbId.deviceId = body[GroupStateFieldNames::DEVICE_ID];
    }

    if (body.containsKey(GroupStateFieldNames::GROUP_ID)) {
      updatedAlias.bulbId.groupId = body[GroupStateFieldNames::GROUP_ID];
    }

    if (body.containsKey(GroupStateFieldNames::DEVICE_TYPE)) {
      updatedAlias.bulbId.deviceType = MiLightRemoteTypeHelpers::remoteTypeFromString(body[GroupStateFieldNames::DEVICE_TYPE].as<const char*>());
    }

    // If alias was updated, delete the old mapping
    if (strcmp(alias->second.alias, updatedAlias.alias) != 0) {
      settings.deleteAlias(id);
    }

    settings.groupIdAliases[updatedAlias.alias] = updatedAlias;
    saveSettings();

    request.response.json[F("success")] = true;
  }
}

void MiLightHttpServer::handleDeleteAliases(RequestContext &request) {
  // buffer current aliases so we can mark them all as deleted
  std::vector<GroupAlias> aliases;
  for (auto & alias : settings.groupIdAliases) {
    aliases.push_back(alias.second);
  }

  ProjectFS.remove(ALIASES_FILE);
  Settings::load(settings);

  // mark all aliases as deleted
  for (auto & alias : aliases) {
    settings.deletedGroupIdAliases[alias.bulbId.getCompactId()] = alias.bulbId;
  }

  if (this->settingsSavedHandler) {
    this->settingsSavedHandler();
  }

  request.response.json[F("success")] = true;
}

void MiLightHttpServer::handleUpdateAliases(RequestContext& request) {
  // buffer current aliases so we can mark any that were removed as deleted
  std::vector<GroupAlias> aliases;
  for (auto & alias : settings.groupIdAliases) {
    aliases.push_back(alias.second);
  }

  Settings::load(settings);

  // mark any aliases that were removed as deleted
  for (auto & alias : aliases) {
    if (settings.groupIdAliases.find(alias.alias) == settings.groupIdAliases.end()) {
      settings.deletedGroupIdAliases[alias.bulbId.getCompactId()] = alias.bulbId;
    }
  }

  saveSettings();

  request.response.json[F("success")] = true;
}

void MiLightHttpServer::saveSettings() {
  settings.save();

  if (this->settingsSavedHandler) {
    this->settingsSavedHandler();
  }
}

void MiLightHttpServer::handleRestoreBackup(RequestContext &request) {
  File backupFile = ProjectFS.open(BACKUP_FILE, "r");
  auto status = BackupManager::restoreBackup(settings, backupFile);

  if (status == BackupManager::RestoreStatus::OK) {
    request.response.json[F("success")] = true;
    request.response.json[F("message")] = F("Backup restored successfully");
  } else {
    request.response.setCode(400);
    request.response.json[F("error")] = static_cast<uint8_t>(status);
  }
}

void MiLightHttpServer::handleCreateBackup(RequestContext &request) {
  File backupFile = ProjectFS.open(BACKUP_FILE, "w");

  if (!backupFile) {
    Serial.println(F("Failed to open backup file"));
    request.response.setCode(500);
    request.response.json[F("error")] = F("Failed to open backup file");
  }

  WriteBufferingStream bufferedStream(backupFile, 64);
  BackupManager::createBackup(settings, bufferedStream);
  bufferedStream.flush();
  backupFile.close();

  backupFile = ProjectFS.open(BACKUP_FILE, "r");
  Serial.printf_P(PSTR("Sending backup file of size %d\n"), backupFile.size());
  server.streamFile(backupFile, APPLICATION_OCTET_STREAM);

  ProjectFS.remove(BACKUP_FILE);
}