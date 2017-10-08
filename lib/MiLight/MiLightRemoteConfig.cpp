#include <MiLightRemoteConfig.h>

const MiLightRemoteConfig* MiLightRemoteConfig::ALL_REMOTES[] = {
  &FUT096Config,
  &FUT091Config,
  &FUT092Config,
  &FUT089Config,
  &FUT098Config
};

const MiLightRemoteConfig* MiLightRemoteConfig::fromType(const String& type) {
  if (type.equalsIgnoreCase("rgbw") || type.equalsIgnoreCase("fut096")) {
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

  if (type.equalsIgnoreCase("rgb") || type.equalsIgnoreCase("fut098")) {
    return &FUT098Config;
  }

  Serial.println(F("ERROR - tried to fetch remote config for type"));

  return NULL;
}

const MiLightRemoteConfig* MiLightRemoteConfig::fromType(MiLightRemoteType type) {
  switch (type) {
    case REMOTE_TYPE_RGBW:
      return &FUT096Config;
    case REMOTE_TYPE_RGB:
      return &FUT098Config;
    case REMOTE_TYPE_CCT:
      return &FUT091Config;
    case REMOTE_TYPE_RGB_CCT:
      return &FUT092Config;
    case REMOTE_TYPE_FUT089:
      return &FUT089Config;
    default:
      Serial.println(F("ERROR - tried to fetch remote config for unknown type"));
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

  Serial.println(F("ERROR - tried to fetch remote config for unknown packet"));

  return NULL;
}

const MiLightRemoteConfig FUT096Config( //rgbw
  new RgbwPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[0],
  REMOTE_TYPE_RGBW,
  "rgbw"
);

const MiLightRemoteConfig FUT091Config( //cct
  new CctPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[1],
  REMOTE_TYPE_CCT,
  "cct"
);

const MiLightRemoteConfig FUT092Config( //rgb+cct
  new RgbCctPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[2],
  REMOTE_TYPE_RGB_CCT,
  "rgb_cct"
);

const MiLightRemoteConfig FUT089Config( //rgb+cct B8 / FUT089
  new FUT089PacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[2],
  REMOTE_TYPE_FUT089,
  "fut089"
);

const MiLightRemoteConfig FUT098Config( //rgb
  new RgbPacketFormatter(),
  MiLightRadioConfig::ALL_CONFIGS[3],
  REMOTE_TYPE_RGB,
  "rgb"
);
