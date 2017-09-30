#include <inttypes.h>
#include <Arduino.h>
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
  void setState(const MiLightStatus on);

  // 7 bits
  bool isSetBrightness() const;
  uint8_t getBrightness() const;
  void setBrightness(uint8_t brightness);

  // 8 bits
  bool isSetHue() const;
  uint8_t getHue() const;
  void setHue(uint8_t hue);

  // 7 bits
  bool isSetSaturation() const;
  uint8_t getSaturation() const;
  void setSaturation(uint8_t saturation);

  // 5 bits
  bool isSetMode() const;
  uint8_t getMode() const;
  void setMode(uint8_t mode);

  // 7 bits
  bool isSetKelvin() const;
  uint8_t getKelvin() const;
  void setKelvin(uint8_t kelvin);

  // 3 bits
  bool isSetBulbMode() const;
  BulbMode getBulbMode() const;
  void setBulbMode(BulbMode mode);

  bool isDirty() const;
  bool setDirty();
  bool clearDirty();

  void patch(const JsonObject& state);
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
                              : 5;
    } fields;
  };

  Data state;
};

struct GroupStateNode {
  GroupState state;
  GroupId nextNode;
  GroupId prevNode;
};

#endif
