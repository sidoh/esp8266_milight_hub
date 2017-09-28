#include <GroupState.h>
#include <Units.h>
#include <MiLightRemoteConfig.h>

const GroupState& GroupState::defaultState(MiLightRemoteType remoteType) {
  static GroupState instances[MiLightRemoteConfig::NUM_REMOTES];
  GroupState& state = instances[remoteType];

  switch (remoteType) {
    case REMOTE_TYPE_RGB:
      state.setBulbMode(BULB_MODE_COLOR);
      break;
    case REMOTE_TYPE_CCT:
      state.setBulbMode(BULB_MODE_WHITE);
      break;
  }

  return state;
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
  _state                = 0;
  _brightness           = 0;
  _brightnessColor      = 0;
  _brightnessMode       = 0;
  _hue                  = 0;
  _saturation           = 0;
  _mode                 = 0;
  _bulbMode             = 0;
  _kelvin               = 0;
  _isSetState           = 0;
  _isSetHue             = 0;
  _isSetBrightness      = 0;
  _isSetBrightnessColor = 0;
  _isSetBrightnessMode  = 0;
  _isSetSaturation      = 0;
  _isSetMode            = 0;
  _isSetKelvin          = 0;
  _isSetBulbMode        = 0;
}

bool GroupState::isSetState() const { return _isSetState; }
MiLightStatus GroupState::getState() const { return _state ? ON : OFF; }
void GroupState::setState(const MiLightStatus state) {
  _isSetState = 1;
  _state = state == ON ? 1 : 0;
}

bool GroupState::isSetBrightness() const {
  if (! _isSetBulbMode) {
    return _isSetBrightness;
  }

  switch (_bulbMode) {
    case BULB_MODE_WHITE:
      return _isSetBrightness;
    case BULB_MODE_COLOR:
      return _isSetBrightnessColor;
    case BULB_MODE_SCENE:
      return _isSetBrightnessMode;
  }

  return false;
}

uint8_t GroupState::getBrightness() const {
  switch (_bulbMode) {
    case BULB_MODE_WHITE:
      return _brightness;
    case BULB_MODE_COLOR:
      return _brightnessColor;
    case BULB_MODE_SCENE:
      return _brightnessMode;
  }

  return 0;
}

void GroupState::setBrightness(uint8_t brightness) {
  uint8_t bulbMode = _bulbMode;
  if (! _isSetBulbMode) {
    bulbMode = BULB_MODE_WHITE;
  }

  switch (bulbMode) {
    case BULB_MODE_WHITE:
      _isSetBrightness = 1;
      _brightness = brightness;
      break;
    case BULB_MODE_COLOR:
      _isSetBrightnessColor = 1;
      _brightnessColor = brightness;
      break;
    case BULB_MODE_SCENE:
      _isSetBrightnessMode = 1;
      _brightnessMode = brightness;
  }
}

bool GroupState::isSetHue() const { return _isSetHue; }
uint8_t GroupState::getHue() const { return _hue; }
void GroupState::setHue(uint8_t hue) {
  _isSetHue = 1;
  _hue = hue;
}

bool GroupState::isSetSaturation() const { return _isSetSaturation; }
uint8_t GroupState::getSaturation() const { return _saturation; }
void GroupState::setSaturation(uint8_t saturation) {
  _isSetSaturation = 1;
  _saturation = saturation;
}

bool GroupState::isSetMode() const { return _isSetMode; }
uint8_t GroupState::getMode() const { return _mode; }
void GroupState::setMode(uint8_t mode) {
  _isSetMode = 1;
  _mode = mode;
}

bool GroupState::isSetKelvin() const { return _isSetKelvin; }
uint8_t GroupState::getKelvin() const { return _kelvin; }
void GroupState::setKelvin(uint8_t kelvin) {
  _isSetKelvin = 1;
  _kelvin = kelvin;
}

bool GroupState::isSetBulbMode() const { return _isSetBulbMode; }
BulbMode GroupState::getBulbMode() const { return static_cast<BulbMode>(_bulbMode); }
void GroupState::setBulbMode(BulbMode bulbMode) {
  _isSetBulbMode = 1;
  _bulbMode = bulbMode;
}

void GroupState::patch(const JsonObject& state) {
  if (state.containsKey("state")) {
    setState(state["state"] == "ON" ? ON : OFF);
  }
  if (state.containsKey("brightness")) {
    setBrightness(Units::rescale(state.get<uint8_t>("brightness"), 100, 255));
  }
  if (state.containsKey("hue")) {
    setHue(Units::rescale<uint8_t, uint16_t>(state["hue"], 255, 360));
    setBulbMode(BULB_MODE_COLOR);
  }
  if (state.containsKey("saturation")) {
    setSaturation(state["saturation"]);
  }
  if (state.containsKey("mode")) {
    setMode(state["mode"]);
    setBulbMode(BULB_MODE_SCENE);
  }
  if (state.containsKey("color_temp")) {
    setKelvin(Units::miredsToWhiteVal(state["color_temp"], 100));
    setBulbMode(BULB_MODE_WHITE);
  }
  if (state.containsKey("command")) {
    const String& command = state["command"];

    if (command == "white_mode") {
      setBulbMode(BULB_MODE_WHITE);
    } else if (command == "night_mode") {
      setBulbMode(BULB_MODE_NIGHT);
    }
  }
}

void GroupState::applyState(JsonObject& state) {
  if (_isSetState) {
    state["state"] = getState() == ON ? "ON" : "OFF";
  }
  if (_isSetBrightness) {
    state["brightness"] = Units::rescale(getBrightness(), 255, 100);
  }
  if (_isSetBulbMode) {
    state["bulb_mode"] = BULB_MODE_NAMES[getBulbMode()];

    if (getBulbMode() == BULB_MODE_COLOR) {
      if (_isSetHue) {
        state["hue"] = Units::rescale<uint8_t, uint16_t>(getHue(), 360, 255);
      }
      if (_isSetSaturation) {
        state["saturation"] = getSaturation();
      }
    } else if (getBulbMode() == BULB_MODE_SCENE) {
      if (_isSetMode) {
        state["mode"] = getMode();
      }
    } else if (getBulbMode() == BULB_MODE_WHITE) {
      if (_isSetKelvin) {
        state["color_temp"] = Units::whiteValToMireds(getKelvin(), 100);
      }
    }
  }
}
