#include "anim_slot.h"
#include "code_0.h"
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
#include "sound.h"

u8 *AnimSlot_Parse(u16 slot, u8 *src)
{
    u32 temp_r0;
    AnimSlot *anim;

    anim = &gAnimSlots[slot];

    anim->activeBank = (src[0] & 1) + 1;
    anim->flags = src[1];
    src += 2;

    anim->numFrames = *src;
    src++;

    anim->speedShift = *src;
    src++;

    anim->destTileX = *src;
    src++;

    anim->destTileY = *src;
    src++;
    anim->width = *src;
    src++;
    anim->height = *src;
    src++;

    anim->tickCounter = 0;
    anim->srcData = src;

    temp_r0 = anim->width * anim->height * anim->numFrames;
    temp_r0 *= 2;

    src += temp_r0;

    return src;
}
// @ 0x080079BC
u8 *AnimSlot_ParseLoop(u16 slot, u8 *src)
{
    u32 temp_r0;
    AnimSlot *anim;

    anim = &gAnimSlots[slot];

    anim->activeBank = (src[0] & 1) + 1;
    anim->flags = src[1];
    src += 2;

    anim->numFrames = *src;
    src++;

    anim->speedShift = *src;
    src++;

    anim->destTileX = *src;
    src++;

    anim->destTileY = *src;
    src++;
    anim->width = *src;
    src++;
    anim->height = *src;
    src++;

    anim->tickCounter = (anim->numFrames << anim->speedShift) - 2;
    anim->srcData = src;

    temp_r0 = anim->width * anim->height * anim->numFrames;
    temp_r0 *= 2;

    src += temp_r0;

    return src;
}
// 推进精灵动画槽 slot 的帧计数, 并把当前帧的图块拷进 0x02006000 图块缓存。
// 描述符布局见 anim_slot.h 的 AnimSlot:
//   activeBank = 模型库/状态(1/2, 0=空闲)  numFrames = 帧数      speedShift = 帧分频移位
//   flags = 标志(bit1=暂停, bit0=一次性)  destTileX/destTileY = 目标图块偏移
//   width/height = 单帧宽/高(字/行)  tickCounter = 帧计数器  srcData = 帧数据指针
// 行跳距 0x80 个 u16; 每帧大小 = width * height * 2 字节。
// 注: rows 在声明处的提前赋值是 GCC2 寄存器分配所需(该死 store 会被删除,
//     但决定字面池加载位置与 home 寄存器选择), 删掉则不匹配。
// @ 0x08007A1C
void AnimSlot_Step(s16 slot)
{
    AnimSlot *ptr;
    u16 frame;
    u8 *dest;
    u8 *src;
    u8 width;
    u8 rows = gAnimSlots[slot].height;
    u32 bankOff;
    u32 rowOff;

    ptr = &gAnimSlots[slot];
    if (ptr->activeBank == 0)
        return;
    if (ptr->flags & 2)
        return;
    ptr->tickCounter++;
    frame = ptr->tickCounter >> ptr->speedShift;
    if (frame >= ptr->numFrames)
    {
        if (ptr->flags & 1)
            gAnimSlots[slot].activeBank = 0;
        gAnimSlots[slot].tickCounter = 0;
        frame = 0;
    }
    if (gAnimSlots[slot].activeBank == 0)
        return;
    bankOff = (gAnimSlots[slot].activeBank - 1) << 15;
    rowOff = (gAnimSlots[slot].destTileY << 8) + 0x02006000;
    dest = (u8 *)(bankOff + rowOff + (gAnimSlots[slot].destTileX << 1));
    src = (u8 *)(gAnimSlots[slot].srcData + (gAnimSlots[slot].width * gAnimSlots[slot].height * frame * 2));
    rows = gAnimSlots[slot].height;
    while (rows != 0)
    {
        width = gAnimSlots[slot].width;
        while (width != 0)
        {
            *(u16 *)dest = *(u16 *)src;
            src += 2;
            dest += 2;
            width--;
        }
        dest += (0x80 - gAnimSlots[slot].width) * 2;
        rows--;
    }
}
// @ 0x08007ADC
INCLUDE_ASM("asm/nonmatchings", sub_8007ADC);
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
/* 选项场景 (世界观/存档界面?)整屏资源装载:
 * HuffUnComp gUnk_087EA0FC[arg0] → 0x02020000 → 0x06000000 (BG tile),
 * LZ77 gUnk_087EA110[arg0] → 0x0600E000 (SBB), gUnk_087EA124[arg0] → PLTT 0x80 半字;
 * 再装载公共 UI 图块 (0x0809Cxxx/0x0809Dxxx 五组) + 对话头像调色板/图块;
 * 配置 BG0-3/混合 (0x1E41/0x1F00), BgMap_FillPattern 铺底, TextBlocks_Render 按
 * gUnk_087E96B4[gChoiceGroupIdx] 渲染文本块, HBlank 水波开启, 最后 Logo_LoadAssets(arg0)
 * 装场景标志精灵, gChoiceSel = gChoiceSubIdx。 */
extern u8 *gUnk_087EA0FC[];
extern u8 *gUnk_087EA110[];
extern u8 *gUnk_087EA124[];
extern u8 *gUnk_087E96B4[];

extern u8 gChoiceSel;
extern u8 gChoiceSubIdx;

void Logo_LoadAssets(u8 sceneId);

// @ 0x08007D5C
void MapBg_LoadInterior(u8 sceneId)
{
    u16 *dest;
    u16 i;

    gVBlankPipelineMode = 3;
    HuffUnComp(gUnk_087EA0FC[sceneId], (void *)0x02020000);
    LZ77UnCompVram((void *)0x02020000, (void *)0x06000000);
    LZ77UnCompVram(gUnk_087EA110[sceneId], (void *)0x0600E000);
    DmaCopy16(3, gUnk_087EA124[sceneId], PLTT, 0x80 * 2);
    LZ77UnCompVram((void *)0x0809CB90, (void *)0x06008000);
    nullsub_5();
    LZ77UnCompVram((void *)0x0809D198, (void *)0x06009000);
    nullsub_5();
    LZ77UnCompVram((void *)0x0809D718, (void *)0x0600A000);
    nullsub_5();
    LZ77UnCompVram((void *)0x0809DCE8, (void *)0x0600B000);
    nullsub_5();
    DmaCopy16(3, 0x0809C834, 0x05000160, 0x30 * 2);
    nullsub_5();
    LZ77UnCompVram((void *)0x08095A1C, (void *)0x0600D000);
    LZ77UnCompVram((void *)0x08095C94, (void *)0x0600D400);
    LZ77UnCompVram((void *)0x08095F14, (void *)0x0600D800);
    LZ77UnCompVram((void *)0x0809619C, (void *)0x0600DC00);

    DmaCopy16(3, 0x08087216, PLTT, 2);

    REG_DISPCNT = (REG_DISPCNT & 0xE000) | 0x1560;
    REG_BG2CNT = 0x3C02;
    REG_BG3CNT = 0x3D01;
    REG_BG0CNT = 0x3F08;

    gBlendControl = 0x1E41;
    gBlendCoefficients = 0x1F00;

    DmaCopy16(3, 0x08393288, 0x0600C000, 0x200);
    DmaCopy16(3, 0x08393688, 0x05000140, 0x20);

    BgMap_FillPattern(0);
    DmaCopy16(3, 0x02005000, 0x0600F000, 0x800);

    dest = (u16 *)0x02005800;
    for (i = 0; i < 0x800; i++)
    {
        *dest++ = 0;
    }

    TextBlocks_Render(gUnk_087E96B4[gChoiceGroupIdx]);
    gHBlankEffectMode = 1;
    REG_BG1CNT = 0x1E0F;
    REG_DISPCNT |= 0x200;
    HBlankWave_BuildTables(1);
    Logo_LoadAssets(sceneId);
    gChoiceSel = gChoiceSubIdx;
}

/* 选项场景 LOGO/标志精灵装载 (sceneId==1 时按事件标志 0xFF 二态):
 *   sceneId==1 且 EventFlags_Test(0xFF): 双图块 (gUnk_080873BC / +0x144) + 双调色板 (-0x180/-0x160),
 *     两个精灵 gLogoSpriteNodes[0] (attr2=0x2A00), [1] (attr2=0x3A10);
 *   sceneId==1 未解锁: 单图块 gUnk_0808727C + 调色板 (gUnk_0808727C-0x60 = gUnk_0808721C),
 *     单精灵 tileOffsetY=0x20, gLogoAnimDirection/gLogoAnimTimer 清零 (呼吸动画复位);
 *   sceneId!=1: gLogoSpriteNodes 清空。
 * 代码生成要点: gfx 指针 ±偏移 形态 (r4 缓存 + adds r1, r4, r2) 才能命中目标的字面池内联布局,
 * 用独立符号 (gUnk_08087500 等) 会多出 4 个池导致 ROM 布局漂移。 */
extern const u16 gUnk_0808721C[];
extern const u16 gUnk_0808723C[];
extern const u16 gUnk_0808725C[];

extern const u8 gUnk_0808727C[];
extern const u8 gUnk_080873BC[];
extern const u8 gUnk_08087500[];
// @ 0x08007FB8
void Logo_LoadAssets(u8 sceneId)
{
    SpriteNode *sprNode;
    u8 idx;
    u16 attr0, attr1, attr2;

    if (sceneId == 1)
    {
        if (EventFlags_Test(0xFF) != 0)
        {
            LZ77UnCompVram(gUnk_080873BC, (void *)0x06014000);
            LZ77UnCompVram(gUnk_080873BC + 0x144, (void *)0x06014200);
            DmaCopy16(3, gUnk_080873BC - 0x180, (void *)0x05000240, 0x20);
            DmaCopy16(3, gUnk_080873BC - 0x160, (void *)0x05000260, 0x20);

            idx = Sprite_AllocNode();

            sprNode = &gSpriteNodePool[idx];
            gLogoSpriteNodes[0] = idx;
            attr0 = 0;
            attr1 = 0x8000;
            attr2 = 0x2A00;
            sprNode->tileOffsetX = 0;
            sprNode->tileOffsetY = 0;
            Sprite_InitChainNode(sprNode, 1, attr0, attr1, attr2);

            idx = Sprite_AllocNode();

            sprNode = &gSpriteNodePool[idx];
            gLogoSpriteNodes[1] = idx;
            attr1 = 0x8000;
            attr2 = 0x3A10;
            sprNode->tileOffsetX = 0;
            sprNode->tileOffsetY = 0;
            Sprite_InitChainNode(sprNode, 1, 0, attr1, attr2);
        }
        else
        {
            LZ77UnCompVram(gUnk_0808727C, (void *)0x06014000);
            DmaCopy16(3, gUnk_0808727C - 0x60, (void *)0x05000240, 0x20);

            idx = Sprite_AllocNode();
            sprNode = &gSpriteNodePool[idx];
            gLogoSpriteNodes[0] = idx;
            attr0 = 0;
            attr1 = 0x8000;
            attr2 = 0x2a00;
            sprNode->tileOffsetX = 0;
            sprNode->tileOffsetY = 0x20;

            Sprite_InitChainNode(sprNode, 1, attr0, attr1, attr2);

            gLogoAnimDirection = 0;
            gLogoAnimTimer = 0;
        }
    }
    else
    {
        gLogoSpriteNodes[0] = 0;
        gLogoSpriteNodes[1] = 0;
    }
}
/* gChoiceDataBase 声明见 include/data_805769C.h (const u8[]) */

// @ 0x08008124
u32 ChoiceMenu_BuildList(void)
{
    u8 *p;
    u8 i;

    p = (u8 *)gChoiceDataBase;
    /* 跳过 gChoiceGroupIdx 组记录: 每组含两个以 0xFF 结尾的字段 */
    for (i = 0; i != gChoiceGroupIdx; i = (u8)(i + 1))
    {
        if (*p != 0xFF)
        {
            do
            {
                while (*++p != 0xFF)
                    ;
                p++;
            } while (*p != 0xFF);
        }
        p++;
    }

    /* 再跳过 gChoiceSubIdx 组记录: 每组含一个以 0xFF 结尾的字段 */
    for (i = 0; i != gChoiceSubIdx; i = (u8)(i + 1))
    {
        if (*p != 0xFF)
        {
            while (*++p != 0xFF)
                ;
        }
        p++;
    }

    gChoiceListPtr = p;
    gChoiceCursor = 0;

    /* 走到本组记录末尾, 统计跳过的字节数 */
    i = 0;
    while (*p != 0xFF)
    {
        p++;
        i = (u8)(i + 1);
    }
    gChoiceListLen = i;
}

// @ 0x080081C0
void BattleIntro_Cursor(void)
{

    if (gLogoSpriteNodes[1] != 0)
    {
        Sprite_EnqueueRender(0xb0, 0x18, gLogoSpriteNodes[1], 0, 1);
        Sprite_EnqueueRender(0xD0, 0x30, gLogoSpriteNodes[0], 0, 1);
        return;
    }

    if (gLogoSpriteNodes[0] != 0)
    {
        if (gLogoAnimDirection == 0)
        {
            gLogoAnimTimer++;
            if (gLogoAnimTimer == 0xC0)
            {
                gLogoAnimDirection = 1;
            }
        }
        else
        {
            gLogoAnimTimer--;
            if (gLogoAnimTimer == 0)
            {
                gLogoAnimDirection = 0;
            }
        }

        Sprite_EnqueueRender(0x68, 0x30, gLogoSpriteNodes[0], (gLogoAnimTimer >> 6) + 0x10, 1);
    }
}
// @ 0x08008254
INCLUDE_ASM("asm/nonmatchings", ChoiceMenu_HandleInput);
/* Select one of the 88 portrait assets and seed its 8x8 dialogue tilemap. */
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

// @ 0x080088B4
void ScreenFade_Start(u16 flags, s16 step, s16 param)
{
    gScreenFadeParam = param;
    gScreenFadeStep = step;

    if (step < 0)
    {
        gScreenFadeProgress = 0x1B0;
    }
    else
    {
        gScreenFadeProgress = 0;
    }

    gScreenFadeFlags = flags;
}

// @ 0x080088F4
void AnimSlot_BankReload(void)
{
    if ((u32) * (u16 *)0x0300467C <= 0xFCU)
    {
        *(s32 *)0x03004620 = 0x02006000;
        *(s32 *)0x0300482C = 0x02001000;
        *(s32 *)0x03004690 = 0x02001800;
        *(s32 *)0x030047E8 = 0x02001C00;
        sub_80052F8();
        *(s32 *)0x03004620 = 0x0200E000;
        *(s32 *)0x0300482C = 0x02003000;
        *(s32 *)0x03004690 = 0x02003800;
        *(s32 *)0x030047E8 = 0x02003C00;
        sub_80052F8();
    }
}

/* 用 DMA0 把 gWindowTransitionScanlineTable 里的一个 WIN0H 值按扫描线喂给 REG_WIN0H,
 * 索引 = VCOUNT 的三角波 (0→81→0 跨一屏) → 窗口水平边界随扫描线摆动。
 *
 * ⚠ 全 ROM 无调用点 (code.s 里 6360 个 bl 全部已符号化, 没有 `bl Win0H_WaveDmaByVCount`;
 *   0x08008978 也未作为指针出现在任何数据里) → **死代码**。
 *   注: 旧名 HBlank_WaveDma / 旧注释"声音DMA0旋转" 是误读 —— 目的端 0x04000040
 *   是 REG_WIN0H (io.h: REG_OFFSET_WIN0H=0x40), 不是声音 FIFO (FIFO A/B 在 0xA0/0xAC)。
 */
// @ 0x08008978
void Win0H_WaveDmaByVCount(void)
{
    u16 waveIdx;
    u8 subState = (u8)(gScreenTransitionState - 1);
    if (subState > 1)
    {
        return;
    }
    /* VCOUNT 低字节 (字节读 = 目标的 ldrb, 不能写 (u8)REG_VCOUNT) */
    waveIdx = *(u8 *)&REG_VCOUNT;
    if (waveIdx > 0x9F)
    { /* > 159: 屏外扫描线 */
        if (waveIdx <= 0xE1)
        {
            return;
        }
        waveIdx = 0;
    }
    else
    {
        waveIdx = waveIdx + 1;
    }
    if (waveIdx > 0x51)
    { /* 三角波: 0x52 半周期 */
        waveIdx = waveIdx - 0x52;
    }
    else
    {
        waveIdx = 0x51 - waveIdx;
    }
    DmaSet(0, gWindowTransitionScanlineTable + (waveIdx * 2), (void *)REG_ADDR_WIN0H, 0xE0400001);
}
// @ 0x080089E0
void ScreenFx_SetMode(u16 mode)
{

    switch (mode)
    {
        case 1:
            gWindowTransitionProgress = 0;
            REG_WIN0H = DISPLAY_WIDTH;
            REG_WIN0V = DISPLAY_HEIGHT;
            REG_WININ = 0x3F;
            REG_WINOUT = 0;
            break;
        case 0:
            gWindowTransitionProgress = DISPLAY_WIDTH;
            break;
        default:
            PaletteFx_Apply(mode - 3);
            break;
    }

    gScreenTransitionState = mode;
}

// @ 0x08008A3C
void AnimSlots_Release(void)
{
    s16 i;

    for (i = 0; i < NUM_ANIM_SLOTS; i++)
    {
        gAnimSlots[i].activeBank = 0;
    }
}

// @ 0x08008A60
void AnimSlots_StepAll(void)
{
    s16 i;

    for (i = 0; i < 16; i++)
    {
        AnimSlot_Step(i);
    }
}
// @ 0x08008A80
void BgTiles_LoadUiSet(u8 skipFont)
{
    if (skipFont == 0)
    {
        LZ77UnCompVram((void *)0x0809C8B4, (void *)0x0600C800);
        nullsub_5();
    }
    LZ77UnCompVram((void *)0x0809CB90, (void *)0x06008000);
    nullsub_5();
    LZ77UnCompVram((void *)0x0809D198, (void *)0x06009000);
    nullsub_5();
    LZ77UnCompVram((void *)0x0809D718, (void *)0x0600A000);
    nullsub_5();
    LZ77UnCompVram((void *)0x0809DCE8, (void *)0x0600B000);
    nullsub_5();

    DmaCopy16(3, 0x0809C834, 0x05000160, 0x60);

    nullsub_5();
}
// @ 0x08008B14
void BgScroll_LoadFromTable(u16 mapIdx)
{

    gCameraMinX = gMapViewportBoundsTable[mapIdx].cameraMinXBlocks << 6;

    gCameraMinY = gMapViewportBoundsTable[mapIdx].cameraMinYBlocks << 6;

    gMapWidthPx = gCameraMinX + (gMapViewportBoundsTable[mapIdx].mapWidthBlocks << 6);

    gMapHeightPx = gCameraMinY + (gMapViewportBoundsTable[mapIdx].mapHeightBlocks << 6);
}
// @ 0x08008B5C
void PlayerSheets_Load(void)
{
    // gSlotGfxId.field_0 = gPartyMemberIds[0];
    gSlotGfxId[0] = gPartyMemberIds[0];
    gSlotPalId[0] = gPartyMemberIds[0];

    LoadSpriteSheetGfx(0, gPartyMemberIds[0]);
    LoadSpriteSheetPal(0, gPartyMemberIds[0]);
    // gSlotGfxId.field_1 = 11;
    gSlotGfxId[1] = 11;
    gSlotPalId[1] = 11;
    LoadSpriteSheetGfx(1, 0xBU);
    LoadSpriteSheetPal(1, 0xBU);
}
// 0x087EA1A0: 248 项指针表, 每项指向一组 "精灵动画模型" 记录
//   记录块格式: u16 count; 随后 count 条变长记录 (见 AnimSlot_Parse)

// 把 gUnk_087EA1A0[setId] 这一组动画模型 (共 *ptr 条) 逐条解析进
// gAnimSlots[] 精灵模型描述符数组, 起始槽位为 startSlot。
// 槽位号 = startSlot + 记录序号, 与 MapScene_LoadEventAnimations (整组装入槽位 0..) 是同族写法。
// @ 0x08008BA4
void AnimSlot_LoadSet(u8 setId, u8 startSlot)
{
    u8 *src;
    u16 endSlot;
    u16 slot;

    src = gUnk_087EA1A0[setId];
    endSlot = *(u16 *)src + startSlot;
    src += 2;

    for (slot = startSlot; slot < endSlot; slot++)
    {
        src = AnimSlot_Parse(slot, src);
    }
}

// @ 0x08008BE4
void AnimSlot_Pause(u8 slot)
{
    gAnimSlots[slot].flags |= ANIM_SLOT_FLAG_PAUSED;
}

// @ 0x08008BFC
void AnimSlot_Resume(u8 slot)
{
    gAnimSlots[slot].flags &= 0xFD;
}

// @ 0x08008C14
u8 AnimSlot_Active(u8 slot)
{
    return gAnimSlots[slot].activeBank;
}

/* 重载单个精灵表槽位: 图块用 gSlotGfxId[slot]、调色板用 gSlotPalId[slot],
 * 0xFF 表示该部分不动。整块受 gObjGraphicsSetId 的 bit7 屏蔽。 */
// @ 0x08008C24
void ReloadSpriteSheet(u8 slot)
{

    if (!(gObjGraphicsSetId & GFXSET_NO_SPRITE_LOAD))
    {
        if (gSlotGfxId[slot] != 0xFF)
        {
            LoadSpriteSheetGfx(slot, gSlotGfxId[slot]);
        }

        if (gSlotPalId[slot] != 0xFF)
        {
            LoadSpriteSheetPal(slot, gSlotPalId[slot]);
        }
    }
}

/* 全量重载全部 12 个精灵表槽位(0x06011400 + 12*0x900 = 0x06018000 正好到 VRAM 尾)。 */
// @ 0x08008C70
void ReloadAllSpriteSheets(void)
{
    u8 i;

    for (i = 0; i < 12; i++)
    {
        if (!(gObjGraphicsSetId & GFXSET_NO_SPRITE_LOAD))
        {
            if (gSlotGfxId[i] != 0xFF)
            {
                LoadSpriteSheetGfx(i, gSlotGfxId[i]);
            }
            if (gSlotPalId[i] != 0xFF)
            {
                LoadSpriteSheetPal(i, gSlotPalId[i]);
            }
        }
    }
}

extern const u8 gChoiceDestTable[];

/* 把"当前选项号"解析成一个目的地像素坐标, 写进 gChoiceDestX / gChoiceDestY。
 *
 * gChoiceDestTable @0x08087648 是**分组变长表**: 每组 = [count][count × {x, y}],
 * 组间无填充, 由 count 推出下一组起点 (count*2 是数据字节数, 再 +1 跳过 count 字节)。
 * 实测 5 组, count = 5/7/9/9/5, 共 35 个目的地, 消耗 75/76 字节 (末 1 字节为 0 终止)。
 * 值域 x∈12..200, y∈32..128 → 240×160 屏幕的**像素坐标**。
 *
 * 组号 = gChoiceGroupIdx (由 ChoiceMenu_BuildList 从 gChoiceDataBase 分层记录流定位);
 * 选项号 = 调用者传入 (Scene_EnterDoor 传 gChoiceListPtr[gChoiceCursor] 的低 nibble)。
 *
 * 代码生成要点 (已逐字节验证, bytecmp OK 88B):
 *   - 必须写成 `skipLen = *ptr << 1` 的**先读后自增**结构, 目标才是
 *     `ldrb; lsls #0x19; lsrs #0x18` + `adds r1,#1` 的形态
 *   - 循环里 `ptr += skipLen; i++; skipLen = *ptr<<1; ptr++;` 的顺序不能调
 *     (目标把 ptr++ 放在读 count 之后, 与 while 的底部测试配合)
 *   - 取项写成 `ptr + (arg0 << 1)` 再 `ptr[0]`/`ptr[1]`, 不要合并成 `ptr[arg0*2]`
 */
// @ 0x08008CC0
void ChoiceMenu_ResolveDest(u8 choiceIdx)
{
    const u8 *ptr;
    u8 i;
    u8 skipLen;

    ptr = gChoiceDestTable;
    i = 0;
    skipLen = *ptr << 1;
    ptr++;

    while (i != gChoiceGroupIdx)
    {
        ptr += skipLen;
        i++;
        skipLen = *ptr << 1;
        ptr++;
    }

    ptr = ptr + (choiceIdx << 1);
    gChoiceDestX = ptr[0];
    gChoiceDestY = ptr[1];
}
// }

// @ 0x08008D18
void DialogPortrait_FlushPending(void)
{
    u32 i;
    u16 *dest;

    if (gPendingPortraitSlot != 0)
    {

        LZ77UnCompVram(gPendingPortraitGfx, (void *)(((gPendingPortraitSlot - 1) >> 1) * 0x800 + 0x0600D000));

        i = (gPendingPortraitSlot - 1) >> 1;

        dest = (u16 *)(i * 0x20 + 0x050001C0);
        DmaCopy16(3, gPendingPortraitPalette, dest, 0x20);

        gPendingPortraitSlot = 0;
    }
}

// @ 0x08008D78
u16 Camera_GetDrawOffset(void)
{
    switch (gCameraDrawMode)
    {
        case 2:
            return gDrawCamY >> 4;

        case 4:
            return gDrawCamY - 0x20;

        case 7:
            return gDrawCamY - gCameraPosY;

        default:
            return 0;
    }
}

// @ 0x08008DCC
void Script_SetEnvSet(u8 songId)
{
    gEnvScriptSetId = songId;
}

// @ 0x08008DD8
void BgPal_ResetFirst(void)
{
    DmaCopy16(3, (void *)0x08087216, (void *)0x05000000, 2);
}

// @ 0x08008DF8
void AnimSlot_PlayOnce(u16 slot, u8 *data)
{
    u16 count;

    count = *(u16 *)data;
    data += 2;

    while (count != 0)
    {
        data = AnimSlot_ParseLoop(slot, data);

        AnimSlot_Step(slot);

        gAnimSlots[slot].activeBank = 0;
        count--;
    }
}
// @ 0x08008E44
void BgMap_FillRow(u8 mode)
{
    u16 *dest;
    u16 i, j;
    u16 val;

    dest = (u16 *)0x020053A8;

    val = 32;

    if (mode != 0)
    {
        val = 0;
    }

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 8; j++)
        {
            *dest = val + 0xA200;
            dest++;
            val++;
        }
        dest += 24;
    }
    gViewportFlags[13] = 1;
}

// @ 0x08008E94
void MapBg_FlushPending(void)
{
    switch (gIntroBgTransferStage)
    {
        case 1:
            switch (gIntroBgTileSetIndex)
            {
                case 0:
                    DmaCopy32(3, (void *)0x02020000, (void *)0x06000000, 0xc00);
                    break;

                case 1:
                    DmaCopy32(3, (void *)0x02020000, (void *)0x06000C00, 0xc0);
                    break;
            }
            gIntroBgTransferStage = 0;
            break;

        case 2:
            DmaCopy32(3, (void *)0x02020000, (void *)0x0600E000, 0x800);
            gIntroBgTransferStage = 0;
            break;
    }
}
/* 按当前地图从 0x08088400 的 256 项表中装载宝箱对象；见 ChestSpawnEntry。 */
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
// @ 0x0800908C
