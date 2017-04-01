#include <MiLightClient.h>
#include <MiLightRadioConfig.h>

#ifndef _V6_COMMAND_HANDLER_H
#define _V6_COMMAND_HANDLER_H 

enum V6CommandTypes {
  V6_PAIR = 0x3D,
  V6_UNPAIR = 0x3E,
  V6_COMMAND = 0x31
};

class V6CommandHandler {
public:
  static V6CommandHandler* ALL_HANDLERS[];
  static const size_t NUM_HANDLERS;
  
  V6CommandHandler(uint16_t commandId, MiLightRadioConfig& radioConfig)
    : commandId(commandId),
      radioConfig(radioConfig)
  { }
  
  virtual bool handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint8_t commandType,
    uint32_t command,
    uint32_t commandArg
  );
  
  const uint16_t commandId;
  MiLightRadioConfig& radioConfig;
  
protected:
  
  virtual bool handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint32_t command,
    uint32_t commandArg
  ) = 0;
};

class V6CommandDemuxer : public V6CommandHandler {
public:
  V6CommandDemuxer(V6CommandHandler* handlers[], size_t numHandlers)
    : V6CommandHandler(0, MilightRgbwConfig),  
      handlers(handlers),
      numHandlers(numHandlers)
  { }
  
  virtual bool handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint8_t commandType,
    uint32_t command,
    uint32_t commandArg
  );
  
protected:
  V6CommandHandler** handlers;
  size_t numHandlers;
  
  virtual bool handleCommand(
    MiLightClient* client, 
    uint16_t deviceId,
    uint8_t group,
    uint32_t command,
    uint32_t commandArg
  );
};

#endif