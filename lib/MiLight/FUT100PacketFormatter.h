#ifndef _FUT100_PACKET_FORMATTER_H
#define _FUT100_PACKET_FORMATTER_H

#include <V2PacketFormatter.h>

enum MiLightFUT100Command {
  FUT100_ON_G_LOW   = 0x01,
  FUT100_COLOR      = 0x02,
  FUT100_BRIGHTNESS = 0x05,
  FUT100_MODE       = 0x06,
  FUT100_KELVIN     = 0x07,
  FUT100_SATURATION = 0x07,
  FUT100_ON_G_HIGH  = 0x08
};

enum MiLightFUT100Arguments {
  FUT100_MODE_SPEED_UP   = 0x12,
  FUT100_MODE_SPEED_DOWN = 0x13,
  FUT100_WHITE_MODE      = 0x14
};

class FUT100PacketFormatter : public V2PacketFormatter {
public:
  FUT100PacketFormatter()
    : V2PacketFormatter(REMOTE_TYPE_FUT100, 0x25, 100) 
  { }

  virtual void updateStatus(MiLightStatus status, uint8_t groupId) override;
  virtual void updateBrightness(uint8_t value) override;
  virtual void updateHue(uint16_t value) override;
  virtual void updateColorRaw(uint8_t value) override;
  virtual void updateColorWhite() override;
  virtual void updateTemperature(uint8_t value) override;
  virtual void updateSaturation(uint8_t value) override;
  virtual void enableNightMode() override;

  virtual void modeSpeedDown() override;
  virtual void modeSpeedUp() override;
  virtual void updateMode(uint8_t mode) override;

  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result) override;
};

#endif
