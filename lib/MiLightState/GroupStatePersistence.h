#include <GroupState.h>

#ifndef _GROUP_STATE_PERSISTENCE_H
#define _GROUP_STATE_PERSISTENCE_H

struct PersistedStateNode {
  GroupState state;
  GroupId next;
  GroupId prev;
};

class GroupStatePersistence {
public:
  void get(const GroupId& id, GroupState& state);
  void set(const GroupId& id, const GroupState& state);

  void clear(const GroupId& id);

private:

  static char* buildFilename(const GroupId& id, char* buffer);
};

#endif
