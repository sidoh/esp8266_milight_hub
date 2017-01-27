#include <MiLightClient.h>

uint8_t MiLightClient::nextSequenceNum() {
  return sequenceNum++;
}

bool MiLightClient::available() {
  return radio.available();
}

void MiLightClient::read(MiLightPacket& packet) {
  uint8_t *packetBytes = reinterpret_cast<uint8_t*>(&packet);
  size_t length = sizeof(packet);
  radio.read(packetBytes, length);
}

void MiLightClient::write(MiLightPacket& packet, const unsigned int resendCount) {
  uint8_t *packetBytes = reinterpret_cast<uint8_t*>(&packet);
  
  for (int i = 0; i < resendCount; i++) {
    radio.write(packetBytes, sizeof(packet));
  }
  Serial.println();
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
  packet.brightnessGroupId = (packetBrightnessValue << 3) | groupId;
  packet.button = button;
  packet.sequenceNum = nextSequenceNum();
  
  write(packet);
}
    
void MiLightClient::updateColor(const uint16_t deviceId, const uint8_t groupId, const uint16_t hue) {
  write(deviceId, hue, 0, groupId, MiLightButton::COLOR);
}

void MiLightClient::updateBrightness(const uint16_t deviceId, const uint8_t groupId, const uint8_t brightness) {
  write(deviceId, 0, brightness, groupId, MiLightButton::BRIGHTNESS);
}

void MiLightClient::updateStatus(const uint16_t deviceId, const uint8_t groupId, MiLightStatus status) {
  uint8_t button = MiLightButton::GROUP_1_ON + ((groupId - 1)*2) + status;
  write(deviceId, 0, 0, 0, static_cast<MiLightButton>(button));
}

void MiLightClient::updateColorWhite(const uint16_t deviceId, const uint8_t groupId) {
  write(deviceId, 0, 0, groupId, MiLightButton::COLOR_WHITE);
}