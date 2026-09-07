#include "code_0.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "menu_slot.h"
#include "save.h"
#include "sound.h"

// @ 0x08020D50
void sub_8020D50(void *arg0, u8 arg1)
{
    u16 newval;
    if (*(u8 *)((u8 *)arg0 + 0xBE) > 0xA)
        return;
    *(u8 *)((u32)arg0 + 0xA3) = sub_804BBDC(sub_801D19C(arg0, arg1), 1, 0x1F, 0x1F, 0x1F, 0x10, 0, 3);
    newval = 0x80 | *(u16 *)((u32)arg0 + 0xB0);
    *(u16 *)((u32)arg0 + 0xB0) = newval;
}
// @ 0x08020DA0
void sub_8020DA0(void *arg0, u8 arg1)
{
    u16 *reg;
    if (*(u8 *)((u8 *)arg0 + 0xBE) > 0xA)
        return;
    reg = (u16 *)((u32)arg0 + 0xB0);
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
extern u8 gUnk_03000730_arr[];

// @ 0x08020DF0
void sub_8020DF0(u8 *arg0)
{
    u8 i;
    u32 ret;
    u8 val;
    u8 flag;
    val = *(u8 *)(arg0 + 0xBD);
    flag = 1;
    if (val <= 4)
    {
        flag = 0;
    }
    ret = sub_8046480(arg0, gUnk_03000730_arr, flag);
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
void sub_8020E90(u8 *arg0)
{
    if (arg0[0xBE] <= 10)
    {
        gUnk_03000748[arg0[0xBE]] = 1;
    }
}
// @ 0x08020EAC
u8 sub_8020EAC(u8 *arg0)
{
    u8 result;

    result = 0;
    if (arg0[0xBE] <= 10)
    {
        result = gUnk_03000748[arg0[0xBE]];
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
typedef struct Unk_8020F4C
{
    u8 pad24[0x24];
    u16 field_24;
    u8 pad26[0x2A - 0x26];
    u16 field_2A;
    u8 pad2B[0x35 - 0x2C];
    u8 field_35;
    u8 pad36[0x36 - 0x36];
    u8 field_36;
    u8 field_37;
    u8 field_38;
    u8 pad39[0xB0 - 0x39];
    u16 field_B0;
    u8 padB1[0xBB - 0xB2];
    u8 field_BB;
    u8 field_BC;
    u8 padBD[0xBE - 0xBD];
    u8 field_BE;
    u8 field_BF;
    u8 field_C0;
} Unk_8020F4C;

// @ 0x08020F4C
void sub_8020F4C(Unk_8020F4C *arg0)
{
    arg0->field_BB = 0;
    arg0->field_BC = 0xFF;
    arg0->field_B0 = 0;
    arg0->field_BE = 0xB;
    arg0->field_36 = 0;
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
void sub_8020FB8(void *arg0, u16 arg1, u16 arg2, u16 arg3, u8 arg4)
{
    Unk_8020F4C *ptr = (Unk_8020F4C *)arg0;
    ptr->field_B0 = ptr->field_B0 & 0xFF0F;
    ptr->field_B0 = 0x10 | ptr->field_B0;
    gUnk_03000618 = arg3;
    gUnk_0300061A = 0;
    gUnk_0300061C = ptr->field_37;
    gUnk_0300061E = ptr->field_38;
    gUnk_03000620 = arg1 - ptr->field_37;
    gUnk_03000622 = arg2 - ptr->field_38;
    gUnk_03000624 = arg4;
}
// @ 0x0802103C
void sub_802103C(u8 *arg0, u8 arg1, u16 arg2)
{
    u16 *ptr;

    ptr = (u16 *)(arg0 + 0xB0);
    *ptr = (*ptr & 0xFF0F) | 0x60 | arg2;
    arg0[0xBD] = arg1;
}

typedef struct Unk_8021064
{
    u16 field_0;
    u8 field_2;
    u8 field_3;
} Unk_8021064;

extern Unk_8021064 gUnk_03000670[];
extern u8 gUnk_0861C664[];

// @ 0x08021064
void sub_8021064(u8 arg0)
{
    u8 i;
    gUnk_0300068C = 0;
    gUnk_0300068E = 1;
    gUnk_0300068D = 0;
    for (i = 0; i < 7; i++)
    {
        gUnk_03000670[i].field_0 = 0;
        gUnk_03000670[i].field_2 = 0;
        gUnk_03000670[i].field_3 = 0;
    }
    sub_804C2FC((u32)(gUnk_0861C664 + arg0 * 0x20), 0xF, 1);
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

// @ 0x080210C0
void sub_80210C0(void *arg0, u8 arg1)
{
    u16 newval;
    Unk_0839B2A4 *tbl = gUnk_0839B2A4;
    sub_801B81C((u8 *)arg0 + 0x3C, *(u8 *)((u8 *)arg0 + 0xBF), *(u8 *)((u8 *)arg0 + 0xC0), 0xDA << 1, 0xE, tbl[0].field_0,
                tbl[0].field_4 + (arg1 << 5), (u16)(0x541 + tbl[0].field_8), tbl[0].field_A, 2);
    *(u8 *)((u8 *)arg0 + 0x66) = 3;
    newval = 0x2000 | *(u16 *)((u8 *)arg0 + 0xB0);
    *(u16 *)((u8 *)arg0 + 0xB0) = newval;
}

// @ 0x08021130
void MenuSlot_ResetAll(void)
{
    u8 i;

    for (i = 0; i < 10; i++)
    {
        gMenuSlotStates[i][0] = 0;
        gMenuSlotStates[i][1] = 0;
        gMenuSlotStates[i][2] = 0;
        gMenuSlotStates[i][3] = 0;
        gMenuSlotStates[i][4] = 0;
    }
    gMenuMasterCursor = 0;
}
// @ 0x08021184
void sub_8021184(u8 arg0, u8 *arg1)
{
    u8 b;
    u8 idx;
    u8 *ptr;

    b = (u8)arg0;
    ptr = arg1 + 0xBE;
    if (*ptr != 0)
        idx = *ptr - 1;
    else
        idx = *ptr;

    switch ((s8)b)
    {
        case 0:
            gUnk_0300076A = gMenuMasterCursor;
            break;
        case 3:
            if ((s8)gUnk_0300076A != (s8)gMenuSlotStates[idx][0])
            {
                gUnk_0300076C |= 2;
            }
            gUnk_0300076A = gMenuSlotStates[idx][0];
            break;
        case 6:
            if ((s8)gMenuSlotStates[idx][2] < gUnk_03000770)
            {
                gUnk_03000781 = gMenuSlotStates[idx][1];
                gUnk_03000782 = gMenuSlotStates[idx][2];
            }
            else
            {
                gUnk_03000781 = (gUnk_03000770 - 1 <= 1) ? 0 : gUnk_03000770 - 3;
                gUnk_03000782 = gUnk_03000770 - 1;
            }
            break;
        case 7:
            if ((s8)gMenuSlotStates[idx][4] < gUnk_03000808)
            {
                gUnk_03000809 = gMenuSlotStates[idx][3];
                gUnk_0300080A = gMenuSlotStates[idx][4];
            }
            else
            {
                gUnk_03000809 = (gUnk_03000808 - 1 <= 1) ? 0 : gUnk_03000808 - 3;
                gUnk_0300080A = gUnk_03000808 - 1;
            }
            break;
    }
}
// @ 0x080212B4
INCLUDE_ASM("asm/nonmatchings", sub_80212B4);
// @ 0x0802151C
INCLUDE_ASM("asm/nonmatchings", sub_802151C);
// @ 0x08021700
u8 sub_8021700(void)
{
    u8 ret = 0;
    if (gUnk_03000812 < gUnk_03000811)
    {
        Unk_8020F4C *obj = (Unk_8020F4C *)(GetObjPool() + gUnk_0300080C[gUnk_03000812] * 0xC8);
        switch (gUnk_03000813)
        {
            case 0:
                sub_80207DC(obj, obj->field_BF, obj->field_C0, obj->field_2A, obj->field_35);
                gUnk_03000813 = 1;
                break;
            case 1:
                if (!(obj->field_24 & 0x800))
                {
                    gUnk_03000812 = gUnk_03000812 + 1;
                    gUnk_03000813 = 0;
                }
                break;
        }
    }
    else
    {
        ret = 1;
    }
    return ret;
}
extern u32 DialogCtx_GetField_C_wide(u8) __asm__("DialogCtx_GetField_C");

// @ 0x08021788
void sub_8021788(u8 arg0)
{
    switch (gUnk_03000816)
    {
        case 0:
            if (gUnk_03000818 & 0x1000)
            {
                if (DialogCtx_GetField_C_wide(0) == 0)
                {
                    gUnk_03000818 &= ~0x1000;
                }
                else
                {
                    u32 v = DialogCtx_GetField_C_wide(0);
                    if (v == 4)
                    {
                        if (((unsigned short)v & gUnk_0300076C) == 0)
                        {
                            sub_802181C((void *)0x02035AC0, 0x18, 2, arg0);
                        }
                    }
                }
            }
            break;
        case 1:
            gUnk_03000818 |= 0x1000;
            gUnk_03000816 = 0;
            break;
        case 2:
            gUnk_03000816 = 0;
            break;
    }
}
// @ 0x0802181C
INCLUDE_ASM("asm/nonmatchings", sub_802181C);
// @ 0x0802192C
INCLUDE_ASM("asm/nonmatchings", sub_802192C);
// @ 0x08022458
INCLUDE_ASM("asm/nonmatchings", sub_8022458);
// @ 0x08022550
INCLUDE_ASM("asm/nonmatchings", sub_8022550);
// @ 0x08022710
INCLUDE_ASM("asm/nonmatchings", sub_8022710);
// @ 0x08022F2C
INCLUDE_ASM("asm/nonmatchings", sub_8022F2C);
// @ 0x080230BC
INCLUDE_ASM("asm/nonmatchings", sub_80230BC);
// @ 0x08023320
INCLUDE_ASM("asm/nonmatchings", sub_8023320);
// @ 0x08023414
INCLUDE_ASM("asm/nonmatchings", sub_8023414);
// @ 0x08023820
INCLUDE_ASM("asm/nonmatchings", sub_8023820);
// @ 0x080244BC
INCLUDE_ASM("asm/nonmatchings", sub_80244BC);
// @ 0x08024618
INCLUDE_ASM("asm/nonmatchings", sub_8024618);
// @ 0x080246E8
INCLUDE_ASM("asm/nonmatchings", sub_80246E8);
// @ 0x08024820
void sub_8024820(void)
{
    s8 idx = (s8)gUnk_03000768;
    switch (idx)
    {
        case 0:
        case 1:
        case 2:
        case 4:
            sub_8018798(0, -1);
            sub_8018798(1, -1);
            sub_8018798(2, 0);
            sub_8018798(3, 0);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
        case 3:
            sub_8018798(0, -1);
            sub_8018798(1, -1);
            sub_8018798(2, 0);
            sub_8018798(3, 0);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
        case 5:
            sub_8018798(0, 0);
            sub_8018798(1, 0);
            sub_8018798(2, 0);
            sub_8018798(3, 0);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
        case 6:
            sub_8018798(0, 0);
            sub_8018798(1, 0);
            sub_8018798(2, -1);
            sub_8018798(3, -1);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
        case 7:
        case 8:
        case 9:
            sub_8018798(0, -1);
            sub_8018798(1, -1);
            sub_8018798(2, 0);
            sub_8018798(3, 0);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
    }
}
// @ 0x08024940
INCLUDE_ASM("asm/nonmatchings", sub_8024940);
// @ 0x0802550C
void sub_802550C(u8 value)
{
    gUnk_03000816 = value;
}
// @ 0x08025518
INCLUDE_ASM("asm/nonmatchings", sub_8025518);
// @ 0x08025638
void sub_8025638(void)
{
    gUnk_03000814 = -1;
    gUnk_03000815 = -1;
}
// @ 0x08025650
void sub_8025650(arg0, arg1) u8 *arg0;
u8 arg1;
{
    u16 *p;
    p = (u16 *)(arg0 + 0x20E);
    if (arg1 == 0)
        return;
    gUnk_0300076E = (gUnk_0300076E + 1) % 16;
    if (arg1 & 1)
    {
        if (arg1 & 0x10)
        {
            u16 c = gUnk_0300076E;
            p[15] = (c >> 3) + 0xB9DE;
        }
        else
        {
            p[15] = 0xB001;
        }
        p += 0xA0;
        if (arg1 & 0x20)
        {
            u16 c2 = gUnk_0300076E;
            p[15] = (c2 >> 3) + 0xB1DE;
        }
        else
        {
            p[15] = 0xB001;
        }
    }
    else
    {
        p[15] = 0xB000;
        p = (u16 *)(arg0 + 0x34E);
        p[15] = 0xB000;
    }
}
// @ 0x080256E4
void sub_80256E4(u16 *base)
{
    s32 palette;
    u8 row;
    s32 bits;
    s32 y;

    for (row = gUnk_03000781; row < (s8)gUnk_03000781 + 3 && row < gUnk_03000770; row = (u8)(row + 1))
    {
        bits = gUnk_03000784 >> row;
        palette = 1;
        if ((bits & 1) != 0)
            palette = (row == (s8)gUnk_03000782) ? 2 : 0;
        palette += 0xB;
        y = (row - (s8)gUnk_03000781) * 2 + 8;
        BgMap_PalFillRect(base, palette, 8, y, 9, 2);
    }
}
// @ 0x0802576C
void sub_802576C(u8 *obj)
{
    u8 i;
    u8 style;
    u8 selected;

    for (i = gUnk_03000809; i < gUnk_03000809 + 3 && i < gUnk_03000808; i++)
    {
        selected = gUnk_0300080A;
        style = 0;
        if (i == selected)
            style = 2;
        BgMap_PalFillRect(obj, style + 0xB, 8, (i - gUnk_03000809) * 2 + 8, 9, 2);
    }
}
// @ 0x080257D8
INCLUDE_ASM("asm/nonmatchings", sub_80257D8);
// @ 0x08025994
INCLUDE_ASM("asm/nonmatchings", sub_8025994);
// @ 0x08025DA8
INCLUDE_ASM("asm/nonmatchings", sub_8025DA8);
// @ 0x080260BC
INCLUDE_ASM("asm/nonmatchings", sub_80260BC);
