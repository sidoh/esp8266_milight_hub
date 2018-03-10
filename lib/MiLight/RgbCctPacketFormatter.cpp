#include <RgbCctPacketFormatter.h>
#include <V2RFEncoding.h>
#include <Units.h>

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
  // To convert from [0, 100] scale:
  //   * Multiply by 2
  //   * Reverse direction (increasing values should be cool -> warm)
  //   * Start scale at 0xCC
  uint8_t cmdValue = ((100 - value) * 2) + RGB_CCT_KELVIN_REMOTE_END;

  // when updating temperature, the bulb switches to white.  If we are not already
  // in white mode, that makes changing temperature annoying because the current hue/mode
  // is lost.  So lookup our current bulb mode, and if needed, reset the hue/mode after
  // changing the temperature
  GroupState ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_RGB_CCT);
  BulbMode originalBulbMode = ourState.getBulbMode();

  // now make the temperature change
  command(RGB_CCT_KELVIN, cmdValue);

  // and return to our original mode
  if ((settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_WHITE)) {
    switchMode(ourState, originalBulbMode);
  }
}

// update saturation.  This only works when in Color mode, so if not in color we switch to color,
// make the change, and switch back again.
void RgbCctPacketFormatter::updateSaturation(uint8_t value) {
   // look up our current mode 
  GroupState ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_RGB_CCT);
  BulbMode originalBulbMode = ourState.getBulbMode();

  // are we already in white?  If not, change to white
  if ((settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_COLOR)) {
    updateHue(ourState.getHue());
  }

  // now make the saturation change
  uint8_t remapped = value + RGB_CCT_SATURATION_OFFSET;
  command(RGB_CCT_SATURATION, remapped);

  if ((settings->enableAutomaticModeSwitching) && (originalBulbMode != BulbMode::BULB_MODE_COLOR)) {
    switchMode(ourState, originalBulbMode);
  }
}

void RgbCctPacketFormatter::updateColorWhite() {
  // there is no direct white command, so let's look up our prior temperature and set that, which
  // causes the bulb to go white 
  GroupState ourState = this->stateStore->get(this->deviceId, this->groupId, REMOTE_TYPE_RGB_CCT);
  uint8_t value = ((100 - ourState.getKelvin()) * 2) + RGB_CCT_KELVIN_REMOTE_END;

  // issue command to set kelvin to prior value, which will drive to white
  command(RGB_CCT_KELVIN, value);
}

void RgbCctPacketFormatter::enableNightMode() {
  uint8_t arg = groupCommandArg(OFF, groupId);
  command(RGB_CCT_ON | 0x80, arg);
}

BulbId RgbCctPacketFormatter::parsePacket(const uint8_t *packet, JsonObject& result) {
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
      result["command"] = "night_mode";
    } else if (arg == RGB_CCT_MODE_SPEED_DOWN) {
      result["command"] = "mode_speed_down";
    } else if (arg == RGB_CCT_MODE_SPEED_UP) {
      result["command"] = "mode_speed_up";
    } else if (arg < 5) { // Group is not reliably encoded in group byte. Extract from arg byte
      result["state"] = "ON";
      bulbId.groupId = arg;
    } else {
      result["state"] = "OFF";
      bulbId.groupId = arg-5;
    }
  } else if (command == RGB_CCT_COLOR) {
    uint8_t rescaledColor = (arg - RGB_CCT_COLOR_OFFSET) % 0x100;
    uint16_t hue = Units::rescale<uint16_t, uint16_t>(rescaledColor, 360, 255.0);
    result["hue"] = hue;
  } else if (command == RGB_CCT_KELVIN) {
    // Packet range is [0x94, 0x92, ..., 0xCC]. Remote sends values outside this
    // range, so normalize.
    uint8_t temperature = arg;
    if (arg < 0xCC && arg >= 0xB0) {
      temperature = 0xCC;
    } else if (arg > 0x94 && arg <= 0xAF) {
      temperature = 0x94;
    }

    temperature = (temperature + (0x100 - RGB_CCT_KELVIN_REMOTE_END)) % 0x100;
    temperature /= 2;
    temperature = (100 - temperature);
    temperature = constrain(temperature, 0, 100);

    result["color_temp"] = Units::whiteValToMireds(temperature, 100);
  // brightness == saturation
  } else if (command == RGB_CCT_BRIGHTNESS && arg >= (RGB_CCT_BRIGHTNESS_OFFSET - 15)) {
    uint8_t level = constrain(arg - RGB_CCT_BRIGHTNESS_OFFSET, 0, 100);
    result["brightness"] = Units::rescale<uint8_t, uint8_t>(level, 255, 100);
  } else if (command == RGB_CCT_SATURATION) {
    result["saturation"] = constrain(arg - RGB_CCT_SATURATION_OFFSET, 0, 100);
  } else if (command == RGB_CCT_MODE) {
    result["mode"] = arg;
  } else {
    result["button_id"] = command;
    result["argument"] = arg;
  }

  return bulbId;
}
