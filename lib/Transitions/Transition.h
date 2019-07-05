#include <BulbId.h>
#include <ArduinoJson.h>
#include <GroupStateField.h>
#include <stdint.h>
#include <stddef.h>
#include <functional>

#pragma once

class Transition {
public:
  static const size_t DEFAULT_STEP_SIZE = 1;

  using TransitionFn = std::function<void(const BulbId& bulbId, GroupStateField field, uint16_t value)>;
  const size_t id;
  const BulbId bulbId;

  Transition(
    size_t id,
    const BulbId& bulbId,
    uint16_t stepSize,
    size_t period,
    TransitionFn callback
  );

  void tick();
  virtual bool isFinished() = 0;
  void serialize(JsonObject& doc);

  static size_t calculatePeriod(int16_t distance, size_t stepSize, size_t duration);

protected:
  const int16_t stepSize;
  const size_t period;
  const TransitionFn callback;
  unsigned long lastSent;

  virtual void step() = 0;
  virtual void childSerialize(JsonObject& doc) = 0;
  static void stepValue(int16_t& current, int16_t end, int16_t stepSize);
};