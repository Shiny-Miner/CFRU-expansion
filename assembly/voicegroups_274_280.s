@ Voicegroups 274-280 from pokefirered-expansion.
@ Only their transitive instrument/sample dependencies are included.
@ Samples are defined in instruments_274_280.s.
@ MIDI flags: -V 127 -G voicegroup274 (or -G 274).
	.section .rodata
	.align 2
	.macro voice_directsound base_midi_key:req, pan:req, sample_data_pointer:req, attack:req, decay:req, sustain:req, release:req
	.byte 0
	_voice_directsound \base_midi_key, \pan, \sample_data_pointer, \attack, \decay, \sustain, \release
	.endm

	.macro voice_directsound_no_resample base_midi_key:req, pan:req, sample_data_pointer:req, attack:req, decay:req, sustain:req, release:req
	.byte 8
	_voice_directsound \base_midi_key, \pan, \sample_data_pointer, \attack, \decay, \sustain, \release
	.endm

	.macro voice_directsound_alt base_midi_key:req, pan:req, sample_data_pointer:req, attack:req, decay:req, sustain:req, release:req
	.byte 16
	_voice_directsound \base_midi_key, \pan, \sample_data_pointer, \attack, \decay, \sustain, \release
	.endm

	.macro _voice_directsound base_midi_key:req, pan:req, sample_data_pointer:req, attack:req, decay:req, sustain:req, release:req
	.byte \base_midi_key
	.byte 0
	.if \pan != 0
	.byte (0x80 | \pan)
	.else
	.byte 0
	.endif
	.4byte \sample_data_pointer
	.byte \attack
	.byte \decay
	.byte \sustain
	.byte \release
	.endm

	.macro voice_square_1 base_midi_key:req, pan:req, sweep:req, duty_cycle:req, attack:req, decay:req, sustain:req, release:req
	_voice_square_1 1, \base_midi_key, \pan, \sweep, \duty_cycle, \attack, \decay, \sustain, \release
	.endm

	.macro voice_square_1_alt base_midi_key:req, pan:req, sweep:req, duty_cycle:req, attack:req, decay:req, sustain:req, release:req
	_voice_square_1 9, \base_midi_key, \pan, \sweep, \duty_cycle, \attack, \decay, \sustain, \release
	.endm

	.macro _voice_square_1 type:req, base_midi_key:req, pan:req, sweep:req, duty_cycle:req, attack:req, decay:req, sustain:req, release:req
	.byte \type
	.byte \base_midi_key
	.if \pan != 0
	.byte (0x80 | \pan)
	.else
	.byte 0
	.endif
	.byte \sweep
	.byte (\duty_cycle & 0x3)
	.byte 0, 0, 0
	.byte (\attack  & 0x7)
	.byte (\decay   & 0x7)
	.byte (\sustain & 0xF)
	.byte (\release & 0x7)
	.endm

	.macro voice_square_2 base_midi_key:req, pan:req, duty_cycle:req, attack:req, decay:req, sustain:req, release:req
	_voice_square_2 2, \base_midi_key, \pan, \duty_cycle, \attack, \decay, \sustain, \release
	.endm

	.macro voice_square_2_alt base_midi_key:req, pan:req, duty_cycle:req, attack:req, decay:req, sustain:req, release:req
	_voice_square_2 10, \base_midi_key, \pan, \duty_cycle, \attack, \decay, \sustain, \release
	.endm

	.macro _voice_square_2 type:req, base_midi_key:req, pan:req, duty_cycle:req, attack:req, decay:req, sustain:req, release:req
	.byte \type
	.byte \base_midi_key
	.if \pan != 0
	.byte (0x80 | \pan)
	.else
	.byte 0
	.endif
	.byte 0
	.byte (\duty_cycle & 0x3)
	.byte 0, 0, 0
	.byte (\attack  & 0x7)
	.byte (\decay   & 0x7)
	.byte (\sustain & 0xF)
	.byte (\release & 0x7)
	.endm

	.macro voice_programmable_wave base_midi_key:req, pan:req, wave_samples_pointer:req, attack:req, decay:req, sustain:req, release:req
	_voice_programmable_wave 3, \base_midi_key, \pan, \wave_samples_pointer, \attack, \decay, \sustain, \release
	.endm

	.macro voice_programmable_wave_alt base_midi_key:req, pan:req, wave_samples_pointer:req, attack:req, decay:req, sustain:req, release:req
	_voice_programmable_wave 11, \base_midi_key, \pan, \wave_samples_pointer, \attack, \decay, \sustain, \release
	.endm

	.macro _voice_programmable_wave type:req, base_midi_key:req, pan:req, wave_samples_pointer:req, attack:req, decay:req, sustain:req, release:req
	.byte \type
	.byte \base_midi_key
	.if \pan != 0
	.byte (0x80 | \pan)
	.else
	.byte 0
	.endif
	.byte 0
	.4byte \wave_samples_pointer
	.byte (\attack  & 0x7)
	.byte (\decay   & 0x7)
	.byte (\sustain & 0xF)
	.byte (\release & 0x7)
	.endm

	.macro voice_noise base_midi_key:req, pan:req, period:req, attack:req, decay:req, sustain:req, release:req
	_voice_noise 4, \base_midi_key, \pan, \period, \attack, \decay, \sustain, \release
	.endm

	.macro voice_noise_alt base_midi_key:req, pan:req, period:req, attack:req, decay:req, sustain:req, release:req
	_voice_noise 12, \base_midi_key, \pan, \period, \attack, \decay, \sustain, \release
	.endm

	.macro _voice_noise type:req, base_midi_key:req, pan:req, period:req, attack:req, decay:req, sustain:req, release:req
	.byte \type
	.byte \base_midi_key
	.if \pan != 0
	.byte (0x80 | \pan)
	.else
	.byte 0
	.endif
	.byte 0
	.byte (\period & 0x1)
	.byte 0, 0, 0
	.byte (\attack  & 0x7)
	.byte (\decay   & 0x7)
	.byte (\sustain & 0xF)
	.byte (\release & 0x7)
	.endm

	.macro voice_keysplit voice_group_pointer:req, keysplit_table_pointer:req
	.byte 0x40, 0, 0, 0
	.4byte \voice_group_pointer
	.4byte \keysplit_table_pointer
	.endm

	.macro voice_keysplit_all voice_group_pointer:req
	.byte 0x80, 0, 0, 0
	.4byte \voice_group_pointer
	.4byte 0
	.endm

	.align 2
.set KeySplitTable2, . - 36
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable25, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1

	.align 2
.set KeySplitTable27, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1

	.align 2
.set KeySplitTable3, . - 36
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable32, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3

	.align 2
.set KeySplitTable35, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4

	.align 2
.set KeySplitTable36, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1

	.align 2
.set KeySplitTable37, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3

	.align 2
.set KeySplitTable38, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable39, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable4, . - 24
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1

	.align 2
.set KeySplitTable40, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable41, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3

	.align 2
.set KeySplitTable42, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1

	.align 2
.set KeySplitTable43, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1

	.align 2
.set KeySplitTable44, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1

	.align 2
.set KeySplitTable45, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3

	.align 2
.set KeySplitTable46, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable47, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable48, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4

	.align 2
.set KeySplitTable49, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable5, . - 36
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1

	.align 2
.set KeySplitTable50, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable51, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable52, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable53, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3

	.align 2
.set KeySplitTable54, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable55, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3

	.align 2
.set KeySplitTable56, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable57, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2

	.align 2
.set KeySplitTable58, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 4
	.byte 5
	.byte 5
	.byte 5
	.byte 5
	.byte 5
	.byte 5
	.byte 5
	.byte 5
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6
	.byte 6

	.align 2
.set KeySplitTable9, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1

	.align 2
.set KeySplitTableGBAPiano1Custom, . - 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 0
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 1
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 2
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3
	.byte 3

	.align 2
voicegroup001:
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 1, 6, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_programmable_wave 60, 0, ProgrammableWaveData_1, 0, 7, 15, 1
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_2 60, 0, 2, 0, 1, 6, 0
	voice_programmable_wave 60, 0, ProgrammableWaveData_3, 0, 7, 15, 1
	voice_square_1 60, 0, 0, 2, 0, 1, 6, 0
	voice_square_2 60, 0, 3, 0, 1, 6, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 0, 0, 1, 6, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 255, 0

	.align 2
voicegroup005:
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_trumpet_60, 255, 0, 193, 127
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_trumpet_72, 255, 0, 193, 127
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_trumpet_84, 255, 0, 193, 127
	voice_square_1_alt 60, 0, 38, 2, 1, 0, 0, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1_alt 60, 0, 36, 2, 0, 1, 4, 2
	voice_square_1_alt 60, 0, 21, 2, 0, 0, 15, 2

	.align 2
voicegroup006:
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_tuba_39, 255, 0, 255, 165
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_tuba_51, 255, 0, 255, 165

	.align 2
voicegroup007:
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_french_horn_60, 255, 0, 224, 165
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_french_horn_72, 255, 0, 218, 165

	.align 2
voicegroup009:
	voice_keysplit_all voicegroup001
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_timpani, 255, 246, 0, 226
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup007, KeySplitTable5
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_2_alt 60, 0, 2, 0, 1, 9, 0
	voice_programmable_wave_alt 60, 0, ProgrammableWaveData_7, 0, 7, 15, 0
	voice_square_1_alt 60, 0, 0, 2, 0, 1, 9, 0
	voice_square_2_alt 60, 0, 3, 0, 1, 7, 0
	voice_square_1_alt 60, 0, 0, 3, 0, 1, 7, 0

	.align 2
voicegroup198:
	voice_directsound 60, 0, DirectSoundWaveData_dp_el_pia2_c3_22, 255, 249, 0, 208
	voice_directsound 60, 0, DirectSoundWaveData_dp_el_pia2_c4_22, 255, 249, 0, 208

	.align 2
voicegroup216:
	voice_directsound 60, 0, DirectSoundWaveData_dp_timpany_e_16, 255, 248, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_dp_timpany_as_16, 255, 248, 0, 205

	.align 2
voicegroup233:
	voice_directsound 60, 0, DirectSoundWaveData_hg_051pnoc2, 255, 249, 0, 224
	voice_directsound 60, 0, DirectSoundWaveData_hg_052pnoc3, 255, 249, 0, 224
	voice_directsound 60, 0, DirectSoundWaveData_hg_053pnoc4, 255, 249, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_054pnoc5, 255, 248, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_055pnoc6, 255, 247, 0, 205

	.align 2
voicegroup234:
	voice_directsound 60, 0, DirectSoundWaveData_hg_005pnoffc4, 255, 249, 0, 171
	voice_directsound 60, 0, DirectSoundWaveData_hg_006pnoffc5, 255, 249, 0, 171

	.align 2
voicegroup235:
	voice_directsound 60, 0, DirectSoundWaveData_hg_082octpnoc2, 255, 249, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_083octpnoc3, 255, 249, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_084octpnoc4, 255, 249, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_085octpnoc5, 255, 249, 0, 205

	.align 2
voicegroup236:
	voice_directsound 60, 0, DirectSoundWaveData_hg_043rhodc4, 255, 250, 0, 210
	voice_directsound 60, 0, DirectSoundWaveData_hg_044rhodc5, 255, 250, 0, 210

	.align 2
voicegroup237:
	voice_directsound 60, 0, DirectSoundWaveData_hg_043rhodc4, 255, 250, 0, 32
	voice_directsound 60, 0, DirectSoundWaveData_hg_044rhodc5, 255, 250, 0, 32

	.align 2
voicegroup238:
	voice_directsound 60, 0, DirectSoundWaveData_hg_148vibrc4, 255, 246, 0, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_149vibrc5, 255, 246, 0, 205

	.align 2
voicegroup239:
	voice_directsound 60, 0, DirectSoundWaveData_hg_068marimc4, 255, 246, 0, 214
	voice_directsound 60, 0, DirectSoundWaveData_hg_069marimc5, 255, 246, 0, 214
	voice_directsound 60, 0, DirectSoundWaveData_hg_070marimc6, 255, 246, 0, 214

	.align 2
voicegroup240:
	voice_directsound 60, 0, DirectSoundWaveData_hg_156xylo1c4, 255, 248, 25, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_155xylo1a4, 255, 248, 25, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_157xylo1f5, 255, 248, 57, 192

	.align 2
voicegroup241:
	voice_directsound 60, 0, DirectSoundWaveData_hg_147tbellg4, 255, 248, 25, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_146tbellc5, 255, 248, 25, 205

	.align 2
voicegroup242:
	voice_directsound 60, 0, DirectSoundWaveData_hg_049glocknc4, 255, 248, 2, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_050glocknc5, 255, 248, 2, 205

	.align 2
voicegroup243:
	voice_directsound 60, 0, DirectSoundWaveData_hg_071musboxc4, 255, 246, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_072musboxc5, 255, 246, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_073musboxc6, 255, 246, 0, 205

	.align 2
voicegroup244:
	voice_directsound 60, 0, DirectSoundWaveData_hg_038drawbc3, 255, 197, 227, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_039drawbc4, 255, 0, 255, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_040drawbc5, 255, 0, 255, 205

	.align 2
voicegroup245:
	voice_directsound 60, 0, DirectSoundWaveData_hg_091jazorgc3, 255, 0, 255, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_092jazorgc4, 255, 0, 255, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_093jazorgc5, 255, 0, 255, 205

	.align 2
voicegroup246:
	voice_directsound 60, 0, DirectSoundWaveData_hg_002accorc3, 255, 0, 217, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_003accorc4, 255, 0, 217, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_004accorc5, 254, 197, 217, 192

	.align 2
voicegroup247:
	voice_directsound 60, 0, DirectSoundWaveData_hg_077nylong2, 255, 251, 6, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_078nylong3, 255, 251, 6, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_076nylone4, 255, 251, 6, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_075nylonc5, 255, 251, 6, 205

	.align 2
voicegroup248:
	voice_directsound 60, 0, DirectSoundWaveData_hg_117marting2, 255, 251, 14, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_118marting3, 255, 251, 14, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_116martine4, 255, 251, 14, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_115martinc5, 255, 251, 14, 192

	.align 2
voicegroup249:
	voice_directsound 60, 0, DirectSoundWaveData_dp_disgt_e1_16, 255, 253, 0, 32
	voice_directsound 60, 0, DirectSoundWaveData_dp_disgt_c2_16, 255, 253, 0, 32
	voice_directsound 60, 0, DirectSoundWaveData_dp_disgt_e3_16, 255, 253, 0, 32
	voice_directsound 60, 0, DirectSoundWaveData_dp_disgt_e4_16, 255, 253, 0, 32

	.align 2
voicegroup250:
	voice_directsound 60, 0, DirectSoundWaveData_dp_disgt_e1_16, 255, 253, 0, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_disgt_c2_16, 255, 253, 0, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_disgt_e3_16, 255, 253, 0, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_disgt_e4_16, 255, 253, 0, 171

	.align 2
voicegroup251:
	voice_directsound 60, 0, DirectSoundWaveData_hg_029cleang2, 255, 246, 0, 171
	voice_directsound 60, 0, DirectSoundWaveData_hg_030cleang3, 255, 246, 0, 171
	voice_directsound 60, 0, DirectSoundWaveData_hg_028cleane4, 255, 246, 0, 171
	voice_directsound 60, 0, DirectSoundWaveData_hg_027cleanc5, 255, 246, 0, 171

	.align 2
voicegroup252:
	voice_directsound 60, 0, DirectSoundWaveData_hg_000uprigc2, 255, 251, 0, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_001uprigc3, 255, 251, 0, 192

	.align 2
voicegroup253:
	voice_directsound 60, 0, DirectSoundWaveData_hg_041ampega2, 255, 234, 159, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_042ampega3, 255, 234, 159, 192

	.align 2
voicegroup254:
	voice_directsound 60, 0, DirectSoundWaveData_hg_123elastkc2, 255, 42, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_hg_124elastkc3, 255, 42, 227, 171

	.align 2
voicegroup255:
	voice_directsound 60, 0, DirectSoundWaveData_hg_120str1c4, 255, 242, 191, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_119str1a4, 255, 242, 191, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_122str1f5, 254, 242, 191, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_121str1c6, 254, 242, 191, 192

	.align 2
voicegroup256:
	voice_directsound 60, 0, DirectSoundWaveData_hg_152violne4, 254, 242, 255, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_151violnc5, 254, 246, 255, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_150violna5, 254, 246, 255, 205

	.align 2
voicegroup257:
	voice_directsound 60, 0, DirectSoundWaveData_hg_021cellog2, 254, 242, 255, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_020celloe3, 254, 246, 255, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_019celloc4, 254, 246, 255, 205

	.align 2
voicegroup258:
	voice_directsound 60, 0, DirectSoundWaveData_hg_102pizzg2, 255, 242, 25, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_100pizze3, 255, 242, 25, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_099pizzc4, 255, 242, 25, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_098pizza4, 255, 242, 25, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_101pizzf5, 255, 239, 25, 205

	.align 2
voicegroup259:
	voice_directsound 60, 0, DirectSoundWaveData_hg_058harpc3, 255, 248, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_059harpc4, 255, 246, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_060harpc5, 255, 242, 0, 205

	.align 2
voicegroup260:
	voice_directsound 60, 0, DirectSoundWaveData_hg_133timpg3, 255, 251, 25, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_132timpc4, 255, 251, 25, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_134timpg4, 255, 251, 25, 192

	.align 2
voicegroup261:
	voice_directsound 60, 0, DirectSoundWaveData_hg_144tptc4, 255, 242, 191, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_143tpta4, 255, 242, 191, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_145tptg5, 255, 234, 191, 205

	.align 2
voicegroup262:
	voice_directsound 60, 0, DirectSoundWaveData_hg_142tbng2, 255, 242, 159, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_141tbne3, 255, 242, 159, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_140tbnc4, 255, 242, 159, 205

	.align 2
voicegroup263:
	voice_directsound 60, 0, DirectSoundWaveData_hg_016fhsolof3, 255, 0, 252, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_015fhsolod4, 255, 0, 252, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_014fhsolob4, 255, 0, 252, 192

	.align 2
voicegroup264:
	voice_directsound 60, 0, DirectSoundWaveData_hg_048fhnsf3, 255, 216, 219, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_047fhnsd4, 255, 216, 219, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_046fhnsb4, 255, 216, 255, 205

	.align 2
voicegroup265:
	voice_directsound 60, 0, DirectSoundWaveData_hg_081oboee4, 255, 242, 191, 214
	voice_directsound 60, 0, DirectSoundWaveData_hg_080oboec5, 255, 242, 191, 214
	voice_directsound 60, 0, DirectSoundWaveData_hg_079oboea5, 255, 242, 191, 214

	.align 2
voicegroup266:
	voice_directsound 60, 0, DirectSoundWaveData_hg_025clarig3, 255, 242, 127, 214
	voice_directsound 60, 0, DirectSoundWaveData_hg_024clarie4, 255, 242, 127, 214
	voice_directsound 60, 0, DirectSoundWaveData_hg_023claric5, 255, 242, 127, 214

	.align 2
voicegroup267:
	voice_directsound 60, 0, DirectSoundWaveData_hg_095flt1c4, 255, 246, 159, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_094flt1a4, 255, 246, 127, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_097flt1f5, 255, 246, 127, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_096flt1c6, 255, 246, 78, 192

	.align 2
voicegroup268:
	voice_directsound 60, 0, DirectSoundWaveData_hg_012bsne2, 255, 242, 127, 214
	voice_directsound 60, 0, DirectSoundWaveData_hg_011bsnc3, 255, 242, 127, 214
	voice_directsound 60, 0, DirectSoundWaveData_hg_010bsna3, 255, 242, 127, 214

	.align 2
voicegroup269:
	voice_directsound 60, 0, DirectSoundWaveData_hg_086orchitc4, 255, 248, 25, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_087orchitc5, 255, 248, 25, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_089orchitg5, 255, 248, 25, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_088orchitc6, 255, 248, 25, 205

	.align 2
voicegroup270:
	voice_directsound 60, 0, DirectSoundWaveData_hg_125squarc4, 255, 242, 191, 171
	voice_directsound 60, 0, DirectSoundWaveData_hg_126squarc5, 255, 242, 191, 171
	voice_directsound 60, 0, DirectSoundWaveData_hg_127squarc6, 255, 234, 191, 171

	.align 2
voicegroup271:
	voice_directsound 60, 0, DirectSoundWaveData_hg_067kotoe3, 255, 246, 0, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_066kotoc4, 255, 246, 0, 192
	voice_directsound 60, 0, DirectSoundWaveData_hg_065kotoa4, 255, 246, 0, 192

	.align 2
voicegroup272:
	voice_directsound 60, 0, DirectSoundWaveData_hg_shamisend3, 255, 250, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_shamisenas3, 255, 248, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_shamiseng4, 255, 247, 0, 205

	.align 2
voicegroup273:
	voice_directsound 60, 0, DirectSoundWaveData_hg_pno4c2, 255, 249, 0, 224
	voice_directsound 60, 0, DirectSoundWaveData_hg_pno4a2, 255, 249, 0, 224
	voice_directsound 60, 0, DirectSoundWaveData_hg_pno4f3, 255, 249, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_pno4d4, 255, 248, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_pno4b4, 255, 247, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_pno4g5, 255, 247, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_pno4e6, 255, 247, 0, 205

	.align 2
	.global voicegroup274
voicegroup274:
	voice_keysplit voicegroup005, KeySplitTableGBAPiano1Custom
	voice_keysplit voicegroup233, KeySplitTable35
	voice_keysplit voicegroup234, KeySplitTable36
	voice_keysplit voicegroup236, KeySplitTable9
	voice_keysplit voicegroup237, KeySplitTable9
	voice_keysplit voicegroup238, KeySplitTable36
	voice_directsound 60, 0, DirectSoundWaveData_unknown_koto_high, 255, 0, 206, 242
	voice_keysplit voicegroup239, KeySplitTable38
	voice_directsound 60, 0, DirectSoundWaveData_unknown_koto_low, 255, 0, 206, 242
	voice_directsound 60, 0, DirectSoundWaveData_hg_jinglebell, 255, 0, 255, 171
	voice_keysplit voicegroup238, KeySplitTable36
	voice_keysplit voicegroup239, KeySplitTable38
	voice_keysplit voicegroup240, KeySplitTable39
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_wind, 67, 243, 155, 224
	voice_directsound 60, 0, DirectSoundWaveData_hg_jinglebell, 255, 0, 255, 171
	voice_keysplit voicegroup242, KeySplitTable36
	voice_keysplit voicegroup243, KeySplitTable38
	voice_keysplit voicegroup250, KeySplitTable32
	voice_keysplit voicegroup244, KeySplitTable40
	voice_keysplit voicegroup246, KeySplitTable40
	voice_keysplit voicegroup247, KeySplitTable41
	voice_keysplit voicegroup252, KeySplitTable42
	voice_keysplit voicegroup247, KeySplitTable41
	voice_keysplit voicegroup253, KeySplitTable43
	voice_keysplit voicegroup006, KeySplitTable2
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fretless_bass, 255, 253, 0, 149
	voice_keysplit voicegroup258, KeySplitTable48
	voice_keysplit voicegroup253, KeySplitTable43
	voice_keysplit voicegroup251, KeySplitTable41
	voice_keysplit voicegroup254, KeySplitTable44
	voice_keysplit voicegroup255, KeySplitTable45
	voice_directsound 60, 0, DirectSoundWaveData_unused_sc88pro_unison_slap, 255, 165, 180, 216
	voice_keysplit voicegroup256, KeySplitTable46
	voice_keysplit voicegroup257, KeySplitTable47
	voice_keysplit voicegroup266, KeySplitTable52
	voice_keysplit voicegroup259, KeySplitTable40
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_harp, 255, 242, 51, 242
	voice_keysplit voicegroup269, KeySplitTable55
	voice_keysplit voicegroup260, KeySplitTable40
	voice_keysplit voicegroup261, KeySplitTable39
	voice_keysplit voicegroup262, KeySplitTable47
	voice_keysplit voicegroup248, KeySplitTable41
	voice_keysplit voicegroup263, KeySplitTable49
	voice_keysplit voicegroup264, KeySplitTable50
	voice_keysplit voicegroup265, KeySplitTable51
	voice_keysplit voicegroup266, KeySplitTable52
	voice_keysplit voicegroup198, KeySplitTable4
	voice_keysplit voicegroup267, KeySplitTable53
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_flute, 255, 127, 231, 127
	voice_square_2_alt 60, 0, 2, 0, 6, 2, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_altosax_c3_16, 255, 216, 252, 192
	voice_keysplit voicegroup262, KeySplitTable47
	voice_keysplit voicegroup273, KeySplitTable58
	voice_keysplit voicegroup009, KeySplitTable5
	voice_keysplit voicegroup265, KeySplitTable51
	voice_keysplit voicegroup264, KeySplitTable50
	voice_keysplit voicegroup007, KeySplitTable3
	voice_keysplit voicegroup245, KeySplitTable40
	voice_directsound 60, 0, DirectSoundWaveData_dp_slapbass_c1_16, 255, 0, 255, 192
	voice_keysplit voicegroup247, KeySplitTable41
	voice_keysplit voicegroup007, KeySplitTable3
	voice_keysplit voicegroup235, KeySplitTable37
	voice_directsound 60, 0, DirectSoundWaveData_dp_reverscyn_16, 255, 0, 255, 16
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 188, 220, 243
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 188, 220, 243
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 188, 220, 243
	voice_directsound 60, 0, DirectSoundWaveData_dp_orchhitminor60, 255, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_detuned_ep1_low, 128, 249, 0, 188
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup241, KeySplitTable25
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_high, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_dp_bassdr1, 255, 0, 227, 171
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_sawtoothlead60, 255, 197, 255, 171
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup264, KeySplitTable50
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 171
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_noise_alt 60, 0, 0, 0, 1, 6, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1_alt 60, 0, 0, 2, 0, 6, 2, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_orchhitmajor60, 255, 197, 255, 171
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit_all voicegroup281

	.align 2
	.global voicegroup275
voicegroup275:
	voice_keysplit voicegroup005, KeySplitTableGBAPiano1Custom
	voice_keysplit voicegroup233, KeySplitTable35
	voice_keysplit voicegroup235, KeySplitTable37
	voice_keysplit voicegroup236, KeySplitTable9
	voice_keysplit voicegroup237, KeySplitTable9
	voice_keysplit voicegroup234, KeySplitTable36
	voice_keysplit voicegroup273, KeySplitTable58
	voice_directsound 60, 0, DirectSoundWaveData_unknown_koto_high, 255, 0, 206, 242
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup238, KeySplitTable36
	voice_keysplit voicegroup239, KeySplitTable38
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup241, KeySplitTable25
	voice_keysplit voicegroup242, KeySplitTable36
	voice_keysplit voicegroup243, KeySplitTable38
	voice_keysplit voicegroup244, KeySplitTable40
	voice_keysplit voicegroup245, KeySplitTable40
	voice_keysplit voicegroup246, KeySplitTable40
	voice_keysplit voicegroup247, KeySplitTable41
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup248, KeySplitTable41
	voice_keysplit voicegroup251, KeySplitTable41
	voice_keysplit voicegroup252, KeySplitTable42
	voice_keysplit voicegroup253, KeySplitTable43
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fretless_bass, 255, 253, 0, 149
	voice_directsound 60, 0, DirectSoundWaveData_dp_slapbass_c1_16, 255, 0, 255, 192
	voice_keysplit voicegroup254, KeySplitTable44
	voice_keysplit voicegroup255, KeySplitTable45
	voice_keysplit voicegroup006, KeySplitTable2
	voice_keysplit voicegroup256, KeySplitTable46
	voice_directsound 60, 0, DirectSoundWaveData_hg_021cellog2, 254, 242, 255, 205
	voice_keysplit voicegroup258, KeySplitTable48
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_harp, 255, 242, 51, 242
	voice_keysplit voicegroup259, KeySplitTable40
	voice_keysplit voicegroup259, KeySplitTable40
	voice_keysplit voicegroup260, KeySplitTable40
	voice_keysplit voicegroup261, KeySplitTable39
	voice_keysplit voicegroup262, KeySplitTable47
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup263, KeySplitTable49
	voice_keysplit voicegroup264, KeySplitTable50
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_oboe, 43, 188, 103, 165
	voice_keysplit voicegroup266, KeySplitTable52
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_flute, 255, 127, 231, 127
	voice_keysplit voicegroup267, KeySplitTable53
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_whistle, 43, 76, 103, 216
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_noise_alt 60, 0, 0, 0, 1, 6, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_bubbles, 255, 0, 255, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_tenorchoir48, 255, 188, 220, 243
	voice_directsound 60, 0, DirectSoundWaveData_dp_altosax_c3_16, 255, 216, 252, 192
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_unknown_koto_high, 255, 0, 206, 242
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_2_alt 60, 0, 2, 0, 7, 2, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1_alt 60, 0, 0, 2, 0, 7, 2, 0
	voice_programmable_wave_alt 60, 0, ProgrammableWaveData_3, 0, 7, 2, 0
	voice_directsound 60, 0, DirectSoundWaveData_puresquare_50, 255, 242, 25, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit_all voicegroup282

	.align 2
	.global voicegroup276
voicegroup276:
	voice_keysplit voicegroup234, KeySplitTable36
	voice_keysplit voicegroup005, KeySplitTableGBAPiano1Custom
	voice_directsound 60, 0, DirectSoundWaveData_dp_reverscyn_16, 255, 0, 227, 171
	voice_keysplit voicegroup236, KeySplitTable9
	voice_directsound 60, 0, DirectSoundWaveData_unknown_koto_high, 255, 0, 206, 242
	voice_keysplit voicegroup233, KeySplitTable35
	voice_keysplit voicegroup235, KeySplitTable37
	voice_keysplit voicegroup273, KeySplitTable58
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_detuned_ep1_low, 128, 249, 0, 188
	voice_keysplit voicegroup242, KeySplitTable36
	voice_keysplit voicegroup238, KeySplitTable36
	voice_keysplit voicegroup239, KeySplitTable38
	voice_directsound 60, 64, DirectSoundWaveData_dp_808snare_16, 255, 220, 0, 171
	voice_keysplit voicegroup246, KeySplitTable40
	voice_keysplit voicegroup241, KeySplitTable25
	voice_keysplit voicegroup248, KeySplitTable41
	voice_keysplit voicegroup247, KeySplitTable41
	voice_keysplit voicegroup244, KeySplitTable40
	voice_keysplit voicegroup245, KeySplitTable40
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_keysplit voicegroup253, KeySplitTable43
	voice_directsound 60, 0, DirectSoundWaveData_dp_slapbass_c1_16, 255, 0, 255, 171
	voice_keysplit voicegroup254, KeySplitTable44
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_synth_bass, 255, 252, 0, 146
	voice_keysplit voicegroup250, KeySplitTable32
	voice_keysplit voicegroup252, KeySplitTable42
	voice_keysplit voicegroup248, KeySplitTable41
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_slap_bass, 255, 235, 128, 115
	voice_directsound 60, 0, DirectSoundWaveData_dp_slapbass_c1_16, 255, 0, 255, 192
	voice_keysplit voicegroup249, KeySplitTable32
	voice_keysplit voicegroup255, KeySplitTable45
	voice_keysplit voicegroup006, KeySplitTable2
	voice_keysplit voicegroup254, KeySplitTable44
	voice_directsound 60, 0, DirectSoundWaveData_hg_021cellog2, 254, 242, 255, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_098pizza4, 255, 248, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_flute, 255, 127, 231, 127
	voice_keysplit voicegroup258, KeySplitTable48
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_slap_bass, 255, 235, 128, 115
	voice_directsound 60, 0, DirectSoundWaveData_hg_133timpg3, 255, 251, 0, 205
	voice_keysplit voicegroup243, KeySplitTable38
	voice_keysplit voicegroup262, KeySplitTable47
	voice_keysplit voicegroup265, KeySplitTable51
	voice_keysplit voicegroup264, KeySplitTable50
	voice_keysplit voicegroup198, KeySplitTable4
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_oboe, 43, 188, 103, 165
	voice_keysplit voicegroup266, KeySplitTable52
	voice_directsound 60, 0, DirectSoundWaveData_dp_timpany_as_16, 255, 248, 0, 205
	voice_keysplit voicegroup260, KeySplitTable40
	voice_keysplit voicegroup255, KeySplitTable45
	voice_keysplit voicegroup267, KeySplitTable53
	voice_keysplit voicegroup263, KeySplitTable49
	voice_keysplit voicegroup262, KeySplitTable47
	voice_directsound 60, 0, DirectSoundWaveData_unused_sc88pro_unison_slap, 255, 165, 180, 216
	voice_keysplit voicegroup254, KeySplitTable44
	voice_keysplit voicegroup260, KeySplitTable40
	voice_keysplit voicegroup269, KeySplitTable55
	voice_keysplit voicegroup261, KeySplitTable39
	voice_keysplit voicegroup009, KeySplitTable5
	voice_directsound 60, 0, DirectSoundWaveData_dp_bass2, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 255, 171
	voice_keysplit voicegroup261, KeySplitTable39
	voice_directsound 60, 0, DirectSoundWaveData_dp_sawtoothlead60, 255, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_reverscyn_16, 255, 0, 255, 16
	voice_directsound 60, 0, DirectSoundWaveData_dp_spearpillarwind60, 255, 0, 255, 32
	voice_directsound 60, 0, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_keysplit voicegroup250, KeySplitTable32
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_distortion_guitar_high, 255, 165, 154, 165
	voice_directsound 60, 0, DirectSoundWaveData_hg_033orccym, 255, 246, 0, 214
	voice_directsound 60, 0, DirectSoundWaveData_dp_woodbass_d3_16, 255, 251, 0, 171
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_whistle, 43, 76, 103, 216
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_whistle, 43, 76, 103, 216
	voice_keysplit voicegroup198, KeySplitTable4
	voice_keysplit voicegroup269, KeySplitTable55
	voice_keysplit voicegroup264, KeySplitTable50
	voice_directsound 60, 0, DirectSoundWaveData_unused_guitar_separates_power_chord, 255, 0, 255, 127
	voice_keysplit voicegroup269, KeySplitTable55
	voice_keysplit voicegroup268, KeySplitTable54
	voice_directsound 60, 0, DirectSoundWaveData_dp_synthkick, 255, 251, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 188, 220, 243
	voice_directsound 60, 0, DirectSoundWaveData_hg_059harpc4, 255, 246, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_hg_130jdrumh, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_hg_128jdruml, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_sd90_enhanced_delay_shaku, 255, 191, 97, 165
	voice_directsound 60, 0, DirectSoundWaveData_dp_orchhitminor60, 255, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_orchhitminor60, 58, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_purenoise, 240, 242, 0, 192
	voice_keysplit voicegroup270, KeySplitTable38
	voice_directsound 60, 0, DirectSoundWaveData_hg_039drawbc4, 255, 0, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_sawtoothlead60, 255, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_sawtoothlead60, 255, 197, 255, 171
	voice_noise_alt 60, 0, 0, 0, 1, 6, 0
	voice_directsound 60, 0, DirectSoundWaveData_unused_guitar_separates_power_chord, 255, 0, 255, 127
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_detuned_ep1_low, 128, 249, 0, 188
	voice_directsound 60, 0, DirectSoundWaveData_unused_guitar_separates_power_chord, 255, 0, 255, 127
	voice_keysplit voicegroup254, KeySplitTable44
	voice_keysplit voicegroup256, KeySplitTable46
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_fretless_bass, 255, 253, 0, 149
	voice_keysplit voicegroup268, KeySplitTable54
	voice_directsound 60, 0, DirectSoundWaveData_dp_explosion, 67, 255, 0, 245
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_flute, 255, 127, 231, 127
	voice_directsound 60, 0, DirectSoundWaveData_dp_tenorchoir48, 255, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_hg_hoohclap, 255, 0, 255, 0
	voice_noise_alt 60, 0, 0, 0, 1, 6, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_tenorchoir48, 255, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_tenorchoir48, 255, 197, 255, 171
	voice_keysplit voicegroup272, KeySplitTable57
	voice_keysplit voicegroup272, KeySplitTable57
	voice_keysplit voicegroup236, KeySplitTable9
	voice_keysplit voicegroup250, KeySplitTable32
	voice_keysplit voicegroup251, KeySplitTable41
	voice_keysplit voicegroup259, KeySplitTable40
	voice_keysplit voicegroup261, KeySplitTable39
	voice_directsound 60, 0, DirectSoundWaveData_dp_org5_c3_16, 255, 167, 235, 171
	voice_keysplit voicegroup238, KeySplitTable36
	voice_directsound 60, 0, DirectSoundWaveData_dp_sopranochoir60, 255, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_sopranochoir60, 255, 197, 255, 171
	voice_keysplit voicegroup252, KeySplitTable42
	voice_keysplit voicegroup261, KeySplitTable39
	voice_keysplit voicegroup216, KeySplitTable27
	voice_keysplit voicegroup241, KeySplitTable25
	voice_keysplit voicegroup264, KeySplitTable50
	voice_directsound 60, 40, DirectSoundWaveData_hg_130jdrumh, 255, 0, 227, 171
	voice_keysplit voicegroup238, KeySplitTable36
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_orchhitmajor60, 255, 0, 255, 205
	voice_keysplit_all voicegroup284
	voice_keysplit_all voicegroup283

	.align 2
	.global voicegroup277
voicegroup277:
	voice_keysplit voicegroup233, KeySplitTable35
	voice_keysplit voicegroup005, KeySplitTableGBAPiano1Custom
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup236, KeySplitTable9
	voice_keysplit voicegroup234, KeySplitTable36
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup238, KeySplitTable36
	voice_keysplit voicegroup239, KeySplitTable38
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_hg_072musboxc5, 255, 246, 0, 205
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup245, KeySplitTable40
	voice_keysplit voicegroup246, KeySplitTable40
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup252, KeySplitTable42
	voice_keysplit voicegroup253, KeySplitTable43
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_unused_sc88pro_unison_slap, 255, 165, 180, 216
	voice_keysplit voicegroup254, KeySplitTable44
	voice_keysplit voicegroup255, KeySplitTable45
	voice_keysplit voicegroup006, KeySplitTable2
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup258, KeySplitTable48
	voice_keysplit voicegroup259, KeySplitTable40
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup261, KeySplitTable39
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup264, KeySplitTable50
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup266, KeySplitTable52
	voice_directsound 60, 0, DirectSoundWaveData_dp_timpany_as_16, 255, 248, 0, 205
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_flute, 255, 127, 231, 127
	voice_directsound 60, 0, DirectSoundWaveData_hg_096flt1c6, 255, 246, 159, 205
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup262, KeySplitTable47
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup258, KeySplitTable48
	voice_directsound 60, 0, DirectSoundWaveData_dp_reverscyn_16, 255, 0, 255, 16
	voice_keysplit voicegroup261, KeySplitTable39
	voice_keysplit voicegroup255, KeySplitTable45
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_shakuhachi, 255, 0, 255, 204
	voice_directsound 60, 0, DirectSoundWaveData_unknown_koto_high, 255, 246, 159, 224
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_sawtoothlead60, 255, 197, 255, 171
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_2_alt 60, 0, 2, 0, 7, 15, 0
	voice_square_2_alt 60, 0, 2, 0, 7, 15, 6
	voice_square_1_alt 60, 0, 0, 2, 0, 7, 15, 6
	voice_programmable_wave_alt 60, 0, ProgrammableWaveData_3, 0, 7, 15, 6
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_noise_alt 60, 0, 0, 0, 7, 15, 1
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit_all voicegroup284

	.align 2
	.global voicegroup278
voicegroup278:
	voice_keysplit voicegroup005, KeySplitTableGBAPiano1Custom
	voice_keysplit voicegroup233, KeySplitTable35
	voice_keysplit voicegroup236, KeySplitTable9
	voice_keysplit voicegroup237, KeySplitTable9
	voice_keysplit voicegroup238, KeySplitTable36
	voice_keysplit voicegroup239, KeySplitTable38
	voice_keysplit voicegroup241, KeySplitTable25
	voice_keysplit voicegroup245, KeySplitTable40
	voice_keysplit voicegroup247, KeySplitTable41
	voice_keysplit voicegroup250, KeySplitTable32
	voice_keysplit voicegroup252, KeySplitTable42
	voice_keysplit voicegroup253, KeySplitTable43
	voice_directsound 60, 0, DirectSoundWaveData_dp_slapbass_c1_16, 255, 0, 255, 192
	voice_keysplit voicegroup255, KeySplitTable45
	voice_keysplit voicegroup258, KeySplitTable48
	voice_keysplit voicegroup259, KeySplitTable40
	voice_keysplit voicegroup260, KeySplitTable40
	voice_keysplit voicegroup261, KeySplitTable39
	voice_keysplit voicegroup264, KeySplitTable50
	voice_keysplit voicegroup266, KeySplitTable52
	voice_keysplit voicegroup267, KeySplitTable53
	voice_keysplit voicegroup269, KeySplitTable55
	voice_keysplit_all voicegroup285
	voice_square_2_alt 60, 0, 2, 0, 7, 2, 2
	voice_programmable_wave_alt 60, 0, ProgrammableWaveData_1, 0, 7, 11, 3
	voice_square_2_alt 60, 0, 0, 0, 7, 11, 3
	voice_directsound 60, 0, DirectSoundWaveData_puresquare_37, 254, 234, 191, 210
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup234, KeySplitTable36
	voice_keysplit voicegroup236, KeySplitTable9
	voice_keysplit voicegroup238, KeySplitTable36
	voice_keysplit voicegroup245, KeySplitTable40
	voice_keysplit voicegroup254, KeySplitTable44
	voice_keysplit voicegroup255, KeySplitTable45
	voice_keysplit voicegroup247, KeySplitTable41
	voice_keysplit_all voicegroup286
	voice_keysplit voicegroup253, KeySplitTable43
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_dp_sawtoothlead60, 255, 0, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_detuned_ep1_low, 128, 249, 0, 188
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_sc88pro_church_organ3_low, 255, 76, 154, 188
	voice_directsound 60, 0, DirectSoundWaveData_drum_and_percussion_kick, 255, 0, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_synthkick, 255, 0, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_sawtoothlead60, 255, 0, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 171
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0

	.align 2
	.global voicegroup279
voicegroup279:
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_whistle, 43, 76, 103, 216
	voice_keysplit_all voicegroup285
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_sd90_classical_detuned_ep1_low, 128, 249, 0, 188
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_org4_c4_16, 255, 0, 255, 205
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_sawtoothlead60, 255, 197, 255, 171
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup249, KeySplitTable32
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_synbass1_c2_16, 255, 1, 255, 192
	voice_directsound 60, 0, DirectSoundWaveData_dp_synbass2_c1_16, 255, 0, 255, 220
	voice_keysplit voicegroup256, KeySplitTable46
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup260, KeySplitTable40
	voice_keysplit voicegroup255, KeySplitTable45
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_merotim_c3_16, 224, 242, 101, 220
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup262, KeySplitTable47
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup264, KeySplitTable50
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_tenorchoir48, 255, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_sopranochoir60, 255, 197, 255, 171
	voice_directsound 60, 0, DirectSoundWaveData_dp_whistle_c5_16, 255, 246, 78, 192
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 0, DirectSoundWaveData_dp_orchhitminor60, 255, 0, 255, 205
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0

	.align 2
	.global voicegroup280
voicegroup280:
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_directsound 60, 0, DirectSoundWaveData_classical_choir_voice_ahhs, 255, 0, 227, 171
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup271, KeySplitTable56
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup247, KeySplitTable41
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit voicegroup270, KeySplitTable38
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_keysplit_all voicegroup285
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0

	.align 2
voicegroup281:
	voice_directsound 60, 45, DirectSoundWaveData_hg_033orccym, 255, 248, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_dp_sne2_loop, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_033orccym, 255, 248, 0, 214
	voice_directsound 48, 64, DirectSoundWaveData_dp_reverscyn_16, 2, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_hg_033orccym, 255, 250, 0, 214
	voice_directsound 48, 40, DirectSoundWaveData_dp_reverscyn_16, 2, 0, 227, 171
	voice_directsound 60, 40, DirectSoundWaveData_hg_032crash, 255, 250, 0, 214
	voice_directsound 60, 105, DirectSoundWaveData_dp_mute_conga_16, 255, 0, 255, 171
	voice_directsound 60, 105, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_directsound 60, 80, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 205
	voice_directsound 60, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_093snare, 255, 226, 127, 205
	voice_directsound 60, 30, DirectSoundWaveData_hg_135tom, 255, 0, 255, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 78, 0, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 205
	voice_directsound 61, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 205
	voice_directsound 60, 50, DirectSoundWaveData_hg_138tom, 255, 0, 255, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 20, DirectSoundWaveData_hg_022china, 255, 248, 0, 224
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 30, DirectSoundWaveData_dp_053clap, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_bassdr1, 255, 0, 227, 171
	voice_directsound 60, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 242, 0, 171
	voice_directsound 57, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 214
	voice_directsound 59, 20, DirectSoundWaveData_hg_022china, 255, 250, 0, 224
	voice_directsound 60, 84, DirectSoundWaveData_hg_114splash, 255, 250, 0, 171
	voice_directsound 61, 102, DirectSoundWaveData_hg_032crash, 255, 248, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 62, 64, DirectSoundWaveData_hg_032crash, 255, 248, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_dp_oct_snare_16, 255, 0, 255, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_808snare_16, 255, 220, 0, 171
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 248, 0, 214
	voice_directsound 61, 64, DirectSoundWaveData_hg_032crash, 255, 248, 0, 214
	voice_directsound 60, 86, DirectSoundWaveData_dp_ridecynbal, 255, 250, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 30, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 61, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 62, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 90, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 61, 40, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 40, DirectSoundWaveData_dp_reverscyn_16, 255, 0, 255, 236
	voice_directsound 60, 40, DirectSoundWaveData_hg_033orccym, 255, 251, 0, 214
	voice_directsound 60, 58, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 60, 84, DirectSoundWaveData_hg_114splash, 255, 250, 0, 205
	voice_directsound 60, 20, DirectSoundWaveData_dp_060conga, 255, 0, 255, 205
	voice_directsound 57, 26, DirectSoundWaveData_dp_060conga, 255, 0, 255, 205
	voice_directsound 58, 26, DirectSoundWaveData_dp_060conga, 255, 0, 255, 205
	voice_directsound 60, 105, DirectSoundWaveData_dp_sheiker, 255, 234, 127, 205
	voice_directsound 60, 25, DirectSoundWaveData_dp_sheiker, 255, 234, 127, 205
	voice_directsound 61, 58, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_808snare_16, 255, 0, 227, 171
	voice_directsound 61, 105, DirectSoundWaveData_hg_032crash, 255, 250, 0, 171
	voice_directsound 60, 105, DirectSoundWaveData_dp_060conga, 255, 0, 227, 171
	voice_directsound 60, 105, DirectSoundWaveData_dp_060conga, 255, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 239, 0, 205
	voice_directsound 60, 56, DirectSoundWaveData_dp_808snare_16, 255, 0, 227, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_062chh, 255, 78, 0, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 205
	voice_directsound 48, 64, DirectSoundWaveData_dp_reverscyn_16, 3, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 226, 127, 205
	voice_directsound 60, 45, DirectSoundWaveData_hg_062chh, 255, 239, 0, 205
	voice_directsound 60, 45, DirectSoundWaveData_hg_063ohh, 255, 250, 0, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_032crash, 255, 242, 0, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_033orccym, 255, 246, 0, 214
	voice_directsound 60, 40, DirectSoundWaveData_dp_synthsnare, 255, 226, 127, 205
	voice_directsound 61, 40, DirectSoundWaveData_dp_synthsnare, 255, 226, 127, 205
	voice_directsound 60, 25, DirectSoundWaveData_hg_032crash, 255, 249, 0, 205
	voice_directsound 62, 100, DirectSoundWaveData_hg_032crash, 255, 249, 0, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_808snare_16, 255, 226, 127, 205
	voice_directsound 62, 80, DirectSoundWaveData_hg_062chh, 255, 239, 0, 205
	voice_directsound 62, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 63, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 57, 90, DirectSoundWaveData_dp_016bongo, 255, 0, 255, 205
	voice_directsound 60, 26, DirectSoundWaveData_dp_060conga, 255, 0, 255, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 30, DirectSoundWaveData_hg_135tom, 255, 0, 255, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 205
	voice_directsound 60, 50, DirectSoundWaveData_hg_138tom, 255, 0, 255, 205
	voice_directsound 61, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 205
	voice_directsound 63, 105, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_directsound 60, 90, DirectSoundWaveData_dp_triangle_16, 255, 234, 191, 205
	voice_directsound 60, 90, DirectSoundWaveData_sd90_open_triangle, 255, 234, 191, 205
	voice_directsound 61, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 61, 30, DirectSoundWaveData_hg_062chh, 255, 239, 0, 205
	voice_directsound 60, 30, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 205
	voice_directsound 60, 50, DirectSoundWaveData_dp_sne2_loop, 255, 0, 227, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_033orccym, 255, 250, 0, 214

	.align 2
voicegroup282:
	voice_directsound 60, 64, DirectSoundWaveData_dp_sne2_loop, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 78, 0, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_032crash, 255, 250, 0, 205
	voice_directsound 61, 105, DirectSoundWaveData_hg_032crash, 255, 250, 0, 205
	voice_directsound 60, 44, DirectSoundWaveData_dp_triangle_16, 255, 248, 0, 192
	voice_directsound 60, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 60, 34, DirectSoundWaveData_dp_sheiker, 255, 234, 127, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 30, DirectSoundWaveData_dp_053clap, 255, 0, 227, 205
	voice_noise_alt 60, 0, 0, 0, 2, 6, 2
	voice_noise_alt 60, 0, 0, 0, 1, 6, 0
	voice_directsound 60, 32, DirectSoundWaveData_dp_triangle_16, 255, 234, 191, 205
	voice_directsound 60, 32, DirectSoundWaveData_sd90_open_triangle, 255, 234, 191, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 227, 205
	voice_directsound 60, 58, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_093snare, 255, 226, 127, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 61, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 62, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 48, 64, DirectSoundWaveData_dp_reverscyn_16, 2, 0, 227, 171
	voice_directsound 60, 110, DirectSoundWaveData_hg_031cowbl, 255, 0, 255, 205
	voice_directsound 57, 90, DirectSoundWaveData_dp_016bongo, 255, 0, 255, 205
	voice_directsound 60, 90, DirectSoundWaveData_dp_016bongo, 255, 0, 255, 205
	voice_directsound 61, 90, DirectSoundWaveData_dp_016bongo, 255, 0, 255, 205
	voice_directsound 63, 105, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_808snare_16, 255, 0, 227, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_808snare_16, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 226, 127, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthsnare, 255, 226, 127, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 76, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 102, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 60, 30, DirectSoundWaveData_hg_135tom, 255, 0, 255, 205
	voice_directsound 60, 50, DirectSoundWaveData_hg_138tom, 255, 0, 255, 205
	voice_directsound 61, 50, DirectSoundWaveData_hg_138tom, 255, 0, 255, 205
	voice_directsound 60, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 248, 0, 214
	voice_directsound 61, 64, DirectSoundWaveData_hg_032crash, 255, 248, 0, 214
	voice_directsound 60, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 205
	voice_directsound 57, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_dp_bassdr1, 255, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_oct_snare_16, 255, 0, 255, 171
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 242, 0, 171
	voice_directsound 64, 20, DirectSoundWaveData_hg_022china, 255, 250, 0, 224
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 20, DirectSoundWaveData_hg_022china, 255, 250, 0, 224
	voice_directsound 60, 64, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_033orccym, 255, 250, 0, 214
	voice_directsound 61, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_jinglebell, 255, 234, 191, 205
	voice_directsound 57, 26, DirectSoundWaveData_dp_060conga, 255, 0, 255, 205
	voice_directsound 58, 26, DirectSoundWaveData_dp_060conga, 255, 0, 255, 205
	voice_directsound 69, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 227, 171
	voice_directsound 62, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 227, 171
	voice_directsound 60, 20, DirectSoundWaveData_dp_060conga, 255, 0, 255, 205
	voice_directsound 60, 24, DirectSoundWaveData_dp_060conga, 255, 0, 255, 205
	voice_directsound 65, 105, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_directsound 66, 105, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171

	.align 2
voicegroup283:
	voice_directsound 60, 64, DirectSoundWaveData_dp_ridecynbal, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 62, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 64, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 73, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 64, 64, DirectSoundWaveData_sc88pro_rnd_kick, 255, 0, 255, 205
	voice_directsound 65, 64, DirectSoundWaveData_sc88pro_rnd_kick, 255, 0, 227, 205
	voice_directsound 66, 64, DirectSoundWaveData_sc88pro_rnd_kick, 255, 0, 255, 205
	voice_directsound 67, 64, DirectSoundWaveData_sc88pro_rnd_kick, 255, 0, 255, 205
	voice_directsound 68, 64, DirectSoundWaveData_sc88pro_rnd_kick, 255, 0, 255, 205
	voice_directsound 60, 64, DirectSoundWaveData_sc88pro_rnd_kick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 255, 171
	voice_directsound 61, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 255, 171
	voice_directsound 62, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 255, 171
	voice_directsound 65, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 255, 171
	voice_directsound 60, 64, DirectSoundWaveData_sc88pro_rnd_kick, 255, 0, 227, 205
	voice_directsound 62, 64, DirectSoundWaveData_sc88pro_rnd_kick, 255, 0, 227, 205
	voice_directsound 64, 64, DirectSoundWaveData_sc88pro_rnd_kick, 255, 0, 227, 205
	voice_directsound 60, 15, DirectSoundWaveData_hg_135tom, 255, 0, 255, 171
	voice_directsound 60, 30, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 61, 30, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 60, 40, DirectSoundWaveData_hg_138tom, 255, 0, 255, 171
	voice_directsound 60, 100, DirectSoundWaveData_hg_138tom, 255, 0, 255, 171
	voice_directsound 60, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 61, 110, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 62, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 63, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 64, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 65, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 66, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 67, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 68, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 70, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 71, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 72, 120, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 45, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 2
	voice_directsound 46, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 2
	voice_directsound 48, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_hg_hoohclap, 255, 0, 255, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_093snare, 255, 226, 127, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_093snare, 255, 226, 127, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 226, 127, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 171
	voice_directsound 62, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 62, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_sne2_loop, 255, 255, 191, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 0
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 2
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 2
	voice_directsound 62, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 226, 127, 205
	voice_directsound 60, 58, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 66, 64, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_033orccym, 255, 250, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_hg_033orccym, 255, 250, 0, 214
	voice_directsound 60, 20, DirectSoundWaveData_hg_022china, 255, 248, 0, 224
	voice_directsound 61, 20, DirectSoundWaveData_hg_022china, 255, 248, 0, 224
	voice_directsound 62, 20, DirectSoundWaveData_hg_022china, 255, 248, 0, 224
	voice_directsound 60, 64, DirectSoundWaveData_hg_022china, 255, 248, 0, 205
	voice_directsound 60, 100, DirectSoundWaveData_hg_022china, 255, 248, 0, 224
	voice_directsound 61, 100, DirectSoundWaveData_hg_022china, 255, 248, 0, 224
	voice_directsound 62, 100, DirectSoundWaveData_hg_022china, 255, 248, 0, 224
	voice_directsound 64, 107, DirectSoundWaveData_hg_022china, 255, 248, 0, 224
	voice_directsound 60, 40, DirectSoundWaveData_hg_032crash, 255, 250, 0, 205
	voice_directsound 60, 14, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 36, DirectSoundWaveData_hg_032crash, 255, 248, 0, 214
	voice_directsound 64, 44, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 57, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 214
	voice_directsound 61, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 62, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 63, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 64, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 65, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 66, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 67, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 63, 84, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 100, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 114, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 61, 114, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 62, 114, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 63, 114, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 64, 114, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 65, 114, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 66, 114, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 67, 114, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 205
	voice_directsound 62, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 205
	voice_directsound 60, 84, DirectSoundWaveData_hg_114splash, 255, 250, 0, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 78, 0, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 242, 0, 171
	voice_directsound 61, 80, DirectSoundWaveData_hg_062chh, 255, 242, 0, 171
	voice_directsound 60, 104, DirectSoundWaveData_hg_062chh, 255, 242, 0, 171
	voice_directsound 61, 104, DirectSoundWaveData_hg_062chh, 255, 242, 0, 171
	voice_directsound 63, 104, DirectSoundWaveData_hg_062chh, 255, 242, 0, 171
	voice_directsound 60, 80, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 171
	voice_directsound 61, 80, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 171
	voice_directsound 60, 104, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 171
	voice_directsound 72, 104, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 171
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 224, 0, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 205
	voice_directsound 62, 80, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 205
	voice_directsound 60, 44, DirectSoundWaveData_hg_062chh, 255, 0, 255, 0
	voice_directsound 60, 64, DirectSoundWaveData_hg_062chh, 255, 0, 255, 0
	voice_directsound 61, 64, DirectSoundWaveData_hg_062chh, 255, 0, 255, 0
	voice_directsound 62, 64, DirectSoundWaveData_hg_062chh, 255, 0, 255, 0
	voice_directsound 63, 64, DirectSoundWaveData_hg_062chh, 255, 0, 255, 0
	voice_directsound 60, 44, DirectSoundWaveData_hg_063ohh, 255, 0, 255, 0
	voice_directsound 60, 64, DirectSoundWaveData_hg_063ohh, 255, 0, 255, 0
	voice_directsound 61, 64, DirectSoundWaveData_hg_063ohh, 255, 0, 255, 0
	voice_directsound 64, 64, DirectSoundWaveData_hg_031cowbl, 255, 0, 255, 0
	voice_directsound 60, 30, DirectSoundWaveData_dp_tambourine_16, 255, 0, 227, 205
	voice_directsound 60, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_castanet_22, 255, 0, 255, 171
	voice_directsound 61, 64, DirectSoundWaveData_dp_castanet_22, 255, 0, 255, 171

	.align 2
voicegroup284:
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 2
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 255, 0
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 0
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 2
	voice_directsound 51, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 0
	voice_directsound 63, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 66, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 30, DirectSoundWaveData_dp_ridecap, 255, 250, 0, 210
	voice_directsound 49, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 2
	voice_directsound 62, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 2
	voice_directsound 63, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 2
	voice_directsound 58, 64, DirectSoundWaveData_hg_033orccym, 255, 0, 227, 205
	voice_directsound 59, 64, DirectSoundWaveData_hg_033orccym, 255, 0, 227, 205
	voice_directsound 57, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 0
	voice_directsound 59, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 0
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 0
	voice_directsound 62, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 0
	voice_directsound 64, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 255, 0
	voice_directsound 60, 64, DirectSoundWaveData_ethnic_flavours_hyoushigi, 255, 0, 255, 0
	voice_directsound 65, 64, DirectSoundWaveData_ethnic_flavours_hyoushigi, 255, 0, 255, 0
	voice_directsound 60, 64, DirectSoundWaveData_dp_808snare_16, 255, 220, 0, 171
	voice_directsound 60, 70, DirectSoundWaveData_dp_053clap, 255, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_hg_033orccym, 255, 246, 0, 214
	voice_directsound 62, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171
	voice_directsound 57, 90, DirectSoundWaveData_dp_016bongo, 255, 0, 255, 205
	voice_directsound 60, 90, DirectSoundWaveData_dp_016bongo, 255, 0, 255, 205
	voice_directsound 59, 26, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_directsound 60, 105, DirectSoundWaveData_dp_mute_conga_16, 255, 0, 255, 171
	voice_directsound 60, 44, DirectSoundWaveData_dp_triangle_16, 255, 248, 0, 192
	voice_directsound 60, 30, DirectSoundWaveData_hg_135tom, 255, 0, 255, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 205
	voice_directsound 61, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 205
	voice_directsound 60, 50, DirectSoundWaveData_hg_138tom, 255, 0, 255, 205
	voice_directsound 60, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 205
	voice_directsound 61, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 205
	voice_directsound 63, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 205
	voice_directsound 65, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 205
	voice_directsound 63, 100, DirectSoundWaveData_hg_022china, 255, 248, 0, 224
	voice_directsound 64, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_ethnic_flavours_ohtsuzumi, 255, 0, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_ethnic_flavours_ohtsuzumi, 255, 0, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_ethnic_flavours_ohtsuzumi, 255, 0, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_ethnic_flavours_ohtsuzumi, 255, 0, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_ethnic_flavours_ohtsuzumi, 255, 241, 0, 171
	voice_directsound 60, 64, DirectSoundWaveData_hg_hoohclap, 255, 197, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 227, 0
	voice_directsound 60, 64, DirectSoundWaveData_hg_033orccym, 255, 250, 0, 214
	voice_directsound 47, 64, DirectSoundWaveData_hg_hoohclap, 255, 197, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 72, 64, DirectSoundWaveData_dp_synthsnare, 255, 0, 227, 205
	voice_directsound 66, 64, DirectSoundWaveData_dp_synthsnare, 255, 226, 127, 171
	voice_directsound 67, 64, DirectSoundWaveData_dp_synthsnare, 255, 226, 127, 171
	voice_directsound 60, 27, DirectSoundWaveData_hg_114splash, 255, 250, 0, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_114splash, 255, 250, 0, 205
	voice_directsound 60, 100, DirectSoundWaveData_hg_114splash, 255, 250, 0, 205
	voice_directsound 61, 100, DirectSoundWaveData_hg_114splash, 255, 250, 0, 205
	voice_directsound 72, 64, DirectSoundWaveData_dp_triangle_16, 255, 0, 227, 2
	voice_directsound 60, 64, DirectSoundWaveData_sd90_open_triangle, 255, 246, 0, 214
	voice_directsound 59, 20, DirectSoundWaveData_hg_022china, 255, 250, 0, 224
	voice_directsound 61, 102, DirectSoundWaveData_hg_032crash, 255, 248, 0, 214
	voice_directsound 60, 30, DirectSoundWaveData_hg_135tom, 255, 0, 255, 171
	voice_directsound 56, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 58, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 59, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 60, 50, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 62, 50, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 63, 50, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 60, 90, DirectSoundWaveData_hg_138tom, 255, 0, 255, 171
	voice_directsound 61, 90, DirectSoundWaveData_hg_138tom, 255, 0, 255, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_handclup1, 255, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_bassdr1, 255, 0, 227, 171
	voice_directsound 83, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 62, 80, DirectSoundWaveData_dp_closed_hihat_16, 255, 0, 227, 1
	voice_directsound 60, 80, DirectSoundWaveData_dp_open_hihat_16, 255, 241, 0, 0
	voice_directsound 60, 64, DirectSoundWaveData_dp_053clap, 255, 197, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 227, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 239, 0, 205
	voice_directsound 60, 80, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 205
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 205

	.align 2
voicegroup285:
	voice_directsound 60, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 255, 0
	voice_directsound 60, 64, DirectSoundWaveData_dp_bassdr1, 255, 0, 255, 171
	voice_directsound 60, 80, DirectSoundWaveData_dp_closed_hihat_16, 255, 0, 227, 1
	voice_directsound 60, 80, DirectSoundWaveData_dp_open_hihat_16, 255, 241, 0, 0
	voice_directsound 59, 20, DirectSoundWaveData_hg_022china, 255, 250, 0, 226
	voice_directsound 60, 36, DirectSoundWaveData_dp_ridecynbal, 255, 250, 0, 216
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 34, DirectSoundWaveData_dp_sheiker, 255, 234, 127, 205
	voice_directsound 61, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 61, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 227, 1
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 227, 1
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 61, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 62, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 63, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 64, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 65, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 60, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 61, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 62, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 63, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_directsound 64, 64, DirectSoundWaveData_hg_130jdrumh, 255, 0, 255, 1
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_directsound 60, 64, DirectSoundWaveData_dp_handclup1, 255, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_sne2_loop, 255, 255, 191, 171
	voice_directsound 48, 64, DirectSoundWaveData_dp_reverscyn_16, 2, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_bass2, 255, 0, 227, 171
	voice_directsound 61, 64, DirectSoundWaveData_dp_bass2, 255, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_808snare_16, 255, 220, 0, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 255, 171
	voice_directsound 61, 64, DirectSoundWaveData_dp_bassdr2_16, 255, 0, 255, 171
	voice_directsound 61, 58, DirectSoundWaveData_dp_rim1_roop, 255, 220, 0, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_oct_snare_16, 255, 0, 255, 171
	voice_directsound 61, 64, DirectSoundWaveData_dp_oct_snare_16, 255, 0, 255, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_093snare, 255, 226, 127, 171
	voice_directsound 60, 30, DirectSoundWaveData_hg_135tom, 255, 0, 255, 171
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 242, 0, 171
	voice_directsound 60, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 61, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 60, 50, DirectSoundWaveData_hg_138tom, 255, 0, 255, 171
	voice_directsound 60, 80, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 171
	voice_directsound 60, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 61, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 57, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 214
	voice_directsound 58, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 214
	voice_directsound 59, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 214
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 214
	voice_directsound 61, 64, DirectSoundWaveData_hg_032crash, 255, 242, 0, 214
	voice_directsound 60, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171

	.align 2
voicegroup286:
	voice_directsound 61, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171
	voice_directsound 62, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171
	voice_directsound 63, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171
	voice_directsound 64, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171
	voice_directsound 65, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171
	voice_directsound 66, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171
	voice_directsound 67, 58, DirectSoundWaveData_dp_tambourine_16, 255, 0, 255, 171
	voice_directsound 60, 20, DirectSoundWaveData_dp_mute_conga_16, 255, 0, 255, 171
	voice_directsound 60, 24, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_directsound 57, 26, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_directsound 58, 26, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_directsound 59, 26, DirectSoundWaveData_dp_060conga, 255, 0, 255, 171
	voice_directsound 60, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 61, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 62, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 63, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 64, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 65, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 66, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 67, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 68, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 69, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 70, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 71, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 72, 26, DirectSoundWaveData_dp_cabassa_16, 255, 242, 127, 171
	voice_directsound 60, 44, DirectSoundWaveData_dp_triangle_16, 255, 197, 0, 1
	voice_directsound 60, 44, DirectSoundWaveData_dp_triangle_16, 255, 248, 0, 192
	voice_directsound 60, 64, DirectSoundWaveData_dp_handclup1, 255, 0, 227, 171
	voice_directsound 61, 64, DirectSoundWaveData_dp_handclup1, 255, 0, 227, 171
	voice_directsound 60, 64, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 62, 64, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 63, 64, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 64, 64, DirectSoundWaveData_dp_rim1_roop, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 61, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 62, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 63, 64, DirectSoundWaveData_dp_synthkick, 255, 0, 227, 205
	voice_directsound 60, 64, DirectSoundWaveData_dp_093snare, 255, 226, 127, 205
	voice_directsound 60, 30, DirectSoundWaveData_hg_135tom, 255, 0, 255, 171
	voice_directsound 60, 80, DirectSoundWaveData_hg_062chh, 255, 78, 0, 205
	voice_directsound 60, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 61, 40, DirectSoundWaveData_hg_137tom, 255, 0, 255, 171
	voice_directsound 60, 50, DirectSoundWaveData_hg_138tom, 255, 0, 255, 171
	voice_directsound 60, 80, DirectSoundWaveData_hg_063ohh, 255, 241, 0, 205
	voice_directsound 60, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 61, 100, DirectSoundWaveData_hg_136tom, 255, 0, 255, 171
	voice_directsound 60, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 61, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 62, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 63, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 64, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 65, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 66, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 67, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 68, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 69, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 70, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 71, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 72, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 73, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 74, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 75, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 76, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 77, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 78, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 79, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 80, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 81, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 82, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 83, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 84, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 85, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 86, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 87, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 88, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 89, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 90, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 91, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 92, 64, DirectSoundWaveData_hg_032crash, 255, 246, 0, 214
	voice_directsound 60, 30, DirectSoundWaveData_dp_tambourine_16, 255, 224, 227, 205
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0
	voice_square_1 60, 0, 0, 2, 0, 0, 15, 0

