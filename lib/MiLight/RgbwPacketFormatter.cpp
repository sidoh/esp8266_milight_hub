#include <RgbwPacketFormatter.h>
#include <MiLightButtons.h>

void RgbwPacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;
  
  packet[packetPtr++] = RGBW;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = (groupId & 0x07);
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum++;
}

void RgbwPacketFormatter::unpair() { 
  PacketFormatter::updateStatus(ON);
  updateColorWhite();
}

void RgbwPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  uint8_t button = RGBW_GROUP_1_ON + ((groupId - 1)*2) + status;
  command(button, 0);
}
  
void RgbwPacketFormatter::updateBrightness(uint8_t value) {
  // Expect an input value in [0, 100]. Map it down to [0, 25].
  const uint8_t adjustedBrightness = rescale(value, 25, 100);

  // The actual protocol uses a bizarre range where min is 16, max is 23:
  // [16, 15, ..., 0, 31, ..., 23]
  const uint8_t packetBrightnessValue = (
    ((31 - adjustedBrightness) + 17) % 32
  );
  
  command(RGBW_BRIGHTNESS, 0);
  currentPacket[RGBW_BRIGHTNESS_GROUP_INDEX] |= (packetBrightnessValue << 3);
}

void RgbwPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  currentPacket[RGBW_COMMAND_INDEX] = command;
}
  
void RgbwPacketFormatter::updateHue(uint16_t value) {
  const int16_t remappedColor = (value + 40) % 360;
  updateColorRaw(rescale(remappedColor, 255, 360));
}

void RgbwPacketFormatter::updateColorRaw(uint8_t value) {
  currentPacket[RGBW_COLOR_INDEX] = value;
  command(RGBW_COLOR, 0);
}

void RgbwPacketFormatter::updateColorWhite() {
  uint8_t button = RGBW_GROUP_1_MAX_LEVEL + ((groupId - 1)*2);
  command(button, 0);
}

void RgbwPacketFormatter::format(uint8_t const* packet, char* buffer) {
  PacketFormatter::formatV1Packet(packet, buffer);
}