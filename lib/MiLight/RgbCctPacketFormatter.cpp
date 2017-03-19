#include <RgbCctPacketFormatter.h>

#define V2_OFFSET(byte, key) ( V2_OFFSETS[byte-1][key%4] )

uint8_t const RgbCctPacketFormatter::V2_OFFSETS[][4] = {
  { 0x45, 0x1F, 0x14, 0x5C },
  { 0xAB, 0x49, 0x63, 0x91 },
  { 0x2D, 0x1F, 0x4A, 0xEB },
  { 0xAF, 0x03, 0x1D, 0xF3 },
  { 0x5A, 0x22, 0x30, 0x11 },
  { 0x04, 0xD8, 0x71, 0x42 },
  { 0xAF, 0x04, 0xDD, 0x07 },
  { 0xE1, 0x93, 0xB8, 0xE4 }
};

void RgbCctPacketFormatter::reset() {
  size_t packetPtr = 0;
  
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
  
void RgbCctPacketFormatter::command(uint8_t command, uint8_t arg) {
  packet[RGB_CCT_COMMAND_INDEX] = command;
  packet[RGB_CCT_ARGUMENT_INDEX] = arg;
}

void RgbCctPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(RGB_CCT_ON, 0xC0 + groupId + (status == OFF ? 5 : 0));
}

void RgbCctPacketFormatter::updateBrightness(uint8_t brightness) {
  command(RGB_CCT_BRIGHTNESS, 0x4F + brightness);
}
  
void RgbCctPacketFormatter::updateHue(uint16_t value) {
  const int16_t remappedColor = (value + 20) % 360;
  updateColorRaw(rescale(remappedColor, 255, 360));
}

void RgbCctPacketFormatter::updateColorRaw(uint8_t value) {
  command(RGB_CCT_COLOR, 0x15 + value);
}
  
void RgbCctPacketFormatter::updateTemperature(uint8_t value) {
  command(RGB_CCT_KELVIN, (0x4C + value)*2);
}

void RgbCctPacketFormatter::updateSaturation(uint8_t value) {
  command(RGB_CCT_SATURATION, value - 0x33);
}
  
void RgbCctPacketFormatter::updateColorWhite() {
  updateTemperature(0);
}
  
uint8_t* RgbCctPacketFormatter::buildPacket() {
  encodeV2Packet(packet);
  return packet;
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

uint8_t RgbCctPacketFormatter::encodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2) {
  uint8_t value = (byte + s1) % 0x100;
  value = value ^ xorKey;
  value = (value + s2) % 0x100;
  
  return value;
}

void RgbCctPacketFormatter::encodeV2Packet(uint8_t *packet) {
  uint8_t key = xorKey(packet[0]);
  uint8_t sum = key;
  
  printf("Packet: ");
  for (size_t i = 0; i < 9; i++) {
    printf("%02X ", packet[i]);
  }
  printf("\n");
  
  for (size_t i = 1; i <= 7; i++) {
    sum += packet[i];
    packet[i] = encodeByte(packet[i], 0, key, V2_OFFSET(i, packet[0]));
  }
  
  packet[8] = encodeByte(sum, 2, key, V2_OFFSET(8, packet[0]));
}
