#include <FieldTransition.h>
#include <cmath>
#include <algorithm>

FieldTransition::Builder::Builder(size_t id, uint16_t defaultPeriod, const BulbId& bulbId, TransitionFn callback, GroupStateField field, uint16_t start, uint16_t end)
  : Transition::Builder(
      id,
      defaultPeriod,
      bulbId,
      callback,
      max(
        static_cast<size_t>(1),
        static_cast<size_t>(std::ceil(static_cast<int16_t>(end) - start))
      )
  )
  , stepSize(0)
  , field(field)
  , start(start)
  , end(end)
{ }

std::shared_ptr<Transition> FieldTransition::Builder::_build() const {
  size_t numPeriods = getOrComputeNumPeriods();
  size_t period = getOrComputePeriod();

  int16_t distance = end - start;
  int16_t stepSize = ceil(std::abs(distance / static_cast<float>(numPeriods)));

  if (end < start) {
    stepSize = -stepSize;
  }
  if (stepSize == 0) {
    stepSize = end > start ? 1 : -1;
  }

  return std::make_shared<FieldTransition>(
    id,
    bulbId,
    field,
    start,
    end,
    stepSize,
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
  size_t period,
  TransitionFn callback
) : Transition(id, bulbId, period, callback)
  , field(field)
  , currentValue(startValue)
  , endValue(endValue)
  , stepSize(stepSize)
  , finished(false)
{ }

void FieldTransition::step() {
  callback(bulbId, field, currentValue);

  if (currentValue != endValue) {
    Transition::stepValue(currentValue, endValue, stepSize);
  } else {
    finished = true;
  }
}

bool FieldTransition::isFinished() {
  return finished;
}

void FieldTransition::childSerialize(JsonObject& json) {
  json[F("type")] = F("field");
  json[F("field")] = GroupStateFieldHelpers::getFieldName(field);
  json[F("current_value")] = currentValue;
  json[F("end_value")] = endValue;
  json[F("step_size")] = stepSize;
}