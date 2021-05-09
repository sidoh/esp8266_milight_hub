#include <AboutHelper.h>
#include <ArduinoJson.h>
#include <Settings.h>

#ifdef ESP8266
  #include <ESP8266WiFi.h>
#elif ESP32
  #include <WiFi.h>
#endif

String AboutHelper::generateAboutString(bool abbreviated) {
  DynamicJsonDocument buffer(1024);

  generateAboutObject(buffer, abbreviated);

  String body;
  serializeJson(buffer, body);

  return body;
}

void AboutHelper::generateAboutObject(JsonDocument& obj, bool abbreviated) {
  obj["firmware"] = QUOTE(FIRMWARE_NAME);
  obj["version"] = QUOTE(MILIGHT_HUB_VERSION);
  obj["ip_address"] = WiFi.localIP().toString();
#ifdef ESP8266
  obj["reset_reason"] = ESP.getResetReason();
#elif ESP32
  // TODO get reset reason
#endif

  if (! abbreviated) {
    obj["variant"] = QUOTE(FIRMWARE_VARIANT);
    obj["free_heap"] = ESP.getFreeHeap();
#ifdef ESP8266
    obj["arduino_version"] = ESP.getCoreVersion();
#elif ESP32
    obj["arduino_version"] = ESP.getSdkVersion();
#endif
  }
}