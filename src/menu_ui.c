#include "code_0.h"
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

INCLUDE_ASM("asm/nonmatchings", sub_800ACC8);

// @ 0x0800B14C
void SceneBg_Reload(void)
{
    if (gUnk_03004D4C != 0)
    {
        if ((u8)(gUnk_03004D4C - 7) <= 4)
        {
            sub_8016068();
            return;
        }
        if (gUnk_03004D4C == 12)
        {
            sub_80160F4();
            MenuState_Reset();
            return;
        }
        if ((u8)(gUnk_03004D4C - 0x35) <= 7)
        {
            ReloadSpriteSheet(gUnk_03004D4C + 0xCD);
            return;
        }

        DmaCopy16(3, (void *)0x02005800, (void *)0x0600F800, 0x800);
        switch (gUnk_03004D4C)
        {
            case 2:
                LZ77UnCompVram((void *)0x08095A1C, (void *)0x0600D000);
                break;
            case 3:
                LZ77UnCompVram((void *)0x08095C94, (void *)0x0600D400);
                break;
            case 4:
                LZ77UnCompVram((void *)0x08095F14, (void *)0x0600D800);
                break;
            case 5:
                LZ77UnCompVram((void *)0x0809619C, (void *)0x0600DC00);
                break;
        }
        return;
    }

    if (gUnk_03004D40 != 0)
    {
        if ((u8)(gUnk_03004D40 - 3) <= 4)
        {
            sub_8016068();
        }
        else if (gUnk_03004D40 == 2)
        {
            sub_80160F4();
            MenuState_Reset();
        }
        else if ((u8)(gUnk_03004D40 + 0xF) <= 7)
        {
            ReloadSpriteSheet(gUnk_03004D40 + 0x11);
        }
        else
        {
            DmaCopy16(3, (void *)0x02005800, (void *)0x0600F800, 0x800);
            switch (gUnk_03004D40)
            {
                case 0x1F:
                    LZ77UnCompVram((void *)0x08095A1C, (void *)0x0600D000);
                    break;
                case 0x20:
                    LZ77UnCompVram((void *)0x08095C94, (void *)0x0600D400);
                    break;
                case 0x21:
                    LZ77UnCompVram((void *)0x08095F14, (void *)0x0600D800);
                    break;
                case 0x22:
                    LZ77UnCompVram((void *)0x0809619C, (void *)0x0600DC00);
                    break;
            }
        }
        return;
    }

    if (gObjGraphicsSetId != 0xFF)
    {
        DmaCopy16(3, (void *)0x02004000, (void *)0x0600E000, 0x800);
        if (gObjGraphicsSetId != 0xFE)
            DmaCopy16(3, (void *)0x02004800, (void *)0x0600E800, 0x800);
    }
}
// @ 0x0800B2D0
void MenuState_Reset(void)
{
    u8 i;
    gMenuCursorGrp = 0;
    gMenuCursorSel = 0;

    gUnk_03000048.field_0 = 1;
    gUnk_03000048.field_1 = 0;
    gUnk_03000048.field_2 = 0;
    gUnk_03000048.field_3 = 0;
    gUnk_03000048.field_4 = 0x18;
    gUnk_03000048.field_6 = 8;

    for (i = 0; i < 16; i++)
    {
        gMenuCursorStack[i] = 0;
    }
}
// @ 0x0800B314
void MenuHp_Update(void)
{
    u8 i;

    if (gUnk_03000185 == 0)
        return;

    if (gUnk_03000184 == 4)
    {
        Menu_GetFocus();
        for (i = 0; i < 5; i++)
        {
            if (gPartyMemberIds[i] != 0xFF)
            {
                u8 y = i * 5 + 5;
                sub_80161F4(gPartyMemberIds[i], y, 5);
                Hud_DrawLv(gPartyMemberIds[i], y, 6);
            }
        }
    }

    gUnk_03000185 = 0;
}

// @ 0x0800B374
INCLUDE_ASM("asm/nonmatchings", sub_800B374);

/* 把一条消息 (字节编码字符串) 解码成一整行瓦片, 写进单行缓冲 gMsgLineBuf (u16[29])。
 *
 * 消息编码:
 *   0xFF = 结束
 *   0xFE = 转义前缀, 后跟一个高位字节 → 16 位码 ((hi << 8) | 0xFE)
 *   其它 = 直接作为 16 位字符/瓦片码
 * 输出布局: [0xC8 左边框] [N 个内容码] [0xC9 右边框] [0 补齐] —— 共 29 项 (count 到 0x1C)。
 * 边框与补齐固定用调色板 0xB (默认调色板, 见 Msg_ShowById 的实参),
 * 内容用调用者传的 palette。
 *
 * 调用者: Msg_ShowById (src/code_8010F10.c) 先在 gMsgTable 块里跳过 target 个 0xFF
 * 定位到第 target 条消息, 再交给本函数。Text_PutGlyph = 写一个 u16 瓦片项。
 *
 * 代码生成要点 (已逐字节验证):
 *   - `count` 必须是 **u16**: 自增生成 `adds r0,r6,#1; lsls #0x10; lsrs #0x10` (u16 截断)
 *   - 转义分支用 `|` 不用 `+`: 目标是 `orrs r1, r0` (规则 36)
 *   - 两个分支各自重复写一次 Text_PutGlyph 调用, **不能外提** (规则 38): 目标就是两份调用点
 *   - `dst++` 在目标里是 `adds r0,r5,#0; adds r5,#2` (先传后推), 不要写成 `*dst++` 以外的形式
 */
// @ 0x0800BEE4
void Msg_RenderLine(u8 *src, u8 palette)
{
    u16 *dst = gMsgLineBuf;
    u16 count;

    Text_PutGlyph(dst++, 0xC8, 0xB);
    count = 1;
    while (*src != 0xFF)
    {
        u16 ch = *src++;

        if (ch == 0xFE)
        {
            ch = (*src++ << 8) | 0xFE;
            Text_PutGlyph(dst++, ch, palette);
        }
        else
        {
            Text_PutGlyph(dst++, ch, palette);
        }
        count++;
    }

    Text_PutGlyph(dst++, 0xC9, 0xB);
    count++;
    while (count <= 0x1C)
    {
        Text_PutGlyph(dst++, 0, 0xB);
        count++;
    }
}
/* 重建 HUD 队伍精灵实体表 (gUiSprites, 15 项)。
 *
 * 前 5 项 = 队伍成员: 若 gPartyMemberIds[i] != 0xFF 则启用,
 *   x = i*40 + 0x48, y = 8, statusFlags = 0x80 (激活),
 *   field_10 = 基础图块起始 ID = i*48 + 0x200, oamSlotId = i + 0x71 (关联的渲染层 OAM 索引)
 * 其余项与空队伍槽一律清 x/y/statusFlags/oamSlotId;
 * 15 项都额外重置 animTimer = 0 与 lerpFrame = 0 (公共重置)。
 *
 * mode == 0 时多做一步: 调 MenuUi_SpawnAuxSprites(0) 并给实体[5] 的 statusFlags 置 bit3。
 *   (实体[5] 即 gUiSpritesAux, 偏移 0x64 = 5*20, 是第一个非队伍槽)
 * 调用点: sub_800ACC8 传 0, sub_801417C 传 1。
 *
 * 代码生成要点 (已逐字节验证):
 *   - `i` 必须是 **u16**: 自增生成 `adds; lsls #0x10; lsrs #0x10` (规则 68 同类)
 *   - 启用分支的乘法要写成 `i * 40` / `i * 48` 这种十进制常量:
 *     GCC2 拆成 `lsls #2; adds i; lsls #3` (= i*5*8) 与 `lsls #1; adds i; lsls #4` (= i*3*16),
 *     与目标一致; 换成移位形式或先算指针会改变指令数
 *   - 末尾必须写 `gUiSprites[5].statusFlags |= 8`, **不能**用 `gUiSpritesAux` 符号:
 *     目标是 `ldr r0,=0x03000058; adds r0,#0x64` (复用同一池项),
 *     用独立符号会改成 `ldr r0,=0x030000BC` 并多出一个字面池项 (实测多 20 字节)
 *   - `|=` 目标是 `movs r1,#8; orrs r1, r2` (先物化常量再或, 规则 5/76 同类)
 */
// @ 0x0800BF5C
void PartyUi_InitEntities(u8 mode)
{
    u16 i;

    for (i = 0; i < 15; i++)
    {
        if (i < 5 && gPartyMemberIds[i] != 0xFF)
        {
            gUiSprites[i].x = i * 40 + 0x48;
            gUiSprites[i].y = 8;
            gUiSprites[i].statusFlags = 0x80;
            gUiSprites[i].baseTileId = i * 48 + 0x200;
            gUiSprites[i].oamSlotId = i + 0x71;
        }
        else
        {
            gUiSprites[i].x = 0;
            gUiSprites[i].y = 0;
            gUiSprites[i].statusFlags = 0;
            gUiSprites[i].oamSlotId = 0;
        }

        /* 公共重置: 15 项全部清动画计时器与插值帧 */
        gUiSprites[i].animTimer = 0;
        gUiSprites[i].lerpFrame = 0;
    }

    if (mode == 0)
    {
        MenuUi_SpawnAuxSprites(0);
        gUiSprites[5].statusFlags |= 8;
    }
}
// @ 0x0800BFF8
//
// 三位十进制数字写入器 (0–999): 把 value 渲染成 3 个 BG 图块项, 右对齐, 前导零抑制
// (个位必显示; 百/十位为前导零时写空白瓦片)。写入方向朝 dest 的负方向:
//   tiles[0]=百位 写 dest-2, tiles[1]=十位 写 dest-1, tiles[2]=个位 写 dest。
//
// value: 要显示的数值 (调用方只传 HP/MP/Lv, 恒非负; 负值时商下溢会拼出空白前一格的乱码瓦片)
// dest:  tilemap 缓冲 (0x02005800, 行距 32) 中【个位】格子的地址
// base:  图块项基值 = (调色板<<12) | 数字瓦片基号; 数字 d 的表项 = base+0x25A+d,
//        空白瓦片 = base+0x27F。HP/MP 满值时调用方传 0xF000 换高亮调色板, 否则 0xB000。
//
// ⚠ 原始 ROM 缺陷 (字节级匹配保真): 十位空白判定硬编码 tiles[0]==0xB27F 而没跟 base 走,
//   base=0xF000 且数值形如 5/50 时百/十位空白判定失效, 渲染成 "005" 而非 "  5"。
void sub_800BFF8(s16 value, u16 *dest, u32 base)
{
    u16 tiles[3];
    u16 base16;
    u16 count;
    u16 *d;

    base16 = base;
    if (value == 0)
    {
        tiles[0] = base16 + 0x27F;             // 空白
        tiles[1] = base16 + 0x27F;             // 空白
        tiles[2] = base16 + 0x27F - 0x25;      // 空白-0x25 = '0'
        *dest = tiles[2];
        dest--;
    }
    else
    {
        count = 0;
        d = dest - 1;
        while (value >= 0)                     // 减法除法: value / 100 → count (百位)
        {
            value -= 100;
            count++;
        }
        value += 100;                          // 回退最后一次多减的 100 → 余数
        count--;
        if (count == 0)
            count = 0x25;                      // 百位为 0 → 空白瓦片 (表中 0x25A+0x25 处)
        tiles[0] = base16 + (0x25A + count);
        count = 0;
        while (value >= 0)                     // 减法除法: value / 10 → count (十位)
        {
            value -= 10;
            count++;
        }
        value += 10;                           // 余数 = 个位
        count--;
        if (count == 0 && tiles[0] == 0xB27F)  // ⚠ 硬编码 0xB27F (=0xB000 基的空白): base=0xF000 时失效
            count = 0x25;                      // 十位为 0 且百位空白 → 十位也空白 (前导零抑制)
        tiles[1] = (u16)(base16 + 0x25A) + count;
        tiles[2] = (u16)(base16 + 0x25A) + value;
        *dest = tiles[2];
        dest = d;
    }
    *dest = tiles[1];
    dest--;
    *dest = tiles[0];
}

// @ 0x0800C0D8
void BattleIntro_Setup(void)
{
    u16 i;
    u16 attr0;
    u16 attr1;
    u16 attr2;
    struct SpriteNode *obj = (struct SpriteNode *)0x03004380;

    attr0 = gUnk_03000048.field_6;
    attr1 = ((gUnk_03000048.field_4 - 0x20) & 0x1FF);
    attr1 += 0x8000;
    attr2 = 0x21C0;
    Sprite_InitChainNode(obj, 1, attr0, attr1, attr2);

    gSpriteRenderQueue[0] = obj;

    CutsceneAnim_Load(0x4C, 0, 9);

    for (i = 0; i < 5; i++)
    {
        obj++;
        if (gPartyMemberIds[i] != 0xFF)
        {
            attr1 = 0x8028 + ((i * 5) << 3);
            attr2 = ((i + 3) << 12) + ((i * 3 * 16 + 0x200) & 0x3FF);
            Sprite_InitChainNode(obj, 1, 8, attr1, attr2);
            gSpriteRenderQueue[i + 1] = obj;
        }
        Chara_InitDialogArrow(i);
    }
}
// @ 0x0800C194
INCLUDE_ASM("asm/nonmatchings", sub_800C194);
// @ 0x0800C2F8
INCLUDE_ASM("asm/nonmatchings", sub_800C2F8);

// @ 0x0800E170
void MenuUi_SetEntityPos(u8 arg0, u8 arg1, u8 arg2)
{

    switch (arg0)
    {
        case 0:
            gUiSprites[arg2].x = arg2 * 0x28 + 0x48;
            gUiSprites[arg2].y = 8;
            break;
        case 1:
            gUiSprites[arg2].x = 0x28;
            gUiSprites[arg2].y = 8;

            break;
        case 2:
            gUiSprites[arg2].x = 0x30;
            gUiSprites[arg2].y = 16;
            break;
    }
    switch (arg1)
    {
        case 2:
            gUiSprites[arg2].statusFlags &= 0xBF;
            gUiSprites[arg2].statusFlags &= 0xFE;
            break;
        case 1:
            gUiSprites[arg2].statusFlags &= 0xBF;
            gUiSprites[arg2].statusFlags |= 1;
            break;
        case 0:
            gUiSprites[arg2].statusFlags |= 0x40;
            gUiSprites[arg2].statusFlags &= 0xFE;
            break;
    }
}

// @ 0x0800E244
INCLUDE_ASM("asm/nonmatchings", sub_800E244);

typedef struct
{
    u16 x;
    u16 y;
} Vec2;

extern Vec2 *gUnk_087EB1F4[];
extern Vec2 *gUnk_087EB214[];
extern Vec2 *gUnk_087EB22C[];

// @ 0x0800E668
void sub_800E668(u8 arg0)
{
    Vec2 *ptr;

    if (arg0 != 0xFF)
    {
        gMenuCursorStack[gMenuCursorGrp] = gMenuCursorSel;
        gMenuCursorGrp = arg0;
        gMenuCursorSel = gMenuCursorStack[gMenuCursorGrp];
        sub_8010624(0xFF, 0);
        gUnk_03000185 = 1;
    }

    gUnk_03000048.field_C = gUnk_03000048.field_4;
    gUnk_03000048.field_E = gUnk_03000048.field_6;

    if (gTitleIntroState == TITLE_INTRO_DISABLED)
    {
        if (gUnk_03004D40 == 0)
        {
            ptr = &gUnk_087EB1F4[gMenuCursorGrp][gMenuCursorSel];
        }
        else
        {
            ptr = &gUnk_087EB22C[gMenuCursorGrp][gMenuCursorSel];
        }
    }
    else
    {
        ptr = &gUnk_087EB214[gMenuCursorGrp][gMenuCursorSel];
    }

    gUnk_03000048.field_8 = ptr->x;
    gUnk_03000048.field_A = ptr->y;

    gUnk_03000048.field_2 = 8;
    gUnk_03000048.field_0 &= 0xFE;
}

// @ 0x0800E71C
INCLUDE_ASM("asm/matchings", UiSprite_BeginSlide);
/*
struct Vec2
{
    u16 x;
    u16 y;
};
struct MenuCharacter {
    u8 r;      // 0x00 标志位，bit7 可能控制显示
    u8 frameCounter;  // 0x01 帧计数器
    u8 unk2;          // 0x02 动画速度或状态
    u8 unk3;          // 0x03 未知
    u16 pos0_x;        // 0x04 当前位置 X
    u16 pos0_y;        // 0x06 当前位置 Y
    u16 pos1_x;        // 0x08 目标位置 X（可能用于平滑移动）
    u16 pos1_y;        // 0x0A 目标位置 Y
    u16 pos2_x;        // 0x0C 旧位置 X（备份）
    u16 pos2_y;        // 0x0E 旧位置 Y
    u32 unk10;        // 0x10 未知
};

extern struct MenuCharacter gMenuCharacters[];

extern  u8  gPartyMemberIDs[];   // 0x03004AA0
extern  u8  gFormationIDs[]; // 0x03004A88, 长度为5
extern u16 gPositionTable[];      // 0x08098418, 半字表

UI 精灵实体滑动设定 (UiSprite_BeginSlide, asm 已匹配; 下方为等价 C 草稿):
   当前 x/y 存为移动起点 moveStart(0x0C/0x0E), 按模式算终点 moveEnd(0x08/0x0A),
   lerpFrame=8 启动插值 (UiSprites_Update 每帧消费)。
   mode 1: 终点 (0x28, 8);  mode 0: 终点 (idx*0x28+0x48, 8);
   其他:   按 gPartyMemberIDs[idx] 在编队 gFormationIDs[5] 里的位次 +2 查
           gPositionTable 得 (x+0x18, y+8)。
// @ 0x0800E71C
void UiSprite_BeginSlide(u8 idx, u8 mode) {
    UISpriteEntity* sprite;
    u8 ch_id;
    u8 i;
    u16 tbl_idx;
    u16 x, y;

    if(idx == 0xFF)
        return;

    sprite = &gUiSprites[idx];

    sprite->moveStartX = sprite->x;
    sprite->moveStartY = sprite->y;


    switch(mode)
    {
        case 1:
            x = 40;
            y = 8;
            break;

        case 0:
            x = idx * 40 + 72;
            y = 8;
            break;

        default:
            ch_id = gPartyMemberIDs[idx];

            for ( i = 0; i < 5; i++) {
                if (gFormationIDs[i] == ch_id) {
                    break;
                }
            }

            tbl_idx = (i + 2) ;
            x = gPositionTable[tbl_idx << 1 ] + 0x18;
            y = gPositionTable[(tbl_idx << 1) + 1] + 8;
            break;

    }

    sprite->moveEndX = x;
    sprite->moveEndY = y;
    sprite->lerpFrame = 8;
}
*/
/* 菜单/UI 精灵实体的逐帧更新 (由 sub_8014488 与 sub_800C194 每帧调用):
 *   1. 遍历 15 个 gUiSprites (bit7=激活):
 *      - lerpFrame != 0: 8 步定点点插值 (LERP_POS, asrs#3), 走完贴合 moveEnd;
 *      - baseTileId 选片: bit0 动画模式 (animTimer bit3 翻转选 +0/0x10 半片),
 *        bit1 固定 +0x60 变体, 其余直用 baseTileId;
 *      - 把 x/y 写进关联 SpriteNode (gSpriteNodePool[oamSlotId]) 的 OAM 属性:
 *        attr0 <- y (低 8 位), attr1 <- x-0x20 (低 9 位), attr2 <- tileId
 *        (bit2/3 变体: 从 0x5000/0x6000 窗口字库行选块, 直接清低 10 位加 tileId)。 */
// @ 0x0800E7BC
void UiSprites_Update(void)
{
    u8 step;
    u16 i;

    u16 dx, dy;
    u16 tileId;

    UISpriteEntity *sprite;
    struct SpriteNode *node;

    sprite = gUiSprites;

    for (i = 0; i < 15; i++, sprite++)
    {
        if (!(sprite->statusFlags & 0x80))
            continue;

        if (sprite->lerpFrame != 0)
        {
            sprite->lerpFrame--;
            if (sprite->lerpFrame != 0)
            {
                dx = sprite->moveEndX - sprite->moveStartX;
                dy = sprite->moveEndY - sprite->moveStartY;
                step = 8 - sprite->lerpFrame;

                sprite->x = LERP_POS(sprite->moveStartX, dx, step);
                sprite->y = LERP_POS(sprite->moveStartY, dy, step);
            }
            else
            {
                sprite->x = sprite->moveEndX;
                sprite->y = sprite->moveEndY;
            }
        }

        if (sprite->statusFlags & 0x1)
        {
            if (sprite->statusFlags & 0x2)
            {
                tileId = sprite->baseTileId + 0x60;
            }
            else
            {
                sprite->animTimer++;
                tileId = ((sprite->animTimer & 8) << 1) + sprite->baseTileId;
            }
        }
        else
        {
            tileId = sprite->baseTileId;
        }

        node = &gSpriteNodePool[sprite->oamSlotId];

        SET_OAM_Y(node->attr0, sprite->y);
        SET_OAM_X(node->attr1, sprite->x - 0x20);

        if (sprite->statusFlags & 0x04)
        {
            if (sprite->statusFlags & 0x08)
            {
                node->attr2 = (node->attr2 & OAM2_PRIORITY) + tileId - 0x6000;
            }
            else
            {
                node->attr2 = (node->attr2 & OAM2_PRIORITY) + tileId - 0x5000;
            }
        }
        else
        {
            node->attr2 = (node->attr2 & 0xFC00) + tileId;
        }
    }
}
// @ 0x0800E8F8
INCLUDE_ASM("asm/nonmatchings", sub_800E8F8);
// @ 0x0800EAE4
INCLUDE_ASM("asm/nonmatchings", sub_800EAE4);

extern u8 *gUiSpritesAuxDesc[];

/* 菜单辅助精灵生成 (PartyUi_InitEntities mode==0 与 sub_800E244 调用):
 * src = gUiSpritesAuxDesc[arg0]; count = *src++;
 * 前 count 项写 gUiSpritesAux[i]: {u8 x→field_4, u8 y→field_6, statusFlags=0x84,
 *   baseTileId = 0x380 + byte3, oamSlotId = i+0x76 (SpriteNode 池槽), animTimer/lerpFrame = 0},
 * 并 Sprite_InitChainNode(&gSpriteNodePool[oamSlotId], 1, y, x|0x4000, 0xB000|(tile&0x3FF))
 *   (attr0=y 局部变量缓存跨越中间存储 → 目标 r2 存活到 mov ip, r2; attr1 的 0x4000 = X 翻转)。
 * 余下 i..9 清 statusFlags = 0。 */
/* 菜单辅助精灵生成 (待匹配; 语义已完整还原, 卡点 = 零常量/常量池的寄存器分配选择):
 *   src = gUiSpritesAuxDesc[arg0]; count = *src++;
 *   前 count 项写 gUiSpritesAux[i]: {u8 x→field_4, u8 y→field_6, statusFlags=0x84,
 *   baseTileId = 0x380 + byte3, oamSlotId = i+0x76, animTimer/lerpFrame = 0},
 *   并 Sprite_InitChainNode(&gSpriteNodePool[oamSlotId], 1, y, x|0x4000, 0xB000|(tile&0x3FF))
 *   (attr0 = y 经 ip 传递, r2 从 ldrb 起跨整个循环体存活; attr1 的 0x4000 = X 翻转)。
 *   余下 i..9 清 statusFlags = 0。
 *   调用点: PartyUi_InitEntities (mode==0), sub_800E244。
 * 卡点: 目标零常量分配到 sb (`movs r2,#0; mov sb,r2`), 0x4000/0x3FF 常量直接经 r2 物化;
 * 本侧分配到 r9 且常量走 ip 中转 (多余 mov 对) —— 需 permuter 探索声明顺序。
 *
// @ 0x0800EB98
void MenuUi_SpawnAuxSprites(u8 arg0) {
    u8 y;
    u8* src;
    u8 count;
    u8 i;
    UISpriteEntity* obj;

    src = gUiSpritesAuxDesc[arg0];
    count = *src++;
    obj = gUiSpritesAux;

    for(i = 0; i < count; i++)
    {
        obj->x = *src++;
        y = *src;
        obj->y = y;
        src++;
        obj->statusFlags = 0x84;
        obj->baseTileId = (0xE0 << 2) + (*src++);
        obj->oamSlotId = i + 0x76;
        obj->animTimer = 0;
        obj->lerpFrame = 0;

        Sprite_InitChainNode(&gSpriteNodePool[obj->oamSlotId], 1, y, obj->x | 0x4000, (obj->baseTileId & 0x3FF) | 0xB000);
        obj++;
    }

    while(i < 10)
    {
        obj->statusFlags = 0;
        obj++;
        i++;
    }
}
*/
// @ 0x0800EB98
INCLUDE_ASM("asm/matchings", MenuUi_SpawnAuxSprites);

// @ 0x0800EC54
INCLUDE_ASM("asm/nonmatchings", sub_800EC54);
// @ 0x0800F128
INCLUDE_ASM("asm/nonmatchings", sub_800F128);
// @ 0x0800F3AC
/* 屏幕空闲时地点图标列绘制 (主菜单待机画面):
 *   1. ClearBuffer 清 0x02005986 的 24x10 瓦片缓冲 (0xB001 填充, 行距 0x40);
 *   2. 遍历 gScreenIdleIconIds[i + 游标] (i=0..4, 0=无图标跳过);
 *      调色板: 选中项 (gMenuCursorSel-11 == i) = 13 否则 11;
 *      特殊图标再改写: 图标 8 且 EventFlags_Test(0x10D)==0 → 12,
 *                      图标 0x18 且 EventFlags_Test(0xFF)!=0 → 12;
 *   3. off = i*0x80 + 0x180: 写 0xB190 到 0x02005810+off, 0xB191 到 0x02005850+off
 *      (图标框瓦片), 再 Msg_DrawPoolSegment(0x02005812+off, 图标ID, 调色板) + Text_PutGlyph(0xC9, 11)。
 * 匹配要点 (逐字节验证):
 *   - off 用 u32 两步赋值 (off = i<<7; off += 0x180) 防 GCC 合并移位 (lsls #23 形态);
 *   - tileOff/iconId 用 int 局部 (声明序: ptr,tileOff,i,iconId,segIdx) 支配 ClearBuffer
 *     展开 w→r7 / 0xB001→r1 的 global-alloc home;
 *   - tileOff = 0x190 单独成句, 使 *ptr = base + tileOff 走 add r1,r8 (防 ORR 重写);
 *   - segIdx = iconId 放在 tile2 存储之后: 阻断 0x02005812 = 0x02005850-0x3E 的 CSE,
 *     且让 iconId 拷贝 (adds r1,r4,#0) 落到 strh 之后 (与目标调度一致)。
 *   (EXPERIENCE 87 变体: 一个 int 临时变量同时买到拷贝指令位置与伪寄存器生死边界) */
void sub_800F3AC(void)
{
    u16 *ptr;
    int tileOff;
    u8 i;
    int iconId;
    u8 segIdx;
    u16 base;
    u16 tile2;

    ClearBuffer((u16 *)0x02005986, 0x18, 0xA);

    i = 0;
    base = 0xB000;
    tile2 = 0xB191;

    for (; i < 5; i++)
    {
        u8 item = gScreenIdleIconIds[i + gScreenIdleIconCursor];
        iconId = item;
        if (iconId != 0)
        {
            u32 palette = ((gMenuCursorSel - 11) == i) ? 13 : 11;
            u32 off;

            switch (iconId)
            {
                case 8:
                    if (EventFlags_Test(0x10D) == 0)
                        palette = 12;
                    break;
                case 0x18:
                    if (EventFlags_Test(0xFF) != 0)
                        palette = 12;
                    break;
            }

            off = i << 7;
            off += 0x180;
            tileOff = 0x190;
            ptr = (u16 *)(0x02005810 + off);
            *ptr = base + tileOff;
            ptr = (u16 *)(0x02005850 + off);
            *ptr = tile2;
            segIdx = iconId;
            Text_PutGlyph(Msg_DrawPoolSegment((u16 *)(0x02005812 + off), segIdx, palette), 0xC9, 11);
        }
    }
}

extern u8 gUnk_03000199;
extern u8 gUnk_030001A0[];
extern u8 gUnk_03004980[];

extern const u8 gUnk_08095028[][8]; // 物品/名称字符串表 (data_805769C.c / blob), [id] = 8 字符名

static inline void drawSome(u8 var_r7, u8 idx, u8 plttIdx)
{
    u8 i;
    u8 *src;
    u16 *dst;
    u8 ch;
    u8 x, y;

    x = (var_r7 & 1) * 13 + 3;
    y = (var_r7 & 0xFE) + 6;

    if (idx)
    {
        src = gUnk_08095028[idx];
        dst = (u16 *)0x2005800 + x + (y * 32);

        for (i = 0; i < 8; i++)
        {
            ch = *src++;
            if (ch == 0)
            {
                break;
            }
            Text_PutGlyph(dst++, ch, plttIdx);
        }
    }
}

// @ 0x0800F4A8
void MenuUi_DrawItemList(void)
{
    u8 idx;

    u8 var_r7;
    s32 x;
    u8 y;
    u8 *src;
    u16 *dst;
    u8 plttIdx;
    u8 i;
    u8 ch;
    u8 val;
    u8 x1;
    u16 *dst1;

    ClearBuffer((u16 *)0x02005986, 0x18, 0xA);

    idx = gUnk_03000199;

    if (idx == 0)
        idx = 1;

    var_r7 = 0;

    while (idx <= 0xFD)
    {
        val = gUnk_03004980[idx];
        if (val != 0)
        {
            gUnk_030001A0[var_r7] = idx;

            if (gMenuCursorStack[gMenuCursorGrp] == var_r7)
            {
                x = (var_r7 & 1) * 13 + 3;
                y = (var_r7 & ~1) + 6;

                if (idx)
                {
                    src = gUnk_08095028[idx];
                    dst = (u16 *)0x2005800 + x + (y * 32);

                    for (i = 0; i < 8; i++)
                    {
                        ch = *src++;
                        if (ch == 0)
                        {
                            break;
                        }
                        Text_PutGlyph(dst++, ch, 0xD);
                    }
                }
            }
            else
            {
                x = (var_r7 & 1) * 13 + 3;
                y = (var_r7 & ~1) + 6;

                if (idx)
                {
                    src = gUnk_08095028[idx];
                    dst = (u16 *)0x2005800 + x + (y * 32);

                    for (i = 0; i < 8; i++)
                    {
                        ch = *src++;
                        if (ch == 0)
                        {
                            break;
                        }
                        Text_PutGlyph(dst++, ch, 0xB);
                    }
                }
            }
            x1 = (var_r7 & 1) * 13 + 13;
            dst1 = (u16 *)0x02005980 + x1 + ((var_r7 & ~1) * 32);
            sub_800EAE4(dst1, val, 12);
            var_r7++;
        }

        idx++;
        if (var_r7 > 9)
        {
            break;
        }
    }

    while (var_r7 <= 9)
    {
        gUnk_030001A0[var_r7] = 0;
        var_r7++;
    }

    gUnk_03000199 = gUnk_030001A0[0];
}

extern u8 gInvPageUpItems[];
extern u8 gInvPageDownItems[];

/* 从 gUnk_03004980 物品/事件表中, 以 gInvViewState[0] 为起点向下、
 * 以 gInvViewState[9] 为起点向上, 各最多拾取 2 个非零项到
 * gInvPageUpItems[] / gInvPageDownItems[]。 */
// @ 0x0800F670
void sub_800F670(void)
{
    u8 idx;
    u8 count;

    idx = gInvViewState[0];
    gInvPageUpItems[0] = 0;
    gInvPageUpItems[1] = 0;
    idx = (u8)(idx - 1);
    if (idx != 0xFF)
    {
        count = 0;
        while (count <= 1 && idx != 0)
        {
            if (gUnk_03004980[idx] != 0)
            {
                gInvPageUpItems[count] = idx;
                count = (u8)(count + 1);
            }
            idx = (u8)(idx - 1);
        }
    }

    gInvPageDownItems[0] = 0;
    gInvPageDownItems[1] = 0;
    idx = gInvViewState[9];
    if (idx != 0 && idx != 0xFF)
    {
        idx = (u8)(idx + 1);
        count = 0;
        while (count <= 1 && idx <= 0xFD)
        {
            if (gUnk_03004980[idx] != 0)
            {
                gInvPageDownItems[count] = idx;
                count = (u8)(count + 1);
            }
            idx = (u8)(idx + 1);
        }
    }
}

// @ 0x0800F70C
INCLUDE_ASM("asm/nonmatchings", sub_800F70C);
// @ 0x0800FA24
u8 sub_800FA24(void)
{
    PlayerStats *chara;
    u8 partyIdx;
    u16 amt;

    gUnk_030001C8 = 0;
    gUnk_030001B0 = 0x10;

    partyIdx = gMenuCursorStack[0] - 1;
    if (partyIdx == 0xFF)
        return 0x27;

    partyIdx = gPartyMemberIds[partyIdx];
    if (partyIdx != 0)
        partyIdx--;

    chara = &gPartyStats[partyIdx];

    if (gUnk_030001AE == 1)
    {
        if (chara->hp == chara->max_hp)
            return 0x24;
        chara->hp += gUnk_030001AF;
        if (chara->hp > chara->max_hp)
            chara->hp = chara->max_hp;
    }
    else
    {
        if (chara->mp == chara->max_mp)
            return 0x24;
        amt = gUnk_030001AF;
        if (amt == 0)
            amt = 0x3E7;
        chara->mp += amt;
        if (chara->mp > chara->max_mp)
            chara->mp = chara->max_mp;
    }

    sub_800AA84(gUnk_030001A0[gMenuCursorStack[gMenuCursorGrp]], 1);
    if (gInventory[gUnk_030001A0[gMenuCursorStack[gMenuCursorGrp]]] == 0)
    {
        gMenuCursorSel = gMenuCursorStack[gMenuCursorGrp];
        sub_800E668(0xFF);
    }

    sub_8010624(0xFF, 1);
    Sfx_Play(0x17, 1, 0);
    return 0x23;
}
// @ 0x0800FB2C
INCLUDE_ASM("asm/nonmatchings", sub_800FB2C);
// @ 0x0800FDEC
void sub_800FDEC(void)
{
    u8 id;
    u8 mask;
    u8 type;
    u8 i;
    u8 cnt;
    const EnemyCharaStat *e;

    gUnk_030001B9 = 0xFF;
    gUnk_030001BA = 0xFF;

    id = gPartyMemberIds[(u8)(gMenuCursorStack[0] - 1)];
    if (id > 8)
        return;

    if (id != 0)
        id--;

    mask = 1 << id;

    type = gMenuCursorStack[gMenuCursorGrp];
    if (type == 5)
        type = 4;

    i = gItemUseCtx[0];
    if (gUnk_030001B1 != 0)
    {
        i--;
        if (i != 0xFF)
        {
            cnt = 0;
            while (i != 0)
            {
                e = &gCharaBaseData[i];
                if ((e->resistFlags & mask) != 0
                 && (e->formRace & 0xF) == type
                 && gInventory[i] != 0)
                {
                    gUnk_030001B9 = i;
                    cnt++;
                }
                i--;
                if (cnt != 0)
                    break;
            }
        }
        if (cnt == 0)
            gUnk_030001B9 = cnt;
    }

    i = gItemUseCtx[4];
    if (i == 0 || i == 0xFF)
        return;

    i++;
    cnt = 0;

    while (i <= 0xFD)
    {
        e = &gCharaBaseData[i];
        if ((e->resistFlags & mask) != 0
         && (e->formRace & 0xF) == type
         && gInventory[i] != 0)
        {
            gUnk_030001BA = i;
            cnt++;
        }
        i++;
        if (cnt != 0)
            break;
    }
}
// @ 0x0800FF10
/* 装备更换预览: arg0=道具/装备 id (0xFF=不换, 仅刷新), arg1=装备槽 (0..5),
 * arg2=队伍成员索引 (gPartyStats 下标)。流程:
 *   1) 临时把新 id 写入该成员的装备槽 (先存旧值);
 *   2) 调 Stats_RebuildEquipBonuses 重算 gEquipBonus* 全局;
 *   3) 恢复旧值;
 *   4) 把"基础加成 + 该件装备加成"与角色当前装备加成比较,
 *      按 0xC=升 / 0xD=降 写入 gStatArrowIds[] (相等则保持 0xB)。 */
void sub_800FF10(u8 arg0, u8 arg1, u8 arg2)
{
    PlayerStats *ps;
    u8 prev;

    ps = &gPartyStats[arg2];

    if (arg0 == 0xFF)
    {
        gEquipBonusAtkBase = ps->equip_atc;
        gEquipBonusDef2 = ps->equip_def;
        gEquipBonusAgl = ps->equip_agl;
        gEquipBonusMen = ps->equip_men;
        gEquipBonusRes = ps->equip_res;
        gEquipBonusNoa = ps->equip_noa;
        gEquipBonusLuc = ps->equip_luc;
        return;
    }

    switch (arg1)
    {
    case 0:
        prev = ps->equip_slot1;
        ps->equip_slot1 = arg0;
        break;
    case 1:
        prev = ps->equip_slot2;
        ps->equip_slot2 = arg0;
        break;
    case 2:
        prev = ps->equip_slot3;
        ps->equip_slot3 = arg0;
        break;
    case 3:
        prev = ps->equip_slot4;
        ps->equip_slot4 = arg0;
        break;
    case 4:
        prev = ps->equip_slot5;
        ps->equip_slot5 = arg0;
        break;
    case 5:
        prev = ps->equip_slot6;
        ps->equip_slot6 = arg0;
        break;
    }

    Stats_RebuildEquipBonuses(gPartyMemberIds[(u8)(gMenuCursorStack[0] - 1)]);

    switch (arg1)
    {
    case 0:
        ps->equip_slot1 = prev;
        break;
    case 1:
        ps->equip_slot2 = prev;
        break;
    case 2:
        ps->equip_slot3 = prev;
        break;
    case 3:
        ps->equip_slot4 = prev;
        break;
    case 4:
        ps->equip_slot5 = prev;
        break;
    case 5:
        ps->equip_slot6 = prev;
        break;
    }

    gEquipBonusAtkBase += gEquipBonusAtk;
    gEquipBonusDef2 += gEquipBonusDef;

    if (ps->equip_atc > gEquipBonusAtkBase)
        gStatArrowIds[0] = 0xC;
    else if (ps->equip_atc < gEquipBonusAtkBase)
        gStatArrowIds[0] = 0xD;

    if (ps->equip_def > gEquipBonusDef2)
        gStatArrowIds[1] = 0xC;
    else if (ps->equip_def < gEquipBonusDef2)
        gStatArrowIds[1] = 0xD;

    if (ps->equip_agl > gEquipBonusAgl)
        gStatArrowIds[2] = 0xC;
    else if (ps->equip_agl < gEquipBonusAgl)
        gStatArrowIds[2] = 0xD;

    if (ps->equip_men > gEquipBonusMen)
        gStatArrowIds[3] = 0xC;
    else if (ps->equip_men < gEquipBonusMen)
        gStatArrowIds[3] = 0xD;

    if (ps->equip_res > gEquipBonusRes)
        gStatArrowIds[4] = 0xC;
    else if (ps->equip_res < gEquipBonusRes)
        gStatArrowIds[4] = 0xD;

    if (ps->equip_noa > gEquipBonusNoa)
        gStatArrowIds[6] = 0xC;
    else if (ps->equip_noa < gEquipBonusNoa)
        gStatArrowIds[6] = 0xD;

    if (ps->equip_luc > gEquipBonusLuc)
        gStatArrowIds[5] = 0xC;
    else if (ps->equip_luc < gEquipBonusLuc)
        gStatArrowIds[5] = 0xD;
}
// @ 0x08010170
INCLUDE_ASM("asm/nonmatchings", sub_8010170);
// @ 0x0801026C
u8 ItemGetUsePower(u8 arg0, u8 arg1)
{

    PlayerStats *ptr4AC0;
    u8 val;

    if (arg0 != 0)
        arg0--;

    ptr4AC0 = &gPartyStats[arg0];

    val = ItemGetValue(arg1);

    if (ptr4AC0->equip_slot5 == 0xAF)
    {
        val >>= 1;
        if (val == 0)
            val = 1;
        return val;
    }

    if (ptr4AC0->equip_slot6 == 0xAF)
    {
        val >>= 1;
        if (val == 0)
            val = 1;
        return val;
    }

    if (ptr4AC0->equip_slot2 == 0x63)
    {
        if (val > 2)
            val -= 2;
        else
            val = 1;
    }

    if (ptr4AC0->equip_slot3 == 0x83)
    {
        if (val > 2)
            val -= 2;
        else
            val = 1;
    }

    if (ptr4AC0->equip_slot3 == 0x84)
    {
        if (val > 2)
            val -= 2;
        else
            val = 1;
    }

    return val;
}

// @ 0x08010300
INCLUDE_ASM("asm/nonmatchings", sub_8010300);
u8 EventFlags_Test(u16);
extern u8 gUnk_080987C4[];

// @ 0x08010434
u8 WarpTable_Check(void)
{
    u8 i;
    u8 val;

    if (EventFlags_Test(0xBA))
        return 0;

    i = 0;
    while ((val = gUnk_080987C4[i]) != 0)
    {
        if (val == gCurrentMapId)
        {
            i++;
            gMapNpcSetId = gUnk_080987C4[i++];
            gSpawnTileX = gUnk_080987C4[i++];
            gSpawnTileY = gUnk_080987C4[i++];
            gSpawnFacingDir = gUnk_080987C4[i++];
            gMoveCmdSetId = gUnk_080987C4[i] + (gUnk_080987C4[i + 1] << 8);
            gWarpAnimState = 1;
            gUnk_03004D4C = 0x34;
            return 1;
        }
        i += 7;
    }

    return 0;
}

// @ 0x080104F8
INCLUDE_ASM("asm/nonmatchings", sub_80104F8);
// @ 0x08010624
INCLUDE_ASM("asm/nonmatchings", sub_8010624);
// @ 0x08010770
/* 道具/技能菜单的"确认使用"处理 (被 sub_800B374 附近的菜单确认逻辑调用)。
 * arg0 = 0: 对全队执行 (回复类道具, gUnk_030001C5==5 时走全队 hp 恢复循环,
 *            否则保存游标并返回上一层菜单 sub_800E668(0xFF));
 * arg0 != 0: 对单个成员执行。
 * 尾部对 gUnk_030001C3==0x3E (无角色) 清 gPartyFollowFlags 的 bit7,
 * 否则按消耗量 gUnk_030001C4 扣减该成员的 mp。 */
void sub_8010770(u8 arg0)
{
    u8 n;
    u16 i;
    u8 id;

    n = 0;
    if (sub_8010300(gUnk_030001C3) != 0)
    {
        if (gUnk_030001C3 != 0x26)
        {
            gUnk_030001B0 = 0x10;
            if (arg0 == 0)
            {
                if (gUnk_030001C5 == 5)
                {
                    i = 0;
                    id = gPartyMemberIds[0];
                    while (id != 0xFF)
                    {
                        if (id != 0)
                            id--;
                        if (gUnk_030001C6 != 0)
                        {
                            gPartyStats[id].hp += gUnk_030001C6;
                            if (gPartyStats[id].hp > gPartyStats[id].max_hp)
                                gPartyStats[id].hp = gPartyStats[id].max_hp;
                        }
                        else
                        {
                            gPartyStats[id].hp = gPartyStats[id].max_hp;
                        }
                        sub_8010624((u8)i, 2);
                        i = (u16)(i + 1);
                        if (i > 4)
                            break;
                        id = gPartyMemberIds[i];
                    }
                    Sfx_Play(0x17, 1, 0);
                    n++;
                }
                else
                {
                    gMenuCursorStack[gMenuCursorGrp] = gMenuCursorSel;
                    gMenuCursorSel = gMenuCursorStack[15];
                    if (gMenuCursorSel <= 3)
                        gMenuCursorSel = 4;
                    sub_800E668(0xFF);
                    Sfx_Play(1, 0, 0);
                    return;
                }
            }
            else
            {
                id = gPartyMemberIds[gMenuCursorSel - 4];
                if (id != 0)
                    id--;
                if (gPartyStats[id].hp < gPartyStats[id].max_hp)
                {
                    if (gUnk_030001C6 != 0)
                    {
                        gPartyStats[id].hp += gUnk_030001C6;
                        if (gPartyStats[id].hp > gPartyStats[id].max_hp)
                            gPartyStats[id].hp = gPartyStats[id].max_hp;
                    }
                    else
                    {
                        gPartyStats[id].hp = gPartyStats[id].max_hp;
                    }
                    sub_8010624((u8)(gMenuCursorSel - 4), 1);
                    Sfx_Play(0x17, 1, 0);
                    n++;
                }
                else
                {
                    gUnk_030001C8 = 0x24;
                    Sfx_Play(3, 0, 0);
                }
            }
        }
        else
        {
            n = 1;
        }
        if (n == 0)
            return;
        if (gUnk_030001C3 == 0x3E)
        {
            gPartyFollowFlags &= 0x7F;
            sub_800F128(0, gMenuCursorStack[gMenuCursorGrp]);
            return;
        }
        id = gPartyMemberIds[(u8)(gMenuCursorStack[0] - 1)];
        if (id != 0)
            id--;
        gPartyStats[id].mp -= gUnk_030001C4;
    }
    else
    {
        Sfx_Play(3, 0, 0);
    }
}

extern const u8 gScreenIdleIconPageMap[];

/* 收集已看过的开场整屏图: 遍历 gScreenIdleEventFlags 的 bit 0..14,
 * 把对应地图 ID (gScreenIdleIconPageMap[i]) 依次填进 gScreenIdleIconIds 并以 0 补满 16 项,
 * 最后游标归零。调用点: sub_800ACC8 (菜单场景初始化)。
 * 消费者: sub_800C2F8 (游标/翻页) + sub_800F3AC (逐项绘制地点图标)。 */
// @ 0x08010978
void ScreenIdleIcons_BuildList(void)
{
    u8 i;
    u8 i2;

    if (EventFlags_Test(0xFD))
        gScreenIdleEventFlags[1] |= 0x20;

    for (i = 0, i2 = 0; i <= 0xE; i++)
    {
        if ((gScreenIdleEventFlags[i >> 3] >> (i & 7)) & 1)
        {
            gScreenIdleIconIds[i2] = gScreenIdleIconPageMap[i];
            i2++;
        }
    }

    while (i2 <= 0xF)
    {
        gScreenIdleIconIds[i2] = 0;
        i2++;
    }

    gScreenIdleIconCursor = 0;
}
