#include <MiLightClient.h>

MiLightRadio* MiLightClient::getRadio(const MiLightRadioType type) {
  MiLightRadio* radio = NULL;
  
  if (type == RGBW) {
    return rgbwRadio->getRadio();
  } else if (type == CCT) {
    return cctRadio->getRadio();
  } 
  
  if (radio != NULL) {
    radio->configure();
  }
  
  return radio;
}

void MiLightClient::deserializePacket(const uint8_t rawPacket[], MiLightPacket& packet) {
  uint8_t ptr = 0;
  
  packet.deviceType = rawPacket[ptr++];
  packet.deviceId = (rawPacket[ptr++] << 8) | rawPacket[ptr++];
  packet.b1 = rawPacket[ptr++];
  packet.b2 = rawPacket[ptr++];
  packet.b3 = rawPacket[ptr++];
  packet.sequenceNum = rawPacket[ptr++];
}

void MiLightClient::serializePacket(uint8_t rawPacket[], const MiLightPacket& packet) {
  uint8_t ptr = 0;
  
  rawPacket[ptr++] = packet.deviceType;
  
  // big endian
  rawPacket[ptr++] = packet.deviceId >> 8;
  rawPacket[ptr++] = packet.deviceId & 0xFF;
  
  rawPacket[ptr++] = packet.b1;
  rawPacket[ptr++] = packet.b2;
  rawPacket[ptr++] = packet.b3;
  rawPacket[ptr++] = packet.sequenceNum;
}

uint8_t MiLightClient::nextSequenceNum() {
  return sequenceNum++;
}

bool MiLightClient::available(const MiLightRadioType radioType) {
  MiLightRadio* radio = getRadio(radioType);
  radio->begin();
  
  if (radio == NULL) {
    return false;
  }
  
  return radio->available();
}

void MiLightClient::read(const MiLightRadioType radioType, MiLightPacket& packet) {
  MiLightRadio* radio = getRadio(radioType);
  
  if (radio == NULL) {
    return;
  }
  
  uint8_t packetBytes[MILIGHT_PACKET_LENGTH];
  size_t length;
  radio->read(packetBytes, length);
  deserializePacket(packetBytes, packet);
}

void MiLightClient::write(const MiLightRadioType radioType, 
  MiLightPacket& packet, 
  const unsigned int resendCount) {
    
  uint8_t packetBytes[MILIGHT_PACKET_LENGTH];
  serializePacket(packetBytes, packet);
  MiLightRadio* radio = getRadio(radioType);
  
  if (radio == NULL) {
    return;
  }
  
  for (int i = 0; i < resendCount; i++) {
    radio->write(packetBytes, MILIGHT_PACKET_LENGTH);
  }
}


void MiLightClient::writeRgbw(
  const uint16_t deviceId,
  const uint8_t color,
  const uint8_t brightness,
  const uint8_t groupId,
  const uint8_t button) {
  
  MiLightPacket packet;
  packet.deviceType = RGBW;;
  packet.deviceId = deviceId;
  packet.b1 = color;
  packet.b2 = (brightness << 3) | (groupId & 0x07);
  packet.b3 = button;
  packet.sequenceNum = nextSequenceNum();
  
  write(RGBW, packet);
}
    
void MiLightClient::updateColorRaw(const uint16_t deviceId, const uint8_t groupId, const uint16_t color) {
  writeRgbw(deviceId, color, 0, groupId, RGBW_COLOR);
}

void MiLightClient::updateHue(const uint16_t deviceId, const uint8_t groupId, const uint16_t hue) {
  // Map color as a Hue value in [0, 359] to [0, 255]. The protocol also has
  // 0 being roughly magenta (#FF00FF)
  const int16_t remappedColor = (hue + 40) % 360;
  const uint8_t adjustedColor = round(remappedColor * (255 / 360.0));
  
  writeRgbw(deviceId, adjustedColor, 0, groupId, RGBW_COLOR);
}

void MiLightClient::updateBrightness(const uint16_t deviceId, const uint8_t groupId, const uint8_t brightness) {
  // Expect an input value in [0, 100]. Map it down to [0, 25].
  const uint8_t adjustedBrightness = round(brightness * (25 / 100.0));
  
  // The actual protocol uses a bizarre range where min is 16, max is 23:
  // [16, 15, ..., 0, 31, ..., 23]
  const uint8_t packetBrightnessValue = (
    ((31 - adjustedBrightness) + 17) % 32
  );
  
  writeRgbw(deviceId, 0, packetBrightnessValue, groupId, RGBW_BRIGHTNESS);
}

void MiLightClient::updateStatus(const uint16_t deviceId, const uint8_t groupId, MiLightStatus status) {
  uint8_t button = RGBW_GROUP_1_ON + ((groupId - 1)*2) + status;
  writeRgbw(deviceId, 0, 0, groupId, button);
}

void MiLightClient::updateColorWhite(const uint16_t deviceId, const uint8_t groupId) {
  uint8_t button = RGBW_GROUP_1_MAX_LEVEL + ((groupId - 1)*2);
  pressButton(deviceId, groupId, button);
}

void MiLightClient::pair(const uint16_t deviceId, const uint8_t groupId) {
  updateStatus(deviceId, groupId, ON);
}

void MiLightClient::unpair(const uint16_t deviceId, const uint8_t groupId) {
  updateStatus(deviceId, groupId, ON);
  delay(1);
  updateColorWhite(deviceId, groupId);
}
    
void MiLightClient::pressButton(const uint16_t deviceId, const uint8_t groupId, const uint8_t button) {
  writeRgbw(deviceId, 0, 0, groupId, button);
}

void MiLightClient::allOn(const uint16_t deviceId) {
  writeRgbw(deviceId, 0, 0, 0, RGBW_ALL_ON);
}

void MiLightClient::allOff(const uint16_t deviceId) {
  writeRgbw(deviceId, 0, 0, 0, RGBW_ALL_OFF);
}