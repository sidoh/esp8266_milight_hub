#include <RgbPacketFormatter.h>

void RgbPacketFormatter::initializePacket(uint8_t *packet) {
  size_t packetPtr = 0;
  
  packet[packetPtr++] = RGB;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum++;
}

void RgbPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(status == ON ? RGB_ON : RGB_OFF, 0);
}

void RgbPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  currentPacket[RGB_COMMAND_INDEX] = command;
}
  
void RgbPacketFormatter::updateHue(uint16_t value) {
  const int16_t remappedColor = (value + 40) % 360;
  updateColorRaw(rescale(remappedColor, 255, 360));
}

void RgbPacketFormatter::updateColorRaw(uint8_t value) {
  currentPacket[RGB_COLOR_INDEX] = value;
  command(0, 0);
}

void RgbPacketFormatter::updateBrightness(uint8_t value) {
  valueByStepFunction(
    &PacketFormatter::increaseBrightness,
    &PacketFormatter::decreaseBrightness,
    RGB_INTERVALS,
    value / RGB_INTERVALS
  );
}

void RgbPacketFormatter::increaseBrightness() {
  command(RGB_BRIGHTNESS_UP, 0);
}

void RgbPacketFormatter::decreaseBrightness() {
  command(RGB_BRIGHTNESS_DOWN, 0);
}

void RgbPacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf(buffer, "b0       : %02X\n", packet[0]);
  buffer += sprintf(buffer, "ID       : %02X%02X\n", packet[1], packet[2]);
  buffer += sprintf(buffer, "Color    : %02X\n", packet[3]);
  buffer += sprintf(buffer, "Command  : %02X\n", packet[4]);
  buffer += sprintf(buffer, "Sequence : %02X\n", packet[5]);
}