#include <V2PacketFormatter.h>

#ifndef _FUT091_PACKET_FORMATTER_H
#define _FUT091_PACKET_FORMATTER_H

enum class FUT091Command {
  ON_OFF = 0x01,
  BRIGHTNESS = 0x2,
  KELVIN = 0x03
};

class FUT091PacketFormatter : public V2PacketFormatter {
public:
  FUT091PacketFormatter()
    : V2PacketFormatter(REMOTE_TYPE_FUT091, 0x21, 4)    // protocol is 0x21, and there are 4 groups
  { }

  virtual void updateBrightness(uint8_t value);
  virtual void updateTemperature(uint8_t value);
  virtual void enableNightMode();

  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result);
};

#endif
