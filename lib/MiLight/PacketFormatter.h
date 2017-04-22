#include <Arduino.h>
#include <inttypes.h>
#include <functional>
#include <MiLightButtons.h>

#define PACKET_FORMATTER_BUFFER_SIZE 48

#ifndef _PACKET_FORMATTER_H
#define _PACKET_FORMATTER_H

struct PacketStream {
  PacketStream();

  uint8_t* next();
  bool hasNext();

  uint8_t* packetStream;
  size_t numPackets;
  size_t packetLength;
  size_t currentPacket;
};

class PacketFormatter {
public:
  PacketFormatter(const size_t packetLength, const size_t maxPackets = 1);

  typedef void (PacketFormatter::*StepFunction)();

  void updateStatus(MiLightStatus status);
  virtual void updateStatus(MiLightStatus status, uint8_t groupId);
  virtual void command(uint8_t command, uint8_t arg);

  virtual void setHeld(bool held);

  // Mode
  virtual void updateMode(uint8_t value);
  virtual void modeSpeedDown();
  virtual void modeSpeedUp();
  virtual void nextMode();
  virtual void previousMode();

  virtual void pair();
  virtual void unpair();

  // Color
  virtual void updateHue(uint16_t value);
  virtual void updateColorRaw(uint8_t value);
  virtual void updateColorWhite();

  // White temperature
  virtual void increaseTemperature();
  virtual void decreaseTemperature();
  virtual void updateTemperature(uint8_t value);

  // Brightness
  virtual void updateBrightness(uint8_t value);
  virtual void increaseBrightness();
  virtual void decreaseBrightness();
  virtual void enableNightMode();

  virtual void updateSaturation(uint8_t value);

  virtual void reset();

  virtual PacketStream& buildPackets();
  virtual void prepare(uint16_t deviceId, uint8_t groupId);
  virtual void format(uint8_t const* packet, char* buffer);

  static void formatV1Packet(uint8_t const* packet, char* buffer);

  template <typename T>
  static T rescale(T value, uint8_t newMax, float oldMax = 255.0) {
    return round(value * (newMax / oldMax));
  }

  size_t getPacketLength() const;

protected:
  static uint8_t* PACKET_BUFFER;

  uint8_t* currentPacket;
  size_t packetLength;
  uint16_t deviceId;
  uint8_t groupId;
  uint8_t sequenceNum;
  size_t numPackets;
  bool held;
  PacketStream packetStream;

  void pushPacket();
  void valueByStepFunction(StepFunction increase, StepFunction decrease, uint8_t numSteps, uint8_t value);

  virtual void initializePacket(uint8_t* packetStart) = 0;
  virtual void finalizePacket(uint8_t* packet);
};

#endif
