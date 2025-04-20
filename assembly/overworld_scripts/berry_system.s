.thumb
.align 2

.include "../xse_commands.s"
.include "../xse_defines.s"

.global EventScript_BerrySoil
EventScript_BerrySoil:
    lockall
    msgbox gText_WouldYouLikeToPlantBerryHere MSG_YESNO
    compare LASTRESULT YES
    if equal _goto ReturnYes
    goto ReturnNo

ReturnYes:
    setvar LASTRESULT 1
    releaseall
    end

ReturnNo:
    setvar LASTRESULT 0
    releaseall
    end