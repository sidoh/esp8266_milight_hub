#include <MiLightRemoteConfig.h>

const MiLightRemoteConfig* MiLightRemoteConfig::ALL_REMOTES[] = {
  &FUT096Config,
  &FUT091Config,
  &FUT092Config,
  &FUT089Config,
  &FUT098Config
};

const MiLightRemoteConfig* MiLightRemoteConfig::fromType(const String& type) {
  if (type.equalsIgnoreCase("rgb") || type.equalsIgnoreCase("fut096")) {
    return &FUT096Config;
  }

  if (type.equalsIgnoreCase("cct") || type.equalsIgnoreCase("fut091")) {
    return &FUT091Config;
  }

  if (type.equalsIgnoreCase("rgb_cct") || type.equalsIgnoreCase("fut092")) {
    return &FUT092Config;
  }

  if (type.equalsIgnoreCase("fut089")) {
    return &FUT089Config;
  }

  return NULL;
}

const MiLightRemoteConfig* MiLightRemoteConfig::fromType(MiLightRemoteType type) {
  switch (type) {
    case REMOTE_TYPE_RGB:
      return &FUT096Config;
    case REMOTE_TYPE_CCT:
      return &FUT091Config;
    case REMOTE_TYPE_RGB_CCT:
      return &FUT092Config;
    case REMOTE_TYPE_FUT089:
      return &FUT089Config;
    default:
      return NULL;
  }
}

const MiLightRemoteConfig* MiLightRemoteConfig::fromReceivedPacket(
  const MiLightRadioConfig& radioConfig,
  const uint8_t* packet,
  const size_t len
) {
  for (size_t i = 0; i < MiLightRemoteConfig::NUM_REMOTES; i++) {
    const MiLightRemoteConfig* config = MiLightRemoteConfig::ALL_REMOTES[i];
    if (&config->radioConfig == &radioConfig
      && config->packetFormatter->canHandle(packet, len)) {
      return config;
    }
  }

  return NULL;
}
