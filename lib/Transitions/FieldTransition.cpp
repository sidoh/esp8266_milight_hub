#include <FieldTransition.h>
#include <cmath>

FieldTransition::FieldTransition(
  size_t id,
  const BulbId& bulbId,
  GroupStateField field,
  uint16_t startValue,
  uint16_t endValue,
  uint16_t stepSize,
  size_t duration,
  TransitionFn callback
) : Transition(id, bulbId, stepSize, calculatePeriod((endValue - startValue), stepSize, duration), callback)
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
  callback(bulbId, field, currentValue);
  Transition::stepValue(currentValue, endValue, stepSize);
}

bool FieldTransition::isFinished() {
  return currentValue == endValue;
}

void FieldTransition::childSerialize(JsonObject& json) {
  json["type"] = "field";
  json["field"] = GroupStateFieldHelpers::getFieldName(field);
  json["current_value"] = currentValue;
  json["end_value"] = endValue;
  json["step_size"] = stepSize;
}