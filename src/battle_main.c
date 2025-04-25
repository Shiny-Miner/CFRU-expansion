#include "../include/global.h"
#include "../include/battle.h"
#include "../include/battle_anim.h"
#include "../include/battle_ai_script_commands.h"
#include "../include/battle_controllers.h"
#include "../include/battle_interface.h"
#include "../include/battle_main.h"
#include "../include/battle_message.h"
#include "../include/battle_scripts.h"
#include "../include/battle_setup.h"
#include "../include/battle_tower.h"
#include "../include/battle_util.h"
#include "../include/berry.h"
#include "../include/bg.h"
#include "../include/data.h"
#include "../include/decompress.h"
#include "../include/dma3.h"
#include "../include/event_data.h"
#include "../include/evolution_scene.h"
#include "../include/graphics.h"
#include "../include/gpu_regs.h"
#include "../include/item.h"
#include "../include/link.h"
#include "../include/link_rfu.h"
#include "../include/load_save.h"
#include "../include/main.h"
#include "../include/malloc.h"
#include "../include/m4a.h"
#include "../include/palette.h"
#include "../include/party_menu.h"
#include "../include/pokeball.h"
#include "../include/pokedex.h"
#include "../include/pokemon.h"
#include "../include/random.h"
#include "../include/reshow_battle_screen.h"
#include "../include/roamer.h"
#include "../include/safari_zone.h"
#include "../include/scanline_effect.h"
#include "../include/sound.h"
#include "../include/sprite.h"
#include "../include/string_util.h"
#include "../include/strings.h"
#include "../include/task.h"
#include "defines.h"
#include "../include/text.h"
#include "../include/trig.h"
#include "../include/util.h"
#include "../include/window.h"
#include "../include/constants/abilities.h"
#include "../include/constants/battle_move_effects.h"
#include "../include/constants/battle_string_ids.h"
#include "../include/constants/hold_effects.h"
#include "../include/constants/items.h"
#include "../include/constants/moves.h"
#include "../include/constants/party_menu.h"
#include "../include/constants/rgb.h"
#include "../include/constants/songs.h"
#include "../include/constants/trainers.h"
#include "../include/cable_club.h"

#define EVO_MODE_NORMAL     0
#define PALETTES_BG      0x0000FFFF
#define PALETTES_OBJECTS 0xFFFF0000
#define PALETTES_ALL     (PALETTES_BG | PALETTES_OBJECTS)
void TurnValuesCleanUp(bool8 var0);
void RunTurnActionsFunctions(void);
void HandleTurnActionSelectionState(void);
extern u8 DoFieldEndTurnEffects(void) __attribute__((long_call));
extern u8 DoBattlerEndTurnEffects(void) __attribute__((long_call));
extern bool8 HandleFaintedMonActions(void) __attribute__((long_call));
extern bool8 HandleWishPerishSongOnTurnEnd(void) __attribute__((long_call));
extern u16 gChosenMoveByBattler[MAX_BATTLERS_COUNT];
extern u8 gChosenActionByBattler[MAX_BATTLERS_COUNT];
u16 GetEvolutionTargetSpecies(struct Pokemon *mon, u8 type, u16 evolutionItem);

void PlayerTryEvolution(void);
static void WaitForEvolutionThenTryAnother(void);
static void CB2_SetUpReshowBattleScreenAfterEvolution(void);

/*=== New functions ===*/

#define LEFT_PKMN gBattlerPartyIndexes[GetBattlerAtPosition(B_POSITION_PLAYER_LEFT)]
#define RIGHT_PKMN gBattlerPartyIndexes[GetBattlerAtPosition(B_POSITION_PLAYER_RIGHT)]

void CB2_SetUpReshowBattleScreenAfterEvolution(void)
{
    gBattleTerrain = gBattleTerrainBackup; 
    SetMainCallback2(ReshowBattleScreenAfterMenu);
}

#define tSpeciesToEvolveInto data[0]
#define tBattlerPosition     data[1]

static void Task_BeginBattleEvolutionScene(u8 taskId)
{
    if (!gPaletteFade->active)
    {
        u8 battlerPosition;
        u16 SpeciesToEvolveInto;
        FreeAllWindowBuffers();
        gCB2_AfterEvolution = CB2_SetUpReshowBattleScreenAfterEvolution;
        gBattleTerrainBackup = gBattleTerrain; // Store the battle terrain to be reloaded later

        battlerPosition = gTasks[taskId].tBattlerPosition;
        SpeciesToEvolveInto = gTasks[taskId].tSpeciesToEvolveInto;
        DestroyTask(taskId);
        EvolutionScene(&gPlayerParty[battlerPosition], SpeciesToEvolveInto, TRUE, battlerPosition);
    }
}

void PlayerTryEvolution(void)
{
    u16 species;
    u8 taskId; 
    if (gLeveledUpInBattle & gBitTable[LEFT_PKMN] && !gPlayerDoesNotWantToEvolveLeft)
    {
        gLeveledUpInBattle &= ~(gBitTable[LEFT_PKMN]); // Mask the bit
        species = GetEvolutionTargetSpecies(&gPlayerParty[LEFT_PKMN], EVO_MODE_NORMAL, ITEM_NONE);
        if (species != SPECIES_NONE)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gBattleMainFunc = WaitForEvolutionThenTryAnother;
            taskId = CreateTask(Task_BeginBattleEvolutionScene, 0);
            gTasks[taskId].tSpeciesToEvolveInto = species;
            gTasks[taskId].tBattlerPosition = LEFT_PKMN;
            return;
        }
    }
    if (gBattleTypeFlags & BATTLE_TYPE_DOUBLE && gLeveledUpInBattle & gBitTable[RIGHT_PKMN] && !gPlayerDoesNotWantToEvolveRight)
    {
        gLeveledUpInBattle &= ~(gBitTable[RIGHT_PKMN]); // Mask the bit
        species = GetEvolutionTargetSpecies(&gPlayerParty[RIGHT_PKMN], EVO_MODE_NORMAL, ITEM_NONE);
        if (species != SPECIES_NONE)
        {
            BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
            gBattleMainFunc = WaitForEvolutionThenTryAnother;
            taskId = CreateTask(Task_BeginBattleEvolutionScene, 0);
            gTasks[taskId].tSpeciesToEvolveInto = species;
            gTasks[taskId].tBattlerPosition = RIGHT_PKMN;
            return;
        }
    }

    gBattleMainFunc = HandleTurnActionSelectionState;

}

static void WaitForEvolutionThenTryAnother(void)
{
    if (gMain.callback2 == BattleMainCB2 && !gPaletteFade->active)
    {
        gBattleMainFunc = PlayerTryEvolution;
    }
}
