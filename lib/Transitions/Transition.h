#include <BulbId.h>
#include <ArduinoJson.h>
#include <GroupStateField.h>
#include <stdint.h>
#include <stddef.h>
#include <functional>
#include <memory>

#pragma once

class Transition {
public:
  using TransitionFn = std::function<void(const BulbId& bulbId, GroupStateField field, uint16_t value)>;

  // transition commands are in seconds, convert to ms.
  static const uint16_t DURATION_UNIT_MULTIPLIER = 1000;


  class Builder {
  public:
    Builder(size_t id, const BulbId& bulbId, TransitionFn callback);

    Builder& setDuration(float duration);
    Builder& setPeriod(size_t period);
    Builder& setNumPeriods(size_t numPeriods);

    bool isSetDuration() const;
    bool isSetPeriod() const;
    bool isSetNumPeriods() const;

    size_t getOrComputePeriod() const;
    size_t getOrComputeDuration() const;
    size_t getOrComputeNumPeriods() const;

    std::shared_ptr<Transition> build();

  protected:
    size_t id;
    const BulbId& bulbId;
    TransitionFn callback;

  private:
    size_t duration;
    size_t period;
    size_t numPeriods;

    virtual std::shared_ptr<Transition> _build() const = 0;
    size_t numSetParams() const;
  };

  // Default time to wait between steps.  Do this rather than having a fixed step size because it's
  // more capable of adapting to different situations.
  static const size_t DEFAULT_PERIOD = 300;
  static const size_t DEFAULT_NUM_PERIODS = 20; // works out to a duration of 6s
  static const size_t DEFAULT_DURATION = 6000;

  const size_t id;
  const BulbId bulbId;

  Transition(
    size_t id,
    const BulbId& bulbId,
    size_t period,
    TransitionFn callback
  );

  void tick();
  virtual bool isFinished() = 0;
  void serialize(JsonObject& doc);

  static size_t calculatePeriod(int16_t distance, size_t stepSize, size_t duration);

protected:
  const size_t period;
  const TransitionFn callback;
  unsigned long lastSent;

  virtual void step() = 0;
  virtual void childSerialize(JsonObject& doc) = 0;
  static void stepValue(int16_t& current, int16_t end, int16_t stepSize);
};