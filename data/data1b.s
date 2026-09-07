	.section .rodata

	.include "asm/macros.inc"
	.include "constants/gba_constants.inc"

gUnk_087EA580: @ 087EA580
	/* 0x087EA1A0..0x087EA580 (248 项精灵资源配置指针表) 由 src/data_087EA1A0.c 提供
	   (linker 以 ORIGIN+0x7EA1A0 锚位) */
	.incbin "baserom.gba", 0x7EA580, 0x800000 - 0x7EA580
