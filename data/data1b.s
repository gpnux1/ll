	.section .rodata

	.include "asm/macros.inc"
	.include "constants/gba_constants.inc"

gUnk_087EA580: @ 087EA580
	/* 0x087EA580..0x087ED6D4 由 data1b.s 提供 */
	.incbin "baserom.gba", 0x7EA580, 0x7ED6D4 - 0x7EA580
