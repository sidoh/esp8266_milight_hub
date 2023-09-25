#include <GroupAlias.h>

// reads a GroupAlias from a stream in the format:
// <alias>\0<deviceId>\0<deviceType>\0<groupId>
bool GroupAlias::load(Stream &stream) {
  // read id
  id = stream.parseInt();

  // expect null terminator
  char c = stream.read();
  if (c != 0) {
    Serial.printf_P(PSTR("ERROR: alias file invalid. expected null after id but got %c (0x%02x)\n"), c, c);
    return false;
  }

  // read alias
  size_t len = stream.readBytesUntil('\0', alias, MAX_ALIAS_LEN);
  alias[len] = 0;

  // load bulbId
  bulbId.load(stream);

  return true;
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
  // Read number of aliases
  const uint16_t numAliases = stream.parseInt();
  // expect null terminator
  stream.read();

  Serial.printf_P(PSTR("Reading %d aliases\n"), numAliases);

  while (stream.available() && aliases.size() < numAliases) {
    GroupAlias alias;
    if (alias.load(stream)) {
      aliases[String(alias.alias)] = alias;
    }
  }
}

void GroupAlias::saveAliases(Stream &stream, const std::map<String, GroupAlias> &aliases) {
  // Write number of aliases
  stream.print(aliases.size());
  stream.write(0);

  Serial.printf_P(PSTR("Saving %d aliases\n"), aliases.size());

  for (auto & alias : aliases) {
    alias.second.dump(stream);
  }
}