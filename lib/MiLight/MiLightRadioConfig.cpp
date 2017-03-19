#include <MiLightRadioConfig.h>

MiLightRadioConfig* MiLightRadioConfig::fromString(const String& s) {
  if (s.equalsIgnoreCase("rgbw")) {
    return &MilightRgbwConfig;
  } else if (s.equalsIgnoreCase("cct")) {
    return &MilightCctConfig;
  } else if (s.equalsIgnoreCase("rgb_cct")) {
    return &MilightRgbCctConfig;
  }
  
  return NULL;
}