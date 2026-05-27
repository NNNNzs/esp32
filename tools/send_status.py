#!/usr/bin/env python3
import argparse
import os
import sys
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description="Send a Claude Code status over serial.")
    parser.add_argument("status", choices=["idle", "thinking", "approval", "working", "done", "error"])
    parser.add_argument("--port", default=os.environ.get("ESP32_PORT", ""))
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    if not args.port:
        raise SystemExit("Missing serial port. Pass --port or set ESP32_PORT.")

    try:
        import serial
    except ImportError as exc:
        raise SystemExit("pyserial is required for serial status sending. Install it with `pip install pyserial`.") from exc

    with serial.Serial(args.port, args.baud, timeout=1, write_timeout=1) as ser:
        ser.write((args.status + "\n").encode("utf-8"))


if __name__ == "__main__":
    sys.exit(main())
