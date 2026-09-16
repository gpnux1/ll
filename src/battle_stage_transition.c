#include "battle_types.h"
#include "battle_stage_transition.h"
#include "battle_task_services.h"
#include "sound.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"

// @ 0x08031580
u8 sub_8031580(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->slot = 0xFF;
            obj->headA.kindFlags &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031714
u8 sub_8031714(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x080318A8
u8 sub_80318A8(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031A3C
u8 sub_8031A3C(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031BD0
u8 sub_8031BD0(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031D64
u8 sub_8031D64(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031EF8
u8 sub_8031EF8(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x0803208C
u8 sub_803208C(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08032220
u8 sub_8032220(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x080323B4
u8 sub_80323B4(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
