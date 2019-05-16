#include <PacketFormatter.h>

#ifndef _RGBW_PACKET_FORMATTER_H
#define _RGBW_PACKET_FORMATTER_H

#define RGBW_PROTOCOL_ID_BYTE 0xB0

enum MiLightRgbwButton {
  RGBW_ALL_ON            = 0x01,
  RGBW_ALL_OFF           = 0x02,
  RGBW_GROUP_1_ON        = 0x03,
  RGBW_GROUP_1_OFF       = 0x04,
  RGBW_GROUP_2_ON        = 0x05,
  RGBW_GROUP_2_OFF       = 0x06,
  RGBW_GROUP_3_ON        = 0x07,
  RGBW_GROUP_3_OFF       = 0x08,
  RGBW_GROUP_4_ON        = 0x09,
  RGBW_GROUP_4_OFF       = 0x0A,
  RGBW_SPEED_UP          = 0x0B,
  RGBW_SPEED_DOWN        = 0x0C,
  RGBW_DISCO_MODE        = 0x0D,
  RGBW_BRIGHTNESS        = 0x0E,
  RGBW_COLOR             = 0x0F,
  RGBW_ALL_MAX_LEVEL     = 0x11,
  RGBW_ALL_MIN_LEVEL     = 0x12,

  // These are the only mechanism (that I know of) to disable RGB and set the
  // color to white.
  RGBW_GROUP_1_MAX_LEVEL = 0x13,
  RGBW_GROUP_1_MIN_LEVEL = 0x14,
  RGBW_GROUP_2_MAX_LEVEL = 0x15,
  RGBW_GROUP_2_MIN_LEVEL = 0x16,
  RGBW_GROUP_3_MAX_LEVEL = 0x17,
  RGBW_GROUP_3_MIN_LEVEL = 0x18,
  RGBW_GROUP_4_MAX_LEVEL = 0x19,
  RGBW_GROUP_4_MIN_LEVEL = 0x1A,

  // Button codes for night mode. A long press on the corresponding OFF button
  // Not actually needed/used.
  RGBW_ALL_NIGHT = 0x12,
  RGBW_GROUP_1_NIGHT = 0x14,
  RGBW_GROUP_2_NIGHT = 0x16,
  RGBW_GROUP_3_NIGHT = 0x18,
  RGBW_GROUP_4_NIGHT = 0x1A,
};

#define RGBW_COMMAND_INDEX 5
#define RGBW_BRIGHTNESS_GROUP_INDEX 4
#define RGBW_COLOR_INDEX 3
#define RGBW_NUM_MODES 9

class RgbwPacketFormatter : public PacketFormatter {
public:
  RgbwPacketFormatter()
    : PacketFormatter(REMOTE_TYPE_RGBW, 7)
  { }

  virtual bool canHandle(const uint8_t* packet, const size_t len);
  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void updateBrightness(uint8_t value);
  virtual void command(uint8_t command, uint8_t arg);
  virtual void updateHue(uint16_t value);
  virtual void updateColorRaw(uint8_t value);
  virtual void updateColorWhite();
  virtual void format(uint8_t const* packet, char* buffer);
  virtual void unpair();
  virtual void modeSpeedDown();
  virtual void modeSpeedUp();
  virtual void nextMode();
  virtual void previousMode();
  virtual void updateMode(uint8_t mode);
  virtual void enableNightMode();
  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result);

  virtual void initializePacket(uint8_t* packet);

protected:
  static bool isStatusCommand(const uint8_t command);
  uint8_t currentMode();
};

#endif
