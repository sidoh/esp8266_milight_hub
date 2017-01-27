#include <Arduino.h>
#include <MiLightRadio.h>

#ifndef _MILIGHTCLIENT_H
#define _MILIGHTCLIENT_H

enum MiLightDeviceType {
  WHITE = 0xB0,
  RGBW = 0xB8
};

enum MiLightButton {
  ALL_ON      = 0x01,
  ALL_OFF     = 0x02,
  GROUP_1_ON  = 0x03,
  GROUP_1_OFF = 0x04,
  GROUP_2_ON  = 0x05,
  GROUP_2_OFF = 0x06,
  GROUP_3_ON  = 0x07,
  GROUP_3_OFF = 0x08,
  GROUP_4_ON  = 0x09,
  GROUP_4_OFF = 0x0A,
  SPEED_UP    = 0x0B, 
  SPEED_DOWN  = 0x0C, 
  DISCO_MODE  = 0x0D,
  BRIGHTNESS  = 0x0E,
  COLOR       = 0x0F,
  COLOR_WHITE = 0x11
};

enum MiLightStatus { ON = 0, OFF = 1 };
  
#pragma pack(push, 1)
struct MiLightPacket {
  uint8_t deviceType;
  uint16_t deviceId;
  uint8_t color;
  uint8_t brightnessGroupId;
  uint8_t button;
  uint8_t sequenceNum;
};
#pragma pack(pop)

class MiLightClient {
  public:
    MiLightClient(MiLightRadio& radio) :
      radio(radio),
      sequenceNum(0) {}
    
    bool available();
    void read(MiLightPacket& packet);
    void write(MiLightPacket& packet, const unsigned int resendCount = 50);
    
    void write(
      const uint16_t deviceId,
      const uint16_t color,
      const uint8_t brightness,
      const uint8_t groupId,
      const MiLightButton button
    );
    
    void updateColor(const uint16_t deviceId, const uint8_t groupId, const uint16_t hue);
    void updateBrightness(const uint16_t deviceId, const uint8_t groupId, const uint8_t brightness);
    void updateStatus(const uint16_t deviceId, const uint8_t groupId, MiLightStatus status);
    void updateColorWhite(const uint16_t deviceId, const uint8_t groupId);
    
    void allOn(const uint16_t deviceId);
    void allOff(const uint16_t deviceId);
    
  private:
    MiLightRadio& radio;
    uint8_t sequenceNum;
    
    uint8_t nextSequenceNum();
};

#endif