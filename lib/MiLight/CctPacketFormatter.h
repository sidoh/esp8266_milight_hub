#include <PacketFormatter.h>

#ifndef _CCT_PACKET_FORMATTER_H
#define _CCT_PACKET_FORMATTER_H 

#define CCT_COMMAND_INDEX 5

class CctPacketFormatter : public PacketFormatter {
public:
  CctPacketFormatter(size_t packetLength)
    : PacketFormatter(packetLength)
  { }
  
  virtual void status(MiLightStatus status, uint8_t groupId);
  
  virtual void reset();
  
  static uint8_t getCctStatusButton(uint8_t groupId, MiLightStatus status);
};

#endif