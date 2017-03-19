#include <MiLightClient.h>
#include <MiLightRadioConfig.h>
#include <Arduino.h>

MiLightRadio* MiLightClient::switchRadio(const MiLightRadioType type) {
  RadioStack* stack = NULL;
  
  for (int i = 0; i < NUM_RADIOS; i++) {
    if (radios[i]->config.type == type) {
      stack = radios[i];
      break;
    }
  }
  
  if (stack != NULL) {
    MiLightRadio *radio = stack->getRadio();
    
    if (currentRadio->config.type != stack->config.type) {
      radio->configure();
    }
    
    currentRadio = stack;
    formatter = stack->config.packetFormatter;
    return radio;
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
  
  return currentRadio->getRadio()->available();
}

void MiLightClient::read(uint8_t packet[]) {
  if (currentRadio == NULL) {
    return;
  }
  
  size_t length;
  currentRadio->getRadio()->read(packet, length);
}

void MiLightClient::write(uint8_t packet[]) {
  if (currentRadio == NULL) {
    return;
  }
  
  for (int i = 0; i < this->resendCount; i++) {
    currentRadio->getRadio()->write(packet, currentRadio->config.packetLength);
  }
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
  for (size_t i = 0; i < 5; i++) {
    formatter->updateStatus(ON);
    flushPacket();
    delay(1);
  }
}

void MiLightClient::unpair() {
  MiLightRadioType type = currentRadio->config.type;
  
  if (type == RGBW) {
    formatter->updateStatus(ON);
    flushPacket();
    yield();
    formatter->updateColorWhite();
    flushPacket();
  } else if (type == CCT) {
    for (int i = 0; i < 5; i++) {
      formatter->updateStatus(ON);
      flushPacket();
      delay(1);
    }
  } else if (type == RGB_CCT) {
    for (int i = 0; i < 5; i++) {
      formatter->updateStatus(ON, 0);
      flushPacket();
      delay(1);
    }
  }
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

void MiLightClient::formatPacket(MiLightRadioConfig& config, uint8_t* packet, char* buffer) {
  if (config.type == RGBW || config.type == CCT) {
    String format = String("Request type  : %02X\n") 
      + "Device ID     : %02X%02X\n"
      + "b1            : %02X\n"
      + "b2            : %02X\n"
      + "b3            : %02X\n"
      + "Sequence Num. : %02X";
      
    sprintf(
      buffer,
      format.c_str(),
      packet[0],
      packet[1], packet[2],
      packet[3],
      packet[4],
      packet[5],
      packet[6]
    );
  } else {
    for (int i = 0; i < config.packetLength; i++) {
      sprintf(buffer, "%02X ", packet[i]);
      buffer += 3;
    }
    sprintf(buffer, "\n\n");
  }
}
    
void MiLightClient::flushPacket() {
  write(formatter->buildPacket());
  formatter->reset();
}
