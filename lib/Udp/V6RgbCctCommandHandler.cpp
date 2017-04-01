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
  
  if (cmd == V2_STATUS) {
    switch (arg) {
      case V2_RGB_CCT_ON:
      case V2_RGB_CCT_OFF:
        client->updateStatus(arg == V2_RGB_CCT_ON ? ON : OFF);
        break;
        
      case V2_RGB_NIGHT_MODE:
        client->updateBrightness(0);
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
      client->updateColorRaw(arg);
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
