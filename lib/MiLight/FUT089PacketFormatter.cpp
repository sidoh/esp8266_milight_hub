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
  updateColorWhite();
  command(FUT089_KELVIN, 100 - value);
}

void FUT089PacketFormatter::updateSaturation(uint8_t value) {
  command(FUT089_SATURATION, 100 - value);
}

void FUT089PacketFormatter::updateColorWhite() {
  command(FUT089_ON, FUT089_WHITE_MODE);
}

void FUT089PacketFormatter::enableNightMode() {
  uint8_t arg = groupCommandArg(OFF, groupId);
  command(FUT089_ON | 0x80, arg);
}

void FUT089PacketFormatter::parsePacket(const uint8_t *packet, JsonObject& result, GroupStateStore* stateStore) {
  uint8_t packetCopy[V2_PACKET_LEN];
  memcpy(packetCopy, packet, V2_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);

  const uint16_t deviceId = (packetCopy[2] << 8) | packetCopy[3];
  const uint8_t groupId = packetCopy[7];

  result["device_id"] = deviceId;
  result["group_id"] = groupId;
  result["device_type"] = "fut089";

  uint8_t command = (packetCopy[V2_COMMAND_INDEX] & 0x7F);
  uint8_t arg = packetCopy[V2_ARGUMENT_INDEX];

  // only need state for saturation and kelvin (they have the same command ID)
  GroupState* state = NULL;
  if (command == FUT089_SATURATION) {
    GroupId group(deviceId, groupId, REMOTE_TYPE_FUT089);
    state = stateStore->get(group);
  }

  if (command == FUT089_ON) {
    if (arg == FUT089_MODE_SPEED_DOWN) {
      result["command"] = "mode_speed_down";
    } else if (arg == FUT089_MODE_SPEED_UP) {
      result["command"] = "mode_speed_up";
    } else if (arg == FUT089_WHITE_MODE) {
      result["command"] = "white_mode";
    } else if (arg <= 8) { // Group is not reliably encoded in group byte. Extract from arg byte
      result["state"] = "ON";
      result["group_id"] = arg;
    } else if (arg >= 9 && arg <= 17) {
      result["state"] = "OFF";
      result["group_id"] = arg-9;
    }
  } else if (command == FUT089_COLOR) {
    uint8_t rescaledColor = (arg - FUT089_COLOR_OFFSET) % 0x100;
    uint16_t hue = Units::rescale<uint16_t, uint16_t>(rescaledColor, 360, 255.0);
    result["hue"] = hue;
  } else if (command == FUT089_BRIGHTNESS) {
    uint8_t level = constrain(arg, 0, 100);
    result["brightness"] = Units::rescale<uint8_t, uint8_t>(level, 255, 100);
  // saturation == kelvin. arg ranges are the same, so can't distinguish
  // without using state
  } else if (command == FUT089_SATURATION) {
    if (state->getBulbMode() == BULB_MODE_COLOR) {
      result["saturation"] = 100 - constrain(arg, 0, 100);
    } else {
      result["color_temp"] = Units::whiteValToMireds(100 - arg, 100);
    }
  } else if (command == FUT089_MODE) {
    result["mode"] = arg;
  } else {
    result["button_id"] = command;
    result["argument"] = arg;
  }
}
