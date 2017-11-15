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
  V6RgbCctCommandHandler()
    : V6CommandHandler(0x0800, FUT092Config)
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

  void handleUpdateColor(MiLightClient* client, uint32_t color);

};

#endif
