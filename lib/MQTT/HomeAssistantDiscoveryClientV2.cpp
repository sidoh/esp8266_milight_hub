#include <HomeAssistantDiscoveryClientV2.h>
#include <MiLightCommands.h>
#include <Units.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#elif ESP32
  #include <WiFi.h>
#endif

HomeAssistantDiscoveryClientV2::HomeAssistantDiscoveryClientV2(Settings& settings, MqttClient* mqttClient)
  : HomeAssistantDiscoveryClient(settings, mqttClient)
{ }

void HomeAssistantDiscoveryClientV2::sendDiscoverableDevices(const std::map<String, GroupAlias>& aliases) {
#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClientV2: Sending %d discoverable devices...\n"), aliases.size());
#endif

  for (const auto & alias : aliases) {
    addConfig(alias.first.c_str(), alias.second.bulbId);
  }

  addHub();
}

void HomeAssistantDiscoveryClientV2::removeOldDevices(const std::map<uint32_t, BulbId>& aliases) {
#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClientV2: Removing %d discoverable devices...\n"), aliases.size());
#endif

  for (auto itr = aliases.begin(); itr != aliases.end(); ++itr) {
    removeConfig(itr->second);
  }
}

void HomeAssistantDiscoveryClientV2::addConfig(const char* alias, const BulbId& bulbId) {
  String topic = buildTopic(bulbId);
  DynamicJsonDocument config(1024);

  // Unique ID for this device + alias combo
  char uniqueIdBuffer[30];
  snprintf_P(uniqueIdBuffer, sizeof(uniqueIdBuffer), PSTR("%X-%s"), getESPId(), alias);

  // String to ID the firmware version
  char fwVersion[100];
  snprintf_P(fwVersion, sizeof(fwVersion), PSTR("esp8266_milight_hub v%s"), QUOTE(MILIGHT_HUB_VERSION));

  // URL to the device
  char deviceUrl[23];
  snprintf_P(deviceUrl, sizeof(deviceUrl), PSTR("http://%s"), WiFi.localIP().toString().c_str());

  config[F("dev_cla")] = F("light");
  config[F("schema")] = F("json");
  config[F("name")] = F("light");
  // command topic
  config[F("cmd_t")] = mqttClient->bindTopicString(settings.mqttTopicPattern, bulbId);
  // state topic
  config[F("stat_t")] = mqttClient->bindTopicString(settings.mqttStateTopicPattern, bulbId);
  config[F("uniq_id")] = uniqueIdBuffer;

  JsonObject deviceMetadata = config.createNestedObject(F("dev"));
  deviceMetadata[F("name")] = alias;
  deviceMetadata[F("identifiers")] = uniqueIdBuffer;
  deviceMetadata[F("manufacturer")] = F("MiBoxer");
  deviceMetadata[F("via_device")] = String(getESPId());

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

  // supported_color_modes
  JsonArray colorModes = config.createNestedArray(F("sup_clrm"));

  // Flag RGB support
  if (MiLightRemoteTypeHelpers::supportsRgb(bulbId.deviceType)) {
    colorModes.add(F("rgb"));
  }

  // Flag adjustable color temp support
  if (MiLightRemoteTypeHelpers::supportsColorTemp(bulbId.deviceType)) {
    colorModes.add(GroupStateFieldNames::COLOR_TEMP);

    config[F("max_mireds")] = COLOR_TEMP_MAX_MIREDS;
    config[F("min_mireds")] = COLOR_TEMP_MIN_MIREDS;
  }

  // should only have brightness in this list if there are no other color modes
  // https://www.home-assistant.io/integrations/light.mqtt/#supported_color_modes
  if (colorModes.size() == 0) {
    colorModes.add(F("brightness"));
  }

  String message;
  serializeJson(config, message);

#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClientV2: adding discoverable device: %s...\n"), alias);
  Serial.printf_P(PSTR("  topic: %s\nconfig: %s\n"), topic.c_str(), message.c_str());
#endif


  mqttClient->send(topic.c_str(), message.c_str(), true);
}

void HomeAssistantDiscoveryClientV2::addHub(){
  String topic = buildTopicForHub();
  DynamicJsonDocument config(1024);

  // String to ID the firmware version
  char fwVersion[100];
  snprintf_P(fwVersion, sizeof(fwVersion), PSTR("esp8266_milight_hub v%s"), QUOTE(MILIGHT_HUB_VERSION));

  // URL to the device
  char deviceUrl[23];
  snprintf_P(deviceUrl, sizeof(deviceUrl), PSTR("http://%s"), WiFi.localIP().toString().c_str());

  config[F("dev_cla")] = F("firmware");
  config[F("name")] = F("Update");
  config[F("stat_t")] = buildStateTopicForHub();
  config[F("cmd_t")] = buildCommandTopicForHub();

  config[F("uniq_id")]=F("fw");

  JsonObject deviceMetadata = config.createNestedObject(F("dev"));
  deviceMetadata[F("name")] = settings.hostname;
  deviceMetadata[F("sw")] = fwVersion;
  deviceMetadata[F("mf")] = F("espressif");
  deviceMetadata[F("mdl")] = QUOTE(FIRMWARE_VARIANT);   
  deviceMetadata[F("identifiers")] = String(getESPId());
  deviceMetadata[F("cu")] = deviceUrl;

  String message;
  serializeJson(config, message);

#ifdef MQTT_DEBUG
  Serial.printf_P(PSTR("HomeAssistantDiscoveryClientV2: adding discoverable device: %s...\n"), alias);
  Serial.printf_P(PSTR("  topic: %s\nconfig: %s\n"), topic.c_str(), message.c_str());
#endif


  mqttClient->send(topic.c_str(), message.c_str(), true);
}

// Topic syntax:
//   <discovery_prefix>/<component>/[<node_id>/]<object_id>/config
//
// source: https://www.home-assistant.io/docs/mqtt/discovery/
String HomeAssistantDiscoveryClientV2::buildTopicForHub() {
  String topic = settings.homeAssistantDiscoveryPrefix;

  // Don't require the user to entier a "/" (or break things if they do)
  if (! topic.endsWith("/")) {
    topic += "/";
  }

  topic += "update/";
  // Use a static ID that doesn't depend on configuration.
  topic += "milight_hub_" + String(getESPId());
  topic += "/config";

  return topic;
}

String HomeAssistantDiscoveryClientV2::buildStateTopicForHub() {
  return "milight/states/milight_hub_" + String(getESPId());
}

String HomeAssistantDiscoveryClientV2::buildCommandTopicForHub() {
  return "milight/commands/milight_hub_" + String(getESPId());
}