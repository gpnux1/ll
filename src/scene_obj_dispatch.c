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

void sub_801A3C4(Unk_801A5EC *obj)
{
    switch (obj->f_18 & 0xF)
    {
        case 1:
            LZ77UnCompVram(gUnk_087EBE00[obj->f_26 + obj->f_22],
                           (void *)((obj->f_24 << 5) + 0x06010000 + (obj->f_22 << 12)));
            break;
        case 2:
            LZ77UnCompWram(gUnk_087EBE00[obj->f_26 + obj->f_22],
                           (void *)((obj->f_22 << 12) + 0x020212C0));
            break;
        case 3:
            LZ77UnCompWram(gUnk_087EBE00[obj->f_26 + obj->f_22],
                           (void *)((obj->f_22 << 12) + 0x02020E00));
            break;
        case 6:
            if (obj->f_22 == 0)
            {
                DmaFill16(3, 0, (void *)0x0600C000, 0x4000);
                DmaWait(3);
            }
        case 4:
            LZ77UnCompWram(gUnk_087EBE00[obj->f_26 + obj->f_22],
                           (void *)((obj->f_22 << 12) + 0x0202B2C0));
            break;
        case 5:
            LZ77UnCompWram(gUnk_087EBE00[obj->f_26 + obj->f_22],
                           (void *)((obj->f_22 << 12) + 0x020302C0));
            break;
        case 7:
            if (obj->f_22 == 0)
            {
                DmaFill16(3, 0, (void *)0x0600C000, 0x20);
                DmaWait(3);
            }
            LZ77UnCompVram(gUnk_087EBE00[obj->f_26 + obj->f_22],
                           (void *)((obj->f_22 << 12) + 0x0600C020));
            break;
        case 8:
            if (obj->f_22 == 0)
            {
                DmaFill16(3, 0, (void *)0x06008000, 0x20);
                DmaWait(3);
            }
            LZ77UnCompVram(gUnk_087EBE00[obj->f_26 + obj->f_22],
                           (void *)((obj->f_22 << 12) + 0x06008020));
            break;
        case 9:
            LZ77UnCompWram(gUnk_087EBE00[obj->f_26 + obj->f_22],
                           (void *)((obj->f_22 << 12) + 0x02037C28));
            break;
    }

    obj->f_22++;
    if (obj->f_22 >= obj->f_20)
    {
        sub_801A684((u8 *)obj);
        if ((obj->f_18 & 0xF) != 9)
            obj->f_18 = 0xF7FF & obj->f_18;
    }
}
