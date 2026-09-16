#ifndef MAP_VIEW_H
#define MAP_VIEW_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

void sub_8005020();

/* VBlank sprite/palette transfer plus WIN0H iris-transition table generation. */
#define VBlank_UpdateSpriteAndWindow sub_8005020

/* Apply the active fade level to BLDCNT/BLDALPHA/BLDY for the current scanline. */
void ScreenFade_Apply();

/* Per-frame blend register refresh and fade progress update. */
void ScreenFade_Update();

void sub_80052F8();

void sub_80053B4(u16, u16);

/* 返回 1 = 坐标已步进(未受阻挡), 0 = 未移动(对话/传送锁步、被瓦片或实体挡住, 或本帧被区域脚本接管)。
 * 原 ROM 两条出口都显式写 r0 (movs r0,#0 / movs r0,#1), 故返回类型不能是 void。 */
s32 sub_80055E8(u16 *, u16 *, u8, u8); // 按方向码 1..8 (gWalkDirVectors) 步进相机目标坐标, 含瓦片碰撞+滑动+实体阻挡检查

#define MovePlayer sub_80055E8

u16 *MapTile_At(s16, s16);

u16 MapTile_CollisionBits(u16 *, u16, u16);

void Viewport_UpdateScroll();

void BgMap_FillPattern(u16);

void MapBg_LoadFull(u8);

void MapScene_Load();

void MapScene_LoadNpcSlotIds(u8);

void MapScene_InitSprites(u8);

void MapScene_LoadEventAnimations(u8);

#endif // MAP_VIEW_H
