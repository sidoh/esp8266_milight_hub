#include <V2RFEncoding.h>

#define V2_OFFSET(byte, key, jumpStart) ( \
  V2_OFFSETS[byte-1][key%4] \
    + \
  ((jumpStart > 0 && key >= jumpStart && key < jumpStart+0x80) ? 0x80 : 0) \
)

uint8_t const V2RFEncoding::V2_OFFSETS[][4] = {
  { 0x45, 0x1F, 0x14, 0x5C }, // request type
  { 0x2B, 0xC9, 0xE3, 0x11 }, // id 1
  { 0x6D, 0x5F, 0x8A, 0x2B }, // id 2
  { 0xAF, 0x03, 0x1D, 0xF3 }, // command
  { 0x1A, 0xE2, 0xF0, 0xD1 }, // argument
  { 0x04, 0xD8, 0x71, 0x42 }, // sequence
  { 0xAF, 0x04, 0xDD, 0x07 }, // group
  { 0x61, 0x13, 0x38, 0x64 }  // checksum
};

uint8_t V2RFEncoding::xorKey(uint8_t key) {
  // Generate most significant nibble
  const uint8_t shift = (key & 0x0F) < 0x04 ? 0 : 1;
  const uint8_t x = (((key & 0xF0) >> 4) + shift + 6) % 8;
  const uint8_t msn = (((4 + x) ^ 1) & 0x0F) << 4;

  // Generate least significant nibble
  const uint8_t lsn = ((((key & 0xF) + 4)^2) & 0x0F);

  return ( msn | lsn );
}

uint8_t V2RFEncoding::decodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2) {
  uint8_t value = byte - s2;
  value = value ^ xorKey;
  value = value - s1;

  return value;
}

uint8_t V2RFEncoding::encodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2) {
  uint8_t value = byte + s1;
  value = value ^ xorKey;
  value = value + s2;

  return value;
}

void V2RFEncoding::decodeV2Packet(uint8_t *packet) {
  uint8_t key = xorKey(packet[0]);

  for (size_t i = 1; i <= 8; i++) {
    packet[i] = decodeByte(packet[i], 0, key, V2_OFFSET(i, packet[0], V2_OFFSET_JUMP_START));
  }
}

void V2RFEncoding::encodeV2Packet(uint8_t *packet) {
  uint8_t key = xorKey(packet[0]);
  uint8_t sum = key;

  for (size_t i = 1; i <= 7; i++) {
    sum += packet[i];
    packet[i] = encodeByte(packet[i], 0, key, V2_OFFSET(i, packet[0], V2_OFFSET_JUMP_START));
  }

  packet[8] = encodeByte(sum, 2, key, V2_OFFSET(8, packet[0], 0));
}
