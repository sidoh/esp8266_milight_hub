#ifndef _INTPARSING_H
#define _INTPARSING_H

#include <Arduino.h>

template <typename T>
const T strToHex(const char* s, size_t length) {
  T value = 0;
  T base = 1;

  for (int i = length-1; i >= 0; i--) {
    const char c = s[i];

    if (c >= '0' && c <= '9') {
      value += ((c - '0') * base);
    } else if (c >= 'a' && c <= 'f') {
      value += ((c - 'a' + 10) * base);
    } else if (c >= 'A' && c <= 'F') {
      value += ((c - 'A' + 10) * base);
    } else {
      break;
    }

    base <<= 4;
  }

  return value;
}

template <typename T>
const T strToHex(const String& s) {
  return strToHex<T>(s.c_str(), s.length());
}

template <typename T>
const T parseInt(const String& s) {
  if (s.startsWith("0x")) {
    return strToHex<T>(s.substring(2));
  } else {
    return s.toInt();
  }
}

template <typename T>
void hexStrToBytes(const char* s, const size_t sLen, T* buffer, size_t maxLen) {
  int idx = 0;

  for (int i = 0; i < sLen && idx < maxLen; ) {
    buffer[idx++] = strToHex<T>(s+i, 2);
    i+= 2;

    while (i < (sLen - 1) && s[i] == ' ') {
      i++;
    }
  }
}

class IntParsing {
public:
  static void bytesToHexStr(const uint8_t* bytes, const size_t len, char* buffer, size_t maxLen) {
    char* p = buffer;

    for (size_t i = 0; i < len && static_cast<size_t>(p - buffer) < (maxLen - 3); i++) {
      p += sprintf(p, "%02X", bytes[i]);

      if (i < (len - 1)) {
        p += sprintf(p, " ");
      }
    }
  }
};

#endif
