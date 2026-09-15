	.section .rodata

	.include "asm/macros.inc"
	.include "constants/gba_constants.inc"

	/* 0x083988A8..0x0861C784 兜底 blob 由 data1d.s 提供。
	   0x08393B28..0x083988A8 (ObjAnimEntry gUnk_08393B28) 已搬到 src/data_08393B28.c。 */
	.incbin "baserom.gba", 0x3988A8, 0x61C784 - 0x3988A8
