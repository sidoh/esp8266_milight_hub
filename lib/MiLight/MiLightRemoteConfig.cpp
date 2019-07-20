#include <MiLightRemoteConfig.h>
#include <MiLightRemoteType.h>

/**
 * IMPORTANT NOTE: These should be in the same order as MiLightRemoteType.
 */
const MiLightRemoteConfig* MiLightRemoteConfig::ALL_REMOTES[] = {
  &FUT096Config, // rgbw
  &FUT007Config, // cct
  &FUT092Config, // rgb+cct
  &FUT098Config, // rgb
  &FUT089Config, // 8-group rgb+cct (b8, fut089)
  &FUT091Config,
  &FUT020Config
};

const size_t MiLightRemoteConfig::NUM_REMOTES = size(ALL_REMOTES);

const MiLightRemoteConfig* MiLightRemoteConfig::fromType(const String& type) {
  return fromType(MiLightRemoteTypeHelpers::remoteTypeFromString(type));
}

const MiLightRemoteConfig* MiLightRemoteConfig::fromType(MiLightRemoteType type) {
  if (type == REMOTE_TYPE_UNKNOWN || type >= size(ALL_REMOTES)) {
    Serial.print(F("MiLightRemoteConfig::fromType: ERROR - tried to fetch remote config for unknown type: "));
    Serial.println(type);
    return NULL;
  }

  return ALL_REMOTES[type];
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

  // This can happen under normal circumstances, so not an error condition
#ifdef DEBUG_PRINTF
  Serial.println(F("MiLightRemoteConfig::fromReceivedPacket: ERROR - tried to fetch remote config for unknown packet"));
#endif

  return NULL;
}

const MiLightRemoteConfig FUT096Config( //rgbw
  new RgbwPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[0],
  REMOTE_TYPE_RGBW,
  "rgbw",
  4
);

const MiLightRemoteConfig FUT007Config( //cct
  new CctPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[1],
  REMOTE_TYPE_CCT,
  "cct",
  4
);

const MiLightRemoteConfig FUT091Config( //v2 cct
  new FUT091PacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[2],
  REMOTE_TYPE_FUT091,
  "fut091",
  4
);

const MiLightRemoteConfig FUT092Config( //rgb+cct
  new RgbCctPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[2],
  REMOTE_TYPE_RGB_CCT,
  "rgb_cct",
  4
);

const MiLightRemoteConfig FUT089Config( //rgb+cct B8 / FUT089
  new FUT089PacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[2],
  REMOTE_TYPE_FUT089,
  "fut089",
  8
);

const MiLightRemoteConfig FUT098Config( //rgb
  new RgbPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[3],
  REMOTE_TYPE_RGB,
  "rgb",
  0
);

const MiLightRemoteConfig FUT020Config(
  new FUT020PacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[4],
  REMOTE_TYPE_FUT020,
  "fut020",
  0
);