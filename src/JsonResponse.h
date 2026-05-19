#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>

class JsonResponse {
public:
  static void sendCorsHeaders(WebServer& server);
  static void sendOptions(WebServer& server);
  static void sendJson(WebServer& server, int code, JsonDocument& doc);
  static void sendError(WebServer& server, int code, const char* message);
};
