
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

    virtual int begin();
    virtual bool available();
    virtual int read(uint8_t frame[], size_t &frame_length);
    virtual int write(uint8_t frame[], size_t frame_length);
    virtual int resend();
    virtual int configure();
    virtual const MiLightRadioConfig& config();

};




#endif
