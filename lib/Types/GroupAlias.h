#include <Stream.h>
#include <BulbId.h>

#include <map>

#ifndef ESP8266_MILIGHT_HUB_GROUPALIAS_H
#define ESP8266_MILIGHT_HUB_GROUPALIAS_H

#define MAX_ALIAS_LEN 32

struct GroupAlias {
    size_t id;
    char alias[MAX_ALIAS_LEN + 1];
    BulbId bulbId;

    GroupAlias(size_t id, const char* alias, const BulbId& bulbId)
      : id(id)
      , bulbId(bulbId)
    {
      strncpy(this->alias, alias, MAX_ALIAS_LEN);
      this->alias[MAX_ALIAS_LEN] = 0;
    }
    GroupAlias() = default;

    ~GroupAlias() = default;

    bool load(Stream& stream);
    void dump(Stream& stream) const;

    static void loadAliases(Stream& stream, std::map<String, GroupAlias>& aliases);
    static void saveAliases(Stream& stream, const std::map<String, GroupAlias>& aliases);
};

#endif //ESP8266_MILIGHT_HUB_GROUPALIAS_H