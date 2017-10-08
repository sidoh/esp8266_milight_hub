#include <GroupStateStore.h>

GroupStateStore::GroupStateStore(const size_t maxSize)
  : cache(GroupStateCache(maxSize))
{ }

GroupState& GroupStateStore::get(const BulbId& id) {
  GroupState* state = cache.get(id);

  if (state == NULL) {
    trackEviction();
    GroupState loadedState = GroupState::defaultState(id.deviceType);
    persistence.get(id, loadedState);

    state = cache.set(id, loadedState);
  }

  return *state;
}

GroupState& GroupStateStore::set(const BulbId &id, const GroupState& state) {
  GroupState& storedState = get(id);
  storedState = state;
  return storedState;
}

void GroupStateStore::trackEviction() {
  if (cache.isFull()) {
    evictedIds.add(cache.getLru());
  }
}

void GroupStateStore::flush() {
  ListNode<GroupCacheNode*>* curr = cache.getHead();

  while (curr != NULL && curr->data->state.isDirty()) {
    persistence.set(curr->data->id, curr->data->state);
    curr->data->state.clearDirty();
    curr = curr->next;
  }

  while (evictedIds.size() > 0) {
    persistence.clear(evictedIds.shift());
  }
}
