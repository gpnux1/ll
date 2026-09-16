#include "chest_objects.h"

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

// @ 0x08008F28
void ChestObjects_LoadForMap(u8 mapId)
{
    u8 slot;
    u8 recordIndex;
    const ChestSpawnEntry *entry;
    ChestObject *chest;
    ChestObject *chestBase;
    u8 flags;

    slot = 0;
    recordIndex = 0;
    entry = gChestSpawnTable;
    while (1)
    {
        if (mapId == entry->mapId)
        {
            chestBase = gChestObjects;
            chest = &chestBase[slot];
            chest->mapEntryIndex = recordIndex;
            chest->x = entry->tileX << 3;
            chest->y = (entry->tileY << 3) + 8;
            chest->interactionId = entry->itemId;
            flags = gChestFlags[recordIndex >> 3];
            chest->flags = (flags >> (recordIndex & 7)) & 1;
            if (entry->specialFlag != 0)
                chest->flags |= 0x80;
            ChestObject_BuildSprite(slot);
            slot++;
        }

        entry++;
        if (recordIndex == 0xFF)
            break;
        recordIndex++;
        if (slot > 0xF)
            break;
    }

    while (slot <= 0xF)
    {
        gChestObjects[slot].flags |= 0xFF;
        gChestObjects[slot].spriteNodeIdx = 0;
        slot++;
    }
}
// @ 0x08008FD0
void ChestObject_BuildSprite(u8 chestIdx)
{
    struct SpriteNode *sprNode;
    struct SpriteNode *sprSubNode;
    u8 objIdx;
    u16 chestColor;
    u16 attr0;
    u16 attr1;
    u16 attr2;

    objIdx = Sprite_AllocNode();
    gChestObjects[chestIdx].spriteNodeIdx = objIdx;
    sprNode = &gSpriteNodePool[objIdx];

    chestColor = 0x80 & gChestObjects[chestIdx].flags ? 0xF : 0xE;

    if ((0x7F & gChestObjects[chestIdx].flags) == 0)
    {

        attr0 = 0;
        attr1 = 0x4000;
        attr2 = ((chestColor << 12) | 0x896);

        sprNode->tileOffsetX = 0;
        sprNode->tileOffsetY = 0xF0;

        Sprite_InitChainNode(sprNode, 1, attr0, attr1, attr2);
    }
    else
    {
        attr0 = 0;
        attr1 = 0x4000;
        attr2 = ((chestColor << 12) + 0x89C);
        sprNode->tileOffsetX = 0;
        sprNode->tileOffsetY = 0xF0;
        sprSubNode = Sprite_InitChainNode(sprNode, 2, attr0, attr1, attr2);

        attr0 = 0x4000;
        attr1 = 0;
        attr2 = ((chestColor << 12) | 0x89A);
        sprSubNode->tileOffsetX = 0;
        sprSubNode->tileOffsetY = 0xE8;
        Sprite_InitChainNode(sprSubNode, 1, attr0, attr1, attr2);
    }
    sprNode->animStep = 0;
}
