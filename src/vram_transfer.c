#include "code_0.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "globals.h"
#include "data_87E83F0.h"
#include "data_805769C.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "sound.h"

extern void IntrMain();

// @ 0x08004A44
u16 VramTransfer_AllocSlot(void)
{
    u16 i;

    for (i = 0; i < 32; i++)
    {
        if (gVramTransferCounts[i] == 0)
        {
            return i;
        }
    }
    return -1;
}

// @ 0x08004A6C
u8 PalTransfer_AllocSlot()
{
    u8 i;

    for (i = 0; i < 32; i++)
    {
        if (gPalTransferQueue[i].field_1 == 0)
        {
            return i;
        }
    }
    return -1;
}

// @ 0x08004A94
void PalTransfer_Enqueue(u8 arg0, u32 arg1, s8 arg2, u8 arg3)
{

    gPalTransferQueue[arg0].field_0 = arg2;
    gPalTransferQueue[arg0].field_4 = arg1;

    if (arg3 != 0)
    {
        gPalTransferQueue[arg0].field_1 = 2;
    }
    else
    {
        gPalTransferQueue[arg0].field_1 = 1;
    }
}

// @ 0x08004AC0
void VramTransfer_Clear(void)
{
    u16 i;

    for (i = 0; i < 32; i++)
    {
        gVramTransferCounts[i] = 0;
    }
}

// @ 0x08004ADC
void VramTransfer_Flush(void)
{
    u16 i;

    for (i = 0; i < 32; i++)
    {
        if (gVramTransferCounts[i] != 0)
        {
            DmaCopy16(3, gVramTransferQueue[i].src, gVramTransferQueue[i].dest, gVramTransferCounts[i] << 5);
            gVramTransferCounts[i] = 0;
        }
    }
}

// @ 0x08004B2C
void VramTransfer_Enqueue(u16 arg0, void *arg1, void *arg2, u8 arg3)
{

    if (arg0 < 32)
    {
        gVramTransferQueue[arg0].src = arg1;
        gVramTransferQueue[arg0].dest = arg2;
        gVramTransferCounts[arg0] = arg3;
    }
}

// @ 0x08004B60
void PalTransfer_Clear()
{
    u16 i;

    for (i = 0; i < 32; i++)
    {
        gPalTransferQueue[i].field_1 = 0;
        gPalTransferQueue[i].field_0 = 0;
        gPalTransferQueue[i].field_4 = 0;
    }
}

// @ 0x08004B8C
void SpritePool_Clear()
{
    u16 i;

    for (i = 0; i < 128; i++)
    {
        gSpriteNodePool[i].flags = 0;
        gSpriteNodePool[i].next = 0;
    }
}

// @ 0x08004BBC
void Queue34C0_Clear(void)
{
    u16 i;

    for (i = 0; i < 32; i++)
    {
        gOamAffineBuf[i].field_0 = 0;
        gOamAffineBuf[i].field_2 = 0;
        gOamAffineBuf[i].field_4 = 0;
        gOamAffineBuf[i].field_6 = 0;
    }
}

// @ 0x08004BE0
void RenderQueue_Clear(void)
{
    u16 i;
    for (i = 0; i < 128; i++)
    {
        gSpriteRenderQueue[i] = 0;
    }
}

// @ 0x08004BFC
u8 Sprite_AllocNode(void)
{
    u16 i;

    for (i = 2; i < 0x70; i++)
    {
        if (gSpriteNodePool[i].flags == 0)
        {
            return i;
        }
    }
    return 0;
}
static inline u8 findEmpty_Inl()
{
    u16 i;
    for (i = 2; i < 0x70; i++)
    {
        if (gSpriteNodePool[i].flags == 0)
        {
            return i;
        }
    }
    return 0;
}

// @ 0x08004C28
SpriteNode *Sprite_InitChainNode(SpriteNode *sprNode, u8 arg1, u16 arg2, u16 arg3, u16 arg4)
{
    u8 foundIndex;

    sprNode->flags = arg1;
    sprNode->attr0 = arg2;
    sprNode->attr1 = arg3;
    sprNode->attr2 = arg4;
    sprNode->animStep = 0;

    if ((arg1 & 0x7F) == 1)
        return 0;

    foundIndex = findEmpty_Inl();

    if (foundIndex == 0)
    {
        return 0;
    }

    sprNode->next = &gSpriteNodePool[foundIndex];
    return &gSpriteNodePool[foundIndex];
}
/* 精灵表槽位布局 (共 12 个槽, 见 ReloadAllSpriteSheets 的 `i < 12`):
 *   OBJ 图块区 VRAM 0x06011400 + slot * 0x900   每槽 2304 B = 72 个 4bpp 8×8 图块
 *   OBJ 调色板   PLTT 0x05000200 + slot * 32      每槽 16 色
 * gfxId / palId 是角色图形编号(来自 gPartyMemberIds / gSlotGfxId / gSlotPalId)。
 *
 * ⚠ dst 必须先算好放进变量: 直接 `LZ77UnCompVram(tbl[id], 0x06011400 + slot*0x900)` 会让
 * GCC2 先算 src 再算 dst, 尾部多一条 `adds r0, r2, #0` 把 src 拷回 r0。 */
extern u8 *gUnk_087E8430[]; /* 248 项 LZ77 压缩图块指针表 */

// @ 0x08004C8C
void LoadSpriteSheetGfx(u8 slot, u16 gfxId)
{
    void *dst;

    dst = (void *)0x06011400 + slot * 0x900;
    LZ77UnCompVram(gUnk_087E8430[gfxId], dst);
}

/* 同上: src/dst 先各自算好再交给 DmaCopy16, 否则 `vu32 *dmaRegs` 会被 CSE 提到最前面,
 * 目标里它是在 src/dst 之后才 `ldr r2, =0x040000D4` 的。 */
extern u8 gUnk_080B9DFC[][32]; /* 每帧 16 色 BGR555 调色板 */

// @ 0x08004CB8
void LoadSpriteSheetPal(u8 slot, u16 palId)
{
    const u8 *src;
    u8 *dst;

    src = gUnk_080B9DFC[palId];
    dst = (void *)0x05000200 + slot * 32;
    DmaCopy16(3, src, dst, 0x20);
}

/* OBJ(精灵) 图块槽位 146~149 留给“箭头/滚动条”字形, 槽位 150 起是 10 个数字字形
 * (后者由 LoadDigitFontObjTiles 连同 OBJ 调色板一起装入 0x060112C0 / 0x050003C0)。 */
#define UI_ARROW_TILES_2  ((const u8 *)0x08393728) /* ◀ ▶      2 块 =  64 B */
#define UI_ARROW_TILES_4  ((const u8 *)0x08393768) /* ◀ ▬ ▶ ▫  4 块 = 128 B */
#define UI_ARROW_OBJ_VRAM ((void *)0x06011240) /* OBJ 图块基 + 146*32 */

/* 按 arg0 的符号位(= bit7)选一套箭头/滚动条图块, 用 DMA3 以 16 位宽装入 OBJ 图块区。
 * 调用方传的是 gObjGraphicsSetId 的最低字节(按 s8 看), 其 bit7 是图形变体标志:
 *   bit7 = 0  → 2 块(64B), 与 LoadDigitFontObjTiles 的数字字体一起用
 *   bit7 = 1  → 4 块(128B), 此时 LoadDigitFontObjTiles 直接 return 不装数字
 * 注意形参必须是 s8: 目标入口只有 `lsls r0,#0x18` 而没有配对的 asrs,
 * 因为左移已把 bit7 送到符号位, 直接 `cmp r0,#0; blt` 就能完成有符号比较。 */
// @ 0x08004CE8
void LoadArrowObjTiles(s8 arg0)
{
    const u8 *src;
    u16 size;

    if (arg0 >= 0)
    {
        src = UI_ARROW_TILES_2;
        size = 0x40;
    }
    else
    {
        src = UI_ARROW_TILES_4;
        size = 0x80;
    }

    DmaCopy16(3, src, UI_ARROW_OBJ_VRAM, size);
}

// @ 0x08004D20
void Chara_SetGfxPal(u8 arg0, u8 arg1, u8 arg2)
{
    Actor *p;
    p = &gActors[arg0];

    p->gfxSetId = arg1;
    p->paletteId = arg2;

    // gActors[arg0].unk2 = arg1;
    // gActors[arg0].unk3 = arg2;
}

// @ 0x08004D38
void Chara_FreeSprite(u8 arg0)
{
    Actor *ptr2E80;
    struct SpriteNode *node;
    struct SpriteNode *next;

    ptr2E80 = &gActors[arg0];

    node = &gSpriteNodePool[ptr2E80->sprNodeIdx];

    if (node->flags != 0)
    {
        node->flags = 0;

        next = node->next;
        if (next != 0)
        {
            do
            {
                node->next = 0;
                node = next;
                if (node->flags == 0)
                    break;
                node->flags = 0;
                next = node->next;
            } while (next != 0);
        }
    }

    ptr2E80->sprNodeIdx = 0;
}
// @ 0x08004D8C
void Chara_SetCmdPtr(u8 arg0, u8 *arg1)
{
    Actor *ptr2E80;
    ptr2E80 = &gActors[arg0];
    ptr2E80->cmdStream = arg1;
}
// @ 0x08004DA4
void Chara_StartMoving(u8 arg0)
{
    Actor *ptr2E80;
    ptr2E80 = &gActors[arg0];
    ptr2E80->stateFlags |= 0x80;
    ptr2E80->stateFlags &= 0xDF;
    ptr2E80->stepTimer = 1;
    ptr2E80->cmdPc = 0;
    ptr2E80->field_11 = 0;
}
// @ 0x08004DD0
u8 Chara_AnyMoving(void)
{
    u8 i;
    Actor *ptr2E80;
    ptr2E80 = gActors;

    for (i = 0; i < 0x18; i++)
    {
        if (ptr2E80->sprNodeIdx != 0 && ptr2E80->stateFlags & 0x80)
            return 1;
        ptr2E80++;
    }

    return 0;
}
// @ 0x08004E04
void Party_SetFollowMode(void)
{
    gPartyFollowFlags |= 1;
}
/* 请求把 gfxId 这套图块装到精灵表槽 slot: 先记入 gSlotGfxId[], 再把单槽参数
 * (gPendingGfxSlot / gPendingGfxId) 写好并置 PENDING_SPRITE_GFX, 由 PendingSpriteLoad_Flush 延迟消费。 */
// @ 0x08004E14
void SetSlotGfxId(u8 slot, u16 gfxId)
{
    gSlotGfxId[slot] = gfxId;

    gPendingGfxSlot = slot;
    gPendingGfxId = gfxId;
    gPendingSpriteLoad |= PENDING_SPRITE_GFX;
}

/* 同上, 调色板版本(置 PENDING_SPRITE_PAL)。 */
// @ 0x08004E48
void SetSlotPalId(u8 slot, u16 palId)
{
    gSlotPalId[slot] = palId;
    gPendingPalSlot = slot;
    gPendingPalId = palId;
    gPendingSpriteLoad |= PENDING_SPRITE_PAL;
}
/* 当前待处理的精灵装载请求位图; 调用方用 PENDING_SPRITE_GFX / PENDING_SPRITE_PAL 测位。 */
// @ 0x08004E7C
u8 GetPendingSpriteLoad(void)
{
    return gPendingSpriteLoad;
}
// @ 0x08004E88
void Chara_SetPosDir(u8 arg0, s32 arg1, s32 arg2, u8 arg3)
{
    Actor *ptr2E80;
    ptr2E80 = &gActors[arg0];

    ptr2E80->x = arg1 * 8;
    ptr2E80->y = arg2 * 8;
    ptr2E80->field_11 = 0;
    ptr2E80->facingDir = arg3;
    ptr2E80->targetFacing = arg3;
}

// @ 0x08004EB8
u16 Chara_GetDrawZ(Actor *arg0)
{
    if (arg0->stateFlags & 1)
    {
        return arg0->z + Camera_GetDrawOffset();
    }

    return arg0->z;
}
// @ 0x08004EDC
s16 Chara_GetDrawX(Actor *arg0)
{
    if (arg0->stateFlags & 1)
    {
        switch (gCameraDrawMode)
        {
            case 2:
                return arg0->x - (gDrawCamX - 256);
            case 5:
                return arg0->x - (gDrawCamX - gCameraPosX);
        }
    }
    return arg0->x;
}
// @ 0x08004F3C
void Sprite_FreeChain(struct SpriteNode *arg0)
{

    struct SpriteNode *node;
    struct SpriteNode *next;

    if (arg0->flags == 0)
        return;

    arg0->flags = 0;

    node = arg0->next;

    if (node == 0)
        return;

    do
    {
        arg0->next = 0;
        arg0 = node;
        if (node->flags == 0)
            break;
        node->flags = 0;
        node = arg0->next;
    } while (node != 0);
}
// @ 0x08004F64
INCLUDE_ASM("asm/matchings", Sprite_WriteOam);
/* Sprite_WriteOam - 把一个精灵链节点渲染进 OAM 缓冲, 并推进 OAM 写入游标。
 *
 * 语义 (与 ROM 逐条对应, 已用 scripts/bytecmp.sh 定档):
 *   SpriteNode *Sprite_WriteOam(u16 *oamIdx, SpriteNode *node)
 *   {
 *       u16 idx = *oamIdx;
 *
 *       if ((s8)node->flags < 0)        // 负数 = 该节点不渲染 (flags 按有符号链计数用)
 *           return node->next;
 *       if (idx > 0x7F)                 // OAM 缓冲已满
 *           return node->next;
 *       gOamBuffer[*oamIdx].attrs[0] = node->attr0 + (node->attr1 << 16);
 *       gOamBuffer[*oamIdx].attrs[1] = node->attr2;
 *       *oamIdx = idx + 1;
 *       return node->next;              // 链尾为 NULL
 *   }
 *
 * 已确证的代码生成要点:
 *   - `(s8)node->flags < 0` 一击命中 `movs r0,#0; ldrsb r0,[r4,r0]` —— 该形态来自
 *     thumb.md *extendqisi2_insn 的 "地址就是裸寄存器" 分支 (见该分支的 mov+ldrsb 输出)。
 *     flags 在 iwram.h 里是 u8, 写成 u8 的 `< 0` 会被整条折掉 (规则 65), 必须显式 (s8)。
 *   - attrs[0] 用 `+` 不是 `|`: 目标是 `adds r1,r1,r0` (规则 36)。
 *   - 游标自增用的是**第一次**读到的 *oamIdx (r6), 而两处取地址各有一条独立 ldrh。
 *
 * 未合入的原因 (纯 C 复现不了, 已穷举 60+ 形态 + -O1/-Os/-g/-fno-gcse 变体):
 *   上面这份写法的 `idx` 会被 GCC2 CSE 折进第一处取地址 -> 少一条 `ldrh r2,[r5]`,
 *   并连带 r5/r6 互换 (44/68 字节差)。要阻止折叠, 在 cse.c 里只有
 *   `do_not_record`(volatile) 或"中间插入一次真正的 store"两条路, 本函数两者都没有。
 *   唯一逐字节一致的写法是把两处下标写成 `*(volatile u16 *)oamIdx` (bytecmp OK 68B),
 *   但那违反规则 79: 调用方 OAM_FlushFromQueue 传的是它自己的栈上 u16, 无异步共享语义,
 *   volatile 在这里纯粹是代码生成工具 -> 不合入, 保留 INCLUDE_ASM。
 */

// @ 0x08004FA8
void Chara_StartScriptAnim(u8 arg0, u8 arg1)
{
    Actor *ptr2E80;
    ptr2E80 = &gActors[arg0];
    ptr2E80->renderFlags |= 1;
    ptr2E80->field_14 = 0;
    ptr2E80->animIdx = arg1;
}
// @ 0x08004FD0
s32 Chara_AnimWaitDone(u8 arg0)
{
    Actor *ptr;

    if (arg0 < 0x64)
    {
        ptr = &gActors[arg0];

        gCutsceneAnimFlags[ptr->animIdx] &= 0x7F;

        if (ptr->field_14 > 0xFE)
            return 1;
    }
    else
    {
        ptr = &gUnk_03001EE0[arg0];
        ptr->field_14 = 0xFF;
        return 1;
    }

    return 0;
}
