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
  void updateStatus(MiLightStatus status);
  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void updateBrightness(uint8_t value);
  virtual void command(uint8_t command, uint8_t arg);
  virtual void updateMode(uint8_t value);
  virtual void modeSpeedDown();
  virtual void modeSpeedUp();
  
  // rgbw, rgb+cct
  virtual void updateHue(uint16_t value);
  virtual void updateColorRaw(uint8_t value);
  virtual void updateColorWhite();
  
  // cct 
  virtual void increaseTemperature();
  virtual void decreaseTemperature();
  virtual void increaseBrightness();
  virtual void decreaseBrightness();
  
  // rgb+cct
  virtual void updateTemperature(uint8_t value);
  virtual void updateSaturation(uint8_t value);
  
  virtual void reset() = 0;
  
  virtual uint8_t* buildPacket() {
    printf("Packet: ");
    for(size_t i = 0; i < 7; i++) {
      printf("%02X ", this->packet[i]);
    }
    printf("\n");
    return this->packet;
  }
  
  void prepare(uint16_t deviceId, uint8_t groupId) {
    this->deviceId = deviceId;
    this->groupId = groupId;
    reset();
  }
  
  template <typename T>
  static T rescale(T value, uint8_t newMax, float oldMax = 255.0) {
    return round(value * (newMax / oldMax));
  }
  
protected:
  uint8_t* packet;
  uint16_t deviceId;
  uint8_t groupId;
  uint8_t sequenceNum;
};

#endif
