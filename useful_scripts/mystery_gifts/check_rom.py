#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Checks, in your ALREADY BUILT ROM (.gba), that every code from the editor really made it into the game.
The codes are stored in the ROM as plain text; a code reported as MISSING means the build did not include the new list
(delete the build folder and build again) and that code would show "Invalid code!" in the game.

  python check_rom.py  YOUR_ROM.gba  [codes.json]
"""
import json
import os
import sys


def main(argv):
    if not argv:
        print(__doc__)
        return 1
    rom = open(argv[0], 'rb').read()
    js = argv[1] if len(argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), 'codes.json')
    d = json.load(open(js, encoding='utf-8'))
    missing = []
    for c in d['codes']:
        code = c['code']
        found = (code.encode('ascii') + b'\x00') in rom
        print('%-9s flag 0x%X : %s' % (code, d['flag_base'] + c['slot'], 'OK (it is in the ROM)' if found else 'MISSING  <-- would show "Invalid code!"'))
        if not found:
            missing.append(code)
    if missing:
        print('\n%d code(s) MISSING: %s\nDelete the build folder, build again and run this check once more.' % (len(missing), ', '.join(missing)))
        return 1
    print('\nALL GOOD: the %d codes are inside the ROM.' % len(d['codes']))
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
