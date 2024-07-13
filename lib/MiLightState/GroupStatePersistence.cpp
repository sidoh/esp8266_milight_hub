#include <GroupStatePersistence.h>
#include <FS.h>
#ifdef ESP32
  #include <SPIFFS.h>
#endif
#include "ProjectFS.h"

#ifdef ESP8266
    static const char FILE_PREFIX[] = "group_states/";
#elif ESP32
    static const char FILE_PREFIX[] = "/group_states/";
#endif

void GroupStatePersistence::get(const BulbId &id, GroupState& state) {
  char path[30];
  memset(path, 0, 30);
  buildFilename(id, path);

  if (ProjectFS.exists(path)) {
    File f = ProjectFS.open(path, "r");
    state.load(f);
    f.close();
  }
}

void GroupStatePersistence::set(const BulbId &id, const GroupState& state) {
  char path[30];
  memset(path, 0, 30);
  buildFilename(id, path);

  File f = ProjectFS.open(path, "w");
  state.dump(f);
  f.close();
}

void GroupStatePersistence::clear(const BulbId &id) {
  char path[30];
  buildFilename(id, path);

  if (ProjectFS.exists(path)) {
    ProjectFS.remove(path);
  }
}

char* GroupStatePersistence::buildFilename(const BulbId &id, char *buffer) {
  uint32_t compactId = id.getCompactId();
  return buffer + sprintf(buffer, "%s%x", FILE_PREFIX, compactId);
}
