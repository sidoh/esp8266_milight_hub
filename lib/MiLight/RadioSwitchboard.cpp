#include <RadioSwitchboard.h>

RadioSwitchboard::RadioSwitchboard(
  std::shared_ptr<MiLightRadioFactory> radioFactory,
  GroupStateStore* stateStore,
  Settings& settings
) {
  for (size_t i = 0; i < MiLightRadioConfig::NUM_CONFIGS; i++) {
    std::shared_ptr<MiLightRadio> radio = radioFactory->create(MiLightRadioConfig::ALL_CONFIGS[i]);
    radio->begin();
    radios.push_back(radio);
  }

  for (size_t i = 0; i < MiLightRemoteConfig::NUM_REMOTES; i++) {
    MiLightRemoteConfig::ALL_REMOTES[i]->packetFormatter->initialize(stateStore, &settings);
  }
}

size_t RadioSwitchboard::getNumRadios() const {
  return radios.size();
}

std::shared_ptr<MiLightRadio> RadioSwitchboard::switchRadio(size_t radioIx) {
  if (radioIx >= getNumRadios()) {
    return NULL;
  }

  if (this->currentRadio != radios[radioIx]) {
    this->currentRadio = radios[radioIx];
    this->currentRadio->configure();
  }

  return this->currentRadio;
}

std::shared_ptr<MiLightRadio> RadioSwitchboard::switchRadio(const MiLightRemoteConfig* remote) {
  std::shared_ptr<MiLightRadio> radio = NULL;

  for (size_t i = 0; i < radios.size(); i++) {
    if (&this->radios[i]->config() == &remote->radioConfig) {
      radio = switchRadio(i);
      break;
    }
  }

  return radio;
}

void RadioSwitchboard::write(uint8_t* packet, size_t len) {
  if (this->currentRadio == nullptr) {
    return;
  }

  this->currentRadio->write(packet, len);
}

size_t RadioSwitchboard::read(uint8_t* packet) {
  if (currentRadio == nullptr) {
    return 0;
  }

  size_t length;
  currentRadio->read(packet, length);

  return length;
}

bool RadioSwitchboard::available() {
  if (currentRadio == nullptr) {
    return false;
  }

  return currentRadio->available();
}