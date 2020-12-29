#include <Arduino.h>
#include <StringStream.h>
#include <ArduinoJson.h>
#include <GroupStateField.h>
#include <RF24PowerLevel.h>
#include <RF24Channel.h>
#include <Size.h>
#include <LEDStatus.h>
#include <AuthProviders.h>

#include <MiLightRemoteType.h>
#include <BulbId.h>

#include <vector>
#include <memory>
#include <map>

#ifndef _SETTINGS_H_INCLUDED
#define _SETTINGS_H_INCLUDED

#ifndef MILIGHT_HUB_SETTINGS_BUFFER_SIZE
#define MILIGHT_HUB_SETTINGS_BUFFER_SIZE 4096
#endif

#define XQUOTE(x) #x
#define QUOTE(x) XQUOTE(x)

#ifndef FIRMWARE_NAME
#define FIRMWARE_NAME unknown
#endif

#ifndef FIRMWARE_VARIANT
#define FIRMWARE_VARIANT unknown
#endif

#ifndef MILIGHT_HUB_VERSION
#define MILIGHT_HUB_VERSION unknown
#endif

#ifndef MILIGHT_MAX_STATE_ITEMS
#define MILIGHT_MAX_STATE_ITEMS 100
#endif

#ifndef MILIGHT_MAX_STALE_MQTT_GROUPS
#define MILIGHT_MAX_STALE_MQTT_GROUPS 10
#endif

#define SETTINGS_FILE  "/config.json"
#define SETTINGS_TERMINATOR '\0'

#define WEB_INDEX_FILENAME "/web/index.html"

#define MILIGHT_GITHUB_USER "sidoh"
#define MILIGHT_GITHUB_REPO "esp8266_milight_hub"
#define MILIGHT_REPO_WEB_PATH "/data/web/index.html"

#define MINIMUM_RESTART_PERIOD 1
#define DEFAULT_MQTT_PORT 1883
#define MAX_IP_ADDR_LEN 15

enum RadioInterfaceType {
  nRF24 = 0,
  LT8900 = 1,
};

enum class WifiMode {
  B, G, N
};

static const std::vector<GroupStateField> DEFAULT_GROUP_STATE_FIELDS({
  GroupStateField::STATE,
  GroupStateField::BRIGHTNESS,
  GroupStateField::COMPUTED_COLOR,
  GroupStateField::MODE,
  GroupStateField::COLOR_TEMP,
  GroupStateField::BULB_MODE
});

struct GatewayConfig {
  GatewayConfig(uint16_t deviceId, uint16_t port, uint8_t protocolVersion);

  const uint16_t deviceId;
  const uint16_t port;
  const uint8_t protocolVersion;
};

class Settings {
public:
  Settings() :
    adminUsername(""),
    adminPassword(""),
    // CE and CSN pins from nrf24l01
    cePin(4),
    csnPin(15),
    resetPin(0),
    ledPin(-2),
    radioInterfaceType(nRF24),
    packetRepeats(50),
    httpRepeatFactor(1),
    listenRepeats(3),
    discoveryPort(48899),
    simpleMqttClientStatus(false),
    stateFlushInterval(10000),
    mqttStateRateLimit(500),
    mqttDebounceDelay(500),
    mqttRetain(true),
    packetRepeatThrottleThreshold(200),
    packetRepeatThrottleSensitivity(0),
    packetRepeatMinimum(3),
    enableAutomaticModeSwitching(false),
    ledModeWifiConfig(LEDStatus::LEDMode::FastToggle),
    ledModeWifiFailed(LEDStatus::LEDMode::On),
    ledModeOperating(LEDStatus::LEDMode::SlowBlip),
    ledModePacket(LEDStatus::LEDMode::Flicker),
    ledModePacketCount(3),
    hostname("milight-hub"),
    rf24PowerLevel(RF24PowerLevelHelpers::defaultValue()),
    rf24Channels(RF24ChannelHelpers::allValues()),
    groupStateFields(DEFAULT_GROUP_STATE_FIELDS),
    rf24ListenChannel(RF24Channel::RF24_LOW),
    packetRepeatsPerLoop(10),
    wifiMode(WifiMode::N),
    defaultTransitionPeriod(500),
    _autoRestartPeriod(0)
  { }

  ~Settings() { }

  bool isAuthenticationEnabled() const;
  const String& getUsername() const;
  const String& getPassword() const;

  bool isAutoRestartEnabled();
  size_t getAutoRestartPeriod();

  static void load(Settings& settings);

  static RadioInterfaceType typeFromString(const String& s);
  static String typeToString(RadioInterfaceType type);
  static std::vector<RF24Channel> defaultListenChannels();

  void save();
  String toJson(const bool prettyPrint = true);
  void serialize(Print& stream, const bool prettyPrint = false);
  void updateDeviceIds(JsonArray arr);
  void updateGatewayConfigs(JsonArray arr);
  void patch(JsonObject obj);
  String mqttServer();
  uint16_t mqttPort();
  std::map<String, BulbId>::const_iterator findAlias(MiLightRemoteType deviceType, uint16_t deviceId, uint8_t groupId);

  String adminUsername;
  String adminPassword;
  uint8_t cePin;
  uint8_t csnPin;
  uint8_t resetPin;
  int8_t ledPin;
  RadioInterfaceType radioInterfaceType;
  size_t packetRepeats;
  size_t httpRepeatFactor;
  uint8_t listenRepeats;
  uint16_t discoveryPort;
  String _mqttServer;
  String mqttUsername;
  String mqttPassword;
  String mqttTopicPattern;
  String mqttUpdateTopicPattern;
  String mqttStateTopicPattern;
  String mqttClientStatusTopic;
  bool simpleMqttClientStatus;
  size_t stateFlushInterval;
  size_t mqttStateRateLimit;
  size_t mqttDebounceDelay;
  bool mqttRetain;
  size_t packetRepeatThrottleThreshold;
  size_t packetRepeatThrottleSensitivity;
  size_t packetRepeatMinimum;
  bool enableAutomaticModeSwitching;
  LEDStatus::LEDMode ledModeWifiConfig;
  LEDStatus::LEDMode ledModeWifiFailed;
  LEDStatus::LEDMode ledModeOperating;
  LEDStatus::LEDMode ledModePacket;
  size_t ledModePacketCount;
  String hostname;
  RF24PowerLevel rf24PowerLevel;
  std::vector<uint16_t> deviceIds;
  std::vector<RF24Channel> rf24Channels;
  std::vector<GroupStateField> groupStateFields;
  std::vector<std::shared_ptr<GatewayConfig>> gatewayConfigs;
  RF24Channel rf24ListenChannel;
  String wifiStaticIP;
  String wifiStaticIPNetmask;
  String wifiStaticIPGateway;
  size_t packetRepeatsPerLoop;
  std::map<String, BulbId> groupIdAliases;
  std::map<uint32_t, BulbId> deletedGroupIdAliases;
  String homeAssistantDiscoveryPrefix;
  WifiMode wifiMode;
  uint16_t defaultTransitionPeriod;

protected:
  size_t _autoRestartPeriod;

  void parseGroupIdAliases(JsonObject json);
  void dumpGroupIdAliases(JsonObject json);

  static WifiMode wifiModeFromString(const String& mode);
  static String wifiModeToString(WifiMode mode);

  template <typename T>
  void setIfPresent(JsonObject obj, const char* key, T& var) {
    if (obj.containsKey(key)) {
      JsonVariant val = obj[key];
      var = val.as<T>();
    }
  }
};

#endif
