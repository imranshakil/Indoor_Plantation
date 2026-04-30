#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <MHZ19.h>
#include <ESPmDNS.h>
#include <esp_wifi.h>

// ---------- WiFi ----------
const char* ssid = "CIS Tech Ltd.";
const char* password = "CIS@@2025&&";

WebServer server(80);

// ---------- DHT ----------
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ---------- Sensors ----------
#define SOIL1 34
#define SOIL2 35
#define WATER_PIN 33
#define POT_PIN 32

// ---------- LED ----------
#define LED_PIN 25

// ---------- Status LEDs ----------
#define WIFI_LED 18
#define WARN_LED 21

// ---------- CO2 ----------
#define RXD2 16
#define TXD2 17
MHZ19 myMHZ19;
HardwareSerial mySerial(2);

// ---------- Variables ----------
int ledBrightness = 0;
int lastPotValue = 0;

int soil1 = 0;
int soil2 = 0;
int waterValue = 0;

float tempValue = 0.0;
float humValue = 0.0;
int co2Value = 0;

bool soilAlert1 = false;
bool soilAlert2 = false;
bool waterAlert = false;
bool tempAlert = false;
bool humAlert = false;
bool anyAlert = false;

// ---------- Thresholds ----------
int soilThreshold = 2000;
int waterLowPercent = 10;    // Alert when water ≤ 10%
float tempMax = 35.0;
float humMax = 80.0;

// ---------- Timing ----------
unsigned long lastSensorRead = 0;
unsigned long lastPotCheck = 0;

// Helper function to calculate water percentage consistently
int getWaterPercent() {
  if (waterValue <= 1510) {
    return map(waterValue, 0, 1510, 0, 50);
  } else {
    return map(waterValue, 1510, 1830, 50, 100);
  }
}

// ---------- Read Sensors ----------
void readSensors() {
  soil1 = analogRead(SOIL1);
  soil2 = analogRead(SOIL2);

  // Water Level with light smoothing
  int raw = analogRead(WATER_PIN);
  waterValue = (waterValue * 7 + raw) / 8;   // Simple moving average filter

  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) tempValue = t;
  if (!isnan(h)) humValue = h;

  int co2 = myMHZ19.getCO2();
  if (co2 > 0 && co2 < 10000) co2Value = co2;

  // Calculate percentage using helper function
  int waterPercent = getWaterPercent();

  // Alerts
  soilAlert1 = soil1 < soilThreshold;
  soilAlert2 = soil2 < soilThreshold;
  waterAlert = (waterPercent <= 10);        // Alert when 10% or below
  tempAlert = tempValue > tempMax;
  humAlert = humValue > humMax;

  anyAlert = soilAlert1 || soilAlert2 || waterAlert || tempAlert || humAlert;

  digitalWrite(WARN_LED, anyAlert ? HIGH : LOW);
}

// ---------- Pot Control ----------
void updatePot() {
  int raw = analogRead(POT_PIN);
  int mapped = map(raw, 0, 4095, 0, 255);

  if (abs(mapped - lastPotValue) > 5) {
    lastPotValue = mapped;
    ledBrightness = mapped;
    ledcWrite(LED_PIN, ledBrightness);
  }
}

// ---------- Badge ----------
String badge(bool state, String ok, String bad) {
  if (state) return "<span class='badge bad'>" + bad + "</span>";
  return "<span class='badge good'>" + ok + "</span>";
}

// ---------- Routes ----------
void handleRoot() {
  server.send(200, "text/html", getHTML());
}

void handleLED() {
  if (server.hasArg("value")) {
    ledBrightness = constrain(server.arg("value").toInt(), 0, 255);
    ledcWrite(LED_PIN, ledBrightness);
  }
  server.send(200, "text/plain", "OK");
}

// ---------- Setup ----------
void setup() {
  Serial.begin(115200);

  pinMode(WIFI_LED, OUTPUT);
  pinMode(WARN_LED, OUTPUT);
  pinMode(POT_PIN, INPUT);

  digitalWrite(WIFI_LED, LOW);
  digitalWrite(WARN_LED, LOW);

  dht.begin();

  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);
  myMHZ19.begin(mySerial);
  myMHZ19.autoCalibration(false);

  ledcAttach(LED_PIN, 1000, 8);
  ledcWrite(LED_PIN, 0);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    digitalWrite(WIFI_LED, !digitalRead(WIFI_LED));
  }

  digitalWrite(WIFI_LED, HIGH);

  setCpuFrequencyMhz(160);
  WiFi.setSleep(true);
  esp_wifi_set_ps(WIFI_PS_MIN_MODEM);

  MDNS.begin("plant");

  server.on("/", handleRoot);
  server.on("/setLED", handleLED);

  server.begin();

  readSensors();
  Serial.println("FloraGuard started successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ---------- Loop ----------
void loop() {
  server.handleClient();
  yield();

  unsigned long now = millis();

  if (now - lastSensorRead > 5000) {
    lastSensorRead = now;
    readSensors();
  }

  if (now - lastPotCheck > 200) {
    lastPotCheck = now;
    updatePot();
  }
  yield();
}