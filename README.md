# BTCticker

BTCticker is an ESP32 firmware project for the LILYGO T-Display S3. The device presents Bitcoin market data and utility dashboards on the built-in display.

## Features

- Live BTC price and 24h change display
- Animated Bitcoin mode with chart and session panel
- Digital clock mode
- Weather mode
- Button-based mode switching

## Tech Stack

- PlatformIO
- Arduino framework (ESP32)
- TFT_eSPI
- ArduinoJson
- HTTPClient

## Repository Layout

- [src](src): application source files
- [include/modules](include/modules): shared headers, constants, and declarations
- [include/images](include/images): embedded graphics assets
- [scripts/load_env.py](scripts/load_env.py): pre-build .env loader
- [platformio.ini](platformio.ini): PlatformIO build configuration

## Requirements

- VS Code with PlatformIO extension
- ESP32 board supported by the configured environment

## Getting Started

1. Clone the repository.
2. Create a local .env file from the template.
3. Build and upload the firmware.

```powershell
copy .env.example .env
pio run
pio run -t upload
pio device monitor -b 115200
```

## Configuration via .env

Wi-Fi credentials are provided through a local .env file in the repository root.

```env
WIFI_SSID=your_wifi_name
WIFI_PASSWORD=your_wifi_password
APP_TIMEZONE=CET-1CEST,M3.5.0/2,M10.5.0/3
```

Build integration:

- [platformio.ini](platformio.ini) runs [scripts/load_env.py](scripts/load_env.py) as a pre-build step
- [scripts/load_env.py](scripts/load_env.py) reads .env values
- The values are injected as compile-time defines
- [src/wifi_connect.cpp](src/wifi_connect.cpp) uses those defines in Home mode

If credentials are not present, firmware falls back to manual Wi-Fi selection/input flow.

## Security

- .env is ignored by .gitignore
- .env.example is tracked for onboarding
- Credentials are not stored in source headers

## Build Environment

Primary build target is defined in [platformio.ini](platformio.ini):

- Environment: lilygo-t-display-s3
- Framework: arduino
- Dependencies: TFT_eSPI, WiFi, ArduinoJson, HTTPClient

## Troubleshooting

- Build fails due to credentials:
  - Confirm .env exists in the project root
  - Confirm WIFI_SSID, WIFI_PASSWORD, and APP_TIMEZONE are non-empty
- Device fails to connect in Home mode:
  - Verify SSID/password values
  - Use Standard mode to test manual connection

## Contributing

Issues and pull requests are welcome.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
