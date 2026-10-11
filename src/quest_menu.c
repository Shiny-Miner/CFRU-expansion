#include "../include/palette.h"
#include "../include/sound.h"
#include "../include/new/quest_menu.h"
#include "../include/bg.h"
#include "../include/decompress.h"
#include "../include/event_data.h"
#include "../include/gpu_regs.h"
#include "../include/graphics.h"
#include "../include/item.h"
#include "../include/list_menu.h"
#include "../include/international_string_util.h"
#include "../include/main.h"
#include "../include/malloc.h"
#include "../include/menu.h"
#include "../include/menu_helpers.h"
#include "../include/overworld.h"
#include "../include/scanline_effect.h"
#include "../include/script.h"
#include "../include/string_util.h"
#include "../include/constants/field_weather.h"
#include "../include/constants/songs.h"
#include "../include/new/ram_locs.h"
#include "../include/new/Vanilla_functions.h"
#include "../include/sprite.h"
#include "../include/follower_mon_sprites.h"

#ifdef SIDEQUEST_MENU

#define QUEST_OW_SPRITE_MARKER 0x51E7
#define COLOR_MENU_TITLE COLOR(White) // Color for the menu title and sidebar texts
#define COLOR_DESCRIPTION COLOR(White) // Color for the quest description text at the bottom
#define COLOR_MENU_ITEM COLOR(Black) // Color for normal, unselected quest items in the list
#define COLOR_MENU_SELECTED_ITEM COLOR(White) // Color for the currently highlighted/selected quest item (including the arrow)
#define COLOR_MENU_INFO_COLOR COLOR(Red)
#define COLOR_MENU_DONE_COLOR COLOR(Green)

#define gMenuStruct ((struct MenuStruct*) 0x203E008)
#define tilemapbuffer (*((u8**) 0x203E038))

#define BG_MAP_BYTES 0x800
#define VISIBLE_ITEMS 7

// Quest overworld sprite
#define GFX_TAG_QUEST_16X16  0xFDF0
#define GFX_TAG_QUEST_16X32  0xFDF1
#define GFX_TAG_QUEST_32X32  0xFDF2
#define GFX_TAG_QUEST_64X64  0xFDF3

#define QUEST_OW_SPRITE_X 201
#define QUEST_OW_SPRITE_Y 30

extern const struct EventObjectGraphicsInfo *GetEventObjectGraphicsInfo(u16 graphicsId);
extern const struct SpritePalette *GetQuestOverworldPalette(u16 paletteTag);

static u8 sQuestOverworldSpriteId = MAX_SPRITES;
static u16 sQuestOverworldGraphicsId = QUEST_OW_NONE;

// External configuration and resource declarations
extern u8 gNumPages;
extern const u16 FLAG_HIDE_QUEST_STATS;

extern const struct BgTemplate sQuestMenuBGTemplates[];
extern const struct ImageData gQuestMenuBGData;
extern const struct WindowTemplate sQuestMenuWindowTemplates[];
extern void TransferPlttBuffer(void);

extern const u8 gText_QuestGift[];
extern const u8 gText_QuestDone[];
extern const u8 gText_LText[];
extern const u8 gText_RText[];
extern const u8 gText_GetText[];
extern const u8 gText_CloseText[];
extern const u8 sText_Percent[];
extern const u8 sText_Slash[];
extern const u8 sText_Dash[];
extern const u8 gText_ItemDescriptionNotAvailable[];

extern const u8 *const sMenuTitles[];
extern const struct QuestInfo *const gQuestTables[];

// Static string literals for UI rendering elements
static const u8 sText_Arrow[] = {CHAR_ARROW_RIGHT, EOS};
static const u8 sText_Space[] = {CHAR_SPACE, EOS};

// Structure holding the runtime states for an individual quest based on flags
struct QuestStateInfo
{
    bool8 isUnlocked; // True if the quest flag meets or exceeds the unlock state
    bool8 isDone;     // True if the quest flag meets or exceeds the completion state
    bool8 isRewarded; // True if the quest flag meets or exceeds the reward state
};

// Structure holding aggregate progress statistics for a quest page
struct PageStats
{
    u8 total;     // Total number of quests on the page
    u8 unlocked;  // Number of unlocked quests
    u8 completed; // Number of completed quests
    u8 rewarded;  // Number of quests with claimed rewards
};

// Forward declarations for internal helper and callback functions
static u8 GetQuestCountForPage(u8 page);
static u8 GetQuestCountForCurrentPage(void);
static u8 BuildSortedQuestIndices(u8 page, u8 *outIndices, struct PageStats *stats);
static const struct QuestInfo *GetSelectedQuest(const u8 *sortedIndices, u8 total);
static void CleanWindow(u8 windowId);
static void CommitWindow(u8 windowId);
static void LoadMenuBG(void);
static void CB2_QuestMenu(void);
static void Task_QuestMenuFadeIn(u8 taskId);
static void Task_QuestMenuWaitForKeyPress(u8 taskId);
static void Task_QuestMenuFadeOut(u8 taskId);
static void PrintMenuTitle(void);
static void PrintMenuItems(const u8 *sortedIndices, u8 totalQuests);
static void PrintMenuItemDescription(const struct QuestInfo *quest);
static void PrintMenuControls(const struct PageStats *stats);
static void PrintMenuGUI(void);
static u8 FindSortedPageIndex(const u8 *sortedPages, u8 count, u8 page);
static void DestroyQuestOverworldSprite(void);
static bool8 CreateQuestOverworldSprite(u16 graphicsId);
static void UpdateQuestOverworldSprite(const struct QuestInfo *quest);

// Evaluates and populates the runtime state of a quest based on variable values
static void GetQuestStates(const struct QuestInfo *quest,struct QuestStateInfo *outState)
{
    outState->isUnlocked = FlagGet(quest->unlockFlag);
    outState->isDone = FlagGet(quest->doneFlag);
    outState->isRewarded = FlagGet(quest->rewardFlag);
}

// Sorts and indexes quests for a given page (active, locked, completed) and computes statistics
static u8 BuildSortedQuestIndices(u8 page, u8 *outIndices, struct PageStats *stats)
{
    u8 totalCount = GetQuestCountForPage(page);
    const struct QuestInfo *table = gQuestTables[page];

    if (stats != NULL)
    {
        stats->total = totalCount;
        stats->unlocked = 0;
        stats->completed = 0;
        stats->rewarded = 0;
    }

    if (totalCount == 0)
        return 0;

    u8 activeCount = 0;
    u8 lockedCount = 0;
    u8 completedCount = 0;

    u8 activeIndices[totalCount];
    u8 lockedIndices[totalCount];
    u8 completedIndices[totalCount];

    for (u8 i = 0; i < totalCount; i++)
    {
        struct QuestStateInfo state;
        GetQuestStates(&table[i], &state);

        if (stats != NULL)
        {
            if (state.isUnlocked) stats->unlocked++;
            if (state.isDone)     stats->completed++;
            if (state.isRewarded) stats->rewarded++;
        }

        if (outIndices != NULL)
        {
            if (!state.isUnlocked)
            {
                lockedIndices[lockedCount++] = i;
            }
            else if (state.isDone && state.isRewarded)
            {
                completedIndices[completedCount++] = i;
            }
            else
            {
                activeIndices[activeCount++] = i;
            }
        }
    }
    if (outIndices != NULL)
    {
        u8 idx = 0;

        for (u8 i = 0; i < activeCount; i++)
            outIndices[idx++] = activeIndices[i];

        for (u8 i = 0; i < lockedCount; i++)
            outIndices[idx++] = lockedIndices[i];

        for (u8 i = 0; i < completedCount; i++)
            outIndices[idx++] = completedIndices[i];
    }

    return totalCount;
}

// Builds sorted indices for menu pages based on completion status and availability
static u8 BuildSortedPageIndices(u8 *outPages)
{
    if (gNumPages == 0)
        return 0;

    u8 activeCount = 0;
    u8 completedCount = 0;
    u8 completedPages[gNumPages];

    for (u8 i = 0; i < gNumPages; i++)
    {
        struct PageStats stats;
        BuildSortedQuestIndices(i, NULL, &stats);

        if (stats.total == 0 || stats.unlocked == 0)
            continue;

        if (stats.rewarded < stats.total)
        {
            if (outPages != NULL)
                outPages[activeCount] = i;

            activeCount++;
        }
        else
        {
            completedPages[completedCount++] = i;
        }
    }

    if (outPages != NULL)
    {
        for (u8 i = 0; i < completedCount; i++)
            outPages[activeCount + i] = completedPages[i];
    }

    return activeCount + completedCount;
}

// Counts total valid entries in a specific quest table page until the terminator flag
static u8 GetQuestCountForPage(u8 page)
{
    const struct QuestInfo *table = gQuestTables[page];
    u16 count = 0;

    while (table[count].unlockFlag != SPECIES_TABLES_TERMIN)
    {
        count++;
        if (count >= 255)
        {
            count = 255;
            break;
        }
    }

    return (u8)count;
}

// Retrieves the total quest count for the currently active menu page
static u8 GetQuestCountForCurrentPage(void)
{
    return GetQuestCountForPage(gMenuStruct->page);
}

// Finds the sorted index position of a specific page identifier
static u8 FindSortedPageIndex(const u8 *sortedPages, u8 count, u8 page)
{
    for (u8 i = 0; i < count; i++)
    {
        if (sortedPages[i] == page)
            return i;
    }

    return 0;
}

// Returns a pointer to the currently highlighted quest info structure
static const struct QuestInfo *GetSelectedQuest(const u8 *sortedIndices, u8 total)
{
    u8 page = gMenuStruct->page;

    if (gMenuStruct->selectedItem >= total)
        gMenuStruct->selectedItem = total - 1;

    return &gQuestTables[page][sortedIndices[gMenuStruct->selectedItem]];
}

// VBlank callback handling OAM, sprite copies, and palette transfers
static void VBlankCB_QuestMenu(void)
{
    LoadOam();
    ProcessSpriteCopyRequests();
    TransferPlttBuffer();
}

// Main background callback 2 running tasks, sprites, and palette fades
static void MainCB2_QuestMenu(void)
{
    RunTasks();
    AnimateSprites();
    BuildOamBuffer();
    UpdatePaletteFade();
}

// Clears a specified window's pixel buffer
static void CleanWindow(u8 windowId)
{
    FillWindowPixelBuffer(windowId, PIXEL_FILL(0));
}

// Commits window tilemap changes and copies data to VRAM
static void CommitWindow(u8 windowId)
{
    PutWindowTilemap(windowId);
    CopyWindowToVram(windowId, COPYWIN_BOTH);

}

// Decompresses and loads background graphics, tiles, and palettes into VRAM
static void LoadMenuBG(void)
{
    const struct ImageData *img = &gQuestMenuBGData;

    CpuFastFill16(0, (void*)BG_CHAR_ADDR(0), BG_CHAR_SIZE * 4);
    CpuFastFill16(0, tilemapbuffer, BG_MAP_BYTES);

    DecompressAndCopyTileDataToVram(BG_BACKGROUND, img->tiles, 0, 0, 0);
    LZDecompressWram(img->tilemap, tilemapbuffer);
    LoadPalette(img->pal, 0, 0x20);
}

// Main initialization and state machine callback for setting up the quest menu
static void CB2_QuestMenu(void)
{
    switch (gMain.state)
    {
        case MENU_STATE_INIT:
        default:
        {
            u8 sortedPages[gNumPages];
            BuildSortedPageIndices(sortedPages);

            gMenuStruct->cursorPos = 0;
            gMenuStruct->firstVisibleItem = 0;
            gMenuStruct->selectedItem = 0;
            gMenuStruct->page = sortedPages[0];

            SetVBlankCallback(NULL);

            DmaFill16(3, 0, VRAM, VRAM_SIZE);
            DmaFill32(3, 0, OAM, OAM_SIZE);
            DmaFill16(3, 0, PLTT, PLTT_SIZE);

            gMain.state++;
            break;
        }

        case MENU_STATE_RESET:
            ScanlineEffect_Stop();
            ResetTasks();
            ResetSpriteData();
            ResetTempTileDataBuffers();
            ResetPaletteFade();
            FreeAllSpritePalettes();

            gMain.state++;
            break;

        case MENU_STATE_INIT_BGS:
            tilemapbuffer = Calloc(BG_MAP_BYTES);

            ResetBgsAndClearDma3BusyFlags(0);
            InitBgsFromTemplates(0, sQuestMenuBGTemplates, BG_COUNT);
            SetBgTilemapBuffer(BG_BACKGROUND, tilemapbuffer);

            ChangeBgX(BG_BACKGROUND, 0, 0);
            ChangeBgY(BG_BACKGROUND, 0, 0);
            ChangeBgX(BG_TEXT, 0, 0);
            ChangeBgY(BG_TEXT, 0, 0);

            gMain.state++;
            break;

        case MENU_STATE_LOAD_GFX:
            LoadMenuBG();
            gMain.state++;
            break;

        case MENU_STATE_SHOW_BGS:
            if (!free_temp_tile_data_buffers_if_possible())
            {
                ShowBg(BG_TEXT);
                ShowBg(BG_BACKGROUND);
                CopyBgTilemapBufferToVram(BG_BACKGROUND);
                gMain.state++;
            }
            break;

        case MENU_STATE_INIT_WINDOWS:
            InitWindows(sQuestMenuWindowTemplates);
            DeactivateAllTextPrinters();
            gMain.state++;
            break;

        case MENU_STATE_FADE_IN:
            BeginNormalPaletteFade(0xFFFFFFFF, 0, 16, 0, RGB_BLACK);
            PlaySE(SE_WIN_OPEN);
            gMain.state++;
            break;

        case MENU_STATE_START:
            SetVBlankCallback(VBlankCB_QuestMenu);
            PrintMenuGUI();

            CreateTask(Task_QuestMenuFadeIn, 0);
            SetMainCallback2(MainCB2_QuestMenu);

            gMain.state = 0;
            break;
    }
}

// Updates menu selection indices and scrolling positions when navigating up/down
static void UpdateMenuSelection(bool8 movingDown)
{
    u8 itemCount = GetQuestCountForCurrentPage();

    if (itemCount == 0)
        return;

    u8 *cursorPos = &gMenuStruct->cursorPos;
    u8 *firstVisibleItem = &gMenuStruct->firstVisibleItem;
    u8 *selectedItem = &gMenuStruct->selectedItem;

    PlaySE(SE_SELECT);

    if (movingDown)
    {
        if (*selectedItem >= itemCount - 1)
        {
            *selectedItem = 0;
            *firstVisibleItem = 0;
            *cursorPos = 0;
        }
        else
        {
            (*selectedItem)++;

            if (*cursorPos < VISIBLE_ITEMS - 1)
                (*cursorPos)++;
            else
                (*firstVisibleItem)++;
        }
    }
    else
    {
        if (*selectedItem == 0)
        {
            *selectedItem = itemCount - 1;
            *firstVisibleItem =
                (*selectedItem >= VISIBLE_ITEMS)
                ? (*selectedItem - VISIBLE_ITEMS + 1)
                : 0;

            *cursorPos = *selectedItem - *firstVisibleItem;
        }
        else
        {
            (*selectedItem)--;

            if (*cursorPos > 0)
                (*cursorPos)--;
            else
                (*firstVisibleItem)--;
        }
    }

        PrintMenuGUI();
}

// Task handling the fade-in completion transition
static void Task_QuestMenuFadeIn(u8 taskId)
{
    if (!gPaletteFade->active)
        gTasks[taskId].func = Task_QuestMenuWaitForKeyPress;
}

// Task handling user input (D-pad selection, page switching via L/R, and confirmation via A/B)
static void Task_QuestMenuWaitForKeyPress(u8 taskId)
{
    if (gMain.newKeys & B_BUTTON)
    {
        BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
        gTasks[taskId].func = Task_QuestMenuFadeOut;
    }
    else if (gMain.newKeys & (L_BUTTON | R_BUTTON))
    {
        u8 sortedPages[gNumPages];
        u8 totalValidPages = BuildSortedPageIndices(sortedPages);

        if (totalValidPages <= 1)
            return;

        u8 currentSortedIndex =
            FindSortedPageIndex(
                sortedPages,
                totalValidPages,
                gMenuStruct->page
            );

        s8 direction = (gMain.newKeys & L_BUTTON) ? -1 : 1;

        currentSortedIndex =
            (u8)((currentSortedIndex + totalValidPages + direction)
                 % totalValidPages);

        u8 targetPage = sortedPages[currentSortedIndex];

        if (targetPage != gMenuStruct->page)
        {
            gMenuStruct->page = targetPage;
            gMenuStruct->cursorPos = 0;
            gMenuStruct->firstVisibleItem = 0;
            gMenuStruct->selectedItem = 0;

            PlaySE(SE_SELECT);
            PrintMenuGUI();
        }
    }
   else if (gMain.newKeys & A_BUTTON)
{
    u8 currentPage = gMenuStruct->page;
    u8 totalQuests = GetQuestCountForPage(currentPage);
    u8 sortedIndices[totalQuests];
    struct PageStats stats;

    BuildSortedQuestIndices(currentPage, sortedIndices, &stats);

    const struct QuestInfo *quest = GetSelectedQuest(sortedIndices, totalQuests);

    if (quest != NULL)
    {
        struct QuestStateInfo state;
        GetQuestStates(quest, &state);

        if (state.isUnlocked
            && state.isDone
            && !state.isRewarded
            && quest->script != NULL)
        {
            ScriptContext1_SetupScript(quest->script);

            BeginNormalPaletteFade(0xFFFFFFFF, 0, 0, 16, RGB_BLACK);
            gTasks[taskId].func = Task_QuestMenuFadeOut;
        }
    }
}
    else if (JOY_NEW_AND_REPEATED(DPAD_UP))
    {
        UpdateMenuSelection(FALSE);
    }
    else if (JOY_NEW_AND_REPEATED(DPAD_DOWN))
    {
        UpdateMenuSelection(TRUE);
    }
}

// Task handling cleanup and fading out before returning to the overworld
static void Task_QuestMenuFadeOut(u8 taskId)
{
    if (!gPaletteFade->active)
    {
        DestroyQuestOverworldSprite();

        Free(tilemapbuffer);
        tilemapbuffer = NULL;

        ScriptContext2_Disable();
        gMain.state = MENU_STATE_INIT;

        PlaySE(SE_PC_OFF);
        SetMainCallback2(CB2_ReturnToFieldContinueScript);
        DestroyTask(taskId);
    }
}

static void DestroyQuestOverworldSprite(void)
{
    u8 i;

    for (i = 0; i < MAX_SPRITES; i++)
    {
        if (gSprites[i].inUse
            && gSprites[i].data[7] == QUEST_OW_SPRITE_MARKER)
        {
            DestroySprite(&gSprites[i]);
        }
    }

    sQuestOverworldSpriteId = MAX_SPRITES;
    sQuestOverworldGraphicsId = QUEST_OW_NONE;

    FreeSpriteTilesByTag(GFX_TAG_QUEST_16X16);
    FreeSpriteTilesByTag(GFX_TAG_QUEST_16X32);
    FreeSpriteTilesByTag(GFX_TAG_QUEST_32X32);
    FreeSpriteTilesByTag(GFX_TAG_QUEST_64X64);

    FreeSpritePaletteByTag(GFX_TAG_QUEST_16X16);
    FreeSpritePaletteByTag(GFX_TAG_QUEST_16X32);
    FreeSpritePaletteByTag(GFX_TAG_QUEST_32X32);
    FreeSpritePaletteByTag(GFX_TAG_QUEST_64X64);
}

static u16 GetQuestOverworldResourceTag(
    const struct EventObjectGraphicsInfo *graphics)
{
    if (graphics->width == 16 && graphics->height == 16)
        return GFX_TAG_QUEST_16X16;

    if (graphics->width == 16 && graphics->height == 32)
        return GFX_TAG_QUEST_16X32;

    if (graphics->width == 32 && graphics->height == 32)
        return GFX_TAG_QUEST_32X32;

    if (graphics->width == 64 && graphics->height == 64)
        return GFX_TAG_QUEST_64X64;

    return 0;
}

static bool8 CreateQuestOverworldSprite(u16 graphicsId)
{
    const struct EventObjectGraphicsInfo *graphics;
    const struct SpritePalette *sourcePal;
    struct SpritePalette palette;
    struct SpriteSheet sheet;
    struct SpriteTemplate spriteTemplate;
    u16 resourceTag;
    u8 spriteId;

    if (graphicsId == QUEST_OW_NONE)
        return FALSE;

    graphics = GetEventObjectGraphicsInfo(graphicsId);
    if (graphics == NULL)
        return FALSE;

    if (graphics->images == NULL || graphics->images[0].data == NULL)
        return FALSE;

    resourceTag = GetQuestOverworldResourceTag(graphics);
    if (resourceTag == 0)
        return FALSE;

    sourcePal = GetQuestOverworldPalette(graphics->paletteTag1);
    if (sourcePal == NULL)
        return FALSE;

    sheet.data = graphics->images[0].data;
    sheet.size = graphics->size;
    sheet.tag = resourceTag;

    palette.data = sourcePal->data;
    palette.tag = resourceTag;

    LoadSpriteSheet(&sheet);
    LoadSpritePalette(&palette);

    spriteTemplate.tileTag = resourceTag;
    spriteTemplate.paletteTag = resourceTag;
    spriteTemplate.oam = graphics->oam;
    spriteTemplate.anims = gDummySpriteAnimTable;
    spriteTemplate.images = graphics->images;
    spriteTemplate.affineAnims = gDummySpriteAffineAnimTable;
    spriteTemplate.callback = SpriteCallbackDummy;

    spriteId = CreateSprite(
        &spriteTemplate,
        QUEST_OW_SPRITE_X,
        QUEST_OW_SPRITE_Y,
        0
    );

    if (spriteId >= MAX_SPRITES)
    {
        FreeSpriteTilesByTag(resourceTag);
        FreeSpritePaletteByTag(resourceTag);
        return FALSE;
    }

    gSprites[spriteId].data[7] = QUEST_OW_SPRITE_MARKER;

    sQuestOverworldSpriteId = spriteId;
    sQuestOverworldGraphicsId = graphicsId;

    return TRUE;
}

static void UpdateQuestOverworldSprite(const struct QuestInfo *quest)
{
    struct QuestStateInfo state;

    if (quest == NULL)
    {
        DestroyQuestOverworldSprite();
        return;
    }

    GetQuestStates(quest, &state);

    /*
     * Locked quest = no overworld sprite.
     */
    if (!state.isUnlocked)
    {
        DestroyQuestOverworldSprite();
        return;
    }

    /*
     * No sprite assigned to this quest.
     */
    if (quest->graphicsId == QUEST_OW_NONE)
    {
        DestroyQuestOverworldSprite();
        return;
    }

    /*
     * Same sprite is already displayed.
     */
    if (sQuestOverworldSpriteId < MAX_SPRITES
        && sQuestOverworldGraphicsId == quest->graphicsId)
    {
        return;
    }

    /*
     * Different quest selected.
     * Remove previous sprite first.
     */
    DestroyQuestOverworldSprite();

    CreateQuestOverworldSprite(quest->graphicsId);
}

// Renders the list of quest items on the active page window
static void PrintMenuItems(const u8 *sortedIndices, u8 totalQuests)
{
    u8 firstVisibleItem = gMenuStruct->firstVisibleItem;
    u8 cursorPos = gMenuStruct->cursorPos;
    u8 selectedItem = gMenuStruct->selectedItem;
    u8 currentPage = gMenuStruct->page;

    CleanWindow(WINDOW_ITEMS);

    if (totalQuests == 0)
    {
        CommitWindow(WINDOW_ITEMS);
        return;
    }

    const struct QuestInfo *table = gQuestTables[currentPage];
    u8 itemText[32];

    for (u8 slotIndex = 0; slotIndex < VISIBLE_ITEMS; slotIndex++)
    {
        u8 renderedIndex = firstVisibleItem + slotIndex;
        if (renderedIndex >= totalQuests)
            break;

        const struct QuestInfo *quest = &table[sortedIndices[renderedIndex]];
        struct QuestStateInfo state;
        GetQuestStates(quest, &state);

        if (!state.isUnlocked)
        {
            const struct TextColor *lockColor =
                (slotIndex == cursorPos && renderedIndex == selectedItem)
                ? COLOR_MENU_SELECTED_ITEM
                : COLOR_MENU_ITEM;

            WindowPrint(WINDOW_ITEMS, FONT_SIZE_SMALL, 4, slotIndex * 16, lockColor, 0, sText_Dash);
                /*
            * Selected quest is locked.
            * Make sure its overworld sprite is removed.
            */
            if (slotIndex == cursorPos && renderedIndex == selectedItem)
                DestroyQuestOverworldSprite();

            continue;
        }

        if (slotIndex == cursorPos && renderedIndex == selectedItem)
        {
            StringCopy(itemText, sText_Arrow);
            StringAppend(itemText, quest->name);
            WindowPrint(WINDOW_ITEMS, FONT_SIZE_SMALL, 4, slotIndex * 16, COLOR_MENU_SELECTED_ITEM, 0, itemText);
        }
        else
        {
            WindowPrint(WINDOW_ITEMS, FONT_SIZE_SMALL, 4, slotIndex * 16, COLOR_MENU_ITEM, 0, quest->name);
        }

        if (state.isDone)
        {
            if (state.isRewarded)
            {
                WindowPrint(WINDOW_ITEMS, FONT_SIZE_SMALL, 124, slotIndex * 16,
                            COLOR_MENU_DONE_COLOR, 0, gText_QuestDone);
            }
            else
            {
                WindowPrint(WINDOW_ITEMS, FONT_SIZE_SMALL, 124, slotIndex * 16,
                            COLOR_MENU_INFO_COLOR, 0, gText_QuestGift);
            }
        }
    }

    CommitWindow(WINDOW_ITEMS);
}

// Renders the description text for the currently selected quest
static void PrintMenuItemDescription(const struct QuestInfo *quest)
{
    CleanWindow(WINDOW_DESCRIPTION);

    if (quest != NULL)
    {
        struct QuestStateInfo state;
        GetQuestStates(quest, &state);

        const u8 *description = state.isUnlocked ? quest->description : gText_ItemDescriptionNotAvailable;
        WindowPrint(WINDOW_DESCRIPTION, FONT_SIZE_SMALL, 3, 1, COLOR_DESCRIPTION, 0, description);
    }

    CommitWindow(WINDOW_DESCRIPTION);
}

// Renders the title header for the active menu page
static void PrintMenuTitle(void)
{
    CleanWindow(WINDOW_TITLE);

    u8 pageId = gMenuStruct->page;

    WindowPrint(
        WINDOW_TITLE,
        FONT_SIZE_SMALL,
        3,
        0,
        COLOR_MENU_TITLE,
        0,
        sMenuTitles[pageId]
    );

    CommitWindow(WINDOW_TITLE);
}

// Renders control hints, progress counters, and statistics in the sidebar window
static void PrintMenuControls(const struct PageStats *stats)
{
    if (FLAG_HIDE_QUEST_STATS != 0 && FlagGet(FLAG_HIDE_QUEST_STATS))
    {
        CleanWindow(WINDOW_CONTROLS);
        CommitWindow(WINDOW_CONTROLS);
        return;
    }

    bool8 hasUnclaimedReward = stats->completed > stats->rewarded;
    u8 countBuffer[16];
    u8 countText[32];

    u8 percentage = (stats->total > 0) ? (stats->rewarded * 100) / stats->total : 0;

    ConvertIntToDecimalStringN(countBuffer, stats->rewarded, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(countText, countBuffer);
    StringAppend(countText, sText_Slash);

    ConvertIntToDecimalStringN(countBuffer, stats->unlocked, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(countText, countBuffer);
    StringAppend(countText, sText_Space);

    ConvertIntToDecimalStringN(countBuffer, percentage, STR_CONV_MODE_LEFT_ALIGN, 3);
    StringAppend(countText, countBuffer);
    StringAppend(countText, sText_Percent);

    CleanWindow(WINDOW_CONTROLS);

    WindowPrint(WINDOW_CONTROLS, FONT_SIZE_SMALL, 0, 0, COLOR_MENU_TITLE, 0, countText);

    if (BuildSortedPageIndices(NULL) > 1)
    {
        WindowPrint(WINDOW_CONTROLS, FONT_SIZE_SMALL, 0, 12, COLOR_MENU_TITLE, 0, gText_LText);
        WindowPrint(WINDOW_CONTROLS, FONT_SIZE_SMALL, 0, 24, COLOR_MENU_TITLE, 0, gText_RText);
    }

    if (hasUnclaimedReward)
        WindowPrint(WINDOW_CONTROLS, FONT_SIZE_SMALL, 0, 36, COLOR_MENU_TITLE, 0, gText_GetText);

    WindowPrint(WINDOW_CONTROLS, FONT_SIZE_SMALL, 0, 48, COLOR_MENU_TITLE, 0, gText_CloseText);

    CommitWindow(WINDOW_CONTROLS);
}

// Coordinates rendering of all major GUI components on the screen
static void PrintMenuGUI(void)
{
    u8 currentPage = gMenuStruct->page;
    u8 totalQuests = GetQuestCountForPage(currentPage);
    
    u8 sortedIndices[totalQuests];
    struct PageStats stats;

    BuildSortedQuestIndices(currentPage, sortedIndices, &stats);

    const struct QuestInfo *selectedQuest =
        (totalQuests > 0)
        ? GetSelectedQuest(sortedIndices, totalQuests)
        : NULL;

    PrintMenuTitle();
    PrintMenuItems(sortedIndices, totalQuests);
    PrintMenuItemDescription(selectedQuest);
    PrintMenuControls(&stats);

    /*
    * Update overworld sprite for currently selected quest.
    */
    UpdateQuestOverworldSprite(selectedQuest);
}

// Public entry point function to enable script context and launch the quest menu interface
void ShowQuestMenu(void)
{
    ScriptContext2_Enable();

    gMain.state = MENU_STATE_INIT;
    SetMainCallback2(CB2_QuestMenu);
}

#endif