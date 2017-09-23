#include <V6CommandHandler.h>

#ifndef _V6_RGB_COMMAND_HANDLER_H
#define _V6_RGB_COMMAND_HANDLER_H

enum RgbCommandIds {
  V2_RGB_COMMAND_PREFIX  = 0x02,
  V2_RGB_COLOR_PREFIX    = 0x01,
  V2_RGB_BRIGHTNESS_DOWN = 0x01,
  V2_RGB_BRIGHTNESS_UP   = 0x02,
  V2_RGB_SPEED_DOWN      = 0x03,
  V2_RGB_SPEED_UP        = 0x04,
  V2_RGB_MODE_DOWN       = 0x05,
  V2_RGB_MODE_UP         = 0x06,
  V2_RGB_ON              = 0x09,
  V2_RGB_OFF             = 0x0A
};

class V6RgbCommandHandler : public V6CommandHandler {
public:
  V6RgbCommandHandler()
    : V6CommandHandler(0x0500, FUT098Config)
  { }

  virtual bool handleCommand(
    MiLightClient* client,
    uint32_t command,
    uint32_t commandArg
  );

  virtual bool handlePreset(
    MiLightClient* client,
    uint8_t commandLsb,
    uint32_t commandArg
  );

};

#endif
