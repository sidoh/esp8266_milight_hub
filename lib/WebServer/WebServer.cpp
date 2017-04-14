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

void WebServer::_handleRequest() {
  if (this->authEnabled
    && !this->authenticate(this->username.c_str(), this->password.c_str())) {
    this->requestAuthentication();
  } else {
    ESP8266WebServer::_handleRequest();
  }
}

void WebServer::handleClient() {
  if (_currentStatus == HC_NONE) {
    WiFiClient client = _server.available();
    if (!client) {
      return;
    }

    _currentClient = client;
    _currentStatus = HC_WAIT_READ;
    _statusChange = millis();
  }

  if (!_currentClient.connected()) {
    _currentClient = WiFiClient();
    _currentStatus = HC_NONE;
    return;
  }

  // Wait for data from client to become available
  if (_currentStatus == HC_WAIT_READ) {
    if (!_currentClient.available()) {
      if (millis() - _statusChange > HTTP_MAX_DATA_WAIT) {
        _currentClient = WiFiClient();
        _currentStatus = HC_NONE;
      }
      yield();
      return;
    }

    if (!_parseRequest(_currentClient)) {
      _currentClient = WiFiClient();
      _currentStatus = HC_NONE;
      return;
    }
    _currentClient.setTimeout(HTTP_MAX_SEND_WAIT);
    _contentLength = CONTENT_LENGTH_NOT_SET;
    _handleRequest();

    if (!_currentClient.connected()) {
      _currentClient = WiFiClient();
      _currentStatus = HC_NONE;
      return;
    } else {
      _currentStatus = HC_WAIT_CLOSE;
      _statusChange = millis();
      return;
    }
  }

  if (_currentStatus == HC_WAIT_CLOSE) {
    if (millis() - _statusChange > HTTP_MAX_CLOSE_WAIT) {
      _currentClient = WiFiClient();
      _currentStatus = HC_NONE;
    } else {
      yield();
      return;
    }
  }
}
