#include <FUT089PacketFormatter.h>
#include <V2RFEncoding.h>
#include <Units.h>

void FUT089PacketFormatter::modeSpeedDown() {
  command(FUT089_ON, FUT089_MODE_SPEED_DOWN);
}

void FUT089PacketFormatter::modeSpeedUp() {
  command(FUT089_ON, FUT089_MODE_SPEED_UP);
}

void FUT089PacketFormatter::updateMode(uint8_t mode) {
  command(FUT089_MODE, mode);
}

void FUT089PacketFormatter::updateBrightness(uint8_t brightness) {
  command(FUT089_BRIGHTNESS, brightness);
}

void FUT089PacketFormatter::updateHue(uint16_t value) {
  uint8_t remapped = Units::rescale(value, 255, 360);
  updateColorRaw(remapped);
}

void FUT089PacketFormatter::updateColorRaw(uint8_t value) {
  command(FUT089_COLOR, FUT089_COLOR_OFFSET + value);
}

void FUT089PacketFormatter::updateTemperature(uint8_t value) {
  command(FUT089_KELVIN, value);
}

void FUT089PacketFormatter::updateSaturation(uint8_t value) {
  command(FUT089_SATURATION, value);
}

void FUT089PacketFormatter::updateColorWhite() {
  command(FUT089_ON, FUT089_WHITE_MODE);
}

void FUT089PacketFormatter::enableNightMode() {
  uint8_t arg = groupCommandArg(OFF, groupId);
  command(FUT089_ON | 0x80, arg);
}

void FUT089PacketFormatter::parsePacket(const uint8_t *packet, JsonObject& result) {
  uint8_t packetCopy[V2_PACKET_LEN];
  memcpy(packetCopy, packet, V2_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);

  result["device_id"] = (packetCopy[2] << 8) | packetCopy[3];
  result["group_id"] = packetCopy[7];
  result["device_type"] = "rgb_cct";

  if (! result.containsKey("state")) {
    result["state"] = "ON";
  }
}
