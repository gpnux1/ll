
#include "save.h"
#include "code_0.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "globals.h"
#include "include_asm.h"
#include "m4a.h"

// @ 0x080109F8
s32 Save_Fsm(u8 arg0)
{

    switch (gSaveFsmState)
    {

        case 0x1:
            ReadSram((u8 *)SRAM + (gSaveSramBlock << 0xB), (u8 *)0x02021000 + (gSaveSramBlock << 0xB), 0x800);
            gSaveFsmState++;
            return 1;

        case 0x2:
            if (VerifySram((u8 *)(gSaveSramBlock << 0xB) + 0x02021000, (u8 *)SRAM + (gSaveSramBlock << 0xB), 0x800) != 0)
            {
                gSaveFsmState = 0xFD;
            }
            else
            {
                if (gSaveSramBlock <= 0xE)
                {
                    gSaveFsmState--;
                }
                else
                {
                    gSaveFsmState = 0xF9;
                }
                gSaveSramBlock++;
            }
            return 1;

        case 0x3:
            WriteSram((u8 *)(gSaveSramBlock << 0xB) + 0x02021000, (u8 *)SRAM + (gSaveSramBlock << 0xB), 0x800);
            gSaveFsmState++;
            return 1;

        case 0x4:
            if (VerifySram((u8 *)(gSaveSramBlock << 0xB) + 0x02021000, (u8 *)SRAM + (gSaveSramBlock << 0xB), 0x800) != 0)
            {
                gSaveFsmState = 0xFD;
                Msg_ShowById(0x27, 0xB);
                if (gMenuCursorSel > 5U)
                {
                    gMenuCursorSel = gSaveCurSlot + 3;
                    sub_800E668(0xFF);
                }
            }
            else
            {
                if (gSaveSramBlock <= 0xE)
                {
                    gSaveFsmState--;
                }
                else
                {
                    gSaveFsmState = 0xFF;
                    Msg_ShowById(0x19, 0xB);
                    if (gMenuCursorSel > 5)
                    {
                        gMenuCursorSel = gSaveCurSlot + 3;
                        sub_800E668(0xFF);
                    }
                }
                gSaveSramBlock++;
            }
            return 1;

        case 0xF9:
            Save_FillSlot0(0);
            gSaveFsmState = 0xFA;
            return 1;

        case 0xFA:
            Save_FillSlot0(1);
            gSaveFsmState = 0xFB;
            return 1;

        case 0xFB:
            Save_FillSlot0(2);
            gSaveFsmState = 0xFC;
            return 1;

        case 0xFC:
            Save_FillSlot0(3);
            gSaveFsmState = 0xFF;
            return 1;

        case 0xFD:
            *(u8 *)0x02021000 = 0xFF;
            *(u8 *)0x02023000 = 0xFF;
            *(u8 *)0x02025000 = 0xFF;
            *(u8 *)0x02027000 = 0xFF;
            gSaveFsmState = 0xFF;
            return 1;

        case 0xFF:
            if (arg0 == 0)
            {
                SaveUi_DrawSlots();
            }
            gSaveFsmState = 0xFE;
            return 1;

        default:
            return 0;
    }
}

// @ 0x08010BEC
INCLUDE_ASM("asm/matchings", Save_FillSlot0);
// @ 0x08010CCC
INCLUDE_ASM("asm/matchings", Save_FillSlot1);
// @ 0x08010D80
INCLUDE_ASM("asm/matchings", Save_FillSlot2);
// @ 0x08010E58
INCLUDE_ASM("asm/matchings", Save_FillSlot3);