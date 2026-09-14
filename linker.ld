OUTPUT_ARCH(arm)
MEMORY {

		rom     : ORIGIN = (0x08000000 + 0x1000000), LENGTH = 32M
        mixer_iwram : ORIGIN = 0x030028E0, LENGTH = 0x800
        ewram   : ORIGIN = 0x02000000, LENGTH = 4M - 4k
}

SECTIONS {
        .text : {

                FILL (0xABCD)

                __text_start = . ;
                *(.init)
                *(.text)
                *(.ctors)
                *(.dtors)
                *(.rodata)
                *(.rodata.*)
                *(.data)
                *(.data.*)
                *(.fini)
                *(COMMON)
                __text_end  = . ;

                __bss_start__ = . ;
                *(.bss)
                __bss_end__ = . ;
        _end = __bss_end__ ;
        __end__ = __bss_end__ ;
        } >rom = 0xff
        .hq_mixer_hot : {
                KEEP(*(.hq_mixer_hot))
        } >mixer_iwram AT>rom
        HQMixerHotImage = LOADADDR(.hq_mixer_hot);
        ASSERT(SIZEOF(.hq_mixer_hot) == 0x800, "HQ mixer hot area must be 2 KiB")
}
