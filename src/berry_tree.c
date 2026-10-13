#include "defines.h"
#include "../include/random.h"

#define gBerryTrees (gSaveBlock2->berryTrees)
#define HOURS_PER_STAGE 12

static u8 DaysInMonth(u16 year, u8 month)
{
    static const u8 days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12)
        return 0;
    return days[month - 1] + (month == 2 && year % 4 == 0
        && (year % 100 != 0 || year % 400 == 0));
}

static s32 DateInDays(u16 year, u8 month, u8 day)
{
    s32 result = 365 * (year - 1) + (year - 1) / 4
        - (year - 1) / 100 + (year - 1) / 400;
    for (u8 i = 1; i < month; ++i)
        result += DaysInMonth(year, i);
    return result + day - 1;
}

void GetTreeData(void)
{
    // Clear outputs even for invalid IDs, so scripts cannot reuse another tree.
    Var8005 = Var8006 = Var8007 = Var8008 = 0;
    Var8009 = Var800A = Var800B = 0;
    if (Var8002 >= BERRY_TREE_COUNT)
        return;

    struct BerryTreeHack *tree = &gBerryTrees[Var8002];
    u8 stage = tree->stage >> 5;
    if (stage == 0 || stage > 5)
        return;

    Var8005 = stage;
    Var8006 = (tree->stage >> 1) & 0xF;
    Var8007 = tree->stage & 1;
    Var8008 = tree->berryId;
    Var8009 = tree->hourPlanted;
    Var800A = tree->monthPlanted;
    Var800B = tree->dayPlanted;

    u16 year = tree->yearPlanted ? 1999 + tree->yearPlanted : gClock.year;
    if (!tree->yearPlanted && tree->monthPlanted > gClock.month && year > 0)
        --year;
    if (year == 0 || tree->dayPlanted == 0
        || tree->dayPlanted > DaysInMonth(year, tree->monthPlanted)
        || tree->hourPlanted > 23 || gClock.year == 0 || gClock.hour > 23
        || gClock.day == 0 || gClock.day > DaysInMonth(gClock.year, gClock.month))
        return;

    s32 hours = (DateInDays(gClock.year, gClock.month, gClock.day)
        - DateInDays(year, tree->monthPlanted, tree->dayPlanted)) * 24
        + gClock.hour - tree->hourPlanted;
    if (hours < 0)
        return;
    u8 newStage = hours >= 4 * HOURS_PER_STAGE ? 5 : 1 + hours / HOURS_PER_STAGE;
    // Moving the RTC backwards must not shrink an existing plant.
    if (newStage > stage)
    {
        Var8005 = newStage;
        Var8007 = 0;
    }
}

void SetTreeData(void)
{
    if (Var8002 >= BERRY_TREE_COUNT || Var8005 > 5)
        return;
    struct BerryTreeHack *tree = &gBerryTrees[Var8002];
    if (Var8005 == 0)
    {
        memset(tree, 0, sizeof(*tree));
        return;
    }
    // Capture the year on planting without changing the six-byte save layout.
    if ((tree->stage >> 5) == 0)
        tree->yearPlanted = gClock.year >= 2000 && gClock.year <= 2099
            ? gClock.year - 1999 : 0;
    tree->stage = (Var8005 << 5) | (min(Var8006, 4) << 1) | (Var8007 != 0);
    tree->berryId = Var8008;
    tree->hourPlanted = Var8009;
    tree->monthPlanted = Var800A;
    tree->dayPlanted = Var800B;
}

void CalculateYield(void)
{
    u16 minimum = Var8001;
    u16 maximum = Var8000;
    if (maximum < minimum)
        maximum = minimum;
    u16 watered = min(Var8006, 4);
    u32 range = maximum - minimum;
    u32 bonus = range * watered;
    Var800D = minimum + (umodsi(Random(), 4) + bonus) / 4;
}

void SetPlantingTime(void)
{
    Var8009 = gClock.hour;
    Var800A = gClock.month;
    Var800B = gClock.day;
}
