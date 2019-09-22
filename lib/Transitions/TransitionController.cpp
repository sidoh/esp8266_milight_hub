#include <Transition.h>
#include <FieldTransition.h>
#include <ColorTransition.h>
#include <ChangeFieldOnFinishTransition.h>
#include <GroupStateField.h>
#include <MiLightStatus.h>

#include <TransitionController.h>
#include <LinkedList.h>
#include <functional>

using namespace std::placeholders;

TransitionController::TransitionController()
  : callback(std::bind(&TransitionController::transitionCallback, this, _1, _2, _3))
  , currentId(0)
  , defaultPeriod(500)
{ }

void TransitionController::setDefaultPeriod(uint16_t defaultPeriod) {
  this->defaultPeriod = defaultPeriod;
}

void TransitionController::clearListeners() {
  observers.clear();
}

void TransitionController::addListener(Transition::TransitionFn fn) {
  observers.push_back(fn);
}

std::shared_ptr<Transition::Builder> TransitionController::buildColorTransition(const BulbId& bulbId, const ParsedColor& start, const ParsedColor& end) {
  return std::make_shared<ColorTransition::Builder>(
    currentId++,
    defaultPeriod,
    bulbId,
    callback,
    start,
    end
  );
}

std::shared_ptr<Transition::Builder> TransitionController::buildFieldTransition(const BulbId& bulbId, GroupStateField field, uint16_t start, uint16_t end) {
  return std::make_shared<FieldTransition::Builder>(
    currentId++,
    defaultPeriod,
    bulbId,
    callback,
    field,
    start,
    end
  );
}

std::shared_ptr<Transition::Builder> TransitionController::buildStatusTransition(const BulbId& bulbId, MiLightStatus status, uint8_t startLevel) {
  std::shared_ptr<Transition::Builder> transition;

  if (status == ON) {
    // Make sure bulb is on before transitioning brightness
    callback(bulbId, GroupStateField::STATUS, ON);

    transition = buildFieldTransition(bulbId, GroupStateField::LEVEL, startLevel, 100);
  } else {
    transition = std::make_shared<ChangeFieldOnFinishTransition::Builder>(
      currentId++,
      GroupStateField::STATUS,
      OFF,
      buildFieldTransition(bulbId, GroupStateField::LEVEL, startLevel, 0)
    );
  }

  return transition;
}

void TransitionController::addTransition(std::shared_ptr<Transition> transition) {
  activeTransitions.add(transition);
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