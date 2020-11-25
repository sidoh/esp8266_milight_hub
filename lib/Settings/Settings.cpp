#include <Settings.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <IntParsing.h>
#include <algorithm>
#include <JsonHelpers.h>

#define PORT_POSITION(s) ( s.indexOf(':') )

GatewayConfig::GatewayConfig(uint16_t deviceId, uint16_t port, uint8_t protocolVersion)
  : deviceId(deviceId)
  , port(port)
  , protocolVersion(protocolVersion)
{ }

bool Settings::isAuthenticationEnabled() const {
  return adminUsername.length() > 0 && adminPassword.length() > 0;
}

const String& Settings::getUsername() const {
  return adminUsername;
}

const String& Settings::getPassword() const {
  return adminPassword;
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

void Settings::updateDeviceIds(JsonArray arr) {
  this->deviceIds.clear();

  for (size_t i = 0; i < arr.size(); ++i) {
    this->deviceIds.push_back(arr[i]);
  }
}

void Settings::updateGatewayConfigs(JsonArray arr) {
  gatewayConfigs.clear();

  for (size_t i = 0; i < arr.size(); i++) {
    JsonArray params = arr[i];

    if (params.size() == 3) {
      std::shared_ptr<GatewayConfig> ptr = std::make_shared<GatewayConfig>(parseInt<uint16_t>(params[0]), params[1], params[2]);
      gatewayConfigs.push_back(std::move(ptr));
    } else {
      Serial.print(F("Settings - skipped parsing gateway ports settings for element #"));
      Serial.println(i);
    }
  }
}

void Settings::patch(JsonObject parsedSettings) {
  if (parsedSettings.isNull()) {
    Serial.println(F("Skipping patching loaded settings.  Parsed settings was null."));
    return;
  }

  this->setIfPresent(parsedSettings, "admin_username", adminUsername);
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
  this->setIfPresent(parsedSettings, "mqtt_client_status_topic", mqttClientStatusTopic);
  this->setIfPresent(parsedSettings, "simple_mqtt_client_status", simpleMqttClientStatus);
  this->setIfPresent(parsedSettings, "discovery_port", discoveryPort);
  this->setIfPresent(parsedSettings, "listen_repeats", listenRepeats);
  this->setIfPresent(parsedSettings, "state_flush_interval", stateFlushInterval);
  this->setIfPresent(parsedSettings, "mqtt_state_rate_limit", mqttStateRateLimit);
  this->setIfPresent(parsedSettings, "mqtt_debounce_delay", mqttDebounceDelay);
  this->setIfPresent(parsedSettings, "mqtt_retain", mqttRetain);
  this->setIfPresent(parsedSettings, "packet_repeat_throttle_threshold", packetRepeatThrottleThreshold);
  this->setIfPresent(parsedSettings, "packet_repeat_throttle_sensitivity", packetRepeatThrottleSensitivity);
  this->setIfPresent(parsedSettings, "packet_repeat_minimum", packetRepeatMinimum);
  this->setIfPresent(parsedSettings, "enable_automatic_mode_switching", enableAutomaticModeSwitching);
  this->setIfPresent(parsedSettings, "led_mode_packet_count", ledModePacketCount);
  this->setIfPresent(parsedSettings, "hostname", hostname);
  this->setIfPresent(parsedSettings, "wifi_static_ip", wifiStaticIP);
  this->setIfPresent(parsedSettings, "wifi_static_ip_gateway", wifiStaticIPGateway);
  this->setIfPresent(parsedSettings, "wifi_static_ip_netmask", wifiStaticIPNetmask);
  this->setIfPresent(parsedSettings, "packet_repeats_per_loop", packetRepeatsPerLoop);
  this->setIfPresent(parsedSettings, "home_assistant_discovery_prefix", homeAssistantDiscoveryPrefix);
  this->setIfPresent(parsedSettings, "default_transition_period", defaultTransitionPeriod);

  if (parsedSettings.containsKey("wifi_mode")) {
    this->wifiMode = wifiModeFromString(parsedSettings["wifi_mode"]);
  }

  if (parsedSettings.containsKey("rf24_channels")) {
    JsonArray arr = parsedSettings["rf24_channels"];
    rf24Channels = JsonHelpers::jsonArrToVector<RF24Channel, String>(arr, RF24ChannelHelpers::valueFromName);
  }

  if (parsedSettings.containsKey("rf24_listen_channel")) {
    this->rf24ListenChannel = RF24ChannelHelpers::valueFromName(parsedSettings["rf24_listen_channel"]);
  }

  if (parsedSettings.containsKey("rf24_power_level")) {
    this->rf24PowerLevel = RF24PowerLevelHelpers::valueFromName(parsedSettings["rf24_power_level"]);
  }

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
    JsonArray arr = parsedSettings["device_ids"];
    updateDeviceIds(arr);
  }
  if (parsedSettings.containsKey("gateway_configs")) {
    JsonArray arr = parsedSettings["gateway_configs"];
    updateGatewayConfigs(arr);
  }
  if (parsedSettings.containsKey("group_state_fields")) {
    JsonArray arr = parsedSettings["group_state_fields"];
    groupStateFields = JsonHelpers::jsonArrToVector<GroupStateField, const char*>(arr, GroupStateFieldHelpers::getFieldByName);
  }

  if (parsedSettings.containsKey("group_id_aliases")) {
    parseGroupIdAliases(parsedSettings);
  }
}

std::map<String, BulbId>::const_iterator Settings::findAlias(MiLightRemoteType deviceType, uint16_t deviceId, uint8_t groupId) {
  BulbId searchId{ deviceId, groupId, deviceType };

  for (auto it = groupIdAliases.begin(); it != groupIdAliases.end(); ++it) {
    if (searchId == it->second) {
      return it;
    }
  }

  return groupIdAliases.end();
}

void Settings::parseGroupIdAliases(JsonObject json) {
  JsonObject aliases = json["group_id_aliases"];

  // Save group IDs that were deleted so that they can be processed by discovery
  // if necessary
  for (auto it = groupIdAliases.begin(); it != groupIdAliases.end(); ++it) {
    deletedGroupIdAliases[it->second.getCompactId()] = it->second;
  }

  groupIdAliases.clear();

  for (JsonPair kv : aliases) {
    JsonArray bulbIdProps = kv.value();
    BulbId bulbId = {
      bulbIdProps[1].as<uint16_t>(),
      bulbIdProps[2].as<uint8_t>(),
      MiLightRemoteTypeHelpers::remoteTypeFromString(bulbIdProps[0].as<String>())
    };
    groupIdAliases[kv.key().c_str()] = bulbId;

    // If added this round, do not mark as deleted.
    deletedGroupIdAliases.erase(bulbId.getCompactId());
  }
}

void Settings::dumpGroupIdAliases(JsonObject json) {
  JsonObject aliases = json.createNestedObject("group_id_aliases");

  for (std::map<String, BulbId>::iterator itr = groupIdAliases.begin(); itr != groupIdAliases.end(); ++itr) {
    JsonArray bulbProps = aliases.createNestedArray(itr->first);
    BulbId bulbId = itr->second;
    bulbProps.add(MiLightRemoteTypeHelpers::remoteTypeToString(bulbId.deviceType));
    bulbProps.add(bulbId.deviceId);
    bulbProps.add(bulbId.groupId);
  }
}

void Settings::load(Settings& settings) {
  if (SPIFFS.exists(SETTINGS_FILE)) {
    // Clear in-memory settings
    settings = Settings();

    File f = SPIFFS.open(SETTINGS_FILE, "r");

    DynamicJsonDocument json(MILIGHT_HUB_SETTINGS_BUFFER_SIZE);
    auto error = deserializeJson(json, f);
    f.close();

    if (! error) {
      JsonObject parsedSettings = json.as<JsonObject>();
      settings.patch(parsedSettings);
    } else {
      Serial.print(F("Error parsing saved settings file: "));
      Serial.println(error.c_str());
    }
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

void Settings::serialize(Print& stream, const bool prettyPrint) {
  DynamicJsonDocument root(MILIGHT_HUB_SETTINGS_BUFFER_SIZE);

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
  root["mqtt_client_status_topic"] = this->mqttClientStatusTopic;
  root["simple_mqtt_client_status"] = this->simpleMqttClientStatus;
  root["discovery_port"] = this->discoveryPort;
  root["listen_repeats"] = this->listenRepeats;
  root["state_flush_interval"] = this->stateFlushInterval;
  root["mqtt_state_rate_limit"] = this->mqttStateRateLimit;
  root["mqtt_debounce_delay"] = this->mqttDebounceDelay;
  root["mqtt_retain"] = this->mqttRetain;
  root["packet_repeat_throttle_sensitivity"] = this->packetRepeatThrottleSensitivity;
  root["packet_repeat_throttle_threshold"] = this->packetRepeatThrottleThreshold;
  root["packet_repeat_minimum"] = this->packetRepeatMinimum;
  root["enable_automatic_mode_switching"] = this->enableAutomaticModeSwitching;
  root["led_mode_wifi_config"] = LEDStatus::LEDModeToString(this->ledModeWifiConfig);
  root["led_mode_wifi_failed"] = LEDStatus::LEDModeToString(this->ledModeWifiFailed);
  root["led_mode_operating"] = LEDStatus::LEDModeToString(this->ledModeOperating);
  root["led_mode_packet"] = LEDStatus::LEDModeToString(this->ledModePacket);
  root["led_mode_packet_count"] = this->ledModePacketCount;
  root["hostname"] = this->hostname;
  root["rf24_power_level"] = RF24PowerLevelHelpers::nameFromValue(this->rf24PowerLevel);
  root["rf24_listen_channel"] = RF24ChannelHelpers::nameFromValue(rf24ListenChannel);
  root["wifi_static_ip"] = this->wifiStaticIP;
  root["wifi_static_ip_gateway"] = this->wifiStaticIPGateway;
  root["wifi_static_ip_netmask"] = this->wifiStaticIPNetmask;
  root["packet_repeats_per_loop"] = this->packetRepeatsPerLoop;
  root["home_assistant_discovery_prefix"] = this->homeAssistantDiscoveryPrefix;
  root["wifi_mode"] = wifiModeToString(this->wifiMode);
  root["default_transition_period"] = this->defaultTransitionPeriod;

  JsonArray channelArr = root.createNestedArray("rf24_channels");
  JsonHelpers::vectorToJsonArr<RF24Channel, String>(channelArr, rf24Channels, RF24ChannelHelpers::nameFromValue);

  JsonArray deviceIdsArr = root.createNestedArray("device_ids");
  JsonHelpers::copyFrom<uint16_t>(deviceIdsArr, this->deviceIds);

  JsonArray gatewayConfigsArr = root.createNestedArray("gateway_configs");
  for (size_t i = 0; i < this->gatewayConfigs.size(); i++) {
    JsonArray elmt = gatewayConfigsArr.createNestedArray();
    elmt.add(this->gatewayConfigs[i]->deviceId);
    elmt.add(this->gatewayConfigs[i]->port);
    elmt.add(this->gatewayConfigs[i]->protocolVersion);
  }

  JsonArray groupStateFieldArr = root.createNestedArray("group_state_fields");
  JsonHelpers::vectorToJsonArr<GroupStateField, const char*>(groupStateFieldArr, groupStateFields, GroupStateFieldHelpers::getFieldName);

  dumpGroupIdAliases(root.as<JsonObject>());

  if (prettyPrint) {
    serializeJsonPretty(root, stream);
  } else {
    serializeJson(root, stream);
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

WifiMode Settings::wifiModeFromString(const String& mode) {
  if (mode.equalsIgnoreCase("b")) {
    return WifiMode::B;
  } else if (mode.equalsIgnoreCase("g")) {
    return WifiMode::G;
  } else {
    return WifiMode::N;
  }
}

String Settings::wifiModeToString(WifiMode mode) {
  switch (mode) {
    case WifiMode::B:
      return "b";
    case WifiMode::G:
      return "g";
    case WifiMode::N:
    default:
      return "n";
  }
}