
#include "save.h"
#include "battle_types.h"
#include "menu.h"
#include "menu_ui.h"
#include "text_engine.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "globals.h"
#include "engine_core.h"
#include "sprite_engine.h"
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

extern const u8 gSaveSignature[];
extern const u16 word_80981B0[];
extern const u16 gUnk_080981E6[];
extern u8 *const gSaveSlotSourceTable[];
extern u8 gActiveSaveSlot;

#define SAVE_BASE 0x02021000
#define GET_SAVE_SLOT(n) ((u8 *)(SAVE_BASE + ((n) * 0x2000)))

/* 槽首 12B 签名比对: 与 gSaveSignature 一致返回 0, 不符返回 1。
 * Save_FillSlot3 内联展开该检查 (menu.c 的 Save_SigCheck 同义)。 */
static inline u8 Save_SigCheck(u8 *p)
{
    u8 i;

    for (i = 0; i < 12; i++)
    {
        if (*p != gSaveSignature[i])
            return 1;
        p++;
    }

    return 0;
}

// @ 0x08010BEC
u8 Save_FillSlot0(u8 slot)
{
    u8 *slotData;
    u8 *sigPtr;
    u8 i;
    u16 blockIdx;
    u16 offset;
    u16 blockSize;
    u8 checksum;

    if (slot > 3)
        return 0xFF;

    slotData = GET_SAVE_SLOT(slot);
    sigPtr = slotData;

    for (i = 0; i < 12; i++)
    {
        if (*sigPtr != gSaveSignature[i])
        {
            return 1;
        }
        sigPtr++;
    }

    offset = 12;
    blockIdx = 0;
    checksum = 0;

    if (slot != 3)
    {
        while ((blockSize = word_80981B0[blockIdx]) != 0)
        {
            while (blockSize != 0)
            {
                checksum += slotData[offset];
                offset++;
                blockSize--;
            }
            blockIdx++;
        }
    }
    else
    {
        while ((blockSize = gUnk_080981E6[blockIdx]) != 0)
        {
            while (blockSize != 0)
            {
                checksum += slotData[offset];
                offset++;
                blockSize--;
            }
            blockIdx++;
        }
    }

    if (slotData[offset] != checksum)
    {
        sigPtr = GET_SAVE_SLOT(slot);
        *sigPtr = 0xFF;
        return 1;
    }

    return 0;
}
// @ 0x08010CCC
void Save_FillSlot1(u8 slot)
{
    u8 *slotData;
    u8 *src;
    u16 offset;
    u16 blockIdx;
    u16 blockSize;
    u8 checksum;

    if (slot > 3)
        return;

    if (slot < 3)
        gActiveSaveSlot = slot;

    slotData = GET_SAVE_SLOT(slot);

    for (offset = 0; offset < 12; offset++)
        slotData[offset] = 0xFF;

    checksum = 0;
    offset = 12;
    blockIdx = 0;

    while ((blockSize = word_80981B0[blockIdx]) != 0)
    {
        src = gSaveSlotSourceTable[blockIdx];

        while (blockSize != 0)
        {
            slotData[offset] = *src;
            checksum += slotData[offset];
            offset++;
            src++;
            blockSize--;
        }
        blockIdx++;
    }
    slotData[offset] = checksum;

    for (offset = 0; offset < 12; offset++)
        slotData[offset] = gSaveSignature[offset];
}

// @ 0x08010D80
void Save_FillSlot2(u8 slot)
{
    u8 *slotData;
    u8 *src;
    u16 *buf;
    u32 i;
    u16 offset;
    u16 blockSize;

    if (slot > 3)
        return;

    if (slot < 3)
        gActiveSaveSlot = slot;

    slotData = GET_SAVE_SLOT(slot);

    offset = 12;
    i = 0;
    blockSize = word_80981B0[0];

    if (blockSize != 0)
    {
        do
        {
            src = gSaveSlotSourceTable[i];
            i++;

            while (blockSize != 0)
            {
                *src = slotData[offset];
                offset++;
                src++;
                blockSize--;
            }

            i = (u16)i;
            blockSize = word_80981B0[i];
        } while (blockSize != 0);
    }

    SwitchFlags_ClearRange();
    gAfterBattleCounter = 0;
    Followers_SyncToTail();
    gGameState = 12;
    gVBlankPipelineMode = 1;
    Display_ShutdownSequence();

    buf = (u16 *)0x02005800;
    for (offset = 0; offset <= 0x3FF; offset++)
    {
        *buf++ = 0;
    }

    {
        vu32 *dmaRegs = (vu32 *)0x040000D4;
        dmaRegs[0] = 0x02005800;
        dmaRegs[1] = 0x0600F800;
        dmaRegs[2] = 0x80000400;
        dmaRegs[2];
    }
}
// @ 0x08010E58
void Save_FillSlot3(u8 slot)
{
    u8 *p;

    if (slot > 3)
        return;

    if (Save_SigCheck(GET_SAVE_SLOT(slot)) != 0)
    {
        gUnk_03000204[slot] = 0xFF;
        gUnk_03000208[slot] = 0;
        gUnk_03000210[slot] = 0;
        return;
    }

    p = GET_SAVE_SLOT(slot);
    gUnk_03000204[slot] = p[0x1A];
    gUnk_03000208[slot] = p[0x14];
    gUnk_03000210[slot] = p[0xC] + (p[0xD] << 8) + (p[0xE] << 16) + (p[0xF] << 24);
}