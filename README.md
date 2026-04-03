# Invictaeria — ESP32 Airplane Detection System

## Overview

**Invictaeria** is an IoT system built with an ESP32 microcontroller that detects the nearest airplane in real time using the [OpenSky Network API](https://opensky-network.org/), displaying live flight data on an OLED screen.

Developed as an academic project during the CTeSP in Cybersecurity at [ISTEC Porto](https://istec-porto.pt) (Instituto Superior de Tecnologias Avançadas do Porto), the system demonstrates practical skills in embedded systems, API integration, real-time data processing, and hardware prototyping.

Full technical reports are available in [Portuguese](docs/Invictaeria_Relatorio_PT.pdf) and [English](docs/Invictaeria_Report_ENG.pdf).

## How It Works

1. **WiFi Connection** — The ESP32 connects to a configured WiFi network and displays a loading bar on the OLED during the connection process.
2. **Geolocation** — The device determines its own geographic position using IP-based geolocation APIs.
3. **Aircraft Detection** — The system queries the OpenSky Network REST API for all aircraft within a configurable radius (~50 km) of the device's location.
4. **Nearest Aircraft Calculation** — Using the Haversine formula, it calculates the distance to each aircraft and identifies the closest one.
5. **OLED Display** — Flight data (ICAO24, callsign, latitude, longitude, altitude, velocity, distance, and ground status) is rendered on a 128×64 OLED screen, including a directional heading arrow.
6. **Auto-Refresh** — Data refreshes every 30 seconds with a countdown timer displayed on screen.

## Hardware Components

| Component | Specification |
|-----------|--------------|
| **Microcontroller** | ESP32 (WiFi-enabled) |
| **Display** | SSD1306 OLED 128×64 (I²C) |
| **LED Indicators** | Blue LED (GPIO 2) — WiFi status |
| **Power** | USB / 3.3V |

### Wiring Diagram

```
     OLED (SSD1306)           ESP32
   ┌──────────────┐       ┌────────┐
   │  GND  (1) ───────────▶ GND     │
   │  VCC  (2) ───────────▶ 3.3V    │
   │  SCL  (3) ───────────▶ GPIO22  │
   │  SDA  (4) ───────────▶ GPIO21  │
   └──────────────┘        └────────┘
```

## Photos

| | |
|:---:|:---:|
| ![Box Construction](imgs/Box%20Construction.jpg) | ![Device Exterior View](imgs/Device%20Exterior%20View.jpg) |
| Box Construction | Device Exterior View |
| ![System in Operation](imgs/System%20in%20operation.jpg) | ![Top View Open](imgs/Top%20View%20of%20Components%20with%20Enclosure%20Open.jpg) |
| System in Operation | Top View (Open) |
| ![Top View Closed](imgs/Top%20View%20of%20Components%20with%20Enclosure%20Closed.jpg) | ![Top View Open 2](imgs/Top%20View%20of%20Components%20with%20Enclosure%20Open_2.jpg) |
| Top View (Closed) | Top View (Open 2) |

## Software Stack

| Technology | Purpose |
|------------|---------|
| **C++ (Arduino)** | Firmware development |
| **ArduinoJson** | JSON parsing for API responses |
| **Adafruit SSD1306** | OLED display driver |
| **Adafruit GFX** | Graphics primitives library |
| **OpenSky Network API** | Live aircraft transponder data |
| **Arduino IDE** | Development environment |

## Dependencies

The following Arduino libraries are required:

- `WiFi.h` — ESP32 WiFi connectivity
- `HTTPClient.h` — HTTP requests to REST APIs
- `ArduinoJson.h` — JSON deserialization
- `Wire.h` — I²C communication
- `Adafruit_GFX.h` — Graphics base library
- `Adafruit_SSD1306.h` — SSD1306 OLED driver

## Source Files

| File | Description |
|------|-------------|
| [`src/final.ino`](src/final.ino) | Final version with bearing calculation, `ipinfo.io` geolocation, and refined display |
| [`src/projetoESPFuncional.ino`](src/projetoESPFuncional.ino) | Functional version with `ip-api.com` geolocation and LED indicators |

## Project Structure

- README.md — this file
- LICENSE — MIT License
- .portfolio.json — portfolio site integration metadata
- .gitignore — ignores build artifacts and temp files
- docs/
  - Invictaeria_Report_ENG.pdf — full report (English)
  - Invictaeria_Relatorio_PT.pdf — full report (Portuguese)
- diagrams/
  - invictaeria_uml_class_diagram.svg — UML class diagram
  - wiring_oled_esp32.txt — OLED to ESP32 wiring reference
- src/
  - final.ino — final firmware version
  - projetoESPFuncional.ino — functional firmware version
- assets/
  - social-preview.jpg — social preview image for link cards
- imgs/ — project photos

## Contact

- **Email:** sam.oliveira.dev@gmail.com
- **Compose in Gmail:** [Gmail](https://mail.google.com/mail/?view=cm&fs=1&to=sam.oliveira.dev@gmail.com&su=Invictaeria%20inquiry&body=Hi%20Samuel%2C%0A)
- **Compose in Outlook:** [Outlook](https://outlook.live.com/owa/?path=/mail/action/compose&to=sam.oliveira.dev@gmail.com&subject=Invictaeria%20inquiry&body=Hi%20Samuel%2C%0A)
- **LinkedIn:** [linkedin.com/in/jose-samuel-oliveira](https://www.linkedin.com/in/jose-samuel-oliveira)
- **Website:** [sam-ciber-dev.github.io](https://sam-ciber-dev.github.io)

## License

This project is licensed under the [MIT License](LICENSE). See [LICENSE](LICENSE) for details.

## Social Preview

The social preview image used for link cards:

<img src="assets/social-preview.png" alt="Invictaeria — ESP32 Airplane Detection System" width="640">

## Badges

![ESP32](https://img.shields.io/badge/ESP32-E7352C?style=for-the-badge&logo=espressif&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-00979D?style=for-the-badge&logo=arduino&logoColor=white)
![OpenSky](https://img.shields.io/badge/OpenSky_Network-1A1A2E?style=for-the-badge&logo=airplane&logoColor=white)
