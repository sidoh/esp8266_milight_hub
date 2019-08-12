#ifndef _GROUP_STATE_FIELDS_H
#define _GROUP_STATE_FIELDS_H

namespace GroupStateFieldNames {
  static const char UNKNOWN[] = "unknown";
  static const char STATE[] = "state";
  static const char STATUS[] = "status";
  static const char BRIGHTNESS[] = "brightness";
  static const char LEVEL[] = "level";
  static const char HUE[] = "hue";
  static const char SATURATION[] = "saturation";
  static const char COLOR[] = "color";
  static const char MODE[] = "mode";
  static const char KELVIN[] = "kelvin";
  static const char TEMPERATURE[] = "temperature"; //alias for kelvin
  static const char COLOR_TEMP[] = "color_temp";
  static const char BULB_MODE[] = "bulb_mode";
  static const char COMPUTED_COLOR[] = "computed_color";
  static const char EFFECT[] = "effect";
  static const char DEVICE_ID[] = "device_id";
  static const char GROUP_ID[] = "group_id";
  static const char DEVICE_TYPE[] = "device_type";
  static const char OH_COLOR[] = "oh_color";
  static const char HEX_COLOR[] = "hex_color";
  static const char COMMAND[] = "command";
  static const char COMMANDS[] = "commands";
};

enum class GroupStateField {
  UNKNOWN,
  STATE,
  STATUS,
  BRIGHTNESS,
  LEVEL,
  HUE,
  SATURATION,
  COLOR,
  MODE,
  KELVIN,
  COLOR_TEMP,
  BULB_MODE,
  COMPUTED_COLOR,
  EFFECT,
  DEVICE_ID,
  GROUP_ID,
  DEVICE_TYPE,
  OH_COLOR,
  HEX_COLOR
};

class GroupStateFieldHelpers {
public:
  static const char* getFieldName(GroupStateField field);
  static GroupStateField getFieldByName(const char* name);
  static bool isBrightnessField(GroupStateField field);
};

#endif
