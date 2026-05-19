#pragma once

#include <Arduino.h>

struct SensorValues {
  int soil1 = 0;
  int soil2 = 0;
  int waterRaw = 0;
  int waterPercent = 0;
  float temperature = 0.0;
  float humidity = 0.0;
  int co2 = 0;
};

struct AlertValues {
  bool soil1Low = false;
  bool soil2Low = false;
  bool waterLow = false;
  bool temperatureHigh = false;
  bool humidityHigh = false;
  bool anyAlert = false;
};

struct LedValues {
  bool power = false;
  int brightness = 0;
};

struct DeviceState {
  bool wifiConnected = false;
  SensorValues sensors;
  AlertValues alerts;
  LedValues led;
};
