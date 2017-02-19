#include <Arduino.h>

#ifndef _MILIGHT_RADIO_CONFIG
#define _MILIGHT_RADIO_CONFIG 

class MiLightRadioConfig {
public:
  MiLightRadioConfig(const uint16_t syncword0,
  const uint16_t syncword3,
  const uint8_t* channels,
  const size_t numChannels) 
    : syncword0(syncword0),
      syncword3(syncword3),
      channels(channels),
      numChannels(numChannels)
  {}
    
  const uint16_t syncword0;
  const uint16_t syncword3;
  const uint8_t* channels;
  const size_t numChannels;
};

const uint8_t RGBW_CHANNELS[] = {9, 40, 71};
static MiLightRadioConfig MilightRgbwConfig(
  0x147A, 0x258B, RGBW_CHANNELS, 3
);

const uint8_t CCT_CHANNELS[] = {4, 39, 74};
static MiLightRadioConfig MilightCctConfig(
  0x050A, 0x55AA, CCT_CHANNELS, 3 
);

#endif
