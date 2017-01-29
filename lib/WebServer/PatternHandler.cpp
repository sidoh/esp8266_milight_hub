#include <PatternHandler.h>
  
PatternHandler::PatternHandler(
    const String& pattern, 
    const HTTPMethod method, 
    const PatternHandler::TPatternHandlerFn fn
  ) : method(method), fn(fn), tokenPositions(NULL) {
  Vector<StringToken>* tokenPositions = new Vector<StringToken>();
  tokenize(pattern, tokenPositions);
  
  numPatternTokens = tokenPositions->size();
  patternTokens = new String[numPatternTokens];
  
  for (int i = 0; i < tokenPositions->size(); i++) {
    patternTokens[i] = (*tokenPositions)[i].extract(pattern);
  }
  
  delete tokenPositions;
}
  
bool PatternHandler::canHandle(HTTPMethod requestMethod, String requestUri) {
  if (requestMethod != HTTP_ANY && requestMethod != this->method) {
    return false;
  }
  
  if (tokenPositions) {
    delete tokenPositions;
  }
  
  bool canHandle = true;
  
  tokenPositions = new Vector<StringToken>();
  tokenize(requestUri, tokenPositions);
  
  if (numPatternTokens == tokenPositions->size()) {
    for (int i = 0; i < numPatternTokens; i++) {
      const StringToken urlTokenP = (*tokenPositions)[i];
      
      if (!patternTokens[i].startsWith(":") 
        && patternTokens[i] != urlTokenP.extract(requestUri)) {
        canHandle = false;
        break;
      }
    }
  } else {
    canHandle = false;
  }
  
  return canHandle;
}
  
bool PatternHandler::handle(ESP8266WebServer& server, HTTPMethod requestMethod, String requestUri) {
  if (! canHandle(requestMethod, requestUri)) {
    return false;
  }
  
  UrlTokenBindings* bindings = new UrlTokenBindings(patternTokens, tokenPositions, requestUri);
  fn(bindings);
  
  delete bindings;
}

void PatternHandler::tokenize(const String& path, Vector<StringToken>* tokenPositions) {
  int lastStart = 0;
  int currentPosition = 0;
  
  for (int i = 0; i < path.length(); i++) {
    if (path.charAt(i) == '/' || i == path.length()-1) {
      // If we're in the last position, include the last character if it isn't
      // a '/'
      if (path.charAt(i) != '/') {
        currentPosition++;
      }
      
      if (lastStart > 0 && currentPosition > lastStart) {
        StringToken token(lastStart, currentPosition);
        tokenPositions->push_back(token);
      }
      
      lastStart = i+1;
    }
      
    currentPosition++;
  }
}