#include <stddef.h>
#include <inttypes.h>
#include <MiLightButtons.h>
#include <ArduinoJson.h>

#ifndef _GROUP_STATE_H
#define _GROUP_STATE_H

struct GroupId {
  uint16_t deviceId;
  uint8_t groupId;
  MiLightRemoteType deviceType;

  GroupId();
  GroupId(const GroupId& other);
  GroupId(const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType);
  bool operator==(const GroupId& other);
  void operator=(const GroupId& other);
};

enum BulbMode {
  BULB_MODE_WHITE,
  BULB_MODE_COLOR,
  BULB_MODE_SCENE,
  BULB_MODE_NIGHT
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

  // 1 bit
  bool isSetState() const;
  MiLightStatus getState() const;
  bool setState(const MiLightStatus on);

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

  bool isDirty() const;
  inline bool setDirty();
  bool clearDirty();

  bool isMqttDirty() const;
  inline bool setMqttDirty();
  bool clearMqttDirty();

  bool patch(const JsonObject& state);
  void applyState(JsonObject& state);

  void load(Stream& stream);
  void dump(Stream& stream) const;

  static const GroupState& defaultState(MiLightRemoteType remoteType);

private:
  static const size_t DATA_BYTES = 2;
  union Data {
    uint32_t data[DATA_BYTES];
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
                              : 4;
    } fields;
  };

  Data state;
};

struct GroupStateNode {
  GroupState state;
  GroupId nextNode;
  GroupId prevNode;
};

extern const GroupId DEFAULT_GROUP_ID;

#endif
