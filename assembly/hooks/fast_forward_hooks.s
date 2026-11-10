.text
.thumb
.align 2

.include "../asm_defines.s"

.global WaitForVBlankHook

@ Hooked at the entry of WaitForVBlank (0x8000890, 8 bytes overwritten).
@ WaitForVBlank is also called from other places, so only the call from AgbMain
@ (return address 0x80004A6 + 4, thumb bit set = 0x80004AB) is allowed to skip.
.pool
WaitForVBlankHook:
	mov r1, lr
	ldr r2, =0x80004AB
	cmp r1, r2
	bne WaitForVBlankOriginal

	push {lr}
	bl ShouldSkipVBlankWait
	pop {r1}
	mov lr, r1
	cmp r0, #0x0
	beq WaitForVBlankOriginal
	bx lr						@ skip the wait, back into AgbMain's loop

@ Re-create the 4 instructions the hook overwrote, then continue at 0x8000898
WaitForVBlankOriginal:
	ldr r2, =0x300310C			@ &gMain.intrCheck
	ldrh r3, [r2]
	mov r0, #0x1
	bic r3, r0
	ldr r0, =0x8000898 | 1
	bx r0
.pool
