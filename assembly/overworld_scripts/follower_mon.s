.align 2
.thumb

.include "../xse_commands.s"
.include "../xse_defines.s"

.global EventScript_FollowerMon

EventScript_FollowerMon:
    followerfaceplayer
    lock
    callasm SetFollowerMonSpeciesVar
    bufferpokemon 0x0 0x8004
    callasm ShowMysteryGiftMon
    cry 0x8004 0x0
    callasm ShowAnonymousFollowerMessage
    callasm ShowFollowerEmote
    msgbox gStringVar4 MSG_NORMAL
    callasm Remove_PokemonPic
    release
    end
