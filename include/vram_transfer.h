#ifndef VRAM_TRANSFER_H
#define VRAM_TRANSFER_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

/* Module API declarations (was include/code_0.h). */

u16 VramTransfer_AllocSlot();

u8 PalTransfer_AllocSlot();

void PalTransfer_Enqueue(u8, u32, s8, u8);

void VramTransfer_Clear();

void VramTransfer_Flush();

void VramTransfer_Enqueue(u16, void *, void *, u8);

void PalTransfer_Clear();

void SpritePool_Clear();

void Queue34C0_Clear();

void RenderQueue_Clear();

u8 Sprite_AllocNode();

SpriteNode *Sprite_InitChainNode(SpriteNode *, u8, u16, u16, u16);

void LoadSpriteSheetGfx(u8 slot, u16 gfxId); // LZ77 解压缩精灵图块 → OBJ 图块槽 slot

void LoadSpriteSheetPal(u8 slot, u16 palId); // DMA3 装 16 色调色板 → OBJ PLTT 槽 slot

void LoadArrowObjTiles(s8); // 按 bit7 选 2/4 块箭头图块, DMA3 装入 OBJ 图块槽 146

void Chara_SetGfxPal(u8, u8, u8);

void Chara_FreeSprite(u8);

void Chara_SetCmdPtr(u8, u8 *);

void Chara_StartMoving(u8);

u8 Chara_AnyMoving();

void Party_SetFollowMode();

void SetSlotGfxId(u8 slot, u16 gfxId); // 记 gSlotGfxId[slot]=gfxId 并置 PENDING_SPRITE_GFX

void SetSlotPalId(u8 slot, u16 palId); // 记 gSlotPalId[slot]=palId 并置 PENDING_SPRITE_PAL

u8 GetPendingSpriteLoad(); // 返回 gPendingSpriteLoad 位图

void Chara_SetPosDir(u8, s32, s32, u8);

u16 Chara_GetDrawZ(Actor *);

s16 Chara_GetDrawX(Actor *);

void Sprite_FreeChain(struct SpriteNode *);

SpriteNode *Sprite_WriteOam(u16 *, SpriteNode *);

void Chara_StartScriptAnim();

s32 Chara_AnimWaitDone(u8);

#endif // VRAM_TRANSFER_H
