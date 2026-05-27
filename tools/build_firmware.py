#!/usr/bin/env python3
import argparse
import os
import platform
import subprocess
import sys
from pathlib import Path


def run(cmd, cwd=None, env=None):
    subprocess.run(cmd, cwd=cwd, env=env, check=True)


def find_idf_export():
    idf_path = os.environ.get("IDF_PATH")
    if idf_path:
        candidate = Path(idf_path) / "export.sh"
        if candidate.exists():
            return candidate

    for candidate in [
        Path("/opt/esp/idf/export.sh"),
        Path.home() / "esp" / "idf" / "export.sh",
        Path("C:/esp/v6.0.1/esp-idf/export.ps1"),
    ]:
        if candidate.exists():
            return candidate
    return None


def main():
    parser = argparse.ArgumentParser(description="Build the Brookesia Claude firmware.")
    parser.add_argument("--project", default=str(Path(__file__).resolve().parents[1] / "firmware" / "brookesia-claude"))
    parser.add_argument("--port", default=None)
    parser.add_argument("--flash", action="store_true")
    args = parser.parse_args()

    project = Path(args.project)
    export = find_idf_export()
    if export is None:
        raise SystemExit("Could not find ESP-IDF export script. Set IDF_PATH first.")

    env = os.environ.copy()
    if export.suffix == ".ps1":
        cmd = [
            "powershell",
            "-NoProfile",
            "-ExecutionPolicy",
            "Bypass",
            "-File",
            str(export),
            "idf.py",
        ]
        if args.flash:
            cmd.append("flash")
        else:
            cmd.append("build")
        if args.port:
            cmd.extend(["-p", args.port])
        run(cmd, cwd=project, env=env)
        return 0

    shell = "/bin/bash" if platform.system() != "Windows" else "bash"
    action = "flash" if args.flash else "build"
    script = f'. "{export}" >/dev/null 2>&1 && idf.py {action}'
    if args.port:
        script = f'. "{export}" >/dev/null 2>&1 && idf.py -p "{args.port}" {action}'
    run([shell, "-lc", script], cwd=project, env=env)
    return 0


if __name__ == "__main__":
    sys.exit(main())
