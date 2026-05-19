#include "ApiServer.h"
#include "JsonResponse.h"
#include "config.h"
#include <ArduinoJson.h>
#include <WiFi.h>

ApiServer::ApiServer(
  DeviceInfo& deviceInfo,
  DeviceState& state,
  WiFiService& wifiService,
  LedService& ledService,
  SensorService& sensorService
) : server(API_PORT),
    deviceInfo(deviceInfo),
    state(state),
    wifiService(wifiService),
    ledService(ledService),
    sensorService(sensorService) {}

void ApiServer::begin() {
  registerRoutes();
  server.begin();
}

void ApiServer::handle() {
  server.handleClient();
}

void ApiServer::registerRoutes() {
  server.on("/api/device-info", HTTP_GET, [this]() { handleDeviceInfo(); });
  server.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
  server.on("/api/provision", HTTP_POST, [this]() { handleProvision(); });
  server.on("/api/led", HTTP_POST, [this]() { handleLedControl(); });

  server.on("/api/provision", HTTP_OPTIONS, [this]() { handleOptions(); });
  server.on("/api/led", HTTP_OPTIONS, [this]() { handleOptions(); });

  server.onNotFound([this]() { handleNotFound(); });
}

void ApiServer::handleOptions() {
  JsonResponse::sendOptions(server);
}

void ApiServer::handleDeviceInfo() {
  StaticJsonDocument<512> doc;

  doc["success"] = true;
  doc["device_id"] = deviceInfo.getDeviceId();
  doc["ap_name"] = deviceInfo.getApName();
  doc["mdns_name"] = deviceInfo.getMdnsName();
  doc["local_dns"] = deviceInfo.getLocalDns();
  doc["local_ip"] = WiFi.localIP().toString();
  doc["setup_ip"] = WiFi.softAPIP().toString();
  doc["wifi_connected"] = state.wifiConnected;

  JsonResponse::sendJson(server, 200, doc);
}

void ApiServer::handleProvision() {
  Serial.println();
  Serial.println("========================================");
  Serial.println("  [PROVISION] Request received from app");
  Serial.println("========================================");

  if (!server.hasArg("plain")) {
    JsonResponse::sendError(server, 400, "Missing JSON body");
    return;
  }

  StaticJsonDocument<512> requestDoc;
  DeserializationError error = deserializeJson(requestDoc, server.arg("plain"));

  if (error) {
    Serial.print("[PROVISION] ERROR: JSON parse failed: ");
    Serial.println(error.c_str());
    JsonResponse::sendError(server, 400, "Invalid JSON");
    return;
  }

  String devicePassword = requestDoc["device_password"] | "";
  String wifiSSID = requestDoc["wifi_ssid"] | "";
  String wifiPassword = requestDoc["wifi_password"] | "";

  Serial.println("[PROVISION] Parsed credentials from app:");
  Serial.print("  Device Password : ");
  Serial.println(devicePassword.length() > 0 ? "******" : "");
  Serial.print("  Wi-Fi SSID      : ");
  Serial.println(wifiSSID);
  Serial.println("  Wi-Fi Password  : ******");

  if (devicePassword != DEVICE_PASSWORD) {
    JsonResponse::sendError(server, 401, "Wrong device password");
    return;
  }

  if (wifiSSID.length() == 0) {
    JsonResponse::sendError(server, 400, "Wi-Fi SSID is required");
    return;
  }

  Serial.println("[PROVISION] Credentials received. Attempting to connect to home Wi-Fi...");
  Serial.println("[PROVISION] NOTE: Credentials will NOT be saved to flash.");

  bool connected = wifiService.provision(state, wifiSSID, wifiPassword);

  StaticJsonDocument<768> responseDoc;

  if (connected) {
    responseDoc["success"] = true;
    responseDoc["message"] = "Device connected (credentials not saved)";
    responseDoc["device_id"] = deviceInfo.getDeviceId();
    responseDoc["local_ip"] = WiFi.localIP().toString();
    responseDoc["local_dns"] = deviceInfo.getLocalDns();
    responseDoc["status_url"] = deviceInfo.getLocalDns() + "/api/status";
    responseDoc["led_url"] = deviceInfo.getLocalDns() + "/api/led";

    Serial.println("[PROVISION] SUCCESS - Device is online");
    Serial.print("  Device ID    : ");
    Serial.println(deviceInfo.getDeviceId());
    Serial.print("  AP Name      : ");
    Serial.println(deviceInfo.getApName());
    Serial.print("  Home SSID    : ");
    Serial.println(wifiSSID);
    Serial.print("  Local IP     : ");
    Serial.println(WiFi.localIP());
    Serial.print("  Local DNS    : ");
    Serial.println(deviceInfo.getLocalDns());
  } else {
    responseDoc["success"] = false;
    responseDoc["message"] = "Could not connect to home Wi-Fi";
    responseDoc["device_id"] = deviceInfo.getDeviceId();
    responseDoc["setup_ip"] = WiFi.softAPIP().toString();

    Serial.println("[PROVISION] FAILED - Could not connect");
  }

  JsonResponse::sendJson(server, 200, responseDoc);
}

void ApiServer::handleStatus() {
  // Sensors are updated on interval in loop. Force read here only if you want immediate fresh data.
  StaticJsonDocument<1024> doc;

  doc["success"] = true;
  doc["device_id"] = deviceInfo.getDeviceId();
  doc["wifi_connected"] = state.wifiConnected;
  doc["local_ip"] = WiFi.localIP().toString();
  doc["local_dns"] = deviceInfo.getLocalDns();

  JsonObject sensors = doc.createNestedObject("sensors");
  sensors["temperature"] = state.sensors.temperature;
  sensors["humidity"] = state.sensors.humidity;
  sensors["co2"] = state.sensors.co2;
  sensors["soil1"] = state.sensors.soil1;
  sensors["soil2"] = state.sensors.soil2;
  sensors["water_raw"] = state.sensors.waterRaw;
  sensors["water_percent"] = state.sensors.waterPercent;

  JsonObject led = doc.createNestedObject("led");
  led["power"] = state.led.power;
  led["brightness"] = state.led.brightness;

  JsonObject alerts = doc.createNestedObject("alerts");
  alerts["soil1_low"] = state.alerts.soil1Low;
  alerts["soil2_low"] = state.alerts.soil2Low;
  alerts["water_low"] = state.alerts.waterLow;
  alerts["temperature_high"] = state.alerts.temperatureHigh;
  alerts["humidity_high"] = state.alerts.humidityHigh;
  alerts["any_alert"] = state.alerts.anyAlert;

  JsonResponse::sendJson(server, 200, doc);
}

void ApiServer::handleLedControl() {
  if (!server.hasArg("plain")) {
    JsonResponse::sendError(server, 400, "Missing JSON body");
    return;
  }

  StaticJsonDocument<256> requestDoc;
  DeserializationError error = deserializeJson(requestDoc, server.arg("plain"));

  if (error) {
    JsonResponse::sendError(server, 400, "Invalid JSON");
    return;
  }

  bool power = state.led.power;
  int brightness = state.led.brightness;

  if (requestDoc.containsKey("power")) {
    power = requestDoc["power"];
  }

  if (requestDoc.containsKey("brightness")) {
    brightness = constrain(static_cast<int>(requestDoc["brightness"]), 0, 255);
  }

  ledService.set(state, power, brightness);

  StaticJsonDocument<256> responseDoc;
  responseDoc["success"] = true;
  responseDoc["message"] = "LED updated";
  responseDoc["power"] = state.led.power;
  responseDoc["brightness"] = state.led.power ? state.led.brightness : 0;

  JsonResponse::sendJson(server, 200, responseDoc);
}

void ApiServer::handleNotFound() {
  JsonResponse::sendError(server, 404, "API not found");
}
