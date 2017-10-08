#include <GroupState.h>
#include <GroupStateCache.h>
#include <GroupStatePersistence.h>

#ifndef _GROUP_STATE_STORE_H
#define _GROUP_STATE_STORE_H

class GroupStateStore {
public:
  GroupStateStore(const size_t maxSize);

  GroupState& get(const BulbId& id);
  GroupState& set(const BulbId& id, const GroupState& state);

  void flush();

private:
  GroupStateCache cache;
  GroupStatePersistence persistence;
  LinkedList<BulbId> evictedIds;

  void trackEviction();
};

#endif
