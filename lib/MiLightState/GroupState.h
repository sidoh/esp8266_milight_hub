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
  bool isOn();
  void setOn(bool on);

  // 7 bits
  uint8_t getBrightness() const;
  void setBrightness(uint8_t brightness);

  // 8 bits
  uint8_t getHue();
  void setHue(uint8_t hue);

  // 7 bits
  uint8_t getSaturation();
  void setSaturation(uint8_t saturation);

  // 5 bits
  uint8_t getMode();
  void setMode(uint8_t mode);

  // 7 bits
  uint8_t getKelvin();
  void setKelvin(uint8_t kelvin);

  // 3 bits
  BulbMode getBulbMode();
  void setBulbMode(BulbMode mode);

private:
  uint32_t
    _on         : 1,
    _brightness : 7,
    _hue        : 8,
    _saturation : 7,
    _mode       : 4,
    _bulbMode   : 3,
                : 2;
  uint8_t
    _kelvin     : 7,
                : 1;
};

struct GroupStateNode {
  GroupState state;
  GroupId nextNode;
  GroupId prevNode;
};

#endif
