#ifndef _ANIM_SLOT_H
#define _ANIM_SLOT_H

#include "gba/types.h"

#define NUM_ANIM_SLOTS 16

/* AnimSlot 标志位 (flags / field_3) */
#define ANIM_SLOT_FLAG_ONESHOT 0x01 /* 单次播放: 循环结束后自动注销 (activeBank = 0) */
#define ANIM_SLOT_FLAG_PAUSED  0x02 /* 暂停: 保持当前帧, 步进函数跳过更新 */

/**
 * 精灵动画槽结构体 (AnimSlot, 原 Unk_030046A0)
 * 位于 IWRAM 0x030046A0, 共 16 个槽位 (每个 16 字节, 总计 256 字节, 0x030046A0 - 0x030047A0)。
 * 负责解析变长精灵动画帧数据, 并按帧率将图块搬运至 0x02006000 图块缓存。
 */
typedef struct AnimSlot
{
    /* 0x00 */ u8 activeBank;   /* 槽位状态/模型库选择 (0=空闲/未激活, 1=Bank 0 偏移 0x0, 2=Bank 1 偏移 0x8000) */
    /* 0x01 */ u8 numFrames;    /* 动画总帧数 (frame count) */
    /* 0x02 */ u8 speedShift;   /* 帧率分频移位量 (每个逻辑帧持续 1 << speedShift 拍, 当前帧序号 = tickCounter >> speedShift) */
    /* 0x03 */ u8 flags;        /* 播放控制标志 (bit0: ANIM_SLOT_FLAG_ONESHOT, bit1: ANIM_SLOT_FLAG_PAUSED) */
    /* 0x04 */ u8 unk4;         /* 保留字段 / 填充对齐 */
    /* 0x05 */ u8 unk5;         /* 保留字段 / 填充对齐 */
    /* 0x06 */ u8 destTileX;    /* 目标图块在图块缓存中的 X 列偏移 (以 16-bit 字为单位, 字节偏移 = destTileX * 2) */
    /* 0x07 */ u8 destTileY;    /* 目标图块在图块缓存中的 Y 行偏移 (每行 256 字节, 字节偏移 = destTileY * 256) */
    /* 0x08 */ u8 width;        /* 单帧宽度 (以 16-bit 字/图块为单位) */
    /* 0x09 */ u8 height;       /* 单帧高度 (图块行数) */
    /* 0x0A */ u16 tickCounter; /* 内部帧步进计时器 (每次 AnimSlot_Step 自增 1) */
    /* 0x0C */ u8 *srcData;     /* 帧原始图块数据指针 (大小为 width * height * numFrames * 2 字节) */
} AnimSlot;

/* 向后兼容别名 */
typedef struct AnimSlot Unk_030046A0;

extern AnimSlot gAnimSlots[NUM_ANIM_SLOTS];

/* AnimSlot 核心管理与播放 API */
u8 *AnimSlot_Parse(u16 slot, u8 *src);
u8 *AnimSlot_ParseLoop(u16 slot, u8 *src);
void AnimSlot_Step(s16 slot);
void AnimSlots_Release(void);
void AnimSlots_StepAll(void);
void AnimSlot_LoadSet(u8 setId, u8 startSlot);
void AnimSlot_Pause(u8 slot);
void AnimSlot_Resume(u8 slot);
u8 AnimSlot_Active(u8 slot);
void AnimSlot_PlayOnce(u16 slot, u8 *data);
void AnimSlot_BankReload(void);

#endif // _ANIM_SLOT_H
