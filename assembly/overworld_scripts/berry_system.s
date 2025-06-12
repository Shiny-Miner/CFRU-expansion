.thumb
.align 2

.include "../xse_commands.s"
.include "../xse_defines.s"
.include "../asm_defines.s"

.global EventScript_BerrySoil
EventScript_BerrySoil:
    lock
    faceplayer
    msgbox gText_WouldYouLikeToPlantBerryHere MSG_YESNO
    compare LASTRESULT YES
    if equal _goto ReturnYes
    goto ReturnNo

ReturnYes:
    setvar LASTRESULT 1
    giveitem ITEM_ORAN_BERRY 3 0
    giveitem ITEM_CHERI_BERRY 1 0
    giveitem ITEM_SITRUS_BERRY 1 0
    fadescreen 1
    setvar 0x8000 0x7E
    setvar 0x800E 0x0
    callasm BerryCall
    waitmsg
    setvar 0x8000 0x0
    compare 0x800E 0x0
    if equal _goto noberry
    bufferitem 0x0 0x800E
    msgbox planted 2
    release
    end

noberry:
    msgbox noplant 2
    release
    end

ReturnNo:
    setvar LASTRESULT 0
    release
    end
