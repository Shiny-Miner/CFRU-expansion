#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Test battery for the editor (no window). Usage:  python test_editor.py <CFRU project root folder>
Works on temporary copies of the project: it does not change yours. Needs arm-none-eabi-gcc for the compile tests.
"""
import json, os, random, re, shutil, sys, tempfile, time
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..'))
import editor_mystery_gift as e

ROOT = sys.argv[1]
ok = fail = 0


def check(cond, name, detail=''):
    global ok, fail
    if cond:
        ok += 1
    else:
        fail += 1
        print('  FAILED:', name, detail)


def section(t):
    print('\n== ' + t)


def project_copy():
    dst = tempfile.mkdtemp(prefix='CFRU project ')
    for sub in ('src', 'include'):
        shutil.copytree(os.path.join(ROOT, sub), os.path.join(dst, sub), ignore=shutil.ignore_patterns('*.o', '*.gba', '__pycache__'))
    return dst


def code(c, slot=0, **kw):
    x = e.new_entry(slot); x['code'] = c; x['species'] = 'EEVEE'; x.update(kw); return x


def data(*codes):
    d = e.default_data(); d['codes'] = list(codes); return d


L = e.load_lists(ROOT)

section('1) lists read from the project (they honor #ifdef)')
check(len(L['species']) > 300 and len(L['items']) > 300 and len(L['moves']) > 300, 'big lists', {k: len(v) for k, v in L.items()})
check('EEVEE' in L['species'] and 'MASTER_BALL' in L['items'] and 'SURF' in L['moves'], 'basic names present')
check(all(re.fullmatch(r'[A-Z0-9_]+', n) for k in L for n in L[k]), 'all names are valid C identifiers')
check(all(len(set(L[k])) == len(L[k]) for k in L), 'no duplicated names')
check(e._eval_if('defined(X) || 1', {}) is True, 'if: defined(X) || 1')
check(e._eval_if('defined(X) && 1', {}) is False, 'if: defined(X) && 1 (X missing)')
check(e._eval_if('A > 2', {'A': 3}) is True, 'if: A > 2')
check(e._eval_if('__import__("os")', {}) is None, 'if: malicious expression is refused')
tmp = tempfile.mkdtemp()
os.makedirs(os.path.join(tmp, 'src')); os.makedirs(os.path.join(tmp, 'include', 'constants'))
open(os.path.join(tmp, 'src', 'config.h'), 'w').write('#define ACTIVE\n//#define COMMENTED\n')
open(os.path.join(tmp, 'include', 'constants', 'items.h'), 'w').write(
    '#define ITEM_A 1\n#ifdef MISSING\n#define ITEM_B 2\n#else\n#define ITEM_C 3\n#endif\n'
    '#ifdef ACTIVE\n#define ITEM_D 4\n#endif\n#ifdef COMMENTED\n#define ITEM_E 5\n#endif\n'
    '#if defined(ACTIVE) && !defined(MISSING)\n#define ITEM_F 6\n#elif 1\n#define ITEM_G 7\n#endif\n'
    '#define ITEM_H ITEM_A\n#define ITEM_I (0x10)\n/* #define ITEM_J 9 */\n#define ITEM_K 11 // comment\n')
r = e._constants(tmp, 'items.h', 'ITEM_')
check(r == ['A', 'C', 'D', 'F', 'K', 'I'], 'preprocessor: ifdef/else/elif/alias/comments (sorted by value)', r)
shutil.rmtree(tmp)

section('2) code validation')
good = ['A', '0', 'Z', '9', 'AB12CD34', '00000000', 'ZZZZZZZZ', '12345678', 'A1B2C3D4', 'EEVEE001', '1', 'ZZ', 'Q9']
for c in good:
    check(e.validate(data(code(c)), L) == [], 'accepts %r' % c)
bad = ['', 'ABCDEFGHI', '123456789', 'abc', 'Abc', 'A B', 'A-B', 'A_B', 'ÇÃO', 'A\n', ' ', 'A.B', 'ＡＢ', '１２３', 'A\u00a0B', '😀']
for c in bad:
    check(e.validate(data(code(c)), L) != [], 'rejects %r' % c)
check(e.validate(data(code('AB', 0), code('AB', 1)), L) != [], 'rejects a duplicated code')
check(e.validate(data(code('AB', 0), code('CD', 0)), L) != [], 'rejects a duplicated slot')
check(e.validate(data(code('AB', 64)), L) != [], 'rejects a slot out of range')
check(e.validate(data(code('AB', -1)), L) != [], 'rejects a negative slot')
check(e.validate(data(code('AB', species='')), L) != [], 'rejects no Pokemon and no item')
check(e.validate(data(code('AB', level=0)), L) != [], 'rejects level 0')
check(e.validate(data(code('AB', level=101)), L) != [], 'rejects level 101')
check(e.validate(data(code('AB', level=1)), L) == [] and e.validate(data(code('AB', level=100)), L) == [], 'accepts levels 1 and 100')
check(e.validate(data(code('AB', species='', reward='MASTER_BALL', qty=0)), L) != [], 'rejects quantity 0')
check(e.validate(data(code('AB', species='', reward='MASTER_BALL', qty=1000)), L) != [], 'rejects quantity 1000')
check(e.validate(data(code('AB', species='', reward='MASTER_BALL', qty=999)), L) == [], 'accepts quantity 999')
check(e.validate(data(code('AB', nature='XXX')), L) != [], 'rejects an invalid nature')
check(all(e.validate(data(code('AB', nature=n)), L) == [] for n in e.NATURES), 'accepts the 25 natures')
check(e.validate(data(code('AB', species='', reward='MASTER_BALL', moves=['SURF', '', '', ''])), L) != [], 'rejects moves without a Pokemon')
check(e.validate(data(code('AB', species='DOESNOTEXIST')), L) != [], 'rejects a nonexistent Pokemon')
check(e.validate(data(code('AB', held='DOESNOTEXIST')), L) != [], 'rejects a nonexistent held item')
check(e.validate(data(code('AB', moves=['DOESNOTEXIST', '', '', ''])), L) != [], 'rejects a nonexistent move')
check(e.validate(data(code('AB', species='', reward='ODDISH_LEAF', qty=1)), L) != [], 'rejects an item from a disabled #ifdef block (ODDISH_LEAF)')
check(len(e.validate(data(*[code('C%d' % i, i) for i in range(65)]), L)) > 0, 'rejects more than 64 codes')
check(e.validate(data(*[code('C%d' % i, i) for i in range(64)]), L) == [], 'accepts exactly 64 codes')
rnd = random.Random(1)
alpha = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
for _ in range(3000):
    c = ''.join(rnd.choice(alpha) for _ in range(rnd.randint(1, 8)))
    assert e.validate(data(code(c)), L) == [], c
ok += 1
for _ in range(3000):
    n = rnd.randint(1, 8)
    c = ''.join(rnd.choice(alpha) for _ in range(n))
    i = rnd.randrange(n)
    c = c[:i] + rnd.choice('abcdefghijklmnopqrstuvwxyz -_.!@#çãé') + c[i + 1:]
    assert e.validate(data(code(c)), L) != [], repr(c)
ok += 1
print('  fuzz: 3000 valid codes accepted, 3000 with one forbidden character rejected')

section('3) generated file (.h)')
d = e.default_data()
h = e.generate_h(d)
check(all(ord(ch) < 128 for ch in h), 'file is 100% ASCII')
check(h.count('.code = "') == len(d['codes']), 'one line per code')
check('MG_FLAG_LANG' not in h, 'no language flags in the file (English-only version)')
tmpf = tempfile.mktemp(suffix='.h'); open(tmpf, 'w').write(h)
d2 = e.import_h(tmpf); os.remove(tmpf)
check(d2['codes'] == d['codes'] and d2['flag_base'] == d['flag_base'] and d2['music'] == d['music'], 'round trip (generate -> import) is identical')
d3 = e.default_data(); d3['music'] = 0
check('MYSTERY_GIFT_MENU_MUSIC 0x0' in e.generate_h(d3), 'music 0 = no music change')
check('MYSTERY_GIFT_FLAG_BASE 0x950' in h, 'flag base in the file')

section('4) writing into the project with the compiler check')
gcc = e.find_gcc()
check(gcc is not None, 'compiler found', 'no arm-none-eabi-gcc: section 4 is skipped')
if gcc:
    P = project_copy()
    cod = os.path.join(P, 'src', 'mystery_gift_codes.h'); csrc = os.path.join(P, 'src', 'mystery_gift.c')
    os.utime(csrc, (1000000000, 1000000000))
    dest, est = e.write_h(e.default_data(), P)
    check(est == 'ok' and os.path.isfile(dest), 'writes and checks: ok')
    check(os.path.getmtime(csrc) > time.time() - 60, 'mystery_gift.c marked for recompilation (new mtime)')
    good_bytes = open(cod, 'rb').read()
    badd = e.default_data(); badd['codes'][0]['species'] = 'DOESNOTEXIST'
    try:
        e.write_h(badd, P); check(False, 'compiler should reject a nonexistent Pokemon')
    except RuntimeError as ex:
        check('NOTHING was written' in str(ex), 'refuses and warns when it does not compile')
    check(open(cod, 'rb').read() == good_bytes, 'old file restored byte for byte after the refusal')
    bad2 = e.default_data(); bad2['codes'][1]['reward'] = 'ODDISH_LEAF'
    try:
        e.write_h(bad2, P); check(False, 'compiler should reject an item from a disabled block')
    except RuntimeError:
        check(True, 'refuses an item from a disabled #ifdef block')
    os.remove(cod)
    try:
        e.write_h(badd, P)
    except RuntimeError:
        pass
    check(not os.path.exists(cod), 'no old file: leaves no garbage after a refusal')
    dest, est = e.write_h(e.default_data(), '"%s/"' % P)
    check(est == 'ok', 'path with spaces + quotes + trailing slash')
    orig = e.find_gcc; e.find_gcc = lambda: None
    dest, est = e.write_h(e.default_data(), P)
    e.find_gcc = orig
    check(est == 'sem_gcc' and os.path.isfile(dest), 'no compiler: writes and warns (state sem_gcc)')
    os.remove(csrc)
    dest, est = e.write_h(e.default_data(), P)
    check(est == 'sem_fonte', 'no mystery_gift.c: warns (state sem_fonte)')
    try:
        e.write_h(e.default_data(), os.path.join(P, 'does_not_exist')); check(False, 'missing folder')
    except RuntimeError:
        check(True, 'missing folder raises a clear error')
    shutil.rmtree(P)

section('5) flags')
def flagerr(base):
    dd = e.default_data(); dd['flag_base'] = base
    return e.validate(dd, L, 'en', ROOT)
check(flagerr(0x950) == [], '0x950 (default) is free')
check(flagerr(0x9C0) == [], '0x9C0 is free (up to 0x9FF)')
conf = flagerr(0x900)
check(conf and 'FLAG_' in conf[0], '0x900 conflicts with CFRU flags', conf)
check(flagerr(0x8FF) != [] and 'not valid' in flagerr(0x8FF)[0], '0x8FF (below 0x900) is refused')
check(e.validate({**e.default_data(), 'flag_base': 0x18C0}, L) == [], '0x18C0 fits (ends at 0x18FF)')
check(e.validate({**e.default_data(), 'flag_base': 0x18C1}, L) != [], '0x18C1 overflows the 0x18FF limit')
check(e.validate({**e.default_data(), 'flag_base': 0}, L) != [], 'flag 0 is refused')
check(e.validate({**e.default_data(), 'flag_base': -5}, L) != [], 'negative flag is refused')
used = e.flags_in_use(ROOT)
check(0x900 in used and 0x950 not in used, 'detects flags in use (0x900 yes, 0x950 no)')

section('6) codes.json (robustness)')
t = tempfile.mkdtemp(); j = os.path.join(t, 'codes.json')
dd, w = e.load_json(j); check(w is None and len(dd['codes']) == 6, 'no file: uses the examples')
open(j, 'w').write('{ this is not json'); dd, w = e.load_json(j)
check(w and os.path.isfile(w) and len(dd['codes']) == 6, 'corrupted: keeps a copy and starts over')
open(j, 'w').write('[1,2,3]'); dd, w = e.load_json(j); check(w is not None, 'wrong json shape is treated as corrupted')
open(j, 'w').write(json.dumps({'codes': [{'code': 'abc', 'species': 'MEW', 'slot': 3}]}))
dd, w = e.load_json(j)
check(w is None and dd['codes'][0]['code'] == 'ABC' and dd['codes'][0]['level'] == 5 and dd['next_slot'] >= 4,
      'old/incomplete file: fills missing fields, uppercases, sets next_slot')
e.save_json(dd, j); check(not os.path.exists(j + '.tmp') and e.load_json(j)[0]['codes'][0]['code'] == 'ABC', 'saving is atomic (no .tmp left)')
shutil.rmtree(t)

section('7) texts')
check(all(isinstance(v, str) and v.strip() for v in e.TR.values()), 'every text is a non-empty string')
src = open(os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'editor_mystery_gift.py'), encoding='utf-8').read()
used_keys = set(re.findall(r"\b(?:L|t)\(\s*'([a-z_0-9A-Z]+)'", src)) | set(re.findall(r"R\([^,]+,\s*'([a-z_0-9]+)'", src)) | set(re.findall(r"\('(rad_[a-z]+)',", src))
missing = [k for k in used_keys if k not in e.TR]
check(not missing, 'every text key used by the program exists', missing)
check(not re.search(r'[ãõçáíóúâêô]', ' '.join(v for k, v in e.TR.items() if k not in ('col_poke', 'tipo_poke', 'rad_poke', 'g_poke', 'r_poke', 'e_poke'))
                    .replace('Pokémon', 'Pokemon').replace('é', 'e')), 'no Portuguese accents left in the texts')
check(all(e.music_name(m) for m in e.MUSIC_IDS), 'music names exist')
check(len({e.music_name(m) for m in e.MUSIC_IDS}) == len(e.MUSIC_IDS), 'music names are distinct (the window finds the music by its name)')
check(not hasattr(e, 'IDIOMAS'), 'there is no language selector in this version')

print('\nRESULT: %d checks OK, %d FAILED' % (ok, fail))
sys.exit(1 if fail else 0)
