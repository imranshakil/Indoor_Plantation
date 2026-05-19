#pragma once

#include "DeviceState.h"

class LedService {
public:
  void begin(DeviceState& state);
  void set(DeviceState& state, bool power, int brightness);
  void updatePotControl(DeviceState& state);

private:
  unsigned long lastPotCheck = 0;
};
