#include <UrlTokenBindings.h>

UrlTokenBindings::UrlTokenBindings(TokenIterator& patternTokens, TokenIterator& requestTokens)
  : patternTokens(patternTokens),
    requestTokens(requestTokens)
{ }

bool UrlTokenBindings::hasBinding(const char* searchToken) const {
  patternTokens.reset();
  while (patternTokens.hasNext()) {
    const char* token = patternTokens.nextToken();

    if (token[0] == ':' && strcmp(token+1, searchToken) == 0) {
      return true;
    }
  }

  return false;
}

const char* UrlTokenBindings::get(const char* searchToken) const {
  patternTokens.reset();
  requestTokens.reset();

  while (patternTokens.hasNext() && requestTokens.hasNext()) {
    const char* token = patternTokens.nextToken();
    const char* binding = requestTokens.nextToken();

    if (token[0] == ':' && strcmp(token+1, searchToken) == 0) {
      return binding;
    }
  }

  return NULL;
}
