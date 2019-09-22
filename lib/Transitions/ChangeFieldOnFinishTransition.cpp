#include <ChangeFieldOnFinishTransition.h>
#include <MiLightStatus.h>

ChangeFieldOnFinishTransition::Builder::Builder(
  size_t id,
  GroupStateField field,
  uint16_t arg,
  std::shared_ptr<Transition::Builder> delegate
)
  : Transition::Builder(delegate->id, delegate->defaultPeriod, delegate->bulbId, delegate->callback, delegate->getMaxSteps())
  , delegate(delegate)
  , field(field)
  , arg(arg)
{ }

std::shared_ptr<Transition> ChangeFieldOnFinishTransition::Builder::_build() const {
  delegate->setDurationRaw(this->getOrComputeDuration());
  delegate->setPeriod(this->getOrComputePeriod());

  return std::make_shared<ChangeFieldOnFinishTransition>(
    delegate->build(),
    field,
    arg,
    delegate->getPeriod()
  );
}

ChangeFieldOnFinishTransition::ChangeFieldOnFinishTransition(
  std::shared_ptr<Transition> delegate,
  GroupStateField field,
  uint16_t arg,
  size_t period
) : Transition(delegate->id, delegate->bulbId, period, delegate->callback)
  , delegate(delegate)
  , field(field)
  , arg(arg)
  , changeSent(false)
{ }

bool ChangeFieldOnFinishTransition::isFinished() {
  return delegate->isFinished() && changeSent;
}

void ChangeFieldOnFinishTransition::step() {
  if (! delegate->isFinished()) {
    delegate->step();
  } else {
    callback(bulbId, field, arg);
    changeSent = true;
  }
}

void ChangeFieldOnFinishTransition::childSerialize(JsonObject& json) {
  json[F("type")] = F("change_on_finish");
  json[F("field")] = GroupStateFieldHelpers::getFieldName(field);
  json[F("value")] = arg;

  JsonObject child = json.createNestedObject(F("child"));
  delegate->childSerialize(child);
}