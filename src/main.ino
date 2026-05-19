#include <Arduino.h>
#include <WiFi.h>
#include "DeviceInfo.h"
#include "DeviceState.h"
#include "AlertService.h"
#include "SensorService.h"
#include "LedService.h"
#include "WiFiService.h"
#include "ApiServer.h"

DeviceInfo deviceInfo;
DeviceState deviceState;
AlertService alertService;
SensorService sensorService(alertService);
LedService ledService;
WiFiService wifiService(deviceInfo);
ApiServer apiServer(deviceInfo, deviceState, wifiService, ledService, sensorService);

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting FloraGuard ESP32...");

  deviceInfo.begin();
  alertService.begin();
  sensorService.begin(deviceState);
  ledService.begin(deviceState);
  wifiService.begin(deviceState);

  Serial.println("Waiting for app to send Wi-Fi credentials...");

  apiServer.begin();

  Serial.println();
  Serial.println("===== DEVICE READY =====");
  Serial.print("Device ID: ");
  Serial.println(deviceInfo.getDeviceId());
  Serial.print("Setup AP: ");
  Serial.println(deviceInfo.getApName());
  Serial.print("Setup IP: ");
  Serial.println(WiFi.softAPIP());
  Serial.print("Local DNS: ");
  Serial.println(deviceInfo.getLocalDns());
  Serial.println("Connect your phone to the AP above and provision via app.");
}

void loop() {
  apiServer.handle();
  wifiService.update(deviceState);
  sensorService.update(deviceState);
  ledService.updatePotControl(deviceState);
}
