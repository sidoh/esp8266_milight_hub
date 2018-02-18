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

// change the hue (which may also change to color mode).  Also change saturation
// if this was pending.
void FUT089PacketFormatter::updateHue(uint16_t value) {
  uint8_t remapped = Units::rescale(value, 255, 360);
  updateColorRaw(remapped);

  // look up our current mode 
  GroupState ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_FUT089);

  // do we have a saturation pending?
  if (ourState.isPendingSaturation()) {
    // now make the saturation change
    command(FUT089_SATURATION, 100 - ourState.getSaturation());
    ourState.setPendingSaturation(false);

    // clear pending status
    ourState.setPendingSaturation(false);
    this->stateStore->set(this->deviceId, this->groupId, REMOTE_TYPE_FUT089, ourState);
  }
}

void FUT089PacketFormatter::updateColorRaw(uint8_t value) {
  command(FUT089_COLOR, FUT089_COLOR_OFFSET + value);
}

// change the temperature (kelvin).  Note that temperature and saturation share the same command 
// number (7), and they change which they do based on the mode of the lamp (white vs. color mode).
// To make this command work, we need to switch to white mode, make the change, and then flip
// back to the original mode.
void FUT089PacketFormatter::updateTemperature(uint8_t value) {
  // look up our current mode 
  GroupState ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_FUT089);
  BulbMode originalBulbMode = ourState.getBulbMode();

  // are we already in white?  If not, change to white
  if (originalBulbMode != BulbMode::BULB_MODE_WHITE) {
    updateColorWhite();
  }

  // now make the temperature change
  command(FUT089_KELVIN, 100 - value);

  // and return to our original mode
  if (originalBulbMode != BulbMode::BULB_MODE_WHITE) {
    switchMode(ourState, originalBulbMode);
  }
}

// change the saturation.  Note that temperature and saturation share the same command 
// number (7), and they change which they do based on the mode of the lamp (white vs. color mode).
// Therefore, if we are not in color mode, we save the saturation and wait until we do a
// mode change to color (hue) and then change the saturation.
void FUT089PacketFormatter::updateSaturation(uint8_t value) {
  // look up our current mode 
  GroupState ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_FUT089);
  BulbMode originalBulbMode = ourState.getBulbMode();

  // are we already in color?  If not, we can't make the change yet, so store the value
  // so we can make it later
  if (originalBulbMode != BulbMode::BULB_MODE_COLOR) {
    ourState.setPendingSaturation(true);
    ourState.setSaturation(value);
    this->stateStore->set(this->deviceId, this->groupId, REMOTE_TYPE_FUT089, ourState);
    return;
  }
  // now make the saturation change
  command(FUT089_SATURATION, 100 - value);
}

void FUT089PacketFormatter::updateColorWhite() {
  command(FUT089_ON, FUT089_WHITE_MODE);
}

void FUT089PacketFormatter::enableNightMode() {
  uint8_t arg = groupCommandArg(OFF, groupId);
  command(FUT089_ON | 0x80, arg);
}

BulbId FUT089PacketFormatter::parsePacket(const uint8_t *packet, JsonObject& result, GroupStateStore* stateStore) {
  uint8_t packetCopy[V2_PACKET_LEN];
  memcpy(packetCopy, packet, V2_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);

  BulbId bulbId(
    (packetCopy[2] << 8) | packetCopy[3],
    packetCopy[7],
    REMOTE_TYPE_FUT089
  );

  uint8_t command = (packetCopy[V2_COMMAND_INDEX] & 0x7F);
  uint8_t arg = packetCopy[V2_ARGUMENT_INDEX];

  if (command == FUT089_ON) {
    if ((packetCopy[V2_COMMAND_INDEX] & 0x80) == 0x80) {
      result["command"] = "night_mode";
    } else if (arg == FUT089_MODE_SPEED_DOWN) {
      result["command"] = "mode_speed_down";
    } else if (arg == FUT089_MODE_SPEED_UP) {
      result["command"] = "mode_speed_up";
    } else if (arg == FUT089_WHITE_MODE) {
      result["command"] = "white_mode";
    } else if (arg <= 8) { // Group is not reliably encoded in group byte. Extract from arg byte
      result["state"] = "ON";
      bulbId.groupId = arg;
    } else if (arg >= 9 && arg <= 17) {
      result["state"] = "OFF";
      bulbId.groupId = arg-9;
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
    GroupState& state = stateStore->get(bulbId);

    if (state.getBulbMode() == BULB_MODE_COLOR) {
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

  return bulbId;
}
