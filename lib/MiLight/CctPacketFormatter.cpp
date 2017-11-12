#include <CctPacketFormatter.h>

static const uint8_t CCT_PROTOCOL_ID = 0x5A;

bool CctPacketFormatter::canHandle(const uint8_t *packet, const size_t len) {
  return len == packetLength && packet[0] == CCT_PROTOCOL_ID;
}

void CctPacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  packet[packetPtr++] = CCT_PROTOCOL_ID;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = groupId;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum;
  packet[packetPtr++] = sequenceNum++;
}

void CctPacketFormatter::updateBrightness(uint8_t value) {
  valueByStepFunction(
    &PacketFormatter::increaseBrightness,
    &PacketFormatter::decreaseBrightness,
    CCT_INTERVALS,
    value / CCT_INTERVALS
  );
}

void CctPacketFormatter::updateTemperature(uint8_t value) {
  valueByStepFunction(
    &PacketFormatter::increaseTemperature,
    &PacketFormatter::decreaseTemperature,
    CCT_INTERVALS,
    value / CCT_INTERVALS
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
      return OFF;
  }
}

BulbId CctPacketFormatter::parsePacket(const uint8_t* packet, JsonObject& result, GroupStateStore* stateStore) {
  uint8_t command = packet[CCT_COMMAND_INDEX] & 0x7F;

  BulbId bulbId(
    (packet[1] << 8) | packet[2],
    packet[3],
    REMOTE_TYPE_CCT
  );

  uint8_t onOffGroupId = cctCommandIdToGroup(command);
  if (onOffGroupId < 255) {
    result["state"] = cctCommandToStatus(command) == ON ? "ON" : "OFF";
  } else if (command == CCT_BRIGHTNESS_DOWN) {
    result["command"] = "brightness_down";
  } else if (command == CCT_BRIGHTNESS_UP) {
    result["command"] = "brightness_up";
  } else if (command == CCT_TEMPERATURE_DOWN) {
    result["command"] = "temperature_down";
  } else if (command == CCT_TEMPERATURE_UP) {
    result["command"] = "temperature_up";
  } else {
    result["button_id"] = command;
  }

  return bulbId;
}

void CctPacketFormatter::format(uint8_t const* packet, char* buffer) {
  PacketFormatter::formatV1Packet(packet, buffer);
}
