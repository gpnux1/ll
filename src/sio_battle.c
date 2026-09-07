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

// INCLUDE_ASM("asm/nonmatchings", SioBattle_ResetState);
// @ 0x0804F210
void SioBattle_ResetState(void)
{
    u8 i;

    for (i = 0; i <= 4; i++)
    {
        gUnk_03000DF0[i] = 0;
    }

    gUnk_03000E04 = 0;
    gUnk_03000E05 = 0;
}
// @ 0x0804F244
u8 SioBattle_GetState()
{
    return gUnk_03000E04;
}
// @ 0x0804F250
void SioBattle_ClearSlots(void)
{
    u8 i;

    for (i = 0; i <= 9; i++)
    {
        gUnk_03000E08[i].field_0 = 0;
        gUnk_03000E08[i].field_2 = 0;
    }

    gUnk_03000E30 = 0;
}
