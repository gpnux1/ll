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

// @ 0x0804AD54
void sub_804AD54(u16 *ptr)
{
    *(ptr + 0x5B) = 0xB000;
}
extern u8 gUnk_08619A60[];
extern u8 gUnk_08619430[];

// @ 0x0804AD60
void sub_804AD60(void)
{
    ObjHead *obj = (ObjHead *)gUnk_03000918;
    u8 zero;
    u16 flags;

    sub_801B81C((u8 *)obj, 0xF0, 0x50, 0xDA * 2, 0xE, gUnk_08619A60, gUnk_08619430, 0xA8 * 8, 1, 0x402);
    ObjGfxLoad_Step(obj);
    flags = 0xF7FF & obj->kindFlags;
    zero = 0;
    obj->kindFlags = flags;
    sub_801A684(obj);
    gUnk_03000911 = zero;
    gUnk_03000910 = zero;
    gUnk_03000948 = zero;
    obj->f_2A = zero;
    Bgm_Stop();
}
// @ 0x0804ADE0
void sub_804ADE0(void)
{
    gUnk_03000910 = 0;
    gUnk_03000948 = 1;
}
// @ 0x0804ADF8
void sub_804ADF8(void)
{
    ScreenFade_Start(2, 10, 0x32);
    gUnk_03000910 = 0;
    gUnk_03000911 = 0;
    gUnk_03000948 = 2;
    gUnk_0300097E = 0;
}
// @ 0x0804AE2C
// 注: OAM 缓冲用强转常量 (非 gOamBuffer 符号) 才能让 agbcc 逐迭代重物化基址 (RULES 规则102)
#define OAM_BUF ((GameOamData *)0x030035C0)
extern u8 gUnk_08393A24[];
void sub_804AE2C(void)
{
    u16 i;

    if ((gUnk_03000ADE & 1) != 0 && (gUnk_03000ADE & 0xF0) == 0x10)
    {
        gUnk_03000AD9 = *(u8 *)(gUnk_030009D0 + 0x2D);
        gUnk_03000ADA = *(u8 *)(gUnk_030009D0 + 0x2D) - *(u8 *)(gUnk_030009D0 + 0x2E);
        gUnk_03000ADB = 0xA0;
        gUnk_03000ADC = 0;
        i = gUnk_03000AD9;
        if (i > gUnk_03000ADA)
        {
            do
            {
                if (gUnk_03000ADB > OAM_BUF[i].fields.VPos)
                    gUnk_03000ADB = OAM_BUF[i].fields.VPos;
                if (gUnk_03000ADC < gUnk_08393A24[OAM_BUF[i].fields.Size + (OAM_BUF[i].fields.Shape << 2)] * 8
                        + OAM_BUF[i].fields.VPos)
                    gUnk_03000ADC = (u8)(gUnk_08393A24[OAM_BUF[i].fields.Size + (OAM_BUF[i].fields.Shape << 2)] * 8
                        + OAM_BUF[i].fields.VPos);
                gUnk_030009D8[i] = OAM_BUF[i].fields.HPos;
                i--;
            } while (i > gUnk_03000ADA);
        }
        gUnk_03000ADE |= 2;
        gUnk_03000AD8 = (u8)((gUnk_03000AD8 + 1) % 5);
        if (gUnk_03000AD8 == 0)
            gUnk_03000ADD++;
        if ((gUnk_03000ADC - gUnk_03000ADD) < (gUnk_03000ADB - 0x1E))
        {
            gUnk_03000ADE &= ~1;
            gUnk_03000ADE &= ~2;
        }
    }
}
#undef OAM_BUF
// @ 0x0804AF60
INCLUDE_ASM("asm/nonmatchings", sub_804AF60);
// @ 0x0804B080
INCLUDE_ASM("asm/nonmatchings", sub_804B080);
// @ 0x0804B1EC
void sub_804B1EC(void)
{
    gUnk_03000ADE = 0;
}
// @ 0x0804B1F8
void sub_804B1F8(u32 arg0)
{
    gUnk_030009D0 = arg0;
    gUnk_03000ADE |= 0x11;
    gUnk_03000AD8 = 0;
    gUnk_03000ADD = 0;
}
extern u32 gUnk_0861AAA4[];
extern u32 gUnk_0861C764[];

// @ 0x0804B224
void sub_804B224(u16 *flags)
{
    if ((*flags & 0x80) != 0)
    {
        DmaCopy32(3, gUnk_0861AAA4, (void *)0x06012E80, 0x280);
        DmaWait(3);
        sub_804C2FC((u32)gUnk_0861C764, 0xF, 1);
        *flags &= 0xFF7F;
    }
}
/* 0x03000AE8/0x03000BE8 表的 16 字节项视图 (sub_804B288 专用; iwram.h 保持 u8[] 不动)。
 * 必须用结构体成员形式 (见下方 sub_804C4D8 的同名注释 / 规则 11 / 67)。 */
typedef struct
{
    u8 f0;
    u8 f1;
    u8 f2;
    u8 f3;
    u8 f4;
    u8 pad5;
    u16 f6;
    u8 f8;
    u8 pad9[7];
} Unk_804B288Entry;

// @ 0x080
// 战斗动画子系统复位: 清 0x03000AE0/03000AE2/03000CE8 (u16) 与 03000AE4/03000AE5 (u8) 状态字,
// 4 次 DMA fill (共享 sp 上的 u16 fill=0, 控制字 0x81000100=使能+源固定+256 半字) 清
// OBJ/BG 调色板与 0x02036AC0/0x02036CC0, 每次后 DmaWait; 最后把两张 16×16B 表的
// field_0/1 |= 0xFF、field_2-4/8 清零、field_6 (u16) 清零。
// new_var 死赋值 = 锚定 gUnk_03000AE8 池常量的装载位置 (缺了它 GCC2 会把该 ldr 提升到首个 DmaWait 之前)。
void sub_804B288(void)
{
    u8 i;
    vu16 fill;
    u8 *new_var;

    gUnk_03000AE0 = 0;
    gUnk_03000AE2 = 0;
    gUnk_03000CE8 = 0;
    gUnk_03000AE4 = 0;
    gUnk_03000AE5 = 0;
    fill = 0;
    DmaSet(3, &fill, (void *)0x05000200, 0x81000100);
    DmaWait(3);
    fill = 0;
    DmaSet(3, &fill, (void *)0x05000000, 0x81000100);
    DmaWait(3);
    fill = 0;
    DmaSet(3, &fill, (void *)0x02036AC0, 0x81000100);
    DmaWait(3);
    fill = 0;
    DmaSet(3, &fill, (void *)0x02036CC0, 0x81000100);
    DmaWait(3);
    for (i = 0; i <= 15; i++)
    {
        new_var = gUnk_03000AE8;
        ((Unk_804B288Entry *)(new_var + (i * 16)))->f0 |= 0xFF;
        ((Unk_804B288Entry *)(gUnk_03000AE8 + (i * 16)))->f1 |= 0xFF;
        ((Unk_804B288Entry *)(gUnk_03000AE8 + (i * 16)))->f2 = 0;
        ((Unk_804B288Entry *)(gUnk_03000AE8 + (i * 16)))->f3 = 0;
        ((Unk_804B288Entry *)(gUnk_03000AE8 + (i * 16)))->f4 = 0;
        ((Unk_804B288Entry *)(gUnk_03000AE8 + (i * 16)))->f6 = 0;
        ((Unk_804B288Entry *)(gUnk_03000AE8 + (i * 16)))->f8 = 0;
        ((Unk_804B288Entry *)(gUnk_03000BE8 + (i * 16)))->f0 |= 0xFF;
        ((Unk_804B288Entry *)(gUnk_03000BE8 + (i * 16)))->f1 |= 0xFF;
        ((Unk_804B288Entry *)(gUnk_03000BE8 + (i * 16)))->f2 = 0;
        ((Unk_804B288Entry *)(gUnk_03000BE8 + (i * 16)))->f3 = 0;
        ((Unk_804B288Entry *)(gUnk_03000BE8 + (i * 16)))->f4 = 0;
        ((Unk_804B288Entry *)(gUnk_03000BE8 + (i * 16)))->f6 = 0;
        ((Unk_804B288Entry *)(gUnk_03000BE8 + (i * 16)))->f8 = 0;
    }
}
// @ 0x0804B3C0
INCLUDE_ASM("asm/nonmatchings", sub_804B3C0);
// @ 0x0804B458
void sub_804B458(Unk_804B458 *entry, u8 slot, u16 *src, u16 *dest)
{
    u8 width;
    u8 frames;

    entry->field_3 = (entry->field_3 + 1) % entry->field_2;
    if (entry->field_3 == 0)
    {
        width = entry->field_4 & 0xF;
        frames = entry->field_4 >> 4;
        if (entry->field_8 == 0)
            entry->field_6 = (u8)((*(u8 *)&entry->field_6 + 1) % frames);
        else if (entry->field_6 == 0)
            entry->field_6 = frames - 1;
        else
            entry->field_6--;
        sub_804C2A0(src + (entry->field_1 << 4), dest + (slot << 4), width, frames, entry->field_6);
    }
}
// @ 0x0804B4D0
INCLUDE_ASM("asm/nonmatchings", sub_804B4D0);
// @ 0x0804B56C
INCLUDE_ASM("asm/nonmatchings", sub_804B56C);
// @ 0x0804B654
INCLUDE_ASM("asm/nonmatchings", sub_804B654);
// @ 0x0804B7B0
INCLUDE_ASM("asm/nonmatchings", sub_804B7B0);
// @ 0x0804B834
INCLUDE_ASM("asm/nonmatchings", sub_804B834);
void sub_804B8E8(u8 arg0, u8 arg1)
{
    u8 i;
    int empty = -1;
    u8 *entry;

    for (i = 0; i < arg1; i++)
    {
        u8 *base = gUnk_03000AE8;
        u8 mask = 0xFF;
        entry = base + (arg0 + i) * 16;
        {
            u8 temp;
            u8 flags = entry[0];
            u32 v = *(s8 *)&entry[0];
            if (v == empty)
                continue;
            v = 0x20;
            v &= flags;
            if (v == 0)
                sub_804C3A4(entry[1], 1);
            sub_804C420(arg0 + i);
            temp = entry[0];
            temp |= mask;
            entry[0] = temp;
            temp = entry[1];
            temp |= mask;
            entry[1] = temp;
            entry[2] = 0;
            entry[3] = 0;
        }
    }
}
// @ 0x0804B96C
INCLUDE_ASM("asm/nonmatchings", sub_804B96C);
// @ 0x0804BB64
void sub_804BB64(u8 start, u8 count)
{
    u8 i;
    u32 index;
    u8 *base;
    u8 *entry;
    u8 mask;
    u8 value;

    i = 0;
    if (i < count)
    {
        mask = 0xFF;
        do
        {
            base = gUnk_03000AE8;
            index = start + i;
            entry = base + index * 16;
            if ((entry[0] & 0xF) == 3)
            {
                if ((entry[0] & 0x20) == 0)
                    sub_804C3A4(entry[1], 1);
                sub_804C420((u8)index);
                value = entry[0];
                value |= mask;
                entry[0] = value;
                value = entry[1];
                value |= mask;
                entry[1] = value;
                entry[2] = 0;
                entry[3] = 0;
            }
            i++;
        } while (i < count);
    }
}
// @ 0x0804BBDC
INCLUDE_ASM("asm/nonmatchings", sub_804BBDC);
// @ 0x0804BD54
void sub_804BD54(u8 arg0, u8 arg1)
{
    u8 i;
    int empty = -1;
    u8 *entry;

    for (i = 0; i < arg1; i++)
    {
        u8 *base = gUnk_03000BE8;
        u8 mask = 0xFF;
        entry = base + (arg0 + i) * 16;
        {
            u8 temp;
            u8 flags = entry[0];
            u32 v = *(s8 *)&entry[0];
            if (v == empty)
                continue;
            v = 0x20;
            v &= flags;
            if (v == 0)
                sub_804C5F8(entry[1], 1);
            sub_804C674(arg0 + i);
            temp = entry[0];
            temp |= mask;
            entry[0] = temp;
            temp = entry[1];
            temp |= mask;
            entry[1] = temp;
            entry[2] = 0;
            entry[3] = 0;
        }
    }
}
// @ 0x0804BDD8
INCLUDE_ASM("asm/nonmatchings", sub_804BDD8);
// @ 0x0804BE90
void sub_804BE90(u8 arg0, u8 arg1)
{
    u8 i;
    int empty = -1;
    u8 *entry;

    for (i = 0; i < arg1; i++)
    {
        u8 *base = gUnk_03000BE8;
        u8 mask = 0xFF;
        entry = base + (arg0 + i) * 16;
        {
            u8 temp;
            u8 flags = entry[0];
            u32 v = *(s8 *)&entry[0];
            if (v == empty)
                continue;
            v = 0x20;
            v &= flags;
            if (v == 0)
                sub_804C5F8(entry[1], 1);
            sub_804C674(arg0 + i);
            temp = entry[0];
            temp |= mask;
            entry[0] = temp;
            temp = entry[1];
            temp |= mask;
            entry[1] = temp;
            entry[2] = 0;
            entry[3] = 0;
        }
    }
}
// @ 0x0804BF14
INCLUDE_ASM("asm/nonmatchings", sub_804BF14);
// @ 0x0804C10C
void sub_804C10C(u8 start, u8 count)
{
    u8 i;
    u32 index;
    u8 *base;
    u8 *entry;
    u8 mask;
    u8 value;

    i = 0;
    if (i < count)
    {
        mask = 0xFF;
        do
        {
            base = gUnk_03000BE8;
            index = start + i;
            entry = base + index * 16;
            if ((entry[0] & 0xF) == 3)
            {
                if ((entry[0] & 0x20) == 0)
                    sub_804C5F8(entry[1], 1);
                sub_804C674((u8)index);
                value = entry[0];
                value |= mask;
                entry[0] = value;
                value = entry[1];
                value |= mask;
                entry[1] = value;
                entry[2] = 0;
                entry[3] = 0;
            }
            i++;
        } while (i < count);
    }
}
// @ 0x0804C184
void sub_804C184(void)
{
    sub_804C45C();
    sub_804C6B0();
}

// @ 0x0804C194
void *sub_804C194(u8 arg0)
{

    switch (arg0)
    {
        case 0:
            return (void *)0x03000AE8;
        case 1:
            return (void *)0x03000BE8;
    }
    // No return?
}

// @ 0x0804C1B4
void sub_804C1B4(u8 arg0, u8 arg1, u8 arg2)
{
    switch (arg0)
    {
        case 0:
            sub_804C364(arg1, arg2);
            break;
        case 1:
            sub_804C5B8(arg1, arg2);
            break;
    }
}

// @ 0x0804C1E4
void sub_804C1E4(u8 arg0, u8 arg1, u8 arg2)
{
    switch (arg0)
    {
        case 0:
            sub_804C3A4(arg1, arg2);
            break;
        case 1:
            sub_804C5F8(arg1, arg2);
            break;
    }
}
// @ 0x0804C214
u8 sub_804C214(u8 arg0, u8 arg1)
{

    u8 ret = 0;

    switch (arg0)
    {
        case 0:
            if ((gUnk_03000AE0 >> arg1) & 1)
            {
                ret = 1;
            }
            break;
        case 1:
            if ((gUnk_03000AE2 >> arg1) & 1)
            {
                ret = 1;
            }
            break;
    }

    return ret;
}
// @ 0x0804C250
void sub_804C250(u8 arg0, u8 arg1)
{
    switch (arg0)
    {
        case 0:
            sub_804C3E4(arg1);
            break;
        case 1:
            sub_804C638(arg1);
            break;
    }
}
// @ 0x0804C278
void sub_804C278(u8 arg0, u8 arg1)
{
    switch (arg0)
    {
        case 0:
            sub_804C420(arg1);
            break;
        case 1:
            sub_804C674(arg1);
            break;
    }
}

// @ 0x0804C2A0
void sub_804C2A0(u16 *arg0, u16 *arg1, u8 arg2, u8 arg3, u8 arg4)
{
    u8 i;

    for (i = 0; i < arg3; i++)
    {
        arg0[i + arg2] = arg1[arg4 + arg2];

        arg4 = (arg4 + 1) % arg3;
    }
}
// @ 0x0804C2F0
u16 sub_804C2F0(void)
{
    return gUnk_03000AE0;
}
// @ 0x0804C2FC
void sub_804C2FC(u32 arg0, u8 arg1, u8 arg2)
{
    u8 i;

    DmaCopy16(3, arg0, 0x5000200 + (arg1 << 5), arg2 * 0x20);
    DmaWait(3);

    for (i = 0; i < arg2; i++)
    {
        if (!((gUnk_03000AE0 >> (arg1 + i)) & 1))
        {
            gUnk_03000AE0 |= (1 << (arg1 + i));
        }
    }
}
// @ 0x0804C364
void sub_804C364(u8 arg0, u8 arg1)
{
    u8 i;

    for (i = 0; i < arg1; i++)
    {
        if (!((gUnk_03000AE0 >> (arg0 + i)) & 1))
        {
            gUnk_03000AE0 |= (1 << (arg0 + i));
        }
    }
}
// @ 0x0804C3A4
void sub_804C3A4(u8 arg0, u8 arg1)
{
    u8 i;
    for (i = 0; i < arg1; i++)
    {
        if (((gUnk_03000AE0 >> (arg0 + i)) & 1))
        {
            gUnk_03000AE0 &= ~(1 << (arg0 + i));
        }
    }
}

// @ 0x0804C3E4
void sub_804C3E4(u8 arg0)
{
    DmaCopy16(3, 0x05000200 + (arg0 << 5), 0x02036AC0 + (arg0 << 5), 0x20);
    DmaWait(3);
}

// @ 0x0804C420
void sub_804C420(u8 arg0)
{
    DmaCopy32(3, 0x02036AC0 + (arg0 << 5), 0x05000200 + (arg0 << 5), 32);
    DmaWait(3);
}

// @ 0x0804C45C
void sub_804C45C(void)
{
    u8 i;
    u8 *entry;

    for (i = 0; i <= 15; i++)
    {
        entry = gUnk_03000AE8 + i * 16;
        switch (entry[0] & 0xF)
        {
            case 1:
                sub_804B3C0(entry, i, 0x05000200, 0x02036AC0);
                break;
            case 2:
                sub_804B458((Unk_804B458 *)entry, i, (u16 *)0x05000200, (u16 *)0x02036AC0);
                break;
            case 3:
                sub_804B4D0(entry, i, 0x05000200, 0x02036AC0);
                break;
        }
    }
}
/* 0x03000AE8 表的 16 字节项视图 (iwram.h 里只有 u8[] 声明, 不动它)。
 * 必须用结构体成员形式: 写成 `u8 *ptr; ptr[0] |= 0x40;` 时 GCC2 会把 IOR 的
 * 目的寄存器选成常量那个 (`mov r0, ip; orrs r0, r1`), 而目标是
 * `adds r0, r1, #0; orrs r0, r7` (先拷 b 再或常量)。见规则 11 / 67。 */
typedef struct
{
    u8 field_0;
    u8 field_1;
    u8 field_2;
    u8 field_3;
    u8 pad[12];
} Unk_03000AE8;

// @ 0x0804C4D8
void sub_804C4D8(u8 arg0, u8 arg1, u8 arg2)
{
    u8 i;
    Unk_03000AE8 *entry;

    for (i = 0; i < arg1; i++)
    {
        entry = (Unk_03000AE8 *)&gUnk_03000AE8[(arg0 + i) * 16];
        if ((entry->field_0 & 0xF) == 3)
        {
            entry->field_0 |= 0x40;
            entry->field_2 = arg2;
            entry->field_3 = 0;
        }
    }
}

// @ 0x0804C53C
u16 sub_804C53C(void)
{
    return gUnk_03000AE2;
}
// @ 0x0804C548
void sub_804C548(u32 src, u8 slot, u8 count)
{
    u8 i;

    DmaCopy32(3, src, 0x05000000 + (slot << 5), count * 0x20);
    DmaWait(3);
    for (i = 0; i < count; i++)
    {
        if (((gUnk_03000AE2 >> (slot + i)) & 1) == 0)
            gUnk_03000AE2 |= 1 << (slot + i);
    }
}
// @ 0x0804C5B8
void sub_804C5B8(u8 arg0, u8 arg1)
{
    u8 i;

    for (i = 0; i < arg1; i++)
    {
        if (!((gUnk_03000AE2 >> (arg0 + i)) & 1))
        {
            gUnk_03000AE2 |= (1 << (arg0 + i));
        }
    }
}

// @ 0x0804C5F8
void sub_804C5F8(u8 arg0, u8 arg1)
{
    u8 i;

    for (i = 0; i < arg1; i++)
    {
        if (((gUnk_03000AE2 >> (arg0 + i)) & 1))
        {
            gUnk_03000AE2 &= ~(1 << (arg0 + i));
        }
    }
}
// @ 0x0804C638
void sub_804C638(u8 arg0)
{
    DmaCopy32(3, 0x05000000 + (arg0 << 5), 0x02036CC0 + (arg0 << 5), 32);
    DmaWait(3);
}
// @ 0x0804C674
void sub_804C674(u8 arg0)
{
    DmaCopy32(3, 0x02036CC0 + (arg0 << 5), 0x05000000 + (arg0 << 5), 32);
    DmaWait(3);
}
// @ 0x0804C6B0
void sub_804C6B0(void)
{
    u8 i;
    u8 *entry;

    for (i = 0; i <= 15; i++)
    {
        entry = gUnk_03000BE8 + i * 16;
        switch (entry[0] & 0xF)
        {
            case 1:
                sub_804B3C0(entry, i, 0x05000000, 0x02036CC0);
                break;
            case 2:
                sub_804B458((Unk_804B458 *)entry, i, (u16 *)0x05000000, (u16 *)0x02036CC0);
                break;
            case 3:
                sub_804B4D0(entry, i, 0x05000000, 0x02036CC0);
                break;
        }
    }
}
// @ 0x0804C728
void sub_804C728(u8 arg0, u8 arg1, u8 arg2)
{
    u8 i;
    Unk_03000AE8 *entry;

    for (i = 0; i < arg1; i++)
    {
        entry = (Unk_03000AE8 *)&gUnk_03000BE8[(arg0 + i) * 16];
        if ((entry->field_0 & 0xF) == 3)
        {
            entry->field_0 |= 0x40;
            entry->field_2 = arg2;
            entry->field_3 = 0;
        }
    }
}
// @ 0x0804C78C
void sub_804C78C(void)
{
    u8 values[16];
    u8 count;
    u8 i;
    u8 *obj;
    u8 *pool;

    sub_804DE8C();
    pool = GetObjPool();
    count = sub_80489E8(pool, values, 0, 0x43);
    for (i = 0; i < count; i++)
    {
        obj = pool + values[i] * 0xC8;
        if (sub_8045F10(obj, 0x20) == 1)
        {
            switch (obj[0xBE])
            {
                case 0:
                case 1:
                    ((void (*)(u8 *, u8))sub_804CA2C)(obj, values[i]);
                    break;
                case 2:
                    ((void (*)(u8 *, u8))sub_804CAA0)(obj, values[i]);
                    break;
                case 3:
                    ((void (*)(u8 *, u8))sub_804CB18)(obj, values[i]);
                    break;
                case 4:
                    ((void (*)(u8 *, u8))sub_804CB8C)(obj, values[i]);
                    break;
                case 5:
                    ((void (*)(u8 *, u8))sub_804CC00)(obj, values[i]);
                    break;
                case 6:
                    ((void (*)(u8 *, u8))sub_804CC78)(obj, values[i]);
                    break;
                case 7:
                    ((void (*)(u8 *, u8))sub_804CCEC)(obj, values[i]);
                    break;
                case 8:
                    ((void (*)(u8 *, u8))sub_804CD60)(obj, values[i]);
                    break;
                case 9:
                    ((void (*)(u8 *, u8))sub_804CDD4)(obj, values[i]);
                    break;
                case 10:
                    ((void (*)(u8 *, u8))sub_804CE48)(obj, values[i]);
                    break;
            }
        }
    }
    sub_804EF50();
}
// @ 0x0804C890
void sub_804C890(u8 *obj)
{
    u8 i;

    sub_804DE8C();
    for (i = 0; i <= 4; i++)
    {
        u8 *o = obj + i * 0xC8;
        if (sub_8045F10(o, 0x20) == 2)
        {
            u8 r;
            u8 t = i;
            u8 *p;

            Rng_LcgNext();
            r = sub_804C8E0(obj, t);
            p = o + 0xBD;
            t = 0;
            *p = r;
            o[0xBC] = t;
        }
    }
    sub_804EF50();
}
// @ 0x0804C8E0
u8 sub_804C8E0(u8 *obj, u8 arg1)
{
    u8 values[8];
    u8 count;
    u8 i;
    u8 j;
    u8 slot;

    slot = 0;
    count = sub_80489E8(obj, values, 0, 0x7F);
    for (i = 0; i < count; i++)
    {
        if (values[i] == arg1)
        {
            for (j = i; j < count - 1; j++)
                values[j] = values[j + 1];
            count--;
            break;
        }
    }
    if (count == 0)
    {
        if (slot == 0)
            slot = 1;
        else
            slot = 0;
        count = sub_80489E8(obj, values, slot, 0x6F);
        for (i = 0; i < count; i++)
        {
            if (values[i] == arg1)
            {
                for (j = i; j < count - 1; j++)
                    values[j] = values[j + 1];
                count--;
                break;
            }
        }
    }
    return values[((s32 (*)(void))Rng_LcgNext)() % count];
}
// @ 0x0804C9B4
void sub_804C9B4(void)
{
    u8 values[8];
    u8 count;
    u8 i;
    u8 value;
    u8 *pool;
    u8 *obj;

    pool = GetObjPool();
    count = sub_80489E8(pool, values, 0, 0x7F);
    for (i = 0; i < count; i++)
    {
        obj = pool + values[i] * 0xC8;
        if (obj[0xBE] == 9)
        {
            obj[0xBC] = 0;
            pool = GetObjPool();
            count = sub_80489E8(pool, values, 1, 0x7F);
            value = values[((s32 (*)(void))Rng_LcgNext)() % count];
            obj[0xBD] = value;
            break;
        }
    }
}
// @ 0x0804CA2C
void sub_804CA2C(u8 *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8((u8 *)GetObjPool(), values, 1, 0x7F);
    obj[0xBC] = 0;
    if ((s8)gUnk_03000D38[obj[0xBE]] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj[0xBD] = value;
        gUnk_03000D38[obj[0xBE]] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj[0xBD] = gUnk_03000D38[obj[0xBE]];
    }
}

#define DEFINE_RANDOM_SLOT_FUNC_16(name)                                                                                              \
    void name(u8 *obj)                                                                                                                \
    {                                                                                                                                 \
        u8 values[16];                                                                                                                \
        u8 count;                                                                                                                     \
        u8 value;                                                                                                                     \
                                                                                                                                      \
        count = sub_80489E8((u8 *)GetObjPool(), values, 1, 0x7F);                                                                     \
        obj[0xBC] = 0;                                                                                                                \
        if ((s8)gUnk_03000D38[obj[0xBE]] < 0)                                                                                         \
        {                                                                                                                             \
            value = values[((s32 (*)(void))Rng_LcgNext)() % count];                                                                   \
            obj[0xBD] = value;                                                                                                        \
            gUnk_03000D38[obj[0xBE]] = values[((s32 (*)(void))Rng_LcgNext)() % count];                                                \
        }                                                                                                                             \
        else                                                                                                                          \
        {                                                                                                                             \
            obj[0xBD] = gUnk_03000D38[obj[0xBE]];                                                                                     \
        }                                                                                                                             \
    }

#define DEFINE_RANDOM_SLOT_FUNC_24(name)                                                                                              \
    void name(u8 *obj)                                                                                                                \
    {                                                                                                                                 \
        u8 values[24];                                                                                                                \
        u8 count;                                                                                                                     \
        u8 value;                                                                                                                     \
        u8 *base;                                                                                                                     \
                                                                                                                                      \
        base = (u8 *)GetObjPool();                                                                                                    \
        obj[0xBC] = 0;                                                                                                                \
        count = sub_80489E8(base, values, 1, 0x17F);                                                                                  \
        if ((s8)gUnk_03000D38[obj[0xBE]] < 0)                                                                                         \
        {                                                                                                                             \
            value = values[((s32 (*)(void))Rng_LcgNext)() % count];                                                                   \
            obj[0xBD] = value;                                                                                                        \
            gUnk_03000D38[obj[0xBE]] = values[((s32 (*)(void))Rng_LcgNext)() % count];                                                \
        }                                                                                                                             \
        else                                                                                                                          \
        {                                                                                                                             \
            obj[0xBD] = gUnk_03000D38[obj[0xBE]];                                                                                     \
        }                                                                                                                             \
    }

// @ 0x0804CAA0
DEFINE_RANDOM_SLOT_FUNC_24(sub_804CAA0)
// @ 0x0804CB18
void sub_804CB18(u8 *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8((u8 *)GetObjPool(), values, 1, 0x7F);
    obj[0xBC] = 0;
    if ((s8)gUnk_03000D38[obj[0xBE]] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj[0xBD] = value;
        gUnk_03000D38[obj[0xBE]] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj[0xBD] = gUnk_03000D38[obj[0xBE]];
    }
}
// @ 0x0804CB8C
DEFINE_RANDOM_SLOT_FUNC_16(sub_804CB8C)
// @ 0x0804CC00
DEFINE_RANDOM_SLOT_FUNC_24(sub_804CC00)
// @ 0x0804CC78
DEFINE_RANDOM_SLOT_FUNC_16(sub_804CC78)
// @ 0x0804CCEC
DEFINE_RANDOM_SLOT_FUNC_16(sub_804CCEC)
// @ 0x0804CD60
DEFINE_RANDOM_SLOT_FUNC_16(sub_804CD60)
// @ 0x0804CDD4
DEFINE_RANDOM_SLOT_FUNC_16(sub_804CDD4)
// @ 0x0804CE48
DEFINE_RANDOM_SLOT_FUNC_16(sub_804CE48)

#undef DEFINE_RANDOM_SLOT_FUNC_16
#undef DEFINE_RANDOM_SLOT_FUNC_24
// @ 0x0804CEBC
void sub_804CEBC(void)
{
    u8 i;
    u8 *ptr;

    for (i = 0; i <= 10; i++)
    {
        gUnk_03000D38[i] = 0xFF;
    }
}
// @ 0x0804CEE0
INCLUDE_ASM("asm/nonmatchings", sub_804CEE0);
// @ 0x0804D0F8
void sub_804D0F8(u8 *obj)
{
    u8 values[8];
    u8 count = 0;
    u8 i;
    u8 j;
    u8 *pool;

    if (*(u32 *)(*(u32 *)(obj + 0x88) + 0x1C) == 0)
    {
        obj[0xBC] = 0;
        pool = GetObjPool();
        count = sub_80489E8(pool, values, 1, 0x6F);
        if (count <= 1)
        {
            count = sub_80489E8(pool, values, 0, 0x6F);
        }
        for (i = 0; i < count; i++)
        {
            if (*(u8 *)(pool + values[i] * 0xC8 + 0xAC) == obj[0xAC])
            {
                for (j = i; j < count - 1; j++)
                    values[j] = values[j + 1];
                count--;
                break;
            }
        }
        obj[0xBD] = values[(u32)(u8)Rng_LcgNext() % count];
    }
    else
    {
        obj[0xBC] = 3;
    }
}
