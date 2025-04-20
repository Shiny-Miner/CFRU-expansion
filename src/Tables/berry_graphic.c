#include "../../include/berry.h"

static const BerryPrototype sDummyBerry = {
    .berryItemId = ITEM_NONE,
    .objectEventIds = {0},
    .untilNextStage = {0},
    .isLooping = FALSE,
};

static const BerryPrototype sSampleBerry = {
    .berryItemId = ITEM_ORAN_BERRY,
    .objectEventIds = {0},
    .untilNextStage = {0},
    .isLooping = TRUE,
};

const BerryPrototype gBerries[] = {
    sSampleBerry,
};