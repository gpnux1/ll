	.include "asm/macros.inc"
	.include "constants/gba_constants.inc"

	.section .rodata


gUnk_087E83F0: @ 087E83F0
	/* 0x087E9554..0x087E9818 (三张指针表, 740 B) 已搬到 src/data_87E83F0.c */
	.incbin "baserom.gba", 0x7E9818, 0x7EA1A0 - 0x7E9818
