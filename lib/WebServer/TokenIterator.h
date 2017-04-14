#include <Arduino.h>

#ifndef _TOKEN_ITERATOR_H
#define _TOKEN_ITERATOR_H value
class TokenIterator {
public:
  TokenIterator(char* data, size_t length, char sep = ',');

  bool hasNext();
  const char* nextToken();
  void reset();

private:
  char* data;
  char* current;
  size_t length;
  char sep;
  int i;
};
#endif
