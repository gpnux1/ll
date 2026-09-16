#include "anim_slot_core.h"

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

u8 *AnimSlot_Parse(u16 slot, u8 *src)
{
    u32 temp_r0;
    AnimSlot *anim;

    anim = &gAnimSlots[slot];

    anim->activeBank = (src[0] & 1) + 1;
    anim->flags = src[1];
    src += 2;

    anim->numFrames = *src;
    src++;

    anim->speedShift = *src;
    src++;

    anim->destTileX = *src;
    src++;

    anim->destTileY = *src;
    src++;
    anim->width = *src;
    src++;
    anim->height = *src;
    src++;

    anim->tickCounter = 0;
    anim->srcData = src;

    temp_r0 = anim->width * anim->height * anim->numFrames;
    temp_r0 *= 2;

    src += temp_r0;

    return src;
}
// @ 0x080079BC
u8 *AnimSlot_ParseLoop(u16 slot, u8 *src)
{
    u32 temp_r0;
    AnimSlot *anim;

    anim = &gAnimSlots[slot];

    anim->activeBank = (src[0] & 1) + 1;
    anim->flags = src[1];
    src += 2;

    anim->numFrames = *src;
    src++;

    anim->speedShift = *src;
    src++;

    anim->destTileX = *src;
    src++;

    anim->destTileY = *src;
    src++;
    anim->width = *src;
    src++;
    anim->height = *src;
    src++;

    anim->tickCounter = (anim->numFrames << anim->speedShift) - 2;
    anim->srcData = src;

    temp_r0 = anim->width * anim->height * anim->numFrames;
    temp_r0 *= 2;

    src += temp_r0;

    return src;
}
// 推进精灵动画槽 slot 的帧计数, 并把当前帧的图块拷进 0x02006000 图块缓存。
// 描述符布局见 anim_slot.h 的 AnimSlot:
//   activeBank = 模型库/状态(1/2, 0=空闲)  numFrames = 帧数      speedShift = 帧分频移位
//   flags = 标志(bit1=暂停, bit0=一次性)  destTileX/destTileY = 目标图块偏移
//   width/height = 单帧宽/高(字/行)  tickCounter = 帧计数器  srcData = 帧数据指针
// 行跳距 0x80 个 u16; 每帧大小 = width * height * 2 字节。
// 注: rows 在声明处的提前赋值是 GCC2 寄存器分配所需(该死 store 会被删除,
//     但决定字面池加载位置与 home 寄存器选择), 删掉则不匹配。
// @ 0x08007A1C
void AnimSlot_Step(s16 slot)
{
    AnimSlot *ptr;
    u16 frame;
    u8 *dest;
    u8 *src;
    u8 width;
    u8 rows = gAnimSlots[slot].height;
    u32 bankOff;
    u32 rowOff;

    ptr = &gAnimSlots[slot];
    if (ptr->activeBank == 0)
        return;
    if (ptr->flags & 2)
        return;
    ptr->tickCounter++;
    frame = ptr->tickCounter >> ptr->speedShift;
    if (frame >= ptr->numFrames)
    {
        if (ptr->flags & 1)
            gAnimSlots[slot].activeBank = 0;
        gAnimSlots[slot].tickCounter = 0;
        frame = 0;
    }
    if (gAnimSlots[slot].activeBank == 0)
        return;
    bankOff = (gAnimSlots[slot].activeBank - 1) << 15;
    rowOff = (gAnimSlots[slot].destTileY << 8) + 0x02006000;
    dest = (u8 *)(bankOff + rowOff + (gAnimSlots[slot].destTileX << 1));
    src = (u8 *)(gAnimSlots[slot].srcData + (gAnimSlots[slot].width * gAnimSlots[slot].height * frame * 2));
    rows = gAnimSlots[slot].height;
    while (rows != 0)
    {
        width = gAnimSlots[slot].width;
        while (width != 0)
        {
            *(u16 *)dest = *(u16 *)src;
            src += 2;
            dest += 2;
            width--;
        }
        dest += (0x80 - gAnimSlots[slot].width) * 2;
        rows--;
    }
}
