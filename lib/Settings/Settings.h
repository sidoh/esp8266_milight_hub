#include <Arduino.h>
#include <StringStream.h>
#include <ArduinoJson.h>

#ifndef _SETTINGS_H_INCLUDED
#define _SETTINGS_H_INCLUDED

#define SETTINGS_FILE  "/config.json"
#define SETTINGS_TERMINATOR '\0'

#define WEB_INDEX_FILENAME "/index.html"

class Settings {
public:
  Settings() :
    adminUsername(""),
    adminPassword(""),
    // CE and CSN pins from nrf24l01
    cePin(D0),
    csnPin(D8)
  { }

  static void deserialize(Settings& settings, String json);
  static void deserialize(Settings& settings, JsonObject& json);
  static void load(Settings& settings);
  
  void save();
  String toJson(const bool prettyPrint = true);
  void serialize(Stream& stream, const bool prettyPrint = false);
  
  String adminUsername;
  String adminPassword;
  uint8_t cePin;
  uint8_t csnPin;
};

#endif 