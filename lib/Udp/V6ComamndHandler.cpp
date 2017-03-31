#include <V6CommandHandler.h>
#include <V6RgbCctCommandHandler.h>
#include <V6RgbwCommandHandler.h>
#include <V6RgbCommandHandler.h>
#include <V6CctCommandHandler.h>
#include <Size.h>

V6CommandHandler* V6CommandHandler::ALL_HANDLERS[] = {
  new V6RgbCctCommandHandler(0x0800),
  new V6RgbwCommandHandler(0x0700),
  new V6RgbCommandHandler(0x0500),
  new V6CctCommandHandler(0x0100),
};

const size_t V6CommandHandler::NUM_HANDLERS = size(ALL_HANDLERS);

bool V6CommandDemuxer::handleCommand(MiLightClient* client, 
  uint16_t deviceId,
  uint8_t group,
  uint32_t command,
  uint32_t commandArg)
{
  for (size_t i = 0; i < numHandlers; i++) {
    if (((handlers[i]->commandId & command) == handlers[i]->commandId)
      && handlers[i]->handleCommand(client, deviceId, group, command, commandArg)) {
      return true;
    }
  }
  
  return false;
}