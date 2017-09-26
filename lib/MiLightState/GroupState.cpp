#include <GroupState.h>

GroupId::GroupId()
  : deviceId(0),
    groupId(0),
    deviceType(REMOTE_TYPE_UNKNOWN)
{ }

GroupId::GroupId(const GroupId &other)
  : deviceId(other.deviceId),
    groupId(other.groupId),
    deviceType(other.deviceType)
{ }

GroupId::GroupId(
  const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType
)
  : deviceId(deviceId),
    groupId(groupId),
    deviceType(deviceType)
{ }

void GroupId::operator=(const GroupId &other) {
  deviceId = other.deviceId;
  groupId = other.groupId;
  deviceType = other.deviceType;
}

bool GroupId::operator==(const GroupId &other) {
  return deviceId == other.deviceId
    && groupId == other.groupId
    && deviceType == other.deviceType;
}

GroupState::GroupState() {
  _on         = 0;
  _brightness = 0;
  _hue        = 0;
  _saturation = 0;
  _mode       = 0;
  _bulbMode   = 0;
  _kelvin     = 0;
}

bool GroupState::isOn() { return _on; }
void GroupState::setOn(bool on) { _on = on; }

uint8_t GroupState::getBrightness() const { return _brightness; }
void GroupState::setBrightness(uint8_t brightness) { _brightness = brightness; }

uint8_t GroupState::getHue() { return _hue; }
void GroupState::setHue(uint8_t hue) { _hue = hue; }

uint8_t GroupState::getSaturation() { return _saturation; }
void GroupState::setSaturation(uint8_t saturation) { _saturation = saturation; }

uint8_t GroupState::getMode() { return _mode; }
void GroupState::setMode(uint8_t mode) { _mode = mode; }

uint8_t GroupState::getKelvin() { return _kelvin; }
void GroupState::setKelvin(uint8_t kelvin) { _kelvin = kelvin; }

BulbMode GroupState::getBulbMode() { return static_cast<BulbMode>(_bulbMode); }
void GroupState::setBulbMode(BulbMode bulbMode) { _bulbMode = bulbMode; }
