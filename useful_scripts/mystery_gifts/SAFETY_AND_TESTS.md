# Safety and tests

## Does it conflict with CFRU?
Checked against CFRU-expansion and the Stars package:
- The 6 new files do not overwrite anything, and the 64 global names they define (functions, graphics, texts) exist nowhere else.
- No hooks and no `bytereplacement` are used. The only contact point is the optional `special 0x122`, written by
  `install_special.py` (2 lines in `routinepointers`, with a backup). On a Fire Red 1.0 ROM that slot is an empty entry and
  CFRU does not use it. CFRU only turns off the *original* wireless Mystery Gift menu entry, at an address outside the specials table.
- Flags 0x950-0x98F are not used by CFRU. This English build reads **no** language flags at all.
- There are no writable globals (the object has `.data` = `.bss` = 0, as this build requires). Memory is allocated with checks
  and freed on exit; if an allocation fails the game returns to the map without opening the screen.
- Everything it calls (`CreateMon`, `GiveMonNatureAndAbility`, `GiveMonToPlayer`, `AddBagItem`, `CheckBagHasSpace`, `ItemId_GetName`...)
  compiles against CFRU's real headers without warnings.
- It never writes to the save directly; it only uses the game's own functions (`FlagSet`, `AddBagItem`, `GiveMonToPlayer`).
  The whole gift (Pokémon + item + flag) is given at once at the white flash, so there is no "half claimed" state.

## What was tested
| Test | Result |
|---|---|
| Editor logic: validation, file generation, compiler-checked writing, flags, JSON, texts (`tools/tests/test_editor.py`) | 97 checks OK + 6,000 fuzz codes |
| Compile **every** name the editor lists (`tools/tests/test_constants.py`) | 1,462 Pokémon, 793 items, 992 moves: 0 failures |
| The game's real code-matching logic (`tools/tests/test_matching.py`) | 71,943 queries, 0 failures |
| Emulator (mGBA) with a Fire Red 1.0 ROM: all 36 keys | the stored character and the drawn letter match on 36/36 |
| Emulator: 64-code stress table (1-8 characters, digits only, letters only, chained prefixes) | 64/64 accepted on the right line, 40/40 non-members rejected |
| Emulator: full flow | falling gift, shake, burst, white flash, Pokémon page, item-only page, Pokémon + item (two pages), "no room" case |
| Editor window driven by clicks and typing | create, apply, switch rows without losing edits, red error, blocked save, compiler-checked save, delete without reusing a flag |
| End to end | codes created in the window -> generated `.h` -> built into the game -> typed in the emulator: accepted; deleted code and prefixes rejected |

## What was NOT tested
- **The real CFRU does not run in the test emulator**: Pokémon creation and the bag were simulated. Custom moves (and their PP),
  the item really entering the bag, the "bag full" case, the flag really being saved, and the map music coming back on exit
  can only be confirmed in your game. Suggested check, on a **new test save**: `EEVEE001`, `ITEM0001` (look in the bag),
  `MOVES001` (check the 4 moves), `COMBO001` (two pages), repeat `EEVEE001` ("already claimed"), type `ZZZ` ("Invalid code!"),
  leave with B and check the map music; then save, reload and repeat `EEVEE001` (must still be "already claimed").
- The editor was tested on Linux (Python/Tk 8.6), not on Windows.
- If the editor cannot find the compiler (devkitARM not in the PATH) it warns and writes without the compile check.

## If a code says "Invalid code!"
A code created in the editor can only show that if the ROM you are testing was **not built after saving**.
Run `python tools/check_rom.py YOUR_ROM.gba`: if it says `MISSING`, delete the `build` folder and build again.
