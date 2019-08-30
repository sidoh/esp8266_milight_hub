/*
 * PL1167_nRF24.cpp
 *
 * Adapted from work by henryk:
 *  https://github.com/henryk/openmili
 *  Created on: 29 May 2015
 *      Author: henryk
 * Optimizations by khamann:
 *  https://github.com/khmann/esp8266_milight_hub/blob/e3600cef75b102ff3be51a7afdb55ab7460fe712/lib/MiLight/PL1167_nRF24.cpp
 *
 */

#include "PL1167_nRF24.h"
#include <RadioUtils.h>
#include <MiLightRadioConfig.h>

static uint16_t calc_crc(uint8_t *data, size_t data_length);

PL1167_nRF24::PL1167_nRF24(RF24 &radio)
  : _radio(radio)
{ }

int PL1167_nRF24::open() {
  _radio.begin();
  _radio.setAutoAck(false);
  _radio.setDataRate(RF24_1MBPS);
  _radio.disableCRC();

  _syncwordLength = MiLightRadioConfig::SYNCWORD_LENGTH;
  _radio.setAddressWidth(_syncwordLength);

  return recalc_parameters();
}

int PL1167_nRF24::recalc_parameters() {
  size_t nrf_address_length = _syncwordLength;

  // +2 for CRC
  size_t packet_length = _maxPacketLength + 2;

  // Read an extra byte if we don't include the trailer in the syncword
  if (_syncwordLength < 5) {
    ++packet_length;
  }

  if (packet_length > sizeof(_packet) || nrf_address_length < 3) {
    return -1;
  }

  if (_syncwordBytes != nullptr) {
    _radio.openWritingPipe(_syncwordBytes);
    _radio.openReadingPipe(1, _syncwordBytes);
  }

  _receive_length = packet_length;

  _radio.setChannel(2 + _channel);
  _radio.setPayloadSize( packet_length );

  return 0;
}

int PL1167_nRF24::setSyncword(const uint8_t syncword[], size_t syncwordLength) {
  _syncwordLength = syncwordLength;
  _syncwordBytes = syncword;
  return recalc_parameters();
}

int PL1167_nRF24::setMaxPacketLength(uint8_t maxPacketLength) {
  _maxPacketLength = maxPacketLength;
  return recalc_parameters();
}

int PL1167_nRF24::receive(uint8_t channel) {
  if (channel != _channel) {
    _channel = channel;
    int retval = recalc_parameters();
    if (retval < 0) {
      return retval;
    }
  }

  _radio.startListening();
  if (_radio.available()) {
#ifdef DEBUG_PRINTF
  printf("Radio is available\n");
#endif
    internal_receive();
  }

  if(_received) {
#ifdef DEBUG_PRINTF
  if (_packet_length > 0) {
    printf("Received packet (len = %d)!\n", _packet_length);
  }
#endif
    return _packet_length;
  } else {
    return 0;
  }
}

int PL1167_nRF24::readFIFO(uint8_t data[], size_t &data_length)
{
  if (data_length > _packet_length) {
    data_length = _packet_length;
  }
  memcpy(data, _packet, data_length);
  _packet_length -= data_length;
  if (_packet_length) {
    memmove(_packet, _packet + data_length, _packet_length);
  }
  return _packet_length;
}

int PL1167_nRF24::writeFIFO(const uint8_t data[], size_t data_length)
{
  if (data_length > sizeof(_packet)) {
    data_length = sizeof(_packet);
  }
  memcpy(_packet, data, data_length);
  _packet_length = data_length;
  _received = false;

  return data_length;
}

int PL1167_nRF24::transmit(uint8_t channel) {
  if (channel != _channel) {
    _channel = channel;
    int retval = recalc_parameters();
    if (retval < 0) {
      return retval;
    }
    yield();
  }

  _radio.stopListening();
  uint8_t tmp[sizeof(_packet)];
  int outp=0;

  uint16_t crc = calc_crc(_packet, _packet_length);

  // +1 for packet length
  // +2 for crc
  // = 3
  for (int inp = 0; inp < _packet_length + 3; inp++) {
    if (inp < _packet_length) {
      tmp[outp++] = reverseBits(_packet[inp]);}
    else if (inp < _packet_length + 2) {
      tmp[outp++] = reverseBits((crc >> ( (inp - _packet_length) * 8)) & 0xff);
    }
  }

  yield();

  _radio.write(tmp, outp);
  return 0;
}

/**
 * The over-the-air packet structure sent by the PL1167 is as follows (lengths
 * measured in bits)
 *
 * Preamble ( 8) | Syncword (32) | Trailer ( 4) | Packet Len ( 8) | Packet (...)
 *
 * Note that because the Trailer is 4 bits, the remaining data is not byte-aligned.
 *
 * Bit-order is reversed.
 *
 */
int PL1167_nRF24::internal_receive() {
  uint8_t tmp[sizeof(_packet)];
  int outp = 0;

  _radio.read(tmp, _receive_length);

  // HACK HACK HACK: Reset radio
  open();

// Currently, the syncword width is set to 5 in order to include the
// PL1167 trailer.  The trailer is 4 bits, which pushes packet data
// out of byte-alignment.
//
// The following code reads un-byte-aligned packet data.
//
// #ifdef DEBUG_PRINTF
//   Serial.printf_P(PSTR("Packet received (%d bytes) RAW: "), outp);
//   for (int i = 0; i < _receive_length; i++) {
//     Serial.printf_P(PSTR("%02X "), tmp[i]);
//   }
//   Serial.print(F("\n"));
// #endif
//
//   uint16_t buffer = tmp[0];
//
//   for (int inp = 1; inp < _receive_length; inp++) {
//     uint8_t currentByte = tmp[inp];
//     tmp[outp++] = reverseBits((buffer << 4) | (currentByte >> 4));
//     buffer = (buffer << 8) | currentByte;
//   }

  for (int inp = 0; inp < _receive_length; inp++) {
    tmp[outp++] = reverseBits(tmp[inp]);
  }

#ifdef DEBUG_PRINTF
  Serial.printf_P(PSTR("Packet received (%d bytes): "), outp);
  for (int i = 0; i < outp; i++) {
    Serial.printf_P(PSTR("%02X "), tmp[i]);
  }
  Serial.print(F("\n"));
#endif

  if (outp < 2) {
#ifdef DEBUG_PRINTF
    Serial.println(F("Failed CRC: outp < 2"));
#endif
    return 0;
  }

  uint16_t crc = calc_crc(tmp, outp - 2);
  uint16_t recvCrc = (tmp[outp - 1] << 8) | tmp[outp - 2];

  if ( crc != recvCrc ) {
#ifdef DEBUG_PRINTF
    Serial.printf_P(PSTR("Failed CRC: expected %04X, got %04X\n"), crc, recvCrc);
#endif
    return 0;
  }
  outp -= 2;

  memcpy(_packet, tmp, outp);

  _packet_length = outp;
  _received = true;

#ifdef DEBUG_PRINTF
  Serial.printf_P(PSTR("Successfully parsed packet of length %d\n"), _packet_length);
#endif

  return outp;
}

#define CRC_POLY 0x8408

static uint16_t calc_crc(uint8_t *data, size_t data_length) {
  uint16_t state = 0;
  for (size_t i = 0; i < data_length; i++) {
    uint8_t byte = data[i];
    for (int j = 0; j < 8; j++) {
      if ((byte ^ state) & 0x01) {
        state = (state >> 1) ^ CRC_POLY;
      } else {
        state = state >> 1;
      }
      byte = byte >> 1;
    }
  }
  return state;
}