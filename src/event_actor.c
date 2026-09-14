#include "code_0.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"
#include "sound.h"


// @ 0x080264C0
INCLUDE_ASM("asm/nonmatchings", sub_80264C0);
// @ 0x0802698C
/* 战斗对象"多段闪光/抖动演出"状态机 (gObjActStep: 0 → 3 → 4 → 2 → 1 ⇄ 5 → 6 → 7 → 9)。
 * 用第二动画头 headB (obj+0x3C) 做两次 sub_801B81C 装配 (不同表项 0x08393B28+0x3714 / +0x3728),
 * 并在 headB.frameIdx 上做 8 步循环抖动 (frameIdx 在 0xA..0x11 间回绕), 最后 case9 全体概率效果。
 *   case0  sub_80444A4(obj) 清同组伤害 + sub_803F5B4 → 3。
 *   case3  存 headA.f_1E/palSlot; sub_801CE80(obj,5,0x1B4,0xD,0); 计数=0 → 4。
 *   case4  headA.frameIdx > 0x46 → 2。
 *   case2  用 sub_801B81C(&headB, 0x80, 0x77, 0x2EA, 0xE, 表项, 4) 装配; state|=0x2000;
 *          headB.f_2A=2; 计数=0 → 1。
 *   case1  headB.frameIdx==9 → headB/headA.kindFlags 各置 0x100, 计数=0, frameIdx=0xA;
 *          否则 frameIdx>9 时 frameIdx = (frameIdx+1)%8 + 0xA, 计数++;
 *          计数==0x10 → 清 headA 的 0x100; 计数>0x17 → 清 headB 的 0x1000,
 *          再装配 headB (0x74,0x80, 第二表项), headB/state 置 0x2000, headB.f_2A=2 → 5。
 *   case5  headB 的 0x1000 落 → 清 headB 0x1000/state 0x2000, obj->f_B6=1, sub_8044514(0x28) → 6。
 *   case6  gActEventCount 非 0 → 9 否则 → 7。
 *   case7  headA 的 0x1000 落 → 清位 + sub_801CE80 复位 → 9。
 *   case9  无其他演出占用时对 0x03000840[] 中 (v&0xF0)==0x10 项按 Rng%100<=0x27 置 gObjSlotFxCmd; 返回 1。
 * 尾部 sub_803F658 + headA.kindFlags 的 0x1000 收尾。 */
u32 sub_802698C(BattleObj *obj)
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
            sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
            gObjActStepTimer = 0;
            gObjActStep = 4;
            break;
        case 4:
            if (obj->headA.frameIdx > 0x46)
                gObjActStep = 2;
            break;
        case 2:
        {
            sub_801B81C(&obj->headB, 0x80, 0x77, 0x2EA, 0xE,
                        gUnk_08393B28[0x2C1].field_0, gUnk_08393B28[0x2C1].field_4,
                        gUnk_08393B28[0x2C1].field_8, gUnk_08393B28[0x2C1].field_A, 4);
            obj->state |= 0x2000;
            obj->headB.f_2A = 2;
            gObjActStepTimer = 0;
            gObjActStep = 1;
            break;
        }
        case 1:
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
                obj->headB.kindFlags &= 0xEFFF;
                sub_801B81C(&obj->headB, 0x74, 0x80, 0x2EA, 0xE,
                            gUnk_08393B28[0x2C2].field_0, gUnk_08393B28[0x2C2].field_4,
                            gUnk_08393B28[0x2C2].field_8, gUnk_08393B28[0x2C2].field_A, 4);
                obj->headB.kindFlags |= 0x2000;
                obj->state |= 0x2000;
                obj->headB.f_2A = 2;
                gObjActStep = 5;
            }
            break;
        case 5:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            obj->headB.kindFlags &= 0xEFFF;
            obj->state &= 0xDFFF;
            obj->f_B6 = 1;
            sub_8044514(0x28);
            gObjActStep = 6;
            break;
        case 6:
            if (gActEventCount != 0)
                gObjActStep = 9;
            else
                gObjActStep = 7;
            break;
        case 7:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            obj->headA.kindFlags &= 0xEFFF;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
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
                if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
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
// @ 0x08026D08
/* 战斗对象"开场闪光/亮场过渡"演出状态机 (gObjActStep: 0 起手 → 3 等 headB 落地 →
 * 4 起手 2 → 2 等 headA → 5 淡入 → 1 循环闪 → 9 收尾)。
 * 使用第二动画头 headB (obj+0x3C) 与 GBA BLDY 亮度寄存器 0x04000050/0x04000052。
 *   case0  存 headA.f_1E/palSlot; 用 sub_801B81C 装配 headB (锚 0xA,0x13, 表项 0x08393B28+0x37A0);
 *          obj->state |= 0x2000 (跳跃位); headB.f_2A=0; 计数=0 → 3。
 *   case4  headB.frameIdx > 0x23 → sub_801CE80(obj,5,0x1B4,0xD,1); headA.kindFlags |= 0x210;
 *          BLDY=0x400 (0x04000050), BLDALPHA=0xC0F (0x04000052); headA.f_2A=3;
 *          headB.kindFlags |= 0x100 → 2。
 *   case2  headA.kindFlags 的 0x800 落 → 清 headB 的 0x100 与 headA 的 0x200; → 5; 计数=0。
 *   case5  计数<=4: BLDY = sub_801768C(3,0xC,5,计数,2) + 0xC00; 计数++; 否则 → 1; 计数=0。
 *   case1  headA 的 0x1000 落: 若 headA 已置 0x100 则 headA.frameIdx=(frameIdx+1)%5+0x87
 *          否则置 0x100; 计数<=0xE 时 BLDY = sub_801768C(0xF,-0xC,0xF,计数,1)+0xC00 并计数++;
 *          最后 headB 的 0x1000 落 → sub_801CE80 复位 + 清 obj->state 的 0x2000 → 9。
 *   case9  headA.f_2A=3; 返回 2 (动作完成, 无 sub_803F658)。 */
u32 sub_8026D08(BattleObj *obj)
{
    u32 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_801B81C(&obj->headB, 0xA, 0x13, 0, 0xE,
                        gUnk_08393B28[0x2C8].field_0, gUnk_08393B28[0x2C8].field_4,
                        gUnk_08393B28[0x2C8].field_8, gUnk_08393B28[0x2C8].field_A, 6);
            obj->state |= 0x2000;
            obj->headB.f_2A = 0;
            gObjActStepTimer = 0;
            gObjActStep = 3;
            break;
        case 4:
            if (obj->headB.frameIdx <= 0x23)
                break;
            sub_801CE80(obj, 5, 0x1B4, 0xD, 1);
            obj->headA.kindFlags |= 0x210;
            *(volatile u16 *)0x04000050 = 0x400;
            *(volatile u16 *)0x04000052 = 0xC0F;
            obj->headA.f_2A = 3;
            obj->headB.kindFlags |= 0x100;
            gObjActStep = 2;
            break;
        case 2:
            if (obj->headA.kindFlags & 0x800)
                break;
            obj->headB.kindFlags &= 0xFEFF;
            obj->headA.kindFlags &= 0xFDFF;
            gObjActStep = 5;
            gObjActStepTimer = 0;
            break;
        case 5:
            if (gObjActStepTimer <= 4)
            {
                *(volatile u16 *)0x04000052 = sub_801768C(3, 0xC, 5, gObjActStepTimer, 2) + 0xC00;
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStep = 1;
                gObjActStepTimer = 0;
            }
            break;
        case 1:
            if (obj->headA.kindFlags & 0x1000)
            {
                if (obj->headA.kindFlags & 0x100)
                {
                    obj->headA.frameIdx = (obj->headA.frameIdx + 1) % 5 + 0x87;
                }
                else
                {
                    obj->headA.kindFlags |= 0x100;
                }
                if (gObjActStepTimer <= 0xE)
                {
                    *(volatile u16 *)0x04000052 = sub_801768C(0xF, -0xC, 0xF, gObjActStepTimer, 1) + 0xC00;
                    gObjActStepTimer += 1;
                }
            }
            if (obj->headB.kindFlags & 0x1000)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                obj->state &= 0xDFFF;
                gObjActStep = 9;
            }
            break;
        case 9:
            obj->headA.f_2A = 3;
            result = 2;
            break;
    }
    return result;
}
// @ 0x08026F88
/* 战斗对象"召唤伙伴+抖动演出"状态机 (gObjActStep: 0 → 0x12..0x18 → 9)。
 * 双参 (obj, arg1=同池另一对象, 用于 case22 给伙伴置 0x2000)。
 *   case0  记 posX/posY/f_1E/palSlot; sub_80444A4 清同组伤害 +
 *          sub_801CE80(obj,1,0x1B4,0xD,0) 切动画 + sub_803F5B4; 计数=0 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → Sfx(0x57) → 0x13。
 *   0x13   headA.frameIdx > 0x6E → sub_8020CC4(obj,7,0xF,0,0xE,0x2CD,7) 起手;
 *          gSceneFadeOut=0; sub_801A2AC(0x442, 0, 0xC) → 0x14。
 *   0x14   headA.frameIdx <= 0x82 时用 sub_801768C(0,7,0x14, frameIdx-0x6F, 2) 更新
 *          gSceneFadeOut 并 sub_801A2AC(0x442, gSceneFadeOut, 0xC); 否则 → 0x15。
 *   0x15   headA.frameIdx <= 0x95 时 sub_801768C(7,-7,0x13, frameIdx-0x83, 2) 更新同上;
 *          否则 headA.kindFlags |= 0x100; state &= 0xDFFF; DISPCNT(0x04000000) &= 0xFDFF;
 *          sub_801A2AC(0, gSceneFadeOut, 0xC); sub_8044514(0x2D); 计数=0 → 0x16。
 *   0x16   arg1->headB.kindFlags |= 0x2000 → 0x17。
 *   0x17   无其他演出占用 → headA.kindFlags &= 0xFEFF → 0x18。
 *   0x18   headA.kindFlags 到 0x1000 → sub_801CE80 复位 → 9。
 *   9      result = 1。
 * 尾部: gObjActStepTimer += 1 + sub_803F658(obj)。 */
u8 sub_8026F88(BattleObj *obj, BattleObj *arg1)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 1, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x57, 1, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx > 0x6E)
            {
                sub_8020CC4(obj, 7, 0xF, 0, 0xE, 0x2CD, 7);
                gSceneFadeOut = 0;
                sub_801A2AC(0x442, 0, 0xC);
                gObjActStep = 0x14;
            }
            break;
        case 20:
            if (obj->headA.frameIdx <= 0x82)
            {
                gSceneFadeOut = sub_801768C(0, 7, 0x14, (s16)(obj->headA.frameIdx - 0x6F), 2);
                sub_801A2AC(0x442, gSceneFadeOut, 0xC);
            }
            else
            {
                gObjActStep = 0x15;
            }
            break;
        case 21:
            if (obj->headA.frameIdx <= 0x95)
            {
                gSceneFadeOut = sub_801768C(7, -7, 0x13, (s16)(obj->headA.frameIdx - 0x83), 2);
                sub_801A2AC(0x442, gSceneFadeOut, 0xC);
            }
            else
            {
                obj->headA.kindFlags |= 0x100;
                obj->state &= 0xDFFF;
                *(volatile u16 *)0x04000000 &= 0xFDFF;
                sub_801A2AC(0, gSceneFadeOut, 0xC);
                sub_8044514(0x2D);
                gObjActStepTimer = 0;
                gObjActStep = 0x16;
            }
            break;
        case 22:
            arg1->headB.kindFlags |= 0x2000;
            gObjActStep = 0x17;
            break;
        case 23:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                obj->headA.kindFlags &= 0xFEFF;
                gObjActStep = 0x18;
            }
            break;
        case 24:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 9;
            }
            break;
        case 9:
            result = 1;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x0802723C
/* 战斗对象"召唤/技能释放"长演出状态机 (gObjActStep: 0 → 0x12..0x1E → 9)。
 * 与 sub_802B608 同族 (同样的 sub_801A2AC/sub_8019B98/sub_804BDD8/sub_801A348/
 * sub_804BE90 组合), 但本函数展开为更多中间态 (0x15..0x1E)。
 *   case0  记 posX/posY/f_1E/palSlot; sub_80444A4 清同组伤害 +
 *          sub_801CE80(obj,1,0x1B4,0xD,0) 切动画; obj->f_B6=1; sub_803F5B4; 计数=0 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → Sfx(0x57) → 0x13。
 *   0x13   headA.frameIdx > 0x6E → sub_8020CC4(obj,7,0xF,0,0xE,0x2CD,7);
 *          gSceneFadeOut=0; sub_801A2AC(0x442,0,0xC) → 0x14。
 *   0x14   frameIdx<=0x82 → gSceneFadeOut = sub_801768C(0,7,0x14,frameIdx-0x6F,2) +
 *          sub_801A2AC(0x442,gSceneFadeOut,0xC); 否则 → 0x15。
 *   0x15   frameIdx<=0x95 → sub_801768C(7,-7,0x13,frameIdx-0x83,2) 同上; 否则
 *          headA.kindFlags|=0x100; state&=0xDFFF; → 0x16。
 *   0x16   headA.kindFlags = (kindFlags|0x100)&0xEFFF; sub_801A348() → 0x17。
 *   0x17   sub_8019B98(0xD,3,0xE,2)!=0 → Sfx(0x55) + sub_801A2AC(0x1447,9,0xF) +
 *          sub_804BDD8(0xE,1,1,-3,0xC) → 0x18。
 *   0x18   等 headA.kindFlags 的 0x800 落 → sub_8044514(0x3C); 计数=0 → 0x19。
 *   0x19   计数<=0x3B 自增; 否则 计数=0 → 0x1A。
 *   0x1A   计数<=0x1D → gSceneFadeOut = sub_801768C(9,-9,0x1E,cnt,2) + sub_801A2AC(0x1447,..,0xF);
 *          否则 Sfx_StopTrack(1) + sub_804BE90(0xE,1) + sub_801A2AC(0,0,0) +
 *          清 DISPCNT bit9 + state&=0xDFFF → 0x1B。
 *   0x1B   headA.kindFlags &= 0xFEFF → 0x1C。
 *   0x1C   无其他演出占用 → headA.kindFlags &= 0xFEFF → 0x1D。
 *   0x1D   headA.kindFlags 到 0x1000 → sub_801CE80 复位 → 0x1E。
 *   0x1E   headA.kindFlags 的 0x800 落 → 9。
 *   9      result = 1。
 * 尾部: sub_803F658(obj) (本函数无帧计数自增)。 */
u8 sub_802723C(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 1, 0x1B4, 0xD, 0);
            obj->f_B6 = 1;
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x57, 1, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx > 0x6E)
            {
                sub_8020CC4(obj, 7, 0xF, 0, 0xE, 0x2CD, 7);
                gSceneFadeOut = 0;
                sub_801A2AC(0x442, 0, 0xC);
                gObjActStep = 0x14;
            }
            break;
        case 20:
            if (obj->headA.frameIdx <= 0x82)
            {
                gSceneFadeOut = sub_801768C(0, 7, 0x14, (s16)(obj->headA.frameIdx - 0x6F), 2);
                sub_801A2AC(0x442, gSceneFadeOut, 0xC);
            }
            else
            {
                gObjActStep = 0x15;
            }
            break;
        case 21:
            if (obj->headA.frameIdx <= 0x95)
            {
                gSceneFadeOut = sub_801768C(7, -7, 0x13, (s16)(obj->headA.frameIdx - 0x83), 2);
                sub_801A2AC(0x442, gSceneFadeOut, 0xC);
            }
            else
            {
                obj->headA.kindFlags |= 0x100;
                obj->state &= 0xDFFF;
                gObjActStep = 0x16;
            }
            break;
        case 22:
            obj->headA.kindFlags = obj->headA.kindFlags | 0x100;
            obj->headA.kindFlags = obj->headA.kindFlags & 0xEFFF;
            sub_801A348();
            gObjActStep = 0x17;
            break;
        case 23:
            if (sub_8019B98(0xD, 3, 0xE, 2) == 0)
                break;
            Sfx_Play(0x55, 1, 0);
            sub_801A2AC(0x1447, 9, 0xF);
            sub_804BDD8(0xE, 1, 1, -3, 0xC);
            gObjActStep = 0x18;
            break;
        case 24:
        {
            u16 flags = obj->headA.kindFlags & 0x800;
            if (flags != 0)
                break;
            sub_8044514(0x3C);
            gObjActStepTimer = flags;
            gObjActStep = 0x19;
            break;
        }
        case 25:
            if (gObjActStepTimer <= 0x3B)
            {
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 0x1A;
            break;
        case 26:
            if (gObjActStepTimer <= 0x1D)
            {
                gSceneFadeOut = sub_801768C(9, -9, 0x1E, gObjActStepTimer, 2);
                sub_801A2AC(0x1447, gSceneFadeOut, 0xF);
                gObjActStepTimer += 1;
            }
            else
            {
                Sfx_StopTrack(1);
                sub_804BE90(0xE, 1);
                sub_801A2AC(0, 0, 0);
                *(volatile u16 *)(0x80 << 0x13) &= 0xFDFF;
                obj->state &= 0xDFFF;
                gObjActStep = 0x1B;
            }
            break;
        case 27:
            obj->headA.kindFlags &= 0xFEFF;
            gObjActStep = 0x1C;
            break;
        case 28:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                obj->headA.kindFlags &= 0xFEFF;
                gObjActStep = 0x1D;
            }
            break;
        case 29:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x1E;
            }
            break;
        case 30:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 9;
            break;
        case 9:
            result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802761C
/* 战斗对象"特效/背景覆盖"演出状态机 (gObjActStep: 0 起手 → 1..7 等待 → 8 收尾 → 9 全体效果)。
 * 唯一入口是 0x0839CD5C 指针表 idx9 (sub_803F444 对 slot 0xB..0x70 的对象按下标间接调用;
 * 调用者 sub_801BE34/sub_801C484 传入的对象来自 GetObjPool() 槽, 即 BattleObj)。
 * case0 记下 posX/posY/headA.f_1E/headA.palSlot 供尾部复位, 再经 sub_80444A4 (清同组
 * 对象的 dmgAmount) + sub_801CE80(obj,5,0x1B4,0xD,0) (动画切换) + sub_803F5B4 起手;
 * case9 在无其他演出占用时给 0x03000840 里 (v&0xF0)==0x10 的条目按 40% 置 gObjSlotFxCmd。
 * 尾部 headA.kindFlags bit12 (0x1000 = 本对象演出完成) 置位则清位并把暂存的 f_1E/palSlot
 * 交回 sub_801CE80 收尾。零变量可减少尾部 RMW 的调度差异 (见 TSV note)。 */
u8 sub_802761C(BattleObj *obj)
{
    u8 result;
    u8 i;
    u32 zero;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 1;
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                GetObjPool();
                for (i = 0; i < gObjActGroupCount; i++)
                {
                    if ((gObjActGroupSlots[i] & 0xF0) == 0x10)
                    {
                        if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
                        {
                            gObjSlotFxCmd[gObjActGroupSlots[i] & 0xF] = 1;
                        }
                    }
                }
                result = 1;
            }
            break;
    }
    sub_803F658(obj);
    if (obj->headA.kindFlags & 0x1000)
    {
        u16 masked = obj->headA.kindFlags & 0xEFFF;
        zero = 0;
        obj->headA.kindFlags = masked;
        sub_801CE80(obj, zero, gObjActSavedF2A, gObjActSavedPal, zero);
    }
    return result;
}
// @ 0x08027760
/* 战斗对象"换位靠近"演出状态机 (gObjActStep: 0 起手 → 0x12..0x19 → 9)。
 * 与 sub_80282EC 同族但流程不同: 接收第二个 BattleObj* (参1, 由 sub_803F444 分派器传入
 * 同池另一对象), 位置插值以 arg1 的 posX/posY 为目标。
 *   case0  记 posX/posY/headA.f_1E/headA.palSlot, sub_80444A4 清同组伤害 + sub_803F5B4 起手,
 *          Sfx(0x28) → 0x12。
 *   0x12   帧计数 <= 0x31 时把 posX/posY 从暂存值向 (arg1->posX-0x96)/(arg1->posY+0xF)
 *          按 0x32 帧插值; 超过则 Sfx_StopTrack(1) → 0x13。
 *   0x13   sub_801CE80(obj,1,0x1B4,0xB,0) 切动画 → 0x14。
 *   0x14   headA 分片装载到位 (gfxPos >= gfxTotal-1) → posX += 0x34, 计数=0 → 0x15。
 *   0x15   等 headA.kindFlags 的 0x800 落 → 0x16。
 *   0x16   headA.frameIdx > 0x4D → sub_8044514(0x14) → 0x17。
 *   0x17   headA.kindFlags 到 0x1000 → 清 0x1000 置 0x100 → 0x18。
 *   0x18   无其他演出占用时 sub_801CE80 复位 + kindFlags |= 0x100,
 *          posX/posY 还原暂存值 → 0x19。
 *   0x19   headA.kindFlags 的 0x800 落 → posX/posY 还原暂存值 → 9。
 *   9      返回 1。
 * 尾部: gObjActStepTimer += 1; sub_803F658(obj)。无 sub_80187E8, 无尾部 kindFlags 收尾。 */
u8 sub_8027760(BattleObj *obj, BattleObj *obj2)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            Sfx_Play(0x28, 1, 0);
            gObjActStep = 0x12;
            break;
        case 18:
            if (gObjActStepTimer <= 0x31)
            {
                obj->posX = sub_801768C(gObjActSavedX, (obj2->posX - 0x96) - gObjActSavedX, 0x32, gObjActStepTimer, 2);
                obj->posY = sub_801768C(gObjActSavedY, (obj2->posY + 0xF) - gObjActSavedY, 0x32, gObjActStepTimer, 2);
            }
            else
            {
                Sfx_StopTrack(1);
                gObjActStep = 0x13;
            }
            break;
        case 19:
            sub_801CE80(obj, 1, 0x1B4, 0xB, 0);
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headA.gfxPos < obj->headA.gfxTotal - 1)
                break;
            obj->posX += 0x34;
            gObjActStepTimer = 0;
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headA.frameIdx <= 0x4D)
                break;
            sub_8044514(0x14);
            gObjActStep = 0x17;
            break;
        case 23:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            obj->headA.kindFlags = (obj->headA.kindFlags & 0xEFFF) | 0x100;
            gObjActStep = 0x18;
            break;
        case 24:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                obj->headA.kindFlags |= 0x100;
                obj->posX = gObjActSavedX;
                obj->posY = gObjActSavedY;
                gObjActStep = 0x19;
            }
            break;
        case 25:
            if (obj->headA.kindFlags & 0x800)
                break;
            obj->posX = gObjActSavedX;
            obj->posY = gObjActSavedY;
            gObjActStep = 9;
            break;
        case 9:
            result = 1;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x08027A20
/* 战斗对象"前冲/回位"演出状态机 (gObjActStep: 0 起手 → 0x12..0x18 两段位移 → 0x19 收尾 → 9 全体效果)。
 * 与 sub_802761C/sub_802F480 同族 (同样在尾部 sub_803F658 + 帧计数 gObjActStepTimer += 1)。
 * case0 记下 posX/posY/headA.f_1E/headA.palSlot, 经 sub_80444A4 清同组伤害 +
 * sub_801CE80(obj,5,0x1B4,0xB,0) 切动画 + sub_803F5B4 起手, 进入 0x12。
 * 0x12 等动画位 0x800 落; 0x13/0x16 在 frameIdx 到 0x39/0x64 时播 Sfx(0x1D) 并置 0x100
 * (本对象前后半步), 随后 0x14..0x18 用 sub_801768C 逐帧插值 posY (gObjActSavedY 为基准)
 * 或按 gObjActSavedX 偏移写 posX, 完成一趟出去再回来; 收尾 0x19 清 0x1000 并把暂存的
 * 位置/palSlot 交回 sub_801CE80 复位后回 9。9 态在无其他演出占用时返回 1 (动作完成)。 */
u8 sub_8027A20(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xB, 0);
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
            if (obj->headA.frameIdx == 0x39)
            {
                Sfx_Play(0x1D, 2, 0);
                obj->headA.kindFlags |= 0x100;
                gObjActStep = 0x14;
                gObjActStepTimer = 0;
            }
            break;
        case 20:
            if (gObjActStepTimer == 0xD)
                obj->headA.kindFlags |= 0x200;
            if (gObjActStepTimer <= 0x13)
            {
                obj->posY = sub_801768C(gObjActSavedY, -0x64, 0x14, gObjActStepTimer, 2);
            }
            else
            {
                obj->posX = 0xB4;
                gObjActStepTimer = 0;
                gObjActStep = 0x15;
            }
            break;
        case 21:
            if (gObjActStepTimer == 5)
                obj->headA.kindFlags &= 0xFDFF;
            if (gObjActStepTimer <= 0x13)
            {
                obj->posY = sub_801768C(gObjActSavedY - 0x64, 0x64, 0x14, gObjActStepTimer, 1);
            }
            else
            {
                gObjActStepTimer = 0;
                sub_8044514(0x14);
                obj->headA.kindFlags &= 0xFEFF;
                gObjActStep = 0x16;
            }
            break;
        case 22:
            if (obj->headA.frameIdx == 0x64)
            {
                gObjActStepTimer = 0;
                obj->headA.kindFlags |= 0x100;
                Sfx_Play(0x1D, 2, 0);
                gObjActStep = 0x17;
            }
            break;
        case 23:
            if (gObjActStepTimer == 0xD)
                obj->headA.kindFlags |= 0x200;
            if (gObjActStepTimer <= 0x13)
            {
                obj->posY = sub_801768C(gObjActSavedY, -0x64, 0x14, gObjActStepTimer, 2);
            }
            else
            {
                obj->posX = gObjActSavedX + 0xA;
                gObjActStepTimer = 0;
                gObjActStep = 0x18;
            }
            break;
        case 24:
            if (gObjActStepTimer == 5)
                obj->headA.kindFlags &= 0xFDFF;
            if (gObjActStepTimer <= 0x13)
            {
                obj->posY = sub_801768C(gObjActSavedY - 0x64, 0x64, 0x14, gObjActStepTimer, 1);
            }
            else
            {
                gObjActStepTimer = 0;
                obj->headA.kindFlags &= 0xFEFF;
                obj->posX = gObjActSavedX;
                obj->posY = gObjActSavedY;
                Sfx_Play(0x43, 2, 0);
                gObjActStep = 0x19;
            }
            break;
        case 25:
            if (obj->headA.kindFlags & 0x1000)
            {
                obj->headA.kindFlags &= 0xEFFF;
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                obj->posX = gObjActSavedX;
                obj->posY = gObjActSavedY;
                gObjActStep = 9;
            }
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x08027D9C
/* 战斗对象"绕行到伙伴身边"演出状态机 (gObjActStep: 0 起手 → 0x12..0x18 → 9)。
 * 双参 (obj, arg1=同池另一 BattleObj, 同 sub_80282EC)。与 sub_80282EC 的区别在于:
 * 插值结果写到 obj 的 headB 精灵滑动起点 headB.f_2B/f_2C (obj+0x67/0x68),
 * 由 headB 的装配器消费 (参见 ObjHead.f_2B/f_2C 语义), 实现本对象斜向绕到 arg1 身边。
 *   case0  暂存 posX/posY/headA.f_1E/headA.palSlot + sub_80444A4 清同组伤害 +
 *          sub_801CE80(obj,1,0x1B4,0xC,0) 切动画 + sub_803F5B4 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13   headA.frameIdx==0x20 → 按 (posX+0xA, posY-0x14) 锚点播 sub_8020CC4(0x300,0xE,0x2DA,5),
 *          置 headA 的 0x100, 计数=0 → 0x14。
 *   0x14   等 headB.kindFlags 的 0x800 落 → Sfx(0x31) → 0x15。
 *   0x15   计数<=9: 从 (posX+0xA, posY-0x14) 向 arg1 的 (posX, posY+0xC) 斜向插值
 *          headB.f_2B/f_2C; 到 10: 计数=0, sub_8044514(0x14) → 0x16。
 *   0x16   计数<=9: 从 arg1 的 (posX, posY-8) 向 (posX+0xA, posY-0xC) 反向斜向插值回位;
 *          到 10: 计数=0, 清 headA 的 0x100, 清 obj->state 的 0x2000 → 0x17。
 *   0x17   headA.kindFlags 到 0x1000 → sub_801CE80 复位 → 0x18。
 *   0x18   无其他演出占用时清 headA 的 0x100 → 9。
 *   9      返回 1 (无遮挡判定, 与姊妹变体不同)。
 * 尾部 sub_803F658(obj)。 */
u32 sub_8027D9C(BattleObj *obj, BattleObj *arg1)
{
    u32 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 1, 0x1B4, 0xC, 0);
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
            if (obj->headA.frameIdx != 0x20)
                break;
            sub_8020CC4(obj, obj->posX + 0xA, obj->posY - 0x14, 0x300, 0xE, 0x2DA, 5);
            obj->headA.kindFlags |= 0x100;
            gObjActStepTimer = 0;
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x31, 1, 0);
            gObjActStep = 0x15;
            break;
        case 21:
            if (gObjActStepTimer <= 9)
            {
                obj->headB.f_2B = sub_801768C(obj->posX + 0xA, arg1->posX - (obj->posX + 0xA), 0xA, gObjActStepTimer, 2);
                obj->headB.f_2C = sub_801768C(obj->posY - 0x14, (arg1->posY + 0xC) - obj->posY, 0xA, gObjActStepTimer, 2);
            }
            else
            {
                gObjActStepTimer = 0;
                sub_8044514(0x14);
                gObjActStep = 0x16;
            }
            break;
        case 22:
            if (gObjActStepTimer <= 9)
            {
                obj->headB.f_2B = sub_801768C(arg1->posX, (obj->posX + 0xA) - arg1->posX, 0xA, gObjActStepTimer, 1);
                obj->headB.f_2C = sub_801768C(arg1->posY - 8, (obj->posY - 0xC) - arg1->posY, 0xA, gObjActStepTimer, 1);
            }
            else
            {
                gObjActStepTimer = 0;
                obj->headA.kindFlags &= 0xFEFF;
                obj->state &= 0xDFFF;
                gObjActStep = 0x17;
            }
            break;
        case 23:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x18;
            break;
        case 24:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                obj->headA.kindFlags &= 0xFEFF;
                gObjActStep = 9;
            }
            break;
        case 9:
            result = 1;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x08028098
/* 战斗对象"前冲/后撤"演出状态机 (gObjActStep: 0 起手 → 0x12..0x16 → 9 全体效果)。
 * 与 sub_802761C/sub_802F480/sub_8027A20 同族, 但多了第二动画头 (headB) 的相位:
 *   case0  记 posX/posY/headA.f_1E/headA.palSlot, sub_80444A4 清同组伤害 +
 *          sub_801CE80(obj,5,0x1B4,0xC,0) 切 A 动画 + sub_803F5B4 起手 → 0x12。
 *   0x12   headA 分片装载到位 (gfxPos >= gfxTotal-1) → 0x13。
 *   0x13   等 headA.kindFlags 的 0x800 落 → 0x14。
 *   0x14   headA.frameIdx > 0x67 时按 (posX+0x40, posY-0x20) 播 sub_8020CC4(0x300,0xE,0x2DC,5),
 *          清 headB.f_2A 与帧计数 → 0x15。
 *   0x15   等 headB.kindFlags 的 0x800 落 → Sfx(0x19) → 0x16。
 *   0x16   headB.frameIdx==0x2C 时补 Sfx(0x5A); 帧计数到 0x6E 播 sub_8044514(0x37),
 *          到 0x73 回 9。
 *   9     无其他演出占用时返回 1。
 * 尾部: sub_803F658 后, headA.kindFlags 的 0x1000 (A 动画完成) 清位并交回暂存参数复位;
 * headB.kindFlags 的 0x1000 (B 动画完成) 清位并清 obj->state 的 0x2000 (跳跃位) 收尾。 */
u8 sub_8028098(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xC, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.gfxPos < obj->headA.gfxTotal - 1)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headA.frameIdx > 0x67)
            {
                sub_8020CC4(obj, obj->posX + 0x40, obj->posY - 0x20, 0x300, 0xE, 0x2DC, 5);
                obj->headB.f_2A = 0;
                gObjActStepTimer = 0;
                gObjActStep = 0x15;
            }
            break;
        case 21:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x19, 1, 0);
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headB.frameIdx == 0x2C)
                Sfx_Play(0x5A, 1, 0);
            gObjActStepTimer += 1;
            if (gObjActStepTimer == 0x6E)
                sub_8044514(0x37);
            else if (gObjActStepTimer == 0x73)
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
    if (obj->headB.kindFlags & 0x1000)
    {
        obj->headB.kindFlags &= 0xEFFF;
        obj->state &= 0xDFFF;
    }
    return result;
}
// @ 0x080282EC
/* 战斗对象"伙伴换位/支援"演出状态机 (gObjActStep: 0 起手 → 0x12..0x19 → 9)。
 * 与 sub_8028098 同族, 但额外接收第二个 BattleObj* (参1, 由 sub_803F444 分派器传入同池
 * 另一对象, 见 0x0801BFD8 调用点: arg1 = 池基址 + obj->f_BD*0xC8)。位置插值以 arg1->posY
 * 为参照, 让本对象从/向参1 的位置滑动 (换位/归位)。
 *   case0  记 posX/posY/headA.f_1E/headA.palSlot, sub_80444A4 清同组伤害 + sub_803F5B4 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13   计数 <= 9 时按 (arg1->posY+0x10) 与暂存 posY 的差逐帧插值 obj->posY; 到 10 → 0x14。
 *   0x14   sub_801CE80(obj,5,0x1B4,0xC,1) → 0x15。
 *   0x15   headA 装载到位 (gfxPos >= gfxTotal-1) → obj->posX += 0x47, 计数=0 → 0x16。
 *   0x16   headA.frameIdx 命中 0x19/0x6B/0xD1 时分别 Sfx(0x19)/(0x5A), 到 0xD1 播
 *          sub_8044514(0x1E) 并 → 0x17; 之后帧计数 += 1。
 *   0x17   headA.kindFlags 到 0x1000 → 置 0x100 并 → 0x18; 帧计数 += 1。
 *   0x18   等参1 headB.kindFlags 的 0x800 落 → 清 headA 的 0x1100, sub_801CE80 复位,
 *          obj->posX -= 0x47 (退回原点), → 0x19。
 *   0x19   计数 <= 9 时按暂存 posY 与 arg1->posY 的差反向插值回位; 到 10 则 posY 还原 → 9。
 *   9      无其他演出占用时返回 1。
 * 尾部 sub_803F658(obj) 收尾。与 sub_8028098 区别: 单参变双参, 位置围绕参1 换位。 */
u8 sub_80282EC(BattleObj *obj, BattleObj *arg1)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
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
            if (gObjActStepTimer <= 9)
            {
                obj->posY = sub_801768C(gObjActSavedY, (arg1->posY + 0x10) - gObjActSavedY, 0xA, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStep = 0x14;
            }
            break;
        case 20:
            sub_801CE80(obj, 5, 0x1B4, 0xC, 1);
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headA.gfxPos < obj->headA.gfxTotal - 1)
                break;
            obj->posX += 0x47;
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headA.frameIdx == 0x19)
                Sfx_Play(0x19, 1, 0);
            if (obj->headA.frameIdx == 0x6B)
                Sfx_Play(0x5A, 1, 0);
            if (obj->headA.frameIdx == 0xD1)
            {
                sub_8044514(0x1E);
                gObjActStep = 0x17;
            }
            gObjActStepTimer += 1;
            break;
        case 23:
            if (obj->headA.kindFlags & 0x1000)
            {
                obj->headA.kindFlags |= 0x100;
                gObjActStep = 0x18;
            }
            gObjActStepTimer += 1;
            break;
        case 24:
        {
            u16 f = arg1->headB.kindFlags & 0x800;
            if (f != 0)
                break;
            obj->headA.kindFlags &= 0xEEFF;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, f);
            obj->posX -= 0x47;
            gObjActStep = 0x19;
            gObjActStepTimer = 0;
            break;
        }
        case 25:
            if (obj->headA.kindFlags & 0x800)
                break;
            if (gObjActStepTimer <= 9)
            {
                obj->posY = sub_801768C(arg1->posY + 0x10, gObjActSavedY - (arg1->posY + 0x10), 0xA, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->posY = gObjActSavedY;
                gObjActStep = 9;
            }
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x080285A0
/* 战斗对象"归位/后撤到目标"演出状态机 (gObjActStep: 0 起手 → 0x12..0x17 → 9)。
 * 与 sub_8027760 同族双参 (参1 = 同池目标对象), 但用 sub_804B834/sub_804B8E8 做槽位恢复。
 *   case0  记 posX/posY/headA.f_1E/headA.palSlot, sub_80444A4 清同组伤害 +
 *          sub_801CE80(obj,1,0x1B4,0xD,0) 切动画 + sub_803F5B4 起手 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → sub_804B834(0xD,1,3,-0xB,5) + Sfx(0x31) → 0x13。
 *   0x13   headA.frameIdx <= 0x2E 时用 sub_801768C 把 posX/posY 向
 *          (obj2->posX-0x30)/(obj2->posY) 按 0x2F 帧插值; 超过 → 0x14。
 *   0x14   headA.frameIdx > 0x4F → sub_8044514(0x14) → 0x15。
 *   0x15   headA.kindFlags 到 0x1000 → sub_804B8E8(0xD,1) + sub_801CE80 复位 +
 *          posX/posY 还原暂存 → 0x16。
 *   0x16   等 headA.kindFlags 的 0x800 落 → 0x17。
 *   0x17   无其他演出占用时 → 9。
 *   9      返回 1。
 * 尾部: gObjActStepTimer += 1; sub_803F658(obj)。 */
u8 sub_80285A0(BattleObj *obj, BattleObj *obj2)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 1, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(0xD, 1, 3, -0xB, 5);
            Sfx_Play(0x31, 1, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx <= 0x2E)
            {
                obj->posX = sub_801768C(gObjActSavedX, (obj2->posX - 0x30) - gObjActSavedX, 0x2F, (s16)obj->headA.frameIdx, 2);
                obj->posY = sub_801768C(gObjActSavedY, obj2->posY - gObjActSavedY, 0x2F, (s16)obj->headA.frameIdx, 2);
            }
            else
            {
                gObjActStep = 0x14;
            }
            break;
        case 20:
            if (obj->headA.frameIdx <= 0x4F)
                break;
            sub_8044514(0x14);
            gObjActStep = 0x15;
            break;
        case 21:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            sub_804B8E8(0xD, 1);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            obj->posX = gObjActSavedX;
            obj->posY = gObjActSavedY;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x17;
            break;
        case 23:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                gObjActStep = 9;
            break;
        case 9:
            result = 1;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x080287EC
/* 战斗对象"双段归位/重摆位"演出状态机 (gObjActStep: 0 起手 → 0x12..0x19 → 9)。
 * 单参, 用 sub_804B834/sub_804B8E8 (OBJ 槽位 setter/clearer, 首参为 headA.palSlot) 做两轮
 * 槽位重建, 中间经 sub_8020CC4 播一次全屏锚点动画。
 *   case0  暂存 posX/posY/headA.f_1E/headA.palSlot + sub_80444A4 清同组伤害 +
 *          sub_801CE80(obj,5,0x1B4,0xD,0) 切动画 + sub_803F5B4 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 清 palSlot 槽 (sub_804B834(palSlot,1,3,-0xB,5)) → 0x13。
 *   0x13   headA.frameIdx==0x32 时 Sfx(0x64); frameIdx>0x4F 时清槽 sub_804B8E8(palSlot,1) +
 *          sub_801CE80 复位 → 0x14。
 *   0x14   等 headA.kindFlags 的 0x800 落 → 再次 sub_804B834(palSlot,1,3,-0xB,5) → 0x15。
 *   0x15   sub_8020CC4(obj, 0xB4, 0x78, 0x1E6, 0xD, 0x34A, 2) 播锚点动画, 计数=0 → 0x16。
 *   0x16   等 headB.kindFlags 的 0x800 落 → Sfx(0x64) → 0x17。
 *   0x17   headB.kindFlags 到 0x1000 → 清 0x1000 + 清 obj->state 的 0x2000, 计数=0 → 0x18。
 *   0x18   计数非 0 → sub_8044514(0x14) → 0x19。
 *   0x19   无其他演出占用 → sub_801CE80 复位 → 9。
 *   9      等 headA.kindFlags 的 0x800 落 → 再次清槽 sub_804B834(palSlot,...), 返回 1。
 * 尾部 gObjActStepTimer += 1 + sub_803F658(obj)。 */
u8 sub_80287EC(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx == 0x32)
                Sfx_Play(0x64, 1, 0);
            if (obj->headA.frameIdx <= 0x4F)
                break;
            sub_804B8E8(obj->headA.palSlot, 1);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            gObjActStep = 0x15;
            break;
        case 21:
            sub_8020CC4(obj, 0xB4, 0x78, 0x1E6, 0xD, 0x34A, 2);
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x64, 1, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            obj->headB.kindFlags &= 0xEFFF;
            obj->state &= 0xDFFF;
            gObjActStepTimer = 0;
            gObjActStep = 0x18;
            break;
        case 24:
            if (gObjActStepTimer == 0)
                break;
            sub_8044514(0x14);
            gObjActStep = 0x19;
            break;
        case 25:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 9;
            }
            break;
        case 9:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            result = 1;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x08028AD8
INCLUDE_ASM("asm/nonmatchings", sub_8028AD8);
// @ 0x08029268
/* 战斗对象"登场/移动到目标"演出状态机 (gObjActStep: 0 起手 → 1..9)。
 * 与 sub_80285A0/sub_8027760 同族双参 (参1 = 同池目标对象), 但用 headB.f_2B/f_2C 插值。
 *   case0  仅记 palSlot/f_1E/posX/posY 暂存 → 1 (不清伤害、不切动画)。
 *   case1  arg1->dmgAmount = 0, sub_80444A4 清同组伤害 + sub_803F5B4 +
 *          sub_801CE80(obj,1,0x1B4,0xD,0) 切动画 → 2。
 *   case2  等 headA.kindFlags 的 0x800 落 → Sfx(0x31) → 5。
 *   case5  headA.kindFlags 到 0x1000 → 清 0x1000 置 0x100, 按 (posX, posY-0x10) 播
 *          sub_8020CC4(0x1F4,0xE,0x8D,0x25) + Sfx(0x34), 帧计数=0 → 3。
 *   case3  帧计数 <= 9 时插值 headB.f_2B/f_2C 向 arg1 (用 sub_801EC3C(arg1,0/1) 半量偏移);
 *          超过则清零 + sub_8044514(0x14) + obj->state &= 0xDFFF → 6。
 *   case6  无其他演出占用时 sub_8020974(&headA,0x8C,0x1B4,0xD,0x22) → 8。
 *   case8  → 7。
 *   case7  headA.kindFlags 到 0x1000 → sub_80207DC(obj, 暂存值) → 9。
 *   case9  headA.kindFlags 的 0x800 落 → result = 1。
 * 尾部: sub_803F658(obj) (本函数无帧计数自增)。 */
u8 sub_8029268(BattleObj *obj, BattleObj *arg1)
{
    u8 result;
    u32 zero;
    u16 keys;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActStep = 1;
            break;
        case 1:
            arg1->dmgAmount = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            sub_801CE80(obj, 1, 0x1B4, 0xD, 0);
            gObjActStepTimer = 0;
            gObjActStep = 2;
            break;
        case 2:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x31, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            keys = obj->headA.kindFlags & 0xEFFF;
            zero = 0;
            obj->headA.kindFlags = keys;
            obj->headA.kindFlags = obj->headA.kindFlags | 0x100;
            sub_8020CC4(obj, obj->posX, (u8)(obj->posY - 0x10), 0x1F4, 0xE, 0x8D, 0x25);
            gObjActStepTimer = zero;
            Sfx_Play(0x34, 1, 0);
            gObjActStep = 3;
            break;
        case 3:
            if (gObjActStepTimer <= 9)
            {
                obj->headB.f_2B = sub_801768C(obj->posX,
                    (arg1->posX + ((u8)sub_801EC3C(arg1, 0) >> 1)) - obj->posX,
                    0xA, gObjActStepTimer, 2);
                obj->headB.f_2C = sub_801768C(obj->posY - 0x10,
                    (arg1->posY - ((u8)sub_801EC3C(arg1, 1) >> 1)) - (obj->posY - 0x10),
                    0xA, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                sub_8044514(0x14);
                obj->state &= 0xDFFF;
                gObjActStep = 6;
            }
            break;
        case 6:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_8020974(&obj->headA, 0x8C, 0x1B4, 0xD, 0x22);
                gObjActStep = 8;
            }
            break;
        case 8:
            gObjActStep = 7;
            break;
        case 7:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_80207DC(obj, gObjActSavedX, gObjActSavedY, gObjActSavedF2A, gObjActSavedPal);
                gObjActStep = 9;
            }
            break;
        case 9:
            if (!(obj->headA.kindFlags & 0x800))
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x08029510
/* 战斗对象"强化/特效"演出状态机 (gObjActStep: 0 起手 → 1..9)。
 * 单参 (obj), 用 sub_804B834/sub_804B8E8 做槽位恢复; 开局无 Sfx。
 *   case0  记 posX/posY/headA.f_1E/headA.palSlot, sub_80444A4 清同组伤害 +
 *          sub_801CE80(obj,5,0x1B4,0xD,0) 切动画 + sub_803F5B4 起手 → 1。
 *   case1  等 headA.kindFlags 的 0x800 落 → sub_804B834(palSlot,1,3,-0xB,5) → 3。
 *   case3  headA.frameIdx > 0x4F → sub_804B8E8(palSlot,1) +
 *          sub_801CE80(obj,0,0822,0824,0) → 4。
 *   case4  等 headA.kindFlags 的 0x800 落 → sub_804B834(palSlot,1,3,-0xB,5) → 2。
 *   case2  按 (0xB4,0x78) 播 sub_8020CC4(0x1E6,0xD,0x34A,2), 帧计数=0 → 5。
 *   case5  等 headB.kindFlags 到 0x1000 → 清 0x1000, obj->state &= 0xDFFF, 计数=0 → 6。
 *   case6  计数 != 0 → sub_8044514(0x14) → 7。
 *   case7  无其他演出占用时 sub_801CE80(obj,0,0822,0824,0) → 9。
 *   case9  等 headA.kindFlags 的 0x800 落 → sub_804B834(palSlot,1,3,-0xB,5), result = 1。
 * 尾部: gObjActStepTimer += 1; sub_803F658(obj)。 */
u8 sub_8029510(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 1;
            break;
        case 1:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            gObjActStep = 3;
            break;
        case 3:
            if (obj->headA.frameIdx <= 0x4F)
                break;
            sub_804B8E8(obj->headA.palSlot, 1);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 4;
            break;
        case 4:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            gObjActStep = 2;
            break;
        case 2:
            sub_8020CC4(obj, 0xB4, 0x78, 0x1E6, 0xD, 0x34A, 2);
            gObjActStepTimer = 0;
            gObjActStep = 5;
            break;
        case 5:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            obj->headB.kindFlags &= 0xEFFF;
            obj->state &= 0xDFFF;
            gObjActStepTimer = 0;
            gObjActStep = 6;
            break;
        case 6:
            if (gObjActStepTimer == 0)
                break;
            sub_8044514(0x14);
            gObjActStep = 7;
            break;
        case 7:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 9;
            }
            break;
        case 8:
            break;
        case 9:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            result = 1;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x08029784
/* 战斗对象"强化/特效"演出状态机 (变体; gObjActStep: 0 起手 → 1..9)。
 * 与 sub_8029510 同族单参, 但 case0 的 sub_801CE80 末参 = 1, 且 case1 为空 (直接等下一状态),
 * 因此实际序列为 0 → 3 → 4 → 2 → 5..9。
 *   case0  记 posX/posY/f_1E/palSlot, sub_80444A4 + sub_801CE80(obj,5,0x1B4,0xD,1) +
 *          sub_803F5B4, 计数=0, 状态=1。
 *   case3  headA.frameIdx > 0x4F → sub_804B8E8(palSlot,1) + CE80(obj,0,0822,0824,0) → 4。
 *   case4  等 headA.kindFlags 的 0x800 落 → sub_804B834(palSlot,1,3,-0xB,5) → 2。
 *   case2  按 (0xB4,0x78) 播 sub_8020CC4(0x1E6,0xD,0x34A,2), 计数=0 → 5。
 *   case5  等 headB.kindFlags 到 0x1000 → 清 0x1000, state &= 0xDFFF, 计数=0 → 6。
 *   case6  计数 != 0 → sub_8044514(0x14) → 7。
 *   case7  无其他演出占用时 CE80(obj,0,0822,0824,0) → 9。
 *   case9  等 headA.kindFlags 的 0x800 落 → sub_804B834(palSlot,1,3,-0xB,5), result = 1。
 * 尾部: gObjActStepTimer += 1; sub_803F658(obj)。 */
u8 sub_8029784(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 1);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 1;
            break;
        case 1:
            break;
        case 3:
            if (obj->headA.frameIdx <= 0x4F)
                break;
            sub_804B8E8(obj->headA.palSlot, 1);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 4;
            break;
        case 4:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            gObjActStep = 2;
            break;
        case 2:
            sub_8020CC4(obj, 0xB4, 0x78, 0x1E6, 0xD, 0x34A, 2);
            gObjActStepTimer = 0;
            gObjActStep = 5;
            break;
        case 5:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            obj->headB.kindFlags &= 0xEFFF;
            obj->state &= 0xDFFF;
            gObjActStepTimer = 0;
            gObjActStep = 6;
            break;
        case 6:
            if (gObjActStepTimer == 0)
                break;
            sub_8044514(0x14);
            gObjActStep = 7;
            break;
        case 7:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 9;
            }
            break;
        case 8:
            break;
        case 9:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            result = 1;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x080299C8
/* 战斗对象"归位到目标"演出状态机 (双参; gObjActStep: 0 起手 → 1..6 → 9)。
 * 与 sub_80285A0 同族但流程更短, 用 sub_804B834/sub_804B8E8 做 palSlot 槽位恢复。
 *   case0  记 posX/posY/headA.f_1E/headA.palSlot, sub_80444A4 清同组伤害 +
 *          sub_801CE80(obj,1,0x1B4,0xD,0) 切动画 + sub_803F5B4 起手, 计数=0 → 1。
 *   case1  等 headA.kindFlags 的 0x800 落 → sub_804B834(palSlot,1,3,-0xB,5) → 2。
 *   case2  headA.frameIdx <= 0x2E 时用 sub_801768C 把 posX/posY 向
 *          (arg1->posX-0x30)/(arg1->posY) 按 0x2F 帧插值; 超过 → 3。
 *   case3  headA.frameIdx > 0x4F → sub_8044514(0x14) → 4。
 *   case4  headA.kindFlags 到 0x1000 → sub_804B8E8(palSlot,1) + sub_801CE80 复位 +
 *          posX/posY 还原暂存 → 5。
 *   case5  等 headA.kindFlags 的 0x800 落 → sub_804B834(palSlot,1,3,-0xB,5) → 6。
 *   case6  无其他演出占用 (gActWaitBusy0/45/56 全 0) → 9。
 *   9      返回 1。
 * 尾部: gObjActStepTimer += 1; sub_803F658(obj)。 (无 sub_80187E8) */
u8 sub_80299C8(BattleObj *obj, BattleObj *arg1)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 1, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            gObjActStep = 1;
            break;
        case 1:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            gObjActStep = 2;
            break;
        case 2:
            if (obj->headA.frameIdx <= 0x2E)
            {
                obj->posX = sub_801768C(gObjActSavedX, (arg1->posX - 0x30) - gObjActSavedX, 0x2F, (s16)obj->headA.frameIdx, 2);
                obj->posY = sub_801768C(gObjActSavedY, arg1->posY - gObjActSavedY, 0x2F, (s16)obj->headA.frameIdx, 2);
            }
            else
            {
                gObjActStep = 3;
            }
            break;
        case 3:
            if (obj->headA.frameIdx <= 0x4F)
                break;
            sub_8044514(0x14);
            gObjActStep = 4;
            break;
        case 4:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            sub_804B8E8(obj->headA.palSlot, 1);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            obj->posX = gObjActSavedX;
            obj->posY = gObjActSavedY;
            gObjActStep = 5;
            break;
        case 5:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_804B834(obj->headA.palSlot, 1, 3, -0xB, 5);
            gObjActStep = 6;
            break;
        case 6:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                gObjActStep = 9;
            break;
        case 9:
            result = 1;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x08029BF8
u8 sub_8029BF8(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            obj->f_B6 = 0xAC;
            obj->f_B4 = 0;
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->headA.kindFlags |= 0x10;
            sub_801A2AC(0x410, 0x10, 0);
            gObjActStep = 0x12;
            break;
        case 18:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x410, gSceneFadeOut, (u8)(0x10 - gSceneFadeOut));
                gObjActStepTimer += 1;
                break;
            }
            obj->headA.kindFlags |= 0x200;
            gObjActStep = 0x13;
            break;
        case 19:
            sub_801A348();
            ((void (*)(u8, int, int))sub_801A3A8)(1, -0x23, -5);
            gSceneFadeOut = 0;
            sub_801A2AC(0x1747, 0, 0x10);
            gObjActStep = 0x14;
            break;
        case 20:
            if (sub_8019B98(0xF, 3, 0xE, 1) == 0)
                break;
            gObjActStepTimer = 0;
            sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
            Sfx_Play(0x20, 0, 0);
            gObjActStep = 0x15;
            break;
        case 21:
            if (gObjActStepTimer <= 0x13)
            {
                if (gObjActStepTimer == 4)
                    sub_804C728(0, 3, 0x10);
                gSceneFadeOut = sub_801768C(0, 0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x1747, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 0x17;
            break;
        case 23:
            if (gObjActStepTimer <= 0x13)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_8020CC4(obj, 0x78, 0x69, 0x1B4, 0xE, 0x2E7, 0x2404);
            obj->headB.f_2A = 0;
            gObjActStep = 0x18;
            break;
        case 24:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x64, 0, 0);
            gObjActStep = 0x19;
            break;
        case 25:
            if (obj->headB.frameIdx <= 0x1D)
                break;
            obj->headB.kindFlags |= 0x100;
            obj->headB.frameIdx += 1;
            gObjActStep = 0x1C;
            break;
        case 28:
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 6 + 0x1E;
            sub_804BF14(0, 3, 0x1F, 0x14, 0xE, 4, 4, -1, 2);
            sub_804BF14(0xE, 1, 0x1F, 0x14, 0xE, 4, 4, -1, 2);
            sub_804B96C(1, 5, 0x1F, 0x14, 0xE, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            gObjActStep = 0x1D;
            break;
        case 29:
            if (gObjActStepTimer <= 0x13)
            {
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gObjActStep = 0x1E;
            }
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 6 + 0x1E;
            break;
        case 30:
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 6 + 0x1E;
            if (gObjActStepTimer <= 7)
            {
                gObjActStepTimer += 1;
                break;
            }
            BattleUiFlag_Set(0x20);
            obj->state &= 0xDFFF;
            sub_804C728(0, 3, 0x20);
            sub_804C728(0xE, 1, 0x20);
            sub_804C4D8(1, 5, 0x20);
            gObjActStepTimer = 0;
            gObjActStep = 0x1F;
            break;
        case 31:
            if (gObjActStepTimer <= 0x1F)
            {
                gObjActStepTimer += 1;
                break;
            }
            obj->headA.kindFlags &= 0xFDFF;
            sub_801A2AC(0x410, 0, 0x10);
            gObjActStepTimer = 0;
            gObjActStep = 0x20;
            break;
        case 32:
            if (gObjActStepTimer <= 0x1F)
            {
                gSceneFadeOut = sub_801768C(0, 0x10, 0x20, gObjActStepTimer, 2);
                sub_801A2AC(0x410, gSceneFadeOut, (u8)(0x10 - gSceneFadeOut));
                gObjActStepTimer += 1;
                break;
            }
            obj->headA.kindFlags &= 0xFFEF;
            sub_801A2AC(0, 0, 0);
            sub_8044514(0x14);
            gObjActStep = 0x21;
            break;
        case 33:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                gObjActStep = 9;
            break;
        case 9:
            sub_801A3A8(1, 0, 0);
            result = 1;
            break;
        case 26:
        case 27:
        case 34:
        case 35:
        case 36:
        case 37:
            break;
    }
    sub_803F658(obj);
    return result;
}

// @ 0x0802A154
/* 战斗对象"文字框/换装长演出"状态机 (gObjActStep: 0 起手 → 0x12..0x21 → 0x23/0x24 → 9)。
 * 单参; 唯一入口 0x0839CD5C 指针表 idx 0x1A (thumb 0x0802A155)。
 * 用 headA 主头 + sub_801A2AC 文字框/图层 + sub_801768C 插值 gSceneFadeOut/68 双窗口,
 * 中段 sub_804BF14/sub_804B96C/sub_804C728/sub_804C4D8 做装备层切换, 末尾按
 * gUnk_030008A5 (0..2) 从 gUnk_0839DF90 取 (x,y) 对循环。
 *   case0   记 posX/posY/f_1E/palSlot, sub_80444A4; f_B6=1, f_B4=0, sub_803F5B4,
 *           计数=0, headA.kindFlags |= 0x10, sub_801A2AC(0x410,0x10,0), g8A5=0 → 0x12。
 *   0x12    计数<=9: g867 = sub_801768C(0x10,-0x10,0xA,计数,2);
 *           sub_801A2AC(0x410, g867, (u8)(0x10-g867)); 计数++;
 *           否则 headA.kindFlags |= 0x200 → 0x13。
 *   0x13    sub_801A348 + sub_801A3A8(1,-0x23,-5) + g867=0 + sub_801A2AC(0x1747,0,0x10) → 0x14。
 *   0x14    sub_8019B98(0xF,3,0xE,2)!=0 → 计数=0 + sub_804BF14(0,3,7,0xE,0x1C,4,4,-1,2) +
 *           Sfx(0x20,0,0) → 0x15。
 *   0x15    计数<=0x13: 计数==4 时 sub_804C728(0,3,0x10);
 *           g867 = sub_801768C(0,0x10,0x14,计数,2); sub_801A2AC(0x1747,g867,0x10); 计数++;
 *           否则 计数=0 → 0x16。
 *   0x16    计数<=9: g868 = sub_801768C(0x10,-0x10,0xA,计数,2);
 *           sub_801A2AC(0x1747,g867,g868); 计数++; 否则 计数=0 → 0x17。
 *   0x17    sub_804BF14(0,3,7,0xE,0x1C,4,4,-1,2) + sub_804BF14(0xE,1,7,0xE,0x1C,4,4,-1,2) +
 *           sub_804B96C(1,5,7,0xE,0x1C,4,4,-1,2); 计数=0 → 0x18。
 *   0x18    计数<=7 → 计数++; 否则 sub_8020CC4(obj,0x73,0x73,0x1B4,0xE,0x2ED,5) +
 *           headB.f_2A=2 + sub_804C728(0,3,0x20) + sub_804C728(0xE,1,0x20) +
 *           sub_804C4D8(1,5,0x20) → 0x19。
 *   0x19    headB 的 0x800 落 → Sfx(0x68,1,0) + 计数=0 → 0x1A。
 *   0x1A    计数<=0x1F → 计数++; 否则 sub_804BBDC(0,3,7,0xE,0x1C,4,4,2) → 0x1B。
 *   0x1B    Sfx_TrackBusy(1)==0 → state&=0xDFFF + sub_804BD54(0,3) + 计数=0 → 0x1C。
 *   0x1C    计数<=9: g868 = sub_801768C(0,0x10,0xA,计数,2);
 *           sub_801A2AC(0x1747,(u8)(0x10-g868),g868); 计数++;
 *           否则 计数=0 + BattleUiFlag_Set(0x20) → 0x1D。
 *   0x1D    headA.kindFlags &= 0xFDFF + sub_801A2AC(0x410,0,0x10) + 计数=0 → 0x1E。
 *   0x1E    计数<=0x1F: g867 = sub_801768C(0,0x10,0x20,计数,2);
 *           sub_801A2AC(0x410,g867,(u8)(0x10-g867)); 计数++;
 *           否则 清 headA 0xFFEF + sub_801A2AC(0,0,0) +
 *           sub_8020CC4(obj, gUnk_0839DF90[g8A5*2], gUnk_0839DF90[g8A5*2+1], 0x1B4,0xD,0x2EE,2) +
 *           headB.f_2A=0 → 0x1F。
 *   0x1F    headB 的 0x800 落 → 0x20。
 *   0x20    headB.frameIdx > 0xC → sub_804BF14(0,3,0x10,0x1C,0x1F,4,4,-1,2) + 计数=0 → 0x21。
 *   0x21    计数>3 → sub_804C728(0,3,0x10) + 计数=0 + Sfx(0x68,1,0), g8A5==0 ? 0x22 : 0x23;
 *           计数++。
 *   0x22    计数>0xF → sub_8044514(0x5A) + 0x23; headB 到 0x1000 → state&=0xDFFF + 0x23; 计数++。
 *   0x23    headB 到 0x1000 → 清 headB 0x1000 + state&=0xDFFF → 0x24。
 *   0x24    g8A5<=1: g8A5++ + headB.frameIdx=0 + state|=0x2000 +
 *           headB.f_2B/f_2C = gUnk_0839DF90[g8A5*2]/[+1] → 0x20; 否则空闲 → 9。
 *   9       空闲 → sub_801A3A8(1,0,0) + result=1。
 * 尾部 sub_803F658(obj)。 */
u32 sub_802A154(BattleObj *obj)
{
    u32 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            obj->f_B6 = 1;
            obj->f_B4 = 0;
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->headA.kindFlags |= 0x10;
            sub_801A2AC(0x410, 0x10, 0);
            gUnk_030008A5 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x410, gSceneFadeOut, (u8)(0x10 - gSceneFadeOut));
                gObjActStepTimer += 1;
            }
            else
            {
                obj->headA.kindFlags |= 0x200;
                gObjActStep = 0x13;
            }
            break;
        case 19:
            sub_801A348();
            ((void (*)(u8, int, int))sub_801A3A8)(1, -0x23, -5);
            gSceneFadeOut = 0;
            sub_801A2AC(0x1747, 0, 0x10);
            gObjActStep = 0x14;
            break;
        case 20:
            if (sub_8019B98(0xF, 3, 0xE, 2) != 0)
            {
                gObjActStepTimer = 0;
                sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
                Sfx_Play(0x20, 0, 0);
                gObjActStep = 0x15;
            }
            break;
        case 21:
            if (gObjActStepTimer <= 0x13)
            {
                if (gObjActStepTimer == 4)
                    sub_804C728(0, 3, 0x10);
                gSceneFadeOut = sub_801768C(0, 0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x1747, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gObjActStep = 0x16;
            }
            break;
        case 22:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gObjActStep = 0x17;
            }
            break;
        case 23:
            sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
            sub_804BF14(0xE, 1, 7, 0xE, 0x1C, 4, 4, -1, 2);
            sub_804B96C(1, 5, 7, 0xE, 0x1C, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            gObjActStep = 0x18;
            break;
        case 24:
            if (gObjActStepTimer <= 7)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_8020CC4(obj, 0x73, 0x73, 0x1B4, 0xE, 0x2ED, 5);
            obj->headB.f_2A = 2;
            sub_804C728(0, 3, 0x20);
            sub_804C728(0xE, 1, 0x20);
            sub_804C4D8(1, 5, 0x20);
            gObjActStep = 0x19;
            break;
        case 25:
            if (!(obj->headB.kindFlags & 0x800))
            {
                Sfx_Play(0x68, 1, 0);
                gObjActStepTimer = 0;
                gObjActStep = 0x1A;
            }
            break;
        case 26:
            if (gObjActStepTimer <= 0x1F)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_804BBDC(0, 3, 7, 0xE, 0x1C, 4, 4, 2);
            gObjActStep = 0x1B;
            break;
        case 27:
            if (Sfx_TrackBusy(1) != 0)
                break;
            obj->state &= 0xDFFF;
            sub_804BD54(0, 3);
            gObjActStepTimer = 0;
            gObjActStep = 0x1C;
            break;
        case 28:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x1747, (u8)(0x10 - gSceneFadeIn), gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                BattleUiFlag_Set(0x20);
                gObjActStep = 0x1D;
            }
            break;
        case 29:
            obj->headA.kindFlags &= 0xFDFF;
            sub_801A2AC(0x410, 0, 0x10);
            gObjActStepTimer = 0;
            gObjActStep = 0x1E;
            break;
        case 30:
            if (gObjActStepTimer <= 0x1F)
            {
                gSceneFadeOut = sub_801768C(0, 0x10, 0x20, gObjActStepTimer, 2);
                sub_801A2AC(0x410, gSceneFadeOut, (u8)(0x10 - gSceneFadeOut));
                gObjActStepTimer += 1;
            }
            else
            {
                obj->headA.kindFlags &= 0xFFEF;
                sub_801A2AC(0, 0, 0);
                sub_8020CC4(obj, gUnk_0839DF90[gUnk_030008A5 * 2], (gUnk_0839DF90 + 1)[gUnk_030008A5 * 2], 0x1B4, 0xD, 0x2EE, 2);
                obj->headB.f_2A = 0;
                gObjActStep = 0x1F;
            }
            break;
        case 31:
            if (!(obj->headB.kindFlags & 0x800))
                gObjActStep = 0x20;
            break;
        case 32:
            if (obj->headB.frameIdx <= 0xC)
                break;
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            gObjActStep = 0x21;
            break;
        case 33:
            if (gObjActStepTimer > 3)
            {
                sub_804C728(0, 3, 0x10);
                gObjActStepTimer = 0;
                Sfx_Play(0x68, 1, 0);
                if (gUnk_030008A5 == 0)
                    gObjActStep = 0x22;
                else
                    gObjActStep = 0x23;
            }
            gObjActStepTimer += 1;
            break;
        case 34:
            if (gObjActStepTimer > 0xF)
            {
                sub_8044514(0x5A);
                gObjActStep = 0x23;
            }
            if (obj->headB.kindFlags & 0x1000)
            {
                obj->state &= 0xDFFF;
                gObjActStep = 0x23;
            }
            gObjActStepTimer += 1;
            break;
        case 35:
            if (obj->headB.kindFlags & 0x1000)
            {
                obj->headB.kindFlags &= 0xEFFF;
                obj->state &= 0xDFFF;
                gObjActStep = 0x24;
            }
            break;
        case 36:
            if (gUnk_030008A5 <= 1)
            {
                gUnk_030008A5 += 1;
                obj->headB.frameIdx = 0;
                obj->state |= 0x2000;
                obj->headB.f_2B = gUnk_0839DF90[gUnk_030008A5 * 2];
                obj->headB.f_2C = (gUnk_0839DF90 + 1)[gUnk_030008A5 * 2];
                gObjActStep = 0x20;
            }
            else if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                gObjActStep = 9;
            }
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801A3A8(1, 0, 0);
                result = 1;
            }
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802A86C
/* 战斗对象"长演出/换装"状态机 (gObjActStep: 0 → 0x12..0x21 → 9)。
 * 唯一入口 = 0x0839CD5C 指针表 idx 0x18 (thumb 0x0802A86D)。与 sub_8029BF8 同族近乎逐字相同,
 * 差异仅: case0 f_B6=0xA9 (8029BF8=0xAC), case23 sub_8020CC4 arg5=0x2E8 (8029BF8=0x2E7),
 * case28 sub_804BF14/sub_804B96C 参数序 (r2/r3=0xE/0x14, 栈首=0x1F)。
 * 用 gSceneFadeOut/03000868 双窗口插值 + sub_801A348/sub_801A3A8/sub_801A2AC 文字框 +
 * sub_804BF14/sub_804C728/sub_804B96C/sub_804C4D8 多段装备/调色板切换。
 *   case0   记 posX/posY/headA.f_1E/palSlot, sub_80444A4 + f_B6=0xA9 + f_B4=0 + sub_803F5B4,
 *           计数=0, headA.kindFlags |= 0x10, sub_801A2AC(0x410,0x10,0) → 0x12。
 *   0x12    计数<=9: g867 = sub_801768C(0x10,-0x10,0xA,计数,2) + A2AC(0x410,g867,(u8)(0x10-g867))
 *           + 计数++; 超过: headA.kindFlags |= 0x200 → 0x13。
 *   0x13    sub_801A348 + sub_801A3A8(1,-0x23,-5) + g867=0 + A2AC(0x1747,0,0x10) → 0x14。
 *   0x14    sub_8019B98(0xF,3,0xE,1)!=0 → 计数=0 + sub_804BF14(0,3,7,0xE,0x1C,4,4,-1,2) +
 *           Sfx(0x20) → 0x15。
 *   0x15    计数<=0x13: 计数==4 时 sub_804C728(0,3,0x10); g867=sub_801768C(0,0x10,0x14,计数,2) +
 *           A2AC(0x1747,g867,0x10) + 计数++; 超过: 计数=0 → 0x16。
 *   0x16    计数<=9: g868=sub_801768C(0x10,-0x10,0xA,计数,2) + A2AC(0x1747,g867,g868) + 计数++;
 *           超过: 计数=0 → 0x17。
 *   0x17    计数<=0x13: 计数++; 超过: sub_8020CC4(obj,0x78,0x69,0x1B4,0xE,0x2E8,0x2404) +
 *           headB.f_2A=0 → 0x18。
 *   0x18    headB 的 0x800 落 → Sfx(0x64) → 0x19。
 *   0x19    headB.frameIdx > 0x1D → headB.kindFlags |= 0x100 + headB.frameIdx++ → 0x1C。
 *   0x1C    headB.frameIdx = (frameIdx+1)%6 + 0x1E; 三段 sub_804BF14/sub_804BF14/sub_804B96C
 *           (0,3,0xE,0x14 / 0xE,1,0xE,0x14 / 1,5,0xE,0x14, 栈 0x1F,4,4,-1,2); 计数=0 → 0x1D。
 *   0x1D    计数<=0x13: 计数++; 超过: 计数=0 → 0x1E; 两路都更新 headB.frameIdx=(frameIdx+1)%6+0x1E。
 *   0x1E    headB.frameIdx=(frameIdx+1)%6+0x1E; 计数<=7: 计数++; 超过: BattleUiFlag_Set(0x20) +
 *           obj->state &= 0xDFFF + sub_804C728(0,3,0x20) + sub_804C728(0xE,1,0x20) +
 *           sub_804C4D8(1,5,0x20) + 计数=0 → 0x1F。
 *   0x1F    计数<=0x1F: 计数++; 超过: headA.kindFlags &= 0xFDFF + A2AC(0x410,0,0x10) + 计数=0 → 0x20。
 *   0x20    计数<=0x1F: g867=sub_801768C(0,0x10,0x20,计数,2) + A2AC(0x410,g867,(u8)(0x10-g867)) + 计数++;
 *           超过: headA.kindFlags &= 0xFFEF + A2AC(0,0,0) + sub_8044514(0x14) → 0x21。
 *   0x21    无其他演出占用 → 9。
 *   9       sub_801A3A8(1,0,0) + result = 1。
 * 尾部: sub_803F658(obj); return result (无帧计数自增)。 */
u8 sub_802A86C(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            obj->f_B6 = 0xA9;
            obj->f_B4 = 0;
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->headA.kindFlags |= 0x10;
            sub_801A2AC(0x410, 0x10, 0);
            gObjActStep = 0x12;
            break;
        case 18:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x410, gSceneFadeOut, (u8)(0x10 - gSceneFadeOut));
                gObjActStepTimer += 1;
                break;
            }
            obj->headA.kindFlags |= 0x200;
            gObjActStep = 0x13;
            break;
        case 19:
            sub_801A348();
            ((void (*)(u8, int, int))sub_801A3A8)(1, -0x23, -5);
            gSceneFadeOut = 0;
            sub_801A2AC(0x1747, 0, 0x10);
            gObjActStep = 0x14;
            break;
        case 20:
            if (sub_8019B98(0xF, 3, 0xE, 1) == 0)
                break;
            gObjActStepTimer = 0;
            sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
            Sfx_Play(0x20, 0, 0);
            gObjActStep = 0x15;
            break;
        case 21:
            if (gObjActStepTimer <= 0x13)
            {
                if (gObjActStepTimer == 4)
                    sub_804C728(0, 3, 0x10);
                gSceneFadeOut = sub_801768C(0, 0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x1747, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 0x17;
            break;
        case 23:
            if (gObjActStepTimer <= 0x13)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_8020CC4(obj, 0x78, 0x69, 0x1B4, 0xE, 0x2E8, 0x2404);
            obj->headB.f_2A = 0;
            gObjActStep = 0x18;
            break;
        case 24:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x64, 0, 0);
            gObjActStep = 0x19;
            break;
        case 25:
            if (obj->headB.frameIdx <= 0x1D)
                break;
            obj->headB.kindFlags |= 0x100;
            obj->headB.frameIdx += 1;
            gObjActStep = 0x1C;
            break;
        case 28:
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 6 + 0x1E;
            sub_804BF14(0, 3, 0xE, 0x14, 0x1F, 4, 4, -1, 2);
            sub_804BF14(0xE, 1, 0xE, 0x14, 0x1F, 4, 4, -1, 2);
            sub_804B96C(1, 5, 0xE, 0x14, 0x1F, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            gObjActStep = 0x1D;
            break;
        case 29:
            if (gObjActStepTimer <= 0x13)
            {
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gObjActStep = 0x1E;
            }
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 6 + 0x1E;
            break;
        case 30:
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 6 + 0x1E;
            if (gObjActStepTimer <= 7)
            {
                gObjActStepTimer += 1;
                break;
            }
            BattleUiFlag_Set(0x20);
            obj->state &= 0xDFFF;
            sub_804C728(0, 3, 0x20);
            sub_804C728(0xE, 1, 0x20);
            sub_804C4D8(1, 5, 0x20);
            gObjActStepTimer = 0;
            gObjActStep = 0x1F;
            break;
        case 31:
            if (gObjActStepTimer <= 0x1F)
            {
                gObjActStepTimer += 1;
                break;
            }
            obj->headA.kindFlags &= 0xFDFF;
            sub_801A2AC(0x410, 0, 0x10);
            gObjActStepTimer = 0;
            gObjActStep = 0x20;
            break;
        case 32:
            if (gObjActStepTimer <= 0x1F)
            {
                gSceneFadeOut = sub_801768C(0, 0x10, 0x20, gObjActStepTimer, 2);
                sub_801A2AC(0x410, gSceneFadeOut, (u8)(0x10 - gSceneFadeOut));
                gObjActStepTimer += 1;
                break;
            }
            obj->headA.kindFlags &= 0xFFEF;
            sub_801A2AC(0, 0, 0);
            sub_8044514(0x14);
            gObjActStep = 0x21;
            break;
        case 33:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                gObjActStep = 9;
            break;
        case 9:
            sub_801A3A8(1, 0, 0);
            result = 1;
            break;
        case 26:
        case 27:
        case 34:
        case 35:
        case 36:
        case 37:
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802ADC4
/* 战斗对象"召唤/贝斯手演出"状态机 (gObjActStep: 0 起手 → 0x12..0x19 → 9)。
 * 唯一入口 = 0x0839CD5C 指针表 idx 0x19 (thumb 0x0802ADC5), 由 sub_803F444 家族对 BattleObj 调用。
 * 与 sub_802B608 同族 (同样用 sub_801A348/sub_8019B98/sub_801A2AC/sub_804BDD8/sub_804BE90 +
 * gSceneFadeOut 窗口插值), 但中段换用 sub_804B96C/sub_804C4D8 做调色板/装备层切换。
 *   case0   记 posX/posY/headA.f_1E/palSlot, 计数=0 → 0x12。
 *   0x12    headA 的 0x800 落 → 清 headA 0x100 + sub_801A348 → 0x13。
 *   0x13    sub_8019B98(0xE,3,0xE,1) != 0 → Sfx(0x5A) + gSceneFadeOut=0 +
 *           sub_801A2AC(0x1C42,0,0x10) → 0x14。
 *   0x14    计数=0 + sub_804BDD8(0xE,1,1,1,0xF) → 0x15。
 *   0x15    计数<=0x13: gSceneFadeOut = sub_801768C(0,0xC,0x14,计数,2) +
 *           sub_801A2AC(0x1C42,gSceneFadeOut,0x10) + 计数++; 超过: 计数=0,
 *           gSceneFadeOut=0xC + sub_801A2AC(0x1C42,0xC,0x10) → 0x16。
 *   0x16    Sfx_TrackBusy(1)==0 → 计数=0 → 0x17。
 *   0x17    计数<=0x13: gSceneFadeOut = sub_801768C(0xC,-0xC,0x14,计数,2) +
 *           sub_801A2AC(0x1C42,gSceneFadeOut,0x10) + 计数++; 超过: 计数=0 +
 *           Sfx_StopTrack(1) + sub_801A2AC(0,0,0) + state &= 0x2000 + sub_804BE90(0xE,1) +
 *           REG_DISPCNT &= 0xFDFF + sub_804B96C(palSlot,(u8)sub_801B954(&headA),7,0xE,0x1C,4,4,-1,2) +
 *           → 0x18 + Sfx(0x58)。
 *   0x18    计数<=3: 计数++; 否则 sub_804C4D8(palSlot,(u8)sub_801B954(&headA),0x20) + 计数=0 → 0x19。
 *   0x19    计数<=0x1F: 计数++; 否则 → 9。
 *   9       result = 2。
 * 尾部: 计数++ + sub_803F658(obj); return result (0/2)。 */
u8 sub_802ADC4(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            obj->headA.kindFlags &= 0xFEFF;
            sub_801A348();
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_8019B98(0xE, 3, 0xE, 1) == 0)
                break;
            Sfx_Play(0x5A, 1, 0);
            gSceneFadeOut = 0;
            sub_801A2AC(0x1C42, 0, 0x10);
            gObjActStep = 0x14;
            break;
        case 20:
            gObjActStepTimer = 0;
            sub_804BDD8(0xE, 1, 1, 1, 0xF);
            gObjActStep = 0x15;
            break;
        case 21:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0, 0xC, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x1C42, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneFadeOut = 0xC;
                sub_801A2AC(0x1C42, 0xC, 0x10);
                gObjActStep = 0x16;
            }
            break;
        case 22:
            if (Sfx_TrackBusy(1) == 0)
            {
                gObjActStepTimer = 0;
                gObjActStep = 0x17;
            }
            break;
        case 23:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0xC, -0xC, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x1C42, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                Sfx_StopTrack(1);
                sub_801A2AC(0, 0, 0);
                obj->state &= 0x2000;
                sub_804BE90(0xE, 1);
                REG_DISPCNT &= 0xFDFF;
                sub_804B96C(obj->headA.palSlot, (u8)sub_801B954(&obj->headA), 7, 0xE, 0x1C, 4, 4, -1, 2);
                gObjActStep = 0x18;
                Sfx_Play(0x58, 1, 0);
            }
            break;
        case 24:
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_804C4D8(obj->headA.palSlot, (u8)sub_801B954(&obj->headA), 0x20);
            gObjActStepTimer = 0;
            gObjActStep = 0x19;
            break;
        case 25:
            if (gObjActStepTimer <= 0x1F)
            {
                gObjActStepTimer += 1;
                break;
            }
            gObjActStep = 9;
            break;
        case 9:
            result = 2;
            break;
    }
    gObjActStepTimer += 1;
    sub_803F658(obj);
    return result;
}
// @ 0x0802B0F0
/* 战斗对象双参"绕行归位/两段位移"演出状态机 (gObjActStep: 0 → 0x12..0x1E → 9)。
 * 参1 = 目标对象; 与 sub_802C9E8 同族, 但分两段 (case22/28) 用 sub_801768C 把
 * posX/posY/pad_C1 从锚点插值到目标附近。
 *   case0   记 posX/posY/headA.f_1E/palSlot, 计数=0, sub_80444A4 + sub_803F5B4 +
 *           sub_801CE80(obj,1,0x1B4,0xB,0); f_B6=f_B4=0; headA.kindFlags|=0x100 → 0x12。
 *   0x12    headA 的 0x800 落 → sub_8020CC4(obj,(posX-6),(posY+0xA),0x27C,0xD,0x2F4,4) +
 *           headB.kindFlags|=0x100 → 0x13。
 *   0x13    headB 的 0x800 落 → Sfx(0x5A) + 清 headA/headB 的 0x100 → 0x14。
 *   0x14    headB 到 0x1000 → state&=0xDFFF → 0x15。
 *   0x15    headA.frameIdx > 0x5A → Sfx(0x31) + headA.kindFlags|=0x100, 计数=0 → 0x16。
 *   0x16    计数<=0x1D: posX 向 (arg1->posX-8) 插值, posY 向 arg1->posY 插值,
 *           pad_C1 = sub_801768C(0,0x46,0xF,计数,2) + 计数++;
 *           到 0x1E: pad_C1=0, 清 headA 0x100, posY=arg1->posY+2 → 0x17。
 *   0x17    headA.frameIdx > 0x60 → sub_8020CC4(obj,(posX-8),posY,0x27C,0xD,0x2F5,5) → 0x18。
 *   0x18    headB 的 0x800 落 → sub_8044514(0x28) → 0x19。
 *   0x19    headB 到 0x1000 → state&=0xDFFF + 清 headB 0x1000 → 0x1A;
 *           headA.frameIdx > 0x9B → headA.kindFlags|=0x100。
 *   0x1A    headA.frameIdx > 0x9B → 清 headA 0x100 → 0x1B。
 *   0x1B    headA.frameIdx > 0xA5 结束; 否则 headA.kindFlags|=0x100, 计数=0,
 *           锚点=posX/posY, state=0x1C + Sfx(0x31)。
 *   0x1C    计数<=0x1D: posX/posY 向锚点插值, pad_C1 插值; 否则 pad_C1=0, 清 headA 0x100,
 *           posY=锚点Y, posX=锚点X, state=0x1D + Sfx(0x31)。
 *   0x1D    headA 到 0x1000 → sub_801CE80 复位 → 0x1E。
 *   0x1E    headA 的 0x800 落 → 9。
 *   9      无其他演出占用 → result=1。
 * 尾部 sub_803F658(obj)。 */
u8 sub_802B0F0(BattleObj *obj, BattleObj *arg1)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActStepTimer = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            obj->f_B6 = 0;
            obj->f_B4 = 0;
            sub_801CE80(obj, 1, 0x1B4, 0xB, 0);
            obj->headA.kindFlags |= 0x100;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_8020CC4(obj, (u8)(obj->posX - 6), (u8)(obj->posY + 0xA), 0x27C, 0xD, 0x2F4, 4);
            obj->headB.kindFlags |= 0x100;
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x5A, 1, 0);
            obj->headA.kindFlags &= 0xFEFF;
            obj->headB.kindFlags &= 0xFEFF;
            gObjActStep = 0x14;
            break;
        case 20:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            obj->state &= 0xDFFF;
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headA.frameIdx <= 0x5A)
                break;
            Sfx_Play(0x31, 1, 0);
            obj->headA.kindFlags |= 0x100;
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (gObjActStepTimer <= 0x1D)
            {
                obj->posX = sub_801768C(gObjActSavedX, arg1->posX - 8 - gObjActSavedX, 0x1E, gObjActStepTimer, 0);
                obj->posY = sub_801768C(gObjActSavedY, arg1->posY - gObjActSavedY, 0x1E, gObjActStepTimer, 0);
                obj->pad_C1 = sub_801768C(0, 0x46, 0xF, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->pad_C1 = 0;
                obj->headA.kindFlags &= 0xFEFF;
                obj->posY = arg1->posY + 2;
                gObjActStep = 0x17;
            }
            break;
        case 23:
            if (obj->headA.frameIdx <= 0x60)
                break;
            sub_8020CC4(obj, (u8)(obj->posX - 8), obj->posY, 0x27C, 0xD, 0x2F5, 5);
            gObjActStep = 0x18;
            break;
        case 24:
            if (obj->headB.kindFlags & 0x800)
                break;
            sub_8044514(0x28);
            gObjActStep = 0x19;
            break;
        case 25:
            if (obj->headB.kindFlags & 0x1000)
            {
                obj->state &= 0xDFFF;
                obj->headB.kindFlags &= 0xEFFF;
                gObjActStep = 0x1A;
            }
            if (obj->headA.frameIdx > 0x9B)
                obj->headA.kindFlags |= 0x100;
            break;
        case 26:
            if (obj->headA.frameIdx <= 0x9B)
                break;
            obj->headA.kindFlags &= 0xFEFF;
            gObjActStep = 0x1B;
            break;
        case 27:
            if (obj->headA.frameIdx > 0xA5)
                break;
            obj->headA.kindFlags |= 0x100;
            gObjActStepTimer = 0;
            gObjActMoveFromX = obj->posX;
            gObjActMoveFromY = obj->posY;
            gObjActStep = 0x1C;
            Sfx_Play(0x31, 1, 0);
            break;
        case 28:
            if (gObjActStepTimer <= 0x1D)
            {
                obj->posX = sub_801768C(gObjActMoveFromX, gObjActSavedX - gObjActMoveFromX, 0x1E, gObjActStepTimer, 0);
                obj->posY = sub_801768C(gObjActMoveFromY, gObjActSavedY - gObjActMoveFromY, 0x1E, gObjActStepTimer, 0);
                obj->pad_C1 = sub_801768C(0, 0x46, 0xF, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->pad_C1 = 0;
                obj->headA.kindFlags &= 0xFEFF;
                obj->posY = gObjActSavedY;
                obj->posX = gObjActSavedX;
                gObjActStep = 0x1D;
                Sfx_Play(0x31, 1, 0);
            }
            break;
        case 29:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x1E;
            break;
        case 30:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802B608
/* 战斗对象"召唤/技能释放"演出状态机 (gObjActStep: 0 起手 → 0x12..0x18 → 9)。
 *   case0  记 posX/posY/f_1E/palSlot, sub_80444A4 清同组伤害 + sub_801CE80(obj,5,0x1B4,0xC,0)
 *          切动画 + sub_803F5B4 起手; f_B6=2, f_B4=0 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13   headA.kindFlags 到 0x1000 → (kindFlags|0x100)&0xEFFF + sub_801A348 → 0x14。
 *   0x14   sub_8019B98(0xD,3,0xE,2) != 0 → Sfx(0x55) + sub_801A2AC(0x1447,9,0xF) +
 *          sub_804BDD8(0xE,1,1,-3,0xC) + sub_8044514(0x3C) → 0x15。
 *   0x15   帧计数 <= 0x3B 时自增; 超过则清零 → 0x16。
 *   0x16   帧计数 <= 0x1D 时 gSceneFadeOut = sub_801768C(9,-9,0x1E,cnt,2) +
 *          sub_801A2AC(0x1447,gSceneFadeOut,0xF); 超过则 Sfx_StopTrack(1) + sub_804BE90(0xE,1) +
 *          sub_801A2AC(0,0,0) + 清 DISPCNT bit9 + sub_801CE80 复位 → 0x17。
 *   0x17   等 headA.kindFlags 的 0x800 落 → 0x18。
 *   0x18   帧计数=0 → 9。
 *   9      无其他演出占用 → result = 1。
 * 尾部: sub_803F658(obj) (本函数无帧计数自增)。 */
u8 sub_802B608(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xC, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            obj->headA.kindFlags = obj->headA.kindFlags | 0x100;
            obj->headA.kindFlags = obj->headA.kindFlags & 0xEFFF;
            sub_801A348();
            gObjActStep = 0x14;
            break;
        case 20:
            if (sub_8019B98(0xD, 3, 0xE, 2) == 0)
                break;
            Sfx_Play(0x55, 1, 0);
            sub_801A2AC(0x1447, 9, 0xF);
            sub_804BDD8(0xE, 1, 1, -3, 0xC);
            sub_8044514(0x3C);
            gObjActStep = 0x15;
            break;
        case 21:
            if (gObjActStepTimer <= 0x3B)
            {
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (gObjActStepTimer <= 0x1D)
            {
                gSceneFadeOut = sub_801768C(9, -9, 0x1E, gObjActStepTimer, 2);
                sub_801A2AC(0x1447, gSceneFadeOut, 0xF);
                gObjActStepTimer += 1;
            }
            else
            {
                Sfx_StopTrack(1);
                sub_804BE90(0xE, 1);
                sub_801A2AC(0, 0, 0);
                *(volatile u16 *)(0x80 << 0x13) &= 0xFDFF;
                gObjActStep = 0x17;
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            }
            break;
        case 23:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x18;
            break;
        case 24:
            gObjActStepTimer = 0;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802B8BC
/* 战斗对象"闪现/瞬移演出"状态机 (gObjActStep: 0 起手 → 0x12..0x17 → 9)。
 * 与 sub_802F480 同族 (case 结构一致), 差异: case0 动画类型 0xB (F480 为 0xC),
 * case20 多 Sfx_Play(0x36,1,1), case22 多 Sfx_StopTrack(1)。
 *   case0  记 posX/posY/f_1E/palSlot, sub_80444A4 + sub_801CE80(obj,5,0x1B4,0xB,1) +
 *          sub_803F5B4; f_B6=2, f_B4=0 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13   headA.frameIdx > 0x46 → sub_8020CC4(obj,0xB9,0x78,0x2E0,0xD,0x2F9,5) → 0x14。
 *   0x14   等 headB.kindFlags 的 0x800 落 → Sfx(0x36) + sub_8044514(0x5A), 计数=0 → 0x15。
 *   0x15   sub_80471AC()==0 → 0x16; headA.kindFlags 到 0x1000 → 清 0x1000 置 0x100。
 *   0x16   headA.kindFlags 到 0x1000 → Sfx_StopTrack(1) + sub_801CE80 复位 → 0x17。
 *   0x17   headB.kindFlags 到 0x1000 → obj->state &= 0xDFFF → 9。
 *   9      无其他演出占用 → result = 1。
 * 尾部: sub_803F658(obj); gObjActStepTimer += 1。 */
u8 sub_802B8BC(BattleObj *obj)
{
    u8 result;
    u16 keys;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xB, 1);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx <= 0x46)
                break;
            sub_8020CC4(obj, 0xB9, 0x78, 0x2E0, 0xD, 0x2F9, 5);
            gObjActStep = 0x14;
            break;
        case 20:
            keys = obj->headB.kindFlags & 0x800;
            if (keys != 0)
                break;
            Sfx_Play(0x36, 1, 1);
            sub_8044514(0x5A);
            gObjActStepTimer = keys;
            gObjActStep = 0x15;
            break;
        case 21:
            if (sub_80471AC() == 0)
                gObjActStep = 0x16;
            if ((obj->headA.kindFlags & 0x1000) == 0)
                break;
            obj->headA.kindFlags = (obj->headA.kindFlags & 0xEFFF) | 0x100;
            break;
        case 22:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            Sfx_StopTrack(1);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            obj->state &= 0xDFFF;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    gObjActStepTimer += 1;
    return result;
}
// @ 0x0802BB24
/* 战斗对象"召唤伙伴/传送"演出状态机 (gObjActStep: 0 起手 → 0x12..0x17 → 9)。
 * 双参 (obj, arg1=同池另一对象); 与 sub_802B8BC 同族但逻辑更简。
 *   case0  记 posX/posY/f_1E/palSlot, 计数=0, sub_80444A4 + sub_803F5B4,
 *          f_B6=0, f_B4=0, sub_801CE80(obj,1,0x1B4,0xA,0) 切动画 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → Sfx(0x5A) → 0x13。
 *   0x13   等 headA.kindFlags 到 0x1000 → 按 (arg1->posX+4, arg1->posY+4) 播
 *          sub_8020CC4(0x27C,0xD,0x301,5), headB.f_2A=0, kindFlags |= 0x100 → 0x14。
 *   0x14   等 headB.kindFlags 的 0x800 落 → Sfx(0x9A) + sub_801CE80 复位 → 0x15。
 *   0x15   等 headA.kindFlags 的 0x800 落 → 0x16。
 *   0x16   等 headB.kindFlags 到 0x1000 → sub_8044514(0x1E), 计数=0,
 *          obj->state &= 0xDFFF → 0x17。
 *   0x17   无其他演出占用 → 9; 否则计数 += 1。
 *   9      result = 1。
 * 尾部: sub_803F658(obj) (无帧计数自增)。 */
u8 sub_802BB24(BattleObj *obj, BattleObj *arg1)
{
    u8 result;
    u16 keys;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActStepTimer = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            obj->f_B6 = 0;
            obj->f_B4 = 0;
            sub_801CE80(obj, 1, 0x1B4, 0xA, 0);
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x5A, 1, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            sub_8020CC4(obj, (u8)(arg1->posX + 4), (u8)(arg1->posY + 4), 0x27C, 0xD, 0x301, 5);
            obj->headB.f_2A = 0;
            obj->headA.kindFlags |= 0x100;
            gObjActStep = 0x14;
            break;
        case 20:
            keys = obj->headB.kindFlags & 0x800;
            if (keys != 0)
                break;
            Sfx_Play(0x9A, 1, 0);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x16;
            break;
        case 22:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            sub_8044514(0x1E);
            gObjActStepTimer = 0;
            obj->state &= 0xDFFF;
            gObjActStep = 0x17;
            break;
        case 23:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                gObjActStep = 9;
            else
                gObjActStepTimer += 1;
            break;
        case 9:
            result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802BD94
/* 战斗对象"蓄力/多段演出"状态机 (gObjActStep: 0 起手 → 0x12..0x1B → 9)。
 *   case0  记 posX/posY/f_1E/palSlot, sub_80444A4 + sub_801CE80(obj,5,0x1B4,0xD,0) +
 *          sub_803F5B4; 计数=0, f_B6=1, f_B4=0 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → Sfx(0x5A) → 0x13。
 *   0x13   headA.frameIdx==0x6E 时 Sfx(0x68); 到 0x1000 → Sfx_StopTrack(1) +
 *          sub_801CE80 复位 → 0x14。
 *   0x14   等 0x800 落 → 0x15。
 *   0x15   按 (0x82,0x7D) 播 sub_8020CC4(0x1B4,0xA,0x30E,5); headB.f_2A=0 → 0x16。
 *   0x16   等 headB.kindFlags 的 0x800 落 → Sfx(0x34) → 0x17。
 *   0x17   headB.frameIdx > 0xE → sub_804BF14(0,3,0x10,0x1C,0x1F,4,4,-5,2); 计数=0 → 0x18。
 *   0x18   计数<=3 → 计数++; 否则 sub_804C728(0,3,0x10), 计数=0 → 0x19。
 *   0x19   headB.frameIdx==0x28 时 Sfx(0x5A); > 0x51 → Sfx(0x4E,2,1) +
 *          sub_804BF14(同0x17) + 计数=0 + sub_8044514(0x1E) → 0x1A。
 *   0x1A   计数<=3 → 计数++; 否则 sub_804C728(0,3,0x10), 计数=0 → 0x1B。
 *   0x1B   headB.kindFlags 到 0x1000 → Sfx_StopTrack(2) + obj->state &= 0xDFFF → 9。
 *   9      无其他演出占用 → result = 1。
 * 尾部: sub_803F658(obj) (无帧计数自增)。 */
u8 sub_802BD94(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 1;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x5A, 1, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx == 0x6E)
                Sfx_Play(0x68, 1, 0);
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            Sfx_StopTrack(1);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x15;
            break;
        case 21:
            sub_8020CC4(obj, 0x82, 0x7D, 0x1B4, 0xA, 0x30E, 5);
            obj->headB.f_2A = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x34, 1, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (obj->headB.frameIdx <= 0xE)
                break;
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            gObjActStep = 0x18;
            break;
        case 24:
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_804C728(0, 3, 0x10);
            gObjActStepTimer = 0;
            gObjActStep = 0x19;
            break;
        case 25:
            if (obj->headB.frameIdx == 0x28)
                Sfx_Play(0x5A, 1, 0);
            if (obj->headB.frameIdx <= 0x51)
                break;
            Sfx_Play(0x4E, 2, 1);
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            sub_8044514(0x1E);
            gObjActStep = 0x1A;
            break;
        case 26:
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_804C728(0, 3, 0x10);
            gObjActStepTimer = 0;
            gObjActStep = 0x1B;
            break;
        case 27:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            Sfx_StopTrack(2);
            obj->state &= 0xDFFF;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}

INCLUDE_ASM("asm/nonmatchings", sub_802C0EC);

// @ 0x0802C0EC
/* 战斗对象"变身/强化+位移"演出状态机 (单参; gObjActStep: 0 起手 → 0x12..0x19 → 9)。
 * 与 sub_802B608/sub_802BB24 同族 (同样的 sub_801A348/sub_801A2AC/sub_8019B98/sub_804BDD8/
 * sub_804BE90 组合), 但中段多出 sub_8020CC4 锚点演出 (0x7D,0x91) 与一段左右往返位移 (case23)。
 *   case0   记 posX/posY/headA.f_1E, sub_80444A4 清同组伤害 +
 *           sub_801CE80(obj,5,0x1B4,0xD,0) 切动画 + sub_803F5B4 起手, 计数=0,
 *           f_B6=2 / f_B4=0 → 0x12。
 *   0x12    等 headA.kindFlags 的 0x800 落 → Sfx(0x5A) → 0x13。
 *   0x13    headA.frameIdx==0x6E 时 Sfx(0x68); 等 kindFlags 到 0x1000 → Sfx_StopTrack(1) +
 *           sub_801CE80(obj,0,0822,0,0) → 0x14。
 *   0x14    等 headA.kindFlags 的 0x800 落 → sub_801A348 + sub_801A2AC(0x1F47,0xF,8) +
 *           Sfx(0x64) → 0x15。
 *   0x15    sub_8019B98(0xC,3,0xF,2)!=0 → sub_804BDD8(0xF,1,3,1,6) + 计数=0 +
 *           sub_8020CC4(obj,0x7D,0x91,0x27C,0xD,0x33B,5) + headB.f_2A=0 → 0x16。
 *   0x16    等 headB.kindFlags 的 0x800 落 → 0x17。
 *   0x17    计数<=0x31: headA.frameIdx = (frameIdx+1) % sub_801B95C(&headA) + 0x14,
 *           posX = sub_801768C(gObjActMoveFromX, 0xF0-gObjActMoveFromX, 0x32, 计数, 2); 计数++;
 *           超过: 计数=0, pad_C1=posY, posX=暂存 gObjActSavedX, state&=0xDFFF,
 *           sub_8044514(0x28) → 0x18。
 *   0x18    计数<=0x27: gSceneFadeOut=sub_801768C(0xF,-0xF,0x28,计数,2),
 *           gSceneFadeIn=sub_801768C(8,8,0x28,计数,2), sub_801A2AC(0x1F47,67,68); 计数++;
 *           超过: sub_801A2AC(0,0,0) + sub_804BE90(0xF,1) + REG_DISPCNT&=~0x200 +
 *           sub_801CE80(obj,0,0822,2,0) + headA.kindFlags|=0x100 → 0x19。
 *   0x19    等空闲 (case25/26/27 为空)。
 *   9       无其他演出占用 → 返回 1。
 * 尾部 sub_803F658(obj)。 */
// u8 sub_802C0EC(BattleObj *obj)
// {
//     u8 result;
//     u8 zero;

//     result = 0;
//     GetObjPool();
//     sub_80187E8();
//     switch (gObjActStep)
//     {
//         case 0:
//             gObjActSavedX = obj->posX;
//             gObjActSavedY = obj->posY;
//             gObjActSavedF2A = obj->headA.f_1E;
//             sub_80444A4(obj);
//             zero = 0;
//             sub_801CE80(obj, 5, 0x1B4, 0xD, zero);
//             sub_803F5B4(obj);
//             gObjActStepTimer = 0;
//             obj->f_B6 = 2;
//             obj->f_B4 = 0;
//             gObjActStep = 0x12;
//             break;
//         case 18:
//             if (obj->headA.kindFlags & 0x800)
//                 break;
//             Sfx_Play(0x5A, 1, 0);
//             gObjActStep = 0x13;
//             break;
//         case 19:
//             if (obj->headA.frameIdx == 0x6E)
//                 Sfx_Play(0x68, 1, 0);
//             if (!(obj->headA.kindFlags & 0x1000))
//                 break;
//             Sfx_StopTrack(1);
//             sub_801CE80(obj, 0, gObjActSavedF2A, zero, 0);
//             gObjActStep = 0x14;
//             break;
//         case 20:
//             if (obj->headA.kindFlags & 0x800)
//                 break;
//             sub_801A348();
//             sub_801A2AC(0x1F47, 0xF, 8);
//             Sfx_Play(0x64, 1, 0);
//             gObjActStep = 0x15;
//             break;
//         case 21:
//             if (sub_8019B98(0xC, 3, 0xF, 2) == 0)
//                 break;
//             sub_804BDD8(0xF, 1, 3, 1, 6);
//             gObjActStepTimer = 0;
//             sub_8020CC4(obj, 0x7D, 0x91, 0x27C, 0xD, 0x33B, 5);
//             obj->headB.f_2A = 0;
//             gObjActStep = 0x16;
//             break;
//         case 22:
//             if (obj->headB.kindFlags & 0x800)
//                 break;
//             gObjActStep = 0x17;
//             break;
//         case 23:
//             if (gObjActStepTimer <= 0x31)
//             {
//                 obj->headA.frameIdx = (obj->headA.frameIdx + 1) % sub_801B95C(&obj->headA) + 0x14;
//                 obj->posX = sub_801768C(gObjActMoveFromX, 0xF0 - gObjActMoveFromX, 0x32, gObjActStepTimer, 2);
//                 gObjActStepTimer += 1;
//             }
//             else
//             {
//                 gObjActStepTimer = 0;
//                 obj->pad_C1 = obj->posY;
//                 obj->posX = gObjActSavedX;
//                 obj->state &= 0xDFFF;
//                 sub_8044514(0x28);
//                 gObjActStep = 0x18;
//             }
//             break;
//         case 24:
//             if (gObjActStepTimer <= 0x27)
//             {
//                 gSceneFadeOut = sub_801768C(0xF, -0xF, 0x28, gObjActStepTimer, 2);
//                 gSceneFadeIn = sub_801768C(8, 8, 0x28, gObjActStepTimer, 2);
//                 sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
//                 gObjActStepTimer += 1;
//             }
//             else
//             {
//                 sub_801A2AC(0, 0, 0);
//                 sub_804BE90(0xF, 1);
//                 *(volatile u16 *)(0x80 << 0x13) &= 0xFDFF;
//                 zero = 2;
//                 sub_801CE80(obj, 0, gObjActSavedF2A, zero, 0);
//                 obj->headA.kindFlags |= 0x100;
//                 gObjActStep = 0x19;
//             }
//             break;
//         case 25:
//         case 26:
//         case 27:
//             break;
//         case 9:
//             if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
//                 result = 1;
//             break;
//     }
//     sub_803F658(obj);
//     return result;
// }
// @ 0x0802C490
/* 战斗对象"多段演出/蓄力"状态机 (单参; gObjActStep: 0 起手 → 0x12..0x18 → 9)。
 * 与 sub_802C0EC 相邻, 更像"冲刺攻击"演出: 先用 headA 动画进入, 中段用 sub_8020CC4 播
 * headB 锚点演出 (0xBA,0x7D 锚点, 0x1B4/0xA/0x312/5 参数), 收尾清 obj->state 的 0x2000。
 *   case0   记 posX/posY/headA.f_1E/headA.palSlot, sub_80444A4 清同组伤害 +
 *           sub_801CE80(obj,5,0x1B4,0xD,0) 切动画 + sub_803F5B4 起手, 计数=0,
 *           f_B6=2 / f_B4=0x14 (供消费方), → 0x12。
 *   0x12    等 headA.kindFlags 的 0x800 落 → Sfx(0x5A) → 0x13。
 *   0x13    headA.frameIdx==0x6E 时 Sfx(0x68); 等 kindFlags 到 0x1000 → Sfx_StopTrack(1) +
 *           sub_801CE80 复位 → 0x14。
 *   0x14    等 headA.kindFlags 的 0x800 落 → 0x15。
 *   0x15    sub_8020CC4(obj,0xBA,0x7D,0x1B4,0xA,0x312,5) 播 headB 锚点动画, headB.f_2A=0 → 0x16。
 *   0x16    等 headB.kindFlags 的 0x800 落 → Sfx(0x55) → 0x17。
 *   0x17    headB.frameIdx > 0x13 → sub_8044514(0x1E) + 计数=0 → 0x18。
 *   0x18    headB.kindFlags 到 0x1000 → Sfx_StopTrack(1) + obj->state &= 0xDFFF + → 9;
 *           随后帧计数 += 1 (本 case 恒递增)。
 *   9       无其他演出占用 (gActWaitBusy0/45/56 全 0) → result=1; 否则帧计数 += 1。
 * 尾部 sub_803F658(obj)。注意只有 case0x18 与 case9 的等待分支会递增 gObjActStepTimer。 */
u8 sub_802C490(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0x14;
            gObjActStep = 0x12;
            break;
        case 0x12:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x5A, 1, 0);
            gObjActStep = 0x13;
            break;
        case 0x13:
            if (obj->headA.frameIdx == 0x6E)
                Sfx_Play(0x68, 1, 0);
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            Sfx_StopTrack(1);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x14;
            break;
        case 0x14:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x15;
            break;
        case 0x15:
            sub_8020CC4(obj, 0xBA, 0x7D, 0x1B4, 0xA, 0x312, 5);
            obj->headB.f_2A = 0;
            gObjActStep = 0x16;
            break;
        case 0x16:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x55, 1, 0);
            gObjActStep = 0x17;
            break;
        case 0x17:
            if (obj->headB.frameIdx <= 0x13)
                break;
            sub_8044514(0x1E);
            gObjActStepTimer = 0;
            gObjActStep = 0x18;
            break;
        case 0x18:
            if (obj->headB.kindFlags & 0x1000)
            {
                Sfx_StopTrack(1);
                obj->state &= 0xDFFF;
                gObjActStep = 9;
            }
            gObjActStepTimer += 1;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            else
                gObjActStepTimer += 1;
            break;
        case 25:
        case 26:
        case 27:
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802C714
/* 战斗对象"多段演出/蓄力"状态机 (gObjActStep: 0 起手 → 0x12..0x19 → 9)。
 * 与 sub_802BD94 同族单参, 但 case0 f_B6=2; case22 Sfx(0x4E,1,1); case23 阈值 0x18 +
 * sub_8044514(0x32); 只有 case 0x12..0x19 (无 0x1A/0x1B)。
 *   case0  记 posX/posY/headA.f_1E/headA.palSlot, sub_80444A4 + sub_801CE80(obj,5,0x1B4,0xD,0) +
 *          sub_803F5B4; 计数=0, f_B6=2, f_B4=0 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → Sfx(0x5A) → 0x13。
 *   0x13   headA.frameIdx==0x6E 时 Sfx(0x68); 到 0x1000 → Sfx_StopTrack(1) +
 *          sub_801CE80 复位 → 0x14。
 *   0x14   等 0x800 落 → 0x15。
 *   0x15   sub_8020CC4(obj,0x82,0x7D,0x1B4,0xA,0x313,5); headB.f_2A=0 → 0x16。
 *   0x16   等 headB.kindFlags 的 0x800 落 → Sfx(0x4E,1,1) → 0x17。
 *   0x17   headB.frameIdx > 0x18 → sub_804BF14(0,3,0x10,0x1C,0x1F,4,4,-1,2); 计数=0 +
 *          sub_8044514(0x32) → 0x18。
 *   0x18   计数>3 → state=0x19 + sub_804C728(0,3,0x10); 否则计数++ (与 0x19 共享尾块)。
 *   0x19   headB.frameIdx==0x5B 时 Sfx_StopTrack(1); headB.kindFlags 到 0x1000 →
 *          obj->state &= 0xDFFF + state=9; 尾部计数++。
 *   9      无其他演出占用 (gActWaitBusy0/45/56 全 0) → result=1。
 * 尾部 sub_803F658(obj)。 */
u8 sub_802C714(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x5A, 1, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx == 0x6E)
                Sfx_Play(0x68, 1, 0);
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            Sfx_StopTrack(1);
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x15;
            break;
        case 21:
            sub_8020CC4(obj, 0x82, 0x7D, 0x1B4, 0xA, 0x313, 5);
            obj->headB.f_2A = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x4E, 1, 1);
            gObjActStep = 0x17;
            break;
        case 23:
            if (obj->headB.frameIdx <= 0x18)
                break;
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            sub_8044514(0x32);
            gObjActStep = 0x18;
            break;
        case 24:
            if (gObjActStepTimer > 3)
            {
                gObjActStep = 0x19;
                sub_804C728(0, 3, 0x10);
                break;
            }
            gObjActStepTimer += 1;
            break;
        case 25:
            if (obj->headB.frameIdx == 0x5B)
                Sfx_StopTrack(1);
            if (obj->headB.kindFlags & 0x1000)
            {
                obj->state &= 0xDFFF;
                gObjActStep = 9;
            }
            gObjActStepTimer += 1;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802C9E8
/* 战斗对象"多段演出/绕到目标+位移"演出状态机 (gObjActStep: 0 起手 → 0x12..0x1B → 0x17 → 9)。
 * 双参 (obj, arg1=同池另一 BattleObj); 唯一入口 0x0839CDE4 指针表 handler。
 * 与 sub_8027D9C/sub_802D728 同族: case22 起把 headB.f_2B/f_2C 从锚点斜向插值到 arg1 附近,
 * case30 再用 sub_801768C 把 posX/posY/pad_C1 向 arg1 正上方位移收尾。
 *   case0   记 posX/posY/headA.f_1E/palSlot, 计数=0, sub_80444A4 清同组伤害 +
 *           sub_803F5B4 + sub_801CE80(obj,1,0x1B4,0xA,0) 切动画; f_B6=0, f_B4=0xF → 0x12。
 *   0x12    headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13    headA.frameIdx > 0x17 → Sfx(0x66,1,1) → 0x14。
 *   0x14    headA.frameIdx > 0x47 → Sfx_StopTrack(1) → 0x15。
 *   0x15    headA.frameIdx > 0x52 → 0x16。
 *   0x16    headA 到 0x1000 → 清位并置 0x100; 由 (posX+0x2E, posY-0x2E) 播
 *           sub_8020CC4(0x27C,0xC,0x31A,5) → 0x17。
 *   0x17    headA 到 0x1000 → 清位并置 0x100; headB 的 0x800 未落时置 headB 0x100,
 *           计数=0 → 0x18。
 *   0x18    headA 到 0x1000 → 清位并置 0x100; 计数<=9 时把 headB.frameIdx 循环 +1 (模 0x14)
 *           并把 headB.f_2B/f_2C 从 gObjActMoveFromX/F 向 arg1 的 (posX, posY+4) 插值;
 *           到 10: Sfx(0x67,1,0), 清 headB 0x100, frameIdx=0x14, f_2B/f_2C=arg1 坐标+4,
 *           计数=0, sub_8044514(0x14) → 0x19。
 *   0x19    headA 到 0x1000 → 清位并置 0x100; headB 到 0x1000 → 清 state 的 0x2000 → 0x1A;
 *           计数 += 1。
 *   0x1A    sub_801CE80 复位暂存的 f_1E/palSlot → 0x1B; 计数 += 1。
 *   0x1B    headA 的 0x800 落 → 9; 计数 += 1。
 *   0x1E    计数<=0x1D: posX/posY 向 (arg1->posX-8, arg1->posY) 插值, pad_C1 = 插值 0→0x46;
 *           到 0x1E: pad_C1=0, 清 headA 0x100, posY=arg1->posY+2 → 0x17。
 *   9       计数 += 1; 无其他演出占用 → 返回 1。
 * 尾部 sub_803F658(obj)。 */
u8 sub_802C9E8(BattleObj *obj, BattleObj *arg1)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActStepTimer = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            sub_801CE80(obj, 1, 0x1B4, 0xA, 0);
            obj->f_B6 = 0;
            obj->f_B4 = 0xF;
            gObjActStep = 0x12;
            break;
        case 18:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx > 0x17)
            {
                Sfx_Play(0x66, 1, 1);
                gObjActStep = 0x14;
            }
            break;
        case 20:
            if (obj->headA.frameIdx > 0x47)
            {
                Sfx_StopTrack(1);
                gObjActStep = 0x15;
            }
            break;
        case 21:
            if (obj->headA.frameIdx > 0x52)
                gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headA.kindFlags & 0x1000)
                obj->headA.kindFlags = (obj->headA.kindFlags & 0xEFFF) | 0x100;
            gObjActMoveFromX = obj->posX + 0x2E;
            gObjActMoveFromY = obj->posY - 0x2E;
            sub_8020CC4(obj, gObjActMoveFromX, gObjActMoveFromY, 0x27C, 0xC, 0x31A, 5);
            gObjActStep = 0x17;
            break;
        case 23:
            if (obj->headA.kindFlags & 0x1000)
                obj->headA.kindFlags = (obj->headA.kindFlags & 0xEFFF) | 0x100;
            if (!(obj->headB.kindFlags & 0x800))
            {
                obj->headB.kindFlags |= 0x100;
                gObjActStepTimer = 0;
                gObjActStep = 0x18;
            }
            break;
        case 24:
            if (obj->headA.kindFlags & 0x1000)
                obj->headA.kindFlags = (obj->headA.kindFlags & 0xEFFF) | 0x100;
            if (gObjActStepTimer <= 9)
            {
                obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 0x14;
                obj->headB.f_2B = sub_801768C(gObjActMoveFromX, arg1->posX - gObjActMoveFromX, 0xA, gObjActStepTimer, 0);
                obj->headB.f_2C = sub_801768C(gObjActMoveFromY, (arg1->posY + 4) - gObjActMoveFromY, 0xA, gObjActStepTimer, 0);
                gObjActStepTimer += 1;
            }
            else
            {
                Sfx_Play(0x67, 1, 0);
                obj->headB.kindFlags &= 0xFEFF;
                obj->headB.frameIdx = 0x14;
                obj->headB.f_2B = arg1->posX + 4;
                obj->headB.f_2C = arg1->posY + 4;
                gObjActStepTimer = 0;
                sub_8044514(0x14);
                gObjActStep = 0x19;
            }
            break;
        case 25:
            if (obj->headA.kindFlags & 0x1000)
                obj->headA.kindFlags = (obj->headA.kindFlags & 0xEFFF) | 0x100;
            if (obj->headB.kindFlags & 0x1000)
            {
                obj->state &= 0xDFFF;
                gObjActStep = 0x1A;
            }
            gObjActStepTimer += 1;
            break;
        case 26:
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x1B;
            gObjActStepTimer += 1;
            break;
        case 27:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 9;
            gObjActStepTimer += 1;
            break;
        case 30:
            if (gObjActStepTimer <= 0x1D)
            {
                obj->posX = sub_801768C(gObjActSavedX, (arg1->posX - 8) - gObjActSavedX, 0x1E, gObjActStepTimer, 0);
                obj->posY = sub_801768C(gObjActSavedY, arg1->posY - gObjActSavedY, 0x1E, gObjActStepTimer, 0);
                obj->pad_C1 = sub_801768C(0, 0x46, 0xF, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->pad_C1 = 0;
                obj->headA.kindFlags &= 0xFEFF;
                obj->posY = arg1->posY + 2;
                gObjActStep = 0x17;
            }
            break;
        case 9:
            gObjActStepTimer += 1;
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802CE90
/* 战斗对象"冲刺/跳跃攻击"演出状态机 (gObjActStep: 0 起手 → 0x12..0x1A → 9)。
 * 与 sub_802C714/sub_802BD94 同族单参, 但用 headA 主头动画打两段 sub_8020CC4 +
 * 用 sub_801768C/sub_801A2AC/sub_804BE90 做位置插值与屏幕效果。
 *   case0  记 posX/posY/headA.f_1E/palSlot, sub_80444A4 + sub_801CE80(obj,5,0x1B4,0xA,0) +
 *          sub_803F5B4; 计数=0, f_B6=2, f_B4=0 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13   headA.frameIdx > 0x37 → headA.kindFlags |= 0x100 +
 *          sub_8020CC4(obj,(u8)(posX+0x46),(u8)(posY-0x20),0x27C,0xC,0x31C,5) → 0x14。
 *   0x14   等 headB.kindFlags 的 0x800 落 → Sfx(0x55,1,0) → 0x15。
 *   0x15   headB.kindFlags 到 0x1000 → Sfx_StopTrack(1) + obj->state &= 0xDFFF +
 *          headA.kindFlags &= 0xFEFF → 0x16。
 *   0x16   headA.kindFlags 到 0x1000 → sub_801CE80 复位 → 0x17。
 *   0x17   等 headA.kindFlags 的 0x800 落 → sub_8020CC4(obj,0x74,0x80,0x2EA,0xE,0x2C2,5) → 0x18。
 *   0x18   等 headB.kindFlags 的 0x800 落 → headB.kindFlags |= 0x2000; 计数=0 → 0x19。
 *   0x19   headB.kindFlags 到 0x1000 → headB.kindFlags &= 0xEFFF + state &= 0xDFFF +
 *          f_B6=1 + sub_8044514(0x28) → 9。
 *   0x1A   计数<=0x18: 计数++ + gSceneFadeOut = sub_801768C(7,-7,0x19,计数,2) +
 *          sub_801A2AC(0x1C47,gSceneFadeOut,0xF); 否则 sub_804BE90(0xE,1) +
 *          sub_801A2AC(0,0,0) + DISPCNT &= 0xFDFF + state &= 0xDFFF → 9。
 *   9      无其他演出占用 (gActWaitBusy0/45/56 全 0) → result=1。
 * 尾部 sub_803F658(obj)。 */
u8 sub_802CE90(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xA, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx <= 0x37)
                break;
            obj->headA.kindFlags |= 0x100;
            sub_8020CC4(obj, (u8)(obj->posX + 0x46), (u8)(obj->posY - 0x20), 0x27C, 0xC, 0x31C, 5);
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x55, 1, 0);
            gObjActStep = 0x15;
            break;
        case 21:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            Sfx_StopTrack(1);
            obj->state &= 0xDFFF;
            obj->headA.kindFlags &= 0xFEFF;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x17;
            }
            break;
        case 23:
            if (obj->headA.kindFlags & 0x800)
                break;
            sub_8020CC4(obj, 0x74, 0x80, 0x2EA, 0xE, 0x2C2, 5);
            gObjActStep = 0x18;
            break;
        case 24:
            if (obj->headB.kindFlags & 0x800)
                break;
            obj->headB.kindFlags |= 0x2000;
            gObjActStepTimer = 0;
            gObjActStep = 0x19;
            break;
        case 25:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            obj->headB.kindFlags &= 0xEFFF;
            obj->state &= 0xDFFF;
            obj->f_B6 = 1;
            sub_8044514(0x28);
            gObjActStep = 9;
            break;
        case 26:
            if (gObjActStepTimer <= 0x18)
            {
                gObjActStepTimer += 1;
                gSceneFadeOut = sub_801768C(7, -7, 0x19, gObjActStepTimer, 2);
                sub_801A2AC(0x1C47, gSceneFadeOut, 0xF);
                break;
            }
            sub_804BE90(0xE, 1);
            sub_801A2AC(0, 0, 0);
            REG_DISPCNT &= 0xFDFF;
            obj->state &= 0xDFFF;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802D1FC
/* 战斗对象"放声/蓄力"演出状态机 (gObjActStep: 0 → 0x12..0x17 → 9)。
 * 单参, 头部两段 headA/headB kindFlags 等待 + 一次位置锁定后的 sub_8020CC4 锚点动画。
 *   case0  暂存 posX/posY/headA.f_1E/palSlot + sub_80444A4 +
 *          sub_801CE80(obj,5,0x1B4,0xA,0) + sub_803F5B4; 计数=0, f_B6=2, f_B4=0 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13   headA.frameIdx > 0x37 → headA.kindFlags |= 0x100 +
 *          sub_8020CC4(obj,(u8)(posX+0x3C),(u8)(posY-0x18),0x27C,0xC,0x320,5) → 0x14。
 *   0x14   等 headB.kindFlags 的 0x800 落 → Sfx(0x64) → 0x15。
 *   0x15   headB.kindFlags 到 0x1000 → Sfx_StopTrack(1) + obj->state &= 0xDFFF +
 *          headA.kindFlags &= 0xFEFF + sub_8044514(0x28) → 0x16。
 *   0x16   headA.kindFlags 到 0x1000 → sub_801CE80 复位 → 0x17。
 *   0x17   等 headA.kindFlags 的 0x800 落 → 9。
 *   9      无其他演出占用 (gActWaitBusy0/45/56 全 0) → result=1。
 * 尾部 sub_803F658(obj)。 */
u8 sub_802D1FC(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xA, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx <= 0x37)
                break;
            obj->headA.kindFlags |= 0x100;
            sub_8020CC4(obj, (u8)(obj->posX + 0x3C), (u8)(obj->posY - 0x18), 0x27C, 0xC, 0x320, 5);
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x64, 1, 0);
            gObjActStep = 0x15;
            break;
        case 21:
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            Sfx_StopTrack(1);
            obj->state &= 0xDFFF;
            obj->headA.kindFlags &= 0xFEFF;
            sub_8044514(0x28);
            gObjActStep = 0x16;
            break;
        case 22:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802D454
/* 战斗对象"蓄力/放声"演出状态机 (gObjActStep: 0 起手 → 0x12..0x19 → 9)。
 * 双参: 参1 = 同池目标对象 (case22 等其 headB.kindFlags 的 0x800 落)。
 * 用 sub_804BF14(9参)/sub_804C728 做窗口设置, 末尾 case9 按概率给命中槽写状态 2。
 *   case0  暂存 posX/posY/headA.f_1E/palSlot + sub_80444A4 +
 *          sub_801CE80(obj,5,0x1B4,0xA,2) + sub_803F5B4; 计数=0; f_B6=0x31D, f_B4=0xC → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13   headA.frameIdx > 0x27 → Sfx(0x9A) + sub_804BF14(0,3,0x10,0x1C,0x1F,4,4,-1,2);
 *          计数=0 → 0x14。
 *   0x14   计数<=3 → 计数++; 否则 sub_804C728(0,3,0x10) → 0x15。
 *   0x15   headA.frameIdx > 0x57 → headA.kindFlags |= 0x100 + 计数=0 +
 *          sub_8044514(0x32) → 0x16。
 *   0x16   等参1 headB.kindFlags 的 0x800 落 → Sfx(0x69) → 0x17。
 *   0x17   无其他演出占用 (gActWaitBusy0/45/56 全 0) → headA.kindFlags &= 0xFEFF → 0x18。
 *   0x18   headA.kindFlags 到 0x1000 → sub_801CE80 复位 → 0x19。
 *   0x19   等 headA.kindFlags 的 0x800 落 → 9。
 *   9     遍历 gObjActGroupSlots 命中槽 (高半字节==0x10) 按 40/100 概率写
 *          gObjSlotFxCmd[低半字节]=2, 返回 1。
 * 尾部 sub_803F658(obj)。 */
u8 sub_802D454(BattleObj *obj, BattleObj *obj2)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xA, 2);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 0x31D;
            obj->f_B4 = 0xC;
            gObjActStep = 0x12;
            break;
        case 18:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx > 0x27)
            {
                Sfx_Play(0x9A, 1, 0);
                sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
                gObjActStepTimer = 0;
                gObjActStep = 0x14;
            }
            break;
        case 20:
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_804C728(0, 3, 0x10);
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headA.frameIdx > 0x57)
            {
                obj->headA.kindFlags |= 0x100;
                gObjActStepTimer = 0;
                sub_8044514(0x32);
                gObjActStep = 0x16;
            }
            break;
        case 22:
            if (obj2->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x69, 1, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                obj->headA.kindFlags &= 0xFEFF;
                gObjActStep = 0x18;
            }
            break;
        case 24:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x19;
            }
            break;
        case 25:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 9;
            break;
        case 9:
        {
            u8 i;

            GetObjPool();
            for (i = 0; i < gObjActGroupCount; i++)
            {
                if ((gObjActGroupSlots[i] & 0xF0) != 0x10)
                    continue;
                if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
                    gObjSlotFxCmd[gObjActGroupSlots[i] & 0xF] = 2;
            }
            result = 1;
            break;
        }
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802D728
/* 战斗对象双参"目标重定位/多段插值"演出状态机 (gObjActStep: 0 → 0x12..0x17 → 9)。
 * 参1 = 目标对象 obj2; 用 sub_801768C 把 headB 精灵锚点 (headB.f_2B/f_2C) 平滑插值到目标坐标。
 *   case0  暂存 posX/posY/headA.f_1E/palSlot; 计数=0; sub_80444A4 + sub_803F5B4;
 *          f_B6=f_B4=0; sub_801CE80(obj,1,0x1B4,0xD,0) → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13   headA.frameIdx==0xC → Sfx(0x5A); frameIdx>0x79 → Sfx(0x31) +
 *          gObjActMoveFromX=(u8)(posX+0x1D), gObjActMoveFromY=(u8)(posY-0x2F) +
 *          sub_8020CC4(obj, 锚点X, 锚点Y, 0x27C, 0xE, 0x328, 5) + headB.kindFlags|=0x100 → 0x14。
 *   0x14   等 headB.kindFlags 的 0x800 落 → 计数=0 → 0x15。
 *   0x15   headA.kindFlags 到 0x1000 → (kindFlags|=0x100) &= 0xEFFF;
 *          计数<=9: headB.frameIdx=(frameIdx+1)%9 + headB.f_2B/f_2C =
 *          sub_801768C(锚点, 目标-锚点, 0x1E, 计数, 0) + 计数++; 否则 headB.frameIdx=0xA,
 *          headB.kindFlags&=0xFEFF, headB.f_2B/f_2C=obj2->posX/posY, sub_8044514(0x3C) → 0x16。
 *   0x16   headA.kindFlags 到 0x1000 → obj->state&=0xDFFF + sub_801CE80 复位 +
 *          headA.kindFlags|=0x100 → 0x17。
 *   0x17   等 headA.kindFlags 的 0x800 落 → 9 + headA.kindFlags&=0xFEFF。
 *   9      无其他演出占用 (gActWaitBusy0/45/56 全 0) → result=1。
 * 尾部 sub_803F658(obj)。 */
u8 sub_802D728(BattleObj *obj, BattleObj *obj2)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActStepTimer = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            obj->f_B6 = 0;
            obj->f_B4 = 0;
            sub_801CE80(obj, 1, 0x1B4, 0xD, 0);
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx == 0xC)
                Sfx_Play(0x5A, 1, 0);
            if (obj->headA.frameIdx <= 0x79)
                break;
            Sfx_Play(0x31, 1, 0);
            gObjActMoveFromX = (u8)(obj->posX + 0x1D);
            gObjActMoveFromY = (u8)(obj->posY - 0x2F);
            sub_8020CC4(obj, gObjActMoveFromX, gObjActMoveFromY, 0x27C, 0xE, 0x328, 5);
            obj->headB.kindFlags |= 0x100;
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headB.kindFlags & 0x800)
                break;
            gObjActStepTimer = 0;
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headA.kindFlags & 0x1000)
            {
                obj->headA.kindFlags |= 0x100;
                obj->headA.kindFlags &= 0xEFFF;
            }
            if (gObjActStepTimer <= 9)
            {
                obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 9;
                obj->headB.f_2B = sub_801768C(gObjActMoveFromX, obj2->posX - gObjActMoveFromX, 0x1E, gObjActStepTimer, 0);
                obj->headB.f_2C = sub_801768C(gObjActMoveFromY, obj2->posY - gObjActMoveFromY, 0x1E, gObjActStepTimer, 0);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->headB.frameIdx = 0xA;
                obj->headB.kindFlags &= 0xFEFF;
                obj->headB.f_2B = obj2->posX;
                obj->headB.f_2C = obj2->posY;
                sub_8044514(0x3C);
                gObjActStep = 0x16;
            }
            break;
        case 22:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            obj->state &= 0xDFFF;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            obj->headA.kindFlags |= 0x100;
            gObjActStep = 0x17;
            break;
        case 23:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            obj->headA.kindFlags &= 0xFEFF;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802DA78
/* 战斗对象"多段连击/旋转攻击"演出状态机 (gObjActStep: 0 起手 → 0x12..0x1A → 9)。
 * 同 sub_802CE90 族单参, 但用 headA.frameIdx 的 %0x1F 循环驱动两段 sub_8020CC4。
 *   case0  记 posX/posY/headA.f_1E/palSlot, sub_80444A4 +
 *          sub_801CE80(obj,5,gObjActSavedF2A,0xD,0) + sub_803F5B4; 计数=0xA, f_B6=1, f_B4=0 → 0x12。
 *   0x12   等 headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13   headA.frameIdx > 6 → sub_8020CC4(obj,(u8)(posX+0x3C),posY,0x1B4,0xE,0x329,5) +
 *          headA.kindFlags |= 0x100 → 0x14。
 *   0x14   等 headB.kindFlags 的 0x800 落 → state=0x15 + Sfx(0x5B,1,0) + headA.kindFlags &= 0xFEFF。
 *   0x15   headA.frameIdx > 0x5F → Sfx(0x5A,1,0) + sub_8044514(0x3C) + 计数=0 +
 *          headA/headB.kindFlags |= 0x100 + sub_804BF14(...) → 0x19。
 *   0x19   headA.frameIdx=(frameIdx+1)%0x1F+0x60; headB.frameIdx=(frameIdx+1)%0x10+0x5A;
 *          计数<=3 → 计数++; 否则 计数=0 + sub_804C728(0,3,0x20) → 0x1A。
 *   0x1A   同 0x19 但 headA +0x61, 计数<=0x1D; 否则 headB.frameIdx=0x6B +
 *          headB.kindFlags &= 0xFEFF → 0x16。
 *   0x16   headA.frameIdx=(frameIdx+1)%0x1F+0x61; headB.kindFlags 到 0x1000 →
 *          state &= 0xDFFF + headA.kindFlags &= 0xFEFF + headA.frameIdx=0x80 → 0x17。
 *   0x17   headA.kindFlags 到 0x1000 → sub_801CE80 复位 + headA.kindFlags |= 0x100 → 0x18。
 *   0x18   等 headA.kindFlags 的 0x800 落 → state=9 + headA.kindFlags &= 0xFEFF。
 *   9      无其他演出占用 → result=1。
 * 尾部 sub_803F658(obj)。注意 case25/26 的 `计数++` 共享同一尾块。 */
u8 sub_802DA78(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, gObjActSavedF2A, 0xD, 0);
            sub_803F5B4(obj);
            gObjActStepTimer = 0xA;
            obj->f_B6 = 1;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx <= 6)
                break;
            sub_8020CC4(obj, (u8)(obj->posX + 0x3C), obj->posY, 0x1B4, 0xE, 0x329, 5);
            obj->headA.kindFlags |= 0x100;
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headB.kindFlags & 0x800)
                break;
            gObjActStep = 0x15;
            Sfx_Play(0x5B, 1, 0);
            obj->headA.kindFlags &= 0xFEFF;
            break;
        case 21:
            if (obj->headA.frameIdx <= 0x5F)
                break;
            Sfx_Play(0x5A, 1, 0);
            sub_8044514(0x3C);
            gObjActStepTimer = 0;
            obj->headA.kindFlags |= 0x100;
            obj->headB.kindFlags |= 0x100;
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStep = 0x19;
            break;
        case 25:
            obj->headA.frameIdx = (obj->headA.frameIdx + 1) % 0x1F + 0x60;
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 0x10 + 0x5A;
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            sub_804C728(0, 3, 0x20);
            gObjActStep = 0x1A;
            break;
        case 26:
            obj->headA.frameIdx = (obj->headA.frameIdx + 1) % 0x1F + 0x61;
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 0x10 + 0x5A;
            if (gObjActStepTimer <= 0x1D)
            {
                gObjActStepTimer += 1;
                break;
            }
            obj->headB.frameIdx = 0x6B;
            obj->headB.kindFlags &= 0xFEFF;
            gObjActStep = 0x16;
            break;
        case 22:
            obj->headA.frameIdx = (obj->headA.frameIdx + 1) % 0x1F + 0x61;
            if (!(obj->headB.kindFlags & 0x1000))
                break;
            obj->state &= 0xDFFF;
            obj->headA.kindFlags &= 0xFEFF;
            obj->headA.frameIdx = 0x80;
            gObjActStep = 0x17;
            break;
        case 23:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            obj->headA.kindFlags |= 0x100;
            gObjActStep = 0x18;
            break;
        case 24:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            obj->headA.kindFlags &= 0xFEFF;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802DE04
INCLUDE_ASM("asm/nonmatchings", sub_802DE04);
// @ 0x0802DFDC
u32 sub_802DFDC(BattleObj *arg)
{
    u32 ret;
    u16 keys;

    ret = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = arg->posX;
            gObjActSavedY = arg->posY;
            gObjActSavedF2A = arg->headA.f_1E;
            gObjActSavedPal = arg->headA.palSlot;
            sub_80444A4((BattleObj *)arg);
            sub_801CE80(arg, 5, 0x1B4, 0xC, 1);
            sub_803F5B4((BattleObj *)arg);
            gObjActStepTimer = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 2;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (arg->headA.frameIdx > 0x46)
            {
                sub_8020CC4(arg, 0xB9, 0x78, 0x2E0, 0xE, 0x2F9, 5);
                gObjActStep = 0x14;
            }
            break;
        case 20:
            keys = arg->headB.kindFlags & 0x800;
            if (keys != 0)
                break;
            sub_8044514(0x5A);
            gObjActStepTimer = keys;
            gObjActStep = 0x15;
            break;
        case 21:
            if (sub_80471AC() == 0)
                gObjActStep = 0x16;
            if ((arg->headA.kindFlags & 0x1000) == 0)
                break;
            arg->headA.kindFlags = (arg->headA.kindFlags & 0xEFFF) | 0x100;
            break;
        case 22:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_801CE80(arg, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (!(arg->headB.kindFlags & 0x1000))
                break;
            keys = arg->state & 0xDFFF;
            arg->state = keys;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                ret = 1;
            break;
    }
    sub_803F658((BattleObj *)arg);
    gObjActStepTimer += 1;
    return ret;
}
// @ 0x0802E234
/* 战斗对象"横移演出"状态机 (gObjActStep: 0 起手 → 0x12..0x17 → 9)。
 * 单参; 唯一入口 0x0839CE04 指针表 handler。用 headA 主头动画, 中段把 posX 从暂存
 * gObjActSavedX 插值到 0xF0 (向右横移), 末尾复位。
 *   case0   记 posX/posY/headA.f_1E/palSlot, 计数=0, sub_80444A4 清同组伤害 +
 *           sub_803F5B4; f_B6=0, f_B4=0, sub_801CE80(obj,1,0x1B4,0xA,0) 切动画 → 0x12。
 *   0x12    headA.kindFlags 的 0x800 落 → 0x13。
 *   0x13    headA.frameIdx > 0x27 → Sfx(0x63,1,0) → 0x14。
 *   0x14    headA.frameIdx > 0x4C → 计数=0, Sfx(0x36,1,0), headA.kindFlags |= 0x100 → 0x15。
 *   0x15    计数<=0x13: 计数==0xA 时 sub_8044514(0x14); headA.frameIdx 循环 +1 (模 0x15) 后
 *           +0x4D; posX = sub_801768C(gObjActSavedX, 0xF0-g8628, 0x14, 计数, 2); 计数++;
 *           到 0x14: 清 headA 0x100, frameIdx=0x64, posX=g8628 → 0x16。
 *   0x16    headA 到 0x1000 → sub_801CE80 复位暂存 f_1E/palSlot → 0x17。
 *   0x17    headA 的 0x800 落 → 9。
 *   9       无其他演出占用 → 返回 1。
 * 尾部 sub_803F658(obj)。 */
u8 sub_802E234(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActStepTimer = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            obj->f_B6 = 0;
            obj->f_B4 = 0;
            sub_801CE80(obj, 1, 0x1B4, 0xA, 0);
            gObjActStep = 0x12;
            break;
        case 18:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx > 0x27)
            {
                Sfx_Play(0x63, 1, 0);
                gObjActStep = 0x14;
            }
            break;
        case 20:
            if (obj->headA.frameIdx > 0x4C)
            {
                gObjActStepTimer = 0;
                Sfx_Play(0x36, 1, 0);
                obj->headA.kindFlags |= 0x100;
                gObjActStep = 0x15;
            }
            break;
        case 21:
            if (gObjActStepTimer <= 0x13)
            {
                if (gObjActStepTimer == 0xA)
                    sub_8044514(0x14);
                obj->headA.frameIdx = (obj->headA.frameIdx + 1) % 0x15 + 0x4D;
                obj->posX = sub_801768C(gObjActSavedX, 0xF0 - gObjActSavedX, 0x14, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->headA.kindFlags &= 0xFEFF;
                obj->headA.frameIdx = 0x64;
                obj->posX = gObjActSavedX;
                gObjActStep = 0x16;
            }
            break;
        case 22:
            if (obj->headA.kindFlags & 0x1000)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x17;
            }
            break;
        case 23:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802E49C
u32 sub_802E49C(BattleObj *arg, BattleObj *arg2)
{
    u32 ret;

    ret = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = arg->posX;
            gObjActSavedY = arg->posY;
            gObjActSavedF2A = arg->headA.f_1E;
            gObjActSavedPal = arg->headA.palSlot;
            sub_80444A4(arg);
            sub_801CE80(arg, 5, 0x1B4, 0xC, 0);
            arg->headA.kindFlags |= 0x100;
            sub_803F5B4(arg);
            gObjActStepTimer = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 2;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            sub_8020CC4(arg, arg2->posX, arg2->posY, 0x27C, 0xE, 0x30C, 5);
            gObjActStep = 0x13;
            break;
        case 19:
            if (arg->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x5E, 1, 0);
            arg->headA.kindFlags &= 0xFEFF;
            gObjActStep = 0x14;
            break;
        case 20:
            if (arg->headA.frameIdx <= 0x83)
                break;
            Sfx_Play(0x58, 1, 0);
            gObjActStep = 0x15;
            break;
        case 21:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_801CE80(arg, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            arg->state &= 0xDFFF;
            gObjActStep = 0x16;
            break;
        case 22:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                ret = 1;
            break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x0802E6C8
/* 战斗对象"多段演出/蓄力"状态机 (gObjActStep: 0 起手 → 0x1C..0x1F → 0x15..0x1B → 9)。
 * 单参; 唯一入口 0x0839CE10 指针表 handler。前半段 0x1C..0x1F 走 headA 主头锚点演出,
 * 后半段 0x15..0x1B 与 sub_802BD94 同族 (headB 副头 + sub_804BF14/sub_804C728 两轮窗口设置)。
 *   case0   记 posX/posY/f_1E/palSlot, sub_80444A4 清同组伤害 +
 *           sub_801CE80(obj,5,0x1B4,0xD,1) 切动画 + sub_803F5B4; 计数=0, f_B6=2, f_B4=0 → 0x1C。
 *   0x1C    headA.frameIdx > 0xA9 → headA.kindFlags |= 0x100 +
 *           sub_8020CC4(obj,(u8)(posX+4),(u8)(posY-0x10),0x27C,0xA,0x315,4) → 0x1D。
 *   0x1D    headB 的 0x800 落 → Sfx(0x59,1,0) + 清 headA 0x100 → 0x1E。
 *   0x1E    headB.frameIdx==0xC8 时 Sfx(0x5A,1,0); headB 到 0x1000 → 清 headB 0x1000 +
 *           清 obj->state 0x2000; headA 到 0x1000 → 清 state 0x2000 + sub_801CE80 复位 → 0x1F。
 *   0x1F    headA 的 0x800 落 → 0x15。
 *   0x15    sub_8020CC4(obj,0x82,0x7D,0x1B4,0xA,0x30E,5) + headB.f_2A=0 → 0x16。
 *   0x16    headB 的 0x800 落 → Sfx(0x34,1,0) → 0x17。
 *   0x17    headB.frameIdx > 0xE → sub_804BF14(0,3,0x10,0x1C,0x1F,4,4,-1,2) + 计数=0 → 0x18。
 *   0x18    计数<=3 → 计数++; 否则 sub_804C728(0,3,0x10), 计数=0 → 0x19。
 *   0x19    headB.frameIdx==0x28 时 Sfx(0x5A,1,0); > 0x51 → Sfx(0x4E,2,1) +
 *           sub_804BF14(同0x17) + 计数=0 + sub_8044514(0x1E) → 0x1A。
 *   0x1A    计数<=3 → 计数++; 否则 sub_804C728(0,3,0x10), 计数=0 → 0x1B。
 *   0x1B    headB 到 0x1000 → Sfx_StopTrack(2) + obj->state &= 0xDFFF → 9。
 *   9       无其他演出占用 → result = 1。
 * 尾部: sub_803F658(obj) (无帧计数自增)。 */
u8 sub_802E6C8(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 1);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x1C;
            break;
        case 28:
            if (obj->headA.frameIdx <= 0xA9)
                break;
            obj->headA.kindFlags |= 0x100;
            sub_8020CC4(obj, (u8)(obj->posX + 4), (u8)(obj->posY - 0x10), 0x27C, 0xA, 0x315, 4);
            gObjActStep = 0x1D;
            break;
        case 29:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x59, 1, 0);
            obj->headA.kindFlags &= 0xFEFF;
            gObjActStep = 0x1E;
            break;
        case 30:
            if (obj->headB.frameIdx == 0xC8)
                Sfx_Play(0x5A, 1, 0);
            if (obj->headB.kindFlags & 0x1000)
            {
                obj->headB.kindFlags &= 0xEFFF;
                obj->state &= 0xDFFF;
            }
            if (obj->headA.kindFlags & 0x1000)
            {
                obj->state &= 0xDFFF;
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x1F;
            }
            break;
        case 31:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 0x15;
            break;
        case 21:
            sub_8020CC4(obj, 0x82, 0x7D, 0x1B4, 0xA, 0x30E, 5);
            obj->headB.f_2A = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x34, 1, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (obj->headB.frameIdx <= 0xE)
                break;
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            gObjActStep = 0x18;
            break;
        case 24:
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_804C728(0, 3, 0x10);
            gObjActStepTimer = 0;
            gObjActStep = 0x19;
            break;
        case 25:
            if (obj->headB.frameIdx == 0x28)
                Sfx_Play(0x5A, 1, 0);
            if (obj->headB.frameIdx <= 0x51)
                break;
            Sfx_Play(0x4E, 2, 1);
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            sub_8044514(0x1E);
            gObjActStep = 0x1A;
            break;
        case 26:
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_804C728(0, 3, 0x10);
            gObjActStepTimer = 0;
            gObjActStep = 0x1B;
            break;
        case 27:
            if (obj->headB.kindFlags & 0x1000)
            {
                Sfx_StopTrack(2);
                obj->state &= 0xDFFF;
                gObjActStep = 9;
            }
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802EAC4
INCLUDE_ASM("asm/nonmatchings", sub_802EAC4);
// @ 0x0802EDD8
/* 战斗对象"多段演出/蓄力"状态机 (与 sub_802E6C8 同族; 表项 sub_8020CC4/0x1B4 段 + sub_8044514 收尾)。
 * 唯一入口 = 0x0839CD5C 指针表 idx 0x2E (thumb 0x0802EDD9), 由 sub_803F444 家族分派器对 BattleObj 调用。
 *   case0   记 posX/posY/headA.f_1E/palSlot, sub_80444A4 清同组伤害 +
 *           sub_801CE80(obj,5,0x1B4,0xD,1) 切动画 + sub_803F5B4; 计数=0, f_B6=2, f_B4=0 → 0x1C。
 *   0x1C    headA.frameIdx > 0xA9 → headA.kindFlags |= 0x100 +
 *           sub_8020CC4(obj,(u8)(posX+4),(u8)(posY-0x10),0x27C,0xA,0x316,4) → 0x1D。
 *   0x1D    headB 的 0x800 落 → Sfx(0x59,1,0) + 清 headA 0x100 → 0x1E。
 *   0x1E    headB.frameIdx==0xC8 时 Sfx(0x5A,1,0); headB 到 0x1000 → 清 headB 0x1000 +
 *           清 obj->state 0x2000; headA 到 0x1000 → 清 state 0x2000 + sub_801CE80 复位 → 0x1F。
 *   0x1F    headA 的 0x800 落 → 0x15。
 *   0x15    sub_8020CC4(obj,0xBA,0x7D,0x1B4,0xA,0x312,5) + headB.f_2A=0 → 0x16。
 *   0x16    headB 的 0x800 落 → Sfx(0x55,1,0) → 0x17。
 *   0x17    headB.frameIdx > 0x13 → sub_8044514(0x1E) + 计数=0 → 0x18。
 *   0x18    headB 到 0x1000 → Sfx_StopTrack(1) + 清 state 0x2000 → 9; 每帧计数++。
 *   9       无其他演出占用 → result = 1; 否则计数++。
 * 尾部: sub_803F658(obj); return result。 */
u8 sub_802EDD8(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 1);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x1C;
            break;
        case 28:
            if (obj->headA.frameIdx <= 0xA9)
                break;
            obj->headA.kindFlags |= 0x100;
            sub_8020CC4(obj, (u8)(obj->posX + 4), (u8)(obj->posY - 0x10), 0x27C, 0xA, 0x316, 4);
            gObjActStep = 0x1D;
            break;
        case 29:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x59, 1, 0);
            obj->headA.kindFlags &= 0xFEFF;
            gObjActStep = 0x1E;
            break;
        case 30:
            if (obj->headB.frameIdx == 0xC8)
                Sfx_Play(0x5A, 1, 0);
            if (obj->headB.kindFlags & 0x1000)
            {
                obj->headB.kindFlags &= 0xEFFF;
                obj->state &= 0xDFFF;
            }
            if (obj->headA.kindFlags & 0x1000)
            {
                obj->state &= 0xDFFF;
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x1F;
            }
            break;
        case 31:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 0x15;
            break;
        case 21:
            sub_8020CC4(obj, 0xBA, 0x7D, 0x1B4, 0xA, 0x312, 5);
            obj->headB.f_2A = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x55, 1, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (obj->headB.frameIdx <= 0x13)
                break;
            sub_8044514(0x1E);
            gObjActStepTimer = 0;
            gObjActStep = 0x18;
            break;
        case 24:
            if (obj->headB.kindFlags & 0x1000)
            {
                Sfx_StopTrack(1);
                obj->state &= 0xDFFF;
                gObjActStep = 9;
            }
            gObjActStepTimer += 1;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            else
                gObjActStepTimer += 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802F100
u8 sub_802F100(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 1);
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x1C;
            break;
        case 28:
            if (obj->headA.frameIdx <= 0xA9)
                break;
            obj->headA.kindFlags |= 0x100;
            sub_8020CC4(obj, (u8)(obj->posX + 4), (u8)(obj->posY - 0x10), 0x27C, 0xA, 0x317, 4);
            gObjActStep = 0x1D;
            break;
        case 29:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x59, 1, 0);
            obj->headA.kindFlags &= 0xFEFF;
            gObjActStep = 0x1E;
            break;
        case 30:
            if (obj->headB.frameIdx == 0xC8)
                Sfx_Play(0x5A, 1, 0);
            if (obj->headB.kindFlags & 0x1000)
            {
                obj->headB.kindFlags &= 0xEFFF;
                obj->state &= 0xDFFF;
            }
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            obj->state &= 0xDFFF;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x1F;
            break;
        case 31:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x15;
            break;
        case 21:
            sub_8020CC4(obj, 0x82, 0x7D, 0x1B4, 0xA, 0x313, 5);
            obj->headB.f_2A = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x4E, 1, 1);
            gObjActStep = 0x17;
            break;
        case 23:
            if (obj->headB.frameIdx <= 0x18)
                break;
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            sub_8044514(0x32);
            gObjActStep = 0x18;
            break;
        case 24:
            if (gObjActStepTimer > 3)
            {
                gObjActStep = 0x19;
                sub_804C728(0, 3, 0x10);
                break;
            }
            gObjActStepTimer += 1;
            break;
        case 25:
            if (obj->headB.frameIdx == 0x5B)
                Sfx_StopTrack(1);
            if (obj->headB.kindFlags & 0x1000)
            {
                obj->state &= 0xDFFF;
                gObjActStep = 9;
            }
            gObjActStepTimer += 1;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802F480
u32 sub_802F480(BattleObj *arg)
{
    u32 ret;
    u16 keys;

    ret = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = arg->posX;
            gObjActSavedY = arg->posY;
            gObjActSavedF2A = arg->headA.f_1E;
            gObjActSavedPal = arg->headA.palSlot;
            sub_80444A4((BattleObj *)arg);
            sub_801CE80(arg, 5, 0x1B4, 0xC, 1);
            sub_803F5B4((BattleObj *)arg);
            gObjActStepTimer = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 2;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (arg->headA.frameIdx > 0x46)
            {
                sub_8020CC4(arg, 0xB9, 0x78, 0x2E0, 0xE, 0x2F9, 5);
                gObjActStep = 0x14;
            }
            break;
        case 20:
            keys = arg->headB.kindFlags & 0x800;
            if (keys != 0)
                break;
            sub_8044514(0x5A);
            gObjActStepTimer = keys;
            gObjActStep = 0x15;
            break;
        case 21:
            if (sub_80471AC() == 0)
                gObjActStep = 0x16;
            if ((arg->headA.kindFlags & 0x1000) == 0)
                break;
            arg->headA.kindFlags = (arg->headA.kindFlags & 0xEFFF) | 0x100;
            break;
        case 22:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_801CE80(arg, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (!(arg->headB.kindFlags & 0x1000))
                break;
            keys = arg->state & 0xDFFF;
            arg->state = keys;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                ret = 1;
            break;
    }
    sub_803F658((BattleObj *)arg);
    gObjActStepTimer += 1;
    return ret;
}
// @ 0x0802F6D8
/* 战斗对象"换装/形态切换"演出状态机 (gObjActStep: 0 起手 → 0x12..0x19 → 9)。
 * 单参; 用 headA 主头 + sub_804B96C/sub_804C4D8 做头盔/装备层切换, 中段 posX 从暂存向
 * 0xF0 横移, 末尾 pad_C1 从 posY 插值回 0。
 *   case0   记 posX/posY/f_1E/palSlot, 计数=0, sub_80444A4 清同组伤害 + sub_803F5B4 +
 *           sub_801CE80(obj,1,0x1B4,0xD,0) 切动画; f_B6=0x339, f_B4=0 → 0x12。
 *   0x12    headA 的 0x800 落 → Sfx(0x59,0,0) → 0x13。
 *   0x13    headA.frameIdx > 0x21 → headA.kindFlags |= 0x100 +
 *           sub_804B96C(palSlot,(u8)sub_801B954(&headA),0x10,0x1C,0x1F,4,4,-1,2) → 0x14。
 *   0x14    计数<=3 → 计数++; 否则 sub_804C4D8(palSlot,(u8)sub_801B954(&headA),0x10) +
 *           计数=0 → 0x15 + Sfx(0x64,1,0)。
 *   0x15    计数<=0xF: posX = sub_801768C(g8628, 0xF0-g8628, 0x10, 计数, 2); 计数++;
 *           否则 计数=0, pad_C1 = posY, posX = g8628, sub_8044514(0x28) → 0x16。
 *   0x16    无其他演出占用 → sub_801CE80 复位 + headA.kindFlags |= 0x100 → 0x17。
 *   0x17    headA 的 0x800 落 → 清 headA 0x100 → 0x18。
 *   0x18    Sfx_TrackBusy(1)==0 → 0x19。
 *   0x19    计数<=0x1D: pad_C1 = sub_801768C(posY, -posY, 0x1E, 计数, 2); 计数++;
 *           否则 计数=0 → 9。
 *   9       result = 1。
 * 尾部: sub_803F658(obj) (无帧计数自增)。 */
u8 sub_802F6D8(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActStepTimer = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            sub_801CE80(obj, 1, 0x1B4, 0xD, 0);
            obj->f_B6 = 0x339;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x59, 0, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (obj->headA.frameIdx <= 0x21)
                break;
            obj->headA.kindFlags |= 0x100;
            sub_804B96C(obj->headA.palSlot, (u8)sub_801B954(&obj->headA), 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStep = 0x14;
            break;
        case 20:
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_804C4D8(obj->headA.palSlot, (u8)sub_801B954(&obj->headA), 0x10);
            gObjActStepTimer = 0;
            gObjActStep = 0x15;
            Sfx_Play(0x64, 1, 0);
            break;
        case 21:
            if (gObjActStepTimer <= 0xF)
            {
                obj->posX = sub_801768C(gObjActSavedX, 0xF0 - gObjActSavedX, 0x10, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                obj->pad_C1 = obj->posY;
                obj->posX = gObjActSavedX;
                sub_8044514(0x28);
                gObjActStep = 0x16;
            }
            break;
        case 22:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                obj->headA.kindFlags |= 0x100;
                gObjActStep = 0x17;
            }
            break;
        case 23:
            if (!(obj->headA.kindFlags & 0x800))
            {
                obj->headA.kindFlags &= 0xFEFF;
                gObjActStep = 0x18;
            }
            break;
        case 24:
            if (Sfx_TrackBusy(1) == 0)
                gObjActStep = 0x19;
            break;
        case 25:
            if (gObjActStepTimer <= 0x1D)
            {
                obj->pad_C1 = sub_801768C(obj->posY, -obj->posY, 0x1E, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gObjActStep = 9;
            }
            break;
        case 26:
        case 27:
            break;
        case 9:
            result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x0802F9EC
u8 sub_802F9EC(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
            obj->headA.kindFlags |= 0x100;
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActMoveFromX = obj->posX;
            gObjActStep = 0x13;
            break;
        case 19:
            if (gObjActStepTimer <= 9)
            {
                obj->posX = sub_801768C(gObjActSavedX, 0x1E - gObjActSavedX, 0xA, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 0x14;
            break;
        case 20:
            if (gObjActStepTimer <= 0x13)
            {
                obj->headA.frameIdx = (obj->headA.frameIdx + 1) % sub_801B95C(&obj->headA) + 0x14;
                obj->posX = sub_801768C(gObjActMoveFromX, 0xF0 - gObjActMoveFromX, 0x14, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            obj->pad_C1 = obj->posY;
            obj->posX = gObjActSavedX;
            sub_8020CC4(obj, 0xF, 0x14, 0, 0xE, 0x33A, 7);
            obj->headB.f_2A = 2;
            sub_801A2AC(0x1F47, 0xA, 0xC);
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x64, 1, 1);
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if (obj->headB.frameIdx <= 0x1B)
                break;
            gObjActMoveFromY = 0;
            gObjActStepTimer = 0;
            sub_8044514(0x28);
            sub_804BF14(0, 3, 0x10, 0x1C, 0x1F, 4, 4, -1, 2);
            gObjActStep = 0x17;
            break;
        case 23:
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 0xD + 0x1C;
            if (gObjActStepTimer <= 3)
            {
                gObjActStepTimer += 1;
                break;
            }
            sub_804C728(0, 3, 0x20);
            gObjActStep = 0x18;
            break;
        case 24:
            if (gObjActMoveFromY <= 4)
            {
                obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 0xD + 0x1C;
                if ((u16)obj->headB.frameIdx == 0x1C)
                    gObjActMoveFromY += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 0x19;
            break;
        case 25:
            if (gObjActStepTimer <= 9)
            {
                obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 0xD + 0x1C;
                gSceneFadeOut = sub_801768C(0xA, -0xA, 0xA, gObjActStepTimer, 2);
                gSceneFadeIn = sub_801768C(0xC, 4, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
                break;
            }
            Sfx_StopTrack(1);
            sub_801A2AC(0, 0, 0);
            obj->state &= 0xDFFF;
            REG_DISPCNT &= 0xFDFF;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            obj->headA.kindFlags |= 0x100;
            gObjActStep = 0x1A;
            break;
        case 26:
            if (obj->headA.kindFlags & 0x800)
                break;
            obj->headA.kindFlags &= 0xFEFF;
            gObjActStepTimer = 0;
            gObjActStep = 0x1B;
            break;
        case 27:
            if (gObjActStepTimer <= 0x1D)
            {
                obj->pad_C1 = sub_801768C(obj->posY, -obj->posY, 0x1E, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}

// @ 0x0802FE98
u8 sub_802FE98(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            sub_80444A4(obj);
            sub_801CE80(obj, 5, 0x1B4, 0xC, 0);
            obj->headA.kindFlags |= 0x100;
            sub_803F5B4(obj);
            gObjActStepTimer = 0;
            obj->f_B6 = 2;
            obj->f_B4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (gObjActStepTimer <= 9)
            {
                obj->posX = sub_801768C(gObjActSavedX, 0x1E - gObjActSavedX, 0xA, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
                break;
            }
            gObjActMoveFromX = obj->posX;
            sub_801A348();
            sub_801A2AC(0x1F47, 0xF, 8);
            Sfx_Play(0x64, 1, 0);
            gObjActStep = 0x14;
            break;
        case 20:
            if (sub_8019B98(0xC, 3, 0xE, 2) == 0)
                break;
            sub_804BDD8(0xE, 1, 3, 1, 6);
            gObjActStepTimer = 0;
            sub_8020CC4(obj, 0x7D, 0x91, 0x27C, 0xD, 0x33B, 5);
            obj->headB.f_2A = 0;
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headB.kindFlags & 0x800)
                break;
            gObjActStep = 0x16;
            break;
        case 22:
            if (gObjActStepTimer <= 0x31)
            {
                obj->headA.frameIdx = (obj->headA.frameIdx + 1) % sub_801B95C(&obj->headA) + 0x14;
                obj->posX = sub_801768C(gObjActMoveFromX, 0xF0 - gObjActMoveFromX, 0x32, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            obj->pad_C1 = obj->posY;
            obj->posX = gObjActSavedX;
            obj->state &= 0xDFFF;
            sub_8044514(0x28);
            gObjActStep = 0x17;
            break;
        case 23:
            if (gObjActStepTimer <= 0x27)
            {
                gSceneFadeOut = sub_801768C(0xF, -0xF, 0x28, gObjActStepTimer, 2);
                gSceneFadeIn = sub_801768C(8, 8, 0x28, gObjActStepTimer, 2);
                sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
                break;
            }
            sub_801A2AC(0, 0, 0);
            sub_804BE90(0xE, 1);
            REG_DISPCNT &= 0xFDFF;
            sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            obj->headA.kindFlags |= 0x100;
            gObjActStep = 0x18;
            break;
        case 24:
            if (obj->headA.kindFlags & 0x800)
                break;
            obj->headA.kindFlags &= 0xFEFF;
            gObjActStepTimer = 0;
            gObjActStep = 0x19;
            break;
        case 25:
            if (gObjActStepTimer <= 0x1D)
            {
                obj->pad_C1 = sub_801768C(obj->posY, -obj->posY, 0x1E, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
        case 26:
        case 27:
            break;
    }
    sub_803F658(obj);
    return result;
}

// @ 0x0803029C
u32 sub_803029C(BattleObj *arg0, BattleObj *arg1)
{
    u32 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = arg0->posX;
            gObjActSavedY = arg0->posY;
            gObjActSavedF2A = arg0->headA.f_1E;
            gObjActSavedPal = arg0->headA.palSlot;
            gObjActStepTimer = 0;
            sub_80444A4(arg0);
            sub_803F5B4(arg0);
            arg0->f_B6 = 0;
            arg0->f_B4 = 0;
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (gObjActStepTimer <= 0x13)
            {
                arg0->posY = sub_801768C(gObjActSavedY, arg1->posY - gObjActSavedY, 0x14, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
                break;
            }
            arg0->posY = arg1->posY;
            sub_801CE80(arg0, 1, 0x1B4, 0xD, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (arg0->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x14;
            break;
        case 20:
            if (arg0->headA.frameIdx <= 7)
                break;
            gObjActMoveFromX = arg0->posX + 4;
            gObjActMoveFromY = arg0->posY + 4;
            sub_8020CC4(arg0, gObjActMoveFromX, gObjActMoveFromY, 0x27C, 0xE, 0x332, 5);
            arg0->headA.kindFlags |= 0x100;
            arg0->headB.kindFlags |= 0x100;
            gObjActStep = 0x15;
            break;
        case 21:
            if (arg0->headB.kindFlags & 0x800)
                break;
            arg0->headA.kindFlags &= 0xFEFF;
            gObjActStep = 0x16;
            break;
        case 22:
            arg0->headB.frameIdx = (arg0->headB.frameIdx + 1) % 0x1F;
            if (arg0->headA.frameIdx <= 0x2E)
                break;
            gObjActStepTimer = 0;
            arg0->headB.frameIdx = 0x20;
            gObjActStep = 0x17;
            break;
        case 23:
            if (gObjActStepTimer <= 0x18)
            {
                if (gObjActStepTimer == 0xF)
                    sub_8044514(0x1E);
                arg0->headB.f_2B = sub_801768C(gObjActMoveFromX, 0xFA - gObjActMoveFromX, 0x19, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                arg0->state &= 0xDFFF;
                gObjActStep = 0x18;
            }
            if (arg0->headA.kindFlags & 0x1000)
                arg0->headA.kindFlags = (arg0->headA.kindFlags & 0xEFFF) | 0x100;
            break;
        case 24:
            if (arg0->headA.kindFlags & 0x100)
            {
                sub_801CE80(arg0, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x19;
            }
            else if (arg0->headA.kindFlags & 0x1000)
            {
                arg0->headA.kindFlags &= 0xEFFF;
                sub_801CE80(arg0, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x19;
            }
            break;
        case 25:
            if (arg0->headA.kindFlags & 0x800)
                break;
            gObjActStepTimer = 0;
            gObjActStep = 0x1A;
            break;
        case 26:
            if (gObjActStepTimer <= 0x13)
            {
                arg0->posY = sub_801768C(arg1->posY, gObjActSavedY - arg1->posY, 0x14, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
                break;
            }
            arg0->posY = gObjActSavedY;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
        case 27:
        case 28:
        case 29:
        case 30:
            break;
    }
    sub_803F658(arg0);
    return result;
}

// @ 0x08030664
/* 战斗对象"双头换装/形态切换"演出状态机 (gObjActStep: 0 起手 → 0x12..0x19 → 9)。
 * 单参; 用 headA/headB 双头做形态切换: case20 同时置两头 0x100 并由锚点播 sub_8020CC4,
 * case21 等两头 0x800 落后清 0x100, case22 用 headB.frameIdx 循环 + headA.frameIdx 阈值
 * 推进, case23 把 headB.f_2B 从 gObjActMoveFromX 向 0xFA 插值。
 *   case0   记 posX/posY/f_1E/palSlot, 计数=0, sub_80444A4 清同组伤害 + sub_803F5B4;
 *           f_B6=0, f_B4=0, 计数=0 → 0x12。
 *   0x12    sub_801CE80(obj,1,0x1B4,0xD,0) 切动画 → 0x13。
 *   0x13    headA 的 0x800 落 → 0x14。
 *   0x14    headA.frameIdx > 7 → g86E=posX+4, g86F=posY+4,
 *           sub_8020CC4(obj,g86E,g86F,0x27C,0xE,0x334,5); 两头 kindFlags |= 0x100 → 0x15。
 *   0x15    headB 的 0x800 落 → 两头 kindFlags &= 0xFEFF → 0x16。
 *   0x16    headB.frameIdx = (headB.frameIdx+1) % 0x1F; headA.frameIdx > 0x2E 时
 *           计数=0, headB.frameIdx=0x28, headB.kindFlags |= 0x100 → 0x17。
 *   0x17    计数<=0x18: 计数==0xF 时 sub_8044514(0x1E);
 *           headB.f_2B = sub_801768C(g86E, 0xFA-g86E, 0x19, 计数, 2); 计数++;
 *           否则 state &= 0xDFFF → 0x18; 之后 headA 到 0x1000 → 清位并置 0x100。
 *   0x18    headA 的 0x100 落 → sub_801CE80 复位 → 0x19;
 *           否则 headA 到 0x1000 → 清 0x1000 + sub_801CE80 复位 → 9。
 *   9       无其他演出占用 → result = 1。
 * 尾部: sub_803F658(obj) (无帧计数自增)。 */
u8 sub_8030664(BattleObj *obj)
{
    u8 result;

    result = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedPal = obj->headA.palSlot;
            gObjActStepTimer = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            obj->f_B6 = 0;
            obj->f_B4 = 0;
            gObjActStepTimer = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            sub_801CE80(obj, 1, 0x1B4, 0xD, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headA.frameIdx > 7)
            {
                gObjActMoveFromX = obj->posX + 4;
                gObjActMoveFromY = obj->posY + 4;
                sub_8020CC4(obj, gObjActMoveFromX, gObjActMoveFromY, 0x27C, 0xE, 0x334, 5);
                obj->headA.kindFlags |= 0x100;
                obj->headB.kindFlags |= 0x100;
                gObjActStep = 0x15;
            }
            break;
        case 21:
            if (obj->headB.kindFlags & 0x800)
                break;
            obj->headA.kindFlags &= 0xFEFF;
            obj->headB.kindFlags &= 0xFEFF;
            gObjActStep = 0x16;
            break;
        case 22:
            obj->headB.frameIdx = (obj->headB.frameIdx + 1) % 0x1F;
            if (obj->headA.frameIdx <= 0x2E)
                break;
            gObjActStepTimer = 0;
            obj->headB.frameIdx = 0x28;
            obj->headB.kindFlags |= 0x100;
            gObjActStep = 0x17;
            break;
        case 23:
            if (gObjActStepTimer <= 0x18)
            {
                if (gObjActStepTimer == 0xF)
                    sub_8044514(0x1E);
                obj->headB.f_2B = sub_801768C(gObjActMoveFromX, 0xFA - gObjActMoveFromX, 0x19, gObjActStepTimer, 2);
                gObjActStepTimer += 1;
            }
            else
            {
                obj->state &= 0xDFFF;
                gObjActStep = 0x18;
            }
            if (obj->headA.kindFlags & 0x1000)
                obj->headA.kindFlags = (obj->headA.kindFlags & 0xEFFF) | 0x100;
            break;
        case 24:
            if (obj->headA.kindFlags & 0x100)
            {
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 0x19;
            }
            else if (obj->headA.kindFlags & 0x1000)
            {
                obj->headA.kindFlags &= 0xEFFF;
                sub_801CE80(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 9;
            }
            break;
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                result = 1;
            break;
    }
    sub_803F658(obj);
    return result;
}
// @ 0x080309B0
u32 sub_80309B0(BattleObj *arg)
{
    u32 ret;
    u16 keys;

    ret = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = arg->posX;
            gObjActSavedY = arg->posY;
            gObjActSavedF2A = arg->headA.f_1E;
            gObjActSavedPal = arg->headA.palSlot;
            sub_80444A4((BattleObj *)arg);
            sub_801CE80(arg, 5, 0x1B4, 0xC, 1);
            sub_803F5B4((BattleObj *)arg);
            gObjActStepTimer = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 2;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0;
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (arg->headA.frameIdx > 0x46)
            {
                sub_8020CC4(arg, 0xB9, 0x78, 0x2E0, 0xE, 0x2F9, 5);
                gObjActStep = 0x14;
            }
            break;
        case 20:
            keys = arg->headB.kindFlags & 0x800;
            if (keys != 0)
                break;
            sub_8044514(0x5A);
            gObjActStepTimer = keys;
            gObjActStep = 0x15;
            break;
        case 21:
            if (sub_80471AC() == 0)
                gObjActStep = 0x16;
            if ((arg->headA.kindFlags & 0x1000) == 0)
                break;
            arg->headA.kindFlags = (arg->headA.kindFlags & 0xEFFF) | 0x100;
            break;
        case 22:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_801CE80(arg, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
            break;
        case 23:
            if (!(arg->headB.kindFlags & 0x1000))
                break;
            keys = arg->state & 0xDFFF;
            arg->state = keys;
            gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
                ret = 1;
            break;
    }
    sub_803F658((BattleObj *)arg);
    gObjActStepTimer += 1;
    return ret;
}
// @ 0x08030C08
u8 sub_8030C08(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            sub_801A2AC(0, 0, 0);
            obj->headA.kindFlags &= 0xFFEF;
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            obj->slot = 0xFF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08030D9C
u8 sub_8030D9C(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->slot = 0xFF;
            obj->headA.kindFlags &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08030F30
u8 sub_8030F30(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->slot = 0xFF;
            obj->headA.kindFlags &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x080310C4
u8 sub_80310C4(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->slot = 0xFF;
            obj->headA.kindFlags &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x08031258
u8 sub_8031258(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->slot = 0xFF;
            obj->headA.kindFlags &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
// @ 0x080313EC
u8 sub_80313EC(BattleObj *obj)
{
    u8 result;
    u16 f2a;
    u16 f2b;

    result = 0;
    switch (gSceneTransStep)
    {
        case 0:
            gObjActStepTimer = 0;
            gSceneFadeOut = 0x10;
            gSceneFadeIn = 0;
            f2a = obj->headA.kindFlags | 0x10;
            obj->headA.kindFlags = f2a;
            sub_801A2AC(0x710, 0x10, 0);
            Sfx_Play(0x5A, 0, 0);
            gSceneTransStep = 1;
            break;
        case 1:
            if (gObjActStepTimer <= 9)
            {
                gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, gSceneFadeIn);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                gSceneTransStep = 2;
            }
            break;
        case 2:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x710, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
            }
            else
            {
                gObjActStepTimer = 0;
                f2b = obj->headA.kindFlags | 0x200;
                obj->headA.kindFlags = f2b;
                gSceneTransStep = 3;
            }
            break;
        case 3:
            Sfx_StopTrack(0);
            gSceneTransStep = 0x15;
            break;
        case 0x15:
            sub_801A2AC(0, 0, 0);
            obj->slot = 0xFF;
            obj->headA.kindFlags &= 0xFFEF;
            result = 1;
            break;
    }
    return result;
}
