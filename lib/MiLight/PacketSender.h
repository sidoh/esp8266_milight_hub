#pragma once

#include <MiLightRadioFactory.h>
#include <MiLightRemoteConfig.h>
#include <PacketQueue.h>
#include <RadioSwitchboard.h>

class PacketSender {
public:
  typedef std::function<void(uint8_t* packet, const MiLightRemoteConfig& config, bool isInternal)> PacketSentHandler;
  static const size_t DEFAULT_PACKET_SENDS_VALUE = 0;

  PacketSender(
    RadioSwitchboard& radioSwitchboard,
    Settings& settings,
    PacketSentHandler packetSentHandler
  );

  bool enqueue(uint8_t* packet, const MiLightRemoteConfig* remoteConfig, const size_t repeatsOverride = 0, bool isInternal = false);
  void loop();

  // Return true if there are queued packets
  bool isSending();

  // Return the number of queued packets
  size_t queueLength() const;
  size_t droppedPackets() const;
  size_t coalescedPackets() const;
  bool isRecentLocalTx(const uint8_t* packet, size_t len, uint32_t windowMs = 1200) const;

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

  // Used to track auto repeat limiting
  unsigned long lastSend;
  uint8_t currentResendCount;

  // This will be pre-computed, but is simply:
  //
  //    (sensitivity / 1000.0) * R
  //
  // Where R is the base number of repeats.
  size_t throttleMultiplier;

  /*
   * Calculates the number of resend packets based on when the last packet
   * was sent using this function:
   *
   *    lastRepeatsValue + (millisSinceLastSend - THRESHOLD) * throttleMultiplier
   *
   * When the last send was more recent than THRESHOLD, the number of repeats
   * will be decreased to a minimum of zero.  When less recent, it will be
   * increased up to a maximum of the default resend count.
   */
  void updateResendCount();

  struct TxEchoSlot {
    uint8_t bytes[MILIGHT_MAX_PACKET_LENGTH];
    uint8_t len = 0;
    uint32_t ts = 0;
  };
  static const size_t TX_ECHO_RING_SIZE = 16;
  TxEchoSlot txEchoRing[TX_ECHO_RING_SIZE];
  size_t txEchoHead = 0;
  volatile uint32_t lastRadioWriteMs = 0;
#ifdef ESP32
  mutable portMUX_TYPE echoMux = portMUX_INITIALIZER_UNLOCKED;
#endif
  void recordTxEcho(const uint8_t* packet, size_t len);
};