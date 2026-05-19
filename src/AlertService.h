#pragma once

#include "DeviceState.h"

class AlertService {
public:
  void begin();
  void update(DeviceState& state);
};
