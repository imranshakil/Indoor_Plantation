#pragma once

#include "DeviceInfo.h"
#include "DeviceState.h"
#include <Arduino.h>

class WiFiService {
public:
  explicit WiFiService(DeviceInfo& deviceInfo);

  void begin(DeviceState& state);
  bool provision(DeviceState& state, const String& ssid, const String& password);
  void update(DeviceState& state);

private:
  void startSetupHotspot();
  void updateWifiLed(DeviceState& state);

  DeviceInfo& deviceInfo;
  unsigned long lastWifiBlink = 0;
  bool wifiLedState = false;
};
