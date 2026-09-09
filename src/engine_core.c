#include "code_0.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "globals.h"
#include "data_87E83F0.h"
#include "data_805769C.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "sound.h"

extern void IntrMain();

// @ 0x080002A0
void VBlank_UpdateGameScreen(void)
{
    u16 val;

    VramTransfer_Flush();

    switch (gBg1ScrollMode)
    {
        case 1:
            REG_BG1HOFS = 0;
            REG_BG1VOFS = 0;
            break;
        case 2:
            gUnk_03002C40++;
            REG_BG1HOFS = (gUnk_03002C40 >> 1) & 0xFF;
            REG_BG1VOFS = 0;
            break;
        case 3:
            gUnk_03002C40++;
            REG_BG1HOFS = (gUnk_03002C40 >> 2) & 0xFF;
            REG_BG1VOFS = 0x10;
            break;
        case 4:
            REG_BG1HOFS = 0x40;
            REG_BG1VOFS = 0;
            break;
        case 5:
            REG_BG1HOFS = (s16)((u8)gCameraPosX >> 3);
            REG_BG1VOFS = 0;
            break;
        case 6:
            REG_BG1HOFS = 0;
            REG_BG1VOFS = 0x1E;
            break;
        default:
            REG_BG1HOFS = (0x1F & gCameraPosX);
            REG_BG1VOFS = (0x1F & gCameraPosY);
            break;
    }

    REG_BG2HOFS = gBG2ScrollX;
    REG_BG2VOFS = gBG2ScrollY;
    REG_BG3HOFS = gBG3ScrollX;
    REG_BG3VOFS = gBG3ScrollY;

    DmaCopy16(3, gOamBuffer, OAM, OAM_SIZE);

    LogoAssets_Load(); // Show Lunar Logo
    DialogPortrait_FlushPending();

    if (gLogoEffectState == 0)
    {
        ScriptPump_ServiceFrame();
    }

    SceneBg_Reload();

    if (gViewportFlags[13] != 0)
    {
        DmaCopy16(3, gUnk_02005380, 0x0600F380, 0x100);
        gViewportFlags[13] = 0U;
    }

    REG_BLDCNT = gBlendControl;
    if (gBlendControl & 0x80)
    {
        REG_BLDY = gBlendCoefficients;
    }
    else
    {
        REG_BLDALPHA = gBlendCoefficients;
    }

    VBlank_UpdateSpriteAndWindow();
    PendingSpriteLoad_Flush();

    switch (gHBlankEffectMode)
    {
        case 0:
        default:
            break;
        case 1:
        case 3:
            val = (gHBlankScrollCounter - 1) & 0x3FF;
            gHBlankScrollCounter = val;
            gHBlankWaveRow = (val >> 2) + gCameraPosY;
            break;
        case 2:
        case 4:
            val = (gHBlankScrollCounter - 1) & 0x7FF;
            gHBlankScrollCounter = val;
            gHBlankWaveRow = (val >> 3) + gCameraPosY;
    }
}

// @ 0x080004F8
void VBlank_UpdateScreenSimple(void)
{
    u16 val;

    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;
    REG_BG1HOFS = 0;
    REG_BG1VOFS = 0;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;
    REG_BG3HOFS = 0;
    REG_BG3VOFS = 0;

    DmaCopy16(3, gOamBuffer, OAM, 0x400);

    REG_BLDCNT = gBlendControl;
    REG_BLDALPHA = gBlendCoefficients;

    DmaCopy16(3, VRAM_BUF_2005800, 0x0600F800, 0x800);

    VBlank_UpdateSpriteAndWindow();
    val = (gHBlankScrollCounter - 1) & 0x3FF;
    gHBlankScrollCounter = val;
    gHBlankWaveRow = (val >> 2) + gCameraPosY;
}

/* H-Blank 中断里逐扫描线调用 (Intr_HandleHBlank → 本函数, 实参 = REG_VCOUNT & 0xFF)。
 * 用 HBlankWave_BuildTables 预先算好的两张表, 把 BG1 的每行偏移重写成水波:
 *   索引 = (gHBlankWaveRow + scanline) & 0xFF, gHBlankWaveRow 由 VBlank 每帧推进 (相位)。
 * mode 1/2 额外叠加摄像机低 5 位, mode 3/4 纯波; mode 0 = 不动 (case 缺省不写)。
 */
// @ 0x080005A8
void HBlankWave_ApplyLineScroll(u16 scanline)
{
    switch (gHBlankEffectMode)
    {
        case 1:
        case 2:
            REG_BG1HOFS = gHBlankWaveH[(gHBlankWaveRow + scanline) & 0xFF] + (gCameraPosX & 0x1F);
            REG_BG1VOFS = gHBlankWaveV[(gHBlankWaveRow + scanline) & 0xFF] + (gCameraPosY & 0x1F);
            break;

        case 3:
            REG_BG1HOFS = gHBlankWaveH[(gHBlankWaveRow + scanline) & 0xFF];
            REG_BG1VOFS = gHBlankWaveV[(gHBlankWaveRow + scanline) & 0xFF];
            break;

        case 4:
            REG_BG1HOFS = gHBlankWaveH[(gHBlankWaveRow + scanline) & 0xFF];
            REG_BG1VOFS = gHBlankWaveV[(gHBlankWaveRow + scanline) & 0xFF];
            break;
    }
}

// VBlankIntr
// @ 0x0800065C
void VBlankIntr(void)
{

    switch (gVBlankPipelineMode)
    {
        case 1:
            m4aSoundVSync();
            VBlank_UpdateGameScreen();
            break;
        case 2:
            m4aSoundVSync();
            sub_8018070();
            break;
        case 3:
            m4aSoundVSync();
            VBlank_UpdateScreenSimple();
            break;
        case 4:
            m4aSoundVSync();
            VramTransfer_Flush();

            REG_BG1HOFS = 0;
            REG_BG1VOFS = 0;
            REG_BG2HOFS = 4;
            REG_BG2VOFS = 0;
            REG_BG3HOFS = 4;
            REG_BG3VOFS = 0;

            REG_BLDCNT = gBlendControl;
            REG_BLDALPHA = gBlendCoefficients;

            DmaCopy16(3, gOamBuffer, OAM, 0x400);
            DmaCopy16(3, VRAM_BUF_2005800, 0x0600F800, 0x800);

            if (gBgTileReloadFlag != 0)
            {
                DmaCopy16(3, VRAM_BUF_2005000, 0x0600F000, 0x800);
                gBgTileReloadFlag = 0;
            }

            VBlank_UpdateSpriteAndWindow();
            break;
        case 5:
            sub_8016F30();
            m4aSoundVSync();
            VramTransfer_Flush();
            PalTransfer_Flush();

            // CpuFastCopy(gOamBuffer, (u32*)0x07000000, 0x400);
            CpuCopy(gOamBuffer, (void *)0x07000000, 0x400);

            REG_BG1HOFS = 0;
            REG_BG1VOFS = 0;
            REG_BG2HOFS = 0;
            REG_BG2VOFS = 0;
            REG_BG3HOFS = 0;
            REG_BG3VOFS = 0;
            REG_BLDCNT = gBlendControl;
            REG_BLDALPHA = gBlendCoefficients;

            // CpuFastCopy(gWindowBgBuf, 0x0600F800, 0x800);
            CpuCopy(VRAM_BUF_2005800, (void *)0x0600F800, 0x800);

            if (gBgTileReloadFlag != 0)
            {
                // CpuFastCopy(gUnk_02005000, 0x0600F000, 0x800);
                CpuCopy(VRAM_BUF_2005000, (void *)0x0600F000, 0x800);
                gBgTileReloadFlag = 0;
            }

            break;
        case 6:
            m4aSoundVSync();

            REG_BG1HOFS = 0;
            REG_BG1VOFS = 0;
            REG_BG2HOFS = 0;
            REG_BG2VOFS = 0;
            REG_BG3HOFS = 0;
            REG_BG3VOFS = 0;

            VramTransfer_Flush();
            DmaCopy16(3, gOamBuffer, OAM, 0x400);

            MapBg_FlushPending();
            ScriptPump_ServiceFrame();

            REG_BLDCNT = gBlendControl;
            if (gBlendControl & 0x80)
            {
                REG_BLDY = gBlendCoefficients;
            }
            else
            {
                REG_BLDALPHA = gBlendCoefficients;
            }

            VBlank_UpdateSpriteAndWindow();
            PendingSpriteLoad_Flush();
            break;
        default:
            m4aSoundVSync();
            break;
    }

    gRandCursor++;

    if (gGameTimer <= 0x0CDFD7EE)
    {
        gGameTimer++;
    }
    else
    {
        gGameTimer = 0x0CDFD7F0;
    }

    gFrameCounter++;
    REG_IME = 0;
    gUnk_03007FF8 |= 1;
    REG_IME = 1;
}

// @ 0x080008CC
void Display_ShutdownSequence(void)
{

    Sfx_StopTrack(0);
    Sfx_StopTrack(1);
    Sfx_StopTrack(2);
    Sfx_StopTrack(3);
    Bgm_SetVolume(0);
    VBlankIntrWait();

    while (REG_DISPSTAT & 1)
        ;

    SoundMain_Frame();
    Bgm_Stop();
    VBlankIntrWait();

    while (REG_DISPSTAT & 1)
        ;

    SoundMain_Frame();
    Sound_VSyncOff();
    VBlankIntrWait();
    while (REG_DISPSTAT & 1)
        ;

    SoundMain_Frame();
    VBlankIntrWait();
    REG_DISPCNT |= DISPCNT_FORCED_BLANK;
}

// @ 0x0800096C
s32 Sio_LinkTask(void)
{

    switch (gSioLinkState)
    {
        case 0:
            REG_RCNT = 0xC000;
            VBlankIntrWait();
            Sio_SetXferCtx((u32 *)&gSioRecvPacket, (u32 *)&gSioSendPacket, 0x10, 0);
            gSioSession.unk5E = 1;
            gSioSession.unk2 = 1;
            gSioLinkState++;
        case 1:
            gSioRecvWord = sub_80171E4();

            if (gSioRecvWord == -1)
            {
                gUnk_030025A8 = 5;
                gSioLinkState = 6;
                break;
            }

            if ((Sio_IsHost() == 0) && (gSioRecvWord <= 0xFF))
            {
                if (gSioRetryTimer == 0)
                {
                    Sio_ClearSlot();
                    gSioLinkState = 4;
                    gUnk_030025A8 = 2;
                }
                gSioRetryTimer--;
                break;
            }

            switch (gSioRecvWord)
            {
                case 0:
                case 1:
                    if (gSioRetryTimer == 0)
                    {
                        Sio_ClearSlot();
                        gSioLinkState = 4;
                        gUnk_030025A8 = 2;
                    }
                    else
                    {
                        gSioRetryTimer--;
                    }
                    break;

                case 2:
                    if (Sio_IsHost() == 0)
                    {
                        Sio_ClearSlot();
                        gSioLinkState = 5;
                        gUnk_030025A8 = 3;
                    }
                    else if (gUnk_030025A8 == 0)
                    {
                        gSioSession.unk5E = 1;
                        gSioSession.unk2 = 1;
                        gUnk_030025A8 = 1;
                    }
                    break;

                case 0x101:
                case 0x102:
                case 0x103:
                case 0x104:
                    gUnk_030025A8 = 3;
                    gSioLinkState = 5;
                    break;

                default:
                    break;
            }

            break;
        case 2:
        case 3:
        case 4:
            Sio_ClearSlot();
            gSioLinkState = 9;
            break;
        case 5:
            Sio_ClearSlot();
            gSioLinkState = 9;
            break;
        case 6:
            gSioLinkState = 0;
            break;
        case 7:
            Sio_ClearSlot();
            break;
        case 8:
            if (gUnk_030025A8 == 0)
            {
                Sio_ClearSlot();
                gSioLinkState = 0;
            }
            break;
        case 9:
            Sio_ClearSlot();
            gSioLinkState = 8;
            break;
    }

    return Sio_IsHost();
}

// @ 0x08000B58
void System_SoftReset(u32 arg0)
{
    u16 i;

    DmaFill16(3, 0, (void *)VRAM, VRAM_SIZE);
    DmaFill32(3, 0, (void *)OAM, OAM_SIZE);
    DmaFill16(3, 0, (void *)PLTT, PLTT_SIZE);
    DmaFill16(3, 0, &gUnk_02004000, 0x2000);

    SpritePool_Clear();
    Queue34C0_Clear();

    gGameTimer = 0;
    gPendingSpriteLoad = 0;
    gLogoEffectState = 0;
    gPaletteFxMode = 0;
    gScreenTransitionState = 0;
    gScreenFadeFlags = 0;
    gSceneEntryFlag = 0xFF;

    for (i = 0; i < 16; i++)
    {
        gViewportFlags[i] = 0;
    }

    for (i = 0; i < 128; i++)
    {
        gOamBuffer[i].attrs[0] = 0;
        gOamBuffer[i].attrs[1] = 0;
    }

    VramTransfer_Clear();
    PalTransfer_Clear();
    MenuEnt_ClearStates();
    SwitchFlags_ClearAll();
    EventFlags_ClearAll();
    ChestFlags_ClearAll();

    gVBlankPipelineMode = 0;
    gUnk_03004D4C = 0;
    gUnk_03004D40 = 0;
    gCameraSnapFlag = 0;
    gDialogueActive = 0;
    gUnk_0300483C = 0;
    gAfterBattleCounter = 0;
}

/* 用 gWaveSineTable 算出两张"逐扫描线 BG1 偏移表" gHBlankWaveH / gHBlankWaveV。
 *
 * mode (写入 gHBlankEffectMode, 见 VBlank_UpdateGameScreen / HBlankWave_ApplyLineScroll):
 *   0 / 其它 = 不构建 (直接 return), 1..4 = 四种涟漪预设
 *
 *   mode  tableMask  hStep hDiv | vStep vDiv   每 255 行内的水平波周期数
 *   1     0x7F        32   12   |  16    12    63.8
 *   2     0x3F         8   16   |  16     4    31.9
 *   3     0x7F        32   16   |   4    32    63.8
 *   4     0x3F         1    4   |   1    16     4.0
 *
 * tableMask 决定取表的哪一半:
 *   0x7F = 整个 128 项周期 (含"负半周"); 0x3F = 只用前 64 项 (0→100→0 的正半拱)。
 *
 * ⚠ 原代码把表当 **u8 无符号**读 (表项 0xFC..0xFF 即负半周 → 252..255), 除数才是 s8
 *   (asm: `lsls #0x18; asrs #0x18` + `bl __divsi3`)。所以 tableMask=0x7F 的 mode 1/3
 *   在负半周会得到一个大正数而非负偏移 (例 mode1: 0,8,0,13 循环), 靠 BG 偏移寄存器回绕
 *   出视觉效果。这是原作者的写法, 不要"顺手修正"成 s8 表 —— 会改变生成字节。
 */
// @ 0x08000C98
void HBlankWave_BuildTables(u16 mode)
{

    u16 i;
    u8 tableMask;
    s8 hStep, vStep;
    s8 hDiv, vDiv;

    tableMask = 0x7F;
    switch (mode)
    {
        case 1:
            hDiv = 12;
            hStep = 32;
            vDiv = 12;
            vStep = 16;
            break;
        case 2:
            hDiv = 16;
            hStep = 8;
            vDiv = 4;
            vStep = 16;
            tableMask = 0x3F;
            break;
        case 3:
            hDiv = 16;
            hStep = 32;
            vDiv = 32;
            vStep = 4;
            break;
        case 4:
            hDiv = 4;
            hStep = 1;
            vDiv = 16;
            vStep = 1;
            tableMask = 0x3F;
            break;
        default:
            return;
    }

    /* 255 = 屏幕可见行数 + 一屏余量, 供 (gHBlankWaveRow + scanline) & 0xFF 索引 */
    for (i = 0; i < 255; i++)
    {
        gHBlankWaveH[i] = gWaveSineTable[(i * hStep) & tableMask] / hDiv;
        gHBlankWaveV[i] = gWaveSineTable[(i * vStep) & tableMask] / vDiv;
    }
}

// @ 0x08000D5C
u32 LZ_UncompressChunk(void)
{
    u8 flag;
    u32 bit_offset;
    u16 lz_token;
    s32 match_length;
    s32 match_offset;
    s32 i;

    if (gLzContext.size > gLzContext.remainingSize)
    {
        gLzContext.size = gLzContext.remainingSize;
    }

    gLzContext.processedSize = 0;

    if (gLzContext.size >= 0)
    {
        while (gLzContext.size > gLzContext.processedSize)
        {
            flag = gLzContext.flags[gLzContext.bitIndex >> 3];
            bit_offset = gLzContext.bitIndex & 7;

            if ((flag >> bit_offset) & 1)
            {
                lz_token = gLzContext.src[0] + (gLzContext.src[1] << 8);
                gLzContext.src += 2;

                match_offset = (lz_token & 0x0FFF) + 1;
                match_length = (lz_token >> 12) + 3;

                for (i = 0; i < match_length; i++)
                {
                    *gLzContext.dest = *(gLzContext.dest - match_offset);
                    gLzContext.dest++;
                    gLzContext.processedSize++;
                }
            }
            else
            {
                *gLzContext.dest++ = *gLzContext.src++;
                gLzContext.processedSize++;
            }

            gLzContext.bitIndex++;
        }
    }

    if (gLzContext.remainingSize > gLzContext.size)
    {
        gLzContext.remainingSize -= gLzContext.processedSize;
        return gLzContext.remainingSize;
    }

    return 0;
}

// @ 0x08000E1C
void Intr_SetMode(u8 arg0)
{

    REG_IME = 0;

    if (arg0 == 0)
    {
        gVBlankPipelineMode = 4;
        DmaCopy16(3, IntrMain, gIntrMainBuf, sizeof(gIntrMainBuf));
        REG_IE = INTR_FLAG_VBLANK | INTR_FLAG_HBLANK | INTR_FLAG_GAMEPAK;
        REG_DISPSTAT = REG_DISPSTAT | 0x10;
    }
    else
    {
        gVBlankPipelineMode = 5;
        DmaCopy16(3, sub_8000170, gIntrMainBuf, sizeof(gIntrMainBuf));
        REG_IE = 0xFFFD & REG_IE;
        REG_DISPSTAT = 0xFFEF & REG_DISPSTAT;
    }

    INTR_VECTOR = &gIntrMainBuf;
    REG_IME = 1;
}

// @ 0x08000ED8
void Display_RestartAfterLoad(void)
{

    while ((REG_VCOUNT & 0xFF) > 0xC8)
        ;

    REG_DISPCNT &= 0xFF7F;
    VBlankIntrWait();

    while (REG_DISPSTAT & DISPSTAT_VBLANK)
        ;

    Sound_VSyncOn();
    Bgm_SetVolume(0);
    VBlankIntrWait();

    while (REG_DISPSTAT & DISPSTAT_VBLANK)
        ;

    SoundMain_Frame();
}

// @ 0x08000F54
void System_ResetToLogo(void)
{
    gVBlankPipelineMode = 0;
    gGameState = GAME_STATE_TITLE_MENU;
    gScenePhase = 0;
    Bgm_Stop();
    VBlankIntrWait();
    SoundMain_Frame();
    Sound_VSyncOff();
    VBlankIntrWait();
    SoundMain_Frame();
}

// @ 0x08000F90
void nullsub_5() { }

// @ 0x08000F94
void VBlankWait_PumpSound(void)
{
    VBlankIntrWait();
    SoundMain_Frame();
}

// @ 0x08000FA4
void VBlankWaitExit_PumpSound(void)
{
    VBlankIntrWait();
    while (REG_DISPSTAT & DISPSTAT_VBLANK)
        ;
    SoundMain_Frame();
}

// @ 0x08000FD0
void LZ_InitContext(u8 *dest, struct Unk_LzData *arg1, u32 arg2)
{
    u8 *ptr;

    gLzContext.unkC = arg1->uncompressedSize;
    gLzContext.remainingSize = gLzContext.unkC;
    gLzContext.dest = dest;
    ptr = arg1->data;
    gLzContext.src = ptr;
    gLzContext.flags = ptr + arg1->size;
    gLzContext.bitIndex = 0;
    gLzContext.size = arg2;
}

// @ 0x08000FF8
u8 Rand_TableNext(void)
{
    return gRandShuffleTable[gRandCursor++];
}

// @ 0x08001014
void EventFlags_ClearAll(void)
{
    u16 i;

    for (i = 0; i < 0x40; i++)
    {
        gEventFlags[i] = 0;
    }
}

// @ 0x08001030
u8 EventFlags_Test(u16 arg0)
{
    return gEventFlags[arg0 >> 3] & (1 << (arg0 & 7));
    // return gEventFlags[arg0 / 8] & (1 << (arg0 % 8));
}

// @ 0x08001050
void EventFlags_Set(u16 arg0)
{
    gEventFlags[arg0 >> 3] |= (1 << (arg0 & 7));
}

// @ 0x08001070
void EventFlags_Reset(u16 arg0)
{
    gEventFlags[arg0 >> 3] &= ~(1 << (arg0 & 7));
}

// @ 0x08001090
void SwitchFlags_ClearAll(void)
{
    u16 i;

    for (i = 0; i < 0x50; i++)
    {
        gSwitchFlags[i] = 0;
    }
}

// @ 0x080010AC
u8 SwitchFlags_Test(u16 arg0)
{
    return gSwitchFlags[arg0 >> 3] & (1 << (arg0 & 7));
}

// @ 0x080010CC
void SwitchFlags_Set(u16 arg0)
{
    gSwitchFlags[arg0 >> 3] |= 1 << (arg0 & 7);
}

// @ 0x080010EC
void SwitchFlags_Reset(u16 arg0)
{
    gSwitchFlags[arg0 >> 3] &= ~(1 << (arg0 & 7));
}

// @ 0x0800110C
void SwitchFlags_ClearRange(void)
{
    u16 i;

    for (i = 0x3D; i < 0x50; i++)
    {
        gSwitchFlags[i] = 0;
    }
}

// 游戏系统初始化与冷启动入口
// 流程概述:
// 1. 复位系统寄存器与 RAM (RegisterRamReset)
// 2. 配置 Waitstate 与 Prefetch (REG_WAITCNT = 0x4014)
// 3. 安装中断处理例程 (IntrMain 与 gIntrTable 拷入 IWRAM)
// 4. 开启中断 (VBlank, HBlank, GamePak)
// 5. 设置初始任务状态:
//    - gMainLoopMode = MAIN_LOOP_GAME (主世界任务分发 Task_DispatchGameState)
//    - gGameState = GAME_STATE_TITLE_MENU (Task_TitleMenuFrame -> TitleMenu_ProcessFrame)
//    - gScenePhase = 0
// 6. 初始化声音引擎 (Sound_Init) 与菜单插槽 (MenuSlot_ResetAll)
// @ 0x08001128
void System_Init(void)
{
    /* BIOS reset flag 3 clears EWRAM and IWRAM while preserving the BIOS-owned
     * top of IWRAM that contains the active stacks and interrupt vector. */
    RegisterRamReset(3);
    REG_WAITCNT = WAITCNT_PREFETCH_ENABLE | WAITCNT_WS0_N_3 | WAITCNT_WS0_S_1;
    gMainLoopMode = MAIN_LOOP_GAME;
    Palette_FillWhite();
    DmaCopy32(3, gIntrTable, gUnk_03001950, sizeof(gUnk_03001950));
    DmaCopy16(3, IntrMain, gIntrMainBuf, sizeof(gIntrMainBuf));

    INTR_VECTOR = gIntrMainBuf;
    gGameState = GAME_STATE_TITLE_MENU;
    gScenePhase = 0;

    REG_IE = INTR_FLAG_VBLANK | INTR_FLAG_HBLANK | INTR_FLAG_GAMEPAK;
    REG_DISPSTAT = DISPSTAT_HBLANK_INTR | DISPSTAT_VBLANK_INTR;
    REG_IME = 1;

    VBlankIntrWait();
    REG_DISPCNT = DISPCNT_MODE_0 | DISPCNT_OBJ_1D_MAP | DISPCNT_FORCED_BLANK | DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_BG2_ON
        | DISPCNT_OBJ_ON | DISPCNT_WIN0_ON;
    ;
    System_SoftReset(0);
    Sound_Init();
    MenuSlot_ResetAll();
    gScenePhase = 0;
}

// @ 0x080011F0
void ReadKeysRaw(void)
{
    u16 keyInput;

    keyInput = ~REG_KEYINPUT;
    gNewKeysRaw = keyInput & ~gHeldKeysRaw;
    gHeldKeysRaw = keyInput;
}

// @ 0x0800121C
void ReadKeys(void)
{
    u16 keyInput;

    keyInput = ~REG_KEYINPUT;
    gNewKeysRaw = keyInput & ~gHeldKeysRaw;
    gHeldKeysRaw = keyInput;
}

// @ 0x08001248
void DummyIntr3() { }

// @ 0x0800124C
void Intr_HandleHBlank(void)
{
    switch (gVBlankPipelineMode)
    {
        case 1:
        case 3:
            HBlankWave_ApplyLineScroll(0xFF & REG_VCOUNT);
            break;
        case 2:
            sub_801887C();
            break;
    }
}

// @ 0x08001284
void DummyIntr4() { }

// @ 0x08001288
void DummyIntr5() { }

// AgbMain - 游戏最顶层执行主入口
//
// 调度模型架构:
// 1. gMainLoopCallbacks 双任务插槽轮转 (gMainLoopMode):
//    - 0: Task_DispatchGameState (常规主世界调度: 地图/剧情/菜单/标题)
//    - 1: BattleTask_Run (独立战斗主循环)
// 2. 开机阶段流转:
//    - System_Init() 选择 MAIN_LOOP_GAME / GAME_STATE_TITLE_MENU
//    - Task_TitleMenuFrame 每帧调用 TitleMenu_ProcessFrame (标题与存档界面状态机)
//    - 玩家选择 "はじめから" (新游戏) -> TitleMenu_ProcessFrame 选择 GAME_STATE_NEW_GAME
//    - 下一帧 Task_DispatchGameState 调用 gGameStateCallbacks[GAME_STATE_NEW_GAME]
//    - NewGame_Init() 构造初始主角/物品并调用 ScriptSet_Load(1, 0, 1) 装载 Burg 村剧情脚本
// @ 0x0800128C
void AgbMain(void)
{
    /* System_Init repeats this assignment after RAM reset. Keeping the first
     * store matches the original boot binary and documents the intended mode. */
    gMainLoopMode = MAIN_LOOP_GAME;
    System_Init();

    while (1)
    {
        gMainLoopCallbacks[gMainLoopMode]();
        VBlankIntrWait();
        SoundMain_Frame();
    }
}

// @ 0x080012B8
void VBlank_UpdateScreenMode5(void)
{
    sub_8016F30();
    m4aSoundVSync();
    VramTransfer_Flush();
    PalTransfer_Flush();
    CpuFastSet(gOamBuffer, (void *)0x07000000, 0x100);

    REG_BG1HOFS = 0;
    REG_BG1VOFS = 0;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;
    REG_BG3HOFS = 0;
    REG_BG3VOFS = 0;
    REG_BLDCNT = gBlendControl;
    REG_BLDALPHA = gBlendCoefficients;

    CpuFastSet(VRAM_BUF_2005800, (void *)0x0600F800, 0x200);
    if (gBgTileReloadFlag != 0)
    {
        CpuFastSet(VRAM_BUF_2005000, (void *)0x0600F000, 0x200);
        gBgTileReloadFlag = 0;
    }
}
