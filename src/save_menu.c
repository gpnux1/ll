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

// @ 0x08010F10
INCLUDE_ASM("asm/nonmatchings", sub_8010F10);

extern u8 gUnk_08095828[][8];

// @ 0x0801114C
void sub_801114C(void)
{
    u16 i;
    u8 j;
    u8 idx;
    PlayerStats *chara;
    u8 paletteId;
    u8 *src;
    u16 *dest;
    u8 charCode;
    u8 y;

    idx = gPartyMemberIds[gPartyMenuIdx];
    if (idx != 0)
        idx--;
    chara = &gPartyStats[idx];

    ClearBuffer((u16 *)0x02005AA0, 8, 6);

    for (i = 0; i < 3; i++)
    {
        idx = chara->skills[i + gSkillMenuPage];

        if (idx == 0xFF || idx == 0x26)
            break;

        if (i == gMenuCursorSel - 16)
        {
            gSkillMenuTmpB = idx;
            paletteId = 0xD;
        }
        else
        {
            paletteId = 0xB;
        }

        y = (i * 2) + 10;

        if (idx != 0xFF)
        {
            src = gUnk_08095828[(u8)(idx - 1)];
            dest = (u16 *)0x2005820 + (y * 32);

            for (j = 0; j < 8; j++)
            {
                charCode = *src++;
                if (charCode == 0)
                {
                    break;
                }
                Text_PutGlyph(dest++, charCode, paletteId);
            }
        }
    }
}
// @ 0x08011268
INCLUDE_ASM("asm/nonmatchings", sub_8011268);

/* 在 16 页道具表 gInvPageItemIds[] 中, 从 gSkillMenuPage+1 页向后找第一个
 * 仍持有的道具页 i (gUnk_03004980[itemId] != 0); 之后至少还要有 2 个持有页
 * 才算有效, 返回页号 i; 没有候选页返回 0, 候选不足返回 0xFF。 */
extern u8 gUnk_03004980[];

// @ 0x080113CC
u8 sub_80113CC(void)
{
    u8 i;
    u8 j;
    u8 count;

    i = gSkillMenuPage + 1;
    while (i <= 15 && gUnk_03004980[gInvPageItemIds[i]] == 0)
        i++;
    if (i > 14)
        return 0;
    count = 0;
    for (j = i + 1; j <= 15; j++)
    {
        if (gUnk_03004980[gInvPageItemIds[j]] != 0)
        {
            count++;
            if (count == 2)
                return i;
        }
    }
    return 0xFF;
}

// @ 0x08011454
INCLUDE_ASM("asm/nonmatchings", sub_8011454);
// @ 0x08012530
INCLUDE_ASM("asm/nonmatchings", sub_8012530);
// @ 0x08012790
INCLUDE_ASM("asm/nonmatchings", SaveUi_LoadScreen);
// @ 0x08013870
INCLUDE_ASM("asm/nonmatchings", sub_8013870);
// @ 0x08013934
INCLUDE_ASM("asm/nonmatchings", sub_8013934);
// @ 0x08013B0C
void sub_8013B0C(u16 arg0)
{
    u16 *tile;
    u16 v;
    u16 base;
    u16 shadow;
    u16 flag;
    int new_var;
    gUnk_03004DBC++;
    new_var = 0x204;
    flag = 0;
    tile = Text_TileAt(10, 2);
    if (arg0 == 0xB0)
    {
        v = (gUnk_03004DBC >> 4) & 3;
        shadow = v;
        *tile = ((shadow + 0xC) << 12) + new_var;
        tile--;
        *tile = ((((v + 1) & 3) + 0xC) << 12) + 0x204;
        tile--;
        *tile = ((((v + 2) & 3) + 0xC) << 12) + new_var;
        tile--;
        *tile = ((((shadow + 3) & 3) + 0xC) << 12) + 0x204;
        tile--;
        *tile = ((((v + 4) & 3) + 0xC) << 12) + new_var;
    }
    else
    {
        v = (gUnk_03004DBC >> 2) & 3;
        if (v == 3)
        {
            v = 1;
            flag = 0x400;
        }
        base = (u16) (0x204 + v);
        shadow = 0xD000;
        *tile = 0xB001;
        tile--;
        if (arg0 > 0x8C)
        {
            *tile = (base + shadow) + flag;
        }
        else
        {
            *tile = 0xB001;
        }
        tile--;
        if (arg0 > 0x69)
        {
            *tile = (base + shadow) + flag;
        }
        else
        {
            *tile = 0xB001;
        }
        tile--;
        if (arg0 > 0x46)
        {
            *tile = (base + shadow) + flag;
        }
        else
        {
            *tile = 0xB001;
        }
        tile--;
        v = arg0;
        if (v > 0x23)
        {
            *tile = (base + shadow) + flag;
        }
        else
        {
            *tile = 0xB001;
        }
    }
}
// @ 0x08013C00
INCLUDE_ASM("asm/nonmatchings", sub_8013C00);
extern u16 gUnk_03004D48;
extern u16 gUnk_03004DE8;
extern u8 gSaveSignature[];
extern u8 Save_Fsm_ByteResult(u8) __asm__("Save_Fsm");

static inline s16 Save_SigCheck(u8 *p)
{
    u8 i;

    for (i = 0; i < 12; i++)
    {
        if (*p != gSaveSignature[i])
        {
            return 1;
        }
        p++;
    }

    return 0;
}

// @ 0x08013F3C
void Save_LoadSlot0(void)
{
    u16 i;

    gUnk_03004D44 = 1;
    gUnk_03004DD0 = 0;

    while (Save_Fsm_ByteResult(1) != 0)
        ;

    if (Save_SigCheck((u8 *)0x02027000) == 0)
    {
        Save_SyncShadow();
        gUnk_03004DE8 = 1;
        return;
    }

    gUnk_03004DE8 = 0;
    gUnk_03004D48 = 0;

    i = 0;
    while (i < 0x5A)
        gSaveTimers[i++] = 0;

    i = 0;
    while (i < 8)
        gSaveFlags[i++] = 0;
}
extern u16 gUnk_080981E6[]; /* 块长度表 {2, 0x5A, 8, 0} */
extern u8 *gUnk_087EB1E8[]; /* 块指针表 {&gUnk_03004D48, gSaveTimers, gSaveFlags, ...} */
extern u8 gSaveSignature[]; /* 12B "LUNAR1_12_09" */

/* 把存档状态序列化进 0x02027000 影子缓冲: 签名(12B) + 各块数据 + 校验和字节。
   签名最后才写, 保证写一半的存档校验不过 (Save_LoadSlot0 的 sigCheck)。 */
// @ 0x08013FE8
void Save_LoadContinue(void)
{
    u8 *buf;
    u16 i;
    u16 count;
    u8 sum;
    u8 *src;
    u16 blk;
    u32 next;

    buf = (u8 *)0x02027000;

    for (i = 0; i <= 0xB; i++)
    {
        buf[i] = 0xFF;
    }

    sum = 0;
    i = 0xC;
    blk = 0;
    while ((count = gUnk_080981E6[blk]) != 0)
    {
        src = gUnk_087EB1E8[blk];
        next = blk + 1;
        while (count != 0)
        {
            buf[i] = *src;
            sum += buf[i];
            i++;
            src++;
            count--;
        }
        blk = next;
    }
    buf[i] = sum;

    for (i = 0; i <= 0xB; i++)
    {
        buf[i] = gSaveSignature[i];
    }
}

// @ 0x08014084
void SaveTimer_CountUsed(void)
{
    u16 i;
    u32 val;
    u8 mask;

    val = (u32)&gUnk_03004DE4;
    *(u16 *)val = 0;

    for (i = 0; i <= 0xAF; i++)
    {
        if ((i & 1) != 0)
        {
            val = gSaveTimers[i >> 1];
            mask = 0xF0;
        }
        else
        {
            val = gSaveTimers[i >> 1];
            mask = 0xF;
        }

        if ((mask & val) != 0)
        {
            gUnk_03004DE4++;
        }
    }
}

// @ 0x080140D0
void SaveTimer_Inc(u8 arg0)
{
    u8 byte;
    u8 nib;
    u8 hi;

    byte = gSaveTimers[arg0 >> 1];

    if ((arg0 & 1) != 0)
    {
        nib = (byte >> 4) + 1;
        if (nib > 5)
        {
            nib = 5;
        }
        hi = nib << 4;
        byte = hi | (byte & 0xF);
    }
    else
    {
        nib = (byte & 0xF) + 1;
        if (nib > 5)
        {
            nib = 5;
        }
        byte = (byte & 0xF0) | nib;
    }

    gSaveTimers[arg0 >> 1] = byte;
}

// @ 0x08014124
void SaveTimer_Dec(u8 arg0)
{
    u8 byte;
    u8 nib;
    u8 hi;

    byte = gSaveTimers[arg0 >> 1];

    if ((arg0 & 1) != 0)
    {
        nib = byte >> 4;
        if (nib == 0)
        {
            nib = 1;
        }
        nib -= 1;
        hi = nib << 4;
        byte = hi | (byte & 0xF);
    }
    else
    {
        nib = byte & 0xF;
        if (nib == 0)
        {
            nib = 1;
        }
        nib -= 1;
        byte = (byte & 0xF0) | nib;
    }

    gSaveTimers[arg0 >> 1] = byte;
}

// @ 0x0801417C
INCLUDE_ASM("asm/nonmatchings", sub_801417C);
/* 把存档菜单的精灵挂进渲染队列: 先追加固定节点 gSpriteNodePool[112],
   再把 15 个菜单 UI 实体中 (statusFlags & 0xC0) == 0x80 的挂进队列下一个空槽。
   队列以 NULL 结尾, 每次都从上次落点继续向后找空槽。 */
// @ 0x08014488
void sub_8014488(void)
{
    u16 i;
    u16 j;

    if ((u8)(gUnk_03004D40 - 9) > 0xE6)
    {
        RenderQueue_Clear();
        return;
    }

    sub_8014A68();
    UiSprites_Update();

    i = 0;
    while (gSpriteRenderQueue[i] != 0)
    {
        i++;
    }

    gSpriteRenderQueue[i] = &gSpriteNodePool[112];

    for (j = 0; j <= 0xE; j++)
    {
        if ((gUiSprites[j].statusFlags & 0xC0) == 0x80)
        {
            if (gSpriteRenderQueue[i] != 0)
            {
                do
                {
                    i++;
                } while (gSpriteRenderQueue[i] != 0);
            }
            gSpriteRenderQueue[i] = &gSpriteNodePool[gUiSprites[j].oamSlotId];
        }
    }
}
/* 存档菜单初始化: 复位菜单状态, 把 15 个 UI 实体槽清零 (前 5 个按在队伍里的
   角色填坐标/图块), 然后建立光标精灵 + 5 个角色头像精灵的链节点并挂进渲染队列。
   attr2 一个变量同时承担两次调用的第 5 参数 —— 这样它才是跨块量, 由 global-alloc
   落在 r0; 若和 field_6 共用变量, 会被 arg2 的寄存器建议拽到 r2。 */
// @ 0x0801455C
void sub_801455C(void)
{
    u16 i;
    u16 attr1;
    u16 attr2;
    u16 attr0;
    SpriteNode *obj;

    MenuState_Reset();
    gUnk_03000048.field_4 = 0x60;
    gUnk_03000048.field_6 = 0x2C;
    sub_800E668(1);

    for (i = 0; i <= 0xE; i++)
    {
        if (i <= 4 && gPartyMemberIds[i] != 0xFF)
        {
            gUiSprites[i].x = 0x48 + ((i * 5) << 3);
            gUiSprites[i].y = 8;
            gUiSprites[i].statusFlags = 0x80;
            gUiSprites[i].baseTileId = i * 3 * 16 + 0x200;
            gUiSprites[i].oamSlotId = 0x71 + i;
        }
        else
        {
            gUiSprites[i].x = 0;
            gUiSprites[i].y = 0;
            gUiSprites[i].statusFlags = 0;
            gUiSprites[i].oamSlotId = 0;
        }
        gUiSprites[i].animTimer = 0;
        gUiSprites[i].lerpFrame = 0;
    }

    MenuUi_SpawnAuxSprites(3);
    gUiSprites[5].statusFlags |= 8;

    obj = (SpriteNode *)0x03004380;
    attr0 = gUnk_03000048.field_6;
    attr1 = ((gUnk_03000048.field_4 - 0x20) & 0x1FF);
    attr1 += 0x8000;
    attr2 = 0x21C0;
    Sprite_InitChainNode(obj, 1, attr0, attr1, attr2);
    gSpriteRenderQueue[0] = obj;

    for (i = 0; i <= 4; i++)
    {
        obj++;
        if (gPartyMemberIds[i] != 0xFF)
        {
            attr1 = 0x8028 + ((i * 5) << 3);
            attr2 = ((i + 3) << 12) + ((i * 3 * 16 + 0x200) & 0x3FF);
            Sprite_InitChainNode(obj, 1, 8, attr1, attr2);
            gSpriteRenderQueue[i + 1] = obj;
        }
    }
}
// @ 0x080146A8
INCLUDE_ASM("asm/nonmatchings", sub_80146A8);
// @ 0x08014A68
INCLUDE_ASM("asm/nonmatchings", sub_8014A68);

extern u8 *D_87EB2A8[];

// const u8 m1[] = {
//     9, 9, 0xF0, 9, 0xFF
// };

extern u8 *D_87EB2A8[];

// const u8* D_87EB2A8[] = {
// m1
// };

// shop
// @ 0x0801543C
void sub_801543C(u8 arg0)
{
    u8 var_r6;
    u8 i;
    u8 *src;
    u8 x, y;
    u16 *dst;
    u8 count;

    for (i = 0; i < 3; i++)
    {
        if (i == *(u8 *)0x03000187)
        {
            var_r6 = 0xD;
        }
        else
        {
            var_r6 = 0xB;
        }
        src = D_87EB2A8[i];
        x = *src++;
        y = *src++;

        dst = (u16 *)0x02005800 + x + y * 32;

        if (arg0 == 0)
        {
            count = 0;
            while (*src != 0xFF)
            {
                src++;
                count++;
            }

            while (count != 0)
            {
                Text_PutGlyph(dst++, 0, 0x0B);
                count--;
            }
        }
        else
        {
            while (*src != 0xFF)
            {
                Text_PutGlyph(dst++, *src++, var_r6);
            }
        }
    }
}

// @ 0x080154E8
INCLUDE_ASM("asm/nonmatchings", sub_80154E8);
// @ 0x08015658
INCLUDE_ASM("asm/nonmatchings", sub_8015658);

// @ 0x08015AF0
INCLUDE_ASM("asm/matchings", sub_8015AF0);
// ⏸ 无候选; 逻辑已全解(见 progress.md): 两处 tilemap 写(0x020059AA/0x02005BEA)+gUnk_08093550 查表;
//   卡在 GCC2 调度: 目标把 store 基址 ldr 插在 (bit|0x826) 之后, 我方版本提前物化基址 → +0xd 起错位。
//   需 gUnk_03000228(IWRAM)/gUnk_08093550(ROM) 符号 + 逐条对齐调度, 待攻。

// @ 0x08015B90
void InvUi_DrawCursors(void)
{

    if (Inv_PrevNonZero() != 0)
    {

        SetBgUnknown((u16 *)0x02005992, (((gUnk_03000198 >> 3) & 1) + 0x826));
    }
    else
    {
        SetBgUnknown1((u16 *)0x02005992, 0xb27F);
    }

    if (Inv_NextNonZero(4) != 0)
    {
        SetBgUnknown((u16 *)0x02005BD2, (((gUnk_03000198 >> 3) & 1) + 0x26));
    }
    else
    {
        SetBgUnknown1((u16 *)0x02005BD2, 0xB27F);
    }
}
// @ 0x08015C18
INCLUDE_ASM("asm/nonmatchings", InvUi_Main);
// @ 0x08015E1C
INCLUDE_ASM("asm/nonmatchings", sub_8015E1C);
// @ 0x08015E88
void Save_ResetReadState(void)
{
    gUnk_03004D44 = 1;
    gUnk_03004DD0 = 0;
}
// @ 0x08015EA0
void Save_StartWrite(void)
{
    Save_LoadContinue();
    gUnk_03004D44 = 3;
    gUnk_03004DD0 = 0;
    *(u8 *)0x03000221 = 1;
    Msg_ShowById(0x18, 0xB);
}

// @ 0x08015ED0
s32 sub_8015ED0(u8 arg0)
{
    u8 *p;
    u8 i;

    if (arg0 > 3)
    {
        return 0xFF;
    }

    p = (u8 *)(0x02021000 + (arg0 * 0x2000));

    for (i = 0; i < 12; i++)
    {
        if (*p != gSaveSignature[i])
        {
            return 1;
        }
        p++;
    }

    return 0;
}

// @ 0x08015F14
void SaveUi_DrawSlots(void)
{
    u8 var_r3;
    u8 i;

    for (i = 0; i < 3; i++)
    {
        Save_FillSlot3(i);

        var_r3 = *(u8 *)0x03000220 == i ? 0xD : 0xB;

        sub_8010F10(i, 8, i * 2 + 5, var_r3);
    }
}
// @ 0x08015F50
u32 SaveTimer_Get(u8 arg0)
{
    u8 temp_r2;
    temp_r2 = gSaveTimers[arg0 >> 1];

    return (arg0 & 1) != 0 ? (temp_r2 >> 4) : (temp_r2 & 15);
}
// @ 0x08015F74
void SaveFlag_Set(u8 x)
{
    gSaveFlags[x >> 3] |= (1 << (x & 7));
}
// @ 0x08015F94
int SaveFlag_Get(u8 arg0)
{
    return (gSaveFlags[arg0 >> 3] >> (arg0 & 7)) & 1;
}
// @ 0x08015FB4
void SaveUi_Open(u8 arg0)
{
    gSaveUiParam = arg0;
    gUnk_03004D40 = 0x28;
    gSaveBusyB = 0;
    gSaveBusyA = 1;
    gMainGameState = 0xD;

    ClearBuffer((u16 *)0x02005800, 30, 20);
}
extern u8 *gUnk_087EB2E0[];
extern u8 gUnk_0809E4E4[][32];

// @ 0x08016038
void sub_8016038(u8 arg0)
{
    u8 temp_r0;

    temp_r0 = gPartyMemberIds[arg0];
    if (temp_r0 != 0xFF)
    {
        LZ77UnCompWram(gUnk_087EB2E0[temp_r0], 0x02020000);
    }
}

// @ 0x08016068
void sub_8016068(arg0) u8 arg0;
{
    u8 temp_r1;

    temp_r1 = gPartyMemberIds[arg0];
    if (temp_r1 != 0xFF)
    {
        DmaCopy32(3, 0x02020000, (arg0 * 0x600) + 0x06014000, 0x6C0);
        DmaCopy16(3, gUnk_0809E4E4[temp_r1], (arg0 << 5) + 0x05000260, 0x20);
    }
}

// @ 0x080160CC
void sub_80160CC(void)
{
    LZ77UnCompWram(gUnk_087EB2E0[11], 0x02020000);
    LZ77UnCompWram(0x080A0B58, 0x02020800);
}
// @ 0x080160F4
void sub_80160F4(void)
{
    DmaCopy32(3, 0x02020000, 0x06013800, 0x800);
    DmaCopy16(3, 0x0809E644, 0x05000240, 0x20);
    DmaCopy32(3, 0x02020800, 0x06017000, 0x1000);
    DmaCopy16(3, 0x080A12D0, 0x05000340, 0x40);
    DmaCopy16(3, gSaveMenuUiPalettes, 0x050001C0, 0x40);
}
#define VRAM_BUF_2005800 (u16 *)0x02005800

// @ 0x08016178
void sub_8016178(u16 arg0)
{
    u16 rows;
    u16 cols;
    u16 *ptr;
    u16 y, x;
    u16 *temp_r5;

    if (arg0 == 0)
        return;

    rows = (arg0 * (arg0 - 1) + 1);

    if (rows > 10)
        rows = 10;

    cols = arg0 << 3;

    if (cols > 15)
        cols = 15;

    ptr = VRAM_BUF_2005800 + (15 - cols) + (10 - rows) * 32;

    cols = cols * 2;
    rows = rows * 2;

    for (y = 0; y < rows; y++)
    {
        temp_r5 = ptr;

        for (x = 0; x < cols; x++)
        {
            *ptr++ = 0xB001;
        }
        ptr = temp_r5 + 0x20;
    }
}
extern u8 gUnk_08098308[];

// @ 0x080161F4
void sub_80161F4(arg0, x, y) u8 arg0;
u8 x;
u8 y;
{
    u16 *ptr;
    u16 i;

    if (arg0 != 0)
    {
        arg0--;
    }
    arg0 <<= 2;
    ptr = VRAM_BUF_2005800 + x + y * 32;

    for (i = 0; i < 4; i++)
    {
        *ptr++ = 0xB240 + gUnk_08098308[arg0 + i];
    }
}

// @ 0x0801624C
void Num_Draw16(s16 arg0, u16 *dest)
{
    sub_800BFF8(arg0, dest, 0xB000);
}

// @ 0x08016260
void Hud_DrawLv(u8 arg0, u8 x, u8 y)
{
    u16 *dest;

    if (arg0 != 0)
        arg0--;

    dest = (u16 *)0x02005800 + x + y * 32;
    *dest = 0xB257;
    dest += 3;

    Num_Draw16(gPartyStats[arg0].lv + 1, dest);
}

// @ 0x080162A8
void Hud_DrawHp(u8 arg0, u8 x, u8 y)
{
    u16 *dest;
    u32 unk;

    if (arg0 != 0)
        arg0--;

    dest = (u16 *)0x02005800 + x + y * 32;
    *dest = 0xB258;
    dest += 3;

    unk = gPartyStats[arg0].hp == gPartyStats[arg0].max_hp ? 0xF000 : 0xB000;

    sub_800BFF8(gPartyStats[arg0].hp, dest, unk);
}

// @ 0x08016308
void Hud_DrawMp(u8 arg0, u8 x, u8 y)
{
    u16 *dest;
    u32 unk; // color

    if (arg0 != 0)
        arg0--;

    dest = (u16 *)0x02005800 + x + y * 32;
    *dest = 0xB259;
    dest += 3;

    unk = gPartyStats[arg0].mp == gPartyStats[arg0].max_mp ? 0xF000 : 0xB000;

    sub_800BFF8(gPartyStats[arg0].mp, dest, unk);
}

/* 窗口/菜单瓦片图 (0x02005800, 行距 32 个 u16) 的**单个字形**写入器。
 *
 * 本项目 UI 文字是 8×16 字形: 一个字占**上下两格** —— 顶格写 tilemap[0], 底格写
 * tilemap + 0x20 (下一行同列)。字形 N 占用字模瓦片 2N 与 2N+1。
 * 写入值是标准 BG 图块项 (位 0-9 = 瓦片号, 位 12-15 = 调色板号), 故 *tilemap = (palette<<12) + tileId。
 *
 * charCode 的三种取值:
 *   0        空白字形, 上下两格都写瓦片 1 —— Text_ClearRect 里手写的 0xB001 就是它的结果
 *   1..0xFF  基本字模块, tileId = charCode * 2
 *   xx|0xFE  转义: 高字节 xx 索引**扩展字模块**, tileId = 0x280 + xx * 2
 *            (0x280 = 640, 与 Text_WriteChars 用的 8×8 块基址 0x200 并列成两个字体块)
 *
 * 调用者: Msg_RenderLine / Msg_DrawPoolSegment / Msg_ShowEndMark / Text_DrawChar /
 *         MenuUi_DrawItemList / Text_WriteOrClear —— 所有 8×16 文字路径都汇到这一个原语。
 *
 * 代码生成要点 (已逐字节验证: fndiff score=0, bytecmp OK 100B)。下面每条都是实测过的坑:
 *   - `palette << 12` **必须每个分支各写一遍**; 外提成公共前缀会让 GCC2 重排块 (2435 -> 2610)
 *   - 外层必须是 `if (charCode != 0) { 转义 / 基本 } else { 空白 }` 的**嵌套**形式;
 *     平铺成 if / else if / else 不匹配 (905) —— 目标的空白字形体在函数末尾, 是条远跳
 *   - 底格瓦片号: 基本分支用 `|= 1` (目标 orrs), 转义分支用 `+= 1` (目标 adds)。
 *     数值等价 (tileId 恒为偶) 但指令不同, 不能统一写法 (规则 36)
 *   - 转义分支的 `>> 7` 不能改写成 `>> 8 << 1` 或 `* 2` —— GCC2 的移位域折叠不同 (5)
 *   - 两格之间用 `tilemap += 0x20` 推进, **不能**写成 tilemap[0] / tilemap[32] 下标 (1610)
 */
#define TILEMAP_ROW_STRIDE 0x20 /* 瓦片图一行 = 32 个 u16 */
#define GLYPH_TILES        2 /* 一个 8×16 字形占 2 个 8×8 字模瓦片 */
#define EXT_GLYPH_BASE     0x280 /* 扩展字模块的瓦片基址 */
#define ESCAPE_PREFIX      0xFE /* charCode 低字节为此值时, 高字节索引扩展字模块 */
#define BLANK_TILE         1 /* 空白字形瓦片 */
#define PAL_SHIFT          12 /* BG 图块项的调色板号位移 */
#define GLYPH_BOTTOM_MASK  (GLYPH_TILES - 1)

// @ 0x08016368
void Text_PutGlyph(u16 *tilemap, u16 charCode, u8 palette)
{
    u16 palAttr;
    u16 tileId;

    if (charCode != 0)
    {
        if ((charCode & 0xFF) == ESCAPE_PREFIX)
        {
            palAttr = palette << PAL_SHIFT;
            tileId = ((charCode & 0xFF00) >> 7) + EXT_GLYPH_BASE;
            *tilemap = palAttr + tileId;
            tilemap += TILEMAP_ROW_STRIDE;
            tileId += 1;
            *tilemap = palAttr + tileId;
        }
        else
        {
            palAttr = palette << PAL_SHIFT;
            tileId = charCode * GLYPH_TILES;
            *tilemap = palAttr + tileId;
            tilemap += TILEMAP_ROW_STRIDE;
            tileId |= GLYPH_BOTTOM_MASK;
            *tilemap = palAttr + tileId;
        }
    }
    else
    {
        palAttr = palette << PAL_SHIFT;
        tileId = BLANK_TILE;
        *tilemap = palAttr + tileId;
        tilemap += TILEMAP_ROW_STRIDE;
        *tilemap = palAttr + tileId;
    }
}
