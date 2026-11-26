#include <V6Fut089CommandHandler.h>

bool V6Fut089CommandHandler::handlePreset(
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

bool V6Fut089CommandHandler::handleCommand(
    MiLightClient* client,
    uint32_t command,
    uint32_t commandArg)
{
  const uint8_t cmd = command & 0x7F;
  const uint8_t arg = commandArg >> 24;
  const bool isHeld = (command & 0x80) == 0x80;

  client->setHeld(isHeld);

  if (cmd == V6_FUT089_STATUS) {
    switch (arg) {
      case V6_FUT089_ON:
        client->updateStatus(ON);
        break;

      case V6_FUT089_OFF:
        if (isHeld) {
          client->enableNightMode();
        } else {
          client->updateStatus(OFF);
        }
        break;

      case V6_FUT089_WHITE_MODE:
        client->updateColorWhite();
        break;

      case V6_FUT089_SPEED_DOWN:
        client->modeSpeedDown();
        break;

      case V6_FUT089_SPEED_UP:
        client->modeSpeedUp();
        break;

      default:
        return false;
    }

    return true;
  }

  switch (cmd) {
    case V6_FUT089_COLOR:
      handleUpdateColor(client, commandArg);
      break;

    case V6_FUT089_KELVIN:
      client->updateTemperature(100 - arg);
      break;

    case V6_FUT089_SATURATION:
      client->updateSaturation(100 - arg);
      break;

    case V6_FUT089_BRIGHTNESS:
      client->updateBrightness(arg);
      break;

    case V6_FUT089_MODE:
      client->updateMode(arg);
      break;

    default:
      return false;
  }

  return true;
}

void V6Fut089CommandHandler::handleUpdateColor(MiLightClient *client, uint32_t color) {
  for (int i = 3; i >= 0; i--) {
    const uint8_t argValue = (color >> (i*8)) & 0xFF;

    client->updateColorRaw(argValue + 0xF6);
  }
}