#include "LEDStatus.h"

LEDStatus::LEDStatus(uint8_t ledPin) {
    _ledPin = ledPin;
    pinMode(_ledPin, OUTPUT);
}

void LEDStatus::handle() {}
