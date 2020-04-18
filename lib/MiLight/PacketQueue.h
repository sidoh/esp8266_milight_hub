#pragma once

#include <memory>

#include <CircularBuffer.h>
#include <MiLightRadioConfig.h>
#include <MiLightRemoteConfig.h>

#ifndef MILIGHT_MAX_QUEUED_PACKETS
#define MILIGHT_MAX_QUEUED_PACKETS 20
#endif

struct QueuedPacket {
  uint8_t packet[MILIGHT_MAX_PACKET_LENGTH];
  const MiLightRemoteConfig* remoteConfig;
  size_t repeatsOverride;

  QueuedPacket();
  QueuedPacket(const QueuedPacket& copy);
  QueuedPacket(const uint8_t* packet, const MiLightRemoteConfig* remoteConfig, size_t repeatsOverride);
  void operator=(const QueuedPacket& copy);
};

class PacketQueue {
public:
  PacketQueue();

  void push(const uint8_t* packet, const MiLightRemoteConfig* remoteConfig, const size_t repeatsOverride);
  QueuedPacket* pop();
  bool isEmpty() const;
  size_t size() const;
  size_t getDroppedPacketCount() const;

private:
  size_t droppedPackets;

  std::shared_ptr<QueuedPacket> checkoutPacket();
  void checkinPacket(std::shared_ptr<QueuedPacket> packet);

  // Not using CircularBuffer to avoid a bunch of unnecessary value copying
  QueuedPacket* head;
  QueuedPacket* tail;
  uint8_t count;
  QueuedPacket queue[MILIGHT_MAX_QUEUED_PACKETS];
};