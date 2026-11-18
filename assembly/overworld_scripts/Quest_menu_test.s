.align 2
.thumb

.include "../xse_commands.s"
.include "../xse_defines.s"
.include "../asm_defines.s"

.global EventScript_QuestMenuTest_Setup
.global EventScript_QuestMenuTest_Open
.global EventScript_QuestMenuTest_Reset

.macro questcmd cmd, quest, sub=0
    setvar 0x8000 \cmd
    setvar 0x8001 \quest
    setvar 0x8002 \sub
    callasm QuestMenu_ScriptCommand+1
.endm

@ Talk to an NPC running this: fills the menu with a mix of states, then opens it.
@ Quest 1 Oak's Request: active, 4 of 10 sub quests done
@ Quest 2 Gym Challenge: active, 3 of 8 sub quests done
@ Quest 3 Parcel Delivery: reward ready
@ Quest 4 and 5: completed
@ Quest 6 and 9: active, no sub quests
@ Quest 7 and 8: unlocked, not started (A must do nothing)
@ Quest 10: reward ready
@ Quest 11: completed
@ Quest 12 and up: locked, shown as ??????
EventScript_QuestMenuTest_Setup:
    lock
    faceplayer
    questcmd QUEST_MENU_RESET, 0
    questcmd QUEST_MENU_SET_ACTIVE, QUEST_1
    questcmd QUEST_MENU_COMPLETE_SUBQUEST, QUEST_1, 0
    questcmd QUEST_MENU_COMPLETE_SUBQUEST, QUEST_1, 1
    questcmd QUEST_MENU_COMPLETE_SUBQUEST, QUEST_1, 2
    questcmd QUEST_MENU_COMPLETE_SUBQUEST, QUEST_1, 6
    questcmd QUEST_MENU_SET_ACTIVE, QUEST_2
    questcmd QUEST_MENU_COMPLETE_SUBQUEST, QUEST_2, 0
    questcmd QUEST_MENU_COMPLETE_SUBQUEST, QUEST_2, 1
    questcmd QUEST_MENU_COMPLETE_SUBQUEST, QUEST_2, 2
    questcmd QUEST_MENU_UNLOCK_QUEST, QUEST_3
    questcmd QUEST_MENU_SET_REWARD, QUEST_3
    questcmd QUEST_MENU_UNLOCK_QUEST, QUEST_4
    questcmd QUEST_MENU_COMPLETE_QUEST, QUEST_4
    questcmd QUEST_MENU_UNLOCK_QUEST, QUEST_5
    questcmd QUEST_MENU_COMPLETE_QUEST, QUEST_5
    questcmd QUEST_MENU_SET_ACTIVE, QUEST_6
    questcmd QUEST_MENU_UNLOCK_QUEST, QUEST_7
    questcmd QUEST_MENU_UNLOCK_QUEST, QUEST_8
    questcmd QUEST_MENU_SET_ACTIVE, QUEST_9
    questcmd QUEST_MENU_UNLOCK_QUEST, QUEST_10
    questcmd QUEST_MENU_SET_REWARD, QUEST_10
    questcmd QUEST_MENU_UNLOCK_QUEST, QUEST_11
    questcmd QUEST_MENU_COMPLETE_QUEST, QUEST_11
    callasm ShowQuestMenu+1
    waitstate
    release
    end

@ Opens the menu without touching any progress (use this to check that favorites,
@ filter state and progress survive closing and reopening)
EventScript_QuestMenuTest_Open:
    lock
    faceplayer
    callasm ShowQuestMenu+1
    waitstate
    release
    end

@ Clears all quest progress
EventScript_QuestMenuTest_Reset:
    lock
    faceplayer
    questcmd QUEST_MENU_RESET, 0
    release
    end
