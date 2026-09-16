#include "sprite_bg_sheet_loader.h"

#include "map_scene_runtime.h"
#include "battle_types.h"
#include "engine_core.h"
#include "map_view.h"
#include "player_stats.h"
#include "scene_mgr.h"
#include "script_vm.h"
#include "sound.h"
#include "sprite_engine.h"
#include "text_engine.h"
#include "vram_transfer.h"
#include "data_87E83F0.h"
#include "data_805769C.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"

// @ 0x08008A3C
void AnimSlots_Release(void)
{
    s16 i;

    for (i = 0; i < NUM_ANIM_SLOTS; i++)
    {
        gAnimSlots[i].activeBank = 0;
    }
}

// @ 0x08008A60
void AnimSlots_StepAll(void)
{
    s16 i;

    for (i = 0; i < 16; i++)
    {
        AnimSlot_Step(i);
    }
}
// @ 0x08008A80
void BgTiles_LoadUiSet(u8 skipFont)
{
    if (skipFont == 0)
    {
        LZ77UnCompVram((void *)0x0809C8B4, (void *)0x0600C800);
        nullsub_5();
    }
    LZ77UnCompVram((void *)0x0809CB90, (void *)0x06008000);
    nullsub_5();
    LZ77UnCompVram((void *)0x0809D198, (void *)0x06009000);
    nullsub_5();
    LZ77UnCompVram((void *)0x0809D718, (void *)0x0600A000);
    nullsub_5();
    LZ77UnCompVram((void *)0x0809DCE8, (void *)0x0600B000);
    nullsub_5();

    DmaCopy16(3, 0x0809C834, 0x05000160, 0x60);

    nullsub_5();
}
// @ 0x08008B14
void BgScroll_LoadFromTable(u16 mapIdx)
{

    gCameraMinX = gMapViewportBoundsTable[mapIdx].cameraMinXBlocks << 6;

    gCameraMinY = gMapViewportBoundsTable[mapIdx].cameraMinYBlocks << 6;

    gMapWidthPx = gCameraMinX + (gMapViewportBoundsTable[mapIdx].mapWidthBlocks << 6);

    gMapHeightPx = gCameraMinY + (gMapViewportBoundsTable[mapIdx].mapHeightBlocks << 6);
}
// @ 0x08008B5C
void PlayerSheets_Load(void)
{
    // gSlotGfxId.field_0 = gPartyMemberIds[0];
    gSlotGfxId[0] = gPartyMemberIds[0];
    gSlotPalId[0] = gPartyMemberIds[0];

    LoadSpriteSheetGfx(0, gPartyMemberIds[0]);
    LoadSpriteSheetPal(0, gPartyMemberIds[0]);
    // gSlotGfxId.field_1 = 11;
    gSlotGfxId[1] = 11;
    gSlotPalId[1] = 11;
    LoadSpriteSheetGfx(1, 0xBU);
    LoadSpriteSheetPal(1, 0xBU);
}
// 0x087EA1A0: 248 项指针表, 每项指向一组 "精灵动画模型" 记录
//   记录块格式: u16 count; 随后 count 条变长记录 (见 AnimSlot_Parse)

// 把 gUnk_087EA1A0[setId] 这一组动画模型 (共 *ptr 条) 逐条解析进
// gAnimSlots[] 精灵模型描述符数组, 起始槽位为 startSlot。
// 槽位号 = startSlot + 记录序号, 与 MapScene_LoadEventAnimations (整组装入槽位 0..) 是同族写法。
// @ 0x08008BA4
void AnimSlot_LoadSet(u8 setId, u8 startSlot)
{
    u8 *src;
    u16 endSlot;
    u16 slot;

    src = gUnk_087EA1A0[setId];
    endSlot = *(u16 *)src + startSlot;
    src += 2;

    for (slot = startSlot; slot < endSlot; slot++)
    {
        src = AnimSlot_Parse(slot, src);
    }
}

// @ 0x08008BE4
void AnimSlot_Pause(u8 slot)
{
    gAnimSlots[slot].flags |= ANIM_SLOT_FLAG_PAUSED;
}

// @ 0x08008BFC
void AnimSlot_Resume(u8 slot)
{
    gAnimSlots[slot].flags &= 0xFD;
}

// @ 0x08008C14
u8 AnimSlot_Active(u8 slot)
{
    return gAnimSlots[slot].activeBank;
}

/* 重载单个精灵表槽位: 图块用 gSlotGfxId[slot]、调色板用 gSlotPalId[slot],
 * 0xFF 表示该部分不动。整块受 gObjGraphicsSetId 的 bit7 屏蔽。 */
// @ 0x08008C24
void ReloadSpriteSheet(u8 slot)
{

    if (!(gObjGraphicsSetId & GFXSET_NO_SPRITE_LOAD))
    {
        if (gSlotGfxId[slot] != 0xFF)
        {
            LoadSpriteSheetGfx(slot, gSlotGfxId[slot]);
        }

        if (gSlotPalId[slot] != 0xFF)
        {
            LoadSpriteSheetPal(slot, gSlotPalId[slot]);
        }
    }
}

/* 全量重载全部 12 个精灵表槽位(0x06011400 + 12*0x900 = 0x06018000 正好到 VRAM 尾)。 */
// @ 0x08008C70
void ReloadAllSpriteSheets(void)
{
    u8 i;

    for (i = 0; i < 12; i++)
    {
        if (!(gObjGraphicsSetId & GFXSET_NO_SPRITE_LOAD))
        {
            if (gSlotGfxId[i] != 0xFF)
            {
                LoadSpriteSheetGfx(i, gSlotGfxId[i]);
            }
            if (gSlotPalId[i] != 0xFF)
            {
                LoadSpriteSheetPal(i, gSlotPalId[i]);
            }
        }
    }
}
