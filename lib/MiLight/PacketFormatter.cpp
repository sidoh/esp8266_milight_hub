#include <PacketFormatter.h>

void PacketFormatter::updateStatus(MiLightStatus status) { 
  updateStatus(status, groupId);
}

void PacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) { }
void PacketFormatter::updateBrightness(uint8_t value) { }
void PacketFormatter::updateMode(uint8_t value) { }
void PacketFormatter::modeSpeedDown() { }
void PacketFormatter::modeSpeedUp() { }
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
  
uint8_t* PacketFormatter::buildPacket() {
  return this->packet;
}

void PacketFormatter::prepare(uint16_t deviceId, uint8_t groupId) {
  this->deviceId = deviceId;
  this->groupId = groupId;
  reset();
}

void PacketFormatter::format(uint8_t const* packet, char* buffer) {
  for (int i = 0; i < packetLength; i++) {
    sprintf(buffer, "%02X ", packet[i]);
    buffer += 3;
  }
  sprintf(buffer, "\n\n");
}

void PacketFormatter::formatV1Packet(uint8_t const* packet, char* buffer) {
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
}