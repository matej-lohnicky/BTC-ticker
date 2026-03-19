# BTCticker (Screenshot Branch)

This branch is intentionally repurposed as a screenshot generator for the LILYGO T-Display S3. On boot, the firmware renders five deterministic screens and streams them over USB serial as RGB565 frames.

## Features

- Automatic boot sequence that renders and captures:
  - WiFi choose mock screen
  - Keyboard mock screen
  - Bitcoin ticker with full graph
  - Weather screen with fixed pleasant values
  - Clock screen fixed at 09:33
- USB serial screenshot stream protocol (`SHOT` header + RGB565 payload)
- Host-side conversion script to generate PNG files

## Tech Stack

- PlatformIO
- Arduino framework (ESP32)
- TFT_eSPI

## Repository Layout

- [src/core](src/core): app loop/controller and shared runtime globals
- [src/modes](src/modes): mode implementations (bitcoin, clock, weather)
- [src/hal](src/hal): hardware-facing modules (display, screenshot transport, brightness)
- [include/modules/modes](include/modules/modes): canonical mode headers
- [include/modules/hal](include/modules/hal): canonical HAL headers
- [include/modules](include/modules): shared non-domain headers
- [include/images](include/images): embedded graphics assets
- [scripts/capture_screenshots.py](scripts/capture_screenshots.py): host-side frame capture and PNG conversion
- [platformio.ini](platformio.ini): PlatformIO build configuration

## Requirements

- VS Code with PlatformIO extension
- ESP32 board supported by the configured environment
- Python 3 with `pyserial` and `Pillow` for host capture

## Getting Started

1. Build and flash the firmware.
2. Run the capture script while the device is connected via USB.

```powershell
pio run
pio run -t upload
python -m pip install pyserial pillow
python scripts/capture_screenshots.py --port COM4 --baud 115200 --output screenshots
```

No `.env`, API keys, weather lookup scripts, or pre-build configuration scripts are required.

Known-good example (from this workspace):

```powershell
python scripts/capture_screenshots.py --port COM3 --baud 115200 --output screenshots --settle 2 --timeout 20
```

If `pio` is not available in your shell, run the same actions through VS Code PlatformIO Build/Upload tasks.

Useful capture flags:

- `--settle 2` to wait longer after reset before listening
- `--timeout 20` to allow slower startup/capture
- `--no-swap-bytes` if colors look wrong in output PNGs

Expected PNG files:

- `wifi_choose.png`
- `keyboard.png`
- `bitcoin.png`
- `weather.png`
- `clock.png`

The script also writes `.rgb565` frame dumps alongside PNGs for debugging.

## Screenshot Protocol

Each frame is sent as:

- `magic[4] = "SHOT"`
- `version: uint8`
- `screen_id: uint8`
- `width: uint16` (little-endian)
- `height: uint16` (little-endian)
- `payload_bytes: uint32` (little-endian)
- `payload: RGB565, row-major`

The stream ends with `screen_id = 255` and an empty payload.

## Build Environment

Primary build target is defined in [platformio.ini](platformio.ini):

- Environment: lilygo-t-display-s3
- Framework: arduino
- Dependencies: TFT_eSPI
- Extra scripts: none

## Troubleshooting

- No frames captured:
  - Close any serial monitor using the same COM port
  - Reset the device and restart `scripts/capture_screenshots.py`
  - Use `--settle 2 --timeout 20` for slower board reset/re-enumeration
  - Re-upload firmware, then run capture again from a fresh boot
- Python import errors:
  - Install dependencies: `python -m pip install pyserial pillow`
- Corrupted output images:
  - Verify baud rate is `115200`
  - Retry capture after a hard reset so the script receives the stream from boot
  - Try `--no-swap-bytes` if color channels appear incorrect



## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
