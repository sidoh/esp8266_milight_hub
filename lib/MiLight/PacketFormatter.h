#include <Arduino.h>
#include <inttypes.h>
#include <MiLightButtons.h>

#ifndef _PACKET_FORMATTER_H
#define _PACKET_FORMATTER_H 

class PacketFormatter {
public:
  PacketFormatter(const size_t packetLength) {
    this->packet = new uint8_t[packetLength];
  }
  
  ~PacketFormatter() {
    delete this->packet;
  }
  
  // all
  virtual void updateStatus(MiLightStatus status);
  virtual void updateBrightness(uint8_t value);
  virtual void updateMode(uint8_t value);
  virtual void modeSpeedDown();
  virtual void modeSpeedUp();
  
  // rgbw, rgb+cct
  virtual void updateHue(uint8_t value);
  virtual void updateColorRaw(uint8_t value);
  
  // cct 
  virtual void increaseTemperature(uint8_t value);
  virtual void decreaseTemperature(uint8_t value);
  
  // rgb+cct
  virtual void updateTemperature(uint8_t value);
  virtual void updateSaturation(uint8_t value);
  
  virtual void reset() = 0;
  
  uint8_t const* buildPacket() {
    return this->packet;
  }
  
  void prepare(uint16_t deviceId, uint8_t groupId) {
    this->deviceId = deviceId;
    this->groupId = groupId;
    reset();
  }
  
  static uint8_t rescale(uint8_t value, uint8_t max) {
    return round(value * (max / 255.0));
  }
  
protected:
  uint8_t* packet;
  uint16_t deviceId;
  uint8_t groupId;
  uint8_t sequenceNum;
};

#endif
