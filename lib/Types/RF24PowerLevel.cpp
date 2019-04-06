#include <RF24PowerLevel.h>
#include <Size.h>

static const char* RF24_POWER_LEVEL_NAMES[] = {
  "MIN",
  "LOW",
  "HIGH",
  "MAX"
};

String RF24PowerLevelHelpers::nameFromValue(const RF24PowerLevel& value) {
  const size_t ix = static_cast<size_t>(value);

  if (ix >= size(RF24_POWER_LEVEL_NAMES)) {
    Serial.println(F("ERROR: unknown RF24 power level label - this is a bug!"));
    return nameFromValue(defaultValue());
  }

  return RF24_POWER_LEVEL_NAMES[ix];
}

RF24PowerLevel RF24PowerLevelHelpers::valueFromName(const String& name) {
  for (size_t i = 0; i < size(RF24_POWER_LEVEL_NAMES); ++i) {
    if (name == RF24_POWER_LEVEL_NAMES[i]) {
      return static_cast<RF24PowerLevel>(i);
    }
  }

  Serial.printf_P(PSTR("WARN: tried to fetch unknown RF24 power level: %s, using default.\n"), name.c_str());

  return defaultValue();
}

uint8_t RF24PowerLevelHelpers::rf24ValueFromValue(const RF24PowerLevel& rF24PowerLevel) {
  return static_cast<uint8_t>(rF24PowerLevel);
}

RF24PowerLevel RF24PowerLevelHelpers::defaultValue() {
  return RF24PowerLevel::RF24_MAX;
}