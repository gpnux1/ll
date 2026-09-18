#include "battle_types.h"
#include "battle_stage_effects.h"
#include "battle_stage_state.h"
#include "battle_stage_dialogue.h" /* sub_803F5B4 / sub_803F658 原型 (本 TU 大量调用) */
#include "battle_object_engine.h"
#include "battle_palette_wipe.h"
#include "battle_task_services.h"
#include "battle_flow_rules.h"
#include "battle_itemuse_rewards.h" /* sub_804E6DC (sub_8041308 case9 命中判定) */
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"
#include "sound.h"

// @ 0x0803FF54
u8 sub_803FF54(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            sub_801CBA4(obj, 8, 0x1B4, 0xD, 0);
            gObjActStep = 0x30;
            gObjActStepTimer = 0;
            break;
        case 0x30:
            if (sub_8048458(obj) == 1)
            {
                gUnk_03000864 = 0;
            }
            else
            {
                gUnk_03000864 = 1;
            }
            gObjActStep = 0x33;
            break;
        case 0x33:
            if (gObjActStepTimer <= 0x31)
            {
                gObjActStepTimer += 1;
            }
            else
            {
                if (gUnk_03000864 == 0)
                {
                    gObjActStep = 0x34;
                }
                else
                {
                    gObjActStep = 0x36;
                }
                gObjActStepTimer = 0;
            }
            break;
        case 0x34:
            if (gObjActStepTimer <= 0x18)
            {
                obj->posX = sub_801768C(gObjActSavedX, 0x100 - gObjActSavedX, 0x19, gObjActStepTimer, 1);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStep = 0x35;
            }
            break;
        case 0x35:
            obj->slot = 0xFF;
            obj->variantClass = 7;
            gObjActStep = 0x38;
            gObjActDoneCount += 1;
            result = 1;
            break;
        case 0x36:
            sub_801CBA4(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x38;
            break;
        case 0x38:
            result = 1;
            break;
    }
    return result;
}
// @ 0x080401AC
u32 sub_80401AC(void)
{
    u32 result;
    BattleObj *pool;
    u8 i;

    result = 0;
    pool = (BattleObj *)GetObjPool();
    switch (gObjActStep)
    {
        case 0:
            gTargetSlotCount = sub_80489E8(pool, gTargetSlotList, 0, 0x43);
            for (i = 0; i < gTargetSlotCount; i++)
            {
                gUnk_03000870[i] = pool[gTargetSlotList[i]].posX;
            }
            gObjActMoveFromX = 0;
            gObjActStep = 0x31;
            gObjActStepTimer = 0;
            break;

        case 0x31:
            if (gObjActMoveFromX < gTargetSlotCount)
            {
                BattleObj *obj = (BattleObj *)(gTargetSlotList[gObjActMoveFromX] * 0xC8 + (u32)pool);
                u8 slot = obj->slot;
                AnimEntry24 *entry = &gUnk_083988A8[slot];
                sub_8020974(&obj->headA, entry->animId, (u16)(gObjActMoveFromX * 32 + 0xDA) * 2, obj->headA.palSlot, 0x401);
                gObjActStep = 0x32;
            }
            else
            {
                gObjActStep = 0x30;
            }
            break;

        case 0x32:
            if ((pool[gTargetSlotList[gObjActMoveFromX]].headA.kindFlags & 0x800) == 0)
            {
                gObjActMoveFromX++;
                gObjActStep = 0x31;
            }
            break;

        case 0x30:
            if (sub_8048310() == 1)
            {
                gUnk_03000864 = 0;
            }
            else
            {
                gUnk_03000864 = 1;
            }
            gObjActMoveFromX = 0;
            gObjActStep = 0x33;
            break;

        case 0x33:
            if (gObjActStepTimer <= 0x31)
            {
                gObjActStepTimer++;
            }
            else
            {
                if (gUnk_03000864 == 0)
                {
                    gObjActStep = 0x34;
                }
                else
                {
                    gObjActStep = 0x36;
                }
                gObjActStepTimer = 0;
            }
            break;

        case 0x34:
            if (gObjActStepTimer <= 0x18)
            {
                for (i = 0; i < gTargetSlotCount; i++)
                {
                    pool[gTargetSlotList[i]].posX = sub_801768C(gUnk_03000870[i], 0x100 - gUnk_03000870[i], 0x19, gObjActStepTimer, 1);
                }
                gObjActStepTimer++;
            }
            else
            {
                for (i = 0; i < gTargetSlotCount; i++)
                {
                    pool[gTargetSlotList[i]].slot |= 0xFF;
                }
                gObjActStep = 0x35;
            }
            break;

        case 0x35:
            gObjActStep = 0x38;
            result = 1;
            break;

        case 0x36:
            if (gObjActMoveFromX < gTargetSlotCount)
            {
                u8 slot = gTargetSlotList[gObjActMoveFromX];
                sub_801CBA4(&pool[slot], 0, slot * 0x16 + 9, pool[slot].headA.palSlot, 0);
                gObjActStep = 0x37;
            }
            else
            {
                sub_80187C0(0x1000);
                gObjActStep = 0x38;
            }
            break;

        case 0x37:
            if ((pool[gTargetSlotList[gObjActMoveFromX]].headA.kindFlags & 0x800) == 0)
            {
                gObjActMoveFromX++;
                gObjActStep = 0x36;
            }
            break;

        case 0x38:
            result = 1;
            break;
    }

    return result;
}
// @ 0x080405A4
u8 sub_80405A4(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActStepTimer = 0;
            if (Rng_LcgNext() % 0x64 <= 0x45 && (sub_80187B4() & 0x220) == 0)
            {
                u16 f2a = obj->headA.kindFlags | 0x20;
                obj->headA.kindFlags = f2a;
                gObjActStep = 2;
            }
            else
            {
                gObjActStep = 9;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                obj->posX = sub_801768C(gObjActSavedX, -gObjActSavedX, 0x14, gObjActStepTimer, gObjActStep);
                gObjActStepTimer += 1;
            }
            else
            {
                sub_80207A4();
                obj->slot = 0xFF;
                obj->variantClass = 7;
                gObjActStep = 9;
            }
            break;
        case 9:
            result = 2;
            break;
    }
    return result;
}
// @ 0x08040690
INCLUDE_ASM("asm/nonmatchings", sub_8040690);

extern u8 gUnk_0839CC4C[];

static inline void ObjAction_InitFrameWait(s16 frames)
{
    gActWaitBusy0 = 1;
    gActWaitBusy1 = 0;
    gActWaitBusy2 = 0;
    if (frames < 0)
        gActWaitFrames = 12;
    else
        gActWaitFrames = frames;
    gActWaitCnt0 = 0;
    gActWaitCnt1 = 0;
    gActWaitSfxId = 0x37;
    gActWaitSfxParam = 0;
}

// @ 0x08040EE8
// ⏸ 语义已 100% 还原 (见下方 0x08040EE8 交接说明), 但字节仍不匹配, 暂不落地:
//    残差 = case6 首个条件里 entry/&gObjActResult 的寄存器命名与两条字面池载入顺序,
//    共 16 处 2 字节差异 (指令流一致)。
//    候选: permuter/sub_8040EE8/base.c (含 ObjActJudgeEntry + limits/prob 视角),
//    详见 docs/handoffs/BLOCKED-8040EE8-20260918.md
INCLUDE_ASM("asm/nonmatchings", sub_8040EE8);

// @ 0x08041308
// 战斗对象"绕后冲刺 + 高空落体插入"演出状态机 (sub_803F444 分派: slot<=0xA 且 fxKind==0、
// slot==4 且 sub_8048B88()==2 时进入; 双参 obj + arg1 协作对象 = 被攻击目标)。
//  0   存 headA.palSlot/f_1E 与自身 posX/posY; 清参演对象/arg1 的 dmgAmount; sub_803F5B4;
//      sub_801CBA4(obj,1,0x1B4,0xC,0) 装动画; f_B6=0x45, f_B4=0 → 0x12。
// 0x12 等 headA.kindFlags bit0x800 落下 → 0x13。
// 0x13 等 headA.frameIdx > 0x8B → Sfx 0x16 → 0x14。
// 0x14 等 headA.kindFlags bit0x1000: 释放 headA 调色板槽, sub_801CBA4 复位姿势,
//      headA.kindFlags |= 0x100 → 0x15。
// 0x15 等 headA.kindFlags bit0x800 落下: 清 bit0x100, 在 arg1->posX 正上方
//      (y=0, 效果 0x1B4/动画号 Rng%5+0x40) 播锚点动画 sub_8020CC4, 置 headB.f_2A=0,
//      计时清零, 记锚点 gObjActMoveFromY = arg1->posY → 0x16。
// 0x16 计时 <= 0x13 时把 headB.f_2C 由 0 插值到 gObjActMoveFromY (20 帧, 模式 1);
//      计时 > 0x13 时清零 + 开 40 帧等待窗 + 锁定 headB.f_2C = gObjActMoveFromY,
//      gObjActMoveFromX = 0 → 0x17。
// 0x17 按 gObjActMoveFromX 分段下滑: 计时 < 0x14-2*i 时以 (s16)(2*i+0xFFEC) 为增量、
//      (0xA-i) 帧插值; 否则计时清零, i<=4 时 i+=2 继续下一段, i>4 时清 state bit0x2000
//      → 0x18。
// 0x18 等待窗三互斥全空 → 9。
// 9    按 arg1->slot(<=0x70 才结算) 与 sub_804E6DC(obj,3)/(obj,4) 的成功结果,
//      以 Rng%100 与 9 / 0x18 比较决定是否打出 999 伤害: 命中返回 3, 否则返回 1。
// 尾部: sub_803F658(obj)。1..8/0xA..0x11 为空 case (跳转表默认项, 表宽 cmp #0x18)。
u8 sub_8041308(BattleObj *obj, BattleObj *arg1)
{
    u8 result;
    u8 count;
    u8 i;
    BattleObj *pool;
    u8 buf[12];

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            pool = (BattleObj *)GetObjPool();
            count = sub_80462E4(obj, buf, 0x6F);
            for (i = 0; i < count; i++)
            {
                pool[buf[i]].dmgAmount = 0;
            }
            sub_803F5B4(obj);
            sub_801CBA4(obj, 1, 0x1B4, 0xC, 0);
            obj->f_B6 = 0x45;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 0x12:
            if (!(obj->headA.kindFlags & 0x800))
            {
                gObjActStep = 0x13;
            }
            break;
        case 0x13:
            if (obj->headA.frameIdx > 0x8B)
            {
                Sfx_Play(0x16, 0, 0);
                gObjActStep = 0x14;
            }
            break;
        case 0x14:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)&obj->headA));
                sub_801CBA4(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                obj->headA.kindFlags |= 0x100;
                gObjActStep = 0x15;
            }
            break;
        case 0x15:
            if (!(obj->headA.kindFlags & 0x800))
            {
                obj->headA.kindFlags &= 0xFEFF;
                sub_8020CC4(obj, arg1->posX, 0, 0x1B4, 0xD, (u16)(Rng_LcgNext() % 5 + 0x40), 5);
                obj->headB.f_2A = 0;
                gObjActStepTimer = 0;
                gObjActMoveFromY = arg1->posY;
                gObjActStep = 0x16;
            }
            break;
        case 0x16:
            if (gObjActStepTimer <= 0x13)
            {
                obj->headB.f_2C = sub_801768C(0, gObjActMoveFromY, 0x14, gObjActStepTimer, 1);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                ObjAction_InitFrameWait(40);
                obj->headB.f_2C = gObjActMoveFromY;
                gObjActMoveFromX = 0;
                gObjActStep = 0x17;
            }
            break;
        case 0x17:
            if (gObjActStepTimer < 0x14 - (gObjActMoveFromX << 1))
            {
                /* 增量 = 2*index - 0x14, 但源码按 u16 视图回绕写成 +0xFFEC:
                 * 必须保留 (u16) 视图, 否则 GCC 折叠成 subs #0x14 并省掉 s16 掩码。 */
                obj->headB.f_2C = sub_801768C(gObjActMoveFromY, (s16)((u16)(gObjActMoveFromX * 2) + 0xFFEC),
                                              0xA - gObjActMoveFromX, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                if (gObjActMoveFromX <= 4)
                {
                    gObjActMoveFromX += 2;
                }
                else
                {
                    obj->state &= 0xDFFF;
                    gObjActStep = 0x18;
                }
            }
            break;
        case 0x18:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                gObjActStep = 9;
            }
            break;
        case 9:
            if (arg1->slot <= 0x70)
            {
                if (sub_804E6DC(obj, 3) >= 0)
                {
                    if (Rng_LcgNext() % 0x64 <= 9)
                    {
                        arg1->dmgAmount = 999;
                        result = 3;
                    }
                    else
                    {
                        result = 1;
                    }
                }
                else if (sub_804E6DC(obj, 4) >= 0)
                {
                    if (Rng_LcgNext() % 0x64 <= 0x18)
                    {
                        arg1->dmgAmount = 999;
                        result = 3;
                    }
                    else
                    {
                        result = 1;
                    }
                }
                else
                {
                    result = 1;
                }
            }
            else
            {
                result = 1;
            }
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x080416F0
// 战斗对象"受击后退 + 在协作对象身上播锚点动画"演出状态机 (双参 obj + arg1 协作对象;
// 自身只演 headA, 锚点特效挂在 arg1 的坐标上)。
// case0  存 headA.palSlot/f_1E 与自身 posX/posY; 清 headA.kindFlags bit0x400; 清 arg1 与
//        sub_80462E4 收集的参演对象 dmgAmount(+0xB2); sub_803F5B4;
//        sub_801CA08(obj,3,0x1B4,0xD,0) 装攻击动画 → 1 (计时清零)。
// case1  headA.frameIdx==0xF 时播 Sfx 0x87; frameIdx>0x28 后在 arg1 位置播锚点动画
//        sub_8020CC4(obj, arg1->posX, arg1->posY, 0x2EA, 0xA, animPtr[0x1A]+4, 5),
//        置 headB.kindFlags bit0x2000、headA.kindFlags bit0x100, headA.frameIdx=0x5D,
//        state bit2, 开 30 帧等待窗 → 2。
// case2  等 headB.kindFlags bit0x800 落下 → 3。
// case3  等 headB.kindFlags bit0x1000: 释放 headB 调色板槽, 清 headB bit0xEFFF,
//        state bit0xDFFF / bit0xFFFB, headA bit0xFEFF, 播 Sfx 0x87 → 6。
// case6  等 headA.kindFlags bit0x1000: 释放 headA 调色板槽, sub_80207DC 还原姿势 → 7。
// case7  等 headB.kindFlags bit0x800 落下 → 9。
// case9  等待窗三互斥全空 → 返回 1 (演出结束)。
// 尾部: sub_803F658(obj)。4/5/8 为空 case (跳转表默认项, 决定表宽 cmp #9)。
u8 sub_80416F0(BattleObj *obj, BattleObj *arg1)
{
    u8 *anim;
    u8 result;
    u8 count;
    u8 i;
    BattleObj *pool;
    u8 buf[12];

    result = 0;
    anim = obj->animPtr;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            obj->headA.kindFlags &= 0xFBFF;
            arg1->dmgAmount = 0;
            pool = (BattleObj *)GetObjPool();
            count = sub_80462E4(obj, buf, 0x6F);
            for (i = 0; i < count; i++)
            {
                pool[buf[i]].dmgAmount = 0;
            }
            sub_803F5B4(obj);
            sub_801CA08(obj, 3, 0x1B4, 0xD, 0);
            gObjActStepTimer = 0;
            gObjActStep = 1;
            break;
        case 1:
            if (obj->headA.frameIdx == 0xF)
            {
                Sfx_Play(0x87, 1, 0);
            }
            if (obj->headA.frameIdx > 0x28)
            {
                sub_8020CC4(obj, arg1->posX, arg1->posY, 0x2EA, 0xA, *(u16 *)(anim + 0x1A) + 4, 5);
                obj->headB.kindFlags |= 0x2000;
                obj->headA.kindFlags |= 0x100;
                obj->headA.frameIdx = 0x5D;
                obj->state |= 4;
                ObjAction_InitFrameWait(30);
                gObjActStep = 2;
            }
            break;
        case 2:
            if (!(obj->headB.kindFlags & 0x800))
            {
                gObjActStep = 3;
            }
            break;
        case 3:
            if (obj->headB.kindFlags & 0x1000)
            {
                sub_804C3A4(obj->headB.palSlot, sub_801B954((ObjHead *)&obj->headB));
                obj->headB.kindFlags &= 0xEFFF;
                obj->state &= 0xDFFF;
                obj->headA.kindFlags &= 0xFEFF;
                obj->state &= 0xFFFB;
                Sfx_Play(0x87, 1, 0);
                gObjActStep = 6;
            }
            break;
        case 6:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)&obj->headA));
                sub_80207DC(obj, obj->posX, obj->posY, gObjActSavedF2A, gObjActSavedPal);
                gObjActStep = 7;
            }
            break;
        case 7:
            if (!(obj->headB.kindFlags & 0x800))
            {
                gObjActStep = 9;
            }
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                result = 1;
            }
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x080419E0
INCLUDE_ASM("asm/nonmatchings", sub_80419E0);

// @ 0x08041EDC
// 战斗对象"蓄力/多段闪光演出"状态机 (0x0839CEC4[3], sub_803F444 的 animPtr+0x1C 分支 handler,
// 双参 obj + arg1 同池协作对象; 与 sub_80422B8/8042784/8042B90 同表同签名):
// case0  存 headA.palSlot/f_1E/posX/posY, 清 headA.kindFlags bit0x400, 清 arg1 与
//        sub_80462E4 收集的参演对象 dmgAmount; sub_803F5B4;
//        slot==0x37 → sub_801CA08(obj,3,0x1B4,0xD,0), 否则 (obj,4,0x1B4,0xC,0) → 1。
// case1  slot==0x5E (长版): 帧 0x1F 调 sub_804B834(0xD,1,2,2,4) 起闪; 帧 0x3D 播 Sfx 0xA6 +
//        sub_804B8E8(0xD,1); 帧到 f_B4 时开 20 帧等待窗并把 gObjActStepTimer 对齐到当前帧 → 2。
//        slot!=0x5E (短版): 帧到 f_B4-5 时开 20 帧等待窗 + Sfx 0x69, 同样对齐计时 → 2。
// case2  slot!=0x5E 时按计时触发闪光: timer==f_B4-1 → sub_804BF14(0,0x10,10,10,10,4,0,-1,2) +
//        sub_804B96C(7,7,10,10,10,4,0,-1,2) 起两层; timer==f_B4+5 → sub_804C10C(0,0x10) +
//        sub_804BB64(7,7) 收两层。随后 headA.kindFlags bit0x1000 时释放调色板槽 + 清位 +
//        sub_80207DC(obj,posX,posY,SavedF2A,SavedPal) 复原 → 9; 每帧 gObjActStepTimer++。
// case9  等待窗三互斥全空 → 返回 1 (演出结束)。
// 尾部: sub_803F658(obj)。case 源码顺序 0,1,2,9 (GCC 生成 1 → (0 | 2 → 9) 决策树, 与源序一致)。
// 注意: 与 sub_80422B8 不同, 本函数起手**不**调 sub_80187E8。
u8 sub_8041EDC(BattleObj *obj, BattleObj *arg1)
{
    u8 result;
    u8 count;
    u8 i;
    u8 *pool;
    u8 buf[12];

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            obj->headA.kindFlags &= 0xFBFF;
            arg1->dmgAmount = 0;
            pool = (u8 *)GetObjPool();
            count = sub_80462E4(obj, buf, 0x6F);
            for (i = 0; i < count; i++)
            {
                *(u16 *)(pool + buf[i] * 0xC8 + 0xB2) = 0;
            }
            sub_803F5B4(obj);
            if (obj->slot == 0x37)
            {
                sub_801CA08(obj, 3, 0x1B4, 0xD, 0);
            }
            else
            {
                sub_801CA08(obj, 4, 0x1B4, 0xC, 0);
            }
            gObjActStep = 1;
            break;
        case 1:
            if (obj->slot == 0x5E)
            {
                if (obj->headA.frameIdx == 0x1F)
                {
                    sub_804B834(0xD, 1, 2, 2, 4);
                }
                if (obj->headA.frameIdx == 0x3D)
                {
                    Sfx_Play(0xA6, 1, 0);
                    sub_804B8E8(0xD, 1);
                }
                if (obj->headA.frameIdx >= obj->f_B4)
                {
                    ObjAction_InitFrameWait(20);
                    gObjActStepTimer = obj->headA.frameIdx;
                    gObjActStep = 2;
                }
            }
            else
            {
                if (obj->headA.frameIdx >= obj->f_B4 - 5)
                {
                    ObjAction_InitFrameWait(20);
                    Sfx_Play(0x69, 1, 0);
                    gObjActStepTimer = obj->headA.frameIdx;
                    gObjActStep = 2;
                }
            }
            break;
        case 2:
            if (obj->slot != 0x5E)
            {
                if (gObjActStepTimer == obj->f_B4 - 1)
                {
                    sub_804BF14(0, 0x10, 0xA, 0xA, 10, 4, 0, -1, 2);
                    sub_804B96C(7, 7, 0xA, 0xA, 10, 4, 0, -1, 2);
                }
                else if (gObjActStepTimer == obj->f_B4 + 5)
                {
                    sub_804C10C(0, 0x10);
                    sub_804BB64(7, 7);
                }
            }
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)&obj->headA));
                obj->headA.kindFlags &= 0xEFFF;
                sub_80207DC(obj, obj->posX, obj->posY, gObjActSavedF2A, gObjActSavedPal);
                gObjActStep = 9;
            }
            gObjActStepTimer += 1;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                result = 1;
            }
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x08042200
u8 sub_8042200(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            sub_801CA08((BattleObj *)obj, 3, 0x1B4, 0xD, result);
            gObjActStep = 2;
            break;
        case 2:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)(&obj->headA)));
                obj->headA.kindFlags &= 0xEFFF;
                sub_80207DC((BattleObj *)obj, obj->posX, obj->posY, gObjActSavedF2A, gObjActSavedPal);
                gObjActStep = 9;
            }
            break;
        case 9:
            result = 2;
            break;
    }
    return result;
}
// @ 0x080422B8
// 战斗对象"按槽号挂特效子头(headB)"演出状态机 (0x0839CEC4[5], sub_803F444 的
// animPtr+0x1C 分支 handler, 双参 obj + arg1 同池协作对象, 与 sub_8042784/8042B90 同表同签名):
// case0  按 obj->slot 在 gObjActSlotFxTable (6 项 ×12B) 里查表命中项 -> gObjActSlotFxCur
//        (未命中取 [0]); 存 headA.palSlot/f_1E/posX/posY; 清 headA.kindFlags bit0x400;
//        清 arg1 与 sub_80462E4 收集的参演对象 dmgAmount(+0xB2); sub_803F5B4;
//        slot==0x52 走 sub_801CA08(obj,3,0x1B4,0xD,0) 否则 (obj,4,...) -> 1。
// case1  等 headA.frameIdx == 表项 frameIdx -> 用表项 animIdx 在 gUnk_08393B28 取动画参数,
//        sub_801B81C 装配 obj->headB (锚点 = obj->posX/posY + 表项 xOff/yOff, 0x2EA, 0xE),
//        置 obj->state bit0x2000, 清 headB.f_2A -> 0x12。
// 0x12   等 headB.kindFlags bit0x800 落下 -> Sfx 0x51 -> 2。
// case2  headB.kindFlags bit0x1000 且 state bit0x2000 时释放 headB 调色板槽并清两标志位,
//        停音轨 -> 8; headA.kindFlags bit0x1000 时释放 headA 调色板槽并清位,
//        sub_801CA08(obj,0,SavedF2A,SavedPal,0) 复原, 归位 SavedX/SavedY -> 7。
// case7  同 case2 前半 (headB 收尾) -> 9。
// case8  同 case2 后半 (headA 收尾) -> 9。
// case9  等待窗互斥全空 -> 返回 1 (演出结束)。
// 尾部: gObjActStepTimer == 表项 timer 时开 40 帧等待窗; 每帧 sub_803F658 后 gObjActStepTimer++。
// case 源码顺序必须 0,1,0x12,2,7,8,9 (决定块布局)。
u8 sub_80422B8(BattleObj *obj, BattleObj *arg1)
{
    u8 result;
    u8 count;
    u8 i;
    u8 j;
    u8 *pool;
    u8 buf[12];

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            for (i = 0; i <= 5; i++)
            {
                if (obj->slot == gObjActSlotFxTable[i].slotId)
                {
                    gObjActSlotFxCur = &gObjActSlotFxTable[i];
                    break;
                }
            }
            if (i > 5)
            {
                gObjActSlotFxCur = &gObjActSlotFxTable[0];
            }
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            obj->headA.kindFlags &= 0xFBFF;
            arg1->dmgAmount = 0;
            pool = (u8 *)GetObjPool();
            count = sub_80462E4(obj, buf, 0x6F);
            for (j = 0; j < count; j++)
            {
                *(u16 *)(pool + buf[j] * 0xC8 + 0xB2) = 0;
            }
            sub_803F5B4(obj);
            if (obj->slot == 0x52)
            {
                sub_801CA08(obj, 3, 0x1B4, 0xD, 0);
            }
            else
            {
                sub_801CA08(obj, 4, 0x1B4, 0xD, 0);
            }
            gObjActStepTimer = 0;
            gObjActStep = 1;
            break;
        case 1:
            if (obj->headA.frameIdx == gObjActSlotFxCur->frameIdx)
            {
                sub_801B81C(&obj->headB,
                            obj->posX + gObjActSlotFxCur->xOff,
                            obj->posY + gObjActSlotFxCur->yOff,
                            0x2EA,
                            0xE,
                            (u32)gUnk_08393B28[gObjActSlotFxCur->animIdx].animScriptPtr,
                            (u32)gUnk_08393B28[gObjActSlotFxCur->animIdx].palettePtr,
                            gUnk_08393B28[gObjActSlotFxCur->animIdx].gfxBaseIdx,
                            gUnk_08393B28[gObjActSlotFxCur->animIdx].gfxTotal,
                            5);
                obj->state |= 0x2000;
                obj->headB.f_2A = 0;
                gObjActStep = 0x12;
            }
            break;
        case 0x12:
            if (!(obj->headB.kindFlags & 0x800))
            {
                Sfx_Play(0x51, 1, 0);
                gObjActStep = 2;
            }
            break;
        case 2:
            if ((obj->headB.kindFlags & 0x1000) && (obj->state & 0x2000))
            {
                sub_804C3A4(obj->headB.palSlot, sub_801B954((ObjHead *)&obj->headB));
                obj->headB.kindFlags &= 0xEFFF;
                obj->state &= 0xDFFF;
                Sfx_StopTrack(1);
                gObjActStep = 8;
            }
            if (obj->headA.kindFlags & 0x1000)
            {
                Sfx_StopTrack(1);
                sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)&obj->headA));
                obj->headA.kindFlags &= 0xEFFF;
                sub_801CA08(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                obj->posX = gObjActSavedX;
                obj->posY = gObjActSavedY;
                gObjActStep = 7;
            }
            break;
        case 7:
            if ((obj->headB.kindFlags & 0x1000) && (obj->state & 0x2000))
            {
                sub_804C3A4(obj->headB.palSlot, sub_801B954((ObjHead *)&obj->headB));
                obj->headB.kindFlags &= 0xEFFF;
                obj->state &= 0xDFFF;
                gObjActStep = 9;
                Sfx_StopTrack(1);
            }
            break;
        case 8:
            if (obj->headA.kindFlags & 0x1000)
            {
                Sfx_StopTrack(1);
                sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)&obj->headA));
                obj->headA.kindFlags &= 0xEFFF;
                sub_801CA08(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                obj->posX = gObjActSavedX;
                obj->posY = gObjActSavedY;
                gObjActStep = 9;
            }
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                result = 1;
            }
            break;
    }
    if (gObjActStepTimer == gObjActSlotFxCur->timer)
    {
        ObjAction_InitFrameWait(40);
    }
    sub_803F658(obj);
    gObjActStepTimer += 1;
    return result;
}
// @ 0x08042784
// 战斗对象"俯冲下压攻击"演出状态机 (0x0839CEC4[6], sub_803F444 的 animPtr+0x1C 分支):
// case0 存 posX/posY/palSlot/headA.f_1E, 清 headA.kindFlags bit0x400, 清 arg1 与
//       参演对象池各槽 dmgAmount(+0xB2); sub_803F5B4; sub_801CA08(obj,4,0x1B4,0xD,0) 装攻击动画
//       → 0x12。
// 0x12  等 headA.kindFlags bit0x800 落 → 1。
// 1     sub_8044394(obj); 等 headA.frameIdx >= f_B4-8 → Sfx 0x69 + 在 arg1 位置
//       (posY-0x60) 播锚点动画 sub_8020CC4(...,0x2EA,...,animPtr[0x1A]+5,5) → 2。
// 2     把 headB.f_2C 从 arg1->posY-0x60 下滑 0x48 (8 帧插值); 计时到 f_B4 时
//       开等待窗 (19 帧, SfxId 0x37) + sub_804BF14(0,3,7,7,7,4,4,-1,2) → 7。
// 7     计时到 f_B4+4 → sub_804C728(0,3,0x10), 清 state bit0x2000 → 8。
// 8     等 kindFlags bit0x1000 → 释放调色板槽 + 清 0x1000, sub_801CA08(obj,0,F2A,Pal,0)
//       恢复原姿势 → 9。
// 9     等待窗互斥全空 → 返回 1 (演出结束)。
// 尾部: 每帧 gObjActStepTimer++ 后 sub_803F658(obj)。
u8 sub_8042784(BattleObj *obj, BattleObj *arg1)
{
    u8 *anim;
    u8 result;
    u8 count;
    u8 i;
    u8 *pool;
    int frames;
    u8 buf[12];

    result = 0;
    sub_80187E8();
    anim = obj->animPtr;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            obj->headA.kindFlags &= 0xFBFF;
            arg1->dmgAmount = 0;
            pool = (u8 *)GetObjPool();
            count = sub_80462E4(obj, buf, 0x6F);
            for (i = 0; i < count; i++)
            {
                *(u16 *)(pool + buf[i] * 0xC8 + 0xB2) = 0;
            }
            sub_803F5B4(obj);
            sub_801CA08(obj, 4, 0x1B4, 0xD, 0);
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 0x12:
            if (!(obj->headA.kindFlags & 0x800))
            {
                gObjActStep = 1;
            }
            break;
        case 1:
            sub_8044394(obj);
            if (obj->headA.frameIdx >= obj->f_B4 - 8)
            {
                Sfx_Play(0x69, 1, 0);
                sub_8020CC4(obj, arg1->posX, arg1->posY - 0x60, 0x2EA, 0xA, *(u16 *)(anim + 0x1A) + 5, 5);
                gObjActStep = 2;
            }
            break;
        case 2:
            obj->headB.f_2C = sub_801768C(arg1->posY - 0x60, 0x48, 8, gObjActStepTimer - (obj->f_B4 - 8), 1);
            if (gObjActStepTimer >= obj->f_B4)
            {
                frames = 0x19;
                ObjAction_InitFrameWait(frames);
                sub_804BF14(0, 3, 7, 7, 7, 4, 4, -1, 2);
                gObjActStep = 7;
            }
            break;
        case 7:
            if (gObjActStepTimer >= obj->f_B4 + 4)
            {
                sub_804C728(0, 3, 0x10);
                obj->state &= 0xDFFF;
                gObjActStep = 8;
            }
            break;
        case 8:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)&obj->headA));
                obj->headA.kindFlags &= 0xEFFF;
                sub_801CA08(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 9;
            }
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                result = 1;
            }
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x08042AB4
u8 sub_8042AB4(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActStepTimer = 0;
            if ((sub_80187B4() & 0x200) == 0)
            {
                u16 f2a = obj->headA.kindFlags | 0x20;
                obj->headA.kindFlags = f2a;
                gObjActStep = 2;
            }
            else
            {
                gObjActStep = 0x12;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                obj->posX = sub_801768C(gObjActSavedX, -gObjActSavedX, 0x14, gObjActStepTimer, gObjActStep);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStep = 9;
            }
            break;
        case 9:
            sub_80207A4();
            obj->variantClass = 7;
            obj->slot = 0xFF;
            result = 2;
            break;
        case 0x12:
            result = 2;
            break;
    }
     return result;
}
// @ 0x08042B90
u8 sub_8042B90(BattleObj *obj, BattleObj *arg1)
{
    u8 result;
    u8 ids[12];
    u8 i;
    BattleObj *pool;
    u8 count;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            obj->headA.kindFlags &= 0xFBFF;
            arg1->dmgAmount = 0;
            pool = (BattleObj *)GetObjPool();
            count = sub_80462E4(obj, ids, 0x6F);
            for (i = 0; i < count; i++)
            {
                pool[ids[i]].dmgAmount = 0;
            }
            sub_803F5B4(obj);
            obj->f_B6 = 1;
            obj->f_B4 = 0;
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 0x12:
            if (gObjActStepTimer <= 0xE)
            {
                obj->pad_C1 = sub_801768C(0, obj->posY - 2, 0xF, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->posX = arg1->posX;
                obj->posY = arg1->posY;
                obj->pad_C1 = arg1->posY - 2;
                gObjActStepTimer = 0;
                gObjActStep = 0x13;
            }
            break;
        case 0x13:
            if (gObjActStepTimer <= 4)
            {
                obj->pad_C1 = sub_801768C(arg1->posY - 2, -arg1->posY, 5, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->pad_C1 = 0;
                obj->posY = arg1->posY + 1;
                gObjActStepTimer = 0;
                ObjAction_InitFrameWait(40);
                Sfx_Play(0x4E, 2, 0);
                gObjActStep = 0x14;
            }
            break;
        case 0x14:
            if (gObjActStepTimer <= 0xE)
            {
                gObjActStepTimer += 1;
            }
            else
            {
                obj->posX = gObjActSavedX;
                obj->posY = gObjActSavedY;
                gObjActStep = 0x15;
            }
            break;
        case 0x15:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                gObjActStep = 0x1C;
            }
            else
            {
                gObjActStepTimer += 1;
            }
            break;
        case 0x1C:
            result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x08042E70
INCLUDE_ASM("asm/nonmatchings", sub_8042E70);
static inline void ObjAction_ClearTargetDamage(BattleObj *obj)
{
    BattleObj *targets;
    u8 count;
    u8 i;
    u8 ids[12];

    targets = (BattleObj *)GetObjPool();
    count = sub_80462E4(obj, ids, 0x6F);
    for (i = 0; i < count; i++)
        targets[ids[i]].dmgAmount = 0;
}

// @ 0x08043554
u32 sub_8043554(BattleObj *obj)
{
    u32 result = 0;
    BattleObj *pool = (BattleObj *)GetObjPool();
    u8 slots[8];
    u8 i;
    u16 animation;

    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = obj->posX;
        gObjActSavedY = obj->posY;
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        gObjActStepTimer = 0;
        ObjAction_ClearTargetDamage(obj);
        sub_803F5B4(obj);
        obj->f_B6 = 0;
        obj->f_B4 = 0;
        for (i = 0; i < gObjActGroupCount; i++)
            slots[i] = gObjActGroupSlots[i] & 0xF;
        sub_8048ACC(slots, gObjActGroupCount, 0xB);
        gObjActMoveFromX = pool[slots[gObjActGroupCount - 1]].posX
            + (pool[slots[0]].posX - pool[slots[gObjActGroupCount - 1]].posX) / 2;
        gObjActStep = 0x12;
        break;
    case 0x12:
        sub_801CA08(obj, 4, 0x1B4, 0xC, 0);
        gObjActStep = 0x13;
        break;
    case 0x13:
        if (!(obj->headA.kindFlags & 0x800))
        {
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 0x14;
        }
        break;
    case 0x14:
        if (obj->headA.kindFlags & 0x1000)
        {
            Sfx_StopTrack(1);
            sub_801CA08(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x15;
        }
        break;
    case 0x15:
        if (!(obj->headA.kindFlags & 0x800))
        {
            switch (obj->slot)
            {
            case 0x5D:
            default:
                animation = 0x34E;
                break;
            case 0x63:
                animation = 0x34F;
                break;
            }
            sub_8020CC4(obj, gObjActMoveFromX, 0x82, 0x1B4, 0xC, animation, 5);
            obj->headB.f_2A = 2;
            gObjActStep = 0x16;
        }
        break;
    case 0x16:
        if (!(obj->headB.kindFlags & 0x800))
            gObjActStep = 0x17;
        break;
    case 0x17:
        if (obj->headB.frameIdx > 0x2D)
        {
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            Sfx_Play(0x1E, 1, 0);
            gObjActStepTimer = 0;
            gObjActStep = 0x18;
        }
        break;
    case 0x18:
        if (gObjActStepTimer <= 3)
            gObjActStepTimer++;
        else
        {
            sub_804C728(0, 3, 0x10);
            ObjAction_InitFrameWait(40);
            gObjActStep = 0x19;
        }
        break;
    case 0x19:
        if (obj->headB.kindFlags & 0x1000)
        {
            obj->state &= 0xDFFF;
            gObjActStep = 9;
        }
        gObjActStepTimer++;
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            result = 1;
        else
            gObjActStepTimer++;
        break;
    case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
    case 0xA: case 0xB: case 0xC: case 0xD: case 0xE: case 0xF:
    case 0x10: case 0x11: case 0x1A: case 0x1B: case 0x1C: case 0x1D:
    case 0x1E: case 0x1F:
        break;
    }
    sub_803F658(obj);
    return result;
}

static inline void ObjAction_InitFrameWaitWithSfx(s16 frames, u16 soundId, u8 soundParam)
{
    gActWaitBusy0 = 1;
    gActWaitBusy1 = 0;
    gActWaitBusy2 = 0;
    if (frames < 0)
        gActWaitFrames = 12;
    else
        gActWaitFrames = frames;
    gActWaitCnt0 = 0;
    gActWaitCnt1 = 0;
    gActWaitSfxId = soundId;
    gActWaitSfxParam = soundParam;
}

// @ 0x08043938
u32 sub_8043938(BattleObj *obj)
{
    u32 ret;
    u8 count;
    u8 i;
    BattleObj *pool;
    u8 buf[12];

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = obj->posX;
        gObjActSavedY = obj->posY;
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        gObjActStepTimer = 0;
        pool = (BattleObj *)GetObjPool();
        count = (u8)sub_80462E4(obj, buf, 0x6F);
        i = 0;
        if (i < count)
        {
            do
            {
                pool[buf[i]].dmgAmount = 0;
                i = (u8)(i + 1);
            } while (i < count);
        }
        sub_803F5B4(obj);
        gObjActStep = 0x12;
        break;
    case 18:
        sub_801CA08(obj, 4, 0x1B4, 0xC, 0);
        gObjActStep = 0x13;
        break;
    case 19:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0xA5, 1, 0);
        gObjActStep = 0x14;
        break;
    case 20:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        Sfx_StopTrack(1);
        sub_801CA08(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
        gObjActStep = 0x15;
        break;
    case 21:
        if (obj->headA.kindFlags & 0x800)
            break;
        ObjAction_InitFrameWaitWithSfx(50, 0x64, 0);
        gObjActStepTimer = 0;
        gObjActStep = 9;
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            ret = 1;
        else
            gObjActStepTimer += 1;
        break;
    }
    sub_803F658(obj);
    return ret;
}

// @ 0x08043B5C
// 战斗对象"归位/突进到目标位"长演出状态机 (0x0839CEC4[11], sub_803F444 的 animPtr+0x1C/+0x1E 分支,
// 双参 obj + arg1): 0 → 0x12 → 0x13 → 0x14 → 0x15 → 0x16 → 0x17 → 0x18 → 0x19 → 0x1A → 0x1B → 9。
// case0  存 posX/posY/palSlot/headA.f_1E; 清 0825; 清目标列表各槽 dmgAmount(+0xB2);
//        sub_803F5B4; f_B6=1 / f_B4=0 (动画表参数); 记锚点 gObjActMoveFromX=arg1->posX,
//        gObjActMoveFromY=arg1->posY - (u8)sub_801EC3C(arg1,1)/2 → 0x12。
// 0x12   sub_801CA08(obj,3,0x1B4,0xC,0) 装攻击动画 → 0x13。
// 0x13   等 headA.kindFlags bit0x800 落 → 0x14。
// 0x14   等 headA.frameIdx > 0x1C → Sfx 0x8B + sub_804B834(0xD,1,4,-3,3)(借槽位演出) → 0x15。
// 0x15   计时 <= 0x13 则 ++, 否则 sub_804B8E8(0xD,1) 还槽 → 0x16。
// 0x16   等 headA.frameIdx > 0x45 → 在自身位置播锚点动画 sub_8020CC4(...,0x27C,...,0x34C,5) → 0x17。
// 0x17   等 headB.kindFlags bit0x800 落 → 0x18。
// 0x18   计时 <= 0xE 则 ++, 否则 Sfx 0x31 → 0x19。
// 0x19   计时 <= 0xE 时把 headB.f_2B/f_2C 从自身 posX/(posY-0x1E) 插值到
//        gObjActMoveFromX/(gObjActMoveFromY-(posY-0x14)) (15 帧, 模式 2); 计时 > 0xE 时
//        清 state bit0x2000, 开 30 帧等待窗 (SfxId 0x37), headA.kindFlags|=0x100 → 0x1A。
// 0x1A   等待窗三互斥全空 → sub_801CA08(obj,0,F2A,Pal,0) 恢复原姿势 → 0x1B; 否则计时 ++。
// 0x1B   等 headA.kindFlags bit0x800 落 → 9。
// 9      等待窗三互斥全空 → 返回 1 (演出结束)。
// 尾部: sub_803F658(obj); 返回 result (存于栈槽 sp+0x18, 全 callee-saved 寄存器已占满)。
u32 sub_8043B5C(BattleObj *obj, BattleObj *arg1)
{
    u32 result;
    u8 count;
    u8 i;
    BattleObj *pool;
    u8 buf[12];

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActStepTimer = 0;
            pool = (BattleObj *)GetObjPool();
            count = sub_80462E4(obj, buf, 0x6F);
            for (i = 0; i < count; i++)
            {
                pool[buf[i]].dmgAmount = 0;
            }
            sub_803F5B4(obj);
            obj->f_B6 = 1;
            obj->f_B4 = 0;
            gObjActMoveFromX = arg1->posX;
            gObjActMoveFromY = arg1->posY - ((u8)sub_801EC3C(arg1, 1) >> 1);
            gObjActStep = 0x12;
            break;
        case 0x12:
            sub_801CA08(obj, 3, 0x1B4, 0xC, 0);
            gObjActStep = 0x13;
            break;
        case 0x13:
            if (!(obj->headA.kindFlags & 0x800))
            {
                gObjActStep = 0x14;
            }
            break;
        case 0x14:
            if (obj->headA.frameIdx > 0x1C)
            {
                Sfx_Play(0x8B, 1, 0);
                sub_804B834(0xD, 1, 4, -3, 3);
                gObjActStepTimer = 0;
                gObjActStep = 0x15;
            }
            break;
        case 0x15:
            if (gObjActStepTimer <= 0x13)
            {
                gObjActStepTimer += 1;
            }
            else
            {
                sub_804B8E8(0xD, 1);
                gObjActStepTimer = 0;
                gObjActStep = 0x16;
            }
            break;
        case 0x16:
            if (obj->headA.frameIdx > 0x45)
            {
                sub_8020CC4(obj, obj->posX, obj->posY - 0x1E, 0x27C, 0xE, 0x34C, 5);
                gObjActStepTimer = 0;
                gObjActStep = 0x17;
            }
            break;
        case 0x17:
            if (!(obj->headB.kindFlags & 0x800))
            {
                gObjActStepTimer = 0;
                gObjActStep = 0x18;
            }
            break;
        case 0x18:
            if (gObjActStepTimer <= 0xE)
            {
                gObjActStepTimer += 1;
            }
            else
            {
                Sfx_Play(0x31, 1, 0);
                gObjActStepTimer = 0;
                gObjActStep = 0x19;
            }
            break;
        case 0x19:
            if (gObjActStepTimer <= 0xE)
            {
                obj->headB.f_2B = sub_801768C(obj->posX, gObjActMoveFromX - obj->posX, 0xF, gObjActStepTimer, 2);
                obj->headB.f_2C = sub_801768C(obj->posY - 0x1E, gObjActMoveFromY - (obj->posY - 0x14), 0xF, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->state &= 0xDFFF;
                gObjActStepTimer = 0;
                ObjAction_InitFrameWait(30);
                obj->headA.kindFlags |= 0x100;
                gObjActStep = 0x1A;
            }
            break;
        case 0x1A:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801CA08(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x1B;
            }
            else
            {
                gObjActStepTimer += 1;
            }
            break;
        case 0x1B:
            if (!(obj->headA.kindFlags & 0x800))
            {
                gObjActStep = 9;
            }
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                result = 1;
            }
            break;
        /* 显式列出全部 0..0x1F 的空 case: 决定跳转表宽度 (cmp #0x1F) 与表尾默认项 */
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
        case 0x10:
        case 0x11:
        case 0x1C:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x08043F90
u32 sub_8043F90(BattleObj *obj)
{
    u32 result;
    u8 count;
    u8 i;
    BattleObj *pool;
    u8 buf[12];

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActStepTimer = 0;
            pool = (BattleObj *)GetObjPool();
            count = sub_80462E4(obj, buf, 0x6F);
            for (i = 0; i < count; i++)
            {
                pool[buf[i]].dmgAmount = 0;
            }
            sub_803F5B4(obj);
            obj->f_B6 = 1;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 0x12:
            sub_801CA08(obj, 4, 0x1B4, 0xC, 0);
            gObjActStep = 0x13;
            break;
        case 0x13:
            if (!(obj->headA.kindFlags & 0x800))
            {
                gObjActStep = 0x14;
            }
            break;
        case 0x14:
            if (obj->headA.frameIdx > 0x42)
            {
                obj->headA.kindFlags |= 0x100;
                gObjActStepTimer = 0;
                gObjActStep = 0x15;
            }
            break;
        case 0x15:
            if (gObjActStepTimer <= 0x13)
            {
                obj->posX = sub_801768C(gObjActSavedX, 0xBA - gObjActSavedX, 0x14, gObjActStepTimer, 2);
                obj->posY = sub_801768C(gObjActSavedY, 0x68 - gObjActSavedY, 0x14, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gObjActStep = 0x16;
            }
            break;
        case 0x16:
            sub_8020CC4(obj, obj->posX, obj->posY - 0x20, 0x27C, 0xE, 0x341, 5);
            gObjActStep = 0x17;
            break;
        case 0x17:
            if (!(obj->headB.kindFlags & 0x800))
            {
                obj->headA.kindFlags |= 0x200;
                obj->state |= 4;
                sub_804BF14(0, 3, 0x1C, 0xE, 7, 4, 4, -1, 2);
                Sfx_Play(0x31, 1, 0);
                gObjActStepTimer = 0;
                gObjActStep = 0x18;
            }
            break;
        case 0x18:
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
            }
            else
            {
                sub_804C728(0, 3, 0x10);
                gObjActStepTimer = 0;
                gObjActStep = 0x19;
            }
            break;
        case 0x19:
            if (obj->headB.kindFlags & 0x1000)
            {
                ObjAction_InitFrameWait(30);
                obj->state &= 0xDFFF;
                gObjActStepTimer = 0;
                gObjActStep = 0x1A;
            }
            break;
        case 0x1A:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801CA08(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                obj->posX = gObjActSavedX;
                obj->posY = gObjActSavedY;
                gObjActStep = 0x1B;
            }
            else
            {
                gObjActStepTimer += 1;
            }
            break;
        case 0x1B:
            if (!(obj->headA.kindFlags & 0x800))
            {
                obj->state &= 0xFFFB;
                gObjActStep = 0x1C;
            }
            break;
        case 0x1C:
            gObjActStep = 9;
            break;
        case 9:
            result = 1;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 0xA:
        case 0xB:
        case 0xC:
        case 0xD:
        case 0xE:
        case 0xF:
        case 0x10:
        case 0x11:
        case 0x1D:
        case 0x1E:
        case 0x1F:
            break;
    }
    sub_803F658(obj);
    return result;
}
