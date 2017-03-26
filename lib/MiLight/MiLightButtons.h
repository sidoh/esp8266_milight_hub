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

enum MiLightCctButton {
  CCT_ALL_ON            = 0x05,
  CCT_ALL_OFF           = 0x09,
  CCT_GROUP_1_ON        = 0x08,
  CCT_GROUP_1_OFF       = 0x0B,
  CCT_GROUP_2_ON        = 0x0D,
  CCT_GROUP_2_OFF       = 0x03,
  CCT_GROUP_3_ON        = 0x07,
  CCT_GROUP_3_OFF       = 0x0A,
  CCT_GROUP_4_ON        = 0x02,
  CCT_GROUP_4_OFF       = 0x06,
  CCT_BRIGHTNESS_DOWN   = 0x04,
  CCT_BRIGHTNESS_UP     = 0x0C,
  CCT_TEMPERATURE_UP    = 0x0E,
  CCT_TEMPERATURE_DOWN  = 0x0F
};

#endif