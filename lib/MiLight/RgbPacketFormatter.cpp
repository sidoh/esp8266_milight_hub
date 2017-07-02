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

void RgbPacketFormatter::pair() {
  for (size_t i = 0; i < 5; i++) {
    command(RGB_SPEED_UP, 0);
  }
}

void RgbPacketFormatter::unpair() {
  for (size_t i = 0; i < 5; i++) {
    command(RGB_SPEED_UP | 0x10, 0);
  }
}

void RgbPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(status == ON ? RGB_ON : RGB_OFF, 0);
}

void RgbPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x80;
  }
  currentPacket[RGB_COMMAND_INDEX] = command;
}

void RgbPacketFormatter::updateHue(uint16_t value) {
  const int16_t remappedColor = (value + 40) % 360;
  updateColorRaw(rescale(remappedColor, 255, 360));
}

void RgbPacketFormatter::updateColorRaw(uint8_t value) {
  command(0, 0);
  currentPacket[RGB_COLOR_INDEX] = value;
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

void RgbPacketFormatter::modeSpeedDown() {
  command(RGB_SPEED_DOWN, 0);
}

void RgbPacketFormatter::modeSpeedUp() {
  command(RGB_SPEED_UP, 0);
}

void RgbPacketFormatter::nextMode() {
  command(RGB_MODE_UP, 0);
}

void RgbPacketFormatter::previousMode() {
  command(RGB_MODE_DOWN, 0);
}

void RgbPacketFormatter::parsePacket(const uint8_t* packet, JsonObject& result) {
  uint8_t command = packet[RGB_COMMAND_INDEX] & 0x7F;

  result["group_id"] = 0;
  result["device_id"] = (packet[1] << 8) | packet[2];
  result["device_type"] = "rgb";

  if (command == RGB_ON) {
    result["state"] = "ON";
  } else if (command == RGB_OFF) {
    result["state"] = "OFF";
  } else if (command == 0) {
    uint16_t remappedColor = rescale<uint16_t, uint16_t>(packet[RGB_COLOR_INDEX], 360.0, 255.0);
    remappedColor = (remappedColor + 320) % 360;
    result["hue"] = remappedColor;
  }
}

void RgbPacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, "b0       : %02X\n", packet[0]);
  buffer += sprintf_P(buffer, "ID       : %02X%02X\n", packet[1], packet[2]);
  buffer += sprintf_P(buffer, "Color    : %02X\n", packet[3]);
  buffer += sprintf_P(buffer, "Command  : %02X\n", packet[4]);
  buffer += sprintf_P(buffer, "Sequence : %02X\n", packet[5]);
}
