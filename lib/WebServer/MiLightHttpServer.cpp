#include "MiBoxerCatalogAsset.h"
#include "RemoteStudioAsset.h"
#include <FS.h>
#include <WiFiUdp.h>
#include <IntParsing.h>
#include <Settings.h>
#include <MiLightHttpServer.h>
#include <RemoteRegistry.h>
#include <MiLightRadioConfig.h>
#include <string.h>
#include <TokenIterator.h>
#include <AboutHelper.h>
#include <GroupAlias.h>
#include <ProjectFS.h>
#include <StreamUtils.h>

#include <index.html.gz.h>
#include <bundle.css.gz.h>
#include <bundle.js.gz.h>
#include <BackupManager.h>

#ifdef ESP32
  #include <SPIFFS.h>
  #include <Update.h>
#endif

using namespace std::placeholders;

void MiLightHttpServer::begin() {
  server
    .buildHandler("/")
    .onSimple(HTTP_GET, std::bind(&MiLightHttpServer::handleServe_P, this, index_html_gz, index_html_gz_len, "text/html"));

  server
    .buildHandler("/remotes-ui")
    .onSimple(HTTP_GET, [this](const UrlTokenBindings*) { server.sendHeader("Location", "/#/remotes", true); server.send(302, "text/plain", ""); });

  server
    .buildHandler("/miboxer_catalog.js")
    .onSimple(HTTP_GET, std::bind(&MiLightHttpServer::handleServe_P, this, miboxer_catalog_js_gz, miboxer_catalog_js_gz_len, "application/javascript"));

  server
    .buildHandler("/remote-studio.js")
    .onSimple(HTTP_GET, [this](const UrlTokenBindings*) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Cache-Control", "no-cache");
      this->handleServe_P(remote_studio_js_gz, remote_studio_js_gz_len, "application/javascript");
    });

  server
    .buildHandler("/api/remotes")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleListRemotes, this, _1))
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleUpsertRemote, this, _1));

  server
    .buildHandler("/api/remotes/delete")
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleDeleteRemote, this, _1));

  server
    .buildHandler("/api/remotes/last_seen")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleLastSeenRemote, this, _1));

  server
    .buildHandler("/api/remotes/virtual_send")
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleVirtualRemoteSend, this, _1));

  server
    .buildHandler("/api/remotes/studio_meta")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleStudioMetaGet, this, _1))
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleStudioMetaPost, this, _1))
    .onSimple(HTTP_OPTIONS, [this](const UrlTokenBindings*) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(204, "text/plain", "");
    });

  server
    .buildHandler("/api/remotes/custom_models")
    .on(HTTP_GET, std::bind(&MiLightHttpServer::handleCustomModelsGet, this, _1))
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleCustomModelsPost, this, _1))
    .onSimple(HTTP_OPTIONS, [this](const UrlTokenBindings*) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(204, "text/plain", "");
    });

  server
    .buildHandler("/api/remotes/custom_models/delete")
    .on(HTTP_POST, std::bind(&MiLightHttpServer::handleCustomModelsDelete, this, _1))
    .onSimple(HTTP_OPTIONS, [this](const UrlTokenBindings*) {
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.sendHeader("Access-Control-Allow-Methods", "POST, OPTIONS");
      server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
      server.send(204, "text/plain", "");
    });

  server
    .buildHandler(bundle_css_filename)
    .onSimple(HTTP_GET, std::bind(&MiLightHttpServer::handleServe_P, this, bundle_css_gz, bundle_css_gz_len, "text/css"));

  server
    .buildHandler(bundle_js_filename)
    .onSimple(HTTP_GET, std::bind(&MiLightHttpServer::handleServe_P, this, bundle_js_gz, bundle_js_gz_len, "application/javascript"));

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
    .buildHandler("/gateways")
    .onSimple(HTTP_GET, std::bind(&MiLightHttpServer::handleListGroups, this))
    .on(HTTP_PUT, std::bind(&MiLightHttpServer::handleBatchUpdateGroups, this, _1));

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

void MiLightHttpServer::onAbout(AboutHandler handler) {
  this->aboutHandler = handler;
}

void MiLightHttpServer::handleClient() {
  server.handleClient();
  wsServer.loop();
}

WiFiClient MiLightHttpServer::client() {
  return server.client();
}

void MiLightHttpServer::on(const char *path, HTTPMethod method, THandlerFunction handler) {
  server.on(path, method, handler);
}

void MiLightHttpServer::handleSystemPost(RequestContext& request) {
  JsonObject requestBody = request.getJsonBody().as<JsonObject>();

  bool handled = false;

  if (requestBody.containsKey(GroupStateFieldNames::COMMAND)) {
    if (requestBody[GroupStateFieldNames::COMMAND] == "restart") {
      Serial.println(F("Restarting..."));
      server.send_P(200, TEXT_PLAIN, PSTR("{\"success\": true}"));

      delay(100);

      ESP.restart();

      handled = true;
    } else if (requestBody[GroupStateFieldNames::COMMAND] == "clear_wifi_config") {
      Serial.println(F("Resetting Wifi and then Restarting..."));
      server.send_P(200, TEXT_PLAIN, PSTR("{\"success\": true}"));

      delay(100);
#ifdef ESP8266
      ESP.eraseConfig();
#elif ESP32
      Serial.println(F("Wifi reset..."));
      WiFi.disconnect(true, true);
      delay(1000);
#endif
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

  if (this->aboutHandler) {
    this->aboutHandler(request.response.json);
  }

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
#ifdef ESP8266
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
#elif ESP32
  HTTPUpload &upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Serial.printf("Update: %s\n", upload.filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { // start with max available size
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) { // true to set the size to the current progress
      Serial.println("Update Success: Rebooting...");
      ESP.restart();
    } else {
      Update.printError(Serial);
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    Update.end();
    Serial.println("Update was aborted");
  }
  delay(0);
#endif
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
  bool normalizedFormat = server.arg("fmt").equalsIgnoreCase("normalized");

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
      state->applyState(obj, bulbId, normalizedFormat ? NORMALIZED_GROUP_STATE_FIELDS : settings.groupStateFields);
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

void MiLightHttpServer::handlePacketSent(uint8_t *packet, const MiLightRemoteConfig& config, const BulbId& bulbId, const JsonObject& result) {
  if (numWsClients > 0) {
    DynamicJsonDocument output(1024);

    output[F("t")] = F("packet");
    output[F("u")].set(result);

    JsonObject device = output.createNestedObject(F("d"));
    device[F("di")] = bulbId.deviceId;
    device[F("gi")] = bulbId.groupId;
    device[F("rt")] = MiLightRemoteTypeHelpers::remoteTypeToString(bulbId.deviceType);

    JsonArray responsePacket = output.createNestedArray(F("p"));
    for (size_t i = 0; i < config.packetFormatter->getPacketLength(); ++i) {
      responsePacket.add(packet[i]);
    }

    const GroupState* bulbState = this->stateStore->get(bulbId);
    if (bulbState != nullptr) {
      JsonObject state = output.createNestedObject(F("s"));
      bulbState->applyState(state, bulbId, NORMALIZED_GROUP_STATE_FIELDS);
    }

    char responseBuffer[300];
    serializeJson(output, responseBuffer);
    wsServer.broadcastTXT(reinterpret_cast<uint8_t*>(responseBuffer));
  }
}

void MiLightHttpServer::handleServe_P(const char* data, size_t length, const char* contentType) {
  const size_t CHUNK_SIZE = 4096; 

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, contentType);

  WiFiClient client = server.client();

  size_t remaining = length;
  while (remaining > 0) {
    size_t chunk = remaining > CHUNK_SIZE ? CHUNK_SIZE : remaining;
    
    // Send chunk size in hexadecimal format
    client.printf("%X\r\n", chunk);
    
    // Send chunk data
    client.write_P(data, chunk);
    client.print("\r\n");
    
    data += chunk;
    remaining -= chunk;
  }

  // Send the terminal chunk
  client.print("0\r\n\r\n");
  client.stop();
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


void MiLightHttpServer::handleListGroups() {
  this->stateStore->flush();

  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json");

  StaticJsonDocument<1024> stateBuffer;
  WiFiClient client = server.client();

  // open array
  server.sendContent("[");

  bool firstGroup = true;
  for (auto & group : settings.groupIdAliases) {
    stateBuffer.clear();

    JsonObject device = stateBuffer.createNestedObject(F("device"));

    device[F("alias")] = group.first;
    device[F("id")] = group.second.id;
    device[F("device_id")] = group.second.bulbId.deviceId;
    device[F("group_id")] = group.second.bulbId.groupId;
    device[F("device_type")] = MiLightRemoteTypeHelpers::remoteTypeToString(group.second.bulbId.deviceType);
    
    GroupState* state = this->stateStore->get(group.second.bulbId);
    JsonObject outputState = stateBuffer.createNestedObject(F("state"));

    if (state != nullptr) {
      state->applyState(outputState, group.second.bulbId, NORMALIZED_GROUP_STATE_FIELDS);
    }

    client.printf("%zx\r\n", measureJson(stateBuffer)+(firstGroup ? 0 : 1));

    if (!firstGroup) {
      client.print(',');
    }
    serializeJson(stateBuffer, client);
    client.printf_P(PSTR("\r\n"));

    firstGroup = false;
    yield();
  }

  // close array
  server.sendContent("]");

  // stop chunked streaming
  server.sendContent("");
  server.client().stop();
}

void MiLightHttpServer::handleBatchUpdateGroups(RequestContext& request) {
  JsonArray body = request.getJsonBody().as<JsonArray>();

  if (body.size() == 0) {
    request.response.setCode(400);
    request.response.json[F("error")] = F("Must specify an array of gateways and updates");
    return;
  }

  for (auto update : body) {
    JsonArray gateways = update[F("gateways")].as<JsonArray>();
    JsonObject stateUpdate = update[F("update")].as<JsonObject>();

    for (auto gateway : gateways) {
      BulbId bulbId(
        gateway[F("device_id")],
        gateway[F("group_id")],
        MiLightRemoteTypeHelpers::remoteTypeFromString(gateway[F("device_type")].as<const char*>())
      );

      this->milightClient->prepare(
        bulbId.deviceType,
        bulbId.deviceId,
        bulbId.groupId
      );
      handleRequest(stateUpdate);
    }
  }

  request.response.json[F("success")] = true;
}

static const char* STUDIO_META_FILE = "/studio_meta.json";
static const char* CUSTOM_MODELS_FILE = "/custom_models.json";

static void loadCustomModelsDoc(DynamicJsonDocument& cmDoc) {
  cmDoc.to<JsonArray>();
  if (ProjectFS.exists(CUSTOM_MODELS_FILE)) {
    File f = ProjectFS.open(CUSTOM_MODELS_FILE, "r");
    if (f) {
      DeserializationError err = deserializeJson(cmDoc, f);
      f.close();
      if (err || !cmDoc.is<JsonArray>()) {
        cmDoc.clear();
        cmDoc.to<JsonArray>();
      }
    }
  }
}

static bool saveCustomModelsDoc(const DynamicJsonDocument& cmDoc) {
  File f = ProjectFS.open(CUSTOM_MODELS_FILE, "w");
  if (!f) return false;
  serializeJson(cmDoc, f);
  f.close();
  return true;
}

static String normalizeHexKey(uint16_t devId, const char* rawHex = nullptr) {
  if (devId > 0) {
    char buf[10];
    snprintf(buf, sizeof(buf), "0x%04X", devId);
    return String(buf);
  }
  if (rawHex && rawHex[0]) {
    uint32_t parsed = strtoul(rawHex, nullptr, 16);
    if (parsed > 0 && parsed <= 0xFFFF) {
      char buf[10];
      snprintf(buf, sizeof(buf), "0x%04X", (uint16_t)parsed);
      return String(buf);
    }
  }
  return String("");
}

static void loadStudioMetaDoc(DynamicJsonDocument& metaDoc) {
  metaDoc.to<JsonObject>();
  if (ProjectFS.exists(STUDIO_META_FILE)) {
    File f = ProjectFS.open(STUDIO_META_FILE, "r");
    if (f) {
      DeserializationError err = deserializeJson(metaDoc, f);
      f.close();
      if (err || !metaDoc.is<JsonObject>()) {
        metaDoc.clear();
        metaDoc.to<JsonObject>();
      }
    }
  }
}

static bool saveStudioMetaDoc(const DynamicJsonDocument& metaDoc) {
  File f = ProjectFS.open(STUDIO_META_FILE, "w");
  if (!f) return false;
  serializeJson(metaDoc, f);
  f.close();
  return true;
}

void MiLightHttpServer::handleListRemotes(RequestContext& request) {
  DynamicJsonDocument doc(24576);
  JsonArray arr = doc.to<JsonArray>();
  RemoteRegistry::instance().serializeRemotes(arr);

  DynamicJsonDocument metaDoc(16384);
  loadStudioMetaDoc(metaDoc);
  JsonObject metaRoot = metaDoc.as<JsonObject>();

  DynamicJsonDocument cmDoc(24576);
  loadCustomModelsDoc(cmDoc);
  JsonArray cmArr = cmDoc.as<JsonArray>();

  for (JsonObject remObj : arr) {
    uint16_t devId = remObj["device_id"] | 0;
    String vm = String(remObj["visual_model"] | "");
    for (JsonObject cm : cmArr) {
      if (vm.length() > 0 && vm == String(cm["id"] | "")) {
        remObj["base_model"] = cm["base_model"] | "B8";
        remObj["is_custom_model"] = true;
        if (cm.containsKey("facia")) remObj["facia"] = cm["facia"];
        break;
      }
    }
    String hx = normalizeHexKey(devId, remObj["hex_id"] | "");
    if (hx.length() > 0 && metaRoot.containsKey(hx)) {
      JsonObject m = metaRoot[hx].as<JsonObject>();
      if (!remObj.containsKey("facia") && m.containsKey("facia")) remObj["facia"] = m["facia"];
      if (m.containsKey("calibration")) remObj["calibration"] = m["calibration"];
      if (m.containsKey("linked_device_id")) remObj["linked_device_id"] = m["linked_device_id"];
      if (m.containsKey("linked_hex_id")) remObj["linked_hex_id"] = m["linked_hex_id"];
    }
  }

  String out;
  serializeJson(doc, out);
  request.response.setCode(200);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, APPLICATION_JSON, out.c_str());
}

void MiLightHttpServer::handleUpsertRemote(RequestContext& request) {
  JsonObject body = request.getJsonBody().as<JsonObject>();
  uint16_t devId = body["device_id"] | (body["id"] | 0);
  if (devId == 0) {
    const char* hxStr = body["hex_id"] | (body["hex"] | "");
    if (hxStr && hxStr[0]) {
      devId = (uint16_t)strtoul(hxStr, nullptr, 16);
      if (devId > 0) body["device_id"] = devId;
    }
  }

  RegisteredRemote item;
  const RegisteredRemote* existing = RemoteRegistry::instance().find(devId);
  if (existing != nullptr) {
    item = *existing; // Preserve existing gestures & touch_zones if caller only updates name/model!
  }
  RemoteRegistry::parseRemoteFromJson(body, item);
  if (item.deviceId == 0 && devId > 0) item.deviceId = devId;

  bool ok = RemoteRegistry::instance().upsert(item);

  // Persist optional studio metadata (facia & calibration) alongside remote upsert
  if (ok && (body.containsKey("facia") || body.containsKey("calibration"))) {
    String hx = normalizeHexKey(item.deviceId);
    if (hx.length() > 0) {
      DynamicJsonDocument metaDoc(16384);
      loadStudioMetaDoc(metaDoc);
      JsonObject metaRoot = metaDoc.as<JsonObject>();
      JsonObject entry = metaRoot.containsKey(hx) ? metaRoot[hx].as<JsonObject>() : metaRoot.createNestedObject(hx);
      if (body.containsKey("facia")) {
        if (body["facia"].isNull()) entry.remove("facia");
        else entry["facia"] = body["facia"];
      }
      if (body.containsKey("calibration")) {
        if (body["calibration"].isNull()) entry.remove("calibration");
        else entry["calibration"] = body["calibration"];
      }
      saveStudioMetaDoc(metaDoc);
    }
  }

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(ok ? 200 : 400, APPLICATION_JSON, ok ? "{\"success\":true}" : "{\"success\":false}");
}

void MiLightHttpServer::handleDeleteRemote(RequestContext& request) {
  JsonObject body = request.getJsonBody().as<JsonObject>();
  uint16_t devId = body["device_id"] | 0;
  if (devId == 0) {
    const char* hxStr = body["hex_id"] | (body["hex"] | "");
    if (hxStr && hxStr[0]) devId = (uint16_t)strtoul(hxStr, nullptr, 16);
  }
  bool ok = RemoteRegistry::instance().remove(devId);
  if (devId > 0) {
    String hx = normalizeHexKey(devId);
    DynamicJsonDocument metaDoc(16384);
    loadStudioMetaDoc(metaDoc);
    JsonObject metaRoot = metaDoc.as<JsonObject>();
    if (metaRoot.containsKey(hx)) {
      metaRoot.remove(hx);
      saveStudioMetaDoc(metaDoc);
    }
  }
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, APPLICATION_JSON, ok ? "{\"success\":true}" : "{\"success\":false}");
}

void MiLightHttpServer::handleStudioMetaGet(RequestContext& request) {
  DynamicJsonDocument metaDoc(16384);
  loadStudioMetaDoc(metaDoc);
  String out;
  serializeJson(metaDoc, out);
  request.response.setCode(200);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, APPLICATION_JSON, out.c_str());
}


static String normalizeShellBase(const String& modelId) {
  if (modelId.endsWith("-B") && modelId.length() > 2) {
    return modelId.substring(0, modelId.length() - 2);
  }
  return modelId;
}

static bool areShellsCompatibleServer(const String& customVisualModel, const DynamicJsonDocument& cmDoc, uint16_t physDevId) {
  if (physDevId == 0) return true;
  String reqBase = "";
  if (customVisualModel == "CUST_B8_3D_ESTATE") reqBase = "B8";
  else if (customVisualModel == "CUST_B0_3D_SUITE") reqBase = "B0";
  else if (customVisualModel == "CUST_B4_3D_HYBRID") reqBase = "B4";
  if (reqBase.length() == 0 && cmDoc.is<JsonArray>()) {
    for (JsonObjectConst cm : cmDoc.as<JsonArrayConst>()) {
      if (String(cm["id"] | "") == customVisualModel) {
        reqBase = String(cm["base_model"] | "");
        break;
      }
    }
  }
  if (reqBase.length() == 0) return true;
  for (const auto& r : RemoteRegistry::instance().remotes) {
    if (r.deviceId == physDevId) {
      if (r.visualModel.startsWith("CUST_")) return false;
      return normalizeShellBase(reqBase) == normalizeShellBase(r.visualModel);
    }
  }
  return true;
}

void MiLightHttpServer::handleStudioMetaPost(RequestContext& request) {
  JsonVariant bodyVar = request.getJsonBody();
  DynamicJsonDocument metaDoc(16384);
  loadStudioMetaDoc(metaDoc);
  JsonObject metaRoot = metaDoc.as<JsonObject>();

  if (bodyVar.is<JsonObject>()) {
    JsonObject body = bodyVar.as<JsonObject>();
    const char* singleHex = body["hex_id"] | (body["hex"] | "");
    uint16_t devId = body["device_id"] | 0;
    if ((singleHex && singleHex[0]) || devId > 0) {
      String hx = normalizeHexKey(devId, singleHex);
      if (hx.length() > 0) {
        JsonObject entry = metaRoot.containsKey(hx) ? metaRoot[hx].as<JsonObject>() : metaRoot.createNestedObject(hx);
        if (body.containsKey("facia")) {
          if (body["facia"].isNull()) entry.remove("facia");
          else entry["facia"] = body["facia"];
        }
        if (body.containsKey("calibration")) {
          if (body["calibration"].isNull()) entry.remove("calibration");
          else entry["calibration"] = body["calibration"];
        }
    if (body.containsKey("linked_device_id")) {
      uint16_t linkId = body["linked_device_id"] | 0;
      if (linkId == 0 || body["linked_device_id"].isNull()) {
        entry.remove("linked_device_id");
        entry.remove("linked_hex_id");
      } else {
        String cVis = "";
        for (const auto& r : RemoteRegistry::instance().remotes) {
          if (r.deviceId == devId || normalizeHexKey(r.deviceId, "") == hx) { cVis = r.visualModel; break; }
        }
        DynamicJsonDocument cmCheck(24576);
        loadCustomModelsDoc(cmCheck);
        if (!areShellsCompatibleServer(cVis, cmCheck, linkId)) {
          server.sendHeader("Access-Control-Allow-Origin", "*");
          server.send(400, APPLICATION_JSON, R"({"success":false,"error":"Incompatible physical remote shell for this custom stencil"})");
          return;
        }
        entry["linked_device_id"] = linkId;
        entry["linked_hex_id"] = String(body["linked_hex_id"] | "");
      }
    }
      }
    } else {
      // Bulk map merge: { "0x4542": { "facia": ..., "calibration": ... } }
      for (JsonPair kv : body) {
        String hx = normalizeHexKey(0, kv.key().c_str());
        if (hx.length() > 0 && kv.value().is<JsonObject>()) {
          JsonObject src = kv.value().as<JsonObject>();
          JsonObject entry = metaRoot.containsKey(hx) ? metaRoot[hx].as<JsonObject>() : metaRoot.createNestedObject(hx);
          if (src.containsKey("facia")) {
            if (src["facia"].isNull()) entry.remove("facia");
            else entry["facia"] = src["facia"];
          }
          if (src.containsKey("calibration")) {
            if (src["calibration"].isNull()) entry.remove("calibration");
            else entry["calibration"] = src["calibration"];
          }
        }
      }
    }
  }
  bool ok = saveStudioMetaDoc(metaDoc);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(ok ? 200 : 500, APPLICATION_JSON, ok ? "{\"success\":true}" : "{\"success\":false}");
}

void MiLightHttpServer::handleCustomModelsGet(RequestContext& request) {
  DynamicJsonDocument cmDoc(24576);
  loadCustomModelsDoc(cmDoc);
  String out;
  serializeJson(cmDoc, out);
  request.response.setCode(200);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, APPLICATION_JSON, out.c_str());
}

void MiLightHttpServer::handleCustomModelsPost(RequestContext& request) {
  JsonObject body = request.getJsonBody().as<JsonObject>();
  String id = String(body["id"] | "");
  String baseModel = String(body["base_model"] | (body["baseModel"] | ""));
  if (id.length() == 0 || baseModel.length() == 0 || !id.startsWith("CUST_")) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, APPLICATION_JSON, "{\"success\":false,\"error\":\"Custom model id must start with CUST_ and specify base_model\"}");
    return;
  }

  DynamicJsonDocument cmDoc(24576);
  loadCustomModelsDoc(cmDoc);
  JsonArray arr = cmDoc.as<JsonArray>();

  bool updated = false;
  for (JsonObject item : arr) {
    if (String(item["id"] | "") == id) {
      item["name"] = body["name"] | id;
      item["base_model"] = baseModel;
      item["stencil_mode"] = body["stencil_mode"] | "full_panel";
      if (body.containsKey("facia")) item["facia"] = body["facia"];
      if (body.containsKey("touch_zones")) item["touch_zones"] = body["touch_zones"];
      updated = true;
      break;
    }
  }

  if (!updated) {
    JsonObject created = arr.createNestedObject();
    created["id"] = id;
    created["name"] = body["name"] | id;
    created["base_model"] = baseModel;
    created["stencil_mode"] = body["stencil_mode"] | "full_panel";
    if (body.containsKey("facia")) created["facia"] = body["facia"];
    if (body.containsKey("touch_zones")) created["touch_zones"] = body["touch_zones"];
  }

  bool ok = saveCustomModelsDoc(cmDoc);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(ok ? 200 : 500, APPLICATION_JSON, ok ? "{\"success\":true}" : "{\"success\":false}");
}

void MiLightHttpServer::handleCustomModelsDelete(RequestContext& request) {
  JsonObject body = request.getJsonBody().as<JsonObject>();
  String id = String(body["id"] | "");
  if (id.length() == 0 || !id.startsWith("CUST_")) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(400, APPLICATION_JSON, "{\"success\":false,\"error\":\"Default factory models are read-only\"}");
    return;
  }

  DynamicJsonDocument cmDoc(24576);
  loadCustomModelsDoc(cmDoc);
  JsonArray arr = cmDoc.as<JsonArray>();

  String fallbackBase = "B8";
  bool removed = false;
  for (size_t i = 0; i < arr.size(); i++) {
    JsonObject item = arr[i].as<JsonObject>();
    if (String(item["id"] | "") == id) {
      fallbackBase = String(item["base_model"] | "B8");
      arr.remove(i);
      removed = true;
      break;
    }
  }

  if (removed) {
    saveCustomModelsDoc(cmDoc);
    // Safe Fallback: Revert any bound RegisteredRemote using this custom model back to its factory base_model!
    bool regChanged = false;
    for (auto& rem : RemoteRegistry::instance().remotes) {
      if (rem.visualModel == id) {
        rem.visualModel = fallbackBase;
        regChanged = true;
      }
    }
    if (regChanged) {
      RemoteRegistry::instance().save();
      RemoteRegistry::instance().publishDiscovery();
    }
  }

  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, APPLICATION_JSON, removed ? "{\"success\":true}" : "{\"success\":false}");
}

void MiLightHttpServer::handleLastSeenRemote(RequestContext& request) {
  DynamicJsonDocument doc(4096);
  JsonObject obj = doc.to<JsonObject>();
  RemoteRegistry::instance().serializeLastSeen(obj);
  String out;
  serializeJson(doc, out);
  server.send(200, APPLICATION_JSON, out.c_str());
}

void MiLightHttpServer::handleVirtualRemoteSend(RequestContext& request) {
  JsonObject body = request.getJsonBody().as<JsonObject>();
  uint16_t devId = body["device_id"] | 0x125C;
  String rType = body["remote_type"] | "fut089";
  uint8_t grp = body["group_id"] | 1;
  String action = body["action"] | "on";
  int32_t val = body["value"] | 0;

  RemoteRegistry::instance().virtualSend(devId, rType, grp, action, val);
  server.send(200, APPLICATION_JSON, "{\"success\":true}");
}
