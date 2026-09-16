#include "battle_types.h"
#include "battle_stage_effects.h"
#include "battle_object_engine.h"
#include "battle_palette_wipe.h"
#include "battle_task_services.h"
#include "battle_flow_rules.h"
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
u8 sub_803FF54(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            sub_801CBA4(obj, 8, 0x1B4, 0xD, 0);
            gObjActStep = 0x30;
            gObjActStepTimer = 0;
            break;
        case 0x30:
            if (sub_8048458(obj) == 1)
            {
                gUnk_03000864 = 0;
            }
            else
            {
                gUnk_03000864 = 1;
            }
            gObjActStep = 0x33;
            break;
        case 0x33:
            if (gObjActStepTimer <= 0x31)
            {
                gObjActStepTimer += 1;
            }
            else
            {
                if (gUnk_03000864 == 0)
                {
                    gObjActStep = 0x34;
                }
                else
                {
                    gObjActStep = 0x36;
                }
                gObjActStepTimer = 0;
            }
            break;
        case 0x34:
            if (gObjActStepTimer <= 0x18)
            {
                obj->posX = sub_801768C(gObjActSavedX, 0x100 - gObjActSavedX, 0x19, gObjActStepTimer, 1);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStep = 0x35;
            }
            break;
        case 0x35:
            obj->slot = 0xFF;
            obj->variantClass = 7;
            gObjActStep = 0x38;
            gObjActDoneCount += 1;
            result = 1;
            break;
        case 0x36:
            sub_801CBA4(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x38;
            break;
        case 0x38:
            result = 1;
            break;
    }
    return result;
}
// @ 0x080401AC
u32 sub_80401AC(void)
{
    u32 result;
    BattleObj *pool;
    u8 i;

    result = 0;
    pool = (BattleObj *)GetObjPool();
    switch (gObjActStep)
    {
        case 0:
            gTargetSlotCount = sub_80489E8(pool, gTargetSlotList, 0, 0x43);
            for (i = 0; i < gTargetSlotCount; i++)
            {
                gUnk_03000870[i] = pool[gTargetSlotList[i]].posX;
            }
            gObjActMoveFromX = 0;
            gObjActStep = 0x31;
            gObjActStepTimer = 0;
            break;

        case 0x31:
            if (gObjActMoveFromX < gTargetSlotCount)
            {
                BattleObj *obj = (BattleObj *)(gTargetSlotList[gObjActMoveFromX] * 0xC8 + (u32)pool);
                u8 slot = obj->slot;
                AnimEntry24 *entry = &gUnk_083988A8[slot];
                sub_8020974(&obj->headA, entry->animId, (u16)(gObjActMoveFromX * 32 + 0xDA) * 2, obj->headA.palSlot, 0x401);
                gObjActStep = 0x32;
            }
            else
            {
                gObjActStep = 0x30;
            }
            break;

        case 0x32:
            if ((pool[gTargetSlotList[gObjActMoveFromX]].headA.kindFlags & 0x800) == 0)
            {
                gObjActMoveFromX++;
                gObjActStep = 0x31;
            }
            break;

        case 0x30:
            if (sub_8048310() == 1)
            {
                gUnk_03000864 = 0;
            }
            else
            {
                gUnk_03000864 = 1;
            }
            gObjActMoveFromX = 0;
            gObjActStep = 0x33;
            break;

        case 0x33:
            if (gObjActStepTimer <= 0x31)
            {
                gObjActStepTimer++;
            }
            else
            {
                if (gUnk_03000864 == 0)
                {
                    gObjActStep = 0x34;
                }
                else
                {
                    gObjActStep = 0x36;
                }
                gObjActStepTimer = 0;
            }
            break;

        case 0x34:
            if (gObjActStepTimer <= 0x18)
            {
                for (i = 0; i < gTargetSlotCount; i++)
                {
                    pool[gTargetSlotList[i]].posX = sub_801768C(gUnk_03000870[i], 0x100 - gUnk_03000870[i], 0x19, gObjActStepTimer, 1);
                }
                gObjActStepTimer++;
            }
            else
            {
                for (i = 0; i < gTargetSlotCount; i++)
                {
                    pool[gTargetSlotList[i]].slot |= 0xFF;
                }
                gObjActStep = 0x35;
            }
            break;

        case 0x35:
            gObjActStep = 0x38;
            result = 1;
            break;

        case 0x36:
            if (gObjActMoveFromX < gTargetSlotCount)
            {
                u8 slot = gTargetSlotList[gObjActMoveFromX];
                sub_801CBA4(&pool[slot], 0, slot * 0x16 + 9, pool[slot].headA.palSlot, 0);
                gObjActStep = 0x37;
            }
            else
            {
                sub_80187C0(0x1000);
                gObjActStep = 0x38;
            }
            break;

        case 0x37:
            if ((pool[gTargetSlotList[gObjActMoveFromX]].headA.kindFlags & 0x800) == 0)
            {
                gObjActMoveFromX++;
                gObjActStep = 0x36;
            }
            break;

        case 0x38:
            result = 1;
            break;
    }

    return result;
}
// @ 0x080405A4
u8 sub_80405A4(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActStepTimer = 0;
            if (Rng_LcgNext() % 0x64 <= 0x45 && (sub_80187B4() & 0x220) == 0)
            {
                u16 f2a = obj->headA.kindFlags | 0x20;
                obj->headA.kindFlags = f2a;
                gObjActStep = 2;
            }
            else
            {
                gObjActStep = 9;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                obj->posX = sub_801768C(gObjActSavedX, -gObjActSavedX, 0x14, gObjActStepTimer, gObjActStep);
                gObjActStepTimer += 1;
            }
            else
            {
                sub_80207A4();
                obj->slot = 0xFF;
                obj->variantClass = 7;
                gObjActStep = 9;
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
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            sub_801CA08((BattleObj *)obj, 3, 0x1B4, 0xD, result);
            gObjActStep = 2;
            break;
        case 2:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)(&obj->headA)));
                obj->headA.kindFlags &= 0xEFFF;
                sub_80207DC((BattleObj *)obj, obj->posX, obj->posY, gObjActSavedF2A, gObjActSavedPal);
                gObjActStep = 9;
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
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActStepTimer = 0;
            if ((sub_80187B4() & 0x200) == 0)
            {
                u16 f2a = obj->headA.kindFlags | 0x20;
                obj->headA.kindFlags = f2a;
                gObjActStep = 2;
            }
            else
            {
                gObjActStep = 0x12;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                obj->posX = sub_801768C(gObjActSavedX, -gObjActSavedX, 0x14, gObjActStepTimer, gObjActStep);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStep = 9;
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
