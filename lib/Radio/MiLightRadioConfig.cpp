#include <MiLightRadioConfig.h>

MiLightRadioConfig MiLightRadioConfig::ALL_CONFIGS[] = {
  MiLightRadioConfig(0x147A, 0x258B, 7, 9, 40, 71, 0xAA, 0x05), // rgbw
  MiLightRadioConfig(0x050A, 0x55AA, 7, 4, 39, 74, 0xAA, 0x05), // cct
  MiLightRadioConfig(0x7236, 0x1809, 9, 8, 39, 70, 0xAA, 0x05), // rgb+cct, fut089
  MiLightRadioConfig(0x9AAB, 0xBCCD, 6, 3, 38, 73, 0x55, 0x0A), // rgb
  MiLightRadioConfig(0x50A0, 0xAA55, 6, 6, 41, 76, 0xAA, 0x0A)  // FUT020
};
