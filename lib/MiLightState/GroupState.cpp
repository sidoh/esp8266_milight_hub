#include <GroupState.h>
#include <Units.h>
#include <MiLightRemoteConfig.h>
#include <RGBConverter.h>

const BulbId DEFAULT_BULB_ID;

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

BulbId::BulbId()
  : deviceId(0),
    groupId(0),
    deviceType(REMOTE_TYPE_UNKNOWN)
{ }

BulbId::BulbId(const BulbId &other)
  : deviceId(other.deviceId),
    groupId(other.groupId),
    deviceType(other.deviceType)
{ }

BulbId::BulbId(
  const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType
)
  : deviceId(deviceId),
    groupId(groupId),
    deviceType(deviceType)
{ }

void BulbId::operator=(const BulbId &other) {
  deviceId = other.deviceId;
  groupId = other.groupId;
  deviceType = other.deviceType;
}

bool BulbId::operator==(const BulbId &other) {
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
  state.fields._mqttDirty            = 0;
  state.fields._isSetNightMode       = 0;
  state.fields._isNightMode          = 0;
}

bool GroupState::isSetField(GroupStateField field) const {
  switch (field) {
    case GroupStateField::COMPUTED_COLOR:
      // Always set -- either send RGB color or white
      return true;
    case GroupStateField::STATE:
    case GroupStateField::STATUS:
      return isSetState();
    case GroupStateField::BRIGHTNESS:
    case GroupStateField::LEVEL:
      return isSetBrightness();
    case GroupStateField::COLOR:
    case GroupStateField::HUE:
      return isSetHue();
    case GroupStateField::SATURATION:
      return isSetSaturation();
    case GroupStateField::MODE:
      return isSetMode();
    case GroupStateField::KELVIN:
    case GroupStateField::COLOR_TEMP:
      return isSetKelvin();
    case GroupStateField::BULB_MODE:
      return isSetBulbMode();
  }

  Serial.print(F("WARNING: tried to check if unknown field was set: "));
  Serial.println(static_cast<unsigned int>(field));

  return false;
}

bool GroupState::isSetState() const { return state.fields._isSetState; }
MiLightStatus GroupState::getState() const { return state.fields._state ? ON : OFF; }
bool GroupState::setState(const MiLightStatus status) {
  if (isSetState() && getState() == status) {
    return false;
  }

  setDirty();
  state.fields._isSetState = 1;
  state.fields._state = status == ON ? 1 : 0;

  return true;
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
bool GroupState::setBrightness(uint8_t brightness) {
  if (isSetBrightness() && getBrightness() == brightness) {
    return false;
  }

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
    default:
      return false;
  }

  return true;
}

bool GroupState::isSetHue() const { return state.fields._isSetHue; }
uint16_t GroupState::getHue() const {
  return Units::rescale<uint16_t, uint16_t>(state.fields._hue, 360, 255);
}
bool GroupState::setHue(uint16_t hue) {
  if (isSetHue() && getHue() == hue) {
    return false;
  }

  setDirty();
  state.fields._isSetHue = 1;
  state.fields._hue = Units::rescale<uint16_t, uint16_t>(hue, 255, 360);

  return true;
}

bool GroupState::isSetSaturation() const { return state.fields._isSetSaturation; }
uint8_t GroupState::getSaturation() const { return state.fields._saturation; }
bool GroupState::setSaturation(uint8_t saturation) {
  if (isSetSaturation() && getSaturation() == saturation) {
    return false;
  }

  setDirty();
  state.fields._isSetSaturation = 1;
  state.fields._saturation = saturation;

  return true;
}

bool GroupState::isSetMode() const { return state.fields._isSetMode; }
uint8_t GroupState::getMode() const { return state.fields._mode; }
bool GroupState::setMode(uint8_t mode) {
  if (isSetMode() && getMode() == mode) {
    return false;
  }

  setDirty();
  state.fields._isSetMode = 1;
  state.fields._mode = mode;

  return true;
}

bool GroupState::isSetKelvin() const { return state.fields._isSetKelvin; }
uint8_t GroupState::getKelvin() const { return state.fields._kelvin; }
uint16_t GroupState::getMireds() const {
  return Units::whiteValToMireds(getKelvin(), 100);
}
bool GroupState::setKelvin(uint8_t kelvin) {
  if (isSetKelvin() && getKelvin() == kelvin) {
    return false;
  }

  setDirty();
  state.fields._isSetKelvin = 1;
  state.fields._kelvin = kelvin;

  return true;
}
bool GroupState::setMireds(uint16_t mireds) {
  return setKelvin(Units::miredsToWhiteVal(mireds, 100));
}

bool GroupState::isSetBulbMode() const { return state.fields._isSetBulbMode; }
BulbMode GroupState::getBulbMode() const {
  BulbMode mode;

  // Night mode is a transient state.  When power is toggled, the bulb returns
  // to the state it was last in.  To handle this case, night mode state is
  // stored separately.
  if (isSetNightMode() && isNightMode()) {
    return BULB_MODE_NIGHT;
  } else {
    return static_cast<BulbMode>(state.fields._bulbMode);
  }
}
bool GroupState::setBulbMode(BulbMode bulbMode) {
  if (isSetBulbMode() && getBulbMode() == bulbMode) {
    return false;
  }

  setDirty();

  // As mentioned in isSetBulbMode, NIGHT_MODE is stored separately.
  if (bulbMode == BULB_MODE_NIGHT) {
    setNightMode(true);
  } else {
    state.fields._isSetBulbMode = 1;
    state.fields._bulbMode = bulbMode;
  }

  return true;
}

bool GroupState::isSetNightMode() const { return state.fields._isSetNightMode; }
bool GroupState::isNightMode() const { return state.fields._isNightMode; }
bool GroupState::setNightMode(bool nightMode) {
  if (isSetNightMode() && isNightMode() == nightMode) {
    return false;
  }

  setDirty();
  state.fields._isSetNightMode = 1;
  state.fields._isNightMode = nightMode;

  return true;
}

bool GroupState::isDirty() const { return state.fields._dirty; }
inline bool GroupState::setDirty() {
  state.fields._dirty = 1;
  state.fields._mqttDirty = 1;
}
bool GroupState::clearDirty() { state.fields._dirty = 0; }

bool GroupState::isMqttDirty() const { return state.fields._mqttDirty; }
bool GroupState::clearMqttDirty() { state.fields._mqttDirty = 0; }

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

bool GroupState::patch(const JsonObject& state) {
  bool changes = false;

  if (state.containsKey("state")) {
    changes |= setState(state["state"] == "ON" ? ON : OFF);
  }
  if (state.containsKey("brightness")) {
    changes |= setBrightness(Units::rescale(state.get<uint8_t>("brightness"), 100, 255));
  }
  if (state.containsKey("hue")) {
    changes |= setHue(state["hue"]);
    changes |= setBulbMode(BULB_MODE_COLOR);
  }
  if (state.containsKey("saturation")) {
    changes |= setSaturation(state["saturation"]);
  }
  if (state.containsKey("mode")) {
    changes |= setMode(state["mode"]);
    changes |= setBulbMode(BULB_MODE_SCENE);
  }
  if (state.containsKey("color_temp")) {
    changes |= setMireds(state["color_temp"]);
    changes |= setBulbMode(BULB_MODE_WHITE);
  }

  // Any changes other than setting mode to night should take device out of
  // night mode.
  if (changes && getBulbMode() == BULB_MODE_NIGHT) {
    setNightMode(false);
  }

  if (state.containsKey("command")) {
    const String& command = state["command"];

    if (command == "white_mode") {
      changes |= setBulbMode(BULB_MODE_WHITE);
      setNightMode(false);
    } else if (command == "night_mode") {
      changes |= setBulbMode(BULB_MODE_NIGHT);
    }
  }

  return changes;
}

void GroupState::applyColor(ArduinoJson::JsonObject& state) {
  uint8_t rgb[3];
  RGBConverter converter;
  converter.hsvToRgb(
    getHue()/360.0,
    // Default to fully saturated
    (isSetSaturation() ? getSaturation() : 100)/100.0,
    1,
    rgb
  );
  applyColor(state, rgb[0], rgb[1], rgb[2]);
}

void GroupState::applyColor(ArduinoJson::JsonObject& state, uint8_t r, uint8_t g, uint8_t b) {
  JsonObject& color = state.createNestedObject("color");
  color["r"] = r;
  color["g"] = g;
  color["b"] = b;
}

void GroupState::applyField(JsonObject& partialState, GroupStateField field) {
  if (isSetField(field)) {
    switch (field) {
      case GroupStateField::STATE:
      case GroupStateField::STATUS:
        partialState[GroupStateFieldHelpers::getFieldName(field)] = getState() == ON ? "ON" : "OFF";
        break;

      case GroupStateField::BRIGHTNESS:
        partialState["brightness"] = Units::rescale(getBrightness(), 255, 100);
        break;

      case GroupStateField::LEVEL:
        partialState["level"] = getBrightness();
        break;

      case GroupStateField::BULB_MODE:
        partialState["bulb_mode"] = BULB_MODE_NAMES[getBulbMode()];
        break;

      case GroupStateField::COLOR:
        if (getBulbMode() == BULB_MODE_COLOR) {
          applyColor(partialState);
        }
        break;

      case GroupStateField::COMPUTED_COLOR:
        if (getBulbMode() == BULB_MODE_COLOR) {
          applyColor(partialState);
        } else {
          applyColor(partialState, 255, 255, 255);
        }
        break;

      case GroupStateField::HUE:
        if (getBulbMode() == BULB_MODE_COLOR) {
          partialState["hue"] = getHue();
        }
        break;

      case GroupStateField::SATURATION:
        if (getBulbMode() == BULB_MODE_COLOR) {
          partialState["saturation"] = getSaturation();
        }
        break;

      case GroupStateField::MODE:
        if (getBulbMode() == BULB_MODE_SCENE) {
          partialState["mode"] = getMode();
        }
        break;

      case GroupStateField::COLOR_TEMP:
        if (getBulbMode() == BULB_MODE_WHITE) {
          partialState["color_temp"] = getMireds();
        }
        break;

      case GroupStateField::KELVIN:
        if (getBulbMode() == BULB_MODE_WHITE) {
          partialState["kelvin"] = getKelvin();
        }
        break;
    }
  }
}

void GroupState::applyState(JsonObject& partialState, GroupStateField* fields, size_t numFields) {
  for (size_t i = 0; i < numFields; i++) {
    applyField(partialState, fields[i]);
  }
}
