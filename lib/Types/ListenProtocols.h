#include <Arduino.h>

#ifndef _LISTEN_PROTOCOLS_H
#define _LISTEN_PROTOCOLS_H

// Convert from indices in MiLightRadioConfig::ALL_CONFIGS to names
class ListenProtocolHelpers {
public:
    static String nameFromIndex(const uint8_t index);
    static uint8_t indexFromName(const String& name);
    static constexpr uint8_t numProtocols() {
        return 5;
    }
};

#endif