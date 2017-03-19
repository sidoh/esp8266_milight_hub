#include <PacketFormatter.h>

#ifndef _CCT_PACKET_FORMATTER_H
#define _CCT_PACKET_FORMATTER_H 

#define CCT_COMMAND_INDEX 4

class CctPacketFormatter : public PacketFormatter {
public:
  CctPacketFormatter(size_t packetLength)
    : PacketFormatter(packetLength)
  { }
  
  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void command(uint8_t command, uint8_t arg);
  virtual void increaseTemperature();
  virtual void decreaseTemperature();
  virtual void increaseBrightness();
  virtual void decreaseBrightness();
  
  virtual void reset();
  
  static uint8_t getCctStatusButton(uint8_t groupId, MiLightStatus status);
};

#endif