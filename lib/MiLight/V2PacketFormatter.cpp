#include <V2PacketFormatter.h>
#include <V2RFEncoding.h>

#define GROUP_COMMAND_ARG(status, groupId, numGroups) ( groupId + (status == OFF ? (numGroups + 1) : 0) )

V2PacketFormatter::V2PacketFormatter(const MiLightRemoteType deviceType, uint8_t protocolId, uint8_t numGroups)
  : PacketFormatter(deviceType, 9),
    protocolId(protocolId),
    numGroups(numGroups)
{ }

bool V2PacketFormatter::canHandle(const uint8_t *packet, const size_t packetLen) {
  uint8_t packetCopy[V2_PACKET_LEN];
  memcpy(packetCopy, packet, V2_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);

#ifdef DEBUG_PRINTF
  Serial.printf_P(PSTR("Testing whether formater for ID %d can handle packet: with protocol ID %d...\n"), protocolId, packetCopy[V2_PROTOCOL_ID_INDEX]);
#endif

  return packetCopy[V2_PROTOCOL_ID_INDEX] == protocolId;
}

void V2PacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  // Always encode with 0x00 key. No utility in varying it.
  packet[packetPtr++] = 0x00;

  packet[packetPtr++] = protocolId;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum++;
  packet[packetPtr++] = groupId;
  packet[packetPtr++] = 0;
}

void V2PacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x80;
  }
  currentPacket[V2_COMMAND_INDEX] = command;
  currentPacket[V2_ARGUMENT_INDEX] = arg;
}

void V2PacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(0x01, GROUP_COMMAND_ARG(status, groupId, numGroups));
}

void V2PacketFormatter::unpair() {
  for (size_t i = 0; i < 5; i++) {
    updateStatus(ON, 0);
  }
}

void V2PacketFormatter::finalizePacket(uint8_t* packet) {
  V2RFEncoding::encodeV2Packet(packet);
}

void V2PacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, PSTR("Raw packet: "));
  for (size_t i = 0; i < packetLength; i++) {
    buffer += sprintf_P(buffer, PSTR("%02X "), packet[i]);
  }

  uint8_t decodedPacket[packetLength];
  memcpy(decodedPacket, packet, packetLength);

  V2RFEncoding::decodeV2Packet(decodedPacket);

  buffer += sprintf_P(buffer, PSTR("\n\nDecoded:\n"));
  buffer += sprintf_P(buffer, PSTR("Key      : %02X\n"), decodedPacket[0]);
  buffer += sprintf_P(buffer, PSTR("b1       : %02X\n"), decodedPacket[1]);
  buffer += sprintf_P(buffer, PSTR("ID       : %02X%02X\n"), decodedPacket[2], decodedPacket[3]);
  buffer += sprintf_P(buffer, PSTR("Command  : %02X\n"), decodedPacket[4]);
  buffer += sprintf_P(buffer, PSTR("Argument : %02X\n"), decodedPacket[5]);
  buffer += sprintf_P(buffer, PSTR("Sequence : %02X\n"), decodedPacket[6]);
  buffer += sprintf_P(buffer, PSTR("Group    : %02X\n"), decodedPacket[7]);
  buffer += sprintf_P(buffer, PSTR("Checksum : %02X"), decodedPacket[8]);
}

uint8_t V2PacketFormatter::groupCommandArg(MiLightStatus status, uint8_t groupId) {
  return GROUP_COMMAND_ARG(status, groupId, numGroups);
}

// helper method to return a bulb to the prior state
void V2PacketFormatter::switchMode(const GroupState& currentState, BulbMode desiredMode) {
  // revert back to the prior mode
  switch (desiredMode) {
    case BulbMode::BULB_MODE_COLOR:
      updateHue(currentState.getHue());
      break;
    case BulbMode::BULB_MODE_NIGHT:
      enableNightMode();
      break;
    case BulbMode::BULB_MODE_SCENE:
      updateMode(currentState.getMode());
      break;
    case BulbMode::BULB_MODE_WHITE:
      updateColorWhite();
      break;
    default:
      Serial.printf_P(PSTR("V2PacketFormatter::switchMode: Request to switch to unknown mode %d\n"), desiredMode);
      break;
  }

}

uint8_t V2PacketFormatter::tov2scale(uint8_t value, uint8_t endValue, uint8_t interval, bool reverse) {
  if (reverse) {
    value = 100 - value;
  }

  return (value * interval) + endValue;
}

uint8_t V2PacketFormatter::fromv2scale(uint8_t value, uint8_t endValue, uint8_t interval, bool reverse, uint8_t buffer) {
  value -= endValue;

  // Deal with underflow
  if (value >= (0xFF - buffer)) {
    value = 0;
  }

  value /= interval;

  if (reverse) {
    value = 100 - value;
  }

  if (value > 100) {
    // overflow
    if (value <= (100 + buffer)) {
      value = 100;
    // underflow (value is unsigned)
    } else {
      value = 0;
    }
  }
  return value;
}