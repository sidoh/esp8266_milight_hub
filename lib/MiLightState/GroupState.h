#include <inttypes.h>
#include <Arduino.h>

#ifndef _GROUP_STATE_H
#define _GROUP_STATE_H

struct GroupId {
  uint16_t deviceId;
  uint8_t groupId;
};

struct GroupState {
  // xxxx xxxx  xxxx xxxx  xxxx xxxx  xxxx xxxx
  // ^..
  uint32_t data;

  // 1 bit
  bool isOn();
  void setOn(bool on);

  // 7 bits
  uint8_t getBrightness();
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
};

struct GroupStateNode {
  GroupState state;
  GroupId nextNode;
  GroupId prevNode;
};

#endif
