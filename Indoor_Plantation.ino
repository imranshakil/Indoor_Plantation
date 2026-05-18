/*
  FloraGuard ESP32 App-Controlled Firmware
  ----------------------------------------
  Functionality:
  1. ESP32 creates setup hotspot for mobile app
  2. App sends home Wi-Fi name/password
  3. ESP32 connects to home Wi-Fi (credentials NOT saved)
  4. ESP32 returns local IP and local DNS to app
  5. App reads sensor data using /api/status
  6. App controls grow light using /api/led
  7. Multiple devices work because each device creates unique AP name and DNS
  8. On every reboot, app must re-send Wi-Fi credentials
*/

// ===================== LIBRARIES =====================
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <MHZ19.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>

// ===================== SERVER & STORAGE =====================
WebServer server(80);
Preferences preferences;

// ===================== DEVICE SECURITY =====================
const char* DEVICE_PASSWORD = "12345678";

// ===================== SENSOR PINS =====================
#define DHTPIN 4
#define DHTTYPE DHT22

#define SOIL1 34
#define SOIL2 35
#define WATER_PIN 33
#define POT_PIN 32

#define LED_PIN 25

#define WIFI_LED 18
#define WARN_LED 21

#define RXD2 16
#define TXD2 17

// ===================== SENSOR OBJECTS =====================
DHT dht(DHTPIN, DHTTYPE);
MHZ19 myMHZ19;
HardwareSerial mySerial(2);

// ===================== DEVICE INFO =====================
String shortId;
String deviceId;
String apName;
String mdnsName;
String localDns;

// ===================== WIFI DATA =====================
// *** CHANGED: Removed savedSSID and savedPassword variables.
// *** Credentials are no longer stored — app must send them on every boot.
bool wifiConnected = false;

// ===================== SENSOR VALUES =====================
int soil1 = 0;
int soil2 = 0;
int waterValue = 0;
int waterPercent = 0;

float tempValue = 0.0;
float humValue = 0.0;
int co2Value = 0;

// ===================== LED CONTROL =====================
int ledBrightness = 0;
bool ledPower = false;

// ===================== ALERTS =====================
bool soilAlert1 = false;
bool soilAlert2 = false;
bool waterAlert = false;
bool tempAlert = false;
bool humAlert = false;
bool anyAlert = false;

// ===================== THRESHOLDS =====================
int soilThreshold = 2000;
int waterLowPercent = 10;
float tempMax = 35.0;
float humMax = 80.0;

// ===================== TIMING =====================
unsigned long lastSensorRead = 0;
unsigned long lastPotCheck = 0;

// ======================================================
// GENERATE UNIQUE DEVICE ID FROM ESP32 MAC ADDRESS
// ======================================================
void generateDeviceInfo() {
  uint64_t mac = ESP.getEfuseMac();

  char idBuffer[7];
  sprintf(idBuffer, "%06X", (uint32_t)(mac & 0xFFFFFF));

  shortId = String(idBuffer);

  deviceId = "PLANT-" + shortId;
  apName = "Plant-Setup-" + shortId;
  mdnsName = "plant-" + shortId;
  localDns = "http://" + mdnsName + ".local";
}

// ======================================================
// WATER PERCENT CALCULATION
// ======================================================
int calculateWaterPercent() {
  int percent;

  if (waterValue <= 1510) {
    percent = map(waterValue, 0, 1510, 0, 50);
  } else {
    percent = map(waterValue, 1510, 1830, 50, 100);
  }

  return constrain(percent, 0, 100);
}

// ======================================================
// READ ALL SENSORS
// ======================================================
void readSensors() {
  soil1 = analogRead(SOIL1);
  soil2 = analogRead(SOIL2);

  int rawWater = analogRead(WATER_PIN);
  waterValue = (waterValue * 7 + rawWater) / 8;
  waterPercent = calculateWaterPercent();

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) tempValue = t;
  if (!isnan(h)) humValue = h;

  int co2 = myMHZ19.getCO2();
  if (co2 > 0 && co2 < 10000) {
    co2Value = co2;
  }

  soilAlert1 = soil1 < soilThreshold;
  soilAlert2 = soil2 < soilThreshold;
  waterAlert = waterPercent <= waterLowPercent;
  tempAlert = tempValue > tempMax;
  humAlert = humValue > humMax;

  anyAlert = soilAlert1 || soilAlert2 || waterAlert || tempAlert || humAlert;

  digitalWrite(WARN_LED, anyAlert ? HIGH : LOW);
}

// ======================================================
// OPTIONAL POTENTIOMETER CONTROL
// ======================================================
void updatePotControl() {
  int raw = analogRead(POT_PIN);
  int mappedValue = map(raw, 0, 4095, 0, 255);

  if (abs(mappedValue - ledBrightness) > 8) {
    ledBrightness = mappedValue;
    ledPower = ledBrightness > 0;
    ledcWrite(LED_PIN, ledBrightness);
  }
}

// ======================================================
// START ESP32 SETUP HOTSPOT
// ======================================================
void startSetupHotspot() {
  WiFi.mode(WIFI_AP_STA);

  WiFi.softAP(apName.c_str(), DEVICE_PASSWORD);

  Serial.println();
  Serial.println("===== SETUP MODE STARTED =====");
  Serial.print("AP Name: ");
  Serial.println(apName);
  Serial.print("AP Password: ");
  Serial.println(DEVICE_PASSWORD);
  Serial.print("Setup IP: ");
  Serial.println(WiFi.softAPIP());
}

// ======================================================
// CONNECT TO HOME WIFI
// ======================================================
bool connectToHomeWiFi(String ssid, String pass) {
  Serial.println();
  Serial.println("Trying to connect to home Wi-Fi...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 20000) {
    delay(500);
    digitalWrite(WIFI_LED, !digitalRead(WIFI_LED));
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    digitalWrite(WIFI_LED, HIGH);

    Serial.println("Home Wi-Fi connected!");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin(mdnsName.c_str())) {
      Serial.print("mDNS started: ");
      Serial.println(localDns);
    } else {
      Serial.println("mDNS failed to start");
    }

    return true;
  }

  wifiConnected = false;
  digitalWrite(WIFI_LED, LOW);

  Serial.println("Home Wi-Fi connection failed");
  return false;
}

// *** CHANGED: Removed loadSavedWiFi() function entirely.
// *** CHANGED: Removed saveWiFi() function entirely.
// *** CHANGED: Removed clearSavedWiFi() function entirely.
// *** None of these are needed since credentials are never saved to flash.

// ======================================================
// COMMON JSON RESPONSE HELPERS
// ======================================================
void sendCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

void handleOptions() {
  sendCorsHeaders();
  server.send(204);
}

// ======================================================
// API: DEVICE INFO
// GET /api/device-info
// ======================================================
void handleDeviceInfo() {
  sendCorsHeaders();

  StaticJsonDocument<512> doc;

  doc["success"] = true;
  doc["device_id"] = deviceId;
  doc["ap_name"] = apName;
  doc["mdns_name"] = mdnsName;
  doc["local_dns"] = localDns;
  doc["local_ip"] = WiFi.localIP().toString();
  doc["setup_ip"] = WiFi.softAPIP().toString();
  doc["wifi_connected"] = wifiConnected;

  String response;
  serializeJson(doc, response);

  server.send(200, "application/json", response);
}

// ======================================================
// API: PROVISION DEVICE
// POST /api/provision
// ======================================================
void handleProvision() {
  sendCorsHeaders();

  Serial.println();
  Serial.println("========================================");
  Serial.println("  [PROVISION] Request received from app");
  Serial.println("========================================");

  if (!server.hasArg("plain")) {
    Serial.println("[PROVISION] ERROR: No JSON body in request.");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing JSON body\"}");
    return;
  }

  Serial.println("[PROVISION] Raw body received from app:");
  Serial.println(server.arg("plain"));
  Serial.println();

  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    Serial.print("[PROVISION] ERROR: JSON parse failed: ");
    Serial.println(error.c_str());
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
    return;
  }

  String devicePassword = doc["device_password"] | "";
  String wifiSSID = doc["wifi_ssid"] | "";
  String wifiPassword = doc["wifi_password"] | "";

  Serial.println("[PROVISION] Parsed credentials from app:");
  Serial.print("  Device Password : ");
  Serial.println(devicePassword);
  Serial.print("  Wi-Fi SSID      : ");
  Serial.println(wifiSSID);
  Serial.print("  Wi-Fi Password  : ");
  Serial.println(wifiPassword);
  Serial.println();

  if (devicePassword != DEVICE_PASSWORD) {
    Serial.println("[PROVISION] ERROR: Wrong device password. Rejecting request.");
    server.send(401, "application/json", "{\"success\":false,\"message\":\"Wrong device password\"}");
    return;
  }

  Serial.println("[PROVISION] Device password OK.");

  if (wifiSSID.length() == 0) {
    Serial.println("[PROVISION] ERROR: Wi-Fi SSID is empty. Rejecting request.");
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Wi-Fi SSID is required\"}");
    return;
  }

  Serial.println("[PROVISION] Credentials received. Attempting to connect to home Wi-Fi...");
  Serial.println("[PROVISION] NOTE: Credentials will NOT be saved to flash."); // *** ADDED ***
  Serial.println("----------------------------------------");

  bool connected = connectToHomeWiFi(wifiSSID, wifiPassword);

  StaticJsonDocument<768> responseDoc;

  if (connected) {
    // *** CHANGED: Removed saveWiFi(wifiSSID, wifiPassword) call here.
    // *** Credentials are used for this session only and lost on reboot.

    responseDoc["success"] = true;
    responseDoc["message"] = "Device connected (credentials not saved)"; // *** CHANGED: Updated message ***
    responseDoc["device_id"] = deviceId;
    responseDoc["local_ip"] = WiFi.localIP().toString();
    responseDoc["local_dns"] = localDns;
    responseDoc["status_url"] = localDns + "/api/status";
    responseDoc["led_url"] = localDns + "/api/led";

    Serial.println();
    Serial.println("========================================");
    Serial.println("  [PROVISION] SUCCESS - Device is online");
    Serial.println("========================================");
    Serial.print("  Device ID    : ");
    Serial.println(deviceId);
    Serial.print("  AP Name      : ");
    Serial.println(apName);
    Serial.print("  Home SSID    : ");
    Serial.println(wifiSSID);
    Serial.print("  Home Password: ");
    Serial.println(wifiPassword);
    Serial.print("  Local IP     : ");
    Serial.println(WiFi.localIP());
    Serial.print("  Gateway IP   : ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("  Subnet Mask  : ");
    Serial.println(WiFi.subnetMask());
    Serial.print("  MAC Address  : ");
    Serial.println(WiFi.macAddress());
    Serial.print("  Signal (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("  mDNS Name    : ");
    Serial.println(mdnsName + ".local");
    Serial.print("  Local DNS    : ");
    Serial.println(localDns);
    Serial.print("  Status URL   : ");
    Serial.println(localDns + "/api/status");
    Serial.print("  LED URL      : ");
    Serial.println(localDns + "/api/led");
    Serial.println("  *** Credentials NOT saved. App must re-send on next boot. ***"); // *** ADDED ***
    Serial.println("========================================");
    Serial.println();

  } else {

    responseDoc["success"] = false;
    responseDoc["message"] = "Could not connect to home Wi-Fi";
    responseDoc["device_id"] = deviceId;
    responseDoc["setup_ip"] = WiFi.softAPIP().toString();

    Serial.println();
    Serial.println("========================================");
    Serial.println("  [PROVISION] FAILED - Could not connect");
    Serial.println("========================================");
    Serial.print("  Device ID   : ");
    Serial.println(deviceId);
    Serial.print("  Tried SSID  : ");
    Serial.println(wifiSSID);
    Serial.print("  Setup AP IP : ");
    Serial.println(WiFi.softAPIP());
    Serial.println("  Check: Is the SSID/password correct?");
    Serial.println("  Check: Is the router in range?");
    Serial.println("========================================");
    Serial.println();
  }

  String response;
  serializeJson(responseDoc, response);

  Serial.println("[PROVISION] Sending JSON response to app:");
  Serial.println(response);
  Serial.println();

  server.send(200, "application/json", response);
}

// ======================================================
// API: STATUS / SENSOR DATA
// GET /api/status
// ======================================================
void handleStatus() {
  sendCorsHeaders();

  readSensors();

  StaticJsonDocument<1024> doc;

  doc["success"] = true;
  doc["device_id"] = deviceId;
  doc["wifi_connected"] = wifiConnected;
  doc["local_ip"] = WiFi.localIP().toString();
  doc["local_dns"] = localDns;

  JsonObject sensors = doc.createNestedObject("sensors");
  sensors["temperature"] = tempValue;
  sensors["humidity"] = humValue;
  sensors["co2"] = co2Value;
  sensors["soil1"] = soil1;
  sensors["soil2"] = soil2;
  sensors["water_raw"] = waterValue;
  sensors["water_percent"] = waterPercent;

  JsonObject led = doc.createNestedObject("led");
  led["power"] = ledPower;
  led["brightness"] = ledBrightness;

  JsonObject alerts = doc.createNestedObject("alerts");
  alerts["soil1_low"] = soilAlert1;
  alerts["soil2_low"] = soilAlert2;
  alerts["water_low"] = waterAlert;
  alerts["temperature_high"] = tempAlert;
  alerts["humidity_high"] = humAlert;
  alerts["any_alert"] = anyAlert;

  String response;
  serializeJson(doc, response);

  server.send(200, "application/json", response);
}

// ======================================================
// API: LED CONTROL
// POST /api/led
// ======================================================
void handleLedControl() {
  sendCorsHeaders();

  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Missing JSON body\"}");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
    return;
  }

  if (doc.containsKey("power")) {
    ledPower = doc["power"];
  }

  if (doc.containsKey("brightness")) {
    ledBrightness = constrain((int)doc["brightness"], 0, 255);
  }

  if (!ledPower) {
    ledcWrite(LED_PIN, 0);
  } else {
    ledcWrite(LED_PIN, ledBrightness);
  }

  StaticJsonDocument<256> responseDoc;
  responseDoc["success"] = true;
  responseDoc["message"] = "LED updated";
  responseDoc["power"] = ledPower;
  responseDoc["brightness"] = ledPower ? ledBrightness : 0;

  String response;
  serializeJson(responseDoc, response);

  server.send(200, "application/json", response);
}

// *** CHANGED: Removed handleResetWiFi() function entirely.
// *** There are no saved credentials to reset, so this endpoint is not needed.

// ======================================================
// REGISTER API ROUTES
// ======================================================
void setupRoutes() {
  server.on("/api/device-info", HTTP_GET, handleDeviceInfo);
  server.on("/api/status", HTTP_GET, handleStatus);

  server.on("/api/provision", HTTP_POST, handleProvision);
  server.on("/api/led", HTTP_POST, handleLedControl);
  // *** CHANGED: Removed /api/reset-wifi POST route — nothing to reset ***
  // *** CHANGED: Removed /api/reset-wifi OPTIONS route ***

  server.on("/api/provision", HTTP_OPTIONS, handleOptions);
  server.on("/api/led", HTTP_OPTIONS, handleOptions);

  server.onNotFound([]() {
    sendCorsHeaders();
    server.send(404, "application/json", "{\"success\":false,\"message\":\"API not found\"}");
  });
}

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting FloraGuard ESP32...");

  pinMode(WIFI_LED, OUTPUT);
  pinMode(WARN_LED, OUTPUT);
  pinMode(POT_PIN, INPUT);

  digitalWrite(WIFI_LED, LOW);
  digitalWrite(WARN_LED, LOW);

  generateDeviceInfo();

  dht.begin();

  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  myMHZ19.begin(mySerial);
  myMHZ19.autoCalibration(false);

  ledcAttach(LED_PIN, 1000, 8);
  ledcWrite(LED_PIN, 0);

  startSetupHotspot();

  // *** CHANGED: Removed loadSavedWiFi() call — nothing is ever saved.
  // *** CHANGED: Removed the savedSSID check and auto-connect block.
  // *** Device always boots into hotspot mode and waits for app credentials.
  Serial.println("Waiting for app to send Wi-Fi credentials...");

  setupRoutes();
  server.begin();

  readSensors();

  Serial.println();
  Serial.println("===== DEVICE READY =====");
  Serial.print("Device ID: ");
  Serial.println(deviceId);
  Serial.print("Setup AP: ");
  Serial.println(apName);
  Serial.print("Setup IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Local DNS: ");
  Serial.println(localDns);
  Serial.println("Connect your phone to the AP above and provision via app."); // *** ADDED ***
}

// ======================================================
// LOOP
// ======================================================
void loop() {
  server.handleClient();

  unsigned long now = millis();

  if (now - lastSensorRead > 5000) {
    lastSensorRead = now;
    readSensors();
  }

  if (now - lastPotCheck > 300) {
    lastPotCheck = now;
    updatePotControl();
  }
}
