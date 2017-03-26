/*
 * MiLightRadio.cpp
 *
 *  Created on: 29 May 2015
 *      Author: henryk
 */

#include "MiLightRadio.h"

#define PACKET_ID(packet, packet_length) ( (packet[1] << 8) | packet[packet_length - 1] )

MiLightRadio::MiLightRadio(AbstractPL1167 &pl1167, const MiLightRadioConfig& config)
  : _pl1167(pl1167), config(config) {
  _waiting = false;
}

int MiLightRadio::begin()
{
  int retval = _pl1167.open();
  if (retval < 0) {
    return retval;
  }
  
  retval = configure();
  if (retval < 0) {
    return retval;
  }

  available();

  return 0;
}

int MiLightRadio::configure() {
  int retval = _pl1167.setCRC(true);
  if (retval < 0) {
    return retval;
  }

  retval = _pl1167.setPreambleLength(3);
  if (retval < 0) {
    return retval;
  }

  retval = _pl1167.setTrailerLength(4);
  if (retval < 0) {
    return retval;
  }

  retval = _pl1167.setSyncword(config.syncword0, config.syncword3);
  if (retval < 0) {
    return retval;
  }

  // +1 to be able to buffer the length 
  retval = _pl1167.setMaxPacketLength(config.getPacketLength() + 1);
  if (retval < 0) {
    return retval;
  }
  
  return 0;
}

bool MiLightRadio::available() {
  if (_waiting) {
#ifdef DEBUG_PRINTF
  printf("_waiting\n");
#endif
    return true;
  }
  
  if (_pl1167.receive(config.channels[0]) > 0) {
#ifdef DEBUG_PRINTF
  printf("MiLightRadio - received packet!\n");
#endif
    size_t packet_length = sizeof(_packet);
    if (_pl1167.readFIFO(_packet, packet_length) < 0) {
      return false;
    }
#ifdef DEBUG_PRINTF
  printf("MiLightRadio - Checking packet length (expecting %d, is %d)\n", _packet[0] + 1U, packet_length);
#endif
    if (packet_length == 0 || packet_length != _packet[0] + 1U) {
      return false;
    }
    uint32_t packet_id = PACKET_ID(_packet, packet_length);
#ifdef DEBUG_PRINTF
  printf("Packet id: %d\n", packet_id);
#endif
    if (packet_id == _prev_packet_id) {
      _dupes_received++;
    } else {
      _prev_packet_id = packet_id;
      _waiting = true;
    }
  }

  return _waiting;
}

int MiLightRadio::dupesReceived()
{
  return _dupes_received;
}


int MiLightRadio::read(uint8_t frame[], size_t &frame_length)
{
  if (!_waiting) {
    frame_length = 0;
    return -1;
  }

  if (frame_length > sizeof(_packet) - 1) {
    frame_length = sizeof(_packet) - 1;
  }

  if (frame_length > _packet[0]) {
    frame_length = _packet[0];
  }

  memcpy(frame, _packet + 1, frame_length);
  _waiting = false;

  return _packet[0];
}

int MiLightRadio::write(uint8_t frame[], size_t frame_length)
{
  if (frame_length > sizeof(_out_packet) - 1) {
    return -1;
  }

  memcpy(_out_packet + 1, frame, frame_length);
  _out_packet[0] = frame_length;

  int retval = resend();
  if (retval < 0) {
    return retval;
  }
  return frame_length;
}

int MiLightRadio::resend()
{
  for (size_t i = 0; i < MiLightRadioConfig::NUM_CHANNELS; i++) {
    _pl1167.writeFIFO(_out_packet, _out_packet[0] + 1);
    _pl1167.transmit(config.channels[i]);
  }
  return 0;
}
