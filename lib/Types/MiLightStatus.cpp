#include <MiLightStatus.h>
#include <ArduinoJson.h>

MiLightStatus parseMilightStatus(JsonVariant val) {
  if (val.is<bool>()) {
    return val.as<bool>() ? ON : OFF;
  } else if (val.is<uint16_t>()) {
    return static_cast<MiLightStatus>(val.as<uint16_t>());
  } else {
    String strStatus(val.as<const char*>());
    return (strStatus.equalsIgnoreCase("on") || strStatus.equalsIgnoreCase("true")) ? ON : OFF;
  }
}