#!/usr/bin/env python3
"""Build Arduino-style keyword.txt for ftSwarm from the header files.

This script reads all *.h files under include/, extracts:
1. classes starting with ftswarm (case-insensitive)
2. public methods for those classes
3. typedefs starting with ftswarm (case-insensitive)
4. enum values for those typedefs
5. #define names starting with ftswarm (case-insensitive)

and writes a single keyword.txt file in the project root.
"""

from __future__ import annotations

import re
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPT_DIR.parent
BUILD_DIR = PROJECT_ROOT / "build"
CORE_ROOT = PROJECT_ROOT / "src" / "ftswarm-core"
INCLUDE_DIR = CORE_ROOT / "include"
OUTPUT_FILE = BUILD_DIR / "keywords.txt"


def read_headers() -> list[tuple[Path, str]]:
    excluded = {"SwOSDefine.h"}
    headers: list[tuple[Path, str]] = []
    for path in sorted(INCLUDE_DIR.rglob("*.h")):
        if path.name in excluded:
            continue
        try:
            text = path.read_text(encoding="utf-8", errors="ignore")
        except Exception:
            continue
        headers.append((path, text))
    return headers


def add_if_ftswarm(names: set[str], value: str) -> None:
    if re.match(r"^ftswarm", value, flags=re.I):
        names.add(value)


def unique_sorted(values: set[str]) -> list[str]:
    return sorted(values, key=lambda s: s.lower())


# ---------- 1) class names ----------
def extract_class_names(headers) -> list[str]:
    classes: set[str] = set()
    for _, text in headers:
        for match in re.finditer(r"(?m)^\s*class\s+([A-Za-z_][A-Za-z0-9_]*)", text):
            name = match.group(1)
            add_if_ftswarm(classes, name)
    return unique_sorted(classes)


# ---------- 2) public methods ----------
def extract_public_methods(headers, class_names: list[str]) -> list[str]:
    methods: set[str] = set()

    for _, text in headers:
        for cls_name in class_names:
            pattern = re.compile(
                rf"(?ms)\bclass\s+{re.escape(cls_name)}\b[^{{]*\{{(.*?)\}}\s*;"
            )
            for match in pattern.finditer(text):
                body = match.group(1)

                # Find explicit public/protected/private sections and scan only public ones.
                for section_match in re.finditer(
                    r"(?ms)(?:^|\n)\s*(public|protected|private)\s*:\s*(.*?)(?=\n\s*(?:public|protected|private)\s*:\s*|\Z)",
                    body,
                ):
                    if section_match.group(1).lower() != "public":
                        continue
                    section = section_match.group(2)
                    for mm in re.finditer(
                        r"(?m)^\s*(?:[A-Za-z_][A-Za-z0-9_:<>\s\*&]+\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*(?:const\s*)?(?:\{[^}]*\}|;)",
                        section,
                    ):
                        name = mm.group(1)
                        if name and name == cls_name:
                            continue
                        if name and not re.match(r"^(?:if|for|while|switch|return|case|sizeof|new|delete)$", name, flags=re.I):
                            methods.add(name)

                # Also catch methods directly in the class body.
                for mm in re.finditer(
                    r"(?m)^\s*(?:[A-Za-z_][A-Za-z0-9_:<>\s\*&]+\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*\([^;]*\)\s*(?:const\s*)?\s*;",
                    body,
                ):
                    name = mm.group(1)
                    if name and name == cls_name:
                        continue
                    if name and not re.match(r"^(?:if|for|while|switch|return|case|sizeof|new|delete)$", name, flags=re.I):
                        methods.add(name)

    return unique_sorted(methods)


# ---------- 3) typedef names ----------
def extract_type_names(headers) -> list[str]:
    types: set[str] = set()
    for _, text in headers:
        for match in re.finditer(r"(?m)^\s*typedef\s+.*?\b([A-Za-z_][A-Za-z0-9_]*)\s*;", text):
            name = match.group(1)
            add_if_ftswarm(types, name)

        for match in re.finditer(
            r"(?m)^\s*typedef\s+enum\s*\{[^}]*\}\s*([A-Za-z_][A-Za-z0-9_]*)\s*;",
            text,
        ):
            name = match.group(1)
            add_if_ftswarm(types, name)

    return unique_sorted(types)


# ---------- 4) enum values ----------
def extract_enum_values(headers, type_names: list[str]) -> list[str]:
    values: set[str] = set()
    for _, text in headers:
        for match in re.finditer(
            r"(?ms)typedef\s+enum\s*\{(.*?)\}\s*([A-Za-z_][A-Za-z0-9_]*)\s*;",
            text,
        ):
            enum_body = match.group(1)
            type_name = match.group(2)
            if type_name.lower() not in {n.lower() for n in type_names}:
                continue
            for ev in re.finditer(r"(?m)\b([A-Za-z_][A-Za-z0-9_]*)\b(?:\s*=\s*[^,\n]+)?", enum_body):
                value_name = ev.group(1)
                add_if_ftswarm(values, value_name)
    return unique_sorted(values)


# ---------- 5) defines ----------
def extract_define_names(headers) -> list[str]:
    defines: set[str] = set()
    for _, text in headers:
        for match in re.finditer(r"(?m)^\s*#define\s+([A-Za-z_][A-Za-z0-9_]*)\b.*", text):
            name = match.group(1)
            add_if_ftswarm(defines, name)
    return unique_sorted(defines)


def build_keyword_lines(class_names, public_methods, type_names, enum_values, define_names) -> list[str]:
    lines: set[str] = set()

    for name in class_names:
        lines.add(f"{name}\tKEYWORD1")

    for name in public_methods:
        lines.add(f"{name}\tKEYWORD2")

    for name in type_names:
        lines.add(f"{name}\tKEYWORD1")

    for name in enum_values:
        lines.add(f"{name}\tLITERAL1")

    for name in define_names:
        lines.add(f"{name}\tLITERAL1")

    ordered = []
    for kind in ("KEYWORD1", "KEYWORD2", "LITERAL1"):
        items = [line for line in lines if line.endswith(f"\t{kind}")]
        items.sort(key=lambda line: line.split("\t", 1)[0].lower())
        ordered.extend(items)
    return ordered


def main() -> None:
    headers = read_headers()
    class_names = extract_class_names(headers)
    public_methods = extract_public_methods(headers, class_names)
    type_names = extract_type_names(headers)
    enum_values = extract_enum_values(headers, type_names)
    define_names = extract_define_names(headers)

    lines = build_keyword_lines(class_names, public_methods, type_names, enum_values, define_names)
    OUTPUT_FILE.write_text("\n".join(lines) + "\n", encoding="utf-8")

    print(f"Classes: {len(class_names)}")
    print(f"Methods: {len(public_methods)}")
    print(f"Types: {len(type_names)}")
    print(f"Enum values: {len(enum_values)}")
    print(f"Defines: {len(define_names)}")
    print(f"Wrote: {OUTPUT_FILE}")


if __name__ == "__main__":
    main()
