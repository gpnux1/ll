#include "map_misc_runtime.h"

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

extern const u8 gChoiceDestTable[];

/* 把"当前选项号"解析成一个目的地像素坐标, 写进 gChoiceDestX / gChoiceDestY。
 *
 * gChoiceDestTable @0x08087648 是**分组变长表**: 每组 = [count][count × {x, y}],
 * 组间无填充, 由 count 推出下一组起点 (count*2 是数据字节数, 再 +1 跳过 count 字节)。
 * 实测 5 组, count = 5/7/9/9/5, 共 35 个目的地, 消耗 75/76 字节 (末 1 字节为 0 终止)。
 * 值域 x∈12..200, y∈32..128 → 240×160 屏幕的**像素坐标**。
 *
 * 组号 = gChoiceGroupIdx (由 ChoiceMenu_BuildList 从 gChoiceDataBase 分层记录流定位);
 * 选项号 = 调用者传入 (Scene_EnterDoor 传 gChoiceListPtr[gChoiceCursor] 的低 nibble)。
 *
 * 代码生成要点 (已逐字节验证, bytecmp OK 88B):
 *   - 必须写成 `skipLen = *ptr << 1` 的**先读后自增**结构, 目标才是
 *     `ldrb; lsls #0x19; lsrs #0x18` + `adds r1,#1` 的形态
 *   - 循环里 `ptr += skipLen; i++; skipLen = *ptr<<1; ptr++;` 的顺序不能调
 *     (目标把 ptr++ 放在读 count 之后, 与 while 的底部测试配合)
 *   - 取项写成 `ptr + (arg0 << 1)` 再 `ptr[0]`/`ptr[1]`, 不要合并成 `ptr[arg0*2]`
 */
// @ 0x08008CC0
void ChoiceMenu_ResolveDest(u8 choiceIdx)
{
    const u8 *ptr;
    u8 i;
    u8 skipLen;

    ptr = gChoiceDestTable;
    i = 0;
    skipLen = *ptr << 1;
    ptr++;

    while (i != gChoiceGroupIdx)
    {
        ptr += skipLen;
        i++;
        skipLen = *ptr << 1;
        ptr++;
    }

    ptr = ptr + (choiceIdx << 1);
    gChoiceDestX = ptr[0];
    gChoiceDestY = ptr[1];
}
// }

// @ 0x08008D18
void DialogPortrait_FlushPending(void)
{
    u32 i;
    u16 *dest;

    if (gPendingPortraitSlot != 0)
    {

        LZ77UnCompVram(gPendingPortraitGfx, (void *)(((gPendingPortraitSlot - 1) >> 1) * 0x800 + 0x0600D000));

        i = (gPendingPortraitSlot - 1) >> 1;

        dest = (u16 *)(i * 0x20 + 0x050001C0);
        DmaCopy16(3, gPendingPortraitPalette, dest, 0x20);

        gPendingPortraitSlot = 0;
    }
}

// @ 0x08008D78
u16 Camera_GetDrawOffset(void)
{
    switch (gCameraDrawMode)
    {
        case 2:
            return gDrawCamY >> 4;

        case 4:
            return gDrawCamY - 0x20;

        case 7:
            return gDrawCamY - gCameraPosY;

        default:
            return 0;
    }
}

// @ 0x08008DCC
void Script_SetEnvSet(u8 songId)
{
    gEnvScriptSetId = songId;
}

// @ 0x08008DD8
void BgPal_ResetFirst(void)
{
    DmaCopy16(3, (void *)0x08087216, (void *)0x05000000, 2);
}

// @ 0x08008DF8
void AnimSlot_PlayOnce(u16 slot, u8 *data)
{
    u16 count;

    count = *(u16 *)data;
    data += 2;

    while (count != 0)
    {
        data = AnimSlot_ParseLoop(slot, data);

        AnimSlot_Step(slot);

        gAnimSlots[slot].activeBank = 0;
        count--;
    }
}
// @ 0x08008E44
void BgMap_FillRow(u8 mode)
{
    u16 *dest;
    u16 i, j;
    u16 val;

    dest = (u16 *)0x020053A8;

    val = 32;

    if (mode != 0)
    {
        val = 0;
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 8; j++)
        {
            *dest = val + 0xA200;
            dest++;
            val++;
        }
        dest += 24;
    }
    gViewportFlags[13] = 1;
}

// @ 0x08008E94
void MapBg_FlushPending(void)
{
    switch (gIntroBgTransferStage)
    {
        case 1:
            switch (gIntroBgTileSetIndex)
            {
                case 0:
                    DmaCopy32(3, (void *)0x02020000, (void *)0x06000000, 0xc00);
                    break;

                case 1:
                    DmaCopy32(3, (void *)0x02020000, (void *)0x06000C00, 0xc0);
                    break;
            }
            gIntroBgTransferStage = 0;
            break;

        case 2:
            DmaCopy32(3, (void *)0x02020000, (void *)0x0600E000, 0x800);
            gIntroBgTransferStage = 0;
            break;
    }
}
/* 按当前地图从 0x08088400 的 256 项表中装载宝箱对象；见 ChestSpawnEntry。 */
