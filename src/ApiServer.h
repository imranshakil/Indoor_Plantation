#pragma once

#include "DeviceInfo.h"
#include "DeviceState.h"
#include "WiFiService.h"
#include "LedService.h"
#include "SensorService.h"
#include <WebServer.h>

class ApiServer {
public:
  ApiServer(
    DeviceInfo& deviceInfo,
    DeviceState& state,
    WiFiService& wifiService,
    LedService& ledService,
    SensorService& sensorService
  );

  void begin();
  void handle();

private:
  void registerRoutes();
  void handleOptions();
  void handleDeviceInfo();
  void handleProvision();
  void handleStatus();
  void handleLedControl();
  void handleNotFound();

  WebServer server;
  DeviceInfo& deviceInfo;
  DeviceState& state;
  WiFiService& wifiService;
  LedService& ledService;
  SensorService& sensorService;
};
