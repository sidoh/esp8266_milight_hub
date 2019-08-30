#include <GroupStatePersistence.h>
#include <FS.h>

static const char FILE_PREFIX[] = "group_states/";

void GroupStatePersistence::get(const BulbId &id, GroupState& state) {
  char path[30];
  memset(path, 0, 30);
  buildFilename(id, path);

  if (SPIFFS.exists(path)) {
    File f = SPIFFS.open(path, "r");
    state.load(f);
    f.close();
  }
}

void GroupStatePersistence::set(const BulbId &id, const GroupState& state) {
  char path[30];
  memset(path, 0, 30);
  buildFilename(id, path);

  File f = SPIFFS.open(path, "w");
  state.dump(f);
  f.close();
}

void GroupStatePersistence::clear(const BulbId &id) {
  char path[30];
  buildFilename(id, path);

  if (SPIFFS.exists(path)) {
    SPIFFS.remove(path);
  }
}

char* GroupStatePersistence::buildFilename(const BulbId &id, char *buffer) {
  uint32_t compactId = id.getCompactId();
  return buffer + sprintf(buffer, "%s%x", FILE_PREFIX, compactId);
}
