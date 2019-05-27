#ifndef _GROUP_STATE_FIELDS_H
#define _GROUP_STATE_FIELDS_H

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
  OH_COLOR
};

class GroupStateFieldHelpers {
public:
  static const char* getFieldName(GroupStateField field);
  static GroupStateField getFieldByName(const char* name);
};

#endif
