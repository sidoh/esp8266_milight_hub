#include <MiLightRadioConfig.h>
  
const MiLightRadioConfig* MiLightRadioConfig::ALL_CONFIGS[] = {
  &MilightRgbwConfig,
  &MilightCctConfig,
  &MilightRgbCctConfig,
  &MilightRgbConfig
};

MiLightRadioConfig* MiLightRadioConfig::fromString(const String& s) {
  if (s.equalsIgnoreCase("rgbw")) {
    return &MilightRgbwConfig;
  } else if (s.equalsIgnoreCase("cct")) {
    return &MilightCctConfig;
  } else if (s.equalsIgnoreCase("rgb_cct")) {
    return &MilightRgbCctConfig;
  } else if (s.equalsIgnoreCase("rgb")) {
    return &MilightRgbConfig;
  }
  
  return NULL;
}

size_t MiLightRadioConfig::getPacketLength() const {
  return packetFormatter->getPacketLength();
}