#include <GroupStateField.h>
#include <Size.h>

static const char* STATE_NAMES[] = {
  GroupStateFieldNames::UNKNOWN,
  GroupStateFieldNames::STATE,
  GroupStateFieldNames::STATUS,
  GroupStateFieldNames::BRIGHTNESS,
  GroupStateFieldNames::LEVEL,
  GroupStateFieldNames::HUE,
  GroupStateFieldNames::SATURATION,
  GroupStateFieldNames::COLOR,
  GroupStateFieldNames::MODE,
  GroupStateFieldNames::KELVIN,
  GroupStateFieldNames::COLOR_TEMP,
  GroupStateFieldNames::BULB_MODE,
  GroupStateFieldNames::COMPUTED_COLOR,
  GroupStateFieldNames::EFFECT,
  GroupStateFieldNames::DEVICE_ID,
  GroupStateFieldNames::GROUP_ID,
  GroupStateFieldNames::DEVICE_TYPE,
  GroupStateFieldNames::OH_COLOR,
  GroupStateFieldNames::HEX_COLOR
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

bool GroupStateFieldHelpers::isBrightnessField(GroupStateField field) {
  switch (field) {
    case GroupStateField::BRIGHTNESS:
    case GroupStateField::LEVEL:
      return true;
    default:
      return false;
  }
}