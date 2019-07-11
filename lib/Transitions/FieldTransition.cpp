#include <FieldTransition.h>
#include <cmath>

FieldTransition::Builder::Builder(size_t id, const BulbId& bulbId, TransitionFn callback, GroupStateField field, uint16_t start, uint16_t end)
  : Transition::Builder(id, bulbId, callback)
  , stepSize(0)
  , field(field)
  , start(start)
  , end(end)
{ }

std::shared_ptr<Transition> FieldTransition::Builder::_build() const {
  size_t duration = getOrComputeDuration();
  size_t numPeriods = getOrComputeNumPeriods();
  size_t period = getOrComputePeriod();
  int16_t distance = end - start;
  int16_t stepSize = distance / numPeriods;

  return std::make_shared<FieldTransition>(
    id,
    bulbId,
    field,
    start,
    end,
    stepSize,
    duration,
    period,
    callback
  );
}

FieldTransition::FieldTransition(
  size_t id,
  const BulbId& bulbId,
  GroupStateField field,
  uint16_t startValue,
  uint16_t endValue,
  int16_t stepSize,
  size_t duration,
  size_t period,
  TransitionFn callback
) : Transition(id, bulbId, period, callback)
  , field(field)
  , currentValue(startValue)
  , endValue(endValue)
  , stepSize(stepSize)
{ }

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