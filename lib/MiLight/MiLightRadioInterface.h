
#ifdef ARDUINO
#include "Arduino.h"
#else
#include <stdint.h>
#include <stdlib.h>
#endif

#ifndef MILIGHTRADIOINTERFACE_H_
#define MILIGHTRADIOINTERFACE_H_

class MiLightRadioInterface
{
  public:

    virtual int begin();
    virtual bool available();
    virtual int read(uint8_t frame[], size_t &frame_length);
    virtual int dupesReceived();
    virtual int write(uint8_t frame[], size_t frame_length);
    virtual int resend();
    virtual int configure();

};




#endif /* MILIGHTRADIOINTERFACE_H_ */
