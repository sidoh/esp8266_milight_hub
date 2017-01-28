#include <MiLightClient.h>

void MiLightClient::deserializePacket(const uint8_t rawPacket[], MiLightPacket& packet) {
  uint8_t ptr = 0;
  
  packet.deviceType = rawPacket[ptr++];
  packet.deviceId = (rawPacket[ptr++] << 8) | rawPacket[ptr++];
  packet.color = rawPacket[ptr++];
  
  packet.brightness = rawPacket[ptr] >> 3;
  packet.groupId = rawPacket[ptr++] & 0x07;
  
  packet.button = rawPacket[ptr++];
  packet.sequenceNum = rawPacket[ptr++];
}

void MiLightClient::serializePacket(uint8_t rawPacket[], const MiLightPacket& packet) {
  uint8_t ptr = 0;
  
  rawPacket[ptr++] = packet.deviceType;
  
  // big endian
  rawPacket[ptr++] = packet.deviceId >> 8;
  rawPacket[ptr++] = packet.deviceId & 0xFF;
  
  rawPacket[ptr++] = packet.color;
  rawPacket[ptr++] = (packet.brightness << 3) | (packet.groupId & 0x07);
  rawPacket[ptr++] = packet.button;
  rawPacket[ptr++] = packet.sequenceNum;
}

uint8_t MiLightClient::nextSequenceNum() {
  return sequenceNum++;
}

bool MiLightClient::available() {
  return radio.available();
}

void MiLightClient::read(MiLightPacket& packet) {
  uint8_t packetBytes[MILIGHT_PACKET_LENGTH];
  size_t length;
  radio.read(packetBytes, length);
  deserializePacket(packetBytes, packet);
}

void MiLightClient::write(MiLightPacket& packet, const unsigned int resendCount) {
  uint8_t packetBytes[MILIGHT_PACKET_LENGTH];
  serializePacket(packetBytes, packet);
  
  for (int i = 0; i < resendCount; i++) {
    radio.write(packetBytes, MILIGHT_PACKET_LENGTH);
  }
}


void MiLightClient::write(
  const uint16_t deviceId,
  const uint16_t color,
  const uint8_t brightness,
  const uint8_t groupId,
  const MiLightButton button) {
    
  // Expect an input value in [0, 255]. Map it down to [0, 25].
  const uint8_t adjustedBrightness = round(brightness * (25 / 255.0));
  
  // The actual protocol uses a bizarre range where min is 16, max is 23:
  // [16, 15, ..., 0, 31, ..., 23]
  const uint8_t packetBrightnessValue = (
    ((31 - adjustedBrightness) + 17) % 32
  );
  
  // Map color as a Hue value in [0, 359] to [0, 255]. The protocol also has
  // 0 being roughly magenta (#FF00FF)
  const int16_t remappedColor = (color + 40) % 360;
  const uint8_t adjustedColor = round(remappedColor * (255 / 359.0));
  
  MiLightPacket packet;
  packet.deviceType = MiLightDeviceType::RGBW;
  packet.deviceId = deviceId;
  packet.color = adjustedColor;
  packet.brightness = packetBrightnessValue;
  packet.groupId = groupId;
  packet.button = button;
  packet.sequenceNum = nextSequenceNum();
  
  write(packet);
}
    
void MiLightClient::updateColor(const uint16_t deviceId, const uint8_t groupId, const uint16_t hue) {
  write(deviceId, hue, 0, groupId, COLOR);
}

void MiLightClient::updateBrightness(const uint16_t deviceId, const uint8_t groupId, const uint8_t brightness) {
  write(deviceId, 0, brightness, groupId, BRIGHTNESS);
}

void MiLightClient::updateStatus(const uint16_t deviceId, const uint8_t groupId, MiLightStatus status) {
  uint8_t button = MiLightButton::GROUP_1_ON + ((groupId - 1)*2) + status;
  write(deviceId, 0, 0, groupId, static_cast<MiLightButton>(button));
}

void MiLightClient::updateColorWhite(const uint16_t deviceId, const uint8_t groupId) {
  uint8_t button = MiLightButton::GROUP_1_MAX_LEVEL + ((groupId - 1)*2);
  pressButton(deviceId, groupId, static_cast<MiLightButton>(button));
}

void MiLightClient::pair(const uint16_t deviceId, const uint8_t groupId) {
  updateStatus(deviceId, groupId, ON);
}

void MiLightClient::unpair(const uint16_t deviceId, const uint8_t groupId) {
  updateStatus(deviceId, groupId, ON);
  delay(1);
  updateColorWhite(deviceId, groupId);
}
    
void MiLightClient::pressButton(const uint16_t deviceId, const uint8_t groupId, MiLightButton button) {
  write(deviceId, 0, 0, groupId, button);
}

void MiLightClient::allOn(const uint16_t deviceId) {
  write(deviceId, 0, 0, 0, ALL_ON);
}

void MiLightClient::allOff(const uint16_t deviceId) {
  write(deviceId, 0, 0, 0, ALL_OFF);
}