#include <TokenIterator.h>

#ifndef _URL_TOKEN_BINDINGS_H
#define _URL_TOKEN_BINDINGS_H

class UrlTokenBindings {
public:
  UrlTokenBindings(TokenIterator& patternTokens, TokenIterator& requestTokens);

  bool hasBinding(const char* key) const;
  const char* get(const char* key) const;

private:
  TokenIterator& patternTokens;
  TokenIterator& requestTokens;
};

#endif
