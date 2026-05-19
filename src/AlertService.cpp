#include "AlertService.h"
#include "config.h"
#include <Arduino.h>

void AlertService::begin() {
  pinMode(WARN_LED_PIN, OUTPUT);
  digitalWrite(WARN_LED_PIN, LOW);
}

void AlertService::update(DeviceState& state) {
  state.alerts.soil1Low = state.sensors.soil1 < SOIL_LOW_THRESHOLD;
  state.alerts.soil2Low = state.sensors.soil2 < SOIL_LOW_THRESHOLD;
  state.alerts.waterLow = state.sensors.waterPercent <= WATER_LOW_PERCENT;
  state.alerts.temperatureHigh = state.sensors.temperature > TEMP_MAX_C;
  state.alerts.humidityHigh = state.sensors.humidity > HUMIDITY_MAX_PERCENT;

  state.alerts.anyAlert = state.alerts.soil1Low ||
                          state.alerts.soil2Low ||
                          state.alerts.waterLow ||
                          state.alerts.temperatureHigh ||
                          state.alerts.humidityHigh;

  digitalWrite(WARN_LED_PIN, state.alerts.anyAlert ? HIGH : LOW);
}
