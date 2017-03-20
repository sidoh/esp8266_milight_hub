#include <PacketFormatter.h>

#ifndef _RGBW_PACKET_FORMATTER_H
#define _RGBW_PACKET_FORMATTER_H 

#define RGBW_COMMAND_INDEX 5
#define RGBW_BRIGHTNESS_GROUP_INDEX 4
#define RGBW_COLOR_INDEX 3

class RgbwPacketFormatter : public PacketFormatter {
public:
  RgbwPacketFormatter(size_t packetLength)
    : PacketFormatter(packetLength)
  { }
  
  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void updateBrightness(uint8_t value);
  virtual void command(uint8_t command, uint8_t arg);
  virtual void updateHue(uint16_t value);
  virtual void updateColorRaw(uint8_t value);
  virtual void updateColorWhite();
  virtual void format(uint8_t const* packet, char* buffer);
  
  virtual void reset();
};

#endif