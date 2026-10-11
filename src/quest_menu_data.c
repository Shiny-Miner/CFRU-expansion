#include "../include/global.h"
#include "../include/new/quest_menu.h"
#include "../include/constants/flags.h"
#include "../include/window.h"
#include "../include/bg.h"

#ifdef SIDEQUEST_MENU

// External declarations for quest item names and descriptions
extern const u8 gText_QuestItem_1[];
extern const u8 gText_QuestDesc_1[];
extern const u8 gText_QuestItem_2[];
extern const u8 gText_QuestDesc_2[];
extern const u8 gText_QuestItem_3[];
extern const u8 gText_QuestDesc_3[];
extern const u8 gText_QuestItem_4[];
extern const u8 gText_QuestDesc_4[];
extern const u8 gText_QuestItem_5[];
extern const u8 gText_QuestDesc_5[];
extern const u8 gText_QuestItem_6[];
extern const u8 gText_QuestDesc_6[];
extern const u8 gText_QuestItem_7[];
extern const u8 gText_QuestDesc_7[];
extern const u8 gText_QuestItem_8[];
extern const u8 gText_QuestDesc_8[];
extern const u8 gText_QuestItem_9[];
extern const u8 gText_QuestDesc_9[];
extern const u8 gText_QuestItem_10[];
extern const u8 gText_QuestDesc_10[];
extern const u8 gText_QuestDone[];
extern const u8 gText_QuestGift[];
extern const u8 EventScript_GiveMons_Special[];

// Array of menu page titles
const u8 *const sMenuTitles[] = {
    gText_MenuTitle,
    gText_MenuTitle2,
};

// Pointers to the respective quest tables for each page
const struct QuestInfo *const gQuestTables[] = {
    gMainQuestTable,
    gSideQuestTable,
};

// Total number of pages available in the quest menu
u8 gNumPages = 2;
const u16 FLAG_HIDE_QUEST_STATS = 0x0021;

// Main quests table using var-based state tracking (unlockState, doneState, rewardState)
const struct QuestInfo gMainQuestTable[] = {
    {
        .unlockFlag = FLAG_QUEST_1_UNLOCKED,
        .doneFlag = FLAG_QUEST_1_COMPLETED,
        .rewardFlag = FLAG_QUEST_1_REWARDED,
        .graphicsId = QUEST_OW(0, 14),
        .name = gText_QuestItem_1,
        .description = gText_QuestDesc_1,
        .script = EventScript_GiveMons_Special,
    },
    {
        .unlockFlag = FLAG_QUEST_2_UNLOCKED,
        .doneFlag = FLAG_QUEST_2_COMPLETED,
        .rewardFlag = FLAG_QUEST_2_REWARDED,
        .graphicsId = QUEST_OW(2, 9),
        .name = gText_QuestItem_2,
        .description = gText_QuestDesc_2,
        .script = NULL,
    },
    {
        .unlockFlag = FLAG_QUEST_3_UNLOCKED,
        .doneFlag = FLAG_QUEST_3_COMPLETED,
        .rewardFlag = FLAG_QUEST_3_REWARDED,
        .graphicsId = QUEST_OW(0, 1),
        .name = gText_QuestItem_3,
        .description = gText_QuestDesc_3,
        .script = NULL,
    },
        {
        .unlockFlag = FLAG_QUEST_4_UNLOCKED,
        .doneFlag = FLAG_QUEST_4_COMPLETED,
        .rewardFlag = FLAG_QUEST_4_REWARDED,
        .graphicsId = QUEST_OW(2, 14),
        .name = gText_QuestItem_4,
        .description = gText_QuestDesc_4,
        .script = NULL,
    },
    // Table terminator (marks the end of the main quest list)
    {
        .unlockFlag = SPECIES_TABLES_TERMIN
    },
};

// Side quests table (currently empty, containing only the table terminator)
const struct QuestInfo gSideQuestTable[] = {
    {
        .unlockFlag = FLAG_QUEST_2_UNLOCKED,
        .doneFlag = FLAG_QUEST_2_COMPLETED,
        .rewardFlag = FLAG_QUEST_2_REWARDED,
        .graphicsId = QUEST_OW(2, 9),
        .name = gText_QuestItem_2,
        .description = gText_QuestDesc_2,
        .script = NULL,
    },
    {
        .unlockFlag = SPECIES_TABLES_TERMIN
    },
};

// Text color configurations for UI rendering
const struct TextColor sWhiteText = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GREY };
const struct TextColor sBlackText = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_DARK_GREY, TEXT_COLOR_LIGHT_GREY };
const struct TextColor sRedText   = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_RED,   TEXT_COLOR_LIGHT_RED };
const struct TextColor sBlueText  = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_BLUE,  TEXT_COLOR_LIGHT_BLUE };
const struct TextColor sGreenText = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_GREEN, TEXT_COLOR_LIGHT_GREEN};
const struct TextColor sGrayText  = { TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_GREY, TEXT_COLOR_DARK_GREY };

// Background layer templates for the menu layout
const struct BgTemplate sQuestMenuBGTemplates[] = {
    // Window Text and UI
    {
        .bg = 0,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
    // Graphic background
    {
        .bg = 3,
        .charBaseIndex = 1,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0,
    },
};

// Window layout templates for UI text boxes and containers
const struct WindowTemplate sQuestMenuWindowTemplates[] = {
    // Window 0: Title and page header area
    {
        .bg = 0,
        .tilemapLeft = 21,
        .tilemapTop = 0,
        .width = 9,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 0,
    },
    // Window 1: Quest items list (main content area on the left)
    {
        .bg = 0,
        .tilemapLeft = 1,  
        .tilemapTop = 1,
        .width = 18,
        .height = 14,
        .paletteNum = 15,
        .baseBlock = 18,
    },
    // Window 2: Quest description box (bottom text container)
    {
        .bg = 0,
        .tilemapLeft = 2,
        .tilemapTop = 15,
        .width = 26,
        .height = 6,
        .paletteNum = 15,
        .baseBlock = 270,
    },
    // Window 3: Sidebar/stats display area (right side)
    {
        .bg = 0,
        .tilemapLeft = 21,    
        .tilemapTop = 7,    
        .width = 8,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 426,
    },
    // Terminator template
    DUMMY_WIN_TEMPLATE
};

// Background graphics assets pointers (tiles, palette, tilemap)
const struct ImageData gQuestMenuBGData = {
    .tiles = Menu_BGTiles,
    .pal = Menu_BGPal,
    .tilemap = Menu_BGMap,
};

#endif