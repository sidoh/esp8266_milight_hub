#include <RgbwPacketFormatter.h>
#include <Units.h>
#include <MiLightCommands.h>

#define STATUS_COMMAND(status, groupId) ( RGBW_GROUP_1_ON + (((groupId) - 1)*2) + (status) )
#define GROUP_FOR_STATUS_COMMAND(buttonId) ( ((buttonId) - 1) / 2 )
#define STATUS_FOR_COMMAND(buttonId) ( ((buttonId) % 2) == 0 ? OFF : ON )

bool RgbwPacketFormatter::canHandle(const uint8_t *packet, const size_t len) {
  return len == packetLength && (packet[0] & 0xF0) == RGBW_PROTOCOL_ID_BYTE;
}

void RgbwPacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  packet[packetPtr++] = RGBW_PROTOCOL_ID_BYTE;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = (groupId & 0x07);
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum++;
}

void RgbwPacketFormatter::unpair() {
  PacketFormatter::updateStatus(ON);
  updateColorWhite();
}

void RgbwPacketFormatter::modeSpeedDown() {
  command(RGBW_SPEED_DOWN, 0);
}

void RgbwPacketFormatter::modeSpeedUp() {
  command(RGBW_SPEED_UP, 0);
}

void RgbwPacketFormatter::nextMode() {
  updateMode((currentMode() + 1) % RGBW_NUM_MODES);
}

void RgbwPacketFormatter::previousMode() {
  updateMode((currentMode() + RGBW_NUM_MODES - 1) % RGBW_NUM_MODES);
}

uint8_t RgbwPacketFormatter::currentMode() {
  const GroupState* state = stateStore->get(deviceId, groupId, REMOTE_TYPE_RGBW);
  return state != NULL ? state->getMode() : 0;
}

void RgbwPacketFormatter::updateMode(uint8_t mode) {
  command(RGBW_DISCO_MODE, 0);
  currentPacket[0] = RGBW_PROTOCOL_ID_BYTE | mode;
}

void RgbwPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(STATUS_COMMAND(status, groupId), 0);
}

void RgbwPacketFormatter::updateBrightness(uint8_t value) {
  // Expect an input value in [0, 100]. Map it down to [0, 25].
  const uint8_t adjustedBrightness = Units::rescale(value, 25, 100);

  // The actual protocol uses a bizarre range where min is 16, max is 23:
  // [16, 15, ..., 0, 31, ..., 23]
  const uint8_t packetBrightnessValue = (
    ((31 - adjustedBrightness) + 17) % 32
  );

  command(RGBW_BRIGHTNESS, 0);
  currentPacket[RGBW_BRIGHTNESS_GROUP_INDEX] |= (packetBrightnessValue << 3);
}

void RgbwPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x80;
  }
  currentPacket[RGBW_COMMAND_INDEX] = command;
}

void RgbwPacketFormatter::updateHue(uint16_t value) {
  const int16_t remappedColor = (value + 40) % 360;
  updateColorRaw(Units::rescale(remappedColor, 255, 360));
}

void RgbwPacketFormatter::updateColorRaw(uint8_t value) {
  command(RGBW_COLOR, 0);
  currentPacket[RGBW_COLOR_INDEX] = value;
}

void RgbwPacketFormatter::updateColorWhite() {
  uint8_t button = RGBW_GROUP_1_MAX_LEVEL + ((groupId - 1)*2);
  command(button, 0);
}

void RgbwPacketFormatter::enableNightMode() {
  uint8_t button = STATUS_COMMAND(OFF, groupId);

  // Bulbs must be OFF for night mode to work in RGBW.
  // Turn it off if it isn't already off.
  const GroupState* state = stateStore->get(deviceId, groupId, REMOTE_TYPE_RGBW);
  if (state == NULL || state->getState() == MiLightStatus::ON) {
    command(button, 0);
  }

  // Night mode command has 0x10 bit set, but is otherwise
  // a repeat of the OFF command.
  command(button | 0x10, 0);
}

BulbId RgbwPacketFormatter::parsePacket(const uint8_t* packet, JsonObject result) {
  uint8_t command = packet[RGBW_COMMAND_INDEX] & 0x7F;

  BulbId bulbId(
    (packet[1] << 8) | packet[2],
    packet[RGBW_BRIGHTNESS_GROUP_INDEX] & 0x7,
    REMOTE_TYPE_RGBW
  );

  if (command >= RGBW_ALL_ON && command <= RGBW_GROUP_4_OFF) {
    result[GroupStateFieldNames::STATE] = (STATUS_FOR_COMMAND(command) == ON) ? "ON" : "OFF";

    // Determine group ID from button ID for on/off. The remote's state is from
    // the last packet sent, not the current one, and that can be wrong for
    // on/off commands.
    bulbId.groupId = GROUP_FOR_STATUS_COMMAND(command);
  } else if (command & 0x10) {
    if ((command % 2) == 0) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::NIGHT_MODE;
    } else {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::SET_WHITE;
    }
    bulbId.groupId = GROUP_FOR_STATUS_COMMAND(command & 0xF);
  } else if (command == RGBW_BRIGHTNESS) {
    uint8_t brightness = 31;
    brightness -= packet[RGBW_BRIGHTNESS_GROUP_INDEX] >> 3;
    brightness += 17;
    brightness %= 32;
    result[GroupStateFieldNames::BRIGHTNESS] = Units::rescale<uint8_t, uint8_t>(brightness, 255, 25);
  } else if (command == RGBW_COLOR) {
    uint16_t remappedColor = Units::rescale<uint16_t, uint16_t>(packet[RGBW_COLOR_INDEX], 360.0, 255.0);
    remappedColor = (remappedColor + 320) % 360;
    result[GroupStateFieldNames::HUE] = remappedColor;
  } else if (command == RGBW_SPEED_DOWN) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::MODE_SPEED_DOWN;
  } else if (command == RGBW_SPEED_UP) {
    result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::MODE_SPEED_UP;
  } else if (command == RGBW_DISCO_MODE) {
    result[GroupStateFieldNames::MODE] = packet[0] & ~RGBW_PROTOCOL_ID_BYTE;
  } else {
    result["button_id"] = command;
  }

  return bulbId;
}

void RgbwPacketFormatter::format(uint8_t const* packet, char* buffer) {
  PacketFormatter::formatV1Packet(packet, buffer);
}
