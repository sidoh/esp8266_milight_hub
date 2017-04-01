#include <V6CommandHandler.h>

#ifndef _V6_RGB_CCT_COMMAND_HANDLER_H
#define _V6_RGB_CCT_COMMAND_HANDLER_H 

enum V2CommandIds {
  V2_COLOR = 0x01,
  V2_SATURATION = 0x02,
  V2_BRIGHTNESS = 0x03,
  V2_STATUS = 0x04,
  V2_KELVIN = 0x05,
  V2_MODE = 0x06
};

enum V2CommandArgIds {
  V2_RGB_CCT_ON = 0x01,
  V2_RGB_CCT_OFF = 0x02,
  V2_RGB_CCT_SPEED_UP = 0x03,
  V2_RGB_CCT_SPEED_DOWN = 0x04,
  V2_RGB_NIGHT_MODE = 0x05
};

class V6RgbCctCommandHandler : public V6CommandHandler {
public:
  V6RgbCctCommandHandler(uint16_t commandId)
    : V6CommandHandler(commandId)
  { }
  
  virtual bool handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint32_t command,
    uint32_t commandArg
  );
  
};

#endif