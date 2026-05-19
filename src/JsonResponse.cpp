#include "JsonResponse.h"

void JsonResponse::sendCorsHeaders(WebServer& server) {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void JsonResponse::sendOptions(WebServer& server) {
  sendCorsHeaders(server);
  server.send(204);
}

void JsonResponse::sendJson(WebServer& server, int code, JsonDocument& doc) {
  sendCorsHeaders(server);
  String response;
  serializeJson(doc, response);
  server.send(code, "application/json", response);
}

void JsonResponse::sendError(WebServer& server, int code, const char* message) {
  StaticJsonDocument<256> doc;
  doc["success"] = false;
  doc["message"] = message;
  sendJson(server, code, doc);
}
