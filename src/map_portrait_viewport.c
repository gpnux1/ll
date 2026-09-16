#include "map_portrait_viewport.h"

#include "map_scene_runtime.h"
#include "battle_types.h"
#include "engine_core.h"
#include "map_view.h"
#include "player_stats.h"
#include "scene_mgr.h"
#include "script_vm.h"
#include "sound.h"
#include "sprite_engine.h"
#include "text_engine.h"
#include "vram_transfer.h"
#include "data_87E83F0.h"
#include "data_805769C.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"

// @ 0x08008620
void DialogPortrait_Set(u8 portraitId, u8 position)
{
    u16 *dst;
    u16 tile;
    u16 row;
    u16 offset;
    u16 col;

    if (portraitId != 0)
    {
        if (portraitId > 0x58)
            portraitId = 0;
        portraitId--;
        gPendingPortraitSlot = position + 1;
        gPendingPortraitGfx = (u8 *)gDialogPortraitGfxTable[portraitId];
        gPendingPortraitPalette = (u16 *)&gDialogPortraitPalettes[gDialogPortraitPaletteIds[portraitId] * 16];
        if (position & 2)
            tile = 0xF2C0;
        else
            tile = 0xE280;
        dst = (u16 *)gDialogPortraitTilemapPtrs[position];
        offset = 0;
        for (row = 0; row < 8; row++)
        {
            for (col = 0; col < 8; col++)
            {
                *dst++ = tile + offset;
                offset++;
            }
            dst += 0x18;
        }
        return;
    }

    dst = (u16 *)gDialogPortraitTilemapPtrs[position];
    for (row = 0; row < 8; row++)
    {
        for (col = 0; col < 8; col++)
            *dst++ = 0;
        dst += 0x18;
    }
    gPendingPortraitSlot = 0;
}

// @ 0x080086FC
void Viewport_UpdateEffects(void)
{
    if (gViewportFlags[0] & 1)
        gViewportFlags[1] = Rand_TableNext() & gViewportFlags[4];

    if (gViewportFlags[0] & 2)
        gViewportFlags[2] = Rand_TableNext() & gViewportFlags[4];

    if (gViewportFlags[0] & 4)
    {
        gViewportFlags[3]++;
        if (gViewportFlags[3] > 0xF)
            gViewportFlags[3] = 0;

        gBlendControl = 0x1C42;
        gBlendCoefficients = 0x0F00 | gViewportFlags[3];
        REG_DISPCNT |= DISPCNT_BG1_ON;
    }
}

extern const u16 gBgPalBackdropWhite[];
extern const u8 *gIntroBgTiles[]; // [id*2]=3KB tile组, [id*2+1]=可选 8-tile 动画组(NULL=无)
extern const u8 *gIntroBgMaps[]; // [id] LZ77 32x20 tilemap -> SBB 3 (0x0600E000)
extern const u16 gIntroBgPalettes[][0x20];

// @ 0x08008788
void IntroBg_Load(u8 arg0)
{
    gIntroBgTransferStage = 0;
    gIntroBgTileSetIndex = 0;
    gVBlankPipelineMode = 6;
    gObjGraphicsSetId = 0xFD;

    Scene_ResetResources();
    VBlankIntrWait();
    SoundMain_Frame();

    LZ77UnCompWram(gIntroBgTiles[arg0 * 2], (void *)0x02020000);
    gIntroBgTileSetIndex = 0;
    gIntroBgTransferStage = 1;
    VBlankIntrWait();
    SoundMain_Frame();

    if (gIntroBgTiles[arg0 * 2 + 1] != 0)
    {
        LZ77UnCompWram(gIntroBgTiles[arg0 * 2 + 1], (void *)0x02020000);
        gIntroBgTileSetIndex = 1;
        gIntroBgTransferStage = 1;
        VBlankIntrWait();
        SoundMain_Frame();
    }

    DmaCopy32(3, gIntroBgPalettes[arg0], PLTT, 0x40);
    DmaCopy16(3, gBgPalBackdropWhite, PLTT, 2);
    VBlankWait_PumpSound();

    LZ77UnCompWram(gIntroBgMaps[arg0], (void *)0x02020000);
    gIntroBgTileSetIndex = 0;
    gIntroBgTransferStage = 2;
    VBlankIntrWait();
    SoundMain_Frame();
    VBlankWait_PumpSound();

    REG_DISPCNT = 0x1960;
    REG_BG1CNT = 0;
    REG_BG2CNT = 0;
    REG_BG3CNT = 0x3C03;
}
