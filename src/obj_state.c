#include "code_0.h"
#include "data_805769C.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"
#include "sound.h"

// @ 0x08044394
void sub_8044394(u8 *obj)
{
    u8 active;
    u16 value;

    active = 0;
    value = 0;
    if (obj[0xBE] > 11 && gUnk_03000884 == 0)
    {
        switch (obj[0xBE])
        {
            case 0x2D:
                if (*(u16 *)(obj + 0x28) != 0)
                {
                    value = 0x69;
                    active = 1;
                }
                break;
            case 0x61:
                if (*(u16 *)(obj + 0x28) > 0x31)
                {
                    value = 0x4E;
                    active = 1;
                }
                break;
            case 0x68:
                if (*(u16 *)(obj + 0x28) > 0x31)
                {
                    value = 0x4E;
                    active = 1;
                }
                break;
            case 0x36:
            case 0x6A:
                if (*(u16 *)(obj + 0x28) > 0x22)
                {
                    value = 0x4E;
                    active = 1;
                }
                break;
        }
        if (active == 1)
        {
            Sfx_Play(value, 2, 0);
            gUnk_03000884 = active;
        }
    }
}
// @ 0x08044414
void sub_8044414(void)
{
    gUnk_03000889 = 0;
}
// @ 0x08044420
u16 sub_8044420(void)
{
    return gUnk_03000882;
}
// @ 0x0804442C
void sub_804442C(u8 arg0)
{
    u8 i;

    gUnk_03000820 = arg0;
    gUnk_03000826 = 0;
    gUnk_03000844 = 0;
    gUnk_03000845 = 0;
    gUnk_03000856 = 0;
    gUnk_03000825 = 0;
    gUnk_0300086A = 1;
    gUnk_03000884 = 0;
    for (i = 0; i <= 0xB; i++)
    {
        gUnk_03004F90[i] = 0;
    }
}
// @ 0x0804448C
void sub_804448C(void)
{
    gUnk_03000865 = 0;
}
// @ 0x08044498
u8 sub_8044498(void)
{
    return gUnk_03000865;
}
// @ 0x080444A4
void sub_80444A4(u8 *arg0)
{
    u8 ids[12];
    u8 i;
    u32 base = GetObjPool();
    u8 count = sub_80462E4(arg0, ids, 0x6F);

    for (i = 0; i < count; i++)
    {
        *(u16 *)(base + ids[i] * 0xC8 + 0xB2) = 0;
    }
}
// @ 0x080444E8
u8 sub_80444E8(void)
{
    if (gUnk_03000844 != 0)
    {
        return 0;
    }
    if (gUnk_03000845 != 0)
    {
        return 0;
    }
    if (gUnk_03000856 != 0)
    {
        return 0;
    }
    return 1;
}
// @ 0x08044514
void sub_8044514(s16 arg0)
{
    gUnk_03000844 = 1;
    gUnk_03000845 = 0;
    gUnk_03000856 = 0;
    if (arg0 < 0)
    {
        gUnk_0300085A = 0xC;
    }
    else
    {
        gUnk_0300085A = arg0;
    }
    gUnk_03000857 = 0;
    gUnk_0300085C = 0;
    gUnk_03000886 = 0x37;
    gUnk_03000888 = 0;
}
// @ 0x08044574
void sub_8044574(s16 arg0, u16 arg1, u8 arg2)
{
    gUnk_03000844 = 1;
    gUnk_03000845 = 0;
    gUnk_03000856 = 0;
    if (arg0 < 0)
    {
        gUnk_0300085A = 0xC;
    }
    else
    {
        gUnk_0300085A = arg0;
    }
    gUnk_03000857 = 0;
    gUnk_0300085C = 0;
    gUnk_03000886 = arg1;
    gUnk_03000888 = arg2;
}
// @ 0x080445E0
u8 *sub_80445E0()
{
    return gUnk_03004F90;
}

// @ 0x080445E8
// 按 gUnk_0300083D 遍历 gUnk_03000840[i]&0xF 索引 GetObjPool 槽,
// obj[0xAC]==arg0[0xAC] 命中时写 gUnk_03004F90[idx]=arg1 (arg1==6 需 sub_8048C30(arg0)==1)。
// 注: 索引表达式须 `off + (u32)base`(off 在前) 才出目标 `adds r0,r0,r3`; 反序出 `adds r0,r3,r0` (差1B)。
//     0xF 直接内联不可抽 mask 变量(抽了多出 sub sp 槽与调度差, 曾差 112B)。
void sub_80445E8(u8 *arg0, u8 arg1)
{
    u8 i;
    u8 *base;
    u32 off;
    u8 *obj;

    base = (u8 *)GetObjPool();
    for (i = 0; i < gUnk_0300083D; i++)
    {
        off = (0xF & gUnk_03000840[i]) * 0xC8;
        obj = (u8 *)(off + (u32)base);
        if (obj[0xAC] == arg0[0xAC])
        {
            if (arg1 != 6 || sub_8048C30(arg0) == 1)
                gUnk_03004F90[0xF & gUnk_03000840[i]] = arg1;
            break;
        }
    }
}
// @ 0x08044680
u8 sub_8044680(u8 *arg0)
{
    if (arg0[0xBE] <= 10)
    {
        return sub_803FF54(arg0);
    }

    return sub_80405A4(arg0);
}
// @ 0x080446A4
u8 sub_80446A4(u8 *arg0)
{
    if (arg0[0xBE] <= 10)
    {
        return gUnk_03000826;
    }

    return 0;
}
extern u16 gUnk_0839DBF6[][4];

// @ 0x080446BC
void sub_80446BC(u8 *arg0)
{
    u8 r5;
    u8 r1;
    u16 threshold;

    r5 = 0;
    r1 = 1;
    if (arg0[0xBE] <= 11)
    {
        return;
    }
    if (gUnk_03000884 != 0)
    {
        return;
    }
    switch ((s8)arg0[0xBC])
    {
        case 0:
            break;
        case 1:
            r5 = 2;
            r1 = 3;
            break;
    }
    threshold = gUnk_0839DBF6[arg0[0xBE] - 0xc][r1];
    if (*(u16 *)(arg0 + 0x28) < threshold)
    {
        return;
    }
    Sfx_Play(gUnk_0839DBF6[arg0[0xBE] - 0xc][r5], 2, 0);
    gUnk_03000884 = 1;
}
// @ 0x08044728
s32 sub_8044728()
{
    return 2;
}
// @ 0x0804472C
s32 sub_804472C()
{
    return 0;
}
// @ 0x08044730
s32 sub_8044730()
{
    return 0;
}
// @ 0x08044734
s32 sub_8044734()
{
    return 0;
}
// @ 0x08044738
s32 sub_8044738()
{
    return 0;
}
