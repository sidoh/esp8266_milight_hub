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
  const BulbId& bulbId,
  GroupStateField field,
  uint16_t startValue,
  uint16_t endValue,
  uint16_t stepSize,
  size_t duration
) {
  activeTransitions.add(
    std::make_shared<FieldTransition>(
      currentId++,
      bulbId,
      field,
      startValue,
      endValue,
      stepSize,
      duration,
      std::bind(&TransitionController::transitionCallback, this, _1, _2, _3)
    )
  );
}

void TransitionController::scheduleTransition(
  const BulbId& bulbId,
  const ParsedColor& startColor,
  const ParsedColor& endColor,
  uint16_t stepSize,
  size_t duration
) {
  activeTransitions.add(
    std::make_shared<ColorTransition>(
      currentId++,
      bulbId,
      startColor,
      endColor,
      stepSize,
      duration,
      std::bind(&TransitionController::transitionCallback, this, _1, _2, _3)
    )
  );
}

void TransitionController::transitionCallback(const BulbId& bulbId, GroupStateField field, uint16_t arg) {
  for (auto it = observers.begin(); it != observers.end(); ++it) {
    (*it)(bulbId, field, arg);
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

ListNode<std::shared_ptr<Transition>>* TransitionController::getTransitions() {
  return activeTransitions.getHead();
}

ListNode<std::shared_ptr<Transition>>* TransitionController::findTransition(size_t id) {
  auto current = getTransitions();

  while (current != nullptr) {
    if (current->data->id == id) {
      return current;
    }
    current = current->next;
  }

  return nullptr;
}

Transition* TransitionController::getTransition(size_t id) {
  auto node = findTransition(id);

  if (node == nullptr) {
    return nullptr;
  } else {
    return node->data.get();
  }
}

bool TransitionController::deleteTransition(size_t id) {
  auto node = findTransition(id);

  if (node == nullptr) {
    return false;
  } else {
    activeTransitions.remove(node);
    return true;
  }
}