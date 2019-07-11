#include <MiLightClient.h>
#include <MiLightRadioConfig.h>
#include <Arduino.h>
#include <RGBConverter.h>
#include <Units.h>
#include <TokenIterator.h>
#include <ParsedColor.h>
#include <MiLightCommands.h>
#include <functional>

using namespace std::placeholders;

const std::map<const char*, std::function<void(MiLightClient*, JsonVariant)>> MiLightClient::FIELD_SETTERS = {
  {GroupStateFieldNames::LEVEL, &MiLightClient::updateBrightness},
  {
    GroupStateFieldNames::BRIGHTNESS,
    [](MiLightClient* client, uint16_t arg) {
      client->updateBrightness(Units::rescale(arg, 255, 100));
    }
  },
  {GroupStateFieldNames::HUE, &MiLightClient::updateHue},
  {GroupStateFieldNames::SATURATION, &MiLightClient::updateSaturation},
  {GroupStateFieldNames::KELVIN, &MiLightClient::updateTemperature},
  {GroupStateFieldNames::TEMPERATURE, &MiLightClient::updateTemperature},
  {
    GroupStateFieldNames::COLOR_TEMP,
    [](MiLightClient* client, uint16_t arg) {
      client->updateTemperature(Units::miredsToWhiteVal(arg, 100));
    }
  },
  {GroupStateFieldNames::MODE, &MiLightClient::updateMode},
  {GroupStateFieldNames::COLOR, &MiLightClient::updateColor},
  {GroupStateFieldNames::EFFECT, &MiLightClient::handleEffect},
  {GroupStateFieldNames::COMMAND, &MiLightClient::handleCommand},
  {GroupStateFieldNames::COMMANDS, &MiLightClient::handleCommands}
};

MiLightClient::MiLightClient(
  RadioSwitchboard& radioSwitchboard,
  PacketSender& packetSender,
  GroupStateStore* stateStore,
  Settings& settings,
  TransitionController& transitions
) : radioSwitchboard(radioSwitchboard)
  , updateBeginHandler(NULL)
  , updateEndHandler(NULL)
  , stateStore(stateStore)
  , settings(settings)
  , packetSender(packetSender)
  , transitions(transitions)
  , repeatsOverride(0)
{ }

void MiLightClient::setHeld(bool held) {
  currentRemote->packetFormatter->setHeld(held);
}

void MiLightClient::prepare(
  const MiLightRemoteConfig* config,
  const uint16_t deviceId,
  const uint8_t groupId
) {
  this->currentRemote = config;

  if (deviceId >= 0 && groupId >= 0) {
    currentRemote->packetFormatter->prepare(deviceId, groupId);
  }
}

void MiLightClient::prepare(
  const MiLightRemoteType type,
  const uint16_t deviceId,
  const uint8_t groupId
) {
  prepare(MiLightRemoteConfig::fromType(type), deviceId, groupId);
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

void MiLightClient::toggleStatus() {
#ifdef DEBUG_CLIENT_COMMANDS
  Serial.printf_P(PSTR("MiLightClient::toggleStatus"));
#endif
  currentRemote->packetFormatter->toggleStatus();
  flushPacket();
}

void MiLightClient::updateColor(JsonVariant json) {
  ParsedColor color = ParsedColor::fromJson(json);

  if (!color.success) {
    Serial.println(F("Error parsing JSON color"));
    return;
  }

  // We consider an RGB color "white" if all color intensities are roughly the
  // same value.  An unscientific value of 10 (~4%) is chosen.
  if ( abs(color.r - color.g) < RGB_WHITE_THRESHOLD
    && abs(color.g - color.b) < RGB_WHITE_THRESHOLD
    && abs(color.r - color.b) < RGB_WHITE_THRESHOLD) {
      this->updateColorWhite();
  } else {
    this->updateHue(color.hue);
    this->updateSaturation(color.saturation);
  }
}

void MiLightClient::update(JsonObject request) {
  if (this->updateBeginHandler) {
    this->updateBeginHandler();
  }

  const uint8_t parsedStatus = this->parseStatus(request);

  // Always turn on first
  if (parsedStatus == ON) {
    this->updateStatus(ON);
  }

  for (auto jsonKv : request) {
    const char* fieldName = jsonKv.key().c_str();
    auto handler = MiLightClient::FIELD_SETTERS.find(fieldName);

    if (handler != FIELD_SETTERS.end()) {
      if (request.containsKey(RequestKeys::TRANSITION)) {
        handleTransition(
          GroupStateFieldHelpers::getFieldByName(fieldName),
          jsonKv.value(),
          request[RequestKeys::TRANSITION].as<float>()
        );
      } else { // set the field directly
        handler->second(this, jsonKv.value());
      }
    }
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

void MiLightClient::handleCommands(JsonArray commands) {
  if (! commands.isNull()) {
    for (size_t i = 0; i < commands.size(); i++) {
      this->handleCommand(commands[i]);
    }
  }
}

void MiLightClient::handleCommand(JsonVariant command) {
  String cmdName;
  JsonObject args;

  if (command.is<JsonObject>()) {
    JsonObject cmdObj = command.as<JsonObject>();
    cmdName = cmdObj[GroupStateFieldNames::COMMAND].as<const char*>();
    args = cmdObj["args"];
  } else if (command.is<const char*>()) {
    cmdName = command.as<const char*>();
  }

  if (cmdName == MiLightCommandNames::UNPAIR) {
    this->unpair();
  } else if (cmdName == MiLightCommandNames::PAIR) {
    this->pair();
  } else if (cmdName == MiLightCommandNames::SET_WHITE) {
    this->updateColorWhite();
  } else if (cmdName == MiLightCommandNames::NIGHT_MODE) {
    this->enableNightMode();
  } else if (cmdName == MiLightCommandNames::LEVEL_UP) {
    this->increaseBrightness();
  } else if (cmdName == MiLightCommandNames::LEVEL_DOWN) {
    this->decreaseBrightness();
  } else if (cmdName == MiLightCommandNames::TEMPERATURE_UP) {
    this->increaseTemperature();
  } else if (cmdName == MiLightCommandNames::TEMPERATURE_DOWN) {
    this->decreaseTemperature();
  } else if (cmdName == MiLightCommandNames::NEXT_MODE) {
    this->nextMode();
  } else if (cmdName == MiLightCommandNames::PREVIOUS_MODE) {
    this->previousMode();
  } else if (cmdName == MiLightCommandNames::MODE_SPEED_DOWN) {
    this->modeSpeedDown();
  } else if (cmdName == MiLightCommandNames::MODE_SPEED_UP) {
    this->modeSpeedUp();
  } else if (cmdName == MiLightCommandNames::TOGGLE) {
    this->toggleStatus();
  } else if (cmdName == MiLightCommandNames::TRANSITION) {
    this->handleTransition(args);
  }
}

void MiLightClient::handleTransition(GroupStateField field, JsonVariant value, float duration) {
  BulbId bulbId = currentRemote->packetFormatter->currentBulbId();
  GroupState* currentState = stateStore->get(bulbId);
  std::shared_ptr<Transition::Builder> transitionBuilder = nullptr;

  if (currentState == nullptr) {
    Serial.println(F("Error planning transition: could not find current bulb state."));
    return;
  }

  if (!currentState->isSetField(field)) {
    Serial.println(F("Error planning transition: current state for field could not be determined"));
    return;
  }

  if (field == GroupStateField::COLOR) {
    ParsedColor currentColor = currentState->getColor();
    ParsedColor endColor = ParsedColor::fromJson(value);

    transitionBuilder = transitions.buildColorTransition(
      bulbId,
      currentColor,
      endColor
    );
  } else {
    uint16_t currentValue = currentState->getFieldValue(field);
    uint16_t endValue = value;

    transitionBuilder = transitions.buildFieldTransition(
      bulbId,
      field,
      currentValue,
      endValue
    );
  }

  if (transitionBuilder == nullptr) {
    Serial.printf_P(PSTR("Unsupported transition field: %s\n"), GroupStateFieldHelpers::getFieldName(field));
    return;
  }

  transitionBuilder->setDuration(duration);
  transitions.addTransition(transitionBuilder->build());
}

void MiLightClient::handleTransition(JsonObject args) {
  if (! args.containsKey(FS(TransitionParams::FIELD))
    || ! args.containsKey(FS(TransitionParams::START_VALUE))
    || ! args.containsKey(FS(TransitionParams::END_VALUE))) {
    Serial.println(F("Ignoring transition missing required arguments"));
    return;
  }

  const char* fieldName = args[FS(TransitionParams::FIELD)];
  GroupStateField field = GroupStateFieldHelpers::getFieldByName(fieldName);
  std::shared_ptr<Transition::Builder> transitionBuilder = nullptr;

  if (field == GroupStateField::UNKNOWN) {
    Serial.printf_P(PSTR("Unknown transition field: %s\n"), fieldName);
    return;
  }

  // These fields can be transitioned directly.
  switch (field) {
    case GroupStateField::HUE:
    case GroupStateField::SATURATION:
    case GroupStateField::BRIGHTNESS:
    case GroupStateField::LEVEL:
    case GroupStateField::KELVIN:
    case GroupStateField::COLOR_TEMP:
      transitionBuilder = transitions.buildFieldTransition(
        currentRemote->packetFormatter->currentBulbId(),
        field,
        args[FS(TransitionParams::START_VALUE)],
        args[FS(TransitionParams::END_VALUE)]
      );
      break;

    default:
      break;
  }

  // Color can be decomposed into hue/saturation and these can be transitioned separately
  if (field == GroupStateField::COLOR) {
    ParsedColor startColor = ParsedColor::fromJson(args[FS(TransitionParams::START_VALUE)]);
    ParsedColor endColor = ParsedColor::fromJson(args[FS(TransitionParams::END_VALUE)]);

    if (! startColor.success) {
      Serial.println(F("Transition - error parsing start color"));
      return;
    }
    if (! endColor.success) {
      Serial.println(F("Transition - error parsing end color"));
      return;
    }

    transitionBuilder = transitions.buildColorTransition(
      currentRemote->packetFormatter->currentBulbId(),
      startColor,
      endColor
    );
  }

  if (transitionBuilder == nullptr) {
    Serial.printf_P(PSTR("Unsupported transition field: %s\n"), fieldName);
    return;
  }

  if (args.containsKey(FS(TransitionParams::DURATION))) {
    transitionBuilder->setDuration(args[FS(TransitionParams::DURATION)]);
  }
  if (args.containsKey(FS(TransitionParams::PERIOD))) {
    transitionBuilder->setPeriod(args[FS(TransitionParams::PERIOD)]);
  }
  if (args.containsKey(FS(TransitionParams::NUM_PERIODS))) {
    transitionBuilder->setNumPeriods(args[FS(TransitionParams::NUM_PERIODS)]);
  }

  transitions.addTransition(transitionBuilder->build());
}

void MiLightClient::handleEffect(const String& effect) {
  if (effect == MiLightCommandNames::NIGHT_MODE) {
    this->enableNightMode();
  } else if (effect == "white" || effect == "white_mode") {
    this->updateColorWhite();
  } else { // assume we're trying to set mode
    this->updateMode(effect.toInt());
  }
}

uint8_t MiLightClient::parseStatus(JsonObject object) {
  JsonVariant status;

  if (object.containsKey(GroupStateFieldNames::STATUS)) {
    status = object[GroupStateFieldNames::STATUS];
  } else if (object.containsKey(GroupStateFieldNames::STATE)) {
    status = object[GroupStateFieldNames::STATE];
  } else {
    return 255;
  }

  if (status.is<bool>()) {
    return status.as<bool>() ? ON : OFF;
  } else {
    String strStatus(status.as<const char*>());
    return (strStatus.equalsIgnoreCase("on") || strStatus.equalsIgnoreCase("true")) ? ON : OFF;
  }
}

void MiLightClient::setRepeatsOverride(size_t repeats) {
  this->repeatsOverride = repeats;
}

void MiLightClient::clearRepeatsOverride() {
  this->repeatsOverride = PacketSender::DEFAULT_PACKET_SENDS_VALUE;
}

void MiLightClient::flushPacket() {
  PacketStream& stream = currentRemote->packetFormatter->buildPackets();

  while (stream.hasNext()) {
    packetSender.enqueue(stream.next(), currentRemote, repeatsOverride);
  }

  currentRemote->packetFormatter->reset();
}

void MiLightClient::onUpdateBegin(EventHandler handler) {
  this->updateBeginHandler = handler;
}

void MiLightClient::onUpdateEnd(EventHandler handler) {
  this->updateEndHandler = handler;
}
