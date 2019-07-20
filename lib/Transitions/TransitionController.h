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

  std::shared_ptr<Transition::Builder> buildColorTransition(const BulbId& bulbId, const ParsedColor& start, const ParsedColor& end);
  std::shared_ptr<Transition::Builder> buildFieldTransition(const BulbId& bulbId, GroupStateField field, uint16_t start, uint16_t end);

  void addTransition(std::shared_ptr<Transition> transition);
  void clear();
  void loop();

  ListNode<std::shared_ptr<Transition>>* getTransitions();
  Transition* getTransition(size_t id);
  ListNode<std::shared_ptr<Transition>>* findTransition(size_t id);
  bool deleteTransition(size_t id);

private:
  Transition::TransitionFn callback;
  LinkedList<std::shared_ptr<Transition>> activeTransitions;
  std::vector<Transition::TransitionFn> observers;
  size_t currentId;

  void transitionCallback(const BulbId& bulbId, GroupStateField field, uint16_t arg);
};