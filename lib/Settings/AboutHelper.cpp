#include <AboutHelper.h>
#include <ArduinoJson.h>
#include <Settings.h>
#include <ESP8266WiFi.h>
#include <ProjectFS.h>

extern "C" {
#include <cont.h>
extern cont_t* g_pcont;
};

String AboutHelper::generateAboutString(bool abbreviated) {
  DynamicJsonDocument buffer(1024);

  generateAboutObject(buffer, abbreviated);

  String body;
  serializeJson(buffer, body);

  return body;
}

void AboutHelper::generateAboutObject(JsonDocument& obj, bool abbreviated) {
  obj[FPSTR("firmware")] = QUOTE(FIRMWARE_NAME);
  obj[FPSTR("version")] = QUOTE(MILIGHT_HUB_VERSION);
  obj[FPSTR("ip_address")] = WiFi.localIP().toString();
  obj[FPSTR("reset_reason")] = ESP.getResetReason();

  if (! abbreviated) {
    obj[FPSTR("variant")] = QUOTE(FIRMWARE_VARIANT);
    obj[FPSTR("free_heap")] = ESP.getFreeHeap();
    obj[FPSTR("arduino_version")] = ESP.getCoreVersion();
    obj[FPSTR("free_stack")] = cont_get_free_stack(g_pcont);

    FSInfo fsInfo;
    ProjectFS.info(fsInfo);
    obj[FPSTR("flash_used")] = fsInfo.usedBytes;
    obj[FPSTR("flash_total")] = fsInfo.totalBytes;
    obj[FPSTR("flash_pct_free")] = fsInfo.totalBytes == 0 ? 0 : (fsInfo.totalBytes - fsInfo.usedBytes) * 100 / fsInfo.totalBytes;
  }
}