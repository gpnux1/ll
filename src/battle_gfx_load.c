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

// @ 0x0801A3C4
extern u8 *gUnk_087EBE00[];

void ObjGfxLoad_Step(ObjHead *obj)
{
    switch (obj->kindFlags & 0xF)
    {
        case 1:
            LZ77UnCompVram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->vramBank << 5) + 0x06010000 + (obj->gfxPos << 12)));
            break;
        case 2:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x020212C0));
            break;
        case 3:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x02020E00));
            break;
        case 6:
            if (obj->gfxPos == 0)
            {
                DmaFill16(3, 0, (void *)0x0600C000, 0x4000);
                DmaWait(3);
            }
        case 4:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x0202B2C0));
            break;
        case 5:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x020302C0));
            break;
        case 7:
            if (obj->gfxPos == 0)
            {
                DmaFill16(3, 0, (void *)0x0600C000, 0x20);
                DmaWait(3);
            }
            LZ77UnCompVram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x0600C020));
            break;
        case 8:
            if (obj->gfxPos == 0)
            {
                DmaFill16(3, 0, (void *)0x06008000, 0x20);
                DmaWait(3);
            }
            LZ77UnCompVram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x06008020));
            break;
        case 9:
            LZ77UnCompWram(gUnk_087EBE00[obj->gfxBaseIdx + obj->gfxPos],
                           (void *)((obj->gfxPos << 12) + 0x02037C28));
            break;
    }

    obj->gfxPos++;
    if (obj->gfxPos >= obj->gfxTotal)
    {
        sub_801A684(obj);
        if ((obj->kindFlags & 0xF) != 9)
            obj->kindFlags = 0xF7FF & obj->kindFlags;
    }
}

// @ 0x0801A5EC
void ObjGfxLoad_Copy(ObjHead *dst, ObjHead *src)
{
    dst->cmdBase0 = src->cmdBase0;
    dst->cmdBase1 = src->cmdBase1;
    dst->jumpTable0 = src->jumpTable0;
    dst->jumpTable1 = src->jumpTable1;
    dst->kindFlags = src->kindFlags;
    dst->f_28 = src->f_28;
    dst->f_1A = src->f_1A;
    dst->frameIdx = src->frameIdx;
    dst->f_1E = src->f_1E;
    dst->palSlot = src->palSlot;
    dst->f_2A = src->f_2A;
    dst->f_2B = src->f_2B;
    dst->f_2C = src->f_2C;
    dst->f_2D = src->f_2D;
    dst->f_2E = src->f_2E;
    dst->scriptPtr = src->scriptPtr;
    dst->palBitsPtr = src->palBitsPtr;
    dst->gfxTotal = src->gfxTotal;
    dst->gfxPos = src->gfxPos;
    dst->vramBank = src->vramBank;
    dst->f_2F = src->f_2F;
    dst->gfxBaseIdx = src->gfxBaseIdx;
}
// @ 0x0801A684
/* 从脚本头 scriptPtr 重算命令流/跳转表四指针 (+0x0..0xC), 清帧游标后按 kind 分发。 */
void sub_801A684(ObjHead *head)
{
    const u16 *data;
    u32 off0;
    u32 off1;
    u16 type;
    u32 palBits;
    u8 pal;
    u8 zero8;
    u16 zero16;
    u8 copied;

    do
    {
        data = head->scriptPtr;
        off0 = (u32)data + data[0];
        head->cmdBase0 = (u16 *)off0;
        off1 = (u32)data + data[1];
        head->cmdBase1 = (u16 *)off1;
        head->jumpTable0 = head->cmdBase0 + 2;
        head->jumpTable1 = head->cmdBase1 + 2;

        // Keep GCC2's byte zero ahead of the independent halfword zero.
        zero8 = off0 & ~off0;
        zero16 = 0;
        head->f_1A = zero16;
        head->frameIdx = zero16;
        head->f_1E = head->vramBank;
        copied = head->f_2F;
        head->palSlot = copied;
        head->f_28 = zero8;
    } while (0);

    type = (head->kindFlags & 0xF) - 6;
    if (type <= 2)
    {
        sub_801A6F4(head);
    }
    else
    {
        palBits = (u32)head->palBitsPtr;
        pal = head->palSlot;
        sub_804C2FC(palBits, pal, sub_801B954(head));
    }
}
// @ 0x0801A6F4
INCLUDE_ASM("asm/nonmatchings", sub_801A6F4);
