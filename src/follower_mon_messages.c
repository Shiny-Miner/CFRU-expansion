#include "defines.h"
#include "../include/bike.h"
#include "../include/field_player_avatar.h"
#include "../include/fieldmap.h"
#include "../include/field_message_box.h"
#include "../include/field_weather.h"
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

#include "../include/party_menu.h"
#include "../include/constants/field_effects.h"

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
#define SITUATIONAL_CHANCE 65
#define MAX_POOLS 32
#define EMOTE_NONE 0xFF

#define FM_LAST_TEXT_A (*(vu16*) 0x203E0CA)
#define FM_LAST_TEXT_B (*(vu16*) 0x203E0CC)

struct TextPool
{
    const u8* const* texts;
    u8 count;
    u8 emote;
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

static void AddPool(struct TextPool* pools, u8* count, const u8* const* texts, u8 size, u8 emote, u8 emoteChance)
{
    if (*count >= MAX_POOLS || size == 0)
        return;

    pools[*count].texts = texts;
    pools[*count].count = size;
    pools[*count].emote = emote;
    pools[*count].emoteChance = emoteChance;
    (*count)++;
}

#define ADD_POOL(arr, emote, chance) AddPool(pools, &count, arr, ARRAY_COUNT(arr), emote, chance)

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

    if (status & STATUS1_SLEEP)
        ADD_POOL(sPool_Asleep, FLDEFF_X_ICON, 60);
    else if (status & STATUS_PSN_ANY)
        ADD_POOL(sPool_Poison, FLDEFF_X_ICON, 60);
    else if (status & STATUS1_PARALYSIS)
        ADD_POOL(sPool_Paralyzed, FLDEFF_X_ICON, 60);
    else if (status & STATUS1_BURN)
        ADD_POOL(sPool_Burned, FLDEFF_X_ICON, 60);
    else if (status & STATUS1_FREEZE)
        ADD_POOL(sPool_Frozen, FLDEFF_X_ICON, 60);

    if (hp != 0 && maxHp != 0 && (hp * 100) / maxHp <= LOW_HP_PERCENT)
        ADD_POOL(sPool_LowHp, FLDEFF_X_ICON, 50);

    if (happiness >= HIGH_HAPPINESS)
        ADD_POOL(sPool_Happy, FLDEFF_SMILEY_FACE_ICON, 60);
    else if (happiness <= LOW_HAPPINESS)
        ADD_POOL(sPool_Lonely, FLDEFF_QUESTION_MARK_ICON, 40);

    switch (gWeatherPtr->currWeather)
    {
        case WEATHER_RAIN_LIGHT:
        case WEATHER_RAIN_MED:
        case WEATHER_RAIN_HEAVY:
            ADD_POOL(sPool_Rain, EMOTE_NONE, 0);
            break;
        case WEATHER_SNOW:
            ADD_POOL(sPool_Snow, EMOTE_NONE, 0);
            break;
        case WEATHER_SANDSTORM:
            ADD_POOL(sPool_Sand, EMOTE_NONE, 0);
            break;
        case WEATHER_FOG_1:
        case WEATHER_FOG_2:
        case WEATHER_FOG_3:
            ADD_POOL(sPool_Fog, FLDEFF_QUESTION_MARK_ICON, 30);
            break;
        case WEATHER_SUNNY:
        case WEATHER_DROUGHT:
            if (IsOnlyDayTime())
                ADD_POOL(sPool_Sunny, EMOTE_NONE, 0);
            break;
    }

    if (IsMorning())
        ADD_POOL(sPool_Morning, EMOTE_NONE, 0);
    else if (IsEvening())
        ADD_POOL(sPool_Evening, EMOTE_NONE, 0);
    else if (IsNightTime())
        ADD_POOL(sPool_Night, EMOTE_NONE, 0);
    else
        ADD_POOL(sPool_Day, EMOTE_NONE, 0);

    switch (gMapHeader.mapType)
    {
        case MAP_TYPE_UNDERGROUND:
            ADD_POOL(sPool_Cave, EMOTE_NONE, 0);
            break;
        case MAP_TYPE_INDOOR:
            ADD_POOL(sPool_Indoor, EMOTE_NONE, 0);
            break;
        case MAP_TYPE_TOWN:
        case MAP_TYPE_CITY:
            ADD_POOL(sPool_Town, EMOTE_NONE, 0);
            break;
        case MAP_TYPE_ROUTE:
            ADD_POOL(sPool_Route, FLDEFF_QUESTION_MARK_ICON, 20);
            break;
    }

    if (GetRandomPartyBuddy(mon) != NULL)
        ADD_POOL(sPool_Buddy, FLDEFF_SMILEY_FACE_ICON, 30);

    if (IsMonShiny(mon))
    {
        ADD_POOL(sPool_Shiny, FLDEFF_DOUBLE_EXCL_MARK_ICON, 70);
        ADD_POOL(sPool_Shiny, FLDEFF_DOUBLE_EXCL_MARK_ICON, 70);
    }

    if (level >= HIGH_LEVEL)
        ADD_POOL(sPool_LevelHigh, EMOTE_NONE, 0);
    else if (level <= LOW_LEVEL)
        ADD_POOL(sPool_LevelLow, FLDEFF_EXCLAMATION_MARK_ICON, 20);

    if (GetMonData(mon, MON_DATA_OT_ID, NULL) != GetPlayerTrainerId())
        ADD_POOL(sPool_OtherTrainer, FLDEFF_QUESTION_MARK_ICON, 30);

    if (GetMonData(mon, MON_DATA_ATK_IV, NULL) >= HIGH_IV || GetMonData(mon, MON_DATA_DEF_IV, NULL) >= HIGH_IV)
        ADD_POOL(sPool_HighIv, EMOTE_NONE, 0);

    if (heldItem == ITEM_LEFTOVERS)
        ADD_POOL(sPool_Leftovers, EMOTE_NONE, 0);
    else if (heldItem != ITEM_NONE)
        ADD_POOL(sPool_HeldItem, EMOTE_NONE, 0);

    if (ballType == ITEM_LUXURY_BALL)
        ADD_POOL(sPool_LuxuryBall, FLDEFF_SMILEY_FACE_ICON, 40);

    if (metLocation != 0)
        ADD_POOL(sPool_MetPlace, FLDEFF_QUESTION_MARK_ICON, 30);

    if (nature < ARRAY_COUNT(sNatureTexts))
        AddPool(pools, &count, sNatureTexts[nature], 2, EMOTE_NONE, 0);

    if (type < ARRAY_COUNT(sTypeTexts) && sTypeTexts[type][0] != NULL)
        AddPool(pools, &count, sTypeTexts[type], 2, EMOTE_NONE, 0);

    if (eggGroup < ARRAY_COUNT(sEggGroupTexts) && sEggGroupTexts[eggGroup].texts != NULL)
        AddPool(pools, &count, sEggGroupTexts[eggGroup].texts, sEggGroupTexts[eggGroup].count, EMOTE_NONE, 0);

    *outCount = count;
}

void ShowAnonymousFollowerMessage(void)
{
    struct TextPool pools[MAX_POOLS];
    struct TextPool rarePool = {sPool_Rare, ARRAY_COUNT(sPool_Rare), FLDEFF_SMILEY_FACE_ICON, 80};
    struct TextPool idlePool = {sPool_Idle, ARRAY_COUNT(sPool_Idle), FLDEFF_QUESTION_MARK_ICON, 10};
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
            Var8005 = FLDEFF_EXCLAMATION_MARK_ICON;
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
            speciesPool.emote = FLDEFF_EXCLAMATION_MARK_ICON;
            speciesPool.emoteChance = 25;
            if (count < MAX_POOLS - 1)
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
        Var8005 = pool->emote;

    StringExpandPlaceholders(gStringVar4, text);
}
