#include "option_scene_loader.h"

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
