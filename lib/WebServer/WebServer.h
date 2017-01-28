#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <PatternHandler.h>

class WebServer : public ESP8266WebServer {
public:
  WebServer(int port) : ESP8266WebServer(port) { }
  
  bool matchesPattern(const String& pattern, const String& url);
  void onPattern(const String& pattern, const HTTPMethod method, PatternHandler::TPatternHandlerFn fn);
  
  inline bool clientConnected() { 
    return _currentClient && _currentClient.connected();
  }
  
private:
  
  void resetPathMatches();
  void checkPatterns();
};

#endif