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

// @ 0x08005020
INCLUDE_ASM("asm/nonmatchings", sub_8005020);
// 屏幕淡化控制: 根据当前扫描线 (REG_VCOUNT) 与目标扫描线 gScreenFadeProgress 的差值,
// 每 16 行推进一桢渐变, 写入 REG_BLDY; 到达目标后恢复/切换 REG_BLDCNT。
//   fadeDelta = ((u16)gScreenFadeProgress - REG_VCOUNT) >> 4   (算术移位, 保留符号)
//   fadeDelta <= 0  → 过渡完成: BLDCNT = gBlendControl, BLDALPHA = gBlendCoefficients
//   (gScreenFadeFlags & 0x11) == 1 → BLDCNT = 0xBE (另一类混合模式)
//   其余         → BLDCNT = 0xFF
// 注: gScreenFadeProgress 在 iwram.h 里是 s16, 必须 (u16) 强转 —— 否则 GCC2 发
//     `ldrsh`(带 movs 零索引) 而非目标的 `ldrh`。参数不能声明成 u8(规则 65)。
// 注: 下面 `if (bldcnt != 0xFF)` 对未初始化变量赋值是**故意保留的死代码** ——
//     它让 GCC2 提前给 bldcnt 选定 callee-saved r4, 否则尾部会重新物化
//     `movs r0,#0xff`, 与目标不一致(删掉差 130 字节)。参见 RULES 规则 89。
// @ 0x080051D0
void ScreenFade_Apply(void)
{
    vu8 *vcountReg;
    s32 fadeDelta;
    u16 blendLevel;
    s16 signedFadeDelta;
    u16 blendControl;

    if (gScreenFadeFlags != 0)
    {
        if (blendControl != 0xFF)
            blendControl = 0xFF;
        vcountReg = (vu8 *)REG_ADDR_VCOUNT;
        blendControl = 0xFF;
        fadeDelta = (((u16)gScreenFadeProgress - *vcountReg) << 16) >> 20;
        blendLevel = (u16)fadeDelta;
        signedFadeDelta = (s16)fadeDelta;
        if (signedFadeDelta <= 0)
        {
            REG_BLDCNT = gBlendControl;
            REG_BLDALPHA = gBlendCoefficients;
            return;
        }
        if ((gScreenFadeFlags & 0x11) == 1)
        {
            if (signedFadeDelta > 0x10)
                blendLevel = 0x10;
            if ((blendLevel << 16) < 0)
                blendLevel = 0;
            REG_BLDY = blendLevel;
            REG_BLDCNT = 0xBE;
            return;
        }
        if (signedFadeDelta > 0x10)
            blendLevel = 0x10;
        if ((blendLevel << 16) < 0)
            blendLevel = 0;
        REG_BLDY = blendLevel;
        REG_BLDCNT = blendControl;
    }
}

// @ 0x0800526C
void ScreenFade_Update(void)
{
    REG_BLDCNT = gBlendControl;
    if (gBlendControl & 0x80)
    {
        REG_BLDY = gBlendCoefficients;
    }
    else
    {
        REG_BLDALPHA = gBlendCoefficients;
    }

    if (gScreenFadeFlags != 0)
    {
        if (!(gScreenFadeFlags & 0x80))
            gScreenFadeProgress += gScreenFadeStep;

        if (gScreenFadeStep < 0)
        {
            if (gScreenFadeProgress <= 0)
                gScreenFadeFlags = 0;
        }
        else
        {
            if (gScreenFadeProgress > 0x1B0)
                gScreenFadeFlags |= 0x80;
        }
    }
}
// @ 0x080052F8
INCLUDE_ASM("asm/nonmatchings", sub_80052F8);
// @ 0x080053B4
INCLUDE_ASM("asm/nonmatchings", sub_80053B4);
// @ 0x080055E8
INCLUDE_ASM("asm/nonmatchings", sub_80055E8);
// @ 0x08005B2C
u16 *MapTile_At(s16 x, s16 y)
{
    s16 screenX, screenY;
    s16 tileX, tileY;
    u16 *buf;

    screenX = x - (gCameraPosX & ~0xF);

    screenY = y - (gCameraPosY & ~0xF);

    if (screenX < 0 || screenY < 0 || screenX > 240 || screenY > 160)
    {
        return 0;
    }

    tileX = screenX >> 3;
    tileY = screenY >> 3;

    if (gUnk_03004688 == 0)
    {
        return (u16 *)0x02004000 + (tileY * 32) + tileX;
    }
    else
    {
        return (u16 *)0x02004800 + (tileY * 32) + tileX;
    }
}
// @ 0x08005BB4
u16 MapTile_CollisionBits(u16 *tiles, u16 x, u16 y)
{
    u16 *src;
    u16 ret;
    u16 xSubtile;
    u16 ySubtile;
    u16 val;

    src = tiles;

    ret = 0;

    xSubtile = x & 7;
    ySubtile = y & 7;

    val = *src & 0x3FF;
    src++;
    if (val <= gUnk_030047AC)
    {
        ret |= 4;
    }

    val = *src & 0x3FF;
    src++;
    if (val <= gUnk_030047AC)
    {
        ret |= 2;
    }

    if (xSubtile != 0)
    {
        val = *src & 0x3FF;
        ret |= 0x100;

        if (val <= gUnk_030047AC)
        {
            ret |= 1;
        }
    }

    if (ySubtile != 0)
    {
        ret |= 0x200;
        src += 0x1E;

        val = *src & 0x3FF;
        src++;
        if (val <= gUnk_030047AC)
        {
            ret |= 0x40;
        }

        val = *src & 0x3FF;
        src++;
        if (val <= gUnk_030047AC)
        {
            ret |= 0x20;
        }

        if (xSubtile != 0)
        {
            val = *src & 0x3FF;
            if (val <= gUnk_030047AC)
            {
                ret |= 0x10;
            }
        }
    }

    return ret;
}
// @ 0x08005C70
INCLUDE_ASM("asm/nonmatchings", Viewport_UpdateScroll);

// @ 0x080064AC
void BgMap_FillPattern(u16 arg0)
{
    u16 x;
    u16 y;
    u16 *dst = (u16 *)0x02005000;

    switch (arg0)
    {
        case 0:
            for (y = 0; y < 32; y++)
            {
                for (x = 0; x < 32; x++)
                {
                    *dst++ = 0xA000 + ((y & 3) << 2) + (x & 3);
                }
            }
            break;

        case 1:
            for (y = 0; y < 32; y++)
            {
                for (x = 0; x < 32; x++)
                {
                    *dst++ = 0xA000;
                }
            }
            break;
    }
}
extern u8 *gUnk_087EA020[];
extern u8 gUnk_082893EC[][0x140];

/* MapSceneDescriptor / gMapSceneDescriptors 见 include/data_805769C.h */

extern u8 *gUnk_087E9AA0[];

/* 地图整背景装载 (场景切换时): 按 gMapSceneDescriptors[arg0] 场景描述符
 *   - field_10 起把 gUnk_087E9AA0[] 里最多 5 块 LZ77 tile 逐块解压到 0x02020000 (每块 4KB 槽);
 *   - DMA 0x4A60 字节 → 0x06000000 (BG VRAM);
 *   - field_12: gUnk_082893EC 子表 (0x140 半字) → BG PLTT; BgPal_ResetFirst 复位底色;
 *   - field_E: gUnk_087EA020[] 指针 (LZ77 tilemap) 解压 → 0x02005000 → 0x0600F000 (SBB)。 */
// @ 0x08006520
void MapBg_LoadFull(u8 arg0)
{
    u8 idx;
    u16 i;

    VBlankIntrWait();
    SoundMain_Frame();
    idx = gMapSceneDescriptors[arg0].tileSetId;
    i = 0;
    while (gUnk_087E9AA0[idx] != 0)
    {
        LZ77UnCompWram(gUnk_087E9AA0[idx], (void *)0x02020000 + (i << 12));
        VBlankIntrWait();
        SoundMain_Frame();

        idx++;
        i++;
        if (i > 4)
            break;
    }

    DmaCopy32(3, 0x02020000, 0x06000000, 0x4A60);

    VBlankIntrWait();
    SoundMain_Frame();

    DmaCopy16(3, &gUnk_082893EC[gMapSceneDescriptors[arg0].bgPaletteId], 0x05000000, 0x140);

    BgPal_ResetFirst();

    DmaCopy16(3, gUnk_087EA020[gMapSceneDescriptors[arg0].tilemapId], 0x02005000, 0x280 * 2);

    DmaCopy16(3, 0x02005000, 0x0600F000, 0x800);

    VBlankIntrWait();
    SoundMain_Frame();
}

// @ 0x0800661C
INCLUDE_ASM("asm/nonmatchings", MapScene_Load);
// @ 0x080071EC
INCLUDE_ASM("asm/nonmatchings", MapScene_LoadNpcSlotIds);
/* 地图场景精灵初始化 (进入场景时): 槽 0 = 主角 (gPartyMemberIds[0]) 图块+调色板,
 * 槽 1 = 固定 11 号模型 (跟随者/光影?); 若场景描述符 npcSlotGroupId 有 NPC 集,
 * 把槽 2..9 里已在用的图块/调色板模型 (gSlotGfxId/gSlotPalId, 0xFF=空) 重载进 OBJ VRAM
 * (VRAM 最多 8 种 NPC 模型, 有的模型不变只是颜色不同)。 */
// @ 0x0800729C
void MapScene_InitSprites(u8 arg0)
{
    u16 i;

    if (gObjGraphicsSetId & 0x80)
        return;

    gSlotGfxId[0] = gPartyMemberIds[0];
    gSlotPalId[0] = gPartyMemberIds[0];
    LoadSpriteSheetGfx(0, gPartyMemberIds[0]);
    LoadSpriteSheetPal(0, gPartyMemberIds[0]);
    gSlotGfxId[1] = 11;
    gSlotPalId[1] = 11;
    LoadSpriteSheetGfx(1, 11);
    LoadSpriteSheetPal(1, 11);
    if (gMapSceneDescriptors[arg0].npcSlotGroupId != 0)
    {
        for (i = 0; i < 8; i++)
        {
            if (gSlotGfxId[i + 2] != 0xFF)
            {
                LoadSpriteSheetGfx(i + 2, gSlotGfxId[i + 2]);
            }
        }

        for (i = 0; i < 8; i++)
        {
            if (gSlotPalId[i + 2] != 0xFF)
            {
                LoadSpriteSheetPal(i + 2, gSlotPalId[i + 2]);
            }
        }
    }
}
// @ 0x08007350
INCLUDE_ASM("asm/nonmatchings", MapScene_LoadEventAnimations);
// @ 0x08007964
