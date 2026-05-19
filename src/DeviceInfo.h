#pragma once

#include <Arduino.h>

class DeviceInfo {
public:
  void begin();

  const String& getShortId() const;
  const String& getDeviceId() const;
  const String& getApName() const;
  const String& getMdnsName() const;
  const String& getLocalDns() const;

private:
  String shortId;
  String deviceId;
  String apName;
  String mdnsName;
  String localDns;
};
