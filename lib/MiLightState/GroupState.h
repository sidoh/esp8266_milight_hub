#include <stddef.h>
#include <inttypes.h>
#include <MiLightConstants.h>
#include <MiLightRadioConfig.h>
#include <GroupStateField.h>
#include <ArduinoJson.h>

#ifndef _GROUP_STATE_H
#define _GROUP_STATE_H

// enable to add debugging on state
// #define DEBUG_STATE

struct BulbId {
  uint16_t deviceId;
  uint8_t groupId;
  MiLightRemoteType deviceType;

  BulbId();
  BulbId(const BulbId& other);
  BulbId(const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType);
  bool operator==(const BulbId& other);
  void operator=(const BulbId& other);
};

enum BulbMode {
  BULB_MODE_WHITE,
  BULB_MODE_COLOR,
  BULB_MODE_SCENE,
  BULB_MODE_NIGHT
};

enum class IncrementDirection : unsigned {
  INCREASE = 1, 
  DECREASE = -1U
};

static const char* BULB_MODE_NAMES[] = {
  "white",
  "color",
  "scene",
  "night"
};

class GroupState {
public:

  GroupState();
  GroupState(const GroupState& other);
  GroupState& operator=(const GroupState& other);

  bool operator==(const GroupState& other) const;
  bool isEqualIgnoreDirty(const GroupState& other) const;
  void print(Stream& stream) const;


  bool isSetField(GroupStateField field) const;
  uint16_t getFieldValue(GroupStateField field) const;
  void setFieldValue(GroupStateField field, uint16_t value);

  bool isSetScratchField(GroupStateField field) const;
  uint16_t getScratchFieldValue(GroupStateField field) const;
  void setScratchFieldValue(GroupStateField field, uint16_t value);

  // 1 bit
  bool isSetState() const;
  MiLightStatus getState() const;
  bool setState(const MiLightStatus on);
  // Return true if status is ON or if the field is unset (i.e., defaults to ON)
  bool isOn() const;

  // 7 bits
  bool isSetBrightness() const;
  uint8_t getBrightness() const;
  bool setBrightness(uint8_t brightness);

  // 8 bits
  bool isSetHue() const;
  uint16_t getHue() const;
  bool setHue(uint16_t hue);

  // 7 bits
  bool isSetSaturation() const;
  uint8_t getSaturation() const;
  bool setSaturation(uint8_t saturation);

  // 5 bits
  bool isSetMode() const;
  bool isSetEffect() const;
  uint8_t getMode() const;
  bool setMode(uint8_t mode);

  // 7 bits
  bool isSetKelvin() const;
  uint8_t getKelvin() const;
  uint16_t getMireds() const;
  bool setKelvin(uint8_t kelvin);
  bool setMireds(uint16_t mireds);

  // 3 bits
  bool isSetBulbMode() const;
  BulbMode getBulbMode() const;
  bool setBulbMode(BulbMode mode);

  // 1 bit
  bool isSetNightMode() const;
  bool isNightMode() const;
  bool setNightMode(bool nightMode);

  bool isDirty() const;
  inline bool setDirty();
  bool clearDirty();

  bool isMqttDirty() const;
  inline bool setMqttDirty();
  bool clearMqttDirty();

  // Patches this state with ONLY the set fields in the other. Returns 
  // true if there were any changes.
  bool patch(const GroupState& other);

  // Patches this state with the fields defined in the JSON state.  Returns 
  // true if there were any changes.
  bool patch(const JsonObject& state);

  // It's a little weird to need to pass in a BulbId here.  The purpose is to
  // support fields like DEVICE_ID, which aren't otherweise available to the
  // state in this class.  The alternative is to have every GroupState object
  // keep a reference to its BulbId, which feels too heavy-weight.
  void applyField(JsonObject& state, const BulbId& bulbId, GroupStateField field);
  void applyState(JsonObject& state, const BulbId& bulbId, GroupStateField* fields, size_t numFields);

  // Attempt to keep track of increment commands in such a way that we can
  // know what state it's in.  When we get an increment command (like "increase 
  // brightness"):
  //   1. If there is no value in the scratch state: assume real state is in 
  //      the furthest value from the direction of the command.  For example, 
  //      if we get "increase," assume the value was 0.
  //   2. If there is a value in the scratch state, apply the command to it.
  //      For example, if we get "decrease," subtract 1 from the scratch.
  //   3. When scratch reaches a known extreme (either min or max), set the
  //      persistent field to that value
  //   4. If there is already a known value for the state, apply it rather
  //      than messing with scratch state.
  // 
  // returns true if a (real, not scratch) state change was made
  bool applyIncrementCommand(GroupStateField field, IncrementDirection dir);

  void load(Stream& stream);
  void dump(Stream& stream) const;

  void debugState(char const *debugMessage);

  static const GroupState& defaultState(MiLightRemoteType remoteType);

private:
  static const size_t DATA_LONGS = 2;
  union StateData {
    uint32_t rawData[DATA_LONGS];
    struct Fields {
      uint32_t
        _state                : 1,
        _brightness           : 7,
        _hue                  : 8,
        _saturation           : 7,
        _mode                 : 4,
        _bulbMode             : 3,
        _isSetState           : 1,
        _isSetHue             : 1;
      uint32_t
        _kelvin               : 7,
        _isSetBrightness      : 1,
        _isSetSaturation      : 1,
        _isSetMode            : 1,
        _isSetKelvin          : 1,
        _isSetBulbMode        : 1,
        _brightnessColor      : 7,
        _brightnessMode       : 7,
        _isSetBrightnessColor : 1,
        _isSetBrightnessMode  : 1,
        _dirty                : 1,
        _mqttDirty            : 1,
        _isSetNightMode       : 1,
        _isNightMode          : 1;
    } fields;
  };

  // Transient scratchpad that is never persisted.  Used to track and compute state for
  // protocols that only have increment commands (like CCT).
  union TransientData {
    uint16_t rawData;
    struct Fields {
      uint16_t 
        _isSetKelvinScratch     : 1,
        _kelvinScratch          : 7,
        _isSetBrightnessScratch : 1,
        _brightnessScratch      : 8;
    } fields;
  };

  StateData state;
  TransientData scratchpad;

  void applyColor(JsonObject& state, uint8_t r, uint8_t g, uint8_t b);
  void applyColor(JsonObject& state);
};

extern const BulbId DEFAULT_BULB_ID;

#endif
