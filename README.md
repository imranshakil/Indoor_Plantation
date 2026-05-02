#  Smart Indoor Plantation System

An IoT-based indoor plantation system designed to monitor plant conditions and provide optimal growth environment using automated lighting and real-time sensor data.

---

##  Project Overview

This project aims to create a smart indoor farming solution that can:
- Monitor environmental conditions
- Provide artificial sunlight using LED grow lights
- Display real-time data via web interface
- Allow remote monitoring and control

The system is built using ESP32 and multiple sensors, making it suitable for homes, research labs, and small-scale indoor farming.

---

##  Features

- **Real-time soil moisture monitoring** — detects when plants need watering
- **Temperature, humidity & CO₂ Level tracking** — ensures optimal growing conditions
- **Light level sensing** — monitors and controls lighting for plant growth
- **Water Level Detection** — alerts when water is needed 
- **Web-based dashboard (HTML interface)** — view live sensor data from any browser
- **Alert system** — notifies when conditions go out of safe range
- **Data logging** — keeps track of environmental history
- **Local Access** via `plant.local` (mDNS)

---

## System Architecture

### Overview
The Smart Indoor Plant Monitoring System is built around an **ESP32 microcontroller** acting as the central processing unit. It collects data from multiple sensors, processes them, and provides real-time visualization through a responsive web dashboard.

### High-Level Architecture

```
[ 24V DC Adapter ] ──► [ 24V Grow LED Supply ]
        │
        └──► [ Buck Converter 24V to 5V ] ──► [ ESP32 + 5V Sensors ]

[ Sensors ] ──► [ ESP32-WROOM-32 ] ──► [ Local Web Server ]
                    │
                    ├──► [ PWM Control ] ──► [ MOSFET Driver ] ──► [ 24V Grow LED ]
                    │
                    ├──► [ WiFi Status LED ]
                    │
                    └──► [ Warning LED ]
```

### Components

| Layer              | Components                                      | Description |
|--------------------|--------------------------------------------------|-----------|
| **Sensor Layer**   | DHT22, HW-038, Soil Moisture (x2), MH-Z19      | Collects environmental and water data |
| **Processing Layer** | ESP32 DevKit + Firmware                        | Reads sensors, processes data, controls outputs |
| **Actuation Layer** | PWM LED Driver + Warning LED                   | Controls grow light brightness |
| **Communication**  | Wi-Fi + mDNS (`plant.local`)                   | Enables local web access |
| **Presentation**   | Responsive Web Dashboard (HTML + CSS + JS)     | Real-time monitoring and control |
| **Power**          | 24V DC Supply                                  | Powers the entire system |


### Data Flow

1. Sensors continuously send analog/digital values to ESP32
2. ESP32 processes raw data and calculates alerts (Soil Low, Water Low, Temp High, etc.)
3. Web server serves real-time HTML page with live sensor values
4. User can control Grow Light brightness via web slider or physical potentiometer
5. System triggers visual warning LED when any critical condition is detected

### Technology Stack

- **Microcontroller**: ESP32 (Dual-core, Wi-Fi + BLE)
- **Firmware**: Arduino C++
- **Web Framework**: ESP32 WebServer + HTML/CSS/JS
- **Networking**: Wi-Fi Station Mode + mDNS
- **Sensors**: Analog + Digital (I2C/UART)

---


## Hardware Used

- **Microcontroller**: ESP32 DevKit
- **Capacitive Soil Moisture Sensors**: x2 (Analog)
- **Water Level Sensor**: HW-038
- **Temperature & Humidity**: DHT22
- **CO₂ Sensor**: MH-Z19
- **Grow Light**: Full Spectrum LED Strip (24V) with PWM control
- **Potentiometer**: 10K Ohm trim potentiometer
- **MOSFET**: FQP30N06N
- **Buck Converter**: LM2596
- **Capacitors**: x2 100uF, 470uF
- **Status LED**: x2 (Red,Green)
  
---

##  Power System

- Input: 24V DC Supply
- Step-down: LM2596 Buck Converter (5V)
- ESP32: Powered via Vin
- Sensors: 3.3V / 5V based on requirement

Capacitors are used at:
- Buck output
- ESP32 input
- CO₂ sensor
- LED Strip

---

## Software & Tools

- Arduino IDE
- ESP32 Board Support
- Libraries: WiFi, WebServer, DHT, MHZ19, ESPmDNS
- EasyEDA

##  Circuit Design

The full schematic diagram is included in this repository:

📄 `SCH_Design_2026-04-30.pdf`

Key highlights:
- 24V main power input
- Buck converter steps down to 5V
- ESP32 operates at 3.3V
- MOSFET controls high-power LED safely
- Capacitors used for noise filtering and stability

---

###  Functionality:
- Reads sensor data
- Hosts a web server
- Displays real-time values
- Controls LED brightness via PWM
- Sends data in structured format (JSON-ready)

---

## How to Access the Web Interface

After uploading the code:
1. Connect to the same WiFi network as the ESP32
2. Open browser and go to: **`http://plant.local`**  
   or use the IP address shown in Serial Monitor

## Web Features

- Live sensor readings
- Water level visualization (rectangular tank)
- Grow light brightness control (Slider + Toggle)
- System alerts with color coding
- Auto refresh every 5 seconds
  
---

##  Future Improvements

- Mobile App Integration
- Cloud Data Storage
- Automated Water Pump System
- AI-based Plant Growth Optimization
- Multi-Plant Support Expansion

---

## File Structure
 
```
Indoor_Plantation
│
├── Indoor_Plantation.ino      # Main firmware code
├── Html.ino                   # HTML dashboard interface code
├── Schematic_Diagram.pdf      # Circuit schematic diagram
└── README.md                  # Project documentation (this file)
```
---

## Team / Credits

| Name | Role |
|---|---|
| [Imran Shakil](https://github.com/imranshakil) | Project Supervisor & Repository Owner |
| [Sarwar An Noor](https://github.com/Sarwar-An-Noor) | Hardware, Firmware & Web Interface Developer |
 
---
 
> 🔗 **Repository:** [https://github.com/imranshakil/Indoor_Plantation](https://github.com/imranshakil/Indoor_Plantation)
##  Notes

This project is developed as part of an IoT-based smart agriculture system focusing on cost-effective and scalable indoor farming solutions.

---
