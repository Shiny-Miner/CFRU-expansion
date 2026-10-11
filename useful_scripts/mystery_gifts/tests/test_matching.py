#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Tests the REAL code-matching logic of the game: extracts MgKeyAscii and MgFindCode from src/mystery_gift.c, builds them into
a normal PC program and checks that every code typed on the on-screen keyboard is found, and that nothing wrong is accepted.
  python test_matching.py <project root folder, or the 'project' folder of the package>
"""
import os, random, re, subprocess, sys, tempfile
raiz = sys.argv[1]
fonte = open(os.path.join(raiz, 'src', 'mystery_gift.c'), encoding='utf-8').read()

def funcao(nome):
    m = re.search(r'static [^\n(]*\b' + nome + r'\([^\n]*\)[^\n]*\n\{.*?\n\}\n', fonte, re.S)
    assert m, nome
    return m.group(0)

C = r'''
#include <stdio.h>
#include <string.h>
typedef unsigned char u8; typedef unsigned int u32; typedef unsigned short u16; typedef u8 bool8;
#define TRUE 1
#define FALSE 0
static int gCount;
#define NELEMS(a) ((u32) gCount)
struct E { const char* code; };
static struct E sMysteryGifts[4000];
static u8 gStringVar2[256];
''' + funcao('MgKeyAscii') + funcao('MgFindCode') + r'''
int main(void)
{
	static char linhas[4000][16]; char buf[64]; int n, q;
	if (scanf("%d", &n) != 1) return 2;
	for (int i = 0; i < n; ++i) { scanf("%15s", linhas[i]); sMysteryGifts[i].code = linhas[i]; }
	gCount = n;
	if (scanf("%d", &q) != 1) return 2;
	for (int k = 0; k < q; ++k)
	{
		scanf("%63s", buf);
		int len = (int) strlen(buf), bad = 0;
		for (int j = 0; j < len; ++j)
		{
			int key = -1;
			for (int c = 0; c < 36; ++c) if (MgKeyAscii(c) == (u8) buf[j]) key = c;   /* the key that types this character */
			if (key < 0) { bad = 1; break; }
			gStringVar2[j] = MgKeyAscii(key);
		}
		gStringVar2[len] = 0;
		printf("%d\n", bad ? -2 : (MgFindCode((u8) len) == 0xFF ? -1 : (int) MgFindCode((u8) len)));
	}
	return 0;
}
'''
d = tempfile.mkdtemp()
open(d + '/t.c', 'w').write(C)
subprocess.check_call(['gcc', '-O1', '-Wall', '-o', d + '/t', d + '/t.c'])

def rodar(tabela, consultas):
    entrada = '%d\n%s\n%d\n%s\n' % (len(tabela), '\n'.join(tabela), len(consultas), '\n'.join(consultas))
    r = subprocess.run([d + '/t'], input=entrada, capture_output=True, text=True, check=True)
    return [int(x) for x in r.stdout.split()]

alfa = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
rnd = random.Random(2024)
falhas = 0; total = 0
def conferir(tabela, consultas, esperado):
    global falhas, total
    got = rodar(tabela, consultas)
    for c, g, w in zip(consultas, got, esperado):
        total += 1
        if g != w:
            falhas += 1
            if falhas < 10: print('  FAILED:', c, 'got', g, 'expected', w)

# 1) exaustivo: TODOS os codigos de 1 e 2 caracteres (1332) numa tabela so
tab = list(alfa) + [a + b for a in alfa for b in alfa]
nao = [a + b + c for a in 'AZ09' for b in 'AZ09' for c in 'AZ09']
for ini in range(0, len(tab), 250):          # a tabela real tem no maximo 64 entradas; o indice do jogo e de 1 byte
    parte = tab[ini:ini + 250]
    conferir(parte, parte, list(range(len(parte))))
    conferir(parte, nao, [-1] * len(nao))
# todo codigo de 1 e 2 caracteres NAO presente numa tabela pequena deve ser recusado
pequena = ['A', 'AB', 'ZZ', '00', '9']
fora = [c for c in tab if c not in pequena]
conferir(pequena, fora, [-1] * len(fora))
# 2) caracteres de borda em todas as posicoes e tamanhos
borda = ['0', '9', 'A', 'Z'] ; tabb = []
for n in range(1, 9):
    for _ in range(40):
        tabb.append(''.join(rnd.choice(borda) for _ in range(n)))
tabb = list(dict.fromkeys(tabb))
conferir(tabb, tabb, list(range(len(tabb))))
# 3) tabelas aleatorias (1..64 codigos) - membros, prefixos, extensoes, quase-iguais, digitos-so, letras-so
for _ in range(400):
    k = rnd.randint(1, 64)
    tabela = []
    while len(tabela) < k:
        modo = rnd.random()
        n = rnd.randint(1, 8)
        if modo < .2: c = ''.join(rnd.choice('0123456789') for _ in range(n))
        elif modo < .4: c = ''.join(rnd.choice('ABCDEFGHIJKLMNOPQRSTUVWXYZ') for _ in range(n))
        elif tabela and modo < .6:
            base = rnd.choice(tabela); c = base[:rnd.randint(1, len(base))] + ''.join(rnd.choice(alfa) for _ in range(rnd.randint(0, 3)))
            c = c[:8]
        else: c = ''.join(rnd.choice(alfa) for _ in range(n))
        if c and c not in tabela: tabela.append(c)
    cons, esp = [], []
    for i, c in enumerate(tabela):
        cons.append(c); esp.append(i)                                   # o proprio codigo
        for p in {c[:-1], c + 'A', 'A' + c, c[:-1] + ('Z' if c[-1] != 'Z' else '0')}:   # prefixo, extensao, quase-iguais
            if p and len(p) <= 8 and p not in tabela:
                cons.append(p); esp.append(-1)
    for _ in range(20):
        c = ''.join(rnd.choice(alfa) for _ in range(rnd.randint(1, 8)))
        cons.append(c); esp.append(tabela.index(c) if c in tabela else -1)
    conferir(tabela, cons, esp)
print('queries tested: %d | failures: %d' % (total, falhas))
print('OK: the game finds every code in the table and rejects everything that is not in it.' if not falhas else 'FAILED')
sys.exit(1 if falhas else 0)
