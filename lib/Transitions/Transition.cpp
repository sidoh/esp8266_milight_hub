#include <Transition.h>
#include <Arduino.h>
#include <cmath>

Transition::Builder::Builder(size_t id, const BulbId& bulbId, TransitionFn callback)
  : id(id)
  , bulbId(bulbId)
  , callback(callback)
  , duration(0)
  , period(0)
  , numPeriods(0)
{ }

Transition::Builder& Transition::Builder::setDuration(float duration) {
  this->duration = duration * DURATION_UNIT_MULTIPLIER;
  return *this;
}

Transition::Builder& Transition::Builder::setPeriod(size_t period) {
  this->period = period;
  return *this;
}

Transition::Builder& Transition::Builder::setNumPeriods(size_t numPeriods) {
  this->numPeriods = numPeriods;
  return *this;
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
    return floor(duration / static_cast<float>(numPeriods));
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
    return ceil(duration / static_cast<float>(period));
  } else {
    return 0;
  }
}

std::shared_ptr<Transition> Transition::Builder::build() {
  // Set defaults for underspecified transitions
  size_t numSet = numSetParams();

  if (numSet == 0) {
    setPeriod(DEFAULT_PERIOD);
    setNumPeriods(DEFAULT_NUM_PERIODS);
  } else if (numSet == 1) {
    if (isSetDuration() || isSetNumPeriods()) {
      setPeriod(DEFAULT_PERIOD);
    } else if (isSetPeriod()) {
      setNumPeriods(DEFAULT_NUM_PERIODS);
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
  , period(period)
  , callback(callback)
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
  json["id"] = id;
  json["period"] = period;
  json["last_sent"] = lastSent;

  JsonObject bulbParams = json.createNestedObject("bulb");
  bulbId.serialize(bulbParams);

  childSerialize(json);
}