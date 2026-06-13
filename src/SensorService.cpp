#include "SensorService.h"
#include "config.h"
#include <Arduino.h>

SensorService::SensorService(AlertService& alertService)
  : alertService(alertService),
    dht(DHT_PIN, DHT_TYPE),
    mhzSerial(2) {}

void SensorService::begin(DeviceState& state) {
  dht.begin();

  mhzSerial.begin(9600, SERIAL_8N1, MHZ19_RX_PIN, MHZ19_TX_PIN);
  mhz19.begin(mhzSerial);
  mhz19.autoCalibration(false);

  update(state, true);
}

void SensorService::update(DeviceState& state, bool force) {
  unsigned long now = millis();
  if (!force && now - lastRead < SENSOR_READ_INTERVAL_MS) {
    return;
  }

  lastRead = now;

  int soil1Raw = analogRead(SOIL1_PIN);
  int soil2Raw = analogRead(SOIL2_PIN);

  state.sensors.soil1 = calculateSoil1Percent(soil1Raw);
  state.sensors.soil2 = calculateSoil2Percent(soil2Raw);

  int rawWater = analogRead(WATER_PIN);
  state.sensors.waterRaw = (state.sensors.waterRaw * 7 + rawWater) / 8;
  state.sensors.waterPercent = calculateWaterPercent(state.sensors.waterRaw);

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (!isnan(temperature)) {
    state.sensors.temperature = temperature;
  }

  if (!isnan(humidity)) {
    state.sensors.humidity = humidity;
  }

  int co2 = mhz19.getCO2();
  if (co2 > 0 && co2 < 10000) {
    state.sensors.co2 = co2;
  }

  alertService.update(state);
}

int SensorService::calculateWaterPercent(int waterRaw) const {
  int percent = 0;

  if (waterRaw <= WATER_RAW_MIDDLE) {
    percent = map(waterRaw, WATER_RAW_EMPTY, WATER_RAW_MIDDLE, 0, 50);
  } else {
    percent = map(waterRaw, WATER_RAW_MIDDLE, WATER_RAW_FULL, 50, 100);
  }

  return constrain(percent, 0, 100);
}
int SensorService::calculateSoil1Percent(int raw) const {

  int percent = map(
    raw,
    SOIL1_RAW_DRY,
    SOIL1_RAW_WET,
    0,
    100
  );

  return constrain(percent, 0, 100);
}

int SensorService::calculateSoil2Percent(int raw) const {

  int percent = map(
    raw,
    SOIL2_RAW_DRY,
    SOIL2_RAW_WET,
    0,
    100
  );

  return constrain(percent, 0, 100);
}
