#include <GroupAlias.h>

// reads a GroupAlias from a stream in the format:
// <alias>,<deviceId>,<deviceType>,<groupId>
void GroupAlias::load(Stream &stream) {
  // read id
  id = stream.parseInt();

  // expect comma
  if (stream.read() != ',') {
    Serial.println(F("ERROR: alias file invalid. expected comma after id"));
    return;
  }

  // read alias
  size_t length = stream.readBytesUntil(',', alias, MAX_ALIAS_LEN);
  alias[length] = 0;

  // load bulbId
  bulbId.load(stream);
}

void GroupAlias::dump(Stream &stream) const {
  // write id
  stream.print(id);
  stream.write(',');

  // write alias
  stream.write(alias, strlen(alias));
  stream.write(',');

  // write bulbId
  bulbId.dump(stream);
}

void GroupAlias::loadAliases(Stream &stream, std::map<String, GroupAlias> &aliases) {
  // file ends in DLE (0x10)
  while (stream.available() && stream.peek() != 0x10) {
    GroupAlias alias;
    alias.load(stream);

    // expect newline after each alias
    uint8_t c = stream.read();

    if (c != '\n') {
      Serial.println("ERROR: alias file invalid. expected newline but got: " + String(c));
      return;
    }

    aliases[String(alias.alias)] = alias;
  }
}

void GroupAlias::saveAliases(Stream &stream, std::map<String, GroupAlias> &aliases) {
  for (auto & alias : aliases) {
    alias.second.dump(stream);
    stream.write('\n');
  }
}