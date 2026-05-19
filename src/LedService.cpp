#include "LedService.h"
#include "config.h"
#include <Arduino.h>

void LedService::begin(DeviceState& state) {
  pinMode(POT_PIN, INPUT);

  ledcAttach(GROW_LED_PIN, GROW_LED_PWM_FREQUENCY, GROW_LED_PWM_RESOLUTION);
  set(state, false, 0);
}

void LedService::set(DeviceState& state, bool power, int brightness) {
  state.led.power = power;
  state.led.brightness = constrain(brightness, 0, 255);

  ledcWrite(GROW_LED_PIN, state.led.power ? state.led.brightness : 0);
}

void LedService::updatePotControl(DeviceState& state) {
  unsigned long now = millis();
  if (now - lastPotCheck < POT_CHECK_INTERVAL_MS) {
    return;
  }

  lastPotCheck = now;

  int raw = analogRead(POT_PIN);
  int mappedValue = map(raw, 0, 4095, 0, 255);

  if (abs(mappedValue - state.led.brightness) > 8) {
    set(state, mappedValue > 0, mappedValue);
  }
}
