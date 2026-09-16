#ifndef MAP_PORTRAIT_VIEWPORT_H
#define MAP_PORTRAIT_VIEWPORT_H

#include "gba/types.h"
#include "iwram.h"
#include "map_scene_runtime.h"

void DialogPortrait_Set(u8 portraitId, u8 position);
void Viewport_UpdateEffects(void);
void IntroBg_Load(u8);

#endif // MAP_PORTRAIT_VIEWPORT_H
