.align 2
.thumb

.include "../xse_commands.s"
.include "../xse_defines.s"
.include "../asm_defines.s"

.global EventScript_Pallet_FatGuy

EventScript_Pallet_FatGuy:
    faceplayer
    lock
    setflag 0x926
    setflag 0x828
    setflag 0x4BD
    givepokemon 0x11A 0x5 0x0 0x0 0x0 0x0
    givepokemon 0x510 0x5 0x0 0x0 0x0 0x0
    givepokemon 0x456 0x5 0x0 0x0 0x0 0x0
    givepokemon 0x22C 0x5 0x0 0x0 0x0 0x0
    givepokemon 0x99 0x5 0x0 0x0 0x0 0x0
    giveegg 0x1
    setvar 0x8001 0xFD
    special 0xD1
    msgbox gText_TestScript MSG_NORMAL
    release
    end

.align 2
.global EventScript_Pallet_Girl

EventScript_Pallet_Girl:
    faceplayer
    lock
    msgbox gText_PalletGirl_Text1 MSG_YESNO
    compare LASTRESULT 0x1
    if YES _goto PalletGirl_ShowFollowerMon
    setflag 0xa02
    setflag 0xa03
    hidefollowermon
    clearflag 0x4BD
    special 0xD2
    release
    end

PalletGirl_ShowFollowerMon:
    showfollowermon
    special 0xD1
    setflag 0x4BD
    release
    end

.align 2

.global EventScriptP_PalletTown_YoungLady
.global EventScript_Pallet_AideGuy

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
EventScriptP_PalletTown_YoungLady:
    lock
    faceplayer
    msgbox gText_PalletTown_YoungLadyAsk MSG_YESNO
    compare LASTRESULT FALSE
    if TRUE _goto EventScriptP_PalletTown_YoungLadyNo

    msgbox gText_PalletTown_YoungLadyOpenMenu MSG_NORMAL
    special 0x9F  @ Opens Pokemon selection menu
    waitstate

    @ Use vars 0x8002 and 0x8003 instead of 0x8004 and 0x800D
    copyvar 0x8002 0x8004
    copyvar 0x8003 0x800D

    compare 0x8002 0x7
    if TRUE _goto EventScriptP_PalletTown_YoungLadyNo
    goto EventScriptP_PalletTown_YoungLadySelected

EventScriptP_PalletTown_YoungLadyNo:
    msgbox gText_PalletTown_YoungLadyNo MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLadySelected:
    special2 0x8003 0x147
    bufferpokemon 0x0 0x8003
    msgbox gText_PalletTown_YoungLadySelected MSG_NORMAL

    @ Reset 0x8004 and 0x800D (Required for Scrolling Multichoice)
    resetvar 0x8004
    resetvar 0x800D

    @ Scrolling Multichoice
    setvar 0x8000 0 @ Multichoice Index
    setvar 0x8001 6 @ Number of options to display at once
    special 0x158   @ Scrolling Multichoice
    waitstate

    @ Based on Tera Type
    switch LASTRESULT
        case 0, EventScriptP_PalletTown_YoungLady_SetNormal
        case 1, EventScriptP_PalletTown_YoungLady_SetFighting
        case 2, EventScriptP_PalletTown_YoungLady_SetFlying
        case 3, EventScriptP_PalletTown_YoungLady_SetPoison
        case 4, EventScriptP_PalletTown_YoungLady_SetGround
        case 5, EventScriptP_PalletTown_YoungLady_SetRock
        case 6, EventScriptP_PalletTown_YoungLady_SetBug
        case 7, EventScriptP_PalletTown_YoungLady_SetGhost
        case 8, EventScriptP_PalletTown_YoungLady_SetSteel
        case 9, EventScriptP_PalletTown_YoungLady_SetFire
        case 10, EventScriptP_PalletTown_YoungLady_SetWater
        case 11, EventScriptP_PalletTown_YoungLady_SetGrass
        case 12, EventScriptP_PalletTown_YoungLady_SetElectric
        case 13, EventScriptP_PalletTown_YoungLady_SetPsychic
        case 14, EventScriptP_PalletTown_YoungLady_SetIce
        case 15, EventScriptP_PalletTown_YoungLady_SetDragon
        case 16, EventScriptP_PalletTown_YoungLady_SetDark
        case 17, EventScriptP_PalletTown_YoungLady_SetFairy
        case 18, EventScriptP_PalletTown_YoungLady_SetStellar
        case 0x7F, EventScriptP_PalletTown_YoungLadyNo


EventScriptP_PalletTown_YoungLady_SetNormal:
    setvar 0x8001 TYPE_NORMAL
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraNormal MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetFighting:
    setvar 0x8001 TYPE_FIGHTING
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraFighting MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetFlying:
    setvar 0x8001 TYPE_FLYING
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraFlying MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetPoison:
    setvar 0x8001 TYPE_POISON
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraPoison MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetGround:
    setvar 0x8001 TYPE_GROUND
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraGround MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetRock:
    setvar 0x8001 TYPE_ROCK
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraRock MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetBug:
    setvar 0x8001 TYPE_BUG
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraBug MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetGhost:
    setvar 0x8001 TYPE_GHOST
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraGhost MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetSteel:
    setvar 0x8001 TYPE_STEEL
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraSteel MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetFire:
    setvar 0x8001 TYPE_FIRE
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraFire MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetWater:
    setvar 0x8001 TYPE_WATER
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraWater MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetGrass:
    setvar 0x8001 TYPE_GRASS
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraGrass MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetElectric:
    setvar 0x8001 TYPE_ELECTRIC
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraElectric MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetPsychic:
    setvar 0x8001 TYPE_PSYCHIC
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraPsychic MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetIce:
    setvar 0x8001 TYPE_ICE
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraIce MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetDragon:
    setvar 0x8001 TYPE_DRAGON
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraDragon MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetDark:
    setvar 0x8001 TYPE_DARK
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraDark MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetFairy:
    setvar 0x8001 TYPE_FAIRY
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraFairy MSG_NORMAL
    release
    end

EventScriptP_PalletTown_YoungLady_SetStellar:
    setvar 0x8001 TYPE_STELLAR
    callasm ChangeTeraTypeInOW
    callasm GetTeraTypeInOW
    buffernumber 0x1 0x4001

    msgbox gText_PalletTown_YoungLadyFetchTeraStellar MSG_NORMAL
    release
    end

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
EventScript_Pallet_AideGuy:
    lock
    faceplayer
        setvar 0x8000 MOVE_GLACIALLANCE
        setvar 0x8001 MOVE_TACKLE
        setvar 0x8002 MOVE_THUNDERWAVE
        setvar 0x8003 0x2E7
        setvar 0x8004 NATURE_MODEST
        setvar 0x8005 0
        setvar 0x8006 16
        setvar 0x8007 16
        setvar 0x8008 16
        setvar 0x8009 16
        setvar 0x800A 16
        setvar 0x800B 16
        givepokemon SPECIES_GARDEVOIR 3 ITEM_LEFTOVERS 0 1 10
        setvar 0x8000 MOVE_WEATHERBALL
        setvar 0x8001 MOVE_THUNDERSHOCK
        setvar 0x8002 MOVE_THUNDERWAVE
        setvar 0x8003 0x2E7
        setvar 0x8004 NATURE_MODEST
        setvar 0x8005 0
        setvar 0x8006 16
        setvar 0x8007 16
        setvar 0x8008 16
        setvar 0x8009 16
        setvar 0x800A 16
        setvar 0x800B 16
        givepokemon SPECIES_GALLADE 2 ITEM_LEFTOVERS 0 1 10
        setflag 0xA08
        setflag 0x828
        additem 0x306 0x1
        msgbox gText_TestScript2 MSG_NORMAL
        callasm DebugSideQuestMenu
        openquestmenu
        release
        end

@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
