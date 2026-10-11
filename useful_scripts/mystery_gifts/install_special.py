#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Registers the Mystery Gift  special 0x122  in your project WITHOUT you editing anything and WITHOUT replacing your file.

  python install_special.py  <CFRU project root folder>             install
  python install_special.py  <CFRU project root folder> --remove    undo

What it does: adds TWO lines ("MysteryGift_Start 081601E8" and a comment) to the project's  routinepointers  file,
right after the line of special 0x12D. Before that it makes a backup copy (routinepointers.bak_mysterygift) and checks
that the 0x122 slot is not in use. Running it twice does not duplicate anything.
"""
import os
import re
import shutil
import sys

NAME = 'MysteryGift_Start'
ADDRESS = '081601E8'          # 0x0815FD60 + 4 * 0x122
ANCHOR = 'sp12D_CompareEnteredPhrase'
COMMENT = '##^ Mystery Gift: special 0x122'


def main(argv):
    if not argv:
        print(__doc__)
        return 1
    root = argv[0]
    path = os.path.join(root, 'routinepointers')
    if not os.path.isfile(path):
        print('ERROR: could not find "%s". Pass the CFRU project root folder (the one with src, include and routinepointers).' % path)
        return 1
    with open(path, encoding='utf-8', errors='ignore', newline='') as f:
        text = f.read()
    nl = '\r\n' if '\r\n' in text else '\n'
    lines = text.split(nl)

    if '--remove' in argv:
        new = [l for l in lines if not l.startswith(NAME + ' ') and not l.startswith(COMMENT)]
        if len(new) == len(lines):
            print('Nothing to remove: the Mystery Gift was not registered.')
            return 0
        with open(path, 'w', encoding='utf-8', newline='') as f:
            f.write(nl.join(new))
        print('Removed. (The backup routinepointers.bak_mysterygift is still there if you need it.)')
        return 0

    if any(l.startswith(NAME + ' ') for l in lines):
        print('Already installed: the Mystery Gift line is already in routinepointers. Nothing to do.')
        return 0
    in_use = [l for l in lines if re.search(r'(?<![0-9A-Fa-f])0?' + ADDRESS + r'(?![0-9A-Fa-f])', l, re.I)]
    if in_use:
        print('ERROR: the address of special 0x122 is already used in your routinepointers:\n  ' + in_use[0])
        print('Nothing was changed. Another special number has to be chosen.')
        return 1
    pos = next((i for i, l in enumerate(lines) if l.startswith(ANCHOR)), None)
    if pos is None:
        print('ERROR: could not find the line "%s" in your routinepointers (different file than expected). Nothing was changed.' % ANCHOR)
        return 1
    shutil.copyfile(path, path + '.bak_mysterygift')
    lines[pos + 1:pos + 1] = ['%s %s' % (NAME, ADDRESS), '%s (0x0815FD60 + 4*0x122)' % COMMENT]
    with open(path, 'w', encoding='utf-8', newline='') as f:
        f.write(nl.join(lines))
    print('Done! special 0x122 registered. Backup: routinepointers.bak_mysterygift')
    print('Now delete the build folder and build again.')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
