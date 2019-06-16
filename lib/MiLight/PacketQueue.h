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
};

class PacketQueue {
public:
  void push(const uint8_t* packet, const MiLightRemoteConfig* remoteConfig);
  std::shared_ptr<QueuedPacket> pop();
  bool isEmpty();
  size_t size();

private:
  std::shared_ptr<QueuedPacket> checkoutPacket();
  void checkinPacket(std::shared_ptr<QueuedPacket> packet);

  LinkedList<std::shared_ptr<QueuedPacket>> queue;
};