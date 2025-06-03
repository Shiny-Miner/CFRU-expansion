#ifndef GUARD_CONSTANTS_QUESTS_H
#define GUARD_CONSTANTS_QUESTS_H

//questmenu scripting command params
#define QUEST_MENU_OPEN                 0   //opens the quest menu (questId = 0)
#define QUEST_MENU_UNLOCK_QUEST         1   //questId = SIDE_QUEST_X (0-indexed)
#define QUEST_MENU_COMPLETE_QUEST       2   //questId = SIDE_QUEST_X (0-indexed)
#define QUEST_MENU_SET_ACTIVE           3   //questId = SIDE_QUEST_X (0-indexed)
#define QUEST_MENU_RESET_ACTIVE         4   //remove current quest from being 'active'
#define QUEST_MENU_BUFFER_QUEST_NAME    5   //buffers a quest name to gStringVar1
#define QUEST_MENU_GET_ACTIVE_QUEST     6   //returns the active quest index (-1, or 0xFF for none)
#define QUEST_MENU_CHECK_UNLOCKED       7   //checks if questId has been unlocked. Returns result to gSpecialVar_Result
#define QUEST_MENU_CHECK_COMPLETE       8   //checks if questId has been completed. Returns result to gSpecialVar_Result

// quest number defines
#define SIDE_QUEST_1          0
#define SIDE_QUEST_2          1
#define SIDE_QUEST_3          2
#define SIDE_QUEST_4          3
#define SIDE_QUEST_5          4
#define SIDE_QUEST_6          5
#define SIDE_QUEST_7          6
#define SIDE_QUEST_8          7
#define SIDE_QUEST_9          8
#define SIDE_QUEST_10         9
#define SIDE_QUEST_11        10
#define SIDE_QUEST_12        11
#define SIDE_QUEST_13        12
#define SIDE_QUEST_14        13
#define SIDE_QUEST_15        14
#define SIDE_QUEST_16        15
#define SIDE_QUEST_17        16
#define SIDE_QUEST_18        17
#define SIDE_QUEST_19        18
#define SIDE_QUEST_20        19
#define SIDE_QUEST_21        20
#define SIDE_QUEST_22        21
#define SIDE_QUEST_23        22
#define SIDE_QUEST_24        23
#define SIDE_QUEST_25        24
#define SIDE_QUEST_26        25
#define SIDE_QUEST_27        26
#define SIDE_QUEST_28        27
#define SIDE_QUEST_29        28
#define SIDE_QUEST_30        29
#define SIDE_QUEST_COUNT     SIDE_QUEST_30 + 1

#endif // GUARD_CONSTANTS_QUESTS_H