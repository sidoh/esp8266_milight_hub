#pragma once

#include <memory>

#include <CircularBuffer.h>
#include <MiLightRadioConfig.h>
#include <MiLightRemoteConfig.h>
#ifdef ESP32
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#endif

#ifndef MILIGHT_MAX_QUEUED_PACKETS
#define MILIGHT_MAX_QUEUED_PACKETS 20
#endif

struct QueuedPacket {
  uint8_t packet[MILIGHT_MAX_PACKET_LENGTH];
  const MiLightRemoteConfig* remoteConfig;
  size_t repeatsOverride;
  bool isInternal;
};

class PacketQueue {
public:
  PacketQueue();

  // Returns true if coalesced in-place over an existing unsent packet (queue length unchanged)
  bool push(const uint8_t* packet, const MiLightRemoteConfig* remoteConfig, const size_t repeatsOverride, bool isInternal = false);
  std::shared_ptr<QueuedPacket> pop();
  bool isEmpty() const;
  size_t size() const;
  size_t getDroppedPacketCount() const;
  size_t getCoalescedPacketCount() const;

private:
  size_t droppedPackets;
  size_t coalescedPackets;
#ifdef ESP32
  mutable SemaphoreHandle_t queueMutex = nullptr;
  inline void lockQueue() const {
    if (queueMutex != nullptr) xSemaphoreTake(queueMutex, portMAX_DELAY);
  }
  inline void unlockQueue() const {
    if (queueMutex != nullptr) xSemaphoreGive(queueMutex);
  }
#endif

  std::shared_ptr<QueuedPacket> checkoutPacket();
  void checkinPacket(std::shared_ptr<QueuedPacket> packet);

  LinkedList<std::shared_ptr<QueuedPacket>> queue;
};