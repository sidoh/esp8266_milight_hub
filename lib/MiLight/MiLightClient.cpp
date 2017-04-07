#include <MiLightClient.h>
#include <MiLightRadioConfig.h>
#include <Arduino.h>

MiLightClient::MiLightClient(MiLightRadioFactory* radioFactory)
  : resendCount(MILIGHT_DEFAULT_RESEND_COUNT),
    currentRadio(NULL),
    numRadios(MiLightRadioConfig::NUM_CONFIGS)
{
  radios = new MiLightRadio*[numRadios];

  for (size_t i = 0; i < numRadios; i++) {
    radios[i] = radioFactory->create(*MiLightRadioConfig::ALL_CONFIGS[i]);
  }

  this->currentRadio = radios[0];
  this->currentRadio->configure();
}

void MiLightClient::begin() {
  for (size_t i = 0; i < numRadios; i++) {
    radios[i]->begin();
  }
}

void MiLightClient::setHeld(bool held) {
  formatter->setHeld(held);
}

MiLightRadio* MiLightClient::switchRadio(const MiLightRadioType type) {
  MiLightRadio* radio = NULL;

  for (int i = 0; i < numRadios; i++) {
    if (this->radios[i]->config().type == type) {
      radio = radios[i];
      break;
    }
  }

  if (radio != NULL) {
    if (currentRadio != radio) {
      radio->configure();
    }

    this->currentRadio = radio;
    this->formatter = radio->config().packetFormatter;

    return radio;
  } else {
    Serial.print(F("MiLightClient - tried to get radio for unknown type: "));
    Serial.println(type);
  }

  return NULL;
}


void MiLightClient::prepare(MiLightRadioConfig& config,
  const uint16_t deviceId,
  const uint8_t groupId) {

  switchRadio(config.type);

  if (deviceId >= 0 && groupId >= 0) {
    formatter->prepare(deviceId, groupId);
  }
}

void MiLightClient::setResendCount(const unsigned int resendCount) {
  this->resendCount = resendCount;
}


bool MiLightClient::available() {
  if (currentRadio == NULL) {
    return false;
  }

  return currentRadio->available();
}
void MiLightClient::read(uint8_t packet[]) {
  if (currentRadio == NULL) {
    return;
  }

  size_t length = currentRadio->config().getPacketLength();

  currentRadio->read(packet, length);
}

void MiLightClient::write(uint8_t packet[]) {
  if (currentRadio == NULL) {
    return;
  }

#ifdef DEBUG_PRINTF
  printf("Sending packet: ");
  for (int i = 0; i < currentRadio->config().getPacketLength(); i++) {
    printf("%02X", packet[i]);
  }
  printf("\n");
  int iStart = millis();
#endif

  for (int i = 0; i < this->resendCount; i++) {
    currentRadio->write(packet, currentRadio->config().getPacketLength());
  }

#ifdef DEBUG_PRINTF
  int iElapsed = millis() - iStart;
  Serial.print("Elapsed: ");
  Serial.println(iElapsed);
#endif
}

void MiLightClient::updateColorRaw(const uint8_t color) {
  formatter->updateColorRaw(color);
  flushPacket();
}

void MiLightClient::updateHue(const uint16_t hue) {
  formatter->updateHue(hue);
  flushPacket();
}

void MiLightClient::updateBrightness(const uint8_t brightness) {
  formatter->updateBrightness(brightness);
  flushPacket();
}

void MiLightClient::updateMode(uint8_t mode) {
  formatter->updateMode(mode);
  flushPacket();
}

void MiLightClient::nextMode() {
  formatter->nextMode();
  flushPacket();
}

void MiLightClient::previousMode() {
  formatter->previousMode();
  flushPacket();
}

void MiLightClient::modeSpeedDown() {
  formatter->modeSpeedDown();
  flushPacket();
}
void MiLightClient::modeSpeedUp() {
  formatter->modeSpeedUp();
  flushPacket();
}

void MiLightClient::updateStatus(MiLightStatus status, uint8_t groupId) {
  formatter->updateStatus(status, groupId);
  flushPacket();
}

void MiLightClient::updateStatus(MiLightStatus status) {
  formatter->updateStatus(status);
  flushPacket();
}

void MiLightClient::updateSaturation(const uint8_t value) {
  formatter->updateSaturation(value);
  flushPacket();
}

void MiLightClient::updateColorWhite() {
  formatter->updateColorWhite();
  flushPacket();
}

void MiLightClient::pair() {
  formatter->pair();
  flushPacket();
}

void MiLightClient::unpair() {
  formatter->unpair();
  flushPacket();
}

void MiLightClient::increaseBrightness() {
  formatter->increaseBrightness();
  flushPacket();
}

void MiLightClient::decreaseBrightness() {
  formatter->decreaseBrightness();
  flushPacket();
}

void MiLightClient::increaseTemperature() {
  formatter->increaseTemperature();
  flushPacket();
}

void MiLightClient::decreaseTemperature() {
  formatter->decreaseTemperature();
  flushPacket();
}

void MiLightClient::updateTemperature(const uint8_t temperature) {
  formatter->updateTemperature(temperature);
  flushPacket();
}

void MiLightClient::command(uint8_t command, uint8_t arg) {
  formatter->command(command, arg);
  flushPacket();
}

void MiLightClient::formatPacket(uint8_t* packet, char* buffer) {
  formatter->format(packet, buffer);
}

void MiLightClient::flushPacket() {
  PacketStream& stream = formatter->buildPackets();
  const size_t prevNumRepeats = this->resendCount;

  // When sending multiple packets, normalize the number of repeats
  if (stream.numPackets > 1) {
    setResendCount(MILIGHT_DEFAULT_RESEND_COUNT);
  }

  while (stream.hasNext()) {
    write(stream.next());

    if (stream.hasNext()) {
      delay(10);
    }
  }

  setResendCount(prevNumRepeats);
  formatter->reset();
}
