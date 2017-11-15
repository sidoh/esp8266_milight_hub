#include <Arduino.h>
#include <MiLightConstants.h>
#include <Size.h>

#ifndef _MILIGHT_RADIO_CONFIG
#define _MILIGHT_RADIO_CONFIG

#define MILIGHT_MAX_PACKET_LENGTH 9

class MiLightRadioConfig {
public:
  static const size_t NUM_CHANNELS = 3;

  MiLightRadioConfig(
    const uint16_t syncword0,
    const uint16_t syncword3,
    const size_t packetLength,
    const uint8_t channel0,
    const uint8_t channel1,
    const uint8_t channel2
  )
    : syncword0(syncword0),
      syncword3(syncword3),
      packetLength(packetLength)
  {
    channels[0] = channel0;
    channels[1] = channel1;
    channels[2] = channel2;
  }

  const uint16_t syncword0;
  const uint16_t syncword3;
  uint8_t channels[3];
  const size_t packetLength;

  static const size_t NUM_CONFIGS = 4;
  static MiLightRadioConfig ALL_CONFIGS[NUM_CONFIGS];
};

#endif
