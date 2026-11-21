#include <PacketSender.h>
#include <MiLightRadioConfig.h>

PacketSender::PacketSender(
  RadioSwitchboard& radioSwitchboard,
  Settings& settings,
  PacketSentHandler packetSentHandler
) : radioSwitchboard(radioSwitchboard)
  , settings(settings)
  , currentPacket(nullptr)
  , packetRepeatsRemaining(0)
  , packetSentHandler(packetSentHandler)
  , lastSend(0)
  , currentResendCount(settings.packetRepeats)
  , throttleMultiplier(
      std::ceil(
        (settings.packetRepeatThrottleSensitivity / 1000.0) * settings.packetRepeats
      )
    )
{ }

bool PacketSender::enqueue(uint8_t* packet, const MiLightRemoteConfig* remoteConfig, const size_t repeatsOverride, bool isInternal) {
#ifdef DEBUG_PRINTF
  Serial.println("Enqueuing packet");
#endif
  size_t repeats = repeatsOverride == DEFAULT_PACKET_SENDS_VALUE
    ? this->currentResendCount
    : repeatsOverride;

  size_t pktLen = remoteConfig->packetFormatter->getPacketLength();
  recordTxEcho(packet, pktLen);
  return queue.push(packet, remoteConfig, repeats, isInternal);
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

bool PacketSender::isSending() {
  return packetRepeatsRemaining > 0 || !queue.isEmpty();
}

void PacketSender::nextPacket() {
#ifdef DEBUG_PRINTF
  Serial.printf("Switching to next packet, %d packets in queue\n", queue.size());
#endif
  currentPacket = queue.pop();
  if (currentPacket == nullptr) {
    packetRepeatsRemaining = 0;
    return;
  }

  if (currentPacket->repeatsOverride > 0) {
    packetRepeatsRemaining = currentPacket->repeatsOverride;
  } else {
    packetRepeatsRemaining = settings.packetRepeats;
  }

  // Adjust resend count according to throttling rules
  updateResendCount();
}

void PacketSender::handleCurrentPacket() {
  // Always switch radio.  could've been listening in another context
  radioSwitchboard.switchRadio(currentPacket->remoteConfig);

  size_t numToSend = std::min(packetRepeatsRemaining, settings.packetRepeatsPerLoop);
  sendRepeats(numToSend);
  packetRepeatsRemaining -= numToSend;

  // If we're done sending this packet, fire the sent packet callback with exact internal/external origin
  if (packetRepeatsRemaining == 0 && packetSentHandler != nullptr) {
    packetSentHandler(currentPacket->packet, *currentPacket->remoteConfig, currentPacket->isInternal);
  }
}

size_t PacketSender::queueLength() const {
  return queue.size();
}

size_t PacketSender::droppedPackets() const {
  return queue.getDroppedPacketCount();
}

size_t PacketSender::coalescedPackets() const {
  return queue.getCoalescedPacketCount();
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

  lastRadioWriteMs = millis();
  recordTxEcho(currentPacket->packet, len);
  for (size_t i = 0; i < num; ++i) {
    radioSwitchboard.write(currentPacket->packet, len);
  }
  lastRadioWriteMs = millis();
  recordTxEcho(currentPacket->packet, len);

#ifdef DEBUG_PRINTF
  int iElapsed = millis() - iStart;
  Serial.print("Elapsed: ");
  Serial.println(iElapsed);
#endif
}

void PacketSender::updateResendCount() {
  unsigned long now = millis();
  long millisSinceLastSend = now - lastSend;
  long x = (millisSinceLastSend - settings.packetRepeatThrottleThreshold);
  long delta = x * throttleMultiplier;
  int signedResends = static_cast<int>(this->currentResendCount) + delta;

  if (signedResends < static_cast<int>(settings.packetRepeatMinimum)) {
    signedResends = settings.packetRepeatMinimum;
  } else if (signedResends > static_cast<int>(settings.packetRepeats)) {
    signedResends = settings.packetRepeats;
  }

  this->currentResendCount = signedResends;
  this->lastSend = now;
}

void PacketSender::recordTxEcho(const uint8_t* packet, size_t len) {
  if (len == 0 || len > MILIGHT_MAX_PACKET_LENGTH) return;
  uint32_t now = millis();
#ifdef ESP32
  portENTER_CRITICAL(&echoMux);
#endif
  for (size_t i = 0; i < TX_ECHO_RING_SIZE; i++) {
    if (txEchoRing[i].len == len && memcmp(txEchoRing[i].bytes, packet, len) == 0) {
      txEchoRing[i].ts = now;
#ifdef ESP32
      portEXIT_CRITICAL(&echoMux);
#endif
      return;
    }
  }
  TxEchoSlot& slot = txEchoRing[txEchoHead];
  memcpy(slot.bytes, packet, len);
  slot.len = (uint8_t)len;
  slot.ts = now;
  txEchoHead = (txEchoHead + 1) % TX_ECHO_RING_SIZE;
#ifdef ESP32
  portEXIT_CRITICAL(&echoMux);
#endif
}

bool PacketSender::isRecentLocalTx(const uint8_t* packet, size_t len, uint32_t windowMs) const {
  if (len == 0 || len > MILIGHT_MAX_PACKET_LENGTH) return false;
  uint32_t now = millis();
  // Hardware half-duplex air guard: if Radio 1 is actively transmitting right now or finished < 280ms ago,
  // any packet arriving on Radio 2 is Radio 1's own RF transmission!
  if (packetRepeatsRemaining > 0 || !queue.isEmpty() || (lastRadioWriteMs > 0 && (now - lastRadioWriteMs <= 280))) {
    return true;
  }
  bool match = false;
#ifdef ESP32
  portENTER_CRITICAL(&echoMux);
#endif
  for (size_t i = 0; i < TX_ECHO_RING_SIZE; i++) {
    if (txEchoRing[i].len == len && (now - txEchoRing[i].ts <= windowMs)) {
      if (memcmp(txEchoRing[i].bytes, packet, len) == 0) {
        match = true;
        break;
      }
    }
  }
#ifdef ESP32
  portEXIT_CRITICAL(&echoMux);
#endif
  return match;
}