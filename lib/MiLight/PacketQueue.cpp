#include <PacketQueue.h>

PacketQueue::PacketQueue()
  : droppedPackets(0)
{ }

void PacketQueue::push(const uint8_t* packet, const MiLightRemoteConfig* remoteConfig, const size_t repeatsOverride) {
  std::shared_ptr<QueuedPacket> qp = checkoutPacket();
  memcpy(qp->packet, packet, remoteConfig->packetFormatter->getPacketLength());
  qp->remoteConfig = remoteConfig;
  qp->repeatsOverride = repeatsOverride;
}

bool PacketQueue::isEmpty() const {
  return queue.size() == 0;
}

size_t PacketQueue::getDroppedPacketCount() const {
  return droppedPackets;
}

std::shared_ptr<QueuedPacket> PacketQueue::pop() {
  return queue.shift();
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
  return queue.size();
}