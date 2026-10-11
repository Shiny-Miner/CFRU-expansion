# Mystery Gift for Pokémon FireRed + CFRU  (English version)

A full-screen **gift code** system for CFRU projects. The player types a code on an on-screen keyboard, a gift box drops,
shakes, bursts open, and the gift appears: a **Pokémon** (custom moves, shiny, hidden ability, held item, nature, IVs 31),
an **item** for the bag, or **both**. Each code can be claimed **once per save**. A built-in menu song plays during the screen.

This is the **English-only** build: all game texts are in English and the package does **not** use any language flags.
You edit everything in a small **window program** (`tools/editor_mystery_gift.py`) - no C code needed.

Screenshots (real emulator captures): see the `previews/` folder.

## Requirements
- A CFRU-expansion project (the usual `src`, `include`, `hooks`, `routinepointers`... layout) for **Fire Red 1.0**.
- `SAVE_BLOCK_EXPANSION` enabled in `src/config.h` (it is on by default; CFRU says "do not comment it out").
  The Mystery Gift flags live above 0x900, which CFRU only saves with that option.
- Python 3 (with Tk, included on Windows) for the editor, and your usual build tools (`arm-none-eabi-gcc`).

## Install (copy and paste, nothing of yours is overwritten)
1. Copy **everything inside `project/`** into the root of your CFRU project. They are 6 new files:
   `src/mystery_gift.c`, `src/mystery_gift_gfx.c`, `src/mystery_gift_codes.h`, `include/new/mystery_gift.h`,
   `strings/mystery_gift.string`, `assembly/overworld_scripts/mystery_gift.s`.
2. *(Optional, only to use `special 0x122`)* run once:
   `python tools/install_special.py "C:\path\to\your\project"`
   It adds 1 line to your `routinepointers` (with a backup), refuses to install if the slot is already used, and `--remove` undoes it.
3. Build (`python scripts/make.py`).
4. In a map script (XSE): `special 0x122` then `waitstate` (or `callasm MysteryGift_Start` then `waitstate`, which needs no step 2).

```
#dynamic 0x800000
#org @start
lock
faceplayer
special 0x122
waitstate
release
end
```
After the `waitstate`, `LASTRESULT` is **1** if the player received something and **0** if they left. Always put `waitstate`
right after the special/callasm. Without step 2, `special 0x122` does nothing and the script would hang at `waitstate`.

## Editing the codes (the editor)
1. Double-click `tools/OPEN_EDITOR.bat` (or run `python tools/editor_mystery_gift.py`).
2. Click **Browse...** and choose your project root folder (the one with `src` and `include`). The editor reads your real Pokémon,
   item and move names from it.
3. Click **New** (or pick a code in the list) and fill in: the code (letters A-Z and digits 0-9, up to 8), whether the gift is a
   Pokémon / an Item / both, species, level, shiny, hidden ability, held item, nature, up to 4 moves, the gift item and its quantity.
4. Click **SAVE AND GENERATE**. The editor only writes the file if the **compiler accepts it** ("Checked with the compiler: OK").
5. Build again. The editor already forces `mystery_gift.c` to be recompiled (the CFRU build does not notice `.h` changes).
6. **Prove it on your ROM:** `python tools/check_rom.py YOUR_ROM.gba` tells, code by code, whether it made it into the game.
   If it says `MISSING`, delete the `build` folder and build again.

Safety features of the editor: it only offers names that **exist and are active** in your project; it refuses a starting flag that
**conflicts** with flags your project already uses; a code's flag is **never reused**, even if you delete the code;
switching codes does not lose what you were editing; a corrupted `codes.json` is kept as a copy and the editor starts over.

### Files the editor touches
`tools/codes.json` (writes) - `<project>/src/mystery_gift_codes.h` (**the only project file it writes**) -
`<project>/src/mystery_gift.c` (only its modification date, to force a rebuild) - it **reads** `include/constants/*.h`,
`src/config.h` and the flag headers. Nothing else.

## Flags
Every code uses its own flag (`0x950 + slot`, up to 64 codes) so it can be claimed only once per save. The **Flag** column of the
list shows each code's flag; to test a code again run `clearflag` on it. The "Starting flag" field only moves the block's starting
point (the editor refuses values that conflict with your project). Make sure 0x950-0x98F are free in your project.

## Options
- **Perfect IVs:** every Pokémon is created with IVs 31. To get CFRU's normal random IVs instead, set `#define MG_PERFECT_IVS FALSE`
  near the top of `src/mystery_gift.c`.
- **Music:** pick the menu music in the editor (songs from the game itself, read from the map headers). The map music returns on exit.
  "None" keeps the map music.
- **Variables:** the screen uses variables 0x8004-0x8006 and the `gStringVar1-4` buffers as scratch space. Do not rely on their
  values after the special; the result is in `LASTRESULT`.

See `SAFETY_AND_TESTS.md` for what was tested and what was not.
