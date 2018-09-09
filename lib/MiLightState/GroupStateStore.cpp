#include <GroupStateStore.h>
#include <MiLightRemoteConfig.h>

GroupStateStore::GroupStateStore(const size_t maxSize, const size_t flushRate)
  : cache(GroupStateCache(maxSize)),
    flushRate(flushRate),
    lastFlush(0)
{ }

GroupState* GroupStateStore::get(const BulbId& id) {
  GroupState* state = cache.get(id);

  if (state == NULL) {
    trackEviction();
    GroupState loadedState = GroupState::defaultState(id.deviceType);

    // For device types with groups, group 0 is a "virtual" group.  All devices paired with the same ID will respond
    // to group 0.  So it doesn't make sense to store group 0 state by itself.
    //
    // For devices that don't have groups, we made the unfortunate decision to represent state using the fake group
    // ID 0, so we can't always ignore group 0.
    const MiLightRemoteConfig* remoteConfig = MiLightRemoteConfig::fromType(id.deviceType);

    if (id.groupId != 0 || remoteConfig == NULL || remoteConfig->numGroups == 0) {
      persistence.get(id, loadedState);
      state = cache.set(id, loadedState);
    } else {
      return NULL;
    }
  }

  return state;
}

GroupState* GroupStateStore::get(const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType) {
  BulbId bulbId(deviceId, groupId, deviceType);
  return get(bulbId);
}

// save state for a bulb.  If id.groupId == 0, will iterate across all groups
// and individually save each group (recursively)
GroupState* GroupStateStore::set(const BulbId &id, const GroupState& state) {
  GroupState* storedState = get(id);
  *storedState = state;

  if (id.groupId == 0) {
    const MiLightRemoteConfig* remote = MiLightRemoteConfig::fromType(id.deviceType);
    BulbId individualBulb(id);

    for (size_t i = 1; i <= remote->numGroups; i++) {
      individualBulb.groupId = i;

      GroupState* individualState = get(individualBulb);
      individualState->patch(state);
    }
  }
  
  return storedState;
}

GroupState* GroupStateStore::set(const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType, const GroupState& state) {
  BulbId bulbId(deviceId, groupId, deviceType);
  return set(bulbId, state);
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
