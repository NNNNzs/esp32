#!/usr/bin/env python3
import argparse
import importlib.util
import sys
from pathlib import Path


def load_builder():
    script = Path(__file__).resolve().parent / "offline_waveshare_wiki.py"
    spec = importlib.util.spec_from_file_location("offline_waveshare_wiki", script)
    module = importlib.util.module_from_spec(spec)
    assert spec and spec.loader
    spec.loader.exec_module(module)
    return module.main


def main():
    parser = argparse.ArgumentParser(description="Rebuild the offline Waveshare wiki mirror.")
    parser.add_argument("--out", default=str(Path(__file__).resolve().parents[1] / "docs" / "waveshare-esp32-s3-touch-lcd-1.83"))
    parser.add_argument("--url", default="https://www.waveshare.net/wiki/ESP32-S3-Touch-LCD-1.83")
    args = parser.parse_args()

    build_wiki = load_builder()
    sys.argv = ["offline_waveshare_wiki.py", "--url", args.url, "--out", args.out]
    return build_wiki()


if __name__ == "__main__":
    sys.exit(main())
