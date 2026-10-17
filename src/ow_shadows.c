#include "defines.h"
#include "../include/event_object_movement.h"
#include "../include/field_effect.h"
#include "../include/field_weather.h"
#include "../include/gpu_regs.h"
#include "../include/sprite.h"
#include "../include/constants/rgb.h"
#include "../include/metatile_behavior.h"
#include "../include/constants/event_objects.h"
#include "../include/constants/field_effects.h"
#include "../include/constants/field_weather.h"

#include "../include/new/character_customization.h"

#ifdef OW_SHADOWS

/*
ow_shadows.c
	Semi-transparent overworld shadows for every event object.
	Ported from Fire-Red-OW-shadows (pokefirered decomp) to CFRU naming:
	ObjectEvent -> EventObject, gObjectEvents -> gEventObjects,
	GetObjectEventGraphicsInfo -> GetEventObjectGraphicsInfo (16-bit id).

	Hooked through "functionrewrites":
		GetAllGroundEffectFlags_OnSpawn / OnBeginStep / OnFinishStep
		DoFlaggedGroundEffects
		Task_WeatherMain
*/



#define SS_ANNE_GFX_ID 151 //Vanilla FR/LG id of the S.S. Anne sprite

//ROM data/functions that are not in BPRE.ld
#define gFieldEffectObjectTemplatePointers ((const struct SpriteTemplate* const *) 0x83A0010)

typedef void (*GroundEffectFunc)(struct EventObject *, struct Sprite *);
typedef void (*GroundFlagFunc)(struct EventObject *, u32 *);
#define ROMFUNC(addr, type) ((type) ((addr) | 1))

#define EventObjectUpdateMetatileBehaviors(obj)           ((void (*)(struct EventObject *)) (0x8067C24 | 1))(obj)
#define GetGroundEffectFlags_Reflection                   ROMFUNC(0x8067C4C, GroundFlagFunc)
#define GetGroundEffectFlags_TallGrassOnSpawn             ROMFUNC(0x8067CA8, GroundFlagFunc)
#define GetGroundEffectFlags_TallGrassOnBeginStep         ROMFUNC(0x8067CC8, GroundFlagFunc)
#define GetGroundEffectFlags_LongGrassOnSpawn             ROMFUNC(0x8067CE8, GroundFlagFunc)
#define GetGroundEffectFlags_LongGrassOnBeginStep         ROMFUNC(0x8067D08, GroundFlagFunc)
#define GetGroundEffectFlags_Tracks                       ROMFUNC(0x8067D28, GroundFlagFunc)
#define GetGroundEffectFlags_SandHeap                     ROMFUNC(0x8067D68, GroundFlagFunc)
#define GetGroundEffectFlags_ShallowFlowingWater          ROMFUNC(0x8067DB8, GroundFlagFunc)
#define GetGroundEffectFlags_Puddle                       ROMFUNC(0x8067E1C, GroundFlagFunc)
#define GetGroundEffectFlags_Ripple                       ROMFUNC(0x8067E4C, GroundFlagFunc)
#define GetGroundEffectFlags_ShortGrass                   ROMFUNC(0x8067E6C, GroundFlagFunc)
#define GetGroundEffectFlags_HotSprings                   ROMFUNC(0x8067EBC, GroundFlagFunc)
#define GetGroundEffectFlags_Seaweed                      ROMFUNC(0x8067F0C, GroundFlagFunc)
#define GetGroundEffectFlags_JumpLanding                  ROMFUNC(0x8067F2C, GroundFlagFunc)

//This file's functions:
static void GetGroundEffectFlags_Shadow(struct EventObject *eventObject, u32 *flags);
void GroundEffect_Shadow(struct EventObject *eventObject, struct Sprite *sprite);

//Order must match the GROUND_EFFECT_FLAG_* bits
static const GroundEffectFunc sGroundEffectFuncs[] =
{
	ROMFUNC(0x806837C, GroundEffectFunc), // TALL_GRASS_ON_SPAWN
	ROMFUNC(0x80683C8, GroundEffectFunc), // TALL_GRASS_ON_MOVE
	ROMFUNC(0x8068414, GroundEffectFunc), // LONG_GRASS_ON_SPAWN
	ROMFUNC(0x8068460, GroundEffectFunc), // LONG_GRASS_ON_MOVE
	ROMFUNC(0x80684AC, GroundEffectFunc), // ICE_REFLECTION
	ROMFUNC(0x80684B8, GroundEffectFunc), // REFLECTION
	ROMFUNC(0x80684C4, GroundEffectFunc), // SHALLOW_FLOWING_WATER
	ROMFUNC(0x80684D4, GroundEffectFunc), // SAND
	ROMFUNC(0x8068500, GroundEffectFunc), // DEEP_SAND
	ROMFUNC(0x80685D0, GroundEffectFunc), // RIPPLES
	ROMFUNC(0x80685DC, GroundEffectFunc), // PUDDLE
	ROMFUNC(0x80685EC, GroundEffectFunc), // SAND_PILE
	ROMFUNC(0x80685FC, GroundEffectFunc), // LAND_IN_TALL_GRASS
	ROMFUNC(0x8068654, GroundEffectFunc), // LAND_IN_LONG_GRASS
	ROMFUNC(0x806867C, GroundEffectFunc), // LAND_IN_SHALLOW_WATER
	ROMFUNC(0x80686AC, GroundEffectFunc), // LAND_IN_DEEP_WATER
	ROMFUNC(0x80686DC, GroundEffectFunc), // LAND_ON_NORMAL_GROUND
	ROMFUNC(0x806870C, GroundEffectFunc), // SHORT_GRASS
	ROMFUNC(0x806871C, GroundEffectFunc), // HOT_SPRINGS
	ROMFUNC(0x806872C, GroundEffectFunc), // SEAWEED
	GroundEffect_Shadow,                  // SHADOW
};

//Hook: replaces the three ROM "GetAllGroundEffectFlags" functions
void GetAllGroundEffectFlags_OnSpawn(struct EventObject *eventObject, u32 *flags)
{
	EventObjectUpdateMetatileBehaviors(eventObject);
	GetGroundEffectFlags_Reflection(eventObject, flags);
	GetGroundEffectFlags_TallGrassOnSpawn(eventObject, flags);
	GetGroundEffectFlags_LongGrassOnSpawn(eventObject, flags);
	GetGroundEffectFlags_SandHeap(eventObject, flags);
	GetGroundEffectFlags_ShallowFlowingWater(eventObject, flags);
	GetGroundEffectFlags_ShortGrass(eventObject, flags);
	GetGroundEffectFlags_HotSprings(eventObject, flags);
	GetGroundEffectFlags_Shadow(eventObject, flags);
}

void GetAllGroundEffectFlags_OnBeginStep(struct EventObject *eventObject, u32 *flags)
{
	EventObjectUpdateMetatileBehaviors(eventObject);
	GetGroundEffectFlags_Reflection(eventObject, flags);
	GetGroundEffectFlags_TallGrassOnBeginStep(eventObject, flags);
	GetGroundEffectFlags_LongGrassOnBeginStep(eventObject, flags);
	GetGroundEffectFlags_Tracks(eventObject, flags);
	GetGroundEffectFlags_SandHeap(eventObject, flags);
	GetGroundEffectFlags_ShallowFlowingWater(eventObject, flags);
	GetGroundEffectFlags_Puddle(eventObject, flags);
	GetGroundEffectFlags_ShortGrass(eventObject, flags);
	GetGroundEffectFlags_HotSprings(eventObject, flags);
	GetGroundEffectFlags_Shadow(eventObject, flags);
}

void GetAllGroundEffectFlags_OnFinishStep(struct EventObject *eventObject, u32 *flags)
{
	EventObjectUpdateMetatileBehaviors(eventObject);
	GetGroundEffectFlags_ShallowFlowingWater(eventObject, flags);
	GetGroundEffectFlags_SandHeap(eventObject, flags);
	GetGroundEffectFlags_Puddle(eventObject, flags);
	GetGroundEffectFlags_Ripple(eventObject, flags);
	GetGroundEffectFlags_ShortGrass(eventObject, flags);
	GetGroundEffectFlags_HotSprings(eventObject, flags);
	GetGroundEffectFlags_Seaweed(eventObject, flags);
	GetGroundEffectFlags_JumpLanding(eventObject, flags);
	GetGroundEffectFlags_Shadow(eventObject, flags);
}

//Hook: replaces ROM DoFlaggedGroundEffects so the extra shadow entry is reachable
void DoFlaggedGroundEffects(struct EventObject *eventObject, struct Sprite *sprite, u32 flags)
{
	u8 i;

	if (eventObject->localId == EVENT_OBJ_ID_CAMERA && eventObject->invisible)
		return;

	for (i = 0; i < NELEMS(sGroundEffectFuncs); i++, flags >>= 1)
	{
		if (flags & 1)
			sGroundEffectFuncs[i](eventObject, sprite);
	}
}

static bool8 IsShadowAllowedForGfx(struct EventObject *eventObject)
{
	return GetEventObjectGraphicsId(eventObject) != SS_ANNE_GFX_ID; //Full 16-bit id
}

static bool8 IsShadowAllowedInWeather(void)
{
	//Rain and horizontal fog also use the OBJ blend registers
	u8 curr = gWeatherPtr->currWeather;
	u8 next = gWeatherPtr->nextWeather;
	bool8 currBad = curr == WEATHER_RAIN_LIGHT || curr == WEATHER_FOG_1;
	bool8 nextBad = next == WEATHER_RAIN_LIGHT || next == WEATHER_FOG_1;

	if (currBad && !gWeatherPtr->weatherChangeComplete)
		nextBad = TRUE; //Weather still changing

	return !(currBad && nextBad);
}

static bool8 IsShadowAllowedOnMetatile(struct EventObject *eventObject)
{
	return !MetatileBehavior_IsTallGrass(eventObject->currentMetatileBehavior)
		&& !MetatileBehavior_IsLongGrass(eventObject->currentMetatileBehavior);
}

void SetEventObjectShadowVisibility(struct EventObject *eventObject, bool8 invisible)
{
	u8 i, j;

	for (i = MAX_SPRITES; i > 0; --i)
	{
		struct Sprite *sprite = &gSprites[i - 1];

		if (!sprite->inUse
		|| sprite->data[0] != eventObject->localId
		|| sprite->data[1] != eventObject->mapNum
		|| sprite->data[2] != eventObject->mapGroup)
			continue;

		for (j = FLDEFFOBJ_SHADOW_S; j <= FLDEFFOBJ_SHADOW_XL; ++j)
		{
			if (sprite->template == gFieldEffectObjectTemplatePointers[j])
			{
				sprite->invisible = invisible;
				return;
			}
		}
	}
}

static void GetGroundEffectFlags_Shadow(struct EventObject *eventObject, u32 *flags)
{
	if (eventObject->invisible || !eventObject->active
	|| !IsShadowAllowedForGfx(eventObject) || !IsShadowAllowedInWeather() || !IsShadowAllowedOnMetatile(eventObject))
	{
		eventObject->hasShadow = FALSE;
		return;
	}

	if (eventObject->hasShadow)
		return;

	*flags |= GROUND_EFFECT_FLAG_SHADOW;
}

/*
Shadow colour
	Change OW_SHADOW_COLOR to recolour every overworld shadow, e.g. RGB(0, 0, 0) black,
	RGB(8, 8, 12) dark blue-grey, RGB(20, 20, 20) light grey. Each channel is 0-31.
	Every opaque colour index of the shadow palette is set to this colour, so it works no matter
	which index the shadow tiles use.
*/
#define OW_SHADOW_COLOR RGB(4, 4, 8)
#define OW_SHADOW_PAL_TAG 0x2D00 //Any tag not used by another sprite palette

#define SHADOW_PAL_ENTRY OW_SHADOW_COLOR
static const u16 sShadowPalette[16] =
{
	0, //Transparent
	SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY,
	SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY,
	SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY,
	SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY, SHADOW_PAL_ENTRY,
};

static const struct SpritePalette sShadowSpritePalette = {sShadowPalette, OW_SHADOW_PAL_TAG};

static void ApplyShadowColor(struct EventObject *eventObject)
{
	u8 i, j, palSlot;

	//FldEff_Shadow doesn't return the sprite id, so find the sprite it just created
	for (i = MAX_SPRITES; i > 0; --i)
	{
		struct Sprite *sprite = &gSprites[i - 1];

		if (!sprite->inUse
		|| sprite->data[0] != eventObject->localId
		|| sprite->data[1] != eventObject->mapNum
		|| sprite->data[2] != eventObject->mapGroup)
			continue;

		for (j = FLDEFFOBJ_SHADOW_S; j <= FLDEFFOBJ_SHADOW_XL; ++j)
		{
			if (sprite->template == gFieldEffectObjectTemplatePointers[j])
			{
				palSlot = LoadSpritePalette(&sShadowSpritePalette); //Returns the existing slot if already loaded
				if (palSlot != 0xFF)
				{
					sprite->oam.paletteNum = palSlot;
					sprite->oam.objMode = ST_OAM_OBJ_BLEND;
					SetGpuReg(REG_OFFSET_BLDCNT, BLDCNT_TGT1_OBJ | BLDCNT_TGT2_ALL | BLDCNT_EFFECT_BLEND);
					SetGpuReg(REG_OFFSET_BLDALPHA, BLDALPHA_BLEND(OW_SHADOW_OPACITY, 16 - OW_SHADOW_OPACITY));
				}
				return;
			}
		}
	}
}

void GroundEffect_Shadow(struct EventObject *eventObject, struct Sprite *sprite)
{
	(void) sprite;
	eventObject->hasShadow = TRUE;
	gFieldEffectArguments[0] = eventObject->localId;
	gFieldEffectArguments[1] = eventObject->mapNum;
	gFieldEffectArguments[2] = eventObject->mapGroup;
	FieldEffectStart(FLDEFF_SHADOW);
	ApplyShadowColor(eventObject);
}

//Hook: replaces ROM Task_WeatherMain (renamed: the original symbol lives in BPRE.ld)
struct WeatherCallbacks
{
	void (*initVars)(void);
	void (*main)(void);
	void (*initAll)(void);
	bool8 (*finish)(void);
};

#define sWeatherFuncs ((const struct WeatherCallbacks*) 0x83C2BC0)
#define gWeatherPalStateFuncs ((void (* const *)(void)) 0x83C2CB0)

void Task_WeatherMainHook(u8 taskId)
{
	u8 i;
	(void) taskId;

	if (gWeatherPtr->currWeather != gWeatherPtr->nextWeather)
	{
		if (!sWeatherFuncs[gWeatherPtr->currWeather].finish()
		&& gWeatherPtr->palProcessingState != WEATHER_PAL_STATE_SCREEN_FADING_OUT)
		{
			//Finished cleaning up previous weather. Now transition to next weather.
			sWeatherFuncs[gWeatherPtr->nextWeather].initVars();
			gWeatherPtr->gammaStepFrameCounter = 0;
			gWeatherPtr->palProcessingState = WEATHER_PAL_STATE_CHANGING_WEATHER;
			gWeatherPtr->currWeather = gWeatherPtr->nextWeather;
			gWeatherPtr->weatherChangeComplete = TRUE;

			//Re-check shadows now that the weather has settled
			for (i = 0; i < EVENT_OBJECTS_COUNT; i++)
				gEventObjects[i].triggerGroundEffectsOnMove = TRUE;
		}
	}
	else
	{
		sWeatherFuncs[gWeatherPtr->currWeather].main();
	}

	gWeatherPalStateFuncs[gWeatherPtr->palProcessingState]();
}

#endif
