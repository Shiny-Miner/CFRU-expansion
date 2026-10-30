/********\

CUSTOM FILE!
THIS IS A CUSTOM FILE THAT ISN'T NORMALLY IN THE CFRU.

Gen-5 style save screen for the FireRed Start Menu.

Layout (240x160, one 16 colour palette, graphics/Save_Screen/saveScreenBg.png):
 - top panel:  date / time, current location, the whole party (6 icons), badges, Pokedex, play time
 - page dots:  a highlight sweeps through the dots while the screen is open
 - bottom box: "Would you like to save the game?" + YES/NO, then "Saving..." (animated)

How it works:
 - sStartMenuActionTable[STARTMENU_SAVE] points to StartMenuSaveScreenCallback (see start_menu.c)
 - the callback leaves the overworld exactly like the vanilla Option/Pokedex callbacks do, then runs this screen
 - saving uses the very same engine calls as the vanilla save dialog
   (save_serialize_map -> IncrementGameStat -> TrySavingData)
 - the screen returns to the Start Menu through CB2_ReturnToFieldWithOpenMenuHook

RAM: no global RAM is used. Everything lives in one heap block (Calloc) that is owned by the screen's task
(the pointer is kept in task data[14] / data[15]) and is freed on exit.

\********/

#include "../include/global.h"
#include "../include/bg.h"
#include "../include/window.h"
#include "../include/sprite.h"
#include "../include/palette.h"
#include "../include/task.h"
#include "../include/main.h"
#include "../include/menu.h"
#include "../include/text.h"
#include "../include/gpu_regs.h"
#include "../include/pokemon.h"
#include "../include/pokemon_icon.h"
#include "../include/overworld.h"
#include "../include/malloc.h"
#include "../include/sound.h"
#include "../include/string_util.h"
#include "../include/field_weather.h"
#include "../include/event_data.h"
#include "../include/pokedex.h"
#include "../include/save.h"
#include "../include/rtc.h"
#include "../include/scanline_effect.h"
#include "../include/constants/flags.h"
#include "../include/constants/game_stat.h"
#include "../include/constants/songs.h"
#include "../include/constants/species.h"
#include "../include/new/Vanilla_functions.h"
#include "../include/new/dns.h" //TransferPlttBuffer
#include "../include/new/ram_locs.h"
#include "../include/new/save_screen.h"

#ifdef BW_SAVING_SCREEN

/* ---------------------------------------------------------------------------------------------------------------
 * Graphics / strings (graphics/Save_Screen/saveScreenBg.png -> grit, strings/save_screen.string)
 * ------------------------------------------------------------------------------------------------------------- */
extern const u8 saveScreenBgTiles[];
extern const u8 saveScreenBgMap[];
extern const u8 saveScreenBgPal[];

extern const u8 gText_SaveScreen_Confirm[];
extern const u8 gText_SaveScreen_Saving1[];
extern const u8 gText_SaveScreen_Saving2[];
extern const u8 gText_SaveScreen_Saving3[];
extern const u8 gText_SaveScreen_Saved[];
extern const u8 gText_SaveScreen_Error[];
extern const u8 gText_SaveScreen_Badges[];
extern const u8 gText_SaveScreen_Pokedex[];
extern const u8 gText_SaveScreen_Time[];
extern const u8 gText_SaveScreen_Yes[];
extern const u8 gText_SaveScreen_No[];
extern const u8 gText_SaveScreen_NoDate[];
extern const u8 gText_SaveScreen_NoClock[];

/* ---------------------------------------------------------------------------------------------------------------
 * Vanilla (ROM) functions that have no declaration in the headers
 * ------------------------------------------------------------------------------------------------------------- */
void __attribute__((long_call)) save_serialize_map(void); //BPRE.ld: 0x80590D8 (vanilla InitSave() calls it first)
bool32 __attribute__((long_call)) IsNationalPokedexEnabled(void); //BPRE.ld
u8* __attribute__((long_call)) GetMapNameGeneric_(u8* dest, u16 mapsec); //BPRE.ld: GetMapName(dest, mapsec, 0)

#define RemoveExtraStartMenuWindows ((void (*)(void))(0x806EF18 | 1)) //Same call every vanilla start menu callback makes
#define GetNationalDexCount ((u16 (*)(u8))(0x8088E8C | 1))            //Used by vanilla SaveStatToString (SAVE_STAT_POKEDEX)
#define GetKantoDexCount ((u16 (*)(u8))(0x8088EDC | 1))               //Used by vanilla SaveStatToString (SAVE_STAT_POKEDEX)
#define gDifferentSaveFileRAM (*(vu8*) 0x2031DB0)                    //gDifferentSaveFile (vanilla save dialog reads/clears it)

void CB2_ReturnToFieldWithOpenMenuHook(void); //overworld.c - reopens the Start Menu (also handles follower palette)

/* ---------------------------------------------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------------------------------------------- */
#define SE_SAVE_SCREEN_SAVED 0x30 //Vanilla plays PlaySE(48) after "saved the game"
#define SE_SAVE_SCREEN_ERROR SE_FALSE //Vanilla plays PlaySE(22) on a save error

#define CH_SPACE 0x00
#define CH_HYPHEN 0xAE
#define CH_SLASH 0xBA
#define CH_COLON 0xF0

#define FONT_ID 2
#define TILEMAP_STRIDE 32

//Party icons
#define ICON_CENTER_X 120
#define ICON_SPACING 38
#define ICON_Y 56
#define ICON_NONE 0xFF

//Page dots (tile row/column of the first dot in the background image, dot highlighted in the png)
#define DOT_ROW 13
#define DOT_FIRST_COL 8
#define DOT_COUNT 14
#define DOT_START_POS 6
#define DOT_STEP_FRAMES 5

//Timings (frames)
#define SAVING_MIN_FRAMES 75
#define SAVING_DOT_FRAMES 14
#define RESULT_MAX_FRAMES 180
#define FLOPPY_BLINK_FRAMES 10

//Palette index used by the floppy disk label in the background image
#define FLOPPY_PAL_INDEX 9

enum
{
	WIN_DATE,
	WIN_PLACE,
	WIN_STATS,
	WIN_TIME,
	WIN_YESNO,
	WIN_BOX,
	WIN_COUNT
};

enum
{
	STATE_WAIT_FADE_IN,
	STATE_CHOOSE,
	STATE_SAVING,
	STATE_RESULT,
	STATE_WAIT_FADE_OUT,
};

enum
{
	INIT_RESET,
	INIT_VIDEO,
	INIT_GFX,
	INIT_TILEMAP,
	INIT_PRINT,
	INIT_ICONS,
	INIT_COPY,
	INIT_FADE,
	INIT_VBLANK,
	INIT_DONE,
};

struct SaveScreen
{
	u16 tilemapBuffer[TILEMAP_STRIDE * 32]; //BG1 tilemap (the BG0 windows allocate their own buffer)
	u16 dotIdleEntry;
	u16 dotActiveEntry;
	u16 timer;
	u8 iconIds[PARTY_SIZE];
	u8 dotPos;
	s8 dotDir;
	u8 dotTimer;
	u8 cursor;     //0 = YES, 1 = NO
	u8 savingDots; //0..2
	u8 floppyBright;
	u8 saveOk;
};

/* ---------------------------------------------------------------------------------------------------------------
 * Templates / tables (all const -> ROM)
 * ------------------------------------------------------------------------------------------------------------- */
static const struct BgTemplate sBgTemplates[] =
{
	{
		.bg = 0,
		.charBaseIndex = 0,
		.mapBaseIndex = 22,
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 0,
		.baseTile = 1,
	},
	{
		.bg = 1,
		.charBaseIndex = 2,
		.mapBaseIndex = 19, //Unique background tiles must stay below tile 192 (screen block 19 starts there)
		.screenSize = 0,
		.paletteMode = 0,
		.priority = 2,
		.baseTile = 0,
	},
};

//Window baseBlocks are consecutive: 52 + 52 + 52 + 40 + 24 + 104 = 324 tiles (BG0 charblock holds 511)
#define WIN_DATE_BASE 0
#define WIN_PLACE_BASE (WIN_DATE_BASE + 26 * 2)
#define WIN_STATS_BASE (WIN_PLACE_BASE + 26 * 2)
#define WIN_TIME_BASE (WIN_STATS_BASE + 26 * 2)
#define WIN_YESNO_BASE (WIN_TIME_BASE + 20 * 2)
#define WIN_BOX_BASE (WIN_YESNO_BASE + 6 * 4)

static const struct WindowTemplate sWindowTemplates[] =
{
	[WIN_DATE] =
	{
		.bg = 0, .tilemapLeft = 2, .tilemapTop = 1, .width = 26, .height = 2,
		.paletteNum = 15, .baseBlock = WIN_DATE_BASE
	},
	[WIN_PLACE] =
	{
		.bg = 0, .tilemapLeft = 2, .tilemapTop = 3, .width = 26, .height = 2,
		.paletteNum = 15, .baseBlock = WIN_PLACE_BASE
	},
	[WIN_STATS] =
	{
		.bg = 0, .tilemapLeft = 2, .tilemapTop = 9, .width = 26, .height = 2,
		.paletteNum = 15, .baseBlock = WIN_STATS_BASE
	},
	[WIN_TIME] =
	{
		.bg = 0, .tilemapLeft = 2, .tilemapTop = 11, .width = 20, .height = 2,
		.paletteNum = 15, .baseBlock = WIN_TIME_BASE
	},
	[WIN_YESNO] =
	{
		.bg = 0, .tilemapLeft = 23, .tilemapTop = 11, .width = 6, .height = 4,
		.paletteNum = 15, .baseBlock = WIN_YESNO_BASE
	},
	[WIN_BOX] =
	{
		.bg = 0, .tilemapLeft = 2, .tilemapTop = 15, .width = 26, .height = 4,
		.paletteNum = 15, .baseBlock = WIN_BOX_BASE
	},
	DUMMY_WIN_TEMPLATE
};

//Palette 15 (BG0 text). Index 0 is transparent so the background image shows through the windows
static const u16 sTextPalette[16] =
{
	[0] = RGB(0, 0, 0),
	[1] = RGB(31, 31, 31), //white text (on the dark panel)
	[2] = RGB(1, 1, 2),    //near black (shadow on the dark panel / text on light boxes)
	[3] = RGB(21, 21, 22), //light grey (labels / shadow on light boxes)
	[4] = RGB(14, 14, 14),
	[5] = RGB(10, 9, 23),
	[6] = RGB(31, 31, 31),
	[7] = RGB(30, 30, 30), //light box fill (YES/NO)
	[8] = RGB(5, 5, 6),    //box border
};

static const u8 sColorValue[3] = {0, 1, 2}; //bg, text, shadow
static const u8 sColorLabel[3] = {0, 3, 2};
static const u8 sColorBox[3] = {0, 2, 3};
static const u8 sColorMenu[3] = {7, 2, 3};

static const u8 sCursorArrow[] = {0xEF, 0xFF}; //Right arrow glyph

static const u16 sFloppyColors[2] = {RGB(10, 9, 23), RGB(19, 18, 29)}; //dim, bright (png colour 9 = bright)

static const s8 sIconDrop[] = {-22, -14, -8, -3, 1, 3, 2, 0, -1, 0}; //Drop-in with a little bounce
static const s8 sIconBob[8] = {0, 0, -1, -1, -2, -2, -1, -1};          //Idle float

/* ---------------------------------------------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------------------------------------------- */
static void CB2_SaveScreenSetup(void);
static void CB2_SaveScreen(void);
static void VBlankCB_SaveScreen(void);
static void Task_SaveScreenInit(u8 taskId);
static void Task_SaveScreenMain(u8 taskId);
static void SpriteCB_SaveScreenIcon(struct Sprite* sprite);

/* ---------------------------------------------------------------------------------------------------------------
 * Task data helpers (keeps the heap pointer out of global RAM)
 * ------------------------------------------------------------------------------------------------------------- */
static struct SaveScreen* GetSaveScreen(u8 taskId)
{
	return (struct SaveScreen*) ((u32) (u16) gTasks[taskId].data[14] | ((u32) (u16) gTasks[taskId].data[15] << 16));
}

static void SetSaveScreen(u8 taskId, struct SaveScreen* ss)
{
	gTasks[taskId].data[14] = (u32) ss & 0xFFFF;
	gTasks[taskId].data[15] = (u32) ss >> 16;
}

/* ---------------------------------------------------------------------------------------------------------------
 * Start Menu entry point
 * ------------------------------------------------------------------------------------------------------------- */
bool8 StartMenuSaveScreenCallback(void)
{
	if (gPaletteFade->active) //Wait for the fade out the Start Menu started (StartMenu_FadeScreenIfLeavingOverworld)
		return FALSE;

	PlayRainStoppingSoundEffect();
	RemoveExtraStartMenuWindows();
	save_serialize_map(); //Vanilla InitSave(): snapshot of the map view used by "Continue". Needs the field still loaded
	CleanupOverworldWindowsAndTilemaps();
	SetMainCallback2(CB2_SaveScreenSetup);
	return TRUE;
}

/* ---------------------------------------------------------------------------------------------------------------
 * Setup
 * ------------------------------------------------------------------------------------------------------------- */
static void CB2_SaveScreenSetup(void)
{
	u8 taskId;
	struct SaveScreen* ss;

	SetVBlankCallback(NULL);
	SetHBlankCallback(NULL);

	ss = Calloc(sizeof(struct SaveScreen));
	if (ss == NULL) //Out of memory: go back to the Start Menu instead of freezing
	{
		SetMainCallback2(CB2_ReturnToFieldWithOpenMenuHook);
		return;
	}

	ResetSpriteData();
	ResetPaletteFade();
	FreeAllSpritePalettes();
	ResetTasks();
	ScanlineEffect_Stop();

	taskId = CreateTask(Task_SaveScreenInit, 0);
	SetSaveScreen(taskId, ss);
	gTasks[taskId].data[0] = INIT_RESET;
	gTasks[taskId].data[1] = 0;

	SetMainCallback2(CB2_SaveScreen);
}

static void CB2_SaveScreen(void)
{
	RunTasks();
	AnimateSprites();
	BuildOamBuffer();
	UpdatePaletteFade();
}

static void VBlankCB_SaveScreen(void)
{
	LoadOam();
	ProcessSpriteCopyRequests();
	TransferPlttBuffer();
}

static void ResetGpu(void)
{
	void* vram = (void*) VRAM;

	DmaClearLarge16(3, vram, VRAM_SIZE, 0x1000);
	DmaClear32(3, (void*) OAM, OAM_SIZE);
	DmaClear16(3, (void*) PLTT, PLTT_SIZE);
	SetGpuReg(REG_OFFSET_DISPCNT, 0);
	SetGpuReg(REG_OFFSET_BG0CNT, 0);
	SetGpuReg(REG_OFFSET_BG0HOFS, 0);
	SetGpuReg(REG_OFFSET_BG0VOFS, 0);
	SetGpuReg(REG_OFFSET_BG1CNT, 0);
	SetGpuReg(REG_OFFSET_BG1HOFS, 0);
	SetGpuReg(REG_OFFSET_BG1VOFS, 0);
	SetGpuReg(REG_OFFSET_BG2CNT, 0);
	SetGpuReg(REG_OFFSET_BG2HOFS, 0);
	SetGpuReg(REG_OFFSET_BG2VOFS, 0);
	SetGpuReg(REG_OFFSET_BG3CNT, 0);
	SetGpuReg(REG_OFFSET_BG3HOFS, 0);
	SetGpuReg(REG_OFFSET_BG3VOFS, 0);
	SetGpuReg(REG_OFFSET_WIN0H, 0);
	SetGpuReg(REG_OFFSET_WIN0V, 0);
	SetGpuReg(REG_OFFSET_WININ, 0);
	SetGpuReg(REG_OFFSET_WINOUT, 0);
	SetGpuReg(REG_OFFSET_BLDCNT, 0);
	SetGpuReg(REG_OFFSET_BLDALPHA, 0);
	SetGpuReg(REG_OFFSET_BLDY, 0);
}

static void ResetBgPos(void)
{
	u8 bg;

	for (bg = 0; bg < 4; ++bg)
	{
		ChangeBgX(bg, 0, 0);
		ChangeBgY(bg, 0, 0);
	}
}

static void InitVideo(struct SaveScreen* ss)
{
	ResetGpu();
	ResetBgsAndClearDma3BusyFlags(0);
	InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
	ResetBgPos();
	InitWindows(sWindowTemplates);
	DeactivateAllTextPrinters();
	SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_1D_MAP | DISPCNT_OBJ_ON);
	SetBgTilemapBuffer(1, ss->tilemapBuffer);
	ShowBg(0);
	ShowBg(1);
	FillBgTilemapBufferRect_Palette0(0, 0, 0, 0, 30, 20);
	FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 30, 20);
}

//Returns TRUE when finished
static bool8 LoadGfx(u8 taskId)
{
	s16* data = gTasks[taskId].data;

	switch (data[1])
	{
		case 0:
			ResetTempTileDataBuffers();
			break;
		case 1:
			DecompressAndCopyTileDataToVram(1, saveScreenBgTiles, 0, 0, 0);
			break;
		case 2:
			if (FreeTempTileDataBuffersIfPossible()) //Still copying
				return FALSE;
			break;
		case 3:
			LoadCompressedPalette(saveScreenBgPal, 0, 0x20);
			LoadPalette(sTextPalette, 0xF0, sizeof(sTextPalette));
			break;
		default:
			data[1] = 0;
			return TRUE;
	}

	++data[1];
	return FALSE;
}

/* ---------------------------------------------------------------------------------------------------------------
 * Text
 * ------------------------------------------------------------------------------------------------------------- */
static void PrintText(u8 windowId, u8 x, u8 y, const u8* color, const u8* str)
{
	AddTextPrinterParameterized4(windowId, FONT_ID, x, y, 0, 0, color, TEXT_SPEED_FF, str);
}

//Prints "label value" and returns nothing; value is placed right after the label
static void PrintLabelAndValue(u8 windowId, u8 x, const u8* label, const u8* value)
{
	PrintText(windowId, x, 1, sColorLabel, label);
	PrintText(windowId, x + GetStringWidth(FONT_ID, label, 0) + 4, 1, sColorValue, value);
}

static void BuildDateAndClockStrings(u8* date, u8* clock)
{
	u8* ptr;

	if (RtcGetErrorStatus() & RTC_ERR_FLAG_MASK) //No usable RTC
	{
		StringCopy(date, gText_SaveScreen_NoDate);
		StringCopy(clock, gText_SaveScreen_NoClock);
		return;
	}

	ptr = ConvertIntToDecimalStringN(date, gClock.month, STR_CONV_MODE_LEFT_ALIGN, 2);
	*ptr++ = CH_SLASH;
	ptr = ConvertIntToDecimalStringN(ptr, gClock.day, STR_CONV_MODE_LEFT_ALIGN, 2);
	*ptr++ = CH_SLASH;
	ConvertIntToDecimalStringN(ptr, gClock.year, STR_CONV_MODE_LEFT_ALIGN, 4);

	ptr = ConvertIntToDecimalStringN(clock, gClock.hour, STR_CONV_MODE_LEADING_ZEROS, 2);
	*ptr++ = CH_COLON;
	ConvertIntToDecimalStringN(ptr, gClock.minute, STR_CONV_MODE_LEADING_ZEROS, 2);
}

static u8 CountBadges(void)
{
	u8 i, count = 0;

	for (i = 0; i < 8; ++i)
	{
		if (FlagGet(FLAG_BADGE01_GET + i))
			++count;
	}

	return count;
}

static u16 GetPokedexCaughtCount(void)
{
	if (IsNationalPokedexEnabled())
		return GetNationalDexCount(FLAG_GET_CAUGHT);

	return GetKantoDexCount(FLAG_GET_CAUGHT);
}

static void PrintPanelTexts(void)
{
	u8 i;
	u8 buffer[32];
	u8 buffer2[16];

	for (i = 0; i < WIN_COUNT; ++i)
		FillWindowPixelBuffer(i, PIXEL_FILL(0));

	//Row 1: date and time
	BuildDateAndClockStrings(buffer, buffer2);
	PrintText(WIN_DATE, 0, 1, sColorValue, buffer);
	PrintText(WIN_DATE, 96, 1, sColorValue, buffer2);

	//Row 2: location
	GetMapNameGeneric_(buffer, gMapHeader.regionMapSectionId);
	PrintText(WIN_PLACE, 0, 1, sColorValue, buffer);

	//Row 3: badges + Pokedex
	ConvertIntToDecimalStringN(buffer, CountBadges(), STR_CONV_MODE_LEFT_ALIGN, 1);
	PrintLabelAndValue(WIN_STATS, 0, gText_SaveScreen_Badges, buffer);

	if (FlagGet(FLAG_SYS_POKEDEX_GET))
	{
		ConvertIntToDecimalStringN(buffer, GetPokedexCaughtCount(), STR_CONV_MODE_LEFT_ALIGN, 3);
		PrintLabelAndValue(WIN_STATS, 112, gText_SaveScreen_Pokedex, buffer);
	}

	//Row 4: play time
	{
		u8* ptr = ConvertIntToDecimalStringN(buffer, gSaveBlock2->playTimeHours, STR_CONV_MODE_LEFT_ALIGN, 3);
		*ptr++ = CH_COLON;
		ConvertIntToDecimalStringN(ptr, gSaveBlock2->playTimeMinutes, STR_CONV_MODE_LEADING_ZEROS, 2);
		PrintLabelAndValue(WIN_TIME, 0, gText_SaveScreen_Time, buffer);
	}

	for (i = 0; i < WIN_COUNT; ++i)
	{
		PutWindowTilemap(i);
		CopyWindowToVram(i, 3);
	}
}

static void PrintBoxText(const u8* str)
{
	FillWindowPixelBuffer(WIN_BOX, PIXEL_FILL(0));
	StringExpandPlaceholders(gStringVar4, str);
	PrintText(WIN_BOX, 4, 1, sColorBox, gStringVar4);
	CopyWindowToVram(WIN_BOX, 2);
}

static void DrawYesNo(struct SaveScreen* ss)
{
	FillWindowPixelBuffer(WIN_YESNO, PIXEL_FILL(8)); //Border
	FillWindowPixelRect(WIN_YESNO, 7, 1, 1, 46, 30);
	PrintText(WIN_YESNO, 14, 1, sColorMenu, gText_SaveScreen_Yes);
	PrintText(WIN_YESNO, 14, 15, sColorMenu, gText_SaveScreen_No);
	PrintText(WIN_YESNO, 4, ss->cursor == 0 ? 1 : 15, sColorMenu, sCursorArrow);
	CopyWindowToVram(WIN_YESNO, 2);
}

static void HideYesNo(void)
{
	FillWindowPixelBuffer(WIN_YESNO, PIXEL_FILL(0)); //Transparent again
	CopyWindowToVram(WIN_YESNO, 2);
}

/* ---------------------------------------------------------------------------------------------------------------
 * Party icons
 * ------------------------------------------------------------------------------------------------------------- */
static void SpriteCB_SaveScreenIcon(struct Sprite* sprite)
{
	//data[0] = intro frame, data[1] = intro delay, data[2] = float phase, data[3] = float counter, data[4] = intro done
	SpriteCB_PokeIcon(sprite); //Vanilla icon frame animation

	if (sprite->data[4] == 0)
	{
		s16 t;

		if (sprite->data[0] < sprite->data[1]) //Waiting for its turn
		{
			++sprite->data[0];
			sprite->invisible = TRUE;
			return;
		}

		sprite->invisible = FALSE;
		t = sprite->data[0] - sprite->data[1];

		if (t < (s16) ARRAY_COUNT(sIconDrop))
		{
			sprite->pos2.y = sIconDrop[t];
			++sprite->data[0];
		}
		else
			sprite->data[4] = 1;
	}
	else
	{
		sprite->data[3] = (sprite->data[3] + 1) & 31;
		sprite->pos2.y = sIconBob[((sprite->data[3] >> 2) + sprite->data[2]) & 7];
	}
}

static void CreatePartyIcons(struct SaveScreen* ss)
{
	u8 i, count = 0, shown = 0;
	s16 startX;

	for (i = 0; i < PARTY_SIZE; ++i)
	{
		ss->iconIds[i] = ICON_NONE;

		if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2, NULL) != SPECIES_NONE)
			++count;
	}

	if (count == 0)
		return;

	LoadMonIconPalettes();
	startX = ICON_CENTER_X - (count * ICON_SPACING) / 2 + ICON_SPACING / 2; //Party is centred in the strip

	for (i = 0; i < PARTY_SIZE; ++i)
	{
		u8 spriteId;
		u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2, NULL); //Eggs show the egg icon

		if (species == SPECIES_NONE)
			continue;

		spriteId = CreateMonIcon(species, SpriteCB_SaveScreenIcon, startX + shown * ICON_SPACING, ICON_Y, 1,
		                         GetMonData(&gPlayerParty[i], MON_DATA_PERSONALITY, NULL), FALSE);

		if (spriteId >= MAX_SPRITES)
			continue;

		gSprites[spriteId].oam.priority = 1; //Above the background image (2), below the text layer (0)
		gSprites[spriteId].invisible = TRUE;
		gSprites[spriteId].data[0] = 0;
		gSprites[spriteId].data[1] = 6 * shown; //Stagger
		gSprites[spriteId].data[2] = shown;
		gSprites[spriteId].data[3] = 0;
		gSprites[spriteId].data[4] = 0;
		ss->iconIds[i] = spriteId;
		++shown;
	}
}

/* ---------------------------------------------------------------------------------------------------------------
 * Small animations
 * ------------------------------------------------------------------------------------------------------------- */
//A highlight sweeps back and forth through the page dots (the tiles are swapped in the BG1 tilemap)
static void AnimateDots(struct SaveScreen* ss)
{
	u16* dots = &ss->tilemapBuffer[DOT_ROW * TILEMAP_STRIDE + DOT_FIRST_COL];

	if (++ss->dotTimer < DOT_STEP_FRAMES)
		return;

	ss->dotTimer = 0;
	dots[ss->dotPos] = ss->dotIdleEntry;

	if (ss->dotPos == 0)
		ss->dotDir = 1;
	else if (ss->dotPos >= DOT_COUNT - 1)
		ss->dotDir = -1;

	ss->dotPos += ss->dotDir;
	dots[ss->dotPos] = ss->dotActiveEntry;
	CopyBgTilemapBufferToVram(1);
}

static void SetFloppyBright(struct SaveScreen* ss, bool8 bright)
{
	ss->floppyBright = bright;
	LoadPalette(&sFloppyColors[bright], FLOPPY_PAL_INDEX, sizeof(u16));
}

static void StartFadeOut(u8 taskId)
{
	BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
	gTasks[taskId].data[0] = STATE_WAIT_FADE_OUT;
}

/* ---------------------------------------------------------------------------------------------------------------
 * Init task
 * ------------------------------------------------------------------------------------------------------------- */
static void Task_SaveScreenInit(u8 taskId)
{
	struct SaveScreen* ss = GetSaveScreen(taskId);
	s16* data = gTasks[taskId].data;

	switch (data[0])
	{
		case INIT_RESET:
			SetVBlankCallback(NULL);
			break;
		case INIT_VIDEO:
			InitVideo(ss);
			break;
		case INIT_GFX:
			if (!LoadGfx(taskId))
				return; //Stay in this step
			break;
		case INIT_TILEMAP:
			CopyToBgTilemapBuffer(1, saveScreenBgMap, 0, 0);
			ss->dotIdleEntry = ss->tilemapBuffer[DOT_ROW * TILEMAP_STRIDE + DOT_FIRST_COL];
			ss->dotActiveEntry = ss->tilemapBuffer[DOT_ROW * TILEMAP_STRIDE + DOT_FIRST_COL + DOT_START_POS];
			ss->dotPos = DOT_START_POS;
			ss->dotDir = 1;
			ss->floppyBright = TRUE;
			SetGpuReg(REG_OFFSET_BG1HOFS, 0);
			break;
		case INIT_PRINT:
			PrintPanelTexts();
			break;
		case INIT_ICONS:
			CreatePartyIcons(ss);
			break;
		case INIT_COPY:
			CopyBgTilemapBufferToVram(0);
			CopyBgTilemapBufferToVram(1);
			break;
		case INIT_FADE:
			BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
			break;
		case INIT_VBLANK:
			SetVBlankCallback(VBlankCB_SaveScreen);
			break;
		default:
			if (gPaletteFade->active)
				return;
			data[0] = STATE_WAIT_FADE_IN;
			data[1] = 0;
			gTasks[taskId].func = Task_SaveScreenMain;
			return;
	}

	++data[0];
}

/* ---------------------------------------------------------------------------------------------------------------
 * Saving (same engine calls as the vanilla SaveDialogCB_DoSave)
 * ------------------------------------------------------------------------------------------------------------- */
static u8 DoSave(void)
{
	u8 status;

	IncrementGameStat(GAME_STAT_SAVED_GAME);

	if (gDifferentSaveFileRAM == TRUE)
	{
		status = TrySavingData(SAVE_OVERWRITE_DIFFERENT_FILE);
		gDifferentSaveFileRAM = FALSE;
	}
	else
		status = TrySavingData(SAVE_NORMAL);

	return status;
}

static void ExitSaveScreen(u8 taskId)
{
	struct SaveScreen* ss = GetSaveScreen(taskId);

	SetVBlankCallback(NULL);
	DestroyTask(taskId);
	ResetSpriteData();
	FreeAllSpritePalettes();
	FreeAllWindowBuffers();
	Free(ss);
	SetMainCallback2(CB2_ReturnToFieldWithOpenMenuHook);
}

static const u8* const sSavingTexts[] =
{
	gText_SaveScreen_Saving1,
	gText_SaveScreen_Saving2,
	gText_SaveScreen_Saving3,
};

/* ---------------------------------------------------------------------------------------------------------------
 * Main task
 * ------------------------------------------------------------------------------------------------------------- */
static void Task_SaveScreenMain(u8 taskId)
{
	struct SaveScreen* ss = GetSaveScreen(taskId);
	s16* data = gTasks[taskId].data;

	if (data[0] != STATE_WAIT_FADE_OUT)
		AnimateDots(ss);

	switch (data[0])
	{
		case STATE_WAIT_FADE_IN:
			if (gPaletteFade->active)
				break;

			PrintBoxText(gText_SaveScreen_Confirm);
			ss->cursor = 0;
			DrawYesNo(ss);
			data[0] = STATE_CHOOSE;
			break;

		case STATE_CHOOSE:
			if (JOY_NEW(DPAD_UP) && ss->cursor != 0)
			{
				PlaySE(SE_SELECT);
				ss->cursor = 0;
				DrawYesNo(ss);
			}
			else if (JOY_NEW(DPAD_DOWN) && ss->cursor != 1)
			{
				PlaySE(SE_SELECT);
				ss->cursor = 1;
				DrawYesNo(ss);
			}
			else if (JOY_NEW(A_BUTTON))
			{
				PlaySE(SE_SELECT);

				if (ss->cursor == 0)
				{
					HideYesNo();
					ss->timer = 0;
					ss->savingDots = 0;
					PrintBoxText(sSavingTexts[0]);
					data[0] = STATE_SAVING;
				}
				else
					StartFadeOut(taskId);
			}
			else if (JOY_NEW(B_BUTTON))
			{
				PlaySE(SE_SELECT);
				StartFadeOut(taskId);
			}
			break;

		case STATE_SAVING:
			++ss->timer;

			if (ss->timer % SAVING_DOT_FRAMES == 0) //"Saving." -> "Saving.." -> "Saving..."
			{
				ss->savingDots = (ss->savingDots + 1) % ARRAY_COUNT(sSavingTexts);
				PrintBoxText(sSavingTexts[ss->savingDots]);
			}

			if (ss->timer % FLOPPY_BLINK_FRAMES == 0)
				SetFloppyBright(ss, !ss->floppyBright);

			if (ss->timer < SAVING_MIN_FRAMES)
				break;

			ss->saveOk = (DoSave() == SAVE_STATUS_OK); //Blocks for a moment, like in the vanilla dialog
			SetFloppyBright(ss, TRUE);
			ss->timer = 0;

			if (ss->saveOk)
			{
				PrintBoxText(gText_SaveScreen_Saved);
				PlaySE(SE_SAVE_SCREEN_SAVED);
			}
			else
			{
				PrintBoxText(gText_SaveScreen_Error);
				PlaySE(SE_SAVE_SCREEN_ERROR);
			}

			data[0] = STATE_RESULT;
			break;

		case STATE_RESULT:
			if (++ss->timer >= RESULT_MAX_FRAMES || (ss->timer > 20 && JOY_NEW(A_BUTTON | B_BUTTON)))
				StartFadeOut(taskId);
			break;

		case STATE_WAIT_FADE_OUT:
			if (!gPaletteFade->active)
				ExitSaveScreen(taskId);
			break;
	}
}
#endif