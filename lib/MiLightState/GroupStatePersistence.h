#include <GroupState.h>

#ifndef _GROUP_STATE_PERSISTENCE_H
#define _GROUP_STATE_PERSISTENCE_H

class GroupStatePersistence {
public:
  void get(const BulbId& id, GroupState& state);
  void set(const BulbId& id, const GroupState& state);

  void clear(const BulbId& id);

private:

  static char* buildFilename(const BulbId& id, char* buffer);
};

#endif
