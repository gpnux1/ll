#ifndef ANIM_SLOT_CORE_H
#define ANIM_SLOT_CORE_H

#include "gba/types.h"
#include "iwram.h"
#include "map_scene_runtime.h"

u8 *AnimSlot_Parse(u16 slot, u8 *src);
u8 *AnimSlot_ParseLoop(u16 slot, u8 *src);
void AnimSlot_Step(s16 slot); // UpdateSpriteAnim
#define UpdateSpriteAnim AnimSlot_Step

#endif // ANIM_SLOT_CORE_H
