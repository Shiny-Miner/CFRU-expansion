#ifndef GUARD_QUEST_MENU_H
#define GUARD_QUEST_MENU_H

#include "../bg.h"
#include "../global.h"
#include "../window.h"
#include "../../src/config.h"

#define QUEST_OW_NONE 0xFFFF
#define QUEST_OW(table, sprite) ((u16)((((u16)(table)) << 8) | ((u16)(sprite))))

struct QuestInfo
{
    u16 unlockFlag;
    u16 doneFlag;
    u16 rewardFlag;
    u16 graphicsId;
    const u8 *name;
    const u8 *description;
    const u8 *script;
};

extern const struct QuestInfo gMainQuestTable[];
extern const struct QuestInfo gSideQuestTable[];

extern const u8 gText_MenuTitle[];
extern const u8 gText_MenuTitle2[];

extern const struct TextColor sWhiteText;
extern const struct TextColor sBlackText;
extern const struct TextColor sRedText;
extern const struct TextColor sBlueText;
extern const struct TextColor sGreenText;
extern const struct TextColor sGrayText;

#define COLOR(color) &s##color##Text

// UI layout constants
#define VISIBLE_ITEMS 7
#define FONT_SIZE 1
#define FONT_SIZE_SMALL 0

struct ImageData
{
    const u8 *tiles;
    const u8 *pal;
    const u16 *tilemap;
};

extern const struct ImageData gQuestMenuBGData;
extern const struct BgTemplate sQuestMenuBGTemplates[];
extern const struct WindowTemplate sQuestMenuWindowTemplates[];

struct MenuStruct 
{
    u8 cursorPos;
    u8 firstVisibleItem;
    u8 selectedItem;
    u8 page;
};

enum MenuStates
{
    MENU_STATE_INIT,
    MENU_STATE_RESET,
    MENU_STATE_INIT_BGS,
    MENU_STATE_LOAD_GFX,
    MENU_STATE_SHOW_BGS,
    MENU_STATE_INIT_WINDOWS,
    MENU_STATE_FADE_IN,
    MENU_STATE_START,
};

enum MenuBGs
{
    BG_TEXT,
    BG_NIL,
    BG_NIL_2,
    BG_BACKGROUND,
    BG_COUNT,
};

enum MenuWindows
{
    WINDOW_TITLE,
    WINDOW_ITEMS,
    WINDOW_DESCRIPTION,
    WINDOW_CONTROLS,
    WINDOW_COUNT,
};

extern const u8 Menu_BGTiles[];
extern const u8 Menu_BGPal[];
extern const u16 Menu_BGMap[];

void ShowQuestMenu(void);

#endif // GUARD_QUEST_MENU_H