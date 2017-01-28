#include <WebServer.h>
#include <PatternHandler.h>

void WebServer::onPattern(const String& pattern, const HTTPMethod method, PatternHandler::TPatternHandlerFn fn) {
  addHandler(new PatternHandler(pattern, method, fn));
}