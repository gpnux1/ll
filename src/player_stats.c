#include "code_0.h"
#include "data_805769C.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"
#include "sound.h"

void ChestObject_Open(u8 arg0)
{
    u8 idx;

    if (gChestObjects[arg0].flags & 1)
    {
        Sfx_Play(9, 0, 0);
    }
    else
    {
        Sfx_Play(8, 0, 0);
    }

    gChestObjects[arg0].flags ^= 1;

    idx = gChestObjects[arg0].mapEntryIndex;

    gChestFlags[idx >> 3] ^= (1 << (idx & 7));

    Sprite_FreeChain(&gSpriteNodePool[gChestObjects[arg0].spriteNodeIdx]);
    ChestObject_BuildSprite(arg0);
    gUnk_03004860 = arg0;
}

/* 把“数字 0~9 字形”连同对应的 OBJ 调色板装进去, 供 HUD/菜单直接拼数字用:
 *   gDigitFontObjTiles (0x08088C40) → 0x060112C0  0x140 B = 10 个 4bpp 8×8 图块 = OBJ 图块槽 150~159
 *   gDigitFontObjPalettes (0x08088C00) → 0x050003C0  0x40 B  = 2 组 16 色 OBJ 调色板(槽 14~15)
 * 受 gObjGraphicsSetId 的 bit7 屏蔽; 槽 146~149 是箭头/滚动条字形(见 LoadArrowObjTiles)。
 * ⚠ `0x80 & gObjGraphicsSetId` 的常量在左不能改位置, 否则 GCC2 生成的指令序列会变(规则 5)。 */
// @ 0x08009114
void LoadDigitFontObjTiles(void)
{
    if (!(GFXSET_NO_SPRITE_LOAD & gObjGraphicsSetId))
    {
        nullsub_5();
        DmaCopy16(3, 0x08088C40, 0x060112C0, 0xA0 * 2);
        DmaCopy16(3, 0x08088C00, 0x050003C0, 0x20 * 2);
    }
}
// @ 0x08009168
void ChestFlags_ClearAll()
{
    u8 i = 0;
    // do{gChestFlags[i++] = 0;}while(i < 0x20);
    while (i < 0x20)
    {
        gChestFlags[i++] = 0;
    }
}
// @ 0x08009184
void ChestFlags_Toggle(u8 arg0)
{
    gChestFlags[arg0 >> 3] ^= (1 << (arg0 & 7));
}
// @ 0x080091A4
u8 ChestFlags_Test(u8 arg0)
{
    u8 val;
    val = gChestFlags[(arg0 >> 3) & 0x1F];
    return (val >> (arg0 & 7)) & 1;
}
// @ 0x080091C4
/* 调色板混合特效逐帧驱动 (gSceneBlendMode 三值 switch; SpriteEngine 每帧调):
 * 8  = 相位 0..0x6F 循环, BLDY 摆 7..14..7 (>>3 三角波+7, 低 5 位保留 gBlendCoefficients 原值);
 * 11 = 相位 &6==0 交替 0x0D03/0x0F03 (BLDY 摆 13/15 的闪烁);
 * 17 = 地图 0x6B 且 gViewportFlags[VF_FADE_PHASE]==1 时相位 ++, 0x1F08|(相位>>1 三角波)
 *      (bit4==0 → 0xF&~x, bit4!=0 → x, 波谷/波峰两相; 注意 0x1F08 低段含 BLDY=8);
 * 尾部: gPaletteFxMode==0 时推进 4 项菜单条目调色板动画计数器 (flags bit2=暂停,
 *   计满按 bit1 决定单次停播还是回零循环; 计满瞬间 strh 0 复用 CSE 自 r6)。 */
void PaletteEffects_Update(void)
{
    u16 value;
    s16 i;

    switch (gSceneBlendMode)
    {
    case 8:
        gPaletteFxPhase++;
        if (gPaletteFxPhase > 0x6F)
            gPaletteFxPhase = 0;
        value = gPaletteFxPhase >> 3;
        if (value > 7)
            value = 14 - value;
        value += 7;
        gBlendCoefficients = (gBlendCoefficients & 0xFFE0) | value;
        break;
    case 11:
        gPaletteFxPhase++;
        if ((gPaletteFxPhase & 6) == 0)
            gBlendCoefficients = 0xD03;
        else
            gBlendCoefficients = 0xF03;
        break;
    case 17:
        if (gCurrentMapId == 0x6B && gViewportFlags[VF_FADE_PHASE] == 1)
        {
            gPaletteFxPhase++;
            if ((gPaletteFxPhase & 0x10) == 0)
                gBlendCoefficients = 0x1F08 | (0xF & ~((gPaletteFxPhase >> 1) & 7));
            else
                gBlendCoefficients = 0x1F08 | ((gPaletteFxPhase >> 1) & 7);
        }
        break;
    }

    if (gPaletteFxMode != 0)
    {
        PaletteFx_Transform();
    }
    else
    {
        for (i = 0; i <= 3; i++)
        {
            if (gUnk_03000010[i] != 0 && (gUnk_03000010[i] & 4) == 0)
            {
                gUnk_03000020[i]++;
                if ((gUnk_03000020[i] >> gUnk_03000018[i]) >= gUnk_03000014[i])
                {
                    if ((gUnk_03000010[i] & 2) != 0)
                        gUnk_03000010[i] = 0;
                    else
                        gUnk_03000020[i] = 0;
                }
            }
        }
    }
}

/* 调色板 DMA 上传: 平时整表刷新; 若 gPaletteFxMode 非零则走特效流程 PaletteFx_Step。
 * 逐项: 标志 gUnk_03000010[i] 非零且未设 bit2 → 计算表内偏移:
 *   idx = gUnk_03000020[i] >> gUnk_03000018[i];  byte = gUnk_03000038[i][idx];
 *   src = gMenuEntPaletteFrames + (byte << 5) + 2;  → DMA3 拷贝 32 字节到 gUnk_03000028[i]。 */
extern const u16 gMenuEntPaletteFrames[];

// @ 0x08009370
void PaletteTransfer_Update(void)
{
    s16 i;

    if (gPaletteFxMode != 0)
    {
        PaletteFx_Step();
    }
    else
    {
        PalTransfer_Flush();
        for (i = 0; i <= 3; i++)
        {
            if (gUnk_03000010[i] != 0 && (gUnk_03000010[i] & 4) == 0)
            {
                u32 src;
                u32 off;
                u8 *base;

                off = ((u32)(*(u8 *)(gUnk_03000038[i] + (gUnk_03000020[i] >> gUnk_03000018[i]))) << 5) + 2;
                base = (u8 *)gMenuEntPaletteFrames;
                src = (u32)(base + off);
                DmaSet(3, src, gUnk_03000028[i], 0x80000010);
            }
        }
    }
}

/* 调色板特效状态复位 (调色板暂存 0x0203E600, 备份 0x0203EA00):
 * mode: 1/7=白闪(开 WIN0 窗口), 3=黑闪, 5=备份当前调色板, 其他=从备份恢复。
 * gPaletteFxMode = mode+1 (供其他特效读取), gPaletteFxPending/gPaletteFxTimer
 * 分别是待上传标志和帧计数器。 */

// @ 0x08009428
void PaletteFx_Apply(u8 arg0)
{
    u16 i;
    u16 fill;
    u16 *dst;

    gPaletteFxPending = 0;
    gPaletteFxMode = arg0 + 1;
    gPaletteFxTimer = 0;

    switch (gPaletteFxMode)
    {
        case 1:
        case 7:
            fill = 0x7FFF;
            REG_WIN0H = 0xF0FF;
            REG_WIN0V = 0x00A0;
            REG_DISPCNT |= DISPCNT_WIN0_ON;
            REG_WININ = 0;
            REG_WINOUT = 0;
        case 3:
            if (gPaletteFxMode == 3)
                fill = 0;

            i = 0x200;
            dst = (u16 *)0x0203E600;
            while (i > 0)
            {
                *dst++ = fill;
                i--;
            }

            DmaCopy16(3, (void *)0x0203E600, PLTT, PLTT_SIZE);
            break;

        case 5:
            Palette_Backup();
            DmaCopy16(3, (void *)0x0203EA00, (void *)0x0203E600, PLTT_SIZE);
            break;

        default:
            DmaCopy16(3, (void *)0x0203EA00, (void *)0x0203E600, PLTT_SIZE);
            break;
    }
}

/* 调色板特效逐帧驱动 (PaletteFx_Apply 之后每帧调用):
 * 若 gPaletteFxPending 置位 → 按 gPaletteFxTimer & 3 选中 4 个调色板暂存区之一,
 *   DMA3 拷贝 0x80 半字到调色板 RAM 对应 0x100 字节段 (0x05000000 + idx*0x100)。
 * 之后清标志、计数器 +1。mode==2/7 (白闪) 且计数器超过阈值 (0x40/0x20) 时
 *   重新断言 WIN0 窗口, 并复位 gPaletteFxMode/gScreenTransitionState。
 * 计数器到 4 时把窗口完全打开 (WIN0V=0x100, WININ/WINOUT=0x3F)。
 * 注: 两个分支内的 u8 局部读取 gPaletteFxMode 是**故意**的 —— 让 GCC2 不跨分支
 *   CSE 该读, 使计数器 c 落 r1、state 落 r0 并重读, 与目标逐字节一致。 */
// @ 0x080094FC
void PaletteFx_Step(void)
{
    u8 counter;
    u32 src;
    u32 c;

    if (gPaletteFxPending == 0)
        return;

    counter = gPaletteFxTimer & 3;
    switch (counter)
    {
        case 0:
            src = 0x0203E600;
            break;
        case 1:
            src = 0x0203E700;
            break;
        case 2:
            src = 0x0203E800;
            break;
        case 3:
            src = 0x0203E900;
            break;
        default:
            break;
    }

    DmaSet(3, src, 0x05000000 + (counter << 8), 0x80000080);

    gPaletteFxPending = 0;
    c = gPaletteFxTimer + 1;
    gPaletteFxTimer = c;

    if (gPaletteFxMode > 6)
    {
        if ((u8)c > 0x20)
        {
            u8 s2 = gPaletteFxMode;
            if (s2 == 7)
            {
                REG_WIN0H = 0xF0;
                REG_WIN0V = 0xA0;
                REG_DISPCNT |= DISPCNT_WIN0_ON;
                REG_WININ = 0;
                REG_WINOUT = 0;
            }
            gPaletteFxMode = 0;
            gScreenTransitionState = 0;
        }
    }
    else if ((u8)c > 0x40)
    {
        u8 s3 = gPaletteFxMode;
        if (s3 == 2)
        {
            REG_WIN0H = 0xF0;
            REG_WIN0V = 0xA0;
            REG_DISPCNT |= DISPCNT_WIN0_ON;
            REG_WININ = 0;
            REG_WINOUT = 0;
        }
        gPaletteFxMode = 0;
        gScreenTransitionState = 0;
    }

    if (gPaletteFxTimer == 4)
    {
        REG_WIN0H = 0xF0;
        REG_WIN0V = 0x100;
        REG_WININ = 0x3F;
        REG_WINOUT = 0x3F;
    }
}

// @ 0x08009600
INCLUDE_ASM("asm/nonmatchings", PaletteFx_Transform);
// @ 0x08009A5C
void MenuEnt_ClearStates(void)
{
    s16 i;

    for (i = 0; i < 4; i++)
    {
        gUnk_03000010[i] = 0;
    }
}
/* 选项/菜单条目描述表 (0x087EA138): 每项指向 {u8 count; count 条变长记录}, 由 MenuEnt_ParseDesc 逐条解析 */
extern u8 *gUnk_087EA138[];

/* 解析 gUnk_087EA138[arg0-1] 组的全部条目 (条目 id 从 0 起, 写 0x02005000 区窗口) */
// @ 0x08009A7C
void MenuEnt_ParseAll(u8 arg0)
{
    u8 count;
    s16 i;
    u8 *src;

    if (arg0 == 0)
        return;

    src = gUnk_087EA138[arg0 - 1];
    count = *src++;

    i = 0;
    while (count != 0)
    {
        src = MenuEnt_ParseDesc(i, src);

        count--;
        i++;
    }
}
/* 解析 gUnk_087EA138[arg1] 组的条目 id [arg0, arg0+count) 区间 */
// @ 0x08009AC4
void MenuEnt_ParseRange(u8 arg0, u8 arg1)
{
    u8 end;
    u8 cur;
    u8 *src = gUnk_087EA138[arg1];

    end = arg0 + *src++;
    cur = arg0;
    while (cur < end)
    {
        src = MenuEnt_ParseDesc(cur, src);
        cur++;
    }
}
// @ 0x08009B04
void MenuEnt_Unlock(u8 arg0)
{
    gUnk_03000010[arg0] &= 0xFB;
}
// @ 0x08009B1C
void MenuEnt_Lock(u8 arg0)
{
    gUnk_03000010[arg0] |= 4;
}

// @ 0x08009B34
u8 MenuEnt_GetState(u8 arg0)
{
    return gUnk_03000010[arg0];
}

// @ 0x08009B44
void Palette_Backup(void)
{
    DmaCopy16(3, PLTT, 0x0203EA00, PLTT_SIZE);
}

// @ 0x08009B64
void Palette_FillWhite(void)
{
    u16 i;
    u16 *dst;
    i = 0x200;
    dst = (u16 *)0x05000000;

    while (i > 0)
    {
        *dst = 0x7FFF;
        dst++;
        i--;
    }
}

// @ 0x08009B84
u8 *MenuEnt_ParseDesc(u8 arg0, u8 *src)
{
    gUnk_03000010[arg0] = src[0];
    gUnk_03000018[arg0] = src[1];
    src += 2;
    gUnk_03000028[arg0] = (*src << 5) + 0x05000002;
    src++;
    gUnk_03000014[arg0] = *src;
    src++;
    gUnk_03000038[arg0] = src;
    src += gUnk_03000014[arg0];
    gUnk_03000020[arg0] = 0;
    return src;
}
/* 静态地图物件图形槽装载: gUnk_0808EA0C 每组 8 字节 = 2 条 4 字节记录
 * {gfxIdx, palIdx, gfxSlot, palSlot}; 0xFF 记录跳过。
 * gfx: LZ77 → 0x06010000 + gfxSlot*0x20; pal: 0x080BABA0[palIdx] → OBJ PLTT 0x05000380+palSlot*0x20。
 * 槽号存入 gUnk_0300496C/gUnk_03004970 (StaticObjs_Spawn 用)。 */
extern u8 gUnk_0808EA0C[][8];
extern u8 gUnk_0300496C[];
extern u8 gUnk_03004970[];

extern u8 *gUnk_087EA33C[];

extern u16 gUnk_080BABA0[][16];

extern u16 *gUnk_087EA38C[];
extern u8 *gUnk_087EA368[];

// @ 0x08009BF0
void StaticObjGfx_LoadPair(u8 arg0)
{
    u8 *var_r4;
    u8 var_r5;

    if (arg0 != 0)
    {
        arg0--;
        var_r4 = &gUnk_0808EA0C[arg0][0];

        for (var_r5 = 0; var_r5 < 2; var_r5++)
        {
            if (var_r4[0] != 0xFF)
            {
                gUnk_0300496C[var_r5] = var_r4[2];
                gUnk_03004970[var_r5] = var_r4[3];
                LZ77UnCompVram(gUnk_087EA33C[var_r4[0]], (u16 *)0x06010000 + (var_r4[2] << 4));

                DmaCopy16(3, (u16 *)gUnk_080BABA0[var_r4[1]], (u16 *)0x05000380 + (var_r5 << 4), 0x20);
            }
            var_r4 += 4;
        }
    }
}

/* 静态地图物件生成: 从 gUnk_087EA38C[arg0-1] 指向的描述块读
 * {u16 count} + count × {u8 gfxSlot, u8 高字节, u16 x, u16 y, u16 z},
 * 填入 gStaticMapObjects[0..count-1]: field_C = 描述半字高字节, field_2/field_3(pal) = 图形槽 idx/+12,
 * dataPtr = gUnk_087EA368[palSlot] 动画描述, field_0=1 激活, 其余清零。 */
// @ 0x08009C84
void StaticObjs_Spawn(u8 arg0)
{
    u16 var_r5;
    u8 idx;
    StaticMapObject *staticObj;
    u16 *src;

    if (arg0 != 0)
    {
        arg0--;
        staticObj = gStaticMapObjects;
        src = gUnk_087EA38C[arg0];
        var_r5 = *src;
        src++;

        while (var_r5 != 0)
        {
            idx = *src;

            staticObj->field_C = *src & 0xFF00;
            src++;
            staticObj->x = *src;
            src++;
            staticObj->y = *src;
            src++;
            staticObj->z = *src;
            src++;
            staticObj->field_2 = gUnk_0300496C[idx];
            staticObj->field_3 = idx + 12;

            staticObj->dataPtr = gUnk_087EA368[gUnk_03004970[idx]];

            staticObj->field_0 = 1;
            staticObj->animTimer = 0;
            staticObj->field_1 = 0;
            staticObj->field_E = 0;

            var_r5--;
            staticObj++;
        }
    }
}

/* ⚠ code_0.h 里的 u8 返回类型会生成 lsls/cmp 截断; 目标是直接 cmp。
 * 本 C 文件 内用宏把调用改指到 s32 原型的本地别名 (同一 ROM 符号, 链接期同一地址)。 */
s32 Sprite_EnqueueRender_S32(u16, u16, u8, u16, u8);

/* 静态地图物件逐帧动画: 遍历 gStaticMapObjects[3] (field_0=激活):
 * animTimer != 0xFF 时按 dataPtr 动画描述换帧 (描述: {u16 首帧偏移, u16 帧时长, u16 帧数, u16 loop 偏移, ...});
 * 每 4 字节一条帧记录, 到点后通过 Sprite_InitChainNode 链重建精灵段 (StaticObj_BuildChain)。
 * 最后用 Sprite_EnqueueRender(x, y, sprNodeIdx, z, 3) 入渲染队列, 返回值写 field_E bit3 (被遮挡)。
 * gUnk_03004D4C != 0 (菜单打开) 时整帧跳过。 */
void StaticObj_BuildChain(u8, u8 *);

// @ 0x08009D34
void StaticObjs_StepAll()
{
    s16 var_ip;
    StaticMapObject *var_r7;
    s16 x;
    u8 *src;
    u8 *arr;
    u8 *framePtr;
    u16 maxDuration;
    u16 loopCount;
    struct SpriteNode *renderObj;
    struct SpriteNode *nextRenderObj;

    if (gUnk_03004D4C != 0)
    {
        return;
    }
    var_r7 = gStaticMapObjects;

    for (var_ip = 0; var_ip < 3; var_ip++)
    {
        if (var_r7->field_0 != 0)
        {
            src = var_r7->dataPtr;
            if (var_r7->animTimer != 0xFF)
            {
                framePtr = src + (src[2] + (src[3] << 8));
                if (var_r7->animTimer == 0)
                {
                    framePtr = src + src[framePtr[8] * 2 + 8] + 4;
                    StaticObj_BuildChain(var_ip, framePtr);
                    var_r7->animTimer++;
                }
                else
                {
                    if ((var_r7->field_E & 8) == 0)
                    {
                        maxDuration = (framePtr[2] + (framePtr[3] << 8));
                        loopCount = (framePtr[6] + (framePtr[7] << 8));
                        framePtr = framePtr + 8;
                        if (var_r7->animTimer >= maxDuration)
                        {
                            var_r7->animTimer = 0;
                        }
                        while (loopCount != 0)
                        {
                            if (var_r7->animTimer == (framePtr[2] + (framePtr[3] << 8)) || var_r7->animTimer == 0)
                            {
                                renderObj = &gSpriteNodePool[var_r7->field_1];
                                renderObj->flags = 0;
                                nextRenderObj = renderObj->next;
                                while (nextRenderObj != 0)
                                {
                                    renderObj->next = 0;
                                    renderObj = nextRenderObj;
                                    renderObj->flags = 0;
                                    nextRenderObj = renderObj->next;
                                }

                                if (var_r7->animTimer == 0)
                                {
                                    arr = src + src[framePtr[0] * 2 + 8] + 4;
                                }
                                else
                                {
                                    arr = src + src[framePtr[4] * 2 + 8] + 4;
                                }

                                StaticObj_BuildChain(var_ip, arr);
                                break;
                            }
                            framePtr += 4;
                            loopCount--;
                        }
                        if (var_r7->animTimer != 0xFF)
                        {
                            var_r7->animTimer++;
                        }
                    }
                }
            }

            if (Sprite_EnqueueRender_S32(var_r7->x, var_r7->y, var_r7->field_1, var_r7->z, 3) != 0)
            {
                var_r7->field_E |= 8;
            }
            else
            {
                var_r7->field_E &= ~8;
            }
        }

        var_r7++;
    }
}

/* 静态地图物件精灵段重建 (每帧动画换帧时调用):
 * var_r6 描述 = {u8 部件数相关偏移, u8, u8 部件数, u8, 之后 部件数 × 6 字节
 *   {u8 y, u8 高字节, u8 x, u8 高字节, u8 tileOffset, u8}}。
 * y 高字节拼进 attr0 (0x80<<7=0x4000 翻转位由 Sprite_InitChainNode 处理),
 * tileId 从 OBJ VRAM 基址 + tileOffset + 槽 field_2 选块, paletteId = field_3。 */
// @ 0x08009E80
void StaticObj_BuildChain(u8 arg0, u8 *var_r6)
{
    struct SpriteNode *subRenderObj;
    struct SpriteNode *renderObj;
    StaticMapObject *staticObj;
    u8 temp_r4;

    u16 temp_r3;
    u8 temp_sl;
    u8 var_r8;

    u8 r6_4;
    u8 r6_0;
    u16 r6_1;

    staticObj = &gStaticMapObjects[arg0];
    temp_r4 = var_r6[0] * 6 + 4;
    var_r8 = var_r6[2];

    staticObj->field_1 = Sprite_AllocNode();

    renderObj = &gSpriteNodePool[staticObj->field_1];

    temp_sl = staticObj->field_2;
    var_r6 = var_r6 + temp_r4;

    while (var_r8 != 0)
    {
        r6_4 = var_r6[4];
        r6_0 = var_r6[0];
        r6_1 = var_r6[1] << 8;
        temp_r3 = var_r6[2] | (var_r6[3] << 8);
        renderObj->tileOffsetX = temp_r3 & 0x1FF;
        r6_1 |= r6_0;
        renderObj->tileOffsetY = r6_0;

        subRenderObj = Sprite_InitChainNode(renderObj, var_r8, r6_1, temp_r3,
                                            ((staticObj->field_3 << 0xC) | staticObj->field_C | (r6_4 + temp_sl)));
        if (renderObj->tileOffsetX > 0xFFU)
        {
            renderObj->tileOffsetX--;
        }
        renderObj = subRenderObj;

        var_r6 += 6;

        var_r8--;
    }
}

// @ 0x08009F48
void StaticObjs_Reset(void)
{
    s16 i;

    for (i = 0; i < 3; i++)
    {
        gStaticMapObjects[i].field_0 = 0;
    }
}
/* 0x080921F0: 9 行 × 8 字节 — 职业(formation)× 八维属性 → 成长曲线号 (t, 0-40) */
extern const u8 gClassStatCurveTable[];
/* 0x080923D8: 按曲线号 t 划分的 100B/段成长表, 每段 [0]=段长, [1..] 逐级增量 */
extern const u8 gStatGrowthCurveTables[];

/* 属性成长查询: 按 (职业 classId, 等级 lv, 属性序号 statIdx) 返回该等级属性值。
 * - statIdx >= 8 && classId <= 10  → 10
 * - classId > 8: 特殊职业的固定值 (999 / 0 / 1 / 3 / 0xFF)
 * - 常规: t = gClassStatCurveTable[classId*8 + statIdx]; 累加
 *   gStatGrowthCurveTables[t*100 + 0..lv] (level+1 项), 全 0 且 statIdx==7 → 1
 * 调用点: sub_800A3C8 (队伍角色逐属性), sub_8048818 (战斗对象 formation)
 * 代码生成要点: ① switch(a) 每个 case 独立写 `return 10;`(合并成 case0..10 会被
 *   折叠成范围测试, 丢失跳表); ② `stride = t*100` 提前命名变量 (规则 30 分步形式);
 *   ③ while 循环比 do-while 更能复现 `cmp r2,r1; bhi` 入口守卫; ④ 首格条件必须
 *   写 `c >= 8` (u8 归一化成 `cmp r4,#7; bls`);
 *   ⑤ 定义必须用 K&R 旧式风格 (与头文件 `u16 sub_8009F70();` 空形参声明配套):
 *      改成全原型会触发 GCC2 的 default-promotion 冲突报错, 且会让已匹配的
 *      调用方 sub_8048818 的 formation 寄存器分配从 r2 漂到 r3 (规则 7 的坑) */
// @ 0x08009F70
u16 sub_8009F70(classId, lv, statIdx)
u8 classId;
u8 lv;
u8 statIdx;
{
    u8 t;
    u16 sum;
    u16 i;
    u16 stride;

    if (statIdx >= 8 && classId <= 10)
    {
        switch (classId)
        {
            case 0:
                return 10;
            case 1:
                return 10;
            case 2:
                return 10;
            case 3:
                return 10;
            case 4:
                return 10;
            case 5:
                return 10;
            case 6:
                return 10;
            case 7:
                return 10;
            case 8:
                return 10;
            case 9:
                return 10;
            case 10:
                return 10;
        }
    }
    if (classId > 8)
    {
        switch (statIdx)
        {
            case 0:
                return 0x3E7;
            case 1:
                if (classId == 9)
                    return 0x3E7;
                return 0;
            case 7:
                if (classId == 9)
                    return 1;
                return 3;
        }
        return 0xFF;
    }

    t = gClassStatCurveTable[classId * 8 + statIdx];
    stride = t * 100;
    sum = 0;
    i = 0;
    while (i <= lv)
    {
        sum = (u16)(sum + gStatGrowthCurveTables[stride + i]);
        i = (u16)(i + 1);
    }
    if (sum == 0 && statIdx == 7)
        sum = 1;
    return sum;
}
/* 0x08093418: 48 项 × 5 字节的表 (在 data/data.s 的 blob 里, 尚无独立符号) —— **名字未定**:
 *   [1] 高 4 位 = 分组号 (实测分布 0:8 2:6 3:8 4:8 5:8 6:4 7:2 15:4 项), 低 4 位含义未定
 *   [4] = 一个数值 (`ItemGetValue` 直接返回它)
 *   [0] 有两种呸此矛盾的读法, 未定谁对:
 *     - 本函数: `[0] <= lv + 1`  →  像"所需等级"; 0xFF = 特殊值
 *     - ItemFindSlot: `[0] == arg0 + 1`  →  像"物品/技能 id"
 *   写入的是 **表下标+1** (所以 skills[] 存的是行号, 0xFF = 空)
 * → 等 `ItemFindSlot` / `ItemGetValue` 的调用方语义查清后再统一改名。*/

/* 从 gUnk_08093418 筛出满足条件的行, 把 **行号+1** 填进 PlayerStats.skills[8],
 * 不足 8 个用 0xFF 补齐。
 *
 * 入选条件: [1] 高 nibble == groupId (groupId<=1 归为 0), 并且
 *   [0] == 0xFF  →  仅当 gPartyMemberIds[0] == 1
 *   [0] != 0xFF  →  [0] <= lv + 1
 *
 * 调用点: Chara_ClearTempStatus / sub_800A1B4 / sub_800A3C8 / sub_80457AC (共 4 处)
 *
 * 代码生成要点 (已逐字节验证):
 *   - `lv + 1` 必须写成对 u8 形参的算术, GCC2 会归一成 `(lv<<24) + (1<<24)` 再 `>>24`
 *     (与直接 `adds #1` 不同, 目标就是后者)
 *   - 两个分支都写 `flag = 1` 而不是 `goto`: GCC2 的 jump-threading 会自动把
 *     "[0]==0xFF 且 pid==1" 那条路直接跳到接受块 (ROM 就是 `b accept` 绕过 `cmp flag,#0`)
 *   - 本函数使用 r8/sb → 有 GCC2 跳函数泄漏风险 (规则 51), 合入后必须看全量 SHA1
 */
// @ 0x0800A048
void Stats_BuildSkillList(u8 *skills, u8 lv, u8 groupId)
{
    u8 i;
    u8 count;
    u8 flag;
    u8 lvLimit;

    lvLimit = lv + 1;
    if (groupId <= 1)
        groupId = 0;
    count = 0;
    for (i = 0; i <= 0x2F; i++)
    {
        if (groupId != (gUnk_08093418[i * 5 + 1] >> 4))
            continue;
        flag = 0;
        if (gUnk_08093418[i * 5] == 0xFF)
        {
            if (gPartyMemberIds[0] == 1)
                flag = 1;
        }
        else if (gUnk_08093418[i * 5] <= lvLimit)
            flag = 1;
        if (flag == 0)
            continue;
        *skills++ = i + 1;
        count++;
    }
    while (count <= 7)
    {
        *skills++ = 0xFF;
        count++;
    }
}
// @ 0x0800A0E4
u8 Chara_GetFormGfx(u8 arg0)
{
    u8 var_r2;
    PlayerStats *ptr;

    if (arg0 != 0)
    {
        arg0 -= 1;
    }

    ptr = &gPartyStats[arg0];
    switch (arg0)
    {
        case 0:
            if (ptr->equip_slot1 == 0x15)
            {
                if (ptr->equip_slot5 == 0xCB || ptr->equip_slot6 == 0xCB)
                {
                    return 0x38;
                }
            }
            return 0x31;
        case 1:
            return 0x32;
        case 2:
            if (ptr->equip_slot5 == 0xBF || ptr->equip_slot6 == 0xBF)
            {
                return 0x39;
            }
            return 0x33;
        case 3:

            if (ptr->equip_slot5 == 0xBF || ptr->equip_slot6 == 0xBF)
            {
                return 0x3A;
            }
            return 0x34;
        case 4:
            if (ptr->equip_slot1 == 0x3B)
            {
                return 0x3B;
            }
            return 0x35;
        case 5:
            if (ptr->equip_slot1 == 0x3C)
            {
                return 0x3C;
            }
            return 0x36;
        default:
            return 0xFF;
    }
}
// @ 0x0800A1B4
void sub_800A1B4(u8 charId)
{
    PlayerStats *st;
    u8 idx;
    u8 i;
    u8 j;
    u32 sum;

    idx = 0;
    if (charId != 0)
        idx = charId - 1;
    st = &gPartyStats[idx];

    if ((u8)(charId - 9) < 2)
    {
        st->lv = 0x62;
        sum = 0;
        i = 0;
        do
        {
            sum += gLevelUpExpTable[i];
            i++;
        } while (i <= 0x61);
        st->exp = sum;
    }
    else
    {
        st->lv = 0;
        st->exp = 0;
    }
    st->field_unk[1] = 0;
    st->field_unk[5] = 0;
    st->equip_slot1 = gStatGrowthTail[idx * 6 + 0];
    st->equip_slot2 = gStatGrowthTail[idx * 6 + 1];
    st->equip_slot3 = gStatGrowthTail[idx * 6 + 2];
    st->equip_slot4 = gStatGrowthTail[idx * 6 + 3];
    st->equip_slot5 = gStatGrowthTail[idx * 6 + 4];
    st->equip_slot6 = gStatGrowthTail[idx * 6 + 5];
    i = st->lv;
    sum = 0;
    j = 0;
    if (sum <= i)
    {
        do
        {
            sum += gLevelUpExpTable[j];
            j++;
        } while (j <= i);
    }
    st->next_exp = sum;
    st->max_hp = sub_8009F70(charId, st->lv, 0);
    st->max_mp = sub_8009F70(charId, st->lv, 1);
    st->base_atc = sub_8009F70(charId, st->lv, 2);
    st->base_def = sub_8009F70(charId, st->lv, 3);
    st->base_agl = sub_8009F70(charId, st->lv, 4);
    st->base_men = sub_8009F70(charId, st->lv, 5);
    st->base_res = sub_8009F70(charId, st->lv, 6);
    st->base_noa = sub_8009F70(charId, st->lv, 7);
    st->base_luc = sub_8009F70(charId, st->lv, 8);
    st->hp = st->max_hp;
    st->mp = st->max_mp;
    st->atc = st->base_atc;
    st->def = st->base_def;
    st->agl = st->base_agl;
    st->men = st->base_men;
    st->res = st->base_res;
    st->noa = st->base_noa;
    st->luc = st->base_luc;
    Stats_BuildSkillList(st->skills, st->lv, charId);
    st->field_unk[0] = Chara_GetFormGfx(charId);
    st->field_unk[2] = 0;
    st->field_unk[3] = 0;
}
/* 角色等级设定 (脚本 opcode Op_SetCharacterLevel 的实现)。
 *   lvParam: 目标等级 (0 = 同步为队长 gPartyStats[0].lv; 否则 = lvParam-1)
 *   charaId: 队伍角色 ID (1-based, 0 = 队长)
 * 若新等级 > 当前等级, 则: 重算经验 (gLevelUpExpTable 累加)、逐属性
 *   (sub_8009F70 九维)、复制到当前值、重建技能列表 (Stats_BuildSkillList)、
 *   图形 (Chara_GetFormGfx)、装备加成 (Stats_RebuildEquipBonuses/Stats_RecalcEquip)。
 * 代码生成要点 (逐字节验证):
 *   - slot 是 u8 局部, 经 (u8)(charaId-1) 截断后才 <<6 索引 gPartyStats
 *   - lv 由 if/else 合并; lv 的 live range 不可跨越 sub_8009F70 调用
 *     (否则 global-alloc 会把 lv 推到 r3 而非 r0, 级联 36 处寄存器漂移)。
 *     解法: loop2 用独立变量 curLv 重载 st->lv, 拆断 lv 的 web。
 *   - loop1 的上界用独立 u8 bound = lv - 1 (强制 u8 截断, 经验 88) */
// @ 0x0800A3C8
void sub_800A3C8(u8 lvParam, u8 charaId)
{
    PlayerStats *st;
    u8 lv;
    u8 slot;
    u8 i;
    u8 j;
    u32 exp;
    u32 nextExp;
    u8 curLv;
    u8 bound;

    slot = charaId;
    if (charaId != 0)
        slot = charaId - 1;
    st = &gPartyStats[slot];
    if (lvParam == 0)
        lv = gPartyStats[0].lv;
    else
        lv = lvParam - 1;

    if (lv > st->lv)
    {
        st->lv = lv;
        bound = lv - 1;
        exp = 0;
        for (i = 0; i <= bound; i++)
            exp += gLevelUpExpTable[i];
        st->exp = exp;

        curLv = st->lv;
        nextExp = 0;
        for (j = 0; j <= curLv; j++)
            nextExp += gLevelUpExpTable[j];
        st->next_exp = nextExp;

        st->max_hp = sub_8009F70(charaId, st->lv, 0);
        st->max_mp = sub_8009F70(charaId, st->lv, 1);
        st->base_atc = sub_8009F70(charaId, st->lv, 2);
        st->base_def = sub_8009F70(charaId, st->lv, 3);
        st->base_agl = sub_8009F70(charaId, st->lv, 4);
        st->base_men = sub_8009F70(charaId, st->lv, 5);
        st->base_res = sub_8009F70(charaId, st->lv, 6);
        st->base_noa = sub_8009F70(charaId, st->lv, 7);
        st->base_luc = sub_8009F70(charaId, st->lv, 8);

        st->hp = st->max_hp;
        st->mp = st->max_mp;
        st->atc = st->base_atc;
        st->def = st->base_def;
        st->agl = st->base_agl;
        st->men = st->base_men;
        st->res = st->base_res;
        st->noa = st->base_noa;
        st->luc = st->base_luc;

        Stats_BuildSkillList(st->skills, st->lv, charaId);
        st->field_unk[0] = Chara_GetFormGfx(charaId);
        st->field_unk[2] = 0;
        st->field_unk[3] = 0;
        Stats_RebuildEquipBonuses(charaId);
        Stats_RecalcEquip(charaId);
    }
}

extern u8 gUnk_087EA580[];

/* 装备加成结算 (按敌人/角色数据表 gUnk_087EA580 的 12B 条目):
 *  arg0 = 角色/敌人 ID (0 直接返回)。
 *  表项 +8 的防御字节: 低 4 位 -1 选一个装备加成栏 += 表项 +6 (HP);
 *                     高 4 位 -1 选一个装备加成栏 += 表项 +7 (攻击)。
 *  7 个加成栏映射: 0=AtkBase 1=Def2 2=Agl 3=Men 4=Res 5=Noa 6=Luc。
 *  最后 ID 落在 [0x22,0x2B] 或 [0x37,0x3E] 时 Noa 额外 +1。
 * 匹配要点: `v = entry[8] & 0xF; if (v - 1 <= 6) switch (v - 1)` 的写法让
 *  val 装载落在 ands 与 subs 之间。第一分支的 `u8 bonusVal = entry[6]` 重读被
 *  CSE 合并成 val 的副本 (不增指令), 作用是缩短 val 的伪寄存器生命周期, 使
 *  其全局分配优先级高于 entry 基址 → val 落 r2/基址落 r3 (规则 112)。 */
// @ 0x0800A534
void sub_800A534(u8 arg0)
{
    u8 *entry;
    u8 *dst;
    u8 val;
    u32 v;

    if (arg0 == 0)
        return;

    entry = &gUnk_087EA580[arg0 * 12];

    v = entry[8] & 0xF;
    val = entry[6];
    if (v - 1 <= 6)
    {
        u8 bonusVal = entry[6];
        switch (v - 1)
        {
            case 0:
                dst = &gEquipBonusAtkBase;
                break;
            case 1:
                dst = &gEquipBonusDef2;
                break;
            case 2:
                dst = &gEquipBonusAgl;
                break;
            case 3:
                dst = &gEquipBonusMen;
                break;
            case 4:
                dst = &gEquipBonusRes;
                break;
            case 5:
                dst = &gEquipBonusNoa;
                break;
            case 6:
                dst = &gEquipBonusLuc;
                break;
        }
        *dst += bonusVal;
    }

    v = entry[8] >> 4;
    val = entry[7];
    if (v - 1 <= 6)
    {
        switch (v - 1)
        {
            case 0:
                dst = &gEquipBonusAtkBase;
                break;
            case 1:
                dst = &gEquipBonusDef2;
                break;
            case 2:
                dst = &gEquipBonusAgl;
                break;
            case 3:
                dst = &gEquipBonusMen;
                break;
            case 4:
                dst = &gEquipBonusRes;
                break;
            case 5:
                dst = &gEquipBonusNoa;
                break;
            case 6:
                dst = &gEquipBonusLuc;
                break;
        }
        *dst += val;
    }

    if ((u8)(arg0 - 0x22) <= 9 || (u8)(arg0 - 0x37) <= 7)
        gEquipBonusNoa += 1;
}
// @ 0x0800A664
void Stats_RebuildEquipBonuses(u8 arg0)
{
    PlayerStats *ptr;
    u8 id1;
    u8 id2;
    u8 id3;
    u8 id4;
    u8 *entry1;
    u8 *entry2;
    u8 *entry3;
    u8 *entry4;
    u8 val;

    if (arg0 != 0)
        arg0--;
    ptr = &gPartyStats[arg0];

    gEquipBonusAtkBase = 0;
    gEquipBonusDef2 = 0;
    gEquipBonusAgl = 0;
    gEquipBonusMen = 0;
    gEquipBonusRes = 0;
    gEquipBonusNoa = 0;
    gEquipBonusLuc = 0;
    gEquipBonusAtk = 0;
    gEquipBonusDef = 0;

    sub_800A534(ptr->equip_slot1);
    sub_800A534(ptr->equip_slot2);
    sub_800A534(ptr->equip_slot3);
    sub_800A534(ptr->equip_slot4);
    sub_800A534(ptr->equip_slot5);
    sub_800A534(ptr->equip_slot6);

    id1 = ptr->equip_slot1;
    id2 = ptr->equip_slot2;
    id3 = ptr->equip_slot3;
    id4 = ptr->equip_slot4;
    entry1 = &gUnk_087EA580[id1 * 12];
    entry2 = &gUnk_087EA580[id2 * 12];
    entry3 = &gUnk_087EA580[id3 * 12];
    entry4 = &gUnk_087EA580[id4 * 12];

    val = entry1[4] & 0xF0;
    if (val == (entry2[4] & 0xF0) && val == (entry3[4] & 0xF0) && val == (entry4[4] & 0xF0))
    {
        switch (val >> 4)
        {
            case 0xF:
                gEquipBonusAtk = 0x22;
                gEquipBonusDef = 0x2D;
                break;
            case 0xE:
                gEquipBonusAtk = 0x3C;
                gEquipBonusDef = 0x3F;
                break;
        }
    }
}

// @ 0x0800A79C
void Stats_RecalcEquip(u8 arg0)
{
    PlayerStats *chara;

    if (arg0 != 0)
        arg0--;
    chara = &gPartyStats[arg0];

    chara->equip_atc = gEquipBonusAtkBase;
    chara->equip_def = gEquipBonusDef2;
    chara->equip_agl = gEquipBonusAgl;
    chara->equip_men = gEquipBonusMen;
    chara->equip_res = gEquipBonusRes;
    chara->equip_noa = gEquipBonusNoa;
    chara->equip_luc = gEquipBonusLuc;

    chara->equip_atc += gEquipBonusAtk;
    chara->equip_def += gEquipBonusDef;

    chara->atc = chara->base_atc + chara->equip_atc;
    chara->def = chara->base_def + chara->equip_def;
    chara->agl = chara->base_agl + chara->equip_agl;
    chara->men = chara->base_men + chara->equip_men;
    chara->res = chara->base_res + chara->equip_res;
    chara->noa = chara->base_noa + chara->equip_noa;
    chara->luc = chara->base_luc + chara->equip_luc;
}
extern u32 gUnk_08092248[];

// @ 0x0800A86C
u8 ExpToLevel(s32 value)
{
    s32 ret;
    u8 i;

    if (value > 9999999)
    {
        value = 9999999;
    }

    i = 0;
    while (value >= 0)
    {
        value -= gUnk_08092248[i];
        i++;
    }

    return i - 1;
}
// @ 0x0800A8A0
u32 LevelToExp(u8 arg0)
{
    u8 i;
    u32 sum = 0;

    for (i = 0; i <= arg0; i++)
    {
        sum += gUnk_08092248[i];
    }
    return sum;
}


// @ 0x0800A8D0
u8 ItemFindSlot(u8 arg0, u8 arg1)
{
    u8 i;
    u8 adjusted = arg0 + 1;

    if (arg1 <= 1)
        arg1 = 0;

    for (i = 0; i <= 0x2F; i++)
    {
        if (arg1 == (gUnk_08093418[i * 5 + 1] >> 4))
            if ((gUnk_08093418[i * 5]) == adjusted)
                return i + 1;
    }

    return 0xFF;
}

// @ 0x0800A924
void Party_InitStats(void)
{
    u8 i;

    for (i = 1; i < 11; i++)
    {
        sub_800A1B4(i);
    }

    for (i = 1; i < 255; i++)
    {
        gInventory[i] = 0;
    }
}
// @ 0x0800A958
u8 ItemGetValue(u8 arg0)
{
    return gUnk_08093418[(arg0 - 1) * 5 + 4];
}
// @ 0x0800A970
void sub_800A970(void *arg0)
{
    *((u16 *)arg0 + 1) = *((u16 *)arg0 + 9);
}
// @ 0x0800A978
void sub_800A978(void *arg0)
{
    *((u16 *)arg0 + 2) = *((u16 *)arg0 + 10);
}

// FullHealParty
//  @ 0x0800A980
void FullHealParty(void)
{
    u8 i;
    u8 charaId;
    PlayerStats *ptr;

    for (i = 0; i < 6; i++)
    {
        charaId = gPartyMemberIds[i];
        if (charaId == 0xFF)
            continue;
        if (charaId != 0)
            charaId--;

        ptr = &gPartyStats[charaId];
        ptr->hp = ptr->max_hp;
        ptr->mp = ptr->max_mp;
    }
}
// @ 0x0800A9C0
void EquipItem(u8 arg0, u8 newEquip, u8 equipSlotId)
{
    u8 oldEquip;
    u8 var_r0;
    u8 *equipSlot;

    PlayerStats *chara;

    var_r0 = arg0;
    if (arg0 != 0)
    {
        var_r0 = arg0 - 1;
    }

    chara = &gPartyStats[var_r0];

    switch (equipSlotId)
    {
        case 1:
            equipSlot = &chara->equip_slot2;
            break;
        case 2:
            equipSlot = &chara->equip_slot3;
            break;
        case 3:
            equipSlot = &chara->equip_slot4;
            break;
        case 4:
            equipSlot = &chara->equip_slot5;
            break;
        case 5:
            equipSlot = &chara->equip_slot6;
            break;
        default:
            equipSlot = &chara->equip_slot1;
            break;
    }
    oldEquip = *equipSlot;
    *equipSlot = newEquip;

    if (oldEquip != 0)
    {
        if (gInventory[oldEquip] < 99)
        {
            AddInventoryItem(oldEquip, 1);
        }
    }

    Stats_RebuildEquipBonuses(arg0);
    Stats_RecalcEquip(arg0);
}

// sub_800AA60 = AddInventoryItem
// @ 0x0800AA60
void sub_800AA60(u8 itemId, u8 count)
{
    s32 totalCount;

    totalCount = gInventory[itemId] + count;
    if (totalCount > 99)
    {
        gInventory[itemId] = 99;
    }
    else
    {
        gInventory[itemId] = totalCount;
    }
}

// sub_800AA84 = RemoveInventoryItem
// @ 0x0800AA84
void sub_800AA84(u8 itemId, u8 count)
{
    s32 totalCount;

    totalCount = gInventory[itemId] - count;
    if (totalCount < 0)
    {
        gInventory[itemId] = 0;
    }
    else
    {
        gInventory[itemId] = totalCount;
    }
}

// @ 0x0800AAA4
void Silver_Add(s32 arg0)
{
    gSilverAmount += arg0;

    if (gSilverAmount > 999999)
    {
        gSilverAmount = 999999;
    }
}
// @ 0x0800AAC0
void Silver_Sub(s32 arg0)
{
    gSilverAmount -= arg0;

    if (gSilverAmount > 999999)
    {
        gSilverAmount = 0;
    }
}
extern u8 gUnk_087EA580[];

// @ 0x0800AADC
u8 sub_800AADC(u8 arg0)
{
    return gUnk_087EA580[arg0 * 12 + 4] & 0xF;
}
extern u8 gUnk_087EA580[];

// @ 0x0800AAF8
u16 sub_800AAF8(u8 arg0)
{
    return gUnk_087EA580[arg0 * 12] + (gUnk_087EA580[arg0 * 12 + 1] << 8);
}
extern u8 gUnk_087EA580[];

// @ 0x0800AB18
u16 sub_800AB18(u8 arg0)
{
    return gUnk_087EA580[arg0 * 12 + 2] + (gUnk_087EA580[arg0 * 12 + 3] << 8);
}

// @ 0x0800AB3C
u8 Party_AnyEquip(void)
{

    u16 i;
    u8 charaId;
    PlayerStats *ptr;

    for (i = 0; i < 5; i++)
    {
        charaId = gPartyMemberIds[i];
        if (charaId == 0xFF)
            continue;
        if (charaId != 0)
            charaId--;

        ptr = &gPartyStats[charaId];

        if (ptr->equip_slot1 != 0)
            return 1;
    }
    return 0;
}
// @ 0x0800AB7C
void Chara_ClearTempStatus(u8 arg0)
{
    PlayerStats *actor;
    if (arg0 <= 1)
    {
        actor = &gPartyStats[0];
        Stats_BuildSkillList(actor->skills, actor->lv, arg0);

        if (actor->field_unk[2] == 1 && (u8)(actor->field_unk[3] - 5) <= 3)
        {
            actor->field_unk[2] = 0;
            actor->field_unk[3] = 0;
        }
    }
}
// @ 0x0800ABBC
void Stats_ClearEquipBonus(void)
{
    gEquipBonusAtkBase = 0;
    gEquipBonusDef2 = 0;
    gEquipBonusAgl = 0;
    gEquipBonusMen = 0;
    gEquipBonusRes = 0;
    gEquipBonusNoa = 0;
    gEquipBonusLuc = 0;
    gEquipBonusAtk = 0;
    gEquipBonusDef = 0;
}
/* PartyForm_ApplyBonus — 队伍形态一致性检查 (0x0800AC08):
 * 4 名角色 (1-based id) 的种族/形态字节 (field_4 高 4 位) 全部相同时,
 * 若形态为 0xE/0xF (特殊形态) 则设置全队攻/防加成 (Stats_RecalcEquip 消费)。 */
// @ 0x0800AC08
void PartyForm_ApplyBonus(u8 arg0, u8 arg1, u8 arg2, u8 arg3)
{
    u8 val;
    EnemyCharaStat *p0, *p1, *p2, *p3;
    p0 = &gCharaBaseData[arg0];
    p1 = &gCharaBaseData[arg1];
    p2 = &gCharaBaseData[arg2];
    p3 = &gCharaBaseData[arg3];

    val = p0->formRace & 0xF0;

    if (val == (p1->formRace & 0xF0) && val == (p2->formRace & 0xF0) && val == (p3->formRace & 0xF0))
    {
        switch (val >> 4)
        {
            case 0xF:
                gEquipBonusAtk = 0x22;
                gEquipBonusDef = 0x2D;

                break;
            case 0xE:
                gEquipBonusAtk = 0x3C;
                gEquipBonusDef = 0x3F;
                break;
        }
    }
}

// FullHealCharacter
//  @ 0x0800ACA4
void FullHealCharacter(u8 arg0)
{
    PlayerStats *ptr;

    if (arg0 != 0)
    {
        arg0--;
    }

    ptr = &gPartyStats[arg0];
    ptr->hp = ptr->max_hp;
    ptr->mp = ptr->max_mp;
}

// @ 0x0800ACC8
