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
// NPC 对话状态机 (gUnk_03000820: 0=开始 1=移动 2=等待 3=按键 5=选择 8=收尾 9=结束):
// case0 存 NPC 位置 (0x03000828/29) 并初始化; case1 播放对话开场动画 (0x368/0x359 按 arg0[0xBE]);
// case3 按键 0x21/0x7C/0x90 分发音效/推进; case5 处理选择确认; case9 无遮挡时结束对话返回 1。
// 注: case0 的 b4/zero 双零变量与 case1 的 keys/b4 写法是字节匹配必需的调度形状 (见 progress.md)。
u32 sub_8032548(u8 *arg0, u8 *arg1)
{
    u32 ret;
    u16 b4;
    int keys;
    u32 zero;
    u16 *b6ptr;
    u32 zero2;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        gUnk_03000828 = arg0[0xBF];
        gUnk_03000829 = arg0[0xC0];
        gUnk_03000825 = 0;
        sub_80444A4(arg0);
        sub_803F5B4(arg0);
        zero = 0;
        zero2 = 0;
        b6ptr = (u16 *)((u8 *)arg0 + 0xB6);
        b4 = zero2;
        *b6ptr = zero2;
        *(u16 *)((u8 *)arg0 + 0xB4) = b4;
        gUnk_03000820 = 1;
        gUnk_0300086B = zero;
        break;
    case 1:
        if (sub_803E58C(arg0, arg1, 0) == 1)
        {
            keys = arg0[0xBE];
            b4 = 0x368;
            if (keys == 0)
            {
                b4 = b4 - 0xF;
            }
            sub_8020974((ObjHead *)(arg0 + 0xC), b4, 0x1B4, 0xD, 2);
            if (arg1[0xBE] <= 0xA)
            {
                keys = *(u16 *)(arg0 + 0x24) | 0x20;
                *(u16 *)(arg0 + 0x24) = keys;
            }
            gUnk_03000820 = 2;
        }
        break;
    case 2:
        if (*(u16 *)(arg0 + 0x24) & 0x800)
        {
            break;
        }
        gUnk_03000820 = 3;
        break;
    case 3:
        if (*(u16 *)(arg0 + 0x28) == 0x21)
        {
            Sfx_Play(0x31, 1, 1);
            sub_8044514(0x5E);
            break;
        }
        if (*(u16 *)(arg0 + 0x28) == 0x7C)
        {
            Sfx_StopTrack(1);
            break;
        }
        if (*(u16 *)(arg0 + 0x28) != 0x90)
        {
            break;
        }
        sub_8044514(0x28);
        gUnk_03000820 = 5;
        break;
    case 5:
        if (*(u16 *)(arg0 + 0x24) & 0x1000)
        {
            sub_804C3A4(arg0[0x35], (u8)sub_801B954((ObjHead *)(arg0 + 0xC)));
            gUnk_0300086B = 0xC;
            gUnk_03000820 = 8;
        }
        break;
    case 8:
        if (sub_803E58C(arg0, arg1, 0) == 1)
        {
            gUnk_03000820 = 9;
        }
        break;
    case 9:
        if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && gUnk_03000856 == 0)
        {
            sub_8045B90(arg0, arg0[0xA1]);
            ret = 1;
        }
        break;
    }
    sub_803F658(arg0);
    return ret;
}
// @ 0x0803272C
// NPC 对话状态机变体 (战斗型对话, gUnk_03000820 同 8032548 十态):
// case0 存位+初始化; case1 sub_803ED34 到位检查+开场动画 (b4=0x36B/-0xF) + 写 0x35E 到 [0xB6];
// case2 等 0x800 后 Sfx+窗口设置 (sub_804BF14 9 参); case3 gUnk_03000825<=3 时 sub_804C728;
// case5 确认 (0x1000) → sub_804C3A4; case8/9 收尾同 8032548。尾 sub_803F658。
// 注: kind/flagval/b6val 独立载体变量与 case1 三次 def 拆分是字节匹配必需 (见 progress.md)。
u32 sub_803272C(u8 *arg0, u8 *arg1)
{
    u32 ret;
    u16 b4;
    u16 kind;
    int keys;
    u16 flags;
    u16 flagval;
    u16 zero2;
    u16 *b6ptr;
    u16 b6val;

    ret = 0;
    sub_80187E8();
    switch (gUnk_03000820)
    {
    case 0:
        gUnk_03000828 = arg0[0xBF];
        gUnk_03000829 = arg0[0xC0];
        gUnk_03000825 = 0;
        sub_80444A4(arg0);
        sub_803F5B4(arg0);
        gUnk_03000820 = 1;
        gUnk_0300086B = 0;
        break;
    case 1:
        if (sub_803ED34(arg0, arg1, 0) == 1)
        {
            kind = arg0[0xBE];
            b4 = 0x36B;
            if (kind == 0)
                b4 = b4 - 0xF;
            sub_8020974((ObjHead *)(arg0 + 0xC), b4, 0x1B4, 0xD, 2);
            b6ptr = (u16 *)((u8 *)arg0 + 0xB6);
            zero2 = 0;
            b6val = 0x35E;
            *b6ptr = b6val;
            *(u16 *)((u8 *)arg0 + 0xB4) = zero2;
            if (arg1[0xBE] <= 0xA)
            {
                keys = *(u16 *)(arg0 + 0x24);
                flagval = 0x20;
                keys = keys | flagval;
                *(u16 *)(arg0 + 0x24) = keys;
            }
            gUnk_03000820 = 2;
        }
        break;
    case 2:
        flags = *(u16 *)(arg0 + 0x24) & 0x800;
        if (flags != 0)
            break;
        Sfx_Play(0x4F, 1, 0);
        sub_8044514(0x28);
        sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
        gUnk_03000825 = 0;
        gUnk_03000820 = 3;
        break;
    case 3:
        if (gUnk_03000825 > 3)
            break;
        sub_804C728(0, 3, 0x10);
        gUnk_03000820 = 5;
        break;
    case 5:
        flags = *(u16 *)(arg0 + 0x24) & 0x1000;
        if (flags == 0)
            break;
        flags = arg0[0x35];
        sub_804C3A4(flags, (u8)sub_801B954((ObjHead *)(arg0 + 0xC)));
        gUnk_0300086B = 0xC;
        gUnk_03000820 = 8;
        break;
    case 8:
        if (sub_803E58C(arg0, arg1, 0) == 1)
            gUnk_03000820 = 9;
        break;
    case 9:
        if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && gUnk_03000856 == 0)
        {
            sub_8045B90(arg0, arg0[0xA1]);
            ret = 1;
        }
        break;
    }
    sub_803F658(arg0);
    return ret;
}
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
            sub_801CBA4((BattleObj *)obj, 0, gUnk_03000822, gUnk_03000824, 0);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x386, 0x1B4, 0xD, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4((BattleObj *)arg, zero, gUnk_03000822, gUnk_03000824, zero);
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
        sub_8020974((ObjHead *)(arg + 0xC), 0x386, 0x1B4, 0xD, 2);
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
        sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
        keys = *(u16 *)&arg[0x24] & 0xEFFF;
        zero = 0;
        *(u16 *)&arg[0x24] = keys;
        sub_801CBA4((BattleObj *)arg, zero, gUnk_03000822, gUnk_03000824, zero);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x386, 0x1B4, 0xD, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4((BattleObj *)arg, zero, gUnk_03000822, gUnk_03000824, zero);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x386, 0x1B4, 0xD, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4((BattleObj *)arg, zero, gUnk_03000822, gUnk_03000824, zero);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x386, 0x1B4, 0xD, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4((BattleObj *)arg, zero, gUnk_03000822, gUnk_03000824, zero);
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
            sub_804C3A4(arg[0x65], sub_801B954((ObjHead *)(arg + 0x3C)));
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
            sub_804C3A4(arg[0x65], sub_801B954((ObjHead *)(arg + 0x3C)));
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x386, 0x1B4, 0xD, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4((BattleObj *)arg, zero, gUnk_03000822, gUnk_03000824, zero);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x386, 0x1B4, 0xD, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            zero = 0;
            *(u16 *)&arg[0x24] = keys;
            sub_801CBA4((BattleObj *)arg, zero, gUnk_03000822, gUnk_03000824, zero);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x387, 0x1B4, 0xD, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974((ObjHead *)(arg + 0xC), 0x388, 0x1B4, 0xD, 2);
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
                sub_801CBA4((BattleObj *)arg, 0, gUnk_03000822, gUnk_03000824, v56);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x387, 0x1B4, 0xD, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974((ObjHead *)(arg + 0xC), 0x388, 0x1B4, 0xD, 2);
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
                sub_801CBA4((BattleObj *)arg, 0, gUnk_03000822, gUnk_03000824, v56);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x387, 0x1B4, 0xC, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974((ObjHead *)(arg + 0xC), 0x388, 0x1B4, 0xC, 2);
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
                sub_801CBA4((BattleObj *)arg, 0, gUnk_03000822, gUnk_03000824, v56);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x387, 0x1B4, 0xC, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974((ObjHead *)(arg + 0xC), 0x388, 0x1B4, 0xC, 2);
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
                sub_801CBA4((BattleObj *)arg, 0, gUnk_03000822, gUnk_03000824, v56);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x387, 0x1B4, 0xC, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974((ObjHead *)(arg + 0xC), 0x388, 0x1B4, 0xC, 2);
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
                sub_801CBA4((BattleObj *)arg, 0, gUnk_03000822, gUnk_03000824, v56);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x387, 0x1B4, 0xC, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974((ObjHead *)(arg + 0xC), 0x388, 0x1B4, 0xC, 2);
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
                sub_801CBA4((BattleObj *)arg, 0, gUnk_03000822, gUnk_03000824, v56);
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
            sub_8020974((ObjHead *)(arg + 0xC), 0x387, 0x1B4, 0xC, 2);
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
            sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
            keys = *(u16 *)&arg[0x24] & 0xEFFF;
            *(u16 *)&arg[0x24] = keys;
            sub_8020974((ObjHead *)(arg + 0xC), 0x388, 0x1B4, 0xC, 2);
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
                sub_801CBA4((BattleObj *)arg, 0, gUnk_03000822, gUnk_03000824, v56);
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
// NPC 剧情对话状态机变体 (单参数, gUnk_03000820 二十一态 0-0x14):
// case0 存 NPC 位置 (0824/0822) + 初始化 + 写 0x3A9 到 [0xB6]; case1 开场动画 0x3A5;
// case2 等 0x800 → Sfx 0xA5 → 5; case5 确认 0x1000 → sub_804C3A4 + 清 0x1000 + 动画 0x3A6
// (第 5 参 0x102) → 0x12; case18 等 0x800 + 清 0x100 → sub_8044514(0x32) → 0x13;
// case19 sub_80471AC()==0 → Sfx 0x64 → 0x14; case20 无遮挡 → sub_801CBA4 归位 (v56 载体) → 6;
// case6 等 0x800 → 9; case9 sub_8045B90 退场 + ret 1。case19/20 共享 0825++。
// 注: 载体变量 (b6ptr/zero2/b6val/v56) 与 sub_803ED34/u8 原型同 803272C 家族套路。
u32 sub_80368FC(u8 *arg0)
{
    u32 ret;
    u8 b4;
    int keys;
    u16 flags;
    u8 v56;
    u16 *b6ptr;
    u16 b6val;
    u16 zero2;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        gUnk_03000824 = arg0[0x35];
        gUnk_03000822 = *(u16 *)(arg0 + 0x2A);
        sub_80444A4(arg0);
        sub_803F5B4(arg0);
        gUnk_03000820 = 1;
        b6ptr = (u16 *)((u8 *)arg0 + 0xB6);
        zero2 = 0;
        b6val = 0x3A9;
        *b6ptr = b6val;
        *(u16 *)((u8 *)arg0 + 0xB4) = zero2;
        break;
    case 1:
        sub_8020974((ObjHead *)(arg0 + 0xC), 0x3A5, 0x1B4, 0xD, 2);
        gUnk_03000820 = 2;
        break;
    case 2:
        if (*(u16 *)(arg0 + 0x24) & 0x800)
            break;
        Sfx_Play(0xA5, 1, 0);
        gUnk_03000820 = 5;
        break;
    case 5:
        if (!(*(u16 *)(arg0 + 0x24) & 0x1000))
            break;
        b4 = arg0[0x35];
        keys = sub_801B954((ObjHead *)(arg0 + 0xC));
        sub_804C3A4(b4, (u8)keys);
        flags = *(u16 *)(arg0 + 0x24) & 0xEFFF;
        *(u16 *)(arg0 + 0x24) = flags;
        sub_8020974((ObjHead *)(arg0 + 0xC), 0x3A6, 0x1B4, 0xD, 0x102);
        gUnk_03000820 = 0x12;
        break;
    case 18:
        if (*(u16 *)(arg0 + 0x24) & 0x800)
            break;
        flags = *(u16 *)(arg0 + 0x24) & 0xFEFF;
        *(u16 *)(arg0 + 0x24) = flags;
        sub_8044514(0x32);
        gUnk_03000820 = 0x13;
        break;
    case 19:
        if (sub_80471AC() == 0)
        {
            Sfx_Play(0x64, 1, 0);
            gUnk_03000820 = 0x14;
        }
        gUnk_03000825 += 1;
        break;
    case 20:
        if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && (v56 = gUnk_03000856) == 0)
        {
            sub_801CBA4(arg0, 0, gUnk_03000822, gUnk_03000824, v56);
            gUnk_03000820 = 6;
        }
        gUnk_03000825 += 1;
        break;
    case 6:
        if (*(u16 *)(arg0 + 0x24) & 0x800)
            break;
        gUnk_03000820 = 9;
        break;
    case 9:
        sub_8045B90(arg0, arg0[0xA1]);
        ret = 1;
        break;
    }
    sub_803F658(arg0);
    return ret;
}
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
        sub_804C3A4(obj[0x35], sub_801B954((ObjHead *)(obj + 0xC)));
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
            sub_801CBA4((BattleObj *)obj, 0, gUnk_03000822, gUnk_03000824, 0);
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
        sub_804C3A4(obj[0x35], sub_801B954((ObjHead *)(obj + 0xC)));
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
            sub_801CBA4((BattleObj *)obj, 0, gUnk_03000822, gUnk_03000824, 0);
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
        sub_804C3A4(obj[0x35], sub_801B954((ObjHead *)(obj + 0xC)));
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
            sub_801CBA4((BattleObj *)obj, 0, gUnk_03000822, gUnk_03000824, 0);
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
        sub_804C3A4(obj[0x35], sub_801B954((ObjHead *)(obj + 0xC)));
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
            sub_801CBA4((BattleObj *)obj, 0, gUnk_03000822, gUnk_03000824, 0);
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
        sub_8020974((ObjHead *)(arg + 0xC), 0x3B9, 0x1B4, 0xD, 2);
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
        sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
        *(u16 *)&arg[0x24] &= 0xEFFF;
        sub_8020974((ObjHead *)(arg + 0xC), 0x3BA, 0x1B4, 0xD, 2);
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
            sub_801CBA4((BattleObj *)arg, 0, gUnk_03000822, gUnk_03000824, 0);
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
        sub_804C3A4(obj[0x35], sub_801B954((ObjHead *)(obj + 0xC)));
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
            sub_801CBA4((BattleObj *)obj, 0, gUnk_03000822, gUnk_03000824, 0);
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
// NPC 对话状态机变体 (gUnk_03000820 十态同 8032548; 开场动画固定 0x3C7):
// case0 存 NPC 位置 (0x03000828/29) 并清 [0xB4]/[0xB6]; case1 sub_803E58C 到位检查+
// 开场动画 0x3C7, arg1[0xBE]<=0xA 时打 0x20 标记; case2 等 0x800 后进 case3;
// case3 arg0[0x28]>0x39 (对话帧超时) 发音效并等 0x28 帧; case5 确认 (0x1000) →
// sub_804C3A4 + gUnk_0300086B=0xC; case8/9 收尾同 8032548 (无遮挡时结束返回 1)。
// 注: case1 的 spr 提载与 b4=0x3C7 (动画 id 走寄存器) 是字节匹配必需的调度形状;
// case0 的 zero/zero2/b6ptr/b4 拆分同 8032548 (见 progress.md)。
u32 sub_8038C84(u8 *arg0, u8 *arg1)
{
    u32 ret;
    u16 keys;
    u32 zero;
    u32 zero2;
    u16 *b6ptr;
    u8 *spr;
    u16 b4;

    ret = 0;
    switch (gUnk_03000820)
    {
    case 0:
        gUnk_03000828 = arg0[0xBF];
        gUnk_03000829 = arg0[0xC0];
        gUnk_03000825 = 0;
        sub_80444A4(arg0);
        sub_803F5B4(arg0);
        zero = 0;
        zero2 = 0;
        b6ptr = (u16 *)((u8 *)arg0 + 0xB6);
        b4 = zero2;
        *b6ptr = zero2;
        *(u16 *)((u8 *)arg0 + 0xB4) = b4;
        gUnk_03000820 = 1;
        gUnk_0300086B = zero;
        break;
    case 1:
        if (sub_803E58C(arg0, arg1, 0) == 1)
        {
            spr = arg0 + 0xC;
            b4 = 0x3C7;
            sub_8020974(spr, b4, 0x1B4, 0xD, 2);
            if (arg1[0xBE] <= 0xA)
            {
                keys = *(u16 *)(arg0 + 0x24) | 0x20;
                *(u16 *)(arg0 + 0x24) = keys;
            }
            gUnk_03000820 = 2;
        }
        break;
    case 2:
        if (*(u16 *)(arg0 + 0x24) & 0x800)
        {
            break;
        }
        gUnk_03000820 = 3;
        break;
    case 3:
        if (*(u16 *)(arg0 + 0x28) <= 0x39)
        {
            break;
        }
        Sfx_Play(0x31, 1, 0);
        sub_8044514(0x28);
        gUnk_03000820 = 5;
        break;
    case 5:
        if (*(u16 *)(arg0 + 0x24) & 0x1000)
        {
            sub_804C3A4(arg0[0x35], (u8)sub_801B954((ObjHead *)(arg0 + 0xC)));
            gUnk_0300086B = 0xC;
            gUnk_03000820 = 8;
        }
        break;
    case 8:
        if (sub_803E58C(arg0, arg1, 0) == 1)
        {
            gUnk_03000820 = 9;
        }
        break;
    case 9:
        if (gUnk_03000844 == 0 && gUnk_03000845 == 0 && gUnk_03000856 == 0)
        {
            sub_8045B90(arg0, arg0[0xA1]);
            ret = 1;
        }
        break;
    }
    sub_803F658(arg0);
    return ret;
}
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
            sub_801CBA4((BattleObj *)obj, 0, gUnk_03000822, gUnk_03000824, 0);
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
