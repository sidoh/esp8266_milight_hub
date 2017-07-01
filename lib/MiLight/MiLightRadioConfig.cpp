#include <MiLightRadioConfig.h>

MiLightRadioConfig* MiLightRadioConfig::ALL_CONFIGS[] = {
  &MilightRgbwConfig,
  &MilightCctConfig,
  &MilightRgbCctConfig,
  &MilightRgbConfig
};

MiLightRadioConfig* MiLightRadioConfig::fromString(const String& s) {
  for (size_t i = 0; i < MiLightRadioConfig::NUM_CONFIGS; i++) {
    MiLightRadioConfig* config = MiLightRadioConfig::ALL_CONFIGS[i];
    if (s.equalsIgnoreCase(config->name)) {
      return config;
    }
  }
  return NULL;
}

MiLightRadioConfig* MiLightRadioConfig::fromType(MiLightRadioType type) {
  for (size_t i = 0; i < MiLightRadioConfig::NUM_CONFIGS; i++) {
    MiLightRadioConfig* config = MiLightRadioConfig::ALL_CONFIGS[i];
    if (config->type == type) {
      return config;
    }
  }
  return NULL;
}

size_t MiLightRadioConfig::getPacketLength() const {
  return packetFormatter->getPacketLength();
}
