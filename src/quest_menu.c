#include "../include/global.h"
#include "../include/script.h"
#include "../include/mystery_event_script.h"
#include "../include/event_data.h"
#include "../include/random.h"
#include "../include/item.h"
#include "../include/overworld.h"
#include "../include/field_screen_effect.h"
#include "../include/quest_log.h"
#include "../include/map_preview_screen.h"
#include "../include/field_weather.h"
#include "../include/field_tasks.h"
#include "../include/field_fadetransition.h"
#include "../include/field_player_avatar.h"
#include "../include/script_movement.h"
#include "../include/event_object_movement.h"
#include "../include/field_message_box.h"
#include "../include/new_menu_helpers.h"
#include "../include/script_menu.h"
#include "../include/field_specials.h"
#include "../include/constants/items.h"
#include "../include/pokemon_storage_system.h"
#include "../include/party_menu.h"
#include "../include/money.h"
#include "../include/coins.h"
#include "../include/battle_setup.h"
#include "../include/shop.h"
#include "../include/slot_machine.h"
#include "../include/field_effect.h"
#include "../include/fieldmap.h"
#include "../include/field_door.h"
#include "../include/item_pc.h"
#include "../include/quest_menu.h"
#include "../include/constants/event_objects.h"
#include "../include/menu.h"
#include "../include/palette.h"
#include "../include/event_data.h"
#include "../include/new/item.h"
#include "../include/list_menu.h"
#include "../include/item_icon.h"
#include "../include/constants/songs.h"
#include "../include/constants/moves.h"
#include "../include/string_util.h"
#include "../include/new/Vanilla_functions.h"

////side quests
//names
extern const u8 gText_SideQuestName_1[];
extern const u8 gText_SideQuestName_2[];
extern const u8 gText_SideQuestName_3[];
extern const u8 gText_SideQuestName_4[];
extern const u8 gText_SideQuestName_5[];
extern const u8 gText_SideQuestName_6[];
extern const u8 gText_SideQuestName_7[];
extern const u8 gText_SideQuestName_8[];
extern const u8 gText_SideQuestName_9[];
extern const u8 gText_SideQuestName_10[];
extern const u8 gText_SideQuestName_11[];
extern const u8 gText_SideQuestName_12[];
extern const u8 gText_SideQuestName_13[];
extern const u8 gText_SideQuestName_14[];
extern const u8 gText_SideQuestName_15[];
extern const u8 gText_SideQuestName_16[];
extern const u8 gText_SideQuestName_17[];
extern const u8 gText_SideQuestName_18[];
extern const u8 gText_SideQuestName_19[];
extern const u8 gText_SideQuestName_20[];
extern const u8 gText_SideQuestName_21[];
extern const u8 gText_SideQuestName_22[];
extern const u8 gText_SideQuestName_23[];
extern const u8 gText_SideQuestName_24[];
extern const u8 gText_SideQuestName_25[];
extern const u8 gText_SideQuestName_26[];
extern const u8 gText_SideQuestName_27[];
extern const u8 gText_SideQuestName_28[];
extern const u8 gText_SideQuestName_29[];
extern const u8 gText_SideQuestName_30[];
//descriptions
extern const u8 gText_SideQuestDesc_1[];
extern const u8 gText_SideQuestDesc_2[];
extern const u8 gText_SideQuestDesc_3[];
extern const u8 gText_SideQuestDesc_4[];
extern const u8 gText_SideQuestDesc_5[];
extern const u8 gText_SideQuestDesc_6[];
extern const u8 gText_SideQuestDesc_7[];
extern const u8 gText_SideQuestDesc_8[];
extern const u8 gText_SideQuestDesc_9[];
extern const u8 gText_SideQuestDesc_10[];
extern const u8 gText_SideQuestDesc_11[];
extern const u8 gText_SideQuestDesc_12[];
extern const u8 gText_SideQuestDesc_13[];
extern const u8 gText_SideQuestDesc_14[];
extern const u8 gText_SideQuestDesc_15[];
extern const u8 gText_SideQuestDesc_16[];
extern const u8 gText_SideQuestDesc_17[];
extern const u8 gText_SideQuestDesc_18[];
extern const u8 gText_SideQuestDesc_19[];
extern const u8 gText_SideQuestDesc_20[];
extern const u8 gText_SideQuestDesc_21[];
extern const u8 gText_SideQuestDesc_22[];
extern const u8 gText_SideQuestDesc_23[];
extern const u8 gText_SideQuestDesc_24[];
extern const u8 gText_SideQuestDesc_25[];
extern const u8 gText_SideQuestDesc_26[];
extern const u8 gText_SideQuestDesc_27[];
extern const u8 gText_SideQuestDesc_28[];
extern const u8 gText_SideQuestDesc_29[];
extern const u8 gText_SideQuestDesc_30[];
//point of contact
extern const u8 gText_SideQuestPOC_1[];
extern const u8 gText_SideQuestPOC_2[];
extern const u8 gText_SideQuestPOC_3[];
extern const u8 gText_SideQuestPOC_4[];
extern const u8 gText_SideQuestPOC_5[];
extern const u8 gText_SideQuestPOC_6[];
extern const u8 gText_SideQuestPOC_7[];
extern const u8 gText_SideQuestPOC_8[];
extern const u8 gText_SideQuestPOC_9[];
extern const u8 gText_SideQuestPOC_10[];
extern const u8 gText_SideQuestPOC_11[];
extern const u8 gText_SideQuestPOC_12[];
extern const u8 gText_SideQuestPOC_13[];
extern const u8 gText_SideQuestPOC_14[];
extern const u8 gText_SideQuestPOC_15[];
extern const u8 gText_SideQuestPOC_16[];
extern const u8 gText_SideQuestPOC_17[];
extern const u8 gText_SideQuestPOC_18[];
extern const u8 gText_SideQuestPOC_19[];
extern const u8 gText_SideQuestPOC_20[];
extern const u8 gText_SideQuestPOC_21[];
extern const u8 gText_SideQuestPOC_22[];
extern const u8 gText_SideQuestPOC_23[];
extern const u8 gText_SideQuestPOC_24[];
extern const u8 gText_SideQuestPOC_25[];
extern const u8 gText_SideQuestPOC_26[];
extern const u8 gText_SideQuestPOC_27[];
extern const u8 gText_SideQuestPOC_28[];
extern const u8 gText_SideQuestPOC_29[];
extern const u8 gText_SideQuestPOC_30[];
//map
extern const u8 gText_SideQuestMap_1[];
extern const u8 gText_SideQuestMap_2[];
extern const u8 gText_SideQuestMap_3[];
extern const u8 gText_SideQuestMap_4[];
extern const u8 gText_SideQuestMap_5[];
extern const u8 gText_SideQuestMap_6[];
extern const u8 gText_SideQuestMap_7[];
extern const u8 gText_SideQuestMap_8[];
extern const u8 gText_SideQuestMap_9[];
extern const u8 gText_SideQuestMap_10[];
extern const u8 gText_SideQuestMap_11[];
extern const u8 gText_SideQuestMap_12[];
extern const u8 gText_SideQuestMap_13[];
extern const u8 gText_SideQuestMap_14[];
extern const u8 gText_SideQuestMap_15[];
extern const u8 gText_SideQuestMap_16[];
extern const u8 gText_SideQuestMap_17[];
extern const u8 gText_SideQuestMap_18[];
extern const u8 gText_SideQuestMap_19[];
extern const u8 gText_SideQuestMap_20[];
extern const u8 gText_SideQuestMap_21[];
extern const u8 gText_SideQuestMap_22[];
extern const u8 gText_SideQuestMap_23[];
extern const u8 gText_SideQuestMap_24[];
extern const u8 gText_SideQuestMap_25[];
extern const u8 gText_SideQuestMap_26[];
extern const u8 gText_SideQuestMap_27[];
extern const u8 gText_SideQuestMap_28[];
extern const u8 gText_SideQuestMap_29[];
extern const u8 gText_SideQuestMap_30[];
//rewards
extern const u8 gText_SideQuestReward_1[];
extern const u8 gText_SideQuestReward_2[];
extern const u8 gText_SideQuestReward_3[];
extern const u8 gText_SideQuestReward_4[];
extern const u8 gText_SideQuestReward_5[];
extern const u8 gText_SideQuestReward_6[];
extern const u8 gText_SideQuestReward_7[];
extern const u8 gText_SideQuestReward_8[];
extern const u8 gText_SideQuestReward_9[];
extern const u8 gText_SideQuestReward_10[];
extern const u8 gText_SideQuestReward_11[];
extern const u8 gText_SideQuestReward_12[];
extern const u8 gText_SideQuestReward_13[];
extern const u8 gText_SideQuestReward_14[];
extern const u8 gText_SideQuestReward_15[];
extern const u8 gText_SideQuestReward_16[];
extern const u8 gText_SideQuestReward_17[];
extern const u8 gText_SideQuestReward_18[];
extern const u8 gText_SideQuestReward_19[];
extern const u8 gText_SideQuestReward_20[];
extern const u8 gText_SideQuestReward_21[];
extern const u8 gText_SideQuestReward_22[];
extern const u8 gText_SideQuestReward_23[];
extern const u8 gText_SideQuestReward_24[];
extern const u8 gText_SideQuestReward_25[];
extern const u8 gText_SideQuestReward_26[];
extern const u8 gText_SideQuestReward_27[];
extern const u8 gText_SideQuestReward_28[];
extern const u8 gText_SideQuestReward_29[];
extern const u8 gText_SideQuestReward_30[];
extern const u8 sText_Quests[];
extern const u8 sText_QuestMenu_Begin[];
extern const u8 sText_QuestMenu_End[];
extern const u8 sText_QuestMenu_Details[];
extern const u8 sText_QuestMenu_Reward[];
extern const u8 sText_Cancel[];
extern const u8 sText_QuestMenu_Unk[];
extern const u8 sText_QuestMenu_Active[];
extern const u8 sText_QuestMenu_Complete[];
extern const u8 sText_QuestMenu_Exit[];
extern const u8 sText_QuestMenu_SelectedQuest[];
extern const u8 sText_QuestMenu_DisplayDetails[];
extern const u8 sText_QuestMenu_DisplayReward[];
extern const u8 sText_QuestMenu_BeginQuest[];
extern const u8 sText_QuestMenu_EndQuest[];
extern const u8 gFameCheckerText_Cancel[];
extern const u8 gText_ReturnToPC[];
extern u8 gExpandedPlaceholder_Empty[];
extern const u8 gText_TimesStrVar1[];
extern const u8 gText_WithdrawItem[];
extern const u8 gText_Var1IsSelected[];

struct ItemPcResources
{
    MainCallback savedCallback;
    u8 moveModeOrigPos;
    u8 itemMenuIconSlot;
    u8 maxShowed;
    u8 nItems;
    u8 scrollIndicatorArrowPairId;
    u16 withdrawQuantitySubmenuCursorPos;
    s16 data[3];
};
struct ListMenuItems2
{
    const u8 *label;
    u32 index;
};

struct ItemPcStaticResources
{
    MainCallback savedCallback;
    u16 scroll;
    u16 row;
    u8 initialized;
};

#define sStateDataPtr (*(struct ItemPcResources **)0x203ADBC)
#define sListMenuItems (*(struct ListMenuItems2 **)0x203ADC4)
#define LIST_NOTHING_CHOSEN -1
#define LIST_CANCEL -2
#define LIST_HEADER -3
#define gMultiuseListMenuTemplate ((struct ListMenuTemplate*) ((u32*) 0x3005E70))
#define sListMenuState (*(struct ItemPcStaticResources *)0x0203ADCC)
#define ITEM_N_A 375
#define SE_HAZURE 26
#define sItemPcSubmenuOptions ((const struct MenuAction *)0x08453F74)


//QUEST MENU
void DebugSideQuestMenu(void);
static void SetQuestMenuInactive(void);
static bool8 IsQuestMenuActive(void);
static bool8 IsActiveQuest(u8 questId);
static void Task_QuestMenuBeginQuest(u8 taskId);
static void Task_QuestMenuEndQuest(u8 taskId);
static void Task_QuestMenuDetails(u8 taskId);
static void Task_QuestMenuCancel(u8 taskId);
static void Task_QuestMenuReward(u8 taskId);
void ItemPc_MoveCursorFunc(s32 itemIndex, bool8 onInit, struct ListMenu * list);
void ItemPc_ItemPrintFunc(u8 windowId, s32 itemId, u8 y);
extern void PlaySE(u16 song);
u16 ItemPc_GetItemIdBySlotId(u16 itemIndex);
extern const u8 gMoveNames[][MOVE_NAME_LENGTH + 1];
extern bool8 IsPCScreenEffectRunning_TurnOff(void);
extern void ItemPc_RemoveScrollIndicatorArrowPair(void);
extern void ItemPc_FreeResources(void);
extern void ItemPcCompaction(void);
extern void Task_ItemPcTurnOff1(u8 taskId);
extern void ItemPc_PrintOrRemoveCursorAt(u8 y, u8 mode);
void Task_ItemPcSubmenuInit(u8 taskId);
extern void GetOrCreateSubmenuWindow(void);
extern void PrintTextArray(u8 windowId, u8 fontId, u8 left, u8 top, u8 lineHeight, u8 itemCount, const struct MenuAction *strs);
extern void Task_ItemPcSubmenuRun(u8 taskId);
extern void ItemPc_SetCursorPosition(void);
extern void ItemPc_AddTextPrinterParameterized(u8 windowId, u8 fontId, const u8 * str, u8 x, u8 y, u8 letterSpacing, u8 lineSpacing, u8 speed, u8 colorIdx);
extern void ItemPc_PrintOrRemoveCursor(s32 listTaskId, u8 mode);
extern void ItemPc_SetMessageWindowPalette(u8 paletteId);
extern void ItemPc_MoveItemModeInit(u8 taskId, s16 itemIndex);
extern void ItemPc_SetBorderStyleOnWindow(u8 windowId);
extern void ItemPc_DestroySubwindow(u8 id);
extern void ItemPc_ReturnFromSubmenu(u8 taskId);
extern void Task_ItemPcCancel(u8 taskId);
extern u16 ItemPc_GetItemIdBySlotId(u16 slotId);
extern u16 ItemPc_GetItemQuantityBySlotId(u16 slotId);
extern bool8 IsPCScreenEffectRunning_TurnOn(void);
extern u8 ItemPc_GetCursorPosition(void);
extern u8 ItemPc_GetOrCreateSubwindow(u8 id);
extern s32 Menu_ProcessInputNoWrapAround(void);


//menu actions
// Selected an incomplete quest
static const struct MenuAction sQuestSubmenuOptions[] =
{
    {sText_QuestMenu_Begin,             {.void_u8 = Task_QuestMenuBeginQuest}},
    {sText_QuestMenu_Details,           {.void_u8 = Task_QuestMenuDetails}},
    {sText_Cancel,                      {.void_u8 = Task_QuestMenuCancel}},
};

// Selected the active quest
static const struct MenuAction sActiveQuestSubmenuOptions[] =
{
    {sText_QuestMenu_End,               {.void_u8 = Task_QuestMenuEndQuest}},
    {sText_QuestMenu_Details,           {.void_u8 = Task_QuestMenuDetails}},
    {sText_Cancel,                      {.void_u8 = Task_QuestMenuCancel}},
};

// completed quest selection
static const struct MenuAction sCompletedQuestSubmenuOptions[] =
{
    {sText_QuestMenu_Reward,            {.void_u8 = Task_QuestMenuReward}},
    {sText_QuestMenu_Details,           {.void_u8 = Task_QuestMenuDetails}},
    {sText_Cancel,                      {.void_u8 = Task_QuestMenuCancel}},
};

#define side_quest(n, d, p, m, r) {.name = n, .desc = d, .poc = p, .map = m, .reward = r}
static const struct SideQuest sSideQuests[SIDE_QUEST_COUNT] =
{
    side_quest(gText_SideQuestName_1,  gText_SideQuestDesc_1,  gText_SideQuestPOC_1,  gText_SideQuestMap_1,  gText_SideQuestReward_1),
    side_quest(gText_SideQuestName_2,  gText_SideQuestDesc_2,  gText_SideQuestPOC_2,  gText_SideQuestMap_2,  gText_SideQuestReward_2),
    side_quest(gText_SideQuestName_3,  gText_SideQuestDesc_3,  gText_SideQuestPOC_3,  gText_SideQuestMap_3,  gText_SideQuestReward_3),
    side_quest(gText_SideQuestName_4,  gText_SideQuestDesc_4,  gText_SideQuestPOC_4,  gText_SideQuestMap_4,  gText_SideQuestReward_4),
    side_quest(gText_SideQuestName_5,  gText_SideQuestDesc_5,  gText_SideQuestPOC_5,  gText_SideQuestMap_5,  gText_SideQuestReward_5),
    side_quest(gText_SideQuestName_6,  gText_SideQuestDesc_6,  gText_SideQuestPOC_6,  gText_SideQuestMap_6,  gText_SideQuestReward_6),
    side_quest(gText_SideQuestName_7,  gText_SideQuestDesc_7,  gText_SideQuestPOC_7,  gText_SideQuestMap_7,  gText_SideQuestReward_7),
    side_quest(gText_SideQuestName_8,  gText_SideQuestDesc_8,  gText_SideQuestPOC_8,  gText_SideQuestMap_8,  gText_SideQuestReward_8),
    side_quest(gText_SideQuestName_9,  gText_SideQuestDesc_9,  gText_SideQuestPOC_9,  gText_SideQuestMap_9,  gText_SideQuestReward_9),
    side_quest(gText_SideQuestName_10, gText_SideQuestDesc_10, gText_SideQuestPOC_10, gText_SideQuestMap_10, gText_SideQuestReward_10),
    side_quest(gText_SideQuestName_11, gText_SideQuestDesc_11, gText_SideQuestPOC_11, gText_SideQuestMap_11, gText_SideQuestReward_11),
    side_quest(gText_SideQuestName_12, gText_SideQuestDesc_12, gText_SideQuestPOC_12, gText_SideQuestMap_12, gText_SideQuestReward_12),
    side_quest(gText_SideQuestName_13, gText_SideQuestDesc_13, gText_SideQuestPOC_13, gText_SideQuestMap_13, gText_SideQuestReward_13),
    side_quest(gText_SideQuestName_14, gText_SideQuestDesc_14, gText_SideQuestPOC_14, gText_SideQuestMap_14, gText_SideQuestReward_14),
    side_quest(gText_SideQuestName_15, gText_SideQuestDesc_15, gText_SideQuestPOC_15, gText_SideQuestMap_15, gText_SideQuestReward_15),
    side_quest(gText_SideQuestName_16, gText_SideQuestDesc_16, gText_SideQuestPOC_16, gText_SideQuestMap_16, gText_SideQuestReward_16),
    side_quest(gText_SideQuestName_17, gText_SideQuestDesc_17, gText_SideQuestPOC_17, gText_SideQuestMap_17, gText_SideQuestReward_17),
    side_quest(gText_SideQuestName_18, gText_SideQuestDesc_18, gText_SideQuestPOC_18, gText_SideQuestMap_18, gText_SideQuestReward_18),
    side_quest(gText_SideQuestName_19, gText_SideQuestDesc_19, gText_SideQuestPOC_19, gText_SideQuestMap_19, gText_SideQuestReward_19),
    side_quest(gText_SideQuestName_20, gText_SideQuestDesc_20, gText_SideQuestPOC_20, gText_SideQuestMap_20, gText_SideQuestReward_20),
    side_quest(gText_SideQuestName_21, gText_SideQuestDesc_21, gText_SideQuestPOC_21, gText_SideQuestMap_21, gText_SideQuestReward_21),
    side_quest(gText_SideQuestName_22, gText_SideQuestDesc_22, gText_SideQuestPOC_22, gText_SideQuestMap_22, gText_SideQuestReward_22),
    side_quest(gText_SideQuestName_23, gText_SideQuestDesc_23, gText_SideQuestPOC_23, gText_SideQuestMap_23, gText_SideQuestReward_23),
    side_quest(gText_SideQuestName_24, gText_SideQuestDesc_24, gText_SideQuestPOC_24, gText_SideQuestMap_24, gText_SideQuestReward_24),
    side_quest(gText_SideQuestName_25, gText_SideQuestDesc_25, gText_SideQuestPOC_25, gText_SideQuestMap_25, gText_SideQuestReward_25),
    side_quest(gText_SideQuestName_26, gText_SideQuestDesc_26, gText_SideQuestPOC_26, gText_SideQuestMap_26, gText_SideQuestReward_26),
    side_quest(gText_SideQuestName_27, gText_SideQuestDesc_27, gText_SideQuestPOC_27, gText_SideQuestMap_27, gText_SideQuestReward_27),
    side_quest(gText_SideQuestName_28, gText_SideQuestDesc_28, gText_SideQuestPOC_28, gText_SideQuestMap_28, gText_SideQuestReward_28),
    side_quest(gText_SideQuestName_29, gText_SideQuestDesc_29, gText_SideQuestPOC_29, gText_SideQuestMap_29, gText_SideQuestReward_29),
    side_quest(gText_SideQuestName_30, gText_SideQuestDesc_30, gText_SideQuestPOC_30, gText_SideQuestMap_30, gText_SideQuestReward_30),
};

// the item IDs that correspond to the quest's difficulty
static const u16 sSideQuestDifficultyItemIds[] = 
{
	ITEM_POKE_BALL,
	ITEM_GREAT_BALL,
	ITEM_ULTRA_BALL,
	ITEM_MASTER_BALL,
};

static const u8 sSideQuestDifficulties[SIDE_QUEST_COUNT] = 
{
    [SIDE_QUEST_1] =  QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_2] =  QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_3] =  QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_4] =  QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_5] =  QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_6] =  QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_7] =  QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_8] =  QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_9] =  QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_10] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_11] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_12] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_13] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_14] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_15] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_16] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_17] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_18] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_19] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_20] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_21] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_22] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_23] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_24] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_25] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_26] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_27] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_28] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_29] = QUEST_DIFFICULTY_EASY,
    [SIDE_QUEST_30] = QUEST_DIFFICULTY_EASY,
};

bool8 ScrCmd_questmenu(struct ScriptContext *ctx)
{
    u8 caseId = ScriptReadByte(ctx);
    u8 questId = VarGet(ScriptReadByte(ctx));

    switch (caseId)
    {
    case QUEST_MENU_OPEN:
    default:
        SetQuestMenuActive();
        BeginNormalPaletteFade(0xFFFFFFFF, 2, 16, 0, 0);
        ItemPc_Init(0, CB2_ReturnToFieldContinueScriptPlayMapMusic);
        ScriptContext1_Stop();
        break;
    case QUEST_MENU_UNLOCK_QUEST:
        GetSetQuestFlag(questId, FLAG_SET_UNLOCKED);
        break;
    case QUEST_MENU_COMPLETE_QUEST:
        GetSetQuestFlag(questId, FLAG_SET_COMPLETED);
        break;
    case QUEST_MENU_SET_ACTIVE:
        SetActiveQuest(questId);
        break;
    case QUEST_MENU_RESET_ACTIVE:
        ResetActiveQuest();
        break;
    case QUEST_MENU_BUFFER_QUEST_NAME:
            CopyQuestName(gStringVar1, questId);
        break;
    case QUEST_MENU_GET_ACTIVE_QUEST:
        gSpecialVar_LastResult = GetActiveQuestIndex();
        break;
    case QUEST_MENU_CHECK_UNLOCKED:
        if (GetSetQuestFlag(questId, FLAG_GET_UNLOCKED))
            gSpecialVar_LastResult = TRUE;
        else
            gSpecialVar_LastResult = FALSE;
        break;
    case QUEST_MENU_CHECK_COMPLETE:
        if (GetSetQuestFlag(questId, FLAG_GET_COMPLETED))
            gSpecialVar_LastResult = TRUE;
        else
            gSpecialVar_LastResult = FALSE;
        break;
    }

    return TRUE;
}
void ItemPc_BuildListMenuTemplate(void)
{
    u16 i;
    if (IsQuestMenuActive())
    {
        for (i = 0; i < sStateDataPtr->nItems; i++)
        {
            if (GetSetQuestFlag(i, FLAG_GET_UNLOCKED))
                sListMenuItems[i].label = sSideQuests[i].name;
            else
                sListMenuItems[i].label = sText_QuestMenu_Unk;

            sListMenuItems[i].index = i;
        }
    }
    else
    {
        for (i = 0; i < sStateDataPtr->nItems; i++)
        {
            sListMenuItems[i].label = ItemId_GetName(gSaveBlock1->pcItems[i].itemId);
            sListMenuItems[i].index = i;
        }
    }

    sListMenuItems[i].label = gFameCheckerText_Cancel;
    sListMenuItems[i].index = LIST_CANCEL;

    gMultiuseListMenuTemplate->items = (const struct ListMenuItem *)sListMenuItems;
    gMultiuseListMenuTemplate->totalItems = sStateDataPtr->nItems + 1;
    gMultiuseListMenuTemplate->windowId = 0;
    gMultiuseListMenuTemplate->header_X = 0;
    gMultiuseListMenuTemplate->item_X = 9;
    gMultiuseListMenuTemplate->cursor_X = 1;
    gMultiuseListMenuTemplate->lettersSpacing = 1;
    gMultiuseListMenuTemplate->itemVerticalPadding = 2;
    gMultiuseListMenuTemplate->upText_Y = 2;
    gMultiuseListMenuTemplate->maxShowed = sStateDataPtr->maxShowed;
    gMultiuseListMenuTemplate->fontId = 2;
    gMultiuseListMenuTemplate->cursorPal = 2;
    gMultiuseListMenuTemplate->fillValue = 0;
    gMultiuseListMenuTemplate->cursorShadowPal = 3;
    gMultiuseListMenuTemplate->moveCursorFunc = ItemPc_MoveCursorFunc;
    gMultiuseListMenuTemplate->itemPrintFunc = ItemPc_ItemPrintFunc;
    gMultiuseListMenuTemplate->scrollMultiple = 0;
    gMultiuseListMenuTemplate->cursorKind = 0;
}
void ItemPc_MoveCursorFunc(s32 itemIndex, bool8 onInit, struct ListMenu * list)
{
    u16 itemId;
    const u8 * desc;
    (void)list;
    if (onInit != TRUE)
        PlaySE(SE_SELECT);
    if (sStateDataPtr->moveModeOrigPos == 0xFF)
    {
        DestroyItemMenuIcon(sStateDataPtr->itemMenuIconSlot ^ 1);
        if (!IsQuestMenuActive())
        {
            if (itemIndex != LIST_CANCEL)
            {
                itemId = ItemPc_GetItemIdBySlotId(itemIndex);
                CreateItemMenuIcon(itemId, sStateDataPtr->itemMenuIconSlot);
                if (ItemId_GetPocket(itemId) == POCKET_TM_CASE)
                    desc = gMoveNames[ItemIdToBattleMoveId(itemId)];
                else
                    desc = ItemId_GetDescription(itemId);
            }
            else
            {
                CreateItemMenuIcon(ITEM_N_A, sStateDataPtr->itemMenuIconSlot);
                desc = gText_ReturnToPC;
            }
        }
        else
        {
            if (itemIndex != LIST_CANCEL)
            {
                if (GetSetQuestFlag(itemIndex, FLAG_GET_UNLOCKED))
                {
                    itemId = sSideQuestDifficultyItemIds[sSideQuestDifficulties[itemIndex]];
                    desc = sSideQuests[itemIndex].desc;
                }
                else
                {
                    itemId = ITEM_NONE;
                    desc = sText_QuestMenu_Unk;
                }

                CreateItemMenuIcon(itemId, sStateDataPtr->itemMenuIconSlot);
            }  
            else
            {
                CreateItemMenuIcon(ITEM_N_A, sStateDataPtr->itemMenuIconSlot);
                desc = sText_QuestMenu_Exit;
            }
        }
        sStateDataPtr->itemMenuIconSlot ^= 1;
        FillWindowPixelBuffer(1, 0);
        ItemPc_AddTextPrinterParameterized(1, 2, desc, 0, 3, 2, 0, 0, 3);
    }
}
void ItemPc_ItemPrintFunc(u8 windowId, s32 itemId, u8 y)
{
    if (sStateDataPtr->moveModeOrigPos != 0xFF)
    {
        if (sStateDataPtr->moveModeOrigPos == (u8)itemId)
            ItemPc_PrintOrRemoveCursorAt(y, 2);
        else
            ItemPc_PrintOrRemoveCursorAt(y, 0xFF);
    }

    if (itemId != LIST_CANCEL)
    {
        if (IsQuestMenuActive())
        {
            if (GetSetQuestFlag(itemId, FLAG_GET_COMPLETED))
                StringCopy(gStringVar4, sText_QuestMenu_Complete);
            else if (IsActiveQuest(itemId))
                StringCopy(gStringVar4, sText_QuestMenu_Active);
            else
                StringCopy(gStringVar4, gExpandedPlaceholder_Empty);

            ItemPc_AddTextPrinterParameterized(windowId, 0, gStringVar4, 110, y, 0, 0, TEXT_SPEED_FF, 1);
        }
        else
        {
            u16 quantity = ItemPc_GetItemQuantityBySlotId(itemId);
            ConvertIntToDecimalStringN(gStringVar1, quantity, STR_CONV_MODE_RIGHT_ALIGN, 3);
            StringExpandPlaceholders(gStringVar4, gText_TimesStrVar1);
            ItemPc_AddTextPrinterParameterized(windowId, 0, gStringVar4, 110, y, 0, 0, 0xFF, 1);
        }
    }
}
void ItemPc_PrintWithdrawItem(void)
{
    if (IsQuestMenuActive())
        ItemPc_AddTextPrinterParameterized(2, 0, sText_Quests, 0, 1, 0, 1, 0, 0);
    else
        ItemPc_AddTextPrinterParameterized(2, 0, gText_WithdrawItem, 0, 1, 0, 1, 0, 0);
}
void Task_ItemPcTurnOff2(u8 taskId)
{
    s16 * data = gTasks[taskId].data;

    SetQuestMenuInactive();
    if (!gPaletteFade.active && !IsPCScreenEffectRunning_TurnOff())
    {
        DestroyListMenuTask(data[0], &sListMenuState.scroll, &sListMenuState.row);
        if (sStateDataPtr->savedCallback != NULL)
            SetMainCallback2(sStateDataPtr->savedCallback);
        else
            SetMainCallback2(sListMenuState.savedCallback);
        ItemPc_RemoveScrollIndicatorArrowPair();
        ItemPc_FreeResources();
        DestroyTask(taskId);
    }
}
void ItemPc_CountPcItems(void)
{
    u16 i;
    if (IsQuestMenuActive())
    {
        sStateDataPtr->nItems = SIDE_QUEST_COUNT;
    }
    else
    {
        ItemPcCompaction();
        sStateDataPtr->nItems = 0;
        for (i = 0; i < PC_ITEMS_COUNT; sStateDataPtr->nItems++, i++)
        {
            if (gSaveBlock1->pcItems[i].itemId == ITEM_NONE)
                break;
        }
    }

    sStateDataPtr->maxShowed = sStateDataPtr->nItems + 1 <= 6 ? sStateDataPtr->nItems + 1 : 6;
}

void Task_ItemPcMain(u8 taskId)
{
    s16 * data = gTasks[taskId].data;
    u16 scroll;
    u16 row;
    s32 input;

    if (!gPaletteFade.active && !IsPCScreenEffectRunning_TurnOn())
    {
        if (!IsQuestMenuActive() && JOY_NEW(SELECT_BUTTON))
        {
            ListMenuGetScrollAndRow(data[0], &scroll, &row);
            if (scroll + row != sStateDataPtr->nItems)
            {
                PlaySE(SE_SELECT);
                ItemPc_MoveItemModeInit(taskId, scroll + row);
                return;
            }
        }

        input = ListMenu_ProcessInput(data[0]);
        ListMenuGetScrollAndRow(data[0], &sListMenuState.scroll, &sListMenuState.row);
        switch (input)
        {
        case LIST_NOTHING_CHOSEN:
            break;
        case LIST_CANCEL:
            PlaySE(SE_SELECT);
            ItemPc_SetInitializedFlag(FALSE);
            gTasks[taskId].func = Task_ItemPcTurnOff1;
            break;
        default:
            if (IsQuestMenuActive())
            {
                if (GetSetQuestFlag(input, FLAG_GET_UNLOCKED))
                {
                    PlaySE(SE_SELECT);
                    ItemPc_SetMessageWindowPalette(1);
                    ItemPc_RemoveScrollIndicatorArrowPair();
                    data[1] = input;
                    //data[2] = QuestMenu_GetItemQuantityBySlotId(input);
                    data[2] = 0;
                    ItemPc_PrintOrRemoveCursor(data[0], 2);            
                    gTasks[taskId].func = Task_ItemPcSubmenuInit;
                }
                else
                {
                    PlaySE(SE_HAZURE);
                }
            }
            else
            {
                PlaySE(SE_SELECT);
                ItemPc_SetMessageWindowPalette(1);
                ItemPc_RemoveScrollIndicatorArrowPair();
                data[1] = input;
                data[2] = ItemPc_GetItemQuantityBySlotId(input);
                ItemPc_PrintOrRemoveCursor(data[0], 2);
                gTasks[taskId].func = Task_ItemPcSubmenuInit;
            }
            break;
        }
    }
}
void Task_ItemPcSubmenuInit(u8 taskId)
{
    s16 * data = gTasks[taskId].data;
    u8 windowId;

    ItemPc_SetBorderStyleOnWindow(4);
    windowId = ItemPc_GetOrCreateSubwindow(0);

    if (IsQuestMenuActive())
    {
        if (GetSetQuestFlag(data[1], FLAG_GET_COMPLETED))
        {
            // completed
            PrintTextArray(4, 2, 8, 2, GetFontAttribute(2, FONTATTR_MAX_LETTER_HEIGHT) + 2, NELEMS(sCompletedQuestSubmenuOptions), sCompletedQuestSubmenuOptions);
            Menu_InitCursor(4, 2, 0, 2, GetFontAttribute(2, FONTATTR_MAX_LETTER_HEIGHT) + 2, NELEMS(sCompletedQuestSubmenuOptions), 0);

        }
        else if (IsActiveQuest(ItemPc_GetCursorPosition()))
        {
            // active
            PrintTextArray(4, 2, 8, 2, GetFontAttribute(2, FONTATTR_MAX_LETTER_HEIGHT) + 2, NELEMS(sActiveQuestSubmenuOptions), sActiveQuestSubmenuOptions);
            Menu_InitCursor(4, 2, 0, 2, GetFontAttribute(2, FONTATTR_MAX_LETTER_HEIGHT) + 2, NELEMS(sActiveQuestSubmenuOptions), 0);
        }
        else
        {
            // unlocked
            PrintTextArray(4, 2, 8, 2, GetFontAttribute(2, FONTATTR_MAX_LETTER_HEIGHT) + 2, NELEMS(sQuestSubmenuOptions), sQuestSubmenuOptions);
            Menu_InitCursor(4, 2, 0, 2, GetFontAttribute(2, FONTATTR_MAX_LETTER_HEIGHT) + 2, NELEMS(sQuestSubmenuOptions), 0);
        }

        StringCopy(gStringVar4, sText_QuestMenu_SelectedQuest);
    }
    else
    {
        PrintTextArray(4, 2, 8, 2, GetFontAttribute(2, FONTATTR_MAX_LETTER_HEIGHT) + 2, 3, sItemPcSubmenuOptions);
        Menu_InitCursor(4, 2, 0, 2, GetFontAttribute(2, FONTATTR_MAX_LETTER_HEIGHT) + 2, 3, 0);
        CopyItemName(ItemPc_GetItemIdBySlotId(data[1]), gStringVar1);
        StringExpandPlaceholders(gStringVar4, gText_Var1IsSelected);
    }

    ItemPc_AddTextPrinterParameterized(windowId, 2, gStringVar4, 0, 2, 1, 0, 0, 1);
    ScheduleBgCopyTilemapToVram(0);
    gTasks[taskId].func = Task_ItemPcSubmenuRun;
}
void Task_ItemPcSubmenuRun(u8 taskId)
{
    s8 input = Menu_ProcessInputNoWrapAround();
    switch (input)
    {
    case -1:
        PlaySE(SE_SELECT);
        Task_ItemPcCancel(taskId);
        break;
    case -2:
        break;
    default:
        PlaySE(SE_SELECT);
        if (IsQuestMenuActive())
        {
            u8 questIndex = ItemPc_GetCursorPosition();

            if (GetSetQuestFlag(questIndex, FLAG_GET_COMPLETED))
                sCompletedQuestSubmenuOptions[input].func.void_u8(taskId);
            else if (IsActiveQuest(questIndex))
                sActiveQuestSubmenuOptions[input].func.void_u8(taskId);
            else       
                sQuestSubmenuOptions[input].func.void_u8(taskId);
        }
        else
        {
            sItemPcSubmenuOptions[input].func.void_u8(taskId);
        }
    }
}
//// NEW - QUEST MENU ////
//// Functions
void SetQuestMenuActive(void)
{
    FlagSet(FLAG_QUEST_MENU_ACTIVE);
}

static void SetQuestMenuInactive(void)
{
    FlagClear(FLAG_QUEST_MENU_ACTIVE);
}

static bool8 IsQuestMenuActive(void)
{
    return FlagGet(FLAG_QUEST_MENU_ACTIVE);
}

s8 GetSetQuestFlag(u8 quest, u8 caseId)
{
    u8 index;
    u8 bit;
    u8 mask;

    index = quest / 8; //8 bits per byte
    bit = quest % 8;
    mask = 1 << bit;

    switch (caseId)
    {
    case FLAG_GET_UNLOCKED:
        return gSaveBlock1->unlockedQuests[index] & mask;
    case FLAG_SET_UNLOCKED:
        gSaveBlock1->unlockedQuests[index] |= mask;
        return 1;
    case FLAG_GET_COMPLETED:
        return gSaveBlock1->completedQuests[index] & mask;
    case FLAG_SET_COMPLETED:
        gSaveBlock1->completedQuests[index] |= mask;
        return 1;
    }

    return -1;  //failure
}

s8 GetActiveQuestIndex(void)
{
    if (gSaveBlock1->activeQuest > 0)
        return (gSaveBlock1->activeQuest - 1);
    else
        return NO_ACTIVE_QUEST;
}

static bool8 IsActiveQuest(u8 questId)
{
    if ((u8)GetActiveQuestIndex() == questId)
        return TRUE;

    return FALSE;
}

void SetActiveQuest(u8 questId)
{
    gSaveBlock1->activeQuest = questId + 1;  // 1-indexed
}

void ResetActiveQuest(void)
{
    gSaveBlock1->activeQuest = 0;
}

static void QuestMenuSubmenuSelectionMessage(u8 taskId)
{
    s16 * data = gTasks[taskId].data;

    ClearStdWindowAndFrameToTransparent(4, FALSE);
    ItemPc_DestroySubwindow(0);
    ClearWindowTilemap(4);
    data[8] = 1;
    PutWindowTilemap(0);
    ScheduleBgCopyTilemapToVram(0);
}

static void Task_QuestMenuCleanUp(u8 taskId)
{
    s16 * data = gTasks[taskId].data;

    ItemPc_DestroySubwindow(2);
    PutWindowTilemap(1);
    DestroyListMenuTask(data[0], &sListMenuState.scroll, &sListMenuState.row);

    ItemPc_CountPcItems();
    ItemPc_SetCursorPosition();
    ItemPc_BuildListMenuTemplate();
    data[0] = ListMenuInit(gMultiuseListMenuTemplate, sListMenuState.scroll, sListMenuState.row);
    ScheduleBgCopyTilemapToVram(0);
    ItemPc_ReturnFromSubmenu(taskId);
}

static void Task_QuestMenuRefreshAfterAcknowledgement(u8 taskId)
{
    if (JOY_NEW(A_BUTTON) || JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_SELECT);
        Task_QuestMenuCleanUp(taskId);
    }
}

static void QuestMenu_DisplaySubMenuMessage(u8 taskId)
{
    s16 * data = gTasks[taskId].data;
    u8 windowId;

    windowId = ItemPc_GetOrCreateSubwindow(2);
    AddTextPrinterParameterized(windowId, 2, gStringVar4, 0, 2, 0, NULL);
    gTasks[taskId].func = Task_QuestMenuRefreshAfterAcknowledgement;
}

static void Task_QuestMenuCancel(u8 taskId)
{
    s16 * data = gTasks[taskId].data;

    ClearStdWindowAndFrameToTransparent(4, FALSE);
    ItemPc_DestroySubwindow(0);
    ClearWindowTilemap(4);
    PutWindowTilemap(0);
    PutWindowTilemap(1);
    ItemPc_PrintOrRemoveCursor(data[0], 1);
    ScheduleBgCopyTilemapToVram(0);
    ItemPc_ReturnFromSubmenu(taskId);
}

static void Task_QuestMenuDetails(u8 taskId)
{
    u8 questIndex = ItemPc_GetCursorPosition();

    QuestMenuSubmenuSelectionMessage(taskId);
    StringCopy(gStringVar1, sSideQuests[questIndex].poc);
    StringCopy(gStringVar2, sSideQuests[questIndex].map);
    StringExpandPlaceholders(gStringVar4, sText_QuestMenu_DisplayDetails);
    QuestMenu_DisplaySubMenuMessage(taskId);
}

static void Task_QuestMenuReward(u8 taskId)
{
    u8 questIndex = ItemPc_GetCursorPosition();

    QuestMenuSubmenuSelectionMessage(taskId);
    StringCopy(gStringVar1, sSideQuests[questIndex].reward);
    StringExpandPlaceholders(gStringVar4, sText_QuestMenu_DisplayReward);
    QuestMenu_DisplaySubMenuMessage(taskId);
}

static void Task_QuestMenuEndQuest(u8 taskId)
{
    u8 questIndex = ItemPc_GetCursorPosition();

    ResetActiveQuest();
    QuestMenuSubmenuSelectionMessage(taskId);
    StringCopy(gStringVar1, sSideQuests[questIndex].name);
    StringExpandPlaceholders(gStringVar4, sText_QuestMenu_EndQuest);
    QuestMenu_DisplaySubMenuMessage(taskId);
}

static void Task_QuestMenuBeginQuest(u8 taskId)
{
    u8 questIndex = ItemPc_GetCursorPosition();

    SetActiveQuest(questIndex);
    QuestMenuSubmenuSelectionMessage(taskId);
    StringCopy(gStringVar1, sSideQuests[questIndex].name);
    StringExpandPlaceholders(gStringVar4, sText_QuestMenu_BeginQuest);
    QuestMenu_DisplaySubMenuMessage(taskId);
}

void CopyQuestName(u8 *dst, u8 questId)
{
    StringCopy(dst, sSideQuests[questId].name);
}


void DebugSideQuestMenu(void)
{
    FlagSet(FLAG_QUEST_MENU_ACTIVE);
    
    GetSetQuestFlag(0, FLAG_SET_UNLOCKED);
    GetSetQuestFlag(3, FLAG_SET_UNLOCKED);
    GetSetQuestFlag(3, FLAG_SET_COMPLETED);
    GetSetQuestFlag(6, FLAG_SET_UNLOCKED);
    GetSetQuestFlag(8, FLAG_SET_UNLOCKED);
    GetSetQuestFlag(10, FLAG_SET_UNLOCKED);
    GetSetQuestFlag(11, FLAG_SET_UNLOCKED);
    GetSetQuestFlag(11, FLAG_SET_COMPLETED);
    GetSetQuestFlag(14, FLAG_SET_UNLOCKED);
    GetSetQuestFlag(20, FLAG_SET_UNLOCKED);
    GetSetQuestFlag(23, FLAG_SET_UNLOCKED);
    
    SetActiveQuest(6);
}
