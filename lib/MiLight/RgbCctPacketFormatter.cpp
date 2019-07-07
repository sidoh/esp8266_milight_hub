#include <RgbCctPacketFormatter.h>
#include <V2RFEncoding.h>
#include <Units.h>
#include <MiLightCommands.h>

void RgbCctPacketFormatter::modeSpeedDown() {
  command(RGB_CCT_ON, RGB_CCT_MODE_SPEED_DOWN);
}

void RgbCctPacketFormatter::modeSpeedUp() {
  command(RGB_CCT_ON, RGB_CCT_MODE_SPEED_UP);
}

void RgbCctPacketFormatter::updateMode(uint8_t mode) {
  lastMode = mode;
  command(RGB_CCT_MODE, mode);
}

void RgbCctPacketFormatter::nextMode() {
  updateMode((lastMode+1)%RGB_CCT_NUM_MODES);
}

void RgbCctPacketFormatter::previousMode() {
  updateMode((lastMode-1)%RGB_CCT_NUM_MODES);
}

void RgbCctPacketFormatter::updateBrightness(uint8_t brightness) {
  command(RGB_CCT_BRIGHTNESS, RGB_CCT_BRIGHTNESS_OFFSET + brightness);
}

// change the hue (which may also change to color mode).
void RgbCctPacketFormatter::updateHue(uint16_t value) {
  uint8_t remapped = Units::rescale(value, 255, 360);
  updateColorRaw(remapped);
}

void RgbCctPacketFormatter::updateColorRaw(uint8_t value) {
  command(RGB_CCT_COLOR, RGB_CCT_COLOR_OFFSET + value);
}

void RgbCctPacketFormatter::updateTemperature(uint8_t value) {
  // Packet scale is [0x94, 0x92, .. 0, .., 0xCE, 0xCC]. Increments of 2.
  // From coolest to warmest.
  uint8_t cmdValue = V2PacketFormatter::tov2scale(value, RGB_CCT_KELVIN_REMOTE_END, 2);

  // when updating temperature, the bulb switches to white.  If we are not already
  // in white mode, that makes changing temperature annoying because the current hue/mode
  // is lost.  So lookup our current bulb mode, and if needed, reset the hue/mode after
  // changing the temperature
  const GroupState* ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_RGB_CCT);

  // now make the temperature change
  command(RGB_CCT_KELVIN, cmdValue);

  // and return to our original mode
  if (ourState != NULL) {
    BulbMode originalBulbMode = ourState->getBulbMode();

    if ((settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_WHITE)) {
      switchMode(*ourState, originalBulbMode);
    }
  }
}

// update saturation.  This only works when in Color mode, so if not in color we switch to color,
// make the change, and switch back again.
void RgbCctPacketFormatter::updateSaturation(uint8_t value) {
   // look up our current mode
  const GroupState* ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_RGB_CCT);
  BulbMode originalBulbMode = BulbMode::BULB_MODE_WHITE;

  if (ourState != NULL) {
    originalBulbMode = ourState->getBulbMode();

    // are we already in white?  If not, change to white
    if ((settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_COLOR)) {
      updateHue(ourState->getHue());
    }
  }

  // now make the saturation change
  uint8_t remapped = value + RGB_CCT_SATURATION_OFFSET;
  command(RGB_CCT_SATURATION, remapped);

  if (ourState != NULL) {
    if ((settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_COLOR)) {
      switchMode(*ourState, originalBulbMode);
    }
  }
}

void RgbCctPacketFormatter::updateColorWhite() {
  // there is no direct white command, so let's look up our prior temperature and set that, which
  // causes the bulb to go white
  const GroupState* ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_RGB_CCT);
  uint8_t value =
    ourState == NULL
      ? 0
      : V2PacketFormatter::tov2scale(ourState->getKelvin(), RGB_CCT_KELVIN_REMOTE_END, 2);

  // issue command to set kelvin to prior value, which will drive to white
  command(RGB_CCT_KELVIN, value);
}

void RgbCctPacketFormatter::enableNightMode() {
  uint8_t arg = groupCommandArg(OFF, groupId);
  command(RGB_CCT_ON | 0x80, arg);
}

BulbId RgbCctPacketFormatter::parsePacket(const uint8_t *packet, JsonObject result) {
  uint8_t packetCopy[V2_PACKET_LEN];
  memcpy(packetCopy, packet, V2_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);

  BulbId bulbId(
    (packetCopy[2] << 8) | packetCopy[3],
    packetCopy[7],
    REMOTE_TYPE_RGB_CCT
  );

  uint8_t command = (packetCopy[V2_COMMAND_INDEX] & 0x7F);
  uint8_t arg = packetCopy[V2_ARGUMENT_INDEX];

  if (command == RGB_CCT_ON) {
    if ((packetCopy[V2_COMMAND_INDEX] & 0x80) == 0x80) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::NIGHT_MODE;
    } else if (arg == RGB_CCT_MODE_SPEED_DOWN) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::MODE_SPEED_DOWN;
    } else if (arg == RGB_CCT_MODE_SPEED_UP) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::MODE_SPEED_UP;
    } else if (arg < 5) { // Group is not reliably encoded in group byte. Extract from arg byte
      result[GroupStateFieldNames::STATE] = "ON";
      bulbId.groupId = arg;
    } else {
      result[GroupStateFieldNames::STATE] = "OFF";
      bulbId.groupId = arg-5;
    }
  } else if (command == RGB_CCT_COLOR) {
    uint8_t rescaledColor = (arg - RGB_CCT_COLOR_OFFSET) % 0x100;
    uint16_t hue = Units::rescale<uint16_t, uint16_t>(rescaledColor, 360, 255.0);
    result[GroupStateFieldNames::HUE] = hue;
  } else if (command == RGB_CCT_KELVIN) {
    uint8_t temperature = V2PacketFormatter::fromv2scale(arg, RGB_CCT_KELVIN_REMOTE_END, 2);
    result[GroupStateFieldNames::COLOR_TEMP] = Units::whiteValToMireds(temperature, 100);
  // brightness == saturation
  } else if (command == RGB_CCT_BRIGHTNESS && arg >= (RGB_CCT_BRIGHTNESS_OFFSET - 15)) {
    uint8_t level = constrain(arg - RGB_CCT_BRIGHTNESS_OFFSET, 0, 100);
    result[GroupStateFieldNames::BRIGHTNESS] = Units::rescale<uint8_t, uint8_t>(level, 255, 100);
  } else if (command == RGB_CCT_SATURATION) {
    result[GroupStateFieldNames::SATURATION] = constrain(arg - RGB_CCT_SATURATION_OFFSET, 0, 100);
  } else if (command == RGB_CCT_MODE) {
    result[GroupStateFieldNames::MODE] = arg;
  } else {
    result["button_id"] = command;
    result["argument"] = arg;
  }

  return bulbId;
}
