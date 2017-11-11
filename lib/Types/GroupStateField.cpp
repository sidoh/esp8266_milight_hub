#include <GroupStateField.h>
#include <Size.h>

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
