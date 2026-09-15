#include "code_0.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"
#include "sound.h"


/* === 战斗 ObjHead 图形加载族 (原 src/battle_gfx_load.c, 2026-09-13 并入本 TU:
   ROM 内 .text 与本文件相邻 [0x0801A3C4,0x0801A884) 紧接本文件, 两文件 include 集
   完全一致, 合并后函数顺序 = ROM 顺序, 全量 make+SHA1 复验通过) === */

// @ 0x0801A3C4
extern u8 *gUnk_087EBE00[];

void ObjGfxLoad_Step(ObjHead *obj)
{
    switch (obj->kindFlags & 0xF)
    {
        case 1:
            LZ77UnCompVram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->vramBank << 5) + 0x06010000 + (obj->gfxPos << 12)));
            break;
        case 2:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x020212C0));
            break;
        case 3:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x02020E00));
            break;
        case 6:
            if (obj->gfxPos == 0)
            {
                DmaFill16(3, 0, (void *)0x0600C000, 0x4000);
                DmaWait(3);
            }
        case 4:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x0202B2C0));
            break;
        case 5:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x020302C0));
            break;
        case 7:
            if (obj->gfxPos == 0)
            {
                DmaFill16(3, 0, (void *)0x0600C000, 0x20);
                DmaWait(3);
            }
            LZ77UnCompVram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x0600C020));
            break;
        case 8:
            if (obj->gfxPos == 0)
            {
                DmaFill16(3, 0, (void *)0x06008000, 0x20);
                DmaWait(3);
            }
            LZ77UnCompVram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x06008020));
            break;
        case 9:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x02037C28));
            break;
    }

    obj->gfxPos++;
    if (obj->gfxPos >= obj->gfxTotal)
    {
        sub_801A684(obj);
        if ((obj->kindFlags & 0xF) != 9)
            obj->kindFlags = 0xF7FF & obj->kindFlags;
    }
}

// @ 0x0801A5EC
void ObjGfxLoad_Copy(ObjHead *dst, ObjHead *src)
{
    dst->cmdBase0 = src->cmdBase0;
    dst->cmdBase1 = src->cmdBase1;
    dst->jumpTable0 = src->jumpTable0;
    dst->jumpTable1 = src->jumpTable1;
    dst->kindFlags = src->kindFlags;
    dst->f_28 = src->f_28;
    dst->f_1A = src->f_1A;
    dst->frameIdx = src->frameIdx;
    dst->f_1E = src->f_1E;
    dst->palSlot = src->palSlot;
    dst->f_2A = src->f_2A;
    dst->f_2B = src->f_2B;
    dst->f_2C = src->f_2C;
    dst->f_2D = src->f_2D;
    dst->f_2E = src->f_2E;
    dst->scriptPtr = src->scriptPtr;
    dst->palBitsPtr = src->palBitsPtr;
    dst->gfxTotal = src->gfxTotal;
    dst->gfxPos = src->gfxPos;
    dst->vramBank = src->vramBank;
    dst->f_2F = src->f_2F;
    dst->gfxBaseIdx = src->gfxBaseIdx;
}
// @ 0x0801A684
/* 从脚本头 scriptPtr 重算命令流/跳转表四指针 (+0x0..0xC), 清帧游标后按 kind 分发。 */
void sub_801A684(ObjHead *head)
{
    const u16 *data;
    u32 off0;
    u32 off1;
    u16 type;
    u32 palBits;
    u8 pal;
    u8 zero8;
    u16 zero16;
    u8 copied;

    do
    {
        data = head->scriptPtr;
        off0 = (u32)data + data[0];
        head->cmdBase0 = (u16 *)off0;
        off1 = (u32)data + data[1];
        head->cmdBase1 = (u16 *)off1;
        head->jumpTable0 = head->cmdBase0 + 2;
        head->jumpTable1 = head->cmdBase1 + 2;

        // Keep GCC2's byte zero ahead of the independent halfword zero.
        zero8 = off0 & ~off0;
        zero16 = 0;
        head->f_1A = zero16;
        head->frameIdx = zero16;
        head->f_1E = head->vramBank;
        copied = head->f_2F;
        head->palSlot = copied;
        head->f_28 = zero8;
    } while (0);

    type = (head->kindFlags & 0xF) - 6;
    if (type <= 2)
    {
        sub_801A6F4(head);
    }
    else
    {
        palBits = (u32)head->palBitsPtr;
        pal = head->palSlot;
        sub_804C2FC(palBits, pal, sub_801B954(head));
    }
}
typedef union {
    BgCnt bg;
    u32 word;
} BgUnion;

// @ 0x0801A6F4
void sub_801A6F4(ObjHead *head)
{
    BgUnion bgcnt;

    switch ((s16)(head->kindFlags & 0xF))
    {
    case 6:
    case 7:
        {
            u8 val = head->f_2A;
            bgcnt.bg.Priority = head->f_2A & 3;
        }
        bgcnt.bg.CharBasep = 3;
        bgcnt.bg.Dummy_5_4 = 0;
        bgcnt.bg.Mosaic = 0;
        bgcnt.bg.ColorMode = 0;
        bgcnt.bg.ScBasep = 15;
        bgcnt.bg.Loop = 1;
        bgcnt.bg.Size = 0;
        REG_BG1CNT = bgcnt.word;
        DmaFill16(3, 0, (void *)0x06007800, 0x800);
        DmaWait(3);
        REG_DISPCNT |= 0x200;
        break;
    case 8:
        ((BgCnt *)&bgcnt)->Priority = head->f_2A;
        ((BgCnt *)&bgcnt)->CharBasep = 2;
        ((BgCnt *)&bgcnt)->Dummy_5_4 = 0;
        ((BgCnt *)&bgcnt)->Mosaic = 0;
        ((BgCnt *)&bgcnt)->ColorMode = 0;
        ((BgCnt *)&bgcnt)->ScBasep = 13;
        ((BgCnt *)&bgcnt)->Loop = 1;
        ((BgCnt *)&bgcnt)->Size = 0;
        REG_BG3CNT = *(u16 *)&bgcnt;
        DmaFill16(3, 0, (void *)0x06006800, 0x800);
        DmaWait(3);

        break;
    }

    sub_804C548((u32)head->palBitsPtr, head->palSlot, (u8)sub_801B954(head));
}

// @ 0x0801A884
INCLUDE_ASM("asm/nonmatchings", sub_801A884);
// @ 0x0801AD0C
INCLUDE_ASM("asm/nonmatchings", sub_801AD0C);
// @ 0x0801B0B8
INCLUDE_ASM("asm/nonmatchings", sub_801B0B8);


// @ 0x0801B570
void sub_801B570(ObjHead *obj)
{
    u16 *p;
    u16 *entry;
    u16 n0;
    u16 value;
    u16 offset;
    u16 count;
    u16 i;
    s16 counter;
    u8 tile;

    if (obj->kindFlags & 0x200)
    {
        return;
    }

    tile = 1;
    counter = obj->cmdBase1[0] - 1;
    while (counter >= 0)
    {
        offset = obj->jumpTable1[counter];
        p = sub_801B8E8((u16 *)((u8 *)obj->cmdBase1 + offset), obj->frameIdx);
        value = *p;
        entry = (u16 *)((u8 *)obj->cmdBase0 + obj->jumpTable0[value]);
        n0 = *entry;
        entry++;
        count = *entry;
        entry++;
        entry += n0 * 4;

        for (i = 0; i < count; i++)
        {
            u16 *saved = entry;
            if (!(obj->kindFlags & 0x800))
            {
                DmaCopy16(3,
                          (void *)(0x0202B2C0 + ((((u32)entry[2]) << 22) >> 17)),
                          (void *)(0x0600C000 + (tile << 5)),
                          gUnk_08393A30[(((u8 *)entry)[3] >> 6) + ((((u8 *)entry)[1] >> 6) << 2)] << 5);
                DmaWait(3);
                tile += gUnk_08393A30[(((u8 *)saved)[3] >> 6) + ((((u8 *)saved)[1] >> 6) << 2)];
            }
            entry += 3;
        }

        counter--;
    }
}
// @ 0x0801B688
INCLUDE_ASM("asm/matchings", sub_801B688);

// @ 0x0801B760
void sub_801B760(u16 arg0)
{
    if (sub_801B790(arg0) != 0)
    {
        return;
    }
    gObjFlagsA[arg0 / 8] |= (1 << (arg0 % 8));
}

// @ 0x0801B790
u8 sub_801B790(u16 arg0)
{
    if ((gObjFlagsA[arg0 / 8] >> (arg0 % 8)) & 1)
        return 1;
    return 0;
}
// @ 0x0801B7B8
void sub_801B7B8(void)
{
    DmaFill32(3, 0, gObjFlagsA, 0x80);
    DmaWait(3);

    DmaFill32(3, 0, gObjFlagsB, 0x80);
    DmaWait(3);
}
typedef struct Unk_8021064
{
    u16 field_0;
    u8 field_2;
    u8 field_3;
} Unk_8021064;

extern Unk_8021064 gUnk_03000670[];
extern u8 gUnk_0861C664[];
extern const u8 gUnk_0861A004[];  /* 字体表内空块 tile (表项3); 数字弹出 DMA 源, 兼作 -0x60 折叠基准 */
extern const u8 gUnk_0861A484[];  /* 字体表内空块 tile (表项39); sub_801D710 kind!=0 图形路径源 */
extern const u8 gUnk_08619FE4[];  /* 字体表内表项2; sub_801D710 kind!=0 图形路径源 */
// @ 0x0801B81C
void sub_801B81C(ObjHead *obj, u8 arg1, u8 arg2, u16 arg3, u8 arg4, u32 arg5, u32 arg6, u16 arg7, u16 arg8, u16 arg9)
{
    u16 value;

    obj->palBitsPtr = (const u8 *)arg6;
    value = arg9;
    obj->kindFlags = value | 0x800;
    obj->gfxBaseIdx = arg7;
    obj->scriptPtr = (const u16 *)arg5;
    // This redundant write is required for the original GCC2 scheduling.
    obj->palBitsPtr = (const u8 *)arg6;
    obj->gfxTotal = arg8;
    obj->vramBank = arg3;
    obj->f_2F = arg4;
    obj->gfxPos = 0;
    obj->frameIdx = 0;
    obj->f_2B = arg1;
    obj->f_2C = arg2;
}

// @ 0x0801B878
u8 sub_801B878(ObjHead *arg0, u8 arg1, u8 *arg2)
{
    s16 kind;

    kind = arg0->kindFlags & 0xF;
    switch (kind)
    {
        case 6:
        case 7:
        case 8:
            sub_801AD0C(arg0);
            return arg1;
        default:
            return sub_801A884(arg0, arg1, arg2);
    }
}
// @ 0x0801B8AC
u8 sub_801B8AC(ObjHead *arg0, u8 arg1)
{
    s16 kind;

    kind = arg0->kindFlags & 0xF;
    switch (kind)
    {
        case 6:
            sub_801B570(arg0);
            return arg1;
        case 7:
        case 8:
            return arg1;
        default:
            return sub_801B0B8(arg0, arg1);
    }
}
// @ 0x0801B8E8
u16 *sub_801B8E8(u16 *ptr, u16 value)
{
    u16 *current = (u16 *)((u8 *)ptr + 2);

    while (*(current + 1) <= value)
    {
        current += 2;
    }
    return current;
}
// @ 0x0801B8FC
u16 *sub_801B8FC(ObjHead *arg0, u8 arg1, u16 arg2)
{
    u16 *current;
    u32 val;

    val = *(u16 *)(arg1 * 2 + (u32)arg0->jumpTable1);
    current = (u16 *)((u32)arg0->cmdBase1 + val + 2);
    while (*(current + 1) <= arg2)
    {
        current += 2;
    }
    return current;
}
// @ 0x0801B920
void sub_801B920(void)
{
    u8 i;

    GameOamData *oamPtr;
    Unk_030034C0 *srcPtr;

    oamPtr = &gOamBuffer[0];
    srcPtr = gOamAffineBuf;

    for (i = 0; i < 32; i++)
    {
        oamPtr->fields.AffineParam = srcPtr->field_0;
        oamPtr++;
        oamPtr->fields.AffineParam = srcPtr->field_2;
        oamPtr++;
        oamPtr->fields.AffineParam = srcPtr->field_4;
        oamPtr++;
        oamPtr->fields.AffineParam = srcPtr->field_6;
        oamPtr++;
        srcPtr++;
    }

    // do {
    //     oamPtr[0].fields.AffineParam = srcPtr->field_0;
    //     oamPtr++;
    //     oamPtr[0].fields.AffineParam = srcPtr->field_2;
    //     oamPtr++;
    //     oamPtr[0].fields.AffineParam = srcPtr->field_4;
    //     oamPtr++;
    //     oamPtr[0].fields.AffineParam = srcPtr->field_6;

    //     oamPtr++;
    //     srcPtr++;
    //     i++;
    // } while (i < 32);
}
// @ 0x0801B954
u8 sub_801B954(ObjHead *head)
{
    return ((u8 *)head->cmdBase0)[2];
}
// @ 0x0801B95C
u16 sub_801B95C(ObjHead *head)
{
    return head->cmdBase1[1];
}
// @ 0x0801B964
INCLUDE_ASM("asm/nonmatchings", sub_801B964);
// @ 0x0801BE34
INCLUDE_ASM("asm/nonmatchings", sub_801BE34);
// @ 0x0801C484
INCLUDE_ASM("asm/nonmatchings", sub_801C484);
// @ 0x0801CE80
// @ 0x0801CA08
/* 战斗对象主头 (headA) 动画切换: kind 选择 animPtr+0x1A 基索引内的动画
 * (variantClass!=0 时 case0/5 改取基索引+2 的变色变体), 查 gUnk_08393B28[idx] 取图形
 * 参数, 尾部经 sub_801B81C 写入 headA; case3/4 额外把表项 +0xC/+0xE 复制到
 * f_B4/f_B6, 并把 animPtr[0x23]/[0x24] 写入 f_C3; variantClass==5 时 flag 追加 0x20。 */
void sub_801CA08(BattleObj *obj, u8 kind, u16 f2a, u8 f35, u8 arg5)
{
    u8 *p = obj->animPtr;
    u16 flag;
    u16 idx;

    switch (kind)
    {
    case 0:
        idx = *(u16 *)(p + 0x1A) + kind;
        flag = 0x409;
        if (obj->variantClass != 0)
            idx = *(u16 *)(p + 0x1A) + 2;
        break;
    case 5:
        idx = *(u16 *)(p + 0x1A);
        flag = 0x401;
        if (obj->variantClass != 0)
            idx += 2;
        break;
    case 1:
        idx = *(u16 *)(p + 0x1A) + kind;
        flag = 0x409;
        break;
    case 2:
        idx = *(u16 *)(p + 0x1A) + kind;
        flag = 0x409;
        break;
    case 3:
        idx = *(u16 *)(p + 0x1A) + (kind + arg5);
        flag = 2;
        obj->f_B4 = gUnk_08393B28[idx].field_C;
        obj->f_B6 = gUnk_08393B28[idx].field_E;
        obj->f_C3 = p[0x23];
        break;
    case 4:
        idx = *(u16 *)(p + 0x1A) + (kind + arg5);
        flag = 2;
        obj->f_B4 = gUnk_08393B28[idx].field_C;
        obj->f_B6 = gUnk_08393B28[idx].field_E;
        obj->f_C3 = p[0x24];
        break;
    }
    if (obj->variantClass == 5)
        flag |= 0x20;
    sub_801B81C(&obj->headA, obj->headA.f_2B, obj->headA.f_2C, f2a, f35,
                (u32)gUnk_08393B28[idx].animScriptPtr, (u32)gUnk_08393B28[idx].palettePtr,
                gUnk_08393B28[idx].gfxBaseIdx, gUnk_08393B28[idx].gfxTotal, flag);
}
// @ 0x0801CBA4
INCLUDE_ASM("asm/nonmatchings", sub_801CBA4);
extern u8 gUnk_08393A3C[];
extern u8 gUnk_08393A40[];
extern u8 gUnk_083987EC[];
extern u8 gUnk_0839DF5C[]; /* 强度斜坡 {1,2,3,4,4,3,2,1} (sub_801CF90 渐变 LUT) */
void *memcpy(void *, const void *, unsigned long);

void sub_801CE80(BattleObj *obj, u8 kind, u16 f2a, u8 f35, u8 arg5)
{
    u8 *p = obj->animPtr;
    const ObjAnimEntry *entry;
    u16 flag;
    u16 idx;
    u16 v1;
    u16 v2;

    switch (kind)
    {
        case 0:
            idx = obj->variantClass ? *(u16 *)(p + 6) : *(u16 *)p;
            entry = &gUnk_08393B28[idx];
            flag = 0x409;
            break;
        case 6:
            idx = obj->variantClass ? *(u16 *)(p + 6) : *(u16 *)p;
            entry = &gUnk_08393B28[idx];
            flag = 0x401;
            break;
        case 3:
        case 4:
            flag = 0x401;
            break;
        case 1:
            idx = *(u16 *)(p + 2);
            entry = &gUnk_08393B28[idx];
            flag = 2;
            v1 = *(u16 *)((const u8 *)entry + 0xC);
            obj->f_B4 = v1;
            v2 = *(u16 *)((const u8 *)entry + 0xE);
            obj->f_B6 = v2;
            break;
        case 2:
            idx = *(u16 *)(p + 4);
            entry = &gUnk_08393B28[idx];
            flag = 0x409;
            break;
        case 5:
        {
            u16 off = arg5 * 2;
            u8 *p8 = p + 8;
            idx = *(u16 *)(p8 + off);
            entry = &gUnk_08393B28[idx];
            flag = 2;
            v1 = *(u16 *)((const u8 *)entry + 0xC);
            obj->f_B4 = v1;
            v2 = *(u16 *)((const u8 *)entry + 0xE);
            obj->f_B6 = v2;
            break;
        }
    }
    if (obj->slot == 0x78)
        flag |= 0x20;
    sub_801B81C(&obj->headA, obj->headA.f_2B, obj->headA.f_2C, f2a, f35, (u32)entry->animScriptPtr, (u32)entry->palettePtr, entry->gfxBaseIdx, entry->gfxTotal, flag);
}
// @ 0x0801CF90
/* 战斗对象 UI 槽位更新 (BattleTask_Run 对 [0x03000244] 池 0xC8 步长逐槽调用):
 * 在 x=arg1*5+5/4, y=0x10/0x11/0x12 处画 3 个 3 位数/子图 (hp、mp、8018FC0)。
 * 调色板参数: state&0x80 时三者同取 flashLevel; 否则先 sub_801D12C(obj,0) 步进, 再由
 * substate 派生 (1/2 直取, 其余看 mp==maxMp 置 3)。state&2 时以强度斜坡
 * gUnk_0839DF5C 与阈值 *(u16*)&obj->animPtr (低半字) 生成 0x80 字节渐变 LUT
 * 写入 0x02021040+arg1*0x80 (5 槽 DMA 上传区), 随后清 state 的 bit1; 末尾
 * state&1 时清 bit0。 */
void sub_801CF90(BattleObj *obj, u8 arg1)
{
    u8 tbl[8];
    u8 palA;
    u8 palB;
    u32 sel;
    u8 flag;
    u16 i;
    u8 j;
    int newVar;
    u8 k;
    u8 *dst;
    u16 *p70;
    u16 *p72;
    u16 v70;

    memcpy(tbl, gUnk_0839DF5C, 8);
    if (obj->state & 0x80)
    {
        sel = (palA = (palB = obj->flashLevel));
        flag = 0;
    }
    else
    {
        sub_801D12C(obj, 0);
        palA = obj->substate;
        palB = 0;
        if (palA == 1)
        {
            palB = 1;
            sel = 1;
        }
        else
        {
            if (palA == 2)
            {
                palB = 2;
                sel = 2;
            }
            else
            {
                p70 = (u16 *)((u8 *)obj + 0x70); /* &mp (裸指针形态为字节匹配所需) */
                p72 = (u16 *)((u8 *)obj + 0x72); /* &maxMp */
                v70 = *p70;
                sel = 0;
                if (v70 == *p72)
                {
                    sel = 3;
                }
            }
        }
        flag = 3;
    }
    sub_8018EA8(obj->hp, (arg1 * 5) + 5, 0x10, palA, flag);
    sub_8018EA8(obj->mp, (arg1 * 5) + 5, 0x11, sel, flag);
    sub_8018FC0(arg1, (arg1 * 5) + 4, 0x12, palB, palA, sel, flag);
    if (obj->state & 2)
    {
        dst = (u8 *)(0x02021040 + (arg1 * 0x80));
        for (i = 0; i < 4; i++)
        {
            for (j = 0; j < 8; j++)
            {
                for (k = 0; k < 4; k++)
                {
                    newVar = ((((i * 8) + (k * 2)) >= *(u16 *)&obj->animPtr) ? (0xF0) : (tbl[j] << 4))
                           + ((((i * 8) + (k * 2)) >= (*(u16 *)&obj->animPtr + 1)) ? (0xF) : (tbl[j]));
                    *dst++ = newVar;
                }
            }
        }
        obj->state &= ~2;
    }
    if (obj->state & 1)
    {
        obj->state &= ~1;
    }
}
// @ 0x0801D12C
void sub_801D12C(BattleObj *obj, u8 state)
{
    s16 value;

    if (obj->slot <= 0xA)
    {
        switch (state)
        {
            case 0:
            case 1:
            case 2:
                value = obj->variantClass;
                switch (value)
                {
                    case 0:
                        if (obj->hp == obj->maxHp)
                            state = 3;
                        break;
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        state = 1;
                        break;
                    case 8:
                        state = 2;
                        break;
                }
                break;
            case 4:
                break;
            case 5:
                value = obj->variantClass;
                switch (value)
                {
                    case 0:
                        break;
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        state = 1;
                        break;
                    case 8:
                        state = 2;
                        break;
                }
                break;
        }
        obj->substate = state;
    }
}
// @ 0x0801D19C
u16 sub_801D19C(BattleObj *obj, u8 kind)
{
    u8 v;
    int ab;

    if (obj->slot <= 0xA)
    {
        v = kind + 3;
        switch (v)
        {
            case 0:
            case 1:
            case 2:
                ab = obj->variantClass;
                switch (ab)
                {
                    case 0:
                        if (obj->hp == obj->maxHp)
                            v = 6;
                        break;
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        v = 4;
                        break;
                    case 8:
                        v = 5;
                        break;
                }
                break;
            case 4:
                break;
            case 5:
                ab = obj->variantClass;
                switch (ab)
                {
                    case 1:
                    case 2:
                    case 3:
                    case 4:
                    case 5:
                    case 6:
                    case 7:
                        v = 1;
                        break;
                }
                break;
        }
        return v;
    }
}
#if 0
INCLUDE_ASM("asm/nonmatchings", sub_801D214);
#else
// @ 0x0801D214
extern u8 gUnk_0861C744[];
u8 sub_801D214(BattleObj *arg0, u8 count)
{
    u8 i;
    OamData oam;
    OamData *buf;
    u16 palette[16];

    for (i = 0; i <= 4; i++)
    {
        if (arg0[i].slot == 0xFF)
            continue;
        if (arg0[i].state & 0x20)
            continue;

        buf = (OamData *)&gOamBuffer[count];

        oam.VPos = 0x90;
        oam.AffineMode = 0;
        oam.ObjMode = 0;
        oam.Mosaic = 0;
        oam.ColorMode = 0;
        oam.Shape = 1;
        oam.HPos = i * 40 + 0x20;
        oam.AffineParamNo_L = 0;
        oam.HFlip = 0;
        oam.VFlip = 0;
        oam.Size = 1;
        oam.CharNo = i * 4 + 0x144;
        oam.Priority = 2;
        oam.Pltt = 6;

        *buf = oam;
        count--;
    }

    DmaCopy32(3, (void *)0x02021040, (void *)0x06012880, 0x280);
    DmaWait(3);

    sub_804C2FC((u32)gUnk_0861C744, 6, 1);
    return count;
}
#endif
// @ 0x0801D378
// 单条 OAM 条目装配: 按 arg0->headA 的图形头把一条 32x8 OBJ 写入 gOamBuffer[index], 返回 index-1
// (与 sub_801D984 同构, 差异在于数据源是 BattleObj 图形头而非 gUnk_03000670 弹出项)。
//   - pal: kindFlags bit15 (0x8000=装载完成后清除) 置位时取 f_2F (图形装载完成回填的最终调色板槽),
//     否则取 palSlot (装配前指定的槽号)。
//   - VPos = posY-5, HPos = posX-0x10 (9-bit 位域由 OAM 结构体自动截断到 & 0x1FF)。
//   - ObjMode = kindFlags bit4 (半透明/混色请求) != 0 → 1。
//   - Shape=1/Size=1 → 32x8; CharNo = slot<=0xA ? 0x140 : 0x12E; Priority=3; Pltt=pal。
// 注意: 位域的逐字段赋值顺序与 ObjMode 的布尔化写法 (而非 (kindFlags>>4)&1) 是字节匹配所需,
//       改写成算术掩码链会改变常量物化形状 (movs #1 复用 vs movs #13/negs)。
u8 sub_801D378(BattleObj *arg0, u8 index)
{
    u8 pal;
    GameOamData *o;

    if (arg0->headA.kindFlags & 0x8000)
        pal = arg0->headA.f_2F;
    else
        pal = arg0->headA.palSlot;

    o = &gOamBuffer[index];
    o->fields.VPos = arg0->posY - 5;
    o->fields.AffineMode = 0;
    o->fields.ObjMode = (arg0->headA.kindFlags & 0x10) ? 1 : 0;
    o->fields.Mosaic = 0;
    o->fields.ColorMode = 0;
    o->fields.Shape = 1;
    o->fields.HPos = arg0->posX - 0x10;
    o->fields.AffineParamNo_L = 0;
    o->fields.HFlip = 0;
    o->fields.VFlip = 0;
    o->fields.Size = 1;
    o->fields.CharNo = arg0->slot <= 0xA ? 0x140 : 0x12E;
    o->fields.Priority = 3;
    o->fields.Pltt = pal;

    index--;
    return index;
}
// @ 0x0801D468
void sub_801D468(void)
{
    u8 slots1[5];
    u8 slots2[7];
    u8 slots[12];
    u8 *pool;
    u8 i;
    u8 j;
    u8 count1;
    u8 count2;

    j = 0;
    pool = (u8 *)GetObjPool();
    count1 = sub_80489E8(pool, slots1, 0, 0xE3);
    count2 = sub_80489E8(pool, slots2, 1, 0xE3);
    sub_8048ACC(slots1, count1, 7);
    sub_8048ACC(slots2, count2, 7);
    if (sub_80187B4() & 0x1000)
    {
        sub_80187D4(0x1000);
    }
    else
    {
        for (i = 0; i < count1; i++)
        {
            slots[j] = slots1[i];
            j++;
        }
    }
    for (i = 0; i < count2; i++)
    {
        slots[j] = slots2[i];
        j++;
    }
    sub_8048ACC(slots, j, 7);
    for (i = 0; i < j; i++)
        gUnk_03000638[i] = pool + slots[i] * 0xC8;
    gUnk_03000669 = 0;
    gUnk_03000668 = j;
}
// @ 0x0801D568
/* 伤害数字弹出 (战斗结算把伤害写入 obj->dmgAmount 后, 由 sub_801DEDC/sub_801DF90/
 * sub_8020B04 触发): 槽位 tile 基号 = gDmgPopupSlot*4+0x158。数值夹 ≤999 拆百/十/个,
 * 前导零以 0xFFFF 哨兵抑制 ((s16) 读为 -1 → 字体表空块表项 39); 每块 DMA3 32B 到
 * OBJ VRAM 0x06010000+槽*32 并忙等, 字体基址从 gUnk_0861A004 运行时折叠 -0x60
 * (= gUnk_08619FA4, 表项 39..49)。尾部把 {dmgAmount 原始值, posX-16, posY-8} 记入
 * gUnk_03000670[count] 并 count++ (渲染 sub_801D984, 回收 sub_801DAA0)。
 * 注意: case 1 的具名下标 idx 与字体基址的 -0x60 折叠写法为字节匹配所需, 勿改。 */
void sub_801D568(BattleObj *arg0)
{
    int idx;
    u16 digits[3];
    u16 slot;
    u16 num;
    u16 i;

    slot = gDmgPopupSlot * 4 + 0x158;
    num = arg0->dmgAmount;
    if (num > 999)
        num = 999;
    digits[0] = num / 100;
    digits[1] = (num - (s16)digits[0] * 100) / 10;
    digits[2] = num - ((s16)digits[0] * 100 + (s16)digits[1] * 10);

    DmaCopy32(3, gUnk_0861A004, 0x06010000 + slot * 32, 32);
    DmaWait(3);
    slot++;

    for (i = 0; i <= 2; i++)
    {
        switch (i)
        {
        case 0:
            if ((s16)digits[0] == 0)
                digits[0] = 0xFFFF | digits[0];
            break;
        case 1:
            idx = 1;
            if ((s16)digits[idx] == 0 && (s16)digits[0] <= 0)
                digits[idx] = 0xFFFF | digits[idx];
            break;
        case 2:
            break;
        }
        DmaCopy32(3, gUnk_0861A004 - 0x60 + ((s16)digits[i] + 40) * 32, 0x06010000 + (slot + i) * 32, 32);
        DmaWait(3);
    }

    gUnk_03000670[gDmgPopupSlot].field_0 = arg0->dmgAmount;
    gUnk_03000670[gDmgPopupSlot].field_2 = arg0->posX - 16;
    gUnk_03000670[gDmgPopupSlot].field_3 = arg0->posY - 8;
    gDmgPopupSlot++;
}

// @ 0x0801D710
// 弹出数字重绘/重登记 (VRAM 重载后由 sub_801DE44 清零 gDmgPopupSlot 后逐对象调用):
// kind==0 重画 arg0->f_B2 的 3 位十进制 (前导零以 0xFFFF 哨兵→字体表空块表项39 抑制;
// digits 的读均用 (s16) 视图使哨兵=-1, |= 的读-改-写保持 u16; 字体基址在源码里从
// gUnk_0861A004 运行时折叠 -0x60)。kind!=0 画固定图形 [空][空][表项12][表项2]
// (tile2 源 = gUnk_0861A484-0x360, tile3 = gUnk_08619FE4)。两路都把 {f_B2 原始值,
// f_BF-16, f_C0-8} 重新记入 gUnk_03000670[count] 并 count++。消费者 sub_801D984
// 渲染为 32x8 OBJ (CharNo = 0x158+4*i), sub_801DAA0 计时回收。
// 注意: case 1 的具名下标 idx 是必须的 —— 常量下标会改变 IR 伪寄存器清单,
// 全局性地改变寄存器 home (合并候选 permuter/sub_801D710/candidates/final_readable.c)。
void sub_801D710(BattleObj *arg0, u8 kind)
{
    int idx;
    u16 digits[3];
    u16 slot;
    u16 num;
    u16 i;

    slot = gDmgPopupSlot * 4 + 0x158;
    num = arg0->dmgAmount;
    if (kind == 0)
    {
        if (num > 999)
            num = 999;
        digits[0] = num / 100;
        digits[1] = (num - (s16)digits[0] * 100) / 10;
        digits[2] = num - ((s16)digits[0] * 100 + (s16)digits[1] * 10);

        DmaCopy32(3, gUnk_0861A004, 0x06010000 + slot * 32, 32);
        DmaWait(3);
        slot++;

        for (i = 0; i <= 2; i++)
        {
            switch (i)
            {
            case 0:
                if ((s16)digits[0] == 0)
                    digits[0] = 0xFFFF | digits[0];
                break;
            case 1:
                idx = 1;
                if ((s16)digits[idx] == 0 && (s16)digits[0] <= 0)
                    digits[idx] = 0xFFFF | digits[idx];
                break;
            case 2:
                break;
            }
            DmaCopy32(3, gUnk_0861A004 - 0x60 + ((s16)digits[i] + 40) * 32, 0x06010000 + (slot + i) * 32, 32);
            DmaWait(3);
        }
    }
    else
    {
        DmaCopy32(3, gUnk_0861A484, 0x06010000 + slot * 32, 32);
        DmaWait(3);
        DmaCopy32(3, gUnk_0861A484, 0x06010000 + (slot + 1) * 32, 32);
        DmaWait(3);
        DmaCopy32(3, gUnk_0861A484 - 0x360, 0x06010000 + (slot + 2) * 32, 32);
        DmaWait(3);
        DmaCopy32(3, gUnk_08619FE4, 0x06010000 + (slot + 3) * 32, 32);
        DmaWait(3);
    }

    gUnk_03000670[gDmgPopupSlot].field_0 = arg0->dmgAmount;
    gUnk_03000670[gDmgPopupSlot].field_2 = arg0->posX - 16;
    gUnk_03000670[gDmgPopupSlot].field_3 = arg0->posY - 8;
    gDmgPopupSlot++;
}
// @ 0x0801D984
u8 sub_801D984(u8 arg)
{
    u8 r6;
    u8 r7;
    s8 i;
    u16 tmp;

    r6 = arg;
    if (gDmgPopupSlot != 0)
    {
        u8 e = gDmgPopupLevel;
        tmp = (u16)(-((0x14 - e * 2) / e));
        if (e <= 2)
        {
            r7 = (u8)sub_801768C(0, (s16)tmp, 9 - e, gDmgPopupPhase * 2, 3);
        }
        else
        {
            r7 = 0;
        }
    }

    for (i = 0; i < gDmgPopupSlot; i++)
    {
        GameOamData *o = &gOamBuffer[r6];
        u32 t;

        t = gUnk_03000670[i].field_3 + r7;
        o->fields.VPos = t;
        o->fields.AffineMode = 0;
        o->fields.ObjMode = 0;
        o->fields.Mosaic = 0;
        o->fields.ColorMode = 0;
        o->fields.Shape = 1;
        o->fields.HPos = gUnk_03000670[i].field_2;
        o->fields.AffineParamNo_L = 0;
        o->fields.HFlip = 0;
        o->fields.VFlip = 0;
        o->fields.Size = 1;
        o->fields.CharNo = i * 4 + 0x158;
        o->fields.Priority = 0;
        o->fields.Pltt = 0xF;

        r6--;
    }
    return r6;
}
// @ 0x0801DAA0
u32 sub_801DAA0(void)
{
    u32 ret;
    u8 i;

    ret = 0;
    if (gDmgPopupLevel < 3)
    {
        gDmgPopupPhase = (gDmgPopupPhase + 1) % (10 - gDmgPopupLevel);
        if (gDmgPopupPhase >= 9 - gDmgPopupLevel)
            gDmgPopupLevel = gDmgPopupLevel + 1;
    }
    else if (gDmgPopupLevel <= 0x22)
    {
        gDmgPopupLevel = gDmgPopupLevel + 1;
    }
    else
    {
        ret = 1;
        gDmgPopupSlot = 0;
        gDmgPopupLevel = 1;
        gDmgPopupPhase = 0;
        for (i = 0; i < 7; i++)
        {
            gUnk_03000670[i].field_0 = 0;
            gUnk_03000670[i].field_2 = 0;
            gUnk_03000670[i].field_3 = 0;
        }
        sub_804C2FC((u32)gUnk_0861C664, 0xF, 1);
        sub_804C3A4(0xF, 1);
    }
    return ret;
}
typedef struct Unk_0839B2B0
{
    u32 field_0;
    u32 field_4;
    u16 field_8;
    u16 field_A;
} Unk_0839B2B0; /* 12 字节 */

extern Unk_0839B2B0 gUnk_0839B2B0[];

// @ 0x0801DB3C
void sub_801DB3C(BattleObj *arg0, u8 arg1, u16 arg2)
{
    u8 delta;
    u16 newval;
    Unk_0839B2B0 *t1;
    const ObjAnimEntry *t2;

    if (arg2 <= 2)
    {
        if (arg0->slot <= 0xA)
            delta = 0x10;
        else
            delta = (u8)sub_801EC3C(arg0, 1) >> 1;
        t1 = &gUnk_0839B2B0[arg2];
        sub_801B81C(&arg0->headB, arg0->posX, (u8)(arg0->posY - delta), 0xAD << 2, 0xE,
                    t1->field_0, t1->field_4 + (arg1 << 5), (u16)(0x543 + t1->field_8), t1->field_A, 4);
    }
    else
    {
        t2 = &gUnk_08393B28[arg2];
        sub_801B81C(&arg0->headB, arg0->posX, arg0->posY, 0xC0 << 2, 0xE,
                    (u32)t2->animScriptPtr, (u32)t2->palettePtr, t2->gfxBaseIdx, t2->gfxTotal, 4);
    }
    arg0->headB.f_2A = 3;
    newval = 0x2000 | arg0->state;
    arg0->state = newval;
}

// @ 0x0801DC20
void sub_801DC20(BattleObj *arg0, u8 arg1)
{
    u8 buf[8];
    u8 *pool;
    u8 count;
    u8 i;
    sub_8048D40((BattleObj *)arg0);
    pool = GetObjPool();
    count = sub_80489E8(pool, buf, 0, 0x7F);
    for (i = 0; i < count; i++)
    {
        if (arg0->slot == pool[buf[i] * 0xC8 + 0xBE])
            break;
    }
    gTaskPoolNodes[buf[i]].data = (u32)arg0;
    ListNode_InitKey((UnkNode *)&gTaskPoolNodes[buf[i]], arg1);
    ListNode_InsertSorted(&gTaskPoolHead, (UnkNode *)&gTaskPoolNodes[buf[i]]);
    sub_8045F94((BattleObj *)arg0, 8);
    arg0->dmgAmount = 0;
    BattleFxObjs_Add(arg0);
    if (arg0->slot <= 6)
    {
        *(u16 *)&arg0->animPtr = 0;
        arg0->state |= 2;
    }
    gTaskPoolCount += 1;
}


/* 从对象排序链表 (0x030006A0) 摘除节点, 清对象字段并按 field_BE 分派:
 * ≤0xA → sub_801CBA4, ≤0x70 → sub_801CA08, 其余 (field_BE-0x71 ≤ 0x8D) → sub_801CE80,
 * 均传 (obj, 0, idx*22+9, (u8)(idx+1), 0); 末尾 sub_801D12C(obj,0)。 */
// @ 0x0801DD04
void sub_801DD04(BattleObj *obj, u8 idx, u16 val)
{
    u16 f2a;
    u8 f35;
    /* 摘链用 u32 字指针而非结构体指针: GCC2 别名集按类型划分, 结构体字段存储与
     * 指针标量读取判为无冲突会省掉目标中的两次重载 (经验 150)。 */
    u8 *arr = (u8 *)gTaskPoolNodes;
    u32 off = idx * 0x10;
    u8 *p = (u8 *)arr + 4;
    u32 *pp = (u32 *)(p + off);
    u32 prev = *pp; /* 先读 prev, 位置在 np 计算前 */
    u8 *q = (u8 *)arr + 8;
    u32 *np = (u32 *)(q + off);

    ((u32 *)prev)[2] = *np; /* prev->next = node->next */
    ((u32 *)*np)[1] = *pp;  /* next->prev = node->prev; 存储后重读, GCC2 按 u32 别名集阻塞 CSE */
    *pp = 0;
    *np = 0;

    obj->variantClass = 0;
    obj->dmgAmount = 0;
    obj->hp = val;
    obj->fxKind = 0;

    if (gTaskPoolCount)
        gTaskPoolCount--;

    f2a = idx * 0x16 + 9;
    f35 = (u8)(idx + 1);

    if (obj->slot <= 0xA)
        sub_801CBA4(obj, 0, f2a, f35, 0);
    else if (obj->slot <= 0x70)
        sub_801CA08(obj, 0, f2a, f35, 0);
    else if ((u8)(obj->slot - 0x71) <= 0x8D)
        sub_801CE80(obj, 0, f2a, f35, 0);

    sub_801D12C(obj, 0);
}
typedef struct UnkT_0839B2D4
{
    u32 field_0;
    u32 field_4;
    u16 field_8;
    u16 field_A;
} UnkT_0839B2D4;
extern UnkT_0839B2D4 gUnk_0839B2D4;

// @ 0x0801DDB0
void sub_801DDB0(BattleObj *arg0, u8 arg1)
{
    UnkT_0839B2D4 *tbl = &gUnk_0839B2D4;
    u16 nv;
    sub_801B81C(&arg0->headB, 0x78, 0x50, 0xDA << 1, 0xE, tbl->field_0,
                tbl->field_4 + (arg1 << 5), (u16)(0x549 + tbl->field_8), tbl->field_A, 0x402);
    arg0->headB.f_2A = 3;
    nv = 0x80 | arg0->headB.kindFlags;
    arg0->headB.kindFlags = nv;
    nv = 0x2000 | arg0->state;
    arg0->state = nv;
    REG_DISPCNT |= 0x8000;
    REG_WINOUT |= 0x1400;
}

extern u8 gUnk_03000730_arr[]; // gUnk_03000730 的字节视图
// @ 0x0801DE44
void sub_801DE44(void)
{
    u8 i;
    u8 j;
    u8 flag;
    u32 ptr;

    gDmgPopupSlot = 0;
    gDmgPopupLevel = 1;
    gDmgPopupPhase = 0;
    for (i = 0; i < 7; i++)
    {
        gUnk_03000670[i].field_0 = 0;
        gUnk_03000670[i].field_2 = 0;
        gUnk_03000670[i].field_3 = 0;
    }
    sub_804C2FC((u32)gUnk_0861C664, 0xF, 1);
    ptr = GetObjPool();
    for (j = 0; j < gUnk_0300073D; j++)
    {
        flag = 0;
        if ((gUnk_03000730_arr[j] & 0xF0) == 0)
            flag = 1;
        sub_801D710(ptr + (gUnk_03000730_arr[j] & 0xF) * 0xC8, flag);
    }
}
// @ 0x0801DEDC
void sub_801DEDC(BattleObj *arg0, BattleObj *arg1)
{
    const ObjAnimEntry *entry;
    u8 *anim;
    int off;
    u16 sub;
    s8 kind = arg0->fxKind;
    switch (kind)
    {
    case 0:
        off = *(u16 *)(arg0->animPtr + 0x1A) + 3;
        sub = off;
        entry = &gUnk_08393B28[sub];
        break;
    case 1:
        anim = arg0->animPtr;
        off = *(u16 *)(anim + 0x1A) + 3;
        anim += 0x29;
        off = anim[0] + off;
        sub = off;
        entry = &gUnk_08393B28[sub];
        break;
    }
    switch (entry->targetMode)
    {
    case 0:
        sub_801D568(arg1);
        gDmgPopupPhase = 0;
        break;
    case 1:
    {
        u8 kindBE = arg1->slot;
        u8 count = 7;
        u8 i;
        if (kindBE <= 0xA)
            count = 5;
        for (i = 0; i < count; i++)
        {
            BattleObj *p = (BattleObj *)(i * 0xC8 + (u32)arg1);
            if (p->slot != 0xFF && p->variantClass != 8)
                sub_801D568(p);
        }
        gDmgPopupPhase = 0;
        break;
    }
    }
}
// @ 0x0801DF90
void sub_801DF90(BattleObj *arg0, BattleObj *arg1)
{
    const ObjAnimEntry *entry;
    int off;
    u8 *anim;
    s8 kind = arg0->fxKind;
    switch (kind)
    {
    case 0:
        entry = &gUnk_08393B28[*(u16 *)(arg0->animPtr + 2)];
        break;
    case 1:
        anim = arg0->animPtr;
        off = arg0->animSubIdx * 2;
        anim += 8;
        entry = &gUnk_08393B28[*(u16 *)(anim + off)];
        break;
    }
    switch (entry->targetMode)
    {
    case 0:
        sub_801D568(arg1);
        gDmgPopupPhase = 0;
        break;
    case 1:
    {
        u8 kindBE = arg1->slot;
        u8 count = 7;
        u8 i;
        if (kindBE <= 0xA)
            count = 5;
        for (i = 0; i < count; i++)
        {
            BattleObj *p = (BattleObj *)(i * 0xC8 + (u32)arg1);
            if (p->slot != 0xFF && p->variantClass != 8)
                sub_801D568(p);
        }
        gDmgPopupPhase = 0;
        break;
    }
    }
}
// @ 0x0801E040
u8 sub_801E040(void)
{
    u8 ret = 0;
    if (gFxQueueReadIdx < gFxQueueWriteIdx)
    {
        BattleObj *obj = gFxQueueObjs[gFxQueueReadIdx];
        u8 *st = &obj->slot;
        if (*st <= 0xA)
        {
            sub_801DC20(obj, 0);
            if (*st <= 0xA)
                sub_801CBA4(obj, 3, obj->headA.f_1E, obj->headA.palSlot, ret);
        }
        else if (*st <= 0x70)
        {
            sub_801D12C(obj, 0);
            if (*st <= 0xA)
                sub_801CBA4(obj, 2, obj->headA.f_1E, obj->headA.palSlot, ret);
            else if (*st <= 0x70)
                sub_801CA08(obj, 1, obj->headA.f_1E, obj->headA.palSlot, ret);
            else if ((u8)(*st - 0x71) <= 0x8D)
                sub_801CE80(obj, 2, obj->headA.f_1E, obj->headA.palSlot, ret);
            gUnk_03000630--;
            {
                u16 newval = 4 | obj->state;
                obj->state = newval;
            }
            sub_8045F94((BattleObj *)obj, 8);
        }
        else
        {
            sub_801D12C(obj, 0);
            if (*st <= 0xA)
                sub_801CBA4(obj, 2, obj->headA.f_1E, obj->headA.palSlot, ret);
            else if (*st <= 0x70)
                sub_801CA08(obj, 1, obj->headA.f_1E, obj->headA.palSlot, ret);
            else if ((u8)(*st - 0x71) <= 0x8D)
                sub_801CE80(obj, 2, obj->headA.f_1E, obj->headA.palSlot, ret);
            gUnk_03000630--;
            {
                u16 newval = 4 | obj->state;
                obj->state = newval;
            }
            sub_8045F94((BattleObj *)obj, 8);
        }
        gFxQueueReadIdx++;
    }
    else
    {
        if (gFxQueueWriteIdx != 0)
        {
            u8 s = gFxQueueObjs[0]->slot;
            if (s <= 0xA)
                ret = 1;
            else if (s <= 0x70)
                ret = 2;
            else
            {
                sub_8044414();
                ret = 3;
            }
        }
        gFxQueueReadIdx = 0;
    }
    return ret;
}
// @ 0x0801E1D8
INCLUDE_ASM("asm/nonmatchings", sub_801E1D8);
// @ 0x0801E30C
INCLUDE_ASM("asm/nonmatchings", sub_801E30C);
// @ 0x0801E4D4
/* 战斗效果入队 (等价已匹配的 sub_8020B90): 写队列槽 gFxQueueObjs[count] 后 count++,
 * 对象类型 > 0xB 时同时登记 gUnk_03000718。原 ROM 在 sub_801E4D4/801E30C/801E690 三处
 * 内联展开本逻辑; 因 GCC2 不会内联定义在后面的函数, 此处用 static inline 复现内联形状
 * (全部调用点被集成, 不产生独立副本, 不影响段布局)。 */
static inline void Inl_QueuePushObj(BattleObj *obj)
{
    gFxQueueObjs[gFxQueueWriteIdx] = obj;
    gFxQueueWriteIdx++;
    if (obj->slot > 0xB)
    {
        gUnk_03000718 = (u32)obj;
    }
}

/* 由 arg0[0xBC] 选择的效果查表 (gUnk_08393B28[idx].targetMode) 决定处理模式:
 * 0 → 对 arg1 单体做 [0x6C] -= [0xB2] (下溢清零), 回绕则入队;
 * 1 → 对 arg1 起始的 0xC8 步长成员数组 (5 或 7 个, 按 arg1[0xBE] 分档) 逐个执行同一扣减;
 * 其余 → 无操作。任一回绕标志置位则返回 1。注意本函数在 ROM 中无任何调用者(死代码)。 */
u32 sub_801E4D4(BattleObj *arg0, BattleObj *arg1)
{
    u8 flags[7];
    const ObjAnimEntry *entry;
    u16 idx;
    s32 t;
    u32 result;
    u32 limit;
    u8 wrapped;
    u8 i;

    result = 0;
    for (i = 0; i <= 6; i++)
        flags[i] = 0;

    switch ((s8)arg0->fxKind)
    {
    case 0:
        idx = *(u16 *)(arg0->animPtr + 0x1A) + 3;
        entry = &gUnk_08393B28[idx];
        break;
    case 1:
        t = *(u16 *)(arg0->animPtr + 0x1A) + 3;
        idx = t + *(u8 *)(arg0->animPtr + 0x29);
        entry = &gUnk_08393B28[idx];
        break;
    }

    switch (*(u16 *)((const u8 *)entry + 0x10))
    {
    case 0:
        if (*(s16 *)&arg1->hp - *(s16 *)&arg1->dmgAmount <= 0)
        {
            arg1->hp = *(u16 *)((const u8 *)entry + 0x10);
            wrapped = 1;
        }
        else
        {
            arg1->hp -= arg1->dmgAmount;
            wrapped = 0;
        }
        flags[0] = wrapped;
        if (wrapped != 0)
        {
            Inl_QueuePushObj(arg1);
        }
        break;
    case 1:
        limit = (arg1->slot > 0xA) ? 7 : 5;
        for (i = 0; i < limit; i++)
        {
            BattleObj *member = (BattleObj *)(i * 0xC8 + (u32)arg1);
            if (member->slot == 0xFF)
                continue;
            if (member->variantClass == 8)
                continue;
            if (*(s16 *)&member->hp - *(s16 *)&member->dmgAmount <= 0)
            {
                member->hp = 0;
                wrapped = 1;
            }
            else
            {
                member->hp -= member->dmgAmount;
                wrapped = 0;
            }
            flags[i] = wrapped;
            if (wrapped != 0)
            {
                Inl_QueuePushObj((BattleObj *)(i * 0xC8 + (u32)arg1));
            }
        }
        break;
    }

    i = 0;
    if (flags[0] == 1)
        result = flags[0];
    else
    {
        while (++i <= 6)
        {
            if (flags[i] == 1)
            {
                result = flags[i];
                break;
            }
        }
    }
    return result;
}
// @ 0x0801E690
/* 同 sub_801E4D4 的效果扣减引擎 (arg0[0xBC]→animPtr 查 gUnk_08393B28→targetMode 模式分派),
 * 差异仅在查表入口: mode0=*(u16*)(animPtr+2), mode1=*(u16*)(animPtr+8+animSubIdx*2)
 * (即 animPtr+0x3A 处的 u8 索引; 与已匹配 sub_801DF90 的 dispatch 字节同构)。
 * ROM 中无任何调用者(死代码)。 */
u32 sub_801E690(BattleObj *arg0, BattleObj *arg1)
{
    u8 flags[7];
    const ObjAnimEntry *entry;
    int off;
    u8 *anim;
    u32 result;
    u32 limit;
    u8 wrapped;
    u8 i;

    result = 0;
    for (i = 0; i <= 6; i++)
        flags[i] = 0;

    switch ((s8)arg0->fxKind)
    {
    case 0:
        entry = &gUnk_08393B28[*(u16 *)(arg0->animPtr + 2)];
        break;
    case 1:
        anim = arg0->animPtr;
        off = arg0->animSubIdx * 2;
        anim += 8;
        entry = &gUnk_08393B28[*(u16 *)(anim + off)];
        break;
    }

    switch (*(u16 *)((const u8 *)entry + 0x10))
    {
    case 0:
        if (*(s16 *)&arg1->hp - *(s16 *)&arg1->dmgAmount <= 0)
        {
            arg1->hp = *(u16 *)((const u8 *)entry + 0x10);
            wrapped = 1;
        }
        else
        {
            arg1->hp -= arg1->dmgAmount;
            wrapped = 0;
        }
        flags[0] = wrapped;
        if (wrapped != 0)
        {
            Inl_QueuePushObj(arg1);
        }
        break;
    case 1:
        limit = (arg1->slot > 0xA) ? 7 : 5;
        for (i = 0; i < limit; i++)
        {
            BattleObj *member = (BattleObj *)(i * 0xC8 + (u32)arg1);
            if (member->slot == 0xFF)
                continue;
            if (member->variantClass == 8)
                continue;
            if (*(s16 *)&member->hp - *(s16 *)&member->dmgAmount <= 0)
            {
                member->hp = 0;
                wrapped = 1;
            }
            else
            {
                member->hp -= member->dmgAmount;
                wrapped = 0;
            }
            flags[i] = wrapped;
            if (wrapped != 0)
            {
                Inl_QueuePushObj((BattleObj *)(i * 0xC8 + (u32)arg1));
            }
        }
        break;
    }

    i = 0;
    if (flags[0] == 1)
        result = flags[0];
    else
    {
        while (++i <= 6)
        {
            if (flags[i] == 1)
            {
                result = flags[i];
                break;
            }
        }
    }
    return result;
}
// @ 0x0801E848
INCLUDE_ASM("asm/nonmatchings", sub_801E848);
// @ 0x0801EA70
/* 战斗对象目标选取 (战斗阶段状态机 sub_801BE34 / sub_801C484 的 case 1 调用:
 *   sub_801EA70(gUnk_03000638[gUnk_03000669], pool))。
 * arg0 = 行动对象 (obj 池内一项; 调用点取自 gUnk_03000638[] 指针表),
 * arg1 = obj 池基址 (GetObjPool(), 元素步长 0xC8)。
 *
 * 结果写入 obj->f_BD = 目标对象的池索引; sub_801F884/802192C/804E2AC 再按
 * pool + f_BD*0xC8 取目标对象。
 *
 * 顶层按槽号分流:
 *  - slot > 0xA (非玩家侧 0-0xA): 转交其他引擎 —
 *      slot <= 0x70 走 sub_804CEE0, 否则 (特效/特殊槽) 走 sub_804DD70。
 *  - slot <= 0xA: 按 sub_801F76C(obj) 的动作类别重选目标 (case 0-3):
 *      case 0/3 先看当前目标 pool[f_BD]: 若已是玩家侧有效成员 (case 3 还要求
 *        它是死亡/空槽形态), 且 obj->variantClass == 5 (同组形态), 就在 pool[0..4]
 *        中找与 obj->slot 相同的同伴位次, 交给 sub_804C8E0 剔除该位次后随机取回
 *        一个候选; 否则用 sub_80489E8 收集候选池后随机挑一个
 *        (mode 1 = 玩家侧 0..4, mode 0 = 其余)。
 *      case 1: gGstate324 (sub_80187B4) bit14 置位时放宽到 variantClass ∈ {7,8},
 *        否则只认 7; case 2: 只认 7。通过后按 mode 0 随机挑候选。
 * 随机索引 = (u8)Rng_LcgNext() % 候选数。 */
extern u32 __umodsi3(u32, u32);

void sub_801EA70(BattleObj *obj, BattleObj *pool)
{
    u8 buf[8];
    u8 i;
    u8 n;

    if (obj->slot <= 0xA)
    {
        switch (sub_801F76C(obj))
        {
        case 0:
            if (pool[obj->f_BD].slot <= 0xA)
            {
                if (pool[obj->f_BD].variantClass == 8)
                {
                    if (obj->variantClass == 5)
                    {
                        for (i = 0; i <= 4; i++)
                        {
                            if (pool[i].slot == obj->slot)
                            {
                                obj->f_BD = sub_804C8E0(pool, i);
                                return;
                            }
                        }
                        return;
                    }
                    n = sub_80489E8(pool, buf, 0, 0x7F);
                    obj->f_BD = buf[(u8)Rng_LcgNext() % n];
                    return;
                }
                if (obj->variantClass == 5)
                    return;
                n = sub_80489E8(pool, buf, 1, 0x7F);
                obj->f_BD = buf[(u8)Rng_LcgNext() % n];
                return;
            }
            if (pool[obj->f_BD].slot != 0xFF)
                return;
            if (obj->variantClass != 5)
            {
                n = sub_80489E8(pool, buf, 1, 0x7F);
                obj->f_BD = buf[(u8)Rng_LcgNext() % n];
                return;
            }
            for (i = 0; i <= 4; i++)
            {
                if (pool[i].slot == obj->slot)
                {
                    obj->f_BD = sub_804C8E0(pool, i);
                    return;
                }
            }
            return;

        case 1:
            if (sub_80187B4() & 0x4000)
            {
                if (pool[obj->f_BD].variantClass != 7 && pool[obj->f_BD].variantClass != 8)
                    return;
                n = sub_80489E8(pool, buf, 0, 0x7F);
                obj->f_BD = buf[(u8)Rng_LcgNext() % n];
                return;
            }
            if (pool[obj->f_BD].variantClass != 7)
                return;
            n = sub_80489E8(pool, buf, 0, 0x7F);
            obj->f_BD = buf[(u8)Rng_LcgNext() % n];
            return;

        case 2:
            if (pool[obj->f_BD].variantClass != 7)
                return;
            n = sub_80489E8(pool, buf, 0, 0x7F);
            obj->f_BD = buf[(u8)Rng_LcgNext() % n];
            return;

        case 3:
            if (pool[obj->f_BD].slot <= 0xA)
            {
                if (pool[obj->f_BD].variantClass != 8 && pool[obj->f_BD].slot != 0xFF)
                    return;
                if (obj->variantClass != 5)
                {
                    n = sub_80489E8(pool, buf, 0, 0x7F);
                    obj->f_BD = buf[(u8)Rng_LcgNext() % n];
                    return;
                }
                for (i = 0; i <= 4; i++)
                {
                    if (pool[i].slot == obj->slot)
                    {
                        obj->f_BD = sub_804C8E0(pool, i);
                        return;
                    }
                }
                return;
            }
            if (pool[obj->f_BD].slot != 0xFF)
                return;
            n = sub_80489E8(pool, buf, 0, 0x7F);
            obj->f_BD = buf[(u8)Rng_LcgNext() % n];
            return;
        }
        return;
    }

    if (obj->slot <= 0x70)
        sub_804CEE0(obj, pool);
    else
        sub_804DD70((BattleObj *)obj, (u32)pool);
}
// @ 0x0801EC3C
u32 sub_801EC3C(BattleObj *obj, u8 arg1)
{
    u8 result = 0x20;
    if ((u8)(obj->slot - 0xC) <= 0x64)
    {
        switch (arg1)
        {
        case 0:
            result = gUnk_08393A3C[*(u16 *)(obj->animPtr + 0x20)] * 8;
            break;
        case 1:
            result = gUnk_08393A40[*(u16 *)(obj->animPtr + 0x20)] * 8;
            break;
        }
    }
    else if (obj->slot > 0x71)
    {
        switch (obj->slot - 0x71)
        {
        case 1:
            result = arg1 == 0 ? 8 : 6;
            break;
        case 2:
            result = arg1 == 0 ? 6 : 8;
            break;
        case 4:
            result = arg1 == 0 ? 0xB : 0xE;
            break;
        case 5:
            result = arg1 == 0 ? 0xB : 0xD;
            break;
        case 7:
            result = 4;
            break;
        case 8:
            result = arg1 == 0 ? 0xB : 0xD;
            break;
        case 9:
        case 10:
            result = arg1 == 0 ? 0xA : 8;
            break;
        case 3:
        case 6:
        case 12:
        case 13:
        case 14:
            result = 8;
            break;
        case 15:
            result = 9;
            break;
        case 0:
        case 11:
        case 16:
        default:
            result = 6;
            break;
        }
        result = result * 8;
    }
    return result;
}

#if 1
INCLUDE_ASM("asm/matchings", sub_801ED40);
#else
// @ 0x0801ED40
/* BattleObj 白色着色启动 (sub_801EE6C 的逆操作): 通过 sub_804B654 为对象精灵登记/施加
 * RGB(31,31,31) 白色 tint。color 低 3 字节强制为白色, 未初始化读取的 RMW 是与 ROM
 * 字节一致的必要形态 (GCC2 保留源码级 UB; 高 8 位为栈残留, 消费端不使用)。
 * 分支按 obj->slot:
 *   ≤0xA    : 仅登记 mode=2 tint 记录 (起始记录号 = headA->palSlot, 数量 = headA 字节2);
 *   ≤0x70   : 先 mode=3 即时施加 (palSlot = v+1), 失败 (<0) 回退 mode=2 登记并记录 v;
 *   >0x70   : mode=3 即时施加 (palSlot = 8), 成功才置 headA->kindFlags bit15。
 * 三个全局/字段: gUnk_03000765 = headA->palSlot + 1 (下一调色板槽游标);
 * headA 前序字节 obj[0xAB]==4 时 v 取 gUnk_03000744 (特殊战斗背景槽), 否则取 headA->palSlot;
 * 成功/回退记录的槽号写入 headA->f_2F; kindFlags bit15 与 sub_801EE6C 清除互逆。 */
void sub_801ED40(BattleObj *obj, u8 arg1)
{
    u32 color;
    u8 v;
    u8 ret;
    u16 flags;

    color = (color & 0xFFFFFF00) | 0x1F;
    color = (color & 0xFFFF00FF) | 0x1F00;
    color = (color & 0xFF00FFFF) | 0x1F0000;

    gUnk_03000765 = obj->headA.palSlot + 1;

    if (obj->slot <= 0xA)
    {
        sub_804B654(obj->headA.palSlot, sub_801B954(&obj->headA), &color, arg1, -1, 2);
    }
    else if (obj->slot <= 0x70)
    {
        if (obj->variantClass == 4)
        {
            v = gUnk_03000744;
        }
        else
        {
            v = obj->headA.palSlot;
        }
        ret = sub_804B654(v, sub_801B954(&obj->headA), &color, arg1, (s8)(v + 1), 3);
        if ((s8)ret >= 0)
        {
            obj->headA.f_2F = ret;
        }
        else
        {
            sub_804B654(v, sub_801B954(&obj->headA), &color, arg1, -1, 2);
            obj->headA.f_2F = v;
        }
        flags = 0x8000 | obj->headA.kindFlags;
        obj->headA.kindFlags = flags;
    }
    else
    {
        ret = sub_804B654(obj->headA.palSlot, sub_801B954(&obj->headA), &color, arg1, 8, 3);
        if ((s8)ret >= 0)
        {
            obj->headA.f_2F = ret;
            flags = 0x8000 | obj->headA.kindFlags;
            obj->headA.kindFlags = flags;
        }
    }
}
#endif
// @ 0x0801EE6C
/* BattleObj 白色着色清除 (sub_801ED40 的逆操作): 撤销对象精灵的白色 tint。
 * v 槽基址与 sub_801ED40 同源 (slot>0x0B 且 variantClass==4 → gUnk_03000744, 否则 headA->palSlot);
 * sub_804B7B0(v, headA 字节2) 清除 tint 记录; kindFlags bit15 若在则清 0;
 * slot==0x77 额外 sub_804B834(palSlot,1,3,-11,5) (特殊背景的补充恢复)。 */
void sub_801EE6C(BattleObj *ptr)
{
    u8 v;
    u8 b;

    if (ptr->slot > 0x0B && ptr->variantClass == 4)
    {
        v = gUnk_03000744;
    }
    else
    {
        v = ptr->headA.palSlot;
    }

    b = sub_801B954(&ptr->headA);
    sub_804B7B0(v, b);

    if (ptr->headA.kindFlags & 0x8000)
        ptr->headA.kindFlags &= 0x7FFF;

    if (ptr->slot == 0x77)
        sub_804B834(ptr->headA.palSlot, 1, 3, -11, 5);
}
// @ 0x0801EEE4
INCLUDE_ASM("asm/nonmatchings", sub_801EEE4);
// @ 0x0801F3FC
INCLUDE_ASM("asm/nonmatchings", sub_801F3FC);
// @ 0x0801F76C
/* 战斗对象动作类别 (0-3, 唯一调用者 sub_801EA70 按 u8 分派目标重选策略):
 *  - fxKind==1: 取所选技能 id = (0xA1 > 7 ? 0xA1 : skills[0xA1]) (+0x99 槽数组),
 *    再按技能 id 归为三类: {6,8,9,12,13,30,31,32,33,35,49,51}=1,
 *    {10,25,29}=3, {34}=2, 其余=0。
 *  - fxKind==2: 看 +0xA4 值: ==0xE1 → 2, >0xDC → 1, 否则 0。
 *  - 其他 fxKind: 0。
 * 形状: switch(fxKind) 必须带 case 0 空臂, 才能生成目标
 * `cmp#1/beq/cmp#1/bgt/cmp#2/beq` 二分链 (无 case0 时 GCC 折叠成 cmp#1/cmp#2);
 * 技能取值须写成 `(0xA1>7 ? 0xA1 : skills[0xA1])` 并用 struct 数组成员
 * (skills[]), 才会先算数组基址 arg0+0x99 再算索引 —— 这正是 sub_80489A4/
 * sub_8048764 同族的访问形状。 */
u8 sub_801F76C(BattleObj *obj)
{
    u8 r3 = 0;
    s32 f = (s8)obj->fxKind;
    u8 x;

    switch (f)
    {
    case 0:
        break;
    case 1:
        switch (obj->pad_A1 > 7 ? obj->pad_A1 : obj->skills[obj->pad_A1])
        {
        case 6:
        case 8:
        case 9:
        case 12:
        case 13:
        case 30:
        case 31:
        case 32:
        case 33:
        case 35:
        case 49:
        case 51:
            r3 = 1;
            break;
        case 10:
        case 25:
        case 29:
            r3 = 3;
            break;
        case 34:
            r3 = 2;
            break;
        }
        break;
    case 2:
        x = obj->pad_A4[0];

        if (x == 0xE1)
            r3 = 2;
        else if (x > 0xDC)
            r3 = 1;
        break;
    }

    return r3;
}
// @ 0x0801F884
/* 目标对象"匹配键"计算: 取 f_BD 指向的池对象 (步长 0xC8) 的 +0xAC 属性字节,
 * 供 sub_80462E4 按候选槽过滤 (见 sub_801EA70 的目标选取文档)。
 * 按槽号分三类:
 *  - 玩家侧 (slot<=0xA): 直接取该字节; fxKind==1 时先由 sub_80489A4(obj, 0xA1)
 *    取得技能类别 (0-5) 决定: 0/4=原值, 1=低 nibble, 2=高 nibble (==0x20 映射 0x30),
 *    3/5=0。
 *  - 敌方 (0xB..0x70): 由 animPtr+0x1A (fxKind==1 再加 animPtr[0x29]) 查
 *    gUnk_08393B28, 按其 targetMode 模式分派 (0=原值, 1=0, 2=低 nibble ==2 映射 1,
 *    3=高 nibble ==0x20 映射 0x10)。
 *  - 特效/特殊 (>0x70): 表索引改取 animPtr+2 (fxKind==0) 或
 *    animPtr+8+animSubIdx*2 (fxKind==1)。
 * 返回 0 表示调用者不做 nibble 过滤 (复制全部候选槽)。
 * 注: switch case 顺序按 ROM 分派顺序书写 (case1 在前), 与同族 sub_801E4D4 一致。 */
u8 sub_801F884(BattleObj *obj)
{
    u8 result = 0;
    u8 *pool = (u8 *)GetObjPool();
    u16 idx;
    s32 t;
    u8 *anim;
    int off;
    const ObjAnimEntry *entry;

    if (obj->slot <= 0xA)
    {
        switch ((s8)obj->fxKind)
        {
        case 1:
            switch (sub_80489A4((u8 *)obj, obj->pad_A1))
            {
            case 0:
            case 4:
                result = pool[obj->f_BD * 0xC8 + 0xAC];
                break;
            case 1:
                result = pool[obj->f_BD * 0xC8 + 0xAC] & 0xF;
                break;
            case 2:
                result = pool[obj->f_BD * 0xC8 + 0xAC] & 0xF0;
                if (result == 0x20)
                    result = 0x30;
                break;
            case 3:
            case 5:
                result = 0;
                break;
            }
            break;
        case 0:
        case 2:
            result = pool[obj->f_BD * 0xC8 + 0xAC];
            break;
        }
    }
    else
    {
        if (obj->slot <= 0x70)
        {
            switch ((s8)obj->fxKind)
            {
            case 0:
                idx = *(u16 *)(obj->animPtr + 0x1A) + 3;
                entry = &gUnk_08393B28[idx];
                break;
            case 1:
                t = *(u16 *)(obj->animPtr + 0x1A) + 3;
                idx = t + *(u8 *)(obj->animPtr + 0x29);
                entry = &gUnk_08393B28[idx];
                break;
            }
        }
        else
        {
            switch ((s8)obj->fxKind)
            {
            case 0:
                entry = &gUnk_08393B28[*(u16 *)(obj->animPtr + 2)];
                break;
            case 1:
                anim = obj->animPtr;
                off = obj->animSubIdx * 2;
                anim += 8;
                entry = &gUnk_08393B28[*(u16 *)(anim + off)];
                break;
            }
        }
        switch (entry->targetMode)
        {
        case 0:
            result = pool[obj->f_BD * 0xC8 + 0xAC];
            break;
        case 1:
            result = 0;
            break;
        case 2:
            result = pool[obj->f_BD * 0xC8 + 0xAC] & 0xF;
            if (result == 2)
                result = 1;
            break;
        case 3:
            result = pool[obj->f_BD * 0xC8 + 0xAC] & 0xF0;
            if (result == 0x20)
                result = 0x10;
            break;
        }
    }
    return result;
}
// @ 0x0801FA10
void sub_801FA10(BattleObj *obj, u8 kind)
{
    u16 val;
    u8 z;

    val = obj->state & 0xFFF0;
    z = 0;
    val |= kind & 0xF;
    obj->state = val;
    switch (val & 0xF)
    {
    case 1:
        sub_801B81C(&obj->headA, obj->headA.f_2B, obj->headA.f_2C, 0, z, 0x0856B440, 0x08553734, 0, 1, 0x403);
        break;
    case 2:
        sub_801B81C(&obj->headA, obj->headA.f_2B, obj->headA.f_2C, 0, z, 0x0856B494, 0x08553734, 1, 1, 0x403);
        break;
    }
}
// @ 0x0801FAB8
INCLUDE_ASM("asm/nonmatchings", sub_801FAB8);
// 场景对象 (0xC8 字节, 池 0x02037028); 与 MOD-05 共用布局 (见 src/code_8020D50.c)
// @ 0x0801FEBC
/* 对象滑动子状态装配: 清 state bits4-7 后置 0x20, 把本地滑动区间
 * [headA.f_2B, headA.f_2C] 与活动上限 (0xB4 / 0xF) 写入 gUnk_03000618..24
 * 供逐帧滑动消费; dh = 0xB4 - headA.f_2B > 0 时置 headA.kindFlags bit5,
 * 末尾按 kind=1 重装头部。 */
void sub_801FEBC(BattleObj *arg0, u16 arg1, u8 arg2)
{
    s32 dh;

    arg0->state = arg0->state & 0xFF0F;
    arg0->state = 0x20 | arg0->state;
    gUnk_03000618 = arg1;
    gUnk_0300061A = 0;
    gUnk_0300061C = arg0->headA.f_2B;
    gUnk_0300061E = arg0->headA.f_2C;
    gUnk_03000620 = (dh = 0xB4 - arg0->headA.f_2B);
    gUnk_03000622 = 0xF - arg0->headA.f_2C;
    gUnk_03000624 = arg2;
    if (dh > 0)
        arg0->headA.kindFlags |= 0x20;
    sub_801FA10(arg0, 1);
}
// @ 0x0801FF40
/* 战斗对象"待选槽"挑选 (模式 0 = 敌方/杂项, 1 = 玩家侧; 由 sub_802151C 操作码 15/16 调用):
 * 1. 清 buf[0..4];
 * 2. 建一个随机种子 (sub_8018838(Rng_LcgNext()), 再丢弃一次 Rng_LcgNext());
 * 3. 遍历 0x03000690 的对象链 (节点 key<=0xFE): 每个节点数据对象 +0xB2 (dmgAmount) 逐帧 +1
 *    (与 sub_8020AE4 的逐帧累加同一语义), 并让内部计数 i 递增 (+1 经 u8 截断);
 *      - mode==1 → slot = i, 结束;
 *      - 否则按 (n+1)*40 vs rand%101 的概率判定 (n=该对象 dmgAmount), 命中 → slot = i;
 *      - 未命中且 n>4 时把该对象的 memberIdx (+0xBB) 收进 buf;
 *      - 继续下一节点。
 * 4. count=buf 收集数 (r7):
 *      - count!=0: slot = buf[Rng % count], 再在 gTaskPoolNodes (16B/节点, .data 指向对象) 里
 *        找 memberIdx==slot 的位次 i, 命中则 slot=i;
 *      - count==0 且 slot>=0: 在 gTaskPoolNodes 里找 memberIdx 与当前节点数据对象相同的位次。
 * 5. 再用 sub_80489E8 收集一个候选槽表 buf (mode 0, 上限 0x100), 在其中找与 slot 相同
 *    memberIdx 的项, 命中则 slot = buf[i] (池槽索引)。
 * 返回 slot (s8; >=0 为有效槽, 调用者据此走 sub_802103C)。
 *
 * 注: n 在前两个查找循环中是**未初始化局部变量** —— 这是 ROM 的真 UB (经验 58/155):
 * 目标在 r8 上直接 `cmp r6, r8`, r8 到函数后半才由 sub_80489E8 结果写入。为字节忠实保留。
 * 链表头 0x03000690 是 UnkNode 哨兵 (sub_801B964 用 ListNode_Init 初始化), 故取 ->next。 */
s8 sub_801FF40(u8 mode)
{
    u8 buf[5];
    s8 slot;
    s8 i;
    u8 count;
    u8 n;
    TaskPoolNode *node;
    u8 *pool;

    slot = -1;
    i = 0;

    for (; i <= 4; i++)
        buf[i] = 0;

    sub_8018838(((u32 (*)(void))Rng_LcgNext)());
    ((u32 (*)(void))Rng_LcgNext)();

    node = (TaskPoolNode *)gTaskPoolHead.next;
    count = 0;

    while (node->key <= 0xFE)
    {
        *(u16 *)(node->data + 0xB2) += 1;
        i = (u8)(i + 1);
        if (mode == 1)
        {
            slot = i;
            break;
        }
        if ((u8)(((u32 (*)(void))Rng_LcgNext)() % 101) <= (s32)((*(u16 *)(node->data + 0xB2) + 1) * 40))
        {
            slot = i;
            break;
        }
        if (*(u16 *)(node->data + 0xB2) > 4)
        {
            buf[count] = *(u8 *)(node->data + 0xBB);
            count = (u8)(count + 1);
        }
        node = node->next;
    }

    if (count != 0)
    {
        slot = buf[((u32 (*)(void))Rng_LcgNext)() % count];
        for (i = 0; i < n; i++)
        {
            if (*(u8 *)(gTaskPoolNodes[i].data + 0xBB) == (s8)slot)
            {
                slot = i;
                break;
            }
        }
    }
    else if (slot >= 0)
    {
        for (i = 0; i < n; i++)
        {
            if (*(u8 *)(node->data + 0xBB) == *(u8 *)(gTaskPoolNodes[i].data + 0xBB))
            {
                slot = i;
                break;
            }
        }
    }

    pool = (u8 *)GetObjPool();
    n = sub_80489E8(pool, buf, 0, 0x100);
    for (i = 0; i < n; i++)
    {
        if ((s8)slot == ((BattleObj *)pool)[buf[i]].memberIdx)
        {
            slot = buf[i];
            break;
        }
    }

    return slot;
}
// @ 0x080200E8
void sub_80200E8(BattleObj *obj, PlayerStats *stats, u8 arg2)
{
    u8 i;
    s16 v;

    obj->maxHp = stats->max_hp;
    obj->maxMp = stats->max_mp;
    obj->hp = stats->hp;
    obj->mp = stats->mp;
    obj->atc = stats->base_atc + stats->equip_atc;
    obj->def = stats->base_def + stats->equip_def;
    obj->agl = stats->base_agl + stats->equip_agl;
    obj->men = stats->base_men + stats->equip_men;
    obj->res = stats->base_res + stats->equip_res;
    obj->noa = stats->noa;
    obj->lv = stats->lv;
    obj->variantClass = 0;
    obj->statMods[0] = 0;
    obj->statMods[1] = 0;
    obj->statMods[2] = 0;
    obj->statMods[3] = 0;
    obj->statMods[4] = 0;
    obj->equipSlots[0] = stats->equip_slot1;
    obj->equipSlots[1] = stats->equip_slot2;
    obj->equipSlots[2] = stats->equip_slot3;
    obj->equipSlots[3] = stats->equip_slot4;
    obj->equipSlots[4] = stats->equip_slot5;
    obj->equipSlots[5] = stats->equip_slot6;
    sub_8048B5C((u8 *)obj, stats->field_unk[1]);
    for (i = 0; i <= 7; i++)
    {
        v = stats->skills[i];
        if (v != 0xFF && v != 0x26)
            obj->skills[i] = v - 1;
        else
            obj->skills[i] |= 0xFF;
    }
    sub_8045BF4(obj);
    obj->pad_AC[0] = arg2;
    sub_8045EB8((u8 *)obj);
    sub_801D12C(obj, 0);
    obj->f_C3 = 0x10;
    obj->pad_C4[0] = 0x10;
}
// @ 0x08020228
// 敌方战斗对象属性装载 (玩家侧姊妹 = sub_80200E8): 按 obj->slot 索引 gUnk_083987EC
// 敌人属性表 (0x2C 字节/项): animPtr=表项指针, lv=(u8)表项[0], variantClass=0,
// maxHp/hp=表项[2] (双写), maxMp/mp=表项[4] (双写), atc/def/agl/men/res=表项[6..0xE],
// noa=(u8)表项[0x10], statMods[0..4]=0, f_C3=表项[0x23],
// pad_C4[0]=gUnk_08393A3C[表项[0x20]]*8, lv 加 sub_8047FCC((u8)sub_80187A8())。
// slot>0x70 (特效/空槽): 清 statMods[0..4] 后转 sub_802031C(arg0, obj)。尾部 +0xAC=arg2。
// 注意: lv 的和值须经具名 int 临时承接 (直接写入会改变加法目的寄存器的分配)。
void sub_8020228(u8 *arg0, BattleObj *arg1, u8 arg2)
{
    u8 *entry;
    s32 ret;
    int sum;

    if (arg1->slot <= 0x70)
    {
        entry = &gUnk_083987EC[arg1->slot * 0x2C];
        arg1->animPtr = entry;
        arg1->lv = *(u16 *)(entry + 0);
        arg1->variantClass = 0;
        arg1->maxHp = *(u16 *)(entry + 2);
        arg1->maxMp = *(u16 *)(entry + 4);
        arg1->hp = *(u16 *)(entry + 2);
        arg1->mp = *(u16 *)(entry + 4);
        arg1->atc = *(u16 *)(entry + 6);
        arg1->def = *(u16 *)(entry + 8);
        arg1->agl = *(u16 *)(entry + 0xA);
        arg1->men = *(u16 *)(entry + 0xC);
        arg1->res = *(u16 *)(entry + 0xE);
        arg1->noa = *(u16 *)(entry + 0x10);
        arg1->statMods[0] = 0;
        arg1->statMods[1] = 0;
        arg1->statMods[2] = 0;
        arg1->statMods[3] = 0;
        arg1->statMods[4] = 0;
        arg1->f_C3 = *(entry + 0x23);
        arg1->pad_C4[0] = gUnk_08393A3C[*(u16 *)(entry + 0x20)] * 8;
        ret = sub_8047FCC((u8)sub_80187A8());
        sum = ret + arg1->lv;
        arg1->lv = sum;
    }
    else
    {
        arg1->statMods[0] = 0;
        arg1->statMods[1] = 0;
        arg1->statMods[2] = 0;
        arg1->statMods[3] = 0;
        arg1->statMods[4] = 0;
        sub_802031C(arg0, arg1);
    }

    arg1->pad_AC[0] = arg2;
}
// @ 0x0802031C
INCLUDE_ASM("asm/nonmatchings", sub_802031C);
// @ 0x08020648
INCLUDE_ASM("asm/nonmatchings", sub_8020648);
// @ 0x08020798
u8 sub_8020798(void)
{
    return gUnk_03000744;
}
// @ 0x080207A4
void sub_80207A4(void)
{
    gUnk_03000630--;
}

// @ 0x080207B4
u8 sub_80207B4(void *arg0)
{
    if (sub_80187B4() & 0x20)
    {
        return sub_801C484(arg0);
    }
    return sub_801BE34(arg0);
}

/* 场景对象按 field_BE 分发到三种行为: ≤0xA → sub_801CBA4, ≤0x70 → sub_801CA08,
 * 其余 (field_BE-0x71 ≤ 0x8D) → sub_801CE80; 均传 (obj, 0, f2a, f35, 0)。 */
// @ 0x080207DC
void sub_80207DC(BattleObj *obj, u8 bf, u8 c0, u16 f2a, u8 f35)
{
    if (obj->slot <= 0xA)
        sub_801CBA4(obj, 0, f2a, f35, 0);
    else if (obj->slot <= 0x70)
        sub_801CA08(obj, 0, f2a, f35, 0);
    else if ((u8)(obj->slot - 0x71) <= 0x8D)
        sub_801CE80(obj, 0, f2a, f35, 0);
}

/* 场景对象按 field_BE 分派 (sub_80207DC 的变体): ≤0xA → sub_801CBA4(…, 0xA, …),
 * ≤0x70 → sub_801CA08(…, 5, …), 其余 (field_BE-0x71 ≤ 0x8D) → sub_801CE80(…, 0, …)。 */
// @ 0x08020840
void sub_8020840(BattleObj *obj, u8 bf, u8 c0, u16 f2a, u8 f35)
{
    if (obj->slot <= 0xA)
        sub_801CBA4(obj, 0xA, f2a, f35, 0);
    else if (obj->slot <= 0x70)
        sub_801CA08(obj, 5, f2a, f35, 0);
    else if ((u8)(obj->slot - 0x71) <= 0x8D)
        sub_801CE80(obj, 0, f2a, f35, 0);
}

/* 场景对象按 field_BE 分派 (sub_80207DC/840 变体): 先 sub_801D12C(obj,0), 再
 * <=0xA→sub_801CBA4(,2,), <=0x70→sub_801CA08(,1,), 否则 (field_BE-0x71<=0x8D)→sub_801CE80(,2,)。 */
// @ 0x080208A4
void sub_80208A4(BattleObj *obj)
{
    sub_801D12C(obj, 0);
    if (obj->slot <= 0xA)
        sub_801CBA4(obj, 2, obj->headA.f_1E, obj->headA.palSlot, 0);
    else if (obj->slot <= 0x70)
        sub_801CA08(obj, 1, obj->headA.f_1E, obj->headA.palSlot, 0);
    else if ((u8)(obj->slot - 0x71) <= 0x8D)
        sub_801CE80(obj, 2, obj->headA.f_1E, obj->headA.palSlot, 0);
}
// @ 0x08020914
void sub_8020914(BattleObj *arg0)
{
    if (arg0->slot <= 10)
    {
        sub_801CBA4(arg0, 3, arg0->headA.f_1E, arg0->headA.palSlot, 0);
    }
}
// @ 0x0802093C
void sub_802093C(BattleObj *arg0)
{
    u8 *ptr;
    u8 *addr;
    u8 new_var;
    s8 val;

    if (arg0->slot > 0xB)
    {
        ptr = arg0->animPtr;
        val = *(s8 *)&arg0->fxKind;
        switch (val)
        {
            case 0:
                addr = ptr + 0x23;
                break;
            case 1:
                addr = ptr + 0x24;
                break;
            default:
                return;
        }
        new_var = *addr;
        arg0->f_C3 = new_var;
    }
}

// @ 0x08020974
void sub_8020974(ObjHead *arg0, u16 arg1, u16 arg2, u8 arg3, u16 arg4)
{
    const ObjAnimEntry *entry = &gUnk_08393B28[arg1];

    sub_801B81C(arg0, arg0->f_2B, arg0->f_2C, arg2, arg3, (u32)entry->animScriptPtr, (u32)entry->palettePtr, entry->gfxBaseIdx, entry->gfxTotal, arg4);
}

// @ 0x080209C8
void sub_80209C8(BattleObj *arg0)
{
    u16 *ptr;
    u16 new_var;

    if (arg0->slot <= 6)
    {
        ptr = (u16 *)&arg0->animPtr;
        if (*ptr <= 0x1F)
        {
            *ptr += 4;
            ptr = &arg0->state;
            new_var = *ptr | 2;
            *ptr = new_var;
        }
    }
}

// @ 0x080209EC
void sub_80209EC(BattleObj *ptr)
{
    u16 *st;
    u16 new_var;

    if (ptr->slot <= 6)
    {
        *(u16 *)&ptr->animPtr = 0;
        st = &ptr->state;
        new_var = *st | 2;
        *st = new_var;
    }
}
typedef struct Unk_0839B2A4
{
    u32 field_0;
    u32 field_4;
    u16 field_8;
    u16 field_A;
    u8 pad_B[8];
} Unk_0839B2A4;

extern Unk_0839B2A4 gUnk_0839B2A4[];

// @ 0x08020A0C
void sub_8020A0C(BattleObj *arg0, u8 arg1)
{
    u16 newval;
    Unk_0839B2A4 *tbl = gUnk_0839B2A4;
    sub_801B81C(&arg0->headB, arg0->posX, arg0->posY, 0xDA << 1, 0xE, tbl[0].field_0,
                tbl[0].field_4 + (arg1 << 5), (u16)(0x541 + tbl[0].field_8), tbl[0].field_A, 2);
    arg0->headB.f_2A = 3;
    newval = 0x2000 | arg0->state;
    arg0->state = newval;
}
// @ 0x08020A7C
u8 sub_8020A7C(BattleObj *arg0)
{
    u8 i;
    u8 ret;

    ret = 1;
    for (i = 0; i <= 4; i++)
    {
        if (sub_8045F10(arg0 + i, 0x114) == 1)
        {
            ret = 0;
        }
    }

    return ret;
}
// @ 0x08020AB0
u8 sub_8020AB0(void)
{
    u8 buf[8];
    u8 ret;

    ret = sub_80489E8(GetObjPool(), buf, 0, 0x6B);
    if (sub_8044498() == 0)
    {
        return 1;
    }
    return ret != 0;
}

// @ 0x08020AE4
/* 待选池逐帧计时: 遍历 0x03000690 待选链 (同 sub_801FF40), 每个节点数据对象的
 * dmgAmount (+0xB2) 自增 1。与 sub_801FF40 的循环体同源。 */
void sub_8020AE4(void)
{
    TaskPoolNode *node = (TaskPoolNode *)gTaskPoolHead.next;

    while (node->key <= 0xFE)
    {
        (*(u16 *)(node->data + 0xB2))++;
        node = node->next;
    }
}
// @ 0x08020B04
void sub_8020B04(BattleObj *arg0)
{
    u8 ids[12];
    u8 i;
    u32 base = GetObjPool();
    u8 count = sub_80462E4((BattleObj *)arg0, ids, 0x7F);
    for (i = 0; i < count; i++)
    {
        sub_801D568((BattleObj *)(base + ids[i] * 0xC8));
    }
}
/*
extern u8 sub_80462E4(void *, u8 *, u8);
extern void sub_801D568(void *);

// @ 0x08020B04
void sub_8020B04(void *arg0)
{
    u8 buf[12];
    u8 count;
    u8 i;
    u8 *base;

    base = (u8 *)GetObjPool();
    count = sub_80462E4((BattleObj *)arg0, buf, 0x7F);
    for (i = 0; i < count; i++)
    {
        sub_801D568(base + buf[i] * 0xC8);
    }
}
*/
// @ 0x08020B48
u32 sub_8020B48(void)
{
    return gUnk_03000718;
}
// @ 0x08020B54
void sub_8020B54(void)
{
    u8 i;
    for (i = 0; i < 7; i++)
        gFxQueueObjs[i] = 0;
    gFxQueueWriteIdx = 0;
    gFxQueueReadIdx = 0;
    // The do-while barrier keeps GCC2's byte-store order 714,715,716 (local_alloc tiebreak, 经验 116).
    do
    {
        gUnk_03000716 = 0;
    } while (0);
}
// @ 0x08020B90
void sub_8020B90(BattleObj *arg0)
{
    gFxQueueObjs[gFxQueueWriteIdx] = arg0;
    gFxQueueWriteIdx++;
    if (arg0->slot > 0xB)
    {
        gUnk_03000718 = (u32)arg0;
    }
}

// @ 0x08020BC0
u8 sub_8020BC0(BattleObj *arg0)
{
    s32 diff;
    u16 *ptr;

    ptr = &arg0->hp;
    diff = *(s16 *)&arg0->hp - *(s16 *)&arg0->dmgAmount;
    if (diff <= 0)
    {
        *ptr = 0;
        return 1;
    }
    *ptr -= arg0->dmgAmount;
    return 0;
}

// @ 0x08020BF0
u8 sub_8020BF0(BattleObj *arg0)
{
    u8 value;

    value = gFxQueueObjs[0]->slot;
    if ((u8)(value - 0xB) <= 0x65)
    {
        return sub_801E848();
    }
    if ((u8)(value - 0x71) <= 0x8D)
    {
        return sub_8020C2C();
    }
}
typedef u8 (*UnkFunc20C2C)(BattleObj *);
extern UnkFunc20C2C gUnk_0839CE7C[];

// @ 0x08020C2C
u8 sub_8020C2C(void)
{
    return gUnk_0839CE7C[gFxQueueObjs[0]->slot - 0x71](gFxQueueObjs[0]);
}

// @ 0x08020C58
void sub_8020C58(BattleObj *entries, u32 arg1)
{
    u8 count = *(u8 *)gUnk_0300062C;
    u8 i;
    for (i = 0; i < count; i++)
    {
        BattleObj *entry = (BattleObj *)(i * 0xC8 + (u32)entries);
        if (entry->slot == 0xFF)
            continue;
        if (entry->state == 8 || entry->state == 5)
            continue;
        if (!(sub_80187B4() & 0x20))
            sub_804CEE0(entry, arg1);
        else
            sub_804DD70((BattleObj *)entry, arg1);
    }
}

// @ 0x08020CC4
void sub_8020CC4(void *arg0, u8 arg1, u8 arg2, u16 arg3, u8 arg4, u16 arg5, u16 arg6)
{
    u16 newval;
    sub_801B81C((ObjHead *)((u8 *)arg0 + 0x3C), arg1, arg2, arg3, arg4, (u32)gUnk_08393B28[arg5].animScriptPtr, (u32)gUnk_08393B28[arg5].palettePtr,
                gUnk_08393B28[arg5].gfxBaseIdx, gUnk_08393B28[arg5].gfxTotal, arg6);
    newval = 0x2000 | *(u16 *)((u8 *)arg0 + 0xB0);
    *(u16 *)((u8 *)arg0 + 0xB0) = newval;
}
/* ---- 场景对象 API (0x08020D50..0x080210C0) 自 battle_menu_windows.c 迁入 (2026-09-13 defcdgg-zcode):
 * 本段是 battle_menu_windows.c 的连续前缀, 链接序 battle_object_engine.o → battle_menu_windows.o 不变, ROM 布局不受影响。
 * 对象参数由 void 指针 / u8 指针 / Unk_8020F4C 指针统一为 BattleObj 指针; Unk_8020F4C 是 BattleObj 的冗余别名视图, 已删除
 * (field_24=headA.kindFlags, field_2A=headA.f_1E, field_35=headA.palSlot, field_36=headA.f_2A,
 * field_37/38=headA.f_2B/f_2C, field_B0=state, field_BB/BC/BD/BE=memberIdx/fxKind/f_BD/slot)。 ---- */

// @ 0x08020D50
void sub_8020D50(BattleObj *arg0, u8 arg1)
{
    u16 newval;
    if (arg0->slot > 0xA)
        return;
    *(u8 *)((u32)arg0 + 0xA3) = sub_804BBDC(sub_801D19C(arg0, arg1), 1, 0x1F, 0x1F, 0x1F, 0x10, 0, 3);
    newval = 0x80 | arg0->state;
    arg0->state = newval;
}
// @ 0x08020DA0
void sub_8020DA0(BattleObj *arg0, u8 arg1)
{
    u16 *reg;
    if (arg0->slot > 0xA)
        return;
    reg = &arg0->state;
    if (!(*reg & 0x80))
        return;
    sub_804BD54(sub_801D19C(arg0, arg1), 1);
    *reg = 0xFF7F & *reg;
}
// @ 0x08020DE4
void sub_8020DE4(void)
{
    gUnk_0300071C = 0;
}

// @ 0x08020DF0
void sub_8020DF0(BattleObj *arg0)
{
    u8 i;
    u32 ret;
    u8 val;
    u8 flag;
    val = arg0->f_BD;
    flag = 1;
    if (val <= 4)
    {
        flag = 0;
    }
    ret = sub_8046480((u8 *)arg0, gUnk_03000730_arr, flag);
    gUnk_0300073D = ret;
    gUnk_0300073C = 0;
    for (i = 0; i < (u8)ret; i++)
    {
        if (gUnk_03000730_arr[i] & 0xF0)
        {
            gUnk_0300073C++;
        }
    }
}
// @ 0x08020E54
u32 *sub_8020E54(void)
{
    return &gUnk_03000730;
}
// @ 0x08020E5C
u8 sub_8020E5C(void)
{
    return gUnk_0300073D;
}
// @ 0x08020E68
u32 sub_8020E68(void)
{
    return gUnk_0300062C;
}
// @ 0x08020E74
void sub_8020E74(void)
{
    u8 i;
    for (i = 0; i <= 10; i++)
    {
        gUnk_03000748[i] = 0;
    }
}
// @ 0x08020E90
void sub_8020E90(BattleObj *arg0)
{
    if (arg0->slot <= 10)
    {
        gUnk_03000748[arg0->slot] = 1;
    }
}
// @ 0x08020EAC
u8 sub_8020EAC(BattleObj *arg0)
{
    u8 result;

    result = 0;
    if (arg0->slot <= 10)
    {
        result = gUnk_03000748[arg0->slot];
    }
    return result;
}
// @ 0x08020EC8
void sub_8020EC8(void)
{
    u8 i;

    gUnk_03000763 = 0;
    for (i = 0; i <= 10; i++)
    {
        gUnk_03000758[i] = 0;
    }
}
// @ 0x08020EEC
void sub_8020EEC(u8 value)
{
    gUnk_03000758[gUnk_03000763] = value;
    gUnk_03000763++;
}

extern u32 gUnk_087ED6A8[];

// @ 0x08020F08
void sub_8020F08(void)
{
    u8 i;
    for (i = 0; i < gUnk_03000763; i++)
    {
        sub_804C2FC(gUnk_087ED6A8[gUnk_03000758[i]], i + 1, 1);
    }
}
// @ 0x08020F4C
void sub_8020F4C(BattleObj *arg0)
{
    arg0->memberIdx = 0;
    arg0->fxKind = 0xFF;
    arg0->state = 0;
    arg0->slot = 0xB;
    arg0->headA.f_2A = 0;
    gUnk_03000618 = 0;
    gUnk_0300061A = 0;
    gUnk_0300061C = 0;
    gUnk_0300061E = 0;
    gUnk_03000620 = 0;
    gUnk_03000622 = 0;
    gUnk_03000624 = 0;
    sub_801FA10(arg0, 0x31);
}
// @ 0x08020FB8
void sub_8020FB8(BattleObj *ptr, u16 arg1, u16 arg2, u16 arg3, u8 arg4)
{
    ptr->state = ptr->state & 0xFF0F;
    ptr->state = 0x10 | ptr->state;
    gUnk_03000618 = arg3;
    gUnk_0300061A = 0;
    gUnk_0300061C = ptr->headA.f_2B;
    gUnk_0300061E = ptr->headA.f_2C;
    gUnk_03000620 = arg1 - ptr->headA.f_2B;
    gUnk_03000622 = arg2 - ptr->headA.f_2C;
    gUnk_03000624 = arg4;
}
// @ 0x0802103C
void sub_802103C(BattleObj *arg0, u8 arg1, u16 arg2)
{
    u16 *ptr;

    ptr = &arg0->state;
    *ptr = (*ptr & 0xFF0F) | 0x60 | arg2;
    arg0->f_BD = arg1;
}

// @ 0x08021064
void sub_8021064(u8 arg0)
{
    u8 i;
    gDmgPopupSlot = 0;
    gDmgPopupLevel = 1;
    gDmgPopupPhase = 0;
    for (i = 0; i < 7; i++)
    {
        gUnk_03000670[i].field_0 = 0;
        gUnk_03000670[i].field_2 = 0;
        gUnk_03000670[i].field_3 = 0;
    }
    sub_804C2FC((u32)(gUnk_0861C664 + arg0 * 0x20), 0xF, 1);
}

// @ 0x080210C0
void sub_80210C0(BattleObj *arg0, u8 arg1)
{
    u16 newval;
    Unk_0839B2A4 *tbl = gUnk_0839B2A4;
    sub_801B81C(&arg0->headB, arg0->posX, arg0->posY, 0xDA << 1, 0xE, tbl[0].field_0,
                tbl[0].field_4 + (arg1 << 5), (u16)(0x541 + tbl[0].field_8), tbl[0].field_A, 2);
    arg0->headB.f_2A = 3;
    newval = 0x2000 | arg0->state;
    arg0->state = newval;
}
