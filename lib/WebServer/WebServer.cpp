#include <WebServer.h>
#include <PatternHandler.h>

void WebServer::onAuthenticated(const String &uri, THandlerFunction handler) {
  THandlerFunction authHandler = [this, handler]() {
    if (this->validateAuthentiation()) {
      handler();
    }
  };

  ESP8266WebServer::on(uri, authHandler);
}

void WebServer::onAuthenticated(const String &uri, HTTPMethod method, THandlerFunction handler) {
  THandlerFunction authHandler = [this, handler]() {
    if (this->validateAuthentiation()) {
      handler();
    }
  };

  ESP8266WebServer::on(uri, method, authHandler);
}

void WebServer::onAuthenticated(const String &uri, HTTPMethod method, THandlerFunction handler, THandlerFunction ufn) {
  THandlerFunction authHandler = [this, handler]() {
    if (this->validateAuthentiation()) {
      handler();
    }
  };

  ESP8266WebServer::on(uri, method, authHandler, ufn);
}

void WebServer::onPattern(const String& pattern, const HTTPMethod method, PatternHandler::TPatternHandlerFn handler) {
  addHandler(new PatternHandler(pattern, method, handler));
}

void WebServer::onPatternAuthenticated(const String& pattern, const HTTPMethod method, PatternHandler::TPatternHandlerFn fn) {
  PatternHandler::TPatternHandlerFn authHandler = [this, fn](UrlTokenBindings* bindings) {
    if (this->validateAuthentiation()) {
      fn(bindings);
    }
  };

  addHandler(new PatternHandler(pattern, method, authHandler));
}



void WebServer::requireAuthentication(const String& username, const String& password) {
  this->username = String(username);
  this->password = String(password);
  this->authEnabled = true;
}

void WebServer::disableAuthentication() {
  this->authEnabled = false;
}

bool WebServer::validateAuthentiation() {
  if (this->authEnabled && 
    !authenticate(this->username.c_str(), this->password.c_str())) {
      requestAuthentication();
      return false;
    }
    return true;
}

