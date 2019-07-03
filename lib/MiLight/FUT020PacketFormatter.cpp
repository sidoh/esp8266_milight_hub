#include <FUT020PacketFormatter.h>
#include <Units.h>

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

    // case FUT020Command::COLOR:
    //   result[F("hue")]
  }

  return bulbId;
}