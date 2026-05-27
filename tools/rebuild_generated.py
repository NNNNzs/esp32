#!/usr/bin/env python3
import argparse
import importlib.util
import shutil
import sys
from pathlib import Path


def load_main(name):
    script = Path(__file__).resolve().parent / f"{name}.py"
    spec = importlib.util.spec_from_file_location(name, script)
    module = importlib.util.module_from_spec(spec)
    assert spec and spec.loader
    spec.loader.exec_module(module)
    return module.main


def remove_path(path: Path):
    if path.is_dir():
        shutil.rmtree(path)
    elif path.exists():
        path.unlink()


def main():
    parser = argparse.ArgumentParser(description="Clean and rebuild generated project content.")
    parser.add_argument("--project-root", default=str(Path(__file__).resolve().parents[1]))
    parser.add_argument("--skip-wiki", action="store_true")
    parser.add_argument("--skip-firmware", action="store_true")
    args = parser.parse_args()

    root = Path(args.project_root)

    remove_path(root / "firmware" / "brookesia-claude" / "build")
    remove_path(root / "firmware" / "brookesia-claude" / "managed_components")
    remove_path(root / "docs" / "waveshare-esp32-s3-touch-lcd-1.83")

    if not args.skip_wiki:
        rebuild_wiki_main = load_main("rebuild_offline_wiki")
        sys.argv = ["rebuild_offline_wiki.py"]
        rebuild_wiki_main()

    if not args.skip_firmware:
        build_firmware_main = load_main("build_firmware")
        sys.argv = ["build_firmware.py"]
        build_firmware_main()


if __name__ == "__main__":
    sys.exit(main())
