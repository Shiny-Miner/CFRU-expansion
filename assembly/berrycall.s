.thumb
.thumb_func
.align 2
.global BerryCall

BerryCall:
    push  {lr}
    ldr   r0, =0x03005090 @gTasks
    mov   r1, #1
    strh  r1, [r0, #14] @.data[3]
    mov   r0, #0 @taskId
    ldr   r1, =0x080A1821 @FieldUseFunc_BerryPuch
    bl    linker
    pop   {pc}

linker: bx r1
