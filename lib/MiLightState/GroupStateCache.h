#include <GroupState.h>
#include <LinkedList.h>

#ifndef _GROUP_STATE_CACHE_H
#define _GROUP_STATE_CACHE_H

struct GroupCacheNode {
  GroupCacheNode() {}
  GroupCacheNode(const GroupId& id, const GroupState& state)
    : id(id), state(state) { }

  GroupId id;
  GroupState state;
};

class GroupStateCache {
public:
  GroupStateCache(const size_t maxSize);

  GroupState* get(const GroupId& id);
  GroupState* set(const GroupId& id, const GroupState& state);
  GroupId getLru();
  bool isFull() const;
  ListNode<GroupCacheNode*>* getHead();

private:
  LinkedList<GroupCacheNode*> cache;
  const size_t maxSize;

  GroupState* getInternal(const GroupId& id);
};

#endif
