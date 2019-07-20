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
#if STATE_DEBUG
    printf(
      "Couldn't fetch state for 0x%04X / %d / %s in the cache, getting it from persistence\n",
      id.deviceId,
      id.groupId,
      MiLightRemoteConfig::fromType(id.deviceType)->name.c_str()
    );
#endif
    trackEviction();
    GroupState loadedState = GroupState::defaultState(id.deviceType);

    const MiLightRemoteConfig* remoteConfig = MiLightRemoteConfig::fromType(id.deviceType);

    if (remoteConfig == NULL) {
      return NULL;
    }

    persistence.get(id, loadedState);
    state = cache.set(id, loadedState);
  }

  return state;
}

GroupState* GroupStateStore::get(const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType) {
  BulbId bulbId(deviceId, groupId, deviceType);
  return get(bulbId);
}

// Save state for a bulb.
//
// Notes:
//
// * For device types with groups, group 0 is a "virtual" group.  All devices paired with the same ID will
//   respond to group 0.  When state for an individual (i.e., != 0) group is changed, the state for
//   group 0 becomes out of sync and should be cleared.
//
// * If id.groupId == 0, will iterate across all groups and individually save each group (recursively)
//
GroupState* GroupStateStore::set(const BulbId &id, const GroupState& state) {
  BulbId otherId(id);
  GroupState* storedState = get(id);
  storedState->patch(state);

  if (id.groupId == 0) {
    const MiLightRemoteConfig* remote = MiLightRemoteConfig::fromType(id.deviceType);

#ifdef STATE_DEBUG
    Serial.printf_P(PSTR("Fanning out group 0 state for device ID 0x%04X (%d groups in total)\n"), id.deviceId, remote->numGroups);
    state.debugState("group 0 state = ");
#endif

    for (size_t i = 1; i <= remote->numGroups; i++) {
      otherId.groupId = i;

      GroupState* individualState = get(otherId);
      individualState->patch(state);
    }
  } else {
    otherId.groupId = 0;
    GroupState* group0State = get(otherId);

    group0State->clearNonMatchingFields(state);
  }

  return storedState;
}

GroupState* GroupStateStore::set(const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType, const GroupState& state) {
  BulbId bulbId(deviceId, groupId, deviceType);
  return set(bulbId, state);
}

void GroupStateStore::clear(const BulbId& bulbId) {
  GroupState* state = get(bulbId);

  if (state != NULL) {
    state->initFields();
    state->patch(GroupState::defaultState(bulbId.deviceType));
  }
}

void GroupStateStore::trackEviction() {
  if (cache.isFull()) {
    evictedIds.add(cache.getLru());

#ifdef STATE_DEBUG
    BulbId bulbId = evictedIds.getLast();
    printf(
      "Evicting from cache: 0x%04X / %d / %s\n",
      bulbId.deviceId,
      bulbId.groupId,
      MiLightRemoteConfig::fromType(bulbId.deviceType)->name.c_str()
    );
#endif
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
