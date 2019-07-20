#include <GroupState.h>
#include <GroupStateCache.h>
#include <GroupStatePersistence.h>

#ifndef _GROUP_STATE_STORE_H
#define _GROUP_STATE_STORE_H

class GroupStateStore {
public:
  GroupStateStore(const size_t maxSize, const size_t flushRate);

  /*
   * Returns the state for the given BulbId.  If accessing state for a valid device
   * (i.e., NOT group 0) and no state exists, its state will be initialized with a
   * default.
   *
   * Otherwise, we return NULL.
   */
  GroupState* get(const BulbId& id);
  GroupState* get(const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType);

  /*
   * Sets the state for the given BulbId.  State will be marked as dirty and
   * flushed to persistent storage.
   */
  GroupState* set(const BulbId& id, const GroupState& state);
  GroupState* set(const uint16_t deviceId, const uint8_t groupId, const MiLightRemoteType deviceType, const GroupState& state);

  void clear(const BulbId& id);

  /*
   * Flushes all states to persistent storage.  Returns true iff anything was
   * flushed.
   */
  bool flush();

  /*
   * Flushes at most one dirty state to persistent storage.  Rate limit
   * specified by Settings.
   */
  void limitedFlush();

private:
  GroupStateCache cache;
  GroupStatePersistence persistence;
  LinkedList<BulbId> evictedIds;
  const size_t flushRate;
  unsigned long lastFlush;

  void trackEviction();
};

#endif
