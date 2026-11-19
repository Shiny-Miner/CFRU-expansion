#!/usr/bin/env python3

import re
import struct
import sys
import zlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
GFX = ROOT / "graphics" / "Battle_UI" / "Textbox"
OUT = ROOT / "src" / "battle_textbox_data.h"
CONFIG = ROOT / "src" / "config.h"
CONFIG_FLAG = "NEW_BATTLE_TEXTBOX"

TILE_BASE = 0x370
MSG_SEGMENTS = ((0x03, 15), (0x192, 46), (0x1F0, 16))
MAX_BUTTON_TILES = 144
CURSOR_TILES = 7
RING_TILES = 9
PILL_COLOUR = 0x775A
KEEP = 0x7C1F
TRANSPARENT = -1

ACTION_STATES = ("Fight", "Bag", "Pokemon", "Run")
SAFARI_STATES = ("Ball", "Bait", "Rock", "Run")


class BuildError(RuntimeError):
    pass


class Png:
    def __init__(self, path):
        self.path = path
        self.palette = None
        self.trns = None
        data = path.read_bytes()
        if data[:8] != b"\x89PNG\r\n\x1a\n":
            raise BuildError(f"{path.name} is not a PNG file")
        pos = 8
        idat = b""
        while pos < len(data):
            length, kind = struct.unpack(">I4s", data[pos:pos + 8])
            body = data[pos + 8:pos + 8 + length]
            pos += 12 + length
            if kind == b"IHDR":
                self.width, self.height, self.depth, self.ctype, _, _, interlace = struct.unpack(">IIBBBBB", body)
                if interlace:
                    raise BuildError(f"{path.name} is interlaced, save it without interlacing")
            elif kind == b"PLTE":
                self.palette = [tuple(body[i:i + 3]) for i in range(0, len(body), 3)]
            elif kind == b"tRNS":
                self.trns = body
            elif kind == b"IDAT":
                idat += body
            elif kind == b"IEND":
                break
        channels = {2: 3, 3: 1, 6: 4}.get(self.ctype)
        if channels is None or (self.ctype != 3 and self.depth != 8) or (self.ctype == 3 and self.depth not in (1, 2, 4, 8)):
            raise BuildError(f"{path.name}: save it as an 8-bit RGB, RGBA or indexed PNG")
        self.indexed = self.ctype == 3
        self.channels = channels
        self.rows = self._unfilter(zlib.decompress(idat))

    def _unfilter(self, raw):
        stride = (self.width * self.channels * self.depth + 7) // 8
        bpp = max(1, self.channels * self.depth // 8)
        prev = bytearray(stride)
        pos = 0
        rows = []
        for _ in range(self.height):
            ftype = raw[pos]
            line = bytearray(raw[pos + 1:pos + 1 + stride])
            pos += 1 + stride
            for i in range(stride):
                left = line[i - bpp] if i >= bpp else 0
                up = prev[i]
                if ftype == 1:
                    line[i] = (line[i] + left) & 255
                elif ftype == 2:
                    line[i] = (line[i] + up) & 255
                elif ftype == 3:
                    line[i] = (line[i] + ((left + up) >> 1)) & 255
                elif ftype == 4:
                    ul = prev[i - bpp] if i >= bpp else 0
                    p = left + up - ul
                    pa, pb, pc = abs(p - left), abs(p - up), abs(p - ul)
                    pred = left if pa <= pb and pa <= pc else up if pb <= pc else ul
                    line[i] = (line[i] + pred) & 255
            prev = line
            rows.append(self._pixels(line))
        return rows

    def _pixels(self, line):
        if self.ctype == 2:
            return [(line[i], line[i + 1], line[i + 2], 255) for i in range(0, self.width * 3, 3)]
        if self.ctype == 6:
            return [tuple(line[i:i + 4]) for i in range(0, self.width * 4, 4)]
        if self.depth == 8:
            return list(line[:self.width])
        per = 8 // self.depth
        mask = (1 << self.depth) - 1
        return [(line[x // per] >> (8 - self.depth * (x % per + 1))) & mask for x in range(self.width)]

    def index(self, x, y):
        if not self.indexed:
            raise BuildError(f"{self.path.name} must be an indexed PNG")
        return self.rows[y][x]

    def colour(self, x, y):
        value = self.rows[y][x]
        if not self.indexed:
            return value
        if self.palette is None or value >= len(self.palette):
            raise BuildError(f"{self.path.name}: pixel ({x},{y}) uses a palette entry that does not exist")
        alpha = self.trns[value] if self.trns is not None and value < len(self.trns) else 255
        return self.palette[value] + (alpha,)

    def word(self, x, y):
        r, g, b, a = self.colour(x, y)
        if a < 128:
            return TRANSPARENT
        return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def load(name, width, height):
    path = GFX / name
    if not path.is_file():
        raise BuildError(f"missing {path.relative_to(ROOT)}")
    png = Png(path)
    if (png.width, png.height) != (width, height):
        raise BuildError(f"{name} must be {width}x{height} pixels, it is {png.width}x{png.height}")
    return png


def load_palettes():
    png = load("Palettes.png", 16, 7)
    rows = [[png.word(x, y) for x in range(16)] for y in range(7)]
    luts = []
    for row in rows[:6]:
        lut = {}
        for i, word in enumerate(row):
            lut.setdefault(word, i)
        lut[TRANSPARENT] = 0
        luts.append(lut)
    words = [[0] + [w if w >= 0 else 0 for w in row[1:]] for row in rows[:6]]
    window = [0xFFFF if w in (KEEP, TRANSPARENT) or i == 0 else w for i, w in enumerate(rows[6])]
    return luts, words, window


def tile_indexes(png, tx, ty, lut):
    out = []
    for y in range(8):
        for x in range(8):
            word = png.word(tx * 8 + x, ty * 8 + y)
            if word not in lut:
                return None, (tx * 8 + x, ty * 8 + y, word)
            out.append(lut[word])
    return bytes(out), None


def pack(tiles):
    words = []
    for tile in tiles:
        for y in range(8):
            for x in (0, 4):
                words.append(sum(tile[y * 8 + x + k] << (4 * k) for k in range(4)))
    return words


def sheet_tiles(name, width_tiles, height_tiles, count):
    png = load(name, width_tiles * 8, height_tiles * 8)
    tiles = []
    for t in range(count):
        tx, ty = t % width_tiles, t // width_tiles
        tile = bytearray()
        for y in range(8):
            for x in range(8):
                index = png.index(tx * 8 + x, ty * 8 + y)
                if index > 15:
                    raise BuildError(f"{name}: pixel ({tx * 8 + x},{ty * 8 + y}) uses colour index {index}, only 0-15 exist")
                tile.append(index)
        tiles.append(bytes(tile))
    return tiles


class Builder:
    def __init__(self):
        self.luts, self.words, self.window = load_palettes()
        self.msg_tiles = []
        self.msg_index = {}

    def fail(self, name, tx, ty, bad):
        x, y, word = bad
        shown = "transparent" if word == TRANSPARENT else "#%02X%02X%02X" % ((word & 31) << 3, ((word >> 5) & 31) << 3, ((word >> 10) & 31) << 3)
        raise BuildError(f"{name}: pixel ({x},{y}) in tile ({tx},{ty}) has colour {shown} which is not in Palettes.png")

    def slot(self, i):
        for base, count in MSG_SEGMENTS:
            if i < count:
                return base + i
            i -= count
        raise BuildError("too many message tiles")

    def msg_tile(self, png, tx, ty):
        tile, bad = tile_indexes(png, tx, ty, self.luts[0])
        if tile is None:
            self.fail(png.path.name, tx, ty, bad)
        if tile not in self.msg_index:
            self.msg_index[tile] = len(self.msg_tiles)
            self.msg_tiles.append(tile)
        return self.slot(self.msg_index[tile])

    def button_tile(self, png, tx, ty, tiles, index):
        first = 12 + (1 if tx >= 22 else 0) + (2 if ty >= 3 else 0)
        bad = None
        for bank in (first, 12, 13, 14, 15):
            tile, bad = tile_indexes(png, tx, ty, self.luts[bank - 10])
            if tile is not None:
                key = (bank, tile)
                if key not in index:
                    index[key] = len(tiles)
                    tiles.append(tile)
                return (TILE_BASE + index[key]) | (bank << 12)
        self.fail(png.path.name, tx, ty, bad)

    def message_map(self, png):
        return [[self.msg_tile(png, tx, ty) for tx in range(30)] for ty in range(6)]

    def action_maps(self, names, prefix):
        tiles, index, maps = [], {}, []
        for name in names:
            png = load(f"{prefix}{name}.png", 240, 48)
            maps.append([[self.msg_tile(png, tx, ty) if tx < 14 else self.button_tile(png, tx, ty, tiles, index)
                          for tx in range(30)] for ty in range(6)])
        if len(tiles) > MAX_BUTTON_TILES:
            raise BuildError(f"{prefix}*.png needs {len(tiles)} button tiles, the limit is {MAX_BUTTON_TILES}")
        return tiles, maps

    def counter_tiles(self):
        png = load("Safari_Counter.png", 96, 32)
        tiles = []
        for ty in range(4):
            for tx in range(12):
                tile, bad = tile_indexes(png, tx, ty, self.luts[0])
                if tile is None:
                    self.fail(png.path.name, tx, ty, bad)
                tiles.append(tile)
        return tiles


def generate():
    b = Builder()
    msg_map = b.message_map(load("Message.png", 240, 48))
    moves_map = b.message_map(load("Moves.png", 240, 48))
    tiles, act_maps = b.action_maps(ACTION_STATES, "Action_")
    safari_tiles, safari_maps = b.action_maps(SAFARI_STATES, "Safari_Action_")
    counter = b.counter_tiles()
    cursor = sheet_tiles("Cursor.png", 8, 1, CURSOR_TILES)
    ring = sheet_tiles("Ring.png", 3, 3, RING_TILES)
    if len(b.msg_tiles) > sum(count for _, count in MSG_SEGMENTS):
        raise BuildError(f"the message and move menu pictures need {len(b.msg_tiles)} unique tiles, the limit is {sum(count for _, count in MSG_SEGMENTS)}")

    fmt = lambda a: ",".join("0x%04X" % v for v in a)
    out = [
        "// AUTO-GENERATED by scripts/battle_textbox.py from graphics/Battle_UI/Textbox/, do not edit",
        "#define BTB_TILE_COUNT %d" % len(tiles),
        "#define BTB_SAFARI_TILE_COUNT %d" % len(safari_tiles),
        "#define BTB_TILE_BASE 0x%X" % TILE_BASE,
        "#define BTB_MSG_TILE_COUNT %d" % len(b.msg_tiles),
        "#define BTB_MSG_SEG0_BASE 0x%X" % MSG_SEGMENTS[0][0],
        "#define BTB_MSG_SEG0_COUNT %d" % MSG_SEGMENTS[0][1],
        "#define BTB_MSG_SEG1_BASE 0x%X" % MSG_SEGMENTS[1][0],
        "#define BTB_MSG_SEG1_COUNT %d" % MSG_SEGMENTS[1][1],
        "#define BTB_MSG_SEG2_BASE 0x%X" % MSG_SEGMENTS[2][0],
        "#define BTB_PILL_COLOUR 0x%04X //Type pill chip colour" % PILL_COLOUR,
        "#define BTB_PANEL_COLOUR 0x%04X //Panel/window fill colour" % b.words[0][15],
        "const u16 gBTBTiles[] = {" + fmt(pack(tiles)) + "};",
        "const u16 gBTBSafariTiles[] = {" + fmt(pack(safari_tiles)) + "};",
        "const u16 gBTBSafariCounterTiles[768] = {" + fmt(pack(counter)) + "};",
        "const u16 gBTBMsgTiles[] = {" + fmt(pack(b.msg_tiles)) + "};",
        "const u16 gBTBCursorTiles[112] = {" + fmt(pack(cursor)) + "};",
        "const u16 gBTBRingTiles[144] = {" + fmt(pack(ring)) + "};",
        "const u16 gBTBPal1[16] = {" + fmt(b.words[1]) + "};",
        "const u16 gBTBPal0[16] = {" + fmt(b.words[0]) + "};",
        "const u16 gBTBButtonPals[64] = {" + fmt([w for row in b.words[2:6] for w in row]) + "};",
        "const u16 gBTBPal5[16] = {" + fmt(b.window) + "}; //0xFFFF = keep",
        "const u16 gBTBMsgMap[6][30] = {" + ",".join("{" + fmt(r) + "}" for r in msg_map) + "};",
        "const u16 gBTBMovesMap[6][30] = {" + ",".join("{" + fmt(r) + "}" for r in moves_map) + "};",
        "const u16 gBTBSafariActionMaps[4][6][30] = {" + ",".join("{" + ",".join("{" + fmt(r) + "}" for r in m) + "}" for m in safari_maps) + "};",
        "const u16 gBTBActionMaps[4][6][30] = {" + ",".join("{" + ",".join("{" + fmt(r) + "}" for r in m) + "}" for m in act_maps) + "};",
    ]
    return "\n".join(out) + "\n"


def enabled():
    text = CONFIG.read_text(errors="replace")
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return re.search(r"^[ \t]*#[ \t]*define[ \t]+" + CONFIG_FLAG + r"\b", text, re.M) is not None


def build():
    if not enabled():
        return
    try:
        text = generate()
    except BuildError as e:
        print(f"Error: graphics/Battle_UI/Textbox: {e}", file=sys.stderr)
        sys.exit(1)
    if OUT.is_file() and OUT.read_bytes() == text.encode():
        return
    OUT.write_bytes(text.encode())
    print("Generated", OUT.relative_to(ROOT))


if __name__ == "__main__":
    if len(sys.argv) != 2 or sys.argv[1] != "build":
        print("usage: battle_textbox.py build", file=sys.stderr)
        sys.exit(2)
    build()
