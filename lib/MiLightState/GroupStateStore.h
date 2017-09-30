#include <GroupState.h>

#ifndef _STATE_CACHE_H
#define _STATE_CACHE_H

class GroupStateStore {
public:
  bool get(const GroupId& id, GroupState& state);
  void set(const GroupId& id, const GroupState& state);

private:
  void evictOldest(GroupState& state);
};

#endif
