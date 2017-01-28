#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <Vector.h>

struct PatternHandler {
  PatternHandler(const String& pattern, const HTTPMethod method, const ESP8266WebServer::THandlerFunction fn)
    : pattern(pattern),
      method(method),
      fn(fn)
    { }
    
  const String pattern;
  const HTTPMethod method;
  const ESP8266WebServer::THandlerFunction fn;
};

class WebServer : public ESP8266WebServer {
public:
  WebServer(int port) : 
    ESP8266WebServer(port) { 
    
    ESP8266WebServer::onNotFound([&]() { checkPatterns(); });
  }
  
  ~WebServer() {
    delete buffer;
  }
  
  bool matchesPattern(const String& pattern, const String& url);
  
  void onPattern(const String& pattern, const HTTPMethod method, const THandlerFunction fn);
  
  String arg(String key) {
    if (pathMatches && pathMatches->containsKey(key)) {
      return pathMatches->get<String>(key);
    }
    
    return ESP8266WebServer::arg(key);
  }
  
  inline bool clientConnected() { 
    return _currentClient && _currentClient.connected();
  }
  
  inline void onNotFound(THandlerFunction fn) {
    notFoundHandler = fn;
  }
  
private:
  
  void resetPathMatches();
  void checkPatterns();

  DynamicJsonBuffer* buffer;
  JsonObject* pathMatches;
  Vector<PatternHandler*> handlers;
  THandlerFunction notFoundHandler;

};

#endif