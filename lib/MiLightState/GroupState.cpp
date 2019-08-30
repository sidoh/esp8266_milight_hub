#include <GroupState.h>
#include <Units.h>
#include <MiLightRemoteConfig.h>
#include <RGBConverter.h>
#include <BulbId.h>
#include <MiLightCommands.h>

static const char* BULB_MODE_NAMES[] = {
  "white",
  "color",
  "scene",
  "night"
};

const BulbId DEFAULT_BULB_ID;

const GroupStateField GroupState::ALL_PHYSICAL_FIELDS[] = {
  GroupStateField::BULB_MODE,
  GroupStateField::HUE,
  GroupStateField::KELVIN,
  GroupStateField::MODE,
  GroupStateField::SATURATION,
  GroupStateField::STATE,
  GroupStateField::BRIGHTNESS
};

static const GroupStateField ALL_SCRATCH_FIELDS[] = {
  GroupStateField::BRIGHTNESS,
  GroupStateField::KELVIN
};

// Number of units each increment command counts for
static const uint8_t INCREMENT_COMMAND_VALUE = 10;

static const GroupState DEFAULT_STATE = GroupState();
static const GroupState DEFAULT_RGB_ONLY_STATE = GroupState::initDefaultRgbState();
static const GroupState DEFAULT_WHITE_ONLY_STATE = GroupState::initDefaultWhiteState();

GroupState GroupState::initDefaultRgbState() {
  GroupState state;
  state.setBulbMode(BULB_MODE_COLOR);
  return state;
}

GroupState GroupState::initDefaultWhiteState() {
  GroupState state;
  state.setBulbMode(BULB_MODE_WHITE);
  return state;
}

const GroupState& GroupState::defaultState(MiLightRemoteType remoteType) {
  switch (remoteType) {
    case REMOTE_TYPE_RGB:
      return DEFAULT_RGB_ONLY_STATE;
      break;
    case REMOTE_TYPE_CCT:
    case REMOTE_TYPE_FUT091:
      return DEFAULT_WHITE_ONLY_STATE;
      break;

    default:
      // No modifications needed
      break;
  }

  return DEFAULT_STATE;
}

void GroupState::initFields() {
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
  return *this;
}

GroupState::GroupState()
  : previousState(NULL)
{
  initFields();
}

GroupState::GroupState(const GroupState& other)
  : previousState(NULL)
{
  memcpy(state.rawData, other.state.rawData, DATA_LONGS * sizeof(uint32_t));
  scratchpad.rawData = other.scratchpad.rawData;
}

GroupState::GroupState(const GroupState* previousState, JsonObject jsonState)
  : previousState(previousState)
{
  initFields();

  if (previousState != NULL) {
    this->scratchpad = previousState->scratchpad;
  }

  patch(jsonState);
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

bool GroupState::clearField(GroupStateField field) {
  bool clearedAny = false;

  switch (field) {
    // Always set and can't be cleared
    case GroupStateField::COMPUTED_COLOR:
    case GroupStateField::DEVICE_ID:
    case GroupStateField::GROUP_ID:
    case GroupStateField::DEVICE_TYPE:
      break;

    case GroupStateField::STATE:
    case GroupStateField::STATUS:
      clearedAny = isSetState();
      state.fields._isSetState = 0;
      break;

    case GroupStateField::BRIGHTNESS:
    case GroupStateField::LEVEL:
      clearedAny = clearBrightness();
      break;

    case GroupStateField::COLOR:
    case GroupStateField::HUE:
    case GroupStateField::OH_COLOR:
    case GroupStateField::HEX_COLOR:
      clearedAny = isSetHue();
      state.fields._isSetHue = 0;
      break;

    case GroupStateField::SATURATION:
      clearedAny = isSetSaturation();
      state.fields._isSetSaturation = 0;
      break;

    case GroupStateField::MODE:
    case GroupStateField::EFFECT:
      clearedAny = isSetMode();
      state.fields._isSetMode = 0;
      break;

    case GroupStateField::KELVIN:
    case GroupStateField::COLOR_TEMP:
      clearedAny = isSetKelvin();
      state.fields._isSetKelvin = 0;
      break;

    case GroupStateField::BULB_MODE:
      clearedAny = isSetBulbMode();
      state.fields._isSetBulbMode = 0;

      // Clear brightness as well
      clearedAny = clearBrightness() || clearedAny;
      break;

    default:
      Serial.printf_P(PSTR("Attempted to clear unknown field: %d\n"), static_cast<uint8_t>(field));
      break;
  }

  return clearedAny;
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
    case GroupStateField::OH_COLOR:
    case GroupStateField::HEX_COLOR:
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
    default:
      Serial.print(F("WARNING: tried to check if unknown field was set: "));
      Serial.println(static_cast<unsigned int>(field));
      break;
  }

  return false;
}

bool GroupState::isSetScratchField(GroupStateField field) const {
  switch (field) {
    case GroupStateField::BRIGHTNESS:
      return scratchpad.fields._isSetBrightnessScratch;
    case GroupStateField::KELVIN:
      return scratchpad.fields._isSetKelvinScratch;
    default:
      Serial.print(F("WARNING: tried to check if unknown scratch field was set: "));
      Serial.println(static_cast<unsigned int>(field));
      break;
  }

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
    default:
      Serial.print(F("WARNING: tried to fetch value for unknown field: "));
      Serial.println(static_cast<unsigned int>(field));
      break;
  }

  return 0;
}

uint16_t GroupState::getParsedFieldValue(GroupStateField field) const {
  switch (field) {
    case GroupStateField::LEVEL:
      return getBrightness();
    case GroupStateField::BRIGHTNESS:
      return Units::rescale(getBrightness(), 255, 100);
    case GroupStateField::COLOR_TEMP:
      return getMireds();
    default:
      return getFieldValue(field);
  }
}

uint16_t GroupState::getScratchFieldValue(GroupStateField field) const {
  switch (field) {
    case GroupStateField::BRIGHTNESS:
      return scratchpad.fields._brightnessScratch;
    case GroupStateField::KELVIN:
      return scratchpad.fields._kelvinScratch;
    default:
      Serial.print(F("WARNING: tried to fetch value for unknown scratch field: "));
      Serial.println(static_cast<unsigned int>(field));
      break;
  }

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
  // If we don't know what mode we're in, just assume white mode.  Do this for a few
  // reasons:
  //   * Some bulbs don't have multiple modes
  //   * It's confusing to not have a default
  if (! isSetBulbMode()) {
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
bool GroupState::clearBrightness() {
  bool cleared = false;

  if (!state.fields._isSetBulbMode) {
    cleared = state.fields._isSetBrightness;
    state.fields._isSetBrightness = 0;
  } else {
    switch (state.fields._bulbMode) {
      case BULB_MODE_COLOR:
        cleared = state.fields._isSetBrightnessColor;
        state.fields._isSetBrightnessColor = 0;
        break;

      case BULB_MODE_SCENE:
        cleared = state.fields._isSetBrightnessMode;
        state.fields._isSetBrightnessMode = 0;
        break;

      case BULB_MODE_WHITE:
        cleared = state.fields._isSetBrightness;
        state.fields._isSetBrightness = 0;
        break;
    }
  }

  return cleared;
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

bool GroupState::isSetBulbMode() const {
  return  (isSetNightMode() && isNightMode()) || state.fields._isSetBulbMode;
}
BulbMode GroupState::getBulbMode() const {
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

  return true;
}
bool GroupState::clearDirty() {
  state.fields._dirty = 0;
  return true;
}

bool GroupState::isMqttDirty() const { return state.fields._mqttDirty; }
bool GroupState::clearMqttDirty() {
  state.fields._mqttDirty = 0;
  return true;
}

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
  if (previousState != NULL && previousState->isSetField(field)) {
    int8_t currentValue = static_cast<int8_t>(previousState->getFieldValue(field));
    int8_t newValue = currentValue + (dirValue * INCREMENT_COMMAND_VALUE);

#ifdef STATE_DEBUG
    previousState->debugState("Updating field from increment command");
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

bool GroupState::clearNonMatchingFields(const GroupState& other) {
#ifdef STATE_DEBUG
  this->debugState("Clearing fields.  Current state");
  other.debugState("Other state");
#endif

  bool clearedAny = false;

  for (size_t i = 0; i < size(ALL_PHYSICAL_FIELDS); ++i) {
    GroupStateField field = ALL_PHYSICAL_FIELDS[i];

    if (other.isSetField(field) && isSetField(field) && getFieldValue(field) != other.getFieldValue(field)) {
      if (clearField(field)) {
        clearedAny = true;
      }
    }
  }

#ifdef STATE_DEBUG
  this->debugState("Result");
#endif

  return clearedAny;
}

void GroupState::patch(const GroupState& other) {
#ifdef STATE_DEBUG
  other.debugState("Patching existing state with: ");
  Serial.println();
#endif

  for (size_t i = 0; i < size(ALL_PHYSICAL_FIELDS); ++i) {
    GroupStateField field = ALL_PHYSICAL_FIELDS[i];

    // Handle night mode separately.  Should always set this field.
    if (field == GroupStateField::BULB_MODE && other.isNightMode()) {
      setFieldValue(field, other.getFieldValue(field));
    }
    // Otherwise...
    // Conditions:
    //   * Only set anything if field is set in other state
    //   * Do not patch anything other than STATE if bulb is off
    else if (other.isSetField(field) && (field == GroupStateField::STATE || isOn())) {
      setFieldValue(field, other.getFieldValue(field));
    }
  }

  for (size_t i = 0; i < size(ALL_SCRATCH_FIELDS); ++i) {
    GroupStateField field = ALL_SCRATCH_FIELDS[i];

    // All scratch field updates require that the bulb is on.
    if (isOn() && other.isSetScratchField(field)) {
      setScratchFieldValue(field, other.getScratchFieldValue(field));
    }
  }
}

/*
  Update group state to reflect a packet state

  Called both when a packet is sent locally, and when an intercepted packet is read
  (see main.cpp onPacketSentHandler)

  Returns true if the packet changes affects a state change
*/
bool GroupState::patch(JsonObject state) {
  bool changes = false;

#ifdef STATE_DEBUG
  Serial.print(F("Patching existing state with: "));
  serializeJson(state, Serial);
  Serial.println();
#endif

  if (state.containsKey(GroupStateFieldNames::STATE)) {
    bool stateChange = setState(state[GroupStateFieldNames::STATE] == "ON" ? ON : OFF);
    changes |= stateChange;
  }

  // Devices do not support changing their state while off, so don't apply state
  // changes to devices we know are off.

  if (isOn() && state.containsKey(GroupStateFieldNames::BRIGHTNESS)) {
    bool stateChange = setBrightness(Units::rescale(state[GroupStateFieldNames::BRIGHTNESS].as<uint8_t>(), 100, 255));
    changes |= stateChange;
  }
  if (isOn() && state.containsKey(GroupStateFieldNames::HUE)) {
    changes |= setHue(state[GroupStateFieldNames::HUE]);
    changes |= setBulbMode(BULB_MODE_COLOR);
  }
  if (isOn() && state.containsKey(GroupStateFieldNames::SATURATION)) {
    changes |= setSaturation(state[GroupStateFieldNames::SATURATION]);
  }
  if (isOn() && state.containsKey(GroupStateFieldNames::MODE)) {
    changes |= setMode(state[GroupStateFieldNames::MODE]);
    changes |= setBulbMode(BULB_MODE_SCENE);
  }
  if (isOn() && state.containsKey(GroupStateFieldNames::COLOR_TEMP)) {
    changes |= setMireds(state[GroupStateFieldNames::COLOR_TEMP]);
    changes |= setBulbMode(BULB_MODE_WHITE);
  }

  if (state.containsKey(GroupStateFieldNames::COMMAND)) {
    const String& command = state[GroupStateFieldNames::COMMAND];

    if (isOn() && command == MiLightCommandNames::SET_WHITE) {
      changes |= setBulbMode(BULB_MODE_WHITE);
    } else if (command == MiLightCommandNames::NIGHT_MODE) {
      changes |= setBulbMode(BULB_MODE_NIGHT);
    } else if (isOn() && command == "brightness_up") {
      changes |= applyIncrementCommand(GroupStateField::BRIGHTNESS, IncrementDirection::INCREASE);
    } else if (isOn() && command == "brightness_down") {
      changes |= applyIncrementCommand(GroupStateField::BRIGHTNESS, IncrementDirection::DECREASE);
    } else if (isOn() && command == MiLightCommandNames::TEMPERATURE_UP) {
      changes |= applyIncrementCommand(GroupStateField::KELVIN, IncrementDirection::INCREASE);
      changes |= setBulbMode(BULB_MODE_WHITE);
    } else if (isOn() && command == MiLightCommandNames::TEMPERATURE_DOWN) {
      changes |= applyIncrementCommand(GroupStateField::KELVIN, IncrementDirection::DECREASE);
      changes |= setBulbMode(BULB_MODE_WHITE);
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

void GroupState::applyColor(JsonObject state) const {
  ParsedColor color = getColor();
  applyColor(state, color.r, color.g, color.b);
}

void GroupState::applyColor(JsonObject state, uint8_t r, uint8_t g, uint8_t b) const {
  JsonObject color = state.createNestedObject(GroupStateFieldNames::COLOR);
  color["r"] = r;
  color["g"] = g;
  color["b"] = b;
}

void GroupState::applyOhColor(JsonObject state) const {
  ParsedColor color = getColor();

  char ohColorStr[13];
  sprintf(ohColorStr, "%d,%d,%d", color.r, color.g, color.b);

  state[GroupStateFieldNames::COLOR] = ohColorStr;
}

void GroupState::applyHexColor(JsonObject state) const {
  ParsedColor color = getColor();

  char hexColor[8];
  sprintf(hexColor, "#%02X%02X%02X", color.r, color.g, color.b);

  state[GroupStateFieldNames::COLOR] = hexColor;
}

// gather partial state for a single field; see GroupState::applyState to gather many fields
void GroupState::applyField(JsonObject partialState, const BulbId& bulbId, GroupStateField field) const {
  if (isSetField(field)) {
    switch (field) {
      case GroupStateField::STATE:
      case GroupStateField::STATUS:
        partialState[GroupStateFieldHelpers::getFieldName(field)] = getState() == ON ? "ON" : "OFF";
        break;

      case GroupStateField::BRIGHTNESS:
        partialState[GroupStateFieldNames::BRIGHTNESS] = Units::rescale(getBrightness(), 255, 100);
        break;

      case GroupStateField::LEVEL:
        partialState[GroupStateFieldNames::LEVEL] = getBrightness();
        break;

      case GroupStateField::BULB_MODE:
        partialState[GroupStateFieldNames::BULB_MODE] = BULB_MODE_NAMES[getBulbMode()];
        break;

      case GroupStateField::COLOR:
        if (getBulbMode() == BULB_MODE_COLOR) {
          applyColor(partialState);
        }
        break;

      case GroupStateField::OH_COLOR:
        if (getBulbMode() == BULB_MODE_COLOR) {
          applyOhColor(partialState);
        }
        break;

      case GroupStateField::HEX_COLOR:
        if (getBulbMode() == BULB_MODE_COLOR) {
          applyHexColor(partialState);
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
          partialState[GroupStateFieldNames::HUE] = getHue();
        }
        break;

      case GroupStateField::SATURATION:
        if (getBulbMode() == BULB_MODE_COLOR) {
          partialState[GroupStateFieldNames::SATURATION] = getSaturation();
        }
        break;

      case GroupStateField::MODE:
        if (getBulbMode() == BULB_MODE_SCENE) {
          partialState[GroupStateFieldNames::MODE] = getMode();
        }
        break;

      case GroupStateField::EFFECT:
        if (getBulbMode() == BULB_MODE_SCENE) {
          partialState[GroupStateFieldNames::EFFECT] = String(getMode());
        } else if (isSetBulbMode() && getBulbMode() == BULB_MODE_WHITE) {
          partialState[GroupStateFieldNames::EFFECT] = "white_mode";
        } else if (getBulbMode() == BULB_MODE_NIGHT) {
          partialState[GroupStateFieldNames::EFFECT] = MiLightCommandNames::NIGHT_MODE;
        }
        break;

      case GroupStateField::COLOR_TEMP:
        if (isSetBulbMode() && getBulbMode() == BULB_MODE_WHITE) {
          partialState[GroupStateFieldNames::COLOR_TEMP] = getMireds();
        }
        break;

      case GroupStateField::KELVIN:
        if (isSetBulbMode() && getBulbMode() == BULB_MODE_WHITE) {
          partialState[GroupStateFieldNames::KELVIN] = getKelvin();
        }
        break;

      case GroupStateField::DEVICE_ID:
        partialState[GroupStateFieldNames::DEVICE_ID] = bulbId.deviceId;
        break;

      case GroupStateField::GROUP_ID:
        partialState[GroupStateFieldNames::GROUP_ID] = bulbId.groupId;
        break;

      case GroupStateField::DEVICE_TYPE:
        {
          const MiLightRemoteConfig* remoteConfig = MiLightRemoteConfig::fromType(bulbId.deviceType);
          if (remoteConfig) {
            partialState[GroupStateFieldNames::DEVICE_TYPE] = remoteConfig->name;
          }
        }
        break;

      default:
        Serial.printf_P(PSTR("Tried to apply unknown field: %d\n"), static_cast<uint8_t>(field));
        break;
    }
  }
}

// helper function to debug the current state (in JSON) to the serial port
void GroupState::debugState(char const *debugMessage) const {
#ifdef STATE_DEBUG
  // using static to keep large buffers off the call stack
  StaticJsonDocument<500> jsonDoc;
  JsonObject jsonState = jsonDoc.to<JsonObject>();

  // define fields to show (if count changes, make sure to update count to applyState below)
  std::vector<GroupStateField> fields({
      GroupStateField::LEVEL,
      GroupStateField::BULB_MODE,
      GroupStateField::COLOR_TEMP,
      GroupStateField::EFFECT,
      GroupStateField::HUE,
      GroupStateField::KELVIN,
      GroupStateField::MODE,
      GroupStateField::SATURATION,
      GroupStateField::STATE
  });

  // Fake id
  BulbId id;

  // use applyState to build JSON of all fields (from above)
  applyState(jsonState, id, fields);
  // convert to string and print
  Serial.printf("%s: ", debugMessage);
  serializeJson(jsonState, Serial);
  Serial.println("");
  Serial.printf("Raw data: %08X %08X\n", state.rawData[0], state.rawData[1]);
#endif
}

bool GroupState::isSetColor() const {
  return isSetHue();
}

ParsedColor GroupState::getColor() const {
  uint8_t rgb[3];
  RGBConverter converter;
  uint16_t hue = getHue();
  uint8_t sat = isSetSaturation() ? getSaturation() : 100;

  converter.hsvToRgb(
    hue / 360.0,
    // Default to fully saturated
    sat / 100.0,
    1,
    rgb
  );

  return {
    .success = true,
    .hue = hue,
    .r = rgb[0],
    .g = rgb[1],
    .b = rgb[2],
    .saturation = sat
  };
}

// build up a partial state representation based on the specified GrouipStateField array.  Used
// to gather a subset of states (configurable in the UI) for sending to MQTT and web responses.
void GroupState::applyState(JsonObject partialState, const BulbId& bulbId, std::vector<GroupStateField>& fields) const {
  for (std::vector<GroupStateField>::const_iterator itr = fields.begin(); itr != fields.end(); ++itr) {
    applyField(partialState, bulbId, *itr);
  }
}

bool GroupState::isPhysicalField(GroupStateField field) {
  for (size_t i = 0; i < size(ALL_PHYSICAL_FIELDS); ++i) {
    if (field == ALL_PHYSICAL_FIELDS[i]) {
      return true;
    }
  }
  return false;
}