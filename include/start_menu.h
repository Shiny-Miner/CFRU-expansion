#ifndef GUARD_START_MENU_H
#define GUARD_START_MENU_H

#include "global.h"

typedef void (*MainCallback)(void);

bool8 __attribute__((long_call)) IsUpdateLinkStateCBActive(void);
void __attribute__((long_call)) ShowStartMenu(void);
void __attribute__((long_call)) DestroySafariZoneStatsWindow(void);
void __attribute__((long_call)) AppendToList(u8* list, u8* pos, u8 newEntry);
void DrawTime(void);

//#define BW_START_MENU

struct StartMenuResources
{
  u8 cursorpos[2];
  u8* sBgTilemapBuffer;
  u8 NumStartMenuItems[2];
  u8 CurrentOptionsTable[2][13]; 
  u8 IconSpriteIds[6];
};   

#define sStartMenuPtr (*((struct StartMenuResources**) 0x203E038))

#endif // GUARD_START_MENU_H
