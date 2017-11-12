#include <GroupStateStore.h>
#include <MiLightRemoteConfig.h>

GroupStateStore::GroupStateStore(const size_t maxSize, const size_t flushRate)
  : cache(GroupStateCache(maxSize)),
    flushRate(flushRate),
    lastFlush(0)
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

  if (id.groupId == 0) {
    const MiLightRemoteConfig* remote = MiLightRemoteConfig::fromType(id.deviceType);
    BulbId individualBulb(id);

    for (size_t i = 1; i <= remote->numGroups; i++) {
      individualBulb.groupId = i;
      set(individualBulb, state);
    }
  }

  return storedState;
}

void GroupStateStore::trackEviction() {
  if (cache.isFull()) {
    evictedIds.add(cache.getLru());
  }
}

bool GroupStateStore::flush() {
  ListNode<GroupCacheNode*>* curr = cache.getHead();
  bool anythingFlushed = false;

  while (curr != NULL && curr->data->state.isDirty() && !anythingFlushed) {
    persistence.set(curr->data->id, curr->data->state);
    curr->data->state.clearDirty();

#ifdef STATE_DEBUG
    BulbId bulbId = curr->data->id;
    printf(
      "Flushing dirty state for 0x%04X / %d / %s\n",
      bulbId.deviceId,
      bulbId.groupId,
      MiLightRemoteConfig::fromType(bulbId.deviceType)->name.c_str()
    );
#endif

    curr = curr->next;
    anythingFlushed = true;
  }

  while (evictedIds.size() > 0 && !anythingFlushed) {
    persistence.clear(evictedIds.shift());
    anythingFlushed = true;
  }

  return anythingFlushed;
}

void GroupStateStore::limitedFlush() {
  unsigned long now = millis();

  if ((lastFlush + flushRate) < now) {
    if (flush()) {
      lastFlush = now;
    }
  }
}
