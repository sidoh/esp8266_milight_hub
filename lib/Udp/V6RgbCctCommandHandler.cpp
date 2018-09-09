#include <V6RgbCctCommandHandler.h>

bool V6RgbCctCommandHandler::handlePreset(
    MiLightClient* client,
    uint8_t commandLsb,
    uint32_t commandArg)
{
  if (commandLsb == 0) {
    const uint8_t saturation = commandArg >> 24;
    const uint8_t color = (commandArg >> 16);
    const uint8_t brightness = (commandArg >> 8);

    client->updateBrightness(brightness);
    client->updateColorRaw(color);
    client->updateSaturation(saturation);
  } else if (commandLsb == 1) {
    const uint8_t brightness = (commandArg >> 16);
    const uint8_t kelvin = (commandArg >> 8);

    client->updateBrightness(brightness);
    client->updateTemperature(0x64 - kelvin);
  } else {
    return false;
  }

  return true;
}

bool V6RgbCctCommandHandler::handleCommand(
    MiLightClient* client,
    uint32_t command,
    uint32_t commandArg)
{
  const uint8_t cmd = command & 0x7F;
  const uint8_t arg = commandArg >> 24;

  client->setHeld((command & 0x80) == 0x80);

  if (cmd == V2_STATUS) {
    switch (arg) {
      case V2_RGB_CCT_ON:
      case V2_RGB_CCT_OFF:
        client->updateStatus(arg == V2_RGB_CCT_ON ? ON : OFF);
        break;

      case V2_RGB_NIGHT_MODE:
        client->enableNightMode();
        break;

      case V2_RGB_CCT_SPEED_DOWN:
        client->modeSpeedDown();
        break;

      case V2_RGB_CCT_SPEED_UP:
        client->modeSpeedUp();
        break;

      default:
        return false;
    }

    return true;
  }

  switch (cmd) {
    case V2_COLOR:
      handleUpdateColor(client, commandArg);
      break;

    case V2_KELVIN:
      client->updateTemperature(100 - arg);
      break;

    case V2_BRIGHTNESS:
      client->updateBrightness(arg);
      break;

    case V2_SATURATION:
      client->updateSaturation(100 - arg);
      break;

    case V2_MODE:
      client->updateMode(arg-1);
      break;

    default:
      return false;
  }

  return true;
}

/*
 * Arguments are 32 bits. Most commands use the first byte, but color arguments
 * can use all four. Triggered in app when quickly transitioning through colors.
 */
void V6RgbCctCommandHandler::handleUpdateColor(MiLightClient *client, uint32_t color) {
  for (int i = 3; i >= 0; i--) {
    const uint8_t argValue = (color >> (i*8)) & 0xFF;

    client->updateColorRaw(argValue + 0xF6);
  }
}
