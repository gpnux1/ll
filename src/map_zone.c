#include "map_zone.h"

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

// @ 0x08007ADC
s32 sub_8007ADC(s16 arg0, s16 arg1)
{
    u16 i;
    s16 sx;
    s16 sy;
    u8 *cells;
    u8 count;
    int mask;

    for (i = 1; i <= 3; i++)
    {
        gZoneCheckTileXs[i] = gZoneCheckTileXs[i] | 0xFF;
        gZoneCheckTileYs[i] = gZoneCheckTileYs[i] | 0xFF;
    }

    sx = arg0;
    gZoneCheckTileXs[0] = sx >> 4;
    sy = arg1;
    gZoneCheckTileYs[0] = sy >> 4;

    mask = 0xF;
    if ((u16)sx & mask)
    {
        gZoneCheckTileXs[1] = (sx >> 4) + 1;
        gZoneCheckTileYs[1] = sy >> 4;
        if ((sy & mask) > 8)
        {
            gZoneCheckTileXs[2] = sx >> 4;
            gZoneCheckTileYs[2] = (sy >> 4) + 1;
            gZoneCheckTileXs[3] = (sx >> 4) + 1;
            gZoneCheckTileYs[3] = (sy >> 4) + 1;
        }
    }
    else if ((sy & mask) > 8)
    {
        gZoneCheckTileXs[2] = sx >> 4;
        gZoneCheckTileYs[2] = (sy >> 4) + 1;
    }

    cells = (u8 *)*gMapZoneHeader;
    count = *cells++;
    while (count != 0)
    {
        for (i = 0; i <= 3; i++)
        {
            if (gZoneCheckTileXs[i] != 0xFF && gZoneCheckTileXs[i] == cells[0] && gZoneCheckTileYs[i] == cells[1])
            {
                gMapZoneType = cells[2];
                gMapZoneEntryIdx = cells[3];
                return 1;
            }
        }
        cells += 4;
        count--;
    }

    return 0;
}
/* 按 gMapZoneType 分发命中区域的触发动作 (记录表 = gMapZoneHeader[type+1], 记录下标 gMapZoneEntryIdx):
 * 0=换图: 装载点 5 字段 + state 3 + 清开关位图; 1=图内传送: 4 字段 + state 4;
 * 2=state 8 (byte 0x47BC/0x47E0); 3=开关未置则跑脚本(2B 记录)返回 1; 4=A 键+朝向门控跑脚本(4B 记录)返回 0;
 * type>4 (含 0xFF 未命中) 返回 1。 */
// @ 0x08007BD0
s32 MapZone_Trigger(void)
{
    u32 *header = gMapZoneHeader;
    u8 type = gMapZoneType;
    u32 ofs = type * 4 + 4;
    u8 *rec = (u8 *)*(u32 *)((u8 *)header + ofs);
    u8 scriptId;
    u8 dir;

    switch (type)
    {
        case 0:
            rec += gMapZoneEntryIdx * 8;
            gMapNpcSetId = *rec;
            rec++;
            gSpawnTileX = *rec;
            rec++;
            gSpawnTileY = *rec;
            rec++;
            gSpawnFacingDir = *rec;
            rec++;
            gMoveCmdSetId = rec[0] + (rec[1] << 8);
            gGameState = GAME_STATE_SCENE_REQUEST_MAP;
            SwitchFlags_ClearRange();
            return 1;
        case 1:
            rec += gMapZoneEntryIdx * 8;
            gSpawnTileX = *rec;
            rec++;
            gSpawnTileY = *rec;
            rec++;
            gSpawnFacingDir = *rec;
            rec++;
            gMoveCmdSetId = rec[0] + (rec[1] << 8);
            gGameState = GAME_STATE_SCENE_ENTER_MAP;
            return 1;
        case 2:
            rec += gMapZoneEntryIdx * 4;
            gChoiceGroupIdx = rec[0];
            gChoiceSubIdx = rec[1];
            gGameState = GAME_STATE_ENTER_DOOR;
            SwitchFlags_ClearRange();
            return 1;
        case 3:
            rec += gMapZoneEntryIdx * 2;
            scriptId = *rec;
            if (SwitchFlags_Test(rec[1]) != 0)
                return 0;
            ScriptPump_JumpToEntry(scriptId, 2);
            return 1;
        case 4:
            rec += gMapZoneEntryIdx * 4;
            scriptId = *rec;
            rec++;
            if ((gNewKeysRaw & 1) == 0)
                return 0;
            dir = rec[1];
            if (dir <= 7 && dir != gPlayerMoveDir)
                return 0;
            if (SwitchFlags_Test(rec[0]) != 0)
                return 0;
            ScriptPump_JumpToEntry(scriptId, 2);
            return 0;
        default:
            return 1;
    }
}
