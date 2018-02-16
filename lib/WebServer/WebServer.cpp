#include <WebServer.h>
#include <PatternHandler.h>

void WebServer::onPattern(const String& pattern, const HTTPMethod method, PatternHandler::TPatternHandlerFn fn) {
  addHandler(new PatternHandler(pattern, method, fn));
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
    } else
      return true;
}

