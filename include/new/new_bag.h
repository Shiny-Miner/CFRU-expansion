#pragma once
#include "../global.h"

/**
 * \file new_bag.h
 * \brief New 6-tab bag screen (Potions/Items, Balls, TM/HM, Key Items, Mega Stones, Evolution).
 */

void NewBag_GoToBagMenu(u8 location, u8 pocket, void (*bagCallback)(void)); //Hook: vanilla GoToBagMenu (0x8107DB4)
void NewBag_DisplayItemMessageInBag(u8 taskId, u8 fontId, const u8* str, void (*followUp)(u8)); //Hook: 0x8108E70
void NewBag_ReturnToBagFromContextMenu(u8 taskId); //Hook: 0x810A1F8
void NewBag_BerryPouchSetExitCallback(void (*cb)(void));   //Hook: 0x813D934
void NewBag_BerryPouchStartFadeToExitCallback(u8 taskId); //Hook: 0x813D808
void NewBag_ReturnToTMCaseMenu(void);      //Hook: 0x8124DA0
void NewBag_ReturnToBerryPouchMenu(void);  //Hook: 0x8124DB0
