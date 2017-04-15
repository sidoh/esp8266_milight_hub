#include <RgbCctPacketFormatter.h>

#define V2_OFFSET(byte, key, jumpStart) ( \
  pgm_read_byte(&V2_OFFSETS[byte-1][key%4]) \
    + \
  ((jumpStart > 0 && key >= jumpStart && key <= jumpStart+0x80) ? 0x80 : 0) \
)

#define GROUP_COMMAND_ARG(status, groupId) ( groupId + (status == OFF ? 5 : 0) )

uint8_t const RgbCctPacketFormatter::V2_OFFSETS[][4] = {
  { 0x45, 0x1F, 0x14, 0x5C }, // request type
  { 0x2B, 0xC9, 0xE3, 0x11 }, // id 1
  { 0x6D, 0x5F, 0x8A, 0x2B }, // id 2
  { 0xAF, 0x03, 0x1D, 0xF3 }, // command
  { 0x1A, 0xE2, 0xF0, 0xD1 }, // argument
  { 0x04, 0xD8, 0x71, 0x42 }, // sequence
  { 0xAF, 0x04, 0xDD, 0x07 }, // group
  { 0x61, 0x13, 0x38, 0x64 }  // checksum
};

void RgbCctPacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  // Always encode with 0x00 key. No utility in varying it.
  packet[packetPtr++] = 0x00;

  packet[packetPtr++] = 0x20;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum++;
  packet[packetPtr++] = groupId;
  packet[packetPtr++] = 0;
}

void RgbCctPacketFormatter::unpair() {
  for (size_t i = 0; i < 5; i++) {
    updateStatus(ON, 0);
  }
}

void RgbCctPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x80;
  }
  currentPacket[RGB_CCT_COMMAND_INDEX] = command;
  currentPacket[RGB_CCT_ARGUMENT_INDEX] = arg;
}

void RgbCctPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(RGB_CCT_ON, GROUP_COMMAND_ARG(status, groupId));
}

void RgbCctPacketFormatter::modeSpeedDown() {
  command(RGB_CCT_ON, RGB_CCT_MODE_SPEED_DOWN);
}

void RgbCctPacketFormatter::modeSpeedUp() {
  command(RGB_CCT_ON, RGB_CCT_MODE_SPEED_UP);
}

void RgbCctPacketFormatter::updateMode(uint8_t mode) {
  lastMode = mode;
  command(RGB_CCT_MODE, mode);
}

void RgbCctPacketFormatter::nextMode() {
  updateMode((lastMode+1)%RGB_CCT_NUM_MODES);
}

void RgbCctPacketFormatter::previousMode() {
  updateMode((lastMode-1)%RGB_CCT_NUM_MODES);
}

void RgbCctPacketFormatter::updateBrightness(uint8_t brightness) {
  command(RGB_CCT_BRIGHTNESS, 0x8F + brightness);
}

void RgbCctPacketFormatter::updateHue(uint16_t value) {
  uint8_t remapped = rescale(value, 255, 360);
  updateColorRaw(remapped);
}

void RgbCctPacketFormatter::updateColorRaw(uint8_t value) {
  command(RGB_CCT_COLOR, 0x5F + value);
}

void RgbCctPacketFormatter::updateTemperature(uint8_t value) {
  command(RGB_CCT_KELVIN, 0x94 - (value*2));
}

void RgbCctPacketFormatter::updateSaturation(uint8_t value) {
  uint8_t remapped = value + 0xD;
  command(RGB_CCT_SATURATION, remapped);
}

void RgbCctPacketFormatter::updateColorWhite() {
  updateTemperature(0);
}

void RgbCctPacketFormatter::enableNightMode() {
  uint8_t arg = GROUP_COMMAND_ARG(OFF, groupId);
  command(RGB_CCT_ON | 0x80, arg);
}

void RgbCctPacketFormatter::finalizePacket(uint8_t* packet) {
  encodeV2Packet(packet);
}

uint8_t RgbCctPacketFormatter::xorKey(uint8_t key) {
  // Generate most significant nibble
  const uint8_t shift = (key & 0x0F) < 0x04 ? 0 : 1;
  const uint8_t x = (((key & 0xF0) >> 4) + shift + 6) % 8;
  const uint8_t msn = (((4 + x) ^ 1) & 0x0F) << 4;

  // Generate least significant nibble
  const uint8_t lsn = ((((key & 0xF) + 4)^2) & 0x0F);

  return ( msn | lsn );
}

uint8_t RgbCctPacketFormatter::decodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2) {
  uint8_t value = byte - s2;
  value = value ^ xorKey;
  value = value - s1;

  return value;
}

uint8_t RgbCctPacketFormatter::encodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2) {
  uint8_t value = byte + s1;
  value = value ^ xorKey;
  value = value + s2;

  return value;
}

void RgbCctPacketFormatter::decodeV2Packet(uint8_t *packet) {
  uint8_t key = xorKey(packet[0]);

  for (size_t i = 1; i <= 8; i++) {
    packet[i] = decodeByte(packet[i], 0, key, V2_OFFSET(i, packet[0], V2_OFFSET_JUMP_START));
  }
}

void RgbCctPacketFormatter::encodeV2Packet(uint8_t *packet) {
  uint8_t key = xorKey(packet[0]);
  uint8_t sum = key;

  for (size_t i = 1; i <= 7; i++) {
    sum += packet[i];
    packet[i] = encodeByte(packet[i], 0, key, V2_OFFSET(i, packet[0], V2_OFFSET_JUMP_START));
  }

  packet[8] = encodeByte(sum, 2, key, V2_OFFSET(8, packet[0], 0));
}

void RgbCctPacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, PSTR("Raw packet: "));
  for (int i = 0; i < packetLength; i++) {
    buffer += sprintf_P(buffer, PSTR("%02X "), packet[i]);
  }

  uint8_t decodedPacket[packetLength];
  memcpy(decodedPacket, packet, packetLength);

  decodeV2Packet(decodedPacket);

  buffer += sprintf_P(buffer, PSTR("\n\nDecoded:\n"));
  buffer += sprintf_P(buffer, PSTR("Key      : %02X\n"), decodedPacket[0]);
  buffer += sprintf_P(buffer, PSTR("b1       : %02X\n"), decodedPacket[1]);
  buffer += sprintf_P(buffer, PSTR("ID       : %02X%02X\n"), decodedPacket[2], decodedPacket[3]);
  buffer += sprintf_P(buffer, PSTR("Command  : %02X\n"), decodedPacket[4]);
  buffer += sprintf_P(buffer, PSTR("Argument : %02X\n"), decodedPacket[5]);
  buffer += sprintf_P(buffer, PSTR("Sequence : %02X\n"), decodedPacket[6]);
  buffer += sprintf_P(buffer, PSTR("Group    : %02X\n"), decodedPacket[7]);
  buffer += sprintf_P(buffer, PSTR("Checksum : %02X"), decodedPacket[8]);
}
