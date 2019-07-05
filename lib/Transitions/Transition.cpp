#include <Transition.h>
#include <Arduino.h>
#include <cmath>

Transition::Transition(
  size_t id,
  uint16_t stepSize,
  size_t period,
  TransitionFn callback
) : id(id)
  , stepSize(stepSize)
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