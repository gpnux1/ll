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
u8 sub_80405A4(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = obj->headA.palSlot;
            gUnk_03000822 = obj->headA.f_1E;
            gUnk_03000828 = obj->posX;
            gUnk_03000829 = obj->posY;
            gUnk_03000825 = 0;
            if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x45 && (sub_80187B4() & 0x220) == 0)
            {
                u16 f2a = obj->headA.kindFlags | 0x20;
                obj->headA.kindFlags = f2a;
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
                obj->posX = sub_801768C(gUnk_03000828, -gUnk_03000828, 0x14, gUnk_03000825, gUnk_03000820);
                gUnk_03000825 += 1;
            }
            else
            {
                sub_80207A4();
                obj->slot = 0xFF;
                obj->variantClass = 7;
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
u8 sub_8042200(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = obj->headA.palSlot;
            gUnk_03000822 = obj->headA.f_1E;
            sub_801CA08((BattleObj *)obj, 3, 0x1B4, 0xD, result);
            gUnk_03000820 = 2;
            break;
        case 2:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)(&obj->headA)));
                obj->headA.kindFlags &= 0xEFFF;
                sub_80207DC((BattleObj *)obj, obj->posX, obj->posY, gUnk_03000822, gUnk_03000824);
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
u8 sub_8042AB4(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gUnk_03000820)
    {
        case 0:
            gUnk_03000824 = obj->headA.palSlot;
            gUnk_03000822 = obj->headA.f_1E;
            gUnk_03000828 = obj->posX;
            gUnk_03000829 = obj->posY;
            gUnk_03000825 = 0;
            if ((sub_80187B4() & 0x200) == 0)
            {
                u16 f2a = obj->headA.kindFlags | 0x20;
                obj->headA.kindFlags = f2a;
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
                obj->posX = sub_801768C(gUnk_03000828, -gUnk_03000828, 0x14, gUnk_03000825, gUnk_03000820);
                gUnk_03000825 += 1;
            }
            else
            {
                gUnk_03000820 = 9;
            }
            break;
        case 9:
            sub_80207A4();
            obj->variantClass = 7;
            obj->slot = 0xFF;
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
