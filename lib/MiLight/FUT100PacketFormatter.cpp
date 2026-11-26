#include <FUT100PacketFormatter.h>
#include <V2RFEncoding.h>
#include <Units.h>
#include <MiLightCommands.h>

void FUT100PacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  this->groupId = groupId;

  if (groupId <= 8) {
    uint8_t arg = groupId + (status == OFF ? 9 : 0);
    command(FUT100_ON_G_LOW, arg);
  } else {
    uint8_t arg = ((groupId - 9) * 2) + (status == ON ? 0 : 1);
    command(FUT100_ON_G_HIGH, arg);
  }
}

void FUT100PacketFormatter::modeSpeedDown() {
  command(FUT100_ON_G_LOW, FUT100_MODE_SPEED_DOWN);
}

void FUT100PacketFormatter::modeSpeedUp() {
  command(FUT100_ON_G_LOW, FUT100_MODE_SPEED_UP);
}

void FUT100PacketFormatter::updateMode(uint8_t mode) {
  command(FUT100_MODE, mode);
}

void FUT100PacketFormatter::updateBrightness(uint8_t brightness) {
  command(FUT100_BRIGHTNESS, brightness);
}

void FUT100PacketFormatter::updateHue(uint16_t value) {
  uint8_t remapped = Units::rescale(value, 255, 360);
  updateColorRaw(remapped);
}

void FUT100PacketFormatter::updateColorRaw(uint8_t value) {
  command(FUT100_COLOR, value);
}

void FUT100PacketFormatter::updateTemperature(uint8_t value) {
  const GroupState* ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_FUT100);
  BulbMode originalBulbMode = BulbMode::BULB_MODE_WHITE;

  if (ourState != NULL) {
    originalBulbMode = ourState->getBulbMode();
    if (originalBulbMode != BulbMode::BULB_MODE_WHITE) {
      updateColorWhite();
    }
  }

  command(FUT100_KELVIN, 100 - value);

  if (ourState != NULL && (settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_WHITE)) {
    switchMode(*ourState, originalBulbMode);
  }
}

void FUT100PacketFormatter::updateSaturation(uint8_t value) {
  const GroupState* ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_FUT100);
  BulbMode originalBulbMode = BulbMode::BULB_MODE_WHITE;

  if (ourState != NULL) {
    originalBulbMode = ourState->getBulbMode();
  }

  if (ourState != NULL && (settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_COLOR)) {
    updateHue(ourState->getHue());
  }

  command(FUT100_SATURATION, 100 - value);

  if (ourState != NULL && (settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_COLOR)) {
    switchMode(*ourState, originalBulbMode);
  }
}

void FUT100PacketFormatter::updateColorWhite() {
  command(FUT100_ON_G_LOW, FUT100_WHITE_MODE);
}

void FUT100PacketFormatter::enableNightMode() {
  if (groupId <= 8) {
    uint8_t arg = groupId + 9; 
    command(FUT100_ON_G_LOW | 0x80, arg);
  } else {
    uint8_t arg = ((groupId - 9) * 2) + 1;
    command(FUT100_ON_G_HIGH | 0x80, arg);
  }
}

BulbId FUT100PacketFormatter::parsePacket(const uint8_t *packet, JsonObject result) {
  if (stateStore == NULL) {
    Serial.println(F("ERROR: stateStore not set. Prepare was not called! **THIS IS A BUG**"));
    BulbId fakeId(0, 0, REMOTE_TYPE_FUT100);
    return fakeId;
  }

  uint8_t packetCopy[V2_PACKET_LEN];
  memcpy(packetCopy, packet, V2_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);

  BulbId bulbId(
    (packetCopy[2] << 8) | packetCopy[3],
    packetCopy[7],
    REMOTE_TYPE_FUT100
  );

  uint8_t rawCmd = packetCopy[V2_COMMAND_INDEX];
  uint8_t cmd = (rawCmd & 0x7F); 
  uint8_t arg = packetCopy[V2_ARGUMENT_INDEX];
  bool isNightMode = (rawCmd & 0x80) == 0x80;

  if (cmd == FUT100_ON_G_LOW) {
    if (isNightMode) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::NIGHT_MODE;
      if (arg >= 9 && arg <= 17) bulbId.groupId = arg - 9;
    } else if (arg == FUT100_MODE_SPEED_DOWN) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::MODE_SPEED_DOWN;
    } else if (arg == FUT100_MODE_SPEED_UP) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::MODE_SPEED_UP;
    } else if (arg == FUT100_WHITE_MODE) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::SET_WHITE;
    } else if (arg <= 8) { 
      result[GroupStateFieldNames::STATE] = "ON";
      bulbId.groupId = arg;
    } else if (arg >= 9 && arg <= 17) {
      result[GroupStateFieldNames::STATE] = "OFF";
      bulbId.groupId = arg - 9;
    }
  } else if (cmd == FUT100_ON_G_HIGH) {
    bulbId.groupId = (arg / 2) + 9;
    if (isNightMode) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::NIGHT_MODE;
    } else {
      result[GroupStateFieldNames::STATE] = ((arg % 2) == 0) ? "ON" : "OFF";
    }
  } else if (cmd == FUT100_COLOR) {
    result[GroupStateFieldNames::HUE] = Units::rescale<uint16_t, uint16_t>(arg, 360, 255.0);
  } else if (cmd == FUT100_BRIGHTNESS) {
    result[GroupStateFieldNames::BRIGHTNESS] = Units::rescale<uint8_t, uint8_t>(constrain(arg, 0, 100), 255, 100);
  } else if (cmd == FUT100_KELVIN) { 
    const GroupState* state = stateStore->get(bulbId);
    if (state != NULL && state->getBulbMode() == BULB_MODE_COLOR) {
      result[GroupStateFieldNames::SATURATION] = 100 - constrain(arg, 0, 100);
    } else {
      result[GroupStateFieldNames::COLOR_TEMP] = Units::whiteValToMireds(100 - arg, 100);
    }
  } else if (cmd == FUT100_MODE) {
    result[GroupStateFieldNames::MODE] = arg;
  } else {
    result["button_id"] = cmd;
    result["argument"] = arg;
  }

  return bulbId;
}
