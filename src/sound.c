
#include "sound.h"
#include "battle_types.h"
#include "battle_task_services.h"
#include "gba/gba.h"
#include "globals.h"
#include "include_asm.h"
#include "m4a.h"

extern struct MusicPlayerInfo *gMPlayInfos2[];
extern struct SongHeader *gSongHeaderTable[];

// @ 0x080533F0
void SoundMain_Frame(void)
{
    u16 volume;
    if (gSoundTaskFlags & 2)
    {
        volume = sub_801768C(gFadeFromVolume, 0x100, gFadeDuration, gFadeCounter, 0);
        Bgm_SetVolume(volume);
        if (gFadeCounter < gFadeDuration)
        {
            gFadeCounter++;
        }
        else
        {
            gSoundTaskFlags &= ~2;
        }
    }
    else if (gSoundTaskFlags & 4)
    {
        volume = sub_801768C(gFadeFromVolume, -gFadeFromVolume, gFadeDuration, gFadeCounter, 0);
        Bgm_SetVolume(volume);
        if (gFadeCounter < gFadeDuration)
        {
            gFadeCounter++;
        }
        else
        {
            gSoundTaskFlags &= ~4;
        }
    }

    m4aSoundMain();
    SoundTracks_Frame();
}

// @ 0x080534B4
void SoundTracks_Frame(void)
{
    u8 i;
    u16 songNum;

    for (i = 0; i <= 3; i++)
    {
        if ((gSfxTrackActiveBits >> i) & 1)
        {
            if ((u16)gMPlayInfos2[i]->status == 0)
            {
                gSfxTrackActiveBits &= ~(1 << i);
                if ((gSfxTrackLoopBits >> i) & 1)
                {
                    songNum = gSfxTrackSongIds[i];
                    MPlayStart(gMPlayInfos2[i], gSongHeaderTable[songNum]);
                    gSfxTrackActiveBits |= (1 << i);
                    gSfxTrackSongIds[i] = songNum;
                    if ((gSfxTrackLoopBits >> i) & 1)
                    {
                        gSfxTrackLoopBits &= ~(1 << i);
                    }
                    gSfxTrackLoopBits |= (1 << i);
                }
                if ((gSfxTrackFadeBits >> i) & 1)
                {
                    m4aMPlayFadeIn((struct MusicPlayerInfo *)0x03005AB0, 2);
                    gSfxTrackFadeBits &= ~(1 << i);
                }
            }
        }
    }
}

// @ 0x0805359C
void Sound_Init()
{
    m4aSoundInit();
    gSoundTaskFlags = 0;
    gPlayingSongId = 0;
    gBgmVolume = 0x100;
    gFadeDuration = 0;
    gSfxTrackActiveBits = 0;
    gSfxTrackLoopBits = 0;
    gSfxTrackFadeBits = 0;
}

// @ 0x080535E8
u16 Sound_GetFlags(void)
{
    return gSoundTaskFlags;
}

// @ 0x080535F4
void Sound_VSyncOff(void)
{
    m4aSoundVSyncOff();
    gSoundTaskFlags |= 1;
}

// @ 0x0805360C
void Sound_VSyncOn(void)
{
    m4aSoundVSyncOn();
    gSoundTaskFlags &= 0xFFFE;
}

// @ 0x08053628
void Bgm_Play(u8 arg0, u16 arg1)
{
    gPlayingSongId = arg0;
    switch (gPlayingSongId)
    {
        case 0x3F:
            gPlayingSongId = 0x11B;
            break;
        case 0x40:
            gPlayingSongId = 0x11C;
            break;
        case 0x41:
            gPlayingSongId = 0x11A;
            break;
    }
    m4aSongNumStart(gPlayingSongId);
    m4aMPlayImmInit((struct MusicPlayerInfo *)0x03005AB0);
    Bgm_SetVolume(arg1);
}

// @ 0x08053688
void Bgm_Stop(void)
{
    m4aSongNumStop(gPlayingSongId);
}

// @ 0x0805369C
void Bgm_SetVolume(u16 volume)
{

    if (volume < 2)
    {
        volume = 2;
    }
    *(u16 *)0x03000F3C = volume;
    m4aMPlayVolumeControl((struct MusicPlayerInfo *)0x03005AB0, 0xFF, volume);
}

// @ 0x080536C0
void Bgm_FadeIn(u8 arg0)
{
    gFadeFromVolume = 0;
    gFadeDuration = arg0;
    gFadeCounter = 0;
    gSoundTaskFlags |= 2;
}

// @ 0x080536EC
void Bgm_FadeOut(u8 arg0)
{
    gFadeFromVolume = gBgmVolume;
    gFadeDuration = arg0;
    gFadeCounter = 0;
    gSoundTaskFlags |= 4;
}

// @ 0x08053720
void Bgm_Continue(void)
{
    m4aSongNumContinue(gPlayingSongId);
}

// @ 0x08053734
u8 Sfx_TrackBusy(u8 arg0)
{
    return (gSfxTrackActiveBits >> arg0) & 1;
}

// @ 0x0805374C
void Sfx_Play(u16 arg0, u8 arg1, u8 arg2)
{
    MPlayStart(gMPlayInfos2[arg1], gSongHeaderTable[arg0]);
    gSfxTrackActiveBits |= 1 << arg1;
    gSfxTrackSongIds[arg1] = arg0;
    if ((gSfxTrackLoopBits >> arg1) & 1)
    {
        gSfxTrackLoopBits &= ~(1 << arg1);
    }
    gSfxTrackLoopBits |= arg2 << arg1;
}

// @ 0x080537C0
void Sfx_PlayFade(u16 arg0, u8 arg1)
{
    MPlayStart(gMPlayInfos2[arg1], gSongHeaderTable[arg0]);
    gSfxTrackActiveBits |= 1 << arg1;
    gSfxTrackSongIds[arg1] = arg0;
    if ((gSfxTrackFadeBits >> arg1) & 1)
    {
        gSfxTrackFadeBits &= ~(1 << arg1);
    }
    gSfxTrackFadeBits |= (1 << arg1);
    m4aMPlayFadeOutTemporarily((struct MusicPlayerInfo *)0x03005AB0, 2);
}

// @ 0x08053838
void Sfx_StopTrack(u8 arg0)
{
    m4aMPlayStop(gMPlayInfos2[arg0]);

    gSfxTrackActiveBits &= ~(1 << arg0);

    if ((gSfxTrackLoopBits >> arg0) & 1)
    {
        gSfxTrackLoopBits &= ~(1 << arg0);
    }
}

// @ 0x08053884
s32 Sfx_GetLoopFlag(u8 arg0)
{
    return (gSfxTrackLoopBits >> arg0) & 1;
}