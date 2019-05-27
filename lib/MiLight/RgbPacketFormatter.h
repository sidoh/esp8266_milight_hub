#include <PacketFormatter.h>

#ifndef _RGB_PACKET_FORMATTER_H
#define _RGB_PACKET_FORMATTER_H

#define RGB_COMMAND_INDEX 4
#define RGB_COLOR_INDEX 3
#define RGB_INTERVALS 10

enum MiLightRgbButton {
  RGB_OFF             = 0x01,
  RGB_ON              = 0x02,
  RGB_BRIGHTNESS_UP   = 0x03,
  RGB_BRIGHTNESS_DOWN = 0x04,
  RGB_SPEED_UP        = 0x05,
  RGB_SPEED_DOWN      = 0x06,
  RGB_MODE_UP         = 0x07,
  RGB_MODE_DOWN       = 0x08,
  RGB_PAIR            = RGB_SPEED_UP
};

class RgbPacketFormatter : public PacketFormatter {
public:
  RgbPacketFormatter()
    : PacketFormatter(REMOTE_TYPE_RGB, 6, 20)
  { }

  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void updateBrightness(uint8_t value);
  virtual void increaseBrightness();
  virtual void decreaseBrightness();
  virtual void command(uint8_t command, uint8_t arg);
  virtual void updateHue(uint16_t value);
  virtual void updateColorRaw(uint8_t value);
  virtual void format(uint8_t const* packet, char* buffer);
  virtual void pair();
  virtual void unpair();
  virtual void modeSpeedDown();
  virtual void modeSpeedUp();
  virtual void nextMode();
  virtual void previousMode();
  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result);

  virtual void initializePacket(uint8_t* packet);
};

#endif
