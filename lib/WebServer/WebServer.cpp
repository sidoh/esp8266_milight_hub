#include <WebServer.h>
#include <Vector.h>

struct StringToken {
  StringToken(const int start, const int end) 
    : start(start), end(end) { }
  
  int start;
  int end;
  
  const String extract(const String& s) {
    return s.substring(start, end);
  }
};

Vector<StringToken> tokenize(const String& path) {
  Vector<StringToken> tokenPositions;
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
        tokenPositions.push_back(token);
      }
      
      lastStart = i+1;
    }
      
    currentPosition++;
  }
  
  return tokenPositions;
}

void WebServer::resetPathMatches() {
  delete buffer;
  buffer = new DynamicJsonBuffer();
  pathMatches = &buffer->createObject();
}

bool WebServer::matchesPattern(const String& pattern, const String& url) {
  Vector<StringToken> patternTokens = tokenize(pattern);
  Vector<StringToken> urlTokens = tokenize(url);
  
  for (int i = 0; i < patternTokens.size(); i++) {
    String a = patternTokens[i].extract(pattern);
    String b = urlTokens.size() <= i ? "" : urlTokens[i].extract(url);
  }
  
  if (patternTokens.size() != urlTokens.size()) {
    return false;
  }
  
  for (int i = 0; i < patternTokens.size(); i++) {
    const String& pT = patternTokens[i].extract(pattern);
    const String& uT = urlTokens[i].extract(url);
    
    if (!pT.startsWith(":") && pT != uT) {
      return false;
    }
  }
  
  resetPathMatches();
  
  for (int i = 0; i < patternTokens.size(); i++) {
    // Trim off leading ':'
    const String& pT = patternTokens[i].extract(pattern).substring(1);
    const String& uT = urlTokens[i].extract(url);
    
    (*pathMatches)[pT] = uT;
  }
  
  return true;
}

void WebServer::checkPatterns() {
  for (int i = 0; i < handlers.size(); i++) {
    const PatternHandler* handler = handlers[i];
    
    if (method() == handler->method 
      && matchesPattern(handler->pattern, uri())) {
      handler->fn();
      return;
    }
  }
  
  if (notFoundHandler) {
    notFoundHandler();
  } else {
    send(404, "text/plain", String("Not found: ") + _currentUri);
  }
}

void WebServer::onPattern(const String& pattern, const HTTPMethod method, const THandlerFunction fn) {
  handlers.push_back(new PatternHandler(pattern, method, fn));
}
