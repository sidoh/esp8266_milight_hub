/*
 * PL1167_nRF24.cpp
 *
 *  Created on: 29 May 2015
 *      Author: henryk
 */

#include "PL1167_nRF24.h"

static uint16_t calc_crc(uint8_t *data, size_t data_length);
static uint8_t reverse_bits(uint8_t data);
static void demangle_packet(uint8_t *in, uint8_t *out) ;

PL1167_nRF24::PL1167_nRF24(RF24 &radio)
  : _radio(radio) { }

static const uint8_t pipe[] = {0xd1, 0x28, 0x5e, 0x55, 0x55};

int PL1167_nRF24::open()
{
  _radio.begin();
  return recalc_parameters();
}

int PL1167_nRF24::recalc_parameters()
{
  int nrf_address_length = _preambleLength - 1 + _syncwordLength;
  int address_overflow = 0;
  if (nrf_address_length > 5) {
    address_overflow = nrf_address_length - 5;
    nrf_address_length = 5;
  }
  int packet_length = address_overflow + ( (_trailerLength + 7) / 8) + _maxPacketLength;
  if (_crc) {
    packet_length += 2;
  }

  if (packet_length > sizeof(_packet) || nrf_address_length < 3) {
    return -1;
  }

  uint8_t preamble = 0;
  if (_syncword0 & 0x01) {
    preamble = 0x55;
  } else {
    preamble = 0xAA;
  }

  int nrf_address_pos = nrf_address_length;
  for (int i = 0; i < _preambleLength - 1; i++) {
    _nrf_pipe[ --nrf_address_pos ] = reverse_bits(preamble);
  }

  if (nrf_address_pos) {
    _nrf_pipe[ --nrf_address_pos ] = reverse_bits(_syncword0 & 0xff);
  }
  if (nrf_address_pos) {
    _nrf_pipe[ --nrf_address_pos ] = reverse_bits( (_syncword0 >> 8) & 0xff);
  }

  if (_syncwordLength == 4) {
    if (nrf_address_pos) {
      _nrf_pipe[ --nrf_address_pos ] = reverse_bits(_syncword3 & 0xff);
    }
    if (nrf_address_pos) {
      _nrf_pipe[ --nrf_address_pos ] = reverse_bits( (_syncword3 >> 8) & 0xff);
    }
  }

  _receive_length = packet_length;
  _preamble = preamble;

  _nrf_pipe_length = nrf_address_length;
  _radio.setAddressWidth(_nrf_pipe_length);
  _radio.openWritingPipe(_nrf_pipe);
  _radio.openReadingPipe(1, _nrf_pipe);

  _radio.setChannel(2 + _channel);


  _radio.setPayloadSize( packet_length );
  _radio.setAutoAck(false);
  _radio.setPALevel(RF24_PA_MAX);
  _radio.setDataRate(RF24_1MBPS);
  _radio.disableCRC();

  return 0;
}


int PL1167_nRF24::setPreambleLength(uint8_t preambleLength)
{
  if (preambleLength > 8) {
    return -1;
  }
  _preambleLength = preambleLength;
  return recalc_parameters();
}


int PL1167_nRF24::setSyncword(uint16_t syncword0, uint16_t syncword3)
{
  _syncwordLength = 4;
  _syncword0 = syncword0;
  _syncword3 = syncword3;
  return recalc_parameters();
}

int PL1167_nRF24::setTrailerLength(uint8_t trailerLength)
{
  if (trailerLength < 4) {
    return -1;
  }
  if (trailerLength > 18) {
    return -1;
  }
  if (trailerLength & 0x01) {
    return -1;
  }
  _trailerLength = trailerLength;
  return recalc_parameters();
}

int PL1167_nRF24::setCRC(bool crc)
{
  _crc = crc;
  return recalc_parameters();
}

int PL1167_nRF24::setMaxPacketLength(uint8_t maxPacketLength)
{
  _maxPacketLength = maxPacketLength;
  return recalc_parameters();
}

int PL1167_nRF24::receive(uint8_t channel)
{
  if (channel != _channel) {
    _channel = channel;
    int retval = recalc_parameters();
    if (retval < 0) {
      return retval;
    }
  }

  _radio.startListening();
  if (_radio.available()) {
    internal_receive();
  }

  if(_received) {
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

int PL1167_nRF24::transmit(uint8_t channel)
{
  if (channel != _channel) {
    _channel = channel;
    int retval = recalc_parameters();
    if (retval < 0) {
      return retval;
    }
  }

  _radio.stopListening();
  uint8_t tmp[sizeof(_packet)];

  uint8_t trailer = (_packet[0] & 1) ? 0x55 : 0xAA;  // NOTE: This is a guess, it might also be based upon the last
  // syncword bit, or fixed
  int outp = 0;

  for (; outp < _receive_length; outp++) {
    uint8_t outbyte = 0;

    if (outp + 1 + _nrf_pipe_length < _preambleLength) {
      outbyte = _preamble;
    } else if (outp + 1 + _nrf_pipe_length < _preambleLength + _syncwordLength) {
      int syncp = outp - _preambleLength + 1 + _nrf_pipe_length;
      switch (syncp) {
        case 0:
          outbyte = _syncword0 & 0xFF;
          break;
        case 1:
          outbyte = (_syncword0 >> 8) & 0xFF;
          break;
        case 2:
          outbyte = _syncword3 & 0xFF;
          break;
        case 3:
          outbyte = (_syncword3 >> 8) & 0xFF;
          break;
      }
    } else if (outp + 1 + _nrf_pipe_length < _preambleLength + _syncwordLength + (_trailerLength / 8) ) {
      outbyte = trailer;
    } else {
      break;
    }

    tmp[outp] = reverse_bits(outbyte);
  }

  int buffer_fill;
  bool last_round = false;
  uint16_t buffer = 0;
  uint16_t crc;
  if (_crc) {
    crc = calc_crc(_packet, _packet_length);
  }

  buffer = trailer >> (8 - (_trailerLength % 8));
  buffer_fill = _trailerLength % 8;
  for (int inp = 0; inp < _packet_length + (_crc ? 2 : 0) + 1; inp++) {
    if (inp < _packet_length) {
      buffer |= _packet[inp] << buffer_fill;
      buffer_fill += 8;
    } else if (_crc && inp < _packet_length + 2) {
      buffer |= ((crc >>  ( (inp - _packet_length) * 8)) & 0xff) << buffer_fill;
      buffer_fill += 8;
    } else {
      last_round = true;
    }

    while (buffer_fill > (last_round ? 0 : 8)) {
      if (outp >= sizeof(tmp)) {
        return -1;
      }
      tmp[outp++] = reverse_bits(buffer & 0xff);
      buffer >>= 8;
      buffer_fill -= 8;
    }
  }

  _radio.write(tmp, outp);
  return 0;
}


int PL1167_nRF24::internal_receive()
{
  uint8_t tmp[sizeof(_packet)];
  int outp = 0;

  _radio.read(tmp, _receive_length);

  // HACK HACK HACK: Reset radio
  open();

  uint8_t shift_amount = _trailerLength % 8;
  uint16_t buffer = 0;

#ifdef DEBUG_PRINTF
  printf("Packet received: ");
  for (int i = 0; i < _receive_length; i++) {
    printf("%02X", reverse_bits(tmp[i]));
  }
  printf("\n");
#endif

  for (int inp = 0; inp < _receive_length; inp++) {
    uint8_t inbyte = reverse_bits(tmp[inp]);
    buffer = (buffer >> 8) | (inbyte << 8);

    if (inp + 1 + _nrf_pipe_length < _preambleLength) {
      if (inbyte != _preamble) {
#ifdef DEBUG_PRINTF
        printf("Preamble fail (%i: %02X)\n", inp, inbyte);
#endif
        return 0;
      }
    } else if (inp + 1 + _nrf_pipe_length < _preambleLength + _syncwordLength) {
      int syncp = inp - _preambleLength + 1 + _nrf_pipe_length;
      switch (syncp) {
        case 0:
          if (inbyte != _syncword0 & 0xFF) {
#ifdef DEBUG_PRINTF
            printf("Sync 0l fail (%i: %02X)\n", inp, inbyte);
#endif
            return 0;
          } break;
        case 1:
          if (inbyte != (_syncword0 >> 8) & 0xFF) {
#ifdef DEBUG_PRINTF
            printf("Sync 0h fail (%i: %02X)\n", inp, inbyte);
#endif
            return 0;
          } break;
        case 2:
          if ((_syncwordLength == 4) && (inbyte != _syncword3 & 0xFF)) {
#ifdef DEBUG_PRINTF
            printf("Sync 3l fail (%i: %02X)\n", inp, inbyte);
#endif
            return 0;
          } break;
        case 3:
          if ((_syncwordLength == 4) && (inbyte != (_syncword3 >> 8) & 0xFF)) {
#ifdef DEBUG_PRINTF
            printf("Sync 3h fail (%i: %02X)\n", inp, inbyte);
#endif
            return 0;
          } break;
      }
    } else if (inp + 1 + _nrf_pipe_length < _preambleLength + _syncwordLength + ((_trailerLength + 7) / 8) ) {

    } else {
      tmp[outp++] = buffer >> shift_amount;
    }
  }


#ifdef DEBUG_PRINTF
  printf("Packet transformed: ");
  for (int i = 0; i < outp; i++) {
    printf("%02X", tmp[i]);
  }
  printf("\n");
#endif


  if (_crc) {
    if (outp < 2) {
      return 0;
    }
    uint16_t crc = calc_crc(tmp, outp - 2);
    if ( ((crc & 0xff) != tmp[outp - 2]) || (((crc >> 8) & 0xff) != tmp[outp - 1]) ) {
      return 0;
    }
    outp -= 2;
  }

  memcpy(_packet, tmp, outp);
  _packet_length = outp;
  _received = true;
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

static uint8_t reverse_bits(uint8_t data) {
  uint8_t result = 0;
  for (int i = 0; i < 8; i++) {
    result <<= 1;
    result |= data & 1;
    data >>= 1;
  }
  return result;
}
