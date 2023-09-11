#include <GroupAlias.h>

// reads a GroupAlias from a stream in the format:
// <alias>\0<deviceId>\0<deviceType>\0<groupId>
void GroupAlias::load(Stream &stream) {
  // read id
  id = stream.parseInt();

  // expect null terminator
  char c = stream.read();
  if (c != 0) {
    Serial.printf_P(PSTR("ERROR: alias file invalid. expected null after id but got %c\n"), c);
    return;
  }

  // read alias
  size_t len = stream.readBytesUntil('\0', alias, MAX_ALIAS_LEN);
  alias[len] = 0;

  // load bulbId
  bulbId.load(stream);
}

void GroupAlias::dump(Stream &stream) const {
  // write id and alias
  stream.print(id);
  stream.print(static_cast<char>(0));
  stream.print(alias);
  stream.print(static_cast<char>(0));

  // write bulbId
  bulbId.dump(stream);
}

void GroupAlias::loadAliases(Stream &stream, std::map<String, GroupAlias> &aliases) {
  // file ends in DLE (0x10)
  while (stream.available() && stream.peek() != 0x10) {
    GroupAlias alias;
    alias.load(stream);

    aliases[String(alias.alias)] = alias;
  }
}

void GroupAlias::saveAliases(Stream &stream, std::map<String, GroupAlias> &aliases) {
  for (auto & alias : aliases) {
    alias.second.dump(stream);
  }
}