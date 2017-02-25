#include <Arduino.h>
#include <StringStream.h>
#include <ArduinoJson.h>

#ifndef _SETTINGS_H_INCLUDED
#define _SETTINGS_H_INCLUDED

#define SETTINGS_FILE  "/config.json"
#define SETTINGS_TERMINATOR '\0'

#define WEB_INDEX_FILENAME "/index.html"

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
    deviceIds(NULL),
    gatewayConfigs(NULL),
    numDeviceIds(0),
    numGatewayConfigs(0),
    packetRepeats(10),
    httpRepeatFactor(5)
  { }
  
  ~Settings() {
    if (deviceIds) {
      delete deviceIds;
    }
  }
  
  bool hasAuthSettings() {
    return adminUsername.length() > 0 && adminPassword.length() > 0;
  }

  static void deserialize(Settings& settings, String json);
  static void deserialize(Settings& settings, JsonObject& json);
  static void load(Settings& settings);
  
  void save();
  String toJson(const bool prettyPrint = true);
  void serialize(Stream& stream, const bool prettyPrint = false);
  void updateDeviceIds(JsonArray& arr);
  void updateGatewayConfigs(JsonArray& arr);
  void patch(JsonObject& obj);
  
  String adminUsername;
  String adminPassword;
  uint8_t cePin;
  uint8_t csnPin;
  uint16_t *deviceIds;
  GatewayConfig **gatewayConfigs;
  size_t numGatewayConfigs;
  size_t numDeviceIds;
  size_t packetRepeats;
  size_t httpRepeatFactor;
};

#endif 