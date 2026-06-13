#pragma once

#include <Arduino.h>
#include <DHT.h>

// ===================== DEVICE SECURITY =====================
// Change this before production.
constexpr const char* DEVICE_PASSWORD = "12345678";

// ===================== SENSOR PINS =====================
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t DHT_TYPE = DHT22;

constexpr uint8_t SOIL1_PIN = 34;
constexpr uint8_t SOIL2_PIN = 35;
constexpr uint8_t WATER_PIN = 33;
constexpr uint8_t POT_PIN = 32;

constexpr uint8_t GROW_LED_PIN = 25;
constexpr uint8_t WIFI_LED_PIN = 2;
constexpr uint8_t WARN_LED_PIN = 21;

constexpr uint8_t MHZ19_RX_PIN = 16;
constexpr uint8_t MHZ19_TX_PIN = 17;

// ===================== LED PWM =====================
constexpr uint32_t GROW_LED_PWM_FREQUENCY = 1000;
constexpr uint8_t GROW_LED_PWM_RESOLUTION = 8;

// ===================== THRESHOLDS =====================
/ Soil Sensor 1 Calibration
constexpr int SOIL1_RAW_DRY = 3450;
constexpr int SOIL1_RAW_WET = 1760;

// Soil Sensor 2 Calibration
constexpr int SOIL2_RAW_DRY = 2870;
constexpr int SOIL2_RAW_WET = 1020;

// Moisture Alert Threshold
constexpr int SOIL_LOW_PERCENT = 30;
constexpr int WATER_LOW_PERCENT = 20;
constexpr float TEMP_MAX_C = 35.0;
constexpr float HUMIDITY_MAX_PERCENT = 80.0;

// ===================== TIMING =====================
constexpr unsigned long SENSOR_READ_INTERVAL_MS = 5000;
constexpr unsigned long POT_CHECK_INTERVAL_MS = 300;
constexpr unsigned long WIFI_LED_BLINK_INTERVAL_MS = 500;
constexpr unsigned long WIFI_CONNECT_TIMEOUT_MS = 20000;
constexpr unsigned long WIFI_CONNECT_POLL_MS = 500;

// ===================== WATER SENSOR CALIBRATION =====================
constexpr int WATER_RAW_EMPTY = 160;
constexpr int WATER_RAW_FULL  = 330;

// ===================== API =====================
constexpr uint16_t API_PORT = 80;


// ===================== CLOUD / DJANGO API For Backups=====================
constexpr const char* CLOUD_SENSOR_LOG_URL = "http://192.168.68.148:8000/api/plantation/sensor-log";
constexpr const char* CLOUD_EVENT_LOG_URL  = "http://192.168.68.148:8000/api/plantation/event-log";
constexpr const char* API_TOKEN = "CHANGE_THIS_SECRET_TOKEN";

constexpr unsigned long CLOUD_SENSOR_SEND_INTERVAL_MS = 300000; // 5 minute
