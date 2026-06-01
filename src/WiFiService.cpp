#include "WiFiService.h"
#include "config.h"
#include <WiFi.h>
#include <ESPmDNS.h>

WiFiService::WiFiService(DeviceInfo& deviceInfo) : deviceInfo(deviceInfo) {}

void WiFiService::begin(DeviceState& state) {
  pinMode(WIFI_LED_PIN, OUTPUT);
  digitalWrite(WIFI_LED_PIN, LOW);
 

  startSetupHotspot();
  state.wifiConnected = false;
  state.wifiLedMode = WIFI_AP_MODE;
}

void WiFiService::startSetupHotspot() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(deviceInfo.getApName().c_str(), DEVICE_PASSWORD);

  Serial.println();
  Serial.println("===== SETUP MODE STARTED =====");
  Serial.print("AP Name: ");
  Serial.println(deviceInfo.getApName());
  Serial.print("AP Password: ");
  Serial.println(DEVICE_PASSWORD);
  Serial.print("Setup IP: ");
  Serial.println(WiFi.softAPIP());
}

bool WiFiService::provision(DeviceState& state, const String& ssid, const String& password) {
  Serial.println();
  Serial.println("Trying to connect to home Wi-Fi...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_AP_STA);
  state.wifiLedMode = WIFI_CONNECTING;
  WiFi.begin(ssid.c_str(), password.c_str());

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < WIFI_CONNECT_TIMEOUT_MS) {
    delay(WIFI_CONNECT_POLL_MS);
    Serial.print(".");
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    state.wifiConnected = true;
    state.wifiLedMode = WIFI_CONNECTED;
    digitalWrite(WIFI_LED_PIN, HIGH);

    Serial.println("Home Wi-Fi connected!");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin(deviceInfo.getMdnsName().c_str())) {
      Serial.print("mDNS started: ");
      Serial.println(deviceInfo.getLocalDns());
    } else {
      Serial.println("mDNS failed to start");
    }

    return true;
  }

    state.wifiConnected = false;
    state.wifiLedMode = WIFI_AP_MODE;
    digitalWrite(WIFI_LED_PIN, LOW);

  Serial.println("Home Wi-Fi connection failed");
  return false;
}

void WiFiService::update(DeviceState& state) {
  if (WiFi.status() != WL_CONNECTED) {
    state.wifiConnected = false;
  }

  updateWifiLed(state);
}

void WiFiService::updateWifiLed(DeviceState& state) {

  unsigned long now = millis();

  // App connected to ESP hotspot
  if (!state.wifiConnected &&
      WiFi.softAPgetStationNum() > 0) {

    state.wifiLedMode = WIFI_APP_CONNECTED;
  }

  switch (state.wifiLedMode) {

    // ESP hotspot active
    case WIFI_AP_MODE:

      if (now - lastWifiBlink >= 700) {

        lastWifiBlink = now;

        wifiLedState = !wifiLedState;

        digitalWrite(
          WIFI_LED_PIN,
          wifiLedState
        );
      }

      break;

    // Phone connected to hotspot
    case WIFI_APP_CONNECTED:

      digitalWrite(WIFI_LED_PIN, HIGH);

      break;

    // Connecting to home Wi-Fi
    case WIFI_CONNECTING:

      if (now - lastWifiBlink >= 150) {

        lastWifiBlink = now;

        wifiLedState = !wifiLedState;

        digitalWrite(
          WIFI_LED_PIN,
          wifiLedState
        );
      }

      break;

    // Connected to home Wi-Fi
    case WIFI_CONNECTED:

      digitalWrite(WIFI_LED_PIN, HIGH);

      break;
  }
}
