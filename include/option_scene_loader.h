#ifndef OPTION_SCENE_LOADER_H
#define OPTION_SCENE_LOADER_H

#include "gba/types.h"
#include "iwram.h"
#include "map_scene_runtime.h"

void MapBg_LoadInterior(u8);
void Logo_LoadAssets(u8);
u32 ChoiceMenu_BuildList(void);
void BattleIntro_Cursor(void);
void ChoiceMenu_HandleInput(u16);

#endif // OPTION_SCENE_LOADER_H
