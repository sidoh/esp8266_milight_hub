#include <PacketFormatter.h>

#define RGB_CCT_COMMAND_INDEX 4
#define RGB_CCT_ARGUMENT_INDEX 5
#define RGB_CCT_NUM_MODES 9
#define V2_OFFSET_JUMP_START 0x54
#define RGB_CCT_PACKET_LEN 9

#define RGB_CCT_COLOR_OFFSET 0x5F
#define RGB_CCT_BRIGHTNESS_OFFSET 0x8F
#define RGB_CCT_SATURATION_OFFSET 0xD
#define RGB_CCT_KELVIN_OFFSET 0x94

// Remotes have a larger range
#define RGB_CCT_KELVIN_REMOTE_OFFSET 0x4C
#define RGB_CCT_KELVIN_REMOTE_START  0xE8

#ifndef _RGB_CCT_PACKET_FORMATTER_H
#define _RGB_CCT_PACKET_FORMATTER_H

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

class RgbCctPacketFormatter : public PacketFormatter {
public:
  static uint8_t const V2_OFFSETS[][4];

  RgbCctPacketFormatter()
    : PacketFormatter(RGB_CCT_PACKET_LEN),
      lastMode(0)
  { }

  virtual void initializePacket(uint8_t* packet);

  virtual void updateStatus(MiLightStatus status, uint8_t group);
  virtual void updateBrightness(uint8_t value);
  virtual void command(uint8_t command, uint8_t arg);
  virtual void updateHue(uint16_t value);
  virtual void updateColorRaw(uint8_t value);
  virtual void updateColorWhite();
  virtual void updateTemperature(uint8_t value);
  virtual void updateSaturation(uint8_t value);
  virtual void format(uint8_t const* packet, char* buffer);
  virtual void unpair();
  virtual void enableNightMode();

  virtual void modeSpeedDown();
  virtual void modeSpeedUp();
  virtual void updateMode(uint8_t mode);
  virtual void nextMode();
  virtual void previousMode();

  virtual void finalizePacket(uint8_t* packet);
  virtual void parsePacket(const uint8_t* packet, JsonObject& result);

  static void encodeV2Packet(uint8_t* packet);
  static void decodeV2Packet(uint8_t* packet);
  static uint8_t xorKey(uint8_t key);
  static uint8_t encodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2);
  static uint8_t decodeByte(uint8_t byte, uint8_t s1, uint8_t xorKey, uint8_t s2);

protected:

  uint8_t lastMode;
};

#endif
