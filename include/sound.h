#ifndef _SOUND_H
#define _SOUND_H

#include "gba/types.h"
#include "iwram.h"
#include "battle_types.h"

void SoundMain_Frame();
void SoundTracks_Frame();
void Sound_Init();
u16 Sound_GetFlags();
void Sound_VSyncOff();
void Sound_VSyncOn();
void Bgm_Play(u8, u16);
void Bgm_Stop();
void Bgm_SetVolume(u16);
void Bgm_FadeIn(u8);
void Bgm_FadeOut(u8);
void Bgm_Continue();
u8 Sfx_TrackBusy(u8);
void Sfx_Play(u16, u8, u8);
void Sfx_PlayFade(u16, u8);
void Sfx_StopTrack(u8);
s32 Sfx_GetLoopFlag(u8);

#endif