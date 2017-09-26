#include <GroupState.h>

#ifndef _GROUP_STATE_STORE_H
#define _GROUP_STATE_STORE_H

class GroupStateStore {
public:
  virtual const GroupState* get(const GroupId& id) = 0;
  virtual void set(const GroupId& id, const GroupState& state) = 0;
};

#endif
