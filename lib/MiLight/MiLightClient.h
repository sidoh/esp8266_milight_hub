#include <Arduino.h>
#include <MiLightRadio.h>
#include <PL1167_nRF24.h>
#include <RF24.h>

#ifndef _MILIGHTCLIENT_H
#define _MILIGHTCLIENT_H

#define MILIGHT_PACKET_LENGTH 7

enum MiLightDeviceType {
  WHITE = 0xB0,
  RGBW = 0xB8
};

enum MiLightButton {
  ALL_ON            = 0x01,
  ALL_OFF           = 0x02,
  GROUP_1_ON        = 0x03,
  GROUP_1_OFF       = 0x04,
  GROUP_2_ON        = 0x05,
  GROUP_2_OFF       = 0x06,
  GROUP_3_ON        = 0x07,
  GROUP_3_OFF       = 0x08,
  GROUP_4_ON        = 0x09,
  GROUP_4_OFF       = 0x0A,
  SPEED_UP          = 0x0B, 
  SPEED_DOWN        = 0x0C, 
  DISCO_MODE        = 0x0D,
  BRIGHTNESS        = 0x0E,
  COLOR             = 0x0F,
  ALL_MAX_LEVEL     = 0x11,
  ALL_MIN_LEVEL     = 0x12,
  
  // These are the only mechanism (that I know of) to disable RGB and set the
  // color to white.
  GROUP_1_MAX_LEVEL = 0x13,
  GROUP_1_MIN_LEVEL = 0x14,
  GROUP_2_MAX_LEVEL = 0x15,
  GROUP_2_MIN_LEVEL = 0x16,
  GROUP_3_MAX_LEVEL = 0x17,
  GROUP_3_MIN_LEVEL = 0x18,
  GROUP_4_MAX_LEVEL = 0x19,
  GROUP_4_MIN_LEVEL = 0x1A,
};

enum MiLightStatus { ON = 0, OFF = 1 };
  
struct MiLightPacket {
  uint8_t deviceType;
  uint16_t deviceId;
  uint8_t color;
  uint8_t brightness;
  uint8_t groupId;
  uint8_t button;
  uint8_t sequenceNum;
};

class MiLightClient {
  public:
    MiLightClient(uint8_t cePin, uint8_t csnPin) :
      sequenceNum(0) {
      rf = new RF24(cePin, csnPin);
      prf = new PL1167_nRF24(*rf);
      radio = new MiLightRadio(*prf);
    }
    
    ~MiLightClient() {
      delete rf;
      delete prf;
      delete radio;
    }
    
    void begin() {
      radio->begin();
    }
    
    bool available();
    void read(MiLightPacket& packet);
    void write(MiLightPacket& packet, const unsigned int resendCount = 50);
    
    void write(
      const uint16_t deviceId,
      const uint8_t color,
      const uint8_t brightness,
      const uint8_t groupId,
      const MiLightButton button
    );
    
    void updateColorRaw(const uint16_t deviceId, const uint8_t groupId, const uint16_t color);
    
    void updateHue(const uint16_t deviceId, const uint8_t groupId, const uint16_t hue);
    void updateBrightness(const uint16_t deviceId, const uint8_t groupId, const uint8_t brightness);
    void updateStatus(const uint16_t deviceId, const uint8_t groupId, MiLightStatus status);
    void updateColorWhite(const uint16_t deviceId, const uint8_t groupId);
    void pair(const uint16_t deviceId, const uint8_t groupId);
    void unpair(const uint16_t deviceId, const uint8_t groupId);
    
    void allOn(const uint16_t deviceId);
    void allOff(const uint16_t deviceId);
    
    void pressButton(const uint16_t deviceId, const uint8_t groupId, MiLightButton button);
    
    static void deserializePacket(const uint8_t rawPacket[], MiLightPacket& packet);
    static void serializePacket(uint8_t rawPacket[], const MiLightPacket& packet);
    
  private:
    RF24 *rf;
    PL1167_nRF24 *prf;
    MiLightRadio* radio;
    uint8_t sequenceNum;
    
    uint8_t nextSequenceNum();
};

#endif