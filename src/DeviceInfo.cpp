#include "DeviceInfo.h"
#include <Esp.h>

void DeviceInfo::begin() {
  uint64_t mac = ESP.getEfuseMac();

  char idBuffer[7];
  snprintf(idBuffer, sizeof(idBuffer), "%06X", static_cast<uint32_t>(mac & 0xFFFFFF));

  shortId = String(idBuffer);
  deviceId = "PLANT-" + shortId;
  apName = "Plant-Setup-" + shortId;
  mdnsName = "plant-" + shortId;
  localDns = "http://" + mdnsName + ".local";
}

const String& DeviceInfo::getShortId() const { return shortId; }
const String& DeviceInfo::getDeviceId() const { return deviceId; }
const String& DeviceInfo::getApName() const { return apName; }
const String& DeviceInfo::getMdnsName() const { return mdnsName; }
const String& DeviceInfo::getLocalDns() const { return localDns; }
