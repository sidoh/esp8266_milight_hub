
#ifdef ARDUINO
#include "Arduino.h"
#else
#include <stdint.h>
#include <stdlib.h>
#endif

#include <MiLightRadioConfig.h>

#ifndef _MILIGHT_RADIO_H_
#define _MILIGHT_RADIO_H_

class MiLightRadio {
  public:

    virtual int begin() = 0;
    virtual bool available() = 0;
    virtual int read(uint8_t frame[], size_t &frame_length) = 0;
    virtual int write(uint8_t frame[], size_t frame_length) = 0;
    virtual int resend() = 0;
    virtual int configure() = 0;
    virtual const MiLightRadioConfig& config() = 0;

};




#endif
