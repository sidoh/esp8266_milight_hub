#include <GroupStateField.h>
#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>
#include <functional>
#include <Transition.h>

#pragma once

class FieldTransition : public Transition {
public:
  FieldTransition(
    size_t id,
    const BulbId& bulbId,
    GroupStateField field,
    uint16_t startValue,
    uint16_t endValue,
    uint16_t stepSize,
    size_t duration,
    TransitionFn callback
  );

  virtual bool isFinished() override;

private:
  const GroupStateField field;
  int16_t currentValue;
  const int16_t endValue;

  virtual void step() override;
  virtual void childSerialize(JsonObject& json) override;
};