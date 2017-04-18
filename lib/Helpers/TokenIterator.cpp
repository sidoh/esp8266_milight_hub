#include <TokenIterator.h>

TokenIterator::TokenIterator(char* data, size_t length, const char sep)
  : data(data),
    current(data),
    length(length),
    sep(sep),
    i(0)
{
  for (size_t i = 0; i < length; i++) {
    if (data[i] == sep) {
      data[i] = 0;
    }
  }
}

const char* TokenIterator::nextToken() {
  if (i >= length) {
    return NULL;
  }

  char* token = current;
  char* nextToken = current;

  for (; i < length && *nextToken != 0; i++, nextToken++);

  if (i == length) {
    nextToken = NULL;
  } else {
    i = (nextToken - data);

    if (i < length) {
      nextToken++;
    } else {
      nextToken = NULL;
    }
  }

  current = nextToken;

  return token;
}

void TokenIterator::reset() {
  current = data;
  i = 0;
}

bool TokenIterator::hasNext() {
  return i < length;
}
