#!/usr/bin/env python3
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SWOS_HEADER = ROOT / "src" / "ftswarm-core" / "include" / "SwOS.h"
BUILD_DIR = ROOT / "build"


def extract_swos_version(header_path: Path) -> str:
    text = header_path.read_text(encoding="utf-8")
    match = re.search(r'(?m)^\s*#\s*define\s+SWOSVERSION\s+"([^"]+)"', text)
    if match:
        return match.group(1)
    raise ValueError(f"SWOSVERSION not found in {header_path}")


def run_build_arduino_library(version: str) -> bool:
    """Run build_arduino_library.py with the version parameter."""
    build_library_script = Path(__file__).parent / "build_arduino_library.py"
    if not build_library_script.exists():
        print(f"Warning: build_arduino_library.py not found at {build_library_script}")
        return False
    
    try:
        result = subprocess.run(
            [sys.executable, str(build_library_script), version],
            cwd=str(ROOT),
            capture_output=True,
            text=True,
            timeout=60
        )
        
        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)
        
        return result.returncode == 0
    except subprocess.TimeoutExpired:
        print("Error: build_arduino_library.py timed out")
        return False
    except Exception as e:
        print(f"Error running build_arduino_library.py: {e}")
        return False


def run_build_pio_package(version: str) -> bool:
    """Run build-pio-package.py with the version parameter."""
    build_package_script = Path(__file__).parent / "build-pio-package.py"
    if not build_package_script.exists():
        print(f"Warning: build-pio-package.py not found at {build_package_script}")
        return False

    try:
        result = subprocess.run(
            [sys.executable, str(build_package_script), version],
            cwd=str(ROOT),
            capture_output=True,
            text=True,
            timeout=60,
        )

        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)

        return result.returncode == 0
    except subprocess.TimeoutExpired:
        print("Error: build-pio-package.py timed out")
        return False
    except Exception as e:
        print(f"Error running build-pio-package.py: {e}")
        return False


def run_build_firmware() -> bool:
    """Run build-firmware.py for all configured PlatformIO environments."""
    build_firmware_script = Path(__file__).parent / "build-firmware.py"
    if not build_firmware_script.exists():
        print(f"Warning: build-firmware.py not found at {build_firmware_script}")
        return False

    try:
        result = subprocess.run(
            [sys.executable, str(build_firmware_script)],
            cwd=str(ROOT),
            capture_output=True,
            text=True,
        )

        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)

        return result.returncode == 0
    except Exception as e:
        print(f"Error running build-firmware.py: {e}")
        return False


if __name__ == "__main__":
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    version = extract_swos_version(SWOS_HEADER)
    version_file = BUILD_DIR / "swos_version.txt"
    version_file.write_text(version + "\n", encoding="utf-8")

    print(f"Build directory: {BUILD_DIR}")
    print(f"SWOSVERSION: {version}")
    print(f"Saved to: {version_file}")
    
    # Call build_arduino_library.py with the version
    print(f"\nBuilding Arduino library with version {version}...")
    if run_build_arduino_library(version):
        print("Arduino library build completed successfully")
    else:
        print("Warning: Arduino library build encountered an error")

    print(f"\nBuilding PlatformIO package with version {version}...")
    if run_build_pio_package(version):
        print("PlatformIO package build completed successfully")
    else:
        print("Warning: PlatformIO package build encountered an error")

    print("\nBuilding firmware for all PlatformIO environments...")
    if run_build_firmware():
        print("Firmware build completed successfully")
    else:
        print("Warning: Firmware build encountered an error")

