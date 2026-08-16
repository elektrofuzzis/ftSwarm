#!/usr/bin/env python3
"""Build Arduino library package from ftSwarm core sources and library template.

This script:
1. Creates structure in build/arduino/ftSwarm/
2. Copies ftswarm-core/include/ with subdirectories to library/src/
3. Copies ftswarm-core/src/ with subdirectories to library/src/
4. Copies platform/arduino/library-template/ files as library definitions
5. Calls build_keywords.py and stores keywords.txt in library root
6. Updates library metadata (library.properties, library.json) with version

Usage:
    python build_arduino_library.py [version]
    
    version - optional version string (e.g., "0.7.1") to update library metadata
"""

from __future__ import annotations

import json
import shutil
import subprocess
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent
BUILD_DIR = PROJECT_ROOT / "build"
CORE_DIR = PROJECT_ROOT / "src" / "ftswarm-core"
ARDUINO_DIR = BUILD_DIR / "arduino"
LIBRARY_DIR = ARDUINO_DIR / "ftSwarm"
LIBRARY_SRC_DIR = LIBRARY_DIR / "src"
TEMPLATE_DIR = PROJECT_ROOT / "src" / "platform" / "arduino" / "library-template"
ZIP_PATH = BUILD_DIR / "ftSwarm"


def remove_existing(path: Path) -> None:
    """Remove directory or file if it exists."""
    if path.exists():
        if path.is_dir():
            shutil.rmtree(path)
        else:
            path.unlink()


def copy_directory_recursive(src: Path, dest: Path) -> None:
    """Copy directory contents recursively, preserving directory structure."""
    if not src.exists():
        raise FileNotFoundError(f"Source directory does not exist: {src}")
    
    dest.mkdir(parents=True, exist_ok=True)
    for item in src.rglob("*"):
        if item.is_file():
            rel_path = item.relative_to(src)
            target = dest / rel_path
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(item, target)


def copy_template_files(src: Path, dest: Path) -> None:
    """Copy template files from library-template directory contents to dest."""
    if not src.exists():
        print(f"Note: Template directory not found at {src} - skipping template copy")
        return
    
    dest.mkdir(parents=True, exist_ok=True)
    
    # If src is a directory containing a single ftSwarm folder, go into it
    items = list(src.iterdir())
    if len(items) == 1 and items[0].is_dir() and items[0].name == "ftSwarm":
        src = items[0]
    
    # Copy all contents from src to dest
    for item in src.iterdir():
        target = dest / item.name
        if item.is_file():
            shutil.copy2(item, target)
        elif item.is_dir():
            shutil.copytree(item, target, dirs_exist_ok=True)


def run_build_keywords() -> bool:
    """Run build_keywords.py and return success status."""
    keywords_script = SCRIPT_DIR / "build_keywords.py"
    if not keywords_script.exists():
        print(f"Warning: build_keywords.py not found at {keywords_script}")
        return False
    
    try:
        result = subprocess.run(
            [sys.executable, str(keywords_script)],
            cwd=str(PROJECT_ROOT),
            capture_output=True,
            text=True,
            timeout=30
        )
        
        print("build_keywords.py output:")
        if result.stdout:
            print(result.stdout)
        if result.stderr:
            print(result.stderr, file=sys.stderr)
        
        return result.returncode == 0
    except subprocess.TimeoutExpired:
        print("Error: build_keywords.py timed out")
        return False
    except Exception as e:
        print(f"Error running build_keywords.py: {e}")
        return False


def integrate_keywords_into_library() -> None:
    """Copy generated keywords.txt into library root."""
    src_keywords = BUILD_DIR / "keywords.txt"
    if src_keywords.exists():
        dest_keywords = LIBRARY_DIR / "keywords.txt"
        shutil.copy2(src_keywords, dest_keywords)
        print(f"Integrated keywords: {dest_keywords}")
    else:
        print(f"Warning: keywords.txt not found at {src_keywords}")


def update_library_properties(version: str) -> None:
    """Update version in library.properties if it exists."""
    props_file = LIBRARY_DIR / "library.properties"
    if not props_file.exists():
        print(f"Note: library.properties not found at {props_file}")
        return
    
    content = props_file.read_text(encoding="utf-8")
    # Replace version line or add it if not present
    lines = content.split("\n")
    updated_lines = []
    version_found = False
    
    for line in lines:
        if line.strip().startswith("version="):
            updated_lines.append(f"version={version}")
            version_found = True
        else:
            updated_lines.append(line)
    
    if not version_found:
        updated_lines.append(f"version={version}")
    
    props_file.write_text("\n".join(updated_lines), encoding="utf-8")
    print(f"Updated library.properties with version: {version}")


def update_library_json(version: str) -> None:
    """Update version in library.json if it exists."""
    json_file = LIBRARY_DIR / "library.json"
    if not json_file.exists():
        print(f"Note: library.json not found at {json_file}")
        return
    
    try:
        data = json.loads(json_file.read_text(encoding="utf-8"))
        data["version"] = version
        json_file.write_text(json.dumps(data, indent=2), encoding="utf-8")
        print(f"Updated library.json with version: {version}")
    except Exception as e:
        print(f"Warning: Failed to update library.json: {e}")


def main() -> None:
    # Get version from command line argument if provided
    version = sys.argv[1] if len(sys.argv) > 1 else None
    
    print(f"Building Arduino library in {LIBRARY_DIR}")
    if version:
        print(f"Library version: {version}")
    
    # Clean existing build
    remove_existing(ARDUINO_DIR)
    remove_existing(ZIP_PATH.with_suffix(".zip"))
    
    # Create library structure
    LIBRARY_DIR.mkdir(parents=True, exist_ok=True)
    LIBRARY_SRC_DIR.mkdir(parents=True, exist_ok=True)
    
    # Copy template files (library metadata: library.properties, etc.)
    copy_template_files(TEMPLATE_DIR, LIBRARY_DIR)
    print(f"Copied library template from: {TEMPLATE_DIR}")
    
    # Copy core include files with subdirectories
    include_dir = CORE_DIR / "include"
    if include_dir.exists():
        copy_directory_recursive(include_dir, LIBRARY_SRC_DIR)
        print(f"Copied includes from: {include_dir}")
    else:
        raise FileNotFoundError(f"Include directory not found: {include_dir}")
    
    # Copy core source files with subdirectories
    src_dir = CORE_DIR / "src"
    if src_dir.exists():
        copy_directory_recursive(src_dir, LIBRARY_SRC_DIR)
        print(f"Copied sources from: {src_dir}")
    else:
        raise FileNotFoundError(f"Source directory not found: {src_dir}")
    
    # Update library metadata with version if provided
    if version:
        update_library_properties(version)
        update_library_json(version)
    
    # Generate and integrate keywords
    print("\nGenerating keywords...")
    if run_build_keywords():
        integrate_keywords_into_library()
    else:
        print("Warning: Keywords generation failed, continuing without keywords")
    
    # Create archive
    print(f"\nCreating archive...")
    archive_path = shutil.make_archive(
        str(ZIP_PATH),
        "zip",
        root_dir=str(ARDUINO_DIR),
        base_dir="ftSwarm"
    )
    
    print(f"\n[OK] Build complete:")
    print(f"  Library root: {LIBRARY_DIR}")
    print(f"  Library src:  {LIBRARY_SRC_DIR}")
    print(f"  Archive:      {archive_path}")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)
