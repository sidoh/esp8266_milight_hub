#include <MiLightClient.h>
#include <MiLightRadioConfig.h>
#include <Arduino.h>

#define V2_OFFSET(byte, key) ( V2_OFFSETS[byte-1][key%4] )

uint8_t const MiLightClient::V2_OFFSETS[][4] = {
  { 0x45, 0x1F, 0x14, 0x5C },
  { 0xAB, 0x49, 0x63, 0x91 },
  { 0x2D, 0x1F, 0x4A, 0xEB },
  { 0xAF, 0x03, 0x1D, 0xF3 },
  { 0x5A, 0x22, 0x30, 0x11 },
  { 0x04, 0xD8, 0x71, 0x42 },
  { 0xAF, 0x04, 0xDD, 0x07 },
  { 0xE1, 0x93, 0xB8, 0xE4 }
};

MiLightRadio* MiLightClient::getRadio(const MiLightRadioType type) {
  MiLightRadioStack* stack = NULL;
  
  if (type == RGBW) {
    stack = rgbwRadio;
  } else if (type == CCT) {
    stack = cctRadio;
  } else if (type == RGB_CCT) {
    stack = rgbCctRadio;
  }
  
  if (stack != NULL) {
    MiLightRadio *radio = stack->getRadio();
    
    if (currentRadio != stack->type) {
      radio->configure();
    }
    
    currentRadio = stack->type;
    return radio;
  }
  
  return NULL;
}

void MiLightClient::setResendCount(const unsigned int resendCount) {
  this->resendCount = resendCount;
}

uint8_t MiLightClient::nextSequenceNum() {
  return sequenceNum++;
}

bool MiLightClient::available(const MiLightRadioType radioType) {
  MiLightRadio* radio = getRadio(radioType);
  
  if (radio == NULL) {
    return false;
  }
  
  return radio->available();
}

void MiLightClient::read(const MiLightRadioType radioType, uint8_t packet[]) {
  MiLightRadio* radio = getRadio(radioType);
  
  if (radio == NULL) {
    return;
  }
  
  size_t length;
  radio->read(packet, length);
}

void MiLightClient::write(const MiLightRadioConfig& radioConfig, 
  uint8_t packet[]) {
    
  MiLightRadio* radio = getRadio(radioConfig.type);
  
  if (radio == NULL) {
    return;
  }
  
  for (int i = 0; i < this->resendCount; i++) {
    radio->write(packet, radioConfig.packetLength);
  }
}

void MiLightClient::writeRgbw(
  const uint16_t deviceId,
  const uint8_t color,
  const uint8_t brightness,
  const uint8_t groupId,
  const uint8_t button) {
  
  uint8_t packet[MilightRgbwConfig.packetLength];
  size_t packetPtr = 0;
  
  packet[packetPtr++] = RGBW;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = color;
  packet[packetPtr++] = (brightness << 3) | (groupId & 0x07);
  packet[packetPtr++] = button;
  packet[packetPtr++] = nextSequenceNum();
  
  write(MilightRgbwConfig, packet);
}

void MiLightClient::writeCct(const uint16_t deviceId,
  const uint8_t groupId,
  const uint8_t button) {
    
  uint8_t packet[MilightRgbwConfig.packetLength];
  uint8_t sequenceNum = nextSequenceNum();
  size_t packetPtr = 0;
  
  packet[packetPtr++] = CCT;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = groupId;
  packet[packetPtr++] = button;
  packet[packetPtr++] = sequenceNum;
  packet[packetPtr++] = sequenceNum;
  
  write(MilightCctConfig, packet);
}

void MiLightClient::writeRgbCct(const uint16_t deviceId,
  const uint8_t command,
  const uint8_t arg,
  const uint8_t group) {
    
  uint8_t packet[MilightRgbCctConfig.packetLength];
  uint8_t sequenceNum = nextSequenceNum();
  size_t packetPtr = 0;
  
  packet[packetPtr++] = 0x00;
  packet[packetPtr++] = RGB_CCT;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = command;
  packet[packetPtr++] = arg;
  packet[packetPtr++] = sequenceNum;
  packet[packetPtr++] = group;
  packet[packetPtr++] = 0;
  
  printf("Constructed raw packet: ");
  for (int i = 0; i < MilightRgbCctConfig.packetLength; i++) {
    printf("%02X ", packet[i]);
  }
  printf("\n");
  
  encodeV2Packet(packet);
  
  write(MilightRgbCctConfig, packet);
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

void MiLightClient::updateStatus(const MiLightRadioType type, const uint16_t deviceId, const uint8_t groupId, MiLightStatus status) {
  if (type == RGBW) {
    uint8_t button = RGBW_GROUP_1_ON + ((groupId - 1)*2) + status;
    writeRgbw(deviceId, 0, 0, groupId, button);
  } else if (type == RGB_CCT) {
    writeRgbCct(
      deviceId,
      RGB_CCT_ON,
      0xC0 + groupId + (status == OFF ? 0x05 : 0x00),
      groupId
    );
  } else {
    writeCct(deviceId, groupId, getCctStatusButton(groupId, status));
  }
}

void MiLightClient::updateColorWhite(const uint16_t deviceId, const uint8_t groupId) {
  uint8_t button = RGBW_GROUP_1_MAX_LEVEL + ((groupId - 1)*2);
  pressButton(RGBW, deviceId, groupId, button);
}

void MiLightClient::pair(const MiLightRadioType type, const uint16_t deviceId, const uint8_t groupId) {
  if (type == RGBW || type == CCT) {
    updateStatus(type, deviceId, groupId, ON);
  } else if (type == RGB_CCT) {
    updateStatus(type, deviceId, groupId, ON);
    delay(1);
    updateStatus(type, deviceId, groupId, ON);
  }
}

void MiLightClient::unpair(const MiLightRadioType type, const uint16_t deviceId, const uint8_t groupId) {
  if (type == RGBW) {
    updateStatus(type, deviceId, groupId, ON);
    delay(1);
    updateColorWhite(deviceId, groupId);
  } else if (type == CCT) {
    for (int i = 0; i < 5; i++) {
      updateStatus(type, deviceId, groupId, ON);
      delay(1);
    }
  } else if (type == RGB_CCT) {
    for (int i = 0; i < 5; i++) {
      updateStatus(type, deviceId, 0, ON);
      delay(1);
    }
  }
}
    
void MiLightClient::pressButton(const MiLightRadioType type, const uint16_t deviceId, const uint8_t groupId, const uint8_t button) {
  if (type == RGBW) {
    writeRgbw(deviceId, 0, 0, groupId, button);
  } else if (type == CCT) {
    writeCct(deviceId, groupId, button);
  }
}

void MiLightClient::allOn(const MiLightRadioType type, const uint16_t deviceId) {
  if (type == RGBW) {
    writeRgbw(deviceId, 0, 0, 0, RGBW_ALL_ON);
  } else if (type == CCT) {
    writeCct(deviceId, 0, CCT_ALL_ON);
  } else if (type == RGB_CCT) {
    updateStatus(RGB_CCT, deviceId, 0, ON);
  }
}

void MiLightClient::allOff(const MiLightRadioType type, const uint16_t deviceId) {
  if (type == RGBW) {
    writeRgbw(deviceId, 0, 0, 0, RGBW_ALL_OFF);
  } else if (type == CCT) {
    writeCct(deviceId, 0, CCT_ALL_OFF);
  } else if (type == RGB_CCT) {
    updateStatus(RGB_CCT, deviceId, 0, OFF);
  }
}

void MiLightClient::increaseCctBrightness(const uint16_t deviceId, const uint8_t groupId) {
  writeCct(deviceId, groupId, CCT_BRIGHTNESS_UP);
}

void MiLightClient::decreaseCctBrightness(const uint16_t deviceId, const uint8_t groupId) {
  writeCct(deviceId, groupId, CCT_BRIGHTNESS_DOWN);
}

void MiLightClient::updateCctBrightness(const uint16_t deviceId, const uint8_t groupId, const uint8_t brightness) {
  for (int i = 0; i < MILIGHT_CCT_INTERVALS; i++) {
    decreaseCctBrightness(deviceId, groupId);
  }
  for (int i = 0; i < brightness/10; i++) {
    increaseCctBrightness(deviceId, groupId);
  }
}

void MiLightClient::increaseTemperature(const uint16_t deviceId, const uint8_t groupId) {
  writeCct(deviceId, groupId, CCT_TEMPERATURE_UP);
}

void MiLightClient::decreaseTemperature(const uint16_t deviceId, const uint8_t groupId) {
  writeCct(deviceId, groupId, CCT_TEMPERATURE_DOWN);
}

void MiLightClient::updateTemperature(const uint16_t deviceId, const uint8_t groupId, const uint8_t temperature) {
  for (int i = 0; i < MILIGHT_CCT_INTERVALS; i++) {
    decreaseTemperature(deviceId, groupId);
  }
  for (int i = 0; i < temperature; i++) {
    increaseTemperature(deviceId, groupId);
  }
}

uint8_t MiLightClient::getCctStatusButton(uint8_t groupId, MiLightStatus status) {
  uint8_t button = 0;
  
  if (status == ON) {
    switch(groupId) {
      case 1:
        button = CCT_GROUP_1_ON;
        break;
      case 2:
        button = CCT_GROUP_2_ON;
        break;
      case 3:
        button = CCT_GROUP_3_ON;
        break;
      case 4:
        button = CCT_GROUP_4_ON;
        break;
    }
  } else {
    switch(groupId) {
      case 1:
        button = CCT_GROUP_1_OFF;
        break;
      case 2:
        button = CCT_GROUP_2_OFF;
        break;
      case 3:
        button = CCT_GROUP_3_OFF;
        break;
      case 4:
        button = CCT_GROUP_4_OFF;
        break;
    }
  }
  
  return button;
}

MiLightRadioType MiLightClient::getRadioType(const String& typeName) {
  if (typeName.equalsIgnoreCase("rgbw")) {
    return RGBW;
  } else if (typeName.equalsIgnoreCase("cct")) {
    return CCT;
  } else if (typeName.equalsIgnoreCase("rgb_cct")) {
    return RGB_CCT;
  } else {
    return UNKNOWN;
  }
}

const MiLightRadioConfig& MiLightClient::getRadioConfig(const String& typeName) {
  switch (getRadioType(typeName)) {
    case RGBW:
      return MilightRgbwConfig;
    case CCT:
      return MilightCctConfig;
    case RGB_CCT:
      return MilightRgbCctConfig;
    default:
      Serial.print("Unknown radio type: ");
      Serial.println(typeName);
      return MilightRgbwConfig;
  }
}
    
void MiLightClient::formatPacket(MiLightRadioConfig& config, uint8_t* packet, char* buffer) {
  if (config.type == RGBW || config.type == CCT) {
    String format = String("Request type  : %02X\n") 
      + "Device ID     : %02X%02X\n"
      + "b1            : %02X\n"
      + "b2            : %02X\n"
      + "b3            : %02X\n"
      + "Sequence Num. : %02X";
      
    sprintf(
      buffer,
      format.c_str(),
      packet[0],
      packet[1], packet[2],
      packet[3],
      packet[4],
      packet[5],
      packet[6]
    );
  } else {
    for (int i = 0; i < config.packetLength; i++) {
      sprintf(buffer, "%02X ", packet[i]);
      buffer += 3;
    }
    sprintf(buffer, "\n\n");
  }
}

uint8_t MiLightClient::xorKey(uint8_t key) {
  // Generate most significant nibble
  const uint8_t shift = (key & 0x0F) < 0x04 ? 0 : 1;
  const uint8_t x = (((key & 0xF0) >> 4) + shift + 6) % 8;
  const uint8_t msn = (((4 + x) ^ 1) & 0x0F) << 4;

  // Generate least significant nibble
  const uint8_t lsn = ((((key & 0xF) + 4)^2) & 0x0F);

  return ( msn | lsn );
}

uint8_t MiLightClient::encodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2) {
  uint8_t value = (byte + s1) % 0x100;
  value = value ^ xorKey;
  value = (value + s2) % 0x100;
  
  return value;
}

void MiLightClient::encodeV2Packet(uint8_t *packet) {
  uint8_t key = xorKey(packet[0]);
  uint8_t sum = key;
  
  for (size_t i = 1; i <= 7; i++) {
    sum += packet[i];
    packet[i] = encodeByte(packet[i], 0, key, V2_OFFSET(i, packet[0]));
  }
  
  packet[8] = encodeByte(sum, 2, key, V2_OFFSET(8, packet[0]));
  
  printf("encoded packet: ");
  for (int i = 0; i < MilightRgbCctConfig.packetLength; i++) {
    printf("%02X ", packet[i]);
  }
  printf("\n");
}