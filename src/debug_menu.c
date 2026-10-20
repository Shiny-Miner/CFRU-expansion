#include "defines.h"
#include "../include/coins.h"
#include "../include/money.h"
#include "../include/event_data.h"
#include "../include/item.h"
#include "../include/constants/items.h"

#include "../include/new/build_pokemon.h"
#include "../include/new/item.h"
#include "../include/new/util.h"
#include "../include/script.h"
#include "../include/random.h"
#include "../include/menu.h"
#include "../include/item_icon.h"
#include "../include/pokemon_icon.h"
#include "../include/sound.h"
#include "../include/sprite.h"
#include "../include/string_util.h"
#include "../include/task.h"
#include "../include/text.h"
#include "../include/window.h"
#include "../include/constants/songs.h"

extern const u8 gText_DebugSelector_PokemonNo[];
extern const u8 gText_DebugSelector_Step1[];
extern const u8 gText_DebugSelector_Step10[];
extern const u8 gText_DebugSelector_Step100[];
extern const u8 gText_DebugSelector_Step1000[];
extern const u8 gText_DebugSelector_Controls[];
extern const u8 gText_DebugSelector_ItemNo[];
extern const u8 gText_DebugSelector_FlagNo[];
extern const u8 gText_DebugSelector_Hex[];
extern const u8 gText_DebugSelector_True[];
extern const u8 gText_DebugSelector_False[];
extern const u8 gText_DebugSelector_VarNo[];
extern const u8 gText_DebugSelector_Value[];
extern const u8 gText_DebugSelector_Step10000[];

static const struct TextColor sDebugSelectorTextColor =
{
	.bgColor = TEXT_COLOR_TRANSPARENT,
	.fgColor = TEXT_COLOR_DARK_GREY,
	.shadowColor = TEXT_COLOR_LIGHT_GREY,
};

enum
{
	DEBUG_SELECTOR_SPECIES,
	DEBUG_SELECTOR_DIGIT,
	DEBUG_SELECTOR_WINDOW,
	DEBUG_SELECTOR_ICON,
	DEBUG_SELECTOR_ICON_SPECIES, //Species whose palette is currently loaded for the icon
};

static void Task_DebugMenuPokemonSelector(u8 taskId);
static void DebugMenu_DrawPokemonSelector(u8 taskId, bool8 updateIcon);
static void DebugMenu_ClosePokemonSelector(u8 taskId, bool8 confirmed);
static u8 DebugMenu_CreateSelectorIcon(u16 species);

static const u16 sDebugSelectorSteps[] = {1, 10, 100, 1000};
static const u8 *const sDebugSelectorStepTexts[] =
{
	gText_DebugSelector_Step1,
	gText_DebugSelector_Step10,
	gText_DebugSelector_Step100,
	gText_DebugSelector_Step1000,
};

//Loads the icon palette for the species, creates the icon, and makes sure the sprite
//actually points at the palette slot that holds the right colours.
static u8 DebugMenu_CreateSelectorIcon(u16 species)
{
	u8 palTagIndex = GetValidMonIconPalIndex(species);
	u8 spriteId;

	LoadMonIconPalette(species);
	spriteId = CreateMonIcon(species, SpriteCB_PokeIcon, 202, 47, 0, 0, FALSE);
	if (spriteId < MAX_SPRITES)
	{
		u8 palSlot = IndexOfSpritePaletteTag(POKE_ICON_BASE_PAL_TAG + palTagIndex);
		if (palSlot != 0xFF)
			gSprites[spriteId].oam.paletteNum = palSlot;
		gSprites[spriteId].oam.priority = 0;
		gSprites[spriteId].invisible = FALSE;
	}
	return spriteId;
}

void DebugMenu_StartPokemonSelector(void)
{
	u8 taskId = CreateTask(Task_DebugMenuPokemonSelector, 0x50);
	struct Task *task = &gTasks[taskId];

	task->data[DEBUG_SELECTOR_SPECIES] = 1;
	task->data[DEBUG_SELECTOR_DIGIT] = 0;
	task->data[DEBUG_SELECTOR_WINDOW] = CreateWindowFromRect(1, 1, 26, 9);
	task->data[DEBUG_SELECTOR_ICON] = DebugMenu_CreateSelectorIcon(1);
	task->data[DEBUG_SELECTOR_ICON_SPECIES] = 1;
	DrawStdWindowFrame(task->data[DEBUG_SELECTOR_WINDOW], TRUE);
	DebugMenu_DrawPokemonSelector(taskId, FALSE);
}

static void DebugMenu_DrawPokemonSelector(u8 taskId, bool8 updateIcon)
{
	struct Task *task = &gTasks[taskId];
	u16 species = task->data[DEBUG_SELECTOR_SPECIES];
	u8 windowId = task->data[DEBUG_SELECTOR_WINDOW];

	if (updateIcon)
	{
		//Destroy the old sprite first, then release its palette, then load the new one
		if (task->data[DEBUG_SELECTOR_ICON] < MAX_SPRITES)
			DestroyMonIcon(&gSprites[task->data[DEBUG_SELECTOR_ICON]]);
		SafeFreeMonIconPalette(task->data[DEBUG_SELECTOR_ICON_SPECIES]);
		task->data[DEBUG_SELECTOR_ICON] = DebugMenu_CreateSelectorIcon(species);
		task->data[DEBUG_SELECTOR_ICON_SPECIES] = species;
	}

	FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
	WindowPrint(windowId, 2, 4, 3, &sDebugSelectorTextColor, TEXT_SPEED_FF, gText_DebugSelector_PokemonNo);
	ConvertIntToDecimalStringN(gStringVar1, species, STR_CONV_MODE_RIGHT_ALIGN, 4);
	WindowPrint(windowId, 2, 92, 3, &sDebugSelectorTextColor, TEXT_SPEED_FF, gStringVar1);
	WindowPrint(windowId, 2, 4, 20, &sDebugSelectorTextColor, TEXT_SPEED_FF, gSpeciesNames[species]);
	WindowPrint(windowId, 2, 4, 39, &sDebugSelectorTextColor, TEXT_SPEED_FF,
				sDebugSelectorStepTexts[task->data[DEBUG_SELECTOR_DIGIT]]);
	WindowPrint(windowId, 2, 4, 55, &sDebugSelectorTextColor, TEXT_SPEED_FF, gText_DebugSelector_Controls);
	CopyWindowToVram(windowId, COPYWIN_GFX);
}

static void Task_DebugMenuPokemonSelector(u8 taskId)
{
	struct Task *task = &gTasks[taskId];
	s32 species = task->data[DEBUG_SELECTOR_SPECIES];
	u16 step = sDebugSelectorSteps[task->data[DEBUG_SELECTOR_DIGIT]];
	bool8 changed = FALSE;

	if (JOY_NEW(DPAD_UP))
	{
		task->data[DEBUG_SELECTOR_DIGIT] = (task->data[DEBUG_SELECTOR_DIGIT] + 1) & 3;
		PlaySE(SE_SELECT);
		DebugMenu_DrawPokemonSelector(taskId, FALSE);
	}
	else if (JOY_NEW(DPAD_DOWN))
	{
		task->data[DEBUG_SELECTOR_DIGIT] = (task->data[DEBUG_SELECTOR_DIGIT] + 3) & 3;
		PlaySE(SE_SELECT);
		DebugMenu_DrawPokemonSelector(taskId, FALSE);
	}
	else if (JOY_NEW_AND_REPEATED(DPAD_RIGHT))
	{
		species += step;
		if (species >= NUM_SPECIES)
			species = 1;
		changed = TRUE;
	}
	else if (JOY_NEW_AND_REPEATED(DPAD_LEFT))
	{
		species -= step;
		if (species < 1)
			species = NUM_SPECIES - 1;
		changed = TRUE;
	}
	else if (JOY_NEW(A_BUTTON))
	{
		PlaySE(SE_SELECT);
		DebugMenu_ClosePokemonSelector(taskId, TRUE);
	}
	else if (JOY_NEW(B_BUTTON))
	{
		DebugMenu_ClosePokemonSelector(taskId, FALSE);
	}

	if (changed)
	{
		task->data[DEBUG_SELECTOR_SPECIES] = species;
		PlaySE(SE_SELECT);
		DebugMenu_DrawPokemonSelector(taskId, TRUE);
	}
}

static void DebugMenu_ClosePokemonSelector(u8 taskId, bool8 confirmed)
{
	struct Task *task = &gTasks[taskId];

	gSpecialVar_LastResult = confirmed ? task->data[DEBUG_SELECTOR_SPECIES] : 0;
	if (task->data[DEBUG_SELECTOR_ICON] < MAX_SPRITES)
		DestroyMonIcon(&gSprites[task->data[DEBUG_SELECTOR_ICON]]);
	SafeFreeMonIconPalette(task->data[DEBUG_SELECTOR_ICON_SPECIES]);
	ClearStdWindowAndFrame(task->data[DEBUG_SELECTOR_WINDOW], TRUE);
	RemoveWindow(task->data[DEBUG_SELECTOR_WINDOW]);
	DestroyTask(taskId);
	EnableBothScriptContexts();
}

enum
{
	DEBUG_NUMBER_ITEM,
	DEBUG_NUMBER_FLAG,
	DEBUG_NUMBER_VAR,
	DEBUG_NUMBER_VALUE,
};

#define tNumber       data[0]
#define tDigit        data[1]
#define tWindow       data[2]
#define tNumberType   data[3]
#define tMinimum      data[4]
#define tMaximum      data[5]
#define tIcon         data[6]

#define DEBUG_ITEM_ICON_TAG 0xDAF0

static void Task_DebugMenuNumberSelector(u8 taskId);

static const u8 *const sDebugNumberTitles[] =
{
	gText_DebugSelector_ItemNo,
	gText_DebugSelector_FlagNo,
	gText_DebugSelector_VarNo,
	gText_DebugSelector_Value,
};

static void DebugMenu_ConvertToHexString(u8 *dest, u16 number)
{
	u8 i;

	for (i = 0; i < 4; ++i)
	{
		u8 digit = (number >> ((3 - i) * 4)) & 0xF;
		dest[i] = digit < 10 ? CHAR_0 + digit : CHAR_A + digit - 10;
	}
	dest[4] = EOS;
}

static void DebugMenu_DrawNumberSelector(u8 taskId)
{
	struct Task *task = &gTasks[taskId];
	u16 number = (u16)task->tNumber;
	u8 windowId = task->tWindow;
	u8 digits = (task->tNumberType >= DEBUG_NUMBER_VAR) ? 5 : 4;

	FillWindowPixelBuffer(windowId, PIXEL_FILL(1));
	WindowPrint(windowId, 2, 4, 3, &sDebugSelectorTextColor, TEXT_SPEED_FF,
				sDebugNumberTitles[task->tNumberType]);
	ConvertIntToDecimalStringN(gStringVar1, number, STR_CONV_MODE_RIGHT_ALIGN, digits);
	WindowPrint(windowId, 2, 100, 3, &sDebugSelectorTextColor, TEXT_SPEED_FF, gStringVar1);
	if (task->tNumberType == DEBUG_NUMBER_ITEM)
		WindowPrint(windowId, 2, 4, 21, &sDebugSelectorTextColor, TEXT_SPEED_FF, ItemId_GetName(number));
	else if (task->tNumberType == DEBUG_NUMBER_FLAG)
	{
		WindowPrint(windowId, 2, 4, 21, &sDebugSelectorTextColor, TEXT_SPEED_FF, gText_DebugSelector_Hex);
		DebugMenu_ConvertToHexString(gStringVar2, number);
		WindowPrint(windowId, 2, 100, 21, &sDebugSelectorTextColor, TEXT_SPEED_FF, gStringVar2);
	}
	else if (task->tNumberType == DEBUG_NUMBER_VAR)
	{
		ConvertIntToDecimalStringN(gStringVar2, VarGet(number), STR_CONV_MODE_RIGHT_ALIGN, 5);
		WindowPrint(windowId, 2, 4, 21, &sDebugSelectorTextColor, TEXT_SPEED_FF, gStringVar2);
	}
	WindowPrint(windowId, 2, 4, 39, &sDebugSelectorTextColor, TEXT_SPEED_FF,
				task->tDigit == 4 ? gText_DebugSelector_Step10000 : sDebugSelectorStepTexts[task->tDigit]);
	if (task->tNumberType == DEBUG_NUMBER_FLAG)
	{
		WindowPrint(windowId, 2, 4, 55, &sDebugSelectorTextColor, TEXT_SPEED_FF,
					FlagGet(number) ? gText_DebugSelector_True : gText_DebugSelector_False);
	}
	else
	{
		WindowPrint(windowId, 2, 4, 55, &sDebugSelectorTextColor, TEXT_SPEED_FF, gText_DebugSelector_Controls);
	}
	CopyWindowToVram(windowId, COPYWIN_GFX);
}

static void DebugMenu_CreateItemSelectorIcon(struct Task *task, u16 item)
{
	if (task->tIcon < MAX_SPRITES)
		DestroySprite(&gSprites[task->tIcon]);
	FreeSpriteTilesByTag(DEBUG_ITEM_ICON_TAG);
	FreeSpritePaletteByTag(DEBUG_ITEM_ICON_TAG);
	task->tIcon = AddItemIconSprite(DEBUG_ITEM_ICON_TAG, DEBUG_ITEM_ICON_TAG, item);
	if (task->tIcon < MAX_SPRITES)
	{
		gSprites[task->tIcon].pos1.x = 202;
		gSprites[task->tIcon].pos1.y = 47;
		gSprites[task->tIcon].oam.priority = 0;
		gSprites[task->tIcon].invisible = FALSE;
	}
}

static void DebugMenu_StartNumberSelector(u8 type, u16 minimum, u16 maximum)
{
	u8 taskId = CreateTask(Task_DebugMenuNumberSelector, 0x50);
	struct Task *task = &gTasks[taskId];

	task->tNumberType = type;
	task->tMinimum = minimum;
	task->tMaximum = maximum;
	task->tNumber = minimum;
	task->tDigit = 0;
	task->tWindow = CreateWindowFromRect(1, 1, 26, 9);
	task->tIcon = MAX_SPRITES;
	Var8001 = FALSE;
	DrawStdWindowFrame(task->tWindow, TRUE);
	if (type == DEBUG_NUMBER_ITEM)
		DebugMenu_CreateItemSelectorIcon(task, minimum);
	DebugMenu_DrawNumberSelector(taskId);
}

void DebugMenu_StartItemSelector(void)
{
	DebugMenu_StartNumberSelector(DEBUG_NUMBER_ITEM, 1, ITEMS_COUNT - 1);
}

void DebugMenu_StartFlagSelector(void)
{
	DebugMenu_StartNumberSelector(DEBUG_NUMBER_FLAG, 0, 0x1FFF);
}

void DebugMenu_StartVarSelector(void)
{
	DebugMenu_StartNumberSelector(DEBUG_NUMBER_VAR, 0x4000, 0x51FF);
}

void DebugMenu_StartValueSelector(void)
{
	DebugMenu_StartNumberSelector(DEBUG_NUMBER_VALUE, 0, 0xFFFF);
}

static void DebugMenu_CloseNumberSelector(u8 taskId, bool8 confirmed)
{
	struct Task *task = &gTasks[taskId];
	gSpecialVar_LastResult = (u16)task->tNumber;
	Var8001 = confirmed;
	if (task->tNumberType == DEBUG_NUMBER_ITEM)
	{
		if (task->tIcon < MAX_SPRITES)
			DestroySprite(&gSprites[task->tIcon]);
		FreeSpriteTilesByTag(DEBUG_ITEM_ICON_TAG);
		FreeSpritePaletteByTag(DEBUG_ITEM_ICON_TAG);
	}
	ClearStdWindowAndFrame(task->tWindow, TRUE);
	RemoveWindow(task->tWindow);
	DestroyTask(taskId);
	EnableBothScriptContexts();
}

static void Task_DebugMenuNumberSelector(u8 taskId)
{
	struct Task *task = &gTasks[taskId];
	u32 number = (u16)task->tNumber;
	u32 minimum = (u16)task->tMinimum;
	u32 maximum = (u16)task->tMaximum;
	u8 digitCount = (task->tNumberType >= DEBUG_NUMBER_VAR) ? 5 : 4;
	u32 step = task->tDigit == 4 ? 10000 : sDebugSelectorSteps[task->tDigit];
	bool8 changed = FALSE;

	if (JOY_NEW(DPAD_UP))
	{
		task->tDigit = (task->tDigit + 1) % digitCount;
		PlaySE(SE_SELECT);
		DebugMenu_DrawNumberSelector(taskId);
	}
	else if (JOY_NEW(DPAD_DOWN))
	{
		task->tDigit = (task->tDigit + digitCount - 1) % digitCount;
		PlaySE(SE_SELECT);
		DebugMenu_DrawNumberSelector(taskId);
	}
	else if (JOY_NEW_AND_REPEATED(DPAD_RIGHT))
	{
		number = number + step > maximum ? minimum : number + step;
		changed = TRUE;
	}
	else if (JOY_NEW_AND_REPEATED(DPAD_LEFT))
	{
		number = number < minimum + step ? maximum : number - step;
		changed = TRUE;
	}
	else if (JOY_NEW(A_BUTTON))
	{
		PlaySE(SE_SELECT);
		DebugMenu_CloseNumberSelector(taskId, TRUE);
	}
	else if (JOY_NEW(B_BUTTON))
	{
		DebugMenu_CloseNumberSelector(taskId, FALSE);
	}

	if (changed)
	{
		task->tNumber = number;
		if (task->tNumberType == DEBUG_NUMBER_ITEM)
			DebugMenu_CreateItemSelectorIcon(task, number);
		PlaySE(SE_SELECT);
		DebugMenu_DrawNumberSelector(taskId);
	}
}

#undef tNumber
#undef tDigit
#undef tWindow
#undef tNumberType
#undef tMinimum
#undef tMaximum
#undef tIcon
#undef DEBUG_ITEM_ICON_TAG

void DebugMenu_ProcessSetFlag(void)
{
	u32 i;

	switch (gSpecialVar_LastResult) {
		case 0: //Badges
			for (i = FLAG_BADGE01_GET; i <= FLAG_BADGE08_GET; ++i)
				FlagSet(i);
			break;
		case 1: //Game Clear
			FlagSet(FLAG_SYS_GAME_CLEAR);
			break;
		case 2: //Pokedexes
			FlagSet(FLAG_SYS_POKEDEX_GET);
			FlagSet(FLAG_SYS_DEXNAV);
			break;
		case 3: //Fly Spots
			for (i = 0x890; i <= 0x8CA; ++i)
				FlagSet(i);
			FlagSet(FLAG_SYS_SEVII_MAP_123);
			FlagSet(FLAG_SYS_SEVII_MAP_4567);
			break;
		case 4: //Custom Flag and Var - Modify this number
			//FlagSet(0x152D);
			VarSet(0x500C, 0x28);
			//FlagClear(0x15AC);
			break;
	}
}

void DebugMenu_ProcessGiveItem(void)
{
	u32 i;

	switch (gSpecialVar_LastResult) {
		case 0: //Useful Key Items
			AddBagItem(ITEM_BICYCLE, 1);
			AddBagItem(ITEM_TOWN_MAP, 1);
			AddBagItem(ITEM_OLD_ROD, 1);
			AddBagItem(ITEM_GOOD_ROD, 1);
			AddBagItem(ITEM_SUPER_ROD, 1);
			AddBagItem(ITEM_VS_SEEKER, 1);
			AddBagItem(ITEM_ITEMFINDER, 1);
			AddBagItem(ITEM_MEGA_RING, 1);
			break;
		case 1: //General Useful Items
			AddBagItem(ITEM_MAX_REPEL, 100);
			AddBagItem(ITEM_ESCAPE_ROPE, 100);
			AddBagItem(ITEM_HEART_SCALE, 100);
			#if (defined ITEM_HM01_CUT && defined ITEM_HM08_ROCK_CLIMB)
			for (i = ITEM_HM01_CUT; i <= ITEM_HM08_ROCK_CLIMB; ++i)
				AddBagItem(i, 1);
			#endif

			AddBagItem(ITEM_RED_SHARD, 100);
			AddBagItem(ITEM_BLUE_SHARD, 100);
			AddBagItem(ITEM_YELLOW_SHARD, 100);
			AddBagItem(ITEM_GREEN_SHARD, 100);
			break;
		case 2: //Poke Balls
			for (i = ITEM_MASTER_BALL; i <= ITEM_PREMIER_BALL; ++i)
				AddBagItem(i, 100);

			AddBagItem(ITEM_PARK_BALL, 100);
			AddBagItem(ITEM_CHERISH_BALL, 100);
			AddBagItem(ITEM_DUSK_BALL, 100);
			AddBagItem(ITEM_HEAL_BALL, 100);
			AddBagItem(ITEM_QUICK_BALL, 100);
			AddBagItem(ITEM_FAST_BALL, 100);
			AddBagItem(ITEM_LEVEL_BALL, 100);
			AddBagItem(ITEM_LURE_BALL, 100);
			AddBagItem(ITEM_HEAVY_BALL, 100);
			AddBagItem(ITEM_LOVE_BALL, 100);
			AddBagItem(ITEM_FRIEND_BALL, 100);
			AddBagItem(ITEM_MOON_BALL, 100);
			AddBagItem(ITEM_SPORT_BALL, 100);
			AddBagItem(ITEM_BEAST_BALL, 100);
			AddBagItem(ITEM_DREAM_BALL, 100);
			break;
		case 3: //Berries
			for (i = ITEM_CHERI_BERRY; i <= ITEM_STARF_BERRY; ++i)
				AddBagItem(i, 100);
			
			AddBagItem(ITEM_ENIGMA_BERRY, 100);
			AddBagItem(ITEM_OCCA_BERRY, 100);
			AddBagItem(ITEM_PASSHO_BERRY, 100);
			AddBagItem(ITEM_WACAN_BERRY, 100);
			AddBagItem(ITEM_RINDO_BERRY, 100);
			AddBagItem(ITEM_YACHE_BERRY, 100);
			AddBagItem(ITEM_CHOPLE_BERRY, 100);
			AddBagItem(ITEM_KEBIA_BERRY, 100);
			AddBagItem(ITEM_SHUCA_BERRY, 100);
			AddBagItem(ITEM_COBA_BERRY, 100);
			AddBagItem(ITEM_PAYAPA_BERRY, 100);
			AddBagItem(ITEM_TANGA_BERRY, 100);
			AddBagItem(ITEM_CHARTI_BERRY, 100);
			AddBagItem(ITEM_KASIB_BERRY, 100);
			AddBagItem(ITEM_HABAN_BERRY, 100);
			AddBagItem(ITEM_COLBUR_BERRY, 100);
			AddBagItem(ITEM_BABIRI_BERRY, 100);
			AddBagItem(ITEM_CHILAN_BERRY, 100);
			AddBagItem(ITEM_MICLE_BERRY, 100);
			AddBagItem(ITEM_CUSTAP_BERRY, 100);
			AddBagItem(ITEM_JABOCA_BERRY, 100);
			AddBagItem(ITEM_ROWAP_BERRY, 100);
			AddBagItem(ITEM_ROSELI_BERRY, 100);
			AddBagItem(ITEM_KEE_BERRY, 100);
			AddBagItem(ITEM_MARANGA_BERRY, 100);
			break;
		case 4: //TMs & HMs
			#ifdef UNBOUND //Remove if you want this, enums can't be #ifdefed
			for (i = ITEM_TM01_FOCUS_PUNCH; i <= ITEM_TM50_OVERHEAT; ++i)
				AddBagItem(i, 1);

			for (i = ITEM_TM51_ROOST; i <= ITEM_TM58_ENDURE; ++i)
				AddBagItem(i, 1);

			for (i = ITEM_TM59_DRAGON_PULSE; i <= ITEM_TM120_NATURE_POWER; ++i)
				AddBagItem(i, 1);

			for (i = ITEM_HM01_CUT; i <= ITEM_HM08_ROCK_CLIMB; ++i)
				AddBagItem(i, 1);
			#endif
			break;
		case 5: //All items
			for (i = 0; i < ITEMS_COUNT; ++i)
			{
				const u8* name = ItemId_GetName(i);
				if (name[0] != 0xAC && name[0] != 0xFF) //'?', ' '
					AddBagItem(i, 1);
			}
			break;
	}
}

void DebugMenu_SetTeamToLevel100(void)
{
	for (u32 i = 0; i < PARTY_SIZE; ++i)
	{
		u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2, NULL);
		if (species != SPECIES_NONE && species != SPECIES_EGG)
		{
			u32 exp = gExperienceTables[gBaseStats[species].growthRate][MAX_LEVEL];
			SetMonData(&gPlayerParty[i], MON_DATA_EXP, &exp);
			CalculateMonStats(&gPlayerParty[i]);
			HealMon(&gPlayerParty[i]);
		}
	}
}

void DebugMenu_MaxMoneyAndCoins(void)
{
	AddMoney(&gSaveBlock1->money, 0xFFFFFFFF);
	SetCoins(999999999);
}

void DebugMenu_ShinyTeam(void)
{
	for (u32 i = 0; i < PARTY_SIZE; ++i)
	{
		u16 species = GetMonData(&gPlayerParty[i], MON_DATA_SPECIES2, NULL);
		if (species != SPECIES_NONE && species != SPECIES_EGG)
			ForceMonShiny(&gPlayerParty[i]);
	}
}

void DebugMenu_HealTeam(void)
{
	HealPlayerParty();
}
#include "../include/pokemon.h"
#include "../include/constants/species.h"
#include "../include/constants/pokemon.h"
#include "../include/constants/vars.h"
#include "../include/new/terastallization.h"
#define VAR_8000 0x8000
#define VAR_8001 0x8001

void DebugMenu_GivePokemonFromVar(void)
{
	u16 species = VarGet(VAR_8000);
	struct Pokemon mon;

	gSpecialVar_LastResult = FALSE;
	if (species == 0 || species >= NUM_SPECIES)
		return;

	u8 slot = CalculatePlayerPartyCount();
	if (slot >= PARTY_SIZE)
		return;

	CreateMon(&mon, species, 50, 32, TRUE, 0, OT_ID_PLAYER_ID, 0);
	if (VarGet(VAR_8001) == TRUE)
		ForceMonShiny(&mon);
	else if (IsMonShiny(&mon))
		GiveMonNatureAndAbility(&mon, GetNature(&mon),
							GetMonData(&mon, MON_DATA_PERSONALITY, NULL) & 1,
							FALSE, TRUE, TRUE);
	gPlayerParty[slot] = mon;
	gSpecialVar_LastResult = TRUE;
}
void DebugMenu_GiveItemFromVar(void)
{
    u16 item = VarGet(VAR_8000);
    if (item == 0 || item >= ITEMS_COUNT)
        return;

    AddBagItem(item, 1);
}
#define LINKER_FUNC_ADDR ((void *) (0x08088E74 + 1))
typedef void (*LinkerFunc)(u16 species, u16 param);

void DebugMenu_Dex(void)
{
    LinkerFunc Linker = (LinkerFunc)LINKER_FUNC_ADDR;

    for (u16 i = 1; i <= 0x401; i++)
    {
        Linker(i, 2);
        Linker(i, 3);
    }
}

#define FUNC_FLY_UNLOCK    ((void *) (0x0806E680 + 1))
#define FUNC_FINAL_CALL    ((void *) (0x08000544 + 1))
#define FINAL_ARG          ((void *) (0x080C4EF8 + 1))

typedef void (*FlyFunc)(u16 mapSec);
typedef void (*FinalFunc)(void *ptr);

void DebugMenu_Fly(void)
{
    FlyFunc UnlockFlySpot = (FlyFunc)FUNC_FLY_UNLOCK;
    FinalFunc FinalCall = (FinalFunc)FUNC_FINAL_CALL;

    for (u16 i = 0x890; i < 0x8C4; i++)
        UnlockFlySpot(i);

    FinalCall((void *)FINAL_ARG);
}
void DebugMenu_SetterFlag(void)
{
	u16 flag = VarGet(VAR_8000);
	FlagSet(flag);
}
void DebugMenu_SetterVar(void)
{
	u16 var = VarGet(VAR_DEBUG_MENU_SET_CUSTOM_VAR);
	u16 value = VarGet(VAR_DEBUG_MENU_SET_CUSTOM_VAR_VALUE);
	VarSet(var, value);
}
