	.section .rodata

	.include "asm/macros.inc"
	.include "constants/gba_constants.inc"

gMPlayInfos2: @ 087EDC80
	/* 0x087EDC80..0x08800000 兜底 blob 由 data1c.s 提供 */
	.incbin "baserom.gba", 0x7EDC80, 0x800000 - 0x7EDC80
