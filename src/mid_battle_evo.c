#include "defines.h"
#include "defines_battle.h"
#include "../include/battle.h"
#include "../include/battle_string_ids.h"
#include "../include/bg.h"
#include "../include/evolution_scene.h"
#include "../include/gpu_regs.h"
#include "../include/m4a.h"
#include "../include/malloc.h"
#include "../include/overworld.h"
#include "../include/palette.h"
#include "../include/party_menu.h"
#include "../include/pokemon_summary_screen.h"
#include "../include/scanline_effect.h"
#include "../include/sound.h"
#include "../include/sprite.h"
#include "../include/string_util.h"
#include "../include/task.h"
#include "../include/window.h"

#include "../include/constants/songs.h"

#include "../include/gba/macro.h"

#include "../include/new/battle_terrain.h"
#include "../include/new/battle_util.h"
#include "../include/new/dynamax.h"
#include "../include/new/frontier.h"
#include "../include/new/terastallization.h"
#include "../include/new/build_pokemon.h"
#include "../include/new/evolution.h"
#include "../include/new/learn_move.h"
#include "../include/new/mid_battle_evo.h"
#include "../include/new/Vanilla_functions.h"

#ifdef MID_BATTLE_EVO
void PlayerTryEvolution(void);
static void WaitForEvolutionThenTryAnother(void);
static void CB2_SetUpReshowBattleScreenAfterEvolution(void);
static void Task_EvolutionScene(u8 taskId);
static void UpdateEvolvedBattleMon(struct Pokemon *mon);
static void UpdateEvolvedBattleMoves(struct Pokemon *mon);
static void CB2_MidBattleEvolutionLoadGraphics(void);
static void EvolutionScene(struct Pokemon* mon, u16 postEvoSpecies, bool8 canStopEvo, u8 partyId);
static u16 TryGetFemaleGenderedSpecies(u16 species, u32 personality);

struct EvoInfo
{
    u8 preEvoSpriteId;
    u8 postEvoSpriteId;
    u8 evoTaskId;
    u8 delayTimer;
    u16 savedPalette[48];
};

extern struct EvoInfo *sEvoStructPtr;

#define gMonFrontPicTable ((const struct CompressedSpriteSheet*) *((u32*) 0x8000128))
#define tState              data[0]
#define tPreEvoSpecies      data[1]
#define tPostEvoSpecies     data[2]
#define tCanStop            data[3]
#define tBits               data[3]
#define tLearnsFirstMove    data[4]
#define tLearnMoveState     data[6]
#define tLearnMoveYesState  data[7]
#define tLearnMoveNoState   data[8]
#define tEvoWasStopped      data[9]
#define tPartyId            data[10]

static void EvoDummyFunc(void)
{
}

static void CB2_SetUpReshowBattleScreenAfterEvolution(void)
{
    gBattleTerrain = gNewBS->midBattleEvolution.savedTerrain;
    memcpy(gBattleCommunication, gNewBS->midBattleEvolution.savedCommunication, sizeof(gBattleCommunication));
    gNewBS->midBattleEvolution.active = FALSE;
    SetMainCallback2(ReshowBattleScreenAfterMenu);
}

#define tSpeciesToEvolveInto data[0]
#define tPartyToEvolve       data[1]

static void Task_BeginBattleEvolutionScene(u8 taskId)
{
    if (!gPaletteFade->active)
    {
        u8 partyId = gTasks[taskId].tPartyToEvolve;
        u16 species = gTasks[taskId].tSpeciesToEvolveInto;

        gCB2_AfterEvolution = CB2_SetUpReshowBattleScreenAfterEvolution;
        DestroyTask(taskId);
        EvolutionScene(&gPlayerParty[partyId], species, TRUE, partyId);
    }
}

void PlayerTryEvolution(void)
{
    u8 position;

    if (gNewBS == NULL || !gMain.inBattle || gBattleOutcome != 0
     || gBattleTypeFlags & (BATTLE_TYPE_LINK | BATTLE_TYPE_SAFARI | BATTLE_TYPE_POKE_DUDE
                         | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_TRAINER_TOWER
                         | BATTLE_TYPE_EREADER_TRAINER | BATTLE_TYPE_BENJAMIN_BUTTERFREE))
    {
        gBattleMainFunc = HandleTurnActionSelectionState;
        return;
    }

    // Don't reset the scene while a controller/animation is still using it.
    if (gBattleExecBuffer || gPaletteFade->active)
        return;

    for (position = B_POSITION_PLAYER_LEFT; position <= B_POSITION_PLAYER_RIGHT; position += BIT_FLANK)
    {
        u8 bank, partyId, taskId;
        u16 species;
        struct Pokemon *mon;

        if (position == B_POSITION_PLAYER_RIGHT && !IS_DOUBLE_BATTLE)
            break;
        bank = GetBattlerAtPosition(position);
        if (bank >= gBattlersCount || !IsBattlerAlive(bank))
            continue;
        partyId = gBattlerPartyIndexes[bank];
        if (partyId >= PARTY_SIZE
         || (gBattleTypeFlags & BATTLE_TYPE_INGAME_PARTNER && partyId >= PARTY_SIZE / 2)
         || !(gLeveledUpInBattle & gBitTable[partyId])
         || (gNewBS->midBattleEvolution.cancelledParty & gBitTable[partyId]))
            continue;

        mon = &gPlayerParty[partyId];
        // Defer temporary battle forms; keep the level-up bit for later/post-battle.
        if (IS_TRANSFORMED(bank) || IsDynamaxed(bank) || IsTerastallized(bank)
         || SPECIES(bank) != mon->species || gMonSpritesGfxPtr == NULL)
            continue;
        species = GetEvolutionTargetSpecies(mon, EVO_MODE_NORMAL, ITEM_NONE);
        if (species == SPECIES_NONE || species >= NUM_SPECIES)
            continue;
        taskId = CreateTask(Task_BeginBattleEvolutionScene, 0);
        if (taskId >= NUM_TASKS)
            return;

        gLeveledUpInBattle &= ~gBitTable[partyId];
        gNewBS->midBattleEvolution.battler = bank;
        gNewBS->midBattleEvolution.savedTerrain = gBattleTerrain;
        memcpy(gNewBS->midBattleEvolution.savedCommunication, gBattleCommunication, sizeof(gBattleCommunication));
        memcpy(gNewBS->midBattleEvolution.originalMoves, mon->moves, sizeof(mon->moves));
        gNewBS->midBattleEvolution.active = TRUE;
        // Recalculation must start from the current HP, not heal battle damage.
        SetMonData(mon, MON_DATA_HP, &gBattleMons[bank].hp);
        SetMonData(mon, MON_DATA_STATUS, &gBattleMons[bank].status1);
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
        gBattleMainFunc = WaitForEvolutionThenTryAnother;
        gTasks[taskId].tSpeciesToEvolveInto = species;
        gTasks[taskId].tPartyToEvolve = partyId;
        return;
    }

    gBattleMainFunc = HandleTurnActionSelectionState;
}

static void WaitForEvolutionThenTryAnother(void)
{
    if (!gNewBS->midBattleEvolution.active
     && gMain.callback2 == BattleMainCB2 && !gPaletteFade->active)
        gBattleMainFunc = PlayerTryEvolution;
}

static void EvolutionScene(struct Pokemon* mon, u16 postEvoSpecies, bool8 canStopEvo, u8 partyId)
{
    u8 name[20];
    u16 currSpecies, spriteSpecies;
    u32 trainerId, personality;
    const struct CompressedSpritePalette* pokePal;
    u8 id;

    sEvoStructPtr = AllocZeroed(sizeof(struct EvoInfo));
    if (sEvoStructPtr == NULL)
    {
        // Defer to post-battle rather than loop forever after an allocation failure.
        gLeveledUpInBattle |= gBitTable[partyId];
        gNewBS->midBattleEvolution.cancelledParty |= gBitTable[partyId];
        SetMainCallback2(gCB2_AfterEvolution);
        return;
    }
    FreeAllWindowBuffers();
    SetHBlankCallback(NULL);
    SetVBlankCallback(NULL);
    CpuFill32(0, (void *)(VRAM), VRAM_SIZE);

    SetGpuReg(REG_OFFSET_MOSAIC, 0);
    SetGpuReg(REG_OFFSET_WIN0H, 0);
    SetGpuReg(REG_OFFSET_WIN0V, 0);
    SetGpuReg(REG_OFFSET_WIN1H, 0);
    SetGpuReg(REG_OFFSET_WIN1V, 0);
    SetGpuReg(REG_OFFSET_WININ, 0);
    SetGpuReg(REG_OFFSET_WINOUT, 0);

    ResetPaletteFade();

    gBattle_BG0_X = 0;
    gBattle_BG0_Y = 0;
    gBattle_BG1_X = 0;
    gBattle_BG1_Y = 0;
    gBattle_BG2_X = 0;
    gBattle_BG2_Y = 0;
    gBattle_BG3_X = 256;
    gBattle_BG3_Y = 0;

    gBattleTerrain = BATTLE_TERRAIN_PLAIN;

    InitBattleBgsVideo();
    LoadBattleTextboxAndBackground();
    ResetSpriteData();
    ScanlineEffect_Stop();
    ResetTasks();
    FreeAllSpritePalettes();

    gReservedSpritePaletteCount = 4;

    GetMonData(mon, MON_DATA_NICKNAME, name);
    StringCopy_Nickname(gStringVar1, name);
    StringCopy(gStringVar2, gSpeciesNames[postEvoSpecies]);

    // preEvo sprite
    currSpecies = GetMonData(mon, MON_DATA_SPECIES, NULL);
    trainerId = GetMonData(mon, MON_DATA_OT_ID, NULL);
    personality = GetMonData(mon, MON_DATA_PERSONALITY, NULL);

    spriteSpecies = TryGetFemaleGenderedSpecies(currSpecies, personality);

    DecompressPicFromTable(&gMonFrontPicTable[spriteSpecies],
                             gMonSpritesGfxPtr->sprites[B_POSITION_OPPONENT_LEFT],
                             spriteSpecies);
    pokePal = GetMonSpritePalStructFromOtIdPersonality(spriteSpecies, trainerId, personality);
    LoadCompressedPalette(pokePal->data, OBJ_PLTT_ID(1), PLTT_SIZE_4BPP);

    SetMultiuseSpriteTemplateToPokemon(spriteSpecies, B_POSITION_OPPONENT_LEFT);
    gMultiuseSpriteTemplate->affineAnims = gDummySpriteAffineAnimTable;
    sEvoStructPtr->preEvoSpriteId = id = CreateSprite(gMultiuseSpriteTemplate, 120, 64, 30);

    gSprites[id].callback = SpriteCallbackDummy_2;
    gSprites[id].oam.paletteNum = 1;
    gSprites[id].invisible = TRUE;

    spriteSpecies = TryGetFemaleGenderedSpecies(postEvoSpecies, personality);

    // postEvo sprite
    DecompressPicFromTable(&gMonFrontPicTable[spriteSpecies],
                             gMonSpritesGfxPtr->sprites[B_POSITION_OPPONENT_RIGHT],
                             spriteSpecies);
    pokePal = GetMonSpritePalStructFromOtIdPersonality(spriteSpecies, trainerId, personality);
    LoadCompressedPalette(pokePal->data, OBJ_PLTT_ID(2), PLTT_SIZE_4BPP);

    SetMultiuseSpriteTemplateToPokemon(spriteSpecies, B_POSITION_OPPONENT_RIGHT);
    gMultiuseSpriteTemplate->affineAnims = gDummySpriteAffineAnimTable;
    sEvoStructPtr->postEvoSpriteId = id = CreateSprite(gMultiuseSpriteTemplate, 120, 64, 30);
    gSprites[id].callback = SpriteCallbackDummy_2;
    gSprites[id].oam.paletteNum = 2;
    gSprites[id].invisible = TRUE;

    LoadEvoSparkleSpriteAndPal();

    sEvoStructPtr->evoTaskId = id = CreateTask(Task_EvolutionScene, 0);
    gTasks[id].tState = 0;
    gTasks[id].tPreEvoSpecies = currSpecies;
    gTasks[id].tPostEvoSpecies = postEvoSpecies;
    gTasks[id].tCanStop = canStopEvo;
    gTasks[id].tLearnsFirstMove = TRUE;
    gTasks[id].tEvoWasStopped = FALSE;
    gTasks[id].tPartyId = partyId;

    memcpy(&sEvoStructPtr->savedPalette, &gPlttBufferUnfaded[BG_PLTT_ID(2)], sizeof(sEvoStructPtr->savedPalette));

    SetGpuReg(REG_OFFSET_DISPCNT, DISPCNT_OBJ_ON | DISPCNT_BG_ALL_ON | DISPCNT_OBJ_1D_MAP);

    SetHBlankCallback(EvoDummyFunc);
    SetVBlankCallback(VBlankCB_EvolutionScene);
    m4aMPlayAllStop();
    HelpSystem_Disable();
    SetMainCallback2(CB2_EvolutionSceneUpdate);
}

static void Task_EvolutionScene(u8 taskId)
{
    u32 var;
    struct Pokemon* mon = &gPlayerParty[gTasks[taskId].tPartyId];

    // check if B Button was held, so the evolution gets stopped
    if (gMain.heldKeys & B_BUTTON
        && gTasks[taskId].tState == EVOSTATE_WAIT_CYCLE_MON_SPRITE
        && gTasks[sEvoGraphicsTaskId].isActive
        && gTasks[taskId].tBits & TASK_BIT_CAN_STOP)
    {
        gTasks[taskId].tState = EVOSTATE_CANCEL;
        gNewBS->midBattleEvolution.cancelledParty |= gBitTable[gTasks[taskId].tPartyId];
        gTasks[sEvoGraphicsTaskId].tEvoStopped = TRUE;
        StopBgAnimation();
        return;
    }

    switch (gTasks[taskId].tState)
    {
    case EVOSTATE_FADE_IN:
        BeginNormalPaletteFade(PALETTES_ALL, 0, 0x10, 0, RGB_BLACK);
        gSprites[sEvoStructPtr->preEvoSpriteId].invisible = FALSE;
        gTasks[taskId].tState++;
        ShowBg(0);
        ShowBg(1);
        ShowBg(2);
        ShowBg(3);
        break;
    case EVOSTATE_INTRO_MSG:
        if (!gPaletteFade->active)
        {
            StringExpandPlaceholders(gStringVar4, (u8*)0x83fe672);
            BattlePutTextOnWindow(gStringVar4, B_WIN_MSG);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_INTRO_MON_ANIM:
        if (!IsTextPrinterActive(0))
        {
            PlayCry1(gTasks[taskId].tPreEvoSpecies, 0);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_INTRO_SOUND:
        if (IsCryFinished()) // wait for animation, play tu du SE
        {
            PlaySE(MUS_EVOLUTION_INTRO);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_START_MUSIC:
        if (!IsSEPlaying())
        {
            // Start music, fade background to black
            PlayNewMapMusic(MUS_EVOLUTION);
            gTasks[taskId].tState++;
            BeginNormalPaletteFade(0x1C, 4, 0, 0x10, RGB_BLACK);
        }
        break;
    case EVOSTATE_START_BG_AND_SPARKLE_SPIRAL:
        if (!gPaletteFade->active)
        {
            StartBgAnimation(FALSE);
            sEvoGraphicsTaskId = EvolutionSparkles_SpiralUpward(17);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_SPARKLE_ARC:
        if (!gTasks[sEvoGraphicsTaskId].isActive)
        {
            gTasks[taskId].tState++;
            sEvoStructPtr->delayTimer = 1;
            sEvoGraphicsTaskId = EvolutionSparkles_ArcDown();
        }
        break;
    case EVOSTATE_CYCLE_MON_SPRITE:
        if (!gTasks[sEvoGraphicsTaskId].isActive)
        {
            sEvoGraphicsTaskId = CycleEvolutionMonSprite(sEvoStructPtr->preEvoSpriteId, sEvoStructPtr->postEvoSpriteId);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_WAIT_CYCLE_MON_SPRITE:
        if (--sEvoStructPtr->delayTimer == 0)
        {
            sEvoStructPtr->delayTimer = 3;
            if (!gTasks[sEvoGraphicsTaskId].isActive)
                gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_SPARKLE_CIRCLE:
        sEvoGraphicsTaskId = EvolutionSparkles_CircleInward();
        gTasks[taskId].tState++;
        break;
    case EVOSTATE_SPARKLE_SPRAY:
        if (!gTasks[sEvoGraphicsTaskId].isActive)
        {
            sEvoGraphicsTaskId = EvolutionSparkles_SprayAndFlash(gTasks[taskId].tPostEvoSpecies);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_EVO_SOUND:
        if (!gTasks[sEvoGraphicsTaskId].isActive)
        {
            PlaySE(SE_EXP);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_RESTORE_SCREEN:
        if (IsSEPlaying())
        {
            m4aMPlayAllStop();
            memcpy(&gPlttBufferUnfaded[BG_PLTT_ID(2)], sEvoStructPtr->savedPalette, sizeof(sEvoStructPtr->savedPalette));
            RestoreBgAfterAnim();
            BeginNormalPaletteFade(0x1C, 0, 0x10, 0, RGB_BLACK);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_EVO_MON_ANIM:
        if (!gPaletteFade->active)
        {
            PlayCry1(gTasks[taskId].tPostEvoSpecies, 0);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_SET_MON_EVOLVED:
        if (IsCryFinished())
        {
            StringExpandPlaceholders(gStringVar4, (u8*)0x83fe688);
            BattlePutTextOnWindow(gStringVar4, B_WIN_MSG);
            PlayBGM(MUS_EVOLVED);
            gTasks[taskId].tState++;
            SetMonData(mon, MON_DATA_SPECIES, (void *)(&gTasks[taskId].tPostEvoSpecies));
            CalculateMonStats(mon);
            EvolutionRenameMon(mon, gTasks[taskId].tPreEvoSpecies, gTasks[taskId].tPostEvoSpecies);
            GetSetPokedexFlag(SpeciesToNationalPokedexNum(gTasks[taskId].tPostEvoSpecies), FLAG_SET_SEEN);
            GetSetPokedexFlag(SpeciesToNationalPokedexNum(gTasks[taskId].tPostEvoSpecies), FLAG_SET_CAUGHT);
            IncrementGameStat(GAME_STAT_EVOLVED_POKEMON);
            UpdateEvolvedBattleMon(mon);
        }
        break;
    case EVOSTATE_TRY_LEARN_MOVE:
        if (!IsTextPrinterActive(0))
        {
            HelpSystem_Enable();
            var = gTasks[taskId].tEvoWasStopped ? MOVE_NONE
                : MonTryLearningNewMoveAfterEvolution(mon, gTasks[taskId].tLearnsFirstMove);
            if (var != MOVE_NONE && !gTasks[taskId].tEvoWasStopped)
            {
                u8 text[20];

                if (!(gTasks[taskId].tBits & TASK_BIT_LEARN_MOVE))
                {
                    StopMapMusic();
                    PlayBattleBGM();
                }
                gTasks[taskId].tBits |= TASK_BIT_LEARN_MOVE;
                gTasks[taskId].tLearnsFirstMove = FALSE;
                gTasks[taskId].tLearnMoveState = MVSTATE_INTRO_MSG_1;
                GetMonData(mon, MON_DATA_NICKNAME, text);
                StringCopy_Nickname(gBattleTextBuff1, text);

                if (var == MON_HAS_MAX_MOVES)
                    gTasks[taskId].tState = EVOSTATE_REPLACE_MOVE;
                else if (var == MON_ALREADY_KNOWS_MOVE)
                    break;
                else
                {
                    gTasks[taskId].tState = EVOSTATE_LEARNED_MOVE;
                }
            }
            else // no move to learn, or evolution was canceled
            {
                BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
                gTasks[taskId].tState++;
            }
        }
        break;
    case EVOSTATE_END:
        if (!gPaletteFade->active)
        {
            if (!(gTasks[taskId].tBits & TASK_BIT_LEARN_MOVE))
            {
                StopMapMusic();
                if (gMain.inBattle && gBattleOutcome == 0)
                    PlayBattleBGM(); // If battle is still ongoing, replay battle music
                else
                    Overworld_PlaySpecialMapMusic();
            }
            if (!gTasks[taskId].tEvoWasStopped)
            {
                UpdateEvolvedBattleMoves(mon);
                CreateShedinja(gTasks[taskId].tPreEvoSpecies, mon);
            }

            DestroyTask(taskId);
            FREE_AND_SET_NULL(sEvoStructPtr);
            FreeAllWindowBuffers();
            SetMainCallback2(gCB2_AfterEvolution);
        }
        break;
    case EVOSTATE_CANCEL:
        if (!gTasks[sEvoGraphicsTaskId].isActive)
        {
            m4aMPlayAllStop();
            BeginNormalPaletteFade(0x6001C, 0, 0x10, 0, RGB_WHITE);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_CANCEL_MON_ANIM:
        if (!gPaletteFade->active)
        {
            PlayCry1(gTasks[taskId].tPreEvoSpecies, 0);
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_CANCEL_MSG:
        if (IsCryFinished())
        {
            if (gTasks[taskId].tEvoWasStopped)
                StringExpandPlaceholders(gStringVar4, (u8*)0x83fe6d0);
            else
                StringExpandPlaceholders(gStringVar4, (u8*)0x83fe6b5);

            BattlePutTextOnWindow(gStringVar4, B_WIN_MSG);
            gTasks[taskId].tEvoWasStopped = TRUE;
            gTasks[taskId].tState = EVOSTATE_TRY_LEARN_MOVE;
        }
        break;
    case EVOSTATE_LEARNED_MOVE:
        if (!IsTextPrinterActive(0) && !IsSEPlaying())
        {
            BufferMoveToLearnIntoBattleTextBuff2();
            PlayFanfare(MUS_LEVEL_UP);
            BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_PKMNLEARNEDMOVE - BATTLESTRINGS_TABLE_START]);
            BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
            gTasks[taskId].tLearnsFirstMove = 0x40; // re-used as a counter
            gTasks[taskId].tState++;
        }
        break;
    case EVOSTATE_TRY_LEARN_ANOTHER_MOVE:
        if (!IsTextPrinterActive(0) && !IsSEPlaying() && --gTasks[taskId].tLearnsFirstMove == 0)
            gTasks[taskId].tState = EVOSTATE_TRY_LEARN_MOVE;
        break;
    case EVOSTATE_REPLACE_MOVE:
        switch (gTasks[taskId].tLearnMoveState)
        {
        case MVSTATE_INTRO_MSG_1:
            if (!IsTextPrinterActive(0) && !IsSEPlaying())
            {
                // "{mon} is trying to learn {move}"
                BufferMoveToLearnIntoBattleTextBuff2();
                BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_TRYTOLEARNMOVE1 - BATTLESTRINGS_TABLE_START]);
                BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
                gTasks[taskId].tLearnMoveState++;
            }
            break;
        case MVSTATE_INTRO_MSG_2:
            if (!IsTextPrinterActive(0) && !IsSEPlaying())
            {
                // "But, {mon} can't learn more than four moves"
                BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_TRYTOLEARNMOVE2 - BATTLESTRINGS_TABLE_START]);
                BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
                gTasks[taskId].tLearnMoveState++;
            }
            break;
        case MVSTATE_INTRO_MSG_3:
            if (!IsTextPrinterActive(0) && !IsSEPlaying())
            {
                // "Delete a move to make room for {move}?"
                BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_TRYTOLEARNMOVE3 - BATTLESTRINGS_TABLE_START]);
                BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
                gTasks[taskId].tLearnMoveYesState = MVSTATE_SHOW_MOVE_SELECT;
                gTasks[taskId].tLearnMoveNoState = MVSTATE_ASK_CANCEL;
                gTasks[taskId].tLearnMoveState++;
            }
            break;
        case MVSTATE_PRINT_YES_NO:
            if (!IsTextPrinterActive(0) && !IsSEPlaying())
            {
                HandleBattleWindow(23, 8, 29, 13, 0);
                BattlePutTextOnWindow((u8*)0x83fe791, B_WIN_YESNO);
                gTasks[taskId].tLearnMoveState++;
                sEvoCursorPos = 0;
                BattleCreateYesNoCursorAt2();
            }
            break;
        case MVSTATE_HANDLE_YES_NO:
            // This Yes/No is used for both the initial "delete move?" prompt
            // and for the "stop learning move?" prompt
            // What Yes/No do next is determined by tLearnMoveYesState / tLearnMoveNoState
            if (JOY_NEW(DPAD_UP) && sEvoCursorPos != 0)
            {
                // Moved onto YES
                PlaySE(SE_SELECT);
                BattleDestroyYesNoCursorAt2();
                sEvoCursorPos = 0;
                BattleCreateYesNoCursorAt2();
            }
            if (JOY_NEW(DPAD_DOWN) && sEvoCursorPos == 0)
            {
                // Moved onto NO
                PlaySE(SE_SELECT);
                BattleDestroyYesNoCursorAt2();
                sEvoCursorPos = 1;
                BattleCreateYesNoCursorAt2();
            }
            if (JOY_NEW(A_BUTTON))
            {
                HandleBattleWindow(0x17, 8, 0x1D, 0xD, WINDOW_CLEAR);
                PlaySE(SE_SELECT);

                if (sEvoCursorPos != 0)
                {
                    // NO
                    gTasks[taskId].tLearnMoveState = gTasks[taskId].tLearnMoveNoState;
                }
                else
                {
                    // YES
                    gTasks[taskId].tLearnMoveState = gTasks[taskId].tLearnMoveYesState;
                    if (gTasks[taskId].tLearnMoveState == MVSTATE_SHOW_MOVE_SELECT)
                        BeginNormalPaletteFade(PALETTES_ALL, 0, 0, 0x10, RGB_BLACK);
                }
            }
            if (JOY_NEW(B_BUTTON))
            {
                // Equivalent to selecting NO
                HandleBattleWindow(0x17, 8, 0x1D, 0xD, WINDOW_CLEAR);
                PlaySE(SE_SELECT);
                gTasks[taskId].tLearnMoveState = gTasks[taskId].tLearnMoveNoState;
            }
            break;
        case MVSTATE_SHOW_MOVE_SELECT:
            if (!gPaletteFade->active)
            {
                FreeAllWindowBuffers();
                ShowSelectMovePokemonSummaryScreen(gPlayerParty, gTasks[taskId].tPartyId,
                            gPlayerPartyCount - 1, CB2_MidBattleEvolutionLoadGraphics,
                            gMoveToLearn);
                gTasks[taskId].tLearnMoveState++;
            }
            break;
        case MVSTATE_HANDLE_MOVE_SELECT:
            if (!gPaletteFade->active && gMain.callback2 == CB2_EvolutionSceneUpdate)
            {
                var = GetMoveSlotToReplace();
                if (var == MAX_MON_MOVES)
                {
                    // Didn't select move slot
                    gTasks[taskId].tLearnMoveState = MVSTATE_ASK_CANCEL;
                }
                else
                {
                    // Selected move to forget
                    u16 move = GetMonData(mon, var + MON_DATA_MOVE1, NULL);
                    if (IsHMMove2(move))
                    {
                        // Can't forget HMs
                        BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_HMMOVESCANTBEFORGOTTEN - BATTLESTRINGS_TABLE_START]);
                        BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
                        gTasks[taskId].tLearnMoveState = MVSTATE_RETRY_AFTER_HM;
                    }
                    else
                    {
                        // Forget move
                        PREPARE_MOVE_BUFFER(gBattleTextBuff2, move)
                        RemoveMonPPBonus(mon, var);
                        SetMonMoveSlot(mon, gMoveToLearn, var);
                        gTasks[taskId].tLearnMoveState++;
                    }
                }
            }
            break;
        case MVSTATE_FORGET_MSG_1:
            BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_123POOF - BATTLESTRINGS_TABLE_START]);
            BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
            gTasks[taskId].tLearnMoveState++;
            break;
        case MVSTATE_FORGET_MSG_2:
            if (!IsTextPrinterActive(0) && !IsSEPlaying())
            {
                BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_PKMNFORGOTMOVE - BATTLESTRINGS_TABLE_START]);
                BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
                gTasks[taskId].tLearnMoveState++;
            }
            break;
        case MVSTATE_LEARNED_MOVE:
            if (!IsTextPrinterActive(0) && !IsSEPlaying())
            {
                BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_ANDELLIPSIS - BATTLESTRINGS_TABLE_START]);
                BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
                gTasks[taskId].tState = EVOSTATE_LEARNED_MOVE;
            }
            break;
        case MVSTATE_ASK_CANCEL:
            BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_STOPLEARNINGMOVE - BATTLESTRINGS_TABLE_START]);
            BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
            gTasks[taskId].tLearnMoveYesState = MVSTATE_CANCEL;
            gTasks[taskId].tLearnMoveNoState = MVSTATE_INTRO_MSG_1;
            gTasks[taskId].tLearnMoveState = MVSTATE_PRINT_YES_NO;
            break;
        case MVSTATE_CANCEL:
            BattleStringExpandPlaceholdersToDisplayedString(gBattleStringsTable[STRINGID_DIDNOTLEARNMOVE - BATTLESTRINGS_TABLE_START]);
            BattlePutTextOnWindow(gDisplayedStringBattle, B_WIN_MSG);
            gTasks[taskId].tState = EVOSTATE_TRY_LEARN_MOVE;
            break;
        case MVSTATE_RETRY_AFTER_HM:
            if (!IsTextPrinterActive(0) && !IsSEPlaying())
                gTasks[taskId].tLearnMoveState = MVSTATE_SHOW_MOVE_SELECT;
            break;
        }
        break;
    }
}

// Only update fields changed by evolution. This is not a switch-in/revive:
// preserve consumed items, volatile statuses, stat stages, Substitute and timers.
static void UpdateEvolvedBattleMon(struct Pokemon *mon)
{
    u8 bank = gNewBS->midBattleEvolution.battler;
    u16 species = GetMonData(mon, MON_DATA_SPECIES, NULL);

    gBattleMons[bank].species = species;
    RELOAD_BATTLE_STATS(bank, mon);
    if (gBattleTypeFlags & BATTLE_TYPE_CAMOMONS)
        UpdateTypesForCamomons(bank);
    else
    {
        gBattleMons[bank].type1 = gBaseStats[species].type1;
        gBattleMons[bank].type2 = gBaseStats[species].type2;
    }
    gBattleMons[bank].type3 = TYPE_BLANK;
    *GetAbilityLocation(bank) = GetMonAbility(mon);
    ClearBattlerAbilityHistory(bank);
    GetMonData(mon, MON_DATA_NICKNAME, gBattleMons[bank].nickname);
    gStatuses3[bank] &= ~(STATUS3_SWITCH_IN_ABILITY_DONE | STATUS3_ILLUSION);
    ClearTemporarySpeciesSpriteData(bank, TRUE);
}

static void UpdateEvolvedBattleMoves(struct Pokemon *mon)
{
    u8 bank = gNewBS->midBattleEvolution.battler;
    u8 i;

    for (i = 0; i < MAX_MON_MOVES; ++i)
    {
        // Do not erase a temporary Mimic move, or reset PP in unchanged slots.
        if (mon->moves[i] != gNewBS->midBattleEvolution.originalMoves[i])
        {
            gBattleMons[bank].moves[i] = mon->moves[i];
            gBattleMons[bank].pp[i] = mon->pp[i];
            gBattleMons[bank].ppBonuses = (gBattleMons[bank].ppBonuses & ~(3 << (i * 2)))
                                      | (mon->ppBonuses & (3 << (i * 2)));
            gDisableStructs[bank].mimickedMoves &= ~gBitTable[i];
        }
    }
    if (gBattleTypeFlags & BATTLE_TYPE_CAMOMONS)
        UpdateTypesForCamomons(bank);
}

static void CB2_MidBattleEvolutionLoadGraphics(void)
{
    u8 taskId = sEvoStructPtr->evoTaskId;
    u16 species = gTasks[taskId].tPostEvoSpecies;
    struct Pokemon *mon = &gPlayerParty[gTasks[taskId].tPartyId];

    // The ROM reloader reads this task field for graphics only. Keep the
    // actual species for learnsets, nickname updates and Shedinja creation.
    gTasks[taskId].tPostEvoSpecies = TryGetFemaleGenderedSpecies(species, mon->personality);
    CB2_EvolutionSceneLoadGraphics();
    gTasks[taskId].tPostEvoSpecies = species;
}

static u16 TryGetFemaleGenderedSpecies(u16 species, u32 personality)
{
	if (GetGenderFromSpeciesAndPersonality(species, personality) == MON_FEMALE)
	{
		switch (species) {
			case SPECIES_HIPPOPOTAS:
				species = SPECIES_HIPPOPOTAS_F;
				break;
			case SPECIES_HIPPOWDON:
				species = SPECIES_HIPPOWDON_F;
				break;
			case SPECIES_UNFEZANT:
				species = SPECIES_UNFEZANT_F;
				break;
			case SPECIES_FRILLISH:
				species = SPECIES_FRILLISH_F;
				break;
			case SPECIES_JELLICENT:
				species = SPECIES_JELLICENT_F;
				break;
			case SPECIES_PYROAR:
				species = SPECIES_PYROAR_FEMALE;
				break;
		}
	}
	else if (species == SPECIES_XERNEAS && !gMain.inBattle)
		species = SPECIES_XERNEAS_NATURAL;
	
	return species;
}
#endif
