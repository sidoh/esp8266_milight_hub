#pragma once

#include <ArduinoJson.h>

enum MiLightStatus {
  ON = 0,
  OFF = 1
};

MiLightStatus parseMilightStatus(JsonVariant s);