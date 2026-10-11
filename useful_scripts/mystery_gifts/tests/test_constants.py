#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Compiles (syntax check only) EVERY Pokemon, item and move that the editor lists, in batches of 64 codes, using the editor's
own .h generator and the real headers of your project. Any name that would break the build shows up here.

  python test_constants.py <CFRU project root folder>
Needs arm-none-eabi-gcc in the PATH (devkitARM).
"""
import os, subprocess, sys, tempfile
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import editor_mystery_gift as e

raiz = sys.argv[1]
L = e.load_lists(raiz)
n = max(len(L['species']), len(L['items']), len(L['moves']))
CF = ['-mthumb', '-mcpu=arm7tdmi', '-march=armv4t', '-fsyntax-only', '-w']
falhas = []
lotes = (n + 63) // 64
for b in range(lotes):
    cods = []
    for k in range(64):
        i = b * 64 + k
        if i >= n:
            break
        c = e.new_entry(k)
        c['code'] = 'T%02d' % k
        c['species'] = L['species'][i % len(L['species'])]
        c['held'] = L['items'][i % len(L['items'])]
        c['reward'] = L['items'][(i * 7 + 3) % len(L['items'])]
        c['moves'] = [L['moves'][(i + j * 251) % len(L['moves'])] for j in range(4)]
        c['moves'][0] = L['moves'][i % len(L['moves'])]
        cods.append(c)
    d = e.default_data(); d['codes'] = cods
    pasta = tempfile.mkdtemp(dir=os.path.join(raiz, 'src'))
    try:
        open(os.path.join(pasta, 'mystery_gift_codes.h'), 'w').write(e.generate_h(d))
        fonte = os.path.join(pasta, 't.c')
        open(fonte, 'w').write('#include "../defines.h"\n#include "../../include/constants/moves.h"\n'
                               '#include "../../include/constants/items.h"\n#include "mystery_gift_codes.h"\n'
                               'const struct MysteryGiftEntry* f(void){return sMysteryGifts;}\n')
        r = subprocess.run(['arm-none-eabi-gcc'] + CF + [fonte], capture_output=True, text=True, cwd=os.path.join(raiz, 'src'))
        if r.returncode != 0:
            falhas.append((b, r.stderr[:400]))
    finally:
        for f in os.listdir(pasta): os.remove(os.path.join(pasta, f))
        os.rmdir(pasta)
print('batches compiled: %d  (%d Pokemon, %d items, %d moves)' % (lotes, len(L['species']), len(L['items']), len(L['moves'])))
if falhas:
    print('FAILURES:', len(falhas))
    for b, m in falhas[:5]: print(' batch', b, m)
    sys.exit(1)
print('OK: every name listed by the editor compiles.')
