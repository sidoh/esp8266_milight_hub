#include <Arduino.h>
#include <PacketFormatter.h>
#include <RgbCctPacketFormatter.h>
#include <RgbwPacketFormatter.h>
#include <CctPacketFormatter.h>
#include <MiLightButtons.h>

#ifndef _MILIGHT_RADIO_CONFIG
#define _MILIGHT_RADIO_CONFIG 

class MiLightRadioConfig {
public:
  static const size_t NUM_CHANNELS = 3;
  
  MiLightRadioConfig(const uint16_t syncword0,
  const uint16_t syncword3,
  PacketFormatter* packetFormatter,
  const MiLightRadioType type,
  const uint8_t channel0,
  const uint8_t channel1,
  const uint8_t channel2) 
    : syncword0(syncword0),
      syncword3(syncword3),
      packetFormatter(packetFormatter),
      type(type)
  {
    channels[0] = channel0;
    channels[1] = channel1;
    channels[2] = channel2;
  }
    
  const uint16_t syncword0;
  const uint16_t syncword3;
  uint8_t channels[3];
  PacketFormatter* packetFormatter;
  const MiLightRadioType type;
  
  static MiLightRadioConfig* fromString(const String& s);
  size_t getPacketLength() const;
};

static MiLightRadioConfig MilightRgbwConfig(
  0x147A, 0x258B, new RgbwPacketFormatter(8), RGBW, 9, 40, 71
);

static MiLightRadioConfig MilightCctConfig(
  0x050A, 0x55AA, new CctPacketFormatter(8), CCT, 4, 39, 74
);

static MiLightRadioConfig MilightRgbCctConfig(
  0x7236, 0x1809, new RgbCctPacketFormatter(9), RGB_CCT, 8, 39, 70
);

#endif
