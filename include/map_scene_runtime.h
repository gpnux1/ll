#ifndef MAP_SCENE_RUNTIME_H
#define MAP_SCENE_RUNTIME_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

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

/* AnimSlot / map-scene runtime API. */
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

/* Generated module API declarations (was include/code_0.h). */

u8 *AnimSlot_Parse(u16, u8 *);

u8 *AnimSlot_ParseLoop(u16, u8 *);

void AnimSlot_Step(s16); // UpdateSpriteAnim: 推进精灵动画槽帧计数, 并把当前帧图块拷进 0x02006000 图块缓存

#define UpdateSpriteAnim AnimSlot_Step

/* 形参必须是 s16 而非 u16: 唯一调用方 sub_80055E8 在两处调用点都用 `asrs #0x10`
 * 做**符号**扩展后传参 (u16 形参会生成 lsrs)。函数体内只做 `sx = arg0`,
 * 故 u16→s16 不改变 sub_8007ADC 自身的字节 (fncheck 244B 仍 OK)。 */
s32 sub_8007ADC(s16, s16); // 算 (x,y) 16x16 足迹覆盖的至多 4 个瓦片坐标, 在 gMapZoneHeader 的 cells 表查区域; 命中写

// gMapZoneType/gMapZoneEntryIdx 返回 1
#define MapZone_FindAt sub_8007ADC

s32 sub_8007BD0(
    void); // 按 gMapZoneType 0..4 分发: 0=换图(装载点+state3) 1=图内传送(state4) 2=state8 3=首次进入触发脚本 4=朝向触发脚本

#define MapZone_Trigger sub_8007BD0

void MapBg_LoadInterior(u8);

void Logo_LoadAssets(u8);

u32 ChoiceMenu_BuildList(); // 返回类型非 void 但体内无 return —— 原 ROM 即如此(占住 r0 使寄存器整体上移)

void BattleIntro_Cursor();

void ChoiceMenu_HandleInput(u16);

void DialogPortrait_Set(u8 portraitId, u8 position);

void Viewport_UpdateEffects();

void IntroBg_Load(u8);

void ScreenFade_Start(u16, s16, s16);

void AnimSlot_BankReload();

void Win0H_WaveDmaByVCount();

void ScreenFx_SetMode(u16);

void AnimSlots_Release();

void AnimSlots_StepAll();

void BgTiles_LoadUiSet(u8);

void BgScroll_LoadFromTable(u16);

void PlayerSheets_Load();

void AnimSlot_LoadSet(u8, u8); // LoadSpriteAnimSet: 把 gUnk_087EA1A0[setId] 一组精灵动画模型装入 gAnimSlots[startSlot..]

#define LoadSpriteAnimSet AnimSlot_LoadSet

void AnimSlot_Pause(u8);

void AnimSlot_Resume(u8);

u8 AnimSlot_Active(u8);

void ReloadSpriteSheet(u8 slot); // 按 slot 重载单个精灵表(图块+调色板)

void ReloadAllSpriteSheets(); // 循环 slot 0..11 全部重载

void ChoiceMenu_ResolveDest(u8);

void DialogPortrait_FlushPending(void);

u16 Camera_GetDrawOffset();

void Script_SetEnvSet(u8);

void BgPal_ResetFirst();

void AnimSlot_PlayOnce(u16, u8 *);

void BgMap_FillRow(u8);

void MapBg_FlushPending();

void ChestObjects_LoadForMap(u8);

void ChestObject_BuildSprite(u8);

#endif // MAP_SCENE_RUNTIME_H
