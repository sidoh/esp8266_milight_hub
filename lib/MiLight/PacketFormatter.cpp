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