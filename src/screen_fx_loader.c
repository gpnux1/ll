#include "screen_fx_loader.h"

#include "map_scene_runtime.h"
#include "battle_types.h"
#include "engine_core.h"
#include "map_view.h"
#include "player_stats.h"
#include "scene_mgr.h"
#include "script_vm.h"
#include "sound.h"
#include "sprite_engine.h"
#include "text_engine.h"
#include "vram_transfer.h"
#include "data_87E83F0.h"
#include "data_805769C.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"

// @ 0x080088B4
void ScreenFade_Start(u16 flags, s16 step, s16 param)
{
    gScreenFadeParam = param;
    gScreenFadeStep = step;

    if (step < 0)
    {
        gScreenFadeProgress = 0x1B0;
    }
    else
    {
        gScreenFadeProgress = 0;
    }

    gScreenFadeFlags = flags;
}

// @ 0x080088F4
void AnimSlot_BankReload(void)
{
    if ((u32) * (u16 *)0x0300467C <= 0xFCU)
    {
        *(s32 *)0x03004620 = 0x02006000;
        *(s32 *)0x0300482C = 0x02001000;
        *(s32 *)0x03004690 = 0x02001800;
        *(s32 *)0x030047E8 = 0x02001C00;
        sub_80052F8();
        *(s32 *)0x03004620 = 0x0200E000;
        *(s32 *)0x0300482C = 0x02003000;
        *(s32 *)0x03004690 = 0x02003800;
        *(s32 *)0x030047E8 = 0x02003C00;
        sub_80052F8();
    }
}

/* 用 DMA0 把 gWin0HWaveTable 里的一个 WIN0H 值按扫描线喂给 REG_WIN0H,
 * 索引 = VCOUNT 的三角波 (0→81→0 跨一屏) → 窗口水平边界随扫描线摆动。
 *
 * ⚠ 全 ROM 无调用点 (code.s 里 6360 个 bl 全部已符号化, 没有 `bl Win0H_WaveDmaByVCount`;
 *   0x08008978 也未作为指针出现在任何数据里) → **死代码**。
 *   注: 旧名 HBlank_WaveDma / 旧注释"声音DMA0旋转" 是误读 —— 目的端 0x04000040
 *   是 REG_WIN0H (io.h: REG_OFFSET_WIN0H=0x40), 不是声音 FIFO (FIFO A/B 在 0xA0/0xAC)。
 */
// @ 0x08008978
void Win0H_WaveDmaByVCount(void)
{
    u16 waveIdx;
    u8 subState = (u8)(gScreenTransitionState - 1);
    if (subState > 1)
    {
        return;
    }
    /* VCOUNT 低字节 (字节读 = 目标的 ldrb, 不能写 (u8)REG_VCOUNT) */
    waveIdx = *(u8 *)&REG_VCOUNT;
    if (waveIdx > 0x9F)
    { /* > 159: 屏外扫描线 */
        if (waveIdx <= 0xE1)
        {
            return;
        }
        waveIdx = 0;
    }
    else
    {
        waveIdx = waveIdx + 1;
    }
    if (waveIdx > 0x51)
    { /* 三角波: 0x52 半周期 */
        waveIdx = waveIdx - 0x52;
    }
    else
    {
        waveIdx = 0x51 - waveIdx;
    }
    DmaSet(0, gWin0HWaveTable + (waveIdx * 2), (void *)REG_ADDR_WIN0H, 0xE0400001);
}
// @ 0x080089E0
void ScreenFx_SetMode(u16 mode)
{

    switch (mode)
    {
        case 1:
            gWindowTransitionProgress = 0;
            REG_WIN0H = DISPLAY_WIDTH;
            REG_WIN0V = DISPLAY_HEIGHT;
            REG_WININ = 0x3F;
            REG_WINOUT = 0;
            break;
        case 0:
            gWindowTransitionProgress = DISPLAY_WIDTH;
            break;
        default:
            PaletteFx_Apply(mode - 3);
            break;
    }

    gScreenTransitionState = mode;
}
