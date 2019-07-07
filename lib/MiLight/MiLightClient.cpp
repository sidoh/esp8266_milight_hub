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

  if (request.containsKey(GroupStateFieldNames::COMMAND)) {
    this->handleCommand(request[GroupStateFieldNames::COMMAND]);
  }

  if (request.containsKey(GroupStateFieldNames::COMMANDS)) {
    JsonArray commands = request[GroupStateFieldNames::COMMANDS];

    if (! commands.isNull()) {
      for (size_t i = 0; i < commands.size(); i++) {
        this->handleCommand(commands[i]);
      }
    }
  }

  //Homeassistant - Handle effect
  if (request.containsKey(GroupStateFieldNames::EFFECT)) {
    this->handleEffect(request[GroupStateFieldNames::EFFECT]);
  }

  if (request.containsKey(GroupStateFieldNames::HUE)) {
    this->updateHue(request[GroupStateFieldNames::HUE]);
  }
  if (request.containsKey(GroupStateFieldNames::SATURATION)) {
    this->updateSaturation(request[GroupStateFieldNames::SATURATION]);
  }

  // Convert RGB to HSV
  if (request.containsKey(GroupStateFieldNames::COLOR)) {
    this->updateColor(request[GroupStateFieldNames::COLOR]);
  }

  if (request.containsKey(GroupStateFieldNames::LEVEL)) {
    this->updateBrightness(request[GroupStateFieldNames::LEVEL]);
  }
  // HomeAssistant
  if (request.containsKey(GroupStateFieldNames::BRIGHTNESS)) {
    uint8_t scaledBrightness = Units::rescale(request[GroupStateFieldNames::BRIGHTNESS].as<uint8_t>(), 100, 255);
    this->updateBrightness(scaledBrightness);
  }

  if (request.containsKey("temperature")) {
    this->updateTemperature(request["temperature"]);
  }
  if (request.containsKey(GroupStateFieldNames::KELVIN)) {
    this->updateTemperature(request[GroupStateFieldNames::KELVIN]);
  }
  // HomeAssistant
  if (request.containsKey(GroupStateFieldNames::COLOR_TEMP)) {
    this->updateTemperature(
      Units::miredsToWhiteVal(request[GroupStateFieldNames::COLOR_TEMP], 100)
    );
  }

  if (request.containsKey(GroupStateFieldNames::MODE)) {
    this->updateMode(request[GroupStateFieldNames::MODE]);
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

void MiLightClient::handleTransition(JsonObject args) {
  if (! args.containsKey(F("field"))
    || ! args.containsKey(F("start_value"))
    || ! args.containsKey(F("end_value"))
    || ! args.containsKey(F("duration"))) {
    Serial.println(F("Ignoring transition missing required arguments"));
    return;
  }

  const char* fieldName = args[F("field")];
  const size_t duration = args[F("duration")];
  const size_t stepSize =
    args.containsKey(F("step_size"))
      ? args[F("step_size")]
      : Transition::DEFAULT_STEP_SIZE;

  GroupStateField field = GroupStateFieldHelpers::getFieldByName(fieldName);

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
      transitions.scheduleTransition(
        currentRemote->packetFormatter->currentBulbId(),
        field,
        args[F("start_value")],
        args[F("end_value")],
        stepSize,
        duration
      );
      return;

    default:
      break;
  }

  // Color can be decomposed into hue/saturation and these can be transitioned separately
  if (field == GroupStateField::COLOR) {
    ParsedColor startColor = ParsedColor::fromJson(args[F("start_value")]);
    ParsedColor endColor = ParsedColor::fromJson(args[F("end_value")]);

    if (! startColor.success) {
      Serial.println(F("Transition - error parsing start color"));
      return;
    }
    if (! endColor.success) {
      Serial.println(F("Transition - error parsing end color"));
      return;
    }

    transitions.scheduleTransition(
      currentRemote->packetFormatter->currentBulbId(),
      startColor,
      endColor,
      stepSize,
      duration
    );

    return;
  }

  Serial.printf_P(PSTR("Unsupported transition field: %s\n"), fieldName);
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
