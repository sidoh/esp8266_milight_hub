#include <LinkedList.h>
#include <GroupStateStore.h>

#ifndef _GROUP_STATE_CACHE_H
#define _GROUP_STATE_CACHE_H

struct GroupCacheNode {
  GroupCacheNode() {}
  GroupCacheNode(const GroupId& id, const GroupState& state)
    : id(id), state(state) { }

  GroupId id;
  GroupState state;
};

class GroupStateCache : public GroupStateStore {
public:
  GroupStateCache(const size_t maxSize);

  const GroupState* get(const GroupId& id);
  void set(const GroupId& id, const GroupState& state);

private:
  LinkedList<GroupCacheNode*> cache;
  const size_t maxSize;

  GroupState* getInternal(const GroupId& id);
};

#endif
