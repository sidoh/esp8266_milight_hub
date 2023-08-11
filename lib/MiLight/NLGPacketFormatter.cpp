#include <NLGPacketFormatter.h>
#include <MiLightCommands.h>

bool NLGPacketFormatter::canHandle(const uint8_t *packet, const size_t len) {
  return len == packetLength && packet[0] == 0x00;
}

/*
00 01 02 03 04 05 06 07 08 09 
======================= =CRC=
07 00 20 FE 00 01 B1 00  All-On
07 00 20 FE 00 02 B3 00  All-Off

07 00 20 FE 07 07 B5 00  Group1 An
07 00 20 FE 00 08 B7 00  Group1 Aus

07 00 20 FE 09 09 B9 00  Group2 An
07 00 20 FE 00 0A BB 00  Group2 Aus

07 00 20 FE 0B 0B BD 00  Group3 An
07 00 20 FE 00 0C BF 00  Group3 Aus

07 00 20 FE 0D 0D C1 00  Group4 An
07 00 20 FE 00 0E C3 00  Group4 Aus

07 00 20 FE 00 03 C5 00  heller
07 00 20 FE 00 06 C7 00  dunkler

07 00 20 FE 00 04 CB 00  wärmer
07 00 20 FE 00 05 CD 00  kälter
*/

void NLGPacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  // Byte 0: Packet length = 7 bytes

  // Byte 1: NLG protocol
  packet[packetPtr++] = 0x00;

  // Byte 2 and 3: Device ID
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;

  // Byte 4: Active Group filled in later
  packet[packetPtr++] = 0x00; //groupToGroupId(groupId);


  // Byte 5: Bulb command, filled in later
  packet[packetPtr++] = 0x00;

  
  // Byte 6: Packet sequence number 0..254
  // increased by 2
  sequenceNum += 2;
  packet[packetPtr++] = sequenceNum;

  // Byte 7: Zero
  packet[packetPtr++] = 0x00;

  // Byte 8: CRC LSB
  // Byte 9: CRC MSB
}

void NLGPacketFormatter::finalizePacket(uint8_t* packet) {
}

void NLGPacketFormatter::updateBrightness(uint8_t value) {
  const GroupState* state = this->stateStore->get(deviceId, groupId, MiLightRemoteType::REMOTE_TYPE_NLG);
  int8_t knownValue = (state != NULL && state->isSetBrightness()) ? state->getBrightness() : -1;

  valueByStepFunction(
    &PacketFormatter::increaseBrightness,
    &PacketFormatter::decreaseBrightness,
    20,
    value / 20,
    knownValue / 20
  );
}

void NLGPacketFormatter::updateTemperature(uint8_t value) {
  const GroupState* state = this->stateStore->get(deviceId, groupId, MiLightRemoteType::REMOTE_TYPE_NLG);
  int8_t knownValue = (state != NULL && state->isSetKelvin()) ? state->getKelvin() : -1;

  valueByStepFunction(
    &PacketFormatter::increaseTemperature,
    &PacketFormatter::decreaseTemperature,
    20,
    value / 20,
    knownValue / 20
  );
}

void NLGPacketFormatter::command(uint8_t command, uint8_t arg) {
  // arg1 1 = send active group
  pushPacket();
  if(arg) currentPacket[3] = groupToGroupId(groupId)+1;
  else currentPacket[3] = 0;

  currentPacket[4] = command;
}

void NLGPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(groupToGroupId(groupId) +1 +status, !status);
}


uint8_t NLGPacketFormatter::groupToGroupId(uint8_t group){
  switch(group) {
    case 1:
      return 0x06;
    case 2:
      return 0x08;
    case 3:
      return 0x0A;
    case 4:
      return 0x0C;
    default:
      return 0x00;
  }
}

BulbId NLGPacketFormatter::parsePacket(const uint8_t* packet, JsonObject result) {
  Serial.println("NLG parse");
  uint8_t command = packet[4]; 

  if(command > 6) { // On/Off
    groupId = groupIdToGroup(command); // groupid only included if command >6
    
    result[GroupStateFieldNames::STATE] = command & 0x01 ? "ON" : "OFF";

  } else {
    // DPAD 
    switch(command){
        case 0x03:
          result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::LEVEL_UP;
          break;

        case 0x04:
          result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::TEMPERATURE_DOWN;
          break;

        case 0x05:
          result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::TEMPERATURE_UP;
          break;

        case 0x06:
          result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::LEVEL_DOWN;
          break;

        default:
          result["button_id"] = command;
          break;
      }
  }

  BulbId bulbId((packet[1] << 8) | packet[2], groupId, REMOTE_TYPE_NLG);

  sequenceNum = packet[5];

  return bulbId;
}

uint8_t NLGPacketFormatter::groupIdToGroup(uint8_t groupId){
  printf("find group: 0x%x\n", groupId);
  switch((groupId -1) & 0b00001110) {
    case 0x06:
      return 1;
    case 0x08:
      return 2;
    case 0x0A:
      return 3;
    case 0x0C:
      return 4;
    default:
    printf("group undefined: 0x%x\n", (groupId -1) & 0b00001110);
      return 255;
  }
}

void NLGPacketFormatter::increaseTemperature() {
  command(0x04, 1);
}

void NLGPacketFormatter::decreaseTemperature() {
  command(0x05, 1);
}

void NLGPacketFormatter::increaseBrightness() {
  command(0x03, 1);
}

void NLGPacketFormatter::decreaseBrightness() {
  command(0x06, 1);
}

void NLGPacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, PSTR("Byte 1 (00)   : %02X\n"), packet[0]) ;
  buffer += sprintf_P(buffer, PSTR("Device ID     : %02X%02X\n"), packet[1], packet[2]);
  buffer += sprintf_P(buffer, PSTR("Group         : %02X\n"), packet[3]);
  buffer += sprintf_P(buffer, PSTR("CMD           : %02X\n"), packet[4]);
  buffer += sprintf_P(buffer, PSTR("Sequence Num. : %02X\n"), packet[5]);
  buffer += sprintf_P(buffer, PSTR("Byte 7 (00)   : %02X\n"), packet[6]) ;
  //buffer += sprintf_P(buffer, PSTR("Payload Chksum: %02X%02X\n"), packet[7], packet[8]);
}
