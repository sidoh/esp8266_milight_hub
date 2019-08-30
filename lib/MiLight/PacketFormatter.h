#include <Arduino.h>
#include <inttypes.h>
#include <functional>
#include <MiLightRemoteType.h>
#include <ArduinoJson.h>
#include <GroupState.h>
#include <GroupStateStore.h>
#include <Settings.h>

#ifndef _PACKET_FORMATTER_H
#define _PACKET_FORMATTER_H

// Most packets sent is for CCT bulbs, which always includes 10 down commands
// and can include up to 10 up commands.  CCT packets are 7 bytes.
//   (10 * 7) + (10 * 7) = 140
#define PACKET_FORMATTER_BUFFER_SIZE 140

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
  PacketFormatter(const MiLightRemoteType deviceType, const size_t packetLength, const size_t maxPackets = 1);

  // Ideally these would be constructor parameters.  We could accomplish this by
  // wrapping PacketFormaters in a factory, as Settings and StateStore are not
  // available at construction time.
  //
  // For now, just rely on the user calling this method.
  void initialize(GroupStateStore* stateStore, const Settings* settings);

  typedef void (PacketFormatter::*StepFunction)();

  virtual bool canHandle(const uint8_t* packet, const size_t len);

  void updateStatus(MiLightStatus status);
  void toggleStatus();
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

  virtual BulbId parsePacket(const uint8_t* packet, JsonObject result);
  virtual BulbId currentBulbId() const;

  static void formatV1Packet(uint8_t const* packet, char* buffer);

  size_t getPacketLength() const;

protected:
  const MiLightRemoteType deviceType;
  size_t packetLength;
  size_t numPackets;
  uint8_t* currentPacket;
  bool held;
  uint16_t deviceId;
  uint8_t groupId;
  uint8_t sequenceNum;
  PacketStream packetStream;
  GroupStateStore* stateStore = NULL;
  const Settings* settings = NULL;

  void pushPacket();

  // Get field into a desired state using only increment/decrement commands.  Do this by:
  //   1. Driving it down to its minimum value
  //   2. Applying the appropriate number of increase commands to get it to the desired
  //      value.
  // If the current state is already known, take that into account and apply the exact
  // number of rpeeats for the appropriate command.
  void valueByStepFunction(StepFunction increase, StepFunction decrease, uint8_t numSteps, uint8_t targetValue, int8_t knownValue = -1);

  virtual void initializePacket(uint8_t* packetStart) = 0;
  virtual void finalizePacket(uint8_t* packet);
};

#endif
