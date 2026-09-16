#ifndef SCREEN_FX_LOADER_H
#define SCREEN_FX_LOADER_H

#include "gba/types.h"
#include "iwram.h"
#include "map_scene_runtime.h"

void ScreenFade_Start(u16, s16, s16);
void AnimSlot_BankReload(void);
void Win0H_WaveDmaByVCount(void);
void ScreenFx_SetMode(u16);

#endif // SCREEN_FX_LOADER_H
