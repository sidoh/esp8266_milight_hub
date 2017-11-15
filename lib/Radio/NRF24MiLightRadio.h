#ifdef ARDUINO
#include "Arduino.h"
#else
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <RF24.h>
#include <PL1167_nRF24.h>
#include <MiLightRadioConfig.h>
#include <MiLightRadio.h>

#ifndef _NRF24_MILIGHT_RADIO_H_
#define _NRF24_MILIGHT_RADIO_H_

class NRF24MiLightRadio : public MiLightRadio {
  public:
    NRF24MiLightRadio(RF24& rf, const MiLightRadioConfig& config);

    int begin();
    bool available();
    int read(uint8_t frame[], size_t &frame_length);
    int dupesReceived();
    int write(uint8_t frame[], size_t frame_length);
    int resend();
    int configure();
    const MiLightRadioConfig& config();

  private:
    PL1167_nRF24 _pl1167;
    const MiLightRadioConfig& _config;
    uint32_t _prev_packet_id;

    uint8_t _packet[10];
    uint8_t _out_packet[10];
    bool _waiting;
    int _dupes_received;
};



#endif
