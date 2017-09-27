#include <GroupStateCache.h>

GroupStateCache::GroupStateCache(const size_t maxSize)
  : maxSize(maxSize)
{ }

const GroupState* GroupStateCache::get(const GroupId& id) {
  GroupState* state = getInternal(id);

  if (state == NULL) {
    return &GroupState::defaultState();
  } else {
    return state;
  }
}

void GroupStateCache::set(const GroupId& id, const GroupState& state) {
  GroupCacheNode* pushedNode = NULL;
  if (cache.size() >= maxSize) {
    pushedNode = cache.pop();
  }

  GroupState* cachedState = getInternal(id);
  if (cachedState == NULL) {
    if (pushedNode == NULL) {
      cache.unshift(new GroupCacheNode(id, state));
    } else {
      pushedNode->id = id;
      pushedNode->state = state;
      cache.unshift(pushedNode);
    }
  } else {
    *cachedState = state;
  }
}

GroupState* GroupStateCache::getInternal(const GroupId& id) {
  ListNode<GroupCacheNode*>* cur = cache.getHead();

  while (cur != NULL) {
    if (cur->data->id == id) {
      GroupState* result = &cur->data->state;
      cache.spliceToFront(cur);
      return result;
    }
    cur = cur->next;
  }

  return NULL;
}
