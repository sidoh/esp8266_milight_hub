#include <AboutHelper.h>
#include <ArduinoJson.h>
#include <Settings.h>

#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <cont.h>
#elif ESP32

#include <WiFi.h>
#include <SPIFFS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#endif

#include <ProjectFS.h>

#ifdef ESP8266
extern "C" {
  extern cont_t* g_pcont;
}
#endif

String AboutHelper::generateAboutString(bool abbreviated) {
  DynamicJsonDocument buffer(1024);

  generateAboutObject(buffer, abbreviated);

  String body;
  serializeJson(buffer, body);

  return body;
}

void AboutHelper::generateAboutObject(JsonDocument &obj, bool abbreviated) {
  obj[FPSTR("firmware")] = QUOTE(FIRMWARE_NAME);
  obj[FPSTR("version")] = QUOTE(MILIGHT_HUB_VERSION);
  obj[FPSTR("ip_address")] = WiFi.localIP().toString();
#ifdef ESP8266
  obj[FPSTR("reset_reason")] = ESP.getResetReason();
#elif ESP32
  obj[FPSTR("reset_reason")] = String(esp_reset_reason());
#endif

  if (!abbreviated) {
    obj[FPSTR("variant")] = QUOTE(FIRMWARE_VARIANT);
    obj[FPSTR("free_heap")] = ESP.getFreeHeap();
#ifdef ESP8266
    obj[FPSTR("arduino_version")] = ESP.getCoreVersion();
    obj[FPSTR("free_stack")] = cont_get_free_stack(g_pcont);
#elif ESP32
    obj[FPSTR("arduino_version")] = ESP.getSdkVersion();
    obj[FPSTR("free_stack")] = uxTaskGetStackHighWaterMark(nullptr);
#endif

#ifdef ESP8266
    FSInfo fsInfo;
ProjectFS.info(fsInfo);
obj[FPSTR("flash_used")] = fsInfo.usedBytes;
obj[FPSTR("flash_total")] = fsInfo.totalBytes;
obj[FPSTR("flash_pct_free")] = fsInfo.totalBytes == 0 ? 0 : (fsInfo.totalBytes - fsInfo.usedBytes) * 100 / fsInfo.totalBytes;
#elif ESP32

    obj[FPSTR("flash_used")] = ProjectFS.usedBytes();
    obj[FPSTR("flash_total")] = ProjectFS.totalBytes();
    obj[FPSTR("flash_pct_free")] =
            ProjectFS.totalBytes() == 0 ? 0 : (ProjectFS.totalBytes() - ProjectFS.usedBytes()) * 100 /
                                              ProjectFS.totalBytes();
#endif
  }
}