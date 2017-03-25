#include <PacketFormatter.h>

#ifndef _CCT_PACKET_FORMATTER_H
#define _CCT_PACKET_FORMATTER_H 

#define CCT_COMMAND_INDEX 4
#define CCT_INTERVALS 10

class CctPacketFormatter : public PacketFormatter {
public:
  CctPacketFormatter()
    : PacketFormatter(7, 20)
  { }
  
  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void command(uint8_t command, uint8_t arg);
  
  virtual void updateTemperature(uint8_t value);
  virtual void increaseTemperature();
  virtual void decreaseTemperature();
  
  virtual void updateBrightness(uint8_t value);
  virtual void increaseBrightness();
  virtual void decreaseBrightness();
  virtual void format(uint8_t const* packet, char* buffer);
  
  virtual void initializePacket(uint8_t* packet);
  
  static uint8_t getCctStatusButton(uint8_t groupId, MiLightStatus status);
};

#endif