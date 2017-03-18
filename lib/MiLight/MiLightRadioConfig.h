#include <Arduino.h>

#ifndef _MILIGHT_RADIO_CONFIG
#define _MILIGHT_RADIO_CONFIG 

enum MiLightRadioType {
  UNKNOWN = 0,
  RGBW  = 0xB8,
  CCT   = 0x5A,
  RGB_CCT = 0x20
};

class MiLightRadioConfig {
public:
  MiLightRadioConfig(const uint16_t syncword0,
  const uint16_t syncword3,
  const size_t packetLength,
  const uint8_t* channels,
  const size_t numChannels,
  const MiLightRadioType type) 
    : syncword0(syncword0),
      syncword3(syncword3),
      packetLength(packetLength),
      channels(channels),
      numChannels(numChannels),
      type(type)
  {}
    
  const uint16_t syncword0;
  const uint16_t syncword3;
  const size_t packetLength;
  const uint8_t* channels;
  const size_t numChannels;
  const MiLightRadioType type;
};

const uint8_t RGBW_CHANNELS[] = {9, 40, 71};
static MiLightRadioConfig MilightRgbwConfig(
  0x147A, 0x258B, 7, RGBW_CHANNELS, 3, RGBW
);

const uint8_t CCT_CHANNELS[] = {4, 39, 74};
static MiLightRadioConfig MilightCctConfig(
  0x050A, 0x55AA, 7, CCT_CHANNELS, 3, CCT
);

const uint8_t RGBCCT_CHANNELS[] = {70, 39, 8};
static MiLightRadioConfig MilightRgbCctConfig(
  0x7236, 0x1809, 9, RGBCCT_CHANNELS, 3, RGB_CCT
);

#endif
