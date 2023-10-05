#include <HomeAssistantDiscoveryClient.h>
#include <MiLightCommands.h>
#include <Units.h>
#include <ESP8266WiFi.h>

HomeAssistantDiscoveryClient::HomeAssistantDiscoveryClient(Settings& settings, MqttClient* mqttClient)
  : settings(settings)
  , mqttClient(mqttClient)
{ }

void HomeAssistantDiscoveryClient::sendDiscoverableDevices(const std::map<String, GroupAlias>& aliases) {
#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClient: Sending %d discoverable devices...\n"), aliases.size());
#endif

  for (const auto & alias : aliases) {
    addConfig(alias.first.c_str(), alias.second.bulbId);
  }
}

void HomeAssistantDiscoveryClient::removeOldDevices(const std::map<uint32_t, BulbId>& aliases) {
#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClient: Removing %d discoverable devices...\n"), aliases.size());
#endif

  for (auto itr = aliases.begin(); itr != aliases.end(); ++itr) {
    removeConfig(itr->second);
  }
}

void HomeAssistantDiscoveryClient::removeConfig(const BulbId& bulbId) {
  // Remove by publishing an empty message
  String topic = buildTopic(bulbId);
  mqttClient->send(topic.c_str(), "", true);
}

void HomeAssistantDiscoveryClient::addConfig(const char* alias, const BulbId& bulbId) {
  String topic = buildTopic(bulbId);
  DynamicJsonDocument config(1024);

  // Unique ID for this device + alias combo
  char uniqueIdBuffer[30];
  snprintf_P(uniqueIdBuffer, sizeof(uniqueIdBuffer), PSTR("%X-%s"), ESP.getChipId(), alias);

  // String to ID the firmware version
  char fwVersion[100];
  snprintf_P(fwVersion, sizeof(fwVersion), PSTR("esp8266_milight_hub v%s"), QUOTE(MILIGHT_HUB_VERSION));

  // URL to the device
  char deviceUrl[23];
  snprintf_P(deviceUrl, sizeof(deviceUrl), PSTR("http://%s"), WiFi.localIP().toString().c_str());

  config[F("dev_cla")] = F("light");
  config[F("schema")] = F("json");
  config[F("name")] = alias;
  // command topic
  config[F("cmd_t")] = mqttClient->bindTopicString(settings.mqttTopicPattern, bulbId);
  // state topic
  config[F("stat_t")] = mqttClient->bindTopicString(settings.mqttStateTopicPattern, bulbId);
  config[F("uniq_id")] = uniqueIdBuffer;

  JsonObject deviceMetadata = config.createNestedObject(F("dev"));
  deviceMetadata[F("name")] = settings.hostname;
  deviceMetadata[F("sw")] = fwVersion;
  deviceMetadata[F("mf")] = F("espressif");
  deviceMetadata[F("mdl")] = QUOTE(FIRMWARE_VARIANT);
  deviceMetadata[F("identifiers")] = String(ESP.getChipId());
  deviceMetadata[F("cu")] = deviceUrl;

  // HomeAssistant only supports simple client availability
  if (settings.mqttClientStatusTopic.length() > 0 && settings.simpleMqttClientStatus) {
    // availability topic
    config[F("avty_t")] = settings.mqttClientStatusTopic;
    // payload_available
    config[F("pl_avail")] = F("connected");
    // payload_not_available
    config[F("pl_not_avail")] = F("disconnected");
  }

  // Configure supported commands based on the bulb type

  // All supported bulbs support brightness and night mode
  config[GroupStateFieldNames::BRIGHTNESS] = true;
  config[GroupStateFieldNames::EFFECT] = true;

  // effect_list
  JsonArray effects = config.createNestedArray(F("fx_list"));
  effects.add(MiLightCommandNames::NIGHT_MODE);

  // These bulbs support switching between rgb/white, and have a "white_mode" command
  switch (bulbId.deviceType) {
    case REMOTE_TYPE_FUT089:
    case REMOTE_TYPE_RGB_CCT:
    case REMOTE_TYPE_RGBW:
      effects.add("white_mode");
      break;
    default:
      break; //nothing
  }

  // All bulbs except CCT have 9 modes.  FUT029 and RGB/FUT096 has 9 modes, but they
  // are not selectable directly.  There are only "next mode" commands.
  switch (bulbId.deviceType) {
    case REMOTE_TYPE_CCT:
    case REMOTE_TYPE_RGB:
    case REMOTE_TYPE_FUT020:
      break;
    default:
      addNumberedEffects(effects, 0, 8);
      break;
  }

  // Flag RGB support
  if (MiLightRemoteTypeHelpers::supportsRgb(bulbId.deviceType)) {
    config[F("rgb")] = true;
  }

  // Flag adjustable color temp support
  if (MiLightRemoteTypeHelpers::supportsColorTemp(bulbId.deviceType)) {
    config[GroupStateFieldNames::COLOR_TEMP] = true;
    config[F("max_mirs")] = COLOR_TEMP_MAX_MIREDS;
    config[F("min_mirs")] = COLOR_TEMP_MIN_MIREDS;
  }

  String message;
  serializeJson(config, message);

#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClient: adding discoverable device: %s...\n"), alias);
  Serial.printf_P(PSTR("  topic: %s\nconfig: %s\n"), topic.c_str(), message.c_str());
#endif


  mqttClient->send(topic.c_str(), message.c_str(), true);
}

// Topic syntax:
//   <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
//
// source: https://www.home-assistant.io/docs/mqtt/discovery/
String HomeAssistantDiscoveryClient::buildTopic(const BulbId& bulbId) {
  String topic = settings.homeAssistantDiscoveryPrefix;

  // Don't require the user to entier a "/" (or break things if they do)
  if (! topic.endsWith("/")) {
    topic += "/";
  }

  topic += "light/";
  // Use a static ID that doesn't depend on configuration.
  topic += "milight_hub_" + String(ESP.getChipId());

  // make the object ID based on the actual parameters rather than the alias.
  topic += "/";
  topic += MiLightRemoteTypeHelpers::remoteTypeToString(bulbId.deviceType);
  topic += "_";
  topic += bulbId.getHexDeviceId();
  topic += "_";
  topic += bulbId.groupId;
  topic += "/config";

  return topic;
}

String HomeAssistantDiscoveryClient::bindTopicVariables(const String& topic, const char* alias, const BulbId& bulbId) {
  String boundTopic = topic;
  String hexDeviceId = bulbId.getHexDeviceId();

  boundTopic.replace(":device_alias", alias);
  boundTopic.replace(":device_id", hexDeviceId);
  boundTopic.replace(":hex_device_id", hexDeviceId);
  boundTopic.replace(":dec_device_id", String(bulbId.deviceId));
  boundTopic.replace(":device_type", MiLightRemoteTypeHelpers::remoteTypeToString(bulbId.deviceType));
  boundTopic.replace(":group_id", String(bulbId.groupId));

  return boundTopic;
}

void HomeAssistantDiscoveryClient::addNumberedEffects(JsonArray& effectList, uint8_t start, uint8_t end) {
  for (uint8_t i = start; i <= end; ++i) {
    effectList.add(String(i));
  }
}
