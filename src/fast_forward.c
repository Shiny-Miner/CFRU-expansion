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

// Extra 1: don't fast forward while a map is being loaded (warp garbled-screen fix attempt).
// Needs in BPRE.ld:  CB2_LoadMap2 = 0x805674C | 1;
//#define FF_SKIP_DURING_MAP_LOAD

// Extra 2: let palette fades (and so warp fades) run at fast-forward speed.
// UpdatePaletteFade() returns early while sPlttBufferTransferPending is set, and only the real
// VBlank clears it, so fades normally advance once per frame no matter how fast the logic runs.
// We clear it ourselves (cheap: no palette copy, the next real VBlank uploads the result).
// No BPRE.ld changes needed.
#define FF_FADE_BOOST
#define SPLTT_BUFFER_TRANSFER_PENDING (*(vu32*) 0x2037AC8)

// NOTE: no counter variable on purpose. CFRU's linker script puts .bss/.data inside the ROM,
// so a plain 'static u8' can never be written to.

#ifdef FF_SKIP_DURING_MAP_LOAD
#include "../include/overworld.h"
void CB2_LoadMap2(void);
#endif

// Called (via the asm hook) from AgbMain only. Returns TRUE to skip this frame's VBlank wait.
bool8 ShouldSkipVBlankWait(void)
{
	// A lone L / R press waits for release (see HandleDeferredLRActions). Drop it if the player
	// lost control in the meantime, so it can't fire later out of nowhere.
	#ifdef FLAG_FAST_FORWARD
	if (FF_LR_PENDING && (gMain.callback2 != CB2_Overworld || ScriptContext2_IsEnabled()))
		FF_LR_PENDING = 0;
	#endif

	// Never speed up link/wireless: both sides must stay in lockstep.
	if (gReceivedRemoteLinkPlayers || gWirelessCommType)
		return FALSE;

	// Silent L + R toggle for every screen that isn't the free-roaming overworld (battles, menus,
	// intro scenes...). The overworld has its own version with a message (TryToggleFastForward in
	// read_keys.c), which consumes the key press so it isn't handled twice.
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

	// Active only while toggled on with L + R (see TryToggleFastForward in read_keys.c).
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
