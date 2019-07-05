#include <FieldTransition.h>
#include <cmath>

FieldTransition::FieldTransition(
  size_t id,
  GroupStateField field,
  uint16_t startValue,
  uint16_t endValue,
  uint16_t stepSize,
  size_t duration,
  TransitionFn callback
) : Transition(id, stepSize, calculatePeriod((endValue - startValue), stepSize, duration), callback)
  , field(field)
  , currentValue(startValue)
  , endValue(endValue)
{
  if (endValue < startValue) {
    stepSize = -stepSize;
  } else if (endValue == startValue) {
    stepSize = 0;
  }
}

void FieldTransition::step() {
  callback(field, currentValue);
  Transition::stepValue(currentValue, endValue, stepSize);
}

bool FieldTransition::isFinished() {
  return currentValue == endValue;
}