#include <Arduino.h>
#include <StringStream.h>
#include <ArduinoJson.h>
//Eigene Includes
#include <IPAddress.h>


#ifndef _SETTINGS_H_INCLUDED
#define _SETTINGS_H_INCLUDED

#define XQUOTE(x) #x
#define QUOTE(x) XQUOTE(x)

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

enum RadioInterfaceType {
  nRF24 = 0,
  LT8900 = 1,
};

class GatewayConfig {
public:
  GatewayConfig(uint16_t deviceId, uint16_t port, uint8_t protocolVersion)
    : deviceId(deviceId),
      port(port),
      protocolVersion(protocolVersion)
    { }

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
    cePin(D0),
    csnPin(D8),
    resetPin(0),
    radioInterfaceType(nRF24),
    deviceIds(NULL),
    gatewayConfigs(NULL),
    numDeviceIds(0),
    numGatewayConfigs(0),
    packetRepeats(50),
    httpRepeatFactor(1),
    listenRepeats(3),
    _autoRestartPeriod(0),
    discoveryPort(48899),
    stateFlushInterval(10000),
    mqttStateRateLimit(500),
    packetRepeatThrottleThreshold(200),
    packetRepeatThrottleSensitivity(0),
    packetRepeatMinimum(3)
  { }

  ~Settings() {
    if (deviceIds) {
      delete deviceIds;
    }
  }

  bool hasAuthSettings();
  bool isAutoRestartEnabled();
  size_t getAutoRestartPeriod();

  static void deserialize(Settings& settings, String json);
  static void load(Settings& settings);

  static RadioInterfaceType typeFromString(const String& s);
  static String typeToString(RadioInterfaceType type);

  void save();
  String toJson(const bool prettyPrint = true);
  void serialize(Stream& stream, const bool prettyPrint = false);
  void updateDeviceIds(JsonArray& arr);
  void updateGatewayConfigs(JsonArray& arr);
  void patch(JsonObject& obj);
  String mqttServer();
  uint16_t mqttPort();

  String adminUsername;
  String adminPassword;
  uint8_t cePin;
  uint8_t csnPin;
  uint8_t resetPin;
  RadioInterfaceType radioInterfaceType;
  uint16_t *deviceIds;
  GatewayConfig **gatewayConfigs;
  size_t numGatewayConfigs;
  size_t numDeviceIds;
  size_t packetRepeats;
  size_t httpRepeatFactor;
  String _mqttServer;
  String mqttUsername;
  String mqttPassword;
  String mqttTopicPattern;
  String mqttUpdateTopicPattern;
  String mqttStateTopicPattern;
  uint16_t discoveryPort;
  uint8_t listenRepeats;
  size_t stateFlushInterval;
  size_t mqttStateRateLimit;
  size_t packetRepeatThrottleSensitivity;
  size_t packetRepeatThrottleThreshold;
  size_t packetRepeatMinimum;

  //Eigene Variablen
  String hostname;
  String otaPass;
  String staticIp;
  String staticMask;
  String staticGate;
  IPAddress _staticIp;
  IPAddress _staticMask;
  IPAddress _staticGate;
  uint8_t sdaPin;
  uint8_t sclPin;
  uint8_t mqttPin1;
  uint8_t mqttPin2;
  uint8_t mqttPin3;
  uint8_t mqttPin4;
  String mqttClientId;
  String mqttSensorTopicPattern;

protected:
  size_t _autoRestartPeriod;

  template <typename T>
  void setIfPresent(JsonObject& obj, const char* key, T& var) {
    if (obj.containsKey(key)) {
      var = obj.get<T>(key);
    }
  }
};

#endif
