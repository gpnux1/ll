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
void sub_8021184(s8 arg0, u8 *arg1)
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
            if ((s8)gMenuSlotStates[idx][2] < gMenuListCount)
            {
                gMenuListTop = gMenuSlotStates[idx][1];
                gMenuListCursor = gMenuSlotStates[idx][2];
            }
            else
            {
                gMenuListTop = (gMenuListCount - 1 <= 1) ? 0 : gMenuListCount - 3;
                gMenuListCursor = gMenuListCount - 1;
            }
            break;
        case 7:
            if ((s8)gMenuSlotStates[idx][4] < gMenuList2Count)
            {
                gMenuList2Top = gMenuSlotStates[idx][3];
                gMenuList2Cursor = gMenuSlotStates[idx][4];
            }
            else
            {
                gMenuList2Top = (gMenuList2Count - 1 <= 1) ? 0 : gMenuList2Count - 3;
                gMenuList2Cursor = gMenuList2Count - 1;
            }
            break;
    }
}
// @ 0x080212B4
INCLUDE_ASM("asm/nonmatchings", sub_80212B4);
// @ 0x0802151C
u8 sub_802151C(u8 arg0, BattleObj *arg1)
{
    u8 buf[8];
    u8 count;
    u8 matchCount;
    u16 i;
    u8 *pool;
    u16 flag;

    pool = (u8 *)GetObjPool();
    count = sub_80489E8((BattleObj *)pool, buf, 0, 0x7F);

    matchCount = 0;
    for (i = 0; i < count; i++)
    {
        BattleObj *obj = (BattleObj *)(buf[i] * 0xC8 + (u32)pool);
        if (obj->slot != 9)
        {
            if (sub_8045F10(obj, 0x3C) == 2)
                matchCount++;
        }
    }

    for (i = 0; i < count; i++)
    {
        if (((BattleObj *)(pool + buf[i] * 0xC8))->slot == 9)
        {
            count--;
            break;
        }
    }

    if (matchCount < count)
    {
        flag = sub_80187B4() & 0x4000;
        if (!flag)
        {
            gUnk_03000768 = arg0;
            sub_8021184((s8)gUnk_03000768, NULL);
            gUnk_0300076B = flag;
            gUnk_0300076C = 2;
            gUnk_03000769 = flag;
            gUnk_0300076E = flag;

            sub_801FA10(arg1, 2);
            Bg0_InitClear();
            sub_80196D4(0, 0x02035AC0, 0xB, 2, 2, flag, flag, 0x1E, 5);
            gMenuWindowFlags = flag;
            sub_802550C(1);
            sub_804C2FC(0x0861C624, 0xD, 2);
            sub_8024820();
            sub_804DE8C();
            sub_80187C0(0x100);

            gMenuObjLoadCount = gMenuObjLoadIdx = 0;

            for (i = 0; i < count; i++)
            {
                if ((s8)((BattleObj *)(pool + buf[i] * 0xC8))->fxKind == 3)
                {
                    gMenuObjLoadSlots[gMenuObjLoadCount] = buf[i];
                    ((BattleObj *)(pool + buf[i] * 0xC8))->fxKind = 0;
                    gMenuObjLoadCount++;
                }
            }

            return 0;
        }
        sub_804C78C();
    }
    else
    {
        sub_804C890((BattleObj *)pool);
        if (sub_80187B4() & 0x4000)
            sub_804C78C();
    }

    return 1;
}
// @ 0x08021700
u8 sub_8021700(void)
{
    u8 ret = 0;
    if (gMenuObjLoadIdx < gMenuObjLoadCount)
    {
        BattleObj *obj = (BattleObj *)(GetObjPool() + gMenuObjLoadSlots[gMenuObjLoadIdx] * 0xC8);
        switch (gMenuObjLoadPhase)
        {
            case 0:
                sub_80207DC(obj, obj->posX, obj->posY, obj->headA.f_1E, obj->headA.palSlot);
                gMenuObjLoadPhase = 1;
                break;
            case 1:
                if (!(obj->headA.kindFlags & 0x800))
                {
                    gMenuObjLoadIdx = gMenuObjLoadIdx + 1;
                    gMenuObjLoadPhase = 0;
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
    switch (gMenuWindowPhase)
    {
        case 0:
            if (gMenuWindowFlags & 0x1000)
            {
                if (DialogCtx_GetField_C_wide(0) == 0)
                {
                    gMenuWindowFlags &= ~0x1000;
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
            gMenuWindowFlags |= 0x1000;
            gMenuWindowPhase = 0;
            break;
        case 2:
            gMenuWindowPhase = 0;
            break;
    }
}
// @ 0x0802181C
INCLUDE_ASM("asm/nonmatchings", sub_802181C);
// @ 0x0802192C
INCLUDE_ASM("asm/nonmatchings", sub_802192C);

extern const u8 gUnk_08393A74[];
extern const u8 gUnk_08393A54[];

// @ 0x08022458
u8 sub_8022458(u8 arg0)
{
    const s8 *rec;
    u8 off;
    u8 i;

    if ((s8)gUnk_03000768 < 0)
        return arg0;

    rec = (const s8 *)(gUnk_08393A74 + ((s8)gUnk_03000768 << 4));
    if (rec[0] < 0)
        rec = (const s8 *)(gUnk_08393A74 + (rec[1] << 4));

    off = 0;
    for (i = 0; i < rec[0]; i++)
    {
        vu32 *dmaRegs = (vu32 *)0x040000D4;
        const s8 *p = rec + 8;
        u8 b = p[i];
        {
            const u8 *t = gUnk_08393A54 + (b << 2);

            dmaRegs[0] = (vu32)((const u8 *)0x08619AA4 + (t[0] << 5));
            dmaRegs[1] = (vu32)(0x06010000 + ((0x258 + off) << 5));
            dmaRegs[2] = 0x80000000 | (gUnk_08393A30[(t[1] << 2) + t[2]] << 4);
            dmaRegs[2];
            while (dmaRegs[2] & 0x80000000)
                ;
        }

        {
            const u8 *t = gUnk_08393A54 + (b << 2);
            off += gUnk_08393A30[(t[1] << 2) + t[2]];
        }
        arg0--;
    }

    return arg0;
}
// @ 0x08022550
INCLUDE_ASM("asm/matchings", sub_8022550);
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
    gMenuWindowPhase = value;
}
// @ 0x08025518
INCLUDE_ASM("asm/matchings", sub_8025518);
// @ 0x08025638
void sub_8025638(void)
{
    gMenuSelSlot0 = -1;
    gMenuSelSlot1 = -1;
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

    for (row = gMenuListTop; row < (s8)gMenuListTop + 3 && row < gMenuListCount; row = (u8)(row + 1))
    {
        bits = gMenuListRowBits >> row;
        palette = 1;
        if ((bits & 1) != 0)
            palette = (row == (s8)gMenuListCursor) ? 2 : 0;
        palette += 0xB;
        y = (row - (s8)gMenuListTop) * 2 + 8;
        BgMap_PalFillRect(base, palette, 8, y, 9, 2);
    }
}
// @ 0x0802576C
void sub_802576C(u8 *obj)
{
    u8 i;
    u8 style;
    u8 selected;

    for (i = gMenuList2Top; i < gMenuList2Top + 3 && i < gMenuList2Count; i++)
    {
        selected = gMenuList2Cursor;
        style = 0;
        if (i == selected)
            style = 2;
        BgMap_PalFillRect(obj, style + 0xB, 8, (i - gMenuList2Top) * 2 + 8, 9, 2);
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
