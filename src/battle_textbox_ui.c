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

#define BTB_MSG_TOP   14   //Message box rows 14-19 (tilemap block 0)
#define BTB_ACT_TOP   34   //Action menu rows 34-39 (tilemap block 1)
#define BTB_MOVE_TOP  54   //Move selection boxes rows 54-59 (tilemap block 1)

#define BTB_PLATE_COLOUR 0x1CA5 //RGB(44,47,64): plate behind the type/category icons
#define BTB_GWINDOWS     ((struct Window*) 0x20204B4)
#define BTB_GMAIN_CB2    (*(vu32*) 0x30030F4) //gMain.callback2
#define BTB_BATTLE_CB2   0x8011101           //BattleMainCB2 | 1

#define BTB_PLTT_UNFADED ((vu16*) 0x20371F8) //gPlttBufferUnfaded
#define BTB_PLTT_FADED   ((vu16*) 0x20375F8) //gPlttBufferFaded

typedef void (*VoidFn)(void);
typedef void (*TilemapCopyFn)(u8 bg, const void* src, u16 mode, u16 offset);
typedef void (*CompPalFn)(const void* src, u16 offset, u16 size);

//---------------------------------------------------------------------------------------------------------------------
// FIX (Safari Zone): corrupted opponent health box
// The vanilla Safari health box sprite sheet (tag 0xD70B) is only used by the Safari counter box, which this feature hides and
// replaces. Older versions repointed that sheet entry to the fixed address 0x0880A020, which is only a valid compressed image
// in one specific ROM. In any other ROM the game decompressed random data into the heap, corrupting the health box graphics.
// This is a valid LZ77 image (0x1000 bytes of zeros) that lives inside the build, so it works in every ROM. 'repoints' points
// the one place that loads the sheet (literal at 0x0803488C) to this struct.
//---------------------------------------------------------------------------------------------------------------------
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

//Safari Zone battles have their own menu (BALL / BAIT / ROCK / RUN), so they use a second set of button tiles
static bool8 BTB_IsSafariBattle(void)
{
	return (*(vu32*) 0x2022B4C & 0x80) != 0; //gBattleTypeFlags & BATTLE_TYPE_SAFARI
}

static void BTB_DrawSafariCounter(void); //Defined with the other Safari code below

static void BTB_UploadButtonTiles(void)
{
	if (BTB_IsSafariBattle())
		CpuSet(gBTBSafariTiles, (void*) (0x6000000 + BTB_TILE_BASE * 32), BTB_SAFARI_TILE_COUNT * 16); //16-bit copy (VRAM)
	else
		CpuSet(gBTBTiles, (void*) (0x6000000 + BTB_TILE_BASE * 32), BTB_TILE_COUNT * 16); //16-bit copy (VRAM)
}

static void BTB_UploadPanelTiles(void)
{
	//Panel/frame tiles. They live in segments below tile 0x200 that no battle window uses (tiles 0x200-0x3FF are rewritten by the
	//background entry overlay and by the move animation backgrounds),
	//and they are re-sent every time a menu is drawn, so the panel can never stay corrupted.
	u16 total = BTB_MSG_TILE_COUNT;
	u16 done = 0;
	u16 count = (total < BTB_MSG_SEG0_COUNT) ? total : BTB_MSG_SEG0_COUNT;

	CpuSet(gBTBMsgTiles, (void*) (0x6000000 + BTB_MSG_SEG0_BASE * 32), count * 16); //16-bit copy (VRAM)
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
	//The vanilla window-frame tiles (Yes/No box) are queued for DMA at battle start and would overwrite an early copy,
	//so these are (re)uploaded every time a menu is drawn.
	CpuSet(gBTBRingTiles, (void*) (0x6000000 + 0x22 * 32), 9 * 16);
	CpuSet(&gBTBCursorTiles[0], (void*) (0x6000000 + 1 * 32), 16);
	CpuSet(&gBTBCursorTiles[16], (void*) (0x6000000 + 2 * 32), 16);
	CpuSet(&gBTBCursorTiles[32], (void*) (0x6000000 + 0x20 * 32), 16);
	CpuSet(&gBTBCursorTiles[48], (void*) (0x6000000 + 0x1C * 32), 4 * 16); //swap-mode arrows: tiles 0x1C-0x1F
}

static void BTB_RecolorWindowPal(void)
{
	//Palette 5: text palette of the battle windows (moves, PP, yes/no) -> dark panel + light text
	for (u8 i = 0; i < 16; ++i)
		if (gBTBPal5[i] != 0xFFFF)
			BTB_SetPal(5, i, gBTBPal5[i]);
}

static void BTB_ApplyAction(u8 state, u16* buf, u8 firstColumn)
{
	//Only the button area is rewritten after the first draw: the left columns hold the text window's own tiles
	for (u8 r = 0; r < 6; ++r)
		for (u8 c = firstColumn; c < 30; ++c)
			buf[(BTB_ACT_TOP + r) * 32 + c] = (BTB_IsSafariBattle() ? gBTBSafariActionMaps : gBTBActionMaps)[state & 3][r][c];
}

void BTB_LoadBattleTextboxAndBackground(void)
{
	//Vanilla part: original tiles/tilemap/palette (keeps unused original tiles intact)
	LZDecompressVram((const void*) 0x8D00000, (void*) 0x6000000);
	((TilemapCopyFn) 0x8002041)(0, (const void*) 0x8D0051C, 0, 0);
	((CompPalFn) 0x80703A9)((const void*) 0x8D004D8, 0, 0x40);

	//New art tiles (panel tiles are sent by BTB_UploadRingAndCursorTiles)
	BTB_UploadButtonTiles();
	//Vanilla tiles 1, 2 and 0x20 (move cursor arrow / blank) recoloured for the dark panel (+ Yes/No ring)
	BTB_UploadRingAndCursorTiles();

	//Tilemap: message box, action menu (state 0) and move selection boxes
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

	((VoidFn) 0x800F381)(); //LoadBattleMenuWindowGfx
	//Yes/No box frame: vanilla loads the options-menu frame into tiles 0x22-0x2A (+ palette slot 1); see BTB_UploadRingAndCursorTiles
	((VoidFn) 0x800F40D)(); //Load battle terrain (vanilla tail of the original function)


	//Palettes (after terrain, which can touch other slots)
	LoadPalette(gBTBPal0, 0, 32);
	LoadPalette(gBTBPal1, 16, 32); //Yes/No frame colours
	LoadPalette(gBTBButtonPals, 192, 128); //slots 12-15 (8-11 are used in double battles)
	BTB_RecolorWindowPal();
}

/*
 * FIX: broken message box (green/red dashes on the left edge, ragged right edge, "teeth" on the bottom) after Oak's
 * dialogue in the tutorial (first) battle.
 * When the Oak dialogue window closes, the game calls the vanilla function at 0x080EB524, which redraws the ORIGINAL
 * message-box frame into BG0 rows 14-19 with FillBgTilemapBufferRect (frame tiles 3-17). Those tile numbers belong to
 * the vanilla art; with the new textbox the same slots hold different pieces (colour tabs, buttons, panel fill), so the
 * frame came out scrambled. This replacement draws the NEW message panel (same cells, same rows) instead.
 * Hooked in 'hooks'; the vanilla function is replaced entirely and returns straight to its caller.
 */
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
	//Cursor is part of the button art; the next Create call redraws the right state
}

void BTB_MoveSelectionCreateCursorAt(u8 cursorPos, u8 baseTileNum)
{
	//Same as vanilla (1x2 arrow tiles in the move box)...
	BTB_UploadRingAndCursorTiles();
	u16* buf = GetBgTilemapBuffer(0);
	u8 x = (cursorPos & 1) * 9 + 1;
	u8 y = (cursorPos & 2) + 55;
	buf[y * 32 + x] = baseTileNum + 1;
	buf[(y + 1) * 32 + x] = baseTileNum + 2;
	CopyBgTilemapBufferToVram(0);

	//...plus: the type/category strip. Palette 6 index 14 is the dark pill colour; the strip background is repainted
	//to index 14 too (see BTB_BlitMoveInfoIcon), so it becomes a slightly lighter "plate" while letters stay white.
	BTB_SetPal(6, 14, BTB_PLATE_COLOUR);
}

//Vanilla blit_move_info_icon rewritten (same behaviour) + in battle, window 8 (type/category strip) gets a dark plate:
//every colour-15 (white) pixel connected to the window border becomes colour 14, so the letters/symbols inside
//the icons (also colour 15) stay white.
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
	u8 mask[128]; //1 bit per pixel (max 64x16)
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
	//Data pointers come from the original function's literal pool (CFRU repoints them, so read them at runtime)
	const u8* table = (const u8*) *(vu32*) 0x8107DAC;
	const u8* gfx = (const u8*) *(vu32*) 0x8107DB0;
	const u8* entry = table + (iconId << 2);
	u16 offset = entry[2] | (entry[3] << 8);
	BlitBitmapRectToWindow(windowId, gfx + offset * 32, 0, 0, 128, 128, x, y, entry[0], entry[1]);

	if (windowId == 8 && BTB_GMAIN_CB2 == BTB_BATTLE_CB2)
		BTB_PaintStripPlate(windowId);
}

//---------------------------------------------------------------------------------------------------------------------
// Safari Zone: balls counter
// The vanilla "SAFARI BALLS / Left: N" box is a healthbox sprite that gets text blitted into its tiles. Its art does not
// survive the hack's healthbox graphics, so it is replaced by a text window drawn on BG0 (over the sprite, which is hidden).
//---------------------------------------------------------------------------------------------------------------------
#define BTB_SAFARI_WIN_BASE 0x110 //48 free tiles (not used by any battle window in Safari battles)
#define BTB_SAFARI_COL      17    //Tile column / row of the panel (screen position x=136, y=72)
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
	u8 colours[3] = {15, 1, 6}; //panel colour, text, shadow (palette 0)
	u8 text[16];
	u16 win = AddWindow(&template);
	if (win == 0xFF)
		return;

	CpuSet(gBTBSafariCounterTiles, BTB_GWINDOWS[win].tileData, 48 * 16); //Panel art
	AddTextPrinterParameterized3(win, BTB_SAFARI_FONT, 6, 4, colours, 0, (const u8*) 0x83FE80F); //"SAFARI BALLS"
	ConvertIntToDecimalStringN(StringCopy(text, (const u8*) 0x83FE81F), BTB_SAFARI_BALLS, STR_CONV_MODE_LEFT_ALIGN, 5); //"Left: N"
	s32 width = ((BTB_StringWidthFn) 0x8005ED5)(BTB_SAFARI_FONT, text, 0);
	if (width > 80) width = 80; //Very large (expanded) ball counts must not leave the panel
	AddTextPrinterParameterized3(win, BTB_SAFARI_FONT, 90 - width, 16, colours, 0, text);

	//Copy straight to VRAM (the window is removed right away) and fill the tilemap for both scroll positions
	CpuSet(BTB_GWINDOWS[win].tileData, (void*) (0x6000000 + BTB_SAFARI_WIN_BASE * 32), 48 * 16);
	u16* buf = GetBgTilemapBuffer(0);
	for (u8 r = 0; r < 4; ++r)
	{
		for (u8 c = 0; c < 12; ++c)
		{
			u16 entry = BTB_SAFARI_WIN_BASE + r * 12 + c;
			buf[(BTB_SAFARI_ROW + r) * 32 + BTB_SAFARI_COL + c] = entry;      //message state
			buf[(BTB_SAFARI_ROW + 20 + r) * 32 + BTB_SAFARI_COL + c] = entry; //action menu state (BG scrolled 20 rows)
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

	if ((gBattleTypeFlags & 0x80) && healthboxSpriteId == gHealthboxSpriteIds[0]) //Safari player box stays hidden
	{
		BTB_HideSafariHealthbox(healthboxSpriteId);
		BTB_DrawSafariCounter(); //Called when the box would slide in: the BG0 tilemap may have been reset by now
		return;
	}

	sprite->invisible = FALSE;
	gSprites[sprite->data[5]].invisible = FALSE; //healthbar sprite
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
