#ifndef SPRITE_BG_SHEET_LOADER_H
#define SPRITE_BG_SHEET_LOADER_H

#include "gba/types.h"
#include "iwram.h"
#include "map_scene_runtime.h"

void AnimSlots_Release(void);
void AnimSlots_StepAll(void);
void BgTiles_LoadUiSet(u8);
void BgScroll_LoadFromTable(u16);
void PlayerSheets_Load(void);
void AnimSlot_LoadSet(u8, u8);
#define LoadSpriteAnimSet AnimSlot_LoadSet
void AnimSlot_Pause(u8);
void AnimSlot_Resume(u8);
u8 AnimSlot_Active(u8);
void ReloadSpriteSheet(u8 slot);
void ReloadAllSpriteSheets(void);

#endif // SPRITE_BG_SHEET_LOADER_H
