#include <V6CctCommandHandler.h>

bool V6CctCommandHandler::handlePreset(
    MiLightClient* client,
    uint8_t commandLsb,
    uint32_t commandArg)
{
  return false;
}

bool V6CctCommandHandler::handleCommand(
    MiLightClient* client,
    uint32_t command,
    uint32_t commandArg)
{
  const uint8_t cmd = command & 0x7F;
  const uint8_t arg = commandArg >> 24;

  client->setHeld((command & 0x80) == 0x80);

  if (cmd == V2_CCT_COMMAND_PREFIX) {
    switch (arg) {
      case V2_CCT_ON:
        client->updateStatus(ON);
        break;

      case V2_CCT_OFF:
        client->updateStatus(OFF);
        break;

      case V2_CCT_BRIGHTNESS_DOWN:
        client->decreaseBrightness();
        break;

      case V2_CCT_BRIGHTNESS_UP:
        client->increaseBrightness();
        break;

      case V2_CCT_TEMPERATURE_DOWN:
        client->decreaseTemperature();
        break;

      case V2_CCT_TEMPERATURE_UP:
        client->increaseTemperature();
        break;

      case V2_CCT_NIGHT_LIGHT:
        client->enableNightMode();
        break;

      default:
        return false;
    }

    return true;
  }

  return false;
}
