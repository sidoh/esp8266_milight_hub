#ifndef _INTPARSING_H
#define _INTPARSING_H

#include <Arduino.h>

template <typename T>
const T strToHex(const String& s) {
  T value = 0;
  T base = 1;
  
  for (int i = s.length() - 1; i >= 0; i--) {
    const char c = s.charAt(i);
    
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
const T parseInt(const String& s) {
  if (s.startsWith("0x")) {
    return strToHex<T>(s.substring(2));
  } else {
    return s.toInt();
  }
}

#endif