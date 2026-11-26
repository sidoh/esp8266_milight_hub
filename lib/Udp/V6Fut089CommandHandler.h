#include <V6CommandHandler.h>

#ifndef _V6_FUT089_COMMAND_HANDLER_H
#define _V6_FUT089_COMMAND_HANDLER_H

enum V6Fut089CommandIds {
  V6_FUT089_COLOR = 0x01,
  V6_FUT089_KELVIN = 0x02,
  V6_FUT089_SATURATION = 0x03,
  V6_FUT089_BRIGHTNESS = 0x04,
  V6_FUT089_MODE = 0x05,
  V6_FUT089_STATUS = 0x06
};

enum V6Fut089CommandArgIds {
  V6_FUT089_ON = 0x01,
  V6_FUT089_OFF = 0x02,
  V6_FUT089_SPEED_UP = 0x03,
  V6_FUT089_SPEED_DOWN = 0x04,
  V6_FUT089_WHITE_MODE = 0x05
};

class V6Fut089CommandHandler : public V6CommandHandler {
public:
  V6Fut089CommandHandler()
    : V6CommandHandler(0x0A00, FUT089Config)
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