#include <PacketQueue.h>
#include <V2RFEncoding.h>

static bool extractV2CoalesceKey(const uint8_t* rawPkt, size_t len, uint32_t& outTargetKey, uint8_t& outCmd) {
  if (len != 9) return false;
  uint8_t dec[9];
  memcpy(dec, rawPkt, 9);
  V2RFEncoding::decodeV2Packet(dec);
  // Key = protocolId (b1) | deviceId (b2,b3) | groupId (b7); Command = b4 & 0x7F
  outTargetKey = ((uint32_t)dec[1] << 24) | ((uint32_t)dec[2] << 16) | ((uint32_t)dec[3] << 8) | (uint32_t)dec[7];
  outCmd = dec[4] & 0x7F;
  return true;
}

PacketQueue::PacketQueue()
  : droppedPackets(0), coalescedPackets(0)
{
#ifdef ESP32
  queueMutex = xSemaphoreCreateMutex();
#endif
}

bool PacketQueue::push(const uint8_t* packet, const MiLightRemoteConfig* remoteConfig, const size_t repeatsOverride, bool isInternal) {
  size_t pktLen = remoteConfig->packetFormatter->getPacketLength();
  uint32_t newTargetKey = 0;
  uint8_t newCmd = 0;
  bool isV2 = extractV2CoalesceKey(packet, pktLen, newTargetKey, newCmd);

#ifdef ESP32
  lockQueue();
#endif
  // Latest-Wins Slot Coalescing: overwrite unsent stale packet for same V2 (device, group, command)!
  if (isV2) {
    for (ListNode<std::shared_ptr<QueuedPacket>>* node = queue.getHead(); node != NULL; node = node->next) {
      std::shared_ptr<QueuedPacket> existing = node->data;
      if (existing != nullptr && existing->remoteConfig == remoteConfig) {
        uint32_t exKey = 0;
        uint8_t exCmd = 0;
        if (extractV2CoalesceKey(existing->packet, pktLen, exKey, exCmd) && exKey == newTargetKey && exCmd == newCmd) {
          memcpy(existing->packet, packet, pktLen);
          existing->repeatsOverride = repeatsOverride;
          existing->isInternal = isInternal;
          coalescedPackets++;
#ifdef ESP32
          unlockQueue();
#endif
          return true;
        }
      }
    }
  }

  std::shared_ptr<QueuedPacket> qp = checkoutPacket();
  memcpy(qp->packet, packet, pktLen);
  qp->remoteConfig = remoteConfig;
  qp->repeatsOverride = repeatsOverride;
  qp->isInternal = isInternal;
#ifdef ESP32
  unlockQueue();
#endif
  return false;
}

bool PacketQueue::isEmpty() const {
#ifdef ESP32
  lockQueue();
#endif
  bool empty = (queue.size() == 0);
#ifdef ESP32
  unlockQueue();
#endif
  return empty;
}

size_t PacketQueue::getDroppedPacketCount() const {
  return droppedPackets;
}

size_t PacketQueue::getCoalescedPacketCount() const {
  return coalescedPackets;
}

std::shared_ptr<QueuedPacket> PacketQueue::pop() {
#ifdef ESP32
  lockQueue();
#endif
  std::shared_ptr<QueuedPacket> item = (queue.size() > 0) ? queue.shift() : nullptr;
#ifdef ESP32
  unlockQueue();
#endif
  return item;
}

std::shared_ptr<QueuedPacket> PacketQueue::checkoutPacket() {
  if (queue.size() == MILIGHT_MAX_QUEUED_PACKETS) {
    ++droppedPackets;
    return queue.getLast();
  } else {
    std::shared_ptr<QueuedPacket> packet = std::make_shared<QueuedPacket>();
    queue.add(packet);
    return packet;
  }
}

void PacketQueue::checkinPacket(std::shared_ptr<QueuedPacket> packet) {
}

size_t PacketQueue::size() const {
#ifdef ESP32
  lockQueue();
#endif
  size_t s = queue.size();
#ifdef ESP32
  unlockQueue();
#endif
  return s;
}