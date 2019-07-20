#include <GroupState.h>
#include <LinkedList.h>

#ifndef _GROUP_STATE_CACHE_H
#define _GROUP_STATE_CACHE_H

struct GroupCacheNode {
  GroupCacheNode() {}
  GroupCacheNode(const BulbId& id, const GroupState& state)
    : id(id), state(state) { }

  BulbId id;
  GroupState state;
};

class GroupStateCache {
public:
  GroupStateCache(const size_t maxSize);
  ~GroupStateCache();

  GroupState* get(const BulbId& id);
  GroupState* set(const BulbId& id, const GroupState& state);
  BulbId getLru();
  bool isFull() const;
  ListNode<GroupCacheNode*>* getHead();

private:
  LinkedList<GroupCacheNode*> cache;
  const size_t maxSize;

  GroupState* getInternal(const BulbId& id);
};

#endif
