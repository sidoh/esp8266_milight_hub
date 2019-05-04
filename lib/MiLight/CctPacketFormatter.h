#include <PacketFormatter.h>

#ifndef _CCT_PACKET_FORMATTER_H
#define _CCT_PACKET_FORMATTER_H

#define CCT_COMMAND_INDEX 4
#define CCT_INTERVALS 10

enum MiLightCctButton {
  CCT_ALL_ON            = 0x05,
  CCT_ALL_OFF           = 0x09,
  CCT_GROUP_1_ON        = 0x08,
  CCT_GROUP_1_OFF       = 0x0B,
  CCT_GROUP_2_ON        = 0x0D,
  CCT_GROUP_2_OFF       = 0x03,
  CCT_GROUP_3_ON        = 0x07,
  CCT_GROUP_3_OFF       = 0x0A,
  CCT_GROUP_4_ON        = 0x02,
  CCT_GROUP_4_OFF       = 0x06,
  CCT_BRIGHTNESS_DOWN   = 0x04,
  CCT_BRIGHTNESS_UP     = 0x0C,
  CCT_TEMPERATURE_UP    = 0x0E,
  CCT_TEMPERATURE_DOWN  = 0x0F
};

class CctPacketFormatter : public PacketFormatter {
public:
  CctPacketFormatter()
    : PacketFormatter(REMOTE_TYPE_CCT, 7, 20)
  { }

  virtual bool canHandle(const uint8_t* packet, const size_t len);

  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void command(uint8_t command, uint8_t arg);

  virtual void updateTemperature(uint8_t value);
  virtual void increaseTemperature();
  virtual void decreaseTemperature();

  virtual void updateBrightness(uint8_t value);
  virtual void increaseBrightness();
  virtual void decreaseBrightness();
  virtual void enableNightMode();

  virtual void format(uint8_t const* packet, char* buffer);
  virtual void initializePacket(uint8_t* packet);
  virtual void finalizePacket(uint8_t* packet);
  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result);

  static uint8_t getCctStatusButton(uint8_t groupId, MiLightStatus status);
  static uint8_t cctCommandIdToGroup(uint8_t command);
  static MiLightStatus cctCommandToStatus(uint8_t command);
};

#endif
