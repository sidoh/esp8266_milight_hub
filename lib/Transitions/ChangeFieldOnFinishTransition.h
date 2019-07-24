#include <Transition.h>

#pragma once

class ChangeFieldOnFinishTransition : public Transition {
public:

  class Builder : public Transition::Builder {
  public:
    Builder(size_t id, GroupStateField field, uint16_t arg, std::shared_ptr<Transition::Builder> delgate);

    virtual std::shared_ptr<Transition> _build() const override;

  private:
    const std::shared_ptr<Transition::Builder> delegate;
    const GroupStateField field;
    const uint16_t arg;
  };

  ChangeFieldOnFinishTransition(
    std::shared_ptr<Transition> delegate,
    GroupStateField field,
    uint16_t arg,
    size_t period
  );

  virtual bool isFinished() override;

private:
  std::shared_ptr<Transition> delegate;
  const GroupStateField field;
  const uint16_t arg;
  bool changeSent;

  virtual void step() override;
  virtual void childSerialize(JsonObject& json) override;
};