#!/usr/bin/env python3
import gzip
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

def pack_js(src_path: Path, header_path: Path, var_name: str):
    raw = src_path.read_bytes()
    gz = gzip.compress(raw, compresslevel=9, mtime=0)
    hex_lines = []
    for i in range(0, len(gz), 20):
        chunk = ", ".join(f"0x{b:02x}" for b in gz[i:i+20])
        hex_lines.append("  " + chunk + ",")
    header = (
        "#pragma once\n"
        "#include <Arduino.h>\n\n"
        f"const size_t {var_name}_len = {len(gz)};\n"
        f"const char {var_name}[] PROGMEM = {{\n"
        + "\n".join(hex_lines) +
        "\n};\n"
    )
    header_path.write_text(header, encoding="utf-8")
    print(f"Packed {src_path.name} ({len(raw)} B -> {len(gz)} B gz) => {header_path}")

if __name__ == "__main__":
    cat_js = ROOT / "miboxer_catalog.js"
    if cat_js.exists():
        pack_js(cat_js, ROOT / "lib" / "WebServer" / "MiBoxerCatalogAsset.h", "miboxer_catalog_js_gz")
    studio_js = ROOT / "remote_studio.js"
    if studio_js.exists():
        pack_js(studio_js, ROOT / "lib" / "WebServer" / "RemoteStudioAsset.h", "remote_studio_js_gz")
