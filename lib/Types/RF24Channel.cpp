#include <Size.h>
#include <RF24Channel.h>

static const char* RF24_CHANNEL_NAMES[] = {
  "LOW",
  "MID",
  "HIGH"
};

String RF24ChannelHelpers::nameFromValue(const RF24Channel& value) {
  const size_t ix = static_cast<size_t>(value);

  if (ix >= size(RF24_CHANNEL_NAMES)) {
    Serial.println(F("ERROR: unknown RF24 channel label - this is a bug!"));
    return nameFromValue(defaultValue());
  }

  return RF24_CHANNEL_NAMES[ix];
}

RF24Channel RF24ChannelHelpers::valueFromName(const String& name) {
  for (size_t i = 0; i < size(RF24_CHANNEL_NAMES); ++i) {
    if (name == RF24_CHANNEL_NAMES[i]) {
      return static_cast<RF24Channel>(i);
    }
  }

  Serial.printf_P(PSTR("WARN: tried to fetch unknown RF24 channel: %s, using default.\n"), name.c_str());

  return defaultValue();
}

RF24Channel RF24ChannelHelpers::defaultValue() {
  return RF24Channel::RF24_HIGH;
}

std::vector<RF24Channel> RF24ChannelHelpers::allValues() {
  std::vector<RF24Channel> vec;

  for (size_t i = 0; i < size(RF24_CHANNEL_NAMES); ++i) {
    vec.push_back(valueFromName(RF24_CHANNEL_NAMES[i]));
  }

  return vec;
}