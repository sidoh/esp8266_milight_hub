#include <V6RgbwCommandHandler.h>

bool V6RgbwCommandHandler::handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint32_t command,
    uint32_t commandArg)
{
  const uint8_t cmd = command & 0xFF;
  const uint8_t arg = commandArg >> 24;
  
  client->prepare(MilightRgbwConfig, deviceId, 0);
  
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
        client->updateColorWhite();
        client->updateBrightness(0);
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