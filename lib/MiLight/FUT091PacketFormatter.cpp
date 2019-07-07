#include <FUT091PacketFormatter.h>
#include <V2RFEncoding.h>
#include <Units.h>
#include <MiLightCommands.h>

static const uint8_t BRIGHTNESS_SCALE_MAX = 0x97;
static const uint8_t KELVIN_SCALE_MAX = 0xC5;

void FUT091PacketFormatter::updateBrightness(uint8_t value) {
  command(static_cast<uint8_t>(FUT091Command::BRIGHTNESS), V2PacketFormatter::tov2scale(value, BRIGHTNESS_SCALE_MAX, 2));
}

void FUT091PacketFormatter::updateTemperature(uint8_t value) {
  command(static_cast<uint8_t>(FUT091Command::KELVIN), V2PacketFormatter::tov2scale(value, KELVIN_SCALE_MAX, 2, false));
}

void FUT091PacketFormatter::enableNightMode() {
  uint8_t arg = groupCommandArg(OFF, groupId);
  command(static_cast<uint8_t>(FUT091Command::ON_OFF) | 0x80, arg);
}

BulbId FUT091PacketFormatter::parsePacket(const uint8_t *packet, JsonObject result) {
  uint8_t packetCopy[V2_PACKET_LEN];
  memcpy(packetCopy, packet, V2_PACKET_LEN);
  V2RFEncoding::decodeV2Packet(packetCopy);

  BulbId bulbId(
    (packetCopy[2] << 8) | packetCopy[3],
    packetCopy[7],
    REMOTE_TYPE_FUT091
  );

  uint8_t command = (packetCopy[V2_COMMAND_INDEX] & 0x7F);
  uint8_t arg = packetCopy[V2_ARGUMENT_INDEX];

  if (command == (uint8_t)FUT091Command::ON_OFF) {
    if ((packetCopy[V2_COMMAND_INDEX] & 0x80) == 0x80) {
      result[GroupStateFieldNames::COMMAND] = MiLightCommandNames::NIGHT_MODE;
    } else if (arg < 5) { // Group is not reliably encoded in group byte. Extract from arg byte
      result[GroupStateFieldNames::STATE] = "ON";
      bulbId.groupId = arg;
    } else {
      result[GroupStateFieldNames::STATE] = "OFF";
      bulbId.groupId = arg-5;
    }
  } else if (command == (uint8_t)FUT091Command::BRIGHTNESS) {
    uint8_t level = V2PacketFormatter::fromv2scale(arg, BRIGHTNESS_SCALE_MAX, 2, true);
    result[GroupStateFieldNames::BRIGHTNESS] = Units::rescale<uint8_t, uint8_t>(level, 255, 100);
  } else if (command == (uint8_t)FUT091Command::KELVIN) {
    uint8_t kelvin = V2PacketFormatter::fromv2scale(arg, KELVIN_SCALE_MAX, 2, false);
    result[GroupStateFieldNames::COLOR_TEMP] = Units::whiteValToMireds(kelvin, 100);
  } else {
    result["button_id"] = command;
    result["argument"] = arg;
  }

  return bulbId;
}
