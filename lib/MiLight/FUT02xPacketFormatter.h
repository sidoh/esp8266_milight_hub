#include <PacketFormatter.h>

#pragma once

class FUT02xPacketFormatter : public PacketFormatter {
public:
  static const uint8_t FUT02X_COMMAND_INDEX = 4;
  static const uint8_t FUT02X_ARGUMENT_INDEX = 3;
  static const uint8_t NUM_BRIGHTNESS_INTERVALS = 10;

  FUT02xPacketFormatter(MiLightRemoteType type)
    : PacketFormatter(type, 6, 10)
  { }

  virtual bool canHandle(const uint8_t* packet, const size_t len) override;

  virtual void command(uint8_t command, uint8_t arg) override;

  virtual void pair() override;
  virtual void unpair() override;

  virtual void initializePacket(uint8_t* packet) override;
  virtual void format(uint8_t const* packet, char* buffer) override;
};