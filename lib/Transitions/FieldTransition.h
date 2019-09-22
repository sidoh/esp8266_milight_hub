#include <GroupStateField.h>
#include <stdint.h>
#include <stddef.h>
#include <Arduino.h>
#include <functional>
#include <Transition.h>

#pragma once

class FieldTransition : public Transition {
public:

  class Builder : public Transition::Builder {
  public:
    Builder(size_t id, uint16_t defaultPeriod, const BulbId& bulbId, TransitionFn callback, GroupStateField field, uint16_t start, uint16_t end);

    virtual std::shared_ptr<Transition> _build() const override;

  private:
    size_t stepSize;
    GroupStateField field;
    uint16_t start;
    uint16_t end;
  };

  FieldTransition(
    size_t id,
    const BulbId& bulbId,
    GroupStateField field,
    uint16_t startValue,
    uint16_t endValue,
    int16_t stepSize,
    size_t period,
    TransitionFn callback
  );

  virtual bool isFinished() override;

private:
  const GroupStateField field;
  int16_t currentValue;
  const int16_t endValue;
  const int16_t stepSize;
  bool finished;

  virtual void step() override;
  virtual void childSerialize(JsonObject& json) override;
};