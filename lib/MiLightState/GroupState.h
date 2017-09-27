#include <inttypes.h>
#include <Arduino.h>
#include <MiLightButtons.h>

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
  BULB_MODE_SCENE
};

class GroupState {
public:
  GroupState();

  // 1 bit
  bool isSetOn() const;
  bool isOn() const;
  void setOn(bool on);

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

  static const GroupState& defaultState();

private:
  uint32_t
    _on         : 1,
    _brightness : 7,
    _hue        : 8,
    _saturation : 7,
    _mode       : 4,
    _bulbMode   : 3,
    _isSetOn    : 1,
    _isSetHue   : 1;

  uint16_t
    _kelvin          : 7,
    _isSetBrightness : 1,
    _isSetSaturation : 1,
    _isSetMode       : 1,
    _isSetKelvin     : 1,
    _isSetBulbMode   : 1,
                     : 4;
};

struct GroupStateNode {
  GroupState state;
  GroupId nextNode;
  GroupId prevNode;
};

#endif
