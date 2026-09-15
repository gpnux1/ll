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

// @ 0x0804473C
u32 sub_804473C(BattleObj *arg0, u8 *arg1)
{
    u16 result;

    if (arg0->slot <= 0xA)
    {
        switch ((s8)arg0->fxKind)
        {
        case 0:
            result = sub_8044A40(arg0, arg1);
            break;
        case 1:
            switch (sub_8048764(arg0))
            {
            case 0:
            case 1:
            case 2:
            case 38:
            case 39:
            case 40:
            case 42:
            case 48:
            case 53:
            case 55:
            case 58:
            case 59:
                result = sub_8044A40(arg0, arg1);
                break;
            case 17:
            case 18:
            case 19:
            case 20:
                result = 0;
                break;
            default:
                result = sub_8044F4C(arg0, arg1);
                break;
            }
            break;
        case 2:
            result = sub_8045098(arg0, arg1);
            break;
        }
    }
    else
    {
        result = sub_8044A40(arg0, arg1);
    }

    return result;
}
// @ 0x080448A8
// 战斗结果技能 HP 恢复写入 (gSkillHealAmount): 按 (s8)obj[0xBC] 选 stats[0x2F] (case1: stats[0x30+obj[0xC2]])
// 作技能 id, switch 分发到 sub_8044A40 (恢复量基准) 并对 12/34/38/41 加 0xF、14/37/40 加 0x1E。
// do-while(0) 为调度屏障: 撑出 result→r1 / kind→r4 的 home (拆掉则二者互换, 头部 8 条全错)。
typedef struct Stats_BattleView
{
    u8 pad[0x2F];
    u8 unk2F;      /* 技能 id (默认槽) */
    u8 unk30[1];   /* obj[0xC2] 索引的替代槽 (声明 [1], 实际长度由表定) */
} Stats_BattleView;

u32 sub_80448A8(BattleObj *arg0, u8 *arg1)
{
    u16 result;
    Stats_BattleView *stats;
    u8 kind;
    s8 bc;

    result = 0;
    stats = *(Stats_BattleView **)(&arg0->animPtr);
    kind = stats->unk2F;
    bc = (s8)arg0->fxKind;
    if (bc != 0)
    {
        if (bc == 1)
        {
            kind = stats->unk30[arg0->animSubIdx];
        }
    }
    do
    {
        switch (kind)
        {
        case 12:
            result = sub_8044A40(arg0, arg1) + 0xF;
            break;
        case 14:
            result = sub_8044A40(arg0, arg1) + 0x1E;
            break;
        case 34:
            result = sub_8044A40(arg0, arg1) + 0xF;
            break;
        case 37:
            result = sub_8044A40(arg0, arg1) + 0x1E;
            break;
        case 38:
            result = sub_8044A40(arg0, arg1) + 0xF;
            break;
        case 40:
            result = sub_8044A40(arg0, arg1) + 0x1E;
            break;
        case 41:
            result = sub_8044A40(arg0, arg1) + 0xF;
            break;
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
        case 11:
        case 13:
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 35:
        case 36:
        case 39:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
            result = sub_8044A40(arg0, arg1);
            break;
        }
    } while (0);
    gSkillHealAmount = result;
    return result;
}
// @ 0x08044A40
INCLUDE_ASM("asm/nonmatchings", sub_8044A40);
// @ 0x08044F4C
u32 sub_8044F4C(BattleObj *arg0, BattleObj *arg1)
{
    u32 bit;
    u16 mod;
    u16 w9;
    u16 v;
    u16 dmg;
    s16 t;

    if ((bit = arg1->state & 0x1000) != 0)
    {
        arg1->state = arg1->state & 0xEFFF;
        return 0;
    }
    if (arg1->state & 0x10)
    {
        arg1->dmgAmount = bit;
        return 0;
    }
    mod = ((u32 (*)(void))Rng_LcgNext)() % (sub_8047024(arg0, 8) / 10);
    w9 = sub_8047024(arg1, 9);
    v = sub_80472E8(arg0, sub_8048764(arg0), 0);
    dmg = (sub_8047024(arg0, 8) + v * arg0->lv + mod - w9) / 2;
    switch (sub_8047D28(arg1, sub_8047DC8(arg0)))
    {
    case 1:
        dmg = dmg * 2;
        break;
    case 2:
        dmg = (s16)dmg / 2;
        break;
    case 0:
        break;
    }
    if (arg0->state & 0x4000)
    {
        dmg = dmg * 2;
        arg0->state = arg0->state & 0xBFFF;
    }
    if (sub_804E6DC((BattleObj *)(arg0), 0xC) >= 0)
    {
        t = dmg;
        dmg = t + t / 10;
    }
    if ((s16)dmg < 0)
    {
        dmg = 0;
    }
    if ((s16)dmg > 999)
    {
        dmg = 999;
    }
    return dmg;
}
// @ 0x08045098
INCLUDE_ASM("asm/nonmatchings", sub_8045098);
// @ 0x0804519C
INCLUDE_ASM("asm/nonmatchings", sub_804519C);
// @ 0x08045328
// 命中判定: 目标 variantClass ∈{2,3,4} 或攻击者 fxKind==1 → 必中。
// 否则按 2*(agl差) + (atc - 目标def) + (arg2 - 攻击者[0x8B]) 算权重 v, 钳制 [30,100];
// 目标带 kind=2/val=8 的装备时 v -= 50; 最后 (Rng%100) < v 判定命中。
// 注: v 必须是有符号 s16 (u16 会被 agbcc 提升为 SImode, 生成形状不同)。
// 注: +0x8B 落在 animPtr(+0x88) 高字节上, 但 sub_8048C80 亦按独立统计量读取, 布局未定, 暂以裸偏移访问。
u8 sub_8045328(BattleObj *arg0, BattleObj *arg1, u8 arg2)
{
    s16 v;
    u8 t;
    u8 result;

    if (sub_8045F10(arg1, 0x1C) == 2)
        result = 1;
    else if ((s8)arg0->fxKind == 1)
        result = 1;
    else
    {
        t = (arg0->slot <= 0xA) ? ((u8 *)arg0)[0x8B] : 0;
        v = (arg0->agl - arg1->agl) * 2 + (arg0->atc - arg1->def) + (arg2 - t);
        if (v > 0x64)
            v = 0x64;
        if (v <= 0x1D)
            v = 0x1E;
        if (sub_804E76C((BattleObj *)arg1, 2, 8) >= 0)
            v -= 50;
        if ((s16)(((u32 (*)(void))Rng_LcgNext)() % 100) < v)
            result = 1;
        else
            result = 0;
    }
    return result;
}
// @ 0x080453D8
u16 sub_80453D8(void)
{
    u8 *base;
    u16 result;
    u8 i;
    u8 *obj;

    base = (u8 *)GetBuf_37410();
    result = 0;
    for (i = 0; i <= 6; i++)
    {
        obj = (u8 *)(i * 0xC8 + (u32)base);
        if (obj[0xAB] == 8)
        {
            if (sub_80187B4() & 0x20)
                result += *(u16 *)(*(u8 **)(obj + 0x88) + 0x26);
            else
                result += *(u16 *)(*(u8 **)(obj + 0x88) + 0x12);
        }
    }
    return result;
}
u16 sub_804542C(void)
{
    u8 *base;
    u16 result;
    u8 i;
    u8 *obj;

    base = (u8 *)GetBuf_37410();
    result = 0;
    for (i = 0; i <= 6; i++)
    {
        obj = (u8 *)(i * 0xC8 + (u32)base);
        if (obj[0xAB] == 8)
        {
            if (sub_80187B4() & 0x20)
                result += *(u16 *)(*(u8 **)(obj + 0x88) + 0x28);
            else
                result += *(u16 *)(*(u8 **)(obj + 0x88) + 0x14);
        }
    }
    if (sub_804F10C(5, 1) >= 0)
    {
        u16 bonus = (u16)(result / 10);
        result = (u16)(result + bonus);
    }
    Silver_Add(result);
    return result;
}
// @ 0x080454A4
// 队伍 EXP 发放: 对对象池前 5 项 (obj[0xAB]∉{7,8} 且 obj[0xBE]!=0xFF) 经 sub_80487A4 映射到
// gPartyStats 加 EXP 并处理升级/封顶, 返回升级位掩码 (u8)。
// 首循环的死读 `idx = ((volatile u8 *)obj)[0xBE];` 是原代码遗留的空扫描 —— 穷举 40+ 非 volatile
// 形态全被 DCE 删掉, ROM 里的死读必然源于原代码的 volatile 读, 故按规则 121 破例保形。
u8 sub_80454A4(u16 arg0)
{
    u32 base;
    u16 new_var;
    u8 result;
    u8 i;
    u8 *obj;
    u8 idx;
    u8 newLevel;

    base = GetObjPool();
    result = 0;
    for (i = 0; i <= 4; i++)
    {
        obj = (u8 *)(i * 0xC8 + base);
        if ((u8)(obj[0xAB] - 7) > 1)
            idx = ((volatile u8 *)obj)[0xBE];
    }
    new_var = arg0;
    for (i = 0; i <= 4; i++)
    {
        obj = (u8 *)(i * 0xC8 + base);
        if ((u8)(obj[0xAB] - 7) <= 1)
            continue;
        if (obj[0xBE] == 0xFF)
            continue;
        idx = sub_80487A4(i);
        if (idx != 0)
            idx--;
        if (gPartyStats[idx].lv > 0x61)
            continue;
        gPartyStats[idx].exp += new_var;
        if (gPartyStats[idx].exp >= gPartyStats[idx].next_exp)
        {
            result |= (u8)(1 << i);
            newLevel = ExpToLevel(gPartyStats[idx].exp);
            gPartyStats[idx].next_exp = LevelToExp(newLevel);
            gPartyStats[idx].lv = ExpToLevel(gPartyStats[idx].exp);
        }
        if (gPartyStats[idx].exp > 0x98967F)
            gPartyStats[idx].exp = LevelToExp(0x61);
    }
    return result;
}
// @ 0x080455A0
u16 sub_80455A0(u8 objectIndex, u8 stat)
{
    u32 base;
    u8 formation;

    base = GetObjPool();
    formation = gBattleFormationIds[*(u8 *)(base + objectIndex * 0xC8 + 0xBB)];
    if (formation != 0)
        formation--;
    switch (stat)
    {
        case 0:
            return gPartyStats[formation].max_hp;
        case 1:
            return gPartyStats[formation].max_mp;
        case 2:
            return gPartyStats[formation].base_atc;
        case 3:
            return gPartyStats[formation].base_def;
        case 4:
            return gPartyStats[formation].base_agl;
        case 5:
            return gPartyStats[formation].base_men;
        case 6:
            return gPartyStats[formation].base_res;
        case 7:
            return gPartyStats[formation].base_noa;
    }
}
// @ 0x08045688
void sub_8045688(u8 objectIndex, u8 stat, u8 val)
{
    u32 base;
    u8 formation;

    base = GetObjPool();
    formation = gBattleFormationIds[*(u8 *)(base + objectIndex * 0xC8 + 0xBB)];
    if (formation != 0)
        formation--;
    switch (stat)
    {
        case 0:
            gPartyStats[formation].max_hp += val;
            break;
        case 1:
            gPartyStats[formation].max_mp += val;
            break;
        case 2:
            gPartyStats[formation].base_atc += val;
            gPartyStats[formation].atc += val;
            break;
        case 3:
            gPartyStats[formation].base_def += val;
            gPartyStats[formation].def += val;
            break;
        case 4:
            gPartyStats[formation].base_agl += val;
            gPartyStats[formation].agl += val;
            break;
        case 5:
            gPartyStats[formation].base_men += val;
            gPartyStats[formation].men += val;
            break;
        case 6:
            gPartyStats[formation].base_res += val;
            gPartyStats[formation].res += val;
            break;
        case 7:
            gPartyStats[formation].base_noa += val;
            gPartyStats[formation].noa += val;
            break;
    }
}
// @ 0x080457AC
void sub_80457AC(void)
{
    u32 pool;
    u8 f;
    u8 i;
    PlayerStats *st;
    u8 *ob;

    pool = GetObjPool();
    for (i = 0; i <= 4; i++)
    {
        f = gPartyMemberIds[i];
        if (f == 0xff)
            continue;
        if (f != 0)
            f--;
        st = &gPartyStats[f];
        ob = (u8 *)(pool + i * 0xC8);
        st->hp = *(u16 *)(ob + 0x6C) ? *(u16 *)(ob + 0x6C) : 1;
        do
        {
        } while (
            0); /* GCC2 arm_reorg 调度屏障: 强制 fu[1] 的 movs r6,#0 落到 mp ldrh 延迟槽(目标 0x46), 否则被 hoist 到 hp store 前 */
        st->mp = *(u16 *)(ob + 0x70);
        st->equip_slot5 = ob[0x91];
        st->equip_slot6 = ob[0x92];
        if (ob[0x91] == 0xb3 || ob[0x92] == 0xb3)
            st->field_unk[1] = 0;
        else
            st->field_unk[1] = *(u16 *)(ob + 0x88);
        Stats_BuildSkillList(&st->skills[0], st->lv, gPartyMemberIds[i]);
    }
}
// @ 0x08045860
s8 sub_8045860(u8 objectIndex, u8 *buf)
{
    u32 pool;
    u8 *obj;
    u8 formation;
    s8 c;
    u8 id;
    u32 p1;

    p1 = GetObjPool();
    formation = gBattleFormationIds[*(u8 *)(p1 + objectIndex * 0xC8 + 0xBB)];
    pool = GetObjPool();
    for (c = 0; c <= 7; c++)
        buf[c] = 0xff;
    if (formation != 0)
        formation--;
    obj = (u8 *)(pool + objectIndex * 0xC8);
    c = 0;
    while (obj[0xAA] < gPartyStats[formation].lv)
    {
        obj[0xAA]++;
        id = ItemFindSlot(obj[0xAA], gBattleFormationIds[*(u8 *)(GetObjPool() + objectIndex * 0xC8 + 0xBB)]);
        if (id != 0xff)
        {
            if (sub_8048868(objectIndex, id) != 0)
            {
                if (c <= 7)
                {
                    buf[c] = id - 1;
                    c++;
                }
            }
        }
    }
    return c;
}
// @ 0x08045940
u8 sub_8045940(BattleObj *obj, u8 *buf)
{
    u8 i;
    u8 count;

    for (i = 0; i <= 7; i++)
        buf[i] = 0;

    count = 0;
    for (i = 0; i <= 7; i++)
    {
        if (sub_80488CC((u8 *)obj, obj->skills[i]) == 0xFF)
            continue;

        switch (obj->slot)
        {
        case 0:
            buf[count] = i;
            count++;
            break;
        case 1:
            buf[count] = i;
            count++;
            break;
        case 6:
            buf[count] = i;
            count++;
            break;
        case 7:
            buf[count] = i;
            count++;
            break;
        case 2:
            switch (obj->skills[i])
            {
            case 8:
            case 0xD:
                buf[count] = i;
                count++;
                break;
            }
            break;
        case 3:
            switch (obj->skills[i])
            {
            case 0xE:
            case 0xF:
            case 0x10:
                buf[count] = i;
                count++;
                break;
            }
            break;
        case 4:
            switch (obj->skills[i])
            {
            case 0x16:
            case 0x17:
            case 0x18:
            case 0x1A:
            case 0x1B:
            case 0x1C:
                buf[count] = i;
                count++;
                break;
            }
            break;
        case 5:
            switch (obj->skills[i])
            {
            case 0x1E:
            case 0x1F:
            case 0x21:
                buf[count] = i;
                count++;
                break;
            }
            break;
        }
    }
    return count;
}
// @ 0x08045A10
// 技能 MP 充足性校验: 判断角色当前 MP 是否足够释放该技能
// 消耗量计算与姊妹函数 sub_8048934 / sub_8045B90 同构: 查技能表后经两次 sub_804E76C 状态检查 (-2 或折半)
u8 sub_8045A10(BattleObj *obj, u8 index)
{
    u16 currentMp;
    u8 skillId;
    u8 mpCost;
    struct { u64 w; } s;
    s32 diff;

    currentMp = obj->mp;
    s.w = 1;
    skillId = obj->skills[index];
    mpCost = gSkillItemTable[skillId * 5 + 4]; /* 对应 SkillItemEntry.mpCost */
    if (sub_804E76C(obj, 3, s.w) >= 0)
        mpCost = mpCost - 2;
    if (sub_804E76C(obj, 3, 2) >= 0)
        mpCost >>= 1;

    s.w = mpCost;
    diff = (s16)currentMp - s.w;
    if ((s16)diff < 0)
        return 0;
    return 1;
}
// @ 0x08045A74
// 从 list[0..count-1] 中按 obj 槽(0xC8)的字段阈值筛选:
// t1=(u16)(field_6e/10 * arg3), t2=(u16)(field_72/10 * arg3);
// mode 0: field_6c < t1; mode 1: field_70 < t2; mode 2: 两者都满足。
// 命中的索引写回 list[0..n-1], 返回命中数 n。
u8 sub_8045A74(u8 *obj, u8 *list, u8 count, u8 arg3, u8 mode)
{
    u8 buf[5];
    u8 i;
    u8 j;
    u8 *o;
    u16 s1;
    u16 s2;
    u16 t1;
    u16 t2;

    for (j = 0; j <= 4; j++)
        buf[j] = 0;
    arg3 /= 10;
    i = 0;
    for (j = 0; j < count; j++)
    {
        o = obj + (u32)list[j] * 0xC8;
        s1 = *(u16 *)(o + 0x6c);
        s2 = *(u16 *)(o + 0x70);
        t1 = (u16)(*(u16 *)(o + 0x6e) / 10);
        t2 = (u16)(*(u16 *)(o + 0x72) / 10);
        t1 = (u16)(t1 * arg3);
        t2 = (u16)(t2 * arg3);
        switch (mode)
        {
        case 0:
            if (s1 < t1)
            {
                buf[i] = list[j];
                i++;
            }
            break;
        case 1:
            if (s2 < t2)
            {
                buf[i] = list[j];
                i++;
            }
            break;
        case 2:
            if (s1 < t1 && s2 < t2)
            {
                buf[i] = list[j];
                i++;
            }
            break;
        default:
            break;
        }
    }
    for (j = 0; j < i; j++)
        list[j] = buf[j];
    return i;
}
// @ 0x08045B90
void sub_8045B90(BattleObj *obj, u8 index)
{
    u16 original;
    u16 *current;
    u8 amount;
    u8 id;
    s32 wide;
    u8 *data;

    current = &(obj->mp);
    original = *current;
    data = (u8 *)current;
    data += 0x29;
    data += index;
    id = *data;
    amount = gSkillItemTable[id * 5 + 4];
    if (sub_804E76C((BattleObj *)(obj), 3, 1) >= 0)
        amount = amount - 2;
    if (sub_804E76C(obj, 3, 2) >= 0)
        amount >>= 1;
    wide = (s16)original;
    original = wide - amount;
    if ((s16)original < 0)
        original = 0;
    *current = original;
}
// @ 0x08045BF4
// 按 obj->slot (0-10) 分派写 +0x8A 字段; 各分支直写该字段, GCC2 尾合并成末尾一次 strb;
// equipSlots[4]/[5] ==0xCB/0xBF 判定 + equipSlots[0]==0x15/0x3B/0x3C 细分; >10 落 default
// (无 default 分支直落函数尾)。
// 调用者 = 玩家侧装载器 sub_80200E8 (装完 equipSlots/skills 后立即调用) 与
// battle_obj_core.c:2274; 写值 0x30..0x35 / 0x37..0x3B (每槽基值+换装变体) / 0xFF。
// 注: +0x8A 落在 animPtr(+0x88..0x8B) 内部, 且 +0x8A..0x8B 又被 battle_obj_core.c:633
// (`*(u16*)&obj->animPtr + 1`) 与 sub_80488CC:1518 按 u16 阈值读取, 布局未定,
// 故按本文件 240 行的既定做法保留裸偏移; +0x8D/0x91/0x92 已改用 obj->equipSlots。
void sub_8045BF4(BattleObj *obj)
{
    switch (obj->slot)
    {
    case 0:
    case 1:
        if (obj->equipSlots[0] == 0x15)
        {
            if (obj->equipSlots[4] == 0xCB || obj->equipSlots[5] == 0xCB)
                *((u8 *)obj + 0x8A) = 0x37;
            else
                *((u8 *)obj + 0x8A) = 0x30;
        }
        else
            *((u8 *)obj + 0x8A) = 0x30;
        break;
    case 2:
        *((u8 *)obj + 0x8A) = 0x31;
        break;
    case 3:
        if (obj->equipSlots[4] == 0xBF || obj->equipSlots[5] == 0xBF)
            *((u8 *)obj + 0x8A) = 0x38;
        else
            *((u8 *)obj + 0x8A) = 0x32;
        break;
    case 4:
        if (obj->equipSlots[4] == 0xBF || obj->equipSlots[5] == 0xBF)
            *((u8 *)obj + 0x8A) = 0x39;
        else
            *((u8 *)obj + 0x8A) = 0x33;
        break;
    case 5:
        if (obj->equipSlots[0] == 0x3B)
            *((u8 *)obj + 0x8A) = 0x3A;
        else
            *((u8 *)obj + 0x8A) = 0x34;
        break;
    case 6:
        if (obj->equipSlots[0] == 0x3C)
            *((u8 *)obj + 0x8A) = 0x3B;
        else
            *((u8 *)obj + 0x8A) = 0x35;
        break;
    case 7:
    case 8:
    case 9:
    case 10:
        *((u8 *)obj + 0x8A) = 0xFF;
        break;
    }
}
// @ 0x08045D00
/* 战斗对象池的"目标候选收集器": 遍历对象池 (0xC8 步长), 按 mode 把符合条件的
 * 槽号写进输出数组 arg3 (先全填 -1), 供上层对象选择/指向使用。
 *   arg1 = mode (来自 sub_80489A4 → 技能/敌种属性元素):
 *     0/4 → 只写锚点对象自身槽号 arg2;
 *     1   → 敌侧槽 5..0xB 中 +0xAC 低 nibble (属性族) 与锚点相同的;
 *     2   → 敌侧槽 5..0xB 中 (+0xAC & 0xF0) 命中锚点外形组 (0x10→0x10, 0x20/0x30→0x30)
 *           或属 0x20 组的;
 *     3   → 全部敌侧槽 5..0xB;
 *     5   → 全部我方槽 0..4;
 *     6   → 不收集。
 * +0xAC 是装载时由 gUnk_08393B14[memberId] 写入的外形/属性族字节 (高4=外形, 低4=族)。
 * 形状要点 (GCC2.9): ①arg2 声明为 u16 且首句 `arg2 = (u8)arg2;` —— u8/i32 都会让
 * 截断的 lsls/lsrs 目标寄存器变异 (差 2B); ②case 1 的锚点属性必须重写成完整指针
 * 表达式 `*(u8*)(arg0 + arg2*0xC8 + 0xAC) & 0xF` 而非缓存进 sel —— 否则 GCC 把
 * `sel & 0xF` 当循环不变量提到循环外, 少一整个 ldrb (分数 1845→10); 缓存的 sel 仅
 * 供 case 2 使用。 */
void sub_8045D00(BattleObj *arg0, u8 arg1, u16 arg2, s8 *arg3)
{
    s8 found;
    s8 i;
    u8 sel;

    arg2 = (u8)arg2;
    found = 0;
    sel = *((u8 *)arg0 + arg2 * 0xC8 + 0xAC);

    for (i = 0; i <= 6; i++)
        arg3[i] = -1;

    switch (arg1)
    {
    case 3:
        for (i = 5; i <= 0xB; i++)
        {
            if (*((u8 *)arg0 + i * 0xC8 + 0xBE) != 0xFF)
            {
                arg3[found] = i;
                found++;
            }
        }
        break;
    case 1:
        for (i = 5; i <= 0xB; i++)
        {
            if (*((u8 *)arg0 + i * 0xC8 + 0xBE) != 0xFF)
            {
                if ((*((u8 *)arg0 + i * 0xC8 + 0xAC) & 0xF) == (*((u8 *)arg0 + arg2 * 0xC8 + 0xAC) & 0xF))
                {
                    arg3[found] = i;
                    found++;
                }
            }
        }
        break;
    case 2:
    {
        u8 mask;
        switch (sel & 0xF0)
        {
        case 0x10:
            mask = 0x10;
            break;
        case 0x20:
        case 0x30:
            mask = 0x30;
            break;
        }
        for (i = 5; i <= 0xB; i++)
        {
            u8 v;
            if (*((u8 *)arg0 + i * 0xC8 + 0xBE) == 0xFF)
                continue;
            v = *((u8 *)arg0 + i * 0xC8 + 0xAC) & 0xF0;
            if (v == mask || v == 0x20)
            {
                arg3[found] = i;
                found++;
            }
        }
        break;
    }
    case 0:
    case 4:
        arg3[found] = arg2;
        break;
    case 5:
        for (i = 0; i <= 4; i++)
        {
            if (*((u8 *)arg0 + i * 0xC8 + 0xBE) != 0xFF)
            {
                arg3[found] = i;
                found++;
            }
        }
        break;
    case 6:
        break;
    }
}
/* obj+0x6C 起是 0x21 字节的移动/上下文块 (MOD-04: +0x6C/6E = 移动坐标),
 * 紧随其后的 obj+0x8D..0x92 是 6 个角色编号。必须用 Sub6C 视图取号:
 * 写成 `obj + 0x8D` 会被折成单一基址, 目标里 "(p + 0x21) + i" 的三条
 * adds 就少一条; 写成 `base[0x21 + i]` 又变成 "r6 + r5 + 0x21" 的两条。
 * 语义: 逐个编号查 sub_804DD90(id, 1) (= AI 行为字节 bit6-7 分类),
 * 命中 1/2/3 就把 obj+0xB8 的 u16 标志位 0/1/2 置起来。
 * 末尾 `*flags |= extra` 是**原代码遗留的空操作**: extra 恒为 0, GCC2 的
 * CSE 把这条 RMW 整个折叠掉, 但 flow 已按"被读过"把 extra 判成 live,
 * 于是只留下 `movs r7, #0` —— 这就是目标第 4 个 callee-saved 寄存器的
 * 全部痕迹。删掉这一行 → push 退化成 {r4, r5, r6, lr}, 差 10 字节。
 * (同族先例见 RULES 规则 89 / 坑11。) */
typedef struct
{
    /* 0x00 */ u8 pad[0x21];
    /* 0x21 */ u8 ids[6];
} Sub6C;

// @ 0x08045EB8
void sub_8045EB8(u8 *obj)
{
    Sub6C *entry;
    u8 i;
    u16 *flags;
    u8 extra;

    entry = (Sub6C *)(obj + 0x6C);
    i = 0;
    flags = (u16 *)(obj + 0xB8);
    extra = 0;
    for (; i <= 5; i++)
    {
        switch (sub_804DD90(entry->ids[i], 1))
        {
            case 1:
                *flags |= 1;
                break;
            case 2:
                *flags |= 2;
                break;
            case 3:
                *flags |= 4;
                break;
        }
    }
    *flags |= extra;
}
// @ 0x08045F10
u8 sub_8045F10(BattleObj *obj, u16 dirMask)
{
    u8 result;
    u8 dir;
    result = 0;
    if (obj->slot != 0xFF)
    {
        result = 1;
        dir = obj->variantClass;
        if (dir <= 8)
        {
            switch (dir)
            {
                case 0:
                    if (1 & dirMask)
                    {
                        result = 2;
                    }
                    break;

                case 1:
                    if (2 & dirMask)
                    {
                        result = 2;
                    }
                    break;

                case 2:
                    if (4 & dirMask)
                    {
                        result = 2;
                    }
                    break;

                case 3:
                    if (8 & dirMask)
                    {
                        result = 2;
                    }
                    break;

                case 4:
                    if (16 & dirMask)
                    {
                        result = 2;
                    }
                    break;

                case 5:
                    if (32 & dirMask)
                    {
                        result = 2;
                    }
                    break;

                case 6:
                    if (0x40 & dirMask)
                    {
                        result = 2;
                    }
                    break;

                case 7:
                    if (0x80 & dirMask)
                    {
                        result = 2;
                    }
                    break;

                case 8:
                    if (0x100 & dirMask)
                    {
                        result = 2;
                    }
                    break;
            }
        }
    }
    return result;
}
// @ 0x08045F94
// 姊妹 sub_8046060: 方向槽 obj[0xAB] 只增不减; CBA4 普通行走第2实参=0x4 (vs 0xA)。
// 见 sub_8046060 注释与 RULES 规则135 (moveBits/walkOfs/stateFlags 三段链)。
void sub_8045F94(BattleObj *obj, u16 arg1)
{
    u32 zero;
    u16 moveBits;
    int stateFlags;
    int walkOfs;

    if (((u8 *)obj)[0xBE] > 0x70 && arg1 != 8)
        return;
    if (((u8 *)obj)[0xAB] >= arg1)
        return;
    zero = 0;
    ((u8 *)obj)[0xAB] = arg1;
    switch (arg1)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            sub_801D12C((BattleObj *)obj, 1);
            if (((u8 *)obj)[0xBE] <= 0xA)
                sub_801CBA4((BattleObj *)obj, 4, *(u16 *)((u8 *)obj + 0x2A), ((u8 *)obj)[0x35], zero);
            else
                sub_801CA08((BattleObj *)obj, 0, *(u16 *)((u8 *)obj + 0x2A), ((u8 *)obj)[0x35], zero);
            break;
        case 8:
            sub_801D12C((BattleObj *)obj, 2);
            break;
    }
    switch (arg1)
    {
        case 1:
            stateFlags = 0x10 | *(u16 *)((u8 *)obj + 0xB8);
            *(u16 *)((u8 *)obj + 0xB8) = stateFlags;
        case 2:
        case 3:
        case 5:
        case 6:
            moveBits = 0x200 | *(u16 *)((u8 *)obj + 0xB0);
            walkOfs = 0;
            stateFlags = moveBits;
            *(u16 *)((u8 *)obj + 0xB0) = stateFlags;
            ((u8 *)obj)[0xA8] = walkOfs;
            break;
    }
}
// @ 0x08046060
// 对象行动状态推进: 槽号 obj[0xBE]>0x70 时仅接受 arg1==8 (撤退/移除), 否则忽略;
// 方向槽 obj[0xAB] 只增不减 (arg1<=旧值直接忽略)。arg1 1..7 = 步行/移动:
// 先 Obj_SetMoveState(obj,1), 再按对象种类 (obj[0xBE]<=0xA 走普通行走 sub_801CBA4(…,0xA),
// >0xA 走事件类 sub_801CA08) 传 (obj, f2A, f35, 0)。arg1==8 = 离场: Obj_SetMoveState(obj,2)。
// 位段收尾 (arg1 1..7 落入): obj[0xB8]|=0x10 (case1 fallthrough), obj[0xB0]|=0x200, obj[0xA8]=0。
// 第5实参 0 与 obj[0xA8]=0 的 0 是两个独立变量 (r7/r3 分配, 合并会破分配, 见 RULES 规则87家族)。
void sub_8046060(BattleObj *obj, u16 arg1)
{
    u32 zero;
    u16 moveBits;
    int stateFlags;
    int walkOfs;

    if (((u8 *)obj)[0xBE] > 0x70 && arg1 != 8)
        return;
    if (((u8 *)obj)[0xAB] >= arg1)
        return;
    zero = 0;
    ((u8 *)obj)[0xAB] = arg1;
    switch (arg1)
    {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            sub_801D12C((BattleObj *)obj, 1);
            if (((u8 *)obj)[0xBE] <= 0xA)
                sub_801CBA4((BattleObj *)obj, 0xA, *(u16 *)((u8 *)obj + 0x2A), ((u8 *)obj)[0x35], zero);
            else
                sub_801CA08((BattleObj *)obj, 0, *(u16 *)((u8 *)obj + 0x2A), ((u8 *)obj)[0x35], zero);
            break;
        case 8:
            sub_801D12C((BattleObj *)obj, 2);
            break;
    }
    switch (arg1)
    {
        case 1:
            stateFlags = 0x10 | *(u16 *)((u8 *)obj + 0xB8);
            *(u16 *)((u8 *)obj + 0xB8) = stateFlags;
        case 2:
        case 3:
        case 5:
        case 6:
            moveBits = 0x200 | *(u16 *)((u8 *)obj + 0xB0);
            walkOfs = 0;
            stateFlags = moveBits;
            *(u16 *)((u8 *)obj + 0xB0) = stateFlags;
            ((u8 *)obj)[0xA8] = walkOfs;
            break;
    }
}
// @ 0x0804612C
// 对象状态推进 (sub_8046060 家族变体): 方向槽 obj[0xAB] 为 arg1-2 或 arg1==0xB 时进入;
// arg1==0xB 且方向==8 直接返回。方向==8 = 入池: 在 GetObjPool 槽里找 field_BE 与
// obj[0xBE] 匹配的槽下标 i, 调 sub_801DD04(obj, i, *(u16*)(obj+0x6C) + arg2) 挂到节点链表;
// 否则清方向后按 obj[0xBE]<=0xA 走 sub_801CBA4/CA08。尾部清 obj[0xB8] bit4。
// 注意: prev 用 (u16)(arg1+0xFFFE) 写法 (u16 回绕减 2, 见 RULES 135 家族);
// pool 指针池扫后复用为 &obj[0xB8] (规则87: 一个局部兼职两个值买寄存器 home)。
void sub_804612C(BattleObj *obj, u16 arg1, u16 arg2)
{
    u8 *pool;
    u8 i;
    u8 zero;
    u16 prev;
    u16 v;

    prev = (u16)(arg1 + 0xFFFE);
    if (obj->variantClass != prev && arg1 != 0xb)
        return;
    if (arg1 == 0xb && obj->variantClass == 8)
        return;
    if (obj->variantClass == 8)
    {
        pool = (u8 *)GetObjPool();
        for (i = 0; i <= 4; i++)
            if (pool[i * 0xC8 + 0xBE] == obj->slot)
                break;
        v = obj->hp;
        sub_801DD04((BattleObj *)obj, i, (u16)(v + arg2));
    }
    else
    {
        zero = 0;
        obj->variantClass = 0;
        if (obj->slot <= 0xa)
            sub_801CBA4((BattleObj *)obj, 0, obj->headA.f_1E, obj->headA.palSlot, zero);
        else
            sub_801CA08((BattleObj *)obj, 0, obj->headA.f_1E, obj->headA.palSlot, 0);
    }
    if (obj->statusAil & 0x10)
    {
        pool = &obj->statusAil;
        *(u16 *)pool = obj->statusAil & 0xFFEF;
    }
    sub_801D12C((BattleObj *)obj, 0);
}
// @ 0x0804621C
// 敌方对象池槽位筛选:
// 清空 outSlots[0..6] 与 localSlots[0..6]; 收集活动敌方槽位 (5..11, sub_8045F10==2) 到 localSlots;
// 再按 (pool[slot]->pad_AC[0] & mask) == (obj->pad_AC[0] & mask) 过滤出匹配槽位写入 outSlots 并返回数量。
u8 sub_804621C(BattleObj *obj, u8 *outSlots, u8 mask)
{
    u8 localSlots[12];
    u8 *buf;
    BattleObj *pool;
    u8 targetVal;
    u8 count;
    u8 outCount;
    u8 i;
    u8 j;
    u8 endSlot = 12;

    targetVal = obj->pad_AC[0] & mask;
    pool = (BattleObj *)GetObjPool();

    for (j = 0; j <= 6; j++)
        outSlots[j] = 0;

    buf = localSlots;
    count = 0;
    for (i = 0; i <= 6; i++)
        buf[i] = 0;

    i = 5;
    do {
        if (sub_8045F10(&pool[i], 0x1FF) == 2) {
            buf[count] = i;
            count++;
        }
        i++;
    } while (i < endSlot);

    i = count;
    outCount = 0;
    for (j = 0; j < i; j++) {
        if ((pool[localSlots[j]].pad_AC[0] & mask) == targetVal) {
            outSlots[outCount] = localSlots[j];
            outCount++;
        }
    }
    return outCount;
}
// @ 0x080462E4
INCLUDE_ASM("asm/matchings", sub_80462E4); /* 函数清单修正: yaml=[1] 且 .s 已在 matchings/ (坑7) */
// @ 0x08046480
// 行动槽位处理: mode==0 时清空 buf[0..4], 否则清空 buf[0..6]; 用 sub_80462E4(arg0, local, 0x7F)
// 收集槽号到 local[12], 逐个把 local[i] 写进 buf[i], 对满足 sub_8045328(arg0, 槽对象, 0x50)==1
// 且槽对象 field_AB==3 且 Rng%100<=0x45 的槽调 sub_804612C(槽对象, 5, 0), 然后置 buf[i]|=0x10。
u32 sub_8046480(BattleObj *arg0, u8 *buf, u8 mode)
{
    u8 local[12];
    u8 i;
    u8 count;
    u8 *pool;
    u32 stride;

    if (mode == 0)
    {
        for (i = 0; i <= 4; i++)
            buf[i] = 0;
    }
    else
    {
        for (i = 0; i <= 6; i++)
            buf[i] = 0;
    }
    pool = (u8 *)GetObjPool();
    count = sub_80462E4((BattleObj *)arg0, local, 0x7F);
    for (i = 0; i < count; i++)
    {
        stride = 0xC8;
        buf[i] = local[i];
        if ((u8)sub_8045328((BattleObj *)arg0, (BattleObj *)(pool + local[i] * stride), 0x50) == 1)
        {
            if (*(u8 *)(buf[i] * stride + (u32)pool + 0xAB) == 3)
            {
                if (((u32 (*)(void))Rng_LcgNext)() % 100 <= 0x45)
                {
                    sub_804612C((BattleObj *)(pool + buf[i] * stride), 5, 0);
                }
            }
            buf[i] |= 0x10;
        }
    }
    return count;
}
// @ 0x08046558
INCLUDE_ASM("asm/nonmatchings", sub_8046558);
// @ 0x0804666C
// 行动点收集+处理: 清空 buf[0..4] 后调 sub_804DE8C (重置道具/状态区), 收集 obj[0xBE]<=0xA 且
// sub_8045F10(obj,0x43)==2 的对象槽号到 buf, 再对每个收集到的槽号调 sub_80466F0(obj, 槽号)。
// 与已匹配 sub_8046C50 同构 (0x6C50 是写 0xBC=4, 本函数是调 sub_80466F0 处理)。
void sub_804666C(void)
{
    u8 indices[8];
    u8 *buffer;
    u8 *base;
    u8 count;
    u8 i;
    u8 j;
    u8 max;
    u8 limit;

    base = (u8 *)GetObjPool();
    sub_804DE8C();
    buffer = indices;
    count = 0;
    for (i = 0; i <= 4; i++)
        buffer[i] = 0;
    i = 0;
    max = 5;
    while (i < max)
    {
        if (sub_8045F10((BattleObj *)(base + i * 0xC8), 0x43) == 2)
        {
            buffer[count] = i;
            count++;
        }
        i++;
    }
    limit = count;
    for (j = 0; j < limit; j++)
    {
        u8 idx = indices[j];
        sub_80466F0(base + idx * 0xC8, idx);
    }
}
// @ 0x080466F0
INCLUDE_ASM("asm/nonmatchings", sub_80466F0);
// @ 0x08046C50
void sub_8046C50(void)
{
    u8 indices[8];
    u8 *buffer;
    u8 *base;
    u8 count;
    u8 i;
    u8 j;
    u8 max;
    u8 limit;

    base = (u8 *)GetObjPool();
    buffer = indices;
    count = 0;
    for (i = 0; i <= 4; i++)
        buffer[i] = 0;
    i = 0;
    max = 5;
    while (i < max)
    {
        if (sub_8045F10((BattleObj *)(base + i * 0xC8), 0x43) == 2)
        {
            buffer[count] = i;
            count++;
        }
        i++;
    }
    limit = count;
    for (j = 0; j < limit; j++)
        base[indices[j] * 0xC8 + 0xBC] = 4;
}
// @ 0x08046CD4
INCLUDE_ASM("asm/nonmatchings", sub_8046CD4);
// @ 0x08046E18
INCLUDE_ASM("asm/nonmatchings", sub_8046E18);
// @ 0x08046F0C
// 对象属性取值器: 按 gStatRecalcKind (battle stat 索引) 分派读取对象槽字段; case5-9 = 基础值+修正值
// (0x74..0x7C 与 0x7E..0x86 成对相加截断 u16); 越界索引(>14)返回入口 r0 (无显式 return, GCC2 直落 bx lr)。
u16 sub_8046F0C(BattleObj *obj)
{
    switch (gStatRecalcKind)
    {
    case 0:
        return obj->lv;
    case 1:
        return obj->hp;
    case 2:
        return obj->maxHp;
    case 3:
        return obj->mp;
    case 4:
        return obj->maxMp;
    case 5:
        return obj->atc + obj->statMods[0];
    case 6:
        return obj->def + obj->statMods[1];
    case 7:
        return obj->agl + obj->statMods[2];
    case 8:
        return obj->men + obj->statMods[3];
    case 9:
        return obj->res + obj->statMods[4];
    case 10:
        return obj->noa;
    case 11:
        return obj->posX;
    case 12:
        return obj->posY;
    case 13:
        return obj->pad_AC[0] & 0xF;
    case 14:
        return obj->pad_AC[0] >> 4;
    }
}
// @ 0x08047024
INCLUDE_ASM("asm/nonmatchings", sub_8047024);
// @ 0x080471AC
INCLUDE_ASM("asm/nonmatchings", sub_80471AC);
// @ 0x080472E8
INCLUDE_ASM("asm/nonmatchings", sub_80472E8);
// @ 0x0804753C
INCLUDE_ASM("asm/nonmatchings", sub_804753C);
// @ 0x080476DC
INCLUDE_ASM("asm/nonmatchings", sub_80476DC);
// @ 0x08047B1C
INCLUDE_ASM("asm/nonmatchings", sub_8047B1C);
// @ 0x08047D28
u8 sub_8047D28(BattleObj *obj, u8 mask)
{
    u8 result;
    u16 flags1;
    u16 flags2;
    u8 i;
    int v;
    u8 bit;

    result = 0;
    flags2 = 0;
    flags1 = 0;
    if (obj->slot <= 0xA)
    {
        if (sub_804E76C(obj, 2, 6) >= 0)
            flags2 = 3;
        if (sub_804E76C(obj, 2, 7) >= 0)
            flags2 = 0xC;
    }
    else if ((u8)(obj->slot - 0xC) <= 0x64)
    {
        flags1 = *(u16 *)(obj->animPtr + 0x16);
        flags2 = *(u16 *)(obj->animPtr + 0x18);
    }
    else if (obj->slot > 0x70)
    {
        flags1 = *(u16 *)(obj->animPtr + 0x2A);
        flags2 = *(u16 *)(obj->animPtr + 0x2C);
    }
    i = 0;
    bit = 1;
    for (; i <= 3; i++)
    {
        v = (bit << i) & mask;
        if ((flags1 & v) != 0)
            result = 1;
        else if ((flags2 & v) != 0)
            result = 2;
    }
    return result;
}
// @ 0x08047DC8
INCLUDE_ASM("asm/nonmatchings", sub_8047DC8);
// @ 0x08047FCC
s32 sub_8047FCC(u16 arg0)
{
    s8 ret = 0;
    if (arg0 <= 0x39)
    {

        switch (arg0)
        {
            case 0:
                return ret;
            case 1:
                ret = 0;
                break;
            case 2:
                ret = 0xff;
                break;
            case 3:
                ret = 0xfd;
                break;
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                ret = 0xff;
                break;
            case 10:
                ret = 0;
                break;
            case 11:
                ret = 0xfe;
                break;
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 19:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
                ret = 0xff;
                break;
            case 26:
                ret = 7;
                break;
            case 27:
                ret = 0x1e;
                break;
            case 28:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
            case 35:
            case 36:
            case 37:
            case 38:
            case 39:
            case 40:
            case 41:
            case 42:
            case 43:
            case 44:
            case 45:
            case 46:
            case 47:
            case 48:
            case 49:
            case 50:
            case 51:
            case 52:
            case 53:
            case 54:
            case 55:
            case 56:
            case 57:
                ret = 0xff;
                break;
        }
    }
    return ret;
}
// @ 0x080480EC
INCLUDE_ASM("asm/nonmatchings", sub_80480EC);
// @ 0x080481B8
INCLUDE_ASM("asm/nonmatchings", sub_80481B8);
// @ 0x08048310
INCLUDE_ASM("asm/nonmatchings", sub_8048310);
// @ 0x08048458
INCLUDE_ASM("asm/nonmatchings", sub_8048458);
// @ 0x080485A4
INCLUDE_ASM("asm/nonmatchings", sub_80485A4);
// @ 0x08048690
INCLUDE_ASM("asm/nonmatchings", sub_8048690);
// @ 0x08048764
u8 sub_8048764(u8 *arg0)
{
    u8 val;
    u8 *ptr;

    val = arg0[0xA1];
    if (val <= 7)
    {
        ptr = arg0 + 0x99;
        return ptr[val];
    }
    else
    {
        return val;
    }
}

// @ 0x0804877C
u8 sub_804877C(u8 arg0)
{
    u8 index;

    index = sub_80487A4(arg0);
    if (index != 0)
    {
        index--;
    }

    return gPartyStats[index].lv;
}
// @ 0x080487A4
u8 sub_80487A4(u8 arg0)
{
    u32 base;

    base = GetObjPool();
    return gBattleFormationIds[*(u8 *)(base + arg0 * 0xC8 + 0xBB)];
}
// @ 0x080487CC
u8 sub_80487CC(u8 arg0)
{
    u8 i;
    u8 ret;

    ret = sub_80187A8();
    if (ret == 0xA1 || ret == 0xA7)
    {
        return 2;
    }
    for (i = 0; i <= 5; i++)
    {
        if (gBattleFormationIds[i] == gPartyMemberIds[arg0])
        {
            return i;
        }
    }
    return 0;
}
// @ 0x08048818
u16 sub_8048818(u8 objectIndex, u8 arg1)
{
    u32 base;
    u32 formation;
    u8 idx;

    base = GetObjPool();
    formation = gBattleFormationIds[*(u8 *)(base + objectIndex * 0xC8 + 0xBB)];
    idx = formation;
    if (formation != 0)
        idx = formation - 1;
    return sub_8009F70(formation, gPartyStats[idx].lv, arg1);
}
// @ 0x08048868
u8 sub_8048868(u8 objectIndex, u8 skill)
{
    u8 formation;
    u8 i;
    u32 base;

    base = GetObjPool();
    formation = gBattleFormationIds[*(u8 *)(base + objectIndex * 0xC8 + 0xBB)];
    if (formation != 0)
        formation--;

    for (i = 0; i <= 7; i++)
    {
        if (gPartyStats[formation].skills[i] == skill)
            break;
    }
    if (i <= 7)
        return 0;
    return 1;
}
// @ 0x080488CC
u8 sub_80488CC(u8 *obj, u8 skill)
{
    u8 i;
    u8 found;
    u8 first;
    u8 val;
    u8 *skills;
    u8 result;

    result = 0xFF;
    found = 0;
    if (skill <= 0x2F)
    {
        i = 0;
        first = *(obj + 0x99);
        skills = obj + 0x99;
        if (first == 0xFF || first != skill)
        {
            for (;;)
            {
                i++;
                if (i > 7)
                    break;
                val = skills[i];
                if (val == 0xFF || val != skill)
                    continue;
                found = 1;
                break;
            }
            if (found == 0)
                return result;
        }
        if (sub_8045A10(obj, i) != 0)
            result = i;
    }
    else
    {
        if (*(u16 *)(obj + 0x88) > 0x1F)
            result = *(obj + 0x8A);
    }
    return result;
}

// @ 0x08048934
u8 sub_8048934(BattleObj *arg0, u8 arg1)
{
    u8 b;
    u8 val;
    u8 *tbl;
    s8 *ptr;
    int off;

    ptr = (u8 *)arg0 + 0x99;
    b = ptr[arg1];
    tbl = gSkillItemTable;
    off = b * 5 + 4;
    val = *(u8 *)(off + tbl);
    if (sub_804E76C((BattleObj *)(arg0), 3, 1) >= 0)
    {
        val = val - 2;
    }
    if (sub_804E76C(arg0, 3, 2) >= 0)
    {
        val = val / 2;
    }
    return val;
}
// @ 0x08048984
u8 sub_8048984(u8 *arg0, u8 arg1)
{
    u8 *ptr;
    u8 index;

    ptr = arg0 + 0x99;
    index = ptr[arg1];
    return gSkillItemTable[index * 5 + 2] & 0xF;
}
// @ 0x080489A4
u8 sub_80489A4(u8 *arg0, u8 arg1)
{
    if (arg1 <= 7)
    {
        u8 *ptr = arg0 + 0x99;

        arg1 = ptr[arg1];
    }
    return gSkillItemTable[arg1 * 5 + 1] & 0xF;
}
// @ 0x080489C8
u16 sub_80489C8(u8 *arg0, u16 arg1)
{
    s32 diff;

    diff = *(u16 *)(arg0 + 0x72) - *(u16 *)(arg0 + 0x70);
    if (diff < arg1)
    {
        return diff;
    }
    return arg1;
}
// @ 0x080489E8
u8 sub_80489E8(BattleObj *base, u8 *output, u8 mode, u16 flags)
{
    u8 count;
    u8 i;
    u8 end;

    count = 0;
    if (mode == 0)
    {
        for (i = 0; i <= 4; i++)
            output[i] = 0;
        i = 0;
        end = 5;
    }
    else
    {
        for (i = 0; i <= 6; i++)
            output[i] = 0;
        i = 5;
        end = 12;
    }
    for (; i < end; i++)
    {
        if (sub_8045F10(&base[i], flags) == 2)
        {
            output[count] = i;
            count++;
        }
    }
    return count;
}
// @ 0x08048A68
u8 sub_8048A68(BattleObj *arg0)
{
    s16 a;
    s16 b;
    s16 diff;

    a = arg0->hp;
    b = arg0->dmgAmount;
    diff = a - b;
    if (diff <= 0)
    {
        return 1;
    }
    return 0;
}
// @ 0x08048A88
void sub_8048A88(u8 *arg0, s8 arg1, s8 arg2)
{
    u8 val;

    if (arg1 >= arg2)
    {
        return;
    }
    val = sub_8046E18(arg0, arg1, arg2);
    if (val >= arg2)
    {
        return;
    }
    sub_8048A88(arg0, arg1, val - 1);
    sub_8048A88(arg0, val + 1, arg2);
}
// @ 0x08048ACC
void sub_8048ACC(u8 *arg0, u8 arg1, u8 arg2)
{
    s8 val;
    u8 val2;

    if (arg1 <= 1)
    {
        return;
    }
    gStatRecalcKind = arg2;
    gStatRecalcPool = GetObjPool();
    val = arg1 - 1;
    if (val <= 0)
    {
        return;
    }
    val2 = sub_8046E18(arg0, 0, val);
    if (val2 >= val)
    {
        return;
    }
    sub_8048A88(arg0, 0, val2 - 1);
    sub_8048A88(arg0, val2 + 1, val);
}
// @ 0x08048B30
void sub_8048B30(u8 param1, u8 param2, u16 param3)
{
    gFxReqTimer = 0;
    gFxReqKind = param1;
    gFxReqFrames = param2;
    gFxReqAnimIdx = param3;
}
// @ 0x08048B5C
void sub_8048B5C(u8 *arg0, u8 arg1)
{
    if (arg0[0x91] == 0xB3 || arg0[0x92] == 0xB3)
    {
        *(u16 *)(arg0 + 0x88) = 0x20;
    }
    else
    {
        *(u16 *)(arg0 + 0x88) = arg1;
    }
}
extern u8 gUnk_0839CC4C[];

// @ 0x08048B88
u8 sub_8048B88(BattleObj *arg0)
{
    if (arg0->slot <= 10)
    {
        return gUnk_0839CC4C[*((u8 *)arg0 + 0x8D) * 4];
    }
    return 0;
}
typedef struct
{
    u16 unk_0;
    u8 value;
    u8 unk_3;
} Unk_0839CC4C;

extern Unk_0839CC4C gUnk_0839CC4C_entries[];

// @ 0x08048BAC
u8 sub_8048BAC(BattleObj *arg0)
{
    if (arg0->slot <= 10)
    {
        return gUnk_0839CC4C_entries[*((u8 *)arg0 + 0x8D)].value;
    }
    return 0;
}
// @ 0x08048BD0
void sub_8048BD0(BattleObj *arg0)
{
    switch (arg0->slot)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            arg0->f_C3 = 0x10;
            break;
        case 5:
            if (*((u8 *)arg0 + 0x8D) == 0)
            {
                arg0->f_C3 = 8;
            }
            else
            {
                arg0->f_C3 = 0x10;
            }
            break;
        default:
            return;
    }
}
extern u8 gUnk_0839BB4C[];
extern u8 gUnk_0839D5BC[];

// @ 0x08048C30
u8 sub_8048C30(BattleObj *obj)
{
    u8 ret = 0;
    if (obj->slot <= 0xA)
    {
        switch (obj->slot)
        {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            ret = 1;
            break;
        case 8:
        case 9:
        case 10:
            ret = 1;
            break;
        }
    }
    else
    {
        if ((u8)(obj->slot - 0xC) <= 0x64)
        {
            const u8 *tbl = gUnk_0839D5BC;
            int k2 = obj->slot - 0xC;
            int idx = k2 * 6;
            const u8 *p = tbl + 4;
            u8 flagA = *(p + idx);
            const u8 *q = tbl + 5;
            u8 flagB = *(q + idx);
            if (flagA == 1 || flagB == 1)
                ret = 1;
        }
    }
    return ret;
}

// @ 0x08048C80
u8 sub_8048C80(BattleObj *obj)
{
    // 8 字节数组在本函数中未被使用, 但 GCC2 仍为其保留栈帧 (对应目标 sub sp,#8 / add sp,#8);
    // 同文件 sub_804C9B4/sub_804D1B4 等姊妹函数的 values[8] 是真使用, 本函数是模板复刻残留
    u8 values[8];
    u8 ret = 0;
    u32 v1;
    u32 base;
    u32 sw;
    u32 threshold;

    GetObjPool(obj);
    if (obj->slot <= 0xA)
    {
        base = 5;
        v1 = *((u8 *)obj + 0x8B);
    }
    else
    {
        base = 0x14;
        v1 = 0;
    }
    switch (obj->variantClass)
    {
    case 2:
        sw = 5;
        break;
    case 5:
        sw = 0xA;
        break;
    case 3:
    default:
        sw = 0x14;
        break;
    }
    threshold = base + (sw + v1);
    if ((u16)(((u32 (*)(void))Rng_LcgNext)() % 100) < threshold)
        ret = 1;
    return ret;
}
// @ 0x08048CEC
u8 sub_8048CEC(BattleObj *obj)
{
    u8 result;

    result = 0;
    if (obj->slot <= 10)
    {
        if ((s8)obj->fxKind != 2)
        {
            if (*(u16 *)((u8 *)obj + 0x7E) != 0)
                result = 1;
            else if (*(u16 *)((u8 *)obj + 0x80) != 0)
                result = 2;
        }
        else
        {
            if (*(u16 *)((u8 *)obj + 0x7E) != 0)
                result = 3;
            else if (*(u16 *)((u8 *)obj + 0x80) != 0)
                result = 4;
        }
    }
    return result;
}
// @ 0x08048D40
void sub_8048D40(BattleObj *arg0)
{
    if (arg0->slot > 10)
    {
        return;
    }
    *(u16 *)((u8 *)arg0 + 0x7E) = 0;
    *(u16 *)((u8 *)arg0 + 0x80) = 0;
    *(u16 *)((u8 *)arg0 + 0x82) = 0;
    *(u16 *)((u8 *)arg0 + 0x84) = 0;
    *(u16 *)((u8 *)arg0 + 0x86) = 0;
}
// @ 0x08048D64
u16 sub_8048D64(BattleObj *arg0, u16 arg1)
{
    s32 diff;

    diff = arg0->maxHp - arg0->hp;
    if (diff < arg1)
    {
        return diff;
    }

    return arg1;
}
// @ 0x08048D84
u8 sub_8048D84(u8 *arg0, u8 *arg1)
{
    s32 val1 = arg0[0xAC] & 0x0F;
    s32 val2 = arg1[0xAC] & 0x0F;

    if (val1 + val2 <= 5)
    {
        return 1;
    }
    return 0;
}
// @ 0x08048DA4
// 战斗音乐/对话框子系统复位: 清零 0x03000948-0x0300097D 全局区, DialogCtx_Clear3 + Bg0_InitClear,
// sub_80196D4 重置 0x02035AC0 脚本上下文 (5 个栈参数), DMA3 拷 0x40 字节 (0x0861A7A4→0x0600B7C0) 后等待。
void sub_8048DA4(void)
{
    u8 i;
    extern u32 gBattleDlgPool; /* 块级 u32 视图: 文件后方另有 BattleObj 布局视图 Unk_03000970* (经验109 同址多视图), 本函数只写 0 */

    gBattleDlgObjSlot = 0;
    gBattleDlgCellIdx = 0;
    gBattleDlgTileCol = 0;
    gBattleDlgDigitCol = 0;
    gBattleDlgAnimFrame = 0;
    *(u16 *)gBattleDlgSegOff = 0;
    gBattleDlgNextState = 0;
    gBattleDlgShowVal = 0;
    gBattleDlgLvUpMask = 0;
    gBattleDlgShowTarget = 0;
    gBattleDlgLearnCount = 0;
    gBattleDlgLearnIdx = 0;
    gBattleDlgLearnGfx = 0;
    gBattleDlgPool = 0;
    gBattleDlgPartyCount = 0;
    gBattleDlgPartyIdx = 0;
    gBattleDlgFlashState = 0;
    gBattleDlgFlashTimer = 0;
    gBattleDlgFlashPal = 0;
    for (i = 0; i <= 4; i++)
        gBattleDlgPartySlots[i] = 0;
    for (i = 0; i <= 7; i++)
        gBattleDlgLearnList[i] = 0;
    DialogCtx_Clear3();
    Bg0_InitClear();
    sub_80196D4(0, 0x02035AC0, 0xB, 2, 2, 1, 0, 0x1C, 4);
    gBattleDlgObjSlot = 0;
    gBattleIntroState = 0;
    gBattleIntroPhase = 1;
    gBattleDlgShowVal = 0;
    gBattleDlgLvUpMask = 0;
    DmaCopy16(3, 0x0861A7A4, 0x0600B7C0, 0x40);
    DmaWait(3);
    gBattleDlgLearnIdx = 0;
    gBattleDlgLearnCount = 0;
    gBattleDlgFlashState = 0;
    gBattleDlgFlashTimer = 0;
}
// @ 0x08048F0C
void sub_8048F0C(void)
{
    u8 *state;

    state = &gBattleDlgFlashState;
    switch (*state)
    {
        case 0:
            break;
        case 1:
            sub_804B96C(gBattleDlgFlashPal, 1, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gBattleDlgFlashTimer = 0;
            Sfx_Play(0x18, 0, 0);
            *state = 2;
            break;
        case 2:
            if (gBattleDlgFlashTimer <= 3)
            {
                gBattleDlgFlashTimer++;
            }
            else
            {
                sub_804C4D8(gBattleDlgFlashPal, 1, 0x10);
                gBattleDlgFlashTimer = 0;
                *state = 3;
            }
            break;
        case 3:
            if (gBattleDlgFlashTimer <= 0xF)
            {
                gBattleDlgFlashTimer++;
            }
            else
            {
                gBattleDlgFlashTimer = 0;
                *state = 0;
            }
            break;
    }
}
// @ 0x08048FB8
INCLUDE_ASM("asm/nonmatchings", sub_8048FB8);
// @ 0x080492C0
INCLUDE_ASM("asm/nonmatchings", sub_80492C0);
// @ 0x080494F0
INCLUDE_ASM("asm/nonmatchings", sub_80494F0);
/* 数字翻牌显示: 把 arg1 (u16) 按 10000/1000/100/10 拆成 5 位存栈上数组 digits[0..4]。
 * 引导循环跳过前导零 (首个非零位的下标); val==0 时不跳 (i 归 0 → 显示个位)。
 * i 加上全局计数器 gBattleDlgDigitCol (u8 回绕) 选位, 写 arg0[0]/arg0[0x20] 两行 tilemap
 * (值 = digit*2 - 0x4EBC/-0x4EBB, 即 +0xFFFFB144/+0xFFFFB145), 计数器 ++。
 * 返回: val==0 → 1; 已显示到最后一位之后 (i>3) → 1; 否则 0 (还有后续位)。
 * ⚠ 除法通道: 第 1 次 (val/10000) 必须 unsigned (__udivsi3, val 强转 u32),
 *   其余 3 次必须 signed (__divsi3, (int) 强转)。中间和必须写 val - (a + b + c) 括号全式
 *   (非 val - a - b 链式), 否则 GCC 拆成多次 subs 且 CSE 复用 (经验 209 姊妹形)。 */
// @ 0x080497B0
u32 sub_80497B0(u16 *arg0, u16 arg1)
{
    u16 digits[5];
    s32 val;
    u8 i;
    u8 *counter;

    val = arg1;
    digits[0] = (u32)val / 10000;
    digits[1] = (int)(val - digits[0] * 10000) / 1000;
    digits[2] = (int)(val - (digits[0] * 10000 + digits[1] * 1000)) / 100;
    digits[3] = (int)(val - (digits[0] * 10000 + digits[1] * 1000 + digits[2] * 100)) / 10;
    digits[4] = val - (digits[0] * 10000 + digits[1] * 1000 + digits[2] * 100 + digits[3] * 10);

    i = 0;
    if (digits[0] == 0)
    {
        do
        {
            i++;
            if (i > 4)
                break;
        } while (digits[i] == 0);
    }

    if (i > 4 && val == 0)
        i = 0;

    i = i + gBattleDlgDigitCol;
    i = (u8)i;
    counter = &gBattleDlgDigitCol;
    arg0[0] = digits[i] * 2 - 0x4EBC;
    arg0[0x20] = digits[i] * 2 - 0x4EBB;
    (*counter)++;

    if (val == 0)
        return 1;
    if (i <= 3)
        return 0;
    return 1;
}

INCLUDE_ASM("asm/matchings", sub_80498E0);

// @ 0x080498E0
// extern const u8 gUnk_08095028[];
// u32 sub_80498E0(u16 *dest)
// {
//     const u8 *tbl = gUnk_08095028;
//     u8 byte;
//     u8 frame;
//     u16 ofs;
//     u8 tile;

//     byte = ((u8 *)gResultsDropTablePtr)[gResultsDropSelIdx * 4];
//     ofs = byte * 8;
//     frame = gBattleDlgAnimFrame;
//     tile = tbl[ofs + frame];
//     dest[0] = tile * 2 - 0x5000;
//     dest[0x20] = tile * 2 - 0x4FFF;
//     gBattleDlgAnimFrame++;
//     byte = ((u8 *)gResultsDropTablePtr)[gResultsDropSelIdx * 4];
//     ofs = byte * 8;
//     frame = gBattleDlgAnimFrame;
//     tile = tbl[ofs + frame];
//     if (tile == 0)
//         return 1;
//     if (frame > 7)
//         return 1;
//     return 0;
// }
// @ 0x08049958
extern const u8 gUnk_0839CFBA[];
extern u8 gUnk_0839D348[];

u32 sub_8049958(u16 *dest)
{
    u8 b;
    u16 tile;
    u16 *ctx;
    u16 count;
    u16 i;

    if (sub_80187B4() & 0x20)
    {
        const u16 *tbl = (const u16 *)gUnk_0839D348;
        u8 frame = gBattleDlgAnimFrame;
        b = ((u8 *)gResultsStatePtr)[gResultsViewKind * 4];
        tile = tbl[b * 9 + frame];
    }
    else
    {
        const u8 *tbl = gUnk_0839CFBA;
        u8 frame = gBattleDlgAnimFrame;
        b = ((u8 *)gResultsStatePtr)[gResultsViewKind * 4];
        tile = tbl[b * 9 + frame];
    }

    if ((sub_80187B4() & 0x20) && tile > 0xDF)
    {
        count = TileDma_GetCtx((u32 *)&ctx);
        for (i = 0; i < count; i++)
        {
            if (tile == ctx[i])
                break;
        }
        tile = i + 0xE0;
    }

    if (tile != 0)
    {
        dest[0] = tile * 2 - 0x5000;
        dest[0x20] = tile * 2 - 0x4FFF;
    }
    else
    {
        dest[0] = 0xB001;
        dest[0x20] = 0xB001;
    }

    gBattleDlgAnimFrame++;

    if (sub_80187B4() & 0x20)
    {
        const u16 *tbl = (const u16 *)gUnk_0839D348;
        b = ((u8 *)gResultsStatePtr)[gResultsViewKind * 4];
        if (tbl[b * 9 + gBattleDlgAnimFrame] <= 0xEFF)
            return 0;
        return 1;
    }
    else
    {
        const u8 *tbl = gUnk_0839CFBA;
        b = ((u8 *)gResultsStatePtr)[gResultsViewKind * 4];
        if (tbl[b * 9 + gBattleDlgAnimFrame] <= 0xFE)
            return 0;
        return 1;
    }
}
// @ 0x08049AD8
u8 sub_8049AD8(u8 arg0)
{
    u8 s;
    u16 n;
    u8 i;
    u8 idx;
    u16 a;
    u16 b;
    u8 obj;
    int t;

    s = arg0 + 1;
    n = 13 - s;
    for (i = 0; i < n; i++)
    {
        obj = gBattleDlgObjSlot;
        t = i + 251;
        idx = t + s;
        a = sub_80455A0(obj, idx);
        b = sub_8048818(gBattleDlgObjSlot, idx);
        gBattleDlgShowTarget = b - a;
        if ((s16)gBattleDlgShowTarget > 0)
        {
            sub_8045688(gBattleDlgObjSlot, idx, gBattleDlgShowTarget);
            break;
        }
    }
    return s + i;
}
// @ 0x08049B70
INCLUDE_ASM("asm/nonmatchings", sub_8049B70);
// @ 0x08049C1C
// BGM 演出状态机: 按 gBattleIntroState 分派 (0=开场曲判定+FadeIn, 1/3=滑动计数(sub_801768C 插值写
// gBattleIntroObj.f_2B, 计满进下一态), 2=等待计数, 4=完成返回1); case1/3 的自增走 <= 在 then 臂。
// 尾部: gBattleIntroObj.f_2D = 入参槽号, arg0[0] = sub_801A884(gBattleIntroObj, 槽号, &local)。
u8 sub_8049C1C(u8 *arg0)
{
    u8 result;
    u8 b;
    u8 local2;

    result = 0;
    b = arg0[0];
    switch (gBattleIntroState)
    {
    case 0:
        if ((((ObjHead *)gBattleIntroObj)->kindFlags & 0x800) == 0)
        {
            gBattleIntroState = 1;
            if (sub_80187B4() & 0x20)
            {
                Bgm_Play(0x37, 0);
            }
            else
            {
                Bgm_Play(0x36, 0);
            }
            Bgm_FadeIn(0xa);
        }
        break;
    case 1:
        ((ObjHead *)gBattleIntroObj)->f_2B = sub_801768C(0xF0, -0x78, 0x14, gBattleIntroTimer, 2);
        if (gBattleIntroTimer <= 0x13)
        {
            gBattleIntroTimer++;
        }
        else
        {
            gBattleIntroTimer = 0;
            gBattleIntroState = 2;
        }
        break;
    case 2:
        if (gBattleIntroTimer <= 0x31)
        {
            gBattleIntroTimer++;
        }
        else
        {
            gBattleIntroTimer = 0;
            gBattleIntroState = 3;
        }
        break;
    case 3:
        ((ObjHead *)gBattleIntroObj)->f_2B = sub_801768C(0x78, -0x78, 8, gBattleIntroTimer, 1);
        if (gBattleIntroTimer <= 7)
        {
            gBattleIntroTimer++;
        }
        else
        {
            gBattleIntroTimer = 0;
            gBattleIntroState = 4;
        }
        break;
    case 4:
        result = 1;
        break;
    }
    ((ObjHead *)gBattleIntroObj)->f_2D = b;
    arg0[0] = sub_801A884((ObjHead *)gBattleIntroObj, b, &local2);
    return result;
}
// @ 0x08049D58
u8 sub_8049D58(u8 arg0)
{
    switch (gBattleIntroPhase)
    {
    case 0:
    {
        ObjHead *obj;

        if (gBattleIntroState != 0)
        {
            obj = (ObjHead *)gBattleIntroObj;
            if (!(obj->kindFlags & 0x800))
                return sub_801B8AC(obj, obj->f_2D);
        }
        return arg0;
    }
    case 1:
        break;
    case 2:
        if (gBattleIntroState > 1)
        {
            DmaCopy16(3, (void *)0x02035AC0, (void *)0x06007000, 0x800);
            DmaWait(3);
        }
        break;
    }
}
// @ 0x08049DF8
INCLUDE_ASM("asm/nonmatchings", sub_8049DF8);
// @ 0x0804A148
void sub_804A148(void)
{
    BattleObj *pool;
    u8 buf0[8];
    u8 buf1[8];
    u8 count0;
    u8 count1;
    u8 i;

    pool = (BattleObj *)GetObjPool();
    gTurnAilCount = 0;
    gTurnAilMask = 0;

    count0 = sub_80489E8(pool, buf0, 0, 0xFF);
    count1 = sub_80489E8(pool, buf1, 1, 0xFF);

    for (i = 0; i < count0; i++)
    {
        if (pool[buf0[i]].statusAil != 0)
        {
            gTurnAilMask |= pool[buf0[i]].statusAil;
            gTurnAilSlots[gTurnAilCount] = buf0[i];
            gTurnAilCount++;
        }
    }

    for (i = 0; i < count1; i++)
    {
        if (pool[buf1[i]].statusAil != 0)
        {
            // 注: 原版 ROM 存在复制粘贴残留 bug, 此处 OR 运算误读了玩家侧 buf0[i]
            gTurnAilMask |= pool[buf0[i]].statusAil;
            gTurnAilSlots[gTurnAilCount] = buf1[i];
            gTurnAilCount++;
        }
    }

    for (i = 0; i < count0; i++)
    {
        if (pool[buf0[i]].state & 0x10)
        {
            if (pool[buf0[i]].pad_BA[0] <= 1)
            {
                pool[buf0[i]].pad_BA[0]++;
            }
            else
            {
                pool[buf0[i]].state &= ~0x10;
                pool[buf0[i]].pad_BA[0] = 0;
            }
        }
    }

    count0 = sub_80489E8(pool, buf0, 0, 0x2C);
    count1 = sub_80489E8(pool, buf1, 1, 0x2C);

    if (count0 == 1)
    {
        gTurnActSlots[gTurnActCount] = buf0[0];
        gTurnActCount++;
    }
    else
    {
        gTurnActCount = gTurnActIdx = 0;
        for (i = 0; i < count0; i++)
        {
            if (sub_8048C80(&pool[buf0[i]]) == 1)
            {
                gTurnActSlots[gTurnActCount] = buf0[i];
                gTurnActCount++;
            }
        }
    }

    for (i = 0; i < count1; i++)
    {
        if (((u32 (*)(void))Rng_LcgNext)() % 100 <= 39)
        {
            gTurnActSlots[gTurnActCount] = buf1[i];
            gTurnActCount++;
        }
    }

    gTurnStep = 1;
}
// @ 0x0804A368
INCLUDE_ASM("asm/nonmatchings", sub_804A368);
// @ 0x0804AA2C
void sub_804AA2C(u16 mask)
{
    BattleObj *pool;
    u8 buf0[8];
    u8 buf1[8];
    u8 count0;
    u8 count1;
    u8 i;

    pool = (BattleObj *)GetObjPool();
    gTurnAilCount = 0;
    gTurnAilIdx = 0;

    count0 = sub_80489E8(pool, buf0, 0, 0xFF);
    count1 = sub_80489E8(pool, buf1, 1, 0xFF);

    for (i = 0; i < count0; i++)
    {
        if (pool[buf0[i]].statusAil & mask)
        {
            gTurnAilSlots[gTurnAilCount] = buf0[i];
            gTurnAilCount++;
        }
    }

    for (i = 0; i < count1; i++)
    {
        if (pool[buf1[i]].statusAil & mask)
        {
            // 注: 原版 ROM 复制粘贴 bug, 此处误写入玩家侧 buf0[i]
            gTurnAilSlots[gTurnAilCount] = buf0[i];
            gTurnAilCount++;
        }
    }
}
typedef struct
{
    u8 gap[0x35];
    u8 value;
    u8 remaining[146];
} Unk_03000970; /* 200 字节 = BattleObj 同布局视图; gBattleDlgPool 为指向该数组的指针;
                 * +0x35 (value) 即 BattleObj.headA.palSlot (headA@+0x0C + ObjHead.palSlot@+0x29),
                 * sub_8048F0C 以 sub_804B96C/804C4D8 对该调色板槽做闪光淡变 */

extern u8 gBattleDlgObjSlot;
extern u8 gBattleDlgFlashState;
extern u8 gBattleDlgFlashPal;
extern Unk_03000970 *gBattleDlgPool;

// @ 0x0804AB10
void sub_804AB10(void)
{
    gBattleDlgFlashPal = gBattleDlgPool[gBattleDlgObjSlot].value;
    gBattleDlgFlashState = 1;
}
// @ 0x0804AB40
INCLUDE_ASM("asm/nonmatchings", sub_804AB40);
// @ 0x0804ABD0
void sub_804ABD0(void)
{
    u8 i;
    u16 *ptr;

    ptr = gUnk_02035B04;
    for (i = 0; i <= 0x19; i++)
    {
        ptr[i] = 0xB001;
        ptr[i + 0x20] = 0xB001;
    }
}
/* tile 动画帧写入: 按 arg1*18 + gBattleDlgAnimFrame*2 索引 gUnk_0862D574 的 u16 帧表,
 * 把当前帧写入 dest[0]/dest[0x20] 两处 tilemap (值 = data*2 - 0x5000 / -0x4FFF),
 * 帧号 gBattleDlgAnimFrame++ 后检查: >3 或下一帧 == 0xF00 终止符 → 返回 1 (动画结束), 否则 0。 */
extern u8 gUnk_0862D574[];

// @ 0x0804ABF8
u32 sub_804ABF8(u16 *dest, u8 arg1)
{
    u8 *base;
    u16 data;
    u16 off;
    u16 off2;

    base = gUnk_0862D574;
    off = gBattleDlgAnimFrame * 2 + arg1 * 18;
    data = *(u16 *)(base + off);
    dest[0] = data * 2 - 0x5000;
    dest[0x20] = data * 2 - 0x4FFF;

    gBattleDlgAnimFrame++;

    if (gBattleDlgAnimFrame > 3)
        return 1;

    off2 = gBattleDlgAnimFrame * 2 + arg1 * 18;
    if (*(u16 *)(base + off2) == 0xF00)
        return 1;

    return 0;
}

extern u8 gUnk_0839D348[];

// @ 0x0804AC60
void sub_804AC60(void)
{
    u32 local;
    u8 b;
    u8 *entry;

    b = *(u8 *)(gResultsStatePtr + gResultsViewKind);
    entry = gUnk_0839D348 + b * 18;
    if (sub_80187B4() & 0x20)
    {
        sub_8050434(entry, 0x4F1E);
        if ((u16)TileDma_GetCtx(&local) != 0)
        {
            sub_80187C0(0x400);
        }
    }
}
extern u16 gUnk_0839B462[];

// @ 0x0804ACC0
u16 *sub_804ACC0(u8 arg0)
{
    u32 count = 0;
    u32 i = 0;
    u32 local;
    u16 *base;
    u8 arg = arg0;
    u16 *table = gUnk_0839B462;

    if (i < arg)
    {
        base = table;
        do
        {
            if (base[i] == 0xF00)
                count = (u16)(count + 1);
            i = (u16)(i + 1);
        } while (count < arg);
    }
    sub_8050434((u16 *)((u32)i * 2 + (u32)table), 0x6F1E);
    if ((u16)TileDma_GetCtx(&local) != 0)
        sub_80187C0(0x400);
    return (u16 *)((u32)i * 2 + (u32)table);
}
// @ 0x0804AD24
void sub_804AD24(u8 *arg0)
{
    u8 value;

    gBattleIntroTimer = (gBattleIntroTimer + 1) % 16;

    value = gBattleIntroTimer / 8;

    *(u16 *)(arg0 + 0xB6) = 0xB1BE + value;
}
