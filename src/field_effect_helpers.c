#include "../include/global.h"
#include "../include/event_object_movement.h"
#include "../include/field_camera.h"
#include "../include/field_effect.h"
#include "../include/field_effect_helpers.h"
#include "../include/field_weather.h"
#include "../include/fieldmap.h"
#include "../include/metatile_behavior.h"
#include "../include/constants/field_effects.h"
#include "../include/constants/event_objects.h"
#include "../include/constants/songs.h"
#include "../include/gpu_regs.h"

#define OBJECT_EVENTS_COUNT 16
extern struct EventObject gObjectEvents[OBJECT_EVENTS_COUNT];
u8 GetObjectEventIdByLocalIdAndMap(u8, u8, u8);
const struct EventObjectGraphicsInfo *GetEventObjectGraphicsInfo(u8);


const u8 sShadowEffectTemplateIds[] = {
    [SHADOW_SIZE_S]  = FLDEFFOBJ_SHADOW_S,
    [SHADOW_SIZE_M]  = FLDEFFOBJ_SHADOW_M,
    [SHADOW_SIZE_L]  = FLDEFFOBJ_SHADOW_L,
    [SHADOW_SIZE_XL] = FLDEFFOBJ_SHADOW_XL
};

const u16 gShadowVerticalOffsets[] = {
    [SHADOW_SIZE_S]  =  4,
    [SHADOW_SIZE_M]  =  4,
    [SHADOW_SIZE_L]  =  4,
    [SHADOW_SIZE_XL] = 16
};

u32 FldEff_Shadow(void)
{
    u8 objectEventId;
    const struct EventObjectGraphicsInfo *graphicsInfo;
    u8 spriteId;
    objectEventId = GetObjectEventIdByLocalIdAndMap(gFieldEffectArguments[0], gFieldEffectArguments[1], gFieldEffectArguments[2]);
    graphicsInfo = GetEventObjectGraphicsInfo(gObjectEvents[objectEventId].graphicsId);
    spriteId = CreateSpriteAtEnd(gFieldEffectObjectTemplatePointers[sShadowEffectTemplateIds[graphicsInfo->shadowSize]], 0, 0, 0x94);
    if (spriteId != MAX_SPRITES)
    {
        gSprites[spriteId].coordOffsetEnabled = TRUE;
        gSprites[spriteId].data[0] = gFieldEffectArguments[0];
        gSprites[spriteId].data[1] = gFieldEffectArguments[1];
        gSprites[spriteId].data[2] = gFieldEffectArguments[2];
        gSprites[spriteId].data[3] = (graphicsInfo->height >> 1) - gShadowVerticalOffsets[graphicsInfo->shadowSize];

        gSprites[spriteId].oam.objMode = ST_OAM_OBJ_BLEND;
    }

    SetGpuReg(REG_OFFSET_DISPCNT, 0x1F40);
    SetGpuReg(REG_OFFSET_BLDALPHA, 0x0A10);

    return 0;
}
