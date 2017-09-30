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
  state.fields._state                = 0;
  state.fields._brightness           = 0;
  state.fields._brightnessColor      = 0;
  state.fields._brightnessMode       = 0;
  state.fields._hue                  = 0;
  state.fields._saturation           = 0;
  state.fields._mode                 = 0;
  state.fields._bulbMode             = 0;
  state.fields._kelvin               = 0;
  state.fields._isSetState           = 0;
  state.fields._isSetHue             = 0;
  state.fields._isSetBrightness      = 0;
  state.fields._isSetBrightnessColor = 0;
  state.fields._isSetBrightnessMode  = 0;
  state.fields._isSetSaturation      = 0;
  state.fields._isSetMode            = 0;
  state.fields._isSetKelvin          = 0;
  state.fields._isSetBulbMode        = 0;
  state.fields._dirty                = 1;
}

bool GroupState::isSetState() const { return state.fields._isSetState; }
MiLightStatus GroupState::getState() const { return state.fields._state ? ON : OFF; }
void GroupState::setState(const MiLightStatus status) {
  setDirty();
  state.fields._isSetState = 1;
  state.fields._state = status == ON ? 1 : 0;
}

bool GroupState::isSetBrightness() const {
  if (! state.fields._isSetBulbMode) {
    return state.fields._isSetBrightness;
  }

  switch (state.fields._bulbMode) {
    case BULB_MODE_WHITE:
      return state.fields._isSetBrightness;
    case BULB_MODE_COLOR:
      return state.fields._isSetBrightnessColor;
    case BULB_MODE_SCENE:
      return state.fields._isSetBrightnessMode;
  }

  return false;
}

uint8_t GroupState::getBrightness() const {
  switch (state.fields._bulbMode) {
    case BULB_MODE_WHITE:
      return state.fields._brightness;
    case BULB_MODE_COLOR:
      return state.fields._brightnessColor;
    case BULB_MODE_SCENE:
      return state.fields._brightnessMode;
  }

  return 0;
}

void GroupState::setBrightness(uint8_t brightness) {
  setDirty();

  uint8_t bulbMode = state.fields._bulbMode;
  if (! state.fields._isSetBulbMode) {
    bulbMode = BULB_MODE_WHITE;
  }

  switch (bulbMode) {
    case BULB_MODE_WHITE:
      state.fields._isSetBrightness = 1;
      state.fields._brightness = brightness;
      break;
    case BULB_MODE_COLOR:
      state.fields._isSetBrightnessColor = 1;
      state.fields._brightnessColor = brightness;
      break;
    case BULB_MODE_SCENE:
      state.fields._isSetBrightnessMode = 1;
      state.fields._brightnessMode = brightness;
  }
}

bool GroupState::isSetHue() const { return state.fields._isSetHue; }
uint8_t GroupState::getHue() const { return state.fields._hue; }
void GroupState::setHue(uint8_t hue) {
  setDirty();
  state.fields._isSetHue = 1;
  state.fields._hue = hue;
}

bool GroupState::isSetSaturation() const { return state.fields._isSetSaturation; }
uint8_t GroupState::getSaturation() const { return state.fields._saturation; }
void GroupState::setSaturation(uint8_t saturation) {
  setDirty();
  state.fields._isSetSaturation = 1;
  state.fields._saturation = saturation;
}

bool GroupState::isSetMode() const { return state.fields._isSetMode; }
uint8_t GroupState::getMode() const { return state.fields._mode; }
void GroupState::setMode(uint8_t mode) {
  setDirty();
  state.fields._isSetMode = 1;
  state.fields._mode = mode;
}

bool GroupState::isSetKelvin() const { return state.fields._isSetKelvin; }
uint8_t GroupState::getKelvin() const { return state.fields._kelvin; }
void GroupState::setKelvin(uint8_t kelvin) {
  setDirty();
  state.fields._isSetKelvin = 1;
  state.fields._kelvin = kelvin;
}

bool GroupState::isSetBulbMode() const { return state.fields._isSetBulbMode; }
BulbMode GroupState::getBulbMode() const { return static_cast<BulbMode>(state.fields._bulbMode); }
void GroupState::setBulbMode(BulbMode bulbMode) {
  setDirty();
  state.fields._isSetBulbMode = 1;
  state.fields._bulbMode = bulbMode;
}

bool GroupState::isDirty() const { return state.fields._dirty; }
inline bool GroupState::setDirty() { state.fields._dirty = 1; }
bool GroupState::clearDirty() { state.fields._dirty = 0; }

void GroupState::load(Stream& stream) {
  for (size_t i = 0; i < DATA_BYTES; i++) {
    state.data[i] = stream.read();
  }
  clearDirty();
}

void GroupState::dump(Stream& stream) const {
  for (size_t i = 0; i < DATA_BYTES; i++) {
    uint32_t val = state.data[i];
    uint8_t* bytePtr = reinterpret_cast<uint8_t*>(&val);
    stream.write(bytePtr, 4);
  }
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

void GroupState::applyState(JsonObject& partialState) {
  if (state.fields._isSetState) {
    partialState["state"] = getState() == ON ? "ON" : "OFF";
  }
  if (state.fields._isSetBrightness) {
    partialState["brightness"] = Units::rescale(getBrightness(), 255, 100);
  }
  if (state.fields._isSetBulbMode) {
    partialState["bulb_mode"] = BULB_MODE_NAMES[getBulbMode()];

    if (getBulbMode() == BULB_MODE_COLOR) {
      if (state.fields._isSetHue) {
        partialState["hue"] = Units::rescale<uint8_t, uint16_t>(getHue(), 360, 255);
      }
      if (state.fields._isSetSaturation) {
        partialState["saturation"] = getSaturation();
      }
    } else if (getBulbMode() == BULB_MODE_SCENE) {
      if (state.fields._isSetMode) {
        partialState["mode"] = getMode();
      }
    } else if (getBulbMode() == BULB_MODE_WHITE) {
      if (state.fields._isSetKelvin) {
        partialState["color_temp"] = Units::whiteValToMireds(getKelvin(), 100);
      }
    }
  }
}
