#include <Transition.h>
#include <LinkedList.h>
#include <ParsedColor.h>
#include <GroupStateField.h>
#include <memory>
#include <vector>

#pragma once

class TransitionController {
public:
  TransitionController();

  void clearListeners();
  void addListener(Transition::TransitionFn fn);
  void scheduleTransition(
    const BulbId& bulbId,
    GroupStateField field,
    uint16_t startValue,
    uint16_t endValue,
    uint16_t stepSize,
    size_t duration
  );
  void scheduleTransition(
    const BulbId& bulbId,
    const ParsedColor& startColor,
    const ParsedColor& endColor,
    uint16_t stepSize,
    size_t duration
  );
  void clear();
  void loop();

  ListNode<std::shared_ptr<Transition>>* getTransitions();
  Transition* getTransition(size_t id);
  ListNode<std::shared_ptr<Transition>>* findTransition(size_t id);
  bool deleteTransition(size_t id);

private:
  LinkedList<std::shared_ptr<Transition>> activeTransitions;
  std::vector<Transition::TransitionFn> observers;
  size_t currentId;

  void transitionCallback(const BulbId& bulbId, GroupStateField field, uint16_t arg);
};