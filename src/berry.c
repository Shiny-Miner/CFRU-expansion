#include "../include/berry.h"
#include "../include/field_camera.h"
#include "../include/event_object_movement.h"
#include "../include/event_data.h"
#include "../include/item.h"
#include "../include/script.h"
#include "../include/new/Vanilla_functions.h"

extern BerryPrototype gBerries[];
extern struct EventObject gEventObjects[NUM_FIELD_OBJECTS];
extern struct MapHeader gMapHeader;

#define GET_MAP_NUM(map) ((map) & 0xFF)
#define GET_MAP_GROUP(map) ((map) >> 8)

enum
{
    BERRY_NOT_TAKEN,
    BERRY_TAKEN,
    OUT_OF_BAG_SPACE,
};

static bool32 IsDummyBerry(const BerryData *berry);
static void EraseBerryData(BerryData *berry);

// This function should be plugged inside PlayTimeCounter_Update().
void UpdateSaveBlockBerryData(void)
{
    const u32 max = ARRAY_COUNT(gSaveBlock2->berries);
    BerryData *berries = gSaveBlock2->berries;

    for (u32 i = 0; i < max; ++i)
    {
        if (IsDummyBerry(&berries[i]))
            continue;

        u8 berryId = berries[i].berryId;
        bool8 isLooping = gBerries[berryId].isLooping;

        if (berries[i].untilNextStage == 1)
        {
            if (berries[i].currentStage == GROWN_STAGE && !isLooping)
            {
                DestroyBerryHavingId(i);

                // Since a berry was destroyed, we decrement
                // so it correctly parses the next one.
                i--;
            }
            else
            {
                if (berries[i].currentStage == GROWN_STAGE && isLooping)
                    berries[i].currentStage = SPROUTING_STAGE;
                else
                    (berries[i].currentStage)++;

                berries[i].untilNextStage = gBerries[berryId].untilNextStage[berries[i].currentStage];
                UpdateBerryWithStage(&berries[i]);
            }
        }
        else
        {
            berries[i].untilNextStage--;
        }
    }
}

// This function just returns whether a berry is a dummy one.
static bool32 IsDummyBerry(const BerryData *berry)
{
    return berry->berryId == 0xFFFF;
}

// This function returns whether a passed BerryData* is within
// the current map or not.
static bool32 IsBerryInMap(const BerryData *berry)
{
    return (gSaveBlock1->location.mapGroup == GET_MAP_GROUP(berry->map)) && (gSaveBlock1->location.mapNum == GET_MAP_NUM(berry->map));
}

// This funcion "erases" a BerryData variable.
static void EraseBerryData(BerryData *berry)
{
    berry->berryId = 0xFFFF;
}

// This function destroys a berry at a specific index in the SaveBlock.
void DestroyBerryHavingId(const u32 idInSaveBlock)
{
    const u32 max = ARRAY_COUNT(gSaveBlock2->berries);
    BerryData *berries = gSaveBlock2->berries;

    if (idInSaveBlock >= max)
        return;

    u32 i;

    for (i = idInSaveBlock; i + 1 < max; ++i)
        berries[i] = berries[i + 1];

    EraseBerryData(&berries[i]);
}

// This function updates a berry's graphics according to
// the current stage.
void UpdateBerryWithStage(BerryData *berry)
{
    u16 newGraphics = gBerries[berry->berryId].objectEventIds[berry->currentStage];
    struct EventObjectTemplate backup;

    if (!IsBerryInMap(berry))
        return;

    backup = *GetEventObjectTemplateByLocalIdAndMap(berry->objectEventId, berry->map & 0xFF, berry->map >> 8);
    backup.graphicsIdLowerByte = newGraphics & 0xFF;
    backup.graphicsIdUpperByte = newGraphics >> 8;

    // Probably redundant ?
    backup.x = gEventObjects[berry->objectEventId].currentCoords.x;
    backup.y = gEventObjects[berry->objectEventId].currentCoords.y;

    RemoveEventObject(&gEventObjects[berry->objectEventId]);
    TrySpawnEventObjectTemplate(&backup, GET_MAP_NUM(berry->map), GET_MAP_GROUP(berry->map), backup.x, backup.y);
}

// This function returns the count of planted berries already.
u32 GetInUseBerriesCount(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(gSaveBlock2->berries); ++i)
        if (IsDummyBerry(gSaveBlock2->berries + i))
            break;

    return i;
}

// This function appends a brand new berry to the SaveBlock1
// array.
bool32 AppendNewBerry(BerryData *src)
{
    const u32 max = ARRAY_COUNT(gSaveBlock2->berries);
    BerryData *berries = gSaveBlock2->berries;
    u32 i = 0;

    for (i = 0; i < max; ++i)
    {
        if (IsDummyBerry(&berries[i]))
        {
            berries[i] = *src;
            return TRUE;
        }
    }

    return FALSE;
}

// This function should be called when interacting
// with a berry i.g.
// Should all their scripts call this function ?
// Or, alternatively, a metatile behavior ?
void TryToTakeBerryWithObjEventId(u32 objEventId)
{
    const u32 max = ARRAY_COUNT(gSaveBlock2->berries);
    BerryData *berries = gSaveBlock2->berries;
    u32 i;

    for (i = 0; i < max; ++i)
    {
        if (berries[i].objectEventId == objEventId && IsBerryInMap(&berries[i]))
        {
            if (!AddBagItem(gBerries[berries[i].berryId].berryItemId, 1))
                gSpecialVar_LastResult = OUT_OF_BAG_SPACE;
            else
                gSpecialVar_LastResult = BERRY_TAKEN;

            DestroyBerryHavingId(i);
            return;
        }
    }

    gSpecialVar_LastResult = BERRY_NOT_TAKEN;
}

extern const u8 BerrySystem_WouldYouLikeToPlant[];

// This function should also be called when interacting
// with a specific tile behavior.
void TryToPlantBerry(void)
{
    ScriptContext1_SetupScript(BerrySystem_WouldYouLikeToPlant);

    // The player doesn't want to plant a berry.
    if (gSpecialVar_LastResult == FALSE)
        return;
}

void GetFacedBerry(void)
{
    struct Coords16 coords;
    struct EventObject *player;

    coords.x = gSaveBlock1->location.x;
    coords.y = gSaveBlock1->location.y;
    player = &gEventObjects[gPlayerAvatar.eventObjectId];

    // Getting the possible berry in front of the player.
    switch (player->facingDirection)
    {
    case DIR_NORTH:
        coords.y--;
        break;
    case DIR_SOUTH:
        coords.y++;
        break;
    case DIR_WEST:
        coords.x--;
        break;
    case DIR_EAST:
        coords.x++;
        break;
    // In case the player's not facing anywhere ???
    default:
        goto Fail;
        return;
    }

    for (u32 i = 0; i < ARRAY_COUNT(gEventObjects); ++i)
    {
        struct EventObject *eventObject = &gEventObjects[i];

        if (eventObject->currentCoords.x != coords.x || eventObject->currentCoords.y != coords.y)
            continue;

        gSpecialVar_LastResult = i;
        return;
    }

Fail:
    gSpecialVar_LastResult = 0xFFFF;
}