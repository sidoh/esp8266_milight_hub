#include <CasaluxPacketFormatter.h>
#include <MiLightCommands.h>

bool CasaluxPacketFormatter::canHandle(const uint8_t *packet, const size_t len) {
  return len == packetLength && packet[1] == 0x01;
}

void CasaluxPacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  // Byte 0: Packet length = 10 bytes

  // Byte 1: Bulb command, filled in later
  packet[packetPtr++] = 0;

  // Byte 2: Casalux protocol
  packet[packetPtr++] = 0x01;

  // Byte 3: 11
  packet[packetPtr++] = 0x11;

  // Byte 4 and 5: Device ID
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;

  // Byte 6: Zone
  packet[packetPtr++] = groupToGroupId(groupId);

  // Byte 7: Zero
  packet[packetPtr++] = 0;

  // Byte 8: Packet sequence number 0..255
  packet[packetPtr++] = ++sequenceNum;

  // Byte 9 +10: Checksum over previous bytes, including packet length = 7
  // The checksum will be calculated when setting the command field
  packet[packetPtr++] = 0;
  packet[packetPtr++] = 0;

  // Byte 11: CRC LSB
  // Byte 12: CRC MSB
}

void CasaluxPacketFormatter::finalizePacket(uint8_t* packet) {
  uint16_t checksum;

  // Calculate checksum over packet length .. sequenceNum
  checksum = packetLength; // Packet length is not part of packet
  for (uint8_t i = 0; i < packetLength; i++) {
    checksum += currentPacket[i];
  }
  // Store the checksum in the 9th, 10th bytes
  currentPacket[8] = checksum >> 8;
  currentPacket[9] = checksum & 0x00FF;
}

void CasaluxPacketFormatter::updateBrightness(uint8_t value) {
  const GroupState* state = this->stateStore->get(deviceId, groupId, MiLightRemoteType::REMOTE_TYPE_CASALUX);
  int8_t knownValue = (state != NULL && state->isSetBrightness()) ? state->getBrightness() : -1;

  valueByStepFunction(
    &PacketFormatter::increaseBrightness,
    &PacketFormatter::decreaseBrightness,
    20,
    value / 20,
    knownValue / 20
  );
}

void CasaluxPacketFormatter::updateTemperature(uint8_t value) {
  const GroupState* state = this->stateStore->get(deviceId, groupId, MiLightRemoteType::REMOTE_TYPE_CASALUX);
  int8_t knownValue = (state != NULL && state->isSetKelvin()) ? state->getKelvin() : -1;

  valueByStepFunction(
    &PacketFormatter::increaseTemperature,
    &PacketFormatter::decreaseTemperature,
    20,
    value / 20,
    knownValue / 20
  );
}

void CasaluxPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  currentPacket[0] = command;
}

void CasaluxPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  if(status == ON) command(0x6F,0);
  else command(0x60,0);
}

void CasaluxPacketFormatter::increaseTemperature() {
  command(0x69, 0);
}

void CasaluxPacketFormatter::decreaseTemperature() {
  command(0x6A, 0);
}

void CasaluxPacketFormatter::increaseBrightness() {
  command(0x65, 0);
}

void CasaluxPacketFormatter::decreaseBrightness() {
  command(0x66, 0);
}

uint8_t CasaluxPacketFormatter::groupToGroupId(uint8_t group){
  switch(group) {
    case 1:
      return 0xD1;
    case 2:
      return 0xD2;
    case 3:
      return 0xD4;
    case 4:
      return 0xD8;
    default:
      return 0xDF;
  }
}

uint8_t CasaluxPacketFormatter::groupIdToGroup(uint8_t groupId){
  switch(groupId) {
    case 0xD1:
      return 1;
    case 0xD2:
      return 2;
    case 0xD4:
      return 3;
    case 0xD8:
      return 4;
    default:
      return 255;
  }
}

BulbId CasaluxPacketFormatter::parsePacket(const uint8_t* packet, JsonObject result) {
  uint8_t command = packet[0];

  uint8_t onOffGroup = groupIdToGroup(packet[5]);

  BulbId bulbId(
    (packet[2] << 8) | packet[3],
    onOffGroup,
    REMOTE_TYPE_CASALUX
  );

  sequenceNum = packet[7];
  sequenceNum++;
  
  if(onOffGroup < 255) {
    result[GroupStateFieldNames::STATE] = command == 0x6F ? "ON" : "OFF";
  }
  
  if (command == 0x66) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::LEVEL_DOWN;
  } else if (command == 0x65) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::LEVEL_UP;
  } else if (command == 0x6A) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::TEMPERATURE_DOWN;
  } else if (command == 0x69) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::TEMPERATURE_UP;
  } else {
    result["button_id"] = command;
  }

  return bulbId;
}

void CasaluxPacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, PSTR("Request type  : %02X\n"), packet[0]) ;
  //buffer += sprintf_P(buffer, PSTR("Byte 3 (11)   : %02X\n"), packet[2]);
  buffer += sprintf_P(buffer, PSTR("Device ID     : %02X%02X\n"), packet[3], packet[4]);
  buffer += sprintf_P(buffer, PSTR("Group         : %02X\n"), packet[5]);
  //buffer += sprintf_P(buffer, PSTR("Byte 5(00)    : %02X\n"), packet[6]);
  buffer += sprintf_P(buffer, PSTR("Sequence Num. : %02X\n"), packet[7]);
  buffer += sprintf_P(buffer, PSTR("Payload Chksum: %02X%02X\n"), packet[8], packet[9]);
}
