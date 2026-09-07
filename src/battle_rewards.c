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

typedef struct
{
    u8 pad_0[0x10];
    u16 field_10;
    u8 pad_12[2];
} Unk_804D1B4_Entry;

extern Unk_804D1B4_Entry gUnk_08393B28_entries[];

// @ 0x0804D1B4
void sub_804D1B4(u8 *obj, u8 *arg1)
{
    u8 values[8];
    u8 count;
    u8 value;
    Unk_804D1B4_Entry *entry;

    count = sub_80489E8(arg1, values, 0, 0x6F);
    if (((u32 (*)(void))Rng_LcgNext)() % 0x65 < count * 15)
        obj[0xBC] = 1;
    else
        obj[0xBC] = 0;
    switch ((s8)obj[0xBC])
    {
        case 0:
            entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 2)];
            break;
        case 1:
            obj[0xC2] = 0;
            entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 8)];
            break;
    }
    switch (entry->field_10)
    {
        case 0:
            value = values[(u32)(u8)Rng_LcgNext() % count];
            obj[0xBD] = value;
            break;
        case 1:
            obj[0xBD] = 0;
            break;
    }
}
// @ 0x0804D260
void sub_804D260(u8 *obj, u8 *arg1)
{
    u8 values[8];
    u8 count;
    u8 value;
    Unk_804D1B4_Entry *entry;

    count = sub_80489E8(arg1, values, 0, 0x6F);
    if (((u32 (*)(void))Rng_LcgNext)() % 0x65 < count * 10)
        obj[0xBC] = 1;
    else
        obj[0xBC] = 0;
    switch ((s8)obj[0xBC])
    {
        case 0:
            entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 2)];
            break;
        case 1:
            obj[0xC2] = 0;
            entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 8)];
            break;
    }
    switch (entry->field_10)
    {
        case 0:
            value = values[(u32)(u8)Rng_LcgNext() % count];
            obj[0xBD] = value;
            break;
        case 1:
            obj[0xBD] = 0;
            break;
    }
}
// @ 0x0804D310
void sub_804D310(u8 *obj, u8 *arg1)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 zero;
    Unk_804D1B4_Entry *entry;

    count = sub_80489E8(arg1, values, 0, 0x6F);
    if (((u32 (*)(void))Rng_LcgNext)() % 0x65 < count * 10)
        obj[0xBC] = 1;
    else
        obj[0xBC] = 0;
    obj[0xBC] = 0;
    entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 2)];
    zero = 0;
    switch (entry->field_10)
    {
        case 0:
            value = values[(u32)(u8)Rng_LcgNext() % count];
            obj[0xBD] = value;
            break;
        case 1:
            obj[0xBD] = zero;
            break;
    }
}
// @ 0x0804D3A0
void sub_804D3A0(u8 *obj, u8 *arg1)
{
    u8 values[8];
    u8 count;
    u8 value;
    Unk_804D1B4_Entry *entry;

    count = sub_80489E8(arg1, values, 0, 0x6F);
    if (((u32 (*)(void))Rng_LcgNext)() % 0x65 < count * 13)
        obj[0xBC] = 1;
    else
        obj[0xBC] = 0;
    switch ((s8)obj[0xBC])
    {
        case 0:
            entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 2)];
            break;
        case 1:
            obj[0xC2] = 0;
            entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 8)];
            break;
    }
    switch (entry->field_10)
    {
        case 0:
            value = values[(u32)(u8)Rng_LcgNext() % count];
            obj[0xBD] = value;
            break;
        case 1:
            obj[0xBD] = 0;
            break;
    }
}
// @ 0x0804D44C
void sub_804D44C(u8 *obj, u8 *arg1)
{
    u8 values[8];
    u8 count;
    u8 value;
    Unk_804D1B4_Entry *entry;

    count = sub_80489E8(arg1, values, 0, 0x6F);
    if (((u32 (*)(void))Rng_LcgNext)() % 0x65 < count * 10)
        obj[0xBC] = 1;
    else
        obj[0xBC] = 0;
    switch ((s8)obj[0xBC])
    {
        case 0:
            entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 2)];
            break;
        case 1:
            obj[0xC2] = 0;
            entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 8)];
            break;
    }
    switch (entry->field_10)
    {
        case 0:
            value = values[(u32)(u8)Rng_LcgNext() % count];
            obj[0xBD] = value;
            break;
        case 1:
            obj[0xBD] = 0;
            break;
    }
}
// @ 0x0804D4FC
INCLUDE_ASM("asm/nonmatchings", sub_804D4FC);
// @ 0x0804D5B4
INCLUDE_ASM("asm/nonmatchings", sub_804D5B4);
// @ 0x0804D708
// 概率判定+drop道具。⚠ 2026-09-03 还原 INCLUDE_ASM: 原 C 代码比 ROM 少 4 字节
// (ROM 尾部死 store `movs r0,#0; strb r0,[obj+0xBC]` 被 C 编译器优化掉),
// 直接导致全局 +4 位移 bug。待用 do-while 屏障/中间变量复现死 store 后重匹配。
void sub_804D708(u8 *obj, u8 *arg1)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 zero;
    Unk_804D1B4_Entry *entry;

    count = sub_80489E8(arg1, values, 0, 0x6F);
    if (((u32 (*)(void))Rng_LcgNext)() % 0x65 < count * 10)
        obj[0xBC] = 1;
    else
        obj[0xBC] = 0;
    obj[0xBC] = 0;
    entry = &gUnk_08393B28_entries[*(u16 *)(*(u32 *)(obj + 0x88) + 2)];
    zero = 0;
    switch (entry->field_10)
    {
        case 0:
            value = values[(u32)(u8)Rng_LcgNext() % count];
            obj[0xBD] = value;
            break;
        case 1:
            obj[0xBD] = zero;
            break;
    }
}
// @ 0x0804D798
INCLUDE_ASM("asm/matchings", sub_804D798); /* 函数清单修正: tsv=1 且 .s 已在 matchings/ (坑7); 见 INCIDENTS.md */
// @ 0x0804D840
INCLUDE_ASM("asm/nonmatchings", sub_804D840);
// @ 0x0804D8F4
INCLUDE_ASM("asm/nonmatchings", sub_804D8F4);
// @ 0x0804DA04
INCLUDE_ASM("asm/nonmatchings", sub_804DA04);
// @ 0x0804DABC
typedef struct
{
    u8 pad_0[8];
    u16 field_8[4];
} Unk_804DABC_Ptr;

void sub_804DABC(u8 *obj, u8 *arg1)
{
    u8 values[8];
    u8 count;
    u8 value;
    u8 v;
    Unk_804D1B4_Entry *entry;

    count = sub_80489E8(arg1, values, 0, 0x6F);
    if (((u32 (*)(void))Rng_LcgNext)() % 0x65 <= 0x45)
        obj[0xBC] = 1;
    else
        obj[0xBC] = 0;
    obj[0xBC] = 1;
    v = ((u32 (*)(void))Rng_LcgNext)() & 3;
    obj[0xC2] = v;
    if (v == 1)
        obj[0xC2] = 0;
    entry = &gUnk_08393B28_entries[((Unk_804DABC_Ptr *)(*(u32 *)(obj + 0x88)))->field_8[obj[0xC2]]];
    switch (entry->field_10)
    {
    case 0:
        value = values[(u32)(u8)Rng_LcgNext() % count];
        obj[0xBD] = value;
        break;
    case 1:
        obj[0xBD] = 0;
        break;
    }
}
// @ 0x0804DB64
INCLUDE_ASM("asm/nonmatchings", sub_804DB64);
// @ 0x0804DC24
INCLUDE_ASM("asm/nonmatchings", sub_804DC24);
// @ 0x0804DCD8
INCLUDE_ASM("asm/nonmatchings", sub_804DCD8);
