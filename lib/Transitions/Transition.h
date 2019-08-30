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

    void setDurationRaw(size_t duration);

    bool isSetDuration() const;
    bool isSetPeriod() const;
    bool isSetNumPeriods() const;

    size_t getOrComputePeriod() const;
    size_t getOrComputeDuration() const;
    size_t getOrComputeNumPeriods() const;

    size_t getDuration() const;
    size_t getPeriod() const;
    size_t getNumPeriods() const;

    std::shared_ptr<Transition> build();

    const size_t id;
    const BulbId& bulbId;
    const TransitionFn callback;

  private:
    size_t duration;
    size_t period;
    size_t numPeriods;

    virtual std::shared_ptr<Transition> _build() const = 0;
    size_t numSetParams() const;
  };

  // Default time to wait between steps.  Do this rather than having a fixed step size because it's
  // more capable of adapting to different situations.
  static const size_t DEFAULT_PERIOD = 225;
  static const size_t DEFAULT_NUM_PERIODS = 20;
  static const size_t DEFAULT_DURATION = DEFAULT_PERIOD*DEFAULT_NUM_PERIODS;

  // If period goes lower than this, throttle other parameters up to adjust.
  static const size_t MIN_PERIOD = 150;

  const size_t id;
  const BulbId bulbId;
  const TransitionFn callback;

  Transition(
    size_t id,
    const BulbId& bulbId,
    size_t period,
    TransitionFn callback
  );

  void tick();
  virtual bool isFinished() = 0;
  void serialize(JsonObject& doc);
  virtual void step() = 0;
  virtual void childSerialize(JsonObject& doc) = 0;

  static size_t calculatePeriod(int16_t distance, size_t stepSize, size_t duration);

protected:
  const size_t period;
  unsigned long lastSent;

  static void stepValue(int16_t& current, int16_t end, int16_t stepSize);
};