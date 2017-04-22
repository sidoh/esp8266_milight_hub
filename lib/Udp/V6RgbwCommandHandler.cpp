#include <V6RgbwCommandHandler.h>

bool V6RgbwCommandHandler::handlePreset(
    MiLightClient* client,
    uint8_t commandLsb,
    uint32_t commandArg)
{
  if (commandLsb == 0) {
    client->updateColorRaw(commandArg >> 24);
    client->updateBrightness(commandArg >> 16);
  } else if (commandLsb == 1) {
    client->updateColorWhite();
    client->updateBrightness(commandArg >> 16);
  } else {
    return false;
  }

  return true;
}

bool V6RgbwCommandHandler::handleCommand(
    MiLightClient* client,
    uint32_t command,
    uint32_t commandArg)
{
  const uint8_t cmd = command & 0x7F;
  const uint8_t arg = commandArg >> 24;

  client->setHeld((command & 0x80) == 0x80);

  if (cmd == V2_RGBW_COMMAND_PREFIX) {
    switch (arg) {
      case V2_RGBW_ON:
        client->updateStatus(ON);
        break;

      case V2_RGBW_OFF:
        client->updateStatus(OFF);
        break;

      case V2_RGBW_WHITE_ON:
        client->updateColorWhite();
        break;

      case V2_RGBW_NIGHT_LIGHT:
        client->enableNightMode();
        break;

      case V2_RGBW_SPEED_DOWN:
        client->modeSpeedDown();
        break;

      case V2_RGBW_SPEED_UP:
        client->modeSpeedUp();
        break;

      default:
        return false;
    }

    return true;
  } else if (cmd == V2_RGBW_COLOR_PREFIX) {
    client->updateColorRaw(arg);
    return true;
  } else if (cmd == V2_RGBW_BRIGHTNESS_PREFIX) {
    client->updateBrightness(arg);
    return true;
  } else if (cmd == V2_RGBW_MODE_PREFIX) {
    client->updateMode(arg);
    return true;
  }

  return false;
}
