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


// @ 0x08032548
INCLUDE_ASM("asm/nonmatchings", sub_8032548);
// @ 0x0803272C
INCLUDE_ASM("asm/nonmatchings", sub_803272C);
// @ 0x08032948
INCLUDE_ASM("asm/nonmatchings", sub_8032948);
// @ 0x08032D74
u8 sub_8032D74(u8 *obj)
{
    u8 result;

    result = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = obj[0x35];
            gUnk_03000822 = *(u16 *)(obj + 0x2A);
            sub_8048B30(0, 0x1E, obj[0xBE] == 0 ? 0x362 : 0x371);
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (sub_8047B1C(obj) == 1)
                gUnk_03000820 = 0x14;
            break;
        case 20:
            sub_801CBA4(obj, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 6;
            break;
        case 6:
            if (!(*(u16 *)(obj + 0x24) & 0x800))
                gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(obj, obj[0xA1]);
            result = 2;
            break;
    }
    return result;
}
// @ 0x08032EA0
INCLUDE_ASM("asm/nonmatchings", sub_8032EA0);
// @ 0x080334B8
INCLUDE_ASM("asm/nonmatchings", sub_80334B8);
// @ 0x08033988
INCLUDE_ASM("asm/nonmatchings", sub_8033988);
// @ 0x08033E2C
INCLUDE_ASM("asm/nonmatchings", sub_8033E2C);
// @ 0x08034440
u32 sub_8034440(u8 *arg)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            sub_8020DE4();
            gUnk_03000820 = 1;
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x386, 0x1B4, 0xD, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4(arg, zero, gUnk_03000822, gUnk_03000824, zero);
            gUnk_03000820 = 6;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 8;
            break;
        case 8:
            if (sub_801EEE4(arg, GetObjPool(), 0, 0, 0x32) == 1)
                gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    return ret;
}
/*
u32 sub_8034440(u8 *arg)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        sub_8020DE4();
        gUnk_03000820 = 1;
        gUnk_03000824 = arg[0x35];
        gUnk_03000822 = *(u16 *)&arg[0x2A];
        break;
    case 1:
        sub_8020974(arg + 0xC, 0x386, 0x1B4, 0xD, 2);
        gUnk_03000820 = 2;
        break;
    case 2:
        if (*(u16 *)&arg[0x24] & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gUnk_03000820 = 5;
        break;
    case 5:
        if (!(*(u16 *)&arg[0x24] & 0x1000))
            break;
        sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
        keys = *(u16 *)&arg[0x24] & 0xEFFF;
        zero = 0;
        *(u16 *)&arg[0x24] = keys;
        sub_801CBA4(arg, zero, gUnk_03000822, gUnk_03000824, zero);
        gUnk_03000820 = 6;
        break;
    case 6:
        if (*(u16 *)&arg[0x24] & 0x800)
            break;
        gUnk_03000820 = 8;
        break;
    case 8:
        if (sub_801EEE4(arg, GetObjPool(), 0, 0, 0x32) == 1)
            gUnk_03000820 = 9;
        break;
    case 9:
        sub_8045B90(arg, arg[0xA1]);
        ret = 2;
        break;
    }
    return ret;
}
*/
// @ 0x080345AC
u32 sub_80345AC(u8 *arg)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            sub_8020DE4();
            gUnk_03000820 = 1;
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x386, 0x1B4, 0xD, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4(arg, zero, gUnk_03000822, gUnk_03000824, zero);
            gUnk_03000820 = 6;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 8;
            break;
        case 8:
            if (sub_801EEE4(arg, GetObjPool(), 0, 0xB, 0x1E) == 1)
                gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x08034718
u32 sub_8034718(u8 *arg, u8 *arg1)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            sub_8048B30(0, 0x1E, 0x3AD);
            gUnk_03000820 = 1;
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x386, 0x1B4, 0xD, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4(arg, zero, gUnk_03000822, gUnk_03000824, zero);
            gUnk_03000820 = 6;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 0x12;
            break;
        case 0x12:
            if (sub_80476DC(arg, arg1) == 1)
                gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x080348A8
u32 sub_80348A8(u8 *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u8 *ptr;
    u32 zero;
    u16 keys;

    ret = 0;
    pool = GetObjPool();
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000820 = 1;
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            gUnk_0300083C = sub_80489E8((u8 *)pool, gUnk_03000830, 1, 0x7F);
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x386, 0x1B4, 0xD, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4(arg, zero, gUnk_03000822, gUnk_03000824, zero);
            gUnk_03000820 = 6;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            if (sub_80187B4() & 0x220)
                gUnk_03000820 = 9;
            else
                gUnk_03000820 = 0x18;
            break;
        case 24:
            sub_8020CC4(arg, 0x3C, 0x73, 0x1B4, 0xE, 0x37F, 0x14);
            arg[0x66] = 0;
            sub_801A2AC(0x410, 0, 7);
            gUnk_03000820 = 0x19;
            break;
        case 25:
            if (*(u16 *)&arg[0x54] & 0x800)
                break;
            Sfx_Play(0xA5, 0, 0);
            gUnk_03000820 = 0x1A;
            break;
        case 26:
            if (!(*(u16 *)&arg[0x54] & 0x1000))
                break;
            sub_804C3A4(arg[0x65], sub_801B954((void **)(arg + 0x3C)));
            sub_8020CC4(arg, 0x3C, 0x73, 0x1B4, 0xE, 0x380, 0x114);
            gUnk_03000825 = 0;
            gUnk_03000820 = 0x1B;
            break;
        case 27:
            if (*(u16 *)&arg[0x54] & 0x800)
                break;
            for (i = 0; i < gUnk_0300083C; i++)
            {
                ptr = (u8 *)(pool + gUnk_03000830[i] * 0xC8);
                ptr[0xBE] = 0xFF;
                ptr[0xAB] = 7;
                sub_80207A4();
            }
            gUnk_03000820 = 0x1C;
            break;
        case 28:
            if (gUnk_03000825 <= 0x27)
                gUnk_03000825 += 1;
            else
            {
                keys = *(u16 *)&arg[0x54] & 0xFEFF;
                *(u16 *)&arg[0x54] = keys;
                gUnk_03000820 = 0x1D;
            }
            break;
        case 29:
            if (!(*(u16 *)&arg[0x54] & 0x1000))
                break;
            sub_804C3A4(arg[0x65], sub_801B954((void **)(arg + 0x3C)));
            keys = *(u16 *)&arg[0xB0] & 0xDFFF;
            *(u16 *)&arg[0xB0] = keys;
            gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x08034BFC
u32 sub_8034BFC(u8 *arg)
{
    u32 ret;
    u32 pool;
    u8 buf[8];
    u8 i;
    u8 count;
    u32 zero;
    u16 keys;

    ret = 0;
    pool = GetObjPool();
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000820 = 1;
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x386, 0x1B4, 0xD, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4(arg, zero, gUnk_03000822, gUnk_03000824, zero);
            gUnk_03000820 = 6;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 8;
            break;
        case 8:
            count = sub_80489E8((u8 *)pool, buf, 1, 7);
            for (i = 0; i < count; i++)
            {
                if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
                    sub_8045F94(pool + buf[i] * 0xC8, 3);
            }
            gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x08034D94
u32 sub_8034D94(u8 *arg)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            sub_8020DE4();
            gUnk_03000820 = 1;
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x386, 0x1B4, 0xD, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4(arg, zero, gUnk_03000822, gUnk_03000824, zero);
            gUnk_03000820 = 6;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 8;
            break;
        case 8:
            if (sub_801EEE4(arg, GetObjPool(), 1, 0, 0x3C) == 1)
                gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x08034F00
u32 sub_8034F00(u8 *arg)
{
    u32 ret;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            sub_80444A4(arg);
            sub_803F5B4(arg);
            gUnk_03000820 = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x38B;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gUnk_03000825 = zero;
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x387, 0x1B4, 0xD, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974(arg + 0xC, 0x388, 0x1B4, 0xD, 2);
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            sub_8044514(0x32);
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gUnk_03000820 = 0x14;
            }
            gUnk_03000825 += 1;
            break;
        case 20:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && (v56 = gUnk_03000856) == 0)
            {
                sub_801CBA4(arg, 0, gUnk_03000822, gUnk_03000824, v56);
                gUnk_03000820 = 6;
            }
            gUnk_03000825 += 1;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(arg, arg[0xA1]);
            ret = 1;
            break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x08035130
u32 sub_8035130(u8 *arg)
{
    u32 ret;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            sub_80444A4(arg);
            sub_803F5B4(arg);
            gUnk_03000820 = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x38C;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x14;
            gUnk_03000825 = zero;
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x387, 0x1B4, 0xD, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974(arg + 0xC, 0x388, 0x1B4, 0xD, 2);
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            sub_8044514(0x32);
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gUnk_03000820 = 0x14;
            }
            gUnk_03000825 += 1;
            break;
        case 20:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && (v56 = gUnk_03000856) == 0)
            {
                sub_801CBA4(arg, 0, gUnk_03000822, gUnk_03000824, v56);
                gUnk_03000820 = 6;
            }
            gUnk_03000825 += 1;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(arg, arg[0xA1]);
            ret = 1;
            break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x08035360
INCLUDE_ASM("asm/nonmatchings", sub_8035360);
// @ 0x0803586C
u32 sub_803586C(u8 *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            sub_80444A4(arg);
            sub_803F5B4(arg);
            gUnk_03000820 = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x392;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gUnk_03000825 = zero;
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x387, 0x1B4, 0xC, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974(arg + 0xC, 0x388, 0x1B4, 0xC, 2);
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            sub_8044514(0x32);
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gUnk_03000820 = 0x14;
            }
            gUnk_03000825 += 1;
            break;
        case 20:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && (v56 = gUnk_03000856) == 0)
            {
                sub_801CBA4(arg, 0, gUnk_03000822, gUnk_03000824, v56);
                gUnk_03000820 = 6;
            }
            gUnk_03000825 += 1;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gUnk_0300083D; i++)
            {
                if ((gUnk_03000840[i] & 0xF0) == 0x10)
                {
                    if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
                        sub_8045F94(pool + (gUnk_03000840[i] & 0xF) * 0xC8, 2);
                }
            }
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x08035B04
u32 sub_8035B04(u8 *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            sub_80444A4(arg);
            sub_803F5B4(arg);
            gUnk_03000820 = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x393;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gUnk_03000825 = zero;
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x387, 0x1B4, 0xC, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974(arg + 0xC, 0x388, 0x1B4, 0xC, 2);
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            sub_8044514(0x32);
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gUnk_03000820 = 0x14;
            }
            gUnk_03000825 += 1;
            break;
        case 20:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && (v56 = gUnk_03000856) == 0)
            {
                sub_801CBA4(arg, 0, gUnk_03000822, gUnk_03000824, v56);
                gUnk_03000820 = 6;
            }
            gUnk_03000825 += 1;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gUnk_0300083D; i++)
            {
                if ((gUnk_03000840[i] & 0xF0) == 0x10)
                {
                    if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
                        sub_8045F94(pool + (gUnk_03000840[i] & 0xF) * 0xC8, 3);
                }
            }
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x08035D9C
u32 sub_8035D9C(u8 *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            sub_80444A4(arg);
            sub_803F5B4(arg);
            gUnk_03000820 = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x394;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gUnk_03000825 = zero;
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x387, 0x1B4, 0xC, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974(arg + 0xC, 0x388, 0x1B4, 0xC, 2);
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            sub_8044514(0x32);
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gUnk_03000820 = 0x14;
            }
            gUnk_03000825 += 1;
            break;
        case 20:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && (v56 = gUnk_03000856) == 0)
            {
                sub_801CBA4(arg, 0, gUnk_03000822, gUnk_03000824, v56);
                gUnk_03000820 = 6;
            }
            gUnk_03000825 += 1;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gUnk_0300083D; i++)
            {
                if ((gUnk_03000840[i] & 0xF0) == 0x10)
                {
                    if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
                        sub_8045F94(pool + (gUnk_03000840[i] & 0xF) * 0xC8, 5);
                }
            }
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x08036034
u32 sub_8036034(u8 *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            sub_80444A4(arg);
            sub_803F5B4(arg);
            gUnk_03000820 = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x395;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gUnk_03000825 = zero;
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x387, 0x1B4, 0xC, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974(arg + 0xC, 0x388, 0x1B4, 0xC, 2);
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            sub_8044514(0x32);
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gUnk_03000820 = 0x14;
            }
            gUnk_03000825 += 1;
            break;
        case 20:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && (v56 = gUnk_03000856) == 0)
            {
                sub_801CBA4(arg, 0, gUnk_03000822, gUnk_03000824, v56);
                gUnk_03000820 = 6;
            }
            gUnk_03000825 += 1;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gUnk_0300083D; i++)
            {
                if ((gUnk_03000840[i] & 0xF0) == 0x10)
                {
                    if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
                        sub_8045F94(pool + (gUnk_03000840[i] & 0xF) * 0xC8, 6);
                }
            }
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x080362CC
u32 sub_80362CC(u8 *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = arg[0x35];
            gUnk_03000822 = *(u16 *)&arg[0x2A];
            sub_80444A4(arg);
            sub_803F5B4(arg);
            gUnk_03000820 = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x396;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gUnk_03000825 = zero;
            break;
        case 1:
            sub_8020974(arg + 0xC, 0x387, 0x1B4, 0xC, 2);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gUnk_03000820 = 5;
            break;
        case 5:
            if (!(*(u16 *)&arg[0x24] & 0x1000))
                break;
            sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974(arg + 0xC, 0x388, 0x1B4, 0xC, 2);
            gUnk_03000820 = 0x12;
            break;
        case 18:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            sub_8044514(0x32);
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gUnk_03000820 = 0x14;
            }
            gUnk_03000825 += 1;
            break;
        case 20:
            if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && (v56 = gUnk_03000856) == 0)
            {
                sub_801CBA4(arg, 0, gUnk_03000822, gUnk_03000824, v56);
                gUnk_03000820 = 6;
            }
            gUnk_03000825 += 1;
            break;
        case 6:
            if (*(u16 *)&arg[0x24] & 0x800)
                break;
            gUnk_03000820 = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gUnk_0300083D; i++)
            {
                if ((gUnk_03000840[i] & 0xF0) == 0x10)
                {
                    if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x13)
                        sub_8045F94(pool + (gUnk_03000840[i] & 0xF) * 0xC8, 4);
                }
            }
            sub_8045B90(arg, arg[0xA1]);
            ret = 2;
            break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x08036564
INCLUDE_ASM("asm/nonmatchings", sub_8036564);
// @ 0x080368FC
INCLUDE_ASM("asm/nonmatchings", sub_80368FC);
// @ 0x08036B30
INCLUDE_ASM("asm/nonmatchings", sub_8036B30);
// @ 0x08036EA4
INCLUDE_ASM("asm/nonmatchings", sub_8036EA4);
// @ 0x08037078
INCLUDE_ASM("asm/nonmatchings", sub_8037078);
// @ 0x08037388
INCLUDE_ASM("asm/nonmatchings", sub_8037388);
// @ 0x08037868
INCLUDE_ASM("asm/nonmatchings", sub_8037868);
// @ 0x08037C40
INCLUDE_ASM("asm/nonmatchings", sub_8037C40);
// @ 0x08037E14
u32 sub_8037E14(u8 *obj)
{
    u32 ret;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        sub_8020DE4();
        gUnk_03000820 = 0x12;
        gUnk_03000824 = obj[0x35];
        gUnk_03000822 = *(u16 *)(obj + 0x2A);
        break;
    case 18:
        sub_8020974(obj + 0xC, 0x3B9, 0x1B4, 0xC, 2);
        gUnk_03000820 = 0x13;
        break;
    case 19:
        if (*(u16 *)&obj[0x24] & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gUnk_03000820 = 0x14;
        break;
    case 20:
        if (!(*(u16 *)&obj[0x24] & 0x1000))
            break;
        sub_804C3A4(obj[0x35], sub_801B954((void **)(obj + 0xC)));
        *(u16 *)&obj[0x24] &= 0xEFFF;
        sub_8020974(obj + 0xC, 0x3BA, 0x1B4, 0xC, 2);
        gUnk_03000820 = 0x15;
        break;
    case 21:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 0, 0, 0x3C) == 1)
        {
            sub_801CBA4(obj, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 0x17;
        }
        break;
    case 23:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 9;
        break;
    case 9:
        sub_8045B90(obj, obj[0xA1]);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x08037FE8
u32 sub_8037FE8(u8 *obj)
{
    u32 ret;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        sub_8020DE4();
        gUnk_03000820 = 0x12;
        gUnk_03000824 = obj[0x35];
        gUnk_03000822 = *(u16 *)(obj + 0x2A);
        break;
    case 18:
        sub_8020974(obj + 0xC, 0x3B9, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x13;
        break;
    case 19:
        if (*(u16 *)&obj[0x24] & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gUnk_03000820 = 0x14;
        break;
    case 20:
        if (!(*(u16 *)&obj[0x24] & 0x1000))
            break;
        sub_804C3A4(obj[0x35], sub_801B954((void **)(obj + 0xC)));
        *(u16 *)&obj[0x24] &= 0xEFFF;
        sub_8020974(obj + 0xC, 0x3BA, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x15;
        break;
    case 21:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 1, 0, 0x28) == 1)
        {
            sub_801CBA4(obj, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 0x17;
        }
        break;
    case 23:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 9;
        break;
    case 9:
        sub_8045B90(obj, obj[0xA1]);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x080381BC
u32 sub_80381BC(u8 *obj)
{
    u32 ret;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        sub_8020DE4();
        gUnk_03000820 = 0x12;
        gUnk_03000824 = obj[0x35];
        gUnk_03000822 = *(u16 *)(obj + 0x2A);
        break;
    case 18:
        sub_8020974(obj + 0xC, 0x3B9, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x13;
        break;
    case 19:
        if (*(u16 *)&obj[0x24] & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gUnk_03000820 = 0x14;
        break;
    case 20:
        if (!(*(u16 *)&obj[0x24] & 0x1000))
            break;
        sub_804C3A4(obj[0x35], sub_801B954((void **)(obj + 0xC)));
        *(u16 *)&obj[0x24] &= 0xEFFF;
        sub_8020974(obj + 0xC, 0x3BA, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x15;
        break;
    case 21:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 1, 0xC, 0x1E) == 1)
        {
            sub_801CBA4(obj, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 0x17;
        }
        break;
    case 23:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 9;
        break;
    case 9:
        sub_8045B90(obj, obj[0xA1]);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x08038390
u32 sub_8038390(u8 *obj)
{
    u32 ret;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        sub_8020DE4();
        gUnk_03000820 = 0x12;
        gUnk_03000824 = obj[0x35];
        gUnk_03000822 = *(u16 *)(obj + 0x2A);
        break;
    case 18:
        sub_8020974(obj + 0xC, 0x3B9, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x13;
        break;
    case 19:
        if (*(u16 *)&obj[0x24] & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gUnk_03000820 = 0x14;
        break;
    case 20:
        if (!(*(u16 *)&obj[0x24] & 0x1000))
            break;
        sub_804C3A4(obj[0x35], sub_801B954((void **)(obj + 0xC)));
        *(u16 *)&obj[0x24] &= 0xEFFF;
        sub_8020974(obj + 0xC, 0x3BA, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x15;
        break;
    case 21:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 0, 0, 0x3E7) == 1)
        {
            sub_801CBA4(obj, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 0x17;
        }
        break;
    case 23:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 9;
        break;
    case 9:
        sub_8045B90(obj, obj[0xA1]);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x08038568
u32 sub_8038568(u8 *arg, u8 *arg1)
{
    u32 ret;
    u16 t1;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        sub_8020DE4();
        gUnk_03000820 = 0x12;
        gUnk_03000824 = arg[0x35];
        gUnk_03000822 = *(u16 *)(arg + 0x2A);
        break;
    case 18:
        sub_8020974(arg + 0xC, 0x3B9, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x13;
        break;
    case 19:
        if (*(u16 *)&arg[0x24] & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gUnk_03000820 = 0x14;
        break;
    case 20:
        if (!(*(u16 *)&arg[0x24] & 0x1000))
            break;
        sub_804C3A4(arg[0x35], sub_801B954((void **)(arg + 0xC)));
        *(u16 *)&arg[0x24] &= 0xEFFF;
        sub_8020974(arg + 0xC, 0x3BA, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x15;
        break;
    case 21:
        if (!(*(u16 *)&arg[0x24] & 0x800))
            gUnk_03000820 = 0x16;
        break;
    case 22:
        t1 = *(u16 *)(arg1 + 0x6E) / 3;
        if (sub_801EEE4(arg, GetObjPool(), 0, 0xA, t1) == 1)
        {
            sub_801CBA4(arg, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 0x17;
        }
        break;
    case 23:
        if (!(*(u16 *)&arg[0x24] & 0x800))
            gUnk_03000820 = 9;
        break;
    case 9:
        sub_8045B90(arg, arg[0xA1]);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x0803874C
u32 sub_803874C(u8 *obj)
{
    u32 ret;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        sub_8020DE4();
        gUnk_03000820 = 0x12;
        gUnk_03000824 = obj[0x35];
        gUnk_03000822 = *(u16 *)(obj + 0x2A);
        break;
    case 18:
        sub_8020974(obj + 0xC, 0x3B9, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x13;
        break;
    case 19:
        if (*(u16 *)&obj[0x24] & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gUnk_03000820 = 0x14;
        break;
    case 20:
        if (!(*(u16 *)&obj[0x24] & 0x1000))
            break;
        sub_804C3A4(obj[0x35], sub_801B954((void **)(obj + 0xC)));
        *(u16 *)&obj[0x24] &= 0xEFFF;
        sub_8020974(obj + 0xC, 0x3BA, 0x1B4, 0xD, 2);
        gUnk_03000820 = 0x15;
        break;
    case 21:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 0, 0xB, 0x1E) == 1)
        {
            sub_801CBA4(obj, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 0x17;
        }
        break;
    case 23:
        if (!(*(u16 *)&obj[0x24] & 0x800))
            gUnk_03000820 = 9;
        break;
    case 9:
        sub_8045B90(obj, obj[0xA1]);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x08038920
INCLUDE_ASM("asm/nonmatchings", sub_8038920);
// @ 0x08038C84
INCLUDE_ASM("asm/nonmatchings", sub_8038C84);
// @ 0x08038E44
INCLUDE_ASM("asm/nonmatchings", sub_8038E44);
// @ 0x08039024
INCLUDE_ASM("asm/nonmatchings", sub_8039024);
// @ 0x080392C0
u8 sub_80392C0(u8 *obj)
{
    u8 result;

    result = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = obj[0x35];
            gUnk_03000822 = *(u16 *)(obj + 0x2A);
            sub_8048B30(0, 0x1E, 0x3CB);
            gUnk_03000820 = 0x13;
            break;
        case 19:
            if (sub_8047B1C(obj) == 1)
                gUnk_03000820 = 0x14;
            break;
        case 20:
            sub_801CBA4(obj, 0, gUnk_03000822, gUnk_03000824, 0);
            gUnk_03000820 = 6;
            break;
        case 6:
            if (!(*(u16 *)(obj + 0x24) & 0x800))
                gUnk_03000820 = 9;
            break;
        case 9:
            sub_8045B90(obj, obj[0xA1]);
            result = 2;
            break;
    }
    return result;
}
// @ 0x080393E0
INCLUDE_ASM("asm/nonmatchings", sub_80393E0);
// @ 0x08039724
INCLUDE_ASM("asm/nonmatchings", sub_8039724);
// @ 0x08039C38
INCLUDE_ASM("asm/nonmatchings", sub_8039C38);
// @ 0x0803A478
INCLUDE_ASM("asm/nonmatchings", sub_803A478);
// @ 0x0803A8D0
INCLUDE_ASM("asm/nonmatchings", sub_803A8D0);
// @ 0x0803AF60
INCLUDE_ASM("asm/nonmatchings", sub_803AF60);
// @ 0x0803B484
INCLUDE_ASM("asm/nonmatchings", sub_803B484);
// @ 0x0803BBEC
INCLUDE_ASM("asm/nonmatchings", sub_803BBEC);
// @ 0x0803C328
INCLUDE_ASM("asm/nonmatchings", sub_803C328);
// @ 0x0803CE0C
INCLUDE_ASM("asm/nonmatchings", sub_803CE0C);
// @ 0x0803D20C
INCLUDE_ASM("asm/nonmatchings", sub_803D20C);
// @ 0x0803D60C
INCLUDE_ASM("asm/nonmatchings", sub_803D60C);
// @ 0x0803DECC
INCLUDE_ASM("asm/nonmatchings", sub_803DECC);
// @ 0x0803E58C
INCLUDE_ASM("asm/nonmatchings", sub_803E58C);
// @ 0x0803ED34
INCLUDE_ASM("asm/nonmatchings", sub_803ED34);
// @ 0x0803F21C
INCLUDE_ASM("asm/nonmatchings", sub_803F21C);
// @ 0x0803F328
u8 sub_803F328(u8 arg0)
{
    u8 result;
    u8 v;
    int base;

    result = 0;
    switch (gUnk_0300086A)
    {
        case 0:
            result = 1;
            break;
        case 1:
            DialogCtx_Clear3();
            Bg0_InitClear();
            base = 0x02035AC0;
            v = 2;
            sub_80196D4(0, (u8 *)base, 0xB, 2, 2, 1, 2, 0xC, 4);
            gUnk_03000825 = 0;
            gUnk_0300086A = v;
            break;
        case 2:
            if (DialogCtx_GetField_C(0) == 4)
            {
                sub_803F21C((u8 *)0x02035AC0, arg0);
                sub_80187C0(0x400);
                gUnk_0300086A = 3;
            }
            break;
        case 3:
            if (!(sub_80187B4() & 0x400))
                gUnk_0300086A = 4;
            break;
        case 4:
            if (gUnk_03000825 <= 0x27)
                gUnk_03000825 += 1;
            else
            {
                DialogCtx_SetHead(0, 2, 2);
                gUnk_0300086A = 5;
            }
            break;
        case 5:
            v = DialogCtx_GetField_C(0);
            if (v == 0)
            {
                Disp_Bg1Off();
                gUnk_0300086A = v;
            }
            break;
    }
    sub_801933C();
    return result;
}
// @ 0x0803F444
INCLUDE_ASM("asm/nonmatchings", sub_803F444);
// @ 0x0803F5B4
void sub_803F5B4(u8 *obj)
{
    u32 pool;
    u8 i;
    u8 *buf;
    u32 result;

    pool = GetObjPool();
    sub_8020DF0(obj);
    gUnk_0300083D = sub_8020E5C();
    buf = (u8 *)sub_8020E54();
    gUnk_03000840 = buf;
    gUnk_03000858 = 0;
    for (i = 0; i < gUnk_0300083D; i++)
    {
        if ((gUnk_03000840[i] & 0xF0) == 0x10)
        {
            result = sub_804473C(obj, pool + (gUnk_03000840[i] & 0xF) * 0xC8);
            result += *(u16 *)(pool + (gUnk_03000840[i] & 0xF) * 0xC8 + 0xB2);
            *(u16 *)(pool + (gUnk_03000840[i] & 0xF) * 0xC8 + 0xB2) = result;
            gUnk_03000858++;
        }
    }
}
/*
void sub_803F5B4(u8 *arg0)
{
    u32 pool;
    u8 i;
    s32 ret;

    pool = GetObjPool();
    sub_8020DF0(arg0);
    gUnk_0300083D = sub_8020E5C();
    gUnk_03000840 = sub_8020E54();
    gUnk_03000858 = 0;
    for (i = 0; i < gUnk_0300083D; i++) {
        if ((((u8 *)gUnk_03000840)[i] & 0xF0) == 0x10) {
            ret = sub_804473C(arg0, pool + (((u8 *)gUnk_03000840)[i] & 0xF) * 0xC8);
            ret += *(u16 *)(pool + (((u8 *)gUnk_03000840)[i] & 0xF) * 0xC8 + 0xB2);
            *(u16 *)(pool + (((u8 *)gUnk_03000840)[i] & 0xF) * 0xC8 + 0xB2) = ret;
            gUnk_03000858++;
        }
    }
}
*/
// @ 0x0803F658
INCLUDE_ASM("asm/nonmatchings", sub_803F658);
