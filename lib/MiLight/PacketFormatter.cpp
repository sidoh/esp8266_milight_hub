#include <PacketFormatter.h>

uint8_t* PacketFormatter::PACKET_BUFFER = new uint8_t[PACKET_FORMATTER_BUFFER_SIZE];

PacketStream::PacketStream()
    : packetStream(NULL),
      numPackets(0),
      packetLength(0),
      currentPacket(0)
{ }

bool PacketStream::hasNext() {
  return currentPacket < numPackets;
}

uint8_t* PacketStream::next() {
  uint8_t* packet = packetStream + (currentPacket * packetLength);
  currentPacket++;
  return packet;
}

PacketFormatter::PacketFormatter(const size_t packetLength, const size_t maxPackets)
  : packetLength(packetLength),
    numPackets(0),
    currentPacket(NULL),
    held(false)
{
  packetStream.packetLength = packetLength;
  packetStream.packetStream = PACKET_BUFFER;
}

void PacketFormatter::finalizePacket(uint8_t* packet) { }

void PacketFormatter::updateStatus(MiLightStatus status) {
  updateStatus(status, groupId);
}

void PacketFormatter::setHeld(bool held) {
  this->held = held;
}

void PacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) { }
void PacketFormatter::updateBrightness(uint8_t value) { }
void PacketFormatter::updateMode(uint8_t value) { }
void PacketFormatter::modeSpeedDown() { }
void PacketFormatter::modeSpeedUp() { }
void PacketFormatter::nextMode() { }
void PacketFormatter::previousMode() { }
void PacketFormatter::command(uint8_t command, uint8_t arg) { }

void PacketFormatter::updateHue(uint16_t value) { }
void PacketFormatter::updateColorRaw(uint8_t value) { }
void PacketFormatter::updateColorWhite() { }

void PacketFormatter::increaseTemperature() { }
void PacketFormatter::decreaseTemperature() { }
void PacketFormatter::increaseBrightness() { }
void PacketFormatter::decreaseBrightness() { }

void PacketFormatter::updateTemperature(uint8_t value) { }
void PacketFormatter::updateSaturation(uint8_t value) { }

void PacketFormatter::pair() {
  for (size_t i = 0; i < 5; i++) {
    updateStatus(ON);
  }
}

void PacketFormatter::unpair() {
  pair();
}

PacketStream& PacketFormatter::buildPackets() {
  if (numPackets > 0) {
    finalizePacket(currentPacket);
  }

  packetStream.numPackets = numPackets;
  packetStream.currentPacket = 0;

  return packetStream;
}

void PacketFormatter::valueByStepFunction(StepFunction increase, StepFunction decrease, uint8_t numSteps, uint8_t value) {
  for (size_t i = 0; i < numSteps; i++) {
    (this->*decrease)();
  }

  for (size_t i = 0; i < value; i++) {
    (this->*increase)();
  }
}

void PacketFormatter::prepare(uint16_t deviceId, uint8_t groupId) {
  this->deviceId = deviceId;
  this->groupId = groupId;
  reset();
}

void PacketFormatter::reset() {
  this->numPackets = 0;
  this->currentPacket = currentPacket;
  this->held = false;
}

void PacketFormatter::pushPacket() {
  if (numPackets > 0) {
    finalizePacket(currentPacket);
  }

  currentPacket = PACKET_BUFFER + (numPackets * packetLength);
  numPackets++;
  initializePacket(currentPacket);
}

void PacketFormatter::format(uint8_t const* packet, char* buffer) {
  for (int i = 0; i < packetLength; i++) {
    sprintf_P(buffer, "%02X ", packet[i]);
    buffer += 3;
  }
  sprintf_P(buffer, "\n\n");
}

void PacketFormatter::formatV1Packet(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, "Request type  : %02X\n", packet[0]) ;
  buffer += sprintf_P(buffer, "Device ID     : %02X%02X\n", packet[1], packet[2]);
  buffer += sprintf_P(buffer, "b1            : %02X\n", packet[3]);
  buffer += sprintf_P(buffer, "b2            : %02X\n", packet[4]);
  buffer += sprintf_P(buffer, "b3            : %02X\n", packet[5]);
  buffer += sprintf_P(buffer, "Sequence Num. : %02X", packet[6]);
}

size_t PacketFormatter::getPacketLength() const {
  return packetLength;
}
