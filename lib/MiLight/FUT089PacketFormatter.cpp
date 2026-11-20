#include <FUT089PacketFormatter.h>
#include <V2RFEncoding.h>
#include <Units.h>
#include <MiLightCommands.h>

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

// change the hue (which may also change to color mode).
void FUT089PacketFormatter::updateHue(uint16_t value) {
  uint8_t remapped = Units::rescale(value, 255, 360);
  updateColorRaw(remapped);
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
  const GroupState* ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_FUT089);
  BulbMode originalBulbMode;

  if (ourState != NULL) {
    originalBulbMode = ourState->getBulbMode();

    // are we already in white?  If not, change to white
    if (originalBulbMode != BulbMode::BULB_MODE_WHITE) {
      updateColorWhite();
    }
  }

  // now make the temperature change
  command(FUT089_KELVIN, 100 - value);

  // and return to our original mode
  if (ourState != NULL && (settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_WHITE)) {
    switchMode(*ourState, originalBulbMode);
  }
}

// change the saturation.  Note that temperature and saturation share the same command
// number (7), and they change which they do based on the mode of the lamp (white vs. color mode).
// Therefore, if we are not in color mode, we need to switch to color mode, make the change,
// and switch back to the original mode.
void FUT089PacketFormatter::updateSaturation(uint8_t value) {
  // look up our current mode
  const GroupState* ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_FUT089);
  BulbMode originalBulbMode = BulbMode::BULB_MODE_WHITE;

  if (ourState != NULL) {
    originalBulbMode = ourState->getBulbMode();
  }

  // are we already in color?  If not, we need to flip modes
  if (ourState != NULL && (settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_COLOR)) {
    updateHue(ourState->getHue());
  }

  // now make the saturation change
  command(FUT089_SATURATION, 100 - value);

  // and revert back if necessary
  if (ourState != NULL && (settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_COLOR)) {
    switchMode(*ourState, originalBulbMode);
  }
}

void FUT089PacketFormatter::updateColorWhite() {
  command(FUT089_ON, FUT089_WHITE_MODE);
}

void FUT089PacketFormatter::enableNightMode() {
  uint8_t arg = groupCommandArg(OFF, groupId);
  command(FUT089_ON | 0x80, arg);
}

BulbId FUT089PacketFormatter::parsePacket(const uint8_t *packet, JsonObject result) {
  if (stateStore == NULL) {
    BulbId fakeId(0, 0, REMOTE_TYPE_FUT089);
    return fakeId;
  }

  uint8_t packetCopy[V2_PACKET_LEN];
  memcpy(packetCopy, packet, V2_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);

  struct DevZoneTrack { uint16_t devId; uint8_t group; uint8_t lastOnArg; uint32_t lastOnMs; uint32_t lastOffMs; };
  static DevZoneTrack s_zones[8] = {};

  uint16_t parsedDevId = ((uint16_t)packetCopy[2] << 8) | packetCopy[3];
  uint8_t rawKey = packet[0];
  uint8_t rawGroup = packetCopy[7];
  uint8_t rawCmdByte = packetCopy[V2_COMMAND_INDEX];
  uint8_t command = (rawCmdByte & 0x7F);
  bool isHold = (rawCmdByte & 0x80) == 0x80;
  uint8_t arg = packetCopy[V2_ARGUMENT_INDEX];
  uint32_t nowMs = millis();

  int slot = -1, emptySlot = -1;
  for (int i = 0; i < 8; i++) {
    if (s_zones[i].devId == parsedDevId) { slot = i; break; }
    if (emptySlot < 0 && s_zones[i].devId == 0) emptySlot = i;
  }
  if (slot < 0) {
    slot = (emptySlot >= 0) ? emptySlot : 0;
    s_zones[slot] = { parsedDevId, 1, 0xFF, 0, 0 };
  }

  // CRITICAL: Physical B8/FUT089 remotes put rolling data in packetCopy[7] (rawKey != 0x00).
  // Only Hub-generated packets (rawKey == 0x00) carry an explicit zone in packetCopy[7]!
  if (rawGroup <= 8) {
    s_zones[slot].group = rawGroup;
  }
  uint8_t resolvedGroup = (rawGroup <= 8) ? rawGroup : s_zones[slot].group;
  BulbId bulbId(parsedDevId, resolvedGroup, REMOTE_TYPE_FUT089);

  if (command == FUT089_ON) {
    if (arg == FUT089_MODE_SPEED_DOWN) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::MODE_SPEED_DOWN;
    } else if (arg == FUT089_MODE_SPEED_UP) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::MODE_SPEED_UP;
    } else if (arg == FUT089_WHITE_MODE) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::SET_WHITE;
    } else if (arg <= 8) {
    bulbId.groupId = arg;
    s_zones[slot].group = arg;
    // Suppress all trailing cmd=0x81 capacitive hold frames on the SAME zone while finger remains down!
    if (isHold && s_zones[slot].lastOnArg == arg && (nowMs - s_zones[slot].lastOnMs < 1600)) {
      s_zones[slot].lastOnMs = nowMs;
      return DEFAULT_BULB_ID;
    }
    s_zones[slot].lastOnArg = arg;
    s_zones[slot].lastOnMs = nowMs;
    result[GroupStateFieldNames::STATE] = "ON";
    } else if (arg >= 9 && arg <= 17) {
      bulbId.groupId = arg - 9;
      s_zones[slot].group = bulbId.groupId;
      if (isHold) {
        if (nowMs - s_zones[slot].lastOffMs < 550) {
          return DEFAULT_BULB_ID;
        }
        if (nowMs - s_zones[slot].lastOffMs < 2200) {
          result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::NIGHT_MODE;
        } else {
          s_zones[slot].lastOffMs = nowMs;
          result[GroupStateFieldNames::STATE] = "OFF";
        }
      } else {
        s_zones[slot].lastOffMs = nowMs;
        result[GroupStateFieldNames::STATE] = "OFF";
      }
    } else {
      result["button_id"] = rawCmdByte;
      result["argument"] = arg;
    }
  } else if (command == FUT089_COLOR) {
    uint8_t rescaledColor = (arg - FUT089_COLOR_OFFSET) % 0x100;
    uint16_t hue = Units::rescale<uint16_t, uint16_t>(rescaledColor, 360, 255.0);
    result[GroupStateFieldNames::HUE] = hue;
  } else if (command == FUT089_BRIGHTNESS) {
    uint8_t level = constrain(arg, 0, 100);
    result[GroupStateFieldNames::BRIGHTNESS] = Units::rescale<uint8_t, uint8_t>(level, 255, 100);
  } else if (command == FUT089_SATURATION) {
    const GroupState* state = stateStore->get(bulbId);
    if (state != NULL && state->getBulbMode() == BULB_MODE_COLOR) {
      result[GroupStateFieldNames::SATURATION] = 100 - constrain(arg, 0, 100);
    } else {
      result[GroupStateFieldNames::COLOR_TEMP] = Units::whiteValToMireds(100 - arg, 100);
    }
  } else if (command == FUT089_MODE) {
    result[GroupStateFieldNames::MODE] = arg;
  } else {
    result["button_id"] = rawCmdByte;
    result["argument"] = arg;
  }

  return bulbId;
}
