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

// INCLUDE_ASM("asm/nonmatchings", BattleFx_Reset);
// @ 0x0804F210
void BattleFx_Reset(void)
{
    u8 i;

    for (i = 0; i <= 4; i++)
    {
        gBattleFxObjs[i] = 0;
    }

    gBattleFxObjCount = 0;
    gBattleFxState = 0;
}
// @ 0x0804F244
u8 BattleFx_GetObjCount()
{
    return gBattleFxObjCount;
}
// @ 0x0804F250
void BattleDrops_Clear(void)
{
    u8 i;

    for (i = 0; i <= 9; i++)
    {
        gBattleDrops[i].itemId = 0;
        gBattleDrops[i].count = 0;
    }

    gBattleDropCount = 0;
}
