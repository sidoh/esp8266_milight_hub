#include <AboutStringHelper.h>
#include <ArduinoJson.h>
#include <Settings.h>
#include <ESP8266WiFi.h>

String AboutStringHelper::generateAboutString(bool abbreviated) {
  DynamicJsonBuffer buffer;
  JsonObject& response = buffer.createObject();

  response["firmware"] = QUOTE(FIRMWARE_NAME);
  response["version"] = QUOTE(MILIGHT_HUB_VERSION);
  response["ip_address"] = WiFi.localIP().toString();
  response["reset_reason"] = ESP.getResetReason();

  if (! abbreviated) {
    response["variant"] = QUOTE(FIRMWARE_VARIANT);
    response["free_heap"] = ESP.getFreeHeap();
    response["arduino_version"] = ESP.getCoreVersion();
  }

  String body;
  response.printTo(body);

  return body;
}