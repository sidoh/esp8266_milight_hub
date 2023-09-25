#include <Settings.h>
#include <ArduinoJson.h>
#include <IntParsing.h>
#include <algorithm>
#include <JsonHelpers.h>
#include <GroupAlias.h>
#include <ProjectFS.h>
#include <StreamUtils.h>

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

  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::ADMIN_USERNAME), adminUsername);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::ADMIN_PASSWORD), adminPassword);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::CE_PIN), cePin);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::CSN_PIN), csnPin);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::RESET_PIN), resetPin);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::LED_PIN), ledPin);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::PACKET_REPEATS), packetRepeats);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::HTTP_REPEAT_FACTOR), httpRepeatFactor);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::AUTO_RESTART_PERIOD), _autoRestartPeriod);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_SERVER), _mqttServer);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_USERNAME), mqttUsername);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_PASSWORD), mqttPassword);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_TOPIC_PATTERN), mqttTopicPattern);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_UPDATE_TOPIC_PATTERN), mqttUpdateTopicPattern);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_STATE_TOPIC_PATTERN), mqttStateTopicPattern);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_CLIENT_STATUS_TOPIC), mqttClientStatusTopic);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::SIMPLE_MQTT_CLIENT_STATUS), simpleMqttClientStatus);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::DISCOVERY_PORT), discoveryPort);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::LISTEN_REPEATS), listenRepeats);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::STATE_FLUSH_INTERVAL), stateFlushInterval);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_STATE_RATE_LIMIT), mqttStateRateLimit);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_DEBOUNCE_DELAY), mqttDebounceDelay);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::MQTT_RETAIN), mqttRetain);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::PACKET_REPEAT_THROTTLE_THRESHOLD), packetRepeatThrottleThreshold);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::PACKET_REPEAT_THROTTLE_SENSITIVITY), packetRepeatThrottleSensitivity);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::PACKET_REPEAT_MINIMUM), packetRepeatMinimum);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::ENABLE_AUTOMATIC_MODE_SWITCHING), enableAutomaticModeSwitching);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::LED_MODE_PACKET_COUNT), ledModePacketCount);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::HOSTNAME), hostname);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::WIFI_STATIC_IP), wifiStaticIP);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::WIFI_STATIC_IP_GATEWAY), wifiStaticIPGateway);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::WIFI_STATIC_IP_NETMASK), wifiStaticIPNetmask);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::PACKET_REPEATS_PER_LOOP), packetRepeatsPerLoop);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::HOME_ASSISTANT_DISCOVERY_PREFIX), homeAssistantDiscoveryPrefix);
  this->setIfPresent(parsedSettings, FPSTR(SettingsKeys::DEFAULT_TRANSITION_PERIOD), defaultTransitionPeriod);

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::WIFI_MODE))) {
    this->wifiMode = wifiModeFromString(parsedSettings[FPSTR(SettingsKeys::WIFI_MODE)]);
  }

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::RF24_CHANNELS))) {
    JsonArray arr = parsedSettings[FPSTR(SettingsKeys::RF24_CHANNELS)];
    rf24Channels = JsonHelpers::jsonArrToVector<RF24Channel, String>(arr, RF24ChannelHelpers::valueFromName);
  }

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::RF24_LISTEN_CHANNEL))) {
    this->rf24ListenChannel = RF24ChannelHelpers::valueFromName(parsedSettings[FPSTR(SettingsKeys::RF24_LISTEN_CHANNEL)]);
  }

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::RF24_POWER_LEVEL))) {
    this->rf24PowerLevel = RF24PowerLevelHelpers::valueFromName(parsedSettings[FPSTR(SettingsKeys::RF24_POWER_LEVEL)]);
  }

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::LED_MODE_WIFI_CONFIG))) {
    this->ledModeWifiConfig = LEDStatus::stringToLEDMode(parsedSettings[FPSTR(SettingsKeys::LED_MODE_WIFI_CONFIG)]);
  }

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::LED_MODE_WIFI_FAILED))) {
    this->ledModeWifiFailed = LEDStatus::stringToLEDMode(parsedSettings[FPSTR(SettingsKeys::LED_MODE_WIFI_FAILED)]);
  }

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::LED_MODE_OPERATING))) {
    this->ledModeOperating = LEDStatus::stringToLEDMode(parsedSettings[FPSTR(SettingsKeys::LED_MODE_OPERATING)]);
  }

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::LED_MODE_PACKET))) {
    this->ledModePacket = LEDStatus::stringToLEDMode(parsedSettings[FPSTR(SettingsKeys::LED_MODE_PACKET)]);
  }

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::RADIO_INTERFACE_TYPE))) {
    this->radioInterfaceType = Settings::typeFromString(parsedSettings[FPSTR(SettingsKeys::RADIO_INTERFACE_TYPE)]);
  }

  if (parsedSettings.containsKey(FPSTR(SettingsKeys::DEVICE_IDS))) {
    JsonArray arr = parsedSettings[FPSTR(SettingsKeys::DEVICE_IDS)];
    updateDeviceIds(arr);
  }
  if (parsedSettings.containsKey(FPSTR(SettingsKeys::GATEWAY_CONFIGS))) {
    JsonArray arr = parsedSettings[FPSTR(SettingsKeys::GATEWAY_CONFIGS)];
    updateGatewayConfigs(arr);
  }
  if (parsedSettings.containsKey(FPSTR(SettingsKeys::GROUP_STATE_FIELDS))) {
    JsonArray arr = parsedSettings[FPSTR(SettingsKeys::GROUP_STATE_FIELDS)];
    groupStateFields = JsonHelpers::jsonArrToVector<GroupStateField, const char*>(arr, GroupStateFieldHelpers::getFieldByName);
  }

  // this key will only be present in old settings files, but for backwards
  // compatability, parse it if it's present.
  if (parsedSettings.containsKey(FPSTR(SettingsKeys::GROUP_ID_ALIASES))) {
    parseGroupIdAliases(parsedSettings);
  }
}

std::map<String, GroupAlias>::const_iterator Settings::findAlias(MiLightRemoteType deviceType, uint16_t deviceId, uint8_t groupId) {
  BulbId searchId{ deviceId, groupId, deviceType };

  for (auto it = groupIdAliases.begin(); it != groupIdAliases.end(); ++it) {
    if (searchId == it->second.bulbId) {
      return it;
    }
  }

  return groupIdAliases.end();
}

void Settings::parseGroupIdAliases(JsonObject json) {
  JsonObject aliases = json[FPSTR(SettingsKeys::GROUP_ID_ALIASES)].as<JsonObject>();

  // Save group IDs that were deleted so that they can be processed by discovery
  // if necessary
  for (auto it = groupIdAliases.begin(); it != groupIdAliases.end(); ++it) {
    deletedGroupIdAliases[it->second.bulbId.getCompactId()] = it->second.bulbId;
  }

  groupIdAliases.clear();
  size_t id = 1;

  for (JsonPair kv : aliases) {
    JsonArray bulbIdProps = kv.value();
    BulbId bulbId = {
      bulbIdProps[1].as<uint16_t>(),
      bulbIdProps[2].as<uint8_t>(),
      MiLightRemoteTypeHelpers::remoteTypeFromString(bulbIdProps[0].as<String>())
    };
    groupIdAliases[kv.key().c_str()] = GroupAlias(id++, kv.key().c_str(), bulbId);

    // If added this round, do not mark as deleted.
    deletedGroupIdAliases.erase(bulbId.getCompactId());
  }
}

void Settings::dumpGroupIdAliases(JsonObject json) {
  JsonObject aliases = json.createNestedObject(FPSTR(SettingsKeys::GROUP_ID_ALIASES));

  for (auto & groupIdAlias : groupIdAliases) {
    JsonArray bulbProps = aliases.createNestedArray(groupIdAlias.first);
    BulbId bulbId = groupIdAlias.second.bulbId;
    bulbProps.add(MiLightRemoteTypeHelpers::remoteTypeToString(bulbId.deviceType));
    bulbProps.add(bulbId.deviceId);
    bulbProps.add(bulbId.groupId);
  }
}

bool Settings::loadAliases(Settings &settings) {
  if (ProjectFS.exists(ALIASES_FILE)) {
    File f = ProjectFS.open(ALIASES_FILE, "r");
    ReadBufferingStream bufferedReader{f, 64};
    GroupAlias::loadAliases(bufferedReader, settings.groupIdAliases);

    // find current max id
    size_t maxId = 0;
    for (auto & alias : settings.groupIdAliases) {
      maxId = max(maxId, alias.second.id);
    }
    settings.groupIdAliasNextId = maxId + 1;

    printf_P(PSTR("loaded %d aliases\n"), settings.groupIdAliases.size());

    return true;
  } else {
    return false;
  }
}

bool Settings::load(Settings& settings) {
  bool shouldInit = false;

  if (ProjectFS.exists(SETTINGS_FILE)) {
    // Clear in-memory settings
    settings = Settings();

    File f = ProjectFS.open(SETTINGS_FILE, "r");

    DynamicJsonDocument json(MILIGHT_HUB_SETTINGS_BUFFER_SIZE);
    auto error = deserializeJson(json, f);
    f.close();

    if (! error) {
      JsonObject parsedSettings = json.as<JsonObject>();
      settings.patch(parsedSettings);
    } else {
      Serial.print(F("Error parsing saved settings file: "));
      Serial.println(error.c_str());
      Serial.println(F("contents:"));

      f = ProjectFS.open(SETTINGS_FILE, "r");
      Serial.println(f.readString());

      return false;
    }
  } else {
    shouldInit = true;
  }

  // If we loaded aliases from the settings file but not the aliases file,
  // port them over to the aliases file.
  const bool settingKeyAliasesEmpty = settings.groupIdAliases.empty();
  const bool aliasesFileEmpty = loadAliases(settings);

  if (!settingKeyAliasesEmpty && aliasesFileEmpty) {
    Serial.println(F("Porting aliases from settings file to aliases file"));
    shouldInit = true;
  }

  if (shouldInit) {
    settings.save();
  }

  return true;
}

void Settings::save() {
  File f = ProjectFS.open(SETTINGS_FILE, "w");

  if (!f) {
    Serial.println(F("Opening settings file failed"));
    return;
  } else {
    WriteBufferingStream writer{f, 64};
    serialize(f);
    writer.flush();
    f.close();
  }

  File aliasesFile = ProjectFS.open(ALIASES_FILE, "w");

  if (!aliasesFile) {
    Serial.println(F("Opening aliases file failed"));
  } else {
    WriteBufferingStream aliases{aliasesFile, 64};
    GroupAlias::saveAliases(aliases, groupIdAliases);
    aliases.flush();
    aliasesFile.close();
  }
}

void Settings::serialize(Print& stream, const bool prettyPrint) const {
  DynamicJsonDocument root(MILIGHT_HUB_SETTINGS_BUFFER_SIZE);

  root[FPSTR(SettingsKeys::ADMIN_USERNAME)] = this->adminUsername;
  root[FPSTR(SettingsKeys::ADMIN_PASSWORD)] = this->adminPassword;
  root[FPSTR(SettingsKeys::CE_PIN)] = this->cePin;
  root[FPSTR(SettingsKeys::CSN_PIN)] = this->csnPin;
  root[FPSTR(SettingsKeys::RESET_PIN)] = this->resetPin;
  root[FPSTR(SettingsKeys::LED_PIN)] = this->ledPin;
  root[FPSTR(SettingsKeys::RADIO_INTERFACE_TYPE)] = typeToString(this->radioInterfaceType);
  root[FPSTR(SettingsKeys::PACKET_REPEATS)] = this->packetRepeats;
  root[FPSTR(SettingsKeys::HTTP_REPEAT_FACTOR)] = this->httpRepeatFactor;
  root[FPSTR(SettingsKeys::AUTO_RESTART_PERIOD)] = this->_autoRestartPeriod;
  root[FPSTR(SettingsKeys::MQTT_SERVER)] = this->_mqttServer;
  root[FPSTR(SettingsKeys::MQTT_USERNAME)] = this->mqttUsername;
  root[FPSTR(SettingsKeys::MQTT_PASSWORD)] = this->mqttPassword;
  root[FPSTR(SettingsKeys::MQTT_TOPIC_PATTERN)] = this->mqttTopicPattern;
  root[FPSTR(SettingsKeys::MQTT_UPDATE_TOPIC_PATTERN)] = this->mqttUpdateTopicPattern;
  root[FPSTR(SettingsKeys::MQTT_STATE_TOPIC_PATTERN)] = this->mqttStateTopicPattern;
  root[FPSTR(SettingsKeys::MQTT_CLIENT_STATUS_TOPIC)] = this->mqttClientStatusTopic;
  root[FPSTR(SettingsKeys::SIMPLE_MQTT_CLIENT_STATUS)] = this->simpleMqttClientStatus;
  root[FPSTR(SettingsKeys::DISCOVERY_PORT)] = this->discoveryPort;
  root[FPSTR(SettingsKeys::LISTEN_REPEATS)] = this->listenRepeats;
  root[FPSTR(SettingsKeys::STATE_FLUSH_INTERVAL)] = this->stateFlushInterval;
  root[FPSTR(SettingsKeys::MQTT_STATE_RATE_LIMIT)] = this->mqttStateRateLimit;
  root[FPSTR(SettingsKeys::MQTT_DEBOUNCE_DELAY)] = this->mqttDebounceDelay;
  root[FPSTR(SettingsKeys::MQTT_RETAIN)] = this->mqttRetain;
  root[FPSTR(SettingsKeys::PACKET_REPEAT_THROTTLE_SENSITIVITY)] = this->packetRepeatThrottleSensitivity;
  root[FPSTR(SettingsKeys::PACKET_REPEAT_THROTTLE_THRESHOLD)] = this->packetRepeatThrottleThreshold;
  root[FPSTR(SettingsKeys::PACKET_REPEAT_MINIMUM)] = this->packetRepeatMinimum;
  root[FPSTR(SettingsKeys::ENABLE_AUTOMATIC_MODE_SWITCHING)] = this->enableAutomaticModeSwitching;
  root[FPSTR(SettingsKeys::LED_MODE_WIFI_CONFIG)] = LEDStatus::LEDModeToString(this->ledModeWifiConfig);
  root[FPSTR(SettingsKeys::LED_MODE_WIFI_FAILED)] = LEDStatus::LEDModeToString(this->ledModeWifiFailed);
  root[FPSTR(SettingsKeys::LED_MODE_OPERATING)] = LEDStatus::LEDModeToString(this->ledModeOperating);
  root[FPSTR(SettingsKeys::LED_MODE_PACKET)] = LEDStatus::LEDModeToString(this->ledModePacket);
  root[FPSTR(SettingsKeys::LED_MODE_PACKET_COUNT)] = this->ledModePacketCount;
  root[FPSTR(SettingsKeys::HOSTNAME)] = this->hostname;
  root[FPSTR(SettingsKeys::RF24_POWER_LEVEL)] = RF24PowerLevelHelpers::nameFromValue(this->rf24PowerLevel);
  root[FPSTR(SettingsKeys::RF24_LISTEN_CHANNEL)] = RF24ChannelHelpers::nameFromValue(rf24ListenChannel);
  root[FPSTR(SettingsKeys::WIFI_STATIC_IP)] = this->wifiStaticIP;
  root[FPSTR(SettingsKeys::WIFI_STATIC_IP_GATEWAY)] = this->wifiStaticIPGateway;
  root[FPSTR(SettingsKeys::WIFI_STATIC_IP_NETMASK)] = this->wifiStaticIPNetmask;
  root[FPSTR(SettingsKeys::PACKET_REPEATS_PER_LOOP)] = this->packetRepeatsPerLoop;
  root[FPSTR(SettingsKeys::HOME_ASSISTANT_DISCOVERY_PREFIX)] = this->homeAssistantDiscoveryPrefix;
  root[FPSTR(SettingsKeys::WIFI_MODE)] = wifiModeToString(this->wifiMode);
  root[FPSTR(SettingsKeys::DEFAULT_TRANSITION_PERIOD)] = this->defaultTransitionPeriod;

  JsonArray channelArr = root.createNestedArray(FPSTR(SettingsKeys::RF24_CHANNELS));
  JsonHelpers::vectorToJsonArr<RF24Channel, String>(channelArr, rf24Channels, RF24ChannelHelpers::nameFromValue);

  JsonArray deviceIdsArr = root.createNestedArray(FPSTR(SettingsKeys::DEVICE_IDS));
  JsonHelpers::copyFrom<uint16_t>(deviceIdsArr, this->deviceIds);

  JsonArray gatewayConfigsArr = root.createNestedArray(FPSTR(SettingsKeys::GATEWAY_CONFIGS));
  for (size_t i = 0; i < this->gatewayConfigs.size(); i++) {
    JsonArray elmt = gatewayConfigsArr.createNestedArray();
    elmt.add(this->gatewayConfigs[i]->deviceId);
    elmt.add(this->gatewayConfigs[i]->port);
    elmt.add(this->gatewayConfigs[i]->protocolVersion);
  }

  JsonArray groupStateFieldArr = root.createNestedArray(FPSTR(SettingsKeys::GROUP_STATE_FIELDS));
  JsonHelpers::vectorToJsonArr<GroupStateField, const char*>(groupStateFieldArr, groupStateFields, GroupStateFieldHelpers::getFieldName);

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

void Settings::addAlias(const char *alias, const BulbId &bulbId) {
  groupIdAliases[alias] = GroupAlias(groupIdAliasNextId++, alias, bulbId);
}

bool Settings::deleteAlias(size_t id) {
  for (auto it = groupIdAliases.begin(); it != groupIdAliases.end(); ++it) {
    if (it->second.id == id) {
      groupIdAliases.erase(it);
      deletedGroupIdAliases[it->second.bulbId.getCompactId()] = it->second.bulbId;

      return true;
    }
  }

  return false;
}

std::map<String, GroupAlias>::const_iterator Settings::findAliasById(size_t id) {
  for (auto it = groupIdAliases.begin(); it != groupIdAliases.end(); ++it) {
    if (it->second.id == id) {
      return it;
    }
  }

  return groupIdAliases.end();
}