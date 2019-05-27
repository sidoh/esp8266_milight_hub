#include <Arduino.h>
#include <RF24.h>

#ifndef _RF24_POWER_LEVEL_H
#define _RF24_POWER_LEVEL_H

enum class RF24PowerLevel {
  RF24_MIN  = RF24_PA_MIN,  // -18 dBm
  RF24_LOW  = RF24_PA_LOW,  // -12 dBm
  RF24_HIGH = RF24_PA_HIGH, //  -6 dBm
  RF24_MAX  = RF24_PA_MAX   //   0 dBm
};

class RF24PowerLevelHelpers {
public:
  static String nameFromValue(const RF24PowerLevel& value);
  static RF24PowerLevel valueFromName(const String& name);
  static RF24PowerLevel defaultValue();
  static uint8_t rf24ValueFromValue(const RF24PowerLevel& vlaue);
};

#endif