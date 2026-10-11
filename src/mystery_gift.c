#include "defines.h"
#include "../include/bg.h"
#include "../include/gpu_regs.h"
#include "../include/main.h"
#include "../include/malloc.h"
#include "../include/menu.h"
#include "../include/palette.h"
#include "../include/random.h"
#include "../include/script.h"
#include "../include/scanline_effect.h"
#include "../include/sound.h"
#include "../include/sprite.h"
#include "../include/task.h"
#include "../include/text.h"
#include "../include/window.h"
#include "../include/event_data.h"
#include "../include/overworld.h"
#include "../include/battle.h"
#include "../include/item.h"
#include "../include/item_icon.h"
#include "../include/pokemon.h"
#include "../include/string_util.h"
#include "../include/trig.h"
#include "../include/trainer_pokemon_sprites.h"
#include "../include/constants/songs.h"
#include "../include/constants/items.h"
#include "../include/constants/moves.h"
#include "../include/new/build_pokemon.h"
#include "../include/new/catching.h"
#include "../include/new/dns.h"
#include "../include/new/item.h"
#include "../include/new/util.h"
#include "../include/new/mystery_gift.h"

#include "mystery_gift_codes.h"	//<<< SEUS CODIGOS FICAM NESSE ARQUIVO

/*
 * MYSTERY GIFT
 *
 * Flow:
 *   MysteryGift_Start (callasm/special) -> fade out -> CB2_MysteryGift (full screen)
 *   1) code screen (on-screen keyboard)          -> BG1 + text on BG0
 *   2) correct code: stage + falling gift        -> BG2 (BG1 hidden)
 *      -> shakes -> opens (lid flies off, burst) -> white flash -> the Pokemon appears
 *   3) A/B: leaves and the script continues (LASTRESULT = 1 if something was received)
 *
 * WARNING (same as the Stars package): in this build .bss lives in ROM, so there are NO writable
 * global variables. All state is kept in gTasks[].data[], in the gStringVar buffers and in Var8004-8006.
 *   gStringVar2 = typed code (ASCII)        Var8004 = index of the matching code
 *   Var8005     = result of GiveMonToPlayer Var8006 = timer of the message on the bottom strip
 */

//-------------------------------------------------------------------------------------------------
// Layout (the same numbers used by the graphics generator)
//-------------------------------------------------------------------------------------------------
#define CODE_LEN 8
//Code slots: each one is a block of 3x2 tiles (24x16 px) on BG1; the typed character is swapped in the tile map
#define SLOT_COL0 3			//Column (in tiles) of the first slot
#define SLOT_ROW 5			//Row (in tiles)
#define SLOT_TILES_W 3
#define SLOT_PIXEL_W 24
#define SLOT_VISUAL_X 4			//The plate (16x16) sits 4 px from the block edge
#define CARET_Y 36

#define KEY_X0 11
#define KEY_Y0 63
#define KEY_PX 22
#define KEY_PY 19
#define KEY_W 20
#define KEY_H 17
#define KEY_COUNT 38		//0-9, A-Z, DEL, OK
#define KEY_DEL 36
#define KEY_OK 37

#define BOX_X 120		//Center of the gift box
#define BOX_Y 46		//Top of the gift box (64x64)
#define BURST_Y 78
#define MON_X 120
#define MON_Y 72

#define WIN_MAIN 0

//Timings (frames, 60 per second)
#define T_WAIT 18
#define T_SHAKE 90
#define T_OPEN_FLASH 28		//When the white flash starts after the lid flies off
#define T_MSG 120		//How long the error message stays on screen
#define T_REVEAL_CRY 50
#define T_REVEAL_INPUT 110

#define MG_PAL_FX_TAG 0x4D50
#define MG_TILE_LID_TAG 0x4D51
#define MG_TILE_BASE_TAG 0x4D52
#define MG_TILE_BURST_TAG 0x4D53
#define MG_TILE_BURST2_TAG 0x4D58
#define MG_TILE_SPARK_TAG 0x4D54
#define MG_TILE_CURSOR_TAG 0x4D55
#define MG_TILE_CARET_TAG 0x4D56
#define MG_PAL_CURSOR_TAG 0x4D57
#define MG_ITEM_ICON_TAG 0x4D59
#define ITEM_Y 70			//Center of the item icon on the item page
#define MG_MON_PAL_SLOT 4	//OBJ palette used by the Pokemon sprite (the others use 0 and 1)

#define NUM_SPARKLES 4

//TRUE = every Pokemon given by the Mystery Gift has IVs 31 in all stats. FALSE = random CFRU IVs.
#define MG_PERFECT_IVS TRUE

//State (gTasks[].data[])
#define tState data[0]
#define tTimer data[1]
#define tCursor data[2]
#define tLen data[3]
#define tBufLo data[4]
#define tBufHi data[5]
#define tSprCursor data[6]
#define tSprCaret data[7]
#define tSprBase data[8]
#define tSprLid data[9]
#define tSprBurst data[10]
#define tSprMon data[11]
#define tBuf1Lo data[12]		//BG1 tile map buffer (the typed characters are swapped in it)
#define tBuf1Hi data[13]
#define tPage data[14]			//0 = Pokemon page, 1 = item page, 2 = could not give the Pokemon

//IDs of the 4 sparkles: kept in gStringVar3 (a task only has 16 slots)
#define SPARK_ID(i) gStringVar3[i]

#define MG_MSG_TIMER Var8006
#define MG_GIFT_ID Var8004
#define MG_SENT_RESULT Var8005

enum
{
	ST_FADE_IN,
	ST_INPUT,
	ST_FADE_TO_STAGE,
	ST_STAGE_FADE_IN,
	ST_DROP,
	ST_WAIT,
	ST_SHAKE,
	ST_OPEN,
	ST_FLASH_OUT,
	ST_FLASH_IN,
	ST_REVEAL,
	ST_PAGE_FLASH_OUT,		//From the Pokemon page to the item page (gift with Pokemon + item)
	ST_PAGE_FLASH_IN,
	ST_EXIT_FADE,
};

//-------------------------------------------------------------------------------------------------
// Texts (English only; the strings live in strings/mystery_gift.string)
//-------------------------------------------------------------------------------------------------
#define MG_DECLARE_TEXT(name) extern const u8 gText_MG_##name[];

MG_DECLARE_TEXT(Title) MG_DECLARE_TEXT(Prompt) MG_DECLARE_TEXT(Hint) MG_DECLARE_TEXT(Invalid)
MG_DECLARE_TEXT(Used) MG_DECLARE_TEXT(Empty) MG_DECLARE_TEXT(NoRoom) MG_DECLARE_TEXT(Arrived1)
MG_DECLARE_TEXT(Arrived2) MG_DECLARE_TEXT(Received) MG_DECLARE_TEXT(GoodLuck) MG_DECLARE_TEXT(SentToPC)
MG_DECLARE_TEXT(BagFull) MG_DECLARE_TEXT(StoredBag)
extern const u8 gText_MG_TimesX[];
extern const u8 gText_MG_Bang[];
extern const u8 gText_MG_KeyDel[];
extern const u8 gText_MG_KeyOk[];

enum
{
	MSG_TITLE, MSG_PROMPT, MSG_HINT, MSG_INVALID, MSG_USED, MSG_EMPTY, MSG_NOROOM,
	MSG_ARRIVED1, MSG_ARRIVED2, MSG_RECEIVED, MSG_GOODLUCK, MSG_SENTPC, MSG_BAGFULL, MSG_STOREDBAG, MSG_COUNT
};

static const u8* const sMgTexts[MSG_COUNT] =
{
	gText_MG_Title, gText_MG_Prompt, gText_MG_Hint, gText_MG_Invalid, gText_MG_Used, gText_MG_Empty,
	gText_MG_NoRoom, gText_MG_Arrived1, gText_MG_Arrived2, gText_MG_Received, gText_MG_GoodLuck, gText_MG_SentToPC,
	gText_MG_BagFull, gText_MG_StoredBag,
};

static const u8* GetMgText(u8 id)
{
	return sMgTexts[id];
}

//Text colors {background, text, shadow} = indexes into the menu palette 0
static const u8 sColWhite[3] = {0, 1, 2};
static const u8 sColPrompt[3] = {0, 13, 3};
static const u8 sColHint[3] = {0, 10, 3};
static const u8 sColError[3] = {0, 8, 9};

//-------------------------------------------------------------------------------------------------
// Graphics (pre-generated data, see src/mystery_gift_gfx.c)
//-------------------------------------------------------------------------------------------------
extern const u16 gMgPalBg[];		//32 colors: palette 0 (interface) + palette 1 (background)
extern const u16 gMgPalFx[];
extern const u16 gMgPalCursor[];
extern const u32 gMgEntryTiles[];
extern const u32 gMgEntryTilesSize;
extern const u32 gMgStageTiles[];
extern const u32 gMgStageTilesSize;
extern const u16 gMgEntryMap[];
extern const u16 gMgSlotGlyphs[];		//[36][6]: letters/numbers of the code slots (3x2 tiles each)
extern const u16 gMgStageMap[];
extern const u32 gMgBoxLidTiles[];
extern const u32 gMgBoxBaseTiles[];
extern const u32 gMgBurstTiles[];
extern const u32 gMgSparkleTiles[];
extern const u32 gMgCursorTiles[];
extern const u32 gMgCaretTiles[];

static const struct BgTemplate sBgTemplates[] =
{
	{ .bg = 0, .charBaseIndex = 2, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0 },	//text
	{ .bg = 1, .charBaseIndex = 0, .mapBaseIndex = 29, .screenSize = 0, .paletteMode = 0, .priority = 1, .baseTile = 0 },	//code screen
	{ .bg = 2, .charBaseIndex = 1, .mapBaseIndex = 28, .screenSize = 0, .paletteMode = 0, .priority = 2, .baseTile = 0 },	//stage
};

static const struct WindowTemplate sWindowTemplates[] =
{
	{ .bg = 0, .tilemapLeft = 0, .tilemapTop = 0, .width = 30, .height = 20, .paletteNum = 0, .baseBlock = 1 },
	DUMMY_WIN_TEMPLATE
};

#define MG_OAM(shp, sz, prio) \
{ .affineMode = ST_OAM_AFFINE_OFF, .objMode = ST_OAM_OBJ_NORMAL, .shape = SPRITE_SHAPE(shp), .size = SPRITE_SIZE(sz), .priority = prio }

static const struct OamData sOam64x32 = MG_OAM(64x32, 64x32, 1);
static const struct OamData sOam64x64 = MG_OAM(64x64, 64x64, 1);
static const struct OamData sOam16x16 = MG_OAM(16x16, 16x16, 1);
static const struct OamData sOam32x32Cursor = MG_OAM(32x32, 32x32, 0);
static const struct OamData sOam8x8Caret = MG_OAM(8x8, 8x8, 0);

static const struct SpriteSheet sSheetLid = {(const u8*) gMgBoxLidTiles, 1024, MG_TILE_LID_TAG};
static const struct SpriteSheet sSheetBase = {(const u8*) gMgBoxBaseTiles, 1024, MG_TILE_BASE_TAG};
static const struct SpriteSheet sSheetBurst = {(const u8*) gMgBurstTiles, 2048, MG_TILE_BURST_TAG};			//Burst, frame A (64x64)
static const struct SpriteSheet sSheetBurst2 = {(const u8*) gMgBurstTiles + 2048, 2048, MG_TILE_BURST2_TAG};	//Burst, frame B
static const struct SpriteSheet sSheetSparkle = {(const u8*) gMgSparkleTiles, 128, MG_TILE_SPARK_TAG};
static const struct SpriteSheet sSheetCursor = {(const u8*) gMgCursorTiles, 512, MG_TILE_CURSOR_TAG};
static const struct SpriteSheet sSheetCaret = {(const u8*) gMgCaretTiles, 32, MG_TILE_CARET_TAG};
static const struct SpritePalette sPalFx = {gMgPalFx, MG_PAL_FX_TAG};
static const struct SpritePalette sPalCursor = {gMgPalCursor, MG_PAL_CURSOR_TAG};

#define MG_TEMPLATE(tileTag_, palTag_, oam_) \
{ .tileTag = tileTag_, .paletteTag = palTag_, .oam = oam_, .anims = gDummySpriteAnimTable, .images = NULL, \
  .affineAnims = gDummySpriteAffineAnimTable, .callback = SpriteCallbackDummy }

static const struct SpriteTemplate sTemplateLid = MG_TEMPLATE(MG_TILE_LID_TAG, MG_PAL_FX_TAG, &sOam64x32);
static const struct SpriteTemplate sTemplateBase = MG_TEMPLATE(MG_TILE_BASE_TAG, MG_PAL_FX_TAG, &sOam64x32);
static const struct SpriteTemplate sTemplateBurst = MG_TEMPLATE(MG_TILE_BURST_TAG, MG_PAL_FX_TAG, &sOam64x64);
static const struct SpriteTemplate sTemplateBurst2 = MG_TEMPLATE(MG_TILE_BURST2_TAG, MG_PAL_FX_TAG, &sOam64x64);
static const struct SpriteTemplate sTemplateSparkle = MG_TEMPLATE(MG_TILE_SPARK_TAG, MG_PAL_FX_TAG, &sOam16x16);
static const struct SpriteTemplate sTemplateCursor = MG_TEMPLATE(MG_TILE_CURSOR_TAG, MG_PAL_CURSOR_TAG, &sOam32x32Cursor);
static const struct SpriteTemplate sTemplateCaret = MG_TEMPLATE(MG_TILE_CARET_TAG, MG_PAL_CURSOR_TAG, &sOam8x8Caret);

//Gift drop (Y offset per frame; the last one is the resting position) and shake (X offset)
static const s8 sDropOffsets[] =
{
	-100, -92, -83, -73, -62, -50, -37, -23, -8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	-5, -9, -12, -14, -15, -14, -12, -9, -5, -1, 0, 0
};
static const s8 sShake[8] = {0, 2, 3, 2, 0, -2, -3, -2};

//-------------------------------------------------------------------------------------------------
// Utilities
//-------------------------------------------------------------------------------------------------
//Sine 0..255 -> -amplitude..+amplitude (CFRU does not export Sin, only gSineTable)
static s16 MgSin(s16 index, s16 amplitude)
{
	return (amplitude * gSineTable[index & 0xFF]) >> 8;
}

static void* MgGetPtr(s16 lo, s16 hi)
{
	return (void*) ((u32) ((u16) lo) | ((u32) ((u16) hi) << 16));
}

static u8 MgKeyAscii(u8 key)	//key 0..35
{
	return (key < 10) ? '0' + key : 'A' + (key - 10);
}

static void MgGetKeyPos(u8 key, u8* col, u8* row)
{
	if (key < 36)
	{
		*col = key % 10;
		*row = key / 10;
	}
	else
	{
		*col = 6 + (key - 36);
		*row = 3;
	}
}

static u8 MgRowLength(u8 row)
{
	return (row == 3) ? 8 : 10;
}

static u8 MgGetKeyIndex(u8 col, u8 row)
{
	if (row < 3)
		return row * 10 + col;

	return (col < 6) ? 30 + col : 36 + (col - 6);
}

static s16 MgKeyCenterX(u8 col) { return KEY_X0 + KEY_PX * col + KEY_W / 2; }
static s16 MgKeyCenterY(u8 row) { return KEY_Y0 + KEY_PY * row + KEY_H / 2; }

static void MgPrintCentered(u8 fontId, s16 centerX, u8 y, const u8* colors, const u8* str)
{
	s32 x = centerX - GetStringWidth(fontId, str, 0) / 2;

	if (x < 0)
		x = 0;

	AddTextPrinterParameterized3(WIN_MAIN, fontId, x, y, colors, 0, str);
}

//-------------------------------------------------------------------------------------------------
// Code entry screen
//-------------------------------------------------------------------------------------------------
static void MgPrintBottomLine(const u8* str, const u8* colors)
{
	FillWindowPixelRect(WIN_MAIN, PIXEL_FILL(0), 9, 144, 222, 15);	//The whole strip (the font is taller than 12 px)
	MgPrintCentered(0, 120, 143, colors, str);
	CopyWindowToVram(WIN_MAIN, COPYWIN_GFX);
}

static u8 MgAsciiToGlyph(u8 ascii)	//'0'-'9' -> 0-9, 'A'-'Z' -> 10-35
{
	return (ascii <= '9') ? ascii - '0' : 10 + (ascii - 'A');
}

//Shows the typed characters by swapping the slot tiles in the BG1 map; empty slots go back to the original tile
static void MgDrawSlots(struct Task* task, u8 len)
{
	u16* buf = MgGetPtr(task->tBuf1Lo, task->tBuf1Hi);
	u32 i, x, y;

	for (i = 0; i < CODE_LEN; ++i)
	{
		for (y = 0; y < 2; ++y)
		{
			for (x = 0; x < SLOT_TILES_W; ++x)
			{
				u32 dest = (SLOT_ROW + y) * 32 + SLOT_COL0 + i * SLOT_TILES_W + x;

				if (i < len)
					buf[dest] = gMgSlotGlyphs[MgAsciiToGlyph(gStringVar2[i]) * 6 + y * SLOT_TILES_W + x];
				else
					buf[dest] = gMgEntryMap[dest];
			}
		}
	}
	CopyBgTilemapBufferToVram(1);
}

static void MgDrawEntryScreen(void)
{
	FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(0));
	MgPrintCentered(2, 120, 4, sColWhite, GetMgText(MSG_TITLE));
	MgPrintCentered(0, 120, 23, sColPrompt, GetMgText(MSG_PROMPT));
	MgPrintCentered(0, 120, 143, sColHint, GetMgText(MSG_HINT));	//The keys and letters are drawn on BG1
	PutWindowTilemap(WIN_MAIN);
	CopyWindowToVram(WIN_MAIN, COPYWIN_BOTH);
}

static void MgShowMessage(u8 msgId)
{
	PlaySE(SE_WALL_HIT);
	MgPrintBottomLine(GetMgText(msgId), sColError);
	MG_MSG_TIMER = T_MSG;
}

static u8 MgFindCode(u8 len)
{
	u32 i, j;

	for (i = 0; i < NELEMS(sMysteryGifts); ++i)
	{
		const char* code = sMysteryGifts[i].code;
		u8 n = 0;
		bool8 match = TRUE;

		while (code[n] != 0)
			++n;

		if (n != len)
			continue;

		for (j = 0; j < len; ++j)
		{
			u8 c = code[j];

			if (c >= 'a' && c <= 'z')
				c -= 'a' - 'A';	//Accepts lowercase letters in the table

			if (c != gStringVar2[j])
			{
				match = FALSE;
				break;
			}
		}

		if (match)
			return i;
	}

	return 0xFF;
}

//Creates the Pokemon in gEnemyParty[0] (only to show it in the animation). MgGiveRewards is what gives it to the player.
static void MgBuildGiftMon(u8 id)
{
	const struct MysteryGiftEntry* gift = &sMysteryGifts[id];
	struct Pokemon* mon = &gEnemyParty[0];
	u16 item = gift->heldItem;
	u8 nature = gift->nature;

	ZeroMonData(mon);
	CreateMon(mon, gift->species, gift->level, MG_PERFECT_IVS ? 31 : 32, 0, 0, 0, 0);	//fixedIV < 32: CFRU writes this value to all 6 IVs

	if (item != ITEM_NONE)
		SetMonData(mon, MON_DATA_HELD_ITEM, &item);

	if (gift->hiddenAbility)
		mon->hiddenAbility = TRUE;

	if (gift->moves[0] != MOVE_NONE)	//Custom moves (replace the level-up ones; empty slots stay empty)
	{
		u32 i;

		for (i = 0; i < MAX_MON_MOVES; ++i)
		{
			u16 move = gift->moves[i];
			u8 pp = (move != MOVE_NONE) ? gBattleMoves[move].pp : 0;

			SetMonData(mon, MON_DATA_MOVE1 + i, &move);
			SetMonData(mon, MON_DATA_PP1 + i, &pp);
		}
	}

	if (nature >= NUM_NATURES)
		nature = Random() % NUM_NATURES;

	GiveMonNatureAndAbility(mon, nature, GetMonData(mon, MON_DATA_PERSONALITY, NULL) & 1, gift->shiny ? TRUE : IsMonShiny(mon), FALSE, FALSE);
	CalculateMonStats(mon);
	HealMon(mon);
}

//Gives everything at once (Pokemon and/or item) and marks the code as claimed.
//Returns FALSE (without using up the code) if the Pokemon could not be given for lack of room.
static bool8 MgGiveRewards(void)
{
	const struct MysteryGiftEntry* gift = &sMysteryGifts[MG_GIFT_ID];

	if (gift->species != SPECIES_NONE)
	{
		struct Pokemon* mon = &gEnemyParty[0];
		u8 sent = GiveMonToPlayer(mon);

		if (sent > 1)
			return FALSE;	//No room in the party or the PC: nothing was given, the code is still valid

		SetMonPokedexFlags(mon);
		MG_SENT_RESULT = sent;
	}

	if (gift->rewardItem != ITEM_NONE)
		AddBagItem(gift->rewardItem, gift->rewardQty);

	FlagSet(MYSTERY_GIFT_FLAG_BASE + gift->slot);	//Each code can only be claimed once
	gSpecialVar_LastResult = 1;
	return TRUE;
}

static void MgBeginExit(struct Task* task)
{
#if MYSTERY_GIFT_MENU_MUSIC != 0
	FadeOutBGM(4);
#endif
	BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
	task->tState = ST_EXIT_FADE;
}

static void MgTrySubmit(struct Task* task)
{
	const struct MysteryGiftEntry* gift;
	u8 id;

	if (task->tLen == 0)
	{
		MgShowMessage(MSG_EMPTY);
		return;
	}

	id = MgFindCode(task->tLen);

	if (id == 0xFF)
	{
		MgShowMessage(MSG_INVALID);
		return;
	}

	gift = &sMysteryGifts[id];

	if (FlagGet(MYSTERY_GIFT_FLAG_BASE + gift->slot))
		MgShowMessage(MSG_USED);
	else if (gift->species != SPECIES_NONE && gPlayerPartyCount >= PARTY_SIZE && IsPokemonStorageFull())
		MgShowMessage(MSG_NOROOM);
	else if (gift->rewardItem != ITEM_NONE && !CheckBagHasSpace(gift->rewardItem, gift->rewardQty))
		MgShowMessage(MSG_BAGFULL);
	else
	{
		MG_GIFT_ID = id;

		if (gift->species != SPECIES_NONE)
			MgBuildGiftMon(id);

		PlaySE(SE_SELECT);
		BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
		task->tState = ST_FADE_TO_STAGE;
	}
}

static void MgUpdateCursorSprites(struct Task* task)
{
	struct Sprite* cursor = &gSprites[task->tSprCursor];
	struct Sprite* caret = &gSprites[task->tSprCaret];
	u8 col, row;
	u16 color;

	MgGetKeyPos(task->tCursor, &col, &row);
	cursor->pos1.x = MgKeyCenterX(col);
	cursor->pos1.y = MgKeyCenterY(row);

	//Blinks between gold and white
	color = (task->tTimer & 16) ? RGB(31, 31, 31) : RGB(31, 26, 9);
	LoadPalette(&color, 256 + IndexOfSpritePaletteTag(MG_PAL_CURSOR_TAG) * 16 + 1, 2);

	caret->invisible = (task->tLen >= CODE_LEN);
	caret->pos1.x = SLOT_COL0 * 8 + task->tLen * SLOT_PIXEL_W + SLOT_VISUAL_X + 8;
	caret->pos1.y = CARET_Y + ((task->tTimer >> 3) & 1);
}

static void MgEntryInput(struct Task* task)
{
	u8 col, row;
	bool8 moved = FALSE;

	MgGetKeyPos(task->tCursor, &col, &row);

	if (JOY_NEW_AND_REPEATED(DPAD_RIGHT))
	{
		col = (col + 1 >= MgRowLength(row)) ? 0 : col + 1;
		moved = TRUE;
	}
	else if (JOY_NEW_AND_REPEATED(DPAD_LEFT))
	{
		col = (col == 0) ? MgRowLength(row) - 1 : col - 1;
		moved = TRUE;
	}
	else if (JOY_NEW_AND_REPEATED(DPAD_DOWN))
	{
		row = (row >= 3) ? 0 : row + 1;
		if (col >= MgRowLength(row))
			col = MgRowLength(row) - 1;
		moved = TRUE;
	}
	else if (JOY_NEW_AND_REPEATED(DPAD_UP))
	{
		row = (row == 0) ? 3 : row - 1;
		if (col >= MgRowLength(row))
			col = MgRowLength(row) - 1;
		moved = TRUE;
	}

	if (moved)
	{
		task->tCursor = MgGetKeyIndex(col, row);
		PlaySE(SE_SELECT);
	}
	else if (JOY_NEW(A_BUTTON))
	{
		if (task->tCursor == KEY_OK)
			MgTrySubmit(task);
		else if (task->tCursor == KEY_DEL)
			goto backspace;
		else if (task->tLen < CODE_LEN)
		{
			gStringVar2[task->tLen] = MgKeyAscii(task->tCursor);
			++task->tLen;
			gStringVar2[task->tLen] = 0;
			PlaySE(SE_SELECT);
			MgDrawSlots(task, task->tLen);
		}
	}
	else if (JOY_NEW(B_BUTTON))
	{
	backspace:
		if (task->tLen > 0)
		{
			--task->tLen;
			gStringVar2[task->tLen] = 0;
			PlaySE(SE_SELECT);
			MgDrawSlots(task, task->tLen);
		}
		else if (!JOY_NEW(A_BUTTON))
		{
			gSpecialVar_LastResult = 0;	//Left without receiving anything
			MgBeginExit(task);
		}
	}
	else if (JOY_NEW(START_BUTTON))
		MgTrySubmit(task);

	//The error message goes away by itself
	if (MG_MSG_TIMER != 0 && --MG_MSG_TIMER == 0)
		MgPrintBottomLine(GetMgText(MSG_HINT), sColHint);
}

//-------------------------------------------------------------------------------------------------
// Stage / gift animation
//-------------------------------------------------------------------------------------------------
static void MgPrintDialog(const u8* line1, const u8* line2)
{
	FillWindowPixelBuffer(WIN_MAIN, PIXEL_FILL(0));
	MgPrintCentered(2, 120, 126, sColWhite, line1);
	MgPrintCentered(2, 120, 140, sColWhite, line2);
	CopyWindowToVram(WIN_MAIN, COPYWIN_GFX);
}

static void MgSetBoxPos(struct Task* task, s16 offsetX, s16 offsetY)
{
	gSprites[task->tSprLid].pos1.x = BOX_X + offsetX;
	gSprites[task->tSprLid].pos1.y = BOX_Y + offsetY + 16;
	gSprites[task->tSprBase].pos1.x = BOX_X + offsetX;
	gSprites[task->tSprBase].pos1.y = BOX_Y + offsetY + 48;
}

static void MgHideSparkles(void)
{
	u32 i;

	for (i = 0; i < NUM_SPARKLES; ++i)
		gSprites[SPARK_ID(i)].invisible = TRUE;
}

static void MgEnterStage(struct Task* task)
{
	u32 i;

	//The code screen leaves the scene
	HideBg(1);
	ShowBg(2);
	DestroySprite(&gSprites[task->tSprCursor]);
	DestroySprite(&gSprites[task->tSprCaret]);
	FreeSpriteTilesByTag(MG_TILE_CURSOR_TAG);
	FreeSpriteTilesByTag(MG_TILE_CARET_TAG);
	FreeSpritePaletteByTag(MG_PAL_CURSOR_TAG);

	//The gift (lid and body) starts up high
	LoadSpriteSheet(&sSheetLid);
	LoadSpriteSheet(&sSheetBase);
	LoadSpriteSheet(&sSheetBurst);
	LoadSpriteSheet(&sSheetBurst2);
	LoadSpriteSheet(&sSheetSparkle);
	task->tSprLid = CreateSprite(&sTemplateLid, BOX_X, BOX_Y + 16, 1);
	task->tSprBase = CreateSprite(&sTemplateBase, BOX_X, BOX_Y + 48, 2);
	MgSetBoxPos(task, 0, sDropOffsets[0]);

	for (i = 0; i < NUM_SPARKLES; ++i)
	{
		SPARK_ID(i) = CreateSprite(&sTemplateSparkle, BOX_X, BOX_Y, 0);
		gSprites[SPARK_ID(i)].invisible = TRUE;
	}

	task->tSprBurst = 0xFF;
	task->tSprMon = 0xFF;
	MgPrintDialog(GetMgText(MSG_ARRIVED1), GetMgText(MSG_ARRIVED2));
}

static void MgUpdateSparklesCircle(u16 time, s16 centerX, s16 centerY, s16 radiusX, s16 radiusY, u8 speed)
{
	u32 i;

	for (i = 0; i < NUM_SPARKLES; ++i)
	{
		struct Sprite* spark = &gSprites[SPARK_ID(i)];
		u8 angle = (i * 64 + time * speed) & 0xFF;

		spark->pos1.x = centerX + MgSin(angle + 64, radiusX);
		spark->pos1.y = centerY + MgSin(angle, radiusY);
		spark->invisible = FALSE;
	}
}

static void MgCreateMonPic(struct Task* task)
{
	struct Pokemon* mon = &gEnemyParty[0];
	u16 id = CreateMonPicSprite_HandleDeoxys(mon->species, mon->otid, mon->personality, TRUE, MON_X, MON_Y, MG_MON_PAL_SLOT, 0xFFFF);

	if (id == 0xFFFF)
		task->tSprMon = 0xFF;
	else
	{
		task->tSprMon = id;
		gSprites[id].oam.priority = 1;
	}
}

static void MgPrintReceived(void)
{
	const struct Pokemon* mon = &gEnemyParty[0];

	StringCopy(gStringVar4, GetMgText(MSG_RECEIVED));
	StringAppend(gStringVar4, gSpeciesNames[mon->species]);
	StringAppend(gStringVar4, gText_MG_Bang);
	MgPrintDialog(gStringVar4, GetMgText(MG_SENT_RESULT == 1 ? MSG_SENTPC : MSG_GOODLUCK));
}

//Item page: the item icon in the center and the text "You received <item> xN!"
static void MgShowItemPage(struct Task* task)
{
	const struct MysteryGiftEntry* gift = &sMysteryGifts[MG_GIFT_ID];
	u8 icon = AddItemIconSprite(MG_ITEM_ICON_TAG, MG_ITEM_ICON_TAG, gift->rewardItem);

	if (icon < MAX_SPRITES)
	{
		gSprites[icon].pos1.x = MON_X;
		gSprites[icon].pos1.y = ITEM_Y;
		gSprites[icon].oam.priority = 1;
		gSprites[icon].invisible = FALSE;
		task->tSprMon = icon;
	}
	else
		task->tSprMon = 0xFF;

	task->tPage = 1;
	StringCopy(gStringVar4, GetMgText(MSG_RECEIVED));
	StringAppend(gStringVar4, ItemId_GetName(gift->rewardItem));

	if (gift->rewardQty > 1)
	{
		ConvertIntToDecimalStringN(gStringVar1, gift->rewardQty, STR_CONV_MODE_LEFT_ALIGN, 3);
		StringAppend(gStringVar4, gText_MG_TimesX);
		StringAppend(gStringVar4, gStringVar1);
	}

	StringAppend(gStringVar4, gText_MG_Bang);
	MgPrintDialog(gStringVar4, GetMgText(MSG_STOREDBAG));
}

//Frees the sprite on screen (Pokemon or item)
static void MgFreeShownSprite(struct Task* task)
{
	if (task->tSprMon == 0xFF)
		return;

	if (task->tPage == 0)
		FreeAndDestroyMonPicSprite(task->tSprMon);
	else
	{
		DestroySprite(&gSprites[task->tSprMon]);
		FreeSpriteTilesByTag(MG_ITEM_ICON_TAG);
		FreeSpritePaletteByTag(MG_ITEM_ICON_TAG);
	}

	task->tSprMon = 0xFF;
}

//Common part of the exit: restores the game state and returns to the map (the script continues after waitstate)
static void MgReturnToField(void)
{
	ZeroMonData(&gEnemyParty[0]);	//Do not leave the gift Pokemon lying around in memory
	SetVBlankCallback(NULL);
	ResetSpriteData();
	FreeAllSpritePalettes();
#if MYSTERY_GIFT_MENU_MUSIC != 0
	PlayNewMapMusic(GetCurrentMapMusic());	//Restores the map music (the game only switches if it differs from the current one)
#endif
	CB2_ReturnToFieldContinueScriptPlayMapMusic();
}

static void MgCleanupAndExit(u8 taskId)
{
	struct Task* task = &gTasks[taskId];
	void* tm0 = MgGetPtr(task->tBufLo, task->tBufHi);
	void* tm1 = MgGetPtr(task->tBuf1Lo, task->tBuf1Hi);

	MgFreeShownSprite(task);
	FreeAllWindowBuffers();

	if (tm0 != NULL)
		Free(tm0);

	if (tm1 != NULL)
		Free(tm1);

	DestroyTask(taskId);
	MgReturnToField();
}

static void Task_MysteryGift(u8 taskId)
{
	struct Task* task = &gTasks[taskId];
	u16 t = task->tTimer;

	++task->tTimer;

	switch (task->tState)
	{
		case ST_FADE_IN:
			MgUpdateCursorSprites(task);
			if (!gPaletteFade->active)
				task->tState = ST_INPUT;
			break;

		case ST_INPUT:
			MgUpdateCursorSprites(task);
			MgEntryInput(task);
			break;

		case ST_FADE_TO_STAGE:
			if (!gPaletteFade->active)
			{
				MgEnterStage(task);
				task->tTimer = 0;
				BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
				task->tState = ST_STAGE_FADE_IN;
			}
			break;

		case ST_STAGE_FADE_IN:
			if (!gPaletteFade->active)
			{
				task->tTimer = 0;
				task->tState = ST_DROP;
			}
			break;

		case ST_DROP:	//The gift drops and bounces
			MgSetBoxPos(task, 0, sDropOffsets[t]);
			if (t == 8 || t == 28)
				PlaySE(SE_WALL_HIT);
			if ((u32) (t + 1) >= NELEMS(sDropOffsets))
			{
				task->tTimer = 0;
				task->tState = ST_WAIT;
			}
			break;

		case ST_WAIT:
			if (t >= T_WAIT)
			{
				task->tTimer = 0;
				task->tState = ST_SHAKE;
			}
			break;

		case ST_SHAKE:	//Shakes more and more, releasing sparkles
		{
			u8 amp = 1 + (t >= 30) + (t >= 60);

			MgSetBoxPos(task, sShake[(t >> 1) & 7] * amp / 2, 0);

			if (t % 15 == 0)
				PlaySE(SE_SELECT);

			if (t >= 30)
			{
				u32 i;

				for (i = 0; i < NUM_SPARKLES - 1; ++i)
				{
					struct Sprite* spark = &gSprites[SPARK_ID(i)];
					u8 angle = (t * 5 + i * 85) & 0xFF;

					spark->pos1.x = BOX_X + MgSin(angle + 64, 30 + (t % 30) / 2);
					spark->pos1.y = BOX_Y - 4 - ((t + i * 10) % 28);
					spark->invisible = FALSE;
				}
			}

			if (t + 1 >= T_SHAKE)
			{
				//Opens: creates the burst and releases the lid
				task->tSprBurst = CreateSprite(&sTemplateBurst, BOX_X, BURST_Y, 3);
				gSprites[task->tSprBurst].data[7] = CreateSprite(&sTemplateBurst2, BOX_X, BURST_Y, 3);	//Frame B is kept in data[7] of A
				MgSetBoxPos(task, 0, 0);
				PlaySE(SE_SHINY);
				task->tTimer = 0;
				task->tState = ST_OPEN;
			}
			break;
		}

		case ST_OPEN:
		{
			struct Sprite* lid = &gSprites[task->tSprLid];
			struct Sprite* burst = &gSprites[task->tSprBurst];

			if (!lid->invisible)
			{
				lid->pos1.y -= 6;			//The lid flies upward
				lid->pos1.x += 1;
				if (lid->pos1.y < -20)
					lid->invisible = TRUE;
			}

			burst->invisible = (t / 3) & 1;					//Alternates the two burst frames
			gSprites[burst->data[7]].invisible = !burst->invisible;
			MgUpdateSparklesCircle(t, BOX_X, BURST_Y, 14 + t * 8 / 5, 14 + t * 6 / 5, 9);

			if (t >= T_OPEN_FLASH)
			{
				BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_WHITE);
				task->tState = ST_FLASH_OUT;
			}
			break;
		}

		case ST_FLASH_OUT:
			if (!gPaletteFade->active)
			{
				//All white: swaps the gift for the Pokemon and actually gives it
				DestroySprite(&gSprites[task->tSprLid]);
				DestroySprite(&gSprites[task->tSprBase]);
				DestroySprite(&gSprites[gSprites[task->tSprBurst].data[7]]);
				DestroySprite(&gSprites[task->tSprBurst]);
				task->tSprBurst = 0xFF;
				FreeSpriteTilesByTag(MG_TILE_LID_TAG);
				FreeSpriteTilesByTag(MG_TILE_BASE_TAG);
				FreeSpriteTilesByTag(MG_TILE_BURST_TAG);
				FreeSpriteTilesByTag(MG_TILE_BURST2_TAG);
				MgHideSparkles();
				if (!MgGiveRewards())
				{
					static const u8 sEmptyLine[] = {EOS};

					task->tPage = 2;	//Failed: shows the warning and leaves without using up the code
					task->tSprMon = 0xFF;
					MgPrintDialog(GetMgText(MSG_NOROOM), sEmptyLine);
				}
				else if (sMysteryGifts[MG_GIFT_ID].species != SPECIES_NONE)
				{
					task->tPage = 0;
					MgCreateMonPic(task);
					MgPrintReceived();
				}
				else
					MgShowItemPage(task);	//Item-only gift
				BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_WHITE);
				task->tTimer = 0;
				task->tState = ST_FLASH_IN;
			}
			break;

		case ST_FLASH_IN:
			if (!gPaletteFade->active)
			{
				if (task->tPage != 2)
					PlayFanfare(MUS_FANFA1);

				task->tTimer = 0;
				task->tState = ST_REVEAL;
			}
			break;

		case ST_REVEAL:
			if (task->tSprMon != 0xFF)
				gSprites[task->tSprMon].pos1.y = (task->tPage == 0 ? MON_Y : ITEM_Y) + MgSin((t * 8) & 0xFF, 2);	//Sways gently

			MgUpdateSparklesCircle(t, MON_X, 70, 50, 34, 4);

			if (t == T_REVEAL_CRY && task->tPage == 0)
				PlayCry1(gEnemyParty[0].species, 0);

			if (t >= T_REVEAL_INPUT && (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON)))
			{
				if (task->tPage == 0 && sMysteryGifts[MG_GIFT_ID].rewardItem != ITEM_NONE)
				{
					BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_WHITE);	//Gift with Pokemon + item: shows the item
					task->tState = ST_PAGE_FLASH_OUT;
				}
				else
					MgBeginExit(task);
			}
			break;

		case ST_PAGE_FLASH_OUT:
			if (!gPaletteFade->active)
			{
				MgFreeShownSprite(task);
				MgHideSparkles();
				MgShowItemPage(task);
				BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_WHITE);
				task->tState = ST_PAGE_FLASH_IN;
			}
			break;

		case ST_PAGE_FLASH_IN:
			if (!gPaletteFade->active)
			{
				PlayFanfare(MUS_FANFA1);
				task->tTimer = 0;
				task->tState = ST_REVEAL;
			}
			break;

		case ST_EXIT_FADE:
			if (!gPaletteFade->active)
				MgCleanupAndExit(taskId);
			break;
	}
}

//-------------------------------------------------------------------------------------------------
// Start / end of the screen
//-------------------------------------------------------------------------------------------------
static void VBlankCB_MysteryGift(void)
{
	LoadOam();
	ProcessSpriteCopyRequests();
	TransferPlttBuffer();
}

static void CB2_MysteryGiftMain(void)
{
	RunTasks();
	AnimateSprites();
	BuildOamBuffer();
	UpdatePaletteFade();
}

static void CB2_MysteryGift(void)
{
	u8 taskId;
	u16* tm0;
	u16* tm1;
	u32 i;
	u8 cursorId, caretId;

	//Clears what the map left behind
	SetVBlankCallback(NULL);
	SetHBlankCallback(NULL);
	SetGpuReg(REG_OFFSET_DISPCNT, 0);
	SetGpuReg(REG_OFFSET_BG0CNT, 0);
	SetGpuReg(REG_OFFSET_BG1CNT, 0);
	SetGpuReg(REG_OFFSET_BG2CNT, 0);
	SetGpuReg(REG_OFFSET_BG3CNT, 0);
	SetGpuReg(REG_OFFSET_BG0HOFS, 0);
	SetGpuReg(REG_OFFSET_BG0VOFS, 0);
	SetGpuReg(REG_OFFSET_BG1HOFS, 0);
	SetGpuReg(REG_OFFSET_BG1VOFS, 0);
	SetGpuReg(REG_OFFSET_BG2HOFS, 0);
	SetGpuReg(REG_OFFSET_BG2VOFS, 0);
	SetGpuReg(REG_OFFSET_BG3HOFS, 0);
	SetGpuReg(REG_OFFSET_BG3VOFS, 0);
	SetGpuReg(REG_OFFSET_WIN0H, 0);
	SetGpuReg(REG_OFFSET_WIN1H, 0);
	SetGpuReg(REG_OFFSET_WIN0V, 0);
	SetGpuReg(REG_OFFSET_WIN1V, 0);
	SetGpuReg(REG_OFFSET_WININ, 0);
	SetGpuReg(REG_OFFSET_WINOUT, 0);
	SetGpuReg(REG_OFFSET_MOSAIC, 0);
	SetGpuReg(REG_OFFSET_BLDCNT, 0);
	SetGpuReg(REG_OFFSET_BLDALPHA, 0);
	SetGpuReg(REG_OFFSET_BLDY, 0);

	DmaFill16(3, 0, (void*) VRAM, VRAM_SIZE);
	DmaFill32(3, 0, (void*) OAM, OAM_SIZE);
	DmaFill16(3, 0, (void*) PLTT, PLTT_SIZE);

	ScanlineEffect_Stop();
	ResetTasks();
	ResetSpriteData();
	FreeAllSpritePalettes();
	ResetPaletteFade();
	ResetAllPicSprites();

	//Backgrounds and text window
	ResetBgsAndClearDma3BusyFlags(0);
	InitBgsFromTemplates(0, sBgTemplates, NELEMS(sBgTemplates));
	tm0 = AllocZeroed(BG_SCREEN_SIZE);
	tm1 = AllocZeroed(BG_SCREEN_SIZE);

	if (tm0 == NULL || tm1 == NULL)		//Out of memory: return to the map without opening the screen (never writes through a null pointer)
	{
		if (tm0 != NULL)
			Free(tm0);
		if (tm1 != NULL)
			Free(tm1);

		gSpecialVar_LastResult = 0;
		MgReturnToField();
		return;
	}

	SetBgTilemapBuffer(0, tm0);
	SetBgTilemapBuffer(1, tm1);

	if (!InitWindows(sWindowTemplates))	//Could not get the text window buffer
	{
		FreeAllWindowBuffers();
		Free(tm0);
		Free(tm1);
		gSpecialVar_LastResult = 0;
		MgReturnToField();
		return;
	}

	DeactivateAllTextPrinters();

	//Graphics of the two backgrounds (go straight into VRAM, no buffer)
	LoadBgTiles(1, gMgEntryTiles, gMgEntryTilesSize, 0);
	for (i = 0; i < 32 * 20; ++i)
		tm1[i] = gMgEntryMap[i];
	CopyBgTilemapBufferToVram(1);
	LoadBgTiles(2, gMgStageTiles, gMgStageTilesSize, 0);
	LoadBgTilemap(2, gMgStageMap, 32 * 20 * 2, 0);

	//Palettes: background = 32 colors (2 palettes of 16); objects = sparkles/gift and cursor
	LoadPalette(gMgPalBg, 0, 64);
	LoadSpriteSheet(&sSheetCursor);
	LoadSpriteSheet(&sSheetCaret);
	LoadSpritePalette(&sPalFx);
	LoadSpritePalette(&sPalCursor);
	cursorId = CreateSprite(&sTemplateCursor, MgKeyCenterX(0), MgKeyCenterY(1), 0);
	caretId = CreateSprite(&sTemplateCaret, SLOT_COL0 * 8 + SLOT_VISUAL_X + 8, CARET_Y, 0);

	taskId = CreateTask(Task_MysteryGift, 0);
	gTasks[taskId].tState = ST_FADE_IN;
	gTasks[taskId].tTimer = 0;
	gTasks[taskId].tCursor = 10;	//Starts on the letter A
	gTasks[taskId].tLen = 0;
	gTasks[taskId].tBufLo = (u32) tm0 & 0xFFFF;
	gTasks[taskId].tBufHi = ((u32) tm0 >> 16) & 0xFFFF;
	gTasks[taskId].tBuf1Lo = (u32) tm1 & 0xFFFF;
	gTasks[taskId].tBuf1Hi = ((u32) tm1 >> 16) & 0xFFFF;
	gTasks[taskId].tSprCursor = cursorId;
	gTasks[taskId].tSprCaret = caretId;
	gTasks[taskId].tSprMon = 0xFF;
	gTasks[taskId].tPage = 0;
	gStringVar2[0] = 0;
	MG_MSG_TIMER = 0;
	MG_SENT_RESULT = 0;
	gSpecialVar_LastResult = 0;

	MgDrawEntryScreen();

	//Only BG0 (text) and BG1 (code screen). BG2 (stage) is only turned on during the animation (MgEnterStage): color 0 of the
	//backgrounds is transparent, so with BG2 on it would show through under the code screen.
	ShowBg(0);
	ShowBg(1);
	SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON | DISPCNT_BG0_ON | DISPCNT_BG1_ON);

#if MYSTERY_GIFT_MENU_MUSIC != 0
	PlayBGM(MYSTERY_GIFT_MENU_MUSIC);		//Menu music (the map music comes back on exit)
#endif
	SetVBlankCallback(VBlankCB_MysteryGift);
	BeginNormalPaletteFade(PALETTES_ALL, 0, 16, 0, RGB_BLACK);
	SetMainCallback2(CB2_MysteryGiftMain);
}

static void Task_MysteryGiftStartFade(u8 taskId)
{
	if (!gPaletteFade->active)
	{
		DestroyTask(taskId);
		SetMainCallback2(CB2_MysteryGift);
	}
}

//Entry point: callasm MysteryGift_Start  (or special). Then use "waitstate".
void MysteryGift_Start(void)
{
	typedef char MysteryGiftTableTooBig[(NELEMS(sMysteryGifts) <= MYSTERY_GIFT_MAX_CODES) ? 1 : -1] __attribute__((unused));

	ScriptContext2_Enable();
	gSpecialVar_LastResult = 0;
#if MYSTERY_GIFT_MENU_MUSIC != 0
	FadeOutBGM(4);
#endif
	CreateTask(Task_MysteryGiftStartFade, 0x50);
	BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 16, RGB_BLACK);
}
