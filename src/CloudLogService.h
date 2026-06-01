#pragma once

#include "DeviceInfo.h"
#include "DeviceState.h"

class CloudLogService {
public:
  CloudLogService(DeviceInfo& deviceInfo);

  void update(DeviceState& state);
  void sendSensorLog(DeviceState& state);
  void detectAndSendEvents(DeviceState& state);

private:
  DeviceInfo& deviceInfo;

  unsigned long lastSensorSend = 0;

  bool lastSoil1Low = false;
  bool lastSoil2Low = false;
  bool lastWaterLow = false;
  bool lastTempHigh = false;
  bool lastHumidityHigh = false;

  void sendEvent(const char* eventType, int eventValue, const char* description);
};