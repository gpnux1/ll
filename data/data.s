	.include "asm/macros.inc"
	.include "constants/gba_constants.inc"

	.section .rodata

	.equiv MPLAY_SIZE, 12
	.equiv SONG_SIZE, 8
	

@ gUnk_08057854: @ 08057854
@	.incbin "baserom.gba", 0x57854, 0x6AF214 - 0x57854

@gUnk_08059794: @ 0805881C
@	.incbin "baserom.gba", 0x59794, 0x6AF214 - 0x59794

rom_data:
	/* 0x0808760C..0x080BAF54 已搬到 src/data_805769C.c:
	   gCharNameTextBlock_* + gCharaCmdStream_* + gChestSpawnTable +
	   gDigitFontObjPalettes + gDigitFontObjTiles + gMapSceneDescriptors +
	   gSaveMapUnlockFlags + gMapViewportBoundsTable + gMenuEntDescGroups +
	   gMenuEntPaletteFrames (含原 gFlashFxPaletteTable 区, 173 项调色板动画帧库) +
	   gSaveMenuUiPalettes + 18 段切分 (gAnimModelGroups/byte_8091948/unk_80921F0/unk_8092248/unk_80923D8/unk_80933DC/byte_8093418/byte_80936A0/stru_8095028/stru_8095828/gMapBgTables/gSaveMetaArea/gSaveMiscTables/gMapGfxLz77Blocks/gSaveMenuGfxLz77/gDialogDataBlocks/gUnk_080B9DFC/gDataTail_080BAADC) */
	.incbin "baserom.gba", 0xBAF54, 0x38EEF4 - 0xBAF54

	.global gUnk_0838EEF4
gUnk_0838EEF4:
	/* 0x0838EEF4..0x08393B28 由 data.s 提供;
	   0x08393B28 起为 src/data_08393B28.c;
	   0x083988A8 起为 data/data1d.s */
	.incbin "baserom.gba", 0x38EEF4, 0x393B28 - 0x38EEF4
