#pragma once

#include "DeviceState.h"
#include "AlertService.h"
#include <DHT.h>
#include <MHZ19.h>
#include <HardwareSerial.h>

class SensorService {
public:
  explicit SensorService(AlertService& alertService);

  void begin(DeviceState& state);
  void update(DeviceState& state, bool force = false);

private:
  int calculateWaterPercent(int waterRaw) const;

  AlertService& alertService;
  DHT dht;
  MHZ19 mhz19;
  HardwareSerial mhzSerial;
  unsigned long lastRead = 0;
};
