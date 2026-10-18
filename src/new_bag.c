#include "../include/global.h"
#include "../include/bg.h"
#include "../include/decompress.h"
#include "../include/event_data.h"
#include "../include/gpu_regs.h"
#include "../include/item.h"
#include "../include/item_icon.h"
#include "../include/item_menu.h"
#include "../include/list_menu.h"
#include "../include/main.h"
#include "../include/malloc.h"
#include "../include/menu.h"
#include "../include/menu_helpers.h"
#include "../include/new_menu_helpers.h"
#include "../include/overworld.h"
#include "../include/palette.h"
#include "../include/pokemon.h"
#include "../include/scanline_effect.h"
#include "../include/sound.h"
#include "../include/sprite.h"
#include "../include/string_util.h"
#include "../include/task.h"
#include "../include/text.h"
#include "../include/window.h"
#include "../include/constants/songs.h"
#include "../include/constants/species.h"
#include "../include/script_pokemon_80F8.h"
#include "../include/new/item.h"

#ifdef NEW_BAG_UI_EXPANSION

#include "../include/new/new_bag.h"
#include "../include/new/Vanilla_functions.h"

/*
 * New bag screen (replaces every vanilla bag screen except the tutorial ones).
 *
 * Storage is untouched: the 6 lists are views over the existing pockets, so saves and every other
 * CFRU feature keep working.
 *   0 ITEMS (everything not in another list + berries)   1 BALLS   2 TM/HM
 *   3 KEY ITEMS   4 MEGA STONES   5 EVOLUTION ITEMS
 *
 * Speed: all static art (tab frames, row cards, panels) lives in the BG image. The selection bar,
 * tab selector and scroll thumb are sprites, so moving the cursor only moves sprites and redraws the
 * small description box. The row text is only redrawn when the list scrolls or the tab changes.
 *
 * Compatibility: sBagMenuDisplay (0x203AD10) points to a block whose first 20 bytes follow the vanilla
 * BagMenuAlloc layout, and data[0] of the main task is a (dummy) vanilla list-menu task, so every vanilla
 * / CFRU item-use function (ItemMenu_SetExitCallback, ItemMenu_StartFadeToExitCallback, ...) works as is.
 */

#define NB_TABS         6
#define NB_ROWS         5
#define NB_MAX_ENTRIES  780
#define NB_MARK_LOC     0xAB
#define NB_MARK_ID      0xBEEF

#define NB_LOC_FIELD    0
#define NB_LOC_PARTY    1
#define NB_LOC_SHOP     2
#define NB_LOC_ITEMPC   3
#define NB_LOC_PCBOX    4
#define NB_LOC_BATTLE   5
#define NB_LOC_LAST     11

#define NB_TAG_TAB_TILES  0xB200
#define NB_TAG_TAB_PAL    0xB100
#define NB_TAG_TAB_DIM    0xB110
#define NB_TAG_BAG_TILES  0xB300
#define NB_TAG_BAG_PAL    0xB301
#define NB_TAG_ICON_TILES 0xB400
#define NB_TAG_ICON_PAL   0xB401
#define NB_TAG_SEL_PAL    0xB500
#define NB_TAG_SEL_L      0xB600
#define NB_TAG_SEL_M      0xB601
#define NB_TAG_SEL_R      0xB602
#define NB_TAG_TABSEL     0xB603
#define NB_TAG_THUMB      0xB604

#define gNB (*((struct NewBag**) 0x203AD10)) //sBagMenuDisplay
#define gNBLastTab (*((u8*) 0x203E0C7))      //Free byte in CFRU's RAM map


#ifdef NEWBAG_PROFILE
//Test only: cycles/64 spent in a piece of code, stored for the emulator harness to read
#define NB_PROF_BEGIN() do { *(vu16*) 0x400010E = 0; *(vu16*) 0x400010C = 0; *(vu16*) 0x400010E = 0x81; } while (0)
#define NB_PROF_END(slot) (*(vu16*) (0x203FF00 + (slot) * 2) = *(vu16*) 0x400010C)
#else
#define NB_PROF_BEGIN()
#define NB_PROF_END(slot)
#endif

//The GBA CPU (ARMv4T) has no BLX: every call into the vanilla ROM must be a register call (long_call)
#define NB_FAR __attribute__((long_call))

#define NB_RGB(r, g, b) ((r) | ((g) << 5) | ((b) << 10))

enum
{
	NB_WIN_TABS,
	NB_WIN_ROWS,
	NB_WIN_DETAIL,
	NB_WIN_MONEY,
	NB_WIN_POPUP,
	NB_WIN_LIST, //Hidden dummy window (BG1) that hosts the vanilla list-menu task needed by the exit pipeline
};

//Palette 15 colour indexes (text + small boxes)
enum
{
	C_TRANSPARENT,
	C_WHITE,
	C_SHADOW,
	C_LIGHTGREY,
	C_CARD,
	C_CARD_BORDER,
	C_SEL_FILL,
	C_RED,
	C_GOLD,
	C_MIDGREY,
	C_PANEL,
	C_BLUE,
	C_DARKRED,
};

enum
{
	NB_ACT_USE,
	NB_ACT_GIVE,
	NB_ACT_TOSS,
	NB_ACT_REGISTER,
	NB_ACT_CANCEL,
	NB_ACT_YES,
	NB_ACT_NO,
	NB_ACT_USE_BATTLE,
};

enum
{
	NB_Q_TOSS,
	NB_Q_SELL,
	NB_Q_DEPOSIT,
};

#define NB_QUEUE_SIZE 48
#define NB_CHUNK_GLYPHS 10

enum
{
	NB_KIND_LABEL,
	NB_KIND_ROW,
	NB_KIND_DETAIL,
	NB_KIND_PREP, //Not text: wraps the description and queues its lines
};

enum
{
	NB_COL_WHITE,
	NB_COL_GREY,
	NB_COL_DIM,
	NB_COL_RED,
};

struct NBChunk
{
	u8 kind;
	u8 idx;
	u8 win;
	u8 font;
	u8 color;
	u8 x;
	u8 y;
	u8 text[NB_CHUNK_GLYPHS + 2];
};

struct NewBag
{
	//---- Vanilla BagMenuAlloc compatible header (do not reorder) ----
	void (*exitCB)(void);        //0x00
	u8 itemOriginalLocation;     //0x04 (marker NB_MARK_LOC)
	u8 vanillaBits;              //0x05
	u16 contextMenuSelectedItem; //0x06 (marker NB_MARK_ID)
	u8 pocketScrollArrowsTask;   //0x08
	u8 pocketSwitchArrowsTask;   //0x09
	u8 nItems[3];                //0x0A
	u8 maxShowed[3];             //0x0D
	u8 data[4];                  //0x10
	//---- New bag data ----
	u8 tab;
	u8 loc;
	u8 itemIconSprite;
	u8 bagSprite;
	u8 tabSprites[NB_TABS];
	u8 selSprites[5];
	u8 tabSelSprite;
	u8 thumbSprite;
	u8 menuCursor;
	u8 menuCount;
	u8 menuActions[4];
	u8 qMode;
	u8 detailTimer;
	u8 moneyShown;
	u8 popupShown;
	u8 qHead;         //Text chunk queue (text printing is slow: it is drawn a few glyphs per frame)
	u8 qCount;
	u8 detailPrep;    //Description still has to be wrapped and queued
	struct NBChunk queue[NB_QUEUE_SIZE];
	u8 dirty;         //Bit per window whose pixels must be copied to VRAM (done in VBlank)
	u8 descLines;
	u8 descLine[3][96];
	u16 count;
	u16 top;      //First visible entry
	u16 sel;      //Highlighted entry (== count means CANCEL)
	u16 qty;      //Quantity picked for toss / sell / deposit
	u16 tabSel[NB_TABS];
	u16 tabTop[NB_TABS];
	void (*followUp)(u8);
	u16 ids[NB_MAX_ENTRIES];
	u16 qtys[NB_MAX_ENTRIES];
	u8 isBerry[NB_MAX_ENTRIES];
};

extern struct ListMenuItem* sBagListMenuItems;
extern u8 (*sBagListMenuItemStrings)[22];
extern void* sBagBgTilemapBuffer;
extern const u16 gItemsByType[];
extern NB_FAR void HandleItemRegistration(u16 item);
extern NB_FAR bool8 IsItemRegistered(u16 item);
extern NB_FAR bool8 MenuHelpers_IsLinkActive(void);
extern NB_FAR bool8 InUnionRoom(void);
extern NB_FAR bool8 ItemIsMail(u16 item);
extern NB_FAR void CB2_ChooseMonToGiveItem(void);
extern void SpriteCallbackDummy(struct Sprite* sprite);
extern NB_FAR void TransferPlttBuffer(void);
extern NB_FAR u8 ItemId_GetImportance(u16 itemId);
extern NB_FAR u16 ItemId_GetPrice(u16 itemId);
extern NB_FAR u8 GetPocketByItemId(u16 itemId);
extern NB_FAR ItemUseFunc ItemId_GetBattleFunc(u16 itemId);
extern NB_FAR void InitTMCase(u8 type, void (*exitCallback)(void), u8 allowSelectClose);
extern NB_FAR void InitBerryPouch(u8 type, void (*exitCallback)(void), u8 allowSelectClose);
extern NB_FAR u32 GetMoney(u32* moneyPtr);
extern NB_FAR void AddMoney(u32* moneyPtr, u32 toAdd);
extern NB_FAR bool8 AddPCItem(u16 itemId, u16 count);

//Graphics (generated by grit from graphics/NewBag)
extern const u8 NB_BGTiles[];
extern const u16 NB_BGMap[];
extern const u8 NB_BGPal[];
extern const u8 NB_BagBoyTiles[];
extern const u8 NB_BagBoyPal[];
extern const u8 NB_BagGirlTiles[];
extern const u8 NB_BagGirlPal[];
extern const u8 NB_SelLTiles[];
extern const u8 NB_SelMTiles[];
extern const u8 NB_SelRTiles[];
extern const u8 NB_SelMPal[];
extern const u8 NB_TabSelTiles[];
extern const u8 NB_ThumbTiles[];
extern const u8 NB_Tab0Tiles[];
extern const u8 NB_Tab0Pal[];
extern const u8 NB_Tab1Tiles[];
extern const u8 NB_Tab1Pal[];
extern const u8 NB_Tab2Tiles[];
extern const u8 NB_Tab2Pal[];
extern const u8 NB_Tab3Tiles[];
extern const u8 NB_Tab3Pal[];
extern const u8 NB_Tab4Tiles[];
extern const u8 NB_Tab4Pal[];
extern const u8 NB_Tab5Tiles[];
extern const u8 NB_Tab5Pal[];

//Strings (strings/new_bag.string)
extern const u8 gText_NB_TabLabel0[];
extern const u8 gText_NB_TabLabel1[];
extern const u8 gText_NB_TabLabel2[];
extern const u8 gText_NB_TabLabel3[];
extern const u8 gText_NB_TabLabel4[];
extern const u8 gText_NB_TabLabel5[];
extern const u8 gText_NB_Close[];
extern const u8 gText_NB_Use[];
extern const u8 gText_NB_Give[];
extern const u8 gText_NB_Toss[];
extern const u8 gText_NB_Register[];
extern const u8 gText_NB_Unregister[];
extern const u8 gText_NB_Cancel[];
extern const u8 gText_NB_Yes[];
extern const u8 gText_NB_No[];
extern const u8 gText_NB_TossHowMany[];
extern const u8 gText_NB_ThrowAway[];
extern const u8 gText_NB_ThrewAway[];
extern const u8 gText_NB_NoPokemon[];
extern const u8 gText_NB_CantHold[];
extern const u8 gText_NB_CantWriteMail[];
extern const u8 gText_NB_Reg[];
extern const u8 gText_NB_SellHowMany[];
extern const u8 gText_NB_CantBuy[];
extern const u8 gText_NB_ICanPay[];
extern const u8 gText_NB_WouldThatBeOK[];
extern const u8 gText_NB_Sold[];
extern const u8 gText_NB_For[];
extern const u8 gText_NB_DepositHowMany[];
extern const u8 gText_NB_Deposited[];
extern const u8 gText_NB_NoRoomPC[];
extern const u8 gText_NB_CatHP[];
extern const u8 gText_NB_CatStatus[];
extern const u8 gText_NB_CatPP[];
extern const u8 gText_NB_CatBattle[];
extern const u8 gText_NB_CatBoost[];
extern const u8 gText_NB_CatRepel[];
extern const u8 gText_NB_CatHeld[];
extern const u8 gText_NB_CatMega[];
extern const u8 gText_NB_CatEvo[];
extern const u8 gText_NB_CatTM[];
extern const u8 gText_NB_CatKey[];
extern const u8 gText_NB_CatBall[];
extern const u8 gText_NB_CatBerry[];
extern const u8 gText_NB_CatItem[];

static const u8 sText_Space[] = {0x00, 0xFF};
static const u8 sText_QuestionMark[] = {0xAC, 0xFF};
static const u8 sText_Period[] = {0xAD, 0xFF};

static const u8* const sTabLabels[NB_TABS] =
{
	gText_NB_TabLabel0, gText_NB_TabLabel1, gText_NB_TabLabel2, gText_NB_TabLabel3, gText_NB_TabLabel4, gText_NB_TabLabel5,
};

//Colour triples {background, foreground, shadow}. Background 0 = transparent (shows the BG art / sprites below).
static const u8 sColWhite[3]   = {C_TRANSPARENT, C_WHITE, C_SHADOW};
static const u8 sColGrey[3]    = {C_TRANSPARENT, C_LIGHTGREY, C_TRANSPARENT};
static const u8 sColDim[3]     = {C_TRANSPARENT, C_MIDGREY, C_TRANSPARENT};
static const u8 sColRed[3]     = {C_TRANSPARENT, C_RED, C_TRANSPARENT};
static const u8 sColGold[3]    = {C_PANEL, C_GOLD, C_PANEL};
static const u8 sColPopup[3]   = {C_PANEL, C_WHITE, C_SHADOW};
static const u8 sColPopupSel[3]= {C_SEL_FILL, C_WHITE, C_SHADOW};

static const u16 sNBTextPal[16] =
{
	NB_RGB(31, 0, 31),   //0 transparent
	NB_RGB(30, 30, 31),  //1 white
	NB_RGB(3, 3, 5),     //2 shadow
	NB_RGB(22, 23, 26),  //3 light grey
	NB_RGB(5, 6, 9),     //4 card fill
	NB_RGB(11, 13, 19),  //5 card border
	NB_RGB(9, 4, 6),     //6 selected fill
	NB_RGB(28, 7, 7),    //7 red
	NB_RGB(30, 24, 9),   //8 gold
	NB_RGB(14, 16, 22),  //9 mid grey
	NB_RGB(3, 4, 8),     //10 panel fill
	NB_RGB(11, 21, 29),  //11 blue
	NB_RGB(16, 4, 4),    //12 dark red
	NB_RGB(4, 6, 12),
	NB_RGB(6, 7, 11),
	NB_RGB(17, 19, 25),
};

static const struct BgTemplate sNBBgTemplates[] =
{
	{.bg = 0, .charBaseIndex = 0, .mapBaseIndex = 31, .screenSize = 0, .paletteMode = 0, .priority = 0, .baseTile = 0},
	{.bg = 1, .charBaseIndex = 0, .mapBaseIndex = 29, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0}, //Never shown
	{.bg = 3, .charBaseIndex = 2, .mapBaseIndex = 30, .screenSize = 0, .paletteMode = 0, .priority = 3, .baseTile = 0},
};

static const struct WindowTemplate sNBWindows[] =
{
	[NB_WIN_TABS]   = {.bg = 0, .tilemapLeft = 0,  .tilemapTop = 2,  .width = 30, .height = 2,  .paletteNum = 15, .baseBlock = 1},
	[NB_WIN_ROWS]   = {.bg = 0, .tilemapLeft = 9,  .tilemapTop = 4,  .width = 21, .height = 10, .paletteNum = 15, .baseBlock = 61},
	[NB_WIN_DETAIL] = {.bg = 0, .tilemapLeft = 5,  .tilemapTop = 14, .width = 25, .height = 6,  .paletteNum = 15, .baseBlock = 271},
	[NB_WIN_MONEY]  = {.bg = 0, .tilemapLeft = 0,  .tilemapTop = 12, .width = 9,  .height = 2,  .paletteNum = 15, .baseBlock = 421},
	[NB_WIN_POPUP]  = {.bg = 0, .tilemapLeft = 19, .tilemapTop = 5,  .width = 10, .height = 8,  .paletteNum = 15, .baseBlock = 439},
	[NB_WIN_LIST]   = {.bg = 1, .tilemapLeft = 0,  .tilemapTop = 0,  .width = 2,  .height = 2,  .paletteNum = 15, .baseBlock = 519},
	DUMMY_WIN_TEMPLATE
};

#define OAM_DEF(name, shp, siz, prio) \
static const struct OamData name = \
{ \
	.affineMode = ST_OAM_AFFINE_OFF, \
	.objMode = ST_OAM_OBJ_NORMAL, \
	.shape = SPRITE_SHAPE(shp), \
	.size = SPRITE_SIZE(siz), \
	.priority = prio, \
}

OAM_DEF(sOam16, 16x16, 16x16, 1);
OAM_DEF(sOam32x16, 32x16, 32x16, 1);
OAM_DEF(sOam8x16, 8x16, 8x16, 1);
OAM_DEF(sOam64, 64x64, 64x64, 1);

#define SPR_TEMPLATE(name, tile, pal, oamPtr) \
static const struct SpriteTemplate name = \
{ \
	.tileTag = tile, \
	.paletteTag = pal, \
	.oam = oamPtr, \
	.anims = gDummySpriteAnimTable, \
	.images = NULL, \
	.affineAnims = gDummySpriteAffineAnimTable, \
	.callback = SpriteCallbackDummy, \
}

#define TAB_TEMPLATE(n) \
{ \
	.tileTag = NB_TAG_TAB_TILES + n, \
	.paletteTag = NB_TAG_TAB_PAL + n, \
	.oam = &sOam16, \
	.anims = gDummySpriteAnimTable, \
	.images = NULL, \
	.affineAnims = gDummySpriteAffineAnimTable, \
	.callback = SpriteCallbackDummy, \
}

static const struct SpriteTemplate sTabTemplates[NB_TABS] =
{
	TAB_TEMPLATE(0), TAB_TEMPLATE(1), TAB_TEMPLATE(2), TAB_TEMPLATE(3), TAB_TEMPLATE(4), TAB_TEMPLATE(5),
};

SPR_TEMPLATE(sSelLTemplate, NB_TAG_SEL_L, NB_TAG_SEL_PAL, &sOam32x16);
SPR_TEMPLATE(sSelMTemplate, NB_TAG_SEL_M, NB_TAG_SEL_PAL, &sOam32x16);
SPR_TEMPLATE(sSelRTemplate, NB_TAG_SEL_R, NB_TAG_SEL_PAL, &sOam32x16);
SPR_TEMPLATE(sTabSelTemplate, NB_TAG_TABSEL, NB_TAG_SEL_PAL, &sOam32x16);
SPR_TEMPLATE(sThumbTemplate, NB_TAG_THUMB, NB_TAG_SEL_PAL, &sOam8x16);
//Backpack: the sheet has 6 poses (64 tiles each). Each tab shows the pose where that kind of item is kept, like the original bag.
#define NB_BAG_FRAME(n) ANIMCMD_FRAME((n) * 64, 0)
static const union AnimCmd sBagAnim0[] = {NB_BAG_FRAME(3), ANIMCMD_END}; //POTIONS: main compartment (flap open)
static const union AnimCmd sBagAnim1[] = {NB_BAG_FRAME(4), ANIMCMD_END}; //BALLS: front ball pockets
static const union AnimCmd sBagAnim2[] = {NB_BAG_FRAME(0), ANIMCMD_END}; //ITEMS: front
static const union AnimCmd sBagAnim3[] = {NB_BAG_FRAME(1), ANIMCMD_END}; //KEY: side pocket
static const union AnimCmd sBagAnim4[] = {NB_BAG_FRAME(5), ANIMCMD_END}; //MEGA
static const union AnimCmd sBagAnim5[] = {NB_BAG_FRAME(2), ANIMCMD_END}; //EVO.: left pocket
static const union AnimCmd* const sBagAnims[NB_TABS] = {sBagAnim0, sBagAnim1, sBagAnim2, sBagAnim3, sBagAnim4, sBagAnim5};

//Little hop every time the pose changes
static void SpriteCB_NBBag(struct Sprite* sprite)
{
	static const s8 sHop[] = {-6, -9, -9, -7, -4, -2, 0};

	if (sprite->data[0] > 0)
	{
		--sprite->data[0];
		sprite->pos2.y = sHop[6 - sprite->data[0]];
	}
	else
		sprite->pos2.y = 0;
}

static const struct SpriteTemplate sBagTemplate =
{
	.tileTag = NB_TAG_BAG_TILES,
	.paletteTag = NB_TAG_BAG_PAL,
	.oam = &sOam64,
	.anims = sBagAnims,
	.images = NULL,
	.affineAnims = gDummySpriteAffineAnimTable,
	.callback = SpriteCB_NBBag,
};

static const struct CompressedSpriteSheet sTabSheets[NB_TABS] =
{
	{NB_Tab0Tiles, 128, NB_TAG_TAB_TILES + 0},
	{NB_Tab1Tiles, 128, NB_TAG_TAB_TILES + 1},
	{NB_Tab2Tiles, 128, NB_TAG_TAB_TILES + 2},
	{NB_Tab3Tiles, 128, NB_TAG_TAB_TILES + 3},
	{NB_Tab4Tiles, 128, NB_TAG_TAB_TILES + 4},
	{NB_Tab5Tiles, 128, NB_TAG_TAB_TILES + 5},
};

static const u8* const sTabPals[NB_TABS] =
{
	NB_Tab0Pal, NB_Tab1Pal, NB_Tab2Pal, NB_Tab3Pal, NB_Tab4Pal, NB_Tab5Pal,
};

static const struct CompressedSpriteSheet sBagSheets[2] =
{
	{NB_BagBoyTiles, 12288, NB_TAG_BAG_TILES},
	{NB_BagGirlTiles, 12288, NB_TAG_BAG_TILES},
};

static const struct CompressedSpriteSheet sSelSheetL = {NB_SelLTiles, 256, NB_TAG_SEL_L};
static const struct CompressedSpriteSheet sSelSheetM = {NB_SelMTiles, 256, NB_TAG_SEL_M};
static const struct CompressedSpriteSheet sSelSheetR = {NB_SelRTiles, 256, NB_TAG_SEL_R};
static const struct CompressedSpriteSheet sTabSelSheet = {NB_TabSelTiles, 256, NB_TAG_TABSEL};
static const struct CompressedSpriteSheet sThumbSheet = {NB_ThumbTiles, 64, NB_TAG_THUMB};

//This file's functions
static void CB2_NewBag(void);
static void CB2_NewBagRun(void);
static void VBlankCB_NewBag(void);
static void Task_NB_Input(u8 taskId);
static void Task_NB_InputMain(u8 taskId);
static void Task_NB_Menu(u8 taskId);
static void Task_NB_WaitMessage(u8 taskId);
static void Task_NB_Qty(u8 taskId);
static void Task_NB_YesNo(u8 taskId);
static void Task_NB_ReturnToList(u8 taskId);
static void NB_BuildEntries(u8 tab);
static void NB_UpdateTabVisuals(void);
static void NB_UpdateSelectionSprites(void);
static void NB_QueueDetail(void);
static void NB_PrintText(const u8* str);
static void NB_SetSelection(s32 newSel, bool8 sound);
static void NB_SwitchTab(s8 dir);
static void NB_OpenMenu(u8 taskId, u16 item);
static void NB_DrawMenu(void);
static void NB_CloseMenu(void);
static void NB_ShowMessage(u8 taskId, const u8* str, void (*followUp)(u8));
static void NB_Exit(u8 taskId);
static void NB_DoAction(u8 taskId, u8 action);
static void NB_RebuildAndReturn(u8 taskId);
static void NB_DrawQty(void);
static void NB_StartQty(u8 taskId, u8 mode, u16 max, const u8* prompt);
static void NB_StartYesNo(u8 taskId);
static void NB_DrawMoney(void);
static void NB_GiveToMon(u8 taskId, u16 item);
static void NB_SellStart(u8 taskId, u16 item);
static void NB_DepositStart(u8 taskId, u16 item);
static void NB_DepositDo(u8 taskId);
static void NB_SellDo(u8 taskId);
static void NB_TossDo(u8 taskId);

//The vanilla ROM was built with agbcc, which does not zero-extend u8/bool8 return values. These aliases of the
//vanilla functions (see BPRE.ld) are declared as returning u32 and truncated here, so the checks are correct.
extern NB_FAR u32 NBv_ItemIsMail(u32 item);
extern NB_FAR u32 NBv_LinkActive(void);
extern NB_FAR u32 NBv_InUnionRoom(void);
extern NB_FAR u32 NBv_Importance(u32 item);
extern NB_FAR u32 NBv_PartyCount(void);
extern NB_FAR u32 NBv_ItemType(u32 item);
extern NB_FAR u32 NBv_Pocket(u32 item);
extern NB_FAR u32 NBv_AddPCItem(u32 item, u32 count);
#define NB_ITEM_IS_MAIL(i)        ((u8) NBv_ItemIsMail(i))
#define NB_LINK_ACTIVE()          ((u8) NBv_LinkActive())
#define NB_IN_UNION_ROOM()        ((u8) NBv_InUnionRoom())
#define NB_IMPORTANCE(i)          ((u8) NBv_Importance(i))
#define NB_PARTY_COUNT()          ((u8) NBv_PartyCount())
#define NB_ITEM_TYPE(i)           ((u8) NBv_ItemType(i))
#define NB_POCKET(i)              ((u8) NBv_Pocket(i))
#define NB_ADD_PC_ITEM(i, n)      ((u8) NBv_AddPCItem(i, n))

//Mail can't be given/written while linked (vanilla rule); every other item is fine
static bool8 NB_MailAllowed(u16 item)
{
	return !NB_ITEM_IS_MAIL(item) || !(NB_LINK_ACTIVE() || NB_IN_UNION_ROOM());
}

static bool8 NB_Active(void)
{
	struct NewBag* bag = gNB;
	return bag != NULL
		&& bag->itemOriginalLocation == NB_MARK_LOC
		&& bag->contextMenuSelectedItem == NB_MARK_ID;
}

//---------------------------------------------------------------------------------------------------
// Entries
//---------------------------------------------------------------------------------------------------
//0 = healing (POTIONS tab), 2 = any other item (ITEMS tab), 4 = mega stones, 5 = evolution items
static u8 NB_TabForRegularItem(u16 item)
{
	u16 t = gItemsByType[item];

	if (t == ITEM_TYPE_MEGA_STONE)
		return 4;
	if (t == ITEM_TYPE_EVOLUTION_STONE || t == ITEM_TYPE_EVOLUTION_ITEM)
		return 5;
	if (t >= ITEM_TYPE_FULL_RESTORE && t <= ITEM_TYPE_PP_RECOVERY)
		return 0; //Health recovery, status recovery, PP recovery
	return 2;
}

static void NB_AddSlots(struct NewBag* bag, const struct ItemSlot* slots, u32 amount, s8 onlyTab, bool8 skipCases, u8 berry)
{
	u32 i;

	for (i = 0; i < amount && bag->count < NB_MAX_ENTRIES; ++i)
	{
		u16 item = slots[i].itemId;
		if (item == ITEM_NONE)
			continue;

		if (onlyTab >= 0 && NB_TabForRegularItem(item) != onlyTab)
			continue;

		//TM Case / Berry Pouch only make sense in the overworld: TMs and berries are listed directly here
		if (skipCases && (item == ITEM_TM_CASE || item == ITEM_BERRY_POUCH))
			continue;

		bag->ids[bag->count] = item;
		bag->qtys[bag->count] = GetBagItemQuantity((u16*) &slots[i].quantity);
		bag->isBerry[bag->count] = berry; //0 normal, 1 berry, 2 TM/HM
		++bag->count;
	}
}

//TMs and berries live in the TM CASE and BERRY POUCH (key items). Only the screens that hand over single items
//(give / sell / deposit) list them directly, inside the ITEMS tab, otherwise they could not be given or sold.
//TMs live only in the TM CASE and berries only in the BERRY POUCH (both key items), exactly like the vanilla bag.
//The key list shows them everywhere except the PC item deposit (where vanilla can't deposit them either).
static void NB_BuildEntries(u8 tab)
{
	struct NewBag* bag = gNB;
	bool8 hideCases = (bag->loc == NB_LOC_ITEMPC);
	bag->count = 0;

	switch (tab) {
		case 0:
			NB_AddSlots(bag, gBagPockets->itemRam, gBagPockets->itemAmount, 0, FALSE, 0);
			break;
		case 1:
			NB_AddSlots(bag, gBagPockets->pokeBallRam, gBagPockets->pokeBallAmount, -1, FALSE, 0);
			break;
		case 2:
			NB_AddSlots(bag, gBagPockets->itemRam, gBagPockets->itemAmount, 2, FALSE, 0);
			break;
		case 3:
			NB_AddSlots(bag, gBagPockets->keyItemRam, gBagPockets->keyItemAmount, -1, hideCases, 0);
			break;
		case 4:
			NB_AddSlots(bag, gBagPockets->itemRam, gBagPockets->itemAmount, 4, FALSE, 0);
			break;
		case 5:
			NB_AddSlots(bag, gBagPockets->itemRam, gBagPockets->itemAmount, 5, FALSE, 0);
			break;
	}
}

//---------------------------------------------------------------------------------------------------
// Drawing
//---------------------------------------------------------------------------------------------------
//The vanilla window copy goes through a DMA queue that only moves a couple of KB per frame (a tab switch took ~8
//frames). The text boxes here are static in VRAM, so their pixels are copied straight into VRAM instead.
//The vanilla window copy goes through a DMA queue that only moves ~1.5KB per frame. The windows here are static
//in VRAM, so their pixels are copied straight into VRAM inside the VBlank handler (no tearing, no waiting).
static void NB_CopyWindowGfx(u8 win)
{
	u16 base = GetWindowAttribute(win, WINDOW_BASE_BLOCK);
	u32 size = GetWindowAttribute(win, WINDOW_WIDTH) * GetWindowAttribute(win, WINDOW_HEIGHT) * 32;
	CpuFastCopy((void*) GetWindowAttribute(win, WINDOW_TILE_DATA), (void*) (VRAM + base * 32), size);
}

static void NB_MarkDirty(u8 win)
{
	gNB->dirty |= (1 << win);
}

//First time a window appears (also copies its tilemap)
static void NB_ShowWindow(u8 win)
{
	PutWindowTilemap(win);
	CopyWindowToVram(win, COPYWIN_MAP);
	NB_MarkDirty(win);
}

static void NB_Box(u8 win, u16 x, u16 y, u16 w, u16 h, u8 border, u8 fill)
{
	FillWindowPixelRect(win, PIXEL_FILL(border), x + 1, y, w - 2, h);
	FillWindowPixelRect(win, PIXEL_FILL(border), x, y + 1, w, h - 2);
	FillWindowPixelRect(win, PIXEL_FILL(fill), x + 1, y + 1, w - 2, h - 2);
}

//Re-wraps a string to fit a given pixel width (descriptions were written for other window sizes)
static void NB_WrapText(u8* dst, const u8* src, u8 font, u16 maxWidth, u32 dstSize)
{
	u32 i = 0;
	u8* lineStart;
	u8* lastSpace = NULL;
	u8* p;

	while (src[i] != EOS && i < dstSize - 1)
	{
		u8 ch = src[i];
		dst[i] = (ch == 0xFE || ch == 0xFA || ch == 0xFB) ? 0x00 : ch; //Newline / scroll / paragraph -> space
		++i;
	}
	dst[i] = EOS;

	lineStart = dst;
	p = dst;
	while (TRUE)
	{
		u8 ch = *p;
		if (ch == 0x00 || ch == EOS)
		{
			s32 w;
			*p = EOS;
			w = GetStringWidth(font, lineStart, 0);
			*p = ch;
			if (w > maxWidth && lastSpace != NULL)
			{
				*lastSpace = 0xFE; //Newline
				lineStart = lastSpace + 1;
				lastSpace = NULL;
				continue; //Test this same position again with the new line start
			}
			if (ch == EOS)
				break;
			lastSpace = p;
		}
		++p;
	}
}

static void NB_PrintWrapped(u8 win, u8 font, u16 x, u16 y, u8 lineH, u8 maxLines, const u8* color, const u8* str, u16 maxW)
{
	u8 buf[256];
	u8 line[96];
	const u8* p;
	u32 n = 0;

	NB_WrapText(buf, str, font, maxW, sizeof(buf));
	p = buf;
	while (*p != EOS && n < maxLines)
	{
		u32 i = 0;
		while (*p != EOS && *p != 0xFE && i < sizeof(line) - 1)
			line[i++] = *p++;
		line[i] = EOS;
		if (*p == 0xFE)
			++p;
		AddTextPrinterParameterized3(win, font, x, y + n * lineH, color, 0, line);
		++n;
	}
}

//Draw (or redraw) one tab label. Returns a glyph-count cost estimate
static const u8* NB_ColorPtr(u8 id)
{
	switch (id) {
		case NB_COL_GREY: return sColGrey;
		case NB_COL_DIM: return sColDim;
		case NB_COL_RED: return sColRed;
		default: return sColWhite;
	}
}

//Queue a string, split into short chunks so that no single print takes longer than ~1/2 frame
static void NB_QueueText(u8 kind, u8 idx, u8 win, u8 font, u16 x, u16 y, u8 color, const u8* str)
{
	struct NewBag* bag = gNB;
	bool8 splittable = TRUE;
	u32 len = StringLength(str);
	u32 pos = 0;
	u32 i;

	for (i = 0; i < len; ++i)
		if (str[i] >= 0xF7 && str[i] != 0xFF) //Control codes can't be cut in the middle
			splittable = FALSE;

	while (pos < len && bag->qCount < NB_QUEUE_SIZE)
	{
		struct NBChunk* ch = &bag->queue[(bag->qHead + bag->qCount) % NB_QUEUE_SIZE];
		u32 n = splittable ? len - pos : (len - pos > NB_CHUNK_GLYPHS ? NB_CHUNK_GLYPHS : len - pos);
		u8 tmp[40];
		u32 w = 0;

		if (splittable)
		{
			n = (len - pos > NB_CHUNK_GLYPHS) ? NB_CHUNK_GLYPHS : len - pos;
			//Don't leave a single glyph alone in the last chunk
			if (len - pos - n == 1)
				n += 1;
			if (pos > 0)
			{
				for (i = 0; i < pos && i < sizeof(tmp) - 1; ++i)
					tmp[i] = str[i];
				tmp[i] = EOS;
				w = GetStringWidth(font, tmp, 0);
			}
		}
		else
			n = len; //Whole string in one chunk (only possible for short strings)

		if (n > NB_CHUNK_GLYPHS + 1 && splittable)
			n = NB_CHUNK_GLYPHS + 1;
		if (n > sizeof(ch->text) - 1)
			n = sizeof(ch->text) - 1;

		ch->kind = kind;
		ch->idx = idx;
		ch->win = win;
		ch->font = font;
		ch->color = color;
		ch->x = x + w;
		ch->y = y;
		for (i = 0; i < n; ++i)
			ch->text[i] = str[pos + i];
		ch->text[n] = EOS;
		pos += n;
		++bag->qCount;
		if (!splittable)
			break;
	}
}

//Removes the pending chunks of a kind (idx 0xFF = every idx)
static void NB_DropChunks(u8 kind, u8 idx)
{
	struct NewBag* bag = gNB;
	u8 n = 0;
	u8 i;
	struct NBChunk tmp[NB_QUEUE_SIZE];

	for (i = 0; i < bag->qCount; ++i)
	{
		struct NBChunk* ch = &bag->queue[(bag->qHead + i) % NB_QUEUE_SIZE];
		if (ch->kind == kind && (idx == 0xFF || ch->idx == idx))
			continue;
		tmp[n++] = *ch;
	}
	for (i = 0; i < n; ++i)
		bag->queue[i] = tmp[i];
	bag->qHead = 0;
	bag->qCount = n;
}

static void NB_QueueLabel(u8 i)
{
	s32 w = GetStringWidth(0, sTabLabels[i], 0);
	u16 x = 4 + i * 38;

	NB_DropChunks(NB_KIND_LABEL, i);
	FillWindowPixelRect(NB_WIN_TABS, PIXEL_FILL(0), x, 0, 34, 16);
	NB_MarkDirty(NB_WIN_TABS);
	NB_QueueText(NB_KIND_LABEL, i, NB_WIN_TABS, 0, x + (34 - w) / 2, 0, i == gNB->tab ? NB_COL_WHITE : NB_COL_DIM, sTabLabels[i]);
}

static void NB_UpdateTabVisuals(void)
{
	struct NewBag* bag = gNB;
	u32 i;

	for (i = 0; i < NB_TABS; ++i)
		gSprites[bag->tabSprites[i]].oam.paletteNum = IndexOfSpritePaletteTag(i == bag->tab ? NB_TAG_TAB_PAL + i : NB_TAG_TAB_DIM + i);

	gSprites[bag->tabSelSprite].pos1.x = 21 + bag->tab * 38;
	if (bag->bagSprite != 0 && gSprites[bag->bagSprite].inUse) //(bagSprite is created after the first call)
	{
		StartSpriteAnim(&gSprites[bag->bagSprite], bag->tab);
		gSprites[bag->bagSprite].data[0] = 6; //Start the hop
	}
}

static const u8* NB_CategoryName(u16 item, u8 kind)
{
	u16 t = gItemsByType[item];

	switch (gNB->tab) {
		case 1: return gText_NB_CatBall;
		case 3: return gText_NB_CatKey;
		case 4: return gText_NB_CatMega;
		case 5: return gText_NB_CatEvo;
	}

	if (kind == 1)
		return gText_NB_CatBerry;
	if (kind == 2)
		return gText_NB_CatTM;
	if (t == 0 && item != ITEM_MAX_REPEL) //Not listed in gItemsByType
		return gText_NB_CatItem;
	if (t <= ITEM_TYPE_REPEL)
		return gText_NB_CatRepel;
	if (t >= ITEM_TYPE_FULL_RESTORE && t <= ITEM_TYPE_HERB_HEAL)
		return gText_NB_CatHP;
	if (t >= ITEM_TYPE_FULL_HEAL && t <= ITEM_TYPE_STATUS_RECOVERY)
		return gText_NB_CatStatus;
	if (t == ITEM_TYPE_PP_RECOVERY)
		return gText_NB_CatPP;
	if (t >= ITEM_TYPE_X_ATTACK && t <= ITEM_TYPE_OTHER_BATTLE_ITEM)
		return gText_NB_CatBattle;
	if (t >= ITEM_TYPE_ABILITY_GIGANTAMAX_MODIFIER && t <= ITEM_TYPE_SPEED_WING)
		return gText_NB_CatBoost;
	if (t >= ITEM_TYPE_FLUTE && t <= ITEM_TYPE_PLATE)
		return gText_NB_CatHeld;
	return gText_NB_CatItem;
}

//Draw (or redraw) one visible row. Returns a glyph-count cost estimate
//(Re)draw one visible row: clears it now and queues its text
static void NB_QueueRow(u8 r)
{
	struct NewBag* bag = gNB;
	u16 idx = bag->top + r;
	u16 y = r * 16;

	NB_DropChunks(NB_KIND_ROW, r);
	FillWindowPixelRect(NB_WIN_ROWS, PIXEL_FILL(0), 0, y, 168, 16);
	NB_MarkDirty(NB_WIN_ROWS);

	if (idx > bag->count)
		return;

	if (idx < bag->count)
	{
		u16 item = bag->ids[idx];
		u8 buf[12];

		NB_QueueText(NB_KIND_ROW, r, NB_WIN_ROWS, 2, 8, y, NB_COL_WHITE, ItemId_GetName(item));

		if (bag->tab == 3)
		{
			if (IsItemRegistered(item))
				NB_QueueText(NB_KIND_ROW, r, NB_WIN_ROWS, 0, 134, y + 1, NB_COL_RED, gText_NB_Reg);
		}
		else
		{
			buf[0] = 0xB9; //x
			ConvertIntToDecimalStringN(buf + 1, bag->qtys[idx], STR_CONV_MODE_RIGHT_ALIGN, 3);
			NB_QueueText(NB_KIND_ROW, r, NB_WIN_ROWS, 2, 154 - GetStringWidth(2, buf, 0), y, NB_COL_GREY, buf);
		}
	}
	else
		NB_QueueText(NB_KIND_ROW, r, NB_WIN_ROWS, 2, 8, y, NB_COL_WHITE, gText_NB_Cancel);
}

static void NB_QueueRows(u8 mask)
{
	u8 r;
	for (r = 0; r < NB_ROWS; ++r)
		if (mask & (1 << r))
			NB_QueueRow(r);
}

static void NB_QueueLabelsFor(u8 a, u8 b)
{
	NB_QueueLabel(a);
	if (b != a)
		NB_QueueLabel(b);
}

//When the rows scroll, text that is still waiting to be drawn moves with its row
static void NB_ShiftRowChunks(s8 delta)
{
	struct NewBag* bag = gNB;
	u8 n = 0;
	u8 i;
	struct NBChunk tmp[NB_QUEUE_SIZE];

	for (i = 0; i < bag->qCount; ++i)
	{
		struct NBChunk* ch = &bag->queue[(bag->qHead + i) % NB_QUEUE_SIZE];
		if (ch->kind == NB_KIND_ROW)
		{
			s16 r = (s16) ch->idx + delta;
			if (r < 0 || r >= NB_ROWS)
				continue; //That row left the list
			tmp[n] = *ch;
			tmp[n].idx = r;
			tmp[n].y += delta * 16;
			++n;
		}
		else
			tmp[n++] = *ch;
	}
	for (i = 0; i < n; ++i)
		bag->queue[i] = tmp[i];
	bag->qHead = 0;
	bag->qCount = n;
}

//Scrolling by exactly one row: shift the rows that are already drawn and only draw the one that appears
static void NB_ScrollRows(s8 dir)
{
	u8* data = (u8*) GetWindowAttribute(NB_WIN_ROWS, WINDOW_TILE_DATA);
	u32 rowBytes = 2 * 21 * 32; //Two tile rows = 16 pixels
	u32 words = rowBytes / 4;
	vu32 *dst, *src; //volatile: keeps the compiler from turning these loops into a memcpy call
	u32 i;

	if (dir > 0) //List moves up: a new row appears at the bottom
	{
		dst = (vu32*) data;
		src = (vu32*) (data + rowBytes);
		for (i = 0; i < words * (NB_ROWS - 1); ++i)
			dst[i] = src[i];
		NB_ShiftRowChunks(-1);
		NB_QueueRow(NB_ROWS - 1);
	}
	else //List moves down: a new row appears at the top
	{
		dst = (vu32*) (data + rowBytes);
		src = (vu32*) data;
		for (i = words * (NB_ROWS - 1); i > 0; --i)
			dst[i - 1] = src[i - 1];
		NB_ShiftRowChunks(1);
		NB_QueueRow(0);
	}
	NB_MarkDirty(NB_WIN_ROWS);
}

static void NB_UpdateSelectionSprites(void)
{
	struct NewBag* bag = gNB;
	u16 total = bag->count + 1;
	u32 i;
	u16 y = 40 + (bag->sel - bag->top) * 16;

	for (i = 0; i < 5; ++i)
		gSprites[bag->selSprites[i]].pos1.y = y;

	if (total > NB_ROWS)
		gSprites[bag->thumbSprite].pos1.y = 41 + (78 - 16) * bag->top / (total - NB_ROWS);
	else
		gSprites[bag->thumbSprite].pos1.y = 41;
	gSprites[bag->thumbSprite].invisible = (total <= NB_ROWS);

	//Remember the position for the next time the bag opens (slot 1 is the vanilla key-item pocket memory: harmless)
	gBagMenuState.cursorPos[1] = bag->sel;
	gBagMenuState.itemsAbove[1] = bag->top;
}

static void NB_UpdateItemIcon(u16 item)
{
	struct NewBag* bag = gNB;

	if (bag->itemIconSprite != 0xFF)
	{
		FreeSpriteTilesByTag(NB_TAG_ICON_TILES);
		FreeSpritePaletteByTag(NB_TAG_ICON_PAL);
		DestroySprite(&gSprites[bag->itemIconSprite]);
		bag->itemIconSprite = 0xFF;
	}

	if (item != ITEM_NONE)
	{
		u8 id = AddItemIconSprite(NB_TAG_ICON_TILES, NB_TAG_ICON_PAL, item);
		if (id != MAX_SPRITES)
		{
			gSprites[id].pos1.x = 27;
			gSprites[id].pos1.y = 140;
			gSprites[id].oam.priority = 1;
			bag->itemIconSprite = id;
		}
	}
}

static void NB_QueueDetail(void)
{
	struct NewBag* bag = gNB;
	u16 item = (bag->sel < bag->count) ? bag->ids[bag->sel] : ITEM_NONE;

	bag->detailTimer = 0;
	NB_DropChunks(NB_KIND_DETAIL, 0xFF);
	NB_DropChunks(NB_KIND_PREP, 0xFF);
	FillWindowPixelBuffer(NB_WIN_DETAIL, PIXEL_FILL(0));
	NB_MarkDirty(NB_WIN_DETAIL);
	NB_UpdateItemIcon(item);
	NB_QueueText(NB_KIND_DETAIL, 0, NB_WIN_DETAIL, 2, 4, 2, NB_COL_WHITE, item == ITEM_NONE ? gText_NB_Cancel : ItemId_GetName(item));

	//Wrapping + category are queued afterwards (they are not cheap either)
	if (bag->qCount < NB_QUEUE_SIZE)
	{
		struct NBChunk* ch = &bag->queue[(bag->qHead + bag->qCount) % NB_QUEUE_SIZE];
		ch->kind = NB_KIND_PREP;
		ch->idx = 0;
		++bag->qCount;
	}
}

//One step of the description box (name, category, then one description line per step).
//Returns a glyph-count cost estimate

//Bounded amount of text rendering per call (printing costs ~10k cycles per glyph) so input is never blocked
//for more than a fraction of a frame


//Messages replace the description box text
static void NB_PrintText(const u8* str)
{
	FillWindowPixelBuffer(NB_WIN_DETAIL, PIXEL_FILL(0));
	NB_DropChunks(NB_KIND_DETAIL, 0xFF); //Cancel any pending description
	NB_DropChunks(NB_KIND_PREP, 0xFF);
	gNB->detailTimer = 0;
	NB_PrintWrapped(NB_WIN_DETAIL, 2, 4, 3, 14, 3, sColWhite, str, 188);
	NB_MarkDirty(NB_WIN_DETAIL);
}

static void NB_DrawMoney(void)
{
	struct NewBag* bag = gNB;
	u8 buf[12];

	if (bag->loc != NB_LOC_SHOP)
		return;

	FillWindowPixelBuffer(NB_WIN_MONEY, PIXEL_FILL(0));
	NB_Box(NB_WIN_MONEY, 0, 0, 70, 16, C_CARD_BORDER, C_PANEL);
	buf[0] = 0xB7; //Pokedollar
	ConvertIntToDecimalStringN(buf + 1, GetMoney(&gSaveBlock1->money), STR_CONV_MODE_LEFT_ALIGN, 7);
	AddTextPrinterParameterized3(NB_WIN_MONEY, 2, 68 - GetStringWidth(2, buf, 0), 0, sColGold, 0, buf);
	if (!bag->moneyShown)
	{
		bag->moneyShown = TRUE;
		NB_ShowWindow(NB_WIN_MONEY);
	}
	else
		NB_MarkDirty(NB_WIN_MONEY);
}

//---------------------------------------------------------------------------------------------------
// Selection / navigation
//---------------------------------------------------------------------------------------------------
//Prepares the description: category next to the name, then the wrapped description lines
static u32 NB_RunPrep(void)
{
	struct NewBag* bag = gNB;
	u16 item = (bag->sel < bag->count) ? bag->ids[bag->sel] : ITEM_NONE;
	const u8* desc;
	u8 wrapped[256];
	const u8* p;
	u32 n = 0;

	if (item != ITEM_NONE)
	{
		u8 buf[24];
		s32 nameW = GetStringWidth(2, ItemId_GetName(item), 0);
		buf[0] = 0x5C; //(
		buf[1] = EOS;
		StringAppend(buf, NB_CategoryName(item, bag->isBerry[bag->sel]));
		StringAppend(buf, (const u8[]) {0x5D, 0xFF}); //)
		NB_QueueText(NB_KIND_DETAIL, 0, NB_WIN_DETAIL, 0, 4 + nameW + 6, 5, NB_COL_DIM, buf);
		desc = ItemId_GetDescription(item);
	}
	else
		desc = gText_NB_Close;

	NB_WrapText(wrapped, desc, 0, 188, sizeof(wrapped));
	p = wrapped;
	while (*p != EOS && n < 3)
	{
		u8 line[96];
		u32 i = 0;
		while (*p != EOS && *p != 0xFE && i < sizeof(line) - 1)
			line[i++] = *p++;
		line[i] = EOS;
		if (*p == 0xFE)
			++p;
		NB_QueueText(NB_KIND_DETAIL, 0, NB_WIN_DETAIL, 0, 4, 14 + n * 9, NB_COL_GREY, line);
		++n;
	}
	return 8;
}

//Draws queued text until the budget (in glyphs; ~10k cycles each) is used up
static void NB_RunWork(s32 budget)
{
	struct NewBag* bag = gNB;

	while (budget > 0 && bag->qCount != 0)
	{
		struct NBChunk ch = bag->queue[bag->qHead];
		bag->qHead = (bag->qHead + 1) % NB_QUEUE_SIZE;
		--bag->qCount;

		if (ch.kind == NB_KIND_PREP)
		{
			budget -= NB_RunPrep();
			continue;
		}

		AddTextPrinterParameterized3(ch.win, ch.font, ch.x, ch.y, NB_ColorPtr(ch.color), 0, ch.text);
		NB_MarkDirty(ch.win);
		budget -= StringLength(ch.text) + 3;
	}
}

static void NB_FlushWork(void)
{
	while (gNB->qCount != 0)
		NB_RunWork(0x7FFF);
}

static void NB_SetSelection(s32 newSel, bool8 sound)
{
	struct NewBag* bag = gNB;
	u16 oldTop = bag->top;

	if (newSel < 0 || newSel > bag->count || newSel == bag->sel)
		return;

	if (sound)
		PlaySE(SE_SELECT);

	bag->sel = newSel;
	if (newSel < bag->top)
		bag->top = newSel;
	else if (newSel >= bag->top + NB_ROWS)
		bag->top = newSel - NB_ROWS + 1;

	if (oldTop != bag->top)
	{
		if (bag->top == oldTop + 1)
			NB_ScrollRows(1);
		else if (bag->top + 1 == oldTop)
			NB_ScrollRows(-1);
		else
			NB_QueueRows(0x1F);
	}

	NB_UpdateSelectionSprites();
	NB_DropChunks(NB_KIND_DETAIL, 0xFF);
	NB_DropChunks(NB_KIND_PREP, 0xFF);
	bag->detailTimer = 2; //Description is drawn once the cursor rests for a moment
}

static void NB_ClampPosition(void)
{
	struct NewBag* bag = gNB;
	u16 total = bag->count + 1;

	if (bag->sel >= total)
		bag->sel = total - 1;
	if (total <= NB_ROWS)
		bag->top = 0;
	else if (bag->top + NB_ROWS > total)
		bag->top = total - NB_ROWS;
	if (bag->sel < bag->top)
		bag->top = bag->sel;
	if (bag->sel >= bag->top + NB_ROWS)
		bag->top = bag->sel - NB_ROWS + 1;
}

static void NB_RefreshAll(void)
{
	NB_ClampPosition();
	NB_QueueRows(0x1F);
	NB_UpdateSelectionSprites();
	NB_QueueDetail();
}

static void NB_SwitchTab(s8 dir)
{
	struct NewBag* bag = gNB;
	u8 old = bag->tab;

	PlaySE(SE_SELECT);
	bag->tabSel[bag->tab] = bag->sel;
	bag->tabTop[bag->tab] = bag->top;

	bag->tab = (bag->tab + NB_TABS + dir) % NB_TABS;
	gNBLastTab = bag->tab;
	bag->sel = bag->tabSel[bag->tab];
	bag->top = bag->tabTop[bag->tab];

	NB_BuildEntries(bag->tab);
	NB_UpdateTabVisuals();
	NB_QueueLabelsFor(old, bag->tab);
	NB_RefreshAll();
}

static void NB_Exit(u8 taskId)
{
	gSpecialVar_ItemId = ITEM_NONE;
	ItemMenu_StartFadeToExitCallback(taskId);
}

//---------------------------------------------------------------------------------------------------
// Pop-up menu
//---------------------------------------------------------------------------------------------------
static const u8* NB_ActionName(u8 action)
{
	switch (action) {
		case NB_ACT_USE:
		case NB_ACT_USE_BATTLE: return gText_NB_Use;
		case NB_ACT_GIVE: return gText_NB_Give;
		case NB_ACT_TOSS: return gText_NB_Toss;
		case NB_ACT_REGISTER: return IsItemRegistered(gSpecialVar_ItemId) ? gText_NB_Unregister : gText_NB_Register;
		case NB_ACT_YES: return gText_NB_Yes;
		case NB_ACT_NO: return gText_NB_No;
		default: return gText_NB_Cancel;
	}
}

static void NB_DrawMenuRow(u8 i)
{
	struct NewBag* bag = gNB;
	bool8 sel = (i == bag->menuCursor);

	NB_Box(NB_WIN_POPUP, 2, 2 + i * 16, 76, 16, sel ? C_RED : C_PANEL, sel ? C_SEL_FILL : C_PANEL);
	AddTextPrinterParameterized3(NB_WIN_POPUP, 2, 12, 2 + i * 16, sel ? sColPopupSel : sColPopup, 0, NB_ActionName(bag->menuActions[i]));
	NB_MarkDirty(NB_WIN_POPUP);
}

//Full draw when the pop-up opens
static void NB_DrawMenu(void)
{
	struct NewBag* bag = gNB;
	u32 i;

	FillWindowPixelBuffer(NB_WIN_POPUP, PIXEL_FILL(0));
	NB_Box(NB_WIN_POPUP, 0, 0, 80, bag->menuCount * 16 + 4, C_CARD_BORDER, C_PANEL);
	for (i = 0; i < bag->menuCount; ++i)
		NB_DrawMenuRow(i);
	if (!bag->popupShown)
	{
		bag->popupShown = TRUE;
		NB_ShowWindow(NB_WIN_POPUP);
	}
	NB_MarkDirty(NB_WIN_POPUP);
}

//Moves the pop-up cursor: only the two rows that change are redrawn
static void NB_MoveMenuCursor(s8 dir)
{
	struct NewBag* bag = gNB;
	u8 old = bag->menuCursor;

	bag->menuCursor += dir;
	NB_DrawMenuRow(old);
	NB_DrawMenuRow(bag->menuCursor);
}

static void NB_CloseMenu(void)
{
	if (!gNB->popupShown)
		return;
	gNB->popupShown = FALSE;
	ClearWindowTilemap(NB_WIN_POPUP);
	CopyWindowToVram(NB_WIN_POPUP, COPYWIN_MAP);
	PutWindowTilemap(NB_WIN_ROWS); //Restores the row tiles that were covered by the pop-up
	CopyWindowToVram(NB_WIN_ROWS, COPYWIN_MAP);
}

static void NB_OpenMenu(u8 taskId, u16 item)
{
	struct NewBag* bag = gNB;
	u8 n = 0;

	if (bag->loc == NB_LOC_BATTLE)
	{
		if (ItemId_GetBattleFunc(item) != NULL)
			bag->menuActions[n++] = NB_ACT_USE_BATTLE;
	}
	else
	{
		if (ItemId_GetFieldFunc(item) != NULL)
			bag->menuActions[n++] = NB_ACT_USE;

		if (bag->tab == 3)
			bag->menuActions[n++] = NB_ACT_REGISTER;
		else
		{
			bag->menuActions[n++] = NB_ACT_GIVE;
			bag->menuActions[n++] = NB_ACT_TOSS;
		}
	}

	bag->menuActions[n++] = NB_ACT_CANCEL;
	bag->menuCount = n;
	bag->menuCursor = 0;
	NB_DrawMenu();
	gTasks[taskId].func = Task_NB_Menu;
}

static void Task_NB_Menu(u8 taskId)
{
	struct NewBag* bag = gNB;

	if (gPaletteFade.active)
		return;

	if (JOY_REPT(DPAD_UP) && bag->menuCursor > 0)
	{
		PlaySE(SE_SELECT);
		NB_MoveMenuCursor(-1);
	}
	else if (JOY_REPT(DPAD_DOWN) && bag->menuCursor + 1 < bag->menuCount)
	{
		PlaySE(SE_SELECT);
		NB_MoveMenuCursor(1);
	}
	else if (JOY_NEW(A_BUTTON))
	{
		PlaySE(SE_SELECT);
		NB_DoAction(taskId, bag->menuActions[bag->menuCursor]);
	}
	else if (JOY_NEW(B_BUTTON))
	{
		PlaySE(SE_SELECT);
		NB_DoAction(taskId, NB_ACT_CANCEL);
	}
}

//---------------------------------------------------------------------------------------------------
// Messages and returning to the list
//---------------------------------------------------------------------------------------------------
static void NB_ShowMessage(u8 taskId, const u8* str, void (*followUp)(u8))
{
	gNB->followUp = followUp;
	NB_PrintText(str);
	gTasks[taskId].func = Task_NB_WaitMessage;
}

static void Task_NB_WaitMessage(u8 taskId)
{
	if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
	{
		PlaySE(SE_SELECT);
		gTasks[taskId].func = gNB->followUp;
	}
}

//Used both by our own flows and as the vanilla "return to bag" hook
static void NB_RebuildAndReturn(u8 taskId)
{
	struct NewBag* bag = gNB;

	NB_CloseMenu();
	NB_BuildEntries(bag->tab);
	NB_RefreshAll();
	if (bag->loc == NB_LOC_SHOP)
		NB_DrawMoney();
	gTasks[taskId].func = Task_NB_Input;
}

static void Task_NB_ReturnToList(u8 taskId)
{
	NB_RebuildAndReturn(taskId);
}

//---------------------------------------------------------------------------------------------------
// Quantity picker (toss / sell / deposit) and Yes/No
//---------------------------------------------------------------------------------------------------
static void NB_DrawQty(void)
{
	struct NewBag* bag = gNB;
	u8 buf[12];
	u16 h = bag->qMode == NB_Q_SELL ? 40 : 24;

	FillWindowPixelBuffer(NB_WIN_POPUP, PIXEL_FILL(0));
	NB_Box(NB_WIN_POPUP, 0, 0, 80, h, C_CARD_BORDER, C_PANEL);
	buf[0] = 0xB9;
	ConvertIntToDecimalStringN(buf + 1, bag->qty, STR_CONV_MODE_LEADING_ZEROS, 3);
	AddTextPrinterParameterized3(NB_WIN_POPUP, 2, 24, 3, sColPopup, 0, buf);

	if (bag->qMode == NB_Q_SELL)
	{
		buf[0] = 0xB7;
		ConvertIntToDecimalStringN(buf + 1, ItemId_GetPrice(gSpecialVar_ItemId) / 2 * bag->qty, STR_CONV_MODE_LEFT_ALIGN, 7);
		AddTextPrinterParameterized3(NB_WIN_POPUP, 2, 76 - GetStringWidth(2, buf, 0), 20, sColGold, 0, buf);
	}
	if (!bag->popupShown)
	{
		bag->popupShown = TRUE;
		NB_ShowWindow(NB_WIN_POPUP);
	}
	else
		NB_MarkDirty(NB_WIN_POPUP);
}

static void NB_StartQty(u8 taskId, u8 mode, u16 max, const u8* prompt)
{
	struct NewBag* bag = gNB;

	bag->qMode = mode;
	bag->qty = 1;
	gTasks[taskId].data[2] = max;
	NB_PrintText(prompt);
	NB_DrawQty();
	gTasks[taskId].func = Task_NB_Qty;
}

static void NB_AfterQty(u8 taskId)
{
	struct NewBag* bag = gNB;

	switch (bag->qMode) {
		case NB_Q_TOSS:
			ConvertIntToDecimalStringN(gStringVar1, bag->qty, STR_CONV_MODE_LEFT_ALIGN, 3);
			CopyItemName(gSpecialVar_ItemId, gStringVar2);
			StringCopy(gStringVar4, gText_NB_ThrowAway);
			StringAppend(gStringVar4, sText_Space);
			StringAppend(gStringVar4, gStringVar1);
			StringAppend(gStringVar4, sText_Space);
			StringAppend(gStringVar4, gStringVar2);
			StringAppend(gStringVar4, sText_QuestionMark);
			NB_PrintText(gStringVar4);
			NB_StartYesNo(taskId);
			break;
		case NB_Q_SELL:
		{
			u8 yen[12];
			yen[0] = 0xB7;
			ConvertIntToDecimalStringN(yen + 1, ItemId_GetPrice(gSpecialVar_ItemId) / 2 * bag->qty, STR_CONV_MODE_LEFT_ALIGN, 7);
			StringCopy(gStringVar4, gText_NB_ICanPay);
			StringAppend(gStringVar4, sText_Space);
			StringAppend(gStringVar4, yen);
			StringAppend(gStringVar4, sText_Period);
			StringAppend(gStringVar4, sText_Space);
			StringAppend(gStringVar4, gText_NB_WouldThatBeOK);
			NB_PrintText(gStringVar4);
			NB_StartYesNo(taskId);
			break;
		}
		case NB_Q_DEPOSIT:
			NB_DepositDo(taskId);
			break;
	}
}

static void Task_NB_Qty(u8 taskId)
{
	struct NewBag* bag = gNB;
	u16 max = gTasks[taskId].data[2];
	bool8 changed = FALSE;

	if (JOY_REPT(DPAD_UP) && bag->qty < max)
	{
		++bag->qty;
		changed = TRUE;
	}
	else if (JOY_REPT(DPAD_DOWN) && bag->qty > 1)
	{
		--bag->qty;
		changed = TRUE;
	}
	else if (JOY_REPT(DPAD_RIGHT) && bag->qty < max)
	{
		bag->qty = (bag->qty + 10 > max) ? max : bag->qty + 10;
		changed = TRUE;
	}
	else if (JOY_REPT(DPAD_LEFT) && bag->qty > 1)
	{
		bag->qty = (bag->qty <= 10) ? 1 : bag->qty - 10;
		changed = TRUE;
	}
	else if (JOY_NEW(A_BUTTON))
	{
		PlaySE(SE_SELECT);
		NB_AfterQty(taskId);
		return;
	}
	else if (JOY_NEW(B_BUTTON))
	{
		PlaySE(SE_SELECT);
		NB_RebuildAndReturn(taskId);
		return;
	}

	if (changed)
	{
		PlaySE(SE_SELECT);
		NB_DrawQty();
	}
}

static void NB_StartYesNo(u8 taskId)
{
	struct NewBag* bag = gNB;

	bag->menuActions[0] = NB_ACT_YES;
	bag->menuActions[1] = NB_ACT_NO;
	bag->menuCount = 2;
	bag->menuCursor = 0; //YES is the default for selling, NO is safer for tossing
	if (bag->qMode == NB_Q_TOSS)
		bag->menuCursor = 1;
	NB_DrawMenu();
	gTasks[taskId].func = Task_NB_YesNo;
}

static void Task_NB_YesNo(u8 taskId)
{
	struct NewBag* bag = gNB;

	if (JOY_REPT(DPAD_UP) && bag->menuCursor > 0)
	{
		PlaySE(SE_SELECT);
		NB_MoveMenuCursor(-1);
	}
	else if (JOY_REPT(DPAD_DOWN) && bag->menuCursor + 1 < bag->menuCount)
	{
		PlaySE(SE_SELECT);
		NB_MoveMenuCursor(1);
	}
	else if (JOY_NEW(A_BUTTON))
	{
		PlaySE(SE_SELECT);
		if (bag->menuActions[bag->menuCursor] == NB_ACT_YES)
		{
			NB_CloseMenu();
			if (bag->qMode == NB_Q_TOSS)
				NB_TossDo(taskId);
			else
				NB_SellDo(taskId);
		}
		else
			NB_RebuildAndReturn(taskId);
	}
	else if (JOY_NEW(B_BUTTON))
	{
		PlaySE(SE_SELECT);
		NB_RebuildAndReturn(taskId);
	}
}

static void NB_TossDo(u8 taskId)
{
	struct NewBag* bag = gNB;

	RemoveBagItem(gSpecialVar_ItemId, bag->qty);
	ConvertIntToDecimalStringN(gStringVar1, bag->qty, STR_CONV_MODE_LEFT_ALIGN, 3);
	CopyItemName(gSpecialVar_ItemId, gStringVar2);
	StringCopy(gStringVar4, gText_NB_ThrewAway);
	StringAppend(gStringVar4, sText_Space);
	StringAppend(gStringVar4, gStringVar1);
	StringAppend(gStringVar4, sText_Space);
	StringAppend(gStringVar4, gStringVar2);
	StringAppend(gStringVar4, sText_Period);
	NB_ShowMessage(taskId, gStringVar4, Task_NB_ReturnToList);
}

static void NB_SellDo(u8 taskId)
{
	struct NewBag* bag = gNB;
	u8 yen[12];
	u32 total = ItemId_GetPrice(gSpecialVar_ItemId) / 2 * bag->qty;

	PlaySE(SE_RS_SHOP);
	RemoveBagItem(gSpecialVar_ItemId, bag->qty);
	AddMoney(&gSaveBlock1->money, total);
	NB_DrawMoney();

	yen[0] = 0xB7;
	ConvertIntToDecimalStringN(yen + 1, total, STR_CONV_MODE_LEFT_ALIGN, 7);
	ConvertIntToDecimalStringN(gStringVar1, bag->qty, STR_CONV_MODE_LEFT_ALIGN, 3);
	CopyItemName(gSpecialVar_ItemId, gStringVar2);
	StringCopy(gStringVar4, gText_NB_Sold);
	StringAppend(gStringVar4, sText_Space);
	StringAppend(gStringVar4, gStringVar1);
	StringAppend(gStringVar4, sText_Space);
	StringAppend(gStringVar4, gStringVar2);
	StringAppend(gStringVar4, sText_Space);
	StringAppend(gStringVar4, gText_NB_For);
	StringAppend(gStringVar4, sText_Space);
	StringAppend(gStringVar4, yen);
	StringAppend(gStringVar4, sText_Period);
	NB_ShowMessage(taskId, gStringVar4, Task_NB_ReturnToList);
}

static void NB_SellStart(u8 taskId, u16 item)
{
	u16 have = gTasks[taskId].data[2];

	if (ItemId_GetPrice(item) == 0)
	{
		NB_ShowMessage(taskId, gText_NB_CantBuy, Task_NB_ReturnToList);
		return;
	}

	gNB->qMode = NB_Q_SELL;
	if (have <= 1)
	{
		gNB->qty = 1;
		NB_AfterQty(taskId);
	}
	else
		NB_StartQty(taskId, NB_Q_SELL, have > 99 ? 99 : have, gText_NB_SellHowMany);
}

static void NB_DepositDo(u8 taskId)
{
	struct NewBag* bag = gNB;

	if (NB_ADD_PC_ITEM(gSpecialVar_ItemId, bag->qty))
	{
		RemoveBagItem(gSpecialVar_ItemId, bag->qty);
		ConvertIntToDecimalStringN(gStringVar1, bag->qty, STR_CONV_MODE_LEFT_ALIGN, 3);
		CopyItemName(gSpecialVar_ItemId, gStringVar2);
		StringCopy(gStringVar4, gText_NB_Deposited);
		StringAppend(gStringVar4, sText_Space);
		StringAppend(gStringVar4, gStringVar1);
		StringAppend(gStringVar4, sText_Space);
		StringAppend(gStringVar4, gStringVar2);
		StringAppend(gStringVar4, sText_Period);
		NB_CloseMenu();
		NB_ShowMessage(taskId, gStringVar4, Task_NB_ReturnToList);
	}
	else
	{
		NB_CloseMenu();
		NB_ShowMessage(taskId, gText_NB_NoRoomPC, Task_NB_ReturnToList);
	}
}

static void NB_DepositStart(u8 taskId, u16 item)
{
	u16 have = gTasks[taskId].data[2];
	(void) item;

	gNB->qMode = NB_Q_DEPOSIT;
	if (have <= 1)
	{
		gNB->qty = 1;
		NB_DepositDo(taskId);
	}
	else
		NB_StartQty(taskId, NB_Q_DEPOSIT, have, gText_NB_DepositHowMany);
}

//Give the item to a Pokemon (bag opened from the party menu / PC box, or "GIVE" in the overworld)
static void NB_GiveToMon(u8 taskId, u16 item)
{
	struct NewBag* bag = gNB;

	if (!NB_MailAllowed(item))
		NB_ShowMessage(taskId, gText_NB_CantWriteMail, Task_NB_ReturnToList);
	else if (bag->tab == 3 || NB_IMPORTANCE(item) != 0)
		NB_ShowMessage(taskId, gText_NB_CantHold, Task_NB_ReturnToList);
	else
		ItemMenu_StartFadeToExitCallback(taskId); //Leaves with gSpecialVar_ItemId set; the vanilla callback gives it
}

static void NB_DoAction(u8 taskId, u8 action)
{
	struct NewBag* bag = gNB;
	s16* data = gTasks[taskId].data;
	u16 item = gSpecialVar_ItemId;

	switch (action) {
		case NB_ACT_USE:
			NB_CloseMenu();
			if (NB_PARTY_COUNT() == 0 && NB_ITEM_TYPE(item) == 1) //Party menu item
				NB_ShowMessage(taskId, gText_NB_NoPokemon, Task_NB_ReturnToList);
			else
				ItemId_GetFieldFunc(item)(taskId);
			break;

		case NB_ACT_USE_BATTLE:
			NB_CloseMenu();
			ItemId_GetBattleFunc(item)(taskId);
			break;

		case NB_ACT_GIVE:
			NB_CloseMenu();
			if (!NB_MailAllowed(item))
				NB_ShowMessage(taskId, gText_NB_CantWriteMail, Task_NB_ReturnToList);
			else if (NB_IMPORTANCE(item) != 0 || bag->tab == 3)
				NB_ShowMessage(taskId, gText_NB_CantHold, Task_NB_ReturnToList);
			else if (NB_PARTY_COUNT() == 0)
				NB_ShowMessage(taskId, gText_NB_NoPokemon, Task_NB_ReturnToList);
			else
			{
				bag->exitCB = CB2_ChooseMonToGiveItem;
				ItemMenu_StartFadeToExitCallback(taskId);
			}
			break;

		case NB_ACT_TOSS:
			NB_CloseMenu();
			if (data[2] <= 1)
			{
				bag->qMode = NB_Q_TOSS;
				bag->qty = 1;
				NB_AfterQty(taskId);
			}
			else
				NB_StartQty(taskId, NB_Q_TOSS, data[2], gText_NB_TossHowMany);
			break;

		case NB_ACT_REGISTER:
			HandleItemRegistration(item);
			NB_RebuildAndReturn(taskId);
			break;

		default: //Cancel
			NB_RebuildAndReturn(taskId);
			break;
	}
}

//---------------------------------------------------------------------------------------------------
// Main input task
//---------------------------------------------------------------------------------------------------
//Give / sell / PC-box give of TMs and berries goes through the TM CASE / BERRY POUCH screens, like the vanilla bag
static void NB_ReturnFromSubmenu(void)
{
	GoToBagMenu(NB_LOC_LAST, 3, NULL);
}

static u8 NB_SubmenuMode(void)
{
	switch (gBagMenuState.location) {
		case NB_LOC_PARTY: return 1;  //TMCASE_GIVE_PARTY / BERRYPOUCH_FROMPARTYGIVE
		case NB_LOC_SHOP: return 2;   //TMCASE_SELL / BERRYPOUCH_FROMMARTSELL
		default: return 3;            //TMCASE_GIVE_PC / BERRYPOUCH_FROMPOKEMONSTORAGEPC
	}
}

static void NB_OpenTMCase(void)
{
	InitTMCase(NB_SubmenuMode(), NB_ReturnFromSubmenu, FALSE);
}

static void NB_OpenBerryPouch(void)
{
	InitBerryPouch(NB_SubmenuMode(), NB_ReturnFromSubmenu, FALSE);
}

static void NB_OnSelect(u8 taskId, u16 item)
{
	struct NewBag* bag = gNB;

	if ((item == ITEM_TM_CASE || item == ITEM_BERRY_POUCH)
	&& (bag->loc == NB_LOC_PARTY || bag->loc == NB_LOC_SHOP || bag->loc == NB_LOC_PCBOX))
	{
		bag->exitCB = (item == ITEM_TM_CASE) ? NB_OpenTMCase : NB_OpenBerryPouch;
		ItemMenu_StartFadeToExitCallback(taskId);
		return;
	}

	switch (bag->loc) {
		case NB_LOC_PARTY:
		case NB_LOC_PCBOX:
			NB_GiveToMon(taskId, item);
			break;
		case NB_LOC_SHOP:
			NB_SellStart(taskId, item);
			break;
		case NB_LOC_ITEMPC:
			NB_DepositStart(taskId, item);
			break;
		default: //Field and battle
			NB_OpenMenu(taskId, item);
			break;
	}
}

static void Task_NB_InputMain(u8 taskId)
{
	struct NewBag* bag = gNB;
	s16* data = gTasks[taskId].data;

	if (gPaletteFade.active)
		return;

	if (bag->detailTimer != 0 && --bag->detailTimer == 0)
		NB_QueueDetail();

	//Input first, then a bounded slice of text rendering
	if (JOY_NEW(L_BUTTON) || JOY_NEW(DPAD_LEFT))
	{
		NB_SwitchTab(-1);
		return;
	}
	if (JOY_NEW(R_BUTTON) || JOY_NEW(DPAD_RIGHT))
	{
		NB_SwitchTab(1);
		return;
	}
	if (JOY_REPT(DPAD_UP))
	{
		NB_SetSelection(bag->sel - 1, TRUE);
		return;
	}
	if (JOY_REPT(DPAD_DOWN))
	{
		NB_SetSelection(bag->sel + 1, TRUE);
		return;
	}

	if (JOY_NEW(A_BUTTON))
	{
		PlaySE(SE_SELECT);
		if (bag->sel >= bag->count)
		{
			NB_Exit(taskId);
			return;
		}
		bag->detailTimer = 0;
		gSpecialVar_ItemId = bag->ids[bag->sel];
		data[1] = bag->sel;
		data[2] = bag->qtys[bag->sel];
		NB_OnSelect(taskId, gSpecialVar_ItemId);
	}
	else if (JOY_NEW(B_BUTTON))
	{
		PlaySE(SE_SELECT);
		NB_Exit(taskId);
	}
}

static void Task_NB_Input(u8 taskId)
{
	if (gPaletteFade.active)
		return;
	Task_NB_InputMain(taskId);
	if (gTasks[taskId].func == Task_NB_Input)
		NB_RunWork(18); //~1/2 frame of text rendering
}

//---------------------------------------------------------------------------------------------------
// Screen set up
//---------------------------------------------------------------------------------------------------
//Whatever screen opened the bag (overworld, battle, party menu...) can leave the blend / window / mosaic registers dirty.
//With alpha blending left on, the OBJ layer (backpack, tabs, icons) is mixed 50% with the background and looks washed out.
static void NB_ResetGpuEffects(void)
{
	SetGpuReg(REG_OFFSET_BLDCNT, 0);
	SetGpuReg(REG_OFFSET_BLDALPHA, 0);
	SetGpuReg(REG_OFFSET_BLDY, 0);
	SetGpuReg(REG_OFFSET_WININ, 0);
	SetGpuReg(REG_OFFSET_WINOUT, 0);
	SetGpuReg(REG_OFFSET_WIN0H, 0);
	SetGpuReg(REG_OFFSET_WIN0V, 0);
	SetGpuReg(REG_OFFSET_WIN1H, 0);
	SetGpuReg(REG_OFFSET_WIN1V, 0);
	SetGpuReg(REG_OFFSET_MOSAIC, 0);
}

static void VBlankCB_NewBag(void)
{
	struct NewBag* bag = gNB;

	LoadOam();
	ProcessSpriteCopyRequests();
	TransferPlttBuffer();

	if (NB_Active() && bag->dirty)
	{
		u32 i;
		for (i = 0; i <= NB_WIN_POPUP; ++i)
			if (bag->dirty & (1 << i))
				NB_CopyWindowGfx(i);
		bag->dirty = 0;
	}
}

static void CB2_NewBagRun(void)
{
	RunTasks();
	AnimateSprites();
	BuildOamBuffer();
	DoScheduledBgTilemapCopiesToVram();
	UpdatePaletteFade();
}

static void NB_LoadSprites(void)
{
	struct NewBag* bag = gNB;
	u32 i, j;
	u8 gender = gSaveBlock2->playerGender ? 1 : 0;
	struct SpritePalette pal;

	//Selection pieces share one palette
	pal.data = (const u16*) NB_SelMPal;
	pal.tag = NB_TAG_SEL_PAL;
	LoadSpritePalette(&pal);
	LoadCompressedSpriteSheet(&sSelSheetL);
	LoadCompressedSpriteSheet(&sSelSheetM);
	LoadCompressedSpriteSheet(&sSelSheetR);
	LoadCompressedSpriteSheet(&sTabSelSheet);
	LoadCompressedSpriteSheet(&sThumbSheet);

	bag->tabSelSprite = CreateSprite(&sTabSelTemplate, 21, 12, 5);
	for (i = 0; i < NB_TABS; ++i)
	{
		u16 dim[16];
		const u16* src = (const u16*) sTabPals[i];

		LoadCompressedSpriteSheet(&sTabSheets[i]);
		pal.data = src;
		pal.tag = NB_TAG_TAB_PAL + i;
		LoadSpritePalette(&pal);

		//Dimmed (unselected) version: blend every colour 50% with grey
		for (j = 0; j < 16; ++j)
		{
			u16 c = src[j];
			u16 r = ((c & 31) + 22) / 2;
			u16 g = (((c >> 5) & 31) + 22) / 2;
			u16 b = (((c >> 10) & 31) + 24) / 2;
			dim[j] = r | (g << 5) | (b << 10);
		}
		pal.data = dim;
		pal.tag = NB_TAG_TAB_DIM + i;
		LoadSpritePalette(&pal);

		bag->tabSprites[i] = CreateSprite(&sTabTemplates[i], 21 + i * 38, 12, 1);
	}

	LoadCompressedSpriteSheet(&sBagSheets[gender]);
	pal.data = (const u16*) (gender ? NB_BagGirlPal : NB_BagBoyPal);
	pal.tag = NB_TAG_BAG_PAL;
	LoadSpritePalette(&pal);
	bag->bagSprite = CreateSprite(&sBagTemplate, 36, 72, 2);
	StartSpriteAnim(&gSprites[bag->bagSprite], bag->tab);

	//Selection bar: left cap, 3 middle pieces, right cap (covers the 160px wide row card)
	bag->selSprites[0] = CreateSprite(&sSelLTemplate, 90, 40, 3);
	bag->selSprites[1] = CreateSprite(&sSelMTemplate, 122, 40, 3);
	bag->selSprites[2] = CreateSprite(&sSelMTemplate, 154, 40, 3);
	bag->selSprites[3] = CreateSprite(&sSelMTemplate, 186, 40, 3);
	bag->selSprites[4] = CreateSprite(&sSelRTemplate, 218, 40, 3);
	bag->thumbSprite = CreateSprite(&sThumbTemplate, 238, 41, 3);
	bag->itemIconSprite = 0xFF;
}

//Vanilla exit pipeline needs a list-menu task id in data[0]; it is never used for input
static void NB_CreateDummyList(u8 taskId)
{
	struct ListMenuTemplate t;
	static const struct ListMenuItem sDummyItem = {NULL, 0};
	u8 emptyName[] = {0xFF};

	if (sBagListMenuItems != NULL)
	{
		Free(sBagListMenuItems);
		sBagListMenuItems = NULL;
	}
	sBagListMenuItems = Malloc(sizeof(struct ListMenuItem));
	sBagListMenuItems[0].name = sText_Space + 1; //Empty string (EOS)
	sBagListMenuItems[0].id = 0;
	(void) sDummyItem;
	(void) emptyName;

	t.items = sBagListMenuItems;
	t.moveCursorFunc = NULL;
	t.itemPrintFunc = NULL;
	t.totalItems = 1;
	t.maxShowed = 1;
	t.windowId = NB_WIN_LIST;
	t.header_X = 0;
	t.item_X = 0;
	t.cursor_X = 0;
	t.upText_Y = 0;
	t.cursorPal = 2;
	t.fillValue = 0;
	t.cursorShadowPal = 3;
	t.lettersSpacing = 0;
	t.itemVerticalPadding = 0;
	t.scrollMultiple = LIST_NO_MULTIPLE_SCROLL;
	t.fontId = 2;
	t.cursorKind = 1; //No cursor
	gTasks[taskId].data[0] = ListMenuInit(&t, 0, 0);
}

static void NB_FinishSetup(void)
{
	struct NewBag* bag = gNB;

	u8 taskId;
	u8 tab = gNBLastTab < NB_TABS ? gNBLastTab : 0;

	bag->tab = tab;
	bag->sel = gBagMenuState.cursorPos[1];
	bag->top = gBagMenuState.itemsAbove[1];
	//Window pixel buffers are not cleared by the game: start from fully transparent so no garbage shows through
	FillWindowPixelBuffer(NB_WIN_TABS, PIXEL_FILL(0));
	FillWindowPixelBuffer(NB_WIN_ROWS, PIXEL_FILL(0));
	FillWindowPixelBuffer(NB_WIN_DETAIL, PIXEL_FILL(0));
	FillWindowPixelBuffer(NB_WIN_MONEY, PIXEL_FILL(0));
	FillWindowPixelBuffer(NB_WIN_POPUP, PIXEL_FILL(0));
	NB_ShowWindow(NB_WIN_TABS);
	NB_ShowWindow(NB_WIN_ROWS);
	NB_ShowWindow(NB_WIN_DETAIL);
	NB_LoadSprites();
	NB_UpdateTabVisuals();
	{ u8 i; for (i = 0; i < NB_TABS; ++i) NB_QueueLabel(i); }
	NB_BuildEntries(tab);

	taskId = CreateTask(Task_NB_Input, 0);
	NB_CreateDummyList(taskId);
	NB_RefreshAll();
	NB_FlushWork(); //First frame is complete before the fade-in starts
	NB_DrawMoney();
}

static void CB2_NewBag(void)
{
	switch (gMain.state) {
		case 0:
		default:
			SetVBlankCallback(NULL);
			DmaFill16(3, 0, VRAM, VRAM_SIZE);
			DmaFill32(3, 0, OAM, OAM_SIZE);
			DmaFill16(3, 0, PLTT, PLTT_SIZE);
			++gMain.state;
			break;

		case 1:
			ScanlineEffect_Stop();
			ResetTasks();
			ResetSpriteData();
			ResetTempTileDataBuffers();
			ResetPaletteFade();
			FreeAllSpritePalettes();
			++gMain.state;
			break;

		case 2:
			sBagBgTilemapBuffer = Calloc(0x800);
			ResetBgsAndClearDma3BusyFlags(0);
			InitBgsFromTemplates(0, sNBBgTemplates, 3);
			SetBgTilemapBuffer(3, sBagBgTilemapBuffer);
			ChangeBgX(0, 0, 0);
			ChangeBgY(0, 0, 0);
			ChangeBgX(3, 0, 0);
			ChangeBgY(3, 0, 0);
			SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
			NB_ResetGpuEffects();
			++gMain.state;
			break;

		case 3:
			CpuFastFill16(0, (void*) BG_CHAR_ADDR(0), BG_CHAR_SIZE * 4);
			DecompressAndCopyTileDataToVram(3, NB_BGTiles, 0, 0, 0);
			LZDecompressWram(NB_BGMap, sBagBgTilemapBuffer);
			LoadPalette(NB_BGPal, 0, 0x40); //32 colours: palette 0 + palette 1
			LoadPalette(sNBTextPal, 0xF0, 0x20);
			++gMain.state;
			break;

		case 4:
			if (!free_temp_tile_data_buffers_if_possible())
			{
				ShowBg(0);
				ShowBg(3);
				CopyBgTilemapBufferToVram(3);
				++gMain.state;
			}
			break;

		case 5:
			InitWindows(sNBWindows);
			DeactivateAllTextPrinters();
			++gMain.state;
			break;

		case 6:
			NB_FinishSetup();
			BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
			SetVBlankCallback(VBlankCB_NewBag);
			SetMainCallback2(CB2_NewBagRun);
			gMain.state = 0;
			break;
	}
}

//---------------------------------------------------------------------------------------------------
// Entry point: hook for vanilla GoToBagMenu(location, pocket, bagCallback)
//---------------------------------------------------------------------------------------------------
//Runs the ORIGINAL instructions the hook overwrote, then continues in the vanilla function. Used for the
//bag screens this file does not replace (Old Man / Teachy TV tutorials, script "select item", ...).
__attribute__((naked)) static void NB_Vanilla_GoToBagMenu(u8 location, u8 pocket, MainCallback bagCallback)
{
	(void) location; (void) pocket; (void) bagCallback;
	asm(".hword 0xB5F0, 0x4647, 0xB480, 0x1C14, 0x0600, 0x0E06, 0x1C37, 0x0609\n"
	    "push {r7}\n"
	    "ldr r7, =0x08107DC5\n"
	    "mov ip, r7\n"
	    "pop {r7}\n"
	    "bx ip\n"
	    ".ltorg\n");
}

void NewBag_GoToBagMenu(u8 location, u8 pocket, MainCallback bagCallback)
{
	struct NewBag* bag;
	u8 loc = (location == NB_LOC_LAST) ? gBagMenuState.location : location;

	#ifdef NEWBAG_TESTLOC
	if (location == NB_LOC_FIELD)
		loc = location = NEWBAG_TESTLOC;
	#endif

	#ifdef NEWBAG_FORCE_VANILLA
	loc = 99;
	#endif

	if (loc > NB_LOC_BATTLE) //Tutorials, TTV, select-item...: leave to the vanilla code
	{
		NB_Vanilla_GoToBagMenu(location, pocket, bagCallback);
		return;
	}

	sBagListMenuItems = NULL;
	sBagListMenuItemStrings = NULL;
	sBagBgTilemapBuffer = NULL;

	bag = Calloc(sizeof(struct NewBag));
	if (bag == NULL)
	{
		SetMainCallback2(bagCallback);
		return;
	}

	if (location != NB_LOC_LAST)
		gBagMenuState.location = location;
	if (bagCallback != NULL)
		gBagMenuState.bagCallback = bagCallback;
	gBagMenuState.pocket = 0; //Keep the vanilla index valid for the vanilla exit code
	(void) pocket;

	gNB = bag;
	bag->itemOriginalLocation = NB_MARK_LOC;
	bag->contextMenuSelectedItem = NB_MARK_ID;
	bag->pocketScrollArrowsTask = 0xFF;
	bag->pocketSwitchArrowsTask = 0xFF;
	bag->itemIconSprite = 0xFF;
	bag->exitCB = NULL;
	bag->loc = loc;

	gTextFlags.autoScroll = FALSE;
	gSpecialVar_ItemId = ITEM_NONE;


	#ifdef NEWBAG_ALLITEMS
	{
		u32 i;
		for (i = 1; i < ITEMS_COUNT; ++i)
			if (NB_POCKET(i) >= POCKET_ITEMS && NB_POCKET(i) <= POCKET_BERRY_POUCH)
				AddBagItem(i, 1);
	}
	#endif

	#ifdef NEWBAG_TEST
	{
		extern NB_FAR u8 GiveMonToPlayer(struct Pokemon* mon);
		static const u16 testItems[] = {ITEM_POTION, ITEM_ETHER, ITEM_PP_UP, ITEM_RARE_CANDY, ITEM_REPEL, ITEM_ESCAPE_ROPE, ITEM_SACRED_ASH,
			ITEM_LONELY_MINT, ITEM_ORANGE_MAIL, ITEM_GRACIDEA, ITEM_HONEY, ITEM_EXP_SHARE, ITEM_BLACK_FLUTE, ITEM_PROTEIN, ITEM_POKE_DOLL,
			ITEM_POMEG_BERRY, ITEM_ORAN_BERRY,
			ITEM_POKE_BALL, ITEM_GREAT_BALL, ITEM_TM01, ITEM_TM01 + 1,
			ITEM_BICYCLE, ITEM_OLD_ROD, ITEM_ITEMFINDER, ITEM_COIN_CASE, ITEM_TOWN_MAP, ITEM_POKE_FLUTE, ITEM_VS_SEEKER, ITEM_FAME_CHECKER,
			ITEM_TEACHY_TV, ITEM_POWDER_JAR, ITEM_TM_CASE, ITEM_BERRY_POUCH, ITEM_EVIV_DISPLAYER, ITEM_PORTABLE_PC, ITEM_POKEVIAL,
			ITEM_CHARIZARDITE_X, ITEM_BLASTOISINITE, ITEM_FIRE_STONE, ITEM_MOON_STONE, ITEM_KINGS_ROCK};
		u32 i;
		for (i = 0; i < NELEMS(testItems); ++i)
			AddBagItem(testItems[i], 5 + i);
		if (CalculatePlayerPartyCount() == 0)
		{
			struct Pokemon mon;
			CreateMon(&mon, SPECIES_BULBASAUR, 20, 32, 0, 0, 0, 0);
			GiveMonToPlayer(&mon);
		}
	}
	#endif
	gMain.state = 0;
	SetMainCallback2(CB2_NewBag);
}

//Hooks for the vanilla message / return helpers used by item-use code (item_use.c).
//When the new bag is not the active screen, execution continues in the ORIGINAL vanilla code.
__attribute__((naked)) static void NB_Vanilla_DisplayItemMessageInBag(u8 taskId, u8 fontId, const u8* str, void (*followUp)(u8))
{
	(void) taskId; (void) fontId; (void) str; (void) followUp;
	asm(".hword 0xB570, 0x464E, 0x4645, 0xB460, 0xB084, 0x1C05, 0x1C0E, 0x4690\n"
	    "push {r7}\n"
	    "ldr r7, =0x08108E81\n"
	    "mov ip, r7\n"
	    "pop {r7}\n"
	    "bx ip\n"
	    ".ltorg\n");
}

__attribute__((naked)) static void NB_Vanilla_ReturnToBagFromContextMenu(u8 taskId)
{
	(void) taskId;
	asm(".hword 0xB570, 0x464E, 0x4645, 0xB460\n"
	    "push {r7}\n"
	    "ldr r7, =0x0810A201\n"
	    "mov ip, r7\n"
	    "pop {r7}\n"
	    "bx ip\n"
	    ".ltorg\n");
}

void NewBag_DisplayItemMessageInBag(u8 taskId, u8 fontId, const u8* str, void (*followUp)(u8))
{
	if (NB_Active())
		NB_ShowMessage(taskId, str, followUp);
	else
		NB_Vanilla_DisplayItemMessageInBag(taskId, fontId, str, followUp);
}

void NewBag_ReturnToBagFromContextMenu(u8 taskId)
{
	if (NB_Active())
		NB_RebuildAndReturn(taskId);
	else
		NB_Vanilla_ReturnToBagFromContextMenu(taskId);
}

//Using a berry makes the vanilla item-use code call the BERRY POUCH screen's own exit helpers (it assumes the pouch
//is open). When the new bag is the active screen they are redirected to the bag's equivalents.
__attribute__((naked)) static void NB_Vanilla_BerryPouchStartFade(u8 taskId)
{
	(void) taskId;
	asm(".hword 0xB510, 0xB081, 0x1C04, 0x0624\n"
	    "push {r7}\n"
	    "ldr r7, =0x0813D811\n"
	    "mov ip, r7\n"
	    "pop {r7}\n"
	    "bx ip\n"
	    ".ltorg\n");
}

void NewBag_BerryPouchSetExitCallback(void (*cb)(void))
{
	if (NB_Active())
		gNB->exitCB = cb;
	else //Vanilla body: sBerryPouchStaticResources->exitCallback = cb
		**(void (***)(void)) 0x203F36C = cb;
}

void NewBag_BerryPouchStartFadeToExitCallback(u8 taskId)
{
	if (NB_Active())
		ItemMenu_StartFadeToExitCallback(taskId);
	else
		NB_Vanilla_BerryPouchStartFade(taskId);
}

//After a TM / berry is used on or given to a Pokemon, the vanilla party menu returns to the TM CASE / BERRY POUCH screen
//(it assumes the item was picked there, and those screens have no exit callback otherwise: black screen). TMs and berries
//are listed directly in the new bag, so both returns go back to the bag instead.
void NewBag_ReturnToTMCaseMenu(void)
{
	GoToBagMenu(NB_LOC_LAST, 3, NULL);
}

void NewBag_ReturnToBerryPouchMenu(void)
{
	GoToBagMenu(NB_LOC_LAST, 3, NULL);
}

#endif
