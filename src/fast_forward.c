#include "defines.h"
#include "../include/event_data.h"
#include "../include/link.h"
#include "../include/main.h"
#include "../include/field_player_avatar.h"
#include "../include/overworld.h"
#include "../include/script.h"
#include "../include/new/read_keys.h"

/*
fast_forward.c
	Decomp-style fast forward: toggle with L + R and the game logic runs several times per
	real frame, while music keeps playing at normal speed.

	Why music is unaffected: m4aSoundMain() is called from VBlankIntr (interrupt), not from
	the main loop, so the sound engine is locked to the real 59.7 Hz. We only skip the
	"wait for next VBlank" at the end of AgbMain's loop.

	Baseline (needs only the gWirelessCommType line in BPRE.ld): while toggled on,
	never wait for VBlank. Extras are OFF by default; enable ONE at a time to see which
	one you want / which one breaks.
*/

// Uncomment to never fast forward during battles (L + R still toggles the flag there).
//#define FF_DISABLE_IN_BATTLE

// Stopgap: run at normal speed while surfing in the overworld. The toggle stays on, so fast forward
// resumes as soon as you leave the water. Comment this out to fast forward while surfing as well.
#define FF_PAUSE_WHILE_SURFING

#define FF_FADE_BOOST
#define SPLTT_BUFFER_TRANSFER_PENDING (*(vu32*) 0x2037AC8)
#ifdef FF_SKIP_DURING_MAP_LOAD
#include "../include/overworld.h"
void CB2_LoadMap2(void);
#endif

bool8 ShouldSkipVBlankWait(void)
{
	#ifdef FLAG_FAST_FORWARD
	if (FF_LR_PENDING && (gMain.callback2 != CB2_Overworld || ScriptContext2_IsEnabled()))
		FF_LR_PENDING = 0;
	#endif

	if (gReceivedRemoteLinkPlayers || gWirelessCommType)
		return FALSE;
	#ifdef FLAG_FAST_FORWARD
	if ((gMain.heldKeysRaw & (L_BUTTON | R_BUTTON)) == (L_BUTTON | R_BUTTON)
	&& (gMain.newKeysRaw & (L_BUTTON | R_BUTTON)))
	{
		FF_TOGGLE_STATE ^= 1;
	}
	#endif

	#ifdef FF_DISABLE_IN_BATTLE
	if (gMain.inBattle)
		return FALSE;
	#endif

	#ifdef FLAG_FAST_FORWARD
	if (!FF_TOGGLE_STATE)
		return FALSE;
	#else
	return FALSE;
	#endif

	#ifdef FF_PAUSE_WHILE_SURFING
	if (gMain.callback2 == CB2_Overworld && TestPlayerAvatarFlags(PLAYER_AVATAR_FLAG_SURFING))
		return FALSE;
	#endif

	#ifdef FF_SKIP_DURING_MAP_LOAD
	if (gMain.callback2 == CB2_LoadMap || gMain.callback2 == CB2_LoadMap2)
		return FALSE;
	#endif

	#ifdef FF_FADE_BOOST
	SPLTT_BUFFER_TRANSFER_PENDING = 0;
	#endif

	return TRUE;
}
