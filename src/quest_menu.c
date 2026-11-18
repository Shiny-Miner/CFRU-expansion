#include "../include/global.h"
#include "../include/palette.h"
#include "../include/sound.h"
#include "../include/new/quest_menu.h"
#include "../include/bg.h"
#include "../include/decompress.h"
#include "../include/event_data.h"
#include "../include/gpu_regs.h"
#include "../include/item_icon.h"
#include "../include/list_menu.h"
#include "../include/main.h"
#include "../include/malloc.h"
#include "../include/menu.h"
#include "../include/overworld.h"
#include "../include/pokemon_icon.h"
#include "../include/scanline_effect.h"
#include "../include/script.h"
#include "../include/sprite.h"
#include "../include/string_util.h"
#include "../include/task.h"
#include "../include/text.h"
#include "../include/text_window.h"
#include "../include/window.h"
#include "../include/constants/songs.h"
#include "../include/new/ram_locs.h"
#include "../include/new/Vanilla_functions.h"

#ifdef SIDEQUEST_MENU

#define NAME_LENGTH         40
#define MAX_SHOWED          4
#define SPRITE_NONE         0xFF

#define QUEST_ICON_X        19
#define QUEST_ICON_Y        136
#define GFX_TAG_QUEST_ICON  0xFDF0
#define GFX_TAG_ITEM_ICON   102

#define BG_TEXT             0
#define BG_BACKGROUND       3
#define BG_MAP_BYTES        0x800

#define WIN_LIST            0
#define WIN_FOOTER          1
#define WIN_HEADER          2

#define BIT_UNLOCKED        0
#define BIT_ACTIVE          1
#define BIT_REWARD          2
#define BIT_COMPLETED       3
#define BIT_FAVORITE        4

#define FADE_STEP           2

_Static_assert((FLAG_QUEST_MENU_START % 8) == 0, "FLAG_QUEST_MENU_START must be a multiple of 8");

struct QuestMenuData
{
    u8 tilemap[BG_MAP_BYTES] ALIGNED(4);
    struct ListMenuTemplate listTemplate;
    struct ListMenuItem items[QUEST_ARRAY_COUNT + 1];
    u8 names[QUEST_ARRAY_COUNT + 1][NAME_LENGTH];
    u8 sortedList[QUEST_COUNT];
    u8 locationText[64];
    u8 descText[160];
    u16 scroll;
    u16 row;
    u16 savedScroll;
    u16 savedRow;
    u8 mainTaskId;
    u8 listTaskId;
    u8 arrowTaskId;
    u8 numItems;
    u8 filterMode;
    u8 sortAZ;
    u8 inSubMode;
    u8 parentQuest;
    u8 iconSprite;
    u16 iconTag;
    u8 iconType;
    u8 pendingAction;
    s16 fadeLevel;
};

#define gQuestMenu (*((struct QuestMenuData**) 0x203E008))

enum
{
    ACTION_NONE,
    ACTION_ENTER_SUB,
    ACTION_LEAVE_SUB,
    ACTION_REBUILD_FILTER,
};

extern const u8 gText_QuestUnknown[];
extern const u8 gText_QuestStateReward[];
extern const u8 gText_QuestStateDone[];
extern const u8 gText_QuestStateActive[];
extern const u8 gText_QuestClose[];
extern const u8 gText_QuestLocation[];
extern const u8 gText_QuestNotStarted[];
extern const u8 gText_QuestReturnForReward[];
extern const u8 gText_QuestNumberDisplay[];
extern const u8 gText_QuestSubButton[];
extern const u8 gText_QuestSubSpace[];
extern const u8 gText_QuestSubNumberSep[];
extern const u8 gText_QuestColorGreen[];
extern const u8 gText_QuestHeaderAll[];
extern const u8 gText_QuestHeaderInactive[];
extern const u8 gText_QuestHeaderActive[];
extern const u8 gText_QuestHeaderReward[];
extern const u8 gText_QuestHeaderDone[];
extern const u8 gText_QuestHeaderSub[];
extern const u8 gText_QuestSortAZ[];
extern const u8 gText_QuestSortAll[];
extern const u8 gText_QuestSortInactive[];
extern const u8 gText_QuestSortActive[];
extern const u8 gText_QuestSortReward[];
extern const u8 gText_QuestSortDone[];

static const u8 sTextHeader[4] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GREY, 0};
static const u8 sTextRed[4] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_RED, TEXT_COLOR_LIGHT_RED, 0};
static const u8 sTextGreen[4] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_GREEN, TEXT_COLOR_LIGHT_GREEN, 0};
static const u8 sTextBlue[4] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_LIGHT_BLUE, TEXT_COLOR_BLUE, 0};
static const u8 sTextWhite[4] = {TEXT_COLOR_TRANSPARENT, TEXT_COLOR_WHITE, TEXT_COLOR_TRANSPARENT, 0};

static const struct BgTemplate sBgTemplates[] =
{
    {
        .bg = BG_TEXT,
        .charBaseIndex = 0,
        .mapBaseIndex = 31,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 0,
        .baseTile = 0,
    },
    {
        .bg = BG_BACKGROUND,
        .charBaseIndex = 2,
        .mapBaseIndex = 30,
        .screenSize = 0,
        .paletteMode = 0,
        .priority = 3,
        .baseTile = 0,
    },
};

static const struct WindowTemplate sWindowTemplates[] =
{
    [WIN_LIST] =
    {
        .bg = BG_TEXT,
        .tilemapLeft = 0,
        .tilemapTop = 2,
        .width = 30,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 1,
    },
    [WIN_FOOTER] =
    {
        .bg = BG_TEXT,
        .tilemapLeft = 0,
        .tilemapTop = 12,
        .width = 30,
        .height = 8,
        .paletteNum = 15,
        .baseBlock = 241,
    },
    [WIN_HEADER] =
    {
        .bg = BG_TEXT,
        .tilemapLeft = 0,
        .tilemapTop = 0,
        .width = 30,
        .height = 2,
        .paletteNum = 15,
        .baseBlock = 481,
    },
    DUMMY_WIN_TEMPLATE
};

void TransferPlttBuffer(void);
extern const struct EventObjectGraphicsInfo *GetEventObjectGraphicsInfo(u16 graphicsId);
extern const struct SpritePalette *GetQuestOverworldPalette(u16 paletteTag);

extern const u32 Menu_BGTiles[];
extern const u32 Menu_BGMap[];
extern const u16 Menu_BGPal[];

static void CB2_QuestMenu(void);
static void VBlankCB_QuestMenu(void);
static void Task_QuestMenuMain(u8 taskId);
static void Task_QuestMenuFade(u8 taskId);
static void Task_QuestMenuExit(u8 taskId);
static void BuildList(void);
static void StartList(bool8 keepCursor);
static void PrintHeader(void);
static void DestroyIcon(void);
static void ShowIcon(u16 sprite, u8 type);

// ---------------------------------------------------------------------------
// Saved state. Quest bits come first, sub quest bits follow them.
// ---------------------------------------------------------------------------

static u8 *GetQuestSaveData(void)
{
    return GetFlagPointer(FLAG_QUEST_MENU_START);
}

static bool8 GetQuestBit(u8 quest, u8 bit)
{
    u8 *data = GetQuestSaveData();
    u16 index = quest * QUEST_STATE_BITS + bit;

    if (data == NULL || quest >= QUEST_COUNT)
        return FALSE;
    return (data[index / 8] >> (index % 8)) & 1;
}

static void SetQuestBit(u8 quest, u8 bit, bool8 value)
{
    u8 *data = GetQuestSaveData();
    u16 index = quest * QUEST_STATE_BITS + bit;

    if (data == NULL || quest >= QUEST_COUNT)
        return;
    if (value)
        data[index / 8] |= 1 << (index % 8);
    else
        data[index / 8] &= ~(1 << (index % 8));
}

static bool8 GetSubBit(u8 quest, u8 child)
{
    u8 *data = GetQuestSaveData();
    u8 id;

    if (data == NULL || quest >= QUEST_COUNT || child >= gSideQuests[quest].numSubQuests)
        return FALSE;
    id = gSideQuests[quest].subQuests[child].id;
    data += QUEST_STATE_BYTES;
    return (data[id / 8] >> (id % 8)) & 1;
}

static void SetSubBit(u8 quest, u8 child)
{
    u8 *data = GetQuestSaveData();
    u8 id;

    if (data == NULL || quest >= QUEST_COUNT || child >= gSideQuests[quest].numSubQuests)
        return;
    id = gSideQuests[quest].subQuests[child].id;
    data += QUEST_STATE_BYTES;
    data[id / 8] |= 1 << (id % 8);
}

u8 QuestMenu_GetSetQuestState(u8 quest, u8 caseId)
{
    switch (caseId)
    {
    case QUEST_GET_UNLOCKED:
        return GetQuestBit(quest, BIT_UNLOCKED);
    case QUEST_GET_INACTIVE:
        return !GetQuestBit(quest, BIT_ACTIVE) && !GetQuestBit(quest, BIT_REWARD) && !GetQuestBit(quest, BIT_COMPLETED);
    case QUEST_GET_ACTIVE:
        return GetQuestBit(quest, BIT_ACTIVE);
    case QUEST_GET_REWARD:
        return GetQuestBit(quest, BIT_REWARD);
    case QUEST_GET_COMPLETED:
        return GetQuestBit(quest, BIT_COMPLETED);
    case QUEST_GET_FAVORITE:
        return GetQuestBit(quest, BIT_FAVORITE);
    case QUEST_SET_UNLOCKED:
        SetQuestBit(quest, BIT_UNLOCKED, TRUE);
        return TRUE;
    case QUEST_SET_INACTIVE:
        SetQuestBit(quest, BIT_ACTIVE, FALSE);
        SetQuestBit(quest, BIT_REWARD, FALSE);
        SetQuestBit(quest, BIT_COMPLETED, FALSE);
        return TRUE;
    case QUEST_SET_ACTIVE:
        SetQuestBit(quest, BIT_UNLOCKED, TRUE);
        SetQuestBit(quest, BIT_ACTIVE, TRUE);
        return TRUE;
    case QUEST_SET_REWARD:
        SetQuestBit(quest, BIT_ACTIVE, FALSE);
        SetQuestBit(quest, BIT_REWARD, TRUE);
        return TRUE;
    case QUEST_SET_COMPLETED:
        SetQuestBit(quest, BIT_ACTIVE, FALSE);
        SetQuestBit(quest, BIT_REWARD, FALSE);
        SetQuestBit(quest, BIT_COMPLETED, TRUE);
        return TRUE;
    case QUEST_SET_FAVORITE:
        SetQuestBit(quest, BIT_FAVORITE, TRUE);
        return TRUE;
    case QUEST_REMOVE_INACTIVE:
    case QUEST_REMOVE_ACTIVE:
        SetQuestBit(quest, BIT_ACTIVE, FALSE);
        return TRUE;
    case QUEST_REMOVE_REWARD:
        SetQuestBit(quest, BIT_REWARD, FALSE);
        return TRUE;
    case QUEST_REMOVE_FAVORITE:
        SetQuestBit(quest, BIT_FAVORITE, FALSE);
        return TRUE;
    }
    return FALSE;
}

u8 QuestMenu_GetSetSubquestState(u8 quest, u8 caseId, u8 childQuest)
{
    if (caseId == QUEST_GET_COMPLETED)
        return GetSubBit(quest, childQuest);
    if (caseId == QUEST_SET_COMPLETED)
    {
        SetSubBit(quest, childQuest);
        return TRUE;
    }
    return FALSE;
}

void QuestMenu_ResetSaveData(void)
{
    u8 *data = GetQuestSaveData();
    u8 i;

    if (data == NULL)
        return;
    for (i = 0; i < QUEST_STATE_BYTES + SUB_STATE_BYTES; i++)
        data[i] = 0;
}

static bool8 HasChildren(u8 quest)
{
    return gSideQuests[quest].numSubQuests != 0
        && gSideQuests[quest].subQuests != NULL
        && !QuestMenu_GetSetQuestState(quest, QUEST_GET_INACTIVE);
}

// ---------------------------------------------------------------------------
// Script interface
// ---------------------------------------------------------------------------

void QuestMenu_ScriptCommand(void)
{
    u8 quest = Var8001;
    u8 child = Var8002;

    Var800D = 0;

    if (Var8000 != QUEST_MENU_OPEN && Var8000 != QUEST_MENU_RESET && quest >= QUEST_COUNT)
        return;

    switch (Var8000)
    {
    case QUEST_MENU_OPEN:
        ShowQuestMenu();
        break;
    case QUEST_MENU_UNLOCK_QUEST:
        QuestMenu_GetSetQuestState(quest, QUEST_SET_UNLOCKED);
        break;
    case QUEST_MENU_SET_ACTIVE:
        QuestMenu_GetSetQuestState(quest, QUEST_SET_ACTIVE);
        break;
    case QUEST_MENU_SET_REWARD:
        QuestMenu_GetSetQuestState(quest, QUEST_SET_REWARD);
        break;
    case QUEST_MENU_COMPLETE_QUEST:
        QuestMenu_GetSetQuestState(quest, QUEST_SET_COMPLETED);
        break;
    case QUEST_MENU_CHECK_UNLOCKED:
        Var800D = QuestMenu_GetSetQuestState(quest, QUEST_GET_UNLOCKED);
        break;
    case QUEST_MENU_CHECK_INACTIVE:
        Var800D = QuestMenu_GetSetQuestState(quest, QUEST_GET_INACTIVE);
        break;
    case QUEST_MENU_CHECK_ACTIVE:
        Var800D = QuestMenu_GetSetQuestState(quest, QUEST_GET_ACTIVE);
        break;
    case QUEST_MENU_CHECK_REWARD:
        Var800D = QuestMenu_GetSetQuestState(quest, QUEST_GET_REWARD);
        break;
    case QUEST_MENU_CHECK_COMPLETE:
        Var800D = QuestMenu_GetSetQuestState(quest, QUEST_GET_COMPLETED);
        break;
    case QUEST_MENU_BUFFER_QUEST_NAME:
        StringCopy(gStringVar1, gSideQuests[quest].name);
        break;
    case QUEST_MENU_COMPLETE_SUBQUEST:
        QuestMenu_GetSetSubquestState(quest, QUEST_SET_COMPLETED, child);
        break;
    case QUEST_MENU_CHECK_SUBQUEST:
        Var800D = QuestMenu_GetSetSubquestState(quest, QUEST_GET_COMPLETED, child);
        break;
    case QUEST_MENU_RESET:
        QuestMenu_ResetSaveData();
        break;
    case QUEST_MENU_BUFFER_SUBQUEST_NAME:
        if (child < gSideQuests[quest].numSubQuests)
            StringCopy(gStringVar1, gSideQuests[quest].subQuests[child].name);
        break;
    }
}

// ---------------------------------------------------------------------------
// List building
// ---------------------------------------------------------------------------

static void SortQuestsByName(u8 *list, u8 count)
{
    u8 i, j, tmp;

    for (i = 0; i < count; i++)
    {
        for (j = i + 1; j < count; j++)
        {
            if (StringCompare(gSideQuests[list[i]].name, gSideQuests[list[j]].name) > 0)
            {
                tmp = list[i];
                list[i] = list[j];
                list[j] = tmp;
            }
        }
    }
}

static bool8 QuestPassesFilter(u8 quest)
{
    if (gQuestMenu->filterMode == QUEST_SORT_DEFAULT)
        return TRUE;
    return QuestMenu_GetSetQuestState(quest, QUEST_GET_UNLOCKED)
        && QuestMenu_GetSetQuestState(quest, gQuestMenu->filterMode);
}

static void MoveCursorFunc(s32 itemIndex, bool8 onInit, struct ListMenu *list);
static void ItemPrintFunc(u8 windowId, s32 itemId, u8 y);

static void BuildList(void)
{
    struct QuestMenuData *m = gQuestMenu;
    u8 n = 0;
    u8 i;

    if (m->inSubMode)
    {
        const struct SideQuest *quest = &gSideQuests[m->parentQuest];

        for (i = 0; i < quest->numSubQuests; i++)
        {
            ConvertIntToDecimalStringN(m->names[n], i + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
            StringAppend(m->names[n], gText_QuestSubNumberSep);
            StringAppend(m->names[n], quest->subQuests[i].name);
            m->items[n].name = m->names[n];
            m->items[n].id = i;
            n++;
        }
    }
    else
    {
        u8 count = 0;

        for (i = 0; i < QUEST_COUNT; i++)
            m->sortedList[i] = i;
        if (m->sortAZ)
            SortQuestsByName(m->sortedList, QUEST_COUNT);

        for (i = 0; i < QUEST_COUNT; i++)
        {
            u8 quest = m->sortedList[i];
            u8 *name = m->names[count];

            if (!QuestPassesFilter(quest))
                continue;

            name[0] = EOS;
            if (QuestMenu_GetSetQuestState(quest, QUEST_GET_FAVORITE))
                StringCopy(name, gText_QuestColorGreen);

            if (QuestMenu_GetSetQuestState(quest, QUEST_GET_UNLOCKED))
            {
                StringAppend(name, gSideQuests[quest].name);
                if (HasChildren(quest))
                    StringAppend(name, gText_QuestSubButton);
            }
            else
                StringAppend(name, gText_QuestUnknown);

            m->items[count].name = name;
            m->items[count].id = quest;
            count++;
        }
        n = count;
    }

    StringCopy(m->names[n], gText_QuestClose);
    m->items[n].name = m->names[n];
    m->items[n].id = LIST_CANCEL;
    n++;
    m->numItems = n;
}

static void PrintHeader(void)
{
    struct QuestMenuData *m = gQuestMenu;
    const u8 *title;
    const u8 *label = NULL;
    u16 done = 0;
    u16 total;
    u8 i;

    FillWindowPixelBuffer(WIN_HEADER, PIXEL_FILL(0));

    if (m->inSubMode)
    {
        title = gText_QuestHeaderSub;
        total = gSideQuests[m->parentQuest].numSubQuests;
        for (i = 0; i < total; i++)
            done += GetSubBit(m->parentQuest, i);
    }
    else
    {
        static const u8 *const titles[] = {gText_QuestHeaderAll, gText_QuestHeaderInactive, gText_QuestHeaderActive, gText_QuestHeaderReward, gText_QuestHeaderDone};
        static const u8 *const labels[] = {gText_QuestSortAll, gText_QuestSortInactive, gText_QuestSortActive, gText_QuestSortReward, gText_QuestSortDone};

        title = titles[m->filterMode];
        label = m->sortAZ ? gText_QuestSortAZ : labels[m->filterMode];
        total = QUEST_COUNT;
        if (m->filterMode == QUEST_SORT_DEFAULT)
        {
            for (i = 0; i < QUEST_COUNT; i++)
                done += QuestMenu_GetSetQuestState(i, QUEST_GET_COMPLETED);
        }
        else
            done = m->numItems - 1;
    }

    ConvertIntToDecimalStringN(gStringVar1, done, STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, total, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringExpandPlaceholders(gStringVar4, gText_QuestNumberDisplay);

    AddTextPrinterParameterized4(WIN_HEADER, 2, 10, 1, 0, 0, sTextHeader, 0, title);
    AddTextPrinterParameterized4(WIN_HEADER, 2, 122, 1, 0, 0, sTextHeader, 0, gStringVar4);
    if (label != NULL)
        AddTextPrinterParameterized4(WIN_HEADER, 0, 168, 4, 0, 0, sTextHeader, 0, label);
    CopyWindowToVram(WIN_HEADER, COPYWIN_GFX);
}

static void ItemPrintFunc(u8 windowId, s32 itemId, u8 y)
{
    struct QuestMenuData *m = gQuestMenu;
    const u8 *text = NULL;
    const u8 *color = sTextHeader;

    if (itemId < 0)
        return;

    if (m->inSubMode)
    {
        if (GetSubBit(m->parentQuest, itemId))
        {
            const u8 *type = gSideQuests[m->parentQuest].subQuests[itemId].type;
            text = (type != NULL) ? type : gText_QuestStateDone;
            color = sTextGreen;
        }
    }
    else if (QuestMenu_GetSetQuestState(itemId, QUEST_GET_UNLOCKED))
    {
        if (QuestMenu_GetSetQuestState(itemId, QUEST_GET_REWARD))
        {
            text = gText_QuestStateReward;
            color = sTextRed;
        }
        else if (QuestMenu_GetSetQuestState(itemId, QUEST_GET_COMPLETED))
        {
            text = gText_QuestStateDone;
            color = sTextGreen;
        }
        else if (QuestMenu_GetSetQuestState(itemId, QUEST_GET_ACTIVE))
        {
            text = gText_QuestStateActive;
            color = sTextBlue;
        }
    }

    if (text != NULL)
        AddTextPrinterParameterized4(windowId, 0, 200, y + 2, 0, 0, color, 0xFF, text);
}

static void PrintFooter(s32 id)
{
    struct QuestMenuData *m = gQuestMenu;
    const u8 *map = NULL;
    const u8 *desc = NULL;
    u16 sprite = QUEST_OW_NONE;
    u8 spriteType = 0;

    FillWindowPixelBuffer(WIN_FOOTER, PIXEL_FILL(0));
    DestroyIcon();

    if (id >= 0)
    {
        if (m->inSubMode)
        {
            const struct SubQuest *sub = &gSideQuests[m->parentQuest].subQuests[id];

            map = sub->map;
            desc = sub->desc;
            sprite = sub->sprite;
            spriteType = sub->spriteType;
        }
        else if (QuestMenu_GetSetQuestState(id, QUEST_GET_UNLOCKED))
        {
            const struct SideQuest *quest = &gSideQuests[id];

            map = quest->map;
            sprite = quest->sprite;
            spriteType = quest->spriteType;
            if (QuestMenu_GetSetQuestState(id, QUEST_GET_REWARD))
            {
                StringCopy(gStringVar2, quest->map);
                StringExpandPlaceholders(m->descText, gText_QuestReturnForReward);
                desc = m->descText;
            }
            else if (QuestMenu_GetSetQuestState(id, QUEST_GET_COMPLETED))
                desc = quest->doneDesc;
            else if (QuestMenu_GetSetQuestState(id, QUEST_GET_ACTIVE))
                desc = quest->desc;
            else
                desc = gText_QuestNotStarted;
        }
    }

    if (map != NULL)
    {
        u8 *location = m->locationText;

        StringCopy(location, gText_QuestLocation);
        StringAppend(location, map);
        AddTextPrinterParameterized4(WIN_FOOTER, 2, 2, 3, 0, 0, sTextWhite, 0, location);
    }
    if (desc != NULL)
        AddTextPrinterParameterized4(WIN_FOOTER, 2, 40, 19, 0, 0, sTextWhite, 0, desc);

    CopyWindowToVram(WIN_FOOTER, COPYWIN_GFX);

    if (map != NULL)
        ShowIcon(sprite, spriteType);
}

static void MoveCursorFunc(s32 itemIndex, bool8 onInit, struct ListMenu *list)
{
    if (!onInit)
        PlaySE(SE_SELECT);
    PrintFooter(itemIndex);
}

static void StartList(bool8 keepCursor)
{
    struct QuestMenuData *m = gQuestMenu;
    struct ListMenuTemplate *t = &m->listTemplate;
    u8 maxScroll;

    if (!keepCursor)
    {
        m->scroll = 0;
        m->row = 0;
    }

    FillWindowPixelBuffer(WIN_LIST, PIXEL_FILL(0));
    BuildList();

    if (m->scroll + m->row >= m->numItems)
    {
        m->scroll = 0;
        m->row = 0;
    }

    t->items = m->items;
    t->moveCursorFunc = MoveCursorFunc;
    t->itemPrintFunc = ItemPrintFunc;
    t->totalItems = m->numItems;
    t->maxShowed = MAX_SHOWED;
    t->windowId = WIN_LIST;
    t->header_X = 0;
    t->item_X = 22;
    t->cursor_X = 12;
    t->upText_Y = 2;
    t->cursorPal = TEXT_COLOR_WHITE;
    t->fillValue = TEXT_COLOR_TRANSPARENT;
    t->cursorShadowPal = TEXT_COLOR_DARK_GREY;
    t->lettersSpacing = 0;
    t->itemVerticalPadding = 2;
    t->scrollMultiple = 0;
    t->fontId = 2;
    t->cursorKind = 0;

    PrintHeader();

    m->listTaskId = ListMenuInit(t, m->scroll, m->row);
    maxScroll = (m->numItems > MAX_SHOWED) ? m->numItems - MAX_SHOWED : 0;
    m->arrowTaskId = AddScrollIndicatorArrowPairParameterized(2, 94, 8, 90, maxScroll, 110, 110, &m->scroll);
}

static void ReloadList(bool8 keepCursor)
{
    struct QuestMenuData *m = gQuestMenu;

    RemoveScrollIndicatorArrowPair(m->arrowTaskId);
    DestroyListMenuTask(m->listTaskId, &m->scroll, &m->row);
    StartList(keepCursor);
}

// ---------------------------------------------------------------------------
// Preview icon
// ---------------------------------------------------------------------------

static void DestroyIcon(void)
{
    struct QuestMenuData *m = gQuestMenu;

    if (m->iconSprite == SPRITE_NONE)
        return;

    DestroySprite(&gSprites[m->iconSprite]);
    if (m->iconType == QUEST_ICON_OBJECT || m->iconType == QUEST_ICON_ITEM)
    {
        FreeSpriteTilesByTag(m->iconTag);
        FreeSpritePaletteByTag(m->iconTag);
    }
    m->iconSprite = SPRITE_NONE;
}

static u8 CreateObjectIcon(u16 graphicsId)
{
    const struct EventObjectGraphicsInfo *graphics;
    const struct SpritePalette *sourcePal;
    struct SpritePalette palette;
    struct SpriteSheet sheet;
    struct SpriteTemplate spriteTemplate;
    u8 spriteId;

    graphics = GetEventObjectGraphicsInfo(graphicsId);
    if (graphics == NULL || graphics->images == NULL || graphics->images[0].data == NULL)
        return SPRITE_NONE;
    if (graphics->width > 32 || graphics->height > 32)
        return SPRITE_NONE;

    sourcePal = GetQuestOverworldPalette(graphics->paletteTag1);
    if (sourcePal == NULL)
        return SPRITE_NONE;

    sheet.data = graphics->images[0].data;
    sheet.size = graphics->size;
    sheet.tag = GFX_TAG_QUEST_ICON;
    palette.data = sourcePal->data;
    palette.tag = GFX_TAG_QUEST_ICON;
    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&palette);

    spriteTemplate.tileTag = GFX_TAG_QUEST_ICON;
    spriteTemplate.paletteTag = GFX_TAG_QUEST_ICON;
    spriteTemplate.oam = graphics->oam;
    spriteTemplate.anims = gDummySpriteAnimTable;
    spriteTemplate.images = graphics->images;
    spriteTemplate.affineAnims = gDummySpriteAffineAnimTable;
    spriteTemplate.callback = SpriteCallbackDummy;

    spriteId = CreateSprite(&spriteTemplate, QUEST_ICON_X, QUEST_ICON_Y, 0);
    if (spriteId >= MAX_SPRITES)
    {
        FreeSpriteTilesByTag(GFX_TAG_QUEST_ICON);
        FreeSpritePaletteByTag(GFX_TAG_QUEST_ICON);
        return SPRITE_NONE;
    }
    return spriteId;
}

static void ShowIcon(u16 sprite, u8 type)
{
    struct QuestMenuData *m = gQuestMenu;
    u8 spriteId = SPRITE_NONE;

    switch (type)
    {
    case QUEST_ICON_OBJECT:
        if (sprite != QUEST_OW_NONE)
        {
            spriteId = CreateObjectIcon(sprite);
            m->iconTag = GFX_TAG_QUEST_ICON;
        }
        break;
    case QUEST_ICON_ITEM:
        spriteId = AddItemIconSprite(GFX_TAG_ITEM_ICON, GFX_TAG_ITEM_ICON, sprite);
        if (spriteId < MAX_SPRITES)
        {
            gSprites[spriteId].pos1.x = QUEST_ICON_X;
            gSprites[spriteId].pos1.y = QUEST_ICON_Y;
        }
        m->iconTag = GFX_TAG_ITEM_ICON;
        break;
    case QUEST_ICON_PKMN:
        LoadMonIconPalettes();
        spriteId = CreateMonIcon(sprite, SpriteCallbackDummy, QUEST_ICON_X, QUEST_ICON_Y, 0, 0, FALSE);
        break;
    }

    if (spriteId >= MAX_SPRITES)
    {
        m->iconSprite = SPRITE_NONE;
        return;
    }

    gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
    m->iconSprite = spriteId;
    m->iconType = type;
}

// ---------------------------------------------------------------------------
// Tasks
// ---------------------------------------------------------------------------

static void SetFadeLevel(s16 level)
{
    gQuestMenu->fadeLevel = level;
    SetGpuReg(REG_OFFSET_BLDALPHA, level | ((16 - level) << 8));
}

static void StartFade(u8 action)
{
    struct QuestMenuData *m = gQuestMenu;

    m->pendingAction = action;
    gTasks[m->mainTaskId].data[0] = 0;
    gTasks[m->mainTaskId].func = Task_QuestMenuFade;
}

static void Task_QuestMenuFade(u8 taskId)
{
    struct QuestMenuData *m = gQuestMenu;
    s16 *data = gTasks[taskId].data;

    if (data[0] == 0)
    {
        SetFadeLevel(m->fadeLevel - FADE_STEP);
        if (m->fadeLevel <= 0)
        {
            SetFadeLevel(0);
            if (m->pendingAction == ACTION_ENTER_SUB)
            {
                m->savedScroll = m->scroll;
                m->savedRow = m->row;
                m->inSubMode = TRUE;
                ReloadList(FALSE);
            }
            else if (m->pendingAction == ACTION_LEAVE_SUB)
            {
                m->inSubMode = FALSE;
                m->scroll = m->savedScroll;
                m->row = m->savedRow;
                ReloadList(TRUE);
            }
            m->pendingAction = ACTION_NONE;
            data[0] = 1;
        }
    }
    else
    {
        SetFadeLevel(m->fadeLevel + FADE_STEP);
        if (m->fadeLevel >= 16)
        {
            SetFadeLevel(16);
            gTasks[taskId].func = Task_QuestMenuMain;
        }
    }
}

static void Task_QuestMenuExit(u8 taskId)
{
    struct QuestMenuData *m = gQuestMenu;

    if (gPaletteFade->active)
        return;

    DestroyIcon();
    RemoveScrollIndicatorArrowPair(m->arrowTaskId);
    DestroyListMenuTask(m->listTaskId, NULL, NULL);
    FreeAllWindowBuffers();
    DestroyTask(taskId);
    Free(m);
    gQuestMenu = NULL;

    SetGpuReg(REG_OFFSET_BLDCNT, 0);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0);
    ScriptContext2_Disable();
    SetMainCallback2(CB2_ReturnToFieldContinueScript);
}

static void StartExit(void)
{
    PlaySE(SE_SELECT);
    BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
    gTasks[gQuestMenu->mainTaskId].func = Task_QuestMenuExit;
}

static void Task_QuestMenuMain(u8 taskId)
{
    struct QuestMenuData *m = gQuestMenu;
    s32 input;
    s32 selected;

    if (gPaletteFade->active)
        return;

    input = ListMenu_ProcessInput(m->listTaskId);
    ListMenuGetScrollAndRow(m->listTaskId, &m->scroll, &m->row);
    selected = m->items[m->scroll + m->row].id;

    if (JOY_NEW(SELECT_BUTTON) && !m->inSubMode && selected >= 0
     && QuestMenu_GetSetQuestState(selected, QUEST_GET_UNLOCKED))
    {
        PlaySE(SE_SELECT);
        if (QuestMenu_GetSetQuestState(selected, QUEST_GET_FAVORITE))
            QuestMenu_GetSetQuestState(selected, QUEST_REMOVE_FAVORITE);
        else
            QuestMenu_GetSetQuestState(selected, QUEST_SET_FAVORITE);
        ReloadList(TRUE);
    }
    else if (JOY_NEW(START_BUTTON) && !m->inSubMode)
    {
        PlaySE(SE_SELECT);
        m->sortAZ ^= 1;
        ReloadList(FALSE);
    }
    else if (JOY_NEW(R_BUTTON) && !m->inSubMode)
    {
        PlaySE(SE_SELECT);
        m->filterMode = (m->filterMode + 1) % 5;
        ReloadList(FALSE);
    }
    else if (input == LIST_CANCEL)
    {
        if (m->inSubMode)
        {
            PlaySE(SE_SELECT);
            StartFade(ACTION_LEAVE_SUB);
        }
        else
            StartExit();
    }
    else if (input >= 0)
    {
        if (!m->inSubMode && QuestMenu_GetSetQuestState(input, QUEST_GET_UNLOCKED) && HasChildren(input))
        {
            PlaySE(SE_SELECT);
            m->parentQuest = input;
            StartFade(ACTION_ENTER_SUB);
        }
    }
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

static void VBlankCB_QuestMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

static void MainCB2_QuestMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    DoScheduledBgTilemapCopiesToVram();
    UpdatePaletteFade();
}

static void LoadMenuBG(void)
{
    struct QuestMenuData *m = gQuestMenu;

    CpuFastFill16(0, (void*) BG_CHAR_ADDR(0), BG_CHAR_SIZE * 4);
    CpuFastFill16(0, m->tilemap, BG_MAP_BYTES);
    DecompressAndCopyTileDataToVram(BG_BACKGROUND, Menu_BGTiles, 0, 0, 0);
    LZDecompressWram(Menu_BGMap, m->tilemap);
    LoadPalette(Menu_BGPal, 0, 0x20);
    LoadPalette(stdpal_get(0), 0xF0, 0x20);
}

static void CB2_QuestMenu(void)
{
    struct QuestMenuData *m;
    u8 i;

    switch (gMain.state)
    {
    case 0:
        SetVBlankCallback(NULL);
        DmaFill16(3, 0, VRAM, VRAM_SIZE);
        DmaFill32(3, 0, OAM, OAM_SIZE);
        DmaFill16(3, 0, PLTT, PLTT_SIZE);
        gMain.state++;
        break;
    case 1:
        ScanlineEffect_Stop();
        ResetTasks();
        ResetSpriteData();
        ResetTempTileDataBuffers();
        ResetPaletteFade();
        FreeAllSpritePalettes();
        gQuestMenu = AllocZeroed(sizeof(struct QuestMenuData));
        m = gQuestMenu;
        m->iconSprite = SPRITE_NONE;
        m->fadeLevel = 16;
        gMain.state++;
        break;
    case 2:
        ResetBgsAndClearDma3BusyFlags(0);
        InitBgsFromTemplates(0, sBgTemplates, ARRAY_COUNT(sBgTemplates));
        SetBgTilemapBuffer(BG_BACKGROUND, gQuestMenu->tilemap);
        ChangeBgX(BG_BACKGROUND, 0, 0);
        ChangeBgY(BG_BACKGROUND, 0, 0);
        ChangeBgX(BG_TEXT, 0, 0);
        ChangeBgY(BG_TEXT, 0, 0);
        gMain.state++;
        break;
    case 3:
        LoadMenuBG();
        gMain.state++;
        break;
    case 4:
        if (!free_temp_tile_data_buffers_if_possible())
        {
            SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_OBJ_1D_MAP);
            ShowBg(BG_TEXT);
            ShowBg(BG_BACKGROUND);
            CopyBgTilemapBufferToVram(BG_BACKGROUND);
            gMain.state++;
        }
        break;
    case 5:
        InitWindows(sWindowTemplates);
        DeactivateAllTextPrinters();
        for (i = 0; i < WIN_HEADER + 1; i++)
        {
            FillWindowPixelBuffer(i, PIXEL_FILL(0));
            PutWindowTilemap(i);
            CopyWindowToVram(i, COPYWIN_BOTH);
        }
        gMain.state++;
        break;
    case 6:
        SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_OBJ | BLDCNT_TGT1_BG0 | BLDCNT_TGT2_BG3 | BLDCNT_EFFECT_BLEND);
        SetGpuReg(REG_OFFSET_BLDALPHA, 16);
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
        PlaySE(SE_WIN_OPEN);
        gMain.state++;
        break;
    case 7:
        SetVBlankCallback(VBlankCB_QuestMenu);
        StartList(FALSE);
        gQuestMenu->mainTaskId = CreateTask(Task_QuestMenuMain, 0);
        SetMainCallback2(MainCB2_QuestMenu);
        break;
    }
}

void ShowQuestMenu(void)
{
    ScriptContext2_Enable();
    SetMainCallback2(CB2_QuestMenu);
}

#endif // SIDEQUEST_MENU
