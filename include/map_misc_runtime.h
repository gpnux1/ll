#ifndef MAP_MISC_RUNTIME_H
#define MAP_MISC_RUNTIME_H

#include "gba/types.h"
#include "iwram.h"
#include "map_scene_runtime.h"

void ChoiceMenu_ResolveDest(u8);
void DialogPortrait_FlushPending(void);
u16 Camera_GetDrawOffset(void);
void Script_SetEnvSet(u8);
void BgPal_ResetFirst(void);
void AnimSlot_PlayOnce(u16, u8 *);
void BgMap_FillRow(u8);
void MapBg_FlushPending(void);

#endif // MAP_MISC_RUNTIME_H
