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
  "computed_color"
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
  COMPUTED_COLOR
};

class GroupStateFieldHelpers {
public:
  static const char* getFieldName(GroupStateField field);
  static GroupStateField getFieldByName(const char* name);
};

#endif
