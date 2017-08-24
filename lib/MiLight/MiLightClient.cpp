#include <MiLightClient.h>
#include <MiLightRadioConfig.h>
#include <Arduino.h>
#include <RGBConverter.h>
#include <Units.h>

MiLightClient::MiLightClient(MiLightRadioFactory* radioFactory)
  : resendCount(MILIGHT_DEFAULT_RESEND_COUNT),
    currentRadio(NULL),
    numRadios(MiLightRadioConfig::NUM_CONFIGS),
    packetSentHandler(NULL)
{
  radios = new MiLightRadio*[numRadios];

  for (size_t i = 0; i < numRadios; i++) {
    radios[i] = radioFactory->create(*MiLightRadioConfig::ALL_CONFIGS[i]);
  }
}

void MiLightClient::begin() {
  for (size_t i = 0; i < numRadios; i++) {
    radios[i]->begin();
  }

  this->currentRadio = radios[0];
  this->currentRadio->configure();
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

  prepare(config.type, deviceId, groupId);
}

void MiLightClient::prepare(MiLightRadioType type,
  const uint16_t deviceId,
  const uint8_t groupId) {

  switchRadio(type);

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

  if (this->packetSentHandler) {
    this->packetSentHandler(packet, currentRadio->config());
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

void MiLightClient::enableNightMode() {
  formatter->enableNightMode();
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

void MiLightClient::update(const JsonObject& request) {
  const uint8_t parsedStatus = this->parseStatus(request);

  // Always turn on first
  if (parsedStatus == ON) {
    this->updateStatus(ON);
  }

  if (request.containsKey("command")) {
    this->handleCommand(request["command"]);
  }

  if (request.containsKey("commands")) {
    JsonArray& commands = request["commands"];

    if (commands.success()) {
      for (size_t i = 0; i < commands.size(); i++) {
        this->handleCommand(commands.get<String>(i));
      }
    }
  }

  if (request.containsKey("hue")) {
    this->updateHue(request["hue"]);
  }
  if (request.containsKey("saturation")) {
    this->updateSaturation(request["saturation"]);
  }

  // Convert RGB to HSV
  if (request.containsKey("color")) {
    JsonObject& color = request["color"];

    uint8_t r = color["r"];
    uint8_t g = color["g"];
    uint8_t b = color["b"];

    double hsv[3];
    RGBConverter converter;
    converter.rgbToHsv(r, g, b, hsv);

    uint16_t hue = round(hsv[0]*360);
    uint8_t saturation = round(hsv[1]*100);

    this->updateHue(hue);
    this->updateSaturation(saturation);
  }

  if (request.containsKey("level")) {
    this->updateBrightness(request["level"]);
  }
  // HomeAssistant
  if (request.containsKey("brightness")) {
    uint8_t scaledBrightness = Units::rescale(request.get<uint8_t>("brightness"), 100, 255);
    this->updateBrightness(scaledBrightness);
  }

  if (request.containsKey("temperature")) {
    this->updateTemperature(request["temperature"]);
  }
  // HomeAssistant
  if (request.containsKey("color_temp")) {
    this->updateTemperature(
      Units::miredsToWhiteVal(request["color_temp"], 100)
    );
  }

  //Homeassistant - Switch to white if white value is set
  if (request.containsKey("white_value")) {
    this->updateColorWhite();
  }
  
  if (request.containsKey("mode")) {
    this->updateMode(request["mode"]);
  }

  // Always turn off last
  if (parsedStatus == OFF) {
    this->updateStatus(OFF);
  }
}

void MiLightClient::handleCommand(const String& command) {
  if (command == "unpair") {
    this->unpair();
  } else if (command == "pair") {
    this->pair();
  } else if (command == "set_white") {
    this->updateColorWhite();
  } else if (command == "night_mode") {
    this->enableNightMode();
  } else if (command == "level_up") {
    this->increaseBrightness();
  } else if (command == "level_down") {
    this->decreaseBrightness();
  } else if (command == "temperature_up") {
    this->increaseTemperature();
  } else if (command == "temperature_down") {
    this->decreaseTemperature();
  } else if (command == "next_mode") {
    this->nextMode();
  } else if (command == "previous_mode") {
    this->previousMode();
  } else if (command == "mode_speed_down") {
    this->modeSpeedDown();
  } else if (command == "mode_speed_up") {
    this->modeSpeedUp();
  }
}

uint8_t MiLightClient::parseStatus(const JsonObject& object) {
  String strStatus;

  if (object.containsKey("status")) {
    strStatus = object.get<char*>("status");
  } else if (object.containsKey("state")) {
    strStatus = object.get<char*>("state");
  } else {
    return 255;
  }

  return (strStatus.equalsIgnoreCase("on") || strStatus.equalsIgnoreCase("true")) ? ON : OFF;
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

void MiLightClient::onPacketSent(PacketSentHandler handler) {
  this->packetSentHandler = handler;
}
