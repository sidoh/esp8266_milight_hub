#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <PatternHandler.h>

#define HTTP_DOWNLOAD_UNIT_SIZE 1460
#define HTTP_MAX_SEND_WAIT 5000 //ms to wait for data chunk to be ACKed
#define HTTP_MAX_CLOSE_WAIT 2000 //ms to wait for the client to close the connection

class WebServer : public ESP8266WebServer {
public:
  WebServer(int port) : ESP8266WebServer(port) { }

  void onAuthenticated(const String &uri, THandlerFunction handler);
  void onAuthenticated(const String &uri, HTTPMethod method, THandlerFunction fn);
  void onAuthenticated(const String &uri, HTTPMethod method, THandlerFunction fn, THandlerFunction ufn);
  void onPattern(const String& pattern, const HTTPMethod method, PatternHandler::TPatternHandlerFn fn);
  void onPatternAuthenticated(const String& pattern, const HTTPMethod method, PatternHandler::TPatternHandlerFn handler);
  bool matchesPattern(const String& pattern, const String& url);
  void requireAuthentication(const String& username, const String& password);
  void disableAuthentication();
  bool validateAuthentiation();

  inline bool clientConnected() {
    return _currentClient && _currentClient.connected();
  }

  bool authenticationRequired() {
    return authEnabled;
  }

protected:

  bool authEnabled;
  String username;
  String password;

};

#endif
