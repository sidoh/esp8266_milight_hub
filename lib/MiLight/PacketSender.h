#pragma once

#include <MiLightRadioFactory.h>
#include <MiLightRemoteConfig.h>
#include <PacketQueue.h>
#include <RadioSwitchboard.h>

class PacketSender {
public:
  typedef std::function<void(uint8_t* packet, const MiLightRemoteConfig& config)> PacketSentHandler;
  static const size_t DEFAULT_PACKET_SENDS_VALUE = 0;

  PacketSender(
    RadioSwitchboard& radioSwitchboard,
    Settings& settings,
    PacketSentHandler packetSentHandler
  );

  void enqueue(uint8_t* packet, const MiLightRemoteConfig* remoteConfig, const size_t repeatsOverride = 0);
  void loop();

  // Return true if there are queued packets
  bool isSending();

private:
  RadioSwitchboard& radioSwitchboard;
  Settings& settings;
  GroupStateStore* stateStore;
  PacketQueue queue;

  // The current packet we're sending and the number of repeats left
  std::shared_ptr<QueuedPacket> currentPacket;
  size_t packetRepeatsRemaining;

  // Handler called after packets are sent.  Will not be called multiple times
  // per repeat.
  PacketSentHandler packetSentHandler;

  // Send a batch of repeats for the current packet
  void handleCurrentPacket();

  // Switch to the next packet in the queue
  void nextPacket();

  // Send repeats of the current packet N times
  void sendRepeats(size_t num);
};