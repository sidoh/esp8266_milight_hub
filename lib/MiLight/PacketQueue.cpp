#include <PacketQueue.h>

void PacketQueue::push(const uint8_t* packet, const MiLightRemoteConfig* remoteConfig) {
  std::shared_ptr<QueuedPacket> qp = checkoutPacket();
  memcpy(qp->packet, packet, remoteConfig->packetFormatter->getPacketLength());
  qp->remoteConfig = remoteConfig;
}

bool PacketQueue::isEmpty() {
  return queue.size() == 0;
}

std::shared_ptr<QueuedPacket> PacketQueue::pop() {
  return queue.shift();
}

std::shared_ptr<QueuedPacket> PacketQueue::checkoutPacket() {
  if (queue.size() == MILIGHT_MAX_QUEUED_PACKETS) {
    return queue.getLast();
  } else {
    std::shared_ptr<QueuedPacket> packet = std::make_shared<QueuedPacket>();
    queue.add(packet);
    return packet;
  }
}

void PacketQueue::checkinPacket(std::shared_ptr<QueuedPacket> packet) {
}

size_t PacketQueue::size() {
  return queue.size();
}