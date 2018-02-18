#include <inttypes.h>
#include <PacketFormatter.h>

#ifndef _V2_PACKET_FORMATTER
#define _V2_PACKET_FORMATTER

#define V2_PACKET_LEN 9

#define V2_PROTOCOL_ID_INDEX 1
#define V2_COMMAND_INDEX 4
#define V2_ARGUMENT_INDEX 5

class V2PacketFormatter : public PacketFormatter {
public:
  V2PacketFormatter(uint8_t protocolId, uint8_t numGroups);

  virtual bool canHandle(const uint8_t* packet, const size_t packetLen);
  virtual void initializePacket(uint8_t* packet);

  virtual void updateStatus(MiLightStatus status, uint8_t group);
  virtual void command(uint8_t command, uint8_t arg);
  virtual void format(uint8_t const* packet, char* buffer);
  virtual void unpair();

  virtual void finalizePacket(uint8_t* packet);

  uint8_t groupCommandArg(MiLightStatus status, uint8_t groupId);

protected:
  const uint8_t protocolId;
  const uint8_t numGroups;
  void switchMode(GroupState currentState, BulbMode desiredMode);
};

#endif
