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

  // APP is now controlling LED
  state.led.appControl = true;

  ledcWrite(GROW_LED_PIN,
            state.led.power ? state.led.brightness : 0);
}

void LedService::updatePotControl(DeviceState& state) {

  unsigned long now = millis();

  if (now - lastPotCheck < POT_CHECK_INTERVAL_MS) {
    return;
  }

  lastPotCheck = now;

  static int lastPotValue = 0;

  int raw = analogRead(POT_PIN);
  int mappedValue = map(raw, 0, 4095, 0, 255);

  // If app is controlling, ignore pot
  if (state.led.appControl) {

    // Detect if user manually moved pot
    if (abs(mappedValue - lastPotValue) > 15) {

      // Pot takes control now
      state.led.appControl = false;

      state.led.power = mappedValue > 0;
      state.led.brightness = mappedValue;

      ledcWrite(
        GROW_LED_PIN,
        state.led.power ? state.led.brightness : 0
      );

      lastPotValue = mappedValue;
    }

    return;
  }

  // Pot control mode
  if (abs(mappedValue - state.led.brightness) > 8) {

    state.led.power = mappedValue > 0;
    state.led.brightness = mappedValue;

    ledcWrite(
      GROW_LED_PIN,
      state.led.power ? state.led.brightness : 0
    );

    lastPotValue = mappedValue;
  }
}
