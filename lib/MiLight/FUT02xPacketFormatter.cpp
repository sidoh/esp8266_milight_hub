#include <FUT02xPacketFormatter.h>

static const uint8_t FUT02X_PACKET_HEADER = 0xA5;

static const uint8_t FUT02X_PAIR_COMMAND = 0x03;
static const uint8_t FUT02X_UNPAIR_COMMAND = 0x03;

void FUT02xPacketFormatter::initializePacket(uint8_t *packet) {
  size_t packetPtr = 0;

  packet[packetPtr++] = 0xA5;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0; // arg
  packet[packetPtr++] = 0; // command
  packet[packetPtr++] = sequenceNum++;
}

bool FUT02xPacketFormatter::canHandle(const uint8_t* packet, const size_t len) {
  return len == packetLength && packet[0] == FUT02X_PACKET_HEADER;
}

void FUT02xPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x10;
  }
  currentPacket[FUT02X_COMMAND_INDEX] = command;
  currentPacket[FUT02X_ARGUMENT_INDEX] = arg;
}

void FUT02xPacketFormatter::pair() {
  for (size_t i = 0; i < 5; i++) {
    command(FUT02X_PAIR_COMMAND, 0);
  }
}

void FUT02xPacketFormatter::unpair() {
  for (size_t i = 0; i < 5; i++) {
    command(FUT02X_PAIR_COMMAND, 0);
  }
}

void FUT02xPacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, PSTR("b0       : %02X\n"), packet[0]);
  buffer += sprintf_P(buffer, PSTR("ID       : %02X%02X\n"), packet[1], packet[2]);
  buffer += sprintf_P(buffer, PSTR("Arg      : %02X\n"), packet[3]);
  buffer += sprintf_P(buffer, PSTR("Command  : %02X\n"), packet[4]);
  buffer += sprintf_P(buffer, PSTR("Sequence : %02X\n"), packet[5]);
}