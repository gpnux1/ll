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


// @ 0x0803FF54
INCLUDE_ASM("asm/nonmatchings", sub_803FF54);
// @ 0x080401AC
INCLUDE_ASM("asm/nonmatchings", sub_80401AC);
// @ 0x080405A4
u8 sub_80405A4(u8 *obj)
{
    u8 result;

    result = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = obj[0x35];
            gUnk_03000822 = *(u16 *)(obj + 0x2A);
            gUnk_03000828 = obj[0xBF];
            gUnk_03000829 = obj[0xC0];
            gUnk_03000825 = 0;
            if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x45 && (sub_80187B4() & 0x220) == 0)
            {
                u16 f2a = *(u16 *)(obj + 0x24) | 0x20;
                *(u16 *)(obj + 0x24) = f2a;
                gUnk_03000820 = 2;
            }
            else
            {
                gUnk_03000820 = 9;
            }
            break;
        case 2:
            if (gUnk_03000825 <= 0x13)
            {
                obj[0xBF] = sub_801768C(gUnk_03000828, -gUnk_03000828, 0x14, gUnk_03000825, gUnk_03000820);
                gUnk_03000825 += 1;
            }
            else
            {
                sub_80207A4();
                obj[0xBE] = 0xFF;
                obj[0xAB] = 7;
                gUnk_03000820 = 9;
            }
            break;
        case 9:
            result = 2;
            break;
    }
    return result;
}
// @ 0x08040690
INCLUDE_ASM("asm/nonmatchings", sub_8040690);
// @ 0x08040EE8
INCLUDE_ASM("asm/nonmatchings", sub_8040EE8);
// @ 0x08041308
INCLUDE_ASM("asm/nonmatchings", sub_8041308);
// @ 0x080416F0
INCLUDE_ASM("asm/nonmatchings", sub_80416F0);
// @ 0x080419E0
INCLUDE_ASM("asm/nonmatchings", sub_80419E0);
// @ 0x08041EDC
INCLUDE_ASM("asm/nonmatchings", sub_8041EDC);
// @ 0x08042200
u8 sub_8042200(u8 *obj)
{
    u8 result;

    result = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = obj[0x35];
            gUnk_03000822 = *(u16 *)(obj + 0x2A);
            sub_801CA08((BattleObj *)obj, 3, 0x1B4, 0xD, result);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (*(u16 *)(obj + 0x24) & 0x1000)
            {
                sub_804C3A4(obj[0x35], sub_801B954((ObjHead *)(obj + 0xC)));
                *(u16 *)(obj + 0x24) &= 0xEFFF;
                sub_80207DC((BattleObj *)obj, obj[0xBF], obj[0xC0], gUnk_03000822, gUnk_03000824);
                gUnk_03000820 = 9;
            }
            break;
        case 9:
            result = 2;
            break;
    }
    return result;
}
// @ 0x080422B8
INCLUDE_ASM("asm/nonmatchings", sub_80422B8);
// @ 0x08042784
INCLUDE_ASM("asm/nonmatchings", sub_8042784);
// @ 0x08042AB4
u8 sub_8042AB4(u8 *obj)
{
    u8 result;

    result = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = obj[0x35];
            gUnk_03000822 = *(u16 *)(obj + 0x2A);
            gUnk_03000828 = obj[0xBF];
            gUnk_03000829 = obj[0xC0];
            gUnk_03000825 = 0;
            if ((sub_80187B4() & 0x200) == 0)
            {
                u16 f2a = *(u16 *)(obj + 0x24) | 0x20;
                *(u16 *)(obj + 0x24) = f2a;
                gUnk_03000820 = 2;
            }
            else
            {
                gUnk_03000820 = 0x12;
            }
            break;
        case 2:
            if (gUnk_03000825 <= 0x13)
            {
                obj[0xBF] = sub_801768C(gUnk_03000828, -gUnk_03000828, 0x14, gUnk_03000825, gUnk_03000820);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000820 = 9;
            }
            break;
        case 9:
            sub_80207A4();
            obj[0xAB] = 7;
            obj[0xBE] = 0xFF;
            result = 2;
            break;
        case 0x12:
            result = 2;
            break;
    }
    return result;
}
// @ 0x08042B90
INCLUDE_ASM("asm/nonmatchings", sub_8042B90);
// @ 0x08042E70
INCLUDE_ASM("asm/nonmatchings", sub_8042E70);
// @ 0x08043554
INCLUDE_ASM("asm/nonmatchings", sub_8043554);
// @ 0x08043938
INCLUDE_ASM("asm/nonmatchings", sub_8043938);
// @ 0x08043B5C
INCLUDE_ASM("asm/nonmatchings", sub_8043B5C);
// @ 0x08043F90
INCLUDE_ASM("asm/nonmatchings", sub_8043F90);
