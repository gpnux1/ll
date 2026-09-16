#include "battle_types.h"
#include "sprite_engine.h"
#include "map_scene_runtime.h"
#include "data_87E83F0.h"
#include "engine_core.h"
#include "map_view.h"
#include "menu.h"
#include "menu_ui.h"
#include "player_stats.h"
#include "sound.h"
#include "vram_transfer.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "globals.h"
#include "data_805769C.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"

extern void IntrMain();

// @ 0x08002154
void Sprites_UpdateFrame(void)
{
    u16 i;
    u16 ret0;
    Actor *ptr03002E80;
    ChestObject *chestObject;

    Viewport_UpdateScroll();
    AnimSlots_StepAll();
    sub_80053B4(gCameraPosX, gCameraPosY);

    if (gObjGraphicsSetId != 0xFF)
    {
        if ((gObjGraphicsSetId & 0x80) != 0 && gObjGraphicsSetId < 0xFE)
        {
            i = 0;
            ptr03002E80 = gActors;
            do
            // for(; i < 19; i++)
            {
                if (ptr03002E80->sprNodeIdx && (ptr03002E80->renderFlags & 1) != 0)
                {
                    ret0 = Chara_StepMove(i);
                    if (ret0 == 1 && ptr03002E80->cmdStream)
                    {
                        ptr03002E80->stepTimer++;
                        ret0 = 2;
                    }
                    if (ret0 <= 1)
                    {
                        Chara_ProcessCmdStream(i);
                    }
                    Sprite_UpdateCharaAnim(i);
                    Sprite_EnqueueRender(Chara_GetDrawX(ptr03002E80), ptr03002E80->y, ptr03002E80->sprNodeIdx,
                                         Chara_GetDrawZ(ptr03002E80), ptr03002E80->renderFlags);
                    if (ptr03002E80->subSprNodeIdx)
                    {
                        gSpriteNodePool[ptr03002E80->subSprNodeIdx].flags = 0;
                        gSpriteNodePool[ptr03002E80->subSprNodeIdx].next = 0;
                        ptr03002E80->subSprNodeIdx = 0;
                    }
                    Sprite_SetupDialogArrow(i);
                }
                ptr03002E80++;
                i++;
            } while (i < 19);
        }
        else
        {
            Party_FollowStep();
            i = 2;
            ptr03002E80 = &gActors[i];
            do
            // for( ; i < 19; i++)
            {
                if (ptr03002E80->sprNodeIdx)
                {
                    if (!gUnk_03004D4C && (!gWarpAnimState || gWarpAnimState == 9) && (ptr03002E80->stateFlags & 0x88) != 8)
                    {
                        ret0 = Chara_StepMove(i);
                        if (ret0 == 1 && ptr03002E80->cmdStream)
                        {
                            ptr03002E80->stepTimer++;
                            ret0 = 2;
                        }
                        if (ret0 <= 1)
                        {
                            Chara_ProcessCmdStream(i);
                        }
                    }
                    Sprite_UpdateCharaAnim(i);
                    if (ptr03002E80->subSprNodeIdx)
                    {
                        gSpriteNodePool[ptr03002E80->subSprNodeIdx].flags = 0;
                        gSpriteNodePool[ptr03002E80->subSprNodeIdx].next = 0;
                        ptr03002E80->subSprNodeIdx = 0;
                    }
                    if (Sprite_EnqueueRender(ptr03002E80->x, ptr03002E80->y, ptr03002E80->sprNodeIdx, ptr03002E80->z,
                                             ptr03002E80->renderFlags))
                    {
                        ptr03002E80->stateFlags |= 8;
                    }
                    else
                    {
                        ptr03002E80->stateFlags &= ~8;
                        Sprite_SetupDialogArrow(i);
                    }
                }
                ptr03002E80++;
                i++;
            } while (i < 19);
        }

        for (i = 0; i < 16; i++)
        {
            if (gChestObjects[i].spriteNodeIdx)
            {
                Sprite_EnqueueRender(gChestObjects[i].x, gChestObjects[i].y, gChestObjects[i].spriteNodeIdx, 0, 255);
            }
        }
    }

    StaticObjs_StepAll();
    OAM_FlushFromQueue();
    PaletteEffects_Update();
}

// @ 0x08002380
void Sprite_SetupDialogArrow(u8 charaId)
{
    Actor *charaObj;
    SpriteNode *sprNode;

    if ((gFrameCounter & 1) == (charaId & 1))
    {
        charaObj = &gActors[charaId];
        if (!(charaObj->renderFlags & 2))
        {
            if (charaObj->subSprNodeIdx == 0)
            {
                charaObj->subSprNodeIdx = Sprite_AllocNode();
            }
            sprNode = &gSpriteNodePool[charaObj->subSprNodeIdx];

            if ((charaObj->renderFlags & 1) != 0)
            {
                sprNode->attr0 = 0x4000;
                sprNode->attr1 = 0x4000;
                sprNode->attr2 = 0x892;
                sprNode->tileOffsetX = 0x1F4;
                sprNode->tileOffsetY = 0xFC;
            }
            else
            {
                sprNode->attr0 = 0x4000;
                sprNode->attr1 = 0;
                sprNode->attr2 = 0x892;
                sprNode->tileOffsetX = 0;
                sprNode->tileOffsetY = 0xFA;
            }
            sprNode->flags = 1;
            sprNode->animStep = 0;

            Sprite_EnqueueRender(Chara_GetDrawX(charaObj), charaObj->y - 0xA0, charaObj->subSprNodeIdx, -0xA0, 0xFE);
        }
    }
}
// @ 0x0800243C
INCLUDE_ASM("asm/matchings", Sprite_EnqueueRender);

/*
// @ 0x0800243C
u8 Sprite_EnqueueRender(s16 x, s16 y, u8 sprNodeIdx, s16 z, u8 arg4)
{
    SpriteNode* sprNode;
    u8 count;

    s16 screenX, screenY;
    s16 offsetX, offsetY;
    u8 flag;
    s16 i;

    flag = 0;

    sprNode = &gSpriteNodePool[sprNodeIdx];
    sprNode->x = x;
    sprNode->y = y;

    count = sprNode->flags & 0x7F;

    if(!(arg4 & 0x81) && (sprNode->animStep == 0 || sprNode->animStep == 2))
    {
        z++;
    }

    if (gViewportFlags[0] & 1) {
        screenX = x - ((gCameraPosX & ~0xF) + gBG3ScrollX);
    } else {
        screenX = x - gCameraPosX;
    }

    if (gViewportFlags[0] & 2)
    {
        screenY = y - ((gCameraPosY & ~0xF) + gBG3ScrollY) - 4 - z;
    } else {
        screenY = y - gCameraPosY - 4 - z;
    }

    while(count != 0)
    {
        gCurSpriteW = gWalkAnimDimTable[ ((sprNode->attr0 >> 11) & 0x18) + ((sprNode->attr1 >> 13) & 6)];
        gCurSpriteH = gWalkAnimDimTable[ ((sprNode->attr0 >> 11) & 0x18) + ((sprNode->attr1 >> 13) & 6) + 1];

        if(sprNode->tileOffsetX > 256)
        {
            offsetX = sprNode->tileOffsetX - 511;
        }
        else
        {
            offsetX = sprNode->tileOffsetX;
        }

        if(sprNode->tileOffsetY > 128)
        {
            offsetY = sprNode->tileOffsetY - 255;
        }
        else
        {
            offsetY = sprNode->tileOffsetY;
        }

        if((x + gCurSpriteW + offsetX) >= gCameraPosX && (x +  offsetX) <= gCameraPosX + 240
              && (y + gCurSpriteH + offsetY - z - 4) >= gCameraPosY && (y +  offsetY - z - 4) <= gCameraPosY + 160)
        {
            sprNode->flags &= ~0x80;
            sprNode->attr0 = (sprNode->attr0 & 0xFF00) + ((screenY + offsetY) & 0xFF);
            sprNode->attr1 = (sprNode->attr1 & 0xFE00) + ((screenX + offsetX) & 0x1FF);
            flag = 1;
        }
        else
        {
            sprNode->flags |= 0x80;
        }

        sprNode = sprNode->next;

        count--;
    }

       if(flag != 0)
    {
        SpriteNode* r5;
        SpriteNode* r1;
        SpriteNode* r2;
        i = 0;

        while(i < 128)
        {
            r1 = gSpriteRenderQueue[i];

            if(r1 == 0)
            {
                gSpriteRenderQueue[i] = &gSpriteNodePool[sprNodeIdx];
                break;
            }

            if((u16)r1->y < y)
            {
                s16 j;
                r2 = r1;
                gSpriteRenderQueue[i] = &gSpriteNodePool[sprNodeIdx];
                // i++;

                j = i + 1;
                while(j < 128)
                {
                    r1 = gSpriteRenderQueue[j];
                    gSpriteRenderQueue[j] = r2;
                    r2 = r1;
                    if(r2 == 0)
                        break;
                    j++;
                }

                return 0;
            }

            i++;
        }
        return 0;
    }
    return 1;
}
*/

/*

// @ 0x0800243C
u8 Sprite_EnqueueRender(s16 x, s16 y, u8 arg2, s16 z, u8 arg4) {
    SpriteNode* sprNode;
    u8 num;
    s16 screenX, screenY;
    s16 offsetX, offsetY;
    u8 updateFlag;
    s16 i;
    s16 j;
    updateFlag = 0;

    sprNode = &gSpriteNodePool[arg2];
    sprNode->x = x;
    sprNode->y = y;

    num = sprNode->flags & 0x7F;

    if(!(arg4 & 0x81) && (sprNode->animStep == 0 || sprNode->animStep == 2))
    {
        z++;
    }

    if (gViewportFlags[0] & 1) {
        screenX = x - ((gCameraPosX & ~0xF) + gBG3ScrollX);
    } else {
        screenX = x - gCameraPosX;
    }

    if (gViewportFlags[0] & 2)
    {
        screenY = y - ((gCameraPosY & ~0xF) + gBG3ScrollY) - 4 - z;
    } else {
        screenY = y - gCameraPosY - 4 - z;
    }

    while(num != 0)
    {
        gCurSpriteW = gWalkAnimDimTable[ ((sprNode->attr0 >> 11) & 0x18) + ((sprNode->attr1 >> 13) & 6)];
        gCurSpriteH = gWalkAnimDimTable[ ((sprNode->attr0 >> 11) & 0x18) + ((sprNode->attr1 >> 13) & 6) + 1];

        if(sprNode->tileOffsetX > 256)
        {
            offsetX = sprNode->tileOffsetX - 511;
        }
        else
        {
            offsetX = sprNode->tileOffsetX;
        }

        if(sprNode->tileOffsetY > 128)
        {
            offsetY = sprNode->tileOffsetY - 255;
        }
        else
        {
            offsetY = sprNode->tileOffsetY;
        }

        if((x + gCurSpriteW + offsetX) >= gCameraPosX && (x +  offsetX) <= gCameraPosX + 240
              && (y + gCurSpriteH + offsetY - z - 4) >= gCameraPosY && (y +  offsetY - z - 4) <= gCameraPosY + 160)
        {
            sprNode->flags &= ~0x80;
            sprNode->attr0 = (sprNode->attr0 & 0xFF00) + ((screenY + offsetY) & 0xFF);
            sprNode->attr1 = (sprNode->attr1 & 0xFE00) + ((screenX + offsetX) & 0x1FF);
            updateFlag = 1;
        }
        else
        {
            sprNode->flags |= 0x80;
        }

        sprNode = sprNode->next;

        num--;
    }

    if(updateFlag != 0)
    {
        struct SpriteNode* r1;
        struct SpriteNode* r2;
        i = 0;

        while(i < 128)
        {
            r1 = gSpriteRenderQueue[i];

            if(r1 == 0)
            {
                gSpriteRenderQueue[i] = &gSpriteNodePool[arg2];
                break;
            }

            if((u16)r1->y < y)
            {
                r2 = r1;
                gSpriteRenderQueue[i] = &gSpriteNodePool[arg2];
                // i++;

                j = (u16)(i + 1);
                while(j < 128)
                {
                    r1 = gSpriteRenderQueue[j];
                    gSpriteRenderQueue[j] = r2;
                    r2 = r1;
                    if(r2 == 0)
                        break;
                    j++;
                }

                return 0;
            }

            i++;
        }
        return 0;
    }
    return 1;
}
*/

// @ 0x0800271C
INCLUDE_ASM("asm/matchings", Sprite_UpdateCharaAnim);
/*

//这些数据在原始rom rodata中的位置是在 tileset_3_080583C4 后面
u8 gWalkAnimFrameMapping[] = {0, 1, 2, 1, 0, 1, 0, 1};
u8 gWalkDirectionMapping[] = {2, 3, 3, 3, 0, 1, 1, 1, 1, 2, 2, 2, 3, 0, 0, 0, 0};

extern SpriteNode gSpriteNodePool[128];

extern SpriteNode* gSpriteRenderQueue[128];

extern Actor gActors[];

// @ 0x0800271C
void Sprite_UpdateCharaAnim(u8 arg0)
{
    Actor *charaObj;
    SpriteNode* sprNode;
    u16 paletteBits;
    u8 directionIndex;
    u8 temp_r7;
    u32 currentFrameTileOffset;

    charaObj = &gActors[arg0];

    if ((charaObj->field_1 & 1) == 0)
    {
        if(!(charaObj->field_12 & 4))
        {
            if(charaObj->field_11 == 0)
            {
                charaObj->walkAnimCounter++;
            }
        }

        charaObj->walkAnimCounter &= 0x1F;

        paletteBits = charaObj->paletteIdx ;
        paletteBits <<= 12;

        sprNode = &gSpriteNodePool[charaObj->sprNodeIdx];

        if ((charaObj->field_1 & 0x7C) == 0)
        {
            sprNode->animStep = charaObj->walkAnimCounter >> 3;

            currentFrameTileOffset = gWalkAnimFrameMapping[sprNode->animStep] * 6;

            directionIndex = gWalkDirectionMapping[charaObj->facing];
            temp_r7 = directionIndex;

            if (temp_r7 == 3)
            {
                directionIndex = 1;
                sprNode->attr1 |= 0x1000;
            }
            else
            {
                sprNode->attr1 &= ~0x1000;
            }
            sprNode->attr2 = ((sprNode->attr2 & 0xC00) | ((directionIndex*18 + currentFrameTileOffset + charaObj->vramSlotIdx * 72 +
0xA0) & 0x3FF)) | paletteBits;

            sprNode = sprNode->next;
            if(sprNode)
            {
                if(temp_r7 == 3)
                {
                    sprNode->attr1 |= 0x1000;
                }
                else
                {
                    sprNode->attr1 &= ~0x1000;
                }
                sprNode->attr2 = ((sprNode->attr2 & 0xC00) | ((directionIndex*18 + currentFrameTileOffset + charaObj->vramSlotIdx * 72
+ 0xA4) & 0x3FF)) | paletteBits;
            }

        }
        else if ((charaObj->field_1 & 4) == 0)
        {
            if ((charaObj->field_1 & 0x10) == 0)
            {
                if ((charaObj->field_1 & 0x20) )
                {
                    if (charaObj->facing != 0)
                    {
                        sprNode->attr2 = (sprNode->attr2 & 0xFC00) + ((charaObj->vramSlotIdx * 72 + 0xB0) & 0x3FF);
                    }
                    else
                    {
                        sprNode->attr2 = (sprNode->attr2 & 0xFC00) + ((charaObj->vramSlotIdx * 72 + 0xC0) & 0x3FF);
                    }

                    sprNode = sprNode->next;
                    sprNode->attr2 = (sprNode->attr2 & 0xFC00) + ((charaObj->vramSlotIdx * 72 + 0xA0) & 0x3FF);
                }
            }
            else
            {
                sprNode->animStep = charaObj->walkAnimCounter >> 3;
                directionIndex = gWalkDirectionMapping[charaObj->facing];

                if (directionIndex == 1)
                {
                    sprNode->attr1 |= 0x1000;
                }
                else
                {
                    sprNode->attr1 &= ~0x1000;
                }
            sprNode->attr2 = ((sprNode->attr2 & 0xC00) | ((charaObj->vramSlotIdx * 72 + 0xA0) & 0x3FF)) | paletteBits;
            }
        }
        else
        {
            if ((charaObj->field_1 & 8) == 0)
            {

                sprNode->animStep = 0;
                temp_r7 =(charaObj->walkAnimCounter >> 3) & 3;

                directionIndex = gWalkDirectionMapping[charaObj->facing + 8];

                if (directionIndex & 1)
                {
                    currentFrameTileOffset = 0x20;
                }
                else
                {
                    currentFrameTileOffset = gWalkAnimFrameMapping[temp_r7 + 4] << 4;
                }

                if (directionIndex & 2)
                {
                    sprNode->attr1 |= 0x1000;
                }
                else
                {
                    sprNode->attr1 &= ~0x1000;
                }

            // sprNode->attr2 = ((sprNode->attr2 & 0xC00) | ((chara->field_2 * 72 +currentFrameTileOffset + 0xA0) & 0x3FF)) |
paletteBits;
            }
            else
            {
                sprNode->animStep = 0;
                temp_r7 = (charaObj->walkAnimCounter >> 3) & 3;
                currentFrameTileOffset = gWalkAnimFrameMapping[temp_r7] * 16;
                sprNode->attr1 &= ~0x1000;
            }
            sprNode->attr2 = ((sprNode->attr2 & 0xC00) | ((charaObj->vramSlotIdx * 72 +currentFrameTileOffset + 0xA0) & 0x3FF)) |
paletteBits;

        }
        return;
    }

    Anim_PlayCustom(arg0);
}

asm(".align 2,0");

*/

// @ 0x080029D8
INCLUDE_ASM("asm/matchings", Anim_PlayCustom);

/*

extern u8* gCutsceneAnimPals[];
extern u8 gCutsceneAnimSlots[];
extern u8 gCutsceneAnimFlags[];

extern u8* gCutsceneAnimScripts[];

u8 PalTransfer_AllocSlot();

void Anim_BuildOamChain(u8 arg0, u8 *arg1);

void PalTransfer_Enqueue(u8 , void* , u8 , u8 );

// @ 0x080029D8
void Anim_PlayCustom(u8 arg0) {
    Actor* chara;
    u8* animDataPtr;
    u8* frameDataPtr;
    u16 offset;

    chara = &gActors[arg0];

    if(chara->currAnimIdx == 0xFF)

        return;

    animDataPtr = gCutsceneAnimScripts[chara->currAnimIdx];

    if(chara->animFrameTimer == 0xFF)
        return;

    // offset = animDataPtr[2] + (animDataPtr[3] << 8);

    frameDataPtr = animDataPtr +  (animDataPtr[2] + (animDataPtr[3] << 8));

    if(chara->animFrameTimer == 0)
    {
        if( animDataPtr[6] + (animDataPtr[7] << 8) == 2)
        {
            PalTransfer_Enqueue(PalTransfer_AllocSlot(), gCutsceneAnimPals[chara->currAnimIdx], gCutsceneAnimSlots[chara->currAnimIdx]
+ 16, 2);
        }
        else
        {
            PalTransfer_Enqueue(PalTransfer_AllocSlot(), gCutsceneAnimPals[chara->currAnimIdx], gCutsceneAnimSlots[chara->currAnimIdx]
+ 16, 0);

        }
        frameDataPtr += 8;

        offset = (frameDataPtr[0] + (frameDataPtr[1] << 8)) << 1;
        // unkPtr = &animDataPtr[offset];

        Anim_BuildOamChain(arg0, (animDataPtr + animDataPtr[offset + 8] + (animDataPtr[offset + 9]<<8)) + 4);
        chara->animFrameTimer++;
    }
    else
    {
        u16 time = frameDataPtr[2] + (frameDataPtr[3] << 8);
        u16 count = frameDataPtr[6] + (frameDataPtr[7] << 8);

        frameDataPtr += 8;

        if(chara->animFrameTimer >= time)
        {
            if((gCutsceneAnimFlags[chara->currAnimIdx] & 0x80) == 0)
            {
                chara->animFrameTimer = 0xFF;
                return;
            }
            else
            {
                chara->animFrameTimer = 0;
            }
        }

        while(count != 0)
        {
            if(chara->animFrameTimer == frameDataPtr[2] + (frameDataPtr[3] << 8) || chara->animFrameTimer == 0)
            {
                Sprite_FreeChain(&gRenderObjects[chara->sprNodeIdx]);
                if(chara->animFrameTimer == 0)
                {
                    offset = (frameDataPtr[0] + (frameDataPtr[1] << 8)) << 1;
                }
                else
                {
                    offset = (frameDataPtr[4] + (frameDataPtr[5] << 8)) << 1;
                }
                // offset = offset << 1;
                // unkPtr = animDataPtr + offset;
                // unkPtr = animDataPtr + unkPtr[8] + (unkPtr[9] << 8);

                Anim_BuildOamChain(arg0, (animDataPtr + animDataPtr[offset+8] + (animDataPtr[offset+9]<<8)) + 4);
                // Anim_BuildOamChain(arg0,  unkPtr + 4);
                break;

            }
            frameDataPtr+=4;
            count--;
        }

        if(chara->animFrameTimer != 0xFF)
        {
            chara->animFrameTimer++;
        }

    }

}
*/

// @ 0x08002B54
INCLUDE_ASM("asm/matchings", Anim_BuildOamChain);

/*
extern u8 gCutsceneAnimSlots[];
extern s8 gSpriteTileCountTable[];

extern u32 gVramBufferPointers[];
extern s8 gWalkAnimDimTable[];

extern u8 gSpriteWidth;
extern u8 gSpriteHeight;

const u8 gSpriteTileCount[4][4] = {
    // Size:   0,  1,  2,  3
    [0] = {    1,  4, 16, 64 }, // Shape 0: Square (8x8, 16x16, 32x32, 64x64)
    [1] = {    2,  4,  8, 32 }, // Shape 1: Horizontal (16x8, 32x8, 32x16, 64x32)
    [2] = {    2,  4,  8, 32 }, // Shape 2: Vertical (8x16, 8x32, 16x32, 32x64)
    [3] = {   99, 99, 99, 99 }  // Shape 3: Prohibited/Invalid
};

// 命名为：Sprite占用Tile数量查找表
const u8 gSpriteTileCountTable[] = {
    1, 4, 16, 64,  // Square: 8x8, 16x16, 32x32, 64x64
    2, 4,  8, 32,  // Horizontal: 16x8, 32x8, 32x16, 64x32
    2, 4,  8, 32,  // Vertical: 8x16, 8x32, 16x32, 32x64
    99, 99, 99, 99 // Invalid/Prohibited Shape
};

const u8 gSpriteDimensionsTable[] = {
    8, 8, 16, 16, 32, 32, 64, 64, // Shape 0: Square
    16, 8, 32, 8, 32, 16, 64, 32, // Shape 1: Horizontal
    8, 16, 8, 32, 16, 32, 32, 64, // Shape 2: Vertical
    0, 0, 0, 0, 0, 0, 0, 0        // Shape 3: Invalid
};

void VramTransfer_Enqueue(u16 id, void* src, void* dest, u8 arg3);

//00 00   idx
//02 00   num

//DF 80 F4 81 00 00 DF 80 04 40 08 00
//SpriteOamAttr[num]

// @ 0x08002B54
void Anim_BuildOamChain(u8 arg0, u8 *arg1) {
    Unk_03002E80 *ptr2E80;
    struct SpriteNode *renderObj;
    struct SpriteNode *subObj;

    u16 vramOffset;
    u8 oamDataOffset;
    u16 val_r8;
    u8 num;
    u8* oamDataPtr;

    u8 tileCount;
    u16 dmaSlotId;
    u8* src;
    u8* dest;

    u8 val_r4_1;

    u16 attr0;
    u16 attr1;
    u16 attr2;
    s16 x;

    ptr2E80 = &gActors[arg0];

    if(arg0 <= 0x12)
    {
        vramOffset = 0x800;
        if (gGameState == GAME_STATE_TITLE_MENU)
        {
            vramOffset = 0x400;
        }
    }
    else
    {
        vramOffset = 0;
    }

    oamDataOffset = arg1[0] * 6 + 4;
    num = arg1[2];

    ptr2E80->sprNodeIdx = Sprite_AllocNode();
    renderObj = &gSpriteNodePool[ptr2E80->sprNodeIdx];

    val_r8 = gCutsceneAnimSlots[ptr2E80->field_16] * 72 + 160;

    oamDataPtr = &arg1[oamDataOffset];

    while(num != 0)
    {

        tileCount = gSpriteTileCountTable[(oamDataPtr[1] >> 6) * 4 + (oamDataPtr[3] >> 6)];

        val_r4_1 = oamDataPtr[4];
        dmaSlotId = VramTransfer_AllocSlot();

        src = (u8*) gVramBufferPointers[ptr2E80->field_16] + (val_r4_1 << 5);
        dest = (u8*)0x06010000 + (val_r8 << 5);
        VramTransfer_Enqueue(dmaSlotId, src, dest, tileCount);

        attr0 = oamDataPtr[0] | (oamDataPtr[1] << 8);

        if ((gCutsceneAnimFlags[ptr2E80->field_16] & 0x40) != 0)
        {
            attr1 = (oamDataPtr[2] + (oamDataPtr[3] << 8)) | 0x1000;
        }
        else
        {
            attr1 = (oamDataPtr[2] + (oamDataPtr[3] << 8));
            renderObj->field_10 = attr1 & 0x1FF;
        }
        renderObj->field_12 = attr0 & 0xFF;

        attr2 = ((oamDataPtr[5] << 8) & ~0xFFF) + (vramOffset +(gCutsceneAnimSlots[ptr2E80->field_16] << 12) +  val_r8);

        subObj = Sprite_InitChainNode(renderObj, num, attr0, attr1, attr2);

        if ((gCutsceneAnimFlags[ptr2E80->field_16] & 0x40) != 0)
        {

            gSpriteWidth = gSpriteDimensionsTable[ ((renderObj->attr0 >> 11) & 0x18) + ((renderObj->attr1 >> 13) & 6)];

            gSpriteHeight = gSpriteDimensionsTable[ ((renderObj->attr0 >> 11) & 0x18) + ((renderObj->attr1 >> 13) & 6) + 1];

            x = (~(attr1 & 0x1FF) - gSpriteWidth) & 0x1FF;
            renderObj->field_10 = x;
        }
        if (renderObj->field_10 > 0xFF)
        {
            renderObj->field_10--;
        }
        renderObj = subObj;

        oamDataPtr += 6;
        val_r8 += tileCount;

        num--;
    }
}

*/

// UpdateEncounter
// @ 0x08002D54
u8 CheckEncounter(void)
{

    // gEncounterEnabled
    if (gEncounterEnabled == 0)
        return 0;

    if (gHeldKeysRaw & DPAD_ANY)
    {
        if (EventFlags_Test(0xBB) == 0)
        {
            // gEncounterTimer
            gEncounterCounter--;
            if (gEncounterCounter == 0)
            {
                // Rand
                gEncounterCounter = ((Rand_TableNext() & 7) << 5) + 0xE8;
                if (gCurrentMapId == 0x7B)
                {
                    gEncounterCounter >>= 1;
                }
                return 1;
            }
        }
        else
        {
            if (gEncounterCounter < 0xE8)
            {
                gEncounterCounter = 0xE8;

                if (gCurrentMapId == 0x7B)
                {
                    gEncounterCounter = 0x74;
                }
            }
        }
    }

    return 0;
}

// @ 0x08002DDC
void LogoBlendEffect_Update(void)
{

    if (gLogoEffectState == 0)
    {
        return;
    }
    switch (gLogoEffectState)
    {
        case 1:
            REG_DISPCNT &= 0xFEFF;
            gBlendControl = 0x1E41;
            gBlendCoefficients = 0xF00;
            gBlendFadeStep = 0;
            return;

        case 2:
        case 3:
        case 4:
        case 10:
            break;

        case 5:
            REG_DISPCNT |= 0x100;
            gBlendCoefficients = 0xF00;
            gLogoEffectState++;
            break;

        case 6:
            gBlendFadeStep++;
            gBlendCoefficients &= 0xF00;
            gBlendCoefficients |= (gBlendFadeStep >> 2) & 0x1F;
            if ((gBlendFadeStep >> 2) == 0x1F)
            {
                gLogoEffectState++;
                gBlendFadeStep = 0;
            }
            break;

        case 7:
            gBlendFadeStep++;
            if (!(gBlendFadeStep & 3))
            {
                gBlendCoefficients -= 0x100;
                if (!(gBlendCoefficients & 0xFF00))
                {
                    gLogoEffectState++;
                    gBlendFadeStep = gBlendCoefficients & 0xFF00;
                }
            }
            break;

        case 8:
            gBlendFadeStep++;
            if (gBlendFadeStep > 0x1B3)
            {
                gLogoEffectState++;
                gBlendFadeStep = 0;
            }
            break;
        case 9:
            gBlendFadeStep++;
            gBlendCoefficients = (((gBlendFadeStep >> 2) & 0x1F) << 8) | ((0x1F - (gBlendFadeStep >> 2)) & 0x1F);
            if (gBlendCoefficients == 0x1F00)
            {
                gLogoEffectState++;
                gBlendFadeStep = 0;
            }
            break;
    }
}

#define GET_PLTT(n)    ((n) + 0)
#define GET_TILEMAP(n) ((n) + 32)

// @ 0x08002F6C
void LogoAssets_Load(void)
{

    switch (gLogoEffectState)
    {
        case 1:
            REG_BG0CNT = BGCNT_SCREENBASE(31) | BGCNT_CHARBASE(2);
            DmaCopy16(3, (void *)GET_PLTT(pltt_08057854), (void *)0x050001C0, 0x20);
            LZ77UnCompVram((void *)GET_TILEMAP(pltt_08057854), (void *)BG_SCREEN_ADDR(31));
            gLogoEffectState++;
            break;
        case 2:
            LZ77UnCompVram(tileset_1_08057A80, (void *)0x06008000);
            gLogoEffectState++;
            break;
        case 3:
            LZ77UnCompVram(tileset_2_08057EEC, (void *)0x06008800);
            gLogoEffectState++;
            break;
        case 4:
            LZ77UnCompVram(tileset_3_080583C4, (void *)0x06009000);
            gLogoEffectState++;
            break;
        case 10:
            REG_DISPCNT &= 0xFEFF;
            gBlendControl = 0x1C12;
            gBlendCoefficients = 0xC07;
            CpuFill16(0, (void *)0x0600F800, 0x800);

            gLogoEffectState = 0;
        default:
            return;
    }
}

// @ 0x08003088
void Task_DispatchGameState(void)
{
    ReadKeys();
    RenderQueue_Clear();
    gGameStateCallbacks[gGameState]();
}

// 地图场景切换，加载数据精灵
//  @ 0x080030B0
void SceneTransition_RequestMap()
{
    if (gScreenTransitionState == 0 && gScenePhase == 1)
    {
        gGameState = GAME_STATE_SCENE_LOAD;
        gVBlankPipelineMode = 1;
    }
    else if (gScenePhase != 1)
    {
        gSceneLoadToggle = (gSceneLoadToggle + 1) & 1;
        gScenePhase = 1;
        Palette_Backup();
        ScreenFx_SetMode(4U);
    }
    Sprites_UpdateFrame();
}

// @ 0x08003114
void Task_DialogueFrame(void)
{
    sub_800ACC8();
    sub_800C194();
    OAM_FlushFromQueue();
}

// @ 0x08003128
void Task_BattleMenuFrame(void)
{
    if (!(0x80 & gScreenFadeFlags) && (gScreenTransitionState == 0))
    {
        ChoiceMenu_HandleInput(gNewKeysRaw);
    }
    BattleIntro_Cursor();
    Party_FollowStep();
    OAM_FlushFromQueue();
    PaletteEffects_Update();
}

// @ 0x08003168
void Scene_ReloadViaMenu()
{
    if (gScreenTransitionState == 0 && gScenePhase == 1)
    {
        Followers_SyncToTail();
        gGameState = GAME_STATE_SCENE_LOAD;
        gVBlankPipelineMode = 1;
    }
    else if (gScenePhase != 1)
    {
        gSceneLoadToggle = (gSceneLoadToggle + 1) & 1;
        gScenePhase = 1;
        Palette_Backup();
        ScreenFx_SetMode(4);
        Bgm_FadeOut(0x2E);
    }
    BattleIntro_Cursor();
    Party_FollowStep();
    OAM_FlushFromQueue();
    PaletteEffects_Update();
}

// @ 0x080031E4
void Task_TitleMenuFrame()
{
    /* The title FSM prepares CPU-side state; these two calls finish the frame
     * by advancing palette transitions and publishing the queued OAM. */
    TitleMenu_ProcessFrame();
    PaletteEffects_Update();
    OAM_FlushFromQueue();
}

// @ 0x080031F8
void Task_TextFrame()
{
    sub_801417C();
    OAM_FlushFromQueue();
}

// @ 0x08003208
void Scene_ResetResources(void)
{
    u16 i;

    Sprites_ReleaseAll();
    RenderQueue_Clear();
    SpritePool_Clear();
    Queue34C0_Clear();

    for (i = 0; i < 128; i++)
    {
        gOamBuffer[i].attrs[0] = 0;
        gOamBuffer[i].attrs[1] = 0;
    }

    VramTransfer_Clear();
    PalTransfer_Clear();
    AnimSlots_Release();
    StaticObjs_Reset();
    OAM_FlushFromQueue();
}

// @ 0x08003254
void Anim_StepChara(u8 arg0)
{
    Anim_PlayCustom(arg0);
}

// @ 0x08003264
void PalTransfer_Flush()
{
    u16 i;
    u32 var_r3;

    for (i = 0; i < 32; i++)
    {
        if (gPalTransferQueue[i].field_1 != 0)
        {
            var_r3 = gPalTransferQueue[i].field_1 == 2 ? 0x40 : 0x20;
            DmaCopy16(3, gPalTransferQueue[i].field_4, 0x05000000 + (gPalTransferQueue[i].field_0 << 5), var_r3);
            gPalTransferQueue[i].field_1 = 0;
        }
    }
}

// @ 0x080032BC
void OAM_FlushFromQueue(void)
{
    u16 i;
    SpriteNode *sprNode;
    u8 count;
    u16 index;

    index = 0;

    for (i = 0; i < 128; i++)
    {
        sprNode = gSpriteRenderQueue[i];
        if (sprNode == NULL)
            continue;

        count = sprNode->flags & 0x7F;
        if (count)
        {
            while (count--)
            {
                if (count == 0xFF)
                    break;
                sprNode = Sprite_WriteOam(&index, sprNode);
                if (sprNode == NULL)
                    break;
            }
        }

        if (index > 0x7F)
            return;
    }

    while (index < 128)
    {
        gOamBuffer[index].attrs[0] = 0xA0;
        gOamBuffer[index].attrs[1] = 0;
        index++;
    }
}

// @ 0x08003348
void Sprites_ReleaseAll(void)
{
    Actor *charaObj;
    u8 sprNodeIdx;
    s16 i;
    u8 count;
    struct SpriteNode *current;
    struct SpriteNode *next;

    for (i = 0; i <= 23; i++)
    {
        charaObj = &gActors[i];
        sprNodeIdx = charaObj->sprNodeIdx;

        if (sprNodeIdx != 0)
        {

            if (charaObj->subSprNodeIdx != 0)
            {
                gSpriteNodePool[charaObj->subSprNodeIdx].flags = 0;
                gSpriteNodePool[charaObj->subSprNodeIdx].next = 0;
                charaObj->subSprNodeIdx = 0;
            }

            count = gSpriteNodePool[sprNodeIdx].flags;
            count &= 0x7F;
            current = &gSpriteNodePool[sprNodeIdx];

            while (count != 0)
            {
                current->flags = 0;
                next = current->next;
                current->next = 0;
                current = next;
                count--;
            }
        }
        gActors[i].sprNodeIdx = 0;
    }
}
// @ 0x080033E8
typedef struct
{
    u16 field_0;
    u8 pad[16 - 2];
} Unk_087EA394;

extern Unk_087EA394 *gUnk_087EA394[];

void Sprites_LoadMapNPCs(u8 arg0)
{
    Unk_087EA394 *ptr2;
    u16 i;
    u16 temp_r0;
    u32 temp_r3;

    if (gObjGraphicsSetId & 0x80)
        return;

    i = temp_r3 = gMapSceneDescriptors[arg0].npcSlotGroupId;
    if (i == 0)
        return;

    temp_r0 = (i - 1) * 18;
    arg0 = gMapNpcSlotGroups[temp_r0];

    ptr2 = gUnk_087EA394[temp_r3 - 1];

    for (i = 2; i < arg0 + 2; i++)
    {
        Chara_InitFromDesc(i, ptr2++);
    }
}
// @ 0x0800345C
INCLUDE_ASM("asm/matchings", Chara_InitFromDesc);
/*
typedef struct{
    u8 field_0;
    u8 field_1;
    u8 field_2;
    u8 field_3;
    u8 field_4;
    u8 field_5;
    u8 field_6;
    u8 field_7;
    u8 field_8;
    u8 field_9;
    u8 field_A;
    u8 field_B;
    u8* field_C;
}UnkStruct;
// @ 0x0800345C
void Chara_InitFromDesc(u8 arg0, UnkStruct* arg1) {
    Actor *chara = &gActors[2];
    struct SpriteNode* renderObj;
    struct SpriteNode* subRenderObj;
    u8 idx;
    u16 attr0, attr1, attr2;

    chara = &gActors[arg0];
    idx = Sprite_AllocNode();

    if(idx > 0x6F)
        return;

    renderObj = &gRenderObjects[idx];
    chara->sprNodeIdx = idx;
    chara->fields_1.all_fields = arg1->field_0;
    chara->field_2 = arg1->field_1;
    chara->paletteId = arg1->field_2;

    chara->facingDir = arg1->field_3;
    chara->x = arg1->field_4 << 3;
    chara->y = (arg1->field_5 + 1) << 3;
    chara->field_A = arg1->field_6;
    chara->field_B = arg1->field_7;
    chara->field_C = arg1->field_8;
    chara->field_D = arg1->field_9;
    chara->field_F = arg1->field_A;
    chara->field_13 = arg1->field_B;
    chara->field_24 = arg1->field_C;
    chara->animTimer = 0;
    chara->field_E = chara->facingDir;
    chara->field_10 = 1;
    chara->field_11 = 0;
    chara->field_12 = 0;
    chara->field_17 = 0;
    chara->field_1A = 0;
    chara->field_18 = 0;
    chara->field_19 = 0;
    chara->field_14 = 0;
    chara->animIdx = 0xFF;

    if(chara->fields_1.stru.bit0 == 0)
    {
        if(chara->fields_1.stru.bit2)
        {
            attr0 = 0;
            attr1 = 0x8000;
            attr2 = ((chara->paletteId << 12) + 0x800) + (((chara->field_2 * 72) + 0xA0) & 0x3FF);
            renderObj->field_10 = 0x1F8;
            renderObj->field_12 = 0xE0;
            Sprite_InitChainNode(renderObj, 1, attr0, attr1, attr2);
        }
        else if(chara->fields_1.stru.bit4)
        {
            attr0 = 0x4000;
            attr1 = 0xC000;
            attr2 = ((chara->paletteId << 12) + 0x800) + (((chara->field_2 * 72) + 0xA0) & 0x3FF);
            renderObj->field_10 = 0x1E8;
            renderObj->field_12 = 0xF8;
            Sprite_InitChainNode(renderObj, 1, attr0, attr1, attr2);
        }
        else if(chara->fields_1.stru.bit5)
        {
            attr1 = 0x8000;
            attr2 = ((chara->paletteId << 12) + 0x800) + (((chara->field_2 * 72) + 0xB0) & 0x3FF);
            renderObj->field_10 = 0x1F8;
            renderObj->field_12 = 0xE8;
            subRenderObj = Sprite_InitChainNode(renderObj, 2, 0, attr1, attr2);

            attr2 = ((chara->paletteId << 12) + 0x800) + (((chara->field_2 * 72) + 0xA0) & 0x3FF);
            subRenderObj->field_10 = 0x1F8;
            subRenderObj->field_12 = 0xC8;

            Sprite_InitChainNode(subRenderObj, 1, 0, attr1, attr2);

        }
        else if(chara->fields_1.stru.bit6)
        {
            attr1 = 0x8000;
            attr2 = ((chara->paletteId << 12) + 0x800) + (((chara->field_2 * 72) + 0xA0) & 0x3FF);
            renderObj->field_10 = 0x1F8;
            renderObj->field_12 = 0xEE;
            subRenderObj = Sprite_InitChainNode(renderObj, 3, 0, attr1, attr2);

            attr2 = ((chara->paletteId << 12) + 0x800) + (((chara->field_2 * 72) + 0xB0) & 0x3FF);
            subRenderObj->field_10 = 0x18;
            subRenderObj->field_12 = 0xEE;
            subRenderObj = Sprite_InitChainNode(subRenderObj, 2, 0, attr1, attr2);

            attr0 = 0x8000;
            attr1 = 0x8000;

            attr2 = ((chara->paletteId << 12) + 0x800) + (((chara->field_2 * 72) + 0xC0) & 0x3FF);
            subRenderObj->field_10 = 0x38;
            subRenderObj->field_12 = 0xEE;
            Sprite_InitChainNode(subRenderObj, 1, attr0, attr1, attr2);
        }
        else
        {
            attr1 = 0x4000;

            attr2 = ((chara->paletteId << 12) + 0x800) + (((chara->field_2 * 72) + 0xA0) & 0x3FF);
            renderObj->field_10 = 0;
            renderObj->field_12 = 0xE8;

            subRenderObj = Sprite_InitChainNode(renderObj, 2, 0, attr1, attr2);

            attr0 = 0x4000;

            attr2 = ((chara->paletteId << 12) + 0x800) + (((chara->field_2 * 72) + 0xA4) & 0x3FF);
            subRenderObj->field_10 = 0;
            subRenderObj->field_12 = 0xF8;
            // attr1 = 0;
            subRenderObj = Sprite_InitChainNode(subRenderObj, 1, attr0, 0, attr2);
        }
    }
    else
    {
        renderObj->field_0 = 0x81;
        renderObj->subObject = NULL;
    }
    renderObj->animFrame = 0;
}

*/
// @ 0x0800375C
void Chara_InitDialogArrow(u8 arg0)
{
    u8 temp_r1;
    Actor *ptr3150;
    ptr3150 = &gDialogArrowActors[arg0];
    temp_r1 = Sprite_AllocNode();
    if (temp_r1 < 0x70)
    {
        ptr3150->sprNodeIdx = 0;
        ptr3150->renderFlags = 2;
        ptr3150->gfxSetId = 9;
        ptr3150->paletteId = 9;
        ptr3150->facingDir = 0;
        ptr3150->x = (gCameraPosX + (arg0 * 5 + 5) * 8);
        ptr3150->y = (gCameraPosY + 0x28);
        ptr3150->field_A = 0;
        ptr3150->field_B = 0;
        ptr3150->field_C = 0;
        ptr3150->field_D = 0;
        ptr3150->field_F = 0;
        ptr3150->field_13 = 0x80;
        ptr3150->cmdStream = 0;
        ptr3150->animTimer = 0;
        ptr3150->targetFacing = 0;
        ptr3150->stepTimer = 1;
        ptr3150->field_11 = 0;
        ptr3150->stateFlags = 0x20;
        ptr3150->cmdPc = 0;
        ptr3150->z = 0;
        ptr3150->subSprNodeIdx = 0;
        ptr3150->field_19 = 0;
        ptr3150->field_14 = 0;
        ptr3150->animIdx = 0xFF;
    }
}

// @ 0x080037DC
void Chara_InitEffect(u8 arg0)
{
    u8 idx;
    Actor *charaObj;
    SpriteNode *sprNode;

    charaObj = &gActors[arg0];

    idx = Sprite_AllocNode();
    if (idx < 0x70)
    {
        sprNode = &gSpriteNodePool[idx];
        charaObj->sprNodeIdx = idx;
        charaObj->renderFlags = 2;
        charaObj->gfxSetId = 5;
        charaObj->paletteId = 5;
        charaObj->facingDir = 0;
        charaObj->field_A = 0;
        charaObj->field_B = 0;
        charaObj->field_C = 0;
        charaObj->field_D = 0;
        charaObj->field_F = 0;
        charaObj->field_13 = 0;
        charaObj->cmdStream = 0;
        charaObj->animTimer = 0;
        charaObj->targetFacing = 0;
        charaObj->stepTimer = 1;
        charaObj->field_11 = 0;
        charaObj->stateFlags = 32;
        charaObj->cmdPc = 0;
        charaObj->z = 0;
        charaObj->subSprNodeIdx = 0;
        charaObj->field_19 = 0;

        sprNode->flags = 0;
        sprNode->animStep = 0;
        sprNode->next = 0;
    }
}

// @ 0x0800384C
void Chara_InitEffectAtPlayer(void)
{
    u8 temp_r1;
    Actor *ptr3150;
    struct SpriteNode *sprNode;

    ptr3150 = &gEffectActor;
    temp_r1 = Sprite_AllocNode();
    if (temp_r1 < 0x70)
    {
        sprNode = &gSpriteNodePool[temp_r1];
        ptr3150->sprNodeIdx = temp_r1;
        ptr3150->renderFlags = 2;
        ptr3150->gfxSetId = 0xA;
        ptr3150->paletteId = 0xA;
        ptr3150->facingDir = 0;
        ptr3150->x = (gCameraTargetX + 8);
        ptr3150->y = (gCameraTargetY + 0xC);
        ptr3150->field_A = 0;
        ptr3150->field_B = 0;
        ptr3150->field_C = 0;
        ptr3150->field_D = 0;
        ptr3150->field_F = 0;
        ptr3150->field_13 = 0x80;
        ptr3150->cmdStream = 0;
        ptr3150->animTimer = 0;
        ptr3150->targetFacing = 0;
        ptr3150->stepTimer = 1;
        ptr3150->field_11 = 0;
        ptr3150->stateFlags = 0x20;
        ptr3150->cmdPc = 0;
        ptr3150->z = 0;
        ptr3150->subSprNodeIdx = 0;
        ptr3150->field_19 = 0;

        sprNode->flags = 128;
        sprNode->animStep = 0;
        sprNode->next = 0;
    }
}
/* 延迟装载的消费者: SetSlotGfxId / SetSlotPalId 只记参数 + 置 gPendingSpriteLoad 位,
 * 真正的搬运在这里做, 做完整体清零。
 * ⚠ 三个必须保持的形态:
 *   1) 两个 if 各读一次 gPendingSpriteLoad (目标是两条独立的 ldrb, 不能提到外面缓存);
 *   2) 位测试写成 `CONST & flags` —— 常量在左(规则 5/78), 换成 `flags & CONST` 会变形态;
 *   3) 装载体必须留在 static inline 的小函数里。把局部变量合并进本函数会让
 *      GCC2 的寄存器分配跑偏(实测: helper 版 5/140 字节差=只剩链接器 thunk,
 *      合并版 59/140)。 */
extern u8 *gUnk_087E8430[]; /* 248 项 LZ77 压缩精灵图块指针表 */
extern u8 gUnk_080B9DFC[][32]; /* 精灵 OBJ 调色板, 每项 16 色 BGR555 */

static inline void Inl_LoadSpriteSheetGfx(u8 slot, u16 gfxId)
{
    u8 *dst = (void *)0x06011400 + slot * 0x900;

    LZ77UnCompVram(gUnk_087E8430[gfxId], dst);
}

static inline void Inl_LoadSpriteSheetPal(u8 slot, u16 palId)
{
    const u8 *src;
    u8 *dst;

    src = gUnk_080B9DFC[palId];
    dst = (void *)0x05000200 + slot * 32;
    DmaCopy16(3, src, dst, 0x20);
}

// @ 0x080038CC
void PendingSpriteLoad_Flush(void)
{
    if (PENDING_SPRITE_GFX & gPendingSpriteLoad)
        Inl_LoadSpriteSheetGfx(gPendingGfxSlot, gPendingGfxId);

    if (PENDING_SPRITE_PAL & gPendingSpriteLoad)
        Inl_LoadSpriteSheetPal(gPendingPalSlot, gPendingPalId);

    gPendingSpriteLoad = 0;
}

// @ 0x08003958
INCLUDE_ASM("asm/nonmatchings", Chara_SetWalkPath);
// @ 0x08003B08
void Chara_ProcessCmdStream(u16 arg0)
{
    u8 var_r5;
    u8 *temp_r2;
    Actor *chara;
    u8 cmd;

    chara = &gActors[arg0];

    if (chara->cmdStream != 0)
    {
        var_r5 = 0;
        while (var_r5 == 0)
        {
            temp_r2 = chara->cmdStream + chara->cmdPc;
            cmd = *temp_r2++;
            switch (cmd)
            {
                case 0xFE:
                    chara->cmdPc = 0;
                    break;
                case 0xFD:
                    chara->stateFlags |= 0x20;
                    chara->stateFlags &= 0x7F;
                    chara->cmdStream = NULL;
                    return;
                case 0xFF:
                    chara->stateFlags &= 0x7B;
                    chara->cmdStream = NULL;
                    var_r5++;
                    break;
                case 3:
                    chara->targetFacing = temp_r2[0];
                    chara->cmdPc += 2;
                    break;
                case 1:
                    chara->cmdPc += 3;
                    chara->targetFacing = temp_r2[0];
                    chara->facingDir = chara->targetFacing;
                    chara->field_F = 0;
                    chara->stepTimer = cmd;
                    chara->field_11 = temp_r2[1];
                    var_r5++;
                    break;

                case 2:
                    chara->stateFlags |= 0x10;
                    chara->cmdPc += 4;

                    chara->targetFacing = *temp_r2++;
                    chara->targetFacing &= 7;

                    chara->field_F = temp_r2[0];
                    chara->stepTimer = temp_r2[1] + 1;
                    return;

                default:
                    chara->cmdPc += 4;
                    chara->targetFacing = temp_r2[0];
                    temp_r2++;
                    chara->field_F = temp_r2[0];
                    chara->stepTimer = temp_r2[1] + 1;
                    var_r5++;
                    break;
            }
            chara->targetFacing &= 7;
            chara->facingDir = chara->targetFacing;
        }
        return;
    }

    if (gDialogueActive == 0)
    {
        switch ((Rand_TableNext() & 7))
        {
            case 1:
            case 4:
            case 5:
            case 6:
                chara->field_11 = 0x10;
                chara->stepTimer = 1;
                break;
            case 2:
                chara->targetFacing++;
                break;
            case 3:
                chara->targetFacing--;
                break;
            case 7:
                break;
        }

        chara->targetFacing &= 7;
        chara->facingDir = chara->targetFacing;
    }
}
// @ 0x08003C54
INCLUDE_ASM("asm/nonmatchings", Chara_StepMove);
// @ 0x08003F40
// 检查玩家面前一格的事件: 命中治疗神像 → 全队回复; 否则依次查 Actor[2..18] (转向)
// 与 16 个宝箱, 返回交互 ID+1 (0 = 无事件)。
u8 CheckFacingEvent(void)
{
    u16 x1;
    u16 y1;
    u16 x2;
    u16 y2;
    s32 eventX;
    s32 eventY;
    u32 rectIdx;
    u16 i;
    const u8 *src;
    Actor *chara;
    ChestObject *chest;
    const u16 *offs;

    chara = &gActors[2];
    offs = gFacingEventOffsets;
    rectIdx = gPlayerMoveDir * 4;
    x1 = offs[rectIdx] + gCameraTargetX;
    y1 = (gFacingEventOffsets[rectIdx + 1] + gCameraTargetY) + 8;
    x2 = x1 + gFacingEventOffsets[rectIdx + 2];
    y2 = y1 + gFacingEventOffsets[rectIdx + 3];

    if (gUnk_03004618 != 0)
    {
        src = &gUnk_087E94F8[gUnk_03004618 * 4];
        eventX = src[2] * 8;
        if (((eventX + 15) > x1) && (eventX < x2))
        {
            eventY = src[3] * 8;
            if (((eventY + 7) > y1) && (eventY < y2))
            {
                ScreenFx_SetMode(7);
                FullHealParty();
                Sfx_Play(0x17, 1, 0);
                gPendingCharaSwitch = src[1];
                gPartyFollowFlags |= 0x80;
                return 0;
            }
        }
    }

    for (i = 2; i <= 0x12; chara++, i++)
    {
        if (((((chara->sprNodeIdx != 0) && ((((u16) chara->x) + 15) > x1)) && (((u16) chara->x) < x2)) && ((((u16) chara->y) + 7) > y1)) && (((u16) chara->y) < y2))
        {
            if ((chara->renderFlags & 0x20) == 0)
            {
                if ((chara->stateFlags & 0x10) == 0)
                {
                    chara->targetFacing = chara->facingDir;
                }
                chara->facingDir = (gPlayerMoveDir + 4) & 7;
            }
            return chara->field_13 + 1;
        }
    }

    chest = gChestObjects;
    for (i = 0; i < 16; chest++, i++)
    {
        if ((((((chest->spriteNodeIdx != 0) && ((chest->flags & 1) == 0)) && ((chest->x + 9) > x1)) && ((chest->x + 7) < x2)) && ((chest->y + 4) > y1)) && ((chest->y - 4) < y2))
        {
            if ((chest->flags & 0x80) == 0)
            {
                ChestObject_Open(i);
                return chest->interactionId + 1;
            }
            else
            {
                if (EventFlags_Test(0x40) != 0)
                {
                    ChestObject_Open(i);
                    return chest->interactionId + 1;
                }
                else
                {
                    return chest->interactionId + 1;
                }
            }
        }
    }

    return 0;
}
// @ 0x080040E4
INCLUDE_ASM("asm/nonmatchings", Party_FollowAnim);

// @ 0x08004358
void Followers_ResetHistory(void)
{
    u16 i;

    if (!(gPartyFollowFlags & 1))
    {
        for (i = 0; i < 8; i++)
        {
            gFollowerHistX[i] = gActors[0].x;
            gFollowerHistY[i] = gActors[0].y;
            gFollowerHistDir[i] = gActors[0].facingDir & 7;
        }

        gActors[1].x = gActors[0].x;
        gActors[1].y = gActors[0].y;
        gActors[1].facingDir = gActors[0].facingDir & 7;
    }
}

// @ 0x080043D4
void Followers_SyncToTail(void)
{
    u16 i;

    for (i = 0; i < 8; i++)
    {
        gFollowerHistX[i] = gActors[0].x;
        gFollowerHistY[i] = gActors[0].y;
        gFollowerHistDir[i] = gActors[0].facingDir;
    }

    gActors[1].x = gFollowerHistX[7];
    gActors[1].y = gFollowerHistY[7];
    gActors[1].facingDir = gFollowerHistDir[7];
    gPartyFollowFlags &= 0x80;
}
// @ 0x0800445C
INCLUDE_ASM("asm/matchings", Party_FollowStep);
/*
// @ 0x0800445C
void Party_FollowStep(void) {
    Actor* chara;
    u16 i;

    u8 a;
    chara = &gActors[0];
    if(chara->sprNodeIdx != 0)
    {
        a = (chara->field_12 & 0x80);
        if(a != 0)
        {
            if(Chara_StepMove(0) < 2)
            {
                Chara_ProcessCmdStream(0);
            }

            for(i = 7; i > 0; i--)
            {
                gFollowerHistX[i] = gFollowerHistX[i - 1];
                gFollowerHistY[i] = gFollowerHistY[i - 1];
                gFollowerHistDir[i] = gFollowerHistDir[i - 1];
            }

            gFollowerHistX[0] = chara->x;
            gFollowerHistY[0] = chara->y;
            gFollowerHistDir[0] = chara->facingDir & 7;

            gPlayerMoveDir = chara->facingDir;
            gCameraTargetX = chara->x;
            gCameraTargetY = chara->y - 8;

        }

        else
        {
            if((u8)(gWarpAnimState - 6) < 2 || gWarpAnimState == 9 || gWarpAnimState == 10)
                return;

            if(chara->field_12 & 0x40)
            {
                chara->field_11 = 0 ;

                for(i = 7; i > 0; i--)
                {
                    gFollowerHistX[i] = gFollowerHistX[i - 1];
                    gFollowerHistY[i] = gFollowerHistY[i - 1];
                    gFollowerHistDir[i] = gFollowerHistDir[i - 1];
                }
                gFollowerHistX[0] = chara->x;
                gFollowerHistY[0] = chara->y;
                gFollowerHistDir[0] = chara->facingDir & 7;

            }
            else
            {
                chara->field_11 = 1;
            }
            chara->facingDir = gPlayerMoveDir;
            chara->x = gCameraTargetX;
            chara->y = gCameraTargetY + 8;

        }

        if((chara->field_1 & 1) == 0)
        {
            Sprite_UpdateCharaAnim(0);
            Sprite_EnqueueRender(chara->x, chara->y, chara->sprNodeIdx, chara->field_1A, chara->field_1 );
            if(chara->field_18 != 0)
            {
                gSpriteNodePool[chara->field_18].field_0 = 0;
                gSpriteNodePool[chara->field_18].subObject = 0;
                chara->field_18 = 0;
            }

            Sprite_SetupDialogArrow(0);
        }
    }

    chara++;
    if(chara->sprNodeIdx != 0)
    {
        if((gPartyFollowFlags & 1) == 0)
        {
            if((u8)( gWarpAnimState - 6) < 2 || gWarpAnimState == 9 || gWarpAnimState == 10)
                return;
            {
                chara->facingDir = gFollowerHistDir[7];
                chara->x = gFollowerHistX[7];
                chara->y = gFollowerHistY[7];
            }

        }
        else
        {
            if(chara->field_12 & 0x80 && Chara_StepMove(1) < 2)
            {
                Chara_ProcessCmdStream(1);
            }
        }

        if((chara->field_1 & 1) == 0)
        {
            Sprite_UpdateCharaAnim(1);
            Sprite_EnqueueRender(chara->x, chara->y, chara->sprNodeIdx, chara->field_1A, chara->field_1 );
            if(chara->field_18 != 0)
            {
                gSpriteNodePool[chara->field_18].field_0 = 0;
                gSpriteNodePool[chara->field_18].subObject = 0;
                chara->field_18 = 0;
            }

            Sprite_SetupDialogArrow(1);
        }
    }

}
*/

extern u8 gUnk_0838EEF4[];

#define CUTSCENE_ANIM_BASE ((u8 *)0x02020000)
/* 把一个过场动画加载到一个缓冲槽。
 *   animId   : gCutsceneAnimConfigTable 下标 (脚本参数 data[1]|data[2]<<8)
 *   slot     : 0..N 缓冲槽, 每槽占 CUTSCENE_ANIM_BASE + slot*0x1000 的 4 KB VRAM 区
 *   slotSel  : 十进制编码 —— ≥100 表示 "减 100 存为动画槽号, 并额外置 flags 的 bit6"
 */
// @ 0x080046DC
void CutsceneAnim_Load(u16 animId, u8 slot, u8 slotSel)
{
    u8 extraFlags;
    u8 animSlot;

    gCutsceneAnimScripts[slot] = gUnk_087E860C[gCutsceneAnimConfigTable[animId].scriptIdx];
    gVramBufferPointers[slot] = (u32)(CUTSCENE_ANIM_BASE + slot * 0x1000);

    if (slotSel > 99)
    {
        extraFlags = 0x40;
        animSlot = slotSel - 100;
    }
    else
    {
        extraFlags = 0;
        animSlot = slotSel;
    }
    gCutsceneAnimFlags[slot] = extraFlags | gCutsceneAnimConfigTable[animId].loopFlag;
    gCutsceneAnimSlots[slot] = animSlot;
    gCutsceneAnimPals[slot] = &gUnk_0838EEF4[gCutsceneAnimConfigTable[animId].palIdx * 32];
    LZ77UnCompWram((void *)gUnk_087E8D84[gCutsceneAnimConfigTable[animId].gfxIdx], (void *)(CUTSCENE_ANIM_BASE + slot * 0x1000));
}

/*

typedef struct{
    u16 field_0;
    u16 field_2;
    u8 field_4;
    u8 field_5;
    u16 field_6;
}Unk_0805888C;

extern Unk_0805888C gUnk_0805888C[];
extern u32 gUnk_087E860C[];
extern u8 gUnk_0838EEF4[];

extern u8* gUnk_087E8D84[];

extern u32 gCutsceneAnimScripts[];
extern u32 gCutsceneAnimVram[];
extern u8 gCutsceneAnimFlags[];
extern u8 gCutsceneAnimSlots[];
extern u8* gCutsceneAnimPals[];

#define VRAM_BASE  ((u8*)0x02020000)
#define VRAM_STRIDE 0x1000

// @ 0x080046DC
void CutsceneAnim_Load(u16 arg0, u8 arg1, u8 arg2) {
    u8 a;
    u8 b;

    gCutsceneAnimScripts[arg1] = gUnk_087E860C[gUnk_0805888C[arg0].field_0];
    gCutsceneAnimVram[arg1] = (u32)(VRAM_BASE + arg1 * VRAM_STRIDE);

    if(arg2 > 99)
    {
        a = 0x40;
        b = arg2 - 100;
    }
    else
    {
        a = 0;
        b = arg2;
    }

    gCutsceneAnimFlags[arg1] = a | gUnk_0805888C[arg0].loopFlag;
    gCutsceneAnimSlots[arg1] = b;

    gCutsceneAnimPals[arg1] = &gUnk_0838EEF4[gUnk_0805888C[arg0].field_4 * 32];
    LZ77UnCompWram(gUnk_087E8D84[gUnk_0805888C[arg0].field_2], (u32)(VRAM_BASE + arg1 * VRAM_STRIDE));

}

*/

typedef struct CutsceneAnimEnemyEntry
{
    u8 pad_0[0x1A];
    u16 animIdx;
    u8 pad_1C[4];
    u16 yOffsetMode;
    u8 pad_22[0xA];
} CutsceneAnimEnemyEntry;

typedef struct CutsceneAnimSpecialEntry
{
    u16 animIdx;
    u8 pad_2[0x3E];
} CutsceneAnimSpecialEntry;

extern CutsceneAnimEnemyEntry gUnk_083989FC[];
extern CutsceneAnimSpecialEntry gUnk_0839C80C[];
extern u8 *gUnk_087EBE00[];

// @ 0x0800478C
void CutsceneAnim_PlayFrame(u16 animEntityId)
{
    u32 slot = 0;
    u16 animIdx;

    if (animEntityId > 0x7F)
    {
        animIdx = gUnk_0839C80C[animEntityId - 0x80].animIdx;
    }
    else
    {
        animIdx = gUnk_083989FC[animEntityId].animIdx;
    }

    gCutsceneAnimScripts[slot] = (u32)gUnk_08393B28[animIdx].animScriptPtr;
    gVramBufferPointers[slot] = (u32)CUTSCENE_ANIM_BASE;
    gCutsceneAnimFlags[slot] = 0x80;
    gCutsceneAnimSlots[slot] = 5;
    gCutsceneAnimPals[slot] = (u8 *)gUnk_08393B28[animIdx].palettePtr;
    LZ77UnCompWram(gUnk_087EBE00[gUnk_08393B28[animIdx].gfxBaseIdx], CUTSCENE_ANIM_BASE);

    if (animEntityId > 0x7F)
    {
        switch (animEntityId)
        {
        case 0x81:
            gActors[0].x = gCameraPosX + 0xA0;
            gActors[0].y = gCameraPosY + 0x68;
            break;
        case 0x82:
            gActors[0].x = gCameraPosX + 0x90;
            gActors[0].y = gCameraPosY + 0x68;
            break;
        case 0x84:
            gActors[0].x = gCameraPosX + 0xA0;
            gActors[0].y = gCameraPosY + 0x78;
            break;
        case 0x85:
            gActors[0].x = gCameraPosX + 0xA0;
            gActors[0].y = gCameraPosY + 0x70;
            break;
        case 0x88:
            gActors[0].x = gCameraPosX + 0x85;
            gActors[0].y = gCameraPosY + 0x7C;
            break;
        case 0x89:
            gActors[0].x = gCameraPosX + 0x90;
            gActors[0].y = gCameraPosY + 0x6C;
            break;
        default:
            gActors[0].x = gCameraPosX + 0x88;
            gActors[0].y = gCameraPosY + 0x68;
            break;
        }
    }
    else
    {
        gActors[0].x = gCameraPosX + 0x88;
        switch (gUnk_083989FC[animEntityId].yOffsetMode)
        {
        case 2:
            gActors[0].y = gCameraPosY + 0x68;
            break;
        case 1:
            gActors[0].y = gCameraPosY + 0x60;
            break;
        default:
            gActors[0].y = gCameraPosY + 0x58;
            break;
        }
    }
}

// @ 0x08004980
void MapGroup_Lookup(void)
{
    u8 i;

    gPendingCharaSwitch = 0xFF;

    for (i = 0; i < 22; i++)
    {
        if (gUnk_087E94FC[i].field_0 == gCurrentMapId)
        {
            gUnk_03004618 = i + 1;
            return;
        }
    }
    gUnk_03004618 = 0;
}

// @ 0x080049C8
void Chara_SetTilePos(u8 arg0, u8 arg1, u8 arg2, u8 arg3)
{

    Actor *ptr = &gActors[arg0];

    if (arg1)
    {
        ptr->y = ((arg2 + 1) << 3) + arg3;
    }
    else
    {
        ptr->x = (arg2 << 3) + arg3;
    }
}

// @ 0x08004A00
void Chara_MoveBy(u8 actorIdx, u8 isY, u8 positive, u8 delta)
{
    u16 val;
    Actor *ptr;
    u16 temp;

    if (positive)
    {
        val = delta;
    }
    else
    {
        val = -delta;
    }

    ptr = &gActors[actorIdx];

    if (isY)
    {
        temp = ptr->y;
        ptr->y = temp + val;
    }
    else
    {
        temp = ptr->x;
        ptr->x = temp + val;
    }
}
