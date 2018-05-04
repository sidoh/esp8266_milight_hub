#ifndef _GROUP_STATE_FIELDS_H
#define _GROUP_STATE_FIELDS_H

static const char* STATE_NAMES[] = {
  "unknown",
  "state",
  "status",
  "brightness",
  "level",
  "hue",
  "saturation",
  "color",
  "mode",
  "kelvin",
  "color_temp",
  "bulb_mode",
  "computed_color",
  "effect",
  "device_id",
  "group_id",
  "device_type"
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
  DEVICE_TYPE
};

class GroupStateFieldHelpers {
public:
  static const char* getFieldName(GroupStateField field);
  static GroupStateField getFieldByName(const char* name);
};

#endif
