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


// @ 0x080264C0
INCLUDE_ASM("asm/nonmatchings", sub_80264C0);
// @ 0x0802698C
INCLUDE_ASM("asm/nonmatchings", sub_802698C);
// @ 0x08026D08
INCLUDE_ASM("asm/nonmatchings", sub_8026D08);
// @ 0x08026F88
INCLUDE_ASM("asm/nonmatchings", sub_8026F88);
// @ 0x0802723C
INCLUDE_ASM("asm/nonmatchings", sub_802723C);
// @ 0x0802761C
u8 sub_802761C(u8 *obj)
{
    u8 result;
    u8 i;
    u32 zero;

    result = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000828 = obj[0xBF];
            gUnk_03000829 = obj[0xC0];
            gUnk_03000822 = *(u16 *)(obj + 0x2A);
            gUnk_03000824 = obj[0x35];
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gUnk_03000825 = 0;
            gUnk_03000820 = 1;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            break;
        case 9:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && gUnk_03000856 == 0)
            {
                GetObjPool();
                for (i = 0; i < gUnk_0300083D; i++)
                {
                    if ((gUnk_03000840[i] & 0xF0) == 0x10)
                    {
                        if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
                        {
                            gUnk_03004F90[gUnk_03000840[i] & 0xF] = 1;
                        }
                    }
                }
                result = 1;
            }
            break;
    }
    sub_803F658(obj);
    if (*(u16 *)(obj + 0x24) & 0x1000)
    {
        u16 masked = *(u16 *)(obj + 0x24) & 0xEFFF;
        zero = 0;
        *(u16 *)(obj + 0x24) = masked;
        sub_801CE80(obj, zero, gUnk_03000822, gUnk_03000824, zero);
    }
    return result;
}
// @ 0x08027760
INCLUDE_ASM("asm/nonmatchings", sub_8027760);
// @ 0x08027A20
INCLUDE_ASM("asm/nonmatchings", sub_8027A20);
// @ 0x08027D9C
INCLUDE_ASM("asm/nonmatchings", sub_8027D9C);
// @ 0x08028098
INCLUDE_ASM("asm/nonmatchings", sub_8028098);
// @ 0x080282EC
INCLUDE_ASM("asm/nonmatchings", sub_80282EC);
// @ 0x080285A0
INCLUDE_ASM("asm/nonmatchings", sub_80285A0);
// @ 0x080287EC
INCLUDE_ASM("asm/nonmatchings", sub_80287EC);
// @ 0x08028AD8
INCLUDE_ASM("asm/nonmatchings", sub_8028AD8);
// @ 0x08029268
INCLUDE_ASM("asm/nonmatchings", sub_8029268);
// @ 0x08029510
INCLUDE_ASM("asm/nonmatchings", sub_8029510);
// @ 0x08029784
INCLUDE_ASM("asm/nonmatchings", sub_8029784);
// @ 0x080299C8
INCLUDE_ASM("asm/nonmatchings", sub_80299C8);
// @ 0x08029BF8
INCLUDE_ASM("asm/nonmatchings", sub_8029BF8);
// @ 0x0802A154
INCLUDE_ASM("asm/nonmatchings", sub_802A154);
// @ 0x0802A86C
INCLUDE_ASM("asm/nonmatchings", sub_802A86C);
// @ 0x0802ADC4
INCLUDE_ASM("asm/nonmatchings", sub_802ADC4);
// @ 0x0802B0F0
INCLUDE_ASM("asm/nonmatchings", sub_802B0F0);
// @ 0x0802B608
INCLUDE_ASM("asm/nonmatchings", sub_802B608);
// @ 0x0802B8BC
INCLUDE_ASM("asm/nonmatchings", sub_802B8BC);
// @ 0x0802BB24
INCLUDE_ASM("asm/nonmatchings", sub_802BB24);
// @ 0x0802BD94
INCLUDE_ASM("asm/nonmatchings", sub_802BD94);
// @ 0x0802C0EC
INCLUDE_ASM("asm/nonmatchings", sub_802C0EC);
// @ 0x0802C490
INCLUDE_ASM("asm/nonmatchings", sub_802C490);
// @ 0x0802C714
INCLUDE_ASM("asm/nonmatchings", sub_802C714);
// @ 0x0802C9E8
INCLUDE_ASM("asm/nonmatchings", sub_802C9E8);
// @ 0x0802CE90
INCLUDE_ASM("asm/nonmatchings", sub_802CE90);
// @ 0x0802D1FC
INCLUDE_ASM("asm/nonmatchings", sub_802D1FC);
// @ 0x0802D454
INCLUDE_ASM("asm/nonmatchings", sub_802D454);
// @ 0x0802D728
INCLUDE_ASM("asm/nonmatchings", sub_802D728);
// @ 0x0802DA78
INCLUDE_ASM("asm/nonmatchings", sub_802DA78);
// @ 0x0802DE04
INCLUDE_ASM("asm/nonmatchings", sub_802DE04);
// @ 0x0802DFDC
u32 sub_802DFDC(u8 *arg)
{
    u32 ret;
    u16 keys;

    ret = 0;
    sub_80187E8();
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000828 = arg[0xBF];
            gUnk_03000829 = arg[0xC0];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            gUnk_03000824 = arg[0x35];
            sub_80444A4(arg);
            sub_801CE80(arg, 5, 0x1B4, 0xC, 1);
            sub_803F5B4(arg);
            gUnk_03000825 = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 2;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0;
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (*(u16 *)&arg[0x28] > 0x46)
            {
                sub_8020CC4(arg, 0xB9, 0x78, 0x2E0, 0xE, 0x2F9, 5);
                gUnk_03000820 = 0x14;
            }
            break;
        case 20:
            keys = *(u16 *)&arg[0x54] & 0x800;
            if (keys != 0)
                break;
            sub_8044514(0x5A);
            gUnk_03000825 = keys;
            gUnk_03000820 = 0x15;
            break;
        case 21:
            if (sub_80471AC() == 0)
                gUnk_03000820 = 0x16;
            if ((*(u16 *)&arg[0x24] & 0x1000) == 0)
                break;
            *(u16 *)&arg[0x24] = (*(u16 *)&arg[0x24] & 0xEFFF) | 0x100;
            break;
        case 22:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_801CE80(arg, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 0x17;
            break;
        case 23:
            if (!(*(u16 *)&arg[0x54] & 0x1000))
                break;
            keys = *(u16 *)&arg[0xB0] & 0xDFFF;
            *(u16 *)&arg[0xB0] = keys;
            gUnk_03000820 = 9;
            break;
        case 9:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && gUnk_03000856 == 0)
                ret = 1;
            break;
    }
    sub_803F658(arg);
    gUnk_03000825 += 1;
    return ret;
}
// @ 0x0802E234
INCLUDE_ASM("asm/nonmatchings", sub_802E234);
// @ 0x0802E49C
INCLUDE_ASM("asm/nonmatchings", sub_802E49C);
// @ 0x0802E6C8
INCLUDE_ASM("asm/nonmatchings", sub_802E6C8);
// @ 0x0802EAC4
INCLUDE_ASM("asm/nonmatchings", sub_802EAC4);
// @ 0x0802EDD8
INCLUDE_ASM("asm/nonmatchings", sub_802EDD8);
// @ 0x0802F100
INCLUDE_ASM("asm/nonmatchings", sub_802F100);
// @ 0x0802F480
u32 sub_802F480(u8 *arg)
{
    u32 ret;
    u16 keys;

    ret = 0;
    sub_80187E8();
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000828 = arg[0xBF];
            gUnk_03000829 = arg[0xC0];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            gUnk_03000824 = arg[0x35];
            sub_80444A4(arg);
            sub_801CE80(arg, 5, 0x1B4, 0xC, 1);
            sub_803F5B4(arg);
            gUnk_03000825 = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 2;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0;
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (*(u16 *)&arg[0x28] > 0x46)
            {
                sub_8020CC4(arg, 0xB9, 0x78, 0x2E0, 0xE, 0x2F9, 5);
                gUnk_03000820 = 0x14;
            }
            break;
        case 20:
            keys = *(u16 *)&arg[0x54] & 0x800;
            if (keys != 0)
                break;
            sub_8044514(0x5A);
            gUnk_03000825 = keys;
            gUnk_03000820 = 0x15;
            break;
        case 21:
            if (sub_80471AC() == 0)
                gUnk_03000820 = 0x16;
            if ((*(u16 *)&arg[0x24] & 0x1000) == 0)
                break;
            *(u16 *)&arg[0x24] = (*(u16 *)&arg[0x24] & 0xEFFF) | 0x100;
            break;
        case 22:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_801CE80(arg, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 0x17;
            break;
        case 23:
            if (!(*(u16 *)&arg[0x54] & 0x1000))
                break;
            keys = *(u16 *)&arg[0xB0] & 0xDFFF;
            *(u16 *)&arg[0xB0] = keys;
            gUnk_03000820 = 9;
            break;
        case 9:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && gUnk_03000856 == 0)
                ret = 1;
            break;
    }
    sub_803F658(arg);
    gUnk_03000825 += 1;
    return ret;
}
// @ 0x0802F6D8
INCLUDE_ASM("asm/nonmatchings", sub_802F6D8);
// @ 0x0802F9EC
INCLUDE_ASM("asm/nonmatchings", sub_802F9EC);
// @ 0x0802FE98
INCLUDE_ASM("asm/nonmatchings", sub_802FE98);
// @ 0x0803029C
INCLUDE_ASM("asm/nonmatchings", sub_803029C);
// @ 0x08030664
INCLUDE_ASM("asm/nonmatchings", sub_8030664);
// @ 0x080309B0
u32 sub_80309B0(u8 *arg)
{
    u32 ret;
    u16 keys;

    ret = 0;
    sub_80187E8();
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000828 = arg[0xBF];
            gUnk_03000829 = arg[0xC0];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            gUnk_03000824 = arg[0x35];
            sub_80444A4(arg);
            sub_801CE80(arg, 5, 0x1B4, 0xC, 1);
            sub_803F5B4(arg);
            gUnk_03000825 = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 2;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0;
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (*(u16 *)&arg[0x28] > 0x46)
            {
                sub_8020CC4(arg, 0xB9, 0x78, 0x2E0, 0xE, 0x2F9, 5);
                gUnk_03000820 = 0x14;
            }
            break;
        case 20:
            keys = *(u16 *)&arg[0x54] & 0x800;
            if (keys != 0)
                break;
            sub_8044514(0x5A);
            gUnk_03000825 = keys;
            gUnk_03000820 = 0x15;
            break;
        case 21:
            if (sub_80471AC() == 0)
                gUnk_03000820 = 0x16;
            if ((*(u16 *)&arg[0x24] & 0x1000) == 0)
                break;
            *(u16 *)&arg[0x24] = (*(u16 *)&arg[0x24] & 0xEFFF) | 0x100;
            break;
        case 22:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_801CE80(arg, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 0x17;
            break;
        case 23:
            if (!(*(u16 *)&arg[0x54] & 0x1000))
                break;
            keys = *(u16 *)&arg[0xB0] & 0xDFFF;
            *(u16 *)&arg[0xB0] = keys;
            gUnk_03000820 = 9;
            break;
        case 9:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && gUnk_03000856 == 0)
                ret = 1;
            break;
    }
    sub_803F658(arg);
    gUnk_03000825 += 1;
    return ret;
}
// @ 0x08030C08
u8 sub_8030C08(u8 *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gUnk_03000889)
    {
        case 0:
            gUnk_03000825 = 0;
            gUnk_03000867 = 0x10;
            gUnk_03000868 = 0;
            f2a = *(u16 *)(obj + 0x24) | 0x10;
            *(u16 *)(obj + 0x24) = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gUnk_03000889 = 1;
            break;
        case 1:
            if (gUnk_03000825 <= 9)
            {
                gUnk_03000868 = sub_801768C(0, 0x10, 0xA, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, gUnk_03000868);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                gUnk_03000889 = 2;
            }
            break;
        case 2:
            if (gUnk_03000825 <= 0x13)
            {
                gUnk_03000867 = sub_801768C(0x10, -0x10, 0x14, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, 0x10);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                f2b = *(u16 *)(obj + 0x24) | 0x200;
                *(u16 *)(obj + 0x24) = f2b;
                gUnk_03000889 = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            sub_801A2AC(0, 0, 0);
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            gUnk_03000889 = 0x15;
            break;
        case 0x15:
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08030D9C
u8 sub_8030D9C(u8 *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gUnk_03000889)
    {
        case 0:
            gUnk_03000825 = 0;
            gUnk_03000867 = 0x10;
            gUnk_03000868 = 0;
            f2a = *(u16 *)(obj + 0x24) | 0x10;
            *(u16 *)(obj + 0x24) = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gUnk_03000889 = 1;
            break;
        case 1:
            if (gUnk_03000825 <= 9)
            {
                gUnk_03000868 = sub_801768C(0, 0x10, 0xA, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, gUnk_03000868);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                gUnk_03000889 = 2;
            }
            break;
        case 2:
            if (gUnk_03000825 <= 0x13)
            {
                gUnk_03000867 = sub_801768C(0x10, -0x10, 0x14, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, 0x10);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                f2b = *(u16 *)(obj + 0x24) | 0x200;
                *(u16 *)(obj + 0x24) = f2b;
                gUnk_03000889 = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gUnk_03000889 = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj[0xBE] = 0xFF;
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08030F30
u8 sub_8030F30(u8 *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gUnk_03000889)
    {
        case 0:
            gUnk_03000825 = 0;
            gUnk_03000867 = 0x10;
            gUnk_03000868 = 0;
            f2a = *(u16 *)(obj + 0x24) | 0x10;
            *(u16 *)(obj + 0x24) = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gUnk_03000889 = 1;
            break;
        case 1:
            if (gUnk_03000825 <= 9)
            {
                gUnk_03000868 = sub_801768C(0, 0x10, 0xA, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, gUnk_03000868);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                gUnk_03000889 = 2;
            }
            break;
        case 2:
            if (gUnk_03000825 <= 0x13)
            {
                gUnk_03000867 = sub_801768C(0x10, -0x10, 0x14, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, 0x10);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                f2b = *(u16 *)(obj + 0x24) | 0x200;
                *(u16 *)(obj + 0x24) = f2b;
                gUnk_03000889 = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gUnk_03000889 = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj[0xBE] = 0xFF;
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x080310C4
u8 sub_80310C4(u8 *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gUnk_03000889)
    {
        case 0:
            gUnk_03000825 = 0;
            gUnk_03000867 = 0x10;
            gUnk_03000868 = 0;
            f2a = *(u16 *)(obj + 0x24) | 0x10;
            *(u16 *)(obj + 0x24) = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gUnk_03000889 = 1;
            break;
        case 1:
            if (gUnk_03000825 <= 9)
            {
                gUnk_03000868 = sub_801768C(0, 0x10, 0xA, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, gUnk_03000868);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                gUnk_03000889 = 2;
            }
            break;
        case 2:
            if (gUnk_03000825 <= 0x13)
            {
                gUnk_03000867 = sub_801768C(0x10, -0x10, 0x14, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, 0x10);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                f2b = *(u16 *)(obj + 0x24) | 0x200;
                *(u16 *)(obj + 0x24) = f2b;
                gUnk_03000889 = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gUnk_03000889 = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj[0xBE] = 0xFF;
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031258
u8 sub_8031258(u8 *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gUnk_03000889)
    {
        case 0:
            gUnk_03000825 = 0;
            gUnk_03000867 = 0x10;
            gUnk_03000868 = 0;
            f2a = *(u16 *)(obj + 0x24) | 0x10;
            *(u16 *)(obj + 0x24) = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gUnk_03000889 = 1;
            break;
        case 1:
            if (gUnk_03000825 <= 9)
            {
                gUnk_03000868 = sub_801768C(0, 0x10, 0xA, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, gUnk_03000868);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                gUnk_03000889 = 2;
            }
            break;
        case 2:
            if (gUnk_03000825 <= 0x13)
            {
                gUnk_03000867 = sub_801768C(0x10, -0x10, 0x14, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, 0x10);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                f2b = *(u16 *)(obj + 0x24) | 0x200;
                *(u16 *)(obj + 0x24) = f2b;
                gUnk_03000889 = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gUnk_03000889 = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj[0xBE] = 0xFF;
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x080313EC
u8 sub_80313EC(u8 *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gUnk_03000889)
    {
        case 0:
            gUnk_03000825 = 0;
            gUnk_03000867 = 0x10;
            gUnk_03000868 = 0;
            f2a = *(u16 *)(obj + 0x24) | 0x10;
            *(u16 *)(obj + 0x24) = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gUnk_03000889 = 1;
            break;
        case 1:
            if (gUnk_03000825 <= 9)
            {
                gUnk_03000868 = sub_801768C(0, 0x10, 0xA, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, gUnk_03000868);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                gUnk_03000889 = 2;
            }
            break;
        case 2:
            if (gUnk_03000825 <= 0x13)
            {
                gUnk_03000867 = sub_801768C(0x10, -0x10, 0x14, gUnk_03000825, 2);
                sub_801A2AC(0x710, gUnk_03000867, 0x10);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000825 = 0;
                f2b = *(u16 *)(obj + 0x24) | 0x200;
                *(u16 *)(obj + 0x24) = f2b;
                gUnk_03000889 = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gUnk_03000889 = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj[0xBE] = 0xFF;
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
