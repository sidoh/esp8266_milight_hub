#include <MiLightClient.h>
#include <MiLightRadioConfig.h>
#include <Arduino.h>
#include <RGBConverter.h>
#include <Units.h>

MiLightClient::MiLightClient(
  MiLightRadioFactory* radioFactory,
  GroupStateStore* stateStore,
  Settings* settings
)
  : baseResendCount(MILIGHT_DEFAULT_RESEND_COUNT),
    currentRadio(NULL),
    currentRemote(NULL),
    numRadios(MiLightRadioConfig::NUM_CONFIGS),
    packetSentHandler(NULL),
    updateBeginHandler(NULL),
    updateEndHandler(NULL),
    stateStore(stateStore),
    settings(settings),
    lastSend(0)
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

  // Little gross to do this here as it's relying on global state.  A better alternative
  // would be to statically construct remote config factories which take in a stateStore 
  // and settings pointer.  The objects could then be initialized by calling the factory
  // in main.
  for (size_t i = 0; i < MiLightRemoteConfig::NUM_REMOTES; i++) {
    MiLightRemoteConfig::ALL_REMOTES[i]->packetFormatter->initialize(stateStore, settings);
  }
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
  prepare(MiLightRemoteConfig::fromType(type), deviceId, groupId);
}

void MiLightClient::setResendCount(const unsigned int resendCount) {
  this->baseResendCount = resendCount;
  this->currentResendCount = resendCount;
  this->throttleMultiplier = ceil((settings->packetRepeatThrottleSensitivity / 1000.0) * this->baseResendCount);
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
  Serial.printf_P(PSTR("Sending packet (%d repeats): \n"), this->currentResendCount);
  for (int i = 0; i < currentRemote->packetFormatter->getPacketLength(); i++) {
    Serial.printf_P(PSTR("%02X "), packet[i]);
  }
  Serial.println();
  int iStart = millis();
#endif

  // send the packet out (multiple times for "reliability")
  for (int i = 0; i < this->currentResendCount; i++) {
    currentRadio->write(packet, currentRemote->packetFormatter->getPacketLength());
  }

  // if we have a packetSendHandler defined (see MiLightClient::onPacketSent), call it now that
  // the packet has been dispatched
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
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::updateColorRaw: Change color to %d\n"), color);
#endif
  currentRemote->packetFormatter->updateColorRaw(color);
  flushPacket();
}

void MiLightClient::updateHue(const uint16_t hue) {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::updateHue: Change hue to %d\n"), hue);
#endif
  currentRemote->packetFormatter->updateHue(hue);
  flushPacket();
}

void MiLightClient::updateBrightness(const uint8_t brightness) {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::updateBrightness: Change brightness to %d\n"), brightness);
#endif
  currentRemote->packetFormatter->updateBrightness(brightness);
  flushPacket();
}

void MiLightClient::updateMode(uint8_t mode) {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::updateMode: Change mode to %d\n"), mode);
#endif
  currentRemote->packetFormatter->updateMode(mode);
  flushPacket();
}

void MiLightClient::nextMode() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::nextMode: Switch to next mode"));
#endif
  currentRemote->packetFormatter->nextMode();
  flushPacket();
}

void MiLightClient::previousMode() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::previousMode: Switch to previous mode"));
#endif
  currentRemote->packetFormatter->previousMode();
  flushPacket();
}

void MiLightClient::modeSpeedDown() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::modeSpeedDown: Speed down\n"));
#endif
  currentRemote->packetFormatter->modeSpeedDown();
  flushPacket();
}
void MiLightClient::modeSpeedUp() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::modeSpeedUp: Speed up"));
#endif
  currentRemote->packetFormatter->modeSpeedUp();
  flushPacket();
}

void MiLightClient::updateStatus(MiLightStatus status, uint8_t groupId) {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::updateStatus: Status %s, groupId %d\n"), status == MiLightStatus::OFF ? "OFF" : "ON", groupId);
#endif
  currentRemote->packetFormatter->updateStatus(status, groupId);
  flushPacket();
}

void MiLightClient::updateStatus(MiLightStatus status) {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::updateStatus: Status %s\n"), status == MiLightStatus::OFF ? "OFF" : "ON");
#endif
  currentRemote->packetFormatter->updateStatus(status);
  flushPacket();
}

void MiLightClient::updateSaturation(const uint8_t value) {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::updateSaturation: Saturation %d\n"), value);
#endif
  currentRemote->packetFormatter->updateSaturation(value);
  flushPacket();
}

void MiLightClient::updateColorWhite() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::updateColorWhite: Color white"));
#endif
  currentRemote->packetFormatter->updateColorWhite();
  flushPacket();
}

void MiLightClient::enableNightMode() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::enableNightMode: Night mode"));
#endif
  currentRemote->packetFormatter->enableNightMode();
  flushPacket();
}

void MiLightClient::pair() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::pair: Pair"));
#endif
  currentRemote->packetFormatter->pair();
  flushPacket();
}

void MiLightClient::unpair() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::unpair: Unpair"));
#endif
  currentRemote->packetFormatter->unpair();
  flushPacket();
}

void MiLightClient::increaseBrightness() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::increaseBrightness: Increase brightness"));
#endif
  currentRemote->packetFormatter->increaseBrightness();
  flushPacket();
}

void MiLightClient::decreaseBrightness() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::decreaseBrightness: Decrease brightness"));
#endif
  currentRemote->packetFormatter->decreaseBrightness();
  flushPacket();
}

void MiLightClient::increaseTemperature() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::increaseTemperature: Increase temperature"));
#endif
  currentRemote->packetFormatter->increaseTemperature();
  flushPacket();
}

void MiLightClient::decreaseTemperature() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.println(F("MiLightClient::decreaseTemperature: Decrease temperature"));
#endif
  currentRemote->packetFormatter->decreaseTemperature();
  flushPacket();
}

void MiLightClient::updateTemperature(const uint8_t temperature) {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::updateTemperature: Set temperature to %d\n"), temperature);
#endif
  currentRemote->packetFormatter->updateTemperature(temperature);
  flushPacket();
}

void MiLightClient::command(uint8_t command, uint8_t arg) {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::command: Execute command %d, argument %d\n"), command, arg);
#endif
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

    int16_t r = color["r"];
    int16_t g = color["g"];
    int16_t b = color["b"];

    // We consider an RGB color "white" if all color intensities are roughly the
    // same value.  An unscientific value of 10 (~4%) is chosen.
    if ( abs(r - g) < RGB_WHITE_THRESHOLD
      && abs(g - b) < RGB_WHITE_THRESHOLD
      && abs(r - b) < RGB_WHITE_THRESHOLD) {
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
  } else if (effect == "white" || effect == "white_mode") {
    this->updateColorWhite();
  } else { // assume we're trying to set mode
    this->updateMode(effect.toInt());
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
  long x = (millisSinceLastSend - settings->packetRepeatThrottleThreshold);
  long delta = x * throttleMultiplier;

  this->currentResendCount = constrain(this->currentResendCount + delta, settings->packetRepeatMinimum, this->baseResendCount);
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

/*
  Register a callback for when packets are sent
*/
void MiLightClient::onPacketSent(PacketSentHandler handler) {
  this->packetSentHandler = handler;
}

void MiLightClient::onUpdateBegin(EventHandler handler) {
  this->updateBeginHandler = handler;
}

void MiLightClient::onUpdateEnd(EventHandler handler) {
  this->updateEndHandler = handler;
}
