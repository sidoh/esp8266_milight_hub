#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <PatternHandler.h>

#define HTTP_DOWNLOAD_UNIT_SIZE 1460
#define HTTP_UPLOAD_BUFLEN 2048
#define HTTP_MAX_DATA_WAIT 1000 //ms to wait for the client to send the request
#define HTTP_MAX_POST_WAIT 1000 //ms to wait for POST data to arrive
#define HTTP_MAX_SEND_WAIT 5000 //ms to wait for data chunk to be ACKed
#define HTTP_MAX_CLOSE_WAIT 2000 //ms to wait for the client to close the connection

class WebServer : public ESP8266WebServer {
public:
  WebServer(int port) : ESP8266WebServer(port) { }

  bool matchesPattern(const String& pattern, const String& url);
  void onPattern(const String& pattern, const HTTPMethod method, PatternHandler::TPatternHandlerFn fn);
  void requireAuthentication(const String& username, const String& password);
  void disableAuthentication();

  inline bool clientConnected() {
    return _currentClient && _currentClient.connected();
  }

  // These are copied / patched from ESP8266WebServer because they aren't
  // virtual. (*barf*)
  void handleClient();
  void _handleRequest();

  bool authenticationRequired() {
    return authEnabled;
  }

protected:

  bool authEnabled;
  String username;
  String password;
};

#endif
