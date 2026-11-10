#pragma once

#include "../global.h"

/**
 * \file read_keys.h
 * \brief Contains functions relating to the user's input being read in. This can be
 *		  used to set up a script to launch when the user presses certain keys.
 */

//Exported Functions
bool8 StartLButtonFunc(void);
bool8 StartRButtonFunc(void);
bool8 TryToggleFastForward(void);
struct FieldInput;
u8 HandleDeferredLRActions(struct FieldInput* input);
#ifdef DEBUG_OVERWORLD_MENU
bool8 DebugMenuComboPressed(void);
#endif

//Functions Hooked In
void ReadKeys(void);

//Exported Data Structures
// keypad struct
struct KeypadSetter
{
	/*0x0*/u16 keyMapToForce;	// key(s) to force
	/*0x2*/u16 keysToIgnore;		// key(s) to ignore
	/*0x4*/u8 keyForcingCounter;	// number of times to force said key(s)
	/*0x5*/u8 keyFlags;			// flags for specific key functions
	/*0x6*/u16 keyToRunScript;	// key to enable script to run
	/*0x8*/const u8* scriptToRun;	// script to run from specified key
};

#define gKeypadSetter ((struct KeypadSetter*) 0x203B7B0)

//Fast forward toggle (L + R). Lives outside the save block so New Game doesn't wipe it.
//0x203E0C7 is used by new_bag.c, so 0x203E0C8 is the free byte (limit is 0x203E0D4 for BTS).
#define FF_TOGGLE_STATE (*(vu8*) 0x203E0C8)

//Pending single L / R presses (bit 0 = L, bit 1 = R), see HandleDeferredLRActions. 0x203E0C9 is free as well.
#define FF_LR_PENDING (*(vu8*) 0x203E0C9)
