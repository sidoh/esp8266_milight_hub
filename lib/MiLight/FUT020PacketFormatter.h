#include <FUT02xPacketFormatter.h>

#pragma once

enum class FUT020Command {
  ON_OFF             = 0x04,
  MODE_SWITCH        = 0x02,
  COLOR_WHITE_TOGGLE = 0x05,
  BRIGHTNESS_DOWN    = 0x01,
  BRIGHTNESS_UP      = 0x03,
  COLOR              = 0x00
};

class FUT020PacketFormatter : public FUT02xPacketFormatter {
public:
  FUT020PacketFormatter()
    : FUT02xPacketFormatter(REMOTE_TYPE_FUT020)
  { }

  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void updateHue(uint16_t value);
  virtual void updateColorRaw(uint8_t value);
  virtual void updateColorWhite();
  virtual void nextMode();
  virtual void updateBrightness(uint8_t value);
  virtual void increaseBrightness();
  virtual void decreaseBrightness();

  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result) override;
};