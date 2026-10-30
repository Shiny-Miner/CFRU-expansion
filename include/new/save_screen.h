#pragma once

/*
 * Gen-5 style "Save" screen for the Start Menu.
 * See src/save_screen.c
 */

//Start Menu callback (replaces the vanilla StartMenuSaveCallback in sStartMenuActionTable)
bool8 StartMenuSaveScreenCallback(void);
