#ifndef _MILIGHT_BUTTONS
#define _MILIGHT_BUTTONS

enum MiLightRemoteType {
  REMOTE_TYPE_UNKNOWN = 255,
  REMOTE_TYPE_RGBW    = 0,
  REMOTE_TYPE_CCT     = 1,
  REMOTE_TYPE_RGB_CCT = 2,
  REMOTE_TYPE_RGB     = 3,
  REMOTE_TYPE_FUT089  = 4,
  REMOTE_TYPE_FUT091  = 5
};

enum MiLightStatus {
  ON = 0,
  OFF = 1
};

#endif
