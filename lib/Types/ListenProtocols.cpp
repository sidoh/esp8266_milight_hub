#include <ListenProtocols.h>

#include <Size.h>

// Must match MiLightRadioConfig::ALL_CONFIGS
static const char* PROTOCOL_NAMES[] = {
  "RGBW",
  "CCT",
  "FUT089",
  "RGB",
  "FUT020"
};

static_assert(size(PROTOCOL_NAMES) == ListenProtocolHelpers::numProtocols(),
    "Protocol names do not match");

String ListenProtocolHelpers::nameFromIndex(const uint8_t index)
{
    if (index < numProtocols()) {
        printf("Protocol %d is %s", (int)index, PROTOCOL_NAMES[index]);
        return PROTOCOL_NAMES[index];
    }
    Serial.println(F("ERROR: unknown listen protocol label - this is a bug!"));
    return PROTOCOL_NAMES[0];
}

uint8_t ListenProtocolHelpers::indexFromName(const String& name)
{
    for (size_t i = 0; i < size(PROTOCOL_NAMES); ++i) {
        if (name == PROTOCOL_NAMES[i]) {
            printf("Protocol %s is %d", name.c_str(), (int)i);
            return static_cast<uint8_t>(i);
        }
    }
    Serial.printf_P(PSTR("WARN: tried to fetch unknown listen protocol: %s, using default.\n"), name.c_str());

    return 0;
}
