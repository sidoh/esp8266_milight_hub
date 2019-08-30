#include <PacketFormatter.h>

static uint8_t* PACKET_BUFFER = new uint8_t[PACKET_FORMATTER_BUFFER_SIZE];

PacketStream::PacketStream()
    : packetStream(PACKET_BUFFER),
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

PacketFormatter::PacketFormatter(const MiLightRemoteType deviceType, const size_t packetLength, const size_t maxPackets)
  : deviceType(deviceType),
    packetLength(packetLength),
    numPackets(0),
    currentPacket(NULL),
    held(false)
{
  packetStream.packetLength = packetLength;
}

void PacketFormatter::initialize(GroupStateStore* stateStore, const Settings* settings) {
  this->stateStore = stateStore;
  this->settings = settings;
}

bool PacketFormatter::canHandle(const uint8_t *packet, const size_t len) {
  return len == packetLength;
}

void PacketFormatter::finalizePacket(uint8_t* packet) { }

void PacketFormatter::updateStatus(MiLightStatus status) {
  updateStatus(status, groupId);
}

void PacketFormatter::toggleStatus() {
  const GroupState* state = stateStore->get(deviceId, groupId, deviceType);

  if (state && state->isSetState() && state->getState() == MiLightStatus::ON) {
    updateStatus(MiLightStatus::OFF);
  } else {
    updateStatus(MiLightStatus::ON);
  }
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
void PacketFormatter::enableNightMode() { }

void PacketFormatter::updateTemperature(uint8_t value) { }
void PacketFormatter::updateSaturation(uint8_t value) { }

BulbId PacketFormatter::parsePacket(const uint8_t *packet, JsonObject result) {
  return DEFAULT_BULB_ID;
}

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

void PacketFormatter::valueByStepFunction(StepFunction increase, StepFunction decrease, uint8_t numSteps, uint8_t targetValue, int8_t knownValue) {
  StepFunction fn;
  size_t numCommands = 0;

  // If current value is not known, drive down to minimum value.  Then we can assume that we
  // know the state (it'll be 0).
  if (knownValue == -1) {
    for (size_t i = 0; i < numSteps; i++) {
      (this->*decrease)();
    }

    fn = increase;
    numCommands = targetValue;
  } else if (targetValue < knownValue) {
    fn = decrease;
    numCommands = (knownValue - targetValue);
  } else if (targetValue > knownValue) {
    fn = increase;
    numCommands = (targetValue - knownValue);
  } else {
    return;
  }

  // Get to the desired value
  for (size_t i = 0; i < numCommands; i++) {
    (this->*fn)();
  }
}

void PacketFormatter::prepare(uint16_t deviceId, uint8_t groupId) {
  this->deviceId = deviceId;
  this->groupId = groupId;
  reset();
}

void PacketFormatter::reset() {
  this->numPackets = 0;
  this->currentPacket = PACKET_BUFFER;
  this->held = false;
}

void PacketFormatter::pushPacket() {
  if (numPackets > 0) {
    finalizePacket(currentPacket);
  }

  // Make sure there's enough buffer to add another packet.
  if ((currentPacket + packetLength) >= PACKET_BUFFER + PACKET_FORMATTER_BUFFER_SIZE) {
    Serial.println(F("ERROR: packet buffer full!  Cannot buffer a new packet.  THIS IS A BUG!"));
    return;
  }

  currentPacket = PACKET_BUFFER + (numPackets * packetLength);
  numPackets++;
  initializePacket(currentPacket);
}

void PacketFormatter::format(uint8_t const* packet, char* buffer) {
  for (size_t i = 0; i < packetLength; i++) {
    sprintf_P(buffer, "%02X ", packet[i]);
    buffer += 3;
  }
  sprintf_P(buffer, "\n\n");
}

void PacketFormatter::formatV1Packet(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, PSTR("Request type  : %02X\n"), packet[0]) ;
  buffer += sprintf_P(buffer, PSTR("Device ID     : %02X%02X\n"), packet[1], packet[2]);
  buffer += sprintf_P(buffer, PSTR("b1            : %02X\n"), packet[3]);
  buffer += sprintf_P(buffer, PSTR("b2            : %02X\n"), packet[4]);
  buffer += sprintf_P(buffer, PSTR("b3            : %02X\n"), packet[5]);
  buffer += sprintf_P(buffer, PSTR("Sequence Num. : %02X"), packet[6]);
}

size_t PacketFormatter::getPacketLength() const {
  return packetLength;
}

BulbId PacketFormatter::currentBulbId() const {
  return BulbId(deviceId, groupId, deviceType);
}