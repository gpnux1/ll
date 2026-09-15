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

typedef void (*UnkFuncDD70)(u8 *, u32);
extern UnkFuncDD70 gUnk_0839CE38[];

// @ 0x0804DD70
void sub_804DD70(BattleObj *ptr, u32 arg1)
{
    gUnk_0839CE38[ptr->slot - 0x71](ptr, arg1);
}
extern const u8 gUnk_087EA580[];
extern const u8 gUnk_0839CEFC[];

// @ 0x0804DD90
u8 sub_804DD90(u8 arg0, u8 arg1)
{
    u8 result;
    const u8 *data;

    result = 0;
    data = gUnk_0839CEFC + gUnk_087EA580[arg0 * 12 + 10] * 3;
    switch (arg1)
    {
        case 0:
            result = data[0] & 0x3F;
            break;
        case 1:
            result = data[0] >> 6;
            break;
        case 2:
            result = data[1] & 0xF;
            break;
        case 3:
            result = (data[1] >> 4) & 3;
            break;
        case 4:
            result = data[1] >> 6;
            break;
        case 5:
            result = data[2] & 3;
            break;
        case 6:
            result = (data[2] >> 3) & 1;
            break;
    }
    return result;
}

// @ 0x0804DE20
void sub_804DE20(void)
{
    u8 i;

    for (i = 0; i <= 15; i++)
    {
        gInvPendingApply[i].itemId = 0;
        gInvPendingApply[i].count = 0;
    }
    gInvPendingApplyCount = 0;
    for (i = 0; i <= 15; i++)
    {
        if (gInventory[gInvPageItemIds[i]] != 0)
        {
            gInvPendingApply[gInvPendingApplyCount].itemId = gInvPageItemIds[i];
            gInvPendingApply[gInvPendingApplyCount].count = gInventory[gInvPageItemIds[i]];
            gInvPendingApplyCount++;
        }
    }
}
// @ 0x0804DE8C
void sub_804DE8C(void)
{
    u8 i;

    for (i = 0; i <= 4; i++)
    {
        gObjInvBackup[i].itemId = 0;
        gObjInvBackup[i].count = 0;
    }
    for (i = 0; i <= 15; i++)
    {
        gInvPageDeltas[i].itemId = 0;
        gInvPageDeltas[i].count = 0;
    }
    gInvPageDeltaCount = 0;
    for (i = 0; i <= 15; i++)
    {
        if (gInventory[gInvPageItemIds[i]] != 0)
        {
            gInvPageDeltas[gInvPageDeltaCount].itemId = gInvPageItemIds[i];
            gInvPageDeltas[gInvPageDeltaCount].count = gInventory[gInvPageItemIds[i]];
            gInvPageDeltaCount++;
        }
    }
}
// @ 0x0804DF14
u8 sub_804DF14(InvListEntry *dest)
{
    u8 count;
    u8 i;

    for (i = 0; i <= 15; i++)
    {
        dest[i].itemId = 0;
        dest[i].count = 0;
    }
    count = 0;
    for (i = 0; i < gInvPageDeltaCount; i++)
    {
        if (gInvPageDeltas[i].count != 0)
        {
            dest[count].itemId = gInvPageDeltas[i].itemId;
            dest[count].count = gInvPageDeltas[i].count;
            count++;
        }
    }
    return count;
}

// @ 0x0804DF74
void sub_804DF74(InvListEntry *entry, u8 *obj, u8 index)
{
    u8 i;
    u8 id;

    gObjInvBackup[index].itemId = entry->itemId;
    gObjInvBackup[index].count = entry->count;
    id = entry->itemId;
    obj[0xA4] = id;
    obj[0xBC] = 2;
    for (i = 0; i < gInvPageDeltaCount; i++)
    {
        if (gInvPageDeltas[i].itemId == entry->itemId)
        {
            gInvPageDeltas[i].count--;
            break;
        }
    }
}
// @ 0x0804DFD8
/* 在物件栏 (u16 tilemap) 上绘制一项 "名称 + 数量":
 *   arg0 = tilemap 基址; arg1 = 列, arg2 + arg4*2 = 行 (arg4 是行号偏移);
 *   arg3 = 4 字节物件条目 (arg3[0] = 名称 id, arg3[1] = 数量);
 *   arg5 = 调色板号 (<<12 组成 tilemap 条目); arg6 != 1 时跳过数量绘制。
 * 名称占 8 格、每格 2 个瓦片; 数量取两位, 十位为 0 时显示空格瓦片 (pal|1)。 */
extern const u8 gUnk_08095028[][8]; /* 道具名称表 (256 项 x 8B), 见 data_805769C.h */

void sub_804DFD8(u16 *arg0, u8 arg1, u8 arg2, u8 *arg3, u8 arg4, u8 arg5, u8 arg6)
{
    u16 *dst;
    const u8 *name;
    u8 digits[2];
    u8 i;

    dst = (u16 *)((u8 *)arg0 + ((arg2 + arg4 * 2) * 32 + arg1) * 2);
    name = gUnk_08095028[arg3[0]];

    for (i = 0; i < 8; i++)
    {
        if (name[i] != 0)
        {
            dst[i] = (arg5 << 12) + name[i] * 2;
            dst[i + 32] = (arg5 << 12) + (name[i] * 2 + 1);
        }
        else
        {
            dst[i] = (arg5 << 12) + 1;
            dst[i + 32] = (arg5 << 12) + 1;
        }
    }

    if (arg6 == 1)
    {
        digits[0] = (u32)arg3[1] / 10;
        digits[1] = (u32)arg3[1] - digits[0] * 10;

        for (i = 0; i < 2; i++)
        {
            if (i == 0 && digits[0] == 0)
            {
                dst[10] = (arg5 << 12) + 1;
                dst[10 + 32] = (arg5 << 12) + 1;
            }
            else
            {
                dst[i + 10] = (arg5 << 12) + (digits[i] + 0xA2) * 2;
                dst[i + 10 + 32] = (arg5 << 12) + ((digits[i] + 0xA2) * 2 + 1);
            }
        }
    }
}
// @ 0x0804E0E4
/* 物件使用演出状态机 (gItemUseFxState 0..13)。返回 1 = 演出结束。
 * 前段按 obj[0xA4] (0xDD..0xE4) 选出一对参数 (a, b) 供 case 4 传给 sub_801EEE4;
 * 后段是状态机: 0 起手→1 播动画→2 等 0x1000 →3 等 0x800 →4 播完→13 收尾。
 * 与 battle_stage_dialogue.c sub_8034440 / battle_stage_actor.c 的同族状态机同型 (zero/keys 临时同样式)。 */
u8 ItemUseFx_RunConsumable(BattleObj *arg0, u32 arg1)
{
    u8 result;
    u8 a;
    u32 b;
    u32 zero;
    u16 keys;

    result = 0;

    switch (*((u8 *)arg0 + 0xA4) - 0xDD)
    {
    case 0:
        a = 0;
        b = 0x1E;
        break;
    case 1:
        a = 0;
        b = 0x96;
        break;
    case 2:
        a = 1;
        b = 0x32;
        break;
    case 3:
        a = 1;
        b = 0x3E7;
        break;
    case 4:
        a = 0xA;
        b = 1;
        break;
    case 5:
        a = 4;
        break;
    case 6:
        a = 3;
        break;
    case 7:
        a = 0xB;
        break;
    }

    switch (gItemUseFxState)
    {
    case 0:
        sub_8020DE4();
        gItemUseFxState = 1;
        gItemUseFxSavedF2A = arg0->headA.f_1E;
        gItemUseFxSavedPalSlot = arg0->headA.palSlot;
        break;
    case 1:
        sub_801CBA4(arg0, 6, 0x1B4, 0xD, 0);
        gItemUseFxState = 2;
        Sfx_Play(0x17, 0, 0);
        break;
    case 2:
        if (arg0->headA.kindFlags & 0x1000)
        {
            u8 v;

            v = arg0->headA.palSlot;
            sub_804C3A4(v, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
            keys = arg0->headA.kindFlags & 0xEFFF;
            zero = 0;
            arg0->headA.kindFlags = keys;
            sub_801CBA4(arg0, zero, gItemUseFxSavedF2A, gItemUseFxSavedPalSlot, zero);
            keys = arg0->headA.kindFlags | 0x100;
            arg0->headA.kindFlags = keys;
            gItemUseFxState = 3;
        }
        break;
    case 3:
        if (!(arg0->headA.kindFlags & 0x800))
        {
            arg0->headA.kindFlags &= 0xFEFF;
            gItemUseFxState = 4;
        }
        break;
    case 4:
        if ((u8)sub_801EEE4(arg0, arg1, 0, a, b) == 1)
        {
            gItemUseFxState = 0xD;
        }
        break;
    case 13:
        result = 1;
        gItemUseFxState = 0;
        break;
    }

    return result;
}
// @ 0x0804E2AC
INCLUDE_ASM("asm/nonmatchings", ItemUseFx_RunWeapon);
// @ 0x0804E6DC
s8 sub_804E6DC(BattleObj *obj, u8 value)
{
    u8 result;
    u8 *data;
    u8 i;

    result = -1;
    if (obj->slot <= 10)
    {
        data = (u8 *)obj + 0x8D;
        if (data[0] != 0 || data[1] != 0 || data[2] != 0 || data[3] != 0 || data[4] != 0 || data[5] != 0 || data[6] != 0
            || data[7] != 0)
        {
            for (i = 0; i <= 5; i++)
            {
                if (gUnk_087EA580[data[i] * 12 + 5] == value)
                {
                    result = i;
                    break;
                }
            }
        }
    }
    return result;
}
// @ 0x0804E76C
// 在 obj->equipSlots (6 槽, +0x8D..0x92) 里找第一个属性命中的槽下标:
// sub_804DD90(槽值, arg1) == arg2 时返回 0-5, 否则 -1; 仅玩家侧 (slot<=0xA) 生效.
// 基址必须写成 obj->equipSlots 整体赋给指针: 折成一次 r5 = obj+0x8D,
// 逐字段 obj->equipSlots[i] 会被展开成 8 条独立地址计算 (差 95B).
// 前置过滤 ROM 读 8 字节 = 6 个槽 + 0x93/0x94 两个从未被写入的 pad, 循环只扫 6 个槽.
s8 sub_804E76C(BattleObj *obj, u8 arg1, u8 arg2)
{
    s8 result;
    u8 i;
    u8 *values;

    result = -1;
    if (obj->slot <= 10)
    {
        values = obj->equipSlots;
        if (values[0] != 0 || values[1] != 0 || values[2] != 0 || values[3] != 0 || values[4] != 0 || values[5] != 0 || values[6] != 0
            || values[7] != 0)
        {
            for (i = 0; i <= 5; i++)
            {
                if (sub_804DD90(values[i], arg1) == arg2)
                {
                    result = i;
                    break;
                }
            }
        }
    }
    return result;
}
static inline u8 CheckObj(BattleObj *obj)
{
    u8 ret = 0;
    u8 v91 = obj->equipSlots[4];
    u8 v92 = obj->equipSlots[5];

    do
    {
        if (v91 == 0 && v92 == 0)
            return 0;
    } while (0);

    if (sub_804DD90(v91, 6))
        ret = 1;
    else if (sub_804DD90(v92, 6))
        ret = 2;

    return ret;
}

// @ 0x0804E7EC
void BattleFxObjs_Add(BattleObj *obj)
{
    u8 slot = CheckObj(obj);

    if (slot != 0)
    {
        obj->equipSlots[slot + 3] = 0;
        gBattleFxObjs[gBattleFxObjCount] = (u32)obj;
        gBattleFxObjCount++;
    }
}
// @ 0x0804E85C
/* 物件"演出"状态机 (gBattleFxState, 返回 1 = 全部播完)。
 * gBattleFxObjs 是对象指针表, 第 0 项是模板对象 (其余对象从它复制状态块):
 *   0: 起手 — sub_8020CC4 播开场, 标记 obj[0x66]=3 → 1
 *   1: 等 obj[0x54] 的 0x800 清掉, 然后把模板的 0x30 字节状态块 (obj+0x3C)
 *      复制到每个后续对象 → 2
 *   2: 等 obj[0x54] 的 0x1000, 刷新演出帧, 清所有对象 obj[0xB0] 的 0x2000 位 → 4
 *   4: 逐对象调 sub_804612C 收尾, 返回 1 */
#define ObjSlot(n) ((u8 *)gBattleFxObjs[n])

typedef struct
{
    u32 w[12]; /* obj+0x3C 起 0x30 字节的状态块 */
} ObjBlk;

u8 BattleFx_Update(void)
{
    u8 result;
    u8 i;

    result = 0;

    switch (gBattleFxState)
    {
    case 0:
        sub_8020CC4(ObjSlot(0), ObjSlot(0)[0xBF], ObjSlot(0)[0xC0], 0x2EA, 0xE, 0xA6, 0x104);
        ObjSlot(0)[0x66] = 3;
        gBattleFxState = 1;
        break;
    case 1:
        if (!(*(u16 *)(ObjSlot(0) + 0x54) & 0x800))
        {
            *(u16 *)(ObjSlot(0) + 0x54) &= 0xFEFF;
            for (i = 1; i < gBattleFxObjCount; i++)
            {
                *(ObjBlk *)(ObjSlot(i) + 0x3C) = *(ObjBlk *)(ObjSlot(0) + 0x3C);
            }
            gBattleFxState = 2;
        }
        break;
    case 2:
        if (*(u16 *)(ObjSlot(0) + 0x54) & 0x1000)
        {
            sub_804C3A4(ObjSlot(0)[0x65], (u8)sub_801B954((ObjHead *)(ObjSlot(0) + 0x3C)));
            for (i = 0; i < gBattleFxObjCount; i++)
            {
                *(u16 *)(ObjSlot(i) + 0xB0) &= 0xDFFF;
            }
            gBattleFxState = 4;
        }
        break;
    case 4:
        for (i = 0; i < gBattleFxObjCount; i++)
        {
            sub_804612C((BattleObj *)(ObjSlot(i)), 0xA, 1);
        }
        result = 1;
        break;
    }

    return result;
}
// @ 0x0804E9DC
INCLUDE_ASM("asm/nonmatchings", BattleDrops_Roll);
// @ 0x0804EC04
INCLUDE_ASM("asm/nonmatchings", sub_804EC04);
// @ 0x0804EEC4
void sub_804EEC4(void)
{
    u8 i;

    for (i = 0; i < gInvPendingApplyCount; i++)
    {
        gInventory[gInvPendingApply[i].itemId] = gInvPendingApply[i].count;
    }
}
// @ 0x0804EF00
void sub_804EF00(u8 arg0)
{
    u8 i;

    if (gObjInvBackup[arg0].itemId == 0)
    {
        return;
    }

    for (i = 0; i < gInvPageDeltaCount; i++)
    {
        if (gInvPageDeltas[i].itemId == gObjInvBackup[arg0].itemId)
        {
            gInvPageDeltas[i].count = gObjInvBackup[arg0].count;
        }
    }
}
// @ 0x0804EF50
void sub_804EF50(void)
{
    u8 i;

    for (i = 0; i < gInvPageDeltaCount; i++)
    {
        if (gInvPageDeltas[i].itemId > 0xDC)
        {
            gInventory[gInvPageDeltas[i].itemId] = gInvPageDeltas[i].count;
        }
    }
}
// @ 0x0804EF90
u8 sub_804EF90(u8 arg0)
{
    u8 i;
    u8 ret = 0xFF;

    for (i = 0; i < gInvPageDeltaCount; i++)
    {
        if (gInvPageDeltas[i].itemId == arg0)
        {
            ret = i;
            break;
        }
    }

    return ret;
}
extern u8 gUnk_0839BB4C[];

// @ 0x0804EFDC
void sub_804EFDC(u8 *base, u8 x, u8 y, u8 *tile, u8 palette)
{
    u16 *dest;
    u8 tp;
    u8 *tbl;
    u16 i;

    dest = (u16 *)(base + (y * 32 + x) * 2);
    tp = sub_804F050(*tile);
    tbl = (u8 *)gUnk_0839BB4C + tp * 24;
    for (i = 0; i <= 0x17; i++)
    {
        if (tbl[i] != 0 && tbl[i] != 0xFF)
        {
            dest[i] = (palette << 12) + tbl[i] * 2;
            dest[i + 0x20] = (palette << 12) + (tbl[i] * 2 + 1);
        }
        else
        {
            dest[i] = (palette << 12) + 1;
            dest[i + 0x20] = (palette << 12) + 1;
        }
    }
}
// @ 0x0804F050
u8 sub_804F050(u8 arg0)
{
    u8 i;

    for (i = 0; i < 16; i++)
    {
        if (arg0 == gInvPageItemIds[i])
            break;
    }

    return i;
}
// @ 0x0804F07C
void ItemUseFx_Reset(void)
{
    gItemUseFxState = 0;
}
// @ 0x0804F088
u8 ItemUseFx_Update(BattleObj *arg0, u32 arg1)
{
    if (arg0->slot > 0xAU)
    {
        return 2;
    }
    if (*((u8 *)arg0 + 0xA4) > 0xDCU)
    {
        return ItemUseFx_RunConsumable(arg0, arg1);
    }
    return ItemUseFx_RunWeapon(arg0, arg1);
}

// 检查战斗对象 arg0 的两个装备槽候选 (+0x91/+0x92 = equipSlots[4]/[5]) 哪个
// 通过 sub_804DD90(id, 6) 校验。仅当 arg1 截断到 u8 后恰好为 6 时才检查;
// 返回 1=前一个(equip5) / 2=后一个(equip6) / 0=都不行。
// 注: 全 ROM 无任何调用点(死代码), 两个已知引用位置都是直接 bl 不传参。
// 注: 两处 do {} while(0) 都是 GCC2 调度/分配屏障, 缺一不可(去掉分别差 48 / 7 字节);
//     `arg1 = (u8)arg1;` 必须显式写且参数声明为 s32 —— 若参数声明 u8,
//     GCC2 会把 `arg1 < 0` 当恒假折叠掉(少两条指令)。
// @ 0x0804F0B8
u8 sub_804F0B8(BattleObj *arg0, s32 arg1)
{
    u8 ret;
    u8 a;
    u8 b;

    arg1 = (u8)arg1;
    ret = 0;
    a = arg0->equipSlots[4];
    b = arg0->equipSlots[5];
    do
    {
        if (a == 0 && b == 0)
            return 0;
    } while (0);
    do
    {
        if (arg1 < 0)
            return ret;
        if (arg1 <= 5)
            return ret;
        if (arg1 != 6)
            return ret;
    } while (0);
    if ((u8)sub_804DD90(a, 6) != 0)
        ret = 1;
    else if ((u8)sub_804DD90(b, 6) != 0)
        ret = 2;
    return ret;
}
// @ 0x0804F10C
// 在 GetObjPool 的空闲槽里找第一个能命中 sub_804E76C(slot, arg0, arg1) 的槽,
// 返回其内部匹配下标 (0..5), 找不到返回 -1。sub_80489E8 先筛出通过
// sub_8045F10(slot, 0x1FF)==2 的槽下标 (0..4) 填进 values。
// 注: 需要 `int idx` 与 `s8 tmp` 两个中间变量才能复现 GCC2 的调度
// (idx 把乘 0xC8 提前; tmp = result 使截断 lsls/lsrs 排在 cmp 之前)。
s8 sub_804F10C(u8 arg0, u8 arg1)
{
    u8 i;
    u8 count;
    int idx;
    u8 values[5];
    s8 result;
    s8 found;
    s8 tmp;
    u8 *pool;

    found = -1;
    pool = (u8 *)GetObjPool();
    count = sub_80489E8(pool, values, 0, 0x1FF);
    for (i = 0; i < count; i++)
    {
        idx = values[i] * 0xC8;
        result = sub_804E76C((BattleObj *)(pool + idx), arg0, arg1);
        tmp = result;
        if (tmp >= 0)
        {
            found = result;
            break;
        }
    }
    return found;
}
// @ 0x0804F17C
// 在 GetObjPool 空闲槽中找所有通过 sub_804E76C(slot, arg1, arg2) 的槽,
// 把槽下标写入 arg0[0..found-1], 返回命中数量 (与 sub_804F10C 收集版同族)。
// arg0 是**输出槽下标数组** (先全清 0..4), 产出单个槽号字节, 不是 BattleObj*:
// 反汇编里对它是 strb/ldrb 逐字节写 0..4 号元素, 不是对象字段访问。
// 注: 用 r8/r9/sl 三个高位寄存器, 有 GCC2 泄漏风险。
u8 sub_804F17C(u8 *arg0, u8 arg1, u8 arg2)
{
    u8 i;
    u8 found;
    u8 count;
    u8 slots[5];
    u8 *pool;

    for (i = 0; i <= 4; i++)
        arg0[i] = 0;
    pool = (u8 *)GetObjPool();
    count = sub_80489E8(pool, slots, 0, 0x1FF);
    found = 0;
    for (i = 0; i < count; i++)
    {
        if (sub_804E76C((BattleObj *)(pool + slots[i] * 0xC8), arg1, arg2) >= 0)
        {
            arg0[found] = slots[i];
            found++;
        }
    }
    return found;
}
