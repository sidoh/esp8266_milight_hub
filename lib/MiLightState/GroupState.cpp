#include <GroupState.h>
#include <Units.h>
#include <MiLightRemoteConfig.h>
#include <RGBConverter.h>

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
uint16_t GroupState::getHue() const {
  return Units::rescale<uint16_t, uint16_t>(state.fields._hue, 360, 255);
}
void GroupState::setHue(uint16_t hue) {
  setDirty();
  state.fields._isSetHue = 1;
  state.fields._hue = Units::rescale<uint16_t, uint16_t>(hue, 255, 360);
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
uint16_t GroupState::getMireds() const {
  return Units::whiteValToMireds(getKelvin(), 100);
}
void GroupState::setKelvin(uint8_t kelvin) {
  setDirty();
  state.fields._isSetKelvin = 1;
  state.fields._kelvin = kelvin;
}
void GroupState::setMireds(uint16_t mireds) {
  setKelvin(Units::miredsToWhiteVal(mireds, 100));
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
    stream.readBytes(reinterpret_cast<uint8_t*>(&state.data[i]), 4);
  }
  clearDirty();
}

void GroupState::dump(Stream& stream) const {
  for (size_t i = 0; i < DATA_BYTES; i++) {
    stream.write(reinterpret_cast<const uint8_t*>(&state.data[i]), 4);
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
    setHue(state["hue"]);
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
    setMireds(state["color_temp"]);
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
  if (isSetState()) {
    partialState["state"] = getState() == ON ? "ON" : "OFF";
  }
  if (isSetBrightness()) {
    partialState["brightness"] = Units::rescale(getBrightness(), 255, 100);
  }
  if (isSetBulbMode()) {
    partialState["bulb_mode"] = BULB_MODE_NAMES[getBulbMode()];

    if (getBulbMode() == BULB_MODE_COLOR) {
      if (isSetHue() && isSetSaturation()) {
        uint8_t rgb[3];
        RGBConverter converter;
        converter.hsvToRgb(getHue()/360.0, getSaturation()/100.0, 1, rgb);
        JsonObject& color = partialState.createNestedObject("color");
        color["r"] = rgb[0];
        color["g"] = rgb[1];
        color["b"] = rgb[2];
      } else if (isSetHue()) {
        partialState["hue"] = getHue();
      } else if (isSetSaturation()) {
        partialState["saturation"] = getSaturation();
      }
    } else if (getBulbMode() == BULB_MODE_SCENE) {
      if (isSetMode()) {
        partialState["mode"] = getMode();
      }
    } else if (getBulbMode() == BULB_MODE_WHITE) {
      if (isSetKelvin()) {
        partialState["color_temp"] = getMireds();
      }
    }
  }
}
