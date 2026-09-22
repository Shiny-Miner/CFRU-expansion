#!/usr/bin/env python3
"""Editor for the overworld tables used by gOverworldTableSwitcher.

Table 0 is read from and written directly to the ROM. The other tables are
edited in the NPCPtr array bodies in src/character_customization.c.

The interface organization is inspired by kimwnasptd's OWM-Qt. This is an
independent implementation for the CFRU source/build workflow; no OWM-Qt code
is incorporated here (the referenced repository declares no reuse license).
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import struct
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Optional


ROM_POINTER_MIN = 0x08000000
ROM_POINTER_MAX = 0x0A000000
MAX_TABLE_ENTRIES = 240
# The vanilla BPRE0 table contains IDs 0..151. The following pointers belong
# to an adjacent SpriteTemplate table. Because they also point into the ROM,
# looking only for the first invalid pointer would expose non-overworld data.
ROM_TABLE_ENTRY_COUNT = 152
PALETTE_ID_CHOICES = ("Automatic",) + tuple(f"0x{tag:04X}" for tag in range(0x1100, 0x11FF))
ANIMATION_TABLE_CHOICES = {
    "Automatic": None,
    **{f"AnimTable_{name}": f"((const union AnimCmd *const *) 0x{address:08X})"
       for name, address in (
           ("PlayerNormal", 0x83A3470), ("Standard", 0x83A3368),
           ("Surfing", 0x83A3584), ("FieldMove", 0x83A3638),
           ("Fishing", 0x83A3668), ("VsSeekerBike", 0x83A3640),
           ("CutTree", 0x83A3660), ("RockSmash", 0x83A3658),
           ("Boulder", 0x83A3314),
       )},
    "AnimTable_FollowerMon": "gFollowerMonAnimTable",
}


class EditorError(RuntimeError):
    pass


@dataclass
class Entry:
    index: int
    value: str
    name: str = ""


@dataclass
class SourceArray:
    name: str
    body_start: int
    body_end: int
    entries: list[Entry]
    value_spans: list[tuple[int, int]]


@dataclass
class SpriteInfo:
    width: int
    height: int
    frames: int
    palette_tag: str
    data_address: str
    frames_address: str
    png_path: Optional[Path] = None
    rom_frames: Optional[list[tuple[bytes, list[tuple[int, int, int, int]]]]] = None


@dataclass
class PendingSprite:
    mode: str
    name: str
    width: int
    height: int
    frames: int
    sheet: Optional[Path]
    target_table: int = -1
    target_index: int = -1
    animation_table: Optional[str] = None
    palette_id: Optional[int] = None


def read_text_preserving(path: Path) -> tuple[str, str]:
    raw = path.read_bytes()
    newline = "\r\n" if b"\r\n" in raw else "\n"
    try:
        return raw.decode("utf-8"), newline
    except UnicodeDecodeError as exc:
        raise EditorError(f"Could not read {path} as UTF-8: {exc}") from exc


def find_matching_brace(text: str, opening: int) -> int:
    depth = 0
    state = "code"
    i = opening
    while i < len(text):
        pair = text[i : i + 2]
        char = text[i]
        if state == "code":
            if pair == "//":
                state = "line_comment"
                i += 2
                continue
            if pair == "/*":
                state = "block_comment"
                i += 2
                continue
            if char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return i
        elif state == "line_comment" and char in "\r\n":
            state = "code"
        elif state == "block_comment" and pair == "*/":
            state = "code"
            i += 2
            continue
        elif state in ("string", "char"):
            if char == "\\":
                i += 2
                continue
            if (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        i += 1
    raise EditorError("Closing brace not found in the C file")


def locate_array(text: str, name: str) -> SourceArray:
    declaration = re.search(
        rf"\b(?:static\s+)?NPCPtr\s+{re.escape(name)}\s*\[\s*\]\s*=\s*\{{",
        text,
    )
    if not declaration:
        raise EditorError(f"Array {name} was not found in character_customization.c")
    opening = text.find("{", declaration.start())
    closing = find_matching_brace(text, opening)
    body_start, body_end = opening + 1, closing
    body = text[body_start:body_end]
    entries: list[Entry] = []
    spans: list[tuple[int, int]] = []
    # Os arrays do projeto usam uma expressao por linha, seguida por virgula.
    pattern = re.compile(r"(?m)^[ \t]*([^,\r\n]+?)(?=[ \t]*,)[ \t]*,")
    for match in pattern.finditer(body):
        value = match.group(1).strip()
        if not value or value.startswith(("//", "/*", "#")):
            continue
        start = body_start + match.start(1)
        end = body_start + match.end(1)
        entries.append(Entry(len(entries), value))
        spans.append((start, end))
    if not entries:
        raise EditorError(f"Array {name} has no recognizable entries")
    return SourceArray(name, body_start, body_end, entries, spans)


def parse_switcher_configuration(text: str) -> tuple[int, list[str]]:
    matches = list(
        re.finditer(
            r"\bgOverworldTableSwitcher\s*\[\s*255\s*\]\s*=\s*\{",
            text,
        )
    )
    if not matches:
        raise EditorError("gOverworldTableSwitcher[255] was not found")
    # A ultima declaracao e a configuracao padrao do bloco #else.
    match = matches[-1]
    opening = text.find("{", match.start())
    closing = find_matching_brace(text, opening)
    body = re.sub(r"//[^\r\n]*|/\*.*?\*/", "", text[opening + 1 : closing], flags=re.S)
    values = [part.strip() for part in body.split(",") if part.strip()]
    if not values:
        raise EditorError("gOverworldTableSwitcher has no table entries")
    address_match = re.search(r"0[xX]([0-9A-Fa-f]+)", values[0])
    if not address_match:
        raise EditorError(
            "The first gOverworldTableSwitcher entry must contain a hexadecimal ROM address"
        )
    address = int(address_match.group(0), 16)
    if not ROM_POINTER_MIN <= address < ROM_POINTER_MAX or address & 3:
        raise EditorError(
            f"The first gOverworldTableSwitcher address is invalid: 0x{address:08X}"
        )
    arrays = [value for value in values[1:] if re.fullmatch(r"sOverworldTable\d+", value)]
    return address, arrays


def parse_switcher_arrays(text: str) -> list[str]:
    return parse_switcher_configuration(text)[1]


def read_rom_entries(path: Path, gba_address: int) -> list[Entry]:
    file_offset = gba_address - ROM_POINTER_MIN
    required_size = file_offset + ROM_TABLE_ENTRY_COUNT * 4
    size = path.stat().st_size
    if size < required_size:
        raise EditorError(f"ROM is too small for address 0x{gba_address:08X}")
    entries: list[Entry] = []
    with path.open("rb") as rom:
        rom.seek(file_offset)
        for index in range(ROM_TABLE_ENTRY_COUNT):
            raw = rom.read(4)
            value = struct.unpack("<I", raw)[0]
            if value != 0 and not ROM_POINTER_MIN <= value < ROM_POINTER_MAX:
                raise EditorError(
                    f"Invalid pointer in table 0, ID {index}: 0x{value:08X}"
                )
            entries.append(Entry(index, f"0x{value:08X}" if value else "NULL"))
    return entries


def parse_int_pointer(value: str) -> int:
    cleaned = value.strip()
    if cleaned.upper() in ("NULL", "0"):
        return 0
    try:
        pointer = int(cleaned, 0)
    except ValueError as exc:
        raise EditorError("Use a hexadecimal address (for example, 0x083A3BB0) or NULL") from exc
    if not ROM_POINTER_MIN <= pointer < ROM_POINTER_MAX:
        raise EditorError("The pointer must be between 0x08000000 and 0x09FFFFFF")
    if pointer & 3:
        raise EditorError("The pointer must be aligned to 4 bytes")
    return pointer


def validate_c_expression(value: str) -> str:
    value = value.strip()
    if not value:
        raise EditorError("The C expression cannot be empty")
    if any(token in value for token in ("\n", "\r", ",", ";", "{", "}")):
        raise EditorError("The entry must be a single C expression without a comma or semicolon")
    if not re.fullmatch(r"[&*()\[\].>+\-\w\s]+", value):
        raise EditorError("The expression contains unsupported characters")
    return value


def make_backup(path: Path) -> Path:
    backup = path.with_name(path.name + ".ow_editor.bak")
    if not backup.exists():
        shutil.copy2(path, backup)
    return backup


class ProjectModel:
    def __init__(self, root: Path):
        self.root = root.resolve()
        self.rom_path = self.root / "BPRE0.gba"
        self.c_path = self.root / "src" / "character_customization.c"
        self.names_path = self.root / "include" / "constants" / "event_objects.h"
        self.source_text = ""
        self.newline = "\n"
        self.rom_gba_address = 0
        self.rom_file_offset = 0
        self.array_names: list[str] = []
        self.tables: list[list[Entry]] = []
        self.names: dict[int, str] = {}
        self.follower_sprites: dict[tuple[str, int], tuple[str, int, Path]] = {}
        self.rom_data = b""
        self.rom_image_tables: list[int] = []
        self.custom_sprites: dict[str, dict] = {}
        self.custom_overrides: dict[tuple[int, int], dict] = {}
        self.dirty: dict[int, dict[int, str]] = {}
        self.reload()

    def reload(self) -> None:
        for path in (self.rom_path, self.c_path):
            if not path.is_file():
                raise EditorError(f"Required file not found: {path}")
        self.source_text, self.newline = read_text_preserving(self.c_path)
        self.rom_data = self.rom_path.read_bytes()
        self.rom_gba_address, self.array_names = parse_switcher_configuration(self.source_text)
        self.rom_file_offset = self.rom_gba_address - ROM_POINTER_MIN
        self.names = self._load_names()
        self.follower_sprites = self._load_follower_sprites()
        custom_path = self.root / "overworld_editor.json"
        custom_items = json.loads(custom_path.read_text(encoding="utf-8")) if custom_path.is_file() else []
        self.custom_sprites = {item["name"]: item for item in custom_items}
        self.custom_overrides = {}
        for item in custom_items:
            if item.get("mode") == "resize" and item.get("target_table", -1) >= 0:
                self.custom_overrides[(item["target_table"], item["target_index"])] = item
        # Compatibility with older replacements that recorded their table 0
        # target only in the repoints file.
        repoints_path = self.root / "repoints"
        if repoints_path.is_file():
            repoints_text = repoints_path.read_text(encoding="utf-8")
            for name, address_text in re.findall(r"(?m)^gOverworldEditorGfx_([A-Za-z0-9_]+)\s+([0-9A-Fa-f]{8})\s*$", repoints_text):
                address = int(address_text, 16)
                delta = address - self.rom_gba_address
                if delta >= 0 and delta % 4 == 0 and delta // 4 < ROM_TABLE_ENTRY_COUNT and name in self.custom_sprites:
                    self.custom_overrides[(0, delta // 4)] = self.custom_sprites[name]
        self.tables = [read_rom_entries(self.rom_path, self.rom_gba_address)]
        for array_name in self.array_names:
            self.tables.append(locate_array(self.source_text, array_name).entries)
        for table in self.tables:
            for entry in table:
                entry.name = self.names.get(entry.index, f"ID {entry.index}")
        self.rom_image_tables = []
        for entry in self.tables[0]:
            pointer = parse_int_pointer(entry.value)
            if pointer:
                offset = self._gba_offset(pointer, len(self.rom_data), "EventObjectGraphicsInfo")
                self.rom_image_tables.append(struct.unpack_from("<I", self.rom_data, offset + 0x1C)[0])
        for table_index in range(1, len(self.tables)):
            for entry in self.tables[table_index]:
                match = re.fullmatch(r"&?(gFollowerMonGfxTable\d+)\s*\[\s*(\d+)\s*\]", entry.value)
                if match:
                    sprite = self.follower_sprites.get((match.group(1), int(match.group(2))))
                    if sprite:
                        entry.name = sprite[0]
                custom = re.fullmatch(r"&?gOverworldEditorGfx_([A-Za-z0-9_]+)", entry.value)
                if custom and custom.group(1) in self.custom_sprites:
                    entry.name = custom.group(1)
        for (table_index, entry_index), item in self.custom_overrides.items():
            if table_index < len(self.tables) and entry_index < len(self.tables[table_index]):
                self.tables[table_index][entry_index].name = item["name"]
        self.dirty.clear()

    def _load_follower_sprites(self) -> dict[tuple[str, int], tuple[str, int, Path]]:
        path = self.root / "src" / "Tables" / "follower_mon_sprites.c"
        if not path.is_file():
            return {}
        text, _ = read_text_preserving(path)
        result: dict[tuple[str, int], tuple[str, int, Path]] = {}
        declaration = re.compile(
            r"const\s+struct\s+EventObjectGraphicsInfo\s+(gFollowerMonGfxTable\d+)\s*\[\s*\]\s*=\s*\{"
        )
        for match in declaration.finditer(text):
            table_name = match.group(1)
            opening = text.find("{", match.start())
            closing = find_matching_brace(text, opening)
            body = text[opening + 1 : closing]
            macros = re.findall(
                r"MON_OW_OBJECT_GRAPHICS(_64)?\s*\(\s*[^,]+,\s*([A-Za-z0-9_]+)\s*\)",
                body,
            )
            for index, (large, name) in enumerate(macros):
                candidates = sorted((self.root / "follower_mon_sprites").glob(f"gFollowingMonPic_*_{name}.png"))
                png = candidates[0] if candidates else self.root / "follower_mon_sprites" / f"{name}.png"
                result[(table_name, index)] = (name, 64 if large else 32, png)
        return result

    @staticmethod
    def _gba_offset(address: int, rom_size: int, label: str) -> int:
        offset = address - 0x08000000
        if address < 0x08000000 or offset < 0 or offset >= rom_size:
            raise EditorError(f"{label} is outside the ROM: 0x{address:08X}")
        return offset

    def _palette_from_rom(self, rom: bytes, tag: int) -> list[tuple[int, int, int, int]]:
        # Vanilla BPRE0 SpritePalette table. Each item is {pointer, tag, padding}.
        palette_table = 0x3A5158
        for offset in range(palette_table, palette_table + 0x90, 8):
            pointer, found_tag, _padding = struct.unpack_from("<IHH", rom, offset)
            if pointer == 0:
                break
            if found_tag == tag:
                pal_offset = self._gba_offset(pointer, len(rom), "Palette")
                colours = []
                for value, in struct.iter_unpack("<H", rom[pal_offset : pal_offset + 32]):
                    r = (value & 31) * 255 // 31
                    g = ((value >> 5) & 31) * 255 // 31
                    b = ((value >> 10) & 31) * 255 // 31
                    colours.append((r, g, b, 0 if not colours else 255))
                return colours
        # A neutral palette keeps a missing tag visible without blocking preview.
        return [(0, 0, 0, 0)] + [(i * 17, i * 17, i * 17, 255) for i in range(1, 16)]

    def sprite_info(self, table: int, index: int) -> Optional[SpriteInfo]:
        value = self.dirty.get(table, {}).get(index, self.tables[table][index].value)
        override = self.custom_overrides.get((table, index))
        if override:
            png = self.root / override["sheet"]
            palette_tag = override.get("palette_tag")
            palette_text = f"0x{palette_tag:04X}" if palette_tag is not None else "gObjectEventSpritePalettes11"
            return SpriteInfo(override["width"], override["height"], override["frames"], palette_text,
                              str(png), "spritesheet PNG (replacement)", png_path=png)
        if table:
            custom = re.fullmatch(r"&?gOverworldEditorGfx_([A-Za-z0-9_]+)", value)
            if custom and custom.group(1) in self.custom_sprites:
                item = self.custom_sprites[custom.group(1)]
                png = self.root / item["sheet"]
                palette_tag = item.get("palette_tag")
                palette_text = f"0x{palette_tag:04X}" if palette_tag is not None else "gObjectEventSpritePalettes11"
                return SpriteInfo(item["width"], item["height"], item["frames"], palette_text,
                                  str(png), "spritesheet PNG", png_path=png)
            match = re.fullmatch(r"&?(gFollowerMonGfxTable\d+)\s*\[\s*(\d+)\s*\]", value)
            if not match:
                return None
            sprite = self.follower_sprites.get((match.group(1), int(match.group(2))))
            if not sprite:
                return None
            name, size, png = sprite
            frames = 1
            if png.is_file():
                try:
                    from PIL import Image
                    with Image.open(png) as image:
                        frames = max(1, image.height // size)
                except Exception:
                    pass
            return SpriteInfo(size, size, frames, "defined in project", str(png), "PNG spritesheet", png_path=png)

        pointer = parse_int_pointer(value)
        if pointer == 0:
            return None
        rom = self.rom_data
        info_offset = self._gba_offset(pointer, len(rom), "EventObjectGraphicsInfo")
        fields = struct.unpack_from("<HHHHhhBBBBIIIII", rom, info_offset)
        _tile, palette1, _palette2, size, width, height = fields[:6]
        images_address = fields[-2]
        frames_offset = self._gba_offset(images_address, len(rom), "Frame table")
        next_tables = [address for address in self.rom_image_tables if address > images_address]
        frame_count = min(32, ((min(next_tables) - images_address) // 8) if next_tables else 1)
        frame_count = max(1, frame_count)
        palette = self._palette_from_rom(rom, palette1)
        rom_frames = []
        first_data_address = 0
        expected = max(1, width * height // 2)
        for frame in range(frame_count):
            data_address, frame_size = struct.unpack_from("<II", rom, frames_offset + frame * 8)
            try:
                data_offset = self._gba_offset(data_address, len(rom), "Graphics data")
            except EditorError:
                break
            if not first_data_address:
                first_data_address = data_address
            raw = rom[data_offset : data_offset + min(frame_size, expected)]
            rom_frames.append((raw.ljust(expected, b"\0"), palette))
        return SpriteInfo(
            width, height, len(rom_frames), f"0x{palette1:04X}", f"0x{first_data_address:08X}",
            f"0x{images_address:08X}", rom_frames=rom_frames
        )

    def _load_names(self) -> dict[int, str]:
        if not self.names_path.is_file():
            return {}
        text, _ = read_text_preserving(self.names_path)
        enum = re.search(r"enum\s+OverworldNPCSprites\s*\{(.*?)\}", text, re.S)
        result: dict[int, str] = {}
        current = 0
        if enum:
            clean = re.sub(r"//[^\r\n]*|/\*.*?\*/", "", enum.group(1), flags=re.S)
            for item in clean.split(","):
                item = item.strip()
                if not item:
                    continue
                parts = item.split("=", 1)
                if len(parts) == 2:
                    try:
                        current = int(parts[1].strip(), 0)
                    except ValueError:
                        continue
                result[current] = parts[0].strip()
                current += 1
        for name, value in re.findall(r"(?m)^#define\s+(EVENT_OBJ_GFX_\w+)\s+(\d+)\b", text):
            result.setdefault(int(value), name)
        return result

    def table_label(self, index: int) -> str:
        if index == 0:
            return f"Table 0 - ROM 0x{self.rom_gba_address:08X} ({len(self.tables[0])} entries)"
        return f"Table {index} - {self.array_names[index - 1]} ({len(self.tables[index])} entries)"

    def set_value(self, table: int, index: int, value: str) -> str:
        if table == 0:
            pointer = parse_int_pointer(value)
            normalized = f"0x{pointer:08X}" if pointer else "NULL"
        else:
            normalized = validate_c_expression(value)
        original = self.tables[table][index].value
        changes = self.dirty.setdefault(table, {})
        if normalized == original:
            changes.pop(index, None)
            if not changes:
                self.dirty.pop(table, None)
        else:
            changes[index] = normalized
        return normalized

    def sprite_animation_table(self, item: dict) -> str:
        """Keep the target's frame layout when replacing an existing sprite."""
        if item.get("animation_table"):
            return item["animation_table"]
        if item.get("mode") == "resize":
            table, index = item["target_table"], item["target_index"]
            previous = self.custom_overrides.get((table, index), {})
            if previous.get("animation_table"):
                return previous["animation_table"]
            value = self.dirty.get(table, {}).get(index, self.tables[table][index].value)
            custom = re.fullmatch(r"&?gOverworldEditorGfx_([A-Za-z0-9_]+)", value)
            if custom:
                previous = self.custom_sprites.get(custom.group(1), {})
                if previous.get("animation_table"):
                    return previous["animation_table"]
            if re.fullmatch(r"0[xX][0-9A-Fa-f]+", value):
                offset = self._gba_offset(int(value, 16), len(self.rom_data), "EventObjectGraphicsInfo")
                address = struct.unpack_from("<I", self.rom_data, offset + 0x18)[0]
                self._gba_offset(address, len(self.rom_data), "Animation table")
                return f"((const union AnimCmd *const *) 0x{address:08X})"
        return "gFollowerMonAnimTable"

    def install_sprites(self, pending: list[PendingSprite]) -> None:
        if not pending:
            return
        try:
            from PIL import Image
        except ImportError as exc:
            raise EditorError("Pillow is required to add spritesheets") from exc
        if any(item.sheet is None for item in pending):
            raise EditorError("Every Add/Resize operation requires a spritesheet before saving")
        metadata_path = self.root / "overworld_editor.json"
        metadata = []
        if metadata_path.is_file():
            metadata = json.loads(metadata_path.read_text(encoding="utf-8"))
        used = {item["name"] for item in metadata}
        graphics_dir = self.root / "graphics" / "overworld_editor"
        graphics_dir.mkdir(parents=True, exist_ok=True)
        (graphics_dir / "gritflags.txt").write_text("-gB4 -fts -fh -pe16 -gu8\n", encoding="utf-8")
        for item in pending:
            clean = re.sub(r"\W+", "_", item.name).strip("_")
            if not clean or clean in used:
                raise EditorError(f"Empty or duplicate overworld name: {item.name}")
            assert item.sheet is not None
            with Image.open(item.sheet) as image:
                if image.mode != "P":
                    raise EditorError(f"{item.sheet.name} must use indexed mode (P)")
                vertical = image.width == item.width and image.height == item.height * item.frames
                horizontal = image.height == item.height and image.width == item.width * item.frames
                if not (vertical or horizontal):
                    raise EditorError(
                        f"{item.sheet.name}: expected {item.width}x{item.height * item.frames} "
                        f"or {item.width * item.frames}x{item.height}"
                    )
                if max(image.getdata(), default=0) > 15:
                    raise EditorError(f"{item.sheet.name} uses more than 16 color indexes")
                destination = graphics_dir / f"gOverworldEditor_{clean}.png"
                if horizontal:
                    vertical_image = Image.new("P", (item.width, item.height * item.frames))
                    vertical_image.putpalette(image.getpalette())
                    for frame in range(item.frames):
                        vertical_image.paste(image.crop((frame * item.width, 0, (frame + 1) * item.width, item.height)), (0, frame * item.height))
                    vertical_image.save(destination)
                else:
                    shutil.copy2(item.sheet, destination)
            metadata.append({
                "name": clean, "width": item.width, "height": item.height,
                "frames": item.frames, "sheet": str(destination.relative_to(self.root)),
                "table_entry": item.mode == "add",
                "mode": item.mode, "target_table": item.target_table,
                "target_index": item.target_index,
                "animation_table": item.animation_table,
                "palette_id": item.palette_id,
            })
            used.add(clean)

        character_text, _ = read_text_preserving(self.c_path)
        palette_decl = re.search(r"gObjectEventSpritePalettes11\s*\[\s*\]\s*=\s*\{", character_text)
        if not palette_decl:
            raise EditorError("gObjectEventSpritePalettes11 was not found")
        palette_open = character_text.find("{", palette_decl.start())
        palette_close = find_matching_brace(character_text, palette_open)
        palette_body = character_text[palette_open + 1 : palette_close]
        base_body = "\n".join(line for line in palette_body.splitlines() if "gOverworldEditor_" not in line)
        occupied = {int(tag, 16) for tag in re.findall(r"0x(11[0-9A-Fa-f]{2})\b", base_body) if int(tag, 16) != 0x11FF}
        next_tag = max(occupied, default=0x10FF) + 1
        reserved = occupied | {item.get("palette_id") for item in metadata} | {item.get("palette_tag") for item in metadata}
        registered = set(occupied)
        for item in metadata:
            tag = item.get("palette_id")
            if tag is None:
                tag = item.get("palette_tag")
            if tag is None:
                while next_tag in reserved:
                    next_tag += 1
                tag = next_tag
                next_tag += 1
            if not 0x1100 <= tag < 0x11FF:
                raise EditorError("Palette ID must be between 0x1100 and 0x11FE; 0x11FF is reserved for the terminator")
            item["palette_tag"] = tag
            item["register_palette"] = tag not in registered
            registered.add(tag)
            reserved.add(tag)

        header = ["#pragma once", "", '#include "../global.h"', ""]
        source = ['#include "../defines.h"', '#include "../../include/follower_mon_sprites.h"',
                  '#include "../../include/new/overworld_editor_sprites.h"', ""]
        pointers = []
        for item in metadata:
            item["animation_table"] = self.sprite_animation_table(item)
            name=item["name"]; width=item["width"]; height=item["height"]; frames=item["frames"]
            symbol=f"gOverworldEditor_{name}"; pal=item["palette_tag"]
            header += [f"extern const u8 {symbol}Tiles[];", f"extern const u16 {symbol}Pal[];",
                       f"extern const struct EventObjectGraphicsInfo gOverworldEditorGfx_{name};"]
            source += [f"static const struct SpriteFrameImage sOverworldEditorFrames_{name}[] =", "{"]
            for frame in range(frames):
                source.append(f"    overworld_frame({symbol}Tiles, {width // 8}, {height // 8}, {frame}),")
            source += ["};", f"const struct EventObjectGraphicsInfo gOverworldEditorGfx_{name} =", "{",
                "    .tileTag = 0xFFFF,", f"    .paletteTag1 = 0x{pal:04X},",
                "    .paletteTag2 = EVENT_OBJ_PAL_TAG_NONE,", f"    .size = ({width} * {height}) / 2,",
                f"    .width = {width},", f"    .height = {height},", "    .shadowSize = SHADOW_SIZE_M,",
                "    .inanimate = FALSE,", "    .disableReflectionPaletteLoad = FALSE,", "    .tracks = TRACKS_FOOT,",
                "    .gender = MALE,", f"    .oam = gEventObjectBaseOam_{width}x{height},",
                f"    .subspriteTables = gEventObjectSpriteOamTables_{width}x{height},",
                f"    .anims = {item['animation_table']},", f"    .images = sOverworldEditorFrames_{name},",
                "    .affineAnims = gDummySpriteAffineAnimTable,", "};", ""]
            if item.get("table_entry", True):
                pointers.append(f"    &gOverworldEditorGfx_{name},")
        header += [""]
        generated_h=self.root/"include/new/overworld_editor_sprites.h"
        generated_c=self.root/"src/Tables/overworld_editor_sprites.c"
        generated_h.write_text("\n".join(header),encoding="utf-8")
        generated_c.write_text("\n".join(source),encoding="utf-8")
        metadata_path.write_text(json.dumps(metadata,indent=2)+"\n",encoding="utf-8")

        text=character_text
        include='#include "../include/new/overworld_editor_sprites.h"'
        if include not in text:
            anchor='#include "../include/new/character_customization.h"'
            text=text.replace(anchor,anchor+"\n"+include,1)
        if pointers:
            array_text="static NPCPtr sOverworldTable7[] =\n{\n"+"\n".join(pointers)+"\n};\n\n"
            existing=re.search(r"static NPCPtr sOverworldTable7\s*\[\s*\]\s*=\s*\{",text)
            if existing:
                opening=text.find("{",existing.start()); closing=find_matching_brace(text,opening)
                text=text[:existing.start()]+array_text.rstrip()+text[closing+2:]
            else:
                marker="\t// create 255 OW tables"
                text=text.replace(marker,array_text+marker,1)
            switch=list(re.finditer(r"\bgOverworldTableSwitcher\s*\[\s*255\s*\]\s*=\s*\{",text))[-1]
            opening=text.find("{",switch.start()); closing=find_matching_brace(text,opening)
            body=text[opening+1:closing]
            if "sOverworldTable7" not in body:
                pos=body.find("\t\t// etc...")
                body=body[:pos]+"\t\tsOverworldTable7,\n"+body[pos:]
                text=text[:opening+1]+body+text[closing:]
        text=re.sub(r"(?m)^\s*\{gOverworldEditor_[A-Za-z0-9_]+Pal,\s*0x[0-9A-Fa-f]+\},\s*\n?", "", text)
        palette_decl = re.search(r"gObjectEventSpritePalettes11\s*\[\s*\]\s*=\s*\{", text)
        palette_open = text.find("{", palette_decl.start())
        palette_close = find_matching_brace(text, palette_open)
        palette_body = text[palette_open + 1 : palette_close]
        generated_palettes = "".join(
            f"\t\t{{gOverworldEditor_{item['name']}Pal, 0x{item['palette_tag']:04X}}},\n" for item in metadata
            if item["register_palette"]
        )
        null_match = re.search(r"(?m)^\s*\{NULL,\s*0x11FF\}\s*,?", palette_body)
        if not null_match:
            raise EditorError("gObjectEventSpritePalettes11 terminator was not found")
        insertion = palette_open + 1 + null_match.start()
        text = text[:insertion] + generated_palettes + text[insertion:]
        text = re.sub(r"(?m)^\s*\[0x17\]\s*=\s*gOverworldEditorPalettes,\s*\n?", "", text)
        repoints=self.root/"repoints"
        repoint_text=repoints.read_text(encoding="utf-8") if repoints.is_file() else ""
        new_items=metadata[-len(pending):]
        for request,item in zip(pending,new_items):
            if request.mode=="resize" and request.target_table==0:
                line=f"gOverworldEditorGfx_{item['name']} {self.rom_gba_address + request.target_index * 4:08X}"
                if line not in repoint_text: repoint_text += "\n"+line
            elif request.mode=="resize":
                array=locate_array(text,self.array_names[request.target_table-1])
                start,end=array.value_spans[request.target_index]
                text=text[:start]+f"&gOverworldEditorGfx_{item['name']}"+text[end:]
        if repoint_text:
            make_backup(repoints); repoints.write_text(repoint_text.rstrip()+"\n",encoding="utf-8")
        make_backup(self.c_path); self.c_path.write_text(text,encoding="utf-8")

    def save(self) -> list[Path]:
        saved: list[Path] = []
        if 0 in self.dirty:
            make_backup(self.rom_path)
            with self.rom_path.open("r+b") as rom:
                for index, value in sorted(self.dirty[0].items()):
                    if index >= len(self.tables[0]):
                        raise EditorError("Attempted to write past the detected end of table 0")
                    rom.seek(self.rom_file_offset + index * 4)
                    rom.write(struct.pack("<I", parse_int_pointer(value)))
                rom.flush()
                os.fsync(rom.fileno())
            saved.append(self.rom_path)

        source_changes = {k: v for k, v in self.dirty.items() if k != 0}
        if source_changes:
            text = self.source_text
            replacements: list[tuple[int, int, str]] = []
            for table_index, changes in source_changes.items():
                array = locate_array(self.source_text, self.array_names[table_index - 1])
                for entry_index, value in changes.items():
                    start, end = array.value_spans[entry_index]
                    replacements.append((start, end, validate_c_expression(value)))
            for start, end, value in sorted(replacements, reverse=True):
                text = text[:start] + value + text[end:]
            make_backup(self.c_path)
            temp = self.c_path.with_name(self.c_path.name + ".ow_editor.tmp")
            try:
                temp.write_bytes(text.encode("utf-8"))
                os.replace(temp, self.c_path)
            finally:
                if temp.exists():
                    temp.unlink()
            saved.append(self.c_path)
        self.reload()
        return saved


def launch_gui(project_root: Path) -> None:
    import tkinter as tk
    from tkinter import filedialog, messagebox, ttk
    try:
        from PIL import Image, ImageTk
    except ImportError:
        Image = ImageTk = None

    class App:
        def __init__(self, window: tk.Tk, initial_root: Path):
            self.window = window
            self.model: Optional[ProjectModel] = None
            self.current_table = 0
            self.current_frame = 0
            self.preview_frames = []
            self.thumbnails = {}
            self.pending: list[PendingSprite] = []
            self.window.title("CFRU - Editor de Overworld Tables")
            self.window.geometry("1060x650")
            self.window.minsize(920, 570)
            self._build()
            self.open_project(initial_root)

        def _build(self) -> None:
            menu = tk.Menu(self.window)
            file_menu = tk.Menu(menu, tearoff=False)
            file_menu.add_command(label="Open project...", command=self.choose_project)
            file_menu.add_command(label="Reload", command=self.reload)
            file_menu.add_separator()
            file_menu.add_command(label="Save", command=self.save)
            file_menu.add_command(label="Exit", command=self.close)
            menu.add_cascade(label="File", menu=file_menu)
            actions_menu = tk.Menu(menu, tearoff=False)
            actions_menu.add_command(label="Add overworld...", command=lambda: self.sprite_dialog("add"))
            actions_menu.add_command(label="Resize overworld...", command=lambda: self.sprite_dialog("resize"))
            actions_menu.add_command(label="Add spritesheet...", command=self.add_sheet)
            actions_menu.add_separator()
            actions_menu.add_command(label="Export spritesheet...", command=self.export_sheet)
            menu.add_cascade(label="Actions", menu=actions_menu)
            self.window.configure(menu=menu)

            root = ttk.Frame(self.window, padding=10)
            root.pack(fill="both", expand=True)
            for column, weight in enumerate((5, 4, 4)):
                root.columnconfigure(column, weight=weight, uniform="main")
            root.rowconfigure(0, weight=1)

            left = ttk.LabelFrame(root, text="OWs/Tables Structure", padding=10)
            left.grid(row=0, column=0, sticky="nsew", padx=(0, 5))
            left.columnconfigure(0, weight=1); left.rowconfigure(2, weight=1)
            self.table_box = ttk.Combobox(left, state="readonly")
            self.table_box.grid(row=0, column=0, sticky="ew", pady=(0, 6))
            self.table_box.bind("<<ComboboxSelected>>", self.change_table)
            self.filter_var = tk.StringVar()
            self.filter_var.trace_add("write", lambda *_: self.refresh_tree())
            ttk.Entry(left, textvariable=self.filter_var).grid(row=1, column=0, sticky="ew", pady=(0, 6))
            tree_wrap = ttk.Frame(left); tree_wrap.grid(row=2, column=0, sticky="nsew")
            tree_wrap.columnconfigure(0, weight=1); tree_wrap.rowconfigure(0, weight=1)
            self.tree = ttk.Treeview(tree_wrap, columns=("preview", "value"), show="tree headings", selectmode="browse")
            self.tree.heading("#0", text="OWs Structure"); self.tree.heading("preview", text="Preview"); self.tree.heading("value", text="Pointer / Reference")
            self.tree.column("#0", width=145); self.tree.column("preview", width=62, anchor="center"); self.tree.column("value", width=160)
            self.tree.grid(row=0, column=0, sticky="nsew")
            scroll = ttk.Scrollbar(tree_wrap, orient="vertical", command=self.tree.yview); scroll.grid(row=0, column=1, sticky="ns")
            self.tree.configure(yscrollcommand=scroll.set); self.tree.bind("<<TreeviewSelect>>", self.select_entry)
            self.path_var = tk.StringVar()
            ttk.Label(left, textvariable=self.path_var, anchor="w").grid(row=3, column=0, sticky="ew", pady=(8, 0))

            middle = ttk.Frame(root); middle.grid(row=0, column=1, sticky="nsew", padx=5)
            middle.columnconfigure(0, weight=1); middle.rowconfigure(1, weight=1)
            edit = ttk.LabelFrame(middle, text="OverWorld Menu", padding=12); edit.grid(row=0, column=0, sticky="new")
            edit.columnconfigure(1, weight=1)
            self.info_vars = {key: tk.StringVar(value="-") for key in ("type", "frames", "pointer", "data", "frames_addr", "palette")}
            labels = (("Type:","type"),("Frames:","frames"),("Pointer Address:","pointer"),("Data Address:","data"),("Frames Address:","frames_addr"),("Palette ID:","palette"))
            for row,(label,key) in enumerate(labels):
                ttk.Label(edit,text=label).grid(row=row,column=0,sticky="w",pady=3)
                if key == "palette":
                    self.palette_var = tk.StringVar(value="Automatic")
                    ttk.Combobox(edit, textvariable=self.palette_var, values=PALETTE_ID_CHOICES,
                                 state="readonly").grid(row=row,column=1,sticky="ew",pady=3)
                else:
                    ttk.Label(edit,textvariable=self.info_vars[key]).grid(row=row,column=1,sticky="w",pady=3)
            ttk.Separator(edit).grid(row=6,column=0,columnspan=2,sticky="ew",pady=9)
            self.selection_var=tk.StringVar(value="Select an overworld"); ttk.Label(edit,textvariable=self.selection_var).grid(row=7,column=0,columnspan=2,sticky="w")
            self.value_var=tk.StringVar(); self.value_entry=ttk.Entry(edit,textvariable=self.value_var); self.value_entry.grid(row=8,column=0,columnspan=2,sticky="ew",pady=6)
            self.value_entry.bind("<Return>",lambda _e:self.apply_edit()); ttk.Button(edit,text="Apply change",command=self.apply_edit).grid(row=9,column=0,columnspan=2,sticky="ew")
            self.animation_var = tk.StringVar(value="Automatic")
            ttk.Label(edit, text="Spritesheet animation:").grid(row=10, column=0, sticky="w", pady=6)
            ttk.Combobox(edit, textvariable=self.animation_var, values=tuple(ANIMATION_TABLE_CHOICES),
                         state="readonly", width=25).grid(row=10, column=1, sticky="ew", pady=6)
            actions=ttk.LabelFrame(middle,text="Tables Menu",padding=12); actions.grid(row=1,column=0,sticky="nsew",pady=(10,0)); actions.columnconfigure(0,weight=1)
            buttons=ttk.Frame(actions); buttons.grid(row=0,column=0,sticky="ew"); buttons.columnconfigure((0,1),weight=1)
            ttk.Button(buttons,text="Add",command=lambda:self.sprite_dialog("add")).grid(row=0,column=0,sticky="ew",padx=(0,3),pady=3)
            ttk.Button(buttons,text="Resize",command=lambda:self.sprite_dialog("resize")).grid(row=0,column=1,sticky="ew",padx=(3,0),pady=3)
            ttk.Button(actions,text="Add spritesheet...",command=self.add_sheet).grid(row=1,column=0,sticky="ew",pady=3)
            ttk.Button(actions,text="Save changes",command=self.save).grid(row=2,column=0,sticky="ew",pady=3)
            ttk.Button(actions,text="Reload files",command=self.reload).grid(row=3,column=0,sticky="ew",pady=3)

            right = ttk.LabelFrame(root, text="Sprite View", padding=12); right.grid(row=0,column=2,sticky="nsew",padx=(5,0)); right.columnconfigure(0,weight=1); right.rowconfigure(0,weight=1)
            self.preview_canvas=tk.Canvas(right,bg="white",highlightthickness=1,highlightbackground="#b5b5b5",width=260,height=330)
            self.preview_canvas.grid(row=0,column=0,sticky="nsew")
            nav=ttk.Frame(right); nav.grid(row=1,column=0,pady=10)
            ttk.Button(nav,text="◀",width=3,command=lambda:self.move_frame(-1)).pack(side="left")
            self.frame_number_var=tk.IntVar(value=0)
            self.frame_spin=ttk.Spinbox(nav,from_=0,to=0,textvariable=self.frame_number_var,width=5,command=self.jump_frame)
            self.frame_spin.pack(side="left",padx=(8,3)); self.frame_spin.bind("<Return>",lambda _e:self.jump_frame())
            self.frame_var=tk.StringVar(value="/ 0"); ttk.Label(nav,textvariable=self.frame_var,width=7,anchor="w").pack(side="left")
            ttk.Button(nav,text="▶",width=3,command=lambda:self.move_frame(1)).pack(side="left")
            ttk.Button(right,text="Export spritesheet...",command=self.export_sheet).grid(row=2,column=0,pady=(0,4))

            bottom=ttk.Frame(root); bottom.grid(row=1,column=0,columnspan=3,sticky="ew",pady=(8,0)); bottom.columnconfigure(0,weight=1)
            self.status_var = tk.StringVar(value="Ready")
            ttk.Label(bottom, textvariable=self.status_var).grid(row=0, column=0, sticky="w")
            self.window.protocol("WM_DELETE_WINDOW", self.close)

        def choose_project(self) -> None:
            selected = filedialog.askdirectory(title="Select the CFRU project root")
            if selected:
                self.open_project(Path(selected))

        def open_project(self, path: Path) -> None:
            try:
                self.model = ProjectModel(path)
            except Exception as exc:
                messagebox.showerror("Could not open project", str(exc))
                return
            self.path_var.set(str(self.model.root))
            labels = [self.model.table_label(i) for i in range(len(self.model.tables))]
            self.table_box.configure(values=labels)
            self.table_box.current(0)
            self.current_table = 0
            self.refresh_tree()
            self.status_var.set(f"Loaded {len(labels)} tables. Table 0 is read directly from the ROM.")

        def effective_value(self, table: int, index: int) -> str:
            assert self.model
            return self.model.dirty.get(table, {}).get(index, self.model.tables[table][index].value)

        def refresh_tree(self) -> None:
            if not self.model:
                return
            selected = self.tree.selection()
            old_index = int(selected[0]) if selected else None
            self.tree.delete(*self.tree.get_children())
            needle = self.filter_var.get().strip().casefold()
            self.thumbnails.clear()
            for entry in self.model.tables[self.current_table]:
                value = self.effective_value(self.current_table, entry.index)
                haystack = f"{entry.index} {entry.name} {value}".casefold()
                if needle and needle not in haystack:
                    continue
                tags = ("changed",) if entry.index in self.model.dirty.get(self.current_table, {}) else ()
                thumb = self.make_thumbnail(entry.index)
                self.tree.insert("", "end", iid=str(entry.index), text=f"Overworld {entry.index}  {entry.name}", image=thumb or "", values=("", value), tags=tags)
            self.tree.tag_configure("changed", background="#fff2b2")
            if old_index is not None and str(old_index) in self.tree.get_children():
                self.tree.selection_set(str(old_index))

        def change_table(self, _event=None) -> None:
            self.current_table = self.table_box.current()
            self.selection_var.set("No entry selected")
            self.value_var.set("")
            self.refresh_tree()

        def select_entry(self, _event=None) -> None:
            if not self.model or not self.tree.selection():
                return
            index = int(self.tree.selection()[0])
            entry = self.model.tables[self.current_table][index]
            self.selection_var.set(f"Table {self.current_table} | ID {index} | {entry.name}")
            self.value_var.set(self.effective_value(self.current_table, index))
            animation = self.model.sprite_animation_table({
                "mode": "resize", "target_table": self.current_table, "target_index": index,
            })
            self.animation_var.set(next((label for label, value in ANIMATION_TABLE_CHOICES.items()
                                         if value == animation), "Automatic"))
            self.show_sprite(index)
            current_palette = self.info_vars["palette"].get()
            self.palette_var.set(current_palette if current_palette in PALETTE_ID_CHOICES else "Automatic")

        @staticmethod
        def decode_4bpp(raw, palette, width, height):
            image = Image.new("RGBA", (width, height))
            pixels = image.load(); tiles_x = max(1, width // 8)
            for y in range(height):
                for x in range(width):
                    tile=(y//8)*tiles_x+(x//8); pos=tile*32+(y%8)*4+(x%8)//2
                    colour=(raw[pos] >> (4 if x&1 else 0)) & 15 if pos < len(raw) else 0
                    pixels[x,y]=palette[colour]
            return image

        def load_frames(self, index):
            if not self.model or Image is None: return [], None
            try: info=self.model.sprite_info(self.current_table,index)
            except Exception: return [], None
            if not info: return [], None
            images=[]
            if info.png_path and info.png_path.is_file():
                with Image.open(info.png_path) as sheet:
                    sheet=sheet.convert("RGBA")
                    for frame in range(info.frames): images.append(sheet.crop((0,frame*info.height,info.width,(frame+1)*info.height)))
            elif info.rom_frames:
                images=[self.decode_4bpp(raw,pal,info.width,info.height) for raw,pal in info.rom_frames]
            return images,info

        def make_thumbnail(self,index):
            frames,_info=self.load_frames(index)
            if not frames or ImageTk is None:return None
            frame=frames[0].copy(); frame.thumbnail((28,36),Image.Resampling.NEAREST)
            photo=ImageTk.PhotoImage(frame); self.thumbnails[index]=photo; return photo

        def show_sprite(self,index):
            self.preview_frames,info=self.load_frames(index); self.current_frame=0
            value=self.effective_value(self.current_table,index)
            pointer=f"0x{self.model.rom_gba_address+index*4:08X}" if self.current_table==0 else f"{self.model.array_names[self.current_table-1]}[{index}]"
            self.info_vars["pointer"].set(pointer)
            if info:
                self.info_vars["type"].set(f"{info.width}x{info.height}"); self.info_vars["frames"].set(str(info.frames)); self.info_vars["data"].set(info.data_address); self.info_vars["frames_addr"].set(info.frames_address); self.info_vars["palette"].set(info.palette_tag)
            else:
                for key in ("type","frames","data","frames_addr","palette"):self.info_vars[key].set("-")
            self.render_frame()

        def render_frame(self):
            self.preview_canvas.delete("all")
            if not self.preview_frames or ImageTk is None:
                self.preview_canvas.create_text(130,150,text="Preview unavailable",fill="#777"); self.frame_number_var.set(0); self.frame_spin.configure(from_=0,to=0); self.frame_var.set("/ 0"); return
            image=self.preview_frames[self.current_frame].copy(); scale=max(1,min(5,220//max(image.size))); image=image.resize((image.width*scale,image.height*scale),Image.Resampling.NEAREST)
            self.preview_photo=ImageTk.PhotoImage(image); self.preview_canvas.update_idletasks(); x=max(130,self.preview_canvas.winfo_width()//2); y=max(150,self.preview_canvas.winfo_height()//2)
            self.preview_canvas.create_image(x,y,image=self.preview_photo)
            self.frame_spin.configure(from_=1,to=len(self.preview_frames)); self.frame_number_var.set(self.current_frame+1); self.frame_var.set(f"/ {len(self.preview_frames)}")

        def move_frame(self,delta):
            if self.preview_frames:self.current_frame=(self.current_frame+delta)%len(self.preview_frames); self.render_frame()

        def jump_frame(self):
            if not self.preview_frames:return
            try:number=int(self.frame_number_var.get())
            except (ValueError,tk.TclError):number=1
            self.current_frame=max(0,min(len(self.preview_frames)-1,number-1)); self.render_frame()

        def export_sheet(self):
            if not self.preview_frames or Image is None:
                messagebox.showwarning("Export","Select an overworld that has frames."); return
            selected=self.tree.selection(); default="overworld.png"
            if selected and self.model:
                entry=self.model.tables[self.current_table][int(selected[0])]
                default=re.sub(r"\W+","_",entry.name).strip("_")+".png"
            path=filedialog.asksaveasfilename(title="Export spritesheet",initialfile=default,defaultextension=".png",filetypes=[("PNG","*.png")])
            if not path:return
            width=max(frame.width for frame in self.preview_frames); height=sum(frame.height for frame in self.preview_frames)
            sheet=Image.new("RGBA",(width,height)); y=0
            for frame in self.preview_frames:sheet.paste(frame,(0,y),frame); y+=frame.height
            sheet.save(path); self.status_var.set(f"Spritesheet exported: {path}")

        def sprite_dialog(self, mode):
            if not self.model:return
            target=(-1,-1)
            if mode=="resize":
                if not self.tree.selection():
                    messagebox.showwarning("Resize","Select an overworld before using Resize."); return
                target=(self.current_table,int(self.tree.selection()[0]))
            dialog=tk.Toplevel(self.window); dialog.title("Add Overworld" if mode=="add" else "Resize Overworld"); dialog.transient(self.window); dialog.grab_set(); dialog.resizable(False,False)
            frame=ttk.Frame(dialog,padding=14); frame.pack(fill="both",expand=True)
            name=tk.StringVar(value=f"Custom_{len(self.pending)+1}"); size=tk.StringVar(value="32x32"); frames=tk.IntVar(value=9); sheet=tk.StringVar()
            ttk.Label(frame,text="Name:").grid(row=0,column=0,sticky="w",pady=4); ttk.Entry(frame,textvariable=name,width=28).grid(row=0,column=1,sticky="ew")
            ttk.Label(frame,text="Dimensions:").grid(row=1,column=0,sticky="w",pady=4); ttk.Combobox(frame,textvariable=size,values=("16x16","16x32","32x32","64x64"),state="readonly").grid(row=1,column=1,sticky="ew")
            ttk.Label(frame,text="Frame count:").grid(row=2,column=0,sticky="w",pady=4); ttk.Spinbox(frame,from_=1,to=32,textvariable=frames).grid(row=2,column=1,sticky="ew")
            ttk.Label(frame,text="Spritesheet:").grid(row=3,column=0,sticky="w",pady=4); ttk.Entry(frame,textvariable=sheet,state="readonly").grid(row=3,column=1,sticky="ew")
            def browse():
                path=filedialog.askopenfilename(parent=dialog,title="Select the spritesheet",filetypes=[("PNG","*.png")]);
                if path:sheet.set(path)
            ttk.Button(frame,text="Escolher...",command=browse).grid(row=3,column=2,padx=(5,0))
            animation = tk.StringVar(value=self.animation_var.get() if mode == "resize" else "Automatic")
            ttk.Label(frame, text="ImageAnimTable:").grid(row=4, column=0, sticky="w", pady=4)
            ttk.Combobox(frame, textvariable=animation, values=tuple(ANIMATION_TABLE_CHOICES),
                         state="readonly", width=25).grid(row=4, column=1, sticky="ew")
            palette = tk.StringVar(value=self.palette_var.get() if mode == "resize" else "Automatic")
            ttk.Label(frame, text="Palette ID:").grid(row=5, column=0, sticky="w", pady=4)
            ttk.Combobox(frame, textvariable=palette, values=PALETTE_ID_CHOICES,
                         state="readonly").grid(row=5, column=1, sticky="ew")
            def confirm():
                try:
                    width,height=map(int,size.get().split("x")); count=int(frames.get())
                    if not 1<=count<=32:raise ValueError
                except ValueError:
                    messagebox.showerror("Invalid value","Choose between 1 and 32 frames.",parent=dialog); return
                request=PendingSprite(mode,name.get().strip(),width,height,count,Path(sheet.get()) if sheet.get() else None,*target,
                                      animation_table=ANIMATION_TABLE_CHOICES[animation.get()],
                                      palette_id=None if palette.get() == "Automatic" else int(palette.get(), 16))
                self.pending.append(request); dialog.destroy(); self.update_pending_status()
            ttk.Button(frame,text="Add to queue",command=confirm).grid(row=6,column=0,columnspan=3,sticky="ew",pady=(12,0))
            dialog.wait_window()

        def add_sheet(self):
            missing=[item for item in self.pending if item.sheet is None]
            selected = self.tree.selection()
            use_selection = bool(selected)
            if missing and selected:
                use_selection = messagebox.askyesno(
                    "Add spritesheet",
                    "Apply the spritesheet to the selected overworld?\n\n"
                    "Choose No to assign it to the pending Add/Resize operation."
                )
            if missing and not use_selection:
                path=filedialog.askopenfilename(title=f"Spritesheet for {missing[0].name}",filetypes=[("PNG","*.png")])
                if path:
                    try:
                        self.validate_sheet(Path(path), missing[0].width, missing[0].height, missing[0].frames)
                    except EditorError as exc:
                        messagebox.showerror("Incompatible spritesheet",str(exc)); return
                    missing[0].sheet=Path(path); self.update_pending_status()
                return
            if not selected or not self.model:
                messagebox.showinfo("Spritesheet","Select an overworld or create a pending Add/Resize operation."); return
            index=int(selected[0])
            try:
                info=self.model.sprite_info(self.current_table,index)
            except EditorError as exc:
                messagebox.showerror("Spritesheet",str(exc)); return
            if not info:
                messagebox.showerror("Spritesheet","The current selection has no recognizable dimensions or frames."); return
            path=filedialog.askopenfilename(title="New spritesheet for the selection",filetypes=[("PNG","*.png")])
            if not path:return
            try:
                self.validate_sheet(Path(path),info.width,info.height,info.frames)
            except EditorError as exc:
                messagebox.showerror("Incompatible spritesheet",str(exc)); return
            entry=self.model.tables[self.current_table][index]
            base=re.sub(r"\W+","_",entry.name).strip("_") or f"Table{self.current_table}_OW{index}"
            name=f"{base}_Sheet_{len(self.pending)+1}"
            self.pending.append(PendingSprite("resize",name,info.width,info.height,info.frames,Path(path),self.current_table,index,
                                              animation_table=ANIMATION_TABLE_CHOICES[self.animation_var.get()],
                                              palette_id=None if self.palette_var.get() == "Automatic" else int(self.palette_var.get(), 16)))
            self.update_pending_status()
            messagebox.showinfo("Spritesheet","The new sheet has been assigned to the selection. Use Save changes to apply it.")

        @staticmethod
        def validate_sheet(path, width, height, frames):
            if Image is None:
                raise EditorError("Pillow is required to validate spritesheets")
            try:
                with Image.open(path) as image:
                    vertical=image.width==width and image.height==height*frames
                    horizontal=image.height==height and image.width==width*frames
                    if not (vertical or horizontal):
                        raise EditorError(
                            f"Incompatible dimensions: received {image.width}x{image.height}; "
                            f"expected {width}x{height * frames} (vertical) or "
                            f"{width * frames}x{height} (horizontal), with {frames} frames."
                        )
                    if image.mode!="P":
                        raise EditorError("The PNG must use indexed mode (P).")
                    if max(image.getdata(),default=0)>15:
                        raise EditorError("The spritesheet must use no more than 16 color indexes.")
            except EditorError:
                raise
            except Exception as exc:
                raise EditorError(f"Could not read the spritesheet: {exc}") from exc

        def update_pending_status(self):
            missing=sum(item.sheet is None for item in self.pending)
            self.status_var.set(f"{len(self.pending)} pending Add/Resize operation(s); {missing} without a spritesheet.")

        def apply_edit(self) -> None:
            if not self.model or not self.tree.selection():
                messagebox.showinfo("Edit", "Select an entry first.")
                return
            index = int(self.tree.selection()[0])
            try:
                normalized = self.model.set_value(self.current_table, index, self.value_var.get())
            except EditorError as exc:
                messagebox.showerror("Invalid value", str(exc))
                return
            self.value_var.set(normalized)
            self.refresh_tree()
            self.tree.selection_set(str(index))
            self.tree.see(str(index)); self.show_sprite(index)
            total = sum(len(changes) for changes in self.model.dirty.values())
            self.status_var.set(f"{total} pending change(s).")

        def save(self) -> None:
            if not self.model or (not self.model.dirty and not self.pending):
                messagebox.showinfo("Save", "There are no pending changes.")
                return
            missing=[item.name for item in self.pending if item.sheet is None]
            if missing:
                messagebox.showwarning("Spritesheet required","Saving is not allowed. Add a spritesheet for:\n\n"+"\n".join(missing)); return
            total = sum(len(changes) for changes in self.model.dirty.values())+len(self.pending)
            selected_index = int(self.tree.selection()[0]) if self.tree.selection() else None
            selected_table = self.current_table
            if not messagebox.askyesno("Confirm", f"Save {total} change(s)? Backups will be created automatically."):
                return
            try:
                saved = self.model.save() if self.model.dirty else []
                if self.pending:
                    self.model.install_sprites(self.pending); saved.extend([self.model.c_path,self.model.root/"overworld_editor.json"]); self.pending.clear(); self.model.reload()
            except Exception as exc:
                messagebox.showerror("Could not save", str(exc))
                return
            self.table_box.configure(values=[self.model.table_label(i) for i in range(len(self.model.tables))])
            if selected_table < len(self.model.tables):
                self.current_table = selected_table
                self.table_box.current(selected_table)
            self.refresh_tree()
            if selected_index is not None and selected_index < len(self.model.tables[self.current_table]):
                item_id = str(selected_index)
                if item_id in self.tree.get_children():
                    self.tree.selection_set(item_id)
                    self.tree.focus(item_id)
                    self.tree.see(item_id)
                    self.select_entry()
            self.status_var.set("Salvo: " + ", ".join(path.name for path in saved))
            messagebox.showinfo("Complete", "Changes saved. The .ow_editor.bak backups were preserved.")

        def reload(self) -> None:
            if not self.model:
                return
            if (self.model.dirty or self.pending) and not messagebox.askyesno("Discard", "Discard unsaved changes?"):
                return
            try:
                self.model.reload()
                self.pending.clear()
            except Exception as exc:
                messagebox.showerror("Could not reload", str(exc))
                return
            self.table_box.configure(values=[self.model.table_label(i) for i in range(len(self.model.tables))])
            self.refresh_tree()
            self.status_var.set("Files reloaded.")

        def close(self) -> None:
            if self.model and (self.model.dirty or self.pending):
                if not messagebox.askyesno("Exit", "There are unsaved changes. Exit anyway?"):
                    return
            self.window.destroy()

    window = tk.Tk()
    App(window, project_root)
    window.mainloop()


def main(argv: Optional[list[str]] = None) -> int:
    parser = argparse.ArgumentParser(description="Tkinter editor for gOverworldTableSwitcher tables")
    parser.add_argument("project", nargs="?", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--check", action="store_true", help="validate and list tables without opening the interface")
    args = parser.parse_args(argv)
    try:
        model = ProjectModel(args.project)
        if args.check:
            print(f"Project: {model.root}")
            for index in range(len(model.tables)):
                print(model.table_label(index))
            return 0
        launch_gui(model.root)
        return 0
    except (EditorError, OSError) as exc:
        print(f"Error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
