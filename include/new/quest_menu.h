#ifndef GUARD_QUEST_MENU_H
#define GUARD_QUEST_MENU_H

#include "../global.h"
#include "../list_menu.h"
#include "../../src/config.h"

#define QUEST_COUNT         30
#define QUEST_1_SUB_COUNT   10
#define QUEST_2_SUB_COUNT   8
#define SUB_QUEST_COUNT     (QUEST_1_SUB_COUNT + QUEST_2_SUB_COUNT)
#define QUEST_ARRAY_COUNT   (SUB_QUEST_COUNT > QUEST_COUNT ? SUB_QUEST_COUNT : QUEST_COUNT)

#define QUEST_STATE_BITS    5
#define QUEST_STATE_BYTES   ((QUEST_COUNT * QUEST_STATE_BITS + 7) / 8)
#define SUB_STATE_BYTES     ((SUB_QUEST_COUNT + 7) / 8)

// Script commands, set in Var8000 before: callasm QuestMenu_ScriptCommand+1
// Var8001 = quest id (parent id for sub quests), Var8002 = sub quest index
#define QUEST_MENU_OPEN                 0
#define QUEST_MENU_UNLOCK_QUEST         1
#define QUEST_MENU_SET_ACTIVE           2
#define QUEST_MENU_SET_REWARD           3
#define QUEST_MENU_COMPLETE_QUEST       4
#define QUEST_MENU_CHECK_UNLOCKED       5
#define QUEST_MENU_CHECK_INACTIVE       6
#define QUEST_MENU_CHECK_ACTIVE         7
#define QUEST_MENU_CHECK_REWARD         8
#define QUEST_MENU_CHECK_COMPLETE       9
#define QUEST_MENU_BUFFER_QUEST_NAME    10
#define QUEST_MENU_COMPLETE_SUBQUEST    11
#define QUEST_MENU_CHECK_SUBQUEST       12
#define QUEST_MENU_BUFFER_SUBQUEST_NAME 13
#define QUEST_MENU_RESET                14

// Icon types for the preview box
#define QUEST_ICON_OBJECT   1   // sprite = QUEST_OW(table, graphicsId)
#define QUEST_ICON_ITEM     2   // sprite = item id
#define QUEST_ICON_PKMN     3   // sprite = species id
#define QUEST_OW_NONE       0xFFFF
#define QUEST_OW(table, sprite) ((u16)((((u16)(table)) << 8) | ((u16)(sprite))))

// List modes. Add QUEST_SORT_AZ for alphabetical, QUEST_SORT_SUBQUEST while inside a sub quest list
#define QUEST_SORT_DEFAULT      0
#define QUEST_SORT_INACTIVE     1
#define QUEST_SORT_ACTIVE       2
#define QUEST_SORT_REWARD       3
#define QUEST_SORT_DONE         4
#define QUEST_SORT_AZ           10
#define QUEST_SORT_SUBQUEST     100

enum QuestCases
{
    QUEST_GET_UNLOCKED,
    QUEST_GET_INACTIVE,
    QUEST_GET_ACTIVE,
    QUEST_GET_REWARD,
    QUEST_GET_COMPLETED,
    QUEST_GET_FAVORITE,
    QUEST_SET_UNLOCKED,
    QUEST_SET_INACTIVE,
    QUEST_SET_ACTIVE,
    QUEST_SET_REWARD,
    QUEST_SET_COMPLETED,
    QUEST_SET_FAVORITE,
    QUEST_REMOVE_INACTIVE,
    QUEST_REMOVE_ACTIVE,
    QUEST_REMOVE_REWARD,
    QUEST_REMOVE_FAVORITE,
};

struct SubQuest
{
    u8 id;                  // unique across all sub quests, picks the save bit
    const u8 *name;
    const u8 *desc;
    const u8 *map;
    u16 sprite;
    u8 spriteType;
    const u8 *type;         // label shown in the state column once completed
};

struct SideQuest
{
    const u8 *name;
    const u8 *desc;
    const u8 *doneDesc;
    const u8 *map;
    u16 sprite;
    u8 spriteType;
    const struct SubQuest *subQuests;
    u8 numSubQuests;
};

extern const struct SideQuest gSideQuests[QUEST_COUNT];

void ShowQuestMenu(void);
void QuestMenu_ScriptCommand(void);
u8 QuestMenu_GetSetQuestState(u8 quest, u8 caseId);
u8 QuestMenu_GetSetSubquestState(u8 quest, u8 caseId, u8 childQuest);
void QuestMenu_ResetSaveData(void);

#endif // GUARD_QUEST_MENU_H
