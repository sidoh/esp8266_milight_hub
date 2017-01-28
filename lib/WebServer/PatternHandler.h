#ifndef _PATTERNHANDLER_H
#define _PATTERNHANDLER_H

#include <Arduino.h>
#include <ESP8266WebServer.h>
#include <Vector.h>
#include <functional>

struct StringToken {
  StringToken(const int start, const int end) 
    : start(start), end(end) { }
  
  int start;
  int end;
  
  const String extract(const String& s) const {
    return s.substring(start, end);
  }
};

class UrlTokenBindings {
public:
  
  UrlTokenBindings(const String* patternTokens, 
    const Vector<StringToken>* urlTokenPositions,
    const String& url
  ) 
    : patternTokens(patternTokens),
      numTokens(urlTokenPositions->size()) 
  {
    urlTokens = new String[numTokens];
    for (int i = 0; i < numTokens; i++) {
      urlTokens[i] = (*urlTokenPositions)[i].extract(url);
    }
  }
  
  ~UrlTokenBindings() {
    delete[] urlTokens;
  }
  
  bool hasBinding(const String& key) const {
    for (int i = 0; i < numTokens; i++) {
      if (patternTokens[i] == key) {
        return true;
      }
    }
    
    return false;
  }
  
  String get(const String& key) const {
    for (int i = 0; i < numTokens; i++) {
      if (patternTokens[i].substring(1) == key) {
        return urlTokens[i];
      }
    }
  }
  
private:
  const String* patternTokens;
  String* urlTokens;
  const size_t numTokens;
};

class PatternHandler : public RequestHandler {
public:
  typedef std::function<void(UrlTokenBindings*)> TPatternHandlerFn;
  
  PatternHandler(const String& pattern, 
    const HTTPMethod method, 
    const TPatternHandlerFn fn);
  
  ~PatternHandler() {
    delete patternTokens;
  }
  
  static void tokenize(const String& path, Vector<StringToken>* tokenPositions);
  
  bool canHandle(HTTPMethod requestMethod, String requestUri) override;
  bool handle(ESP8266WebServer& server, HTTPMethod requesetMethod, String requestUri) override;
  
private:
  size_t numPatternTokens;
  String* patternTokens;
  Vector<StringToken>* tokenPositions;
  const HTTPMethod method;
  const PatternHandler::TPatternHandlerFn fn;
};

#endif
