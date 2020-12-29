#include <CctPacketFormatter.h>
#include <MiLightCommands.h>

static const uint8_t CCT_PROTOCOL_ID = 0x5A;

bool CctPacketFormatter::canHandle(const uint8_t *packet, const size_t len) {
  return len == packetLength && packet[0] == CCT_PROTOCOL_ID;
}

void CctPacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  // Byte 0: Packet length = 7 bytes

  // Byte 1: CCT protocol
  packet[packetPtr++] = CCT_PROTOCOL_ID;

  // Byte 2 and 3: Device ID
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;

  // Byte 4: Zone
  packet[packetPtr++] = groupId;

  // Byte 5: Bulb command, filled in later
  packet[packetPtr++] = 0;

  // Byte 6: Packet sequence number 0..255
  packet[packetPtr++] = sequenceNum++;

  // Byte 7: Checksum over previous bytes, including packet length = 7
  // The checksum will be calculated when setting the command field
  packet[packetPtr++] = 0;

  // Byte 8: CRC LSB
  // Byte 9: CRC MSB
}

void CctPacketFormatter::finalizePacket(uint8_t* packet) {
  uint8_t checksum;

  // Calculate checksum over packet length .. sequenceNum
  checksum = 7; // Packet length is not part of packet
  for (uint8_t i = 0; i < 6; i++) {
    checksum += currentPacket[i];
  }
  // Store the checksum in the sixth byte
  currentPacket[6] = checksum;
}

void CctPacketFormatter::updateBrightness(uint8_t value) {
  const GroupState* state = this->stateStore->get(deviceId, groupId, MiLightRemoteType::REMOTE_TYPE_CCT);
  int8_t knownValue = (state != NULL && state->isSetBrightness()) ? state->getBrightness() / CCT_INTERVALS : -1;

  valueByStepFunction(
    &PacketFormatter::increaseBrightness,
    &PacketFormatter::decreaseBrightness,
    CCT_INTERVALS,
    value / CCT_INTERVALS,
    knownValue
  );
}

void CctPacketFormatter::updateTemperature(uint8_t value) {
  const GroupState* state = this->stateStore->get(deviceId, groupId, MiLightRemoteType::REMOTE_TYPE_CCT);
  int8_t knownValue = (state != NULL && state->isSetKelvin()) ? state->getKelvin() / CCT_INTERVALS : -1;

  valueByStepFunction(
    &PacketFormatter::increaseTemperature,
    &PacketFormatter::decreaseTemperature,
    CCT_INTERVALS,
    value / CCT_INTERVALS,
    knownValue
  );
}

void CctPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x80;
  }
  currentPacket[CCT_COMMAND_INDEX] = command;
}

void CctPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(getCctStatusButton(groupId, status), 0);
}

void CctPacketFormatter::increaseTemperature() {
  command(CCT_TEMPERATURE_UP, 0);
}

void CctPacketFormatter::decreaseTemperature() {
  command(CCT_TEMPERATURE_DOWN, 0);
}

void CctPacketFormatter::increaseBrightness() {
  command(CCT_BRIGHTNESS_UP, 0);
}

void CctPacketFormatter::decreaseBrightness() {
  command(CCT_BRIGHTNESS_DOWN, 0);
}

void CctPacketFormatter::enableNightMode() {
  command(getCctStatusButton(groupId, OFF) | 0x10, 0);
}

uint8_t CctPacketFormatter::getCctStatusButton(uint8_t groupId, MiLightStatus status) {
  uint8_t button = 0;

  if (status == ON) {
    switch(groupId) {
      case 0:
        button = CCT_ALL_ON;
        break;
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
      case 0:
        button = CCT_ALL_OFF;
        break;
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

uint8_t CctPacketFormatter::cctCommandIdToGroup(uint8_t command) {
  switch (command & 0xF) {
    case CCT_GROUP_1_ON:
    case CCT_GROUP_1_OFF:
      return 1;
    case CCT_GROUP_2_ON:
    case CCT_GROUP_2_OFF:
      return 2;
    case CCT_GROUP_3_ON:
    case CCT_GROUP_3_OFF:
      return 3;
    case CCT_GROUP_4_ON:
    case CCT_GROUP_4_OFF:
      return 4;
    case CCT_ALL_ON:
    case CCT_ALL_OFF:
      return 0;
  }

  return 255;
}

MiLightStatus CctPacketFormatter::cctCommandToStatus(uint8_t command) {
  switch (command & 0xF) {
    case CCT_GROUP_1_ON:
    case CCT_GROUP_2_ON:
    case CCT_GROUP_3_ON:
    case CCT_GROUP_4_ON:
    case CCT_ALL_ON:
      return ON;
    case CCT_GROUP_1_OFF:
    case CCT_GROUP_2_OFF:
    case CCT_GROUP_3_OFF:
    case CCT_GROUP_4_OFF:
    case CCT_ALL_OFF:
    default:
      return OFF;
  }
}

BulbId CctPacketFormatter::parsePacket(const uint8_t* packet, JsonObject result) {
  uint8_t command = packet[CCT_COMMAND_INDEX] & 0x7F;

  uint8_t onOffGroupId = cctCommandIdToGroup(command);
  BulbId bulbId(
    (packet[1] << 8) | packet[2],
    onOffGroupId < 255 ? onOffGroupId : packet[3],
    REMOTE_TYPE_CCT
  );

  // Night mode
  if (command & 0x10) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::NIGHT_MODE;
  } else if (onOffGroupId < 255) {
    result[GroupStateFieldNames::STATE] = cctCommandToStatus(command) == ON ? "ON" : "OFF";
  } else if (command == CCT_BRIGHTNESS_DOWN) {
    result[GroupStateFieldNames::COMMAND] = "brightness_down";
  } else if (command == CCT_BRIGHTNESS_UP) {
    result[GroupStateFieldNames::COMMAND] = "brightness_up";
  } else if (command == CCT_TEMPERATURE_DOWN) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::TEMPERATURE_DOWN;
  } else if (command == CCT_TEMPERATURE_UP) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::TEMPERATURE_UP;
  } else {
    result["button_id"] = command;
  }

  return bulbId;
}

void CctPacketFormatter::format(uint8_t const* packet, char* buffer) {
  PacketFormatter::formatV1Packet(packet, buffer);
}
