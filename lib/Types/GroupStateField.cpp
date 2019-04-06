#include <GroupStateField.h>
#include <Size.h>

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
  "device_type",
  "oh_color"
};

GroupStateField GroupStateFieldHelpers::getFieldByName(const char* name) {
  for (size_t i = 0; i < size(STATE_NAMES); i++) {
    if (0 == strcmp(name, STATE_NAMES[i])) {
      return static_cast<GroupStateField>(i);
    }
  }
  return GroupStateField::UNKNOWN;
}

const char* GroupStateFieldHelpers::getFieldName(GroupStateField field) {
  for (size_t i = 0; i < size(STATE_NAMES); i++) {
    if (field == static_cast<GroupStateField>(i)) {
      return STATE_NAMES[i];
    }
  }
  return STATE_NAMES[0];
}
