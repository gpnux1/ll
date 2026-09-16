#ifndef ENGINE_CORE_H
#define ENGINE_CORE_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

extern void sub_8000170();

void VBlankIntr(void);

u8 Rand_TableNext();

s32 Sio_LinkTask();

void EventFlags_ClearAll();

u8 EventFlags_Test(u16);

void EventFlags_Set(u16);

void EventFlags_Reset(u16);

void SwitchFlags_ClearAll();

u8 SwitchFlags_Test(u16);

void SwitchFlags_Set(u16);

void SwitchFlags_Reset(u16);

void SwitchFlags_ClearRange();

void Display_ShutdownSequence();

void Intr_HandleHBlank();

void nullsub_5();

void DummyIntr3();

void DummyIntr4();

void VBlank_UpdateScreenMode5();

/* 逐扫描线水波效果族 (源数据 = gWaveSineTable @0x080576D0, 见 data_805769C.h) */
void HBlankWave_BuildTables(u16 mode);

void HBlankWave_ApplyLineScroll(u16 scanline);

u32 LZ_UncompressChunk(void); // @0x08000D5C 分块 LZ 流式解压 gLzContext, 返回 0 = 全部完成

void LZ_InitContext(u8 *dest, struct LzHeader *arg1, u32 arg2);

void System_ResetToLogo(void);

#endif // ENGINE_CORE_H
