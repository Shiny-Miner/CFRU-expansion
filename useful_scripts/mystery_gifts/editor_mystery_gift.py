#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
MYSTERY GIFT EDITOR - edit codes, Pokemon, moves and items without touching C code.

  1. Double-click OPEN_EDITOR.bat (or run: python editor_mystery_gift.py)
  2. Click "Browse..." and choose the project ROOT folder of your CFRU project (the one that has src and include).
  3. Create/edit the codes and click "SAVE AND GENERATE".
  4. Build the project again (python scripts/make.py).

FILES THIS PROGRAM TOUCHES
  tools/codes.json                      your codes (created/updated when you save)
  <project>/src/mystery_gift_codes.h    WRITTEN (and only this one) - the list of codes that goes into the game
  <project>/src/mystery_gift.c          only its modification DATE is updated ("touch") so the build recompiles it
  <project>/include/constants/*.h       only READ (Pokemon, item and move names) - never changed
  <project>/src/config.h, include/...   only READ (active macros and flags in use)
Before writing, the program COMPILES the result with your compiler (arm-none-eabi-gcc); if it does not compile, nothing is written.

Without a window (optional):
  python editor_mystery_gift.py --generate [--project FOLDER]
  python editor_mystery_gift.py --import FILE.h
"""
import json
import os
import re
import shutil
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
JSON_FILE = os.path.join(HERE, 'codes.json')

MAX_CODES = 64              # maximum number of codes (each one uses 1 flag, forever)
CODE_LENGTH = 8             # maximum length of the typed code (slots on the screen)
DEFAULT_FLAG = 0x950
FLAG_MIN = 0x900            # CFRU only saves flags 0x900 to 0x18FF (expanded block)
FLAG_MAX = 0x18FF

NATURES = ['HARDY', 'LONELY', 'BRAVE', 'ADAMANT', 'NAUGHTY', 'BOLD', 'DOCILE', 'RELAXED', 'IMPISH', 'LAX', 'TIMID',
           'HASTY', 'SERIOUS', 'JOLLY', 'NAIVE', 'MODEST', 'MILD', 'QUIET', 'BASHFUL', 'RASH', 'CALM', 'GENTLE',
           'SASSY', 'CAREFUL', 'QUIRKY']
RANDOM_NATURE = '(random)'   # internal value (stored in the JSON)

# ------------------------------------------------------------------------------------------------ texts (English only)
TR = {
    'titulo': 'Mystery Gift Editor',
    'proj_frame': ' Project and music ',
    'pasta_proj': 'CFRU project folder:',
    'procurar': 'Browse...',
    'music_label': 'Menu music:',
    'flag_ini': 'Starting flag:',
    'lista_frame': ' Codes ',
    'col_codigo': 'Code',
    'col_flag': 'Flag',
    'col_tipo': 'Type',
    'col_poke': 'Pokémon',
    'col_item': 'Item',
    'tipo_poke': 'Pokémon',
    'tipo_item': 'Item',
    'tipo_ambos': 'Poke+Item',
    'form_frame': ' Edit the selected code ',
    'cod_label': 'Code (A-Z and 0-9, up to 8):',
    'presente_e': 'The gift is:',
    'rad_poke': 'Pokémon',
    'rad_item': 'Item',
    'rad_ambos': 'Pokémon + Item',
    'g_poke': ' Pokémon ',
    'especie': 'Species:',
    'nivel': 'Level:',
    'shiny': 'Shiny',
    'ha': 'Hidden ability',
    'segura': 'Holds item:',
    'nature': 'Nature:',
    'aleatoria': '(random)',
    'golpes_dica': 'Moves (leave empty for the normal moves of the level):',
    'golpe_n': 'Move {n}:',
    'g_item': ' Gift item (goes to the bag) ',
    'item_lbl': 'Item:',
    'qtd': 'Quantity:',
    'novo': 'New',
    'duplicar': 'Duplicate',
    'excluir': 'Delete',
    'aplicar': 'Apply changes',
    'importar': 'Import old .h...',
    'salvar': 'SAVE AND GENERATE  (writes src/mystery_gift_codes.h)',
    'dica': 'Tip: each code keeps a fixed flag, so you can delete/reorder without freeing codes already redeemed.',
    'flag_deste': "This code's flag: {f}   (to test again: clearflag {f})",
    'mus_0x111': 'Celadon Game Corner (lively, prize corner)',
    'mus_0x135': 'Prize room / Celadon City',
    'mus_0x12F': 'Pokémon Center (calm)',
    'mus_0x12D': "Prof. Oak's Lab",
    'mus_0x12C': 'Pallet Town',
    'mus_0x134': 'Cerulean City / Fuchsia City',
    'mus_0x139': 'Vermilion City',
    'mus_0x13A': 'Viridian / Pewter / Saffron City',
    'mus_0x133': 'Silph Co.',
    'mus_0x14F': 'Sevii Islands (1, 2 and 3)',
    'mus_0x151': 'Sevii Islands (7 and 6)',
    'mus_0x127': 'Indigo Plateau',
    'mus_0': 'None (keep the map music)',
    'mus_outra': 'Other (0x{id:X})',
    'st_sem_proj': 'Choose the CFRU project folder (Browse... button) so the editor can check the names and generate the file.',
    'st_sem_const': 'Could not find include/constants/species.h inside that folder. Choose the CFRU project root folder.',
    'st_proj_lido': 'Project read: {a} Pokémon, {b} items, {c} moves.',
    'st_novo': 'New code created. Fill it in and click "Apply".',
    'st_copia': 'Copy created: choose a new code for it.',
    'st_excluido': 'Code deleted.',
    'st_aplicado': 'Changes applied to code {c}. Click "SAVE AND GENERATE" when you are done.',
    'st_gerado': 'Done! Generated: {d}  -> now just compile again. {v}',
    'st_importados': 'Imported {n} codes.',
    'dlg_limite': 'All {n} flag numbers have been used (each code keeps its own forever, even if deleted, so nobody who already redeemed is ever freed). No more codes can be created.',
    'dlg_excluir_t': 'Delete',
    'dlg_excluir': 'Delete code {c}?\n\nIts flag (slot {s}) stays reserved and will NOT be reused, so nobody who already redeemed it is freed.',
    'dlg_vazio': '(empty)',
    'dlg_erro': 'Error',
    'dlg_corrija': 'Fix the selected code before saving (see the red message).',
    'dlg_flag_num': 'The starting flag must be a number, for example 0x950.',
    'dlg_nao_gerar': 'Could not generate',
    'dlg_salvei_json': 'I saved codes.json, but you still need to choose the CFRU project folder to write mystery_gift_codes.h.',
    'dlg_escolha_proj': 'Choose the project',
    'dlg_erro_escrever': 'Error while writing',
    'dlg_pronto_t': 'Done',
    'dlg_pronto': 'File generated:\n{d}\n\n{v}\n\nNow just compile again (python scripts/make.py).\nIf a new code still shows as invalid, delete the "build" folder and compile again.',
    'dlg_pasta_titulo': 'Choose the CFRU project root folder (the one with src and include)',
    'dlg_imp_titulo': 'Choose a mystery_gift_codes.h',
    'dlg_imp_erro': 'Could not read it: {e}',
    'dlg_imp_vazio': 'I did not find any code in that file.',
    'dlg_json_corrompido': 'codes.json was corrupted. I kept a copy at "{b}" and started with the examples.',
    'ver_ok': 'Checked with the compiler: OK.',
    'ver_sem_gcc': 'WARNING: I could not find the compiler (arm-none-eabi-gcc); I could not check that the file compiles.',
    'ver_sem_fonte': 'WARNING: src/mystery_gift.c was not found in the project (did you copy the package "projeto" folder?); not checked.',
    'ver_falhou': 'The compiler rejected the result. NOTHING was written.\n{e}',
    'e_nenhum': 'There are no codes.',
    'e_max': 'Maximum of {m} codes (you have {n}).',
    'e_codigo': '{c}: the code must have 1 to {t} characters, only letters A-Z and numbers 0-9.',
    'e_repetido': '{c}: duplicated code.',
    'e_slot_rep': '{c}: duplicated flag number (slot {s}).',
    'e_slot_fora': '{c}: slot out of range 0..{m}.',
    'e_sem_presente': '{c}: choose a Pokémon and/or a gift item.',
    'e_nivel': '{c}: the level must be from 1 to 100.',
    'e_qtd': '{c}: the item quantity must be from 1 to 999.',
    'e_nature': '{c}: invalid nature.',
    'e_nao_existe': '{c}: {r} "{v}" does not exist in the project (or sits in a disabled block of the code).',
    'r_poke': 'Pokémon',
    'r_held': 'held item',
    'r_reward': 'gift item',
    'r_move': 'move',
    'e_golpes_sem_poke': '{c}: moves only apply when there is a Pokémon.',
    'e_flag_faixa': 'Starting flag 0x{b:X} is not valid: CFRU only saves flags 0x{lo:X} to 0x{hi:X}, and the block uses 0x{b:X} to 0x{fim:X}.',
    'e_flag_conflito': 'FLAG CONFLICT: block 0x{b:X}-0x{fim:X} includes flags the project already uses ({l}). Choose another starting flag (e.g. 0x{s:X}).',
}


def t(chave, lang=None, **kw):
    txt = TR[chave]
    return txt.format(**kw) if kw else txt


# ------------------------------------------------------------------------------------------------ music (ids read from the ROM's map headers)
MUSIC_IDS = [0x111, 0x135, 0x12F, 0x12D, 0x12C, 0x134, 0x139, 0x13A, 0x133, 0x14F, 0x151, 0x127, 0]


def music_name(mid, lang='en'):
    chave = 'mus_0' if mid == 0 else 'mus_0x%X' % mid
    return t(chave, lang) if chave in TR else t('mus_outra', lang, id=mid)


# ------------------------------------------------------------------------------------------------ mini preprocessor (only used to read constants)
def _eval_if(expr, defs):
    """evaluates '#if EXPR'. Returns True/False, or None if it cannot be known."""
    expr = re.sub(r'/\*.*?\*/', '', expr)
    expr = re.sub(r'//.*', '', expr).strip()
    expr = re.sub(r'defined\s*\(\s*(\w+)\s*\)', lambda m: '1' if m.group(1) in defs else '0', expr)
    expr = re.sub(r'defined\s+(\w+)', lambda m: '1' if m.group(1) in defs else '0', expr)

    def ident(m):
        v = defs.get(m.group(0))
        return str(v) if isinstance(v, int) and not isinstance(v, bool) else '0'
    expr = re.sub(r'\b[A-Za-z_]\w*\b', ident, expr)
    expr = expr.replace('&&', ' and ').replace('||', ' or ')
    expr = re.sub(r'!(?!=)', ' not ', expr)
    toks = re.findall(r'0[xX][0-9a-fA-F]+|\d+|and|or|not|==|!=|<=|>=|<|>|\(|\)|\+|-|\*|%', expr)
    if re.sub(r'\s+', '', ''.join(toks)) != re.sub(r'\s+', '', expr):
        return None
    try:
        return bool(eval(' '.join(toks), {'__builtins__': {}}, {}))
    except Exception:
        return None


def _scan(arq, defs, prefixo=None):
    """reads a .h honoring #if/#ifdef/#ifndef/#else/#elif/#endif. Updates 'defs'. Returns {name: value} of the active constants."""
    achados = {}
    if not os.path.isfile(arq):
        return achados
    with open(arq, encoding='utf-8', errors='ignore') as f:
        texto = f.read()
    texto = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group(0).count('\n'), texto, flags=re.S)
    linhas, acum = [], ''
    for l in texto.split('\n'):
        if l.rstrip().endswith('\\'):
            acum += l.rstrip()[:-1] + ' '
            continue
        linhas.append(acum + l)
        acum = ''
    pilha = []   # (parent_active, active_now, some_branch_already_active)
    ativo = True
    for l in linhas:
        m = re.match(r'\s*#\s*(\w+)\s*(.*)$', l)
        if not m:
            continue
        d, resto = m.group(1), re.sub(r'//.*', '', m.group(2)).strip()
        if d in ('ifdef', 'ifndef', 'if'):
            if d == 'if':
                v = _eval_if(resto, defs)
                v = True if v is None else v      # uncertain: counts as active (the compiler checks at the end)
            else:
                nome = resto.split()[0] if resto.split() else ''
                v = (nome in defs) if d == 'ifdef' else (nome not in defs)
            pilha.append((ativo, v, v))
            ativo = ativo and v
        elif d == 'elif' and pilha:
            pai, _, ja = pilha.pop()
            v = False if ja else (_eval_if(resto, defs) in (True, None))
            pilha.append((pai, v, ja or v))
            ativo = pai and v
        elif d == 'else' and pilha:
            pai, _, ja = pilha.pop()
            pilha.append((pai, not ja, True))
            ativo = pai and not ja
        elif d == 'endif' and pilha:
            pai, _, _ = pilha.pop()
            ativo = pai
        elif ativo and d == 'define':
            mm = re.match(r'(\w+)(?:\s+(.*))?$', resto)
            if not mm:
                continue
            nome, val = mm.group(1), (mm.group(2) or '').strip()
            if re.fullmatch(r'\(?\s*(0[xX][0-9a-fA-F]+|\d+)\s*\)?', val):
                num = int(re.sub(r'[()\s]', '', val), 0)
                defs[nome] = num
                if prefixo and nome.startswith(prefixo):
                    achados[nome] = num
            else:
                defs[nome] = True if val == '' else val
                achados.pop(nome, None)         # became an alias: not a numeric constant
        elif ativo and d == 'undef':
            nome = resto.split()[0] if resto.split() else ''
            defs.pop(nome, None)
            achados.pop(nome, None)
    return achados


def _constants(raiz, arq, prefixo, excluir=()):
    base = {}
    _scan(os.path.join(raiz, 'src', 'config.h'), base)          # project macros (e.g. UNBOUND not defined)
    achados = _scan(os.path.join(raiz, 'include', 'constants', arq), dict(base), prefixo)
    out = []
    for nome, val in sorted(achados.items(), key=lambda kv: (kv[1], kv[0])):
        curto = nome[len(prefixo):]
        if any(x in nome for x in excluir):
            continue
        if curto in ('NONE', 'COUNT', 'EGG') or curto.endswith('_COUNT') or curto.startswith('NUM_'):
            continue
        if not re.fullmatch(r'[A-Z0-9_]+', curto):
            continue
        out.append(curto)
    return out


def clean_path(p):
    return (p or '').strip().strip('"').strip("'").strip()


def load_lists(raiz):
    raiz = clean_path(raiz)
    return {
        'species': _constants(raiz, 'species.h', 'SPECIES_'),
        'items': _constants(raiz, 'items.h', 'ITEM_', ('ITEM_EFFECT_', 'FREE_SPACE', 'SHINY_SPACE', 'ITEM_TM_CASE_')),
        'moves': _constants(raiz, 'moves.h', 'MOVE_', ('MOVE_EFFECT_',)),
    }


def flags_in_use(raiz):
    """{value: 'FLAG_X'} of the flags 0x900-0x18FF defined by the project (any numeric #define FLAG_)"""
    raiz = clean_path(raiz)
    usadas = {}
    for p in (os.path.join(raiz, 'include'), os.path.join(raiz, 'src')):
        for dp, _, fs in os.walk(p):
            for fn in fs:
                if not fn.endswith('.h') or 'mystery_gift' in fn:
                    continue
                try:
                    txt = open(os.path.join(dp, fn), encoding='utf-8', errors='ignore').read()
                except OSError:
                    continue
                for m in re.finditer(r'#\s*define\s+(FLAG_\w+)\s+\(?\s*(0[xX][0-9a-fA-F]+|\d+)\s*\)?', txt):
                    v = int(m.group(2), 0)
                    if FLAG_MIN <= v <= FLAG_MAX:
                        usadas.setdefault(v, m.group(1))
    return usadas


# ------------------------------------------------------------------------------------------------ data
def new_entry(slot):
    return {'code': '', 'species': '', 'level': 5, 'shiny': False, 'ha': False, 'held': '', 'nature': RANDOM_NATURE,
            'moves': ['', '', '', ''], 'reward': '', 'qty': 1, 'slot': slot}


def default_data():
    def e(code, slot, **kw):
        x = new_entry(slot); x['code'] = code; x.update(kw); return x
    return {
        'project': '', 'flag_base': DEFAULT_FLAG, 'music': 0x111, 'next_slot': 6,
        'codes': [
            e('EEVEE001', 0, species='EEVEE', level=10),
            e('DRATINI', 1, species='DRATINI', level=15, shiny=True),
            e('MEWGIFT', 2, species='MEW', level=30),
            e('MOVES001', 3, species='PIKACHU', level=25, held='LIGHT_BALL',
              moves=['THUNDERBOLT', 'QUICKATTACK', 'IRONTAIL', 'SURF']),
            e('ITEM0001', 4, reward='MASTER_BALL', qty=5),
            e('COMBO001', 5, species='CHARMANDER', level=15, nature='ADAMANT',
              moves=['EMBER', 'SCRATCH', 'SLASH', ''], reward='RARE_CANDY', qty=10),
        ],
    }


def _complete(d):
    base = default_data()
    for k in ('project', 'flag_base', 'music', 'next_slot'):
        d.setdefault(k, base[k])
    d.setdefault('codes', [])
    for c in d['codes']:
        for k, v in new_entry(0).items():
            c.setdefault(k, v)
        c['code'] = str(c['code']).strip().upper()
        c['moves'] = (list(c['moves']) + ['', '', '', ''])[:4]
    usados = [c['slot'] for c in d['codes']]
    if usados:
        d['next_slot'] = max(d['next_slot'], max(usados) + 1)
    return d


def load_json(caminho=None):
    """reads codes.json. If it is corrupted: keeps a copy and starts over with the examples. Returns (data, warning|None)."""
    caminho = caminho or JSON_FILE
    if not os.path.isfile(caminho):
        return default_data(), None
    try:
        with open(caminho, encoding='utf-8') as f:
            d = json.load(f)
        if not isinstance(d, dict):
            raise ValueError('formato inesperado')
        return _complete(d), None
    except Exception:
        bkp = caminho + '.corrompido'
        n = 1
        while os.path.exists(bkp):
            n += 1
            bkp = '%s.corrompido%d' % (caminho, n)
        try:
            shutil.copyfile(caminho, bkp)
        except OSError:
            pass
        return default_data(), bkp


def save_json(d, caminho=None):
    caminho = caminho or JSON_FILE
    tmp = caminho + '.tmp'
    with open(tmp, 'w', encoding='utf-8') as f:
        json.dump(d, f, ensure_ascii=False, indent=2)
    os.replace(tmp, caminho)          # atomic replace: never leaves a half-written file


# ------------------------------------------------------------------------------------------------ validation
def validate(d, listas=None, lang='en', raiz=None):
    """returns the list of error messages (empty = everything is fine)"""
    erros = []
    cods = d['codes']
    if 'flag_base' in d:
        b = d['flag_base']
        fim = b + MAX_CODES - 1
        if not (FLAG_MIN <= b and fim <= FLAG_MAX):
            erros.append(t('e_flag_faixa', lang, b=b, lo=FLAG_MIN, hi=FLAG_MAX, fim=fim))
        elif raiz:
            usadas = flags_in_use(raiz)
            conf = sorted(v for v in usadas if b <= v <= fim)
            if conf:
                sugestao = next((x for x in range(FLAG_MIN, FLAG_MAX - MAX_CODES, 16)
                                 if not any(x <= v < x + MAX_CODES for v in usadas)), FLAG_MIN)
                lista = ', '.join('%s=0x%X' % (usadas[v], v) for v in conf[:4]) + (' ...' if len(conf) > 4 else '')
                erros.append(t('e_flag_conflito', lang, b=b, fim=fim, l=lista, s=sugestao))
        if not cods:
            erros.append(t('e_nenhum', lang))
    if len(cods) > MAX_CODES:
        erros.append(t('e_max', lang, m=MAX_CODES, n=len(cods)))
    vistos, slots = {}, {}
    for i, c in enumerate(cods, 1):
        nome = c['code'] or '(%d)' % i
        if not re.fullmatch(r'[A-Z0-9]{1,%d}' % CODE_LENGTH, c['code']):
            erros.append(t('e_codigo', lang, c=nome, t=CODE_LENGTH))
        if c['code'] in vistos:
            erros.append(t('e_repetido', lang, c=nome))
        vistos[c['code']] = True
        if c['slot'] in slots:
            erros.append(t('e_slot_rep', lang, c=nome, s=c['slot']))
        slots[c['slot']] = True
        if not (0 <= c['slot'] < MAX_CODES):
            erros.append(t('e_slot_fora', lang, c=nome, m=MAX_CODES - 1))
        if not c['species'] and not c['reward']:
            erros.append(t('e_sem_presente', lang, c=nome))
        if c['species'] and not (1 <= int(c['level']) <= 100):
            erros.append(t('e_nivel', lang, c=nome))
        if c['reward'] and not (1 <= int(c['qty']) <= 999):
            erros.append(t('e_qtd', lang, c=nome))
        if c['nature'] != RANDOM_NATURE and c['nature'] not in NATURES:
            erros.append(t('e_nature', lang, c=nome))
        if listas:
            def existe(chave, valor, rotulo):
                if valor and listas[chave] and valor not in listas[chave]:
                    erros.append(t('e_nao_existe', lang, c=nome, r=t(rotulo, lang), v=valor))
            existe('species', c['species'], 'r_poke')
            existe('items', c['held'], 'r_held')
            existe('items', c['reward'], 'r_reward')
            for m in c['moves']:
                existe('moves', m, 'r_move')
        if not c['species'] and any(c['moves']):
            erros.append(t('e_golpes_sem_poke', lang, c=nome))
    return erros


# ------------------------------------------------------------------------------------------------ generate the .h (ASCII only)
def generate_h(d):
    L = []
    L.append('/*')
    L.append(' * FILE GENERATED by editor_mystery_gift.py  -  DO NOT EDIT BY HAND (use the editor).')
    L.append(' * The codes are stored in codes.json next to the editor.')
    L.append(' */')
    L.append('')
    L.append('//First flag used. The code with number N uses the flag MYSTERY_GIFT_FLAG_BASE + N.')
    L.append('//Flags 0x%X to 0x%X are reserved for the Mystery Gift.' % (d['flag_base'], d['flag_base'] + MAX_CODES - 1))
    L.append('#define MYSTERY_GIFT_FLAG_BASE 0x%X' % d['flag_base'])
    L.append('#define MYSTERY_GIFT_MAX_CODES %d' % MAX_CODES)
    L.append('')
    L.append('//Menu music (0 = keep the map music)')
    L.append('#define MYSTERY_GIFT_MENU_MUSIC 0x%X' % d['music'])
    L.append('')
    L.append('struct MysteryGiftEntry')
    L.append('{')
    L.append('\tconst char* code;')
    L.append('\tu8 slot;\t\t\t//Flag number of this code (never changes)')
    L.append('\tu16 species;\t\t//SPECIES_NONE = item-only gift')
    L.append('\tu8 level;')
    L.append('\tu8 shiny;')
    L.append('\tu8 hiddenAbility;')
    L.append('\tu8 nature;\t\t\t//0xFF = random')
    L.append('\tu16 heldItem;\t\t//Item the Pokemon holds')
    L.append('\tu16 moves[4];\t\t//All MOVE_NONE = normal moves for the level')
    L.append('\tu16 rewardItem;\t\t//Item given to the bag (ITEM_NONE = none)')
    L.append('\tu16 rewardQty;')
    L.append('};')
    L.append('')
    L.append('static const struct MysteryGiftEntry sMysteryGifts[] =')
    L.append('{')
    for c in d['codes']:
        sp = 'SPECIES_' + c['species'] if c['species'] else 'SPECIES_NONE'
        held = 'ITEM_' + c['held'] if c['held'] else 'ITEM_NONE'
        rw = 'ITEM_' + c['reward'] if c['reward'] else 'ITEM_NONE'
        nat = 'NATURE_' + c['nature'] if c['nature'] != RANDOM_NATURE else '0xFF'
        mv = ', '.join(('MOVE_' + m) if m else 'MOVE_NONE' for m in c['moves'])
        L.append('\t{ .code = "%s", .slot = %d, .species = %s, .level = %d, .shiny = %s, .hiddenAbility = %s, .nature = %s,'
                 % (c['code'], c['slot'], sp, int(c['level']), 'TRUE' if c['shiny'] else 'FALSE',
                    'TRUE' if c['ha'] else 'FALSE', nat))
        L.append('\t  .heldItem = %s, .moves = {%s}, .rewardItem = %s, .rewardQty = %d },'
                 % (held, mv, rw, int(c['qty']) if c['reward'] else 0))
    L.append('};')
    L.append('')
    dados = json.dumps(d, ensure_ascii=True, separators=(',', ':'))
    L.append('/* MG_EDITOR_DATA ' + dados.replace('*/', '* /') + ' */')
    L.append('')
    txt = '\n'.join(L)
    txt.encode('ascii')                 # guarantees the file is 100% ASCII (no encoding problems on Windows)
    return txt


# ------------------------------------------------------------------------------------------------ check with the compiler
def find_gcc():
    cands = [shutil.which('arm-none-eabi-gcc'), shutil.which('arm-none-eabi-gcc.exe')]
    dk = os.environ.get('DEVKITARM')
    if dk:
        cands += [os.path.join(dk, 'bin', 'arm-none-eabi-gcc'), os.path.join(dk, 'bin', 'arm-none-eabi-gcc.exe')]
    cands += [r'C:\devkitPro\devkitARM\bin\arm-none-eabi-gcc.exe', '/opt/devkitpro/devkitARM/bin/arm-none-eabi-gcc']
    return next((c for c in cands if c and os.path.isfile(c)), None)


def compile_check(raiz, gcc):
    """syntax check of src/mystery_gift.c (which includes the .h). Returns (ok, error_text)"""
    cmd = [gcc, '-mthumb', '-mcpu=arm7tdmi', '-march=armv4t', '-fsyntax-only', '-w', os.path.join('src', 'mystery_gift.c')]
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, cwd=raiz, timeout=120)
    except subprocess.TimeoutExpired:
        return False, 'timeout'
    if r.returncode == 0:
        return True, ''
    linhas = [l for l in (r.stderr or r.stdout).splitlines() if 'error' in l]
    return False, '\n'.join(linhas[:6]) or (r.stderr or r.stdout)[:600]


def write_h(d, raiz, lang='en'):
    """Writes <root>/src/mystery_gift_codes.h ONLY if the result compiles. Returns (path, state) where state is
    'ok' | 'sem_gcc' (no compiler found) | 'sem_fonte' (src/mystery_gift.c missing). If the compiler rejects it,
    raises RuntimeError and restores the old file."""
    raiz = clean_path(raiz)
    pasta = os.path.join(raiz, 'src')
    if not os.path.isdir(pasta):
        raise RuntimeError('"%s" does not exist.' % pasta)
    destino = os.path.join(pasta, 'mystery_gift_codes.h')
    fonte = os.path.join(pasta, 'mystery_gift.c')
    texto = generate_h(d)
    antigo = None
    if os.path.isfile(destino):
        with open(destino, 'rb') as f:
            antigo = f.read()
    with open(destino, 'w', encoding='ascii', newline='\n') as f:
        f.write(texto)
    estado = 'sem_fonte'
    if os.path.isfile(fonte):
        gcc = find_gcc()
        if gcc:
            ok, erro = compile_check(raiz, gcc)
            if not ok:
                if antigo is None:
                    os.remove(destino)
                else:
                    with open(destino, 'wb') as f:
                        f.write(antigo)
                raise RuntimeError(t('ver_falhou', lang, e=erro))
            estado = 'ok'
        else:
            estado = 'sem_gcc'
        # The CFRU build only recompiles a .c when the .c ITSELF is newer than its object file (it does not track .h files).
        os.utime(fonte, None)
    return destino, estado


def state_text(estado, lang):
    return {'ok': t('ver_ok', lang), 'sem_gcc': t('ver_sem_gcc', lang), 'sem_fonte': t('ver_sem_fonte', lang)}[estado]


# ------------------------------------------------------------------------------------------------ import an old/new .h
def import_h(caminho):
    with open(caminho, encoding='utf-8', errors='ignore') as f:
        txt = f.read()
    m = re.search(r'/\* MG_EDITOR_DATA (.*?) \*/', txt, re.S)
    if m:
        return _complete(json.loads(m.group(1)))
    d = default_data(); d['codes'] = []
    base = re.search(r'#define\s+MYSTERY_GIFT_FLAG_BASE\s+(0x[0-9A-Fa-f]+|\d+)', txt)
    if base:
        d['flag_base'] = int(base.group(1), 0)
    pad = re.compile(r'\{\s*"([A-Za-z0-9]+)"\s*,\s*SPECIES_(\w+)\s*,\s*(\d+)\s*,\s*(TRUE|FALSE)\s*,\s*(TRUE|FALSE)\s*,'
                     r'\s*ITEM_(\w+)\s*,\s*(\w+)\s*\}')
    for i, mm in enumerate(pad.finditer(txt)):
        e = new_entry(i)
        e.update(code=mm.group(1).upper(), species=mm.group(2), level=int(mm.group(3)), shiny=mm.group(4) == 'TRUE',
                 ha=mm.group(5) == 'TRUE', held='' if mm.group(6) == 'NONE' else mm.group(6),
                 nature=RANDOM_NATURE if not mm.group(7).startswith('NATURE_') else mm.group(7)[7:])
        d['codes'].append(e)
    d['next_slot'] = len(d['codes'])
    return d


# ------------------------------------------------------------------------------------------------ interface (tkinter)
def open_window():
    import tkinter as tk
    from tkinter import ttk, filedialog, messagebox

    d, aviso_json = load_json()
    estado = {'listas': {'species': [], 'items': [], 'moves': []}, 'sel': None, 'bloq': False, 'lang': 'en'}
    L = lambda k, **kw: t(k, estado['lang'], **kw)

    root = tk.Tk()
    try:
        ttk.Style().theme_use('clam')
    except tk.TclError:
        pass
    reg = []                                  # (widget, key, option, kwargs)

    def R(widget, chave, opcao='text', **kw):
        reg.append((widget, chave, opcao, kw))
        widget.configure(**{opcao: L(chave, **kw)})
        return widget

    # ---- top bar
    topo = ttk.LabelFrame(root)
    R(topo, 'proj_frame')
    topo.pack(fill='x', padx=8, pady=(8, 4))
    v_proj = tk.StringVar(value=d['project'])
    v_flag = tk.StringVar(value='0x%X' % d['flag_base'])
    v_mus = tk.StringVar()

    R(ttk.Label(topo), 'pasta_proj').grid(row=0, column=0, sticky='w', padx=6, pady=4)
    ttk.Entry(topo, textvariable=v_proj, width=60).grid(row=0, column=1, sticky='we', padx=4)
    R(ttk.Button(topo, command=lambda: escolher_pasta()), 'procurar').grid(row=0, column=2, padx=4)
    R(ttk.Label(topo), 'music_label').grid(row=1, column=0, sticky='w', padx=6, pady=4)
    cb_mus = ttk.Combobox(topo, textvariable=v_mus, state='readonly', width=48)
    cb_mus.grid(row=1, column=1, sticky='w', padx=4)
    R(ttk.Label(topo), 'flag_ini').grid(row=1, column=3, sticky='e', padx=(10, 2))
    ttk.Entry(topo, textvariable=v_flag, width=9).grid(row=1, column=4, sticky='w', padx=(0, 8))
    topo.columnconfigure(1, weight=1)

    corpo = ttk.Frame(root)
    corpo.pack(fill='both', expand=True, padx=8, pady=4)

    # ---- list
    esq = ttk.LabelFrame(corpo)
    R(esq, 'lista_frame')
    esq.pack(side='left', fill='y')
    cols = ('code', 'flag', 'tipo', 'poke', 'item')
    tv = ttk.Treeview(esq, columns=cols, show='headings', height=18, selectmode='browse')
    larg = {'code': 90, 'flag': 60, 'tipo': 115, 'poke': 130, 'item': 160}
    chaves_col = {'code': 'col_codigo', 'flag': 'col_flag', 'tipo': 'col_tipo', 'poke': 'col_poke', 'item': 'col_item'}
    for c in cols:
        tv.column(c, width=larg[c], anchor='w')
    tv.pack(side='left', fill='y', padx=(6, 0), pady=6)
    sb = ttk.Scrollbar(esq, orient='vertical', command=tv.yview)
    sb.pack(side='left', fill='y', pady=6, padx=(0, 6))
    tv.configure(yscrollcommand=sb.set)

    # ---- form
    dir_ = ttk.LabelFrame(corpo)
    R(dir_, 'form_frame')
    dir_.pack(side='left', fill='both', expand=True, padx=(8, 0))

    f = {k: tk.StringVar() for k in ('code', 'species', 'held', 'nature', 'm1', 'm2', 'm3', 'm4', 'reward')}
    f['level'] = tk.IntVar(value=5)
    f['qty'] = tk.IntVar(value=1)
    f['shiny'] = tk.BooleanVar()
    f['ha'] = tk.BooleanVar()
    f['tipo'] = tk.StringVar(value='poke')

    def combo_busca(pai, var, chave, largura=26):
        cb = ttk.Combobox(pai, textvariable=var, width=largura)

        def filtrar(ev=None):
            if ev is not None and ev.keysym in ('Up', 'Down', 'Return', 'Escape', 'Tab'):
                return
            txt = var.get().strip().upper().replace(' ', '_')
            base = estado['listas'][chave]
            cb['values'] = [n for n in base if txt in n][:300] if txt else base[:300]
        cb.bind('<KeyRelease>', filtrar)
        cb.bind('<FocusIn>', lambda e: (filtrar(), cb.after(1, lambda: cb.select_range(0, 'end'))))
        return cb

    def sel_tudo(w):
        w.bind('<FocusIn>', lambda e: w.after(1, lambda: w.select_range(0, 'end')), add='+')
        w.bind('<Control-a>', lambda e: (w.select_range(0, 'end'), 'break')[1])

    R(ttk.Label(dir_), 'cod_label').grid(row=0, column=0, sticky='w', padx=8, pady=(8, 2))
    e_code = ttk.Entry(dir_, textvariable=f['code'], width=14, font=('Consolas', 12, 'bold'))
    e_code.grid(row=0, column=1, sticky='w', pady=(8, 2))
    sel_tudo(e_code)

    def forcar_maiusculo(*_):
        v = re.sub(r'[^A-Za-z0-9]', '', f['code'].get()).upper()[:CODE_LENGTH]
        if v != f['code'].get():
            f['code'].set(v)
    f['code'].trace_add('write', forcar_maiusculo)

    flaginfo = tk.StringVar(value='')
    tipo_fr = ttk.Frame(dir_)
    tipo_fr.grid(row=1, column=0, columnspan=2, sticky='w', padx=8, pady=4)
    R(ttk.Label(tipo_fr), 'presente_e').pack(side='left')
    for chave, val in (('rad_poke', 'poke'), ('rad_item', 'item'), ('rad_ambos', 'ambos')):
        rb = ttk.Radiobutton(tipo_fr, value=val, variable=f['tipo'], command=lambda: atualizar_tipo())
        R(rb, chave).pack(side='left', padx=8)

    g_poke = ttk.LabelFrame(dir_)
    R(g_poke, 'g_poke')
    g_poke.grid(row=2, column=0, columnspan=2, sticky='we', padx=8, pady=4)
    R(ttk.Label(g_poke), 'especie').grid(row=0, column=0, sticky='w', padx=6, pady=3)
    c_sp = combo_busca(g_poke, f['species'], 'species'); c_sp.grid(row=0, column=1, sticky='w')
    R(ttk.Label(g_poke), 'nivel').grid(row=0, column=2, sticky='e', padx=(14, 2))
    sp_nivel = ttk.Spinbox(g_poke, from_=1, to=100, textvariable=f['level'], width=5)
    sp_nivel.grid(row=0, column=3, sticky='w'); sel_tudo(sp_nivel)
    R(ttk.Checkbutton(g_poke, variable=f['shiny']), 'shiny').grid(row=1, column=1, sticky='w', pady=2)
    R(ttk.Checkbutton(g_poke, variable=f['ha']), 'ha').grid(row=1, column=2, columnspan=2, sticky='w', pady=2)
    R(ttk.Label(g_poke), 'segura').grid(row=2, column=0, sticky='w', padx=6, pady=3)
    c_held = combo_busca(g_poke, f['held'], 'items'); c_held.grid(row=2, column=1, sticky='w')
    R(ttk.Label(g_poke), 'nature').grid(row=2, column=2, sticky='e', padx=(14, 2))
    cb_nat = ttk.Combobox(g_poke, textvariable=f['nature'], state='readonly', width=12)
    cb_nat.grid(row=2, column=3, sticky='w')
    R(ttk.Label(g_poke), 'golpes_dica').grid(row=3, column=0, columnspan=4, sticky='w', padx=6, pady=(8, 0))
    c_mv = []
    for i in range(4):
        R(ttk.Label(g_poke), 'golpe_n', n=i + 1).grid(row=4 + i // 2, column=(i % 2) * 2, sticky='e', padx=(6, 2), pady=2)
        cb = combo_busca(g_poke, f['m%d' % (i + 1)], 'moves', 22)
        cb.grid(row=4 + i // 2, column=(i % 2) * 2 + 1, sticky='w', pady=2)
        c_mv.append(cb)

    g_item = ttk.LabelFrame(dir_)
    R(g_item, 'g_item')
    g_item.grid(row=3, column=0, columnspan=2, sticky='we', padx=8, pady=4)
    R(ttk.Label(g_item), 'item_lbl').grid(row=0, column=0, sticky='w', padx=6, pady=4)
    c_rw = combo_busca(g_item, f['reward'], 'items'); c_rw.grid(row=0, column=1, sticky='w')
    R(ttk.Label(g_item), 'qtd').grid(row=0, column=2, sticky='e', padx=(14, 2))
    sp_qtd = ttk.Spinbox(g_item, from_=1, to=999, textvariable=f['qty'], width=6)
    sp_qtd.grid(row=0, column=3, sticky='w'); sel_tudo(sp_qtd)

    botoes = ttk.Frame(dir_)
    botoes.grid(row=4, column=0, columnspan=2, sticky='we', padx=8, pady=8)
    ttk.Label(dir_, textvariable=flaginfo, foreground='#036').grid(row=5, column=0, columnspan=2, sticky='w', padx=8, pady=(0, 6))
    status = tk.StringVar(value='')
    lbl_status = ttk.Label(root, textvariable=status, foreground='#a00', wraplength=1000, justify='left')
    lbl_status.pack(fill='x', padx=10)

    def aviso(txt, ok=False):
        lbl_status.configure(foreground='#060' if ok else '#a00')
        status.set(txt)

    # ---------------- helpers
    def nat_para_tela(n):
        return L('aleatoria') if n == RANDOM_NATURE else n

    def tela_para_nat(s):
        return RANDOM_NATURE if (not s or s == TR['aleatoria']) else s

    def listar_musicas():
        cb_mus['values'] = [music_name(m, estado['lang']) for m in MUSIC_IDS]
        v_mus.set(music_name(d['music'], estado['lang']))

    def musica_id():
        nome = v_mus.get()
        for m in MUSIC_IDS:
            if music_name(m, estado['lang']) == nome:
                return m
        return d['music']

    def tipo_de(c):
        if c['species'] and c['reward']:
            return L('tipo_ambos')
        return L('tipo_poke') if c['species'] else L('tipo_item')

    def flag_de(c):
        try:
            base = int(v_flag.get().strip(), 0)
        except ValueError:
            base = d['flag_base']
        return '0x%X' % (base + c['slot'])

    def linha_valores(c):
        it = ('%s x%d' % (c['reward'], c['qty'])) if c['reward'] else '-'
        return (c['code'], flag_de(c), tipo_de(c), c['species'] or '-', it)

    def aplicar_idioma():
        root.title(L('titulo'))
        for w, chave, opcao, kw in reg:
            w.configure(**{opcao: L(chave, **kw)})
        for c in cols:
            tv.heading(c, text=L(chaves_col[c]))
        cb_nat['values'] = [L('aleatoria')] + NATURES
        listar_musicas()
        rodape_dica.configure(text=L('dica'))
        btn_salvar.configure(text=L('salvar'))
        for w, chave in ((b_novo, 'novo'), (b_dup, 'duplicar'), (b_exc, 'excluir'), (b_apl, 'aplicar'), (b_imp, 'importar')):
            w.configure(text=L(chave))
        i = indice_sel()
        recarregar_lista(i)
        if i is not None and i < len(d['codes']):
            preencher(d['codes'][i])
        else:
            atualizar_tipo()
        status.set('')


    # ---------------- logic
    def recarregar_lista(selecionar=None):
        estado['bloq'] = True
        root.after_idle(lambda: estado.update(bloq=False))
        tv.delete(*tv.get_children())
        for i, c in enumerate(d['codes']):
            tv.insert('', 'end', iid=str(i), values=linha_valores(c))
        if selecionar is not None and 0 <= selecionar < len(d['codes']):
            tv.selection_set(str(selecionar)); tv.see(str(selecionar))

    def atualizar_tipo():
        tp = f['tipo'].get()
        for ch in g_poke.winfo_children():
            try:
                ch.configure(state='normal' if tp in ('poke', 'ambos') else 'disabled')
            except tk.TclError:
                pass
        for ch in g_item.winfo_children():
            try:
                ch.configure(state='normal' if tp in ('item', 'ambos') else 'disabled')
            except tk.TclError:
                pass
        cb_nat.configure(state='readonly' if tp in ('poke', 'ambos') else 'disabled')

    def preencher(c):
        flaginfo.set(L('flag_deste', f=flag_de(c)))
        f['code'].set(c['code']); f['species'].set(c['species']); f['level'].set(c['level'])
        f['shiny'].set(c['shiny']); f['ha'].set(c['ha']); f['held'].set(c['held']); f['nature'].set(nat_para_tela(c['nature']))
        for i in range(4):
            f['m%d' % (i + 1)].set(c['moves'][i])
        f['reward'].set(c['reward']); f['qty'].set(c['qty'] or 1)
        f['tipo'].set('ambos' if c['species'] and c['reward'] else ('poke' if c['species'] or not c['reward'] else 'item'))
        atualizar_tipo()

    def ler_form(slot):
        c = new_entry(slot)
        tp = f['tipo'].get()
        c['code'] = f['code'].get().strip().upper()
        if tp in ('poke', 'ambos'):
            c['species'] = f['species'].get().strip().upper().replace(' ', '_')
            try:
                c['level'] = int(f['level'].get())
            except (tk.TclError, ValueError):
                c['level'] = 0
            c['shiny'] = bool(f['shiny'].get()); c['ha'] = bool(f['ha'].get())
            c['held'] = f['held'].get().strip().upper().replace(' ', '_')
            c['nature'] = tela_para_nat(f['nature'].get())
            c['moves'] = [f['m%d' % (i + 1)].get().strip().upper().replace(' ', '_') for i in range(4)]
        if tp in ('item', 'ambos'):
            c['reward'] = f['reward'].get().strip().upper().replace(' ', '_')
            try:
                c['qty'] = int(f['qty'].get())
            except (tk.TclError, ValueError):
                c['qty'] = 0
        return c

    def indice_sel():
        s = tv.selection()
        return int(s[0]) if s else None

    def listas_ou_none():
        return estado['listas'] if any(estado['listas'].values()) else None

    def aplicar(mostrar=True):
        i = estado['sel']
        if i is None or i >= len(d['codes']):
            return True
        novo_c = ler_form(d['codes'][i]['slot'])
        d['codes'][i] = novo_c
        erros = validate({'codes': [novo_c]}, listas_ou_none(), estado['lang'])
        recarregar_lista(i)
        if erros:
            aviso(' | '.join(erros))
            return False
        if mostrar:
            aviso(L('st_aplicado', c=novo_c['code']), True)
        return True

    def ao_selecionar(_=None, force=False):
        if estado['bloq'] and not force:
            return
        i = indice_sel()
        if i is None:
            return
        ant = estado['sel']
        if ant is not None and ant != i and ant < len(d['codes']) and not force:
            d['codes'][ant] = ler_form(d['codes'][ant]['slot'])
            tv.item(str(ant), values=linha_valores(d['codes'][ant]))
        estado['sel'] = i
        preencher(d['codes'][i])
        aviso('')
    tv.bind('<<TreeviewSelect>>', ao_selecionar)

    def proximo_slot_livre():
        """never reuses the slot of a deleted code (its flag may be on in players' saves)"""
        usados = {c['slot'] for c in d['codes']}
        n = max(d.get('next_slot', 0), (max(usados) + 1) if usados else 0)
        return n if n < MAX_CODES else None

    def novo():
        aplicar(False)
        s = proximo_slot_livre()
        if s is None:
            messagebox.showwarning('!', L('dlg_limite', n=MAX_CODES))
            return
        d['next_slot'] = s + 1
        d['codes'].append(new_entry(s))
        i = len(d['codes']) - 1
        estado['sel'] = i
        recarregar_lista(i); preencher(d['codes'][i]); e_code.focus_set()
        aviso(L('st_novo'), True)

    def duplicar():
        i = indice_sel()
        if i is None:
            return
        aplicar(False)
        s = proximo_slot_livre()
        if s is None:
            messagebox.showwarning('!', L('dlg_limite', n=MAX_CODES))
            return
        d['next_slot'] = s + 1
        c = json.loads(json.dumps(d['codes'][i])); c['slot'] = s; c['code'] = ''
        d['codes'].append(c)
        j = len(d['codes']) - 1
        estado['sel'] = j
        recarregar_lista(j); preencher(c); e_code.focus_set()
        aviso(L('st_copia'), True)

    def excluir():
        i = indice_sel()
        if i is None:
            return
        c = d['codes'][i]
        if not messagebox.askyesno(L('dlg_excluir_t'), L('dlg_excluir', c=c['code'] or L('dlg_vazio'), s=c['slot'])):
            return
        d['next_slot'] = max(d.get('next_slot', 0), c['slot'] + 1)
        del d['codes'][i]
        estado['sel'] = None
        recarregar_lista(min(i, len(d['codes']) - 1) if d['codes'] else None)
        if d['codes']:
            ao_selecionar(force=True)
        aviso(L('st_excluido'), True)

    def escolher_pasta():
        p = filedialog.askdirectory(title=L('dlg_pasta_titulo'))
        if p:
            v_proj.set(p); ler_projeto()

    def ler_projeto():
        raiz = clean_path(v_proj.get())
        estado['listas'] = load_lists(raiz) if raiz else {'species': [], 'items': [], 'moves': []}
        n = estado['listas']
        for cb, ch in ((c_sp, 'species'), (c_held, 'items'), (c_rw, 'items')) + tuple((m, 'moves') for m in c_mv):
            cb['values'] = n[ch][:300]
        if not raiz:
            aviso(L('st_sem_proj'))
        elif not any(n.values()):
            aviso(L('st_sem_const'))
        else:
            aviso(L('st_proj_lido', a=len(n['species']), b=len(n['items']), c=len(n['moves'])), True)

    def ler_flag_base():
        try:
            return int(v_flag.get().strip(), 0)
        except ValueError:
            return None

    def salvar_e_gerar():
        lang = estado['lang']
        if not aplicar(False):
            messagebox.showerror(L('dlg_erro'), L('dlg_corrija'))
            return
        fb = ler_flag_base()
        if fb is None:
            messagebox.showerror(L('dlg_erro'), L('dlg_flag_num'))
            return
        d['flag_base'] = fb
        d['project'] = clean_path(v_proj.get())
        d['music'] = musica_id()
        erros = validate(d, listas_ou_none(), lang, d['project'] or None)
        if erros:
            aviso(' | '.join(erros[:6]))
            messagebox.showerror(L('dlg_nao_gerar'), '\n'.join(erros[:12]))
            return
        save_json(d)
        if not d['project']:
            messagebox.showwarning(L('dlg_escolha_proj'), L('dlg_salvei_json'))
            return
        try:
            dest, est = write_h(d, d['project'], lang)
        except Exception as ex:
            aviso(str(ex).split('\n')[0])
            messagebox.showerror(L('dlg_erro_escrever'), str(ex))
            return
        v = state_text(est, lang)
        aviso(L('st_gerado', d=dest, v=v), est == 'ok')
        (messagebox.showinfo if est == 'ok' else messagebox.showwarning)(L('dlg_pronto_t'), L('dlg_pronto', d=dest, v=v))

    def importar():
        p = filedialog.askopenfilename(title=L('dlg_imp_titulo'), filetypes=[('Header C', '*.h'), ('*', '*.*')])
        if not p:
            return
        try:
            novo_d = import_h(p)
        except Exception as ex:
            messagebox.showerror(L('dlg_erro'), L('dlg_imp_erro', e=ex))
            return
        if not novo_d['codes']:
            messagebox.showwarning('!', L('dlg_imp_vazio'))
            return
        for k in ('codes', 'flag_base', 'music', 'next_slot'):
            d[k] = novo_d[k]
        v_flag.set('0x%X' % d['flag_base'])
        estado['sel'] = None
        recarregar_lista(0); ao_selecionar(force=True)
        listar_musicas()
        aviso(L('st_importados', n=len(d['codes'])), True)

    b_novo = ttk.Button(botoes, command=novo); b_novo.pack(side='left')
    b_dup = ttk.Button(botoes, command=duplicar); b_dup.pack(side='left', padx=4)
    b_exc = ttk.Button(botoes, command=excluir); b_exc.pack(side='left')
    b_apl = ttk.Button(botoes, command=aplicar); b_apl.pack(side='left', padx=(16, 0))
    b_imp = ttk.Button(botoes, command=importar); b_imp.pack(side='right')

    rodape = ttk.Frame(root)
    rodape.pack(fill='x', padx=8, pady=(2, 8))
    btn_salvar = ttk.Button(rodape, command=salvar_e_gerar)
    btn_salvar.pack(side='right', ipadx=10, ipady=4)
    rodape_dica = ttk.Label(rodape, foreground='#555')
    rodape_dica.pack(side='left')

    def ao_mudar_flag_base(*_):
        s_ = indice_sel()
        recarregar_lista(s_)
        if s_ is not None and s_ < len(d['codes']):
            preencher(d['codes'][s_])
    v_flag.trace_add('write', ao_mudar_flag_base)

    aplicar_idioma()
    ler_projeto()
    recarregar_lista(0 if d['codes'] else None)
    if d['codes']:
        estado['sel'] = 0
        preencher(d['codes'][0])
    if aviso_json:
        root.after(300, lambda: messagebox.showwarning('!', L('dlg_json_corrompido', b=aviso_json)))
    root.update_idletasks()
    root.minsize(root.winfo_reqwidth(), root.winfo_reqheight())
    root.mainloop()


# ------------------------------------------------------------------------------------------------ command line
def main(argv):
    lang = 'en'
    if '--generate' in argv:
        d, _aviso = load_json()
        raiz = d['project']
        if '--project' in argv:
            raiz = argv[argv.index('--project') + 1]
        raiz = clean_path(raiz)
        listas = load_lists(raiz) if raiz else None
        erros = validate(d, listas if listas and any(listas.values()) else None, lang, raiz or None)
        if erros:
            print('ERRORS:\n - ' + '\n - '.join(erros))
            return 1
        try:
            dest, est = write_h(d, raiz, lang)
        except Exception as ex:
            print(ex)
            return 1
        print('OK:', dest, '|', state_text(est, lang))
        return 0
    if '--import' in argv:
        d = import_h(argv[argv.index('--import') + 1])
        save_json(d)
        print('%d codes -> %s' % (len(d['codes']), JSON_FILE))
        return 0
    open_window()
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
