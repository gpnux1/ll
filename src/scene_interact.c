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


// @ 0x08031580
u8 sub_8031580(u8 *obj)
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
// @ 0x08031714
u8 sub_8031714(u8 *obj)
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
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x080318A8
u8 sub_80318A8(u8 *obj)
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
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031A3C
u8 sub_8031A3C(u8 *obj)
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
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031BD0
u8 sub_8031BD0(u8 *obj)
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
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031D64
u8 sub_8031D64(u8 *obj)
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
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031EF8
u8 sub_8031EF8(u8 *obj)
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
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x0803208C
u8 sub_803208C(u8 *obj)
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
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08032220
u8 sub_8032220(u8 *obj)
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
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x080323B4
u8 sub_80323B4(u8 *obj)
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
            *(u16 *)(obj + 0x24) &= 0xFFEF;
            obj[0xBE] = 0xFF;
            result = 1;
            break;
    }
    return result;
}
