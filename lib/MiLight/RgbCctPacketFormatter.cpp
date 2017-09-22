#include <RgbCctPacketFormatter.h>
#include <Units.h>
#include <V2RFEncoding.h>

#define GROUP_COMMAND_ARG(status, groupId) ( groupId + (status == OFF ? 5 : 0) )

void RgbCctPacketFormatter::initializePacket(uint8_t* packet) {
  size_t packetPtr = 0;

  // Always encode with 0x00 key. No utility in varying it.
  packet[packetPtr++] = 0x00;

  packet[packetPtr++] = 0x20;
  packet[packetPtr++] = deviceId >> 8;
  packet[packetPtr++] = deviceId & 0xFF;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = 0;
  packet[packetPtr++] = sequenceNum++;
  packet[packetPtr++] = groupId;
  packet[packetPtr++] = 0;
}

void RgbCctPacketFormatter::unpair() {
  for (size_t i = 0; i < 5; i++) {
    updateStatus(ON, 0);
  }
}

void RgbCctPacketFormatter::command(uint8_t command, uint8_t arg) {
  pushPacket();
  if (held) {
    command |= 0x80;
  }
  currentPacket[RGB_CCT_COMMAND_INDEX] = command;
  currentPacket[RGB_CCT_ARGUMENT_INDEX] = arg;
}

void RgbCctPacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(RGB_CCT_ON, GROUP_COMMAND_ARG(status, groupId));
}

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

void RgbCctPacketFormatter::updateHue(uint16_t value) {
  uint8_t remapped = Units::rescale(value, 255, 360);
  updateColorRaw(remapped);
}

void RgbCctPacketFormatter::updateColorRaw(uint8_t value) {
  command(RGB_CCT_COLOR, RGB_CCT_COLOR_OFFSET + value);
}

void RgbCctPacketFormatter::updateTemperature(uint8_t value) {
  command(RGB_CCT_KELVIN, RGB_CCT_KELVIN_OFFSET - (value*2));
}

void RgbCctPacketFormatter::updateSaturation(uint8_t value) {
  uint8_t remapped = value + RGB_CCT_SATURATION_OFFSET;
  command(RGB_CCT_SATURATION, remapped);
}

void RgbCctPacketFormatter::updateColorWhite() {
  updateTemperature(0);
}

void RgbCctPacketFormatter::enableNightMode() {
  uint8_t arg = GROUP_COMMAND_ARG(OFF, groupId);
  command(RGB_CCT_ON | 0x80, arg);
}

void RgbCctPacketFormatter::finalizePacket(uint8_t* packet) {
  V2RFEncoding::encodeV2Packet(packet);
}

void RgbCctPacketFormatter::parsePacket(const uint8_t *packet, JsonObject& result) {
  uint8_t packetCopy[RGB_CCT_PACKET_LEN];
  memcpy(packetCopy, packet, RGB_CCT_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);

  result["device_id"] = (packetCopy[2] << 8) | packetCopy[3];
  result["group_id"] = packetCopy[7];
  result["device_type"] = "rgb_cct";

  uint8_t command = (packetCopy[RGB_CCT_COMMAND_INDEX] & 0x7F);
  uint8_t arg = packetCopy[RGB_CCT_ARGUMENT_INDEX];

  if (command == RGB_CCT_ON) {
    if (arg == RGB_CCT_MODE_SPEED_DOWN) {
      result["command"] = "mode_speed_down";
    } else if (arg == RGB_CCT_MODE_SPEED_UP) {
      result["command"] = "mode_speed_up";
    } else if (arg < 5) { // Group is not reliably encoded in group byte. Extract from arg byte
      result["state"] = "ON";
      result["group_id"] = arg;
    } else {
      result["state"] = "OFF";
      result["group_id"] = arg-5;
    }
  } else if (command == RGB_CCT_COLOR) {
    uint8_t rescaledColor = (arg - RGB_CCT_COLOR_OFFSET) % 0x100;
    uint16_t hue = Units::rescale<uint16_t, uint16_t>(rescaledColor, 360, 255.0);
    result["hue"] = hue;
  } else if (command == RGB_CCT_KELVIN) {
    uint8_t temperature =
        static_cast<uint8_t>(
          // Range in packets is 180 - 220 or something like that. Shift to
          // 0..224. Then strip out values out of range [0..24), and (224..255]
          constrain(
            static_cast<uint8_t>(arg + RGB_CCT_KELVIN_REMOTE_OFFSET),
            24,
            224
          )
            +
          // Shift 24 down to 0
          RGB_CCT_KELVIN_REMOTE_START
        )/2; // values are in increments of 2

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

  if (! result.containsKey("state")) {
    result["state"] = "ON";
  }
}

void RgbCctPacketFormatter::format(uint8_t const* packet, char* buffer) {
  buffer += sprintf_P(buffer, PSTR("Raw packet: "));
  for (int i = 0; i < packetLength; i++) {
    buffer += sprintf_P(buffer, PSTR("%02X "), packet[i]);
  }

  uint8_t decodedPacket[packetLength];
  memcpy(decodedPacket, packet, packetLength);

  V2RFEncoding::decodeV2Packet(decodedPacket);

  buffer += sprintf_P(buffer, PSTR("\n\nDecoded:\n"));
  buffer += sprintf_P(buffer, PSTR("Key      : %02X\n"), decodedPacket[0]);
  buffer += sprintf_P(buffer, PSTR("b1       : %02X\n"), decodedPacket[1]);
  buffer += sprintf_P(buffer, PSTR("ID       : %02X%02X\n"), decodedPacket[2], decodedPacket[3]);
  buffer += sprintf_P(buffer, PSTR("Command  : %02X\n"), decodedPacket[4]);
  buffer += sprintf_P(buffer, PSTR("Argument : %02X\n"), decodedPacket[5]);
  buffer += sprintf_P(buffer, PSTR("Sequence : %02X\n"), decodedPacket[6]);
  buffer += sprintf_P(buffer, PSTR("Group    : %02X\n"), decodedPacket[7]);
  buffer += sprintf_P(buffer, PSTR("Checksum : %02X"), decodedPacket[8]);
}
