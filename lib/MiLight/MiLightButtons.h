#ifndef _MILIGHT_BUTTONS
#define _MILIGHT_BUTTONS 

enum MiLightRadioType {
  UNKNOWN = 0,
  RGBW  = 0xB8,
  CCT   = 0x5A,
  RGB_CCT = 0x20,
  RGB = 0xA4
};

enum MiLightStatus { 
  ON = 0, 
  OFF = 1 
};

#endif