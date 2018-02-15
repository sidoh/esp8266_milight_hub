#include <Arduino.h>

#ifndef _LED_STATUS_H
#define _LED_STATUS_H

class LEDStatus {
    public:
        LEDStatus(uint8_t ledPin);
        void handle();

    private:
        uint8_t _ledPin;
};

#endif