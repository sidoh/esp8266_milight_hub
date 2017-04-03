#include <V6CctCommandHandler.h>

bool V6CctCommandHandler::handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint32_t command,
    uint32_t commandArg)
{
  const uint8_t cmd = command & 0xFF;
  const uint8_t arg = commandArg >> 24;
  
  client->prepare(MilightCctConfig, deviceId, group);
  
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
        
      default:
        return false;
    }
    
    return true;
  }
  
  return false;
}