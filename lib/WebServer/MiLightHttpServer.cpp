#include <FS.h>
#include <WiFiUdp.h>
#include <IntParsing.h>
#include <Settings.h>
#include <MiLightHttpServer.h>
#include <MiLightRadioConfig.h>
#include <GithubClient.h>
#include <string.h>
#include <TokenIterator.h>

void MiLightHttpServer::begin() {
  applySettings(settings);

  server.on("/", HTTP_GET, handleServeFile(WEB_INDEX_FILENAME, "text/html", DEFAULT_INDEX_PAGE));
  server.on("/settings", HTTP_GET, handleServeFile(SETTINGS_FILE, "application/json"));
  server.on("/settings", HTTP_PUT, [this]() { handleUpdateSettings(); });
  server.on("/settings", HTTP_POST, [this]() { server.send(200, "text/plain", "success"); }, handleUpdateFile(SETTINGS_FILE));
  server.on("/radio_configs", HTTP_GET, [this]() { handleGetRadioConfigs(); });
  server.onPattern("/gateway_traffic/:type", HTTP_GET, [this](const UrlTokenBindings* b) { handleListenGateway(b); });
  server.onPattern("/gateways/:device_id/:type/:group_id", HTTP_ANY, [this](const UrlTokenBindings* b) { handleUpdateGroup(b); });
  server.onPattern("/raw_commands/:type", HTTP_ANY, [this](const UrlTokenBindings* b) { handleSendRaw(b); });
  server.onPattern("/download_update/:component", HTTP_GET, [this](const UrlTokenBindings* b) { handleDownloadUpdate(b); });
  server.on("/web", HTTP_POST, [this]() { server.send(200, "text/plain", "success"); }, handleUpdateFile(WEB_INDEX_FILENAME));
  server.on("/about", HTTP_GET, [this]() { handleAbout(); });
  server.on("/latest_release", HTTP_GET, [this]() { handleGetLatestRelease(); });
  server.on("/system", HTTP_POST, [this]() { handleSystemPost(); });
  server.on("/firmware", HTTP_POST,
    [this](){
      server.sendHeader("Connection", "close");
      server.sendHeader("Access-Control-Allow-Origin", "*");

      if (Update.hasError()) {
        server.send_P(
          500,
          "text/plain",
          PSTR("Failed updating firmware. Check serial logs for more information. You may need to re-flash the device.")
        );
      } else {
        server.send_P(
          200,
          "text/plain",
          PSTR("Success. Device will now reboot.")
        );
      }

      ESP.restart();
    },
    [this](){
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
  );

  server.begin();
}

void MiLightHttpServer::handleGetLatestRelease() {
  GithubClient client = GithubClient::apiClient();
  String path = GithubClient::buildApiRequest(
    MILIGHT_GITHUB_USER,
    MILIGHT_GITHUB_REPO,
    "/releases/latest"
  );

  Serial.println(path);

  // This is an ugly hack, but probably not worth optimizing. The nice way
  // to do this would be to extract the content len from GitHub's response
  // and stream the body to the server directly. But this would require parsing
  // headers in the response from GitHub, which seems like more trouble than
  // it's worth.
  const String& fsPath = "/_cv.json";
  size_t tries = 0;

  while (tries++ < MAX_DOWNLOAD_ATTEMPTS && !client.download(path, fsPath)) {
    Serial.println(F("Failed download attempt."));
  }

  if (!SPIFFS.exists(fsPath)) {
    server.send_P(500, "text/plain", PSTR("Failed to stream API request from GitHub. Check Serial logs for more information."));
    return;
  }

  File file = SPIFFS.open(fsPath, "r");
  server.streamFile(file, "application/json");
  SPIFFS.remove(fsPath);
}

void MiLightHttpServer::handleClient() {
  server.handleClient();
}

void MiLightHttpServer::handleSystemPost() {
  DynamicJsonBuffer buffer;
  JsonObject& request = buffer.parse(server.arg("plain"));

  bool handled = false;

  if (request.containsKey("command")) {
    if (request["command"] == "restart") {
      Serial.println(F("Restarting..."));
      server.send(200, "text/plain", "true");

      delay(100);

      ESP.restart();
    }
  }

  if (handled) {
    server.send(200, "text/plain", "true");
  } else {
    server.send(400, "text/plain", F("{\"error\":\"Unhandled command\"}"));
  }
}

void MiLightHttpServer::handleDownloadUpdate(const UrlTokenBindings* bindings) {
  GithubClient downloader = GithubClient::rawDownloader();
  const String& component = bindings->get("component");

  if (component.equalsIgnoreCase("web")) {
    Serial.println(F("Attempting to update web UI..."));

    bool result = false;
    size_t tries = 0;

    while (!result && tries++ <= MAX_DOWNLOAD_ATTEMPTS) {
      printf("building url\n");
      String urlPath = GithubClient::buildRepoPath(
        MILIGHT_GITHUB_USER,
        MILIGHT_GITHUB_REPO,
        MILIGHT_REPO_WEB_PATH
      );

      printf("URL: %s\n", urlPath.c_str());

      result = downloader.download(urlPath, WEB_INDEX_FILENAME);
    }

    Serial.println(F("Download complete!"));

    if (result) {
      server.sendHeader("Location", "/");
      server.send(302);
    } else {
      server.send(500, "text/plain", F("Failed to download update from Github. Check serial logs for more information."));
    }
  } else {
    String body = String("Unknown component: ") + component;
    server.send(400, "text/plain", body);
  }
}

void MiLightHttpServer::applySettings(Settings& settings) {
  if (settings.hasAuthSettings()) {
    server.requireAuthentication(settings.adminUsername, settings.adminPassword);
  } else {
    server.disableAuthentication();
  }

  milightClient->setResendCount(settings.packetRepeats);
}

void MiLightHttpServer::onSettingsSaved(SettingsSavedHandler handler) {
  this->settingsSavedHandler = handler;
}

void MiLightHttpServer::handleAbout() {
  DynamicJsonBuffer buffer;
  JsonObject& response = buffer.createObject();

  response["version"] = QUOTE(MILIGHT_HUB_VERSION);
  response["variant"] = QUOTE(FIRMWARE_VARIANT);

  String body;
  response.printTo(body);

  server.send(200, "application", body);
}

void MiLightHttpServer::handleGetRadioConfigs() {
  DynamicJsonBuffer buffer;
  JsonArray& arr = buffer.createArray();

  for (size_t i = 0; i < MiLightRadioConfig::NUM_CONFIGS; i++) {
    const MiLightRadioConfig* config = MiLightRadioConfig::ALL_CONFIGS[i];
    arr.add(config->name);
  }

  String body;
  arr.printTo(body);

  server.send(200, "application/json", body);
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
    this->settingsSavedHandler();

    server.send(200, "application/json", "true");
  } else {
    server.send(400, "application/json", "\"Invalid JSON\"");
  }
}

void MiLightHttpServer::handleListenGateway(const UrlTokenBindings* bindings) {
  bool available = false;
  MiLightRadioConfig* config = MiLightRadioConfig::fromString(bindings->get("type"));

  if (config == NULL) {
    String body = "Unknown device type: ";
    body += bindings->get("type");

    server.send(400, "text/plain", body);
    return;
  }

  milightClient->prepare(*config, 0, 0);

  while (!available) {
    if (!server.clientConnected()) {
      return;
    }

    if (milightClient->available()) {
      available = true;
    }

    yield();
  }

  uint8_t packet[config->getPacketLength()];
  milightClient->read(packet);

  char response[200];
  char* responseBuffer = response;

  responseBuffer += sprintf(responseBuffer, "\nPacket received (%d bytes):\n", sizeof(packet));
  milightClient->formatPacket(packet, responseBuffer);

  server.send(200, "text/plain", response);
}

void MiLightHttpServer::handleUpdateGroup(const UrlTokenBindings* urlBindings) {
  DynamicJsonBuffer buffer;
  JsonObject& request = buffer.parse(server.arg("plain"));

  if (!request.success()) {
    server.send(400, "text/plain", F("Invalid JSON"));
    return;
  }

  milightClient->setResendCount(
    settings.httpRepeatFactor * settings.packetRepeats
  );

  String _deviceIds = urlBindings->get("device_id");
  String _groupIds = urlBindings->get("group_id");
  String _radioTypes = urlBindings->get("type");
  char deviceIds[_deviceIds.length()];
  char groupIds[_groupIds.length()];
  char radioTypes[_radioTypes.length()];
  strcpy(radioTypes, _radioTypes.c_str());
  strcpy(groupIds, _groupIds.c_str());
  strcpy(deviceIds, _deviceIds.c_str());

  TokenIterator deviceIdItr(deviceIds, _deviceIds.length());
  TokenIterator groupIdItr(groupIds, _groupIds.length());
  TokenIterator radioTypesItr(radioTypes, _radioTypes.length());

  while (radioTypesItr.hasNext()) {
    const char* _radioType = radioTypesItr.nextToken();
    MiLightRadioConfig* config = MiLightRadioConfig::fromString(_radioType);

    if (config == NULL) {
      String body = "Unknown device type: ";
      body += String(_radioType);
      server.send(400, "text/plain", body);
      return;
    }

    deviceIdItr.reset();
    while (deviceIdItr.hasNext()) {
      const uint16_t deviceId = parseInt<uint16_t>(deviceIdItr.nextToken());

      groupIdItr.reset();
      while (groupIdItr.hasNext()) {
        const uint8_t groupId = atoi(groupIdItr.nextToken());
        printf("%d,%d,%d\n",config->type,deviceId,groupId);

        milightClient->prepare(*config, deviceId, groupId);
        handleRequest(request);
      }
    }
  }

  server.send(200, "application/json", "true");
}

void MiLightHttpServer::handleRequest(const JsonObject& request) {
  if (request.containsKey("status")) {
    const String& statusStr = request.get<String>("status");
    MiLightStatus status = (statusStr == "on" || statusStr == "true") ? ON : OFF;
    milightClient->updateStatus(status);
  }

  if (request.containsKey("command")) {
    if (request["command"] == "unpair") {
      milightClient->unpair();
    }

    if (request["command"] == "pair") {
      milightClient->pair();
    }

    if (request["command"] == "set_white") {
      milightClient->updateColorWhite();
    }

    if (request["command"] == "level_up") {
      milightClient->increaseBrightness();
    }

    if (request["command"] == "level_down") {
      milightClient->decreaseBrightness();
    }

    if (request["command"] == "temperature_up") {
      milightClient->increaseTemperature();
    }

    if (request["command"] == "temperature_down") {
      milightClient->decreaseTemperature();
    }

    if (request["command"] == "next_mode") {
      milightClient->nextMode();
    }

    if (request["command"] == "previous_mode") {
      milightClient->previousMode();
    }

    if (request["command"] == "mode_speed_down") {
      milightClient->modeSpeedDown();
    }

    if (request["command"] == "mode_speed_up") {
      milightClient->modeSpeedUp();
    }
  }

  if (request.containsKey("hue")) {
    milightClient->updateHue(request["hue"]);
  }

  if (request.containsKey("level")) {
    milightClient->updateBrightness(request["level"]);
  }

  if (request.containsKey("temperature")) {
    milightClient->updateTemperature(request["temperature"]);
  }

  if (request.containsKey("saturation")) {
    milightClient->updateSaturation(request["saturation"]);
  }

  if (request.containsKey("mode")) {
    milightClient->updateMode(request["mode"]);
  }

  milightClient->setResendCount(settings.packetRepeats);
}

void MiLightHttpServer::handleSendRaw(const UrlTokenBindings* bindings) {
  DynamicJsonBuffer buffer;
  JsonObject& request = buffer.parse(server.arg("plain"));
  MiLightRadioConfig* config = MiLightRadioConfig::fromString(bindings->get("type"));

  if (config == NULL) {
    String body = "Unknown device type: ";
    body += bindings->get("type");

    server.send(400, "text/plain", body);
    return;
  }

  uint8_t packet[config->getPacketLength()];
  const String& hexPacket = request["packet"];
  hexStrToBytes<uint8_t>(hexPacket.c_str(), hexPacket.length(), packet, config->getPacketLength());

  size_t numRepeats = MILIGHT_DEFAULT_RESEND_COUNT;
  if (request.containsKey("num_repeats")) {
    numRepeats = request["num_repeats"];
  }

  milightClient->prepare(*config, 0, 0);

  for (size_t i = 0; i < numRepeats; i++) {
    milightClient->write(packet);
  }

  server.send(200, "text/plain", "true");
}
