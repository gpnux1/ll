#include "battle_types.h"
#include "text_engine.h"
#include "battle_itemuse_rewards.h"
#include "menu.h"
#include "menu_ui.h"
#include "vram_transfer.h"
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

extern u8 gUnk_08095828[][8];
extern u16 gUnk_080981E6[];
extern u8 *gUnk_087EB1E8[];

// @ 0x080163CC
void TextBlocks_Render(u8 *src)
{
    u16 *dest;
    u8 x;
    u8 y;
    u8 paletteId;
    u8 charCode;

    while ((x = *src++) != 0xFF)
    {
        y = *src++;
        paletteId = *src++;
        dest = (u16 *)0x02005800 + x + (y * 32);

        while ((charCode = *src++) != 0xFF)
        {
            if (charCode == 0xFE)
            {
                charCode = *src++;
                Text_PutGlyph(dest, (charCode << 8) | 0xFE, paletteId);
            }
            else
            {
                Text_PutGlyph(dest, charCode, paletteId);
            }
            dest++;
        }
    }
}

// @ 0x08016424
u8 Math_DivLoop(s32 *ptr, s32 divisor)
{
    u8 count = 0;
    while (1)
    {
        *ptr -= divisor;
        if (*ptr < 0)
        {
            *ptr += divisor;
            break;
        }
        count++;
    }
    return count;
}

// @ 0x08016444
void Msg_ShowEndMark(u16 *arg0, u16 arg1, u8 arg2)
{
    Text_PutGlyph(Msg_DrawPoolSegment(arg0, arg1, arg2), 0xC9, 0xB);
}
/* 把全局消息池里第 segIdx 段字形串渲染进 dest, 返回推进后的 dest 游标。
 *
 * gMsgPool @0x0830FC04 (708 B, 仍在 data/data.s 的 blob 内, 两端都 4 对齐可单独搬出):
 *   段间用 0xFF 分隔; 串内 0xFE 是转义前缀, 后跟高位字节 -> 16 位字形码 (hi<<8)|0xFE。
 *   先走 segIdx 个 0xFF 定位到目标段, 再逐字发 Text_PutGlyph(dest++, code, palette)。
 *   串的**终止 0xFF 不消耗** —— 调用者 Msg_ShowEndMark 靠它在返回的游标处补右边框 0xC9。
 *
 * 代码生成要点 (已逐字节验证, fndiff score=0):
 *   - `*p` 必须写**三次** (循环测 / ==0xFE 测 / 实参), 目标就是两次 ldrb +
 *     一次 "ldrb r1; adds r0,r1,#0; cmp r0,#0xff" 的共享载入;
 *     提成局部变量 code 会少一条 ldrb 并改变载入时机 -> 不匹配
 *   - `dest++` 必须写在实参位置 (先传后推: adds r0,r5,#0; adds r5,#2), 且要在算 code 之前
 *   - 定位循环写成 `for (i = 0; i != segIdx; ) { if (*p++ != 0xFF) continue; i++; }`
 *     即 p++ 无条件、i++ 只在命中 0xFF 时 —— 与目标的顶部测试 + 两条回边一致
 */
// @ 0x08016460
u16 *Msg_DrawPoolSegment(u16 *dest, u16 segIdx, u8 palette)
{
    const u8 *p;
    u16 i;

    p = (const u8 *)0x0830FC04;

    for (i = 0; i != segIdx;)
    {
        if (*p++ != 0xFF)
            continue;
        i++;
    }

    while (*p != 0xFF)
    {
        if (*p == 0xFE)
        {
            p++;
            Text_PutGlyph(dest++, (*p << 8) | 0xFE, palette);
        }
        else
        {
            Text_PutGlyph(dest++, *p, palette);
        }
        p++;
    }

    return dest;
}
/* 给主文本池 gMsgPoolMain @0x080936A0 (6536 B, 0xFF 分隔) 建**快速跳转索引**。
 *
 * 线性扫过池中的 0xFF 分隔符计数段号 n, 每满 64 段就把该段起点记进
 * gMsgSegIndex @0x030001D0 (64 项 × u32 = 256 B): index[n >> 6] = p。
 * 段 0 的起点单独写在 index[0]。遇到连续两个 0xFF (段末) 即结束。
 *
 * 代码生成要点 (已逐字节验证: fndiff score=0, bytecmp OK 56B):
 *   - 首次写入必须写成 `*(u32 *)0x030001D0 = p;` 而**不能**复用 `index` 变量,
 *     否则 GCC2 会把池载入直接落到 r3, 少掉目标的 `ldr r0; str [r0]; adds r3,r0,#0` 复制
 *   - 且 `n = 0;` 必须夹在首次写入与 `index = ...` 之间 (决定那条 adds 的位置)
 *   - `n` 必须是 u32/int: 用 u16 会给 n++ 加上 lsls/lsrs 截断
 */
// @ 0x080164C0
void Msg_BuildSegmentIndex(void)
{
    const u8 *p;
    u32 *index;
    u32 n;

    p = (const u8 *)0x080936A0;
    *(u32 *)0x030001D0 = (u32)p;
    n = 0;
    index = (u32 *)0x030001D0;

    for (;;)
    {
        while (*p != 0xFF)
            p++;
        p++;
        n++;
        if ((n & 0x3F) == 0)
            index[n >> 6] = (u32)p;
        if (*p == 0xFF)
            return;
    }
}
// @ 0x080164F8
void Msg_Show(u16 arg0)
{
    Msg_ShowById(arg0, 0xB);
}
// @ 0x08016508
void Msg_ShowById(u16 arg0, u8 arg1)
{
    u16 i;
    u32 target;
    u8 *ptr;

    ptr = gMsgTable[arg0 >> 6];

    target = arg0 & 0x3F;

    i = 0;

    while (i != target)
    {
        if (*ptr++ == 0xFF)
            i++;
    }

    Msg_RenderLine(ptr, arg1);
}

// @ 0x0801654C
s32 Text_WriteOrClear(u8 *arg0, u8 arg1, u8 arg2)
{
    u8 x, y;
    u16 *dest;
    u8 len;

    x = *arg0++;
    y = *arg0++;

    dest = (u16 *)0x02005800 + x + y * 32;

    if (arg2 == 0)
    {
        len = 0;
        while (*arg0 != 0xFF)
        {
            arg0++;
            len++;
        }
        while (len > 0)
        {
            Text_PutGlyph(dest++, 0, 0x0B);
            len--;
        }
    }
    else
    {
        while (*arg0 != 0xFF)
        {
            Text_PutGlyph(dest++, *arg0++, arg1);
        }
    }

    // No Return?
}

// @ 0x080165B8
u8 Menu_GetFocus(void)
{
    return gMenuCursorStack[0] - 1;
}
// @ 0x080165C8
void Text_ClearRect(u8 x, u8 y, u8 width, u8 height)
{
    u16 *temp_buf;
    u16 *buf;
    u16 row, col;
    buf = (u16 *)(0x02005800) + x + y * 32;
    for (row = 0; row < height; row++)
    {
        temp_buf = buf;
        for (col = 0; col < width; col++)
        {
            *buf++ = 0xB001;
        }
        buf = temp_buf + 0x20;
    }
}
// @ 0x08016628
void MenuUi_SetExclusive(u8 arg0, u8 arg1)
{
    u8 i;
    UISpriteEntity *p;

    if (gUnk_03004D40 != 0 && arg0 == 0xFF)
    {
        arg0 = 5;
    }

    p = gUiSprites;
    for (i = 0; i < 15; i++)
    {
        if (p->statusFlags != 0 && i != arg0)
        {
            if (arg1 != 0)
            {
                p->statusFlags &= 0xBF;
            }
            else
            {
                p->statusFlags |= 0x40;
            }
        }
        p++;
    }
}

// @ 0x0801667C
void MenuUi_HideAll(void)
{
    u8 temp_r0;
    u8 i;
    UISpriteEntity *p;

    p = gUiSpritesAux;

    for (i = 5; i < 15; i++)
    {
        if (p->statusFlags != 0)
            p->statusFlags |= 0x40;
        p++;
    }
}
extern const u8 gUnk_08095028[];

// @ 0x080166A4
void Text_DrawChar(u8 arg0, u8 arg1, u8 arg2, u8 arg3)
{
    const u8 *src;
    u16 *dest;
    u8 i;
    u8 ch;

    if (arg0 != 0)
    {
        src = gUnk_08095028 + arg0 * 8;
        dest = (u16 *)(0x02005800 + arg1 * 2 + arg2 * 64);
        i = 0;
        while (i <= 7 && (ch = *src++) != 0)
        {
            Text_PutGlyph(dest++, ch, arg3);
            i++;
        }
    }
}
// @ 0x080166FC
void sub_80166FC(u8 charId, u8 x, u8 y, u8 palette)
{
    u8 j;
    u8 charCode;
    u8 *src;
    u16 *dest;

    if (charId == 0xFF)
        return;

    src = gUnk_08095828[(u8)(charId - 1)];
    dest = (u16 *)(x * 2 + 0x02005800 + y * 64);

    for (j = 0; j < 8; j++)
    {
        charCode = *src++;
        if (charCode == 0)
            break;
        Text_PutGlyph(dest++, charCode, palette);
    }
}
// @ 0x08016758
void sub_8016758(u8 x, u8 y, u8 kind)
{
    int xOffset;
    u32 bit;
    u32 state;
    u16 tile;

    state = gUnk_03000198;
    bit = (state >> 3) & 1;
    switch (kind)
    {
        case 0:
            tile = 0x826 | bit;
            break;
        case 1:
            tile = 0x26 + bit;
            break;
        case 2:
            tile = 0x428 | bit;
            break;
        case 3:
            tile = 0x28 + bit;
            break;
        default:
            tile = 0x3F;
            break;
    }

    xOffset = x * 2;
    *(u16 *)(0x02005800 + xOffset + y * 64) = 0xB240 + tile;
}
// @ 0x080167D4
void MenuUi_MoveCursor(u8 arg0, u8 arg1)
{
    u8 ret = Menu_GetFocus();
    if (ret != 0xFF)
    {
        MenuUi_SetEntityPos(arg0, arg1, ret);
    }
}
// @ 0x080167F8
u8 Party_SlotOfMember(u8 arg0)
{
    u8 i;
    u8 val;

    val = gBattleFormationIds[arg0];

    i = 0;
    while (gPartyMemberIds[i] != val)
    {
        i++;
        if (i > 4)
            return i;
    }

    return i;
}
// @ 0x0801682C
void SkillMenu_SaveCursor(void)
{
    u8 var_r2;

    var_r2 = gPartyMemberIds[(u8)(gMenuCursorStack[0] - 1)];
    if (var_r2 != 0)
    {
        var_r2--;
    }
    sub_800FF10(gItemUseCtx[gMenuCursorSel - 6], gMenuCursorStack[gMenuCursorGrp], var_r2);
}
// @ 0x08016878
s32 ItemUse_Execute(void)
{
    return sub_8010170(gMenuCursorStack[gMenuCursorGrp], gItemUseCtx[gMenuCursorSel - 6]);
}
// @ 0x080168A8
void ItemUse_SetCtx(void)
{
    u8 var_r0;

    var_r0 = gPartyMemberIds[gPartyMenuIdx];

    if (var_r0 != 0)
    {
        var_r0--;
    }

    gSkillMenuTmpA = gPartyStats[var_r0].field_unk[2];
    gSkillMenuTmpB = gPartyStats[var_r0].field_unk[3];
}

// @ 0x080168EC
void SkillMenu_RestoreCursor(void)
{
    u8 var_r0;

    var_r0 = gPartyMemberIds[gPartyMenuIdx];

    if (var_r0 != 0)
    {
        var_r0--;
    }

    gPartyStats[var_r0].field_unk[2] = gSkillMenuTmpA;
    gPartyStats[var_r0].field_unk[3] = gSkillMenuTmpB;
}

// @ 0x08016930
u8 SkillMenu_GetSkill(u8 arg0)
{
    s32 temp_r2;
    u8 var_r0;
    u8 ret;

    temp_r2 = gSkillMenuPage + arg0;
    ;

    if (temp_r2 < 8)
    {
        var_r0 = gPartyMemberIds[gPartyMenuIdx];
        if (var_r0 != 0)
            var_r0--;

        ret = gPartyStats[var_r0].skills[temp_r2];

        if (ret != 0x26)
            return ret;
    }

    return 0xFF;
}

// @ 0x08016978
u8 Inv_FindFirstHeld(void)
{
    u8 i;

    for (i = 0; i <= 0xF; i++)
    {
        if (gInventory[gInvPageItemIds[i]] != 0)
        {
            return i + 1;
        }
    }

    return 0;
}

// @ 0x080169AC
u8 Inv_FindPrevHeld(void)
{
    u8 i;

    i = gSkillMenuPage - 1;
    while (i != 0xFF)
    {
        if (gInventory[gInvPageItemIds[i]] != 0)
        {
            return i;
        }
        i--;
    }

    return 0xFF;
}

/* 返回第 page 页对应的道具 id, 若玩家一个都没持有则返回 0xFF。
 * page 越界 (> 15) 同样返回 0xFF —— 调用者 sub_8011268 自己先测过一遍, 这里再兜一次。
 *
 * 与 Inv_FindFirstHeld / Inv_FindPrevHeld / sub_804DE20 / sub_804F050 共用 gInvPageItemIds。
 *
 * 代码生成要点 (已逐字节验证): `page` 必须**就地复用**存道具 id, 不要再声明第二个 u8 局部 ——
 * 多一个 qty 会让 GCC2 把表基址分到 r2 而不是目标的 r0 (实测 score 20, 只差 4 字节)。
 * 同理 gInvPageItemIds 必须是真 extern 数组, 用强转宏写法同样会换寄存器。
 */
// @ 0x080169EC
u8 Inv_FindHeldItemOnPage(u8 page)
{
    if (page > 0xF)
        return 0xFF;

    page = gInvPageItemIds[page];
    if (gInventory[page] == 0)
        return 0xFF;

    return page;
}
/* 把 0x02027000 影子缓冲的存档数据拷回各真实块 (Save_LoadContinue 的逆操作)。
 * 签名校验通过后调用: 逐块按长度表 gUnk_080981E6[i] 拷 count 字节到
 * gUnk_087EB1E8[i] 指向的地址, 源偏移 offset 从 0xC 起连续递增。 */
// @ 0x08016A14
void Save_SyncShadow(void)
{
    u8 *shadow = (u8 *)0x02027000;
    u16 offset = 0xC;
    u32 i = 0;
    u16 len;

    len = gUnk_080981E6[0];
    if (len == 0)
        return;

    do
    {
        u8 *dest = (u8 *)gUnk_087EB1E8[i];
        i++;

        while (len != 0)
        {
            *dest = shadow[offset];
            offset = (u16)(offset + 1);
            dest++;
            len = (u16)(len - 1);
        }

        i = (u16)i;
        len = gUnk_080981E6[i];
    } while (len != 0);
}

// @ 0x08016A6C
void Inv_SeekFirst(void)
{
    u8 i;

    for (i = 1; i <= 0xFD; i++)
    {
        if (gInventory[i] != 0)
        {
            gInvCursor2 = i;
            return;
        }
    }
    gInvCursor2 = 0;
}
// @ 0x08016AA0
u8 Inv_PrevNonZero(void)
{
    u8 index;
    u8 value;
    index = gInvCursor2;

    if (index == 1)
        return 0;

    while (index != 0)
    {
        index--;
        if (gInventory[index] != 0)
            return index;
    }
    return 0;
}
// @ 0x08016AD4
u8 Inv_NextNonZero(u8 arg0)
{
    u8 idx;
    u8 count;

    idx = gInvCursor2;
    count = 0;

    while (count < arg0)
    {
        if (gInventory[idx] != 0)
        {
            count++;
        }
        idx++;

        if (idx == 0xFF)
            return 0;
    }

    while (idx != 0xFF)
    {
        if (gInventory[idx] != 0)
        {
            return idx;
        }
        idx++;
    }

    return 0;
}

/* 道具使用上限检查: 遍历 gPartyMemberIds 前 5 名队员 (0xFF 终止; 表内 id 为 1 基, 先 -- 转 0 基),
 * 排除 arg0 本人, 数 gPartyStats[].field_unk[2]==2 且 field_unk[3]==arg1 的人数,
 * 已达 gInventory[arg1] 持有数则 0 (不可再挂), 否则 1。 */
// @ 0x08016B30
u8 sub_8016B30(u8 arg0, u8 arg1)
{
    u8 charaId;
    u8 count;
    u16 i;
    PlayerStats *ptr;

    count = (i = 0);
    charaId = gPartyMemberIds[0];
    for (; i < 5; i++)
    {
        charaId = gPartyMemberIds[i];
        if (charaId == 0xFF)
        {
            break;
        }
        if (charaId != 0)
        {
            charaId--;
        }
        if (charaId == arg0)
        {
            continue;
        }
        ptr = &gPartyStats[charaId];
        if (ptr->field_unk[2] == 2)
        {
            if (ptr->field_unk[3] == arg1)
            {
                count++;
            }
        }
    }

    if (count >= gInventory[arg1])
    {
        return 0;
    }
    return 1;
}
// @ 0x08016BB0
void SaveUi_OpenLoad(void)
{
    Save_LoadContinue();
    gSaveFsmState = 3;
    gSaveSramBlock = 0xC;
    gSaveModeFlag = 0;
    Msg_ShowById(0x18U, 0xBU);
}
// @ 0x08016BE0
void Text_WriteChars(u16 *dest, u8 *src, u8 arg2)
{

    while (*src != 0xFF)
    {
        *dest = (arg2 << 12) + 0x200 + *src;
        dest++;
        src++;
    }
}

// @ 0x08016C10
void Text_FillHidden(u16 *dest, u8 *src)
{

    while (*src != 0xFF)
    {
        *dest = 0xF200;
        dest++;
        src++;
    }
}

// @ 0x08016C2C
u16 *Text_TileAt(u8 x, u8 y)
{
    return (u16 *)0x2005800 + ((y * 32) + x);
}

// @ 0x08016C44
void sub_8016C44(void)
{
    SpriteNode *ptr = &gSpriteNodePool[112]; // 03004380
    gMenuCursorSprite.y = 0;
    gMenuCursorSprite.x = 0;
    Sprite_InitChainNode(ptr, 1, 0, 0x81E0, 0x21C0);
    gSpriteRenderQueue[0] = ptr;
    sub_800E668(0);
}
// @ 0x08016C88
/* SIO 联机初始化: 关中断清掉串行/DMA0 的 IE 位 (0xFF3F = 保留其它), 开中断;
 * REG_RCNT = 0 把通用 I/O 端口切回 SIO 模式, SIOCNT 先写 0x2000 复位再 `|= 0x4003`
 * 使能多玩家通信与起始位; 然后把会话状态结构整体清零 (0x130 字节), 填好
 * 五个收发缓冲指针 (state+0x30/0x50/0x70/0xB0/0xF0) 与 0x14/0x18 的初值 0x10,
 * 最后再关中断打开 DMA0 IRQ (IE |= 0x80) 并恢复中断。
 * 顺序坑: `CpuFill32` 必须写在 `state = gSioState;` **之前** (目标里宏展开的
 * `movs r6,#0; str r6,[sp]` 落在 `ldr r7,=gSioState` 之前), 反过来写会整体后移两条指令。 */
void sub_8016C88(void)
{
    u8 *state;

    REG_IME = 0;
    REG_IE &= 0xFF3F;
    REG_IME = 1;
    REG_RCNT = 0;
    REG_SIOCNT = 0x2000;
    REG_SIOCNT |= 0x4003;
    CpuFill32(0, gSioState, 0x130);
    state = gSioState;
    *(u32 *)(state + 0x14) = 0x10;
    *(u32 *)(state + 0x18) = 0x10;
    *(u8 **)(state + 0x1C) = state + 0x30;
    *(u8 **)(state + 0x20) = state + 0x50;
    *(u8 **)(state + 0x24) = state + 0x70;
    *(u8 **)(state + 0x28) = state + 0xB0;
    *(u8 **)(state + 0x2C) = state + 0xF0;
    REG_IME = 0;
    REG_IE |= 0x80;
    REG_IME = 1;
}
// @ 0x08016D24
INCLUDE_ASM("asm/nonmatchings", sub_8016D24);
// @ 0x08016E30
void Sio_BuildPacket(u8 *src)
{
    u32 checksum;
    u32 i;
    u16 *packet;
    u8 *state;

    checksum = 0;
    state = gSioState;
    **(u8 *volatile *)(state + 0x1C) = state[0xB];
    (*((u8 *volatile *)(state + 0x1C)))[1] = state[2] ^ state[3];
    *(u16 *)(*(u8 *volatile *)(state + 0x1C) + 2) = 0;
    CpuSet(src, *(u8 *volatile *)(state + 0x1C) + 4, 0x04000006);

    i = 0;
    packet = *(u16 *volatile *)(state + 0x1C);
    for (; i <= 0xD; i++)
        checksum += *packet++;

    *(u16 *)(*(u8 *volatile *)(state + 0x1C) + 2) = ~checksum - 0x10;
    state[4] = 1;
}
// @ 0x08016E80
/* 收包: 关中断交换 0x28/0x2C 双缓冲指针, 清 state[5] (本帧有包) 与 state[3] (收到位图),
 * 再扫两个槽位 (每槽 32 字节 = 14 个 u16 校验区 + 24 字节载荷):
 *   14 个 u16 求和截到 s16 后 == -0x11 即校验通过 (发送端 Sio_BuildPacket 写的是
 *   `~sum - 0x10`, 两端相加正好得 0xFFEF), 命中就把载荷 CpuCopy32 到 arg0 + i*24
 *   并在 state[3] 置第 i 位; 无论命中与否都把载荷区 CpuFill32 清零。
 *   最后 state[2] |= state[3] 把"本帧收到"累积进总位图, 并返回 state[3]。
 * 三处形状靠"变量兼职"还原 (RULES 规则 87/117):
 *   ① 交换的临时量就是 packet 本身 (没有第四个 temp 伪寄存器, 否则 BB0 的 home 全错位);
 *   ② 循环里用的是 st (= state 的第二个伪寄存器), 目标入口块后的 `adds r7, r5, #0`
 *      就是这条拷贝, 单变量写法不会产生它;
 *   ③ `i = 0;` 必须写成循环外的独立语句 (for 的 init 留空), 否则 `movs r6,#0`
 *      会落到拷贝之前, 与目标顺序相反。
 * 返回类型 u8 且末尾多一条 `ldrb r0,[r1,#3]`: 原代码确实 return state[3],
 * 调用方 (sub_8016D24) 忽略返回值 —— 删掉 return 会少两条指令。 */
u8 sub_8016E80(u8 *arg0)
{
    u8 *state;
    u8 *st;
    s32 i;
    u32 j;
    u32 sum;
    u16 *packet;
    u8 recvFlag;

    REG_IME = 0;
    state = gSioState;
    packet = *(u16 **)(state + 0x2C);
    *(u32 *)(state + 0x2C) = *(u32 *)(state + 0x28);
    *(u32 *)(state + 0x28) = (u32)packet;
    recvFlag = state[5];
    state[5] = 0;
    REG_IME = 1;
    state[3] = 0;
    if (recvFlag != 0)
    {
        i = 0;
        st = state;
        for (; i <= 1; i++)
        {
            packet = *(u16 **)(st + 0x2C) + (i << 4);
            sum = 0;
            for (j = 0; j <= 0xD; j++)
                sum += packet[j];
            if ((s16)sum == -0x11)
            {
                CpuCopy32(packet + 2, arg0 + i * 24, 24);
                st[3] |= 1 << i;
            }
            CpuFill32(0, packet + 2, 24);
        }
    }
    gSioState[2] |= gSioState[3];
    return gSioState[3];
}
// @ 0x08016F30
void sub_8016F30(void)
{
    u8 *state;
    u32 mode;
    u32 temp;
    u32 zero;
    u16 sioData;
    vu16 *sio;

    state = gSioState;
    mode = state[0];
    if (mode != 0)
    {
        if (state[1] != 0 && state[6] != 0)
        {
            *(s32 *)(state + 0x18) = -1;
            temp = *(u32 *)(state + 0x28);
            *(u32 *)(state + 0x28) = *(u32 *)(state + 0x24);
            *(u32 *)(state + 0x24) = temp;

            if (state[4] != 0)
            {
                temp = *(u32 *)(state + 0x20);
                *(u32 *)(state + 0x20) = *(u32 *)(state + 0x1C);
                *(u32 *)(state + 0x1C) = temp;
                zero = 0;
                state[4] = zero;
                *(u32 *)(state + 0x14) = zero;
            }

            sio = (vu16 *)REG_ADDR_SIOCNT;
            state[7] = (*(vu32 *)sio << 25) >> 31;
            sioData = 0xFEFE;
            sio[1] = sioData;
            sio[0] |= 0x80;
            REG_TM3CNT_H = 0xC0;
        }
    }
    else
    {
        if (state[9] == 0)
        {
            REG_IME = mode;
            gUnk_03007FF8 |= 0x80;
            REG_IME = 1;
        }
        state[9] = mode;
    }
}
// @ 0x08016FC0
/* SIO 多机串行 IRQ 处理: 读 SIOMLT_RECV 到栈、取 SIO Error 位, 收到 0xFEFE 同步头且
 * 接收列已满(unk_18>0xD)则复位并交换收发双缓冲; 再按 unk_14/unk_18 推进发送数据
 * (SIODATA8=((u16*)unk_20)[unk_14]) 与接收矩阵 (unk_24[2][16]), 主机负责拉高
 * SIO Start 位并启动 TM3 节拍。寄存器按 SioMultiCnt 双 u16 视图访问才出 ROM 的
 * `ldr r,=0x04000128; ... [r,#±2]` 形状 (io.h 分开的 REG_SIOCNT/REG_SIODATA8
 * 地址字面量会差池常量, 不能换); 必须用非 volatile 的 SioMultiCnt 而非 vSioMultiCnt。 */
#define SIO_MULTI_CNT ((SioMultiCnt *)REG_ADDR_SIOCNT)

void sub_8016FC0(void)
{
    void *temp_r1;
    void *temp_r1_2;
    u16 recv[4];

    *(vu64 *)recv = REG_SIOMLT_RECV;
    gSioCommState.errorFlags = SIO_MULTI_CNT->Error;

    if ((recv[0] == 0xFEFE) && (gSioCommState.unk_18 > 0xD))
    {
        gSioCommState.unk_18 = -1;
        temp_r1 = gSioCommState.unk_28;
        gSioCommState.unk_28 = gSioCommState.unk_24;
        gSioCommState.unk_24 = temp_r1;

        if (gSioCommState.swapPending != 0)
        {
            temp_r1_2 = gSioCommState.unk_20;
            gSioCommState.unk_20 = gSioCommState.unk_1C;
            gSioCommState.unk_1C = temp_r1_2;
            gSioCommState.swapPending = 0;
            gSioCommState.unk_14 = 0;
        }
        REG_IME = 0;
        gUnk_03007FF8 |= 0x80;
        REG_IME = 1;
    }

    if (gSioCommState.unk_14 < 0xE)
    {
        SIO_MULTI_CNT->Data = ((u16 *)gSioCommState.unk_20)[gSioCommState.unk_14];
    }

    if (gSioCommState.unk_14 < 0xF)
    {
        gSioCommState.unk_14 += 1;
    }

    if (gSioCommState.unk_18 >= 0)
    {
        s32 var_r3;

        for (var_r3 = 0; var_r3 < 2; var_r3++)
        {
            ((u16(*)[16])gSioCommState.unk_24)[var_r3][gSioCommState.unk_18] = recv[var_r3];
        }

        if (gSioCommState.unk_18 == 0xD)
        {
            gSioCommState.frameHasPacket = 1;
        }
    }

    if (gSioCommState.unk_18 < 0xF)
    {
        gSioCommState.unk_18 += 1;
    }

    if (gSioCommState.isParent)
    {
        REG_TM3CNT_H = 0;
    }

    if ((gSioCommState.unk_14 < 0xF) && gSioCommState.isParent)
    {
        REG_SIOCNT = REG_SIOCNT | 0x80;
        REG_TM3CNT_H = 0xC0;
    }

    gSioCommState.sioInterrupted = 1;
}
#undef SIO_MULTI_CNT

