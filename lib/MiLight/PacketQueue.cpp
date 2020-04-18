#include <PacketQueue.h>

QueuedPacket::QueuedPacket()
  : remoteConfig(nullptr)
  , repeatsOverride(0)
{ }

QueuedPacket::QueuedPacket(const QueuedPacket& copy)
  : remoteConfig(copy.remoteConfig)
  , repeatsOverride(copy.repeatsOverride)
{
  memcpy(this->packet, copy.packet, copy.remoteConfig->packetFormatter->getPacketLength());
}

QueuedPacket::QueuedPacket(const uint8_t* packet, const MiLightRemoteConfig* remoteConfig, size_t repeatsOverride)
  : remoteConfig(remoteConfig)
  , repeatsOverride(repeatsOverride)
{
  memcpy(this->packet, packet, remoteConfig->packetFormatter->getPacketLength());
}

void QueuedPacket::operator=(const QueuedPacket& copy) {
  this->remoteConfig = copy.remoteConfig;
  this->repeatsOverride = copy.repeatsOverride;
}

PacketQueue::PacketQueue()
  : droppedPackets(0)
  , head(queue)
  , tail(queue)
  , count(0)
{ }

void PacketQueue::push(const uint8_t* packet, const MiLightRemoteConfig* remoteConfig, const size_t repeatsOverride) {
  if (++tail == queue + MILIGHT_MAX_QUEUED_PACKETS) {
    tail = queue;
  }

  tail->remoteConfig = remoteConfig;
  tail->repeatsOverride = repeatsOverride;
  memcpy(tail->packet, packet, remoteConfig->packetFormatter->getPacketLength());

  if (count == MILIGHT_MAX_QUEUED_PACKETS) {
    if (++head == queue + MILIGHT_MAX_QUEUED_PACKETS) {
      head = queue;
    }
    ++droppedPackets;
  } else {
    if (count++ == 0) {
      head = tail;
    }
  }
}

bool PacketQueue::isEmpty() const {
  return count == 0;
}

size_t PacketQueue::getDroppedPacketCount() const {
  return droppedPackets;
}

QueuedPacket* PacketQueue::pop() {
  if (count == 0) return head;
  QueuedPacket* r = head++;
  if (head >= queue + MILIGHT_MAX_QUEUED_PACKETS) {
    head = queue;
  }
  count--;
  return r;
}

size_t PacketQueue::size() const {
  return count;
}