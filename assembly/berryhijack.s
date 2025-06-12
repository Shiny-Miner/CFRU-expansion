.thumb
.thumb_func
.global hijackberry

hijackberry:
	ldr r3, var_8000
	ldrh r3, [r3]
	cmp r3, #0x7E
	beq hack
	ldr r3, unk_0203F370
	mov r8, r3
	ldrb r0, [r3,#0x4]
	b return
hack:
	ldr r3, unk_0203F370
	mov r8, r3
	mov r0, #0x5
return:
	ldr r3, returnaddress
	bx r3
.align 2
returnaddress: 		.word 0x0813DB1D
unk_0203F370:		.word 0x0203F370
var_8000:		.word 0x020370B8
