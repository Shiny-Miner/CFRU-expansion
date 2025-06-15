#include "../include/global.h"

#include "../include/event_data.h"
#include "../include/event_object_movement.h"
#include "../include/field_camera.h"
#include "../include/field_control_avatar.h"
#include "../include/field_effect.h"
#include "../include/field_effect_helpers.h"
#include "../include/field_player_avatar.h"
#include "../include/fieldmap.h"
#include "../include/metatile_behavior.h"
#include "../include/overworld.h"
#include "../include/quest_log.h"
#include "../include/random.h"
#include "../include/script.h"
#include "../include/trainer_see.h"
#include "../include/trig.h"
#include "../include/constants/maps.h"
#include "../include/constants/event_object_movement.h"
#include "../include/constants/event_objects.h"
#include "../include/constants/trainer_types.h"
#include "../include/constants/union_room.h"
#include "../include/constants/weather.h"
#include "../include/field_weather.h"
#include "../include/sprite.h"

#define OBJ_EVENT_GFX_SS_ANNE 151
#define OBJ_EVENT_ID_CAMERA 127
#define gWeatherPtr ((struct Weather*) 0x2037F34)
u32 StartFieldEffectForObjectEvent(u8, struct EventObject *);

void GetGroundEffectFlags_Shadow(struct EventObject*, u32*);
extern void GroundEffect_SpawnOnTallGrass(struct EventObject *objEvent, struct Sprite *sprite);    // GROUND_EFFECT_FLAG_TALL_GRASS_ON_SPAWN
extern void GroundEffect_StepOnTallGrass(struct EventObject *objEvent, struct Sprite *sprite);      // GROUND_EFFECT_FLAG_TALL_GRASS_ON_MOVE
extern void GroundEffect_SpawnOnLongGrass(struct EventObject *objEvent, struct Sprite *sprite);      // GROUND_EFFECT_FLAG_LONG_GRASS_ON_SPAWN
extern void GroundEffect_StepOnLongGrass(struct EventObject *objEvent, struct Sprite *sprite);       // GROUND_EFFECT_FLAG_LONG_GRASS_ON_MOVE
extern void GroundEffect_WaterReflection(struct EventObject *objEvent, struct Sprite *sprite);       // GROUND_EFFECT_FLAG_ICE_REFLECTION
extern void GroundEffect_IceReflection(struct EventObject *objEvent, struct Sprite *sprite);         // GROUND_EFFECT_FLAG_REFLECTION
extern void GroundEffect_FlowingWater(struct EventObject *objEvent, struct Sprite *sprite);          // GROUND_EFFECT_FLAG_SHALLOW_FLOWING_WATER
extern void GroundEffect_SandTracks(struct EventObject *objEvent, struct Sprite *sprite);            // GROUND_EFFECT_FLAG_SAND
extern void GroundEffect_DeepSandTracks(struct EventObject *objEvent, struct Sprite *sprite);        // GROUND_EFFECT_FLAG_DEEP_SAND
extern void GroundEffect_Ripple(struct EventObject *objEvent, struct Sprite *sprite);                // GROUND_EFFECT_FLAG_RIPPLES
extern void GroundEfferct_StepOnPuddle(struct EventObject *objEvent, struct Sprite *sprite);          // GROUND_EFFECT_FLAG_PUDDLE
extern void GroundEffect_SandHeap(struct EventObject *objEvent, struct Sprite *sprite);              // GROUND_EFFECT_FLAG_SAND_PILE
extern void GroundEffect_JumpOnTallGrass(struct EventObject *objEvent, struct Sprite *sprite);       // GROUND_EFFECT_FLAG_LAND_IN_TALL_GRASS
extern void GroundEffect_JumpOnLongGrass(struct EventObject *objEvent, struct Sprite *sprite);       // GROUND_EFFECT_FLAG_LAND_IN_LONG_GRASS
extern void GroundEffect_JumpOnShallowWater(struct EventObject *objEvent, struct Sprite *sprite);    // GROUND_EFFECT_FLAG_LAND_IN_SHALLOW_WATER
extern void GroundEffect_JumpOnWater(struct EventObject *objEvent, struct Sprite *sprite);           // GROUND_EFFECT_FLAG_LAND_IN_DEEP_WATER
extern void GroundEffect_JumpLandingDust(struct EventObject *objEvent, struct Sprite *sprite);       // GROUND_EFFECT_FLAG_LAND_ON_NORMAL_GROUND
extern void GroundEffect_ShortGrass(struct EventObject *objEvent, struct Sprite *sprite);            // GROUND_EFFECT_FLAG_SHORT_GRASS
extern void GroundEffect_HotSprings(struct EventObject *objEvent, struct Sprite *sprite);            // GROUND_EFFECT_FLAG_HOT_SPRINGS
extern void GroundEffect_Seaweed(struct EventObject *objEvent, struct Sprite *sprite);               // GROUND_EFFECT_FLAG_SEAWEED
extern void GroundEffect_StepOnPuddle(struct EventObject *objEvent, struct Sprite *sprite);
extern void ObjectEventUpdateMetatileBehaviors(struct EventObject *objEvent);
extern void GetGroundEffectFlags_Reflection(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_TallGrassOnSpawn(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_LongGrassOnSpawn(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_SandHeap(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_ShallowFlowingWater(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_ShortGrass(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_HotSprings(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_TallGrassOnBeginStep(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_LongGrassOnBeginStep(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_Tracks(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_Puddle(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_Ripple(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_Seaweed(struct EventObject *objEvent, u32 *flags);
extern void GetGroundEffectFlags_JumpLanding(struct EventObject *objEvent, u32 *flags);

void GetAllGroundEffectFlags_OnSpawn(struct EventObject *objEvent, u32 *flags)
{
    ObjectEventUpdateMetatileBehaviors(objEvent);
    GetGroundEffectFlags_Reflection(objEvent, flags);
    GetGroundEffectFlags_TallGrassOnSpawn(objEvent, flags);
    GetGroundEffectFlags_LongGrassOnSpawn(objEvent, flags);
    GetGroundEffectFlags_SandHeap(objEvent, flags);
    GetGroundEffectFlags_ShallowFlowingWater(objEvent, flags);
    GetGroundEffectFlags_ShortGrass(objEvent, flags);
    GetGroundEffectFlags_HotSprings(objEvent, flags);
    GetGroundEffectFlags_Shadow(objEvent, flags);
}

void GetAllGroundEffectFlags_OnBeginStep(struct EventObject *objEvent, u32 *flags)
{
    ObjectEventUpdateMetatileBehaviors(objEvent);
    GetGroundEffectFlags_Reflection(objEvent, flags);
    GetGroundEffectFlags_TallGrassOnBeginStep(objEvent, flags);
    GetGroundEffectFlags_LongGrassOnBeginStep(objEvent, flags);
    GetGroundEffectFlags_Tracks(objEvent, flags);
    GetGroundEffectFlags_SandHeap(objEvent, flags);
    GetGroundEffectFlags_ShallowFlowingWater(objEvent, flags);
    GetGroundEffectFlags_Puddle(objEvent, flags);
    GetGroundEffectFlags_ShortGrass(objEvent, flags);
    GetGroundEffectFlags_HotSprings(objEvent, flags);
    GetGroundEffectFlags_Shadow(objEvent, flags);
}


void GetAllGroundEffectFlags_OnFinishStep(struct EventObject *objEvent, u32 *flags)
{
    ObjectEventUpdateMetatileBehaviors(objEvent);
    GetGroundEffectFlags_ShallowFlowingWater(objEvent, flags);
    GetGroundEffectFlags_SandHeap(objEvent, flags);
    GetGroundEffectFlags_Puddle(objEvent, flags);
    GetGroundEffectFlags_Ripple(objEvent, flags);
    GetGroundEffectFlags_ShortGrass(objEvent, flags);
    GetGroundEffectFlags_HotSprings(objEvent, flags);
    GetGroundEffectFlags_Seaweed(objEvent, flags);
    GetGroundEffectFlags_JumpLanding(objEvent, flags);
    GetGroundEffectFlags_Shadow(objEvent, flags);
}




static const u8 sDisallowedIds[] = {
   OBJ_EVENT_GFX_SS_ANNE
};

static const u8 sDisallowedWeathers[] = {
    WEATHER_RAIN,
    WEATHER_FOG_HORIZONTAL,
};

typedef bool8 (*MetatileFunc)(u8);
static const MetatileFunc sDisallowedMetatiles[] = {
    MetatileBehavior_IsTallGrass,
    MetatileBehavior_IsLongGrass,
};

static bool8 IsShadowAllowedInId(struct EventObject *objEvent) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sDisallowedIds); i++) {
        if (sDisallowedIds[i] == objEvent->graphicsId) 
            return FALSE;
    }

    return TRUE;
}

static bool8 IsShadowAllowedInWeather() {
    u8 i;
    bool8 currWeatherDisallowed = FALSE;
    bool8 nextWeatherDisallowed = FALSE;

    for (i = 0; i < ARRAY_COUNT(sDisallowedWeathers); i++) {
        if(gWeatherPtr->currWeather == sDisallowedWeathers[i]) {
            currWeatherDisallowed = TRUE;
            //If the weather hasn't changed completely, no shadow will show
            //Force "nextWeatherDisallowed" to avoid duplicated return statements
            if(!gWeatherPtr->weatherChangeComplete)
                nextWeatherDisallowed = TRUE;
        }
        if(gWeatherPtr->nextWeather == sDisallowedWeathers[i])
            nextWeatherDisallowed = TRUE;

        if (currWeatherDisallowed && nextWeatherDisallowed) {
            return FALSE;
        }
    }

    return TRUE;
}

static bool8 IsShadowAllowedInMetatile(struct EventObject *objEvent) {
    u8 i;

    for (i = 0; i < ARRAY_COUNT(sDisallowedMetatiles); i++) {
        if (sDisallowedMetatiles[i](objEvent->currentMetatileBehavior)) 
            return FALSE;
    }

    return TRUE;
}

void GetGroundEffectFlags_Shadow(struct EventObject *objEvent, u32 *flags) {
    if(objEvent->invisible || !objEvent->active 
        || !IsShadowAllowedInId(objEvent) || !IsShadowAllowedInWeather() || !IsShadowAllowedInMetatile(objEvent)){
        objEvent->hasShadow = FALSE;
        return;
    }

    if(objEvent->hasShadow)
        return;
    *flags |= GROUND_EFFECT_SHADOW;
}


void GroundEffect_Shadow(struct EventObject *objEvent, struct Sprite *sprite) {
    objEvent->hasShadow = TRUE;
    StartFieldEffectForObjectEvent(FLDEFF_SHADOW, objEvent);
}


void (*const sGroundEffectFuncs[])(struct EventObject *objEvent, struct Sprite *sprite) = {
    GroundEffect_SpawnOnTallGrass,      // GROUND_EFFECT_FLAG_TALL_GRASS_ON_SPAWN
    GroundEffect_StepOnTallGrass,       // GROUND_EFFECT_FLAG_TALL_GRASS_ON_MOVE
    GroundEffect_SpawnOnLongGrass,      // GROUND_EFFECT_FLAG_LONG_GRASS_ON_SPAWN
    GroundEffect_StepOnLongGrass,       // GROUND_EFFECT_FLAG_LONG_GRASS_ON_MOVE
    GroundEffect_WaterReflection,       // GROUND_EFFECT_FLAG_ICE_REFLECTION
    GroundEffect_IceReflection,         // GROUND_EFFECT_FLAG_REFLECTION
    GroundEffect_FlowingWater,          // GROUND_EFFECT_FLAG_SHALLOW_FLOWING_WATER
    GroundEffect_SandTracks,            // GROUND_EFFECT_FLAG_SAND
    GroundEffect_DeepSandTracks,        // GROUND_EFFECT_FLAG_DEEP_SAND
    GroundEffect_Ripple,                // GROUND_EFFECT_FLAG_RIPPLES
    GroundEffect_StepOnPuddle,          // GROUND_EFFECT_FLAG_PUDDLE
    GroundEffect_SandHeap,              // GROUND_EFFECT_FLAG_SAND_PILE
    GroundEffect_JumpOnTallGrass,       // GROUND_EFFECT_FLAG_LAND_IN_TALL_GRASS
    GroundEffect_JumpOnLongGrass,       // GROUND_EFFECT_FLAG_LAND_IN_LONG_GRASS
    GroundEffect_JumpOnShallowWater,    // GROUND_EFFECT_FLAG_LAND_IN_SHALLOW_WATER
    GroundEffect_JumpOnWater,           // GROUND_EFFECT_FLAG_LAND_IN_DEEP_WATER
    GroundEffect_JumpLandingDust,       // GROUND_EFFECT_FLAG_LAND_ON_NORMAL_GROUND
    GroundEffect_ShortGrass,            // GROUND_EFFECT_FLAG_SHORT_GRASS
    GroundEffect_HotSprings,            // GROUND_EFFECT_FLAG_HOT_SPRINGS
    GroundEffect_Seaweed,               // GROUND_EFFECT_FLAG_SEAWEED
    GroundEffect_Shadow                 // GROUND_EFFECT_FLAG_SHADOW
};

void DoFlaggedGroundEffects(struct EventObject *objEvent, struct Sprite *sprite, u32 flags)
{
    u8 i;

    if (objEvent->localId == OBJ_EVENT_ID_CAMERA && objEvent->invisible)
        return;

    for (i = 0; i < NELEMS(sGroundEffectFuncs); i++, flags >>= 1)
        if (flags & 1)
            sGroundEffectFuncs[i](objEvent, sprite);
}