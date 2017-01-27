/*
 * MiLightRadio.h
 *
 *  Created on: 29 May 2015
 *      Author: henryk
 */

#ifdef ARDUINO
#include "Arduino.h"
#else
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "AbstractPL1167.h"

#ifndef MILIGHTRADIO_H_
#define MILIGHTRADIO_H_

class MiLightRadio {
  public:
    MiLightRadio(AbstractPL1167 &pl1167);
    int begin();
    bool available();
    int read(uint8_t frame[], size_t &frame_length);
    int dupesReceived();
    int write(uint8_t frame[], size_t frame_length);
    int resend();
  private:
    AbstractPL1167 &_pl1167;
    uint32_t _prev_packet_id;

    uint8_t _packet[8], _out_packet[8];
    bool _waiting;
    int _dupes_received;
};



#endif /* MILIGHTRADIO_H_ */
