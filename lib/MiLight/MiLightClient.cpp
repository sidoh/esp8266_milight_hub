#include <MiLightClient.h>
#include <MiLightRadioConfig.h>
#include <Arduino.h>
#include <RGBConverter.h>
#include <Units.h>

MiLightClient::MiLightClient(
  MiLightRadioFactory* radioFactory,
  GroupStateStore& stateStore,
  size_t throttleThreshold,
  size_t throttleSensitivity,
  size_t packetRepeatMinimum
)
  : baseResendCount(MILIGHT_DEFAULT_RESEND_COUNT),
    currentRadio(NULL),
    currentRemote(NULL),
    numRadios(MiLightRadioConfig::NUM_CONFIGS),
    packetSentHandler(NULL),
    updateBeginHandler(NULL),
    updateEndHandler(NULL),
    stateStore(stateStore),
    lastSend(0),
    throttleThreshold(throttleThreshold),
    throttleSensitivity(throttleSensitivity),
    packetRepeatMinimum(packetRepeatMinimum)
{
  radios = new MiLightRadio*[numRadios];

  for (size_t i = 0; i < numRadios; i++) {
    radios[i] = radioFactory->create(MiLightRadioConfig::ALL_CONFIGS[i]);
  }
}

void MiLightClient::begin() {
  for (size_t i = 0; i < numRadios; i++) {
    radios[i]->begin();
  }

  switchRadio(static_cast<size_t>(0));
}

void MiLightClient::setHeld(bool held) {
  currentRemote->packetFormatter->setHeld(held);
}

size_t MiLightClient::getNumRadios() const {
  return numRadios;
}

MiLightRadio* MiLightClient::switchRadio(size_t radioIx) {
  if (radioIx >= getNumRadios()) {
    return NULL;
  }

  if (this->currentRadio != radios[radioIx]) {
    this->currentRadio = radios[radioIx];
    this->currentRadio->configure();
  }

  return this->currentRadio;
}

MiLightRadio* MiLightClient::switchRadio(const MiLightRemoteConfig* remoteConfig) {
  MiLightRadio* radio;

  for (int i = 0; i < numRadios; i++) {
    if (&this->radios[i]->config() == &remoteConfig->radioConfig) {
      radio = switchRadio(i);
      break;
    }
  }

  return radio;
}

void MiLightClient::prepare(const MiLightRemoteConfig* config,
  const uint16_t deviceId,
  const uint8_t groupId
) {
  switchRadio(config);

  this->currentRemote = config;

  if (deviceId >= 0 && groupId >= 0) {
    currentRemote->packetFormatter->prepare(deviceId, groupId);
  }
}

void MiLightClient::prepare(const MiLightRemoteType type,
  const uint16_t deviceId,
  const uint8_t groupId
) {
  prepare(MiLightRemoteConfig::fromType(type));
}

void MiLightClient::setResendCount(const unsigned int resendCount) {
  this->baseResendCount = resendCount;
  this->currentResendCount = resendCount;
  this->throttleMultiplier = ceil((throttleSensitivity / 1000.0) * this->baseResendCount);
}

bool MiLightClient::available() {
  if (currentRadio == NULL) {
    return false;
  }

  return currentRadio->available();
}

size_t MiLightClient::read(uint8_t packet[]) {
  if (currentRadio == NULL) {
    return 0;
  }

  size_t length;
  currentRadio->read(packet, length);

  return length;
}

void MiLightClient::write(uint8_t packet[]) {
  if (currentRadio == NULL) {
    return;
  }

#ifdef DEBUG_PRINTF
  Serial.printf("Sending packet (%d repeats): \n", this->currentResendCount);
  for (int i = 0; i < currentRemote->packetFormatter->getPacketLength(); i++) {
    Serial.printf("%02X ", packet[i]);
  }
  Serial.println();
  int iStart = millis();
#endif

  for (int i = 0; i < this->currentResendCount; i++) {
    currentRadio->write(packet, currentRemote->packetFormatter->getPacketLength());
  }

  if (this->packetSentHandler) {
    this->packetSentHandler(packet, *currentRemote);
  }

#ifdef DEBUG_PRINTF
  int iElapsed = millis() - iStart;
  Serial.print("Elapsed: ");
  Serial.println(iElapsed);
#endif
}

void MiLightClient::updateColorRaw(const uint8_t color) {
  currentRemote->packetFormatter->updateColorRaw(color);
  flushPacket();
}

void MiLightClient::updateHue(const uint16_t hue) {
  currentRemote->packetFormatter->updateHue(hue);
  flushPacket();
}

void MiLightClient::updateBrightness(const uint8_t brightness) {
  currentRemote->packetFormatter->updateBrightness(brightness);
  flushPacket();
}

void MiLightClient::updateMode(uint8_t mode) {
  currentRemote->packetFormatter->updateMode(mode);
  flushPacket();
}

void MiLightClient::nextMode() {
  currentRemote->packetFormatter->nextMode();
  flushPacket();
}

void MiLightClient::previousMode() {
  currentRemote->packetFormatter->previousMode();
  flushPacket();
}

void MiLightClient::modeSpeedDown() {
  currentRemote->packetFormatter->modeSpeedDown();
  flushPacket();
}
void MiLightClient::modeSpeedUp() {
  currentRemote->packetFormatter->modeSpeedUp();
  flushPacket();
}

void MiLightClient::updateStatus(MiLightStatus status, uint8_t groupId) {
  currentRemote->packetFormatter->updateStatus(status, groupId);
  flushPacket();
}

void MiLightClient::updateStatus(MiLightStatus status) {
  currentRemote->packetFormatter->updateStatus(status);
  flushPacket();
}

void MiLightClient::updateSaturation(const uint8_t value) {
  currentRemote->packetFormatter->updateSaturation(value);
  flushPacket();
}

void MiLightClient::updateColorWhite() {
  currentRemote->packetFormatter->updateColorWhite();
  flushPacket();
}

void MiLightClient::enableNightMode() {
  currentRemote->packetFormatter->enableNightMode();
  flushPacket();
}

void MiLightClient::pair() {
  currentRemote->packetFormatter->pair();
  flushPacket();
}

void MiLightClient::unpair() {
  currentRemote->packetFormatter->unpair();
  flushPacket();
}

void MiLightClient::increaseBrightness() {
  currentRemote->packetFormatter->increaseBrightness();
  flushPacket();
}

void MiLightClient::decreaseBrightness() {
  currentRemote->packetFormatter->decreaseBrightness();
  flushPacket();
}

void MiLightClient::increaseTemperature() {
  currentRemote->packetFormatter->increaseTemperature();
  flushPacket();
}

void MiLightClient::decreaseTemperature() {
  currentRemote->packetFormatter->decreaseTemperature();
  flushPacket();
}

void MiLightClient::updateTemperature(const uint8_t temperature) {
  currentRemote->packetFormatter->updateTemperature(temperature);
  flushPacket();
}

void MiLightClient::command(uint8_t command, uint8_t arg) {
  currentRemote->packetFormatter->command(command, arg);
  flushPacket();
}

void MiLightClient::update(const JsonObject& request) {
  if (this->updateBeginHandler) {
    this->updateBeginHandler();
  }

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

  //Homeassistant - Handle effect
  if (request.containsKey("effect")) {
    this->handleEffect(request["effect"]);
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
    //If close to white
    if( r > 256 - RGB_WHITE_BOUNDARY && g > 256 - RGB_WHITE_BOUNDARY && b > 256 - RGB_WHITE_BOUNDARY) {
        this->updateColorWhite();
    } else {
      double hsv[3];
      RGBConverter converter;
      converter.rgbToHsv(r, g, b, hsv);

      uint16_t hue = round(hsv[0]*360);
      uint8_t saturation = round(hsv[1]*100);

      this->updateHue(hue);
      this->updateSaturation(saturation);
    }
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

  if (request.containsKey("mode")) {
    this->updateMode(request["mode"]);
  }

  // Raw packet command/args
  if (request.containsKey("button_id") && request.containsKey("argument")) {
    this->command(request["button_id"], request["argument"]);
  }

  // Always turn off last
  if (parsedStatus == OFF) {
    this->updateStatus(OFF);
  }

  if (this->updateEndHandler) {
    this->updateEndHandler();
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

void MiLightClient::handleEffect(const String& effect) {
  if (effect == "night_mode") {
    this->enableNightMode();
  } else if (effect == "white") {
    this->updateColorWhite();
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

void MiLightClient::updateResendCount() {
  unsigned long now = millis();
  long millisSinceLastSend = now - lastSend;
  long x = (millisSinceLastSend - throttleThreshold);
  long delta = x * throttleMultiplier;

  this->currentResendCount = constrain(this->currentResendCount + delta, packetRepeatMinimum, this->baseResendCount);
  this->lastSend = now;
}

void MiLightClient::flushPacket() {
  PacketStream& stream = currentRemote->packetFormatter->buildPackets();
  updateResendCount();

  while (stream.hasNext()) {
    write(stream.next());

    if (stream.hasNext()) {
      delay(10);
    }
  }

  currentRemote->packetFormatter->reset();
}

void MiLightClient::onPacketSent(PacketSentHandler handler) {
  this->packetSentHandler = handler;
}

void MiLightClient::onUpdateBegin(EventHandler handler) {
  this->updateBeginHandler = handler;
}

void MiLightClient::onUpdateEnd(EventHandler handler) {
  this->updateEndHandler = handler;
}
