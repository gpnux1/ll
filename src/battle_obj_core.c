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
// @ 0x0801CA08
INCLUDE_ASM("asm/nonmatchings", sub_801CA08);
// @ 0x0801CBA4
INCLUDE_ASM("asm/nonmatchings", sub_801CBA4);
// @ 0x0801CE80
typedef struct Unk_08393B28
{
    u32 field_0;
    u32 field_4;
    u16 field_8;
    u16 field_A;
    u8 pad_C[4];
    u16 field_10;
    u8 pad_12[2];
} Unk_08393B28;

extern Unk_08393B28 gUnk_08393B28[];
extern u8 gUnk_08393A3C[];
extern u8 gUnk_08393A40[];
extern u8 gUnk_083987EC[];

void sub_801CE80(BattleObj *obj, u8 kind, u16 f2a, u8 f35, u8 arg5)
{
    u8 *p = obj->animPtr;
    Unk_08393B28 *entry;
    u16 flag;
    u16 idx;
    u16 v1;
    u16 v2;

    switch (kind)
    {
        case 0:
            idx = obj->f_AB ? *(u16 *)(p + 6) : *(u16 *)p;
            entry = &gUnk_08393B28[idx];
            flag = 0x409;
            break;
        case 6:
            idx = obj->f_AB ? *(u16 *)(p + 6) : *(u16 *)p;
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
            v1 = *(u16 *)((u8 *)entry + 0xC);
            obj->f_B4 = v1;
            v2 = *(u16 *)((u8 *)entry + 0xE);
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
            v1 = *(u16 *)((u8 *)entry + 0xC);
            obj->f_B4 = v1;
            v2 = *(u16 *)((u8 *)entry + 0xE);
            obj->f_B6 = v2;
            break;
        }
    }
    if (obj->slot == 0x78)
        flag |= 0x20;
    sub_801B81C(&obj->headA, obj->headA.f_2B, obj->headA.f_2C, f2a, f35, entry->field_0, entry->field_4, entry->field_8, entry->field_A, flag);
}
// @ 0x0801CF90
INCLUDE_ASM("asm/nonmatchings", sub_801CF90);
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
                value = obj->f_AB;
                switch (value)
                {
                    case 0:
                        if (obj->f_6C == obj->f_6E)
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
                value = obj->f_AB;
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
        obj->f_A2 = state;
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
                ab = obj->f_AB;
                switch (ab)
                {
                    case 0:
                        if (obj->f_6C == obj->f_6E)
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
                ab = obj->f_AB;
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
// @ 0x0801D214
INCLUDE_ASM("asm/nonmatchings", sub_801D214);
// @ 0x0801D378
INCLUDE_ASM("asm/nonmatchings", sub_801D378);
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
INCLUDE_ASM("asm/nonmatchings", sub_801D568);
// @ 0x0801D710
INCLUDE_ASM("asm/nonmatchings", sub_801D710);
// @ 0x0801D984
u8 sub_801D984(u8 arg)
{
    u8 r6;
    u8 r7;
    s8 i;
    u16 tmp;

    r6 = arg;
    if (gUnk_0300068C != 0)
    {
        u8 e = gUnk_0300068E;
        tmp = (u16)(-((0x14 - e * 2) / e));
        if (e <= 2)
        {
            r7 = (u8)sub_801768C(0, (s16)tmp, 9 - e, gUnk_0300068D * 2, 3);
        }
        else
        {
            r7 = 0;
        }
    }

    for (i = 0; i < gUnk_0300068C; i++)
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
    if (gUnk_0300068E < 3)
    {
        gUnk_0300068D = (gUnk_0300068D + 1) % (10 - gUnk_0300068E);
        if (gUnk_0300068D >= 9 - gUnk_0300068E)
            gUnk_0300068E = gUnk_0300068E + 1;
    }
    else if (gUnk_0300068E <= 0x22)
    {
        gUnk_0300068E = gUnk_0300068E + 1;
    }
    else
    {
        ret = 1;
        gUnk_0300068C = 0;
        gUnk_0300068E = 1;
        gUnk_0300068D = 0;
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
    Unk_08393B28 *t2;

    if (arg2 <= 2)
    {
        if (arg0->slot <= 0xA)
            delta = 0x10;
        else
            delta = (u8)sub_801EC3C(arg0, 1) >> 1;
        t1 = &gUnk_0839B2B0[arg2];
        sub_801B81C(&arg0->headB, arg0->f_BF, (u8)(arg0->f_C0 - delta), 0xAD << 2, 0xE,
                    t1->field_0, t1->field_4 + (arg1 << 5), (u16)(0x543 + t1->field_8), t1->field_A, 4);
    }
    else
    {
        t2 = &gUnk_08393B28[arg2];
        sub_801B81C(&arg0->headB, arg0->f_BF, arg0->f_C0, 0xC0 << 2, 0xE,
                    t2->field_0, t2->field_4, t2->field_8, t2->field_A, 4);
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
    sub_8048D40((u8 *)arg0);
    pool = GetObjPool();
    count = sub_80489E8(pool, buf, 0, 0x7F);
    for (i = 0; i < count; i++)
    {
        if (arg0->slot == pool[buf[i] * 0xC8 + 0xBE])
            break;
    }
    gUnk_030006A0[buf[i]].data = (u32)arg0;
    ListNode_InitKey((UnkNode *)&gUnk_030006A0[buf[i]], arg1);
    ListNode_InsertSorted((UnkNode *)0x03000690, (UnkNode *)&gUnk_030006A0[buf[i]]);
    sub_8045F94((u8 *)arg0, 8);
    arg0->f_B2 = 0;
    sub_804E7EC(arg0);
    if (arg0->slot <= 6)
    {
        *(u16 *)&arg0->animPtr = 0;
        arg0->state |= 2;
    }
    *(u8 *)0x030006F0 += 1;
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
    u8 *arr = (u8 *)gUnk_030006A0;
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

    obj->f_AB = 0;
    obj->f_B2 = 0;
    obj->f_6C = val;
    obj->f_BC = 0;

    if (*(u8 *)0x030006F0)
        (*(u8 *)0x030006F0)--;

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

    gUnk_0300068C = 0;
    gUnk_0300068E = 1;
    gUnk_0300068D = 0;
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
    Unk_08393B28 *entry;
    u8 *anim;
    int off;
    u16 sub;
    s8 kind = arg0->f_BC;
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
    switch (entry->field_10)
    {
    case 0:
        sub_801D568(arg1);
        gUnk_0300068D = 0;
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
            if (p->slot != 0xFF && p->f_AB != 8)
                sub_801D568(p);
        }
        gUnk_0300068D = 0;
        break;
    }
    }
}
// @ 0x0801DF90
void sub_801DF90(BattleObj *arg0, BattleObj *arg1)
{
    Unk_08393B28 *entry;
    int off;
    u8 *anim;
    s8 kind = arg0->f_BC;
    switch (kind)
    {
    case 0:
        entry = &gUnk_08393B28[*(u16 *)(arg0->animPtr + 2)];
        break;
    case 1:
        anim = arg0->animPtr;
        off = arg0->f_C2 * 2;
        anim += 8;
        entry = &gUnk_08393B28[*(u16 *)(anim + off)];
        break;
    }
    switch (entry->field_10)
    {
    case 0:
        sub_801D568(arg1);
        gUnk_0300068D = 0;
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
            if (p->slot != 0xFF && p->f_AB != 8)
                sub_801D568(p);
        }
        gUnk_0300068D = 0;
        break;
    }
    }
}
// @ 0x0801E040
u8 sub_801E040(void)
{
    u8 ret = 0;
    if (gUnk_03000715 < gUnk_03000714)
    {
        BattleObj *obj = gUnk_030006F8[gUnk_03000715];
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
            sub_8045F94((u8 *)obj, 8);
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
            sub_8045F94((u8 *)obj, 8);
        }
        gUnk_03000715++;
    }
    else
    {
        if (gUnk_03000714 != 0)
        {
            u8 s = gUnk_030006F8[0]->slot;
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
        gUnk_03000715 = 0;
    }
    return ret;
}
// @ 0x0801E1D8
INCLUDE_ASM("asm/nonmatchings", sub_801E1D8);
// @ 0x0801E30C
INCLUDE_ASM("asm/nonmatchings", sub_801E30C);
// @ 0x0801E4D4
/* 战斗效果入队 (等价已匹配的 sub_8020B90): 写队列槽 gUnk_030006F8[count] 后 count++,
 * 对象类型 > 0xB 时同时登记 gUnk_03000718。原 ROM 在 sub_801E4D4/801E30C/801E690 三处
 * 内联展开本逻辑; 因 GCC2 不会内联定义在后面的函数, 此处用 static inline 复现内联形状
 * (全部调用点被集成, 不产生独立副本, 不影响段布局)。 */
static inline void Inl_QueuePushObj(BattleObj *obj)
{
    gUnk_030006F8[gUnk_03000714] = obj;
    gUnk_03000714++;
    if (obj->slot > 0xB)
    {
        gUnk_03000718 = (u32)obj;
    }
}

/* 由 arg0[0xBC] 选择的效果查表 (gUnk_08393B28[idx].field_10) 决定处理模式:
 * 0 → 对 arg1 单体做 [0x6C] -= [0xB2] (下溢清零), 回绕则入队;
 * 1 → 对 arg1 起始的 0xC8 步长成员数组 (5 或 7 个, 按 arg1[0xBE] 分档) 逐个执行同一扣减;
 * 其余 → 无操作。任一回绕标志置位则返回 1。注意本函数在 ROM 中无任何调用者(死代码)。 */
u32 sub_801E4D4(BattleObj *arg0, BattleObj *arg1)
{
    u8 flags[7];
    Unk_08393B28 *entry;
    u16 idx;
    s32 t;
    u32 result;
    u32 limit;
    u8 wrapped;
    u8 i;

    result = 0;
    for (i = 0; i <= 6; i++)
        flags[i] = 0;

    switch ((s8)arg0->f_BC)
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

    switch (*(u16 *)((u8 *)entry + 0x10))
    {
    case 0:
        if (*(s16 *)&arg1->f_6C - *(s16 *)&arg1->f_B2 <= 0)
        {
            arg1->f_6C = *(u16 *)((u8 *)entry + 0x10);
            wrapped = 1;
        }
        else
        {
            arg1->f_6C -= arg1->f_B2;
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
            if (member->f_AB == 8)
                continue;
            if (*(s16 *)&member->f_6C - *(s16 *)&member->f_B2 <= 0)
            {
                member->f_6C = 0;
                wrapped = 1;
            }
            else
            {
                member->f_6C -= member->f_B2;
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
/* 同 sub_801E4D4 的效果扣减引擎 (arg0[0xBC]→animPtr 查 gUnk_08393B28→field_10 模式分派),
 * 差异仅在查表入口: mode0=*(u16*)(animPtr+2), mode1=*(u16*)(animPtr+8+f_C2*2)
 * (即 animPtr+0x3A 处的 u8 索引; 与已匹配 sub_801DF90 的 dispatch 字节同构)。
 * ROM 中无任何调用者(死代码)。 */
u32 sub_801E690(BattleObj *arg0, BattleObj *arg1)
{
    u8 flags[7];
    Unk_08393B28 *entry;
    int off;
    u8 *anim;
    u32 result;
    u32 limit;
    u8 wrapped;
    u8 i;

    result = 0;
    for (i = 0; i <= 6; i++)
        flags[i] = 0;

    switch ((s8)arg0->f_BC)
    {
    case 0:
        entry = &gUnk_08393B28[*(u16 *)(arg0->animPtr + 2)];
        break;
    case 1:
        anim = arg0->animPtr;
        off = arg0->f_C2 * 2;
        anim += 8;
        entry = &gUnk_08393B28[*(u16 *)(anim + off)];
        break;
    }

    switch (*(u16 *)((u8 *)entry + 0x10))
    {
    case 0:
        if (*(s16 *)&arg1->f_6C - *(s16 *)&arg1->f_B2 <= 0)
        {
            arg1->f_6C = *(u16 *)((u8 *)entry + 0x10);
            wrapped = 1;
        }
        else
        {
            arg1->f_6C -= arg1->f_B2;
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
            if (member->f_AB == 8)
                continue;
            if (*(s16 *)&member->f_6C - *(s16 *)&member->f_B2 <= 0)
            {
                member->f_6C = 0;
                wrapped = 1;
            }
            else
            {
                member->f_6C -= member->f_B2;
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
INCLUDE_ASM("asm/nonmatchings", sub_801EA70);
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
        if (obj->f_AB == 4)
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
// @ 0x0801EE6C
/* BattleObj 白色着色清除 (sub_801ED40 的逆操作): 撤销对象精灵的白色 tint。
 * v 槽基址与 sub_801ED40 同源 (slot>0x0B 且 f_AB==4 → gUnk_03000744, 否则 headA->palSlot);
 * sub_804B7B0(v, headA 字节2) 清除 tint 记录; kindFlags bit15 若在则清 0;
 * slot==0x77 额外 sub_804B834(palSlot,1,3,-11,5) (特殊背景的补充恢复)。 */
void sub_801EE6C(BattleObj *ptr)
{
    u8 v;
    u8 b;

    if (ptr->slot > 0x0B && ptr->f_AB == 4)
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
INCLUDE_ASM("asm/nonmatchings", sub_801F76C);
// @ 0x0801F884
INCLUDE_ASM("asm/nonmatchings", sub_801F884);
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
INCLUDE_ASM("asm/nonmatchings", sub_801FF40);
// @ 0x080200E8
INCLUDE_ASM("asm/nonmatchings", sub_80200E8);
// @ 0x08020228
INCLUDE_ASM("asm/nonmatchings", sub_8020228);
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
        val = *(s8 *)&arg0->f_BC;
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
    Unk_08393B28 *entry = &gUnk_08393B28[arg1];

    sub_801B81C(arg0, arg0->f_2B, arg0->f_2C, arg2, arg3, entry->field_0, entry->field_4, entry->field_8, entry->field_A, arg4);
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
    sub_801B81C(&arg0->headB, arg0->f_BF, arg0->f_C0, 0xDA << 1, 0xE, tbl[0].field_0,
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
        if (sub_8045F10((u8 *)arg0 + i * 0xC8, 0x114) == 1)
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

    ret = sub_80489E8((u8 *)GetObjPool(), buf, 0, 0x6B);
    if (sub_8044498() == 0)
    {
        return 1;
    }
    return ret != 0;
}

typedef struct Unk_8020AE4_node
{
    u8 field_0;
    u8 pad_1[7];
    struct Unk_8020AE4_node *field_8;
    u32 field_C;
} Unk_8020AE4_node;

typedef struct Unk_03000690
{
    u32 field_0;
    u32 field_4;
    Unk_8020AE4_node *field_8;
} Unk_03000690;

// @ 0x08020AE4
void sub_8020AE4(void)
{
    Unk_8020AE4_node *node = ((Unk_03000690 *)0x03000690)->field_8;
    while (node->field_0 <= 0xFE)
    {
        (*(u16 *)(node->field_C + 0xB2))++;
        node = node->field_8;
    }
}
// @ 0x08020B04
void sub_8020B04(u8 *arg0)
{
    u8 ids[12];
    u8 i;
    u32 base = GetObjPool();
    u8 count = sub_80462E4(arg0, ids, 0x7F);
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
    count = sub_80462E4(arg0, buf, 0x7F);
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
        gUnk_030006F8[i] = 0;
    gUnk_03000714 = 0;
    gUnk_03000715 = 0;
    // The do-while barrier keeps GCC2's byte-store order 714,715,716 (local_alloc tiebreak, 经验 116).
    do
    {
        gUnk_03000716 = 0;
    } while (0);
}
// @ 0x08020B90
void sub_8020B90(BattleObj *arg0)
{
    gUnk_030006F8[gUnk_03000714] = arg0;
    gUnk_03000714++;
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

    ptr = &arg0->f_6C;
    diff = *(s16 *)&arg0->f_6C - *(s16 *)&arg0->f_B2;
    if (diff <= 0)
    {
        *ptr = 0;
        return 1;
    }
    *ptr -= arg0->f_B2;
    return 0;
}

// @ 0x08020BF0
u8 sub_8020BF0(BattleObj *arg0)
{
    u8 value;

    value = gUnk_030006F8[0]->slot;
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
    return gUnk_0839CE7C[gUnk_030006F8[0]->slot - 0x71](gUnk_030006F8[0]);
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
            sub_804DD70((u8 *)entry, arg1);
    }
}

// @ 0x08020CC4
void sub_8020CC4(void *arg0, u8 arg1, u8 arg2, u16 arg3, u8 arg4, u16 arg5, u16 arg6)
{
    u16 newval;
    sub_801B81C((ObjHead *)((u8 *)arg0 + 0x3C), arg1, arg2, arg3, arg4, gUnk_08393B28[arg5].field_0, gUnk_08393B28[arg5].field_4,
                gUnk_08393B28[arg5].field_8, gUnk_08393B28[arg5].field_A, arg6);
    newval = 0x2000 | *(u16 *)((u8 *)arg0 + 0xB0);
    *(u16 *)((u8 *)arg0 + 0xB0) = newval;
}
