#include <Arduino.h>
#include <inttypes.h>

#ifndef _V2_RF_ENCODING_H
#define _V2_RF_ENCODING_H

#define V2_OFFSET_JUMP_START 0x54

class V2RFEncoding {
public:
  static void encodeV2Packet(uint8_t* packet);
  static void decodeV2Packet(uint8_t* packet);
  static uint8_t xorKey(uint8_t key);
  static uint8_t encodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2);
  static uint8_t decodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2);

private:
  static uint8_t const V2_OFFSETS[][4];
};

#endif
