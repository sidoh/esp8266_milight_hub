#include <MiLightClient.h>
#include <MiLightRadioConfig.h>

MiLightRadio* MiLightClient::getRadio(const MiLightRadioType type) {
  MiLightRadioStack* stack = NULL;
  
  if (type == RGBW) {
    stack = rgbwRadio;
  } else if (type == CCT) {
    stack = cctRadio;
  } else if (type == RGBW_CCT) {
    stack = rgbwCctRadio;
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

void MiLightClient::write(const MiLightRadioType radioType, 
  uint8_t packet[]) {
    
  MiLightRadio* radio = getRadio(radioType);
  
  if (radio == NULL) {
    return;
  }
  
  for (int i = 0; i < this->resendCount; i++) {
    radio->write(packet, MILIGHT_PACKET_LENGTH);
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
  
  write(RGBW, packet);
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
  
  write(CCT, packet);
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
  } else {
    writeCct(deviceId, groupId, getCctStatusButton(groupId, status));
  }
}

void MiLightClient::updateColorWhite(const uint16_t deviceId, const uint8_t groupId) {
  uint8_t button = RGBW_GROUP_1_MAX_LEVEL + ((groupId - 1)*2);
  pressButton(RGBW, deviceId, groupId, button);
}

void MiLightClient::pair(const MiLightRadioType type, const uint16_t deviceId, const uint8_t groupId) {
  updateStatus(type, deviceId, groupId, ON);
}

void MiLightClient::unpair(const MiLightRadioType type, const uint16_t deviceId, const uint8_t groupId) {
  if (type == RGBW) {
    updateStatus(RGBW, deviceId, groupId, ON);
    delay(1);
    updateColorWhite(deviceId, groupId);
  } else if (type == CCT) {
    for (int i = 0; i < 5; i++) {
      updateStatus(CCT, deviceId, groupId, ON);
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
  }
}

void MiLightClient::allOff(const MiLightRadioType type, const uint16_t deviceId) {
  if (type == RGBW) {
    writeRgbw(deviceId, 0, 0, 0, RGBW_ALL_OFF);
  } else if (type == CCT) {
    writeCct(deviceId, 0, CCT_ALL_OFF);
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
    case RGBW_CCT:
      return MilightRgbwCctConfig;
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