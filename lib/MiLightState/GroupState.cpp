#include <GroupState.h>

const GroupState& GroupState::defaultState() {
  static GroupState instance;
  return instance;
}

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
  _on              = 0;
  _brightness      = 0;
  _hue             = 0;
  _saturation      = 0;
  _mode            = 0;
  _bulbMode        = 0;
  _kelvin          = 0;
  _isSetOn         = 0;
  _isSetHue        = 0;
  _isSetBrightness = 0;
  _isSetSaturation = 0;
  _isSetMode       = 0;
  _isSetKelvin     = 0;
  _isSetBulbMode   = 0;
}

bool GroupState::isSetOn() const { return _isSetOn; }
bool GroupState::isOn() const { return _on; }
void GroupState::setOn(bool on) { _on = on; }

bool GroupState::isSetBrightness() const { return _isSetBrightness; }
uint8_t GroupState::getBrightness() const { return _brightness; }
void GroupState::setBrightness(uint8_t brightness) { _brightness = brightness; }

bool GroupState::isSetHue() const { return _isSetHue; }
uint8_t GroupState::getHue() const { return _hue; }
void GroupState::setHue(uint8_t hue) { _hue = hue; }

bool GroupState::isSetSaturation() const { return _isSetSaturation; }
uint8_t GroupState::getSaturation() const { return _saturation; }
void GroupState::setSaturation(uint8_t saturation) { _saturation = saturation; }

bool GroupState::isSetMode() const { return _isSetMode; }
uint8_t GroupState::getMode() const { return _mode; }
void GroupState::setMode(uint8_t mode) { _mode = mode; }

bool GroupState::isSetKelvin() const { return _isSetKelvin; }
uint8_t GroupState::getKelvin() const { return _kelvin; }
void GroupState::setKelvin(uint8_t kelvin) { _kelvin = kelvin; }

bool GroupState::isSetBulbMode() const { return _isSetBulbMode; }
BulbMode GroupState::getBulbMode() const { return static_cast<BulbMode>(_bulbMode); }
void GroupState::setBulbMode(BulbMode bulbMode) { _bulbMode = bulbMode; }
