/*
 * PL1167_nRF24.h
 *
 *  Created on: 29 May 2015
 *      Author: henryk
 */

#ifdef ARDUINO
#include "Arduino.h"
#endif

#include "RF24.h"

// #define DEBUG_PRINTF

#ifndef PL1167_NRF24_H_
#define PL1167_NRF24_H_

class PL1167_nRF24 {
  public:
    PL1167_nRF24(RF24& radio);
    int open();

    int setSyncword(const uint8_t syncword[], size_t syncwordLength);
    int setMaxPacketLength(uint8_t maxPacketLength);

    int writeFIFO(const uint8_t data[], size_t data_length);
    int transmit(uint8_t channel);
    int receive(uint8_t channel);
    int readFIFO(uint8_t data[], size_t &data_length);

  private:
    RF24 &_radio;

    const uint8_t* _syncwordBytes = nullptr;
    uint8_t _syncwordLength = 4;
    uint8_t _maxPacketLength = 8;

    uint8_t _channel = 0;

    uint8_t _nrf_pipe[5];
    uint8_t _nrf_pipe_length;

    uint8_t _packet_length = 0;
    uint8_t _receive_length = 0;
    uint8_t _preamble = 0;
    uint8_t _packet[32];
    bool _received = false;

    int recalc_parameters();
    int internal_receive();

};


#endif /* PL1167_NRF24_H_ */
