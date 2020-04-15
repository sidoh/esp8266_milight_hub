#include <PacketFormatter.h>

#ifndef _CASALUX_PACKET_FORMATTER_H
#define _CASALUX_PACKET_FORMATTER_H

class CasaluxPacketFormatter : public PacketFormatter {
public:
  CasaluxPacketFormatter()
    : PacketFormatter(REMOTE_TYPE_CASALUX, 10, 20)
  { }

  virtual bool canHandle(const uint8_t* packet, const size_t len);

  virtual void command(uint8_t command, uint8_t arg);
  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void updateTemperature(uint8_t value);
  virtual void increaseTemperature();
  virtual void decreaseTemperature();
  virtual void updateBrightness(uint8_t value);
  virtual void increaseBrightness();
  virtual void decreaseBrightness();
  virtual void format(uint8_t const* packet, char* buffer);
  virtual void initializePacket(uint8_t* packet);
  virtual void finalizePacket(uint8_t* packet);
  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result);
  static uint8_t groupToGroupId(uint8_t group);
  static uint8_t groupIdToGroup(uint8_t groupId);
};

#endif
