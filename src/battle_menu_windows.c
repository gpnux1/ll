#include "battle_types.h"
#include "battle_menu_windows.h"
#include "menu_slot.h"
#include "battle_flow_rules.h"
#include "battle_itemuse_rewards.h"
#include "battle_object_engine.h"
#include "battle_palette_wipe.h"
#include "battle_stage_dialogue.h"
#include "battle_stage_state.h"
#include "battle_task_services.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"
#include "sound.h"

extern u8 gUnk_08393A48[];
extern u8 gUnk_08393A4D[];

// @ 0x08021130
void MenuSlot_ResetAll(void)
{
    u8 i;

    for (i = 0; i < 10; i++)
    {
        gMenuSlotStates[i][0] = 0;
        gMenuSlotStates[i][1] = 0;
        gMenuSlotStates[i][2] = 0;
        gMenuSlotStates[i][3] = 0;
        gMenuSlotStates[i][4] = 0;
    }
    gMenuMasterCursor = 0;
}
// @ 0x08021184
void sub_8021184(s8 arg0, u8 *arg1)
{
    u8 b;
    u8 idx;
    u8 *ptr;

    b = (u8)arg0;
    ptr = arg1 + 0xBE;
    if (*ptr != 0)
        idx = *ptr - 1;
    else
        idx = *ptr;

    switch ((s8)b)
    {
        case 0:
            gUnk_0300076A = gMenuMasterCursor;
            break;
        case 3:
            if ((s8)gUnk_0300076A != (s8)gMenuSlotStates[idx][0])
            {
                gUnk_0300076C |= 2;
            }
            gUnk_0300076A = gMenuSlotStates[idx][0];
            break;
        case 6:
            if ((s8)gMenuSlotStates[idx][2] < gMenuListCount)
            {
                gMenuListTop = gMenuSlotStates[idx][1];
                gMenuListCursor = gMenuSlotStates[idx][2];
            }
            else
            {
                gMenuListTop = (gMenuListCount - 1 <= 1) ? 0 : gMenuListCount - 3;
                gMenuListCursor = gMenuListCount - 1;
            }
            break;
        case 7:
            if ((s8)gMenuSlotStates[idx][4] < gMenuList2Count)
            {
                gMenuList2Top = gMenuSlotStates[idx][3];
                gMenuList2Cursor = gMenuSlotStates[idx][4];
            }
            else
            {
                gMenuList2Top = (gMenuList2Count - 1 <= 1) ? 0 : gMenuList2Count - 3;
                gMenuList2Cursor = gMenuList2Count - 1;
            }
            break;
    }
}
// @ 0x080212B4
INCLUDE_ASM("asm/nonmatchings", sub_80212B4);
// @ 0x0802151C
u8 sub_802151C(u8 arg0, BattleObj *arg1)
{
    u8 buf[8];
    u8 count;
    u8 matchCount;
    u16 i;
    u8 *pool;
    u16 flag;

    pool = (u8 *)GetObjPool();
    count = sub_80489E8((BattleObj *)pool, buf, 0, 0x7F);

    matchCount = 0;
    for (i = 0; i < count; i++)
    {
        BattleObj *obj = (BattleObj *)(buf[i] * 0xC8 + (u32)pool);
        if (obj->slot != 9)
        {
            if (sub_8045F10(obj, 0x3C) == 2)
                matchCount++;
        }
    }

    for (i = 0; i < count; i++)
    {
        if (((BattleObj *)(pool + buf[i] * 0xC8))->slot == 9)
        {
            count--;
            break;
        }
    }

    if (matchCount < count)
    {
        flag = sub_80187B4() & 0x4000;
        if (!flag)
        {
            gUnk_03000768 = arg0;
            sub_8021184((s8)gUnk_03000768, NULL);
            gUnk_0300076B = flag;
            gUnk_0300076C = 2;
            gUnk_03000769 = flag;
            gUnk_0300076E = flag;

            sub_801FA10(arg1, 2);
            Bg0_InitClear();
            sub_80196D4(0, 0x02035AC0, 0xB, 2, 2, flag, flag, 0x1E, 5);
            gMenuWindowFlags = flag;
            sub_802550C(1);
            sub_804C2FC(0x0861C624, 0xD, 2);
            sub_8024820();
            sub_804DE8C();
            sub_80187C0(0x100);

            gMenuObjLoadCount = gMenuObjLoadIdx = 0;

            for (i = 0; i < count; i++)
            {
                if ((s8)((BattleObj *)(pool + buf[i] * 0xC8))->fxKind == 3)
                {
                    gMenuObjLoadSlots[gMenuObjLoadCount] = buf[i];
                    ((BattleObj *)(pool + buf[i] * 0xC8))->fxKind = 0;
                    gMenuObjLoadCount++;
                }
            }

            return 0;
        }
        sub_804C78C();
    }
    else
    {
        sub_804C890((BattleObj *)pool);
        if (sub_80187B4() & 0x4000)
            sub_804C78C();
    }

    return 1;
}
// @ 0x08021700
u8 sub_8021700(void)
{
    u8 ret = 0;
    if (gMenuObjLoadIdx < gMenuObjLoadCount)
    {
        BattleObj *obj = (BattleObj *)(GetObjPool() + gMenuObjLoadSlots[gMenuObjLoadIdx] * 0xC8);
        switch (gMenuObjLoadPhase)
        {
            case 0:
                sub_80207DC(obj, obj->posX, obj->posY, obj->headA.f_1E, obj->headA.palSlot);
                gMenuObjLoadPhase = 1;
                break;
            case 1:
                if (!(obj->headA.kindFlags & 0x800))
                {
                    gMenuObjLoadIdx = gMenuObjLoadIdx + 1;
                    gMenuObjLoadPhase = 0;
                }
                break;
        }
    }
    else
    {
        ret = 1;
    }
    return ret;
}
extern u32 DialogCtx_GetField_C_wide(u8) __asm__("DialogCtx_GetField_C");

// @ 0x08021788
void sub_8021788(u8 arg0)
{
    switch (gMenuWindowPhase)
    {
        case 0:
            if (gMenuWindowFlags & 0x1000)
            {
                if (DialogCtx_GetField_C_wide(0) == 0)
                {
                    gMenuWindowFlags &= ~0x1000;
                }
                else
                {
                    u32 v = DialogCtx_GetField_C_wide(0);
                    if (v == 4)
                    {
                        if (((unsigned short)v & gUnk_0300076C) == 0)
                        {
                            sub_802181C((void *)0x02035AC0, 0x18, 2, arg0);
                        }
                    }
                }
            }
            break;
        case 1:
            gMenuWindowFlags |= 0x1000;
            gMenuWindowPhase = 0;
            break;
        case 2:
            gMenuWindowPhase = 0;
            break;
    }
}
// @ 0x0802181C
INCLUDE_ASM("asm/nonmatchings", sub_802181C);
// @ 0x0802192C
INCLUDE_ASM("asm/nonmatchings", sub_802192C);

extern const u8 gUnk_08393A74[];
extern const u8 gUnk_08393A54[];

// @ 0x08022458
u8 sub_8022458(u8 arg0)
{
    const s8 *rec;
    u8 off;
    u8 i;

    if ((s8)gUnk_03000768 < 0)
        return arg0;

    rec = (const s8 *)(gUnk_08393A74 + ((s8)gUnk_03000768 << 4));
    if (rec[0] < 0)
        rec = (const s8 *)(gUnk_08393A74 + (rec[1] << 4));

    off = 0;
    for (i = 0; i < rec[0]; i++)
    {
        vu32 *dmaRegs = (vu32 *)0x040000D4;
        const s8 *p = rec + 8;
        u8 b = p[i];
        {
            const u8 *t = gUnk_08393A54 + (b << 2);

            dmaRegs[0] = (vu32)((const u8 *)0x08619AA4 + (t[0] << 5));
            dmaRegs[1] = (vu32)(0x06010000 + ((0x258 + off) << 5));
            dmaRegs[2] = 0x80000000 | (gUnk_08393A30[(t[1] << 2) + t[2]] << 4);
            dmaRegs[2];
            while (dmaRegs[2] & 0x80000000)
                ;
        }

        {
            const u8 *t = gUnk_08393A54 + (b << 2);
            off += gUnk_08393A30[(t[1] << 2) + t[2]];
        }
        arg0--;
    }

    return arg0;
}
// @ 0x08022550
INCLUDE_ASM("asm/matchings", sub_8022550);
// @ 0x08022710
INCLUDE_ASM("asm/nonmatchings", sub_8022710);
// @ 0x08022F2C
INCLUDE_ASM("asm/nonmatchings", sub_8022F2C);
typedef union {
    u32 raw;
    struct {
        unsigned int low_0_3 : 4;
        unsigned int field_4_7 : 4;
        unsigned int direction : 4;
        unsigned int confirm : 1;
        unsigned int field_13_15 : 3;
    } bits;
} BattleTargetInput;
typedef char BattleTargetInputSizeCheck[sizeof(BattleTargetInput) == 4 ? 1 : -1];

// @ 0x080230BC
u8 sub_80230BC(BattleObj *actor, BattleObj *pool, BattleObj *cursor, u16 input)
{
    BattleTargetInput keys;
    u8 result;
    BattleObj *target;

    result = 0;
    switch ((s8)gUnk_03000769)
    {
    case 0:
        actor->f_BD = 5;
        if (pool[5].slot == 0xFF)
        {
            do
            {
                actor->f_BD++;
                if (actor->f_BD == 0xC)
                    actor->f_BD = 5;
            } while (pool[actor->f_BD].slot == 0xFF);
        }
        target = &pool[actor->f_BD];
        sub_801ED40(target, 0x10);
        sub_8020FB8(cursor, target->headA.f_2B + ((u8)sub_801EC3C(target, 0) >> 1),
                    target->headA.f_2C - ((u8)sub_801EC3C(target, 1) >> 1), 5, 2);
        sub_8022F2C((u16 *)0x02035AC0, target, 0xA, 2);
        gUnk_03000769 = 6;
        gMenuSelSlot0 = -1;
        gMenuSelSlot1 = -1;
        break;
    case 6:
        if ((cursor->state & 0xF0) == 0x10)
            break;
        target = &pool[actor->f_BD];
        if (input & 0x100)
        {
            actor->f_BD = sub_8022710(target, pool, 0);
            Sfx_Play(0, 0, 0);
        }
        else if (input & 0x200)
        {
            Sfx_Play(0, 0, 0);
            actor->f_BD = sub_8022710(target, pool, 1);
        }
        else if (input & 0x400)
        {
            Sfx_Play(0, 0, 0);
            actor->f_BD = sub_8022710(target, pool, 2);
        }
        else if (input & 0x800)
        {
            Sfx_Play(0, 0, 0);
            actor->f_BD = sub_8022710(target, pool, 3);
        }
        if (input & 0xF00)
        {
            sub_801EE6C(target);
            target = &pool[actor->f_BD];
            sub_801ED40(target, 0x10);
            sub_8020FB8(cursor, target->headA.f_2B + ((u8)sub_801EC3C(target, 0) >> 1),
                        target->headA.f_2C - ((u8)sub_801EC3C(target, 1) >> 1), 5, 2);
            sub_8022F2C((u16 *)0x02035AC0, target, 0xA, 2);
        }
        keys.raw = input;
        if (keys.bits.confirm != 0 && keys.bits.direction == 0 && keys.bits.low_0_3 == 0)
        {
            Sfx_Play(1, 0, 0);
            sub_801EE6C(target);
            gUnk_03000769 = 0x1E;
        }
        break;
    case 0x1E:
        actor->fxKind = 0;
        gUnk_0300076C |= 2;
        sub_8019F08((u16 *)0x02035AC0, 1, 0xA, 2, 9, 2);
        result = 1;
        break;
    }
    return result;
}
// @ 0x08023320
// 战斗菜单"目标选定过渡"状态机: objects = 调用者收集的我方成员指针数组(≤5),
// (s8)gUnk_0300076B 为当前数组游标。case1 确认当前项: 清当前项 headA.f_2A
// (+0x36) 并对 state(+0xB0)|=4, 游标非 0 时同时清 objects[i+1].f_2A;
// 游标为 0 时只置 gUnk_0300076C|=0x30。case3 回退: 清当前(及 i>0 时前一项)
// 的 f_2A, 当前 state|=4, gUnk_0300076C|=0x20。两分支均清 gUnk_0300076E 并
// 落到 case0 的 *state=0x1E 收尾(经入口缓存的状态指针写, 复现 GCC2.9 的
// r4/r6 寄存器形状)。字段命名依据见 docs/handoffs/MATCH-CANDIDATE-8023320-20260917.md。
void sub_8023320(BattleObj **objects)
{
    u8 *state = &gUnk_03000769;

    switch (*(s8 *)state)
    {
    case 1:
        gUnk_0300076E = 0;
        if (*(s8 *)&gUnk_0300076B == 0)
        {
            gUnk_0300076C |= 0x30;
        }
        else
        {
            objects[*(s8 *)&gUnk_0300076B]->headA.f_2A = 0;
            objects[*(s8 *)&gUnk_0300076B]->state |= 4;
            objects[*(s8 *)&gUnk_0300076B + 1]->headA.f_2A = 0;
            gUnk_0300076C |= 0x10;
        }
        gUnk_03000769 = 0x1E;
        break;
    case 3:
        gUnk_0300076E = 0;
        objects[*(s8 *)&gUnk_0300076B]->headA.f_2A = 0;
        if (*(s8 *)&gUnk_0300076B > 0)
            objects[*(s8 *)&gUnk_0300076B - 1]->headA.f_2A = 0;
        objects[*(s8 *)&gUnk_0300076B]->state |= 4;
        gUnk_0300076C |= 0x20;
        /* fallthrough */
    case 0:
        *state = 0x1E;
        break;
    }
}
// @ 0x08023414
void sub_8023414(BattleObj **members, u16 inputFlags)
{
    BattleObj *current;
    BattleObj *adjacent;
    switch (gUnk_0300076C & 0xF0)
    {
    case 0x10:
        current = members[(s8)gUnk_0300076B];
        adjacent = members[(s8)gUnk_0300076B + 1];
        current->posX = sub_801768C(gUnk_08393A48[current->memberIdx], 0x23 - gUnk_08393A48[current->memberIdx], 15, (s16)gUnk_0300076E, 2);
        current->posY = sub_801768C(gUnk_08393A4D[current->memberIdx], 0x23 - gUnk_08393A4D[current->memberIdx], 15, (s16)gUnk_0300076E, 2);
        adjacent->posX = sub_801768C(0x23, gUnk_08393A48[adjacent->memberIdx] - 0x23, 15, (s16)gUnk_0300076E, 2);
        adjacent->posY = sub_801768C(0x23, gUnk_08393A4D[adjacent->memberIdx] - 0x23, 15, (s16)gUnk_0300076E, 2);
        if (gUnk_0300076E <= 14)
        {
            if (inputFlags & 0x1000)
                gUnk_0300076E = 15;
            else
                gUnk_0300076E++;
        }
        else
        {
            adjacent->headA.f_2A = 3;
            gUnk_0300076E = 0;
            adjacent->state &= 0xFFFB;
            gUnk_0300076C &= 0xFF0F;
        }
        break;
    case 0x20:
        current = members[(s8)gUnk_0300076B];
        adjacent = members[(s8)gUnk_0300076B - 1];
        current->posX = sub_801768C(gUnk_08393A48[current->memberIdx], 0x23 - gUnk_08393A48[current->memberIdx], 15, (s16)gUnk_0300076E, 2);
        current->posY = sub_801768C(gUnk_08393A4D[current->memberIdx], 0x23 - gUnk_08393A4D[current->memberIdx], 15, (s16)gUnk_0300076E, 2);
        adjacent->posX = sub_801768C(0x23, gUnk_08393A48[adjacent->memberIdx] - 0x23, 15, (s16)gUnk_0300076E, 2);
        adjacent->posY = sub_801768C(0x23, gUnk_08393A4D[adjacent->memberIdx] - 0x23, 15, (s16)gUnk_0300076E, 2);
        if (gUnk_0300076E <= 14)
        {
            if (inputFlags & 0x1000)
                gUnk_0300076E = 15;
            else
                gUnk_0300076E++;
        }
        else
        {
            adjacent->headA.f_2A = 3;
            gUnk_0300076E = 0;
            adjacent->state &= 0xFFFB;
            gUnk_0300076C &= 0xFF0F;
        }
        break;
    case 0x30:
        current = members[0];
        members[0]->posX = sub_801768C(0x23, gUnk_08393A48[current->memberIdx] - 0x23, 15, (s16)gUnk_0300076E, 2);
        members[0]->posY = sub_801768C(0x23, gUnk_08393A4D[members[0]->memberIdx] - 0x23, 15, (s16)gUnk_0300076E, 2);
        if (gUnk_0300076E <= 14)
        {
            if (inputFlags & 0x1000)
                gUnk_0300076E = 15;
            else
                gUnk_0300076E++;
        }
        else
        {
            members[0]->headA.f_2A = 3;
            members[0]->state &= 0xFFFB;
            gUnk_0300076E = 0;
            gUnk_0300076C &= 0xFF0F;
        }
        break;
    case 0x40:
        current = members[(s8)gUnk_0300076B];
        current->posX = sub_801768C(0x23, gUnk_08393A48[current->memberIdx] - 0x23, 15, (s16)gUnk_0300076E, 2);
        current->posY = sub_801768C(0x23, gUnk_08393A4D[current->memberIdx] - 0x23, 15, (s16)gUnk_0300076E, 2);
        if (gUnk_0300076E <= 14)
        {
            if (inputFlags & 0x1000)
                gUnk_0300076E = 15;
            else
                gUnk_0300076E++;
        }
        else
        {
            current->headA.f_2A = 3;
            current->state &= 0xFFFB;
            gUnk_0300076E = 0;
            gUnk_0300076C &= 0xFF0F;
        }
        break;
    case 0x50:
        current = members[0];
        members[0]->posX = sub_801768C(gUnk_08393A48[current->memberIdx], 0x23 - gUnk_08393A48[current->memberIdx], 15, (s16)gUnk_0300076E, 2);
        members[0]->posY = sub_801768C(gUnk_08393A4D[members[0]->memberIdx], 0x23 - gUnk_08393A4D[members[0]->memberIdx], 15, (s16)gUnk_0300076E, 2);
        if (gUnk_0300076E <= 14)
        {
            if (inputFlags & 0x1000)
                gUnk_0300076E = 15;
            gUnk_0300076E++;
        }
        else
        {
            gUnk_0300076E = 0;
            gUnk_0300076C &= 0xFF0F;
        }
        break;
    }
}
// @ 0x08023820
INCLUDE_ASM("asm/nonmatchings", sub_8023820);
// @ 0x080244BC
/* 字形段表 0x0839B462 (段间以 0xF00 分隔) 与同族的 sub_804ACC0/sub_804AC60 共用;
 * 登记处与 battle_flow_rules.c 一致, 为 TU 局部 extern。 */
extern u16 gUnk_0839B462[];
/* script_vm.h 的 K&R/宽原型逐字同型 (本 TU 未 include 该头)。 */
extern void sub_8050434();
extern u32 TileDma_GetCtx(u32 *);

/* 在战斗菜单窗口行上绘制技能名 (字形段表的第 skillId 段)。
 *   arg0 = BG0 map 基址 (0x02035AC0); arg1 = 施法者对象; arg2 = 窗口内行 (0..2);
 *   arg3 = 技能槽 id (gMenuListItems[] 元素 0..7; >7 时跳过样式位与槽查表, 直接当段号用)。
 * 段号 = obj->skills[arg3], 段定位 = 数 0xF00 分隔符到第 段号 个 (≥ 段表尾 → 停在尾后)。
 * 字形码 ≤0xDF = 静态字模 (tile = 码 * 2); >0xDF = 动态字模: 在 gTileDmaAllocTable
 * (TileDma_GetCtx 写出的表指针, 返回值为登记数) 中查下标 k, tile = (k + 0xE0) * 2。
 * 调色板同 sub_80246E8: 行样式位 gMenuListRowBits 置位 (MP ≥ 消耗) → 11, 灰化 → 12。
 * 每位写上下两格 (dst[i] / dst[i + 0x20]), 空格/终止 = 字形码 0xF00。 */
void sub_80244BC(u16 *tileBuf, BattleObj *obj, u8 row, u16 idx)
{
    u32 dmaCtx;
    u16 *dst;
    u16 *entry;
    u8 *ptr;
    u16 value;
    u16 ctxCount;
    int attr;
    u16 i;
    u16 found;
    u16 k;
    u16 tile;

    dst = (u16 *)((u8 *)tileBuf + (row * 0x80 + 0x210));
    attr = 1;
    if (idx <= 7)
    {
        attr &= ~(gMenuListRowBits >> (row + (s8)gMenuListTop));
        ptr = (u8 *)obj + 0x99;
        idx = ptr[idx];
    }
    attr = (u8)(attr + 0xB);

    i = 0;
    found = 0;
    if (i < idx)
    {
        do
        {
            if (gUnk_0839B462[i] == 0xF00)
                found++;
            i++;
        } while (found < idx);
    }
    entry = &gUnk_0839B462[i];

    value = 0x4F00;
    if (row != 0)
        value = 0x6F1E;
    sub_8050434((u32)entry, value);

    ctxCount = (u16)TileDma_GetCtx(&dmaCtx);

    for (i = 0; entry[i] != 0xF00; i++)
    {
        if (entry[i] <= 0xDF)
        {
            dst[i] = (entry[i] & 0xFF) * 2 + (attr << 12);
            dst[i + 0x20] = (entry[i] & 0xFF) * 2 + 1 + (attr << 12);
        }
        else
        {
            k = 0;
            for (; k < ctxCount; k++)
            {
                if (entry[i] == ((u16 *)dmaCtx)[k])
                    break;
            }
            tile = (u16)(k + 0xE0);
            dst[i] = (attr << 12) + tile * 2;
            dst[i + 0x20] = (attr << 12) + (tile * 2 + 1);
        }
    }
}
// @ 0x08024618
INCLUDE_ASM("asm/nonmatchings", sub_8024618);
// @ 0x080246E8
/* 在战斗菜单窗口行上绘制 3 位十进制数字 (技能 MP 消耗); 由 sub_8023820 case 9 逐行调用。
 *   arg0 = BG0 map 基址 (0x02035AC0, 行距 0x40 字节); arg1 = 施法者战斗对象;
 *   arg2 = 窗口内行号 (0..2); arg3 = 技能槽 id (gMenuListItems[] 的元素, 0..7)。
 * 数值 = sub_8048934(obj, idx) 的 MP 消耗; 字形字符码 '0'..'9' = 0xA2..0xAB, tilemap 条目
 * = 调色板号 (bit12-15) + 字符码 * 2, 每位画上下两格 (+0x20 半字 = 下一 map 行)。
 * 调色板: 行样式位 gMenuListRowBits 置位 (MP >= 消耗, 可用) → 11, 否则灰化 12。
 * 前导零: 十位与百位都为 0 时十位留空 (空格 tile = 调色板基值 + 1); 百位非 0 时写 '0'
 * 字符码 (0xA2) 本身 —— 调用方数值 ≤ 0x46 故不触发, 忠实保留 ROM 行为。 */
void sub_80246E8(u16 *tileBuf, BattleObj *obj, u8 row, u16 idx)
{
    u16 digits[3];
    u16 *dst;
    u8 palette;
    s32 bits;
    u8 v;
    u16 i;
    u32 base;
    u8 code;

    dst = (u16 *)((u8 *)tileBuf + (row * 0x80 + 0x224));
    if (idx > 0x27)
        return;

    bits = gMenuListRowBits >> (row + (s8)gMenuListTop);
    palette = 1;
    palette &= ~bits;

    v = sub_8048934(obj, idx);

    digits[0] = (u8)((u32)v / 100);
    digits[1] = (v - digits[0] * 100) / 10;
    digits[2] = v - (digits[0] * 100 + digits[1] * 10);

    for (i = 0; i <= 2; i++)
    {
        switch (i)
        {
        case 0:
            if (digits[i] != 0)
                digits[i] = 0xA2;
            break;
        case 1:
            if (digits[i] == 0 && digits[0] == 0)
                digits[i] = 0;
            else
                digits[i] += 0xA2;
            break;
        case 2:
            digits[i] += 0xA2;
            break;
        }
    }

    code = palette + 0xB;

    i = 0;
    base = code << 12;

    for (; i <= 2; i++)
    {
        if (digits[i] != 0)
        {
            dst[i] = base + digits[i] * 2;
            dst[i + 0x20] = base + (u16)(digits[i] * 2 + 1);
        }
        else
        {
            dst[i] = base + 1;
            dst[i + 0x20] = base + 1;
        }
    }
}
// @ 0x08024820
void sub_8024820(void)
{
    s8 idx = (s8)gUnk_03000768;
    switch (idx)
    {
        case 0:
        case 1:
        case 2:
        case 4:
            sub_8018798(0, -1);
            sub_8018798(1, -1);
            sub_8018798(2, 0);
            sub_8018798(3, 0);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
        case 3:
            sub_8018798(0, -1);
            sub_8018798(1, -1);
            sub_8018798(2, 0);
            sub_8018798(3, 0);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
        case 5:
            sub_8018798(0, 0);
            sub_8018798(1, 0);
            sub_8018798(2, 0);
            sub_8018798(3, 0);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
        case 6:
            sub_8018798(0, 0);
            sub_8018798(1, 0);
            sub_8018798(2, -1);
            sub_8018798(3, -1);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
        case 7:
        case 8:
        case 9:
            sub_8018798(0, -1);
            sub_8018798(1, -1);
            sub_8018798(2, 0);
            sub_8018798(3, 0);
            sub_8018798(4, 1);
            sub_8018798(5, 2);
            break;
    }
}
// @ 0x08024940
INCLUDE_ASM("asm/nonmatchings", sub_8024940);
// @ 0x0802550C
void sub_802550C(u8 value)
{
    gMenuWindowPhase = value;
}
// @ 0x08025518
INCLUDE_ASM("asm/matchings", sub_8025518);
// @ 0x08025638
void sub_8025638(void)
{
    gMenuSelSlot0 = -1;
    gMenuSelSlot1 = -1;
}
// @ 0x08025650
void sub_8025650(arg0, arg1) u8 *arg0;
u8 arg1;
{
    u16 *p;
    p = (u16 *)(arg0 + 0x20E);
    if (arg1 == 0)
        return;
    gUnk_0300076E = (gUnk_0300076E + 1) % 16;
    if (arg1 & 1)
    {
        if (arg1 & 0x10)
        {
            u16 c = gUnk_0300076E;
            p[15] = (c >> 3) + 0xB9DE;
        }
        else
        {
            p[15] = 0xB001;
        }
        p += 0xA0;
        if (arg1 & 0x20)
        {
            u16 c2 = gUnk_0300076E;
            p[15] = (c2 >> 3) + 0xB1DE;
        }
        else
        {
            p[15] = 0xB001;
        }
    }
    else
    {
        p[15] = 0xB000;
        p = (u16 *)(arg0 + 0x34E);
        p[15] = 0xB000;
    }
}
// @ 0x080256E4
void sub_80256E4(u16 *base)
{
    s32 palette;
    u8 row;
    s32 bits;
    s32 y;

    for (row = gMenuListTop; row < (s8)gMenuListTop + 3 && row < gMenuListCount; row = (u8)(row + 1))
    {
        bits = gMenuListRowBits >> row;
        palette = 1;
        if ((bits & 1) != 0)
            palette = (row == (s8)gMenuListCursor) ? 2 : 0;
        palette += 0xB;
        y = (row - (s8)gMenuListTop) * 2 + 8;
        BgMap_PalFillRect(base, palette, 8, y, 9, 2);
    }
}
// @ 0x0802576C
void sub_802576C(u8 *obj)
{
    u8 i;
    u8 style;
    u8 selected;

    for (i = gMenuList2Top; i < gMenuList2Top + 3 && i < gMenuList2Count; i++)
    {
        selected = gMenuList2Cursor;
        style = 0;
        if (i == selected)
            style = 2;
        BgMap_PalFillRect(obj, style + 0xB, 8, (i - gMenuList2Top) * 2 + 8, 9, 2);
    }
}
// @ 0x080257D8
/* 战斗对象"抓取目标并位移"演出状态机 (gUnk_03000820: 0 → 3 → 4 → 5 → 6 → 7 → 9)。
 * 双参 (obj=本对象, arg1=同池被抓取的协作对象); 由 sub_803F444 经 0x0839CD5C 指针表分派,
 * 是 docs/handoffs/ENGINE-HUB-20260912-a.md 所列 0x080257D8 一族 (共 89 项, idx 由 animPtr[0x2F]
 * 与 obj->fxKind/animSubIdx 选取) 的第一个。
 *   case0  起手置步骤 3。
 *   case3  记 headA.f_1E/palSlot 存 gObjActSavedF2A/gObjActSavedPal, sub_801CE80(obj,1,0x1B4,0xD,0)
 *          切动画, 清帧计数, sub_80444A4 清同组伤害 + sub_803F5B4 起手 → 4。
 *   case4  等 headA.kindFlags 的 0x800 (DMA/装载位) 落 → 5。
 *   case5  逐帧驱动: 帧计数 == obj->f_B4-0xA 时播 Sfx(0x31); == obj->f_B4-5 时
 *          sub_8044514(0x14) 交还演出权; 否则 off = 帧计数-0x64, 当 (u8)off<=5 (即帧计数落在
 *          0x64..0x69 窗口) 时用 sub_801768C 把 arg1->posX 从 gUnk_08393A48[arg1->memberIdx]
 *          处开始插值 5 帧; 期间 headA.kindFlags 的 0x1000 (本对象演出完成) 落时清 0xEFFF,
 *          sub_80207DC(obj,posX,posY,SavedF2A,SavedPal) 收尾绘制, 置步骤 6, 并把 arg1->posX
 *          还原为 gUnk_08393A48[arg1->memberIdx]; 帧计数自增。
 *   case6  → 7 且帧计数清 0。
 *   case7  帧计数 <= 0x18 内自增, 越过则置 9。
 *   case9  result = 1 (演出完成)。
 * 尾部 sub_803F658(obj) 收尾。关键复现点: off 必须是 s32 且用 `off = 帧计数; off -= 0x64;`
 * 两步形式 (写成 `s32 off = 帧计数 - 0x64` 会让 agbcc 先做 24 位提取再减 → 差 2 条指令);
 * 条件写 `(u8)off <= 5` 才生成目标的 lsls/lsrs 掩码; 结果用 `gObjActSavedX = x = call(...)` 一次赋值。 */
u8 sub_80257D8(BattleObj *obj, BattleObj *arg1)
{
    u8 result;
    u8 x;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActStep = 3;
            break;
        case 3:
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_801CE80(obj, 1, 0x1B4, 0xD, 0);
            gObjActStepTimer = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            gObjActStep = 4;
            break;
        case 4:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 5;
            break;
        case 5:
            if (gObjActStepTimer == obj->f_B4 - 0xA)
                Sfx_Play(0x31, 1, 0);
            if (gObjActStepTimer == obj->f_B4 - 5)
            {
                sub_8044514(0x14);
            }
            else
            {
                s32 off = gObjActStepTimer;
                off -= 0x64;
                if ((u8)off <= 5)
                {
                    gObjActSavedX = x = sub_801768C(gUnk_08393A48[arg1->memberIdx], 5, 5, off, 2);
                    arg1->posX = x;
                }
            }
            if (obj->headA.kindFlags & 0x1000)
            {
                obj->headA.kindFlags &= 0xEFFF;
                sub_80207DC(obj, obj->posX, obj->posY, gObjActSavedF2A, gObjActSavedPal);
                gObjActStep = 6;
                arg1->posX = gUnk_08393A48[arg1->memberIdx];
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            gObjActStep = 7;
            gObjActStepTimer = 0;
            break;
        case 7:
            if (gObjActStepTimer > 0x18)
                gObjActStep = 9;
            gObjActStepTimer += 1;
            break;
        case 9:
            result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x08025994
extern u8 gUnk_08393A4D[];
extern u8 gUnk_08393B19[];
extern u8 gUnk_08393B20[];

u32 sub_8025994(BattleObj *actor, BattleObj *targets)
{
    BattleObj *target;
    u16 i;
    u32 done;

    done = 0;
    switch (gObjActStep)
    {
    case 0:
        gObjActStep = 3;
        sub_801B81C(&actor->headB, 120, 120, 0x2EA, 10,
            (u32)gUnk_08393B28[((u16 *)actor->animPtr)[4] + 1].animScriptPtr,
            (u32)gUnk_08393B28[((u16 *)actor->animPtr)[4] + 1].palettePtr,
            gUnk_08393B28[((u16 *)actor->animPtr)[4] + 1].gfxBaseIdx,
            gUnk_08393B28[((u16 *)actor->animPtr)[4] + 1].gfxTotal, 4);
        actor->state |= 0x2000;
        actor->headB.kindFlags |= 0x100;
        actor->headB.f_2A = 2;
        sub_80444A4(actor);
        sub_803F5B4(actor);
        break;
    case 3:
        if (actor->headB.kindFlags & 0x800)
            break;
        gObjActSavedF2A = actor->headA.f_1E;
        gObjActSavedPal = actor->headA.palSlot;
        sub_801CE80(actor, 5, 0x1B4, 7, 0);
        actor->headA.f_2A = 3;
        actor->headA.kindFlags |= 0x100;
        gObjActStepTimer = 0;
        gObjActStep = 4;
        break;
    case 4:
        if (actor->headA.gfxPos == actor->headA.gfxTotal - 1)
        {
            actor->state |= 0x40;
            actor->headA.f_2B += 24;
        }
        if (actor->headA.kindFlags & 0x800)
            break;
        actor->headA.kindFlags &= 0xFEFF;
        actor->headB.kindFlags &= 0xFEFF;
        gObjActStep = 5;
        break;
    case 5:
        if (actor->headB.frameIdx == 0x2A)
            Sfx_Play(0x5C, 1, 0);
        if (actor->headA.frameIdx == 0x55)
        {
            for (i = 0; i < 5; i++)
            {
                target = &targets[i];
                if (sub_8045F10(target, 1) != 0)
                {
                    target->state |= 4;
                    target->headA.kindFlags |= 0x200;
                }
            }
            Sfx_Play(0x64, 1, 0);
        }
        else if (actor->headA.frameIdx == 0xD6)
        {
            for (i = 0; i < 5; i++)
            {
                target = &targets[i];
                if (sub_8045F10(target, 1) != 0)
                {
                    target->state &= 0xFFFB;
                    target->headA.kindFlags &= 0xFDFF;
                }
            }
            actor->headA.f_2A = 0;
            actor->headB.f_2A = 0;
            Sfx_Play(0x50, 1, 0);
            sub_8044514(40);
        }
        if (actor->headB.kindFlags & 0x1000)
        {
            actor->headB.kindFlags &= 0xEFFF;
            actor->state &= 0xDFFF;
            gObjActStep = 6;
            sub_80207DC(actor, actor->posX, actor->posY, gObjActSavedF2A, gObjActSavedPal);
            actor->state &= 0xFFBF;
        }
        break;
    case 6:
        sub_80209C8(actor);
        gObjActStep = 7;
        for (i = 0; i < 5; i++)
        {
            target = &targets[i];
            if (sub_8045F10(target, 0x110) == 1)
                target->dmgAmount = sub_804473C(actor, (u8 *)target);
        }
        break;
    case 7:
        for (i = 0; i < 5; i++)
        {
            target = &targets[i];
            if (sub_8045F10(target, 0x110) == 1)
            {
                target->headA.kindFlags &= 0xEFFF;
                if (target->slot <= 10)
                    sub_80207DC(target, gUnk_08393A48[target->memberIdx], gUnk_08393A4D[target->memberIdx], target->headA.f_1E, target->headA.palSlot);
                else
                    sub_80207DC(target, gUnk_08393B19[target->memberIdx], gUnk_08393B20[target->memberIdx], target->headA.f_1E, target->headA.palSlot);
            }
        }
        gObjActStep = 9;
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            done = 1;
            actor->headA.f_2A = 3;
        }
        break;
    }
    sub_803F658(actor);
    return done;
}
// @ 0x08025DA8
/* 战斗对象"协作/投掷/协同突进"演出状态机 (双参: obj=主动方, arg1=目标/协作方)。
 * 由 sub_803F444 经 0x0839CD5C 指针表分派。
 * 流程与 sub_8027D9C/sub_80260BC 同族:
 *   case 0   → 3
 *   case 3   暂存 headA 参数, sub_801CE80(obj,1,0x1B4,0xB,0) 切动作,
 *            sub_80444A4 + sub_803F5B4 起手, 计时清零 → 0x12
 *   case 0x12 等 headA.kindFlags 的 0x800 (装载位) 落 → 0x13
 *   case 0x13 等 headA.frameIdx > 0x40 → 0x14
 *   case 0x14 sub_801B81C 装配 headB (0x2BE 项), obj->state |= 0x2000 (跳跃/动作),
 *            headB.f_2A = 2, 计时清零 → 0x15
 *   case 0x15 等 headB 0x800 落 → Sfx_Play(0x20, 1, 0) → 0x16
 *   case 0x16 等 headA.frameIdx > 0x6D → Sfx_Play(0x64, 2, 0) → 0x17
 *   case 0x17 计时 <= 9 时用 sub_801768C 把 headB 从 (0x76, 0x5A) 向
 *            (arg1->posX - 0x99, arg1->posY - 0x5A) 插值;
 *            超时置 obj->f_B6 = 0x2BF, sub_8044514(0x28), 清 state 0x2000 → 5
 *   case 5   等 arg1->headB 0x800 落, 设定 arg1->headB.f_2B/f_2C 坐标并转 6
 *   case 6   等 headA 0x1000 (动画完成), 复位 headA 并转 9
 *   case 9   等 gActWaitBusy* 全清, 返回 1
 * 尾部: sub_803F658(obj) 逐帧收尾, 若 headA 0x1000 则复位。 */
u32 sub_8025DA8(BattleObj *obj, BattleObj *arg1)
{
    u32 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActStep = 3;
            break;
        case 3:
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_801CE80(obj, 1, 0x1B4, 0xB, 0);
            sub_80444A4(obj);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx > 0x40)
                gObjActStep = 0x14;
            break;
        case 20:
            sub_801B81C(&obj->headB, 0x5E, 0x38, 0x2EA, 0xD,
                        (u32)gUnk_08393B28[0x2BE].animScriptPtr, (u32)gUnk_08393B28[0x2BE].palettePtr,
                        gUnk_08393B28[0x2BE].gfxBaseIdx, gUnk_08393B28[0x2BE].gfxTotal, 4);
            obj->state |= 0x2000;
            obj->headB.f_2A = 2;
            gObjActStepTimer = 0;
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x20, 1, 0);
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headA.frameIdx > 0x6D)
            {
                Sfx_Play(0x64, 2, 0);
                gObjActStep = 0x17;
            }
            break;
        case 23:
            if (gObjActStepTimer <= 9)
            {
                obj->headB.f_2B = sub_801768C(0x76, arg1->posX - 0x99, 0xA, gObjActStepTimer, 1);
                obj->headB.f_2C = sub_801768C(0x5A, arg1->posY - 0x5A, 0xA, gObjActStepTimer, 1);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->f_B6 = 0x2BF;
                sub_8044514(0x28);
                obj->state &= ~0x2000;
                gObjActStep = 5;
            }
            break;
        case 5:
            if (arg1->headB.kindFlags & 0x800)
                break;
            arg1->headB.f_2B = arg1->posX - 0x23;
            arg1->headB.f_2C = arg1->posY;
            gObjActStep = 6;
            break;
        case 6:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            obj->headA.kindFlags &= 0xEFFF;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    if (obj->headA.kindFlags & 0x1000)
    {
        obj->headA.kindFlags &= 0xEFFF;
        sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
    }
    return result;
}
// @ 0x080260BC
/* 战斗对象"多段闪光/抖动演出"状态机 (变体, gObjActStep: 0 → 3 → 0x12 → 0x13 → 0x14 → 0x15 →
 * 0x16 ⇄ (0x17) → 6 → 0x18 → 9)。与 sub_802698C 同族同构 (指令流 100% 相似), 差异:
 *   - 步骤 PC 采用细分区间 0x12..0x18 (而非 0..9);
 *   - case3 sub_801CE80 复位参数 arg3=0xB (非 0xD);
 *   - case0x12 等 headA.kindFlags 的 0x800 落才推进 (对应 sub_802698C 无此等待);
 *   - case0x14 headB 装配第 5 参 =0xD (非 0xE);
 *   - case0x15 headB.kindFlags 0x800 落时补 Sfx_Play(0x55,1,0);
 *   - case0x16 (≈ sub_802698C case1) 抖动超限时先 Sfx_StopTrack(1) 再重装配 headB;
 *   - case6 (≈ case7) 复位后额外置 headA.kindFlags |= 0x100 并转 0x18, 0x18 清 0x800/0x100 → 9。
 * 其余 (case0/0x13/0x16 抖动/0x17/9 全体概率效果 + 尾部 sub_803F658 & headA.kindFlags 0x1000 收尾)
 * 与 sub_802698C 完全一致。 */
u32 sub_80260BC(BattleObj *obj)
{
    u32 result;
    u8 i;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            sub_80444A4(obj);
            sub_803F5B4(obj);
            gObjActStep = 3;
            break;
        case 3:
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_801CE80(obj, 5, 0x1B4, 0xB, 0);
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 0x12:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 0x13:
            if (obj->headA.frameIdx > 0x46)
                gObjActStep = 0x14;
            break;
        case 0x14:
            sub_801B81C(&obj->headB, 0x80, 0x77, 0x2EA, 0xD,
                        (u32)gUnk_08393B28[0x2C1].animScriptPtr, (u32)gUnk_08393B28[0x2C1].palettePtr,
                        gUnk_08393B28[0x2C1].gfxBaseIdx, gUnk_08393B28[0x2C1].gfxTotal, 4);
            obj->state |= 0x2000;
            obj->headB.f_2A = 2;
            gObjActStepTimer = 0;
            gObjActStep = 0x15;
            break;
        case 0x15:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x55, 1, 0);
            gObjActStep = 0x16;
            break;
        case 0x16:
            if (obj->headB.frameIdx == 9)
            {
                obj->headB.kindFlags |= 0x100;
                obj->headA.kindFlags |= 0x100;
                gObjActStepTimer = 0;
                obj->headB.frameIdx += 1;
                break;
            }
            if (obj->headB.frameIdx <= 9)
                break;
            {
                s32 v = obj->headB.frameIdx + 1;
                obj->headB.frameIdx = v - (v / 8) * 8 + 0xA;
            }
            gObjActStepTimer += 1;
            if (gObjActStepTimer == 0x10)
            {
                obj->headA.kindFlags &= 0xFEFF;
                break;
            }
            if (gObjActStepTimer <= 0x17)
                break;
            {
                Sfx_StopTrack(1);
                obj->headB.kindFlags &= 0xEFFF;
                sub_801B81C(&obj->headB, 0x74, 0x80, 0x2EA, 0xE,
                            (u32)gUnk_08393B28[0x2C2].animScriptPtr, (u32)gUnk_08393B28[0x2C2].palettePtr,
                            gUnk_08393B28[0x2C2].gfxBaseIdx, gUnk_08393B28[0x2C2].gfxTotal, 4);
                obj->headB.kindFlags |= 0x2000;
                obj->state |= 0x2000;
                obj->headB.f_2A = 2;
                gObjActStep = 0x17;
            }
            break;
        case 0x17:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            obj->headB.kindFlags &= 0xEFFF;
            obj->state &= 0xDFFF;
            obj->f_B6 = 1;
            sub_8044514(0x28);
            gObjActStep = 6;
            break;
        case 6:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            obj->headA.kindFlags &= 0xEFFF;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            obj->headA.kindFlags |= 0x100;
            gObjActStep = 0x18;
            break;
        case 0x18:
            if (obj->headA.kindFlags & 0x800)
                break;
            obj->headA.kindFlags &= 0xFEFF;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 != 0 || gActWaitBusy1 != 0 || gActWaitBusy2 != 0)
                break;
            GetObjPool();
            for (i = 0; i < gObjActGroupCount; i++)
            {
                if ((gObjActGroupSlots[i] & 0xF0) != 0x10)
                    continue;
                if (Rng_LcgNext() % 0x64 <= 0x27)
                    gObjSlotFxCmd[gObjActGroupSlots[i] & 0xF] = 1;
            }
            result = 1;
            break;
    }
    sub_803F658(obj);
    if (obj->headA.kindFlags & 0x1000)
    {
        obj->headA.kindFlags &= 0xEFFF;
        sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
    }
    return result;
}
