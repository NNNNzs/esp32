#!/usr/bin/env python3
import argparse
import shutil
import subprocess
import sys
from pathlib import Path


REPOS = [
    ("waveshare-demo", "https://github.com/waveshareteam/ESP32-S3-Touch-LCD-1.83.git"),
    ("xiaozhi-esp32", "https://github.com/78/xiaozhi-esp32.git"),
]


def run(cmd, cwd=None):
    subprocess.run(cmd, cwd=cwd, check=True)


def main():
    parser = argparse.ArgumentParser(description="Fetch reference source trees on demand.")
    parser.add_argument("--root", default=str(Path(__file__).resolve().parents[1] / "_reference_sources"))
    parser.add_argument("--update", action="store_true")
    parser.add_argument("--include-aichats", action="store_true")
    args = parser.parse_args()

    root = Path(args.root)
    root.mkdir(parents=True, exist_ok=True)

    repos = list(REPOS)
    if args.include_aichats:
        repos.append(("ESP32-AIChats", "https://github.com/waveshareteam/ESP32-AIChats.git"))

    for name, url in repos:
        target = root / name
        if target.exists():
            if args.update:
                print(f"Updating {name}")
                run(["git", "-C", str(target), "pull", "--ff-only"])
            else:
                print(f"Already present: {target}")
            continue

        print(f"Cloning {name} into {target}")
        run(["git", "clone", url, str(target)])

    print("\nDone.")
    print(f"Root: {root}")


if __name__ == "__main__":
    sys.exit(main())
