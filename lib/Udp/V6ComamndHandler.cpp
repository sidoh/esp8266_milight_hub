#include <V6CommandHandler.h>
#include <V6RgbCctCommandHandler.h>
#include <V6RgbwCommandHandler.h>
#include <V6RgbCommandHandler.h>
#include <V6CctCommandHandler.h>
#include <Size.h>

V6CommandHandler* V6CommandHandler::ALL_HANDLERS[] = {
  new V6RgbCctCommandHandler(),
  new V6RgbwCommandHandler(),
  new V6RgbCommandHandler(),
  new V6CctCommandHandler()
};

const size_t V6CommandHandler::NUM_HANDLERS = size(ALL_HANDLERS);

bool V6CommandHandler::handleCommand(MiLightClient* client,
  uint16_t deviceId,
  uint8_t group,
  uint8_t commandType,
  uint32_t command,
  uint32_t commandArg)
{
  client->prepare(&remoteConfig, deviceId, group);

  if (commandType == V6_PAIR) {
    client->pair();
  } else if (commandType == V6_UNPAIR) {
    client->unpair();
  } else if (commandType == V6_PRESET) {
    return this->handlePreset(client, command, commandArg);
  } else if (commandType == V6_COMMAND) {
    return this->handleCommand(client, command, commandArg);
  } else {
    return false;
  }

  return true;
}

bool V6CommandDemuxer::handleCommand(MiLightClient* client,
  uint16_t deviceId,
  uint8_t group,
  uint8_t commandType,
  uint32_t command,
  uint32_t commandArg)
{
  for (size_t i = 0; i < numHandlers; i++) {
    if (((handlers[i]->commandId & command) == handlers[i]->commandId)
      && handlers[i]->handleCommand(client, deviceId, group, commandType, command, commandArg)) {
      return true;
    }
  }

  return false;
}

bool V6CommandDemuxer::handleCommand(MiLightClient* client,
  uint32_t commandLsb,
  uint32_t commandArg)
{
  return false;
}

bool V6CommandDemuxer::handlePreset(MiLightClient* client,
  uint8_t commandLsb,
  uint32_t commandArg)
{
  return false;
}
