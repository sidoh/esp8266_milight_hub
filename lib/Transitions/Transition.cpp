#include <Transition.h>
#include <Arduino.h>
#include <cmath>

Transition::Builder::Builder(size_t id, uint16_t defaultPeriod, const BulbId& bulbId, TransitionFn callback, size_t maxSteps)
  : id(id)
  , defaultPeriod(defaultPeriod)
  , bulbId(bulbId)
  , callback(callback)
  , duration(0)
  , period(0)
  , numPeriods(0)
  , maxSteps(maxSteps)
{ }

Transition::Builder& Transition::Builder::setDuration(float duration) {
  this->duration = duration * DURATION_UNIT_MULTIPLIER;
  return *this;
}

void Transition::Builder::setDurationRaw(size_t duration) {
  this->duration = duration;
}

Transition::Builder& Transition::Builder::setPeriod(size_t period) {
  this->period = period;
  return *this;
}

Transition::Builder& Transition::Builder::setDurationAwarePeriod(size_t period, size_t duration, size_t maxSteps) {
  if ((period * maxSteps) < duration) {
    setPeriod(std::ceil(duration / static_cast<float>(maxSteps)));
  } else {
    setPeriod(period);
  }
  return *this;
}

size_t Transition::Builder::getNumPeriods() const {
  return this->numPeriods;
}

size_t Transition::Builder::getDuration() const {
  return this->duration;
}

size_t Transition::Builder::getPeriod() const {
  return this->period;
}

size_t Transition::Builder::getMaxSteps() const {
  return this->maxSteps;
}

bool Transition::Builder::isSetDuration() const {
  return this->duration > 0;
}

bool Transition::Builder::isSetPeriod() const {
  return this->period > 0;
}

bool Transition::Builder::isSetNumPeriods() const {
  return this->numPeriods > 0;
}

size_t Transition::Builder::numSetParams() const {
  size_t setCount = 0;

  if (isSetDuration()) { ++setCount; }
  if (isSetPeriod()) { ++setCount; }
  if (isSetNumPeriods()) { ++setCount; }

  return setCount;
}

size_t Transition::Builder::getOrComputePeriod() const {
  if (period > 0) {
    return period;
  } else if (duration > 0 && numPeriods > 0) {
    size_t computed = floor(duration / static_cast<float>(numPeriods));
    return max(MIN_PERIOD, computed);
  } else {
    return 0;
  }
}

size_t Transition::Builder::getOrComputeDuration() const {
  if (duration > 0) {
    return duration;
  } else if (period > 0 && numPeriods > 0) {
    return period * numPeriods;
  } else {
    return 0;
  }
}

size_t Transition::Builder::getOrComputeNumPeriods() const {
  if (numPeriods > 0) {
    return numPeriods;
  } else if (period > 0 && duration > 0) {
    size_t _numPeriods = ceil(duration / static_cast<float>(period));
    return max(static_cast<size_t>(1), _numPeriods);
  } else {
    return 0;
  }
}

std::shared_ptr<Transition> Transition::Builder::build() {
  // Set defaults for underspecified transitions
  size_t numSet = numSetParams();

  if (numSet == 0) {
    setDuration(DEFAULT_DURATION);
    setDurationAwarePeriod(defaultPeriod, duration, maxSteps);
  } else if (numSet == 1) {
    // If duration is unbound, bind it
    if (! isSetDuration()) {
      setDurationRaw(DEFAULT_DURATION);
    // Otherwise, bind the period
    } else {
      setDurationAwarePeriod(defaultPeriod, duration, maxSteps);
    }
  }

  return _build();
}

Transition::Transition(
  size_t id,
  const BulbId& bulbId,
  size_t period,
  TransitionFn callback
) : id(id)
  , bulbId(bulbId)
  , callback(callback)
  , period(period)
  , lastSent(0)
{ }

void Transition::tick() {
  unsigned long now = millis();

  if ((lastSent + period) <= now
    && ((!isFinished() || lastSent == 0))) { // always send at least once

    step();
    lastSent = now;
  }
}

size_t Transition::calculatePeriod(int16_t distance, size_t stepSize, size_t duration) {
  float fPeriod =
    distance != 0
      ? (duration / (distance / static_cast<float>(stepSize)))
      : 0;

  return static_cast<size_t>(round(fPeriod));
}

void Transition::stepValue(int16_t& current, int16_t end, int16_t stepSize) {
  int16_t delta = end - current;
  if (std::abs(delta) < std::abs(stepSize)) {
    current += delta;
  } else {
    current += stepSize;
  }
}

void Transition::serialize(JsonObject& json) {
  json[F("id")] = id;
  json[F("period")] = period;
  json[F("last_sent")] = lastSent;

  JsonObject bulbParams = json.createNestedObject("bulb");
  bulbId.serialize(bulbParams);

  childSerialize(json);
}