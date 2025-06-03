#ifndef GUARD_QUESTS_H
#define GUARD_QUESTS_H

//#define FLAG_QUEST_MENU_ACTIVE              0x200 //constants/flags.h
#define NO_ACTIVE_QUEST                     -1

//structs
struct SideQuest 
{
	/*0x00*/ const u8* name;
	/*0x04*/ const u8* desc;
	/*0x08*/ const u8* poc;
	/*0x0C*/ const u8* map;
	///*0x10*/ const u8* hint;
	/*0x10*/ const u8* reward;
}; /* size = 0x14 */

extern const struct SideQuest gSideQuests[SIDE_QUEST_COUNT];

enum 
{
	QUEST_DIFFICULTY_EASY,
	QUEST_DIFFICULTY_MEDIUM,
	QUEST_DIFFICULTY_HARD,
	QUEST_DIFFICULTY_EXTREME,
};

enum QuestCases
{
    FLAG_GET_UNLOCKED,      // check if quest is unlocked
    FLAG_SET_UNLOCKED,      // mark unlocked quest
    FLAG_GET_COMPLETED,     // check if quest is completed
    FLAG_SET_COMPLETED,     // mark completed quest
};

//functions
void SetQuestMenuActive(void);
void SetActiveQuest(u8 questId);
void CopyQuestName(u8 *dst, u8 questId);
s8 GetSetQuestFlag(u8 quest, u8 caseId);
void ResetActiveQuest(void);
s8 GetActiveQuestIndex(void);

#endif // GUARD_QUESTS_H
