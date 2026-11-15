#include "defines.h"

#ifdef NEW_BATTLE_TEXTBOX
/*
 * NEW BATTLE TEXTBOX
 * Clean dark UI for the battle message box, the action menu (FIGHT / BAG / POKeMON / RUN buttons with icons),
 * the move selection boxes, the move type/category plate and the Yes/No box.
 *
 * Enable it by adding   #define NEW_BATTLE_TEXTBOX   to src/config.h  (see README_INSTALL.txt).
 *
 * Technical notes
 * - Panel/message tiles live in VRAM tile slots 0x30.. (permanent; the original art only used 0x00-0x2F).
 * - Button tiles (BTB_TILE_BASE in battle_textbox_data.h, 0x250+) are re-uploaded each time the action menu is drawn,
 *   because other battle windows (move names, PP...) reuse that VRAM range.
 * - Palettes: slot 0 (message text colours), 1 (Yes/No frame), 12-15 (one per button; 8-11 are used by double battles).
 *   Slot 5 (window text) and slot 6 (type/category icons) are recoloured.
 */
#include "../include/bg.h"
#include "../include/palette.h"
#include "../include/decompress.h"
#include "../include/window.h"
#include "../include/new/Vanilla_functions.h"
#include "defines_battle.h"
#include "../include/sprite.h"
#include "../include/string_util.h"
#include "../include/menu.h"
#include "battle_textbox_data.h"

#define BTB_MSG_TOP   14
#define BTB_ACT_TOP   34
#define BTB_MOVE_TOP  54

#define BTB_PLATE_COLOUR 0x1CA5
#define BTB_GWINDOWS     ((struct Window*) 0x20204B4)
#define BTB_GMAIN_CB2    (*(vu32*) 0x30030F4)
#define BTB_BATTLE_CB2   0x8011101

#define BTB_PLTT_UNFADED ((vu16*) 0x20371F8)
#define BTB_PLTT_FADED   ((vu16*) 0x20375F8)

typedef void (*VoidFn)(void);
typedef void (*TilemapCopyFn)(u8 bg, const void* src, u16 mode, u16 offset);
typedef void (*CompPalFn)(const void* src, u16 offset, u16 size);


static const u32 sBTBSafariHealthboxSheetLZ[] =
{
	0x00100010,0x00F0007F,0x00F000F0,0x00F000F0,0x00F000F0,0xF000F0FF,0xF000F000,0xF000F000,
	0xF000F000,0x00F0FF00,0x00F000F0,0x00F000F0,0x00F000F0,0xF0FF00F0,0xF000F000,0xF000F000,
	0xF000F000,0xFF00F000,0x00F000F0,0x00F000F0,0x00F000F0,0x00F000F0,0xF000F0FF,0xF000F000,
	0xF000F000,0xF000F000,0x00F0FF00,0x00F000F0,0x00F000F0,0x00F000F0,0xF0FF00F0,0xF000F000,
	0xF000F000,0xF000F000,0xFF00F000,0x00F000F0,0x00F000F0,0x00F000F0,0x00F000F0,0xF000F0FF,
	0xF000F000,0xF000F000,0xF000F000,0x00F0FF00,0x00F000F0,0x00F000F0,0x00F000F0,0xF0FF00F0,
	0xF000F000,0xF000F000,0xF000F000,0xFF00F000,0x00F000F0,0x00F000F0,0x00F000F0,0x00F000F0,
	0xF000F0FF,0xF000F000,0xF000F000,0xF000F000,0x00F0FF00,0x00F000F0,0x00F000F0,0x00F000F0,
	0xF0FF00F0,0xF000F000,0xF000F000,0xF000F000,0xFF00F000,0x00F000F0,0x00F000F0,0x00F000F0,
	0x00F000F0,0xF000F0FF,0xF000F000,0xF000F000,0xF000F000,0x00F0FF00,0x00F000F0,0x00F000F0,
	0x00F000F0,0xF0FF00F0,0xF000F000,0xF000F000,0xF000F000,0xFF00F000,0x00F000F0,0x00F000F0,
	0x00F000F0,0x00F000F0,0xF000F0FF,0xF000F000,0xF000F000,0xF000F000,0x00F0FF00,0x00F000F0,
	0x00F000F0,0x00F000F0,0xF0FF00F0,0xF000F000,0xF000F000,0xF000F000,0xFF00F000,0x00F000F0,
	0x00F000F0,0x00F000F0,0x00F000F0,0xF000F0FF,0xF000F000,0xF000F000,0xF000F000,0x00F0FF00,
	0x00F000F0,0x00F000F0,0x00F000F0,0xF0FF00F0,0xF000F000,0xF000F000,0xF000F000,0xF800F000,
	0x00F000F0,0x00F000F0,0x00000060,
};

const struct CompressedSpriteSheet gBTBSafariHealthboxSheet =
{
	(const u8*) sBTBSafariHealthboxSheetLZ, 0x1000, 0xD70B
};

static void BTB_SetPal(u8 slot, u8 idx, u16 colour)
{
	BTB_PLTT_UNFADED[slot * 16 + idx] = colour;
	BTB_PLTT_FADED[slot * 16 + idx] = colour;
}

static bool8 BTB_IsSafariBattle(void)
{
	return (*(vu32*) 0x2022B4C & 0x80) != 0;
}

static void BTB_DrawSafariCounter(void);

static void BTB_UploadButtonTiles(void)
{
	if (BTB_IsSafariBattle())
		CpuSet(gBTBSafariTiles, (void*) (0x6000000 + BTB_TILE_BASE * 32), BTB_SAFARI_TILE_COUNT * 16);
	else
		CpuSet(gBTBTiles, (void*) (0x6000000 + BTB_TILE_BASE * 32), BTB_TILE_COUNT * 16);
}

static void BTB_UploadPanelTiles(void)
{

	u16 total = BTB_MSG_TILE_COUNT;
	u16 done = 0;
	u16 count = (total < BTB_MSG_SEG0_COUNT) ? total : BTB_MSG_SEG0_COUNT;

	CpuSet(gBTBMsgTiles, (void*) (0x6000000 + BTB_MSG_SEG0_BASE * 32), count * 16);
	done += count;
	if (total > done)
	{
		count = ((total - done) < BTB_MSG_SEG1_COUNT) ? (total - done) : BTB_MSG_SEG1_COUNT;
		CpuSet(&gBTBMsgTiles[done * 16], (void*) (0x6000000 + BTB_MSG_SEG1_BASE * 32), count * 16);
		done += count;
	}
	if (total > done)
		CpuSet(&gBTBMsgTiles[done * 16], (void*) (0x6000000 + BTB_MSG_SEG2_BASE * 32), (total - done) * 16);
}

static void BTB_UploadRingAndCursorTiles(void)
{
	BTB_UploadPanelTiles();
	CpuSet(gBTBRingTiles, (void*) (0x6000000 + 0x22 * 32), 9 * 16);
	CpuSet(&gBTBCursorTiles[0], (void*) (0x6000000 + 1 * 32), 16);
	CpuSet(&gBTBCursorTiles[16], (void*) (0x6000000 + 2 * 32), 16);
	CpuSet(&gBTBCursorTiles[32], (void*) (0x6000000 + 0x20 * 32), 16);
	CpuSet(&gBTBCursorTiles[48], (void*) (0x6000000 + 0x1C * 32), 4 * 16);
}

static void BTB_RecolorWindowPal(void)
{
	for (u8 i = 0; i < 16; ++i)
		if (gBTBPal5[i] != 0xFFFF)
			BTB_SetPal(5, i, gBTBPal5[i]);
}

static void BTB_ApplyAction(u8 state, u16* buf, u8 firstColumn)
{
	for (u8 r = 0; r < 6; ++r)
		for (u8 c = firstColumn; c < 30; ++c)
			buf[(BTB_ACT_TOP + r) * 32 + c] = (BTB_IsSafariBattle() ? gBTBSafariActionMaps : gBTBActionMaps)[state & 3][r][c];
}

void BTB_LoadBattleTextboxAndBackground(void)
{
	LZDecompressVram((const void*) 0x8D00000, (void*) 0x6000000);
	((TilemapCopyFn) 0x8002041)(0, (const void*) 0x8D0051C, 0, 0);
	((CompPalFn) 0x80703A9)((const void*) 0x8D004D8, 0, 0x40);

	BTB_UploadButtonTiles();
	BTB_UploadRingAndCursorTiles();

	u16* buf = GetBgTilemapBuffer(0);
	for (u8 r = 0; r < 6; ++r)
	{
		for (u8 c = 0; c < 30; ++c)
		{
			buf[(BTB_MSG_TOP + r) * 32 + c] = gBTBMsgMap[r][c];
			buf[(BTB_MOVE_TOP + r) * 32 + c] = gBTBMovesMap[r][c];
		}
	}
	BTB_ApplyAction(0, buf, 0);
	CopyBgTilemapBufferToVram(0);

	((VoidFn) 0x800F381)();
	((VoidFn) 0x800F40D)();


	LoadPalette(gBTBPal0, 0, 32);
	LoadPalette(gBTBPal1, 16, 32);
	LoadPalette(gBTBButtonPals, 192, 128);
	BTB_RecolorWindowPal();
}


void BTB_DrawMessageBoxFrame(void)
{
	u16* buf = GetBgTilemapBuffer(0);
	for (u8 r = 0; r < 6; ++r)
		for (u8 c = 0; c < 30; ++c)
			buf[(BTB_MSG_TOP + r) * 32 + c] = gBTBMsgMap[r][c];
}

void BTB_ActionSelectionCreateCursorAt(u8 cursorPos, unusedArg u8 unused)
{
	BTB_UploadButtonTiles();
	BTB_UploadRingAndCursorTiles();
	BTB_RecolorWindowPal();
	BTB_ApplyAction(cursorPos, GetBgTilemapBuffer(0), 13);
	CopyBgTilemapBufferToVram(0);
	if (BTB_IsSafariBattle())
		BTB_DrawSafariCounter();
}

void BTB_ActionSelectionDestroyCursorAt(unusedArg u8 cursorPos)
{
}

void BTB_MoveSelectionCreateCursorAt(u8 cursorPos, u8 baseTileNum)
{
	BTB_UploadRingAndCursorTiles();
	u16* buf = GetBgTilemapBuffer(0);
	u8 x = (cursorPos & 1) * 9 + 1;
	u8 y = (cursorPos & 2) + 55;
	buf[y * 32 + x] = baseTileNum + 1;
	buf[(y + 1) * 32 + x] = baseTileNum + 2;
	CopyBgTilemapBufferToVram(0);

	BTB_SetPal(6, 14, BTB_PLATE_COLOUR);
}

static u8 BTB_GetPx(const u8* data, u8 widthTiles, u8 x, u8 y)
{
	const u8* p = data + ((y >> 3) * widthTiles + (x >> 3)) * 32 + (y & 7) * 4 + ((x & 7) >> 1);
	return (x & 1) ? (*p >> 4) : (*p & 0xF);
}

static void BTB_SetPx(u8* data, u8 widthTiles, u8 x, u8 y, u8 colour)
{
	u8* p = data + ((y >> 3) * widthTiles + (x >> 3)) * 32 + (y & 7) * 4 + ((x & 7) >> 1);
	if (x & 1) *p = (*p & 0x0F) | (colour << 4);
	else       *p = (*p & 0xF0) | colour;
}

static void BTB_PaintStripPlate(u8 windowId)
{
	struct Window* win = &BTB_GWINDOWS[windowId];
	u8 wt = win->window.width, ht = win->window.height;
	if (win->tileData == NULL || wt == 0 || wt > 8 || ht == 0 || ht > 2)
		return;

	u8 w = wt * 8, h = ht * 8;
	u8 mask[128];
	for (u8 i = 0; i < 128; ++i)
		mask[i] = 0;

	bool8 changed = TRUE;
	while (changed)
	{
		changed = FALSE;
		for (u8 y = 0; y < h; ++y)
		{
			for (u8 x = 0; x < w; ++x)
			{
				u16 bit = y * w + x;
				if (mask[bit >> 3] & (1 << (bit & 7)))
					continue;
				if (BTB_GetPx(win->tileData, wt, x, y) != 15)
					continue;

				bool8 touches = (x == 0 || y == 0 || x == w - 1 || y == h - 1);
				if (!touches)
				{
					u16 n;
					n = bit - 1;  if (mask[n >> 3] & (1 << (n & 7))) touches = TRUE;
					n = bit + 1;  if (mask[n >> 3] & (1 << (n & 7))) touches = TRUE;
					n = bit - w;  if (mask[n >> 3] & (1 << (n & 7))) touches = TRUE;
					n = bit + w;  if (mask[n >> 3] & (1 << (n & 7))) touches = TRUE;
				}
				if (touches)
				{
					mask[bit >> 3] |= (1 << (bit & 7));
					changed = TRUE;
				}
			}
		}
	}

	for (u8 y = 0; y < h; ++y)
		for (u8 x = 0; x < w; ++x)
		{
			u16 bit = y * w + x;
			if (mask[bit >> 3] & (1 << (bit & 7)))
				BTB_SetPx(win->tileData, wt, x, y, 14);
		}
}

void BTB_BlitMoveInfoIcon(u8 windowId, u8 iconId, u16 x, u16 y)
{
	const u8* table = (const u8*) *(vu32*) 0x8107DAC;
	const u8* gfx = (const u8*) *(vu32*) 0x8107DB0;
	const u8* entry = table + (iconId << 2);
	u16 offset = entry[2] | (entry[3] << 8);
	BlitBitmapRectToWindow(windowId, gfx + offset * 32, 0, 0, 128, 128, x, y, entry[0], entry[1]);

	if (windowId == 8 && BTB_GMAIN_CB2 == BTB_BATTLE_CB2)
		BTB_PaintStripPlate(windowId);
}


#define BTB_SAFARI_WIN_BASE 0x110
#define BTB_SAFARI_COL      17
#define BTB_SAFARI_ROW      9
#define BTB_SAFARI_FONT     0

#ifdef EXPAND_SAFARI_BALLS
	#define BTB_SAFARI_BALLS (*((vu16*) 0x02039994))
#else
	#define BTB_SAFARI_BALLS (*((vu8*) 0x02039994))
#endif

typedef s32 (*BTB_StringWidthFn)(u8 fontId, const u8* str, s16 letterSpacing);

static void BTB_DrawSafariCounter(void)
{
	struct WindowTemplate template = {0, BTB_SAFARI_COL, BTB_SAFARI_ROW, 12, 4, 0, BTB_SAFARI_WIN_BASE};
	u8 colours[3] = {15, 1, 6};
	u8 text[16];
	u16 win = AddWindow(&template);
	if (win == 0xFF)
		return;

	CpuSet(gBTBSafariCounterTiles, BTB_GWINDOWS[win].tileData, 48 * 16);
	AddTextPrinterParameterized3(win, BTB_SAFARI_FONT, 6, 4, colours, 0, (const u8*) 0x83FE80F);
	ConvertIntToDecimalStringN(StringCopy(text, (const u8*) 0x83FE81F), BTB_SAFARI_BALLS, STR_CONV_MODE_LEFT_ALIGN, 5);
	s32 width = ((BTB_StringWidthFn) 0x8005ED5)(BTB_SAFARI_FONT, text, 0);
	if (width > 80) width = 80; //Very large (expanded) ball counts must not leave the panel
	AddTextPrinterParameterized3(win, BTB_SAFARI_FONT, 90 - width, 16, colours, 0, text);

	CpuSet(BTB_GWINDOWS[win].tileData, (void*) (0x6000000 + BTB_SAFARI_WIN_BASE * 32), 48 * 16);
	u16* buf = GetBgTilemapBuffer(0);
	for (u8 r = 0; r < 4; ++r)
	{
		for (u8 c = 0; c < 12; ++c)
		{
			u16 entry = BTB_SAFARI_WIN_BASE + r * 12 + c;
			buf[(BTB_SAFARI_ROW + r) * 32 + BTB_SAFARI_COL + c] = entry;
			buf[(BTB_SAFARI_ROW + 20 + r) * 32 + BTB_SAFARI_COL + c] = entry;
		}
	}
	CopyBgTilemapBufferToVram(0);
	RemoveWindow(win);
}

static void BTB_HideSafariHealthbox(u8 healthboxSpriteId)
{
	struct Sprite* sprite = &gSprites[healthboxSpriteId];
	sprite->invisible = TRUE;
	gSprites[sprite->oam.affineParam].invisible = TRUE;
}

void BTB_UpdateSafariBallsText(u8 healthboxSpriteId) //Replaces UpdateSafariBallsTextOnHealthbox (0x08049CAC)
{
	BTB_HideSafariHealthbox(healthboxSpriteId);
	BTB_DrawSafariCounter();
}

void BTB_UpdateLeftBallsText(u8 healthboxSpriteId) //Replaces UpdateLeftNoOfBallsTextOnHealthbox (0x08049D10)
{
	BTB_HideSafariHealthbox(healthboxSpriteId);
	BTB_DrawSafariCounter();
}

void BTB_SetHealthboxSpriteVisible(u8 healthboxSpriteId) //Replaces SetHealthboxSpriteVisible (0x080481D4)
{
	struct Sprite* sprite = &gSprites[healthboxSpriteId];

	if ((gBattleTypeFlags & 0x80) && healthboxSpriteId == gHealthboxSpriteIds[0])
	{
		BTB_HideSafariHealthbox(healthboxSpriteId);
		BTB_DrawSafariCounter();
		return;
	}

	sprite->invisible = FALSE;
	gSprites[sprite->data[5]].invisible = FALSE;
	gSprites[sprite->oam.affineParam].invisible = FALSE;
}

/*
 * Hook for the vanilla blit_move_info_icon (0x08107D68). The hook is placed at 0x08107D6C, i.e. right after the
 * original prologue (push {r4, r5, lr}; sub sp, #0x18), so registers r0-r3 still hold the arguments and the original
 * epilogue is repeated here. This avoids needing an entry in 'functionrewrites'.
 */
asm(
	".thumb\n"
	".align 2\n"
	".global BTB_BlitMoveInfoIconHook\n"
	".thumb_func\n"
	"BTB_BlitMoveInfoIconHook:\n"
	"\tbl BTB_BlitMoveInfoIcon\n"
	"\tadd sp, #0x18\n"
	"\tpop {r4, r5}\n"
	"\tpop {r0}\n"
	"\tbx r0\n"
);

#endif //NEW_BATTLE_TEXTBOX
