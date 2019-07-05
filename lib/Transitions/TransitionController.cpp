#include <Transition.h>
#include <FieldTransition.h>
#include <ColorTransition.h>
#include <GroupStateField.h>

#include <TransitionController.h>
#include <LinkedList.h>
#include <functional>

using namespace std::placeholders;

TransitionController::TransitionController()
  : currentId(0)
{ }

void TransitionController::clearListeners() {
  observers.clear();
}

void TransitionController::addListener(Transition::TransitionFn fn) {
  observers.push_back(fn);
}

void TransitionController::scheduleTransition(
  GroupStateField field,
  uint16_t startValue,
  uint16_t endValue,
  uint16_t stepSize,
  size_t duration
) {
  activeTransitions.add(
    std::make_shared<FieldTransition>(
      currentId++,
      field,
      startValue,
      endValue,
      stepSize,
      duration,
      std::bind(&TransitionController::transitionCallback, this, _1, _2)
    )
  );
}

void TransitionController::scheduleTransition(
  const ParsedColor& startColor,
  const ParsedColor& endColor,
  uint16_t stepSize,
  size_t duration
) {
  activeTransitions.add(
    std::make_shared<ColorTransition>(
      currentId++,
      startColor,
      endColor,
      stepSize,
      duration,
      std::bind(&TransitionController::transitionCallback, this, _1, _2)
    )
  );
}

void TransitionController::transitionCallback(GroupStateField field, uint16_t arg) {
  for (auto it = observers.begin(); it != observers.end(); ++it) {
    (*it)(field, arg);
  }
}

void TransitionController::clear() {
  activeTransitions.clear();
}

void TransitionController::loop() {
  auto current = activeTransitions.getHead();

  while (current != nullptr) {
    auto next = current->next;

    Transition& t = *current->data;
    t.tick();

    if (t.isFinished()) {
      activeTransitions.remove(current);
    }

    current = next;
  }
}