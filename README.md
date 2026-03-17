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

- [src/core](src/core): app loop/controller and shared runtime globals
- [src/modes](src/modes): mode implementations (bitcoin, clock, weather)
- [src/hal](src/hal): hardware-facing modules (display, wifi, brightness, ui utils)
- [include/modules/modes](include/modules/modes): canonical mode headers
- [include/modules/hal](include/modules/hal): canonical HAL headers
- [include/modules/generated](include/modules/generated): auto-generated headers
- [include/modules](include/modules): shared non-domain headers
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
WEATHER_CITY=Prague
WEATHER_COUNTRY=CZ
```

Build integration:

- [platformio.ini](platformio.ini) runs [scripts/load_env.py](scripts/load_env.py) as a pre-build step
- [scripts/load_env.py](scripts/load_env.py) reads .env values
- [scripts/load_env.py](scripts/load_env.py) resolves WEATHER_CITY to coordinates and generates [include/modules/generated/weather_location.h](include/modules/generated/weather_location.h)
- Runtime values are injected as compile-time defines
- [src/hal/wifi_connect.cpp](src/hal/wifi_connect.cpp) uses those defines in Home mode
- [src/modes/weather_mode.cpp](src/modes/weather_mode.cpp) uses generated coordinates for weather fetch

Canonical include policy:

- Domain headers live in subfolders only (`modules/modes`, `modules/hal`, `modules/generated`)
- Root-level duplicate wrappers should not be reintroduced

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
  - Confirm WIFI_SSID, WIFI_PASSWORD, APP_TIMEZONE, and WEATHER_CITY are non-empty
  - Confirm the build machine has internet access to resolve WEATHER_CITY during pre-build
- Device fails to connect in Home mode:
  - Verify SSID/password values
  - Use Standard mode to test manual connection

## Contributing

Issues and pull requests are welcome.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
