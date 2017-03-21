#include <PacketFormatter.h>

#define RGB_CCT_COMMAND_INDEX 4
#define RGB_CCT_ARGUMENT_INDEX 5
#define V2_OFFSET_JUMP_START 0x54

#ifndef _RGB_CCT_PACKET_FORMATTER_H
#define _RGB_CCT_PACKET_FORMATTER_H 

class RgbCctPacketFormatter : public PacketFormatter {
public:
  static uint8_t const V2_OFFSETS[][4];
    
  RgbCctPacketFormatter(size_t packetLength)
    : PacketFormatter(packetLength)
  { }
  
  virtual void reset();
  
  virtual void updateStatus(MiLightStatus status, uint8_t group);
  virtual void updateBrightness(uint8_t value);
  virtual void command(uint8_t command, uint8_t arg);
  virtual void updateHue(uint16_t value);
  virtual void updateColorRaw(uint8_t value);
  virtual void updateColorWhite();
  virtual void updateTemperature(uint8_t value);
  virtual void updateSaturation(uint8_t value);
  virtual void format(uint8_t const* packet, char* buffer);
  
  virtual uint8_t* buildPacket();
    
  static void encodeV2Packet(uint8_t* packet);
  static void decodeV2Packet(uint8_t* packet);
  static uint8_t xorKey(uint8_t key);
  static uint8_t encodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2);
  static uint8_t decodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2);
};

#endif