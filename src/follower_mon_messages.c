#include "defines.h"
#include "../include/bike.h"
#include "../include/field_player_avatar.h"
#include "../include/fieldmap.h"
#include "../include/field_message_box.h"
#include "../include/field_weather.h"
#include "../include/party_menu.h"
#include "../include/constants/field_effects.h"
#include "../include/new/follower_emotes.h"
#include "../include/new/exp.h"
#include "../include/gpu_regs.h"
#include "../include/item_icon.h"
#include "../include/item_menu.h"
#include "../include/list_menu.h"
#include "../include/map_name_popup.h"
#include "../include/menu.h"
#include "../include/m4a.h"
#include "../include/naming_screen.h"
#include "../include/overworld.h"
#include "../include/pokemon_summary_screen.h"
#include "../include/pokemon_storage_system.h"
#include "../include/region_map.h"
#include "../include/script.h"
#include "../include/script_menu.h"
#include "../include/sound.h"
#include "../include/sprite.h"
#include "../include/string_util.h"
#include "../include/text.h"
#include "../include/wild_encounter.h"
#include "../include/window.h"
#include "../include/event_data.h"
#include "../include/constants/abilities.h"
#include "../include/constants/items.h"
#include "../include/constants/moves.h"
#include "../include/constants/pokedex.h"
#include "../include/constants/pokemon.h"
#include "../include/constants/songs.h"

#include "../include/new/battle_strings.h"
#include "../include/new/build_pokemon.h"
#include "../include/new/catching.h"
#include "../include/new/damage_calc.h"
#include "../include/new/dns.h"
#include "../include/new/util.h"
#include "../include/new/item.h"
#include "../include/new/learn_move.h"
#include "../include/new_menu_helpers.h"
#include "../include/new/multi.h"
#include "../include/new/overworld.h"
#include "../include/new/pokemon_storage_system.h"
#include "../include/new/ram_locs_battle.h"
#include "../include/new/read_keys.h"
#include "../include/new/roamer.h"
#include "../include/new/text.h"
#include "../include/new/scrolling_multichoice.h"
#include "../include/new/Vanilla_functions_battle.h"
#include "../include/new/wild_encounter.h"
#include "../include/random.h"

#define LOW_HAPPINESS 70
#define HIGH_HAPPINESS 200
#define LOW_HP_PERCENT 20
#define HIGH_LEVEL 70
#define LOW_LEVEL 12
#define HIGH_IV 25
#define ITEM_FIND_CHANCE 3
#define RARE_TEXT_CHANCE 2
#define SITUATIONAL_CHANCE 75
#define MAX_POOLS 40
#define EMOTE_NONE 0xFF

#define FM_LAST_TEXT_A (*(vu16*) 0x203E0CA)
#define FM_LAST_TEXT_B (*(vu16*) 0x203E0CC)

struct TextPool
{
    const u8* const* texts;
    u8 count;
    u8 emote;
    u8 emote2;
    u8 emoteChance;
};

#include "follower_mon_text_pools.h"

extern const u8 sText_FoundItem[];
extern struct Pokemon* GetFirstValidPartyMon(void);
extern struct Pokemon* GetFollowerPartyMon(void);
extern u32 GetPlayerTrainerId(void);

void ShowFollowerMessage(const u8* message)
{
    StringExpandPlaceholders(gStringVar4, message);
    ShowFieldMessage(gStringVar4);
}

#define E_NONE EMOTE_NONE
#define E_EXCL FLDEFF_EXCLAMATION_MARK_ICON
#define E_DEXCL FLDEFF_DOUBLE_EXCL_MARK_ICON
#define E_QUESTION FLDEFF_QUESTION_MARK_ICON
#define E_SMILE FLDEFF_SMILEY_FACE_ICON
#define E_X FLDEFF_X_ICON
#define E_THINK FOLLOWER_EMOTE_THINKING
#define E_HEART FOLLOWER_EMOTE_HEART
#define E_SAD FOLLOWER_EMOTE_SAD

static void AddPool(struct TextPool* pools, u8* count, const u8* const* texts, u8 size, u8 emote, u8 emote2, u8 emoteChance)
{
    if (*count >= MAX_POOLS || size == 0)
        return;

    pools[*count].texts = texts;
    pools[*count].count = size;
    pools[*count].emote = emote;
    pools[*count].emote2 = emote2;
    pools[*count].emoteChance = emoteChance;
    (*count)++;
}

#define ADD_POOL(arr, e1, e2, chance) AddPool(pools, &count, arr, ARRAY_COUNT(arr), e1, e2, chance)
#define ADD_POOL_TWICE(arr, e1, e2, chance) do { ADD_POOL(arr, e1, e2, chance); ADD_POOL(arr, e1, e2, chance); } while (0)

static const u8* PickText(const struct TextPool* pool)
{
    const u8* text = NULL;

    for (u8 tries = 0; tries < 6; tries++)
    {
        text = pool->texts[Random() % pool->count];
        u16 id = (u16) (u32) text;
        if (id != FM_LAST_TEXT_A && id != FM_LAST_TEXT_B)
            break;
    }

    FM_LAST_TEXT_B = FM_LAST_TEXT_A;
    FM_LAST_TEXT_A = (u16) (u32) text;
    return text;
}

static struct Pokemon* GetRandomPartyBuddy(struct Pokemon* follower)
{
    struct Pokemon* buddies[PARTY_SIZE];
    u8 count = 0;

    for (u8 i = 0; i < PARTY_SIZE; ++i)
    {
        struct Pokemon* mon = &gPlayerParty[i];

        if (mon != follower
        && GetMonData(mon, MON_DATA_SPECIES, NULL) != SPECIES_NONE
        && !GetMonData(mon, MON_DATA_IS_EGG, NULL))
            buddies[count++] = mon;
    }

    return count == 0 ? NULL : buddies[Random() % count];
}

static void BuildSituationalPools(struct Pokemon* mon, struct TextPool* pools, u8* outCount)
{
    u8 count = 0;
    u8 partySize = 0;
    bool8 hasEgg = FALSE;
    u8 badges = GetBadgeCount();
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
    u8 happiness = GetMonData(mon, MON_DATA_FRIENDSHIP, NULL);
    u32 hp = GetMonData(mon, MON_DATA_HP, NULL);
    u32 maxHp = GetMonData(mon, MON_DATA_MAX_HP, NULL);
    u32 status = GetMonData(mon, MON_DATA_STATUS, NULL);
    u8 level = GetMonData(mon, MON_DATA_LEVEL, NULL);
    u16 heldItem = GetMonData(mon, MON_DATA_HELD_ITEM, NULL);
    u8 ballType = GetMonData(mon, MON_DATA_POKEBALL, NULL);
    u8 metLocation = GetMonData(mon, MON_DATA_MET_LOCATION, NULL);
    u8 nature = GetNature(mon);
    u8 eggGroup = gBaseStats[species].eggGroup1;
    u8 type = gBaseStats[species].type1;

    for (u8 i = 0; i < PARTY_SIZE; ++i)
    {
        if (GetMonData(&gPlayerParty[i], MON_DATA_SPECIES, NULL) == SPECIES_NONE)
            continue;

        partySize++;
        if (GetMonData(&gPlayerParty[i], MON_DATA_IS_EGG, NULL))
            hasEgg = TRUE;
    }

    if (status & STATUS1_SLEEP)
        ADD_POOL_TWICE(sPool_Asleep, E_X, E_SAD, 100);
    else if (status & STATUS_PSN_ANY)
        ADD_POOL_TWICE(sPool_Poison, E_X, E_SAD, 100);
    else if (status & STATUS1_PARALYSIS)
        ADD_POOL_TWICE(sPool_Paralyzed, E_X, E_SAD, 100);
    else if (status & STATUS1_BURN)
        ADD_POOL_TWICE(sPool_Burned, E_X, E_SAD, 100);
    else if (status & STATUS1_FREEZE)
        ADD_POOL_TWICE(sPool_Frozen, E_X, E_SAD, 100);

    if (hp != 0 && maxHp != 0 && (hp * 100) / maxHp <= LOW_HP_PERCENT)
        ADD_POOL_TWICE(sPool_LowHp, E_SAD, E_X, 100);

    if (happiness >= HIGH_HAPPINESS)
        ADD_POOL_TWICE(sPool_Happy, E_HEART, E_SMILE, 100);
    else if (happiness <= LOW_HAPPINESS)
        ADD_POOL_TWICE(sPool_Lonely, E_SAD, E_QUESTION, 100);

    switch (gWeatherPtr->currWeather)
    {
        case WEATHER_RAIN_LIGHT:
        case WEATHER_RAIN_MED:
        case WEATHER_RAIN_HEAVY:
            ADD_POOL(sPool_Rain, E_SMILE, E_QUESTION, 30);
            break;
        case WEATHER_SNOW:
            ADD_POOL(sPool_Snow, E_SMILE, E_QUESTION, 30);
            break;
        case WEATHER_SANDSTORM:
            ADD_POOL(sPool_Sand, E_X, E_SAD, 40);
            break;
        case WEATHER_FOG_1:
        case WEATHER_FOG_2:
        case WEATHER_FOG_3:
            ADD_POOL(sPool_Fog, E_QUESTION, E_THINK, 60);
            break;
        case WEATHER_SUNNY:
        case WEATHER_DROUGHT:
            if (IsOnlyDayTime())
                ADD_POOL(sPool_Sunny, E_SMILE, E_HEART, 30);
            break;
    }

    if (IsMorning())
        ADD_POOL(sPool_Morning, E_SMILE, E_EXCL, 30);
    else if (IsEvening())
        ADD_POOL(sPool_Evening, E_THINK, E_HEART, 25);
    else if (IsNightTime())
        ADD_POOL(sPool_Night, E_THINK, E_QUESTION, 25);
    else
        ADD_POOL(sPool_Day, E_SMILE, E_NONE, 25);

    switch (gMapHeader.mapType)
    {
        case MAP_TYPE_UNDERGROUND:
            ADD_POOL(sPool_Cave, E_QUESTION, E_EXCL, 35);
            break;
        case MAP_TYPE_INDOOR:
            ADD_POOL(sPool_Indoor, E_QUESTION, E_THINK, 30);
            break;
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
            ADD_POOL(sPool_Town, E_QUESTION, E_SMILE, 30);
            break;
        case MAP_TYPE_ROUTE:
            ADD_POOL(sPool_Route, E_QUESTION, E_EXCL, 45);
            break;
    }

    if (partySize >= 2 && GetRandomPartyBuddy(mon) != NULL)
        ADD_POOL_TWICE(sPool_Buddy, E_HEART, E_SMILE, 80);

    if (partySize == 1)
        ADD_POOL(sPool_Solo, E_SMILE, E_THINK, 50);
    else if (partySize == PARTY_SIZE)
        ADD_POOL(sPool_FullParty, E_SMILE, E_EXCL, 40);

    if (hasEgg)
        ADD_POOL_TWICE(sPool_EggInParty, E_QUESTION, E_HEART, 70);

    if (badges == 0)
        ADD_POOL(sPool_BadgesNone, E_EXCL, E_SMILE, 40);
    else if (badges < 4)
        ADD_POOL(sPool_BadgesFew, E_SMILE, E_THINK, 35);
    else if (badges < 8)
        ADD_POOL(sPool_BadgesMany, E_EXCL, E_THINK, 35);
    else
        ADD_POOL(sPool_BadgesAll, E_HEART, E_SMILE, 50);

    if (IsMonShiny(mon))
    {
        ADD_POOL(sPool_Shiny, E_DEXCL, E_HEART, 100);
        ADD_POOL(sPool_Shiny, E_DEXCL, E_HEART, 100);
    }

    if (level >= HIGH_LEVEL)
        ADD_POOL(sPool_LevelHigh, E_EXCL, E_NONE, 30);
    else if (level <= LOW_LEVEL)
        ADD_POOL(sPool_LevelLow, E_EXCL, E_SMILE, 60);

    if (GetMonData(mon, MON_DATA_OT_ID, NULL) != GetPlayerTrainerId())
        ADD_POOL(sPool_OtherTrainer, E_SAD, E_THINK, 70);

    if (GetMonData(mon, MON_DATA_ATK_IV, NULL) >= HIGH_IV || GetMonData(mon, MON_DATA_DEF_IV, NULL) >= HIGH_IV)
        ADD_POOL(sPool_HighIv, E_EXCL, E_SMILE, 40);

    if (heldItem == ITEM_LEFTOVERS)
        ADD_POOL(sPool_Leftovers, E_SMILE, E_NONE, 40);
    else if (heldItem != ITEM_NONE)
        ADD_POOL(sPool_HeldItem, E_QUESTION, E_EXCL, 40);

    if (ballType == ITEM_LUXURY_BALL)
        ADD_POOL(sPool_LuxuryBall, E_SMILE, E_HEART, 80);

    if (metLocation != 0)
        ADD_POOL(sPool_MetPlace, E_THINK, E_QUESTION, 70);

    if (nature < ARRAY_COUNT(sNatureTexts))
        AddPool(pools, &count, sNatureTexts[nature], 3, E_SMILE, E_NONE, 15);

    if (type < ARRAY_COUNT(sTypeTexts) && sTypeTexts[type][0] != NULL)
        AddPool(pools, &count, sTypeTexts[type], 3, E_SMILE, E_NONE, 15);

    if (eggGroup < ARRAY_COUNT(sEggGroupTexts) && sEggGroupTexts[eggGroup].texts != NULL)
        AddPool(pools, &count, sEggGroupTexts[eggGroup].texts, sEggGroupTexts[eggGroup].count, E_SMILE, E_NONE, 15);

    *outCount = count;
}

void ShowAnonymousFollowerMessage(void)
{
    struct TextPool pools[MAX_POOLS];
    struct TextPool rarePool = {sPool_Rare, ARRAY_COUNT(sPool_Rare), E_HEART, E_SMILE, 100};
    struct TextPool idlePool = {sPool_Idle, ARRAY_COUNT(sPool_Idle), E_THINK, E_QUESTION, 35};
    struct TextPool speciesPool;
    const u8* speciesTexts[4];
    const struct TextPool* pool = &idlePool;
    const u8* text = NULL;
    u8 count = 0;
    u8 speciesCount = 0;
    u8 roll = Random() % 100;
    struct Pokemon* mon = GetFollowerPartyMon();

    Var8005 = EMOTE_NONE;

    if (mon == NULL)
    {
        StringExpandPlaceholders(gStringVar4, sPool_Idle[Random() % ARRAY_COUNT(sPool_Idle)]);
        return;
    }

    if (roll < ITEM_FIND_CHANCE)
    {
        const u16 commonItems[] = {ITEM_POTION, ITEM_POKE_BALL, ITEM_ANTIDOTE, ITEM_PARALYZE_HEAL, ITEM_AWAKENING, ITEM_BURN_HEAL, ITEM_ICE_HEAL};
        u16 item = commonItems[Random() % ARRAY_COUNT(commonItems)];

        if (AddBagItem(item, 1))
        {
            CopyItemName(item, gStringVar2);
            Var8005 = E_EXCL;
            StringExpandPlaceholders(gStringVar4, sText_FoundItem);
            return;
        }
    }

    if (roll < ITEM_FIND_CHANCE + RARE_TEXT_CHANCE)
    {
        pool = &rarePool;
    }
    else if ((Random() % 100) < SITUATIONAL_CHANCE)
    {
        BuildSituationalPools(mon, pools, &count);

        u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);
        for (u8 i = 0; i < ARRAY_COUNT(sSpeciesTexts) && speciesCount < ARRAY_COUNT(speciesTexts); ++i)
        {
            if (sSpeciesTexts[i].species == species)
                speciesTexts[speciesCount++] = sSpeciesTexts[i].text;
        }

        if (speciesCount != 0)
        {
            speciesPool.texts = speciesTexts;
            speciesPool.count = speciesCount;
            speciesPool.emote = E_EXCL;
            speciesPool.emote2 = E_SMILE;
            speciesPool.emoteChance = 60;
            if (count < MAX_POOLS - 2)
            {
                pools[count++] = speciesPool;
                pools[count++] = speciesPool;
            }
        }

        if (count != 0)
            pool = &pools[Random() % count];
    }

    text = PickText(pool);

    if (pool->texts == sPool_Buddy)
        GetMonNickname(GetRandomPartyBuddy(mon), gStringVar2);
    else if (pool->texts == sPool_MetPlace)
        GetMapName(gStringVar2, GetMonData(mon, MON_DATA_MET_LOCATION, NULL), 0);

    if (pool->emote != EMOTE_NONE && (Random() % 100) < pool->emoteChance)
    {
        Var8005 = pool->emote;
        if (pool->emote2 != EMOTE_NONE && (Random() & 1))
            Var8005 = pool->emote2;
    }

    StringExpandPlaceholders(gStringVar4, text);
}
