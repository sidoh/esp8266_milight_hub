#include <GroupState.h>
#include <Units.h>
#include <MiLightRemoteConfig.h>
#include <RGBConverter.h>

const BulbId DEFAULT_BULB_ID;
static const GroupStateField ALL_PHYSICAL_FIELDS[] = {
  GroupStateField::BRIGHTNESS,
  GroupStateField::BULB_MODE,
  GroupStateField::HUE,
  GroupStateField::KELVIN,
  GroupStateField::MODE,
  GroupStateField::SATURATION,
  GroupStateField::STATE
};

// Number of units each increment command counts for
static const uint8_t INCREMENT_COMMAND_VALUE = 10;

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

// determine if now BulbId's are the same.  This compared deviceID (the controller/remote ID) and
// groupId (the group number on the controller, 1-4 or 1-8 depending), but ignores the deviceType
// (type of controller/remote) as this doesn't directly affect the identity of the bulb
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

  scratchpad.fields._isSetBrightnessScratch = 0;
  scratchpad.fields._brightnessScratch      = 0;
  scratchpad.fields._isSetKelvinScratch     = 0;
  scratchpad.fields._kelvinScratch          = 0;
}

GroupState& GroupState::operator=(const GroupState& other) {
  memcpy(state.rawData, other.state.rawData, DATA_LONGS * sizeof(uint32_t));
  scratchpad.rawData = other.scratchpad.rawData;
}

GroupState::GroupState(const GroupState& other) {
  memcpy(state.rawData, other.state.rawData, DATA_LONGS * sizeof(uint32_t));
  scratchpad.rawData = other.scratchpad.rawData;
}

bool GroupState::operator==(const GroupState& other) const {
  return memcmp(state.rawData, other.state.rawData, DATA_LONGS * sizeof(uint32_t)) == 0;
}

bool GroupState::isEqualIgnoreDirty(const GroupState& other) const {
  GroupState meCopy = *this;
  GroupState otherCopy = other;

  meCopy.clearDirty();
  meCopy.clearMqttDirty();
  otherCopy.clearDirty();
  otherCopy.clearMqttDirty();

  return meCopy == otherCopy;
}

void GroupState::print(Stream& stream) const {
  stream.printf("State: %08X %08X\n", state.rawData[0], state.rawData[1]);
}

bool GroupState::isSetField(GroupStateField field) const {
  switch (field) {
    case GroupStateField::COMPUTED_COLOR:
      // Always set -- either send RGB color or white
      return true;
    case GroupStateField::DEVICE_ID:
    case GroupStateField::GROUP_ID:
    case GroupStateField::DEVICE_TYPE:
      // These are always defined
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
    case GroupStateField::EFFECT:
      return isSetEffect();
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

bool GroupState::isSetScratchField(GroupStateField field) const {
  switch (field) {
    case GroupStateField::BRIGHTNESS:
      return scratchpad.fields._isSetBrightnessScratch;
    case GroupStateField::KELVIN:
      return scratchpad.fields._isSetKelvinScratch;
  }

  Serial.print(F("WARNING: tried to check if unknown scratch field was set: "));
  Serial.println(static_cast<unsigned int>(field));

  return false;
}

uint16_t GroupState::getFieldValue(GroupStateField field) const {
  switch (field) {
    case GroupStateField::STATE:
    case GroupStateField::STATUS:
      return getState();
    case GroupStateField::BRIGHTNESS:
      return getBrightness();
    case GroupStateField::HUE:
      return getHue();
    case GroupStateField::SATURATION:
      return getSaturation();
    case GroupStateField::MODE:
      return getMode();
    case GroupStateField::KELVIN:
      return getKelvin();
    case GroupStateField::BULB_MODE:
      return getBulbMode();
  }

  Serial.print(F("WARNING: tried to fetch value for unknown field: "));
  Serial.println(static_cast<unsigned int>(field));

  return 0;
}

uint16_t GroupState::getScratchFieldValue(GroupStateField field) const {
  switch (field) {
    case GroupStateField::BRIGHTNESS:
      return scratchpad.fields._brightnessScratch;
    case GroupStateField::KELVIN:
      return scratchpad.fields._kelvinScratch;
  }

  Serial.print(F("WARNING: tried to fetch value for unknown scratch field: "));
  Serial.println(static_cast<unsigned int>(field));

  return 0;
}

void GroupState::setFieldValue(GroupStateField field, uint16_t value) {
  switch (field) {
    case GroupStateField::STATE:
    case GroupStateField::STATUS:
      setState(static_cast<MiLightStatus>(value));
      break;
    case GroupStateField::BRIGHTNESS:
      setBrightness(value);
      break;
    case GroupStateField::HUE:
      setHue(value);
      break;
    case GroupStateField::SATURATION:
      setSaturation(value);
      break;
    case GroupStateField::MODE:
      setMode(value);
      break;
    case GroupStateField::KELVIN:
      setKelvin(value);
      break;
    case GroupStateField::BULB_MODE:
      setBulbMode(static_cast<BulbMode>(value));
      break;
    default:
      Serial.print(F("WARNING: tried to set value for unknown field: "));
      Serial.println(static_cast<unsigned int>(field));
      break;
  }
}

void GroupState::setScratchFieldValue(GroupStateField field, uint16_t value) {
  switch (field) {
    case GroupStateField::BRIGHTNESS:
      scratchpad.fields._isSetBrightnessScratch = 1;
      scratchpad.fields._brightnessScratch = value;
      break;
    case GroupStateField::KELVIN:
      scratchpad.fields._isSetKelvinScratch = 1;
      scratchpad.fields._kelvinScratch = value;
      break;
    default:
      Serial.print(F("WARNING: tried to set value for unknown scratch field: "));
      Serial.println(static_cast<unsigned int>(field));
      break;
  }
}

bool GroupState::isSetState() const { return state.fields._isSetState; }
MiLightStatus GroupState::getState() const { return state.fields._state ? ON : OFF; }
bool GroupState::isOn() const {
  return !isNightMode() && (!isSetState() || getState() == MiLightStatus::ON);
}
bool GroupState::setState(const MiLightStatus status) {
  if (!isNightMode() && isSetState() && getState() == status) {
    return false;
  }

  setDirty();
  state.fields._isSetState = 1;
  state.fields._state = status == ON ? 1 : 0;

  // Changing status will clear night mode
  setNightMode(false);

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
bool GroupState::isSetEffect() const {
  // only BULB_MODE_COLOR does not have an effect.
  return isSetBulbMode() && getBulbMode() != BULB_MODE_COLOR;
}
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
  for (size_t i = 0; i < DATA_LONGS; i++) {
    stream.readBytes(reinterpret_cast<uint8_t*>(&state.rawData[i]), 4);
  }
  clearDirty();
}

void GroupState::dump(Stream& stream) const {
  for (size_t i = 0; i < DATA_LONGS; i++) {
    stream.write(reinterpret_cast<const uint8_t*>(&state.rawData[i]), 4);
  }
}

bool GroupState::applyIncrementCommand(GroupStateField field, IncrementDirection dir) {
  if (field != GroupStateField::KELVIN && field != GroupStateField::BRIGHTNESS) {
    Serial.print(F("WARNING: tried to apply increment for unsupported field: "));
    Serial.println(static_cast<uint8_t>(field));
    return false;
  }

  int8_t dirValue = static_cast<int8_t>(dir);

  // If there's already a known value, update it
  if (isSetField(field)) {
    int8_t currentValue = static_cast<int8_t>(getFieldValue(field));
    int8_t newValue = currentValue + (dirValue * INCREMENT_COMMAND_VALUE);

#ifdef STATE_DEBUG
    debugState("Updating field from increment command");
#endif

    // For now, assume range for both brightness and kelvin is [0, 100]
    setFieldValue(field, constrain(newValue, 0, 100));
    return true;
  // Otherwise start or update scratch state
  } else {
    if (isSetScratchField(field)) {
      int8_t newValue = static_cast<int8_t>(getScratchFieldValue(field)) + dirValue;

      if (newValue == 0 || newValue == 10) {
        setFieldValue(field, newValue * INCREMENT_COMMAND_VALUE);
        return true;
      } else {
        setScratchFieldValue(field, newValue);
      }
    } else if (dir == IncrementDirection::DECREASE) {
      setScratchFieldValue(field, 9);
    } else {
      setScratchFieldValue(field, 1);
    }

#ifdef STATE_DEBUG
    Serial.print(F("Updated scratch field: "));
    Serial.print(static_cast<int8_t>(field));
    Serial.print(F(" to: "));
    Serial.println(getScratchFieldValue(field));
#endif
  }

  return false;
}

bool GroupState::patch(const GroupState& other) {
  for (size_t i = 0; i < size(ALL_PHYSICAL_FIELDS); ++i) {
    GroupStateField field = ALL_PHYSICAL_FIELDS[i];

    if (other.isSetField(field)) {
      setFieldValue(field, other.getFieldValue(field));
    }
  }
}

/*
  Update group state to reflect a packet state

  Called both when a packet is sent locally, and when an intercepted packet is read
  (see main.cpp onPacketSentHandler)

  Returns true if the packet changes affects a state change
*/
bool GroupState::patch(const JsonObject& state) {
  bool changes = false;

#ifdef STATE_DEBUG
  Serial.print(F("Patching existing state with: "));
  state.printTo(Serial);
  Serial.println();
#endif

  if (state.containsKey("state")) {
    bool stateChange = setState(state["state"] == "ON" ? ON : OFF);
    changes |= stateChange;
  }

  // Devices do not support changing their state while off, so don't apply state
  // changes to devices we know are off.

  if (isOn() && state.containsKey("brightness")) {
    bool stateChange = setBrightness(Units::rescale(state.get<uint8_t>("brightness"), 100, 255));
    changes |= stateChange;
  }
  if (isOn() && state.containsKey("hue")) {
    changes |= setHue(state["hue"]);
    changes |= setBulbMode(BULB_MODE_COLOR);
  }
  if (isOn() && state.containsKey("saturation")) {
    changes |= setSaturation(state["saturation"]);
  }
  if (isOn() && state.containsKey("mode")) {
    changes |= setMode(state["mode"]);
    changes |= setBulbMode(BULB_MODE_SCENE);
  }
  if (isOn() && state.containsKey("color_temp")) {
    changes |= setMireds(state["color_temp"]);
    changes |= setBulbMode(BULB_MODE_WHITE);
  }

  if (state.containsKey("command")) {
    const String& command = state["command"];

    if (isOn() && command == "set_white") {
      changes |= setBulbMode(BULB_MODE_WHITE);
    } else if (command == "night_mode") {
      changes |= setBulbMode(BULB_MODE_NIGHT);
    } else if (isOn() && command == "brightness_up") {
      changes |= applyIncrementCommand(GroupStateField::BRIGHTNESS, IncrementDirection::INCREASE);
    } else if (isOn() && command == "brightness_down") {
      changes |= applyIncrementCommand(GroupStateField::BRIGHTNESS, IncrementDirection::DECREASE);
    } else if (isOn() && command == "temperature_up") {
      changes |= applyIncrementCommand(GroupStateField::KELVIN, IncrementDirection::INCREASE);
    } else if (isOn() && command == "temperature_down") {
      changes |= applyIncrementCommand(GroupStateField::KELVIN, IncrementDirection::DECREASE);
    }
  }

  if (changes) {
    debugState("GroupState::patch: State changed");
  }
  else {
    debugState("GroupState::patch: State not changed");
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

// gather partial state for a single field; see GroupState::applyState to gather many fields
void GroupState::applyField(JsonObject& partialState, const BulbId& bulbId, GroupStateField field) {
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

      case GroupStateField::EFFECT:
        if (getBulbMode() == BULB_MODE_SCENE) {
          partialState["effect"] = String(getMode());
        } else if (getBulbMode() == BULB_MODE_WHITE) {
          partialState["effect"] = "white_mode";
        } else if (getBulbMode() == BULB_MODE_NIGHT) {
          partialState["effect"] = "night_mode";
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

      case GroupStateField::DEVICE_ID:
        partialState["device_id"] = bulbId.deviceId;
        break;

      case GroupStateField::GROUP_ID:
        partialState["group_id"] = bulbId.groupId;
        break;

      case GroupStateField::DEVICE_TYPE:
        const MiLightRemoteConfig* remoteConfig = MiLightRemoteConfig::fromType(bulbId.deviceType);
        if (remoteConfig) {
          partialState["device_type"] = remoteConfig->name;
        }
        break;
    }
  }
}

// helper function to debug the current state (in JSON) to the serial port
void GroupState::debugState(char const *debugMessage) {
#ifdef STATE_DEBUG
  // using static to keep large buffers off the call stack
  static StaticJsonBuffer<500> jsonBuffer;

  // define fields to show (if count changes, make sure to update count to applyState below)
  GroupStateField fields[] {
      GroupStateField::BRIGHTNESS,
      GroupStateField::BULB_MODE,
      GroupStateField::COLOR,
      GroupStateField::COLOR_TEMP,
      GroupStateField::COMPUTED_COLOR,
      GroupStateField::EFFECT,
      GroupStateField::HUE,
      GroupStateField::KELVIN,
      GroupStateField::LEVEL,
      GroupStateField::MODE,
      GroupStateField::SATURATION,
      GroupStateField::STATE,
      GroupStateField::STATUS };

  // since our buffer is reused, make sure to clear it every time
  jsonBuffer.clear();
  JsonObject& jsonState = jsonBuffer.createObject();

  // Fake id
  BulbId id;

  // use applyState to build JSON of all fields (from above)
  applyState(jsonState, id, fields, 13);
  // convert to string and print
  Serial.printf("%s: ", debugMessage);
  jsonState.printTo(Serial);
  Serial.println("");
#endif
}

// build up a partial state representation based on the specified GrouipStateField array.  Used
// to gather a subset of states (configurable in the UI) for sending to MQTT and web responses.
void GroupState::applyState(JsonObject& partialState, const BulbId& bulbId, GroupStateField* fields, size_t numFields) {
  for (size_t i = 0; i < numFields; i++) {
    applyField(partialState, bulbId, fields[i]);
  }
}
