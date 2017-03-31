#include <V6RgbCctCommandHandler.h>

bool V6RgbCctCommandHandler::handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint32_t command,
    uint32_t commandArg)
{
  const uint8_t cmd = command & 0xFF;
  const uint8_t arg = commandArg >> 24;
  
  client->prepare(MilightRgbCctConfig, deviceId, group);
  
  switch (cmd) {
    case V2_STATUS:
      if (arg == 0x01) {
        client->updateStatus(ON);
      } else if (arg == 0x02) {
        client->updateStatus(OFF);
      } else if (arg == 0x05) {
        client->updateBrightness(0);
      }
      break;
      
    case V2_COLOR:
      client->updateColorRaw(arg);
      break;
      
    case V2_KELVIN:
      client->updateTemperature(arg);
      break;
      
    case V2_BRIGHTNESS:
      client->updateBrightness(arg);
      break;
      
    case V2_SATURATION:
      client->updateSaturation(100 - arg);
      break;
      
    default:
      return false;
  }
  
  return true;
}
