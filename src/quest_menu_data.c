#include "../include/global.h"
#include "../include/new/quest_menu.h"
#include "../include/constants/items.h"
#include "../include/constants/species.h"
#include "../include/constants/event_objects.h"

#ifdef SIDEQUEST_MENU

extern const u8 gText_SideQuestName_1[];
extern const u8 gText_SideQuestDesc_1[];
extern const u8 gText_SideQuestDoneDesc_1[];
extern const u8 gText_SideQuestMap_1[];
extern const u8 gText_SideQuestName_2[];
extern const u8 gText_SideQuestDesc_2[];
extern const u8 gText_SideQuestDoneDesc_2[];
extern const u8 gText_SideQuestMap_2[];
extern const u8 gText_SideQuestName_3[];
extern const u8 gText_SideQuestDesc_3[];
extern const u8 gText_SideQuestDoneDesc_3[];
extern const u8 gText_SideQuestMap_3[];
extern const u8 gText_SideQuestName_4[];
extern const u8 gText_SideQuestDesc_4[];
extern const u8 gText_SideQuestDoneDesc_4[];
extern const u8 gText_SideQuestMap_4[];
extern const u8 gText_SideQuestName_5[];
extern const u8 gText_SideQuestDesc_5[];
extern const u8 gText_SideQuestDoneDesc_5[];
extern const u8 gText_SideQuestMap_5[];
extern const u8 gText_SideQuestName_6[];
extern const u8 gText_SideQuestDesc_6[];
extern const u8 gText_SideQuestDoneDesc_6[];
extern const u8 gText_SideQuestMap_6[];
extern const u8 gText_SideQuestName_7[];
extern const u8 gText_SideQuestDesc_7[];
extern const u8 gText_SideQuestDoneDesc_7[];
extern const u8 gText_SideQuestMap_7[];
extern const u8 gText_SideQuestName_8[];
extern const u8 gText_SideQuestDesc_8[];
extern const u8 gText_SideQuestDoneDesc_8[];
extern const u8 gText_SideQuestMap_8[];
extern const u8 gText_SideQuestName_9[];
extern const u8 gText_SideQuestDesc_9[];
extern const u8 gText_SideQuestDoneDesc_9[];
extern const u8 gText_SideQuestMap_9[];
extern const u8 gText_SideQuestName_10[];
extern const u8 gText_SideQuestDesc_10[];
extern const u8 gText_SideQuestDoneDesc_10[];
extern const u8 gText_SideQuestMap_10[];
extern const u8 gText_SideQuestName_11[];
extern const u8 gText_SideQuestDesc_11[];
extern const u8 gText_SideQuestDoneDesc_11[];
extern const u8 gText_SideQuestMap_11[];
extern const u8 gText_SideQuestName_12[];
extern const u8 gText_SideQuestDesc_12[];
extern const u8 gText_SideQuestDoneDesc_12[];
extern const u8 gText_SideQuestMap_12[];
extern const u8 gText_SideQuestName_13[];
extern const u8 gText_SideQuestDesc_13[];
extern const u8 gText_SideQuestDoneDesc_13[];
extern const u8 gText_SideQuestMap_13[];
extern const u8 gText_SideQuestName_14[];
extern const u8 gText_SideQuestDesc_14[];
extern const u8 gText_SideQuestDoneDesc_14[];
extern const u8 gText_SideQuestMap_14[];
extern const u8 gText_SideQuestName_15[];
extern const u8 gText_SideQuestDesc_15[];
extern const u8 gText_SideQuestDoneDesc_15[];
extern const u8 gText_SideQuestMap_15[];
extern const u8 gText_SideQuestName_16[];
extern const u8 gText_SideQuestDesc_16[];
extern const u8 gText_SideQuestDoneDesc_16[];
extern const u8 gText_SideQuestMap_16[];
extern const u8 gText_SideQuestName_17[];
extern const u8 gText_SideQuestDesc_17[];
extern const u8 gText_SideQuestDoneDesc_17[];
extern const u8 gText_SideQuestMap_17[];
extern const u8 gText_SideQuestName_18[];
extern const u8 gText_SideQuestDesc_18[];
extern const u8 gText_SideQuestDoneDesc_18[];
extern const u8 gText_SideQuestMap_18[];
extern const u8 gText_SideQuestName_19[];
extern const u8 gText_SideQuestDesc_19[];
extern const u8 gText_SideQuestDoneDesc_19[];
extern const u8 gText_SideQuestMap_19[];
extern const u8 gText_SideQuestName_20[];
extern const u8 gText_SideQuestDesc_20[];
extern const u8 gText_SideQuestDoneDesc_20[];
extern const u8 gText_SideQuestMap_20[];
extern const u8 gText_SideQuestName_21[];
extern const u8 gText_SideQuestDesc_21[];
extern const u8 gText_SideQuestDoneDesc_21[];
extern const u8 gText_SideQuestMap_21[];
extern const u8 gText_SideQuestName_22[];
extern const u8 gText_SideQuestDesc_22[];
extern const u8 gText_SideQuestDoneDesc_22[];
extern const u8 gText_SideQuestMap_22[];
extern const u8 gText_SideQuestName_23[];
extern const u8 gText_SideQuestDesc_23[];
extern const u8 gText_SideQuestDoneDesc_23[];
extern const u8 gText_SideQuestMap_23[];
extern const u8 gText_SideQuestName_24[];
extern const u8 gText_SideQuestDesc_24[];
extern const u8 gText_SideQuestDoneDesc_24[];
extern const u8 gText_SideQuestMap_24[];
extern const u8 gText_SideQuestName_25[];
extern const u8 gText_SideQuestDesc_25[];
extern const u8 gText_SideQuestDoneDesc_25[];
extern const u8 gText_SideQuestMap_25[];
extern const u8 gText_SideQuestName_26[];
extern const u8 gText_SideQuestDesc_26[];
extern const u8 gText_SideQuestDoneDesc_26[];
extern const u8 gText_SideQuestMap_26[];
extern const u8 gText_SideQuestName_27[];
extern const u8 gText_SideQuestDesc_27[];
extern const u8 gText_SideQuestDoneDesc_27[];
extern const u8 gText_SideQuestMap_27[];
extern const u8 gText_SideQuestName_28[];
extern const u8 gText_SideQuestDesc_28[];
extern const u8 gText_SideQuestDoneDesc_28[];
extern const u8 gText_SideQuestMap_28[];
extern const u8 gText_SideQuestName_29[];
extern const u8 gText_SideQuestDesc_29[];
extern const u8 gText_SideQuestDoneDesc_29[];
extern const u8 gText_SideQuestMap_29[];
extern const u8 gText_SideQuestName_30[];
extern const u8 gText_SideQuestDesc_30[];
extern const u8 gText_SideQuestDoneDesc_30[];
extern const u8 gText_SideQuestMap_30[];
extern const u8 gText_SubQuest1_Name1[];
extern const u8 gText_SubQuest1_Desc1[];
extern const u8 gText_SubQuest1_Map1[];
extern const u8 gText_SubQuest1_Name2[];
extern const u8 gText_SubQuest1_Desc2[];
extern const u8 gText_SubQuest1_Map2[];
extern const u8 gText_SubQuest1_Name3[];
extern const u8 gText_SubQuest1_Desc3[];
extern const u8 gText_SubQuest1_Map3[];
extern const u8 gText_SubQuest1_Name4[];
extern const u8 gText_SubQuest1_Desc4[];
extern const u8 gText_SubQuest1_Map4[];
extern const u8 gText_SubQuest1_Name5[];
extern const u8 gText_SubQuest1_Desc5[];
extern const u8 gText_SubQuest1_Map5[];
extern const u8 gText_SubQuest1_Name6[];
extern const u8 gText_SubQuest1_Desc6[];
extern const u8 gText_SubQuest1_Map6[];
extern const u8 gText_SubQuest1_Name7[];
extern const u8 gText_SubQuest1_Desc7[];
extern const u8 gText_SubQuest1_Map7[];
extern const u8 gText_SubQuest1_Name8[];
extern const u8 gText_SubQuest1_Desc8[];
extern const u8 gText_SubQuest1_Map8[];
extern const u8 gText_SubQuest1_Name9[];
extern const u8 gText_SubQuest1_Desc9[];
extern const u8 gText_SubQuest1_Map9[];
extern const u8 gText_SubQuest1_Name10[];
extern const u8 gText_SubQuest1_Desc10[];
extern const u8 gText_SubQuest1_Map10[];
extern const u8 gText_SubQuest2_Name1[];
extern const u8 gText_SubQuest2_Desc1[];
extern const u8 gText_SubQuest2_Map1[];
extern const u8 gText_SubQuest2_Name2[];
extern const u8 gText_SubQuest2_Desc2[];
extern const u8 gText_SubQuest2_Map2[];
extern const u8 gText_SubQuest2_Name3[];
extern const u8 gText_SubQuest2_Desc3[];
extern const u8 gText_SubQuest2_Map3[];
extern const u8 gText_SubQuest2_Name4[];
extern const u8 gText_SubQuest2_Desc4[];
extern const u8 gText_SubQuest2_Map4[];
extern const u8 gText_SubQuest2_Name5[];
extern const u8 gText_SubQuest2_Desc5[];
extern const u8 gText_SubQuest2_Map5[];
extern const u8 gText_SubQuest2_Name6[];
extern const u8 gText_SubQuest2_Desc6[];
extern const u8 gText_SubQuest2_Map6[];
extern const u8 gText_SubQuest2_Name7[];
extern const u8 gText_SubQuest2_Desc7[];
extern const u8 gText_SubQuest2_Map7[];
extern const u8 gText_SubQuest2_Name8[];
extern const u8 gText_SubQuest2_Desc8[];
extern const u8 gText_SubQuest2_Map8[];

#define SIDE_QUEST(i, spr, type, subs, n) {gText_SideQuestName_##i, gText_SideQuestDesc_##i, gText_SideQuestDoneDesc_##i, gText_SideQuestMap_##i, spr, type, subs, n}
#define SUB_QUEST(sid, q, k, spr, type) {sid, gText_SubQuest##q##_Name##k, gText_SubQuest##q##_Desc##k, gText_SubQuest##q##_Map##k, spr, type, NULL}

// Sprite and type of an entry: QUEST_OW(table, EVENT_OBJ_GFX_x) with QUEST_ICON_OBJECT, an ITEM_x with QUEST_ICON_ITEM,
// a SPECIES_x with QUEST_ICON_PKMN, or QUEST_OW_NONE with 0 for no icon.

static const struct SubQuest sSubQuests1[QUEST_1_SUB_COUNT] =
{
    SUB_QUEST(0, 1, 1, SPECIES_PIDGEY, QUEST_ICON_PKMN),
    SUB_QUEST(1, 1, 2, SPECIES_RATTATA, QUEST_ICON_PKMN),
    SUB_QUEST(2, 1, 3, SPECIES_CATERPIE, QUEST_ICON_PKMN),
    SUB_QUEST(3, 1, 4, SPECIES_WEEDLE, QUEST_ICON_PKMN),
    SUB_QUEST(4, 1, 5, SPECIES_PIKACHU, QUEST_ICON_PKMN),
    SUB_QUEST(5, 1, 6, SPECIES_NIDORAN_F, QUEST_ICON_PKMN),
    SUB_QUEST(6, 1, 7, SPECIES_ZUBAT, QUEST_ICON_PKMN),
    SUB_QUEST(7, 1, 8, SPECIES_GEODUDE, QUEST_ICON_PKMN),
    SUB_QUEST(8, 1, 9, SPECIES_ODDISH, QUEST_ICON_PKMN),
    SUB_QUEST(9, 1, 10, SPECIES_CLEFAIRY, QUEST_ICON_PKMN),
};

static const struct SubQuest sSubQuests2[QUEST_2_SUB_COUNT] =
{
    SUB_QUEST(10, 2, 1, QUEST_OW(0, EVENT_OBJ_GFX_BROCK), QUEST_ICON_OBJECT),
    SUB_QUEST(11, 2, 2, QUEST_OW(0, EVENT_OBJ_GFX_MISTY), QUEST_ICON_OBJECT),
    SUB_QUEST(12, 2, 3, QUEST_OW(0, EVENT_OBJ_GFX_LT_SURGE), QUEST_ICON_OBJECT),
    SUB_QUEST(13, 2, 4, QUEST_OW(0, EVENT_OBJ_GFX_ERIKA), QUEST_ICON_OBJECT),
    SUB_QUEST(14, 2, 5, QUEST_OW(0, EVENT_OBJ_GFX_KOGA), QUEST_ICON_OBJECT),
    SUB_QUEST(15, 2, 6, QUEST_OW(0, EVENT_OBJ_GFX_SABRINA), QUEST_ICON_OBJECT),
    SUB_QUEST(16, 2, 7, QUEST_OW(0, EVENT_OBJ_GFX_BLAINE), QUEST_ICON_OBJECT),
    SUB_QUEST(17, 2, 8, QUEST_OW(0, EVENT_OBJ_GFX_GIOVANNI), QUEST_ICON_OBJECT),
};

const struct SideQuest gSideQuests[QUEST_COUNT] =
{
    SIDE_QUEST(1, QUEST_OW(0, EVENT_OBJ_GFX_OAK), QUEST_ICON_OBJECT, sSubQuests1, QUEST_1_SUB_COUNT),
    SIDE_QUEST(2, QUEST_OW(0, EVENT_OBJ_GFX_BROCK), QUEST_ICON_OBJECT, sSubQuests2, QUEST_2_SUB_COUNT),
    SIDE_QUEST(3, ITEM_OAKS_PARCEL, QUEST_ICON_ITEM, NULL, 0),
    SIDE_QUEST(4, ITEM_HELIX_FOSSIL, QUEST_ICON_ITEM, NULL, 0),
    SIDE_QUEST(5, ITEM_SS_TICKET, QUEST_ICON_ITEM, NULL, 0),
    SIDE_QUEST(6, QUEST_OW(0, EVENT_OBJ_GFX_ROCKET_GRUNT_M), QUEST_ICON_OBJECT, NULL, 0),
    SIDE_QUEST(7, SPECIES_ZAPDOS, QUEST_ICON_PKMN, NULL, 0),
    SIDE_QUEST(8, SPECIES_EEVEE, QUEST_ICON_PKMN, NULL, 0),
    SIDE_QUEST(9, SPECIES_SNORLAX, QUEST_ICON_PKMN, NULL, 0),
    SIDE_QUEST(10, ITEM_GOLD_TEETH, QUEST_ICON_ITEM, NULL, 0),
    SIDE_QUEST(11, ITEM_SUPER_ROD, QUEST_ICON_ITEM, NULL, 0),
    SIDE_QUEST(12, SPECIES_MEWTWO, QUEST_ICON_PKMN, NULL, 0),
    SIDE_QUEST(13, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(14, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(15, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(16, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(17, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(18, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(19, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(20, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(21, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(22, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(23, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(24, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(25, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(26, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(27, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(28, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(29, QUEST_OW_NONE, 0, NULL, 0),
    SIDE_QUEST(30, QUEST_OW_NONE, 0, NULL, 0),
};

#endif
