#include "CloudLogService.h"
#include "config.h"

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

CloudLogService::CloudLogService(DeviceInfo& deviceInfo)
  : deviceInfo(deviceInfo) {}

void CloudLogService::update(DeviceState& state) {
  if (WiFi.status() != WL_CONNECTED) {
    return;
  }

  unsigned long now = millis();

  if (now - lastSensorSend >= CLOUD_SENSOR_SEND_INTERVAL_MS) {
    lastSensorSend = now;
    sendSensorLog(state);
  }

  detectAndSendEvents(state);
}

void CloudLogService::sendSensorLog(DeviceState& state) {
  HTTPClient http;
  http.begin(CLOUD_SENSOR_LOG_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + API_TOKEN);

  StaticJsonDocument<768> doc;

  doc["device_id"] = deviceInfo.getDeviceId();
  doc["temperature_c"] = state.sensors.temperature;
  doc["humidity_percent"] = state.sensors.humidity;
  doc["soil1_raw"] = state.sensors.soil1;
  doc["soil2_raw"] = state.sensors.soil2;
  doc["water_raw"] = state.sensors.waterRaw;
  doc["water_percent"] = state.sensors.waterPercent;
  doc["co2_ppm"] = state.sensors.co2;
  doc["grow_light_power"] = state.led.power;
  doc["grow_light_pwm"] = state.led.brightness;
  doc["wifi_signal"] = WiFi.RSSI();
  doc["uptime_seconds"] = millis() / 1000;

  String payload;
  serializeJson(doc, payload);

  int responseCode = http.POST(payload);

  Serial.print("[CLOUD SENSOR] Response: ");
  Serial.println(responseCode);

  if (responseCode <= 0) {
    Serial.print("[CLOUD SENSOR] Error: ");
    Serial.println(http.errorToString(responseCode));
  }

  http.end();
}

void CloudLogService::detectAndSendEvents(DeviceState& state) {
  if (state.alerts.soil1Low != lastSoil1Low) {
    lastSoil1Low = state.alerts.soil1Low;
    sendEvent(
      state.alerts.soil1Low ? "SOIL1_DRY" : "SOIL1_NORMAL",
      state.sensors.soil1,
      state.alerts.soil1Low ? "Soil sensor 1 is dry" : "Soil sensor 1 returned to normal"
    );
  }

  if (state.alerts.soil2Low != lastSoil2Low) {
    lastSoil2Low = state.alerts.soil2Low;
    sendEvent(
      state.alerts.soil2Low ? "SOIL2_DRY" : "SOIL2_NORMAL",
      state.sensors.soil2,
      state.alerts.soil2Low ? "Soil sensor 2 is dry" : "Soil sensor 2 returned to normal"
    );
  }

  if (state.alerts.waterLow != lastWaterLow) {
    lastWaterLow = state.alerts.waterLow;
    sendEvent(
      state.alerts.waterLow ? "WATER_LOW" : "WATER_NORMAL",
      state.sensors.waterPercent,
      state.alerts.waterLow ? "Water level is low" : "Water level returned to normal"
    );
  }

  if (state.alerts.temperatureHigh != lastTempHigh) {
    lastTempHigh = state.alerts.temperatureHigh;
    sendEvent(
      state.alerts.temperatureHigh ? "TEMP_HIGH" : "TEMP_NORMAL",
      (int)state.sensors.temperature,
      state.alerts.temperatureHigh ? "Temperature is high" : "Temperature returned to normal"
    );
  }

  if (state.alerts.humidityHigh != lastHumidityHigh) {
    lastHumidityHigh = state.alerts.humidityHigh;
    sendEvent(
      state.alerts.humidityHigh ? "HUMIDITY_HIGH" : "HUMIDITY_NORMAL",
      (int)state.sensors.humidity,
      state.alerts.humidityHigh ? "Humidity is high" : "Humidity returned to normal"
    );
  }
}

void CloudLogService::sendEvent(const char* eventType, int eventValue, const char* description) {
  HTTPClient http;
  http.begin(CLOUD_EVENT_LOG_URL);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", String("Bearer ") + API_TOKEN);

  StaticJsonDocument<512> doc;

  doc["device_id"] = deviceInfo.getDeviceId();
  doc["event_type"] = eventType;
  doc["event_value"] = eventValue;
  doc["description"] = description;
  doc["wifi_signal"] = WiFi.RSSI();
  doc["uptime_seconds"] = millis() / 1000;

  String payload;
  serializeJson(doc, payload);

  int responseCode = http.POST(payload);

  Serial.print("[CLOUD EVENT] ");
  Serial.print(eventType);
  Serial.print(" Response: ");
  Serial.println(responseCode);

  http.end();
}