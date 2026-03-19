#!/usr/bin/env python3
"""Capture screenshot frames from BTCticker firmware and convert to PNG files.

Protocol per frame (little-endian):
- magic[4] = b"SHOT"
- version: uint8
- screen_id: uint8
- width: uint16
- height: uint16
- payload_bytes: uint32
- payload: RGB565 bytes (width * height * 2)

A frame with screen_id=255 marks completion.
"""

from __future__ import annotations

import argparse
import pathlib
import struct
import sys
import time

import serial

try:
    from PIL import Image
except ImportError as exc:  # pragma: no cover
    raise SystemExit("Pillow is required: pip install pillow") from exc

MAGIC = b"SHOT"
HEADER = struct.Struct("<4sBBHHI")
COMPLETE_ID = 255

SCREEN_NAMES = {
    1: "wifi_choose",
    2: "keyboard",
    3: "bitcoin",
    4: "weather",
    5: "clock",
}


def reset_esp32(port: serial.Serial, hold: float, settle: float) -> None:
    """Trigger a hardware reset using UART control lines.

    Most ESP32 boards expose EN through USB bridge control signals.
    We keep RTS deasserted to avoid strapping into flash mode, and pulse DTR.
    """
    try:
        port.setRTS(False)
        port.setDTR(False)
        time.sleep(hold)
        port.setDTR(True)
        time.sleep(hold)
        port.setDTR(False)
        time.sleep(settle)
    except serial.SerialException as exc:
        raise RuntimeError(f"Failed to reset ESP32 over serial lines: {exc}") from exc


def read_exact(port: serial.Serial, size: int) -> bytes:
    data = bytearray()
    while len(data) < size:
        chunk = port.read(size - len(data))
        if not chunk:
            raise TimeoutError(f"Timed out while reading {size} bytes")
        data.extend(chunk)
    return bytes(data)


def find_next_magic(port: serial.Serial) -> None:
    window = bytearray()
    while True:
        byte = port.read(1)
        if not byte:
            raise TimeoutError("Timed out waiting for screenshot stream")
        window += byte
        if len(window) > 4:
            window.pop(0)
        if bytes(window) == MAGIC:
            return


def rgb565_to_png(
    raw_data: bytes,
    width: int,
    height: int,
    output_path: pathlib.Path,
    swap_bytes: bool,
) -> None:
    pixels = []
    for i in range(0, len(raw_data), 2):
        if swap_bytes:
            value = (raw_data[i] << 8) | raw_data[i + 1]
        else:
            value = raw_data[i] | (raw_data[i + 1] << 8)
        red = ((value >> 11) & 0x1F) * 255 // 31
        green = ((value >> 5) & 0x3F) * 255 // 63
        blue = (value & 0x1F) * 255 // 31
        pixels.append((red, green, blue))

    image = Image.new("RGB", (width, height))
    image.putdata(pixels)
    image.save(output_path)


def capture_frames(
    port: serial.Serial, output_dir: pathlib.Path, swap_bytes: bool
) -> int:
    saved = 0
    while True:
        find_next_magic(port)
        remainder = read_exact(port, HEADER.size - 4)
        magic, version, screen_id, width, height, payload_bytes = HEADER.unpack(
            MAGIC + remainder
        )

        if magic != MAGIC:
            continue
        if version != 1:
            raise ValueError(f"Unsupported protocol version: {version}")

        if screen_id == COMPLETE_ID:
            return saved

        expected = width * height * 2
        if payload_bytes != expected:
            raise ValueError(
                f"Invalid payload length for screen {screen_id}: got {payload_bytes}, expected {expected}"
            )

        raw_payload = read_exact(port, payload_bytes)
        raw_path = (
            output_dir / f"{SCREEN_NAMES.get(screen_id, f'screen_{screen_id}')}.rgb565"
        )
        png_path = (
            output_dir / f"{SCREEN_NAMES.get(screen_id, f'screen_{screen_id}')}.png"
        )
        raw_path.write_bytes(raw_payload)
        rgb565_to_png(raw_payload, width, height, png_path, swap_bytes=swap_bytes)
        saved += 1

        print(f"Saved {png_path.name} ({width}x{height})")


def main() -> int:
    parser = argparse.ArgumentParser(description="Capture BTCticker screenshot stream")
    parser.add_argument("--port", required=True, help="Serial port (e.g. COM4)")
    parser.add_argument("--baud", type=int, default=115200, help="Serial baud rate")
    parser.add_argument(
        "--output",
        default="screenshots",
        help="Directory for output .rgb565 and .png files",
    )
    parser.add_argument(
        "--timeout",
        type=float,
        default=15.0,
        help="Serial read timeout in seconds",
    )
    parser.add_argument(
        "--settle",
        type=float,
        default=1.5,
        help="Seconds to wait after opening port before listening",
    )
    parser.add_argument(
        "--no-reset",
        action="store_true",
        help="Do not toggle DTR/RTS for ESP reset (wait --settle instead)",
    )
    parser.add_argument(
        "--reset-hold",
        type=float,
        default=0.08,
        help="Seconds for DTR pulse duration when resetting",
    )
    parser.add_argument(
        "--reset-settle",
        type=float,
        default=0.7,
        help="Seconds to wait after reset before capture starts",
    )
    parser.add_argument(
        "--no-swap-bytes",
        action="store_true",
        help="Decode RGB565 without swapping byte order",
    )
    args = parser.parse_args()

    output_dir = pathlib.Path(args.output)
    output_dir.mkdir(parents=True, exist_ok=True)

    print(f"Opening {args.port} @ {args.baud}...")

    def run_capture_once() -> int:
        with serial.Serial(args.port, args.baud, timeout=args.timeout) as capture_port:
            capture_port.reset_input_buffer()
            count = capture_frames(
                capture_port,
                output_dir,
                swap_bytes=not args.no_swap_bytes,
            )
        print(f"Done. Saved {count} frames to {output_dir}")
        return 0

    if args.no_reset:
        print(f"No reset requested; waiting {args.settle:.2f}s...")
        time.sleep(args.settle)
        try:
            return run_capture_once()
        except TimeoutError as exc:
            print(f"Capture timeout: {exc}", file=sys.stderr)
            return 2
        except RuntimeError as exc:
            print(f"Capture setup failed: {exc}", file=sys.stderr)
            return 3

    try:
        # Use a short-lived control connection for reset, then reopen for capture.
        with serial.Serial(args.port, args.baud, timeout=args.timeout) as control_port:
            control_port.reset_input_buffer()
            print("Resetting ESP32 over serial control lines...")
            reset_esp32(control_port, args.reset_hold, args.reset_settle)

        # Some boards briefly detach USB after reset; reopen after settle.
        time.sleep(args.settle)
        return run_capture_once()
    except TimeoutError as exc:
        print(f"Capture timeout after reset: {exc}", file=sys.stderr)
        print("Retrying once without reset...", file=sys.stderr)
        try:
            time.sleep(args.settle)
            return run_capture_once()
        except TimeoutError as fallback_exc:
            print(f"Capture timeout: {fallback_exc}", file=sys.stderr)
            return 2
    except RuntimeError as exc:
        print(f"Capture setup failed: {exc}", file=sys.stderr)
        return 3
    except serial.SerialException as exc:
        print(f"Serial open failed on {args.port}: {exc}", file=sys.stderr)
        return 4


if __name__ == "__main__":
    raise SystemExit(main())
