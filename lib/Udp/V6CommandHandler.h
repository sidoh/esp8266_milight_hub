#include <MiLightClient.h>

#ifndef _V6_COMMAND_HANDLER_H
#define _V6_COMMAND_HANDLER_H 

class V6CommandHandler {
public:
  static V6CommandHandler* ALL_HANDLERS[];
  static const size_t NUM_HANDLERS;
  
  V6CommandHandler(uint16_t commandId)
    : commandId(commandId)
  { }
  
  virtual bool handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint32_t command,
    uint32_t commandArg
  ) = 0;
  
  const uint16_t commandId;
};

class V6CommandDemuxer : public V6CommandHandler {
public:
  V6CommandDemuxer(V6CommandHandler* handlers[], size_t numHandlers)
    : V6CommandHandler(0),  
      handlers(handlers),
      numHandlers(numHandlers)
  { }
  
  virtual bool handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint32_t command,
    uint32_t commandArg
  );
  
protected:
  V6CommandHandler** handlers;
  size_t numHandlers;
};

#endif