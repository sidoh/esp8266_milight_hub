#include <V2PacketFormatter.h>

#ifndef _RGB_CCT_PACKET_FORMATTER_H
#define _RGB_CCT_PACKET_FORMATTER_H

#define RGB_CCT_NUM_MODES 9

#define RGB_CCT_COLOR_OFFSET 0x5F
#define RGB_CCT_BRIGHTNESS_OFFSET 0x8F
#define RGB_CCT_SATURATION_OFFSET 0xD
#define RGB_CCT_KELVIN_OFFSET 0x94

// Remotes have a larger range
#define RGB_CCT_KELVIN_REMOTE_START  0x94
#define RGB_CCT_KELVIN_REMOTE_END    0xCC

enum MiLightRgbCctCommand {
  RGB_CCT_ON = 0x01,
  RGB_CCT_OFF = 0x01,
  RGB_CCT_COLOR = 0x02,
  RGB_CCT_KELVIN = 0x03,
  RGB_CCT_BRIGHTNESS = 0x04,
  RGB_CCT_SATURATION = 0x04,
  RGB_CCT_MODE = 0x05
};

enum MiLightRgbCctArguments {
  RGB_CCT_MODE_SPEED_UP   = 0x0A,
  RGB_CCT_MODE_SPEED_DOWN = 0x0B
};

class RgbCctPacketFormatter : public V2PacketFormatter {
public:
  RgbCctPacketFormatter()
    : V2PacketFormatter(REMOTE_TYPE_RGB_CCT, 0x20, 4),
      lastMode(0)
  { }

  virtual void updateBrightness(uint8_t value);
  virtual void updateHue(uint16_t value);
  virtual void updateColorRaw(uint8_t value);
  virtual void updateColorWhite();
  virtual void updateTemperature(uint8_t value);
  virtual void updateSaturation(uint8_t value);
  virtual void enableNightMode();

  virtual void modeSpeedDown();
  virtual void modeSpeedUp();
  virtual void updateMode(uint8_t mode);
  virtual void nextMode();
  virtual void previousMode();

  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result);

protected:

  uint8_t lastMode;
};

#endif
