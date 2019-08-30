#include <HomeAssistantDiscoveryClient.h>
#include <MiLightCommands.h>

HomeAssistantDiscoveryClient::HomeAssistantDiscoveryClient(Settings& settings, MqttClient* mqttClient)
  : settings(settings)
  , mqttClient(mqttClient)
{ }

void HomeAssistantDiscoveryClient::sendDiscoverableDevices(const std::map<String, BulbId>& aliases) {
#ifdef MQTT_DEBUG
  Serial.println(F("HomeAssistantDiscoveryClient: Sending discoverable devices..."));
#endif

  for (auto itr = aliases.begin(); itr != aliases.end(); ++itr) {
    addConfig(itr->first.c_str(), itr->second);
  }
}

void HomeAssistantDiscoveryClient::removeOldDevices(const std::map<uint32_t, BulbId>& aliases) {
#ifdef MQTT_DEBUG
  Serial.println(F("HomeAssistantDiscoveryClient: Removing discoverable devices..."));
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

  config[F("schema")] = F("json");
  config[F("name")] = alias;
  config[F("command_topic")] = mqttClient->bindTopicString(settings.mqttTopicPattern, bulbId);
  config[F("state_topic")] = mqttClient->bindTopicString(settings.mqttStateTopicPattern, bulbId);
  JsonObject deviceMetadata = config.createNestedObject(F("device"));

  deviceMetadata[F("manufacturer")] = F("esp8266_milight_hub");
  deviceMetadata[F("sw_version")] = QUOTE(MILIGHT_HUB_VERSION);

  JsonArray identifiers = deviceMetadata.createNestedArray(F("identifiers"));
  identifiers.add(ESP.getChipId());
  bulbId.serialize(identifiers);

  // HomeAssistant only supports simple client availability
  if (settings.mqttClientStatusTopic.length() > 0 && settings.simpleMqttClientStatus) {
    config[F("availability_topic")] = settings.mqttClientStatusTopic;
    config[F("payload_available")] = F("connected");
    config[F("payload_not_available")] = F("disconnected");
  }

  // Configure supported commands based on the bulb type

  // All supported bulbs support brightness and night mode
  config[GroupStateFieldNames::BRIGHTNESS] = true;
  config[GroupStateFieldNames::EFFECT] = true;

  JsonArray effects = config.createNestedArray(F("effect_list"));
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

  // These bulbs support RGB color
  switch (bulbId.deviceType) {
    case REMOTE_TYPE_FUT089:
    case REMOTE_TYPE_RGB:
    case REMOTE_TYPE_RGB_CCT:
    case REMOTE_TYPE_RGBW:
      config[F("rgb")] = true;
      break;
    default:
      break; //nothing
  }

  // These bulbs support adjustable white values
  switch (bulbId.deviceType) {
    case REMOTE_TYPE_CCT:
    case REMOTE_TYPE_FUT089:
    case REMOTE_TYPE_FUT091:
    case REMOTE_TYPE_RGB_CCT:
      config[GroupStateFieldNames::COLOR_TEMP] = true;
      break;
    default:
      break; //nothing
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