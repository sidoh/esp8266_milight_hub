#include <PatternHandler.h>

UrlTokenBindings::UrlTokenBindings(TokenIterator& patternTokens, TokenIterator& requestTokens)
  : patternTokens(patternTokens),
    requestTokens(requestTokens)
{ }

bool UrlTokenBindings::hasBinding(const char* searchToken) const {
  patternTokens.reset();
  while (patternTokens.hasNext()) {
    const char* token = patternTokens.nextToken();

    if (strcmp(token, searchToken) == 0) {
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

PatternHandler::PatternHandler(
    const String& pattern,
    const HTTPMethod method,
    const PatternHandler::TPatternHandlerFn fn)
  : method(method),
    fn(fn),
    _pattern(new char[pattern.length() + 1]),
    patternTokens(NULL)
{
  strcpy(_pattern, pattern.c_str());
  patternTokens = new TokenIterator(_pattern, pattern.length(), '/');
}

PatternHandler::~PatternHandler() {
  delete _pattern;
  delete patternTokens;
}

bool PatternHandler::canHandle(HTTPMethod requestMethod, String requestUri) {
  if (this->method != HTTP_ANY && requestMethod != this->method) {
    return false;
  }

  bool canHandle = true;

  char requestUriCopy[requestUri.length() + 1];
  strcpy(requestUriCopy, requestUri.c_str());
  TokenIterator requestTokens(requestUriCopy, requestUri.length(), '/');

  patternTokens->reset();
  while (patternTokens->hasNext() && requestTokens.hasNext()) {
    const char* patternToken = patternTokens->nextToken();
    const char* requestToken = requestTokens.nextToken();

    if (patternToken[0] != ':' && strcmp(patternToken, requestToken) != 0) {
      canHandle = false;
      break;
    }

    if (patternTokens->hasNext() != requestTokens.hasNext()) {
      canHandle = false;
      break;
    }
  }

  return canHandle;
}

bool PatternHandler::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
  if (! canHandle(requestMethod, requestUri)) {
    return false;
  }

  char requestUriCopy[requestUri.length()];
  strcpy(requestUriCopy, requestUri.c_str());
  TokenIterator requestTokens(requestUriCopy, requestUri.length(), '/');

  UrlTokenBindings bindings(*patternTokens, requestTokens);
  fn(&bindings);
}
