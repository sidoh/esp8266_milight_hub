#include <RgbPacketFormatter.h>
#include <Units.h>

void RgbPacketFormatter::initializePacket(uint8_t *packet) {
  size_t packetPtr = 0;

  packet[packetPtr++] = 0xA4;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum++;
}

void RgbPacketFormatter::pair() {
  for (size_t i = 0; i < 5; i++) {
    command(RGB_SPEED_UP, 0);
  }
}

void RgbPacketFormatter::unpair() {
  for (size_t i = 0; i < 5; i++) {
    command(RGB_SPEED_UP | 0x10, 0);
  }
}

void RgbPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(status == ON ? RGB_ON : RGB_OFF, 0);
}

void RgbPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x80;
  }
  currentPacket[RGB_COMMAND_INDEX] = command;
}

void RgbPacketFormatter::updateHue(uint16_t value) {
  const int16_t remappedColor = (value + 40) % 360;
  updateColorRaw(Units::rescale(remappedColor, 255, 360));
}

void RgbPacketFormatter::updateColorRaw(uint8_t value) {
  command(0, 0);
  currentPacket[RGB_COLOR_INDEX] = value;
}

void RgbPacketFormatter::updateBrightness(uint8_t value) {
  const GroupState* state = this->stateStore->get(deviceId, groupId, MiLightRemoteType::REMOTE_TYPE_RGB);
  int8_t knownValue = (state != NULL && state->isSetBrightness()) ? state->getBrightness() : -1;

  valueByStepFunction(
    &PacketFormatter::increaseBrightness,
    &PacketFormatter::decreaseBrightness,
    RGB_INTERVALS,
    value / RGB_INTERVALS,
    knownValue / RGB_INTERVALS
  );
}

void RgbPacketFormatter::increaseBrightness() {
  command(RGB_BRIGHTNESS_UP, 0);
}

void RgbPacketFormatter::decreaseBrightness() {
  command(RGB_BRIGHTNESS_DOWN, 0);
}

void RgbPacketFormatter::modeSpeedDown() {
  command(RGB_SPEED_DOWN, 0);
}

void RgbPacketFormatter::modeSpeedUp() {
  command(RGB_SPEED_UP, 0);
}

void RgbPacketFormatter::nextMode() {
  command(RGB_MODE_UP, 0);
}

void RgbPacketFormatter::previousMode() {
  command(RGB_MODE_DOWN, 0);
}

BulbId RgbPacketFormatter::parsePacket(const uint8_t* packet, JsonObject& result) {
  uint8_t command = packet[RGB_COMMAND_INDEX] & 0x7F;

  BulbId bulbId(
    (packet[1] << 8) | packet[2],
    0,
    REMOTE_TYPE_RGB
  );

  if (command == RGB_ON) {
    result["state"] = "ON";
  } else if (command == RGB_OFF) {
    result["state"] = "OFF";
  } else if (command == 0) {
    uint16_t remappedColor = Units::rescale<uint16_t, uint16_t>(packet[RGB_COLOR_INDEX], 360.0, 255.0);
    remappedColor = (remappedColor + 320) % 360;
    result["hue"] = remappedColor;
  } else if (command == RGB_MODE_DOWN) {
    result["command"] = "previous_mode";
  } else if (command == RGB_MODE_UP) {
    result["command"] = "next_mode";
  } else if (command == RGB_SPEED_DOWN) {
    result["command"] = "mode_speed_down";
  } else if (command == RGB_SPEED_UP) {
    result["command"] = "mode_speed_up";
  } else if (command == RGB_BRIGHTNESS_DOWN) {
    result["command"] = "brightness_down";
  } else if (command == RGB_BRIGHTNESS_UP) {
    result["command"] = "brightness_up";
  } else {
    result["button_id"] = command;
  }

  return bulbId;
}

void RgbPacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, "b0       : %02X\n", packet[0]);
  buffer += sprintf_P(buffer, "ID       : %02X%02X\n", packet[1], packet[2]);
  buffer += sprintf_P(buffer, "Color    : %02X\n", packet[3]);
  buffer += sprintf_P(buffer, "Command  : %02X\n", packet[4]);
  buffer += sprintf_P(buffer, "Sequence : %02X\n", packet[5]);
}
