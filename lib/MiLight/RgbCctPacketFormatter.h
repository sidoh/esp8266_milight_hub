#include <PacketFormatter.h>
#include <MiLightRadioConfig.h>

#define RGB_CCT_COMMAND_INDEX 4
#define RGB_CCT_ARGUMENT_INDEX 5

#ifndef _RGB_CCT_PACKET_FORMATTER_H
#define _RGB_CCT_PACKET_FORMATTER_H 

class RgbCctPacketFormatter : public PacketFormatter {
public:
  RgbCctPacketFormatter()
    : PacketFormatter(MilightRgbCctConfig.packetLength)
  { }
  
  virtual void reset();
  
  virtual void updateStatus(MiLightStatus status);
  virtual void updateBrightness(uint8_t value);
};

#endif