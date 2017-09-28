#include <GroupStateCache.h>

GroupStateCache::GroupStateCache(const size_t maxSize)
  : maxSize(maxSize)
{ }

GroupState* GroupStateCache::get(const GroupId& id) {
  GroupState* state = getInternal(id);

  if (state == NULL) {
    state = set(id, GroupState::defaultState(id.deviceType));
    Serial.println(state->getBrightness());
    return state;
  } else {
    return state;
  }
}

GroupState* GroupStateCache::set(const GroupId& id, const GroupState& state) {
  GroupCacheNode* pushedNode = NULL;
  if (cache.size() >= maxSize) {
    pushedNode = cache.pop();
  }

  GroupState* cachedState = getInternal(id);

  if (cachedState == NULL) {
    if (pushedNode == NULL) {
      GroupCacheNode* newNode = new GroupCacheNode(id, state);
      cachedState = &newNode->state;
      cache.unshift(newNode);
    } else {
      pushedNode->id = id;
      pushedNode->state = state;
      cachedState = &pushedNode->state;
      cache.unshift(pushedNode);
    }
  } else {
    *cachedState = state;
  }

  return cachedState;
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
