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
    Builder(size_t id, uint16_t defaultPeriod, const BulbId& bulbId, TransitionFn callback, size_t maxSteps);

    Builder& setDuration(float duration);
    Builder& setPeriod(size_t period);

    /**
     * Users are typically defining transitions using:
     *   1. The desired end state (and implicitly the start state, assumed to be current)
     *   2. The duraiton
     * The user only cares about the period to the degree that it affects the smoothness of
     * the transition.
     *
     * For example, if the user wants to throttle brightness from 0 -> 100 over 5min, the
     * default period is going to be way too short to enable that.  So we need to force the
     * period to be longer to fit the duration.
     */
    Builder& setDurationAwarePeriod(size_t desiredPeriod, size_t duration, size_t maxSteps);

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
    size_t getMaxSteps() const;

    std::shared_ptr<Transition> build();

    const size_t id;
    const uint16_t defaultPeriod;
    const BulbId& bulbId;
    const TransitionFn callback;

  private:
    size_t duration;
    size_t period;
    size_t numPeriods;
    size_t maxSteps;

    virtual std::shared_ptr<Transition> _build() const = 0;
    size_t numSetParams() const;
  };

  // If period goes lower than this, throttle other parameters up to adjust.
  static const size_t MIN_PERIOD = 150;
  static const size_t DEFAULT_DURATION = 10000;

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