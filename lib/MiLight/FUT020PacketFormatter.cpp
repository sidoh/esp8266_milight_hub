#include <FUT020PacketFormatter.h>
#include <Units.h>

void FUT020PacketFormatter::updateColorRaw(uint8_t color) {
  command(static_cast<uint8_t>(FUT020Command::COLOR), color);
}

void FUT020PacketFormatter::updateHue(uint16_t hue) {
  uint16_t remapped = Units::rescale<uint16_t, uint16_t>(hue, 255.0, 360.0);
  remapped = (remapped + 0xB0) % 0x100;

  updateColorRaw(remapped);
}

void FUT020PacketFormatter::updateColorWhite() {
  command(static_cast<uint8_t>(FUT020Command::COLOR_WHITE_TOGGLE), 0);
}

void FUT020PacketFormatter::nextMode() {
  command(static_cast<uint8_t>(FUT020Command::MODE_SWITCH), 0);
}

void FUT020PacketFormatter::updateBrightness(uint8_t value) {
  const GroupState* state = this->stateStore->get(deviceId, groupId, MiLightRemoteType::REMOTE_TYPE_FUT020);
  int8_t knownValue = (state != NULL && state->isSetBrightness())
    ? state->getBrightness() / FUT02xPacketFormatter::NUM_BRIGHTNESS_INTERVALS
    : -1;

  valueByStepFunction(
    &PacketFormatter::increaseBrightness,
    &PacketFormatter::decreaseBrightness,
    FUT02xPacketFormatter::NUM_BRIGHTNESS_INTERVALS,
    value / FUT02xPacketFormatter::NUM_BRIGHTNESS_INTERVALS,
    knownValue
  );
}

void FUT020PacketFormatter::increaseBrightness() {
  command(static_cast<uint8_t>(FUT020Command::BRIGHTNESS_UP), 0);
}

void FUT020PacketFormatter::decreaseBrightness() {
  command(static_cast<uint8_t>(FUT020Command::BRIGHTNESS_DOWN), 0);
}

void FUT020PacketFormatter::updateStatus(MiLightStatus status, uint8_t groupId) {
  command(static_cast<uint8_t>(FUT020Command::ON_OFF), 0);
}

BulbId FUT020PacketFormatter::parsePacket(const uint8_t* packet, JsonObject result) {
  FUT020Command command = static_cast<FUT020Command>(packet[FUT02xPacketFormatter::FUT02X_COMMAND_INDEX] & 0x0F);

  BulbId bulbId(
    (packet[1] << 8) | packet[2],
    0,
    REMOTE_TYPE_FUT020
  );

  switch (command) {
    case FUT020Command::ON_OFF:
      result[F("state")] = F("ON");
      break;

    case FUT020Command::BRIGHTNESS_DOWN:
      result[F("command")] = F("brightness_down");
      break;

    case FUT020Command::BRIGHTNESS_UP:
      result[F("command")] = F("brightness_up");
      break;

    case FUT020Command::MODE_SWITCH:
      result[F("command")] = F("next_mode");
      break;

    case FUT020Command::COLOR_WHITE_TOGGLE:
      result[F("command")] = F("color_white_toggle");
      break;

    case FUT020Command::COLOR:
      uint16_t remappedColor = Units::rescale<uint16_t, uint16_t>(packet[FUT02xPacketFormatter::FUT02X_ARGUMENT_INDEX], 360.0, 255.0);
      remappedColor = (remappedColor + 113) % 360;
      result[GroupStateFieldNames::HUE] = remappedColor;
      break;
  }

  return bulbId;
}