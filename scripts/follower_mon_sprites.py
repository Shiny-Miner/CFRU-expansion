#!/usr/bin/env python3
"""Extract and rebuild the fixed-size follower-mon overworld sprite slots."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

SCRIPT_DIR = str(Path(__file__).resolve().parent)
if SCRIPT_DIR in sys.path:
    sys.path.remove(SCRIPT_DIR)

try:
    from PIL import Image
except ImportError:
    print("Pillow is not installed. Installing Pillow...")
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "Pillow"])
    from PIL import Image


ROOT = Path(__file__).resolve().parents[1]
SPRITE_DIR = ROOT / "follower_mon_sprites"
MANIFEST = SPRITE_DIR / "manifest.txt"
SOURCE = ROOT / "free_bytereplacements"
HEADER = ROOT / "include" / "follower_mon_sprites.h"
FOLLOWER_TABLE = ROOT / "src" / "Tables" / "follower_mon_sprites.c"
PALETTE_TABLE = ROOT / "src" / "character_customization.c"
NORMAL_PALETTES = ROOT / "assembly" / "followingmon_normal_pals.s"
SLOT_SIZE = 0xC00
SHEET_WIDTH = 96
SHEET_HEIGHT = 64
BASE_GBA_ADDRESS = 0x08900000
DEFINE_RE = re.compile(
    r"^#define\s+(gFollowingMonPic_[A-Za-z0-9_]+Tiles)\s+"
    r"\(\(u8\*\)(0x[0-9A-Fa-f]+)\)\s*$"
)


def read_source_bytes() -> bytes:
    for line in SOURCE.read_text(encoding="ascii").splitlines():
        if line.startswith("followermon_Sprites "):
            return bytes.fromhex(line.split(maxsplit=1)[1])
    built_binary = ROOT / "build" / "follower_mon_sprites.bin"
    if built_binary.is_file():
        return built_binary.read_bytes()
    raise ValueError("follower-mon source bytes were not found")


def read_symbols() -> list[tuple[int, str]]:
    symbols = []
    for line in HEADER.read_text(encoding="utf-8").splitlines():
        match = DEFINE_RE.match(line)
        if not match:
            continue
        symbol, address_text = match.groups()
        relative = int(address_text, 16) - BASE_GBA_ADDRESS
        if relative < 0:
            raise ValueError(f"invalid follower-mon symbol address: {line}")
        symbols.append((relative, symbol))
    if symbols:
        return sorted(symbols)

    generated_asm = ROOT / "build" / "follower_mon_sprites.s"
    asm_symbols = []
    for line in generated_asm.read_text(encoding="ascii").splitlines():
        match = re.match(r"\.set\s+(gFollowingMonPic_[A-Za-z0-9_]+Tiles),\s*(0x[0-9a-fA-F]+)", line)
        if match:
            asm_symbols.append((int(match.group(2), 16), match.group(1)))
    if not asm_symbols:
        raise ValueError("no follower-mon symbols were found in the header or generated assembly")
    base_address = asm_symbols[0][0]
    return [(address - base_address, symbol) for address, symbol in asm_symbols]


def dimensions_for_size(size: int) -> tuple[int, int]:
    dimensions = {
        0xC00: (32, 192),
        0x3000: (64, 384),
    }
    try:
        return dimensions[size]
    except KeyError as exc:
        raise ValueError(f"unsupported follower-mon sheet size: {size:#x}") from exc


def slot_to_image(data: bytes) -> Image.Image:
    width, height = dimensions_for_size(len(data))
    pixels = bytearray(width * height)
    source = 0
    tiles_wide = width // 8
    for tile in range(len(data) // 32):
        tile_x = (tile % tiles_wide) * 8
        tile_y = (tile // tiles_wide) * 8
        for y in range(8):
            for x_pair in range(4):
                packed = data[source]
                source += 1
                target = (tile_y + y) * width + tile_x + x_pair * 2
                pixels[target] = packed & 0xF
                pixels[target + 1] = packed >> 4
    image = Image.frombytes("P", (width, height), bytes(pixels))
    palette = []
    for value in range(16):
        shade = value * 17
        palette.extend((shade, shade, shade))
    image.putpalette(palette + [0] * (768 - len(palette)))
    image.info["transparency"] = 0
    return image


def image_to_slot(path: Path, expected_size: int) -> bytes:
    expected_dimensions = dimensions_for_size(expected_size)
    with Image.open(path) as image:
        if image.size != expected_dimensions:
            raise ValueError(
                f"{path.name}: expected {expected_dimensions[0]}x{expected_dimensions[1]}, got "
                f"{image.width}x{image.height}"
            )
        if image.mode != "P":
            raise ValueError(f"{path.name}: image must use an indexed palette (mode P)")
        pixels = bytes(image.getdata())
    if pixels and max(pixels) > 15:
        raise ValueError(f"{path.name}: image uses palette indexes above 15")

    output = bytearray()
    width, height = expected_dimensions
    tiles_wide = width // 8
    tiles_high = height // 8
    for tile_y in range(tiles_high):
        for tile_x in range(tiles_wide):
            for y in range(8):
                row = (tile_y * 8 + y) * width + tile_x * 8
                for x in range(0, 8, 2):
                    output.append(pixels[row + x] | (pixels[row + x + 1] << 4))
    return bytes(output)


def read_normal_palettes() -> dict[str, list[int]]:
    text = NORMAL_PALETTES.read_text(encoding="utf-8")
    palettes = {}
    pattern = re.compile(
        r"^\.global\s+(gFollowingMonPic_[A-Za-z0-9_]+NormalPal)\s*\n"
        r"\1:\s*\n((?:\s*\.hword[^\n]*\n)+)",
        flags=re.MULTILINE,
    )
    for match in pattern.finditer(text):
        values = [int(value, 16) for value in re.findall(r"0x[0-9A-Fa-f]{1,4}", match.group(2))]
        if len(values) != 16:
            raise ValueError(f"{match.group(1)} does not contain exactly 16 colors")
        palettes[match.group(1)] = values
    return palettes


def sheet_palette_symbols() -> dict[str, str]:
    follower_text = FOLLOWER_TABLE.read_text(encoding="utf-8")
    name_to_tag = {
        match.group(2): match.group(1).strip()
        for match in re.finditer(
            r"MON_OW_OBJECT_GRAPHICS(?:_64)?\(\s*([^,]+),\s*([A-Za-z0-9_]+)\s*\)",
            follower_text,
        )
    }
    palette_text = PALETTE_TABLE.read_text(encoding="utf-8")
    tag_to_palette = {
        match.group(2): match.group(1)
        for match in re.finditer(
            r"\{\s*(gFollowingMonPic_[A-Za-z0-9_]+NormalPal)\s*,\s*([^}\s,]+)",
            palette_text,
        )
    }
    return {name: tag_to_palette[tag] for name, tag in name_to_tag.items() if tag in tag_to_palette}


def gba_palette_to_rgb(values: list[int]) -> list[int]:
    palette = []
    for color in values:
        red = (color & 0x1F) * 255 // 31
        green = ((color >> 5) & 0x1F) * 255 // 31
        blue = ((color >> 10) & 0x1F) * 255 // 31
        palette.extend((red, green, blue))
    return palette + [0] * (768 - len(palette))


def apply_normal_palettes() -> None:
    palettes = read_normal_palettes()
    sheet_palettes = sheet_palette_symbols()
    applied = 0
    for line in MANIFEST.read_text(encoding="ascii").splitlines():
        if not line or line.startswith("#"):
            continue
        _, _, _, symbol, filename, _ = line.split()
        match = re.fullmatch(r"gFollowingMonPic_[0-9]+_(.+)Tiles", symbol)
        if not match:
            raise ValueError(f"invalid follower-mon tile symbol: {symbol}")
        sheet_name = match.group(1)
        palette_symbol = sheet_palettes.get(sheet_name)
        if sheet_name == "DLugia":
            palette_symbol = "gFollowingMonPic_LugiaNormalPal"
        if palette_symbol not in palettes:
            raise ValueError(f"normal palette not found for {symbol}")
        path = SPRITE_DIR / filename
        with Image.open(path) as source:
            image = source.copy()
        image.putpalette(gba_palette_to_rgb(palettes[palette_symbol]))
        image.info["transparency"] = 0
        image.save(path)
        applied += 1
    print(f"Applied normal palettes to {applied} follower-mon sprite sheets")


def extract() -> None:
    data = read_source_bytes()
    symbols = read_symbols()
    SPRITE_DIR.mkdir(exist_ok=True)
    manifest_lines = ["# offset span image_size symbol png padding"]
    for index, (offset, symbol) in enumerate(symbols):
        end = symbols[index + 1][0] if index + 1 < len(symbols) else len(data)
        span = end - offset
        image_size = 0x3000 if span == 0x3000 else 0xC00
        if span not in (0xC00, 0x1000, 0x3000):
            raise ValueError(f"unsupported follower-mon sheet span: {span:#x}")
        filename = symbol.removesuffix("Tiles") + ".png"
        slot_to_image(data[offset:offset + image_size]).save(SPRITE_DIR / filename)
        padding_name = "-"
        if span > image_size:
            padding_name = symbol.removesuffix("Tiles") + ".padding.bin"
            (SPRITE_DIR / padding_name).write_bytes(data[offset + image_size:end])
        manifest_lines.append(
            f"{offset:#08x} {span:#06x} {image_size:#06x} "
            f"{symbol} {filename} {padding_name}"
        )
    MANIFEST.write_text("\n".join(manifest_lines) + "\n", encoding="ascii")
    apply_normal_palettes()
    print(f"Extracted {len(symbols)} follower-mon sprite sheets to {SPRITE_DIR}")


def collect_pngs() -> list[tuple[int, int, int, str, Path, Path | None]]:
    entries = []
    for line in MANIFEST.read_text(encoding="ascii").splitlines():
        if not line or line.startswith("#"):
            continue
        offset, span, image_size, symbol, filename, padding = line.split()
        padding_path = None if padding == "-" else SPRITE_DIR / padding
        entries.append(
            (int(offset, 0), int(span, 0), int(image_size, 0), symbol,
             SPRITE_DIR / filename, padding_path)
        )
    if not entries:
        raise ValueError(f"no entries found in {MANIFEST}")
    for index, entry in enumerate(entries[:-1]):
        if entry[0] + entry[1] != entries[index + 1][0]:
            raise ValueError("manifest entries must be contiguous and ordered")
    return entries


def build(binary_path: Path, asm_path: Path, rom_offset: int) -> None:
    entries = collect_pngs()
    binary_path.parent.mkdir(parents=True, exist_ok=True)
    chunks = []
    for _, span, image_size, _, path, padding_path in entries:
        chunk = image_to_slot(path, image_size)
        if padding_path is not None:
            chunk += padding_path.read_bytes()
        if len(chunk) != span:
            raise ValueError(f"{path.name}: compiled data does not match manifest span")
        chunks.append(chunk)
    binary_path.write_bytes(b"".join(chunks))

    lines = [".section .rodata", ".align 2"]
    for offset, _, _, symbol, _, _ in entries:
        if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", symbol):
            raise ValueError(f"invalid symbol derived from sprite filename: {symbol}")
        lines.extend((f".global {symbol}", f".set {symbol}, {0x08000000 + rom_offset + offset:#010x}"))
    asm_path.write_text("\n".join(lines) + "\n", encoding="ascii")
    print(f"Built {len(entries)} follower-mon sprite sheets ({binary_path.stat().st_size} bytes)")


def migrate_sources() -> None:
    header_text = HEADER.read_text(encoding="utf-8")
    converted = 0

    def replace_define(match: re.Match[str]) -> str:
        nonlocal converted
        converted += 1
        return f"extern const u8 {match.group(1)}[];"

    header_text = re.sub(
        r"^#define\s+(gFollowingMonPic_[A-Za-z0-9_]+Tiles)\s+"
        r"\(\(u8\*\)0x[0-9A-Fa-f]+\)\s*$",
        replace_define,
        header_text,
        flags=re.MULTILINE,
    )
    if converted == 0:
        raise ValueError("no follower-mon address defines were found to migrate")
    HEADER.write_text(header_text, encoding="utf-8")
    SOURCE.write_text(
        "# Free Byte Replacements is a custom file\n"
        "# It adds specific bytes at free space in rom\n"
        "# Follower-mon graphics are now built from follower_mon_sprites/.\n",
        encoding="ascii",
    )
    print(f"Converted {converted} tile defines to extern declarations and cleaned {SOURCE.name}")


def main() -> int:
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(dest="command", required=True)
    subparsers.add_parser("extract")
    subparsers.add_parser("migrate")
    subparsers.add_parser("apply-palettes")
    build_parser = subparsers.add_parser("build")
    build_parser.add_argument("binary", type=Path)
    build_parser.add_argument("assembly", type=Path)
    build_parser.add_argument("offset", type=lambda value: int(value, 0))
    args = parser.parse_args()
    if args.command == "extract":
        extract()
    elif args.command == "migrate":
        migrate_sources()
    elif args.command == "apply-palettes":
        apply_normal_palettes()
    else:
        build(args.binary, args.assembly, args.offset)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
