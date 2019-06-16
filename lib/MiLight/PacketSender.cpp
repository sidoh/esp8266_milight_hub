#include <PacketSender.h>
#include <MiLightradioConfig.h>

PacketSender::PacketSender(
  RadioSwitchboard& radioSwitchboard,
  Settings& settings,
  PacketSentHandler packetSentHandler
) : radioSwitchboard(radioSwitchboard)
  , settings(settings)
  , stateStore(stateStore)
  , currentPacket(nullptr)
  , packetRepeatsRemaining(0)
  , packetSentHandler(packetSentHandler)
{ }

void PacketSender::enqueue(uint8_t* packet, const MiLightRemoteConfig* remoteConfig) {
#ifdef DEBUG_PRINTF
  Serial.println("Enqueuing packet");
#endif
  queue.push(packet, remoteConfig);
}

void PacketSender::loop() {
  // Switch to the next packet if we're done with the current one
  if (packetRepeatsRemaining == 0 && !queue.isEmpty()) {
    nextPacket();
  }

  // If there's a packet we're handling, deal with it
  if (currentPacket != nullptr && packetRepeatsRemaining > 0) {
    handleCurrentPacket();
  }
}

void PacketSender::nextPacket() {
#ifdef DEBUG_PRINTF
  Serial.printf("Switching to next packet, %d packets in queue\n", queue.size());
#endif
  currentPacket = queue.pop();
  packetRepeatsRemaining = settings.packetRepeats;
}

void PacketSender::handleCurrentPacket() {
  // Always switch radio.  could've been listening in another context
  radioSwitchboard.switchRadio(currentPacket->remoteConfig);

  size_t numToSend = std::min(packetRepeatsRemaining, settings.packetRepeatsPerLoop);
  sendRepeats(numToSend);
  packetRepeatsRemaining -= numToSend;

  // If we're done sending this packet, fire the sent packet callback
  if (packetRepeatsRemaining == 0 && packetSentHandler != nullptr) {
    packetSentHandler(currentPacket->packet, *currentPacket->remoteConfig);
  }
}

void PacketSender::sendRepeats(size_t num) {
  size_t len = currentPacket->remoteConfig->packetFormatter->getPacketLength();

#ifdef DEBUG_PRINTF
  Serial.printf_P(PSTR("Sending packet (%d repeats): \n"), num);
  for (size_t i = 0; i < len; i++) {
    Serial.printf_P(PSTR("%02X "), currentPacket->packet[i]);
  }
  Serial.println();
  int iStart = millis();
#endif

  for (size_t i = 0; i < num; ++i) {
    radioSwitchboard.write(currentPacket->packet, len);
  }

#ifdef DEBUG_PRINTF
  int iElapsed = millis() - iStart;
  Serial.print("Elapsed: ");
  Serial.println(iElapsed);
#endif
}