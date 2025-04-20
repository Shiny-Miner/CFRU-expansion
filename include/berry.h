#ifndef GUARD_BERRY_H
#define GUARD_BERRY_H

#include "global.h"
#include "constants/items.h"

enum
{
    BERRY_FIRMNESS_UNKNOWN,
    BERRY_FIRMNESS_VERY_SOFT,
    BERRY_FIRMNESS_SOFT,
    BERRY_FIRMNESS_HARD,
    BERRY_FIRMNESS_VERY_HARD,
    BERRY_FIRMNESS_SUPER_HARD,
};

enum
{
    FLAVOR_SPICY,
    FLAVOR_DRY,
    FLAVOR_SWEET,
    FLAVOR_BITTER,
    FLAVOR_SOUR,
    FLAVOR_COUNT
};

#define NUM_BERRIES 67

enum
{
    SPROUTING_STAGE,
    MIDLIFE_STAGE,
    GROWN_STAGE,
    STAGES_COUNT,
};

/**
 * This struct will just be for the
 * const data. (i.e., the data in the ROM.)
 */
typedef struct
{
    /*0x00*/ u16 berryItemId;                  // The Berry's ID in the item table.
    /*0x02*/ u16 objectEventIds[STAGES_COUNT]; // Object event IDs for the stages of the Berry.
    /*0x08*/ u8 untilNextStage[STAGES_COUNT];  // Number of steps until a specific stage.
    /*0x0B*/ u8 isLooping;                     // Dictates whether this berry goes back to sprouting or not.
} BerryPrototype;                              // =0x0C

void UpdateSaveBlockBerryData(void);
void DestroyBerryHavingId(const u32 idInSaveBlock);
void UpdateBerryWithStage(BerryData *berry);
u32 GetInUseBerriesCount(void);

#endif // GUARD_BERRY_H