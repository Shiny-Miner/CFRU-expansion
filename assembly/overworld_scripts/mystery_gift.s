.align 2
.thumb

.include "../xse_commands.s"
.include "../xse_defines.s"
.include "../asm_defines.s"

@ Mystery Gift - scripts to hook up to an NPC / door / item (or use with "call").
@ Everything (keyboard, animation, text, giving the Pokemon/item) happens inside the screen.
@ LASTRESULT after the waitstate: 1 = received a gift, 0 = left without receiving anything.

.global MysteryGift_Script
.global MysteryGift_Script_Special

@ callasm version: always works, does not depend on anything in the specials table.
MysteryGift_Script:
	lockall
	callasm MysteryGift_Start
	waitstate
	releaseall
	end

@ "special 0x122" version: needs tools/install_special.py to have been run (adds 1 line to routinepointers).
@ (Without it the special does nothing and the waitstate hangs - use the version above if in doubt.)
MysteryGift_Script_Special:
	lockall
	special 0x122
	waitstate
	releaseall
	end
