#include <Settings.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <IntParsing.h>
#include <algorithm>

#define PORT_POSITION(s) ( s.indexOf(':') )

bool Settings::hasAuthSettings() {
  return adminUsername.length() > 0 && adminPassword.length() > 0;
}

bool Settings::isAutoRestartEnabled() {
  return _autoRestartPeriod > 0;
}

size_t Settings::getAutoRestartPeriod() {
  if (_autoRestartPeriod == 0) {
    return 0;
  }

  return std::max(_autoRestartPeriod, static_cast<size_t>(MINIMUM_RESTART_PERIOD));
}

void Settings::deserialize(Settings& settings, String json) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& parsedSettings = jsonBuffer.parseObject(json);
  settings.patch(parsedSettings);
}

void Settings::updateDeviceIds(JsonArray& arr) {
  if (arr.success()) {
    if (this->deviceIds) {
      delete this->deviceIds;
    }

    this->deviceIds = new uint16_t[arr.size()];
    this->numDeviceIds = arr.size();
    arr.copyTo(this->deviceIds, arr.size());
  }
}

void Settings::updateGatewayConfigs(JsonArray& arr) {
  if (arr.success()) {
    if (this->gatewayConfigs) {
      delete[] this->gatewayConfigs;
    }

    this->gatewayConfigs = new GatewayConfig*[arr.size()];
    this->numGatewayConfigs = arr.size();

    for (size_t i = 0; i < arr.size(); i++) {
      JsonArray& params = arr[i];

      if (params.success() && params.size() == 3) {
        this->gatewayConfigs[i] = new GatewayConfig(parseInt<uint16_t>(params[0]), params[1], params[2]);
      } else {
        Serial.print(F("Settings - skipped parsing gateway ports settings for element #"));
        Serial.println(i);
      }
    }
  }
}

void Settings::updateGroupStateFields(JsonArray &arr) {
  if (arr.success()) {
    if (this->groupStateFields) {
      delete this->groupStateFields;
    }

    this->groupStateFields = new GroupStateField[arr.size()];
    this->numGroupStateFields = arr.size();

    for (size_t i = 0; i < arr.size(); i++) {
      String name = arr[i];
      name.toLowerCase();

      this->groupStateFields[i] = GroupStateFieldHelpers::getFieldByName(name.c_str());
    }
  }
}

void Settings::patch(JsonObject& parsedSettings) {
  if (parsedSettings.success()) {
    this->setIfPresent<String>(parsedSettings, "admin_username", adminUsername);
    this->setIfPresent(parsedSettings, "admin_password", adminPassword);
    this->setIfPresent(parsedSettings, "ce_pin", cePin);
    this->setIfPresent(parsedSettings, "csn_pin", csnPin);
    this->setIfPresent(parsedSettings, "reset_pin", resetPin);
    this->setIfPresent(parsedSettings, "led_pin", ledPin);
    this->setIfPresent(parsedSettings, "packet_repeats", packetRepeats);
    this->setIfPresent(parsedSettings, "http_repeat_factor", httpRepeatFactor);
    this->setIfPresent(parsedSettings, "auto_restart_period", _autoRestartPeriod);
    this->setIfPresent(parsedSettings, "mqtt_server", _mqttServer);
    this->setIfPresent(parsedSettings, "mqtt_username", mqttUsername);
    this->setIfPresent(parsedSettings, "mqtt_password", mqttPassword);
    this->setIfPresent(parsedSettings, "mqtt_topic_pattern", mqttTopicPattern);
    this->setIfPresent(parsedSettings, "mqtt_update_topic_pattern", mqttUpdateTopicPattern);
    this->setIfPresent(parsedSettings, "mqtt_state_topic_pattern", mqttStateTopicPattern);
    this->setIfPresent(parsedSettings, "discovery_port", discoveryPort);
    this->setIfPresent(parsedSettings, "listen_repeats", listenRepeats);
    this->setIfPresent(parsedSettings, "state_flush_interval", stateFlushInterval);
    this->setIfPresent(parsedSettings, "mqtt_state_rate_limit", mqttStateRateLimit);
    this->setIfPresent(parsedSettings, "packet_repeat_throttle_threshold", packetRepeatThrottleThreshold);
    this->setIfPresent(parsedSettings, "packet_repeat_throttle_sensitivity", packetRepeatThrottleSensitivity);
    this->setIfPresent(parsedSettings, "packet_repeat_minimum", packetRepeatMinimum);
    this->setIfPresent(parsedSettings, "enable_automatic_mode_switching", enableAutomaticModeSwitching);
    this->setIfPresent(parsedSettings, "led_mode_packet_count", ledModePacketCount);

    if (parsedSettings.containsKey("led_mode_wifi_config")) {
      this->ledModeWifiConfig = LEDStatus::stringToLEDMode(parsedSettings["led_mode_wifi_config"]);
    }

    if (parsedSettings.containsKey("led_mode_wifi_failed")) {
      this->ledModeWifiFailed = LEDStatus::stringToLEDMode(parsedSettings["led_mode_wifi_failed"]);
    }

    if (parsedSettings.containsKey("led_mode_operating")) {
      this->ledModeOperating = LEDStatus::stringToLEDMode(parsedSettings["led_mode_operating"]);
    }

    if (parsedSettings.containsKey("led_mode_packet")) {
      this->ledModePacket = LEDStatus::stringToLEDMode(parsedSettings["led_mode_packet"]);
    }

    if (parsedSettings.containsKey("radio_interface_type")) {
      this->radioInterfaceType = Settings::typeFromString(parsedSettings["radio_interface_type"]);
    }

    if (parsedSettings.containsKey("device_ids")) {
      JsonArray& arr = parsedSettings["device_ids"];
      updateDeviceIds(arr);
    }
    if (parsedSettings.containsKey("gateway_configs")) {
      JsonArray& arr = parsedSettings["gateway_configs"];
      updateGatewayConfigs(arr);
    }
    if (parsedSettings.containsKey("group_state_fields")) {
      JsonArray& arr = parsedSettings["group_state_fields"];
      updateGroupStateFields(arr);
    }
  }
}

void Settings::load(Settings& settings) {
  if (SPIFFS.exists(SETTINGS_FILE)) {
    File f = SPIFFS.open(SETTINGS_FILE, "r");
    String settingsContents = f.readStringUntil(SETTINGS_TERMINATOR);
    f.close();

    deserialize(settings, settingsContents);
  } else {
    settings.save();
  }
}

String Settings::toJson(const bool prettyPrint) {
  String buffer = "";
  StringStream s(buffer);
  serialize(s, prettyPrint);
  return buffer;
}

void Settings::save() {
  File f = SPIFFS.open(SETTINGS_FILE, "w");

  if (!f) {
    Serial.println(F("Opening settings file failed"));
  } else {
    serialize(f);
    f.close();
  }
}

void Settings::serialize(Stream& stream, const bool prettyPrint) {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["admin_username"] = this->adminUsername;
  root["admin_password"] = this->adminPassword;
  root["ce_pin"] = this->cePin;
  root["csn_pin"] = this->csnPin;
  root["reset_pin"] = this->resetPin;
  root["led_pin"] = this->ledPin;
  root["radio_interface_type"] = typeToString(this->radioInterfaceType);
  root["packet_repeats"] = this->packetRepeats;
  root["http_repeat_factor"] = this->httpRepeatFactor;
  root["auto_restart_period"] = this->_autoRestartPeriod;
  root["mqtt_server"] = this->_mqttServer;
  root["mqtt_username"] = this->mqttUsername;
  root["mqtt_password"] = this->mqttPassword;
  root["mqtt_topic_pattern"] = this->mqttTopicPattern;
  root["mqtt_update_topic_pattern"] = this->mqttUpdateTopicPattern;
  root["mqtt_state_topic_pattern"] = this->mqttStateTopicPattern;
  root["discovery_port"] = this->discoveryPort;
  root["listen_repeats"] = this->listenRepeats;
  root["state_flush_interval"] = this->stateFlushInterval;
  root["mqtt_state_rate_limit"] = this->mqttStateRateLimit;
  root["packet_repeat_throttle_sensitivity"] = this->packetRepeatThrottleSensitivity;
  root["packet_repeat_throttle_threshold"] = this->packetRepeatThrottleThreshold;
  root["packet_repeat_minimum"] = this->packetRepeatMinimum;
  root["enable_automatic_mode_switching"] = this->enableAutomaticModeSwitching;
  root["led_mode_wifi_config"] = LEDStatus::LEDModeToString(this->ledModeWifiConfig);
  root["led_mode_wifi_failed"] = LEDStatus::LEDModeToString(this->ledModeWifiFailed);
  root["led_mode_operating"] = LEDStatus::LEDModeToString(this->ledModeOperating);
  root["led_mode_packet"] = LEDStatus::LEDModeToString(this->ledModePacket);
  root["led_mode_packet_count"] = this->ledModePacketCount;

  if (this->deviceIds) {
    JsonArray& arr = jsonBuffer.createArray();
    arr.copyFrom(this->deviceIds, this->numDeviceIds);
    root["device_ids"] = arr;
  }

  if (this->gatewayConfigs) {
    JsonArray& arr = jsonBuffer.createArray();
    for (size_t i = 0; i < this->numGatewayConfigs; i++) {
      JsonArray& elmt = jsonBuffer.createArray();
      elmt.add(this->gatewayConfigs[i]->deviceId);
      elmt.add(this->gatewayConfigs[i]->port);
      elmt.add(this->gatewayConfigs[i]->protocolVersion);
      arr.add(elmt);
    }

    root["gateway_configs"] = arr;
  }

  if (this->groupStateFields) {
    JsonArray& arr = jsonBuffer.createArray();
    for (size_t i = 0; i < this->numGroupStateFields; i++) {
      arr.add(GroupStateFieldHelpers::getFieldName(this->groupStateFields[i]));
    }

    root["group_state_fields"] = arr;
  }

  if (prettyPrint) {
    root.prettyPrintTo(stream);
  } else {
    root.printTo(stream);
  }
}

String Settings::mqttServer() {
  int pos = PORT_POSITION(_mqttServer);

  if (pos == -1) {
    return _mqttServer;
  } else {
    return _mqttServer.substring(0, pos);
  }
}

uint16_t Settings::mqttPort() {
  int pos = PORT_POSITION(_mqttServer);

  if (pos == -1) {
    return DEFAULT_MQTT_PORT;
  } else {
    return atoi(_mqttServer.c_str() + pos + 1);
  }
}

RadioInterfaceType Settings::typeFromString(const String& s) {
  if (s.equalsIgnoreCase("lt8900")) {
    return LT8900;
  } else {
    return nRF24;
  }
}

String Settings::typeToString(RadioInterfaceType type) {
  switch (type) {
    case LT8900:
      return "LT8900";

    case nRF24:
    default:
      return "nRF24";
  }
}
