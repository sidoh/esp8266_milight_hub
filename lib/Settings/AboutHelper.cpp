#include <AboutHelper.h>
#include <ArduinoJson.h>
#include <Settings.h>
#include <ESP8266WiFi.h>

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
  obj["reset_reason"] = ESP.getResetReason();

  if (! abbreviated) {
    obj["variant"] = QUOTE(FIRMWARE_VARIANT);
    obj["free_heap"] = ESP.getFreeHeap();
    obj["arduino_version"] = ESP.getCoreVersion();
  }
}