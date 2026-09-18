#include "battle_types.h"
#include "battle_stage_dialogue.h"
#include "battle_flow_rules.h"
#include "battle_object_engine.h"
#include "battle_palette_wipe.h"
#include "battle_stage_actor.h"
#include "battle_stage_state.h"
#include "battle_task_services.h"
#include "sound.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"

// @ 0x08032548
// NPC 对话状态机 (gObjActStep: 0=开始 1=移动 2=等待 3=按键 5=选择 8=收尾 9=结束):
// case0 存 NPC 位置 (0x03000828/29) 并初始化; case1 播放对话开场动画 (0x368/0x359 按 arg0[0xBE]);
// case3 按键 0x21/0x7C/0x90 分发音效/推进; case5 处理选择确认; case9 无遮挡时结束对话返回 1。
// 注: case0 的 b4/zero 双零变量与 case1 的 keys/b4 写法是字节匹配必需的调度形状 (见 progress.md)。
u32 sub_8032548(BattleObj *arg0, u8 *arg1)
{
    u32 ret;
    u16 b4;
    int keys;
    u32 zero;
    u16 *b6ptr;
    u32 zero2;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg0->posX;
        gObjActSavedY = arg0->posY;
        gObjActStepTimer = 0;
        sub_80444A4((BattleObj *)arg0);
        sub_803F5B4((BattleObj *)arg0);
        zero = 0;
        zero2 = 0;
        b6ptr = &(arg0->f_B6);
        b4 = zero2;
        *b6ptr = zero2;
        arg0->f_B4 = b4;
        gObjActStep = 1;
        gObjActParam = zero;
        break;
    case 1:
        if (sub_803E58C(arg0, arg1, 0) == 1)
        {
            keys = arg0->slot;
            b4 = 0x368;
            if (keys == 0)
            {
                b4 = b4 - 0xF;
            }
            sub_8020974((ObjHead *)(&arg0->headA), b4, 0x1B4, 0xD, 2);
            if (arg1[0xBE] <= 0xA)
            {
                keys = arg0->headA.kindFlags | 0x20;
                arg0->headA.kindFlags = keys;
            }
            gObjActStep = 2;
        }
        break;
    case 2:
        if (arg0->headA.kindFlags & 0x800)
        {
            break;
        }
        gObjActStep = 3;
        break;
    case 3:
        if (arg0->headA.frameIdx == 0x21)
        {
            Sfx_Play(0x31, 1, 1);
            sub_8044514(0x5E);
            break;
        }
        if (arg0->headA.frameIdx == 0x7C)
        {
            Sfx_StopTrack(1);
            break;
        }
        if (arg0->headA.frameIdx != 0x90)
        {
            break;
        }
        sub_8044514(0x28);
        gObjActStep = 5;
        break;
    case 5:
        if (arg0->headA.kindFlags & 0x1000)
        {
            sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
            gObjActParam = 0xC;
            gObjActStep = 8;
        }
        break;
    case 8:
        if (sub_803E58C(arg0, arg1, 0) == 1)
        {
            gObjActStep = 9;
        }
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            sub_8045B90(arg0, arg0->pad_A1);
            ret = 1;
        }
        break;
    }
    sub_803F658((BattleObj *)arg0);
    return ret;
}
// @ 0x0803272C
// NPC 对话状态机变体 (战斗型对话, gObjActStep 同 8032548 十态):
// case0 存位+初始化; case1 sub_803ED34 到位检查+开场动画 (b4=0x36B/-0xF) + 写 0x35E 到 [0xB6];
// case2 等 0x800 后 Sfx+窗口设置 (sub_804BF14 9 参); case3 gObjActStepTimer<=3 时 sub_804C728;
// case5 确认 (0x1000) → sub_804C3A4; case8/9 收尾同 8032548。尾 sub_803F658。
// 注: kind/flagval/b6val 独立载体变量与 case1 三次 def 拆分是字节匹配必需 (见 progress.md)。
u32 sub_803272C(BattleObj *arg0, u8 *arg1)
{
    u32 ret;
    u16 b4;
    u16 kind;
    int keys;
    u16 flags;
    u16 flagval;
    u16 zero2;
    u16 *b6ptr;
    u16 b6val;

    ret = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg0->posX;
        gObjActSavedY = arg0->posY;
        gObjActStepTimer = 0;
        sub_80444A4((BattleObj *)arg0);
        sub_803F5B4((BattleObj *)arg0);
        gObjActStep = 1;
        gObjActParam = 0;
        break;
    case 1:
        if (sub_803ED34(arg0, arg1, 0) == 1)
        {
            kind = arg0->slot;
            b4 = 0x36B;
            if (kind == 0)
                b4 = b4 - 0xF;
            sub_8020974((ObjHead *)(&arg0->headA), b4, 0x1B4, 0xD, 2);
            b6ptr = &(arg0->f_B6);
            zero2 = 0;
            b6val = 0x35E;
            *b6ptr = b6val;
            arg0->f_B4 = zero2;
            if (arg1[0xBE] <= 0xA)
            {
                keys = arg0->headA.kindFlags;
                flagval = 0x20;
                keys = keys | flagval;
                arg0->headA.kindFlags = keys;
            }
            gObjActStep = 2;
        }
        break;
    case 2:
        flags = arg0->headA.kindFlags & 0x800;
        if (flags != 0)
            break;
        Sfx_Play(0x4F, 1, 0);
        sub_8044514(0x28);
        sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
        gObjActStepTimer = 0;
        gObjActStep = 3;
        break;
    case 3:
        if (gObjActStepTimer > 3)
            break;
        sub_804C728(0, 3, 0x10);
        gObjActStep = 5;
        break;
    case 5:
        flags = arg0->headA.kindFlags & 0x1000;
        if (flags == 0)
            break;
        flags = arg0->headA.palSlot;
        sub_804C3A4(flags, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
        gObjActParam = 0xC;
        gObjActStep = 8;
        break;
    case 8:
        if (sub_803E58C(arg0, arg1, 0) == 1)
            gObjActStep = 9;
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            sub_8045B90(arg0, arg0->pad_A1);
            ret = 1;
        }
        break;
    }
    sub_803F658((BattleObj *)arg0);
    return ret;
}
// @ 0x08032948
/* 战斗对话/演出状态机变体 (gObjActStep: 0 → 1 → 2 → 0x12..0x1B → 9)。
 * 与 sub_8032548/sub_803272C 同族双参 (arg1 = 关联对话对象), 但走多段动画装载:
 *   case0   存 posX/posY, 计数=0, sub_80444A4 + sub_803F5B4; f_B6=f_B4=0 → 1; gObjActParam=0。
 *   case1   sub_803E58C(arg0,arg1,3)==1 → b4 = slot==0 ? 0x35F : 0x36E;
 *           sub_8020974(&headA, b4, 0x1B4, 0xD, 2); arg1[0xBE]<=0xA → headA.kindFlags|=0x20 → 2。
 *   case2   headA 0x800 落 → 0x12。
 *   0x12    headA 0x1000 → sub_804C3A4(palSlot,(u8)sub_801B954(&headA)); headA.kindFlags|=0x100;
 *           计数=0; Sfx(0x31,1,0); sub_804BF14(0,3,7,0xE,0x1C,4,4,-1,2) → 0x13。
 *   0x13    计数>3 → 停; 否则 sub_804C728(0,3,0x10) → 0x14。
 *   0x14    b4 = slot==0 ? 0x360 : 0x36F; sub_8020974(&headA, b4, 0x1B4, 0xD, 0x402) → 0x15。
 *   0x15    headA 0x800 落 → 计数=0; sub_804BF14(同 0x12); → 0x16; Sfx(0x31,1,1)。
 *   0x16    计数<=0x13: 计数==4 时 sub_804C728(0,3,0x10);
 *           posX = sub_801768C(gObjActMoveFromX, -0x70, 0x14, 计数, 2); 计数++;
 *           否则 Sfx_StopTrack(1) → 0x17。
 *   0x17    b4 = slot==0 ? 0x361 : 0x370; sub_8020974(&headA, b4, 0x1B4, 0xD, 2) → 0x18。
 *   0x18    headA 0x800 落 → sub_8044514(0x14); 计数=0 → 0x19。
 *   0x19    headA 0x1000 → sub_804C3A4(palSlot, sub_801B954(&headA)); headA.kindFlags|=0x100;
 *           计数=0; gObjActMoveFromX = posX;
 *           gObjActSavedX = gUnk_08393A48[memberIdx]; gObjActSavedY = gUnk_08393A4D[memberIdx] → 0x1A。
 *   0x1A    无其他演出占用 → gObjActParam=0xC → 0x1B; 否则计数++。
 *   0x1B    sub_803E58C(arg0,arg1,0)==1 → 9。
 *   9       sub_8045B90(arg0, arg0->pad_A1); result=1。
 * 尾部 sub_803F658(arg0)。
 * 注: case1/20/23 的 b4 必须写成 `kind == 0 ? X : Y` 三目 (等价 if 会少 2 条 staging 拷贝, 差 4B)。 */
extern u8 gUnk_08393A48[];
extern u8 gUnk_08393A4D[];
u32 sub_8032948(BattleObj *arg0, u8 *arg1)
{
    u32 ret;
    u16 b4;
    u16 kind;
    int keys;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg0->posX;
        gObjActSavedY = arg0->posY;
        gObjActStepTimer = 0;
        sub_80444A4(arg0);
        sub_803F5B4(arg0);
        arg0->f_B6 = 0;
        arg0->f_B4 = 0;
        gObjActStep = 1;
        gObjActParam = 0;
        break;
    case 1:
        if (sub_803E58C(arg0, arg1, 3) == 1)
        {
            kind = arg0->slot;
            b4 = kind == 0 ? 0x35F : 0x36E;
            sub_8020974((ObjHead *)(&arg0->headA), b4, 0x1B4, 0xD, 2);
            if (arg1[0xBE] <= 0xA)
            {
                keys = arg0->headA.kindFlags | 0x20;
                arg0->headA.kindFlags = keys;
            }
            gObjActStep = 2;
        }
        break;
    case 2:
        if (arg0->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x12;
        break;
    case 18:
        if (!(arg0->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
        arg0->headA.kindFlags |= 0x100;
        gObjActStepTimer = 0;
        Sfx_Play(0x31, 1, 0);
        sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
        gObjActStep = 0x13;
        break;
    case 19:
        if (gObjActStepTimer > 3)
            break;
        sub_804C728(0, 3, 0x10);
        gObjActStep = 0x14;
        break;
    case 20:
        kind = arg0->slot;
        b4 = kind == 0 ? 0x360 : 0x36F;
        sub_8020974((ObjHead *)(&arg0->headA), b4, 0x1B4, 0xD, 0x402);
        gObjActStep = 0x15;
        break;
    case 21:
        if (arg0->headA.kindFlags & 0x800)
            break;
        gObjActStepTimer = 0;
        sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
        gObjActStep = 0x16;
        Sfx_Play(0x31, 1, 1);
        break;
    case 22:
        if (gObjActStepTimer <= 0x13)
        {
            if (gObjActStepTimer == 4)
                sub_804C728(0, 3, 0x10);
            arg0->posX = sub_801768C(gObjActMoveFromX, -0x70, 0x14, gObjActStepTimer, 2);
            gObjActStepTimer += 1;
        }
        else
        {
            Sfx_StopTrack(1);
            gObjActStep = 0x17;
        }
        break;
    case 23:
        kind = arg0->slot;
        b4 = kind == 0 ? 0x361 : 0x370;
        sub_8020974((ObjHead *)(&arg0->headA), b4, 0x1B4, 0xD, 2);
        gObjActStep = 0x18;
        break;
    case 24:
        if (arg0->headA.kindFlags & 0x800)
            break;
        sub_8044514(0x14);
        gObjActStepTimer = 0;
        gObjActStep = 0x19;
        break;
    case 25:
        if (!(arg0->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
        arg0->headA.kindFlags |= 0x100;
        gObjActStepTimer = 0;
        gObjActMoveFromX = arg0->posX;
        gObjActSavedX = gUnk_08393A48[arg0->memberIdx];
        gObjActSavedY = gUnk_08393A4D[arg0->memberIdx];
        gObjActStep = 0x1A;
        break;
    case 26:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            gObjActParam = 0xC;
            gObjActStep = 0x1B;
        }
        else
        {
            gObjActStepTimer += 1;
        }
        break;
    case 27:
        if (sub_803E58C(arg0, arg1, 0) == 1)
            gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(arg0, arg0->pad_A1);
        ret = 1;
        break;
    }
    sub_803F658(arg0);
    return ret;
}
// @ 0x08032D74
u8 sub_8032D74(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            sub_8048B30(0, 0x1E, obj->slot == 0 ? 0x362 : 0x371);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_8047B1C(obj) == 1)
                gObjActStep = 0x14;
            break;
        case 20:
            sub_801CBA4((BattleObj *)obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 6;
            break;
        case 6:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(obj, obj->pad_A1);
            result = 2;
            break;
    }
    return result;
}
// @ 0x08032EA0
// 0x0839D4CC 动作表项 6 演出状态机 (sub_8033E2C 姊妹变体, "我方全体"版):
// case0 存 posX/posY/palSlot/f_1E, sub_80489E8 mode0 登记我方候选槽+数量, 步进至 0x26 (38);
// case38-46 前置动画 0x374/0x375/0x376 与音效, sub_801CBA4 恢复调色板, 复位场景 0x1747;
// case18-23 sub_8019B98(0,3,0xE,1) 后 9 参窗口 sub_804BF14, 双向插值淡出淡入 + BattleUiFlag(0x20);
// case23 末尾对候选首槽 sub_8020CC4 锚点动画 0x37E;
// case24 把首槽 headB 动画逐项复制到其余候选 (state|=0x2000, headB.f_2B/2C=各自 posX/posY);
// case25 首槽 headB 0x1000 落 → 恢复调色板并对全体候选 state=(state&0xDFFF)|0x1000 → 9;
// case9 复位 (sub_801A2AC/sub_8045B90) 返回 2; 尾部统一 sub_803F658。
// 字节匹配要点: (1) case23 自对象须用嵌套局部 `obj` 且 case24/25 门控直接内联取址,
// 否则多出 adds 拷贝 (差 6 指令); (2) case25 state 须拆成两条 &=/|= 语句, 否则跨跳
// 合并选中 20/22 而非 19/23, 文本尺寸 1560→1564。
u32 sub_8032EA0(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    BattleObj *ptr;
    u16 keys;

    ret = 0;
    pool = GetObjPool();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg->posX;
        gObjActSavedY = arg->posY;
        gObjActSavedPal = arg->headA.palSlot;
        gObjActSavedF2A = arg->headA.f_1E;
        gObjActStepTimer = 0;
        gTargetSlotCount = sub_80489E8((BattleObj *)pool, gTargetSlotList, 0, 0x7F);
        gObjActStep = 0x26;
        break;
    case 38: // 0x26
        sub_8020974((ObjHead *)(&arg->headA), 0x374, 0x1B4, 0xD, 2);
        gObjActStep = 0x27;
        break;
    case 39: // 0x27
        if (arg->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x31, 0, 0);
        gObjActStep = 0x28;
        break;
    case 40: // 0x28
        if (arg->headA.frameIdx == 0x3A)
            Sfx_Play(0x58, 0, 0);
        if (arg->headA.frameIdx == 0x87)
            Sfx_Play(0x31, 0, 0);
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_8020974((ObjHead *)(&arg->headA), 0x375, 0x1B4, 0xD, 2);
        gObjActStep = 0x29;
        break;
    case 41: // 0x29
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2A;
        break;
    case 42: // 0x2A
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_8020974((ObjHead *)(&arg->headA), 0x376, 0x1B4, 0xD, 2);
        gObjActStep = 0x2B;
        break;
    case 43: // 0x2B
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2C;
        break;
    case 44: // 0x2C
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_801CBA4(arg, 0, gObjActSavedF2A, gObjActSavedPal, 0);
        gObjActStep = 0x2D;
        break;
    case 45: // 0x2D
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2E;
        break;
    case 46: // 0x2E
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1747, 0, 0x10);
        gObjActStep = 0x12;
        break;
    case 18: // 0x12
        if (sub_8019B98(0, 3, 0xE, 1) != 0)
        {
            gObjActStepTimer = 0;
            sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
            Sfx_Play(0x20, 0, 0);
            gObjActStep = 0x13;
        }
        break;
    case 19: // 0x13
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
        gObjActStep = 0x14;
        break;
    case 20: // 0x14
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x15;
        break;
    case 21: // 0x15
        if (gObjActStepTimer <= 0x45)
        {
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x16;
        break;
    case 22: // 0x16
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x17;
        break;
    case 23: // 0x17
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        BattleUiFlag_Set(0x20);
        {
            BattleObj *obj = (BattleObj *)(pool + gTargetSlotList[0] * 0xC8);
            sub_8020CC4(obj, obj->posX, obj->posY, 0x1B4, 0xE, 0x37E, 4);
        }
        gObjActStep = 0x18;
        break;
    case 24: // 0x18
        if (((BattleObj *)(pool + gTargetSlotList[0] * 0xC8))->headB.kindFlags & 0x800)
            break;
        for (i = 1; i < gTargetSlotCount; i++)
        {
            ptr = (BattleObj *)(pool + gTargetSlotList[i] * 0xC8);
            ptr->headB = ((BattleObj *)(pool + gTargetSlotList[0] * 0xC8))->headB;
            ptr->state |= 0x2000;
            ptr->headB.f_2B = ptr->posX;
            ptr->headB.f_2C = ptr->posY;
        }
        Sfx_Play(0x58, 0, 0);
        gObjActStep = 0x19;
        break;
    case 25: // 0x19
        if (!(((BattleObj *)(pool + gTargetSlotList[0] * 0xC8))->headB.kindFlags & 0x1000))
            break;
        sub_804C3A4(arg->headB.palSlot, sub_801B954((ObjHead *)(&arg->headB)));
        for (i = 0; i < gTargetSlotCount; i++)
        {
            ptr = (BattleObj *)(pool + gTargetSlotList[i] * 0xC8);
            ptr->state = ptr->state & 0xDFFF;
            ptr->state = ptr->state | 0x1000;
        }
        gObjActStep = 9;
        break;
    case 9:
        sub_801A2AC(0, 0, 0);
        sub_8045B90(arg, arg->pad_A1);
        ret = 2;
        break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x080334B8
// 0x0839D4CC 动作表项演出状态机 (sub_8032EA0/sub_8033988 的姊妹变体):
// case0 备份 posX/posY/palSlot/f_1E, sub_80444A4 + sub_803F5B4 复位, 步进至 0x26 (38);
// case38-46 前置动画 0x374/0x375/0x376 与音效, sub_801CBA4 调色板恢复, 复位场景 0x1747;
// case18-23 sub_8019B98(1,3,0xE,1) 后 9 参窗口 sub_804BF14, 双向插值淡出淡入 + BattleUiFlag(0x20),
// 并设置 obj->f_B6=0xAC / f_B4=0 与 sub_8044514(0x28);
// case24 三个等待闩 (gActWaitBusy0/1/2) 全清才步进 9, 否则步计数 ++;
// case9 复位 (sub_801A2AC/sub_8045B90) 返回 1; 尾部统一 sub_803F658。
// 字节匹配要点: 目标序言 `bl GetObjPool` 的返回值未被使用 (同族模板复刻残留), 需保留
// 该调用; 未使用的 `u8 values[8]` 维持 sub sp,#0x1c 帧宽 (同 sub_8033988)。
u32 sub_80334B8(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 values[8];

    ret = 0;
    pool = GetObjPool();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg->posX;
        gObjActSavedY = arg->posY;
        gObjActSavedPal = arg->headA.palSlot;
        gObjActSavedF2A = arg->headA.f_1E;
        gObjActStepTimer = 0;
        sub_80444A4(arg);
        sub_803F5B4(arg);
        gObjActStep = 0x26;
        break;
    case 38: // 0x26
        sub_8020974((ObjHead *)(&arg->headA), 0x374, 0x1B4, 0xD, 2);
        gObjActStep = 0x27;
        break;
    case 39: // 0x27
        if (arg->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x31, 0, 0);
        gObjActStep = 0x28;
        break;
    case 40: // 0x28
        if (arg->headA.frameIdx == 0x3A)
            Sfx_Play(0x58, 0, 0);
        if (arg->headA.frameIdx == 0x87)
            Sfx_Play(0x31, 0, 0);
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_8020974((ObjHead *)(&arg->headA), 0x375, 0x1B4, 0xD, 2);
        gObjActStep = 0x29;
        break;
    case 41: // 0x29
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2A;
        break;
    case 42: // 0x2A
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_8020974((ObjHead *)(&arg->headA), 0x376, 0x1B4, 0xD, 2);
        gObjActStep = 0x2B;
        break;
    case 43: // 0x2B
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2C;
        break;
    case 44: // 0x2C
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_801CBA4(arg, 0, gObjActSavedF2A, gObjActSavedPal, 0);
        gObjActStep = 0x2D;
        break;
    case 45: // 0x2D
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2E;
        break;
    case 46: // 0x2E
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1747, 0, 0x10);
        gObjActStep = 0x12;
        break;
    case 18: // 0x12
        if (sub_8019B98(1, 3, 0xE, 1) != 0)
        {
            gObjActStepTimer = 0;
            sub_804BF14(0, 3, 0x1C, 0xE, 7, 4, 4, -1, 2);
            Sfx_Play(0x20, 0, 0);
            gObjActStep = 0x13;
        }
        break;
    case 19: // 0x13
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
        gObjActStep = 0x14;
        break;
    case 20: // 0x14
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x15;
        break;
    case 21: // 0x15
        if (gObjActStepTimer <= 0x45)
        {
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x16;
        break;
    case 22: // 0x16
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x17;
        break;
    case 23: // 0x17
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        BattleUiFlag_Set(0x20);
        arg->f_B6 = 0xAC;
        arg->f_B4 = 0;
        sub_8044514(0x28);
        gObjActStep = 0x18;
        break;
    case 24: // 0x18
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            gObjActStep = 9;
        else
            gObjActStepTimer += 1;
        break;
    case 9:
        sub_801A2AC(0, 0, 0);
        sub_8045B90(arg, arg->pad_A1);
        ret = 1;
        break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x08033988
// 0x0839D4CC 同族演出状态机 (sub_8033E2C 的姊妹变体):
// case0 备份 posX/posY/palSlot/f_1E, sub_8020DE4 清零, 步进至 0x26 (38);
// case38-46 前置动画 0x374/0x375/0x376 与音效, sub_801CBA4 调色板恢复, 复位场景 0x1747;
// case18-23 sub_8019B98(2,3,0xE,1) 后 9 参窗口 sub_804BF14, 双向插值淡出淡入 + BattleUiFlag(0x20);
// case24 sub_801EEE4(arg, pool, 1, 0, 0x3E7) == 1 后步进至 case9;
// case9 复位 (sub_801A2AC/sub_8045B90) 返回 2; 尾部统一 sub_803F658。
u32 sub_8033988(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    // 8 字节数组在本函数中未被使用, 但 GCC2 仍为其保留栈帧 (对应目标 sub sp,#0x1c);
    // 同族 sub_8032EA0/sub_80334B8 同帧宽, 是状态机模板的复刻残留
    u8 values[8];

    ret = 0;
    pool = GetObjPool();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg->posX;
        gObjActSavedY = arg->posY;
        sub_8020DE4();
        gObjActSavedPal = arg->headA.palSlot;
        gObjActSavedF2A = arg->headA.f_1E;
        gObjActStepTimer = 0;
        gObjActStep = 0x26;
        break;
    case 38: // 0x26
        sub_8020974((ObjHead *)(&arg->headA), 0x374, 0x1B4, 0xD, 2);
        gObjActStep = 0x27;
        break;
    case 39: // 0x27
        if (arg->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x31, 0, 0);
        gObjActStep = 0x28;
        break;
    case 40: // 0x28
        if (arg->headA.frameIdx == 0x3A)
            Sfx_Play(0x58, 0, 0);
        if (arg->headA.frameIdx == 0x87)
            Sfx_Play(0x31, 0, 0);
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_8020974((ObjHead *)(&arg->headA), 0x375, 0x1B4, 0xD, 2);
        gObjActStep = 0x29;
        break;
    case 41: // 0x29
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2A;
        break;
    case 42: // 0x2A
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_8020974((ObjHead *)(&arg->headA), 0x376, 0x1B4, 0xD, 2);
        gObjActStep = 0x2B;
        break;
    case 43: // 0x2B
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2C;
        break;
    case 44: // 0x2C
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_801CBA4(arg, 0, gObjActSavedF2A, gObjActSavedPal, 0);
        gObjActStep = 0x2D;
        break;
    case 45: // 0x2D
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2E;
        break;
    case 46: // 0x2E
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1747, 0, 0x10);
        gObjActStep = 0x12;
        break;
    case 18: // 0x12
        if (sub_8019B98(2, 3, 0xE, 1) != 0)
        {
            gObjActStepTimer = 0;
            sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
            Sfx_Play(0x20, 0, 0);
            gObjActStep = 0x13;
        }
        break;
    case 19: // 0x13
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
        gObjActStep = 0x14;
        break;
    case 20: // 0x14
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x15;
        break;
    case 21: // 0x15
        if (gObjActStepTimer <= 0x45)
        {
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x16;
        break;
    case 22: // 0x16
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x17;
        break;
    case 23: // 0x17
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        BattleUiFlag_Set(0x20);
        gObjActStep = 0x18;
        break;
    case 24: // 0x18
        if (sub_801EEE4(arg, GetObjPool(), 1, 0, 0x3E7) == 1)
            gObjActStep = 9;
        break;
    case 9:
        sub_801A2AC(0, 0, 0);
        sub_8045B90(arg, arg->pad_A1);
        ret = 2;
        break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x08033E2C
// 0x0839D4CC 动作表项 7 演出状态机:
// case0 登记敌侧目标(sub_80489E8)并保存动画/位置参数;
// case38-46 播放起手前置动画序列与音效(0x374/0x375/0x376), 淡出画面(0x1747);
// case18-23 窗口设置(sub_804BF14 9参), 双向淡出淡入插值(sub_801768C)并置 BattleUiFlag(0x20);
// case24-29 双头动画(sub_8020CC4), 目标槽位标记(ptr[0xBE]=0xFF, ptr[0xAB]=7, sub_80207A4), 异常清位;
// case9 重置(sub_801A2AC/sub_8045B90)返回 2; 尾部统一部署 sub_803F658。
u32 sub_8033E2C(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u8 *ptr;
    u16 keys;

    ret = 0;
    pool = GetObjPool();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg->posX;
        gObjActSavedY = arg->posY;
        gObjActSavedPal = arg->headA.palSlot;
        gObjActSavedF2A = arg->headA.f_1E;
        gObjActStepTimer = 0;
        gTargetSlotCount = sub_80489E8((BattleObj *)pool, gTargetSlotList, 1, 0x7F);
        gObjActStep = 0x26;
        break;
    case 38: // 0x26
        sub_8020974((ObjHead *)(&arg->headA), 0x374, 0x1B4, 0xD, 2);
        gObjActStep = 0x27;
        break;
    case 39: // 0x27
        if (arg->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x31, 0, 0);
        gObjActStep = 0x28;
        break;
    case 40: // 0x28
        if (arg->headA.frameIdx == 0x3A)
            Sfx_Play(0x58, 0, 0);
        if (arg->headA.frameIdx == 0x87)
            Sfx_Play(0x31, 0, 0);
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_8020974((ObjHead *)(&arg->headA), 0x375, 0x1B4, 0xD, 2);
        gObjActStep = 0x29;
        break;
    case 41: // 0x29
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2A;
        break;
    case 42: // 0x2A
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_8020974((ObjHead *)(&arg->headA), 0x376, 0x1B4, 0xD, 2);
        gObjActStep = 0x2B;
        break;
    case 43: // 0x2B
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2C;
        break;
    case 44: // 0x2C
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_801CBA4(arg, 0, gObjActSavedF2A, gObjActSavedPal, 0);
        gObjActStep = 0x2D;
        break;
    case 45: // 0x2D
        if (arg->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x2E;
        break;
    case 46: // 0x2E
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1747, 0, 0x10);
        gObjActStep = 0x12;
        break;
    case 18: // 0x12
        if (sub_8019B98(3, 3, 0xE, 1) != 0)
        {
            gObjActStepTimer = 0;
            sub_804BF14(0, 3, 7, 0xE, 0x1C, 4, 4, -1, 2);
            Sfx_Play(0x20, 0, 0);
            gObjActStep = 0x13;
        }
        break;
    case 19: // 0x13
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
        gObjActStep = 0x14;
        break;
    case 20: // 0x14
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x15;
        break;
    case 21: // 0x15
        if (gObjActStepTimer <= 0x45)
        {
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x16;
        break;
    case 22: // 0x16
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x17;
        break;
    case 23: // 0x17
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1747, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        BattleUiFlag_Set(0x20);
        if (sub_80187B4() & 0x220)
            gObjActStep = 9;
        else
            gObjActStep = 0x18;
        break;
    case 24: // 0x18
        sub_8020CC4(arg, 0x3C, 0x73, 0x1B4, 0xE, 0x37F, 0x14);
        arg->headB.f_2A = 0;
        sub_801A2AC(0x410, 0, 7);
        gObjActStep = 0x19;
        break;
    case 25: // 0x19
        if (arg->headB.kindFlags & 0x800)
            break;
        Sfx_Play(0xA5, 0, 0);
        gObjActStep = 0x1A;
        break;
    case 26: // 0x1A
        if (!(arg->headB.kindFlags & 0x1000))
            break;
        sub_8020CC4(arg, 0x3C, 0x73, 0x1B4, 0xE, 0x380, 0x114);
        gObjActStepTimer = 0;
        gObjActStep = 0x1B;
        break;
    case 27: // 0x1B
        if (arg->headB.kindFlags & 0x800)
            break;
        for (i = 0; i < gTargetSlotCount; i++)
        {
            ptr = (u8 *)(pool + gTargetSlotList[i] * 0xC8);
            ptr[0xBE] = 0xFF;
            ptr[0xAB] = 7;
            sub_80207A4();
        }
        gObjActStep = 0x1C;
        break;
    case 28: // 0x1C
        if (gObjActStepTimer <= 0x27)
            gObjActStepTimer += 1;
        else
        {
            keys = arg->headB.kindFlags & 0xFEFF;
            arg->headB.kindFlags = keys;
            gObjActStep = 0x1D;
        }
        break;
    case 29: // 0x1D
        if (!(arg->headB.kindFlags & 0x1000))
            break;
        sub_804C3A4(arg->headB.palSlot, sub_801B954((ObjHead *)(&arg->headB)));
        keys = arg->state & 0xDFFF;
        arg->state = keys;
        gObjActStep = 9;
        break;
    case 9:
        sub_801A2AC(0, 0, 0);
        sub_8045B90(arg, arg->pad_A1);
        ret = 2;
        break;
    }
    sub_803F658(arg);
    return ret;
}
// @ 0x08034440
u32 sub_8034440(BattleObj *arg)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            sub_8020DE4();
            gObjActStep = 1;
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x386, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            zero = 0;
            arg->headA.kindFlags = keys;
            sub_801CBA4((BattleObj *)arg, zero, gObjActSavedF2A, gObjActSavedPal, zero);
            gObjActStep = 6;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 8;
            break;
        case 8:
            if (sub_801EEE4(arg, GetObjPool(), 0, 0, 0x32) == 1)
                gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    return ret;
}
/*
u32 sub_8034440(u8 *arg)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        sub_8020DE4();
        gObjActStep = 1;
        gObjActSavedPal = arg[0x35];
        gObjActSavedF2A = *(u16 *)&arg[0x2A];
        break;
    case 1:
        sub_8020974((ObjHead *)(arg + 0xC), 0x386, 0x1B4, 0xD, 2);
        gObjActStep = 2;
        break;
    case 2:
        if (*(u16 *)&arg[0x24] & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gObjActStep = 5;
        break;
    case 5:
        if (!(*(u16 *)&arg[0x24] & 0x1000))
            break;
        sub_804C3A4(arg[0x35], sub_801B954((ObjHead *)(arg + 0xC)));
        keys = *(u16 *)&arg[0x24] & 0xEFFF;
        zero = 0;
        *(u16 *)&arg[0x24] = keys;
        sub_801CBA4((BattleObj *)arg, zero, gObjActSavedF2A, gObjActSavedPal, zero);
        gObjActStep = 6;
        break;
    case 6:
        if (*(u16 *)&arg[0x24] & 0x800)
            break;
        gObjActStep = 8;
        break;
    case 8:
        if (sub_801EEE4(arg, GetObjPool(), 0, 0, 0x32) == 1)
            gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(arg, arg[0xA1]);
        ret = 2;
        break;
    }
    return ret;
}
*/
// @ 0x080345AC
u32 sub_80345AC(BattleObj *arg)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            sub_8020DE4();
            gObjActStep = 1;
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x386, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            zero = 0;
            arg->headA.kindFlags = keys;
            sub_801CBA4((BattleObj *)arg, zero, gObjActSavedF2A, gObjActSavedPal, zero);
            gObjActStep = 6;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 8;
            break;
        case 8:
            if (sub_801EEE4(arg, GetObjPool(), 0, 0xB, 0x1E) == 1)
                gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x08034718
u32 sub_8034718(BattleObj *arg, BattleObj *arg1)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            sub_8048B30(0, 0x1E, 0x3AD);
            gObjActStep = 1;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x386, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            zero = 0;
            arg->headA.kindFlags = keys;
            sub_801CBA4((BattleObj *)arg, zero, gObjActSavedF2A, gObjActSavedPal, zero);
            gObjActStep = 6;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x12;
            break;
        case 0x12:
            if (sub_80476DC(arg, arg1) == 1)
                gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x080348A8
u32 sub_80348A8(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u8 *ptr;
    u32 zero;
    u16 keys;

    ret = 0;
    pool = GetObjPool();
    switch (gObjActStep)
    {
        case 0:
            gObjActStep = 1;
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            gTargetSlotCount = sub_80489E8(pool, gTargetSlotList, 1, 0x7F);
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x386, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            zero = 0;
            arg->headA.kindFlags = keys;
            sub_801CBA4((BattleObj *)arg, zero, gObjActSavedF2A, gObjActSavedPal, zero);
            gObjActStep = 6;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            if (sub_80187B4() & 0x220)
                gObjActStep = 9;
            else
                gObjActStep = 0x18;
            break;
        case 24:
            sub_8020CC4(arg, 0x3C, 0x73, 0x1B4, 0xE, 0x37F, 0x14);
            arg->headB.f_2A = 0;
            sub_801A2AC(0x410, 0, 7);
            gObjActStep = 0x19;
            break;
        case 25:
            if (arg->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 0, 0);
            gObjActStep = 0x1A;
            break;
        case 26:
            if (!(arg->headB.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headB.palSlot, sub_801B954((ObjHead *)(&arg->headB)));
            sub_8020CC4(arg, 0x3C, 0x73, 0x1B4, 0xE, 0x380, 0x114);
            gObjActStepTimer = 0;
            gObjActStep = 0x1B;
            break;
        case 27:
            if (arg->headB.kindFlags & 0x800)
                break;
            for (i = 0; i < gTargetSlotCount; i++)
            {
                ptr = (u8 *)(pool + gTargetSlotList[i] * 0xC8);
                ptr[0xBE] = 0xFF;
                ptr[0xAB] = 7;
                sub_80207A4();
            }
            gObjActStep = 0x1C;
            break;
        case 28:
            if (gObjActStepTimer <= 0x27)
                gObjActStepTimer += 1;
            else
            {
                keys = arg->headB.kindFlags & 0xFEFF;
                arg->headB.kindFlags = keys;
                gObjActStep = 0x1D;
            }
            break;
        case 29:
            if (!(arg->headB.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headB.palSlot, sub_801B954((ObjHead *)(&arg->headB)));
            keys = arg->state & 0xDFFF;
            arg->state = keys;
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x08034BFC
u32 sub_8034BFC(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 buf[8];
    u8 i;
    u8 count;
    u32 zero;
    u16 keys;

    ret = 0;
    pool = GetObjPool();
    switch (gObjActStep)
    {
        case 0:
            gObjActStep = 1;
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x386, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            zero = 0;
            arg->headA.kindFlags = keys;
            sub_801CBA4((BattleObj *)arg, zero, gObjActSavedF2A, gObjActSavedPal, zero);
            gObjActStep = 6;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 8;
            break;
        case 8:
            count = sub_80489E8(pool, buf, 1, 7);
            for (i = 0; i < count; i++)
            {
                if (Rng_LcgNext() % 0x64 <= 0x27)
                    sub_8045F94((BattleObj *)(pool + buf[i] * 0xC8), 3);
            }
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x08034D94
u32 sub_8034D94(BattleObj *arg)
{
    u32 ret;
    u32 zero;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            sub_8020DE4();
            gObjActStep = 1;
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x386, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x3E, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            zero = 0;
            arg->headA.kindFlags = keys;
            sub_801CBA4((BattleObj *)arg, zero, gObjActSavedF2A, gObjActSavedPal, zero);
            gObjActStep = 6;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 8;
            break;
        case 8:
            if (sub_801EEE4(arg, GetObjPool(), 1, 0, 0x3C) == 1)
                gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    return ret;
}
// @ 0x08034F00
u32 sub_8034F00(BattleObj *arg)
{
    u32 ret;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            sub_80444A4((BattleObj *)arg);
            sub_803F5B4((BattleObj *)arg);
            gObjActStep = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x38B;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gObjActStepTimer = zero;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x387, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            arg->headA.kindFlags = keys;
            sub_8020974((ObjHead *)(&arg->headA), 0x388, 0x1B4, 0xD, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            sub_8044514(0x32);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gObjActStep = 0x14;
            }
            gObjActStepTimer += 1;
            break;
        case 20:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && (v56 = gActWaitBusy2) == 0)
            {
                sub_801CBA4((BattleObj *)arg, 0, gObjActSavedF2A, gObjActSavedPal, v56);
                gObjActStep = 6;
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg, arg->pad_A1);
            ret = 1;
            break;
    }
    sub_803F658((BattleObj *)arg);
    return ret;
}
// @ 0x08035130
u32 sub_8035130(BattleObj *arg)
{
    u32 ret;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            sub_80444A4((BattleObj *)arg);
            sub_803F5B4((BattleObj *)arg);
            gObjActStep = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x38C;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x14;
            gObjActStepTimer = zero;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x387, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            arg->headA.kindFlags = keys;
            sub_8020974((ObjHead *)(&arg->headA), 0x388, 0x1B4, 0xD, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            sub_8044514(0x32);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gObjActStep = 0x14;
            }
            gObjActStepTimer += 1;
            break;
        case 20:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && (v56 = gActWaitBusy2) == 0)
            {
                sub_801CBA4((BattleObj *)arg, 0, gObjActSavedF2A, gObjActSavedPal, v56);
                gObjActStep = 6;
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg, arg->pad_A1);
            ret = 1;
            break;
    }
    sub_803F658((BattleObj *)arg);
    return ret;
}
// @ 0x08035360
// 战斗演出状态机变体 (gObjActStep 0→1/2/5/0x12..0x21→9, 单参 obj):
// case0   存 headA 调色/动画恢复参数, sub_80444A4 + sub_803F5B4; f_B6=0x391, f_B4=0,
//         步计时=0 → 1。两个零分别用 r1(f_B4)/r2(步计时); 目标先物化 r2 再 r1,
//         故源码对同一 `zero` 变量在两次使用之间重新赋值 0 (GCC2.9 寄存器分配所需形状)。
// case1   headA 动画 0x387 → 2;  case2 等 0x800 落 → Sfx(0xA5)+ → 5。
// case5   headA 0x1000 → sub_804C3A4 + 清 0xEFFF + 动画 0x388 → 0x12。
// 0x12    等 0x800 落 → 0x13;  0x13 sub_8020CC4(0x50,0x64,...,0x38D,5) + headB.f_2A=0 → 0x14。
// 0x14    等 headB 0x800 落 → 0x15;  0x15 headB 0x1000 → 还原调色 + 动画 0x38E + → 0x16。
// 0x16    等 headB 0x800 落 → Sfx(0x66,1,1) + 步计时=0 → 0x17。
// 0x17    步计时<=0x1D: headB.f_2C=sub_801768C(0x64,-0x82,0x1E,步计时,1), 步计时++; 否则 → 0x18。
// 0x18    sub_8020CC4(0x78,0x50,...,0x390,0x405) + headB.f_2A=0 + 步计时=0 → 0x19。
// 0x19    等 headB 0x800 落 → Sfx(0x69)+sub_804BBDC(...) → 0x1A。
// 0x1A    步计时<=0x3B 则 ++; 否则 sub_804BD54(0,3) → 0x1B。
// 0x1B    sub_8020CC4(0x50,0x78,...,0x38F,5) + headB.f_2A=0 + Sfx_StopTrack(1) → 0x1C。
// 0x1C    等 headB 0x800 落 → Sfx(0x4F,1,0) → 0x1D。
// 0x1D    headB.frameIdx>0xE: 步计时=0; sub_8044514(0x28)+sub_804BF14(...) → 0x1E。
// 0x1E    步计时<=3 则 ++; 否则 sub_804C728(0,3,0x10)+步计时++ → 0x1F。
// 0x1F    headB 0x1000 → 还原调色 + 清 obj.state 0x2000 → 0x20; 每帧步计时++。
// 0x20    三个 gActWaitBusy 清零 → sub_801CBA4(obj,0,保存值) → 0x21。
// 0x21    等 headA 0x800 落 → 9。  9: sub_8045B90(obj,pad_A1), 返回 1。尾部 sub_803F658(obj)。
// 注: 跳转表含 38 项 (0..0x25), 0x22..0x25 落公共尾; case 0x25 显式空分支对应表界。
u32 sub_8035360(BattleObj *obj)
{
    u32 ret = 0;
    u16 *fieldB6;
    u32 zero;
    u16 fieldB4;
    u16 fieldB6Value;
    u16 flags;
    u8 pending;

    switch (gObjActStep)
    {
    case 0:
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        sub_80444A4(obj);
        sub_803F5B4(obj);
        gObjActStep = 1;
        zero = 0;
        fieldB6 = &obj->f_B6;
        fieldB4 = zero;
        zero = 0;
        fieldB6Value = 0x391;
        *fieldB6 = fieldB6Value;
        obj->f_B4 = fieldB4;
        gObjActStepTimer = zero;
        break;
    case 1:
        sub_8020974(&obj->headA, 0x387, 0x1B4, 0xD, 2);
        gObjActStep = 2;
        break;
    case 2:
        if (!(obj->headA.kindFlags & 0x800))
        {
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
        }
        break;
    case 5:
        if (obj->headA.kindFlags & 0x1000)
        {
            sub_804C3A4(obj->headA.palSlot, sub_801B954(&obj->headA));
            obj->headA.kindFlags &= 0xEFFF;
            sub_8020974(&obj->headA, 0x388, 0x1B4, 0xD, 2);
            gObjActStep = 0x12;
        }
        break;
    case 0x12:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 0x13;
        break;
    case 0x13:
        sub_8020CC4(obj, 0x50, 0x64, 0x27C, 0xE, 0x38D, 5);
        obj->headB.f_2A = 0;
        gObjActStep = 0x14;
        break;
    case 0x14:
        if (!(obj->headB.kindFlags & 0x800))
            gObjActStep = 0x15;
        break;
    case 0x15:
        if (obj->headB.kindFlags & 0x1000)
        {
            sub_804C3A4(obj->headB.palSlot, sub_801B954(&obj->headB));
            sub_8020CC4(obj, 0x50, 0x64, 0x27C, 0xE, 0x38E, 0x405);
            obj->headB.f_2A = 0;
            gObjActStep = 0x16;
        }
        break;
    case 0x16:
        flags = obj->headB.kindFlags & 0x800;
        if (flags == 0)
        {
            Sfx_Play(0x66, 1, 1);
            gObjActStepTimer = flags;
            gObjActStep = 0x17;
        }
        break;
    case 0x17:
        if (gObjActStepTimer <= 0x1D)
        {
            obj->headB.f_2C = sub_801768C(0x64, -0x82, 0x1E, gObjActStepTimer, 1);
            gObjActStepTimer++;
        }
        else
            gObjActStep = 0x18;
        break;
    case 0x18:
        sub_8020CC4(obj, 0x78, 0x50, 0x27C, 0xE, 0x390, 0x405);
        obj->headB.f_2A = 0;
        gObjActStepTimer = 0;
        gObjActStep = 0x19;
        break;
    case 0x19:
        if (!(obj->headB.kindFlags & 0x800))
        {
            Sfx_Play(0x69, 1, 0);
            sub_804BBDC(0, 3, 0xF, 0xF, 2, 4, 4, 2);
            gObjActStep = 0x1A;
        }
        break;
    case 0x1A:
        if (gObjActStepTimer <= 0x3B)
            gObjActStepTimer++;
        else
        {
            sub_804BD54(0, 3);
            gObjActStep = 0x1B;
        }
        break;
    case 0x1B:
        sub_8020CC4(obj, 0x50, 0x78, 0x27C, 0xE, 0x38F, 5);
        obj->headB.f_2A = 0;
        gObjActStep = 0x1C;
        Sfx_StopTrack(1);
        break;
    case 0x1C:
        if (!(obj->headB.kindFlags & 0x800))
        {
            Sfx_Play(0x4F, 1, 0);
            gObjActStep = 0x1D;
        }
        break;
    case 0x1D:
        if (obj->headB.frameIdx > 0xE)
        {
            gObjActStepTimer = 0;
            sub_8044514(0x28);
            sub_804BF14(0, 3, 0xF, 0xF, 2, 4, 4, -1, 2);
            gObjActStep = 0x1E;
        }
        break;
    case 0x1E:
        if (gObjActStepTimer <= 3)
            gObjActStepTimer++;
        else
        {
            sub_804C728(0, 3, 0x10);
            gObjActStepTimer++;
            gObjActStep = 0x1F;
        }
        break;
    case 0x1F:
        if (obj->headB.kindFlags & 0x1000)
        {
            sub_804C3A4(obj->headB.palSlot, sub_801B954(&obj->headB));
            obj->state &= 0xDFFF;
            gObjActStep = 0x20;
        }
        gObjActStepTimer++;
        break;
    case 0x20:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && (pending = gActWaitBusy2) == 0)
        {
            sub_801CBA4(obj, 0, gObjActSavedF2A, gObjActSavedPal, pending);
            gObjActStep = 0x21;
        }
        break;
    case 0x21:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(obj, obj->pad_A1);
        ret = 1;
        break;
    case 0x25:
        break;
    }
    sub_803F658(obj);
    return ret;
}
// @ 0x0803586C
u32 sub_803586C(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            sub_80444A4((BattleObj *)arg);
            sub_803F5B4((BattleObj *)arg);
            gObjActStep = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x392;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gObjActStepTimer = zero;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x387, 0x1B4, 0xC, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            arg->headA.kindFlags = keys;
            sub_8020974((ObjHead *)(&arg->headA), 0x388, 0x1B4, 0xC, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            sub_8044514(0x32);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gObjActStep = 0x14;
            }
            gObjActStepTimer += 1;
            break;
        case 20:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && (v56 = gActWaitBusy2) == 0)
            {
                sub_801CBA4((BattleObj *)arg, 0, gObjActSavedF2A, gObjActSavedPal, v56);
                gObjActStep = 6;
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gObjActGroupCount; i++)
            {
                if ((gObjActGroupSlots[i] & 0xF0) == 0x10)
                {
                    if (Rng_LcgNext() % 0x64 <= 0x27)
                        sub_8045F94((BattleObj *)(pool + (gObjActGroupSlots[i] & 0xF) * 0xC8), 2);
                }
            }
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    sub_803F658((BattleObj *)arg);
    return ret;
}
// @ 0x08035B04
u32 sub_8035B04(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            sub_80444A4((BattleObj *)arg);
            sub_803F5B4((BattleObj *)arg);
            gObjActStep = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x393;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gObjActStepTimer = zero;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x387, 0x1B4, 0xC, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            arg->headA.kindFlags = keys;
            sub_8020974((ObjHead *)(&arg->headA), 0x388, 0x1B4, 0xC, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            sub_8044514(0x32);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gObjActStep = 0x14;
            }
            gObjActStepTimer += 1;
            break;
        case 20:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && (v56 = gActWaitBusy2) == 0)
            {
                sub_801CBA4((BattleObj *)arg, 0, gObjActSavedF2A, gObjActSavedPal, v56);
                gObjActStep = 6;
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gObjActGroupCount; i++)
            {
                if ((gObjActGroupSlots[i] & 0xF0) == 0x10)
                {
                    if (Rng_LcgNext() % 0x64 <= 0x27)
                        sub_8045F94((BattleObj *)(pool + (gObjActGroupSlots[i] & 0xF) * 0xC8), 3);
                }
            }
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    sub_803F658((BattleObj *)arg);
    return ret;
}
// @ 0x08035D9C
u32 sub_8035D9C(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            sub_80444A4((BattleObj *)arg);
            sub_803F5B4((BattleObj *)arg);
            gObjActStep = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x394;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gObjActStepTimer = zero;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x387, 0x1B4, 0xC, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            arg->headA.kindFlags = keys;
            sub_8020974((ObjHead *)(&arg->headA), 0x388, 0x1B4, 0xC, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            sub_8044514(0x32);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gObjActStep = 0x14;
            }
            gObjActStepTimer += 1;
            break;
        case 20:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && (v56 = gActWaitBusy2) == 0)
            {
                sub_801CBA4((BattleObj *)arg, 0, gObjActSavedF2A, gObjActSavedPal, v56);
                gObjActStep = 6;
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gObjActGroupCount; i++)
            {
                if ((gObjActGroupSlots[i] & 0xF0) == 0x10)
                {
                    if (Rng_LcgNext() % 0x64 <= 0x27)
                        sub_8045F94((BattleObj *)(pool + (gObjActGroupSlots[i] & 0xF) * 0xC8), 5);
                }
            }
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    sub_803F658((BattleObj *)arg);
    return ret;
}
// @ 0x08036034
u32 sub_8036034(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            sub_80444A4((BattleObj *)arg);
            sub_803F5B4((BattleObj *)arg);
            gObjActStep = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x395;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gObjActStepTimer = zero;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x387, 0x1B4, 0xC, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            arg->headA.kindFlags = keys;
            sub_8020974((ObjHead *)(&arg->headA), 0x388, 0x1B4, 0xC, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            sub_8044514(0x32);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gObjActStep = 0x14;
            }
            gObjActStepTimer += 1;
            break;
        case 20:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && (v56 = gActWaitBusy2) == 0)
            {
                sub_801CBA4((BattleObj *)arg, 0, gObjActSavedF2A, gObjActSavedPal, v56);
                gObjActStep = 6;
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gObjActGroupCount; i++)
            {
                if ((gObjActGroupSlots[i] & 0xF0) == 0x10)
                {
                    if (Rng_LcgNext() % 0x64 <= 0x27)
                        sub_8045F94((BattleObj *)(pool + (gObjActGroupSlots[i] & 0xF) * 0xC8), 6);
                }
            }
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    sub_803F658((BattleObj *)arg);
    return ret;
}
// @ 0x080362CC
u32 sub_80362CC(BattleObj *arg)
{
    u32 ret;
    u32 pool;
    u8 i;
    u32 zero;
    u8 v56;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg->headA.palSlot;
            gObjActSavedF2A = arg->headA.f_1E;
            sub_80444A4((BattleObj *)arg);
            sub_803F5B4((BattleObj *)arg);
            gObjActStep = 1;
            zero = 0;
            ((struct ObjFadeSeq *)arg)->f_b6 = 0x396;
            ((struct ObjFadeSeq *)arg)->f_b4 = 0x23;
            gObjActStepTimer = zero;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg->headA), 0x387, 0x1B4, 0xC, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg->headA.kindFlags & 0x1000))
                break;
            sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
            keys = arg->headA.kindFlags & 0xEFFF;
            arg->headA.kindFlags = keys;
            sub_8020974((ObjHead *)(&arg->headA), 0x388, 0x1B4, 0xC, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg->headA.kindFlags & 0x800)
                break;
            sub_8044514(0x32);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x66, 1, 0);
                gObjActStep = 0x14;
            }
            gObjActStepTimer += 1;
            break;
        case 20:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && (v56 = gActWaitBusy2) == 0)
            {
                sub_801CBA4((BattleObj *)arg, 0, gObjActSavedF2A, gObjActSavedPal, v56);
                gObjActStep = 6;
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            if (arg->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            pool = GetObjPool();
            for (i = 0; i < gObjActGroupCount; i++)
            {
                if ((gObjActGroupSlots[i] & 0xF0) == 0x10)
                {
                    if (Rng_LcgNext() % 0x64 <= 0x13)
                        sub_8045F94((BattleObj *)(pool + (gObjActGroupSlots[i] & 0xF) * 0xC8), 4);
                }
            }
            sub_8045B90(arg, arg->pad_A1);
            ret = 2;
            break;
    }
    sub_803F658((BattleObj *)arg);
    return ret;
}
// @ 0x08036564
u32 sub_8036564(BattleObj *arg0, BattleObj *arg1)
{
    u32 ret;
    int b4;
    u16 keys;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg0->headA.palSlot;
            gObjActSavedF2A = arg0->headA.f_1E;
            sub_80444A4(arg0);
            sub_803F5B4(arg0);
            gObjActStep = 1;
            arg0->f_B6 = 0xAA;
            arg0->f_B4 = 0;
            gObjActStepTimer = 0;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg0->headA), 0x3A0, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg0->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg0->headA.kindFlags & 0x1000))
                break;
            b4 = arg0->headA.palSlot;
            sub_804C3A4(b4, sub_801B954((ObjHead *)(&arg0->headA)));
            arg0->headA.kindFlags &= 0xEFFF;
            sub_8020974((ObjHead *)(&arg0->headA), 0x3A1, 0x1B4, 0xD, 0x102);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg0->headA.kindFlags & 0x800)
                break;
            arg0->headA.kindFlags &= 0xFEFF;
            gObjActStep = 0x13;
            break;
        case 19:
            sub_8020CC4(arg0, (u8)(arg0->posX - 0x10), (u8)(arg0->posY - 0x18), 0x24A, 0xE, 0x3A7, 4);
            gObjActStep = 0x14;
            break;
        case 20:
            if (arg0->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x31, 1, 0);
            gObjActStep = 0x15;
            break;
        case 21:
            if (!(arg0->headB.kindFlags & 0x1000))
                break;
            b4 = arg0->headB.palSlot;
            sub_804C3A4(b4, sub_801B954((ObjHead *)(&arg0->headB)));
            arg0->headB.kindFlags &= 0xEFFF;
            arg0->headB.kindFlags |= 0x100;
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            arg0->headB.frameIdx = (arg0->headB.frameIdx + 1) % 0x16 + 0x18;
            if (gObjActStepTimer <= 9)
            {
                arg0->headB.f_2B = sub_801768C(arg0->posX - 0x10, arg1->posX - (arg0->posX - 0x10), 0xA,
                                                gObjActStepTimer, 2);
                arg0->headB.f_2C = sub_801768C(arg0->posY - 0x18, arg1->posY - (arg0->posY - 0x18), 0xA,
                                                gObjActStepTimer, 2);
                gObjActStepTimer += 1;
                break;
            }
            sub_8044514(0x14);
            arg0->state &= 0xDFFF;
            gObjActStep = 0x17;
            break;
        case 23:
            if (sub_80471AC() == 0)
            {
                Sfx_Play(0x64, 1, 0);
                gObjActStep = 0x18;
            }
            gObjActStepTimer += 1;
            break;
        case 24:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801CBA4(arg0, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 6;
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            if (arg0->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg0, arg0->pad_A1);
            ret = 1;
            break;
        case 3:
        case 4:
        case 7:
        case 8:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
            break;
    }
    sub_803F658(arg0);
    return ret;
}

// @ 0x080368FC
// NPC 剧情对话状态机变体 (单参数, gObjActStep 二十一态 0-0x14):
// case0 存 NPC 位置 (0824/0822) + 初始化 + 写 0x3A9 到 [0xB6]; case1 开场动画 0x3A5;
// case2 等 0x800 → Sfx 0xA5 → 5; case5 确认 0x1000 → sub_804C3A4 + 清 0x1000 + 动画 0x3A6
// (第 5 参 0x102) → 0x12; case18 等 0x800 + 清 0x100 → sub_8044514(0x32) → 0x13;
// case19 sub_80471AC()==0 → Sfx 0x64 → 0x14; case20 无遮挡 → sub_801CBA4 归位 (v56 载体) → 6;
// case6 等 0x800 → 9; case9 sub_8045B90 退场 + ret 1。case19/20 共享 0825++。
// 注: 载体变量 (b6ptr/zero2/b6val/v56) 与 sub_803ED34/u8 原型同 803272C 家族套路。
u32 sub_80368FC(BattleObj *arg0)
{
    u32 ret;
    u8 b4;
    int keys;
    u16 flags;
    u8 v56;
    u16 *b6ptr;
    u16 b6val;
    u16 zero2;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedPal = arg0->headA.palSlot;
        gObjActSavedF2A = arg0->headA.f_1E;
        sub_80444A4((BattleObj *)arg0);
        sub_803F5B4((BattleObj *)arg0);
        gObjActStep = 1;
        b6ptr = &(arg0->f_B6);
        zero2 = 0;
        b6val = 0x3A9;
        *b6ptr = b6val;
        arg0->f_B4 = zero2;
        break;
    case 1:
        sub_8020974((ObjHead *)(&arg0->headA), 0x3A5, 0x1B4, 0xD, 2);
        gObjActStep = 2;
        break;
    case 2:
        if (arg0->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0xA5, 1, 0);
        gObjActStep = 5;
        break;
    case 5:
        if (!(arg0->headA.kindFlags & 0x1000))
            break;
        b4 = arg0->headA.palSlot;
        keys = sub_801B954((ObjHead *)(&arg0->headA));
        sub_804C3A4(b4, (u8)keys);
        flags = arg0->headA.kindFlags & 0xEFFF;
        arg0->headA.kindFlags = flags;
        sub_8020974((ObjHead *)(&arg0->headA), 0x3A6, 0x1B4, 0xD, 0x102);
        gObjActStep = 0x12;
        break;
    case 18:
        if (arg0->headA.kindFlags & 0x800)
            break;
        flags = arg0->headA.kindFlags & 0xFEFF;
        arg0->headA.kindFlags = flags;
        sub_8044514(0x32);
        gObjActStep = 0x13;
        break;
    case 19:
        if (sub_80471AC() == 0)
        {
            Sfx_Play(0x64, 1, 0);
            gObjActStep = 0x14;
        }
        gObjActStepTimer += 1;
        break;
    case 20:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && (v56 = gActWaitBusy2) == 0)
        {
            sub_801CBA4(arg0, 0, gObjActSavedF2A, gObjActSavedPal, v56);
            gObjActStep = 6;
        }
        gObjActStepTimer += 1;
        break;
    case 6:
        if (arg0->headA.kindFlags & 0x800)
            break;
        gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(arg0, arg0->pad_A1);
        ret = 1;
        break;
    }
    sub_803F658((BattleObj *)arg0);
    return ret;
}
// @ 0x08036B30
/* 战斗对象"文字框/字幕演出"状态机 (gObjActStep: 0 → 1 → 2 → 5 → 0x12..0x16 → 0x1A → 0x1B → 9)。
 * 单参, 与 sub_802B608 同族: 用 sub_801A2AC/sub_8019B98/sub_804BDD8/sub_804BE90 +
 * gSceneFadeOut 窗口插值 (sub_801768C) + sub_801CBA4 收尾。
 *   case0  存 headA.f_1E/palSlot; sub_80444A4 + sub_803F5B4; f_B6=2, f_B4=0 → 1。
 *   case1  sub_8020974(&headA, 0x3A0, 0x1B4, 0xD, 2) → 2。
 *   case2  headA 0x800 落 → Sfx(0xA5) → 5。
 *   case5  headA 0x1000 → sub_804C3A4(palSlot,(u8)sub_801B954(&headA)); headA.kindFlags&=0xEFFF;
 *          sub_8020974(&headA, 0x3A1, 0x1B4, 0xD, 2) → 0x12。
 *   0x12   headA 0x800 落 → sub_801A348 → 0x13。
 *   0x13   sub_8019B98(0x10,3,0xE,1)!=0 → Sfx(0x50); gSceneFadeOut=0; sub_801A2AC(0x1C42,0,0x10);
 *          gObjActStepTimer=0; sub_804BDD8(0xE,1,1,1,0xF); sub_8044514(0x46) → 0x14。
 *   0x14   计数<=0x13: gSceneFadeOut=sub_801768C(0,0xC,0x14,计数,2); sub_801A2AC(0x1C42,..,0x10); 计数++;
 *          否则 计数=0, gSceneFadeOut=0xC, sub_801A2AC(0x1C42,0xC,0x10) → 0x15。
 *   0x15   Sfx_TrackBusy(1)==0 → 计数=0 → 0x16。
 *   0x16   计数<=0x13: gSceneFadeOut=sub_801768C(0xC,-0xC,0x14,计数,2); sub_801A2AC; 计数++;
 *          否则 计数=0; Sfx_StopTrack(1); sub_801A2AC(0,0,0); state&=0x2000; sub_804BE90(0xE,1);
 *          DISPCNT&=0xFDFF → 0x1A。
 *   0x1A   无其他演出占用 → sub_801CBA4(obj,0,f_1E,palSlot,0) → 0x1B; 否则 计数++。
 *   0x1B   headA 0x800 落 → 9。
 *   9      sub_8045B90(obj, pad_A1); result=1。
 * 尾部 sub_803F658(obj)。
 * 注: case19 的 `zero`/case21 的 `busy` 两个独立载体变量 + `zero=0` 内嵌为第二实参
 *     是字节匹配必需 (共用变量会使 case21 结果落 callee-saved r4, 差 3B)。 */
u32 sub_8036B30(BattleObj *obj)
{
    u32 ret;
    u8 busy;
    u8 zero;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        sub_80444A4(obj);
        sub_803F5B4(obj);
        gObjActStep = 1;
        obj->f_B6 = 2;
        obj->f_B4 = 0;
        break;
    case 1:
        sub_8020974((ObjHead *)(&obj->headA), 0x3A0, 0x1B4, 0xD, 2);
        gObjActStep = 2;
        break;
    case 2:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0xA5, 1, 0);
        gObjActStep = 5;
        break;
    case 5:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(obj->headA.palSlot, (u8)sub_801B954((ObjHead *)(&obj->headA)));
        obj->headA.kindFlags &= 0xEFFF;
        sub_8020974((ObjHead *)(&obj->headA), 0x3A1, 0x1B4, 0xD, 2);
        gObjActStep = 0x12;
        break;
    case 18:
        if (obj->headA.kindFlags & 0x800)
            break;
        sub_801A348();
        gObjActStep = 0x13;
        break;
    case 19:
        if (sub_8019B98(0x10, 3, 0xE, 1) == 0)
            break;
        Sfx_Play(0x50, 1, 0);
        gSceneFadeOut = 0;
        sub_801A2AC(0x1C42, zero = 0, 0x10);
        gObjActStepTimer = zero;
        sub_804BDD8(0xE, 1, 1, 1, 0xF);
        sub_8044514(0x46);
        gObjActStep = 0x14;
        break;
    case 20:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0, 0xC, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1C42, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gSceneFadeOut = 0xC;
        sub_801A2AC(0x1C42, 0xC, 0x10);
        gObjActStep = 0x15;
        break;
    case 21:
        if ((busy = Sfx_TrackBusy(1)) != 0)
            break;
        gObjActStepTimer = busy;
        gObjActStep = 0x16;
        break;
    case 22:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0xC, -0xC, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1C42, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        Sfx_StopTrack(1);
        sub_801A2AC(0, 0, 0);
        obj->state = obj->state & 0x2000;
        sub_804BE90(0xE, 1);
        *(volatile u16 *)(0x80 << 0x13) &= 0xFDFF;
        gObjActStep = 0x1A;
        break;
    case 26:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            sub_801CBA4(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x1B;
        }
        else
        {
            gObjActStepTimer += 1;
        }
        break;
    case 27:
        if (obj->headA.kindFlags & 0x800)
            break;
        gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(obj, obj->pad_A1);
        ret = 1;
        break;
    }
    sub_803F658(obj);
    return ret;
}
// @ 0x08036EA4
u32 sub_8036EA4(BattleObj *arg0, BattleObj *arg1)
{
    u32 ret;
    u8 b4;
    u32 zero;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg0->headA.palSlot;
            gObjActSavedF2A = arg0->headA.f_1E;
            sub_8048B30(0, 0x1E, 0x3AD);
            gObjActStep = 1;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg0->headA), 0x3A5, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg0->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg0->headA.kindFlags & 0x1000))
                break;
            b4 = arg0->headA.palSlot;
            sub_804C3A4(b4, sub_801B954((ObjHead *)(&arg0->headA)));
            arg0->headA.kindFlags &= 0xEFFF;
            sub_8020974((ObjHead *)(&arg0->headA), 0x3A6, 0x1B4, 0xD, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg0->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80476DC(arg0, arg1) == 1)
                gObjActStep = 0x14;
            break;
        case 20:
            sub_801CBA4(arg0, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 6;
            break;
        case 6:
            if (arg0->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg0, arg0->pad_A1);
            ret = 2;
            break;
    }
    return ret;
}

// @ 0x08037078
u32 sub_8037078(BattleObj *arg0, BattleObj *arg1)
{
    u32 ret;
    int b4;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg0->headA.palSlot;
            gObjActSavedF2A = arg0->headA.f_1E;
            sub_80444A4(arg0);
            sub_803F5B4(arg0);
            gObjActStep = 1;
            arg0->f_B6 = 0xA7;
            arg0->f_B4 = 0;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg0->headA), 0x3A5, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg0->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg0->headA.kindFlags & 0x1000))
                break;
            b4 = arg0->headA.palSlot;
            sub_804C3A4(b4, sub_801B954((ObjHead *)(&arg0->headA)));
            arg0->headA.kindFlags &= 0xEFFF;
            sub_8020974((ObjHead *)(&arg0->headA), 0x3A6, 0x1B4, 0xD, 0x102);
            gObjActStep = 0x1C;
            break;
        case 28:
            if (arg0->headA.kindFlags & 0x800)
                break;
            arg0->headA.kindFlags &= 0xFEFF;
            b4 = 0;
            sub_8020CC4(arg0, arg1->posX, (u8)(arg1->posY - sub_801EC3C(arg1, 1)), 0x27C, 0xE, 0x3B0, 4);
            arg0->headB.f_2A = 0;
            gObjActStep = 0x1D;
            break;
        case 29:
            if (arg0->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x6F, 1, 0);
            gObjActStep = 0x1E;
            break;
        case 30:
            if (!(arg0->headB.kindFlags & 0x1000))
                break;
            b4 = arg0->headB.palSlot;
            sub_804C3A4(b4, sub_801B954((ObjHead *)(&arg0->headB)));
            arg0->state &= 0xDFFF;
            gObjActStep = 0x12;
            break;
        case 18:
            sub_8044514(0x32);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
                gObjActStep = 0x14;
            gObjActStepTimer += 1;
            break;
        case 20:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_801CBA4(arg0, 0, gObjActSavedF2A, gObjActSavedPal, 0);
                gObjActStep = 6;
            }
            gObjActStepTimer += 1;
            break;
        case 6:
            if (arg0->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg0, arg0->pad_A1);
            ret = 1;
            break;
        case 3:
        case 4:
        case 7:
        case 8:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 31:
        case 32:
        case 33:
            break;
    }
    sub_803F658(arg0);
    return ret;
}

// @ 0x08037388
INCLUDE_ASM("asm/nonmatchings", sub_8037388);
// @ 0x08037868
u32 sub_8037868(BattleObj *obj)
{
    u32 ret;
    int b4;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            gObjActStep = 1;
            obj->f_B6 = 1;
            obj->f_B4 = 0x14;
            break;
        case 1:
            sub_8020974((ObjHead *)(&obj->headA), 0x3A0, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            b4 = obj->headA.palSlot;
            sub_804C3A4(b4, sub_801B954((ObjHead *)(&obj->headA)));
            obj->headA.kindFlags &= 0xEFFF;
            sub_8020974((ObjHead *)(&obj->headA), 0x3A1, 0x1B4, 0xD, 0x102);
            gObjActStep = 0x1C;
            break;
        case 28:
            if (obj->headA.kindFlags & 0x800)
                break;
            obj->headA.kindFlags &= 0xFEFF;
            sub_801A348();
            gObjActStep = 0x22;
            break;
        case 34:
            if (sub_8019B98(4, 3, 0xE, 0) == 0)
                break;
            Sfx_Play(0x6D, 1, 0);
            gSceneFadeOut = 0;
            sub_801A2AC(0x1C42, 0, 0x10);
            gObjActStep = 0x1D;
            break;
        case 29:
            gObjActStepTimer = 0;
            sub_804BDD8(0xE, 1, 1, -1, 0xF);
            gObjActStep = 0x1E;
            break;
        case 30:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0, 0xC, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x1C42, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            gSceneFadeOut = 0xC;
            sub_801A2AC(0x1C42, 0xC, 0x10);
            sub_8044514(0x3C);
            gObjActStep = 0x1F;
            break;
        case 31:
            if (gObjActStepTimer > 0x3B)
                gObjActStep = 0x20;
            else
                gObjActStepTimer += 1;
            break;
        case 32:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                gObjActStepTimer = 0;
                gObjActStep = 0x21;
            }
            break;
        case 33:
            if (gObjActStepTimer <= 0x13)
            {
                gSceneFadeOut = sub_801768C(0xC, -0xC, 0x14, gObjActStepTimer, 2);
                sub_801A2AC(0x1C42, gSceneFadeOut, 0x10);
                gObjActStepTimer += 1;
                break;
            }
            gObjActStepTimer = 0;
            Sfx_StopTrack(1);
            sub_801A2AC(0, 0, 0);
            obj->state &= 0x2000;
            sub_804BE90(0xE, 1);
            REG_DISPCNT &= 0xFDFF;
            gObjActStep = 0x14;
            break;
        case 18:
            sub_8044514(0x32);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80471AC() == 0)
                gObjActStep = 0x14;
            gObjActStepTimer += 1;
            break;
        case 20:
            sub_801CBA4(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 6;
            break;
        case 6:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(obj, obj->pad_A1);
            ret = 1;
            break;
        case 3:
        case 4:
        case 7:
        case 8:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
        case 16:
        case 17:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
            break;
    }
    sub_803F658(obj);
    return ret;
}

// @ 0x08037C40
u32 sub_8037C40(BattleObj *arg0, BattleObj *arg1)
{
    u32 ret;
    u8 b4;
    u32 zero;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = arg0->headA.palSlot;
            gObjActSavedF2A = arg0->headA.f_1E;
            sub_8048B30(1, 0x1E, 0x3AD);
            gObjActStep = 1;
            break;
        case 1:
            sub_8020974((ObjHead *)(&arg0->headA), 0x3A5, 0x1B4, 0xD, 2);
            gObjActStep = 2;
            break;
        case 2:
            if (arg0->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0xA5, 1, 0);
            gObjActStep = 5;
            break;
        case 5:
            if (!(arg0->headA.kindFlags & 0x1000))
                break;
            b4 = arg0->headA.palSlot;
            sub_804C3A4(b4, sub_801B954((ObjHead *)(&arg0->headA)));
            arg0->headA.kindFlags &= 0xEFFF;
            sub_8020974((ObjHead *)(&arg0->headA), 0x3A6, 0x1B4, 0xD, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (arg0->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_80476DC(arg0, arg1) == 1)
                gObjActStep = 0x14;
            break;
        case 20:
            sub_801CBA4(arg0, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 6;
            break;
        case 6:
            if (arg0->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(arg0, arg0->pad_A1);
            ret = 2;
            break;
    }
    return ret;
}

// @ 0x08037E14
u32 sub_8037E14(BattleObj *obj)
{
    u32 ret;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        sub_8020DE4();
        gObjActStep = 0x12;
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        break;
    case 18:
        sub_8020974(&obj->headA, 0x3B9, 0x1B4, 0xC, 2);
        gObjActStep = 0x13;
        break;
    case 19:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gObjActStep = 0x14;
        break;
    case 20:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)(&obj->headA)));
        obj->headA.kindFlags &= 0xEFFF;
        sub_8020974(&obj->headA, 0x3BA, 0x1B4, 0xC, 2);
        gObjActStep = 0x15;
        break;
    case 21:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 0, 0, 0x3C) == 1)
        {
            sub_801CBA4((BattleObj *)obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
        }
        break;
    case 23:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(obj, obj->pad_A1);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x08037FE8
u32 sub_8037FE8(BattleObj *obj)
{
    u32 ret;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        sub_8020DE4();
        gObjActStep = 0x12;
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        break;
    case 18:
        sub_8020974(&obj->headA, 0x3B9, 0x1B4, 0xD, 2);
        gObjActStep = 0x13;
        break;
    case 19:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gObjActStep = 0x14;
        break;
    case 20:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)(&obj->headA)));
        obj->headA.kindFlags &= 0xEFFF;
        sub_8020974(&obj->headA, 0x3BA, 0x1B4, 0xD, 2);
        gObjActStep = 0x15;
        break;
    case 21:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 1, 0, 0x28) == 1)
        {
            sub_801CBA4((BattleObj *)obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
        }
        break;
    case 23:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(obj, obj->pad_A1);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x080381BC
u32 sub_80381BC(BattleObj *obj)
{
    u32 ret;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        sub_8020DE4();
        gObjActStep = 0x12;
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        break;
    case 18:
        sub_8020974(&obj->headA, 0x3B9, 0x1B4, 0xD, 2);
        gObjActStep = 0x13;
        break;
    case 19:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gObjActStep = 0x14;
        break;
    case 20:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)(&obj->headA)));
        obj->headA.kindFlags &= 0xEFFF;
        sub_8020974(&obj->headA, 0x3BA, 0x1B4, 0xD, 2);
        gObjActStep = 0x15;
        break;
    case 21:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 1, 0xC, 0x1E) == 1)
        {
            sub_801CBA4((BattleObj *)obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
        }
        break;
    case 23:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(obj, obj->pad_A1);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x08038390
u32 sub_8038390(BattleObj *obj)
{
    u32 ret;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        sub_8020DE4();
        gObjActStep = 0x12;
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        break;
    case 18:
        sub_8020974(&obj->headA, 0x3B9, 0x1B4, 0xD, 2);
        gObjActStep = 0x13;
        break;
    case 19:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gObjActStep = 0x14;
        break;
    case 20:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)(&obj->headA)));
        obj->headA.kindFlags &= 0xEFFF;
        sub_8020974(&obj->headA, 0x3BA, 0x1B4, 0xD, 2);
        gObjActStep = 0x15;
        break;
    case 21:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 0, 0, 0x3E7) == 1)
        {
            sub_801CBA4((BattleObj *)obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
        }
        break;
    case 23:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(obj, obj->pad_A1);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x08038568
u32 sub_8038568(BattleObj *arg, BattleObj *arg1)
{
    u32 ret;
    u16 t1;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        sub_8020DE4();
        gObjActStep = 0x12;
        gObjActSavedPal = arg->headA.palSlot;
        gObjActSavedF2A = arg->headA.f_1E;
        break;
    case 18:
        sub_8020974((ObjHead *)(&arg->headA), 0x3B9, 0x1B4, 0xD, 2);
        gObjActStep = 0x13;
        break;
    case 19:
        if (arg->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gObjActStep = 0x14;
        break;
    case 20:
        if (!(arg->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(arg->headA.palSlot, sub_801B954((ObjHead *)(&arg->headA)));
        arg->headA.kindFlags &= 0xEFFF;
        sub_8020974((ObjHead *)(&arg->headA), 0x3BA, 0x1B4, 0xD, 2);
        gObjActStep = 0x15;
        break;
    case 21:
        if (!(arg->headA.kindFlags & 0x800))
            gObjActStep = 0x16;
        break;
    case 22:
        t1 = arg1->maxHp / 3;
        if (sub_801EEE4(arg, GetObjPool(), 0, 0xA, t1) == 1)
        {
            sub_801CBA4((BattleObj *)arg, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
        }
        break;
    case 23:
        if (!(arg->headA.kindFlags & 0x800))
            gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(arg, arg->pad_A1);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x0803874C
u32 sub_803874C(BattleObj *obj)
{
    u32 ret;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        sub_8020DE4();
        gObjActStep = 0x12;
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        break;
    case 18:
        sub_8020974(&obj->headA, 0x3B9, 0x1B4, 0xD, 2);
        gObjActStep = 0x13;
        break;
    case 19:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gObjActStep = 0x14;
        break;
    case 20:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(obj->headA.palSlot, sub_801B954((ObjHead *)(&obj->headA)));
        obj->headA.kindFlags &= 0xEFFF;
        sub_8020974(&obj->headA, 0x3BA, 0x1B4, 0xD, 2);
        gObjActStep = 0x15;
        break;
    case 21:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 0x16;
        break;
    case 22:
        if (sub_801EEE4(obj, GetObjPool(), 0, 0xB, 0x1E) == 1)
        {
            sub_801CBA4((BattleObj *)obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 0x17;
        }
        break;
    case 23:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(obj, obj->pad_A1);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x08038920
// 战斗演出状态机变体 (gObjActStep: 0 → 0x12..0x1C → 9), 双参 (arg0=自身, arg1=协作/目标对象):
// case0  记录 arg0 头位置 (SavedPal=headA.palSlot, SavedF2A=headA.f_1E) → 0x12。
// 0x12   sub_8020974(&headA, 0x3B9, 0x1B4, 0xD, 2) → 0x13。
// 0x13   等 headA.kindFlags 0x800 → Sfx_Play(0x3E,1,0) → 0x14。
// 0x14   等 0x1000 → sub_804C3A4(headA.palSlot, sub_801B954(&headA)), 清 0x1000
//        → sub_8020974(0x3BA...) → 0x15。
// 0x15   无 0x800 → 0x16。
// 0x16   sub_8020CC4(arg0, arg1.posX, arg1.posY, 0x27C, 0xE, 0x3C1, 0x2004) → 0x17。
// 0x17   等 arg0.headB 0x800 → sub_8020CC4(arg1, arg1.posX, 0, 0x2E0, 0xE, 0x3C2, 0x2005) → 0x18。
// 0x18   等 arg1.headB 0x800 → Sfx_Play(0x59,0,0) + StepTimer=0 → 0x19。
// 0x19   StepTimer<=0x27: arg1.headB.f_2C = sub_801768C(0, arg1.posY - ((u8)sub_801EC3C(arg1,1)>>1), 0x28,
//        StepTimer, 2), StepTimer++; 否则 sub_8020CC4(arg1, arg1.posX, arg1.posY, 0x2E0, 0xE, 0x3C4, 0x2005) → 0x1A。
// 0x1A   等 arg1.headB 0x800 → StepTimer=0 → 0x1B。
// 0x1B   StepTimer<=0x31 → StepTimer++; 否则清 arg1.state 的 0x2000, sub_80187B4 检验后收尾
//        (arg1.variantClass=7, arg1.slot=0xFF, sub_80207A4) + sub_801CBA4(arg0, 保存值) → 0x1C。
// 0x1C   无 headA 0x800 → Sfx_StopTrack(1) + 清 arg0.state 0x2000 → 9。
// 9      sub_8045B90(arg0, pad_A1), 返回 2。
u32 sub_8038920(BattleObj *arg0, BattleObj *arg1)
{
    u32 ret;
    u8 timer;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        gObjActStep = 0x12;
        gObjActSavedPal = arg0->headA.palSlot;
        gObjActSavedF2A = arg0->headA.f_1E;
        break;
    case 18:
        sub_8020974((ObjHead *)(&arg0->headA), 0x3B9, 0x1B4, 0xD, 2);
        gObjActStep = 0x13;
        break;
    case 19:
        if (arg0->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x3E, 1, 0);
        gObjActStep = 0x14;
        break;
    case 20:
        if (!(arg0->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
        arg0->headA.kindFlags &= 0xEFFF;
        sub_8020974((ObjHead *)(&arg0->headA), 0x3BA, 0x1B4, 0xD, 2);
        gObjActStep = 0x15;
        break;
    case 21:
        if (!(arg0->headA.kindFlags & 0x800))
            gObjActStep = 0x16;
        break;
    case 22:
        sub_8020CC4(arg0, arg1->posX, arg1->posY, 0x27C, 0xE, 0x3C1, 0x2004);
        gObjActStep = 0x17;
        break;
    case 23:
        if (arg0->headB.kindFlags & 0x800)
            break;
        sub_8020CC4(arg1, arg1->posX, 0, 0x2E0, 0xE, 0x3C2, 0x2005);
        gObjActStep = 0x18;
        break;
    case 24:
        if (arg1->headB.kindFlags & 0x800)
            break;
        Sfx_Play(0x59, 0, 0);
        gObjActStepTimer = 0;
        gObjActStep = 0x19;
        break;
    case 25:
        if (gObjActStepTimer <= 0x27)
        {
            arg1->headB.f_2C = sub_801768C(0, arg1->posY - ((u8)sub_801EC3C(arg1, 1) >> 1), 0x28, gObjActStepTimer, 2);
            gObjActStepTimer += 1;
        }
        else
        {
            sub_8020CC4(arg1, arg1->posX, arg1->posY, 0x2E0, 0xE, 0x3C4, 0x2005);
            gObjActStep = 0x1A;
        }
        break;
    case 26:
        if (arg1->headB.kindFlags & 0x800)
            break;
        gObjActStepTimer = 0;
        gObjActStep = 0x1B;
        break;
    case 27:
        timer = gObjActStepTimer;
        if (timer <= 0x31)
        {
            gObjActStepTimer = timer + 1;
            break;
        }
        arg1->state &= 0xDFFF;
        if ((sub_80187B4() & 0x220) == 0)
        {
            arg1->variantClass = 7;
            arg1->slot = 0xFF;
            sub_80207A4();
        }
        sub_801CBA4(arg0, 0, gObjActSavedF2A, gObjActSavedPal, 0);
        gObjActStep = 0x1C;
        break;
    case 28:
        if (arg0->headA.kindFlags & 0x800)
            break;
        Sfx_StopTrack(1);
        arg0->state &= 0xDFFF;
        gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(arg0, arg0->pad_A1);
        ret = 2;
        break;
    }
    return ret;
}
// @ 0x08038C84
// NPC 对话状态机变体 (gObjActStep 十态同 8032548; 开场动画固定 0x3C7):
// case0 存 NPC 位置 (0x03000828/29) 并清 [0xB4]/[0xB6]; case1 sub_803E58C 到位检查+
// 开场动画 0x3C7, arg1[0xBE]<=0xA 时打 0x20 标记; case2 等 0x800 后进 case3;
// case3 arg0[0x28]>0x39 (对话帧超时) 发音效并等 0x28 帧; case5 确认 (0x1000) →
// sub_804C3A4 + gObjActParam=0xC; case8/9 收尾同 8032548 (无遮挡时结束返回 1)。
// 注: case1 的 spr 提载与 b4=0x3C7 (动画 id 走寄存器) 是字节匹配必需的调度形状;
// case0 的 zero/zero2/b6ptr/b4 拆分同 8032548 (见 progress.md)。
u32 sub_8038C84(BattleObj *arg0, u8 *arg1)
{
    u32 ret;
    u16 keys;
    u32 zero;
    u32 zero2;
    u16 *b6ptr;
    u8 *spr;
    u16 b4;

    ret = 0;
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg0->posX;
        gObjActSavedY = arg0->posY;
        gObjActStepTimer = 0;
        sub_80444A4((BattleObj *)arg0);
        sub_803F5B4((BattleObj *)arg0);
        zero = 0;
        zero2 = 0;
        b6ptr = &(arg0->f_B6);
        b4 = zero2;
        *b6ptr = zero2;
        arg0->f_B4 = b4;
        gObjActStep = 1;
        gObjActParam = zero;
        break;
    case 1:
        if (sub_803E58C(arg0, arg1, 0) == 1)
        {
            spr = &arg0->headA;
            b4 = 0x3C7;
            sub_8020974(spr, b4, 0x1B4, 0xD, 2);
            if (arg1[0xBE] <= 0xA)
            {
                keys = arg0->headA.kindFlags | 0x20;
                arg0->headA.kindFlags = keys;
            }
            gObjActStep = 2;
        }
        break;
    case 2:
        if (arg0->headA.kindFlags & 0x800)
        {
            break;
        }
        gObjActStep = 3;
        break;
    case 3:
        if (arg0->headA.frameIdx <= 0x39)
        {
            break;
        }
        Sfx_Play(0x31, 1, 0);
        sub_8044514(0x28);
        gObjActStep = 5;
        break;
    case 5:
        if (arg0->headA.kindFlags & 0x1000)
        {
            sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
            gObjActParam = 0xC;
            gObjActStep = 8;
        }
        break;
    case 8:
        if (sub_803E58C(arg0, arg1, 0) == 1)
        {
            gObjActStep = 9;
        }
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            sub_8045B90(arg0, arg0->pad_A1);
            ret = 1;
        }
        break;
    }
    sub_803F658((BattleObj *)arg0);
    return ret;
}
#if 1
INCLUDE_ASM("asm/matchings", sub_8038E44);

#else
// @ 0x08038E44
// NPC 对话状态机变体 (gObjActStep 十态同 sub_8038C84; 起手 bl sub_80187E8 采样按键/连发状态):
// 与 sub_8038C84 同骨架, 仅三处不同: case1 到位检查 sub_803E58C mode=2 (非 0) 且开场动画固定
// 0x3C8 (非 0x3C7); case3 拆成"帧号==0x24 时 Sfx_Play(0x31)+等 0x1E 帧"与"帧号>0x43 时
// Sfx_Play(0x31)+等 0x28 帧→5"两段 (sub_8038C84 只有一帧>0x39→5)。case0/2/5/8/9 与收尾完全同。
u32 sub_8038E44(BattleObj *arg0, u8 *arg1)
{
    u32 ret;
    u16 keys;
    u32 zero;
    u32 zero2;
    u16 *b6ptr;
    u16 b4;

    ret = 0;
    sub_80187E8();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg0->posX;
        gObjActSavedY = arg0->posY;
        gObjActStepTimer = 0;
        sub_80444A4(arg0);
        sub_803F5B4(arg0);
        zero = 0;
        zero2 = 0;
        b6ptr = &(arg0->f_B6);
        b4 = zero2;
        *b6ptr = zero2;
        arg0->f_B4 = b4;
        gObjActStep = 1;
        gObjActParam = zero;
        break;
    case 1:
        if (sub_803E58C(arg0, arg1, 2) == 1)
        {
            sub_8020974(&arg0->headA, 0x3C8, 0x1B4, 0xD, 2);
            if (arg1[0xBE] <= 0xA)
            {
                keys = arg0->headA.kindFlags | 0x20;
                arg0->headA.kindFlags = keys;
            }
            gObjActStep = 2;
        }
        break;
    case 2:
        if (arg0->headA.kindFlags & 0x800)
        {
            break;
        }
        gObjActStep = 3;
        break;
    case 3:
        if (arg0->headA.frameIdx == 0x24)
        {
            Sfx_Play(0x31, 1, 0);
            sub_8044514(0x1E);
        }
        if (arg0->headA.frameIdx > 0x43)
        {
            Sfx_Play(0x31, 1, 0);
            sub_8044514(0x28);
            gObjActStep = 5;
        }
        break;
    case 5:
        if (arg0->headA.kindFlags & 0x1000)
        {
            sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954(&arg0->headA));
            gObjActParam = 0xC;
            gObjActStep = 8;
        }
        break;
    case 8:
        if (sub_803E58C(arg0, arg1, 0) == 1)
        {
            gObjActStep = 9;
        }
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            sub_8045B90(arg0, arg0->pad_A1);
            ret = 1;
        }
        break;
    }
    sub_803F658(arg0);
    return ret;
}
#endif
// @ 0x08039024
u32 sub_8039024(BattleObj *arg0, BattleObj *arg1)
{
    u32 ret;
    int b4;
    u16 flags;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedX = arg0->posX;
            gObjActSavedY = arg0->posY;
            gObjActStepTimer = 0;
            sub_80444A4(arg0);
            sub_803F5B4(arg0);
            arg0->f_B6 = 0;
            arg0->f_B4 = 0;
            gObjActStep = 0x12;
            gObjActParam = 0;
            break;
        case 18:
            if (sub_803E58C(arg0, arg1, 3) == 1)
            {
                sub_8020974((ObjHead *)(&arg0->headA), 0x3C9, 0x1B4, 0xD, 2);
                gObjActStep = 0x13;
            }
            break;
        case 19:
            if (arg0->headA.frameIdx <= 0x40)
                break;
            sub_8020CC4(arg0, (u8)(arg0->posX - 0x7D), (u8)(arg0->posY - 0x10), 0x27C, 0xF, 0x3CA, 0x105);
            gObjActStep = 0x14;
            break;
        case 20:
            if (arg0->headB.kindFlags & 0x800)
                break;
            Sfx_Play(0x1D, 1, 0);
            arg0->headB.kindFlags &= 0xFEFF;
            gObjActStep = 0x15;
            break;
        case 21:
            if (arg0->headA.frameIdx <= 0x45)
                break;
            sub_8044514(0x28);
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if ((arg0->state & 0x2000) && arg0->headB.frameIdx > 0xE)
            {
                arg0->headB.kindFlags &= 0xEFFF;
                arg0->state &= 0xDFFF;
            }
            if (arg0->headA.kindFlags & 0x1000)
            {
                b4 = arg0->headA.palSlot;
                sub_804C3A4(b4, sub_801B954((ObjHead *)(&arg0->headA)));
                flags = arg0->headA.kindFlags | 0x100;
                arg0->headA.kindFlags = flags;
                arg0->state &= 0xDFFF;
                gObjActParam = 0xC;
                gObjActStep = 0x17;
            }
            break;
        case 23:
            if (sub_803E58C(arg0, arg1, 0) == 1)
                gObjActStep = 9;
            break;
        case 9:
            if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
            {
                sub_8045B90(arg0, arg0->pad_A1);
                ret = 1;
            }
            break;
    }
    sub_803F658(arg0);
    return ret;
}

// @ 0x080392C0
u8 sub_80392C0(BattleObj *obj)
{
    u8 result;

    result = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            sub_8048B30(0, 0x1E, 0x3CB);
            gObjActStep = 0x13;
            break;
        case 19:
            if (sub_8047B1C(obj) == 1)
                gObjActStep = 0x14;
            break;
        case 20:
            sub_801CBA4((BattleObj *)obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 6;
            break;
        case 6:
            if (!(obj->headA.kindFlags & 0x800))
                gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(obj, obj->pad_A1);
            result = 2;
            break;
    }
    return result;
}
// @ 0x080393E0
u32 sub_80393E0(BattleObj *obj)
{
    u32 ret;
    int b4;

    ret = 0;
    switch (gObjActStep)
    {
        case 0:
            gObjActSavedPal = obj->headA.palSlot;
            gObjActSavedF2A = obj->headA.f_1E;
            gObjActSavedX = obj->posX;
            gObjActSavedY = obj->posY;
            gObjActStepTimer = 0;
            sub_80444A4(obj);
            sub_803F5B4(obj);
            obj->f_B6 = 0;
            obj->f_B4 = 0;
            sub_8020974((ObjHead *)(&obj->headA), 0x83, 0x1B4, 0xD, 2);
            gObjActStep = 0x12;
            break;
        case 18:
            if (obj->headA.kindFlags & 0x800)
                break;
            Sfx_Play(0x42, 1, 0);
            gObjActStep = 0x13;
            break;
        case 19:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            b4 = obj->headA.palSlot;
            sub_804C3A4(b4, sub_801B954((ObjHead *)(&obj->headA)));
            obj->headA.kindFlags &= 0xEFFF;
            sub_8020974((ObjHead *)(&obj->headA), 0x3D8, 0x1B4, 0xD, 0x502);
            gObjActStepTimer = 0;
            gObjActStep = 0x14;
            break;
        case 20:
            if (obj->headA.kindFlags & 0x800)
                break;
            obj->headA.kindFlags &= 0xFEFF;
            sub_8020CC4(obj, (u8)(obj->posX - 0x64), obj->posY, 0x218, 0xE, 0x3DE, 0x425);
            gObjActStep = 0x15;
            break;
        case 21:
            if (obj->headB.kindFlags & 0x800)
                break;
            sub_8044514(0x5A);
            gObjActStepTimer = 0;
            gObjActStep = 0x16;
            break;
        case 22:
            if ((u8)(gActWaitBusy0 - 1) > 5)
            {
                Sfx_Play(0x3A, 1, 1);
                gObjActStep = 0x17;
            }
            gObjActStepTimer += 1;
            break;
        case 23:
            if (gObjActStepTimer == 0x50)
            {
                Sfx_StopTrack(1);
                b4 = obj->headB.palSlot;
                sub_804C3A4(b4, sub_801B954((ObjHead *)(&obj->headB)));
                obj->state &= 0xDFFF;
            }
            if (gObjActStepTimer > 0x59)
            {
                sub_8020974((ObjHead *)(&obj->headA), 0x3D9, 0x1B4, 0xD, 2);
                gObjActStep = 0x18;
            }
            gObjActStepTimer += 1;
            break;
        case 24:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 0x19;
            break;
        case 25:
            if (!(obj->headA.kindFlags & 0x1000))
                break;
            b4 = obj->headA.palSlot;
            sub_804C3A4(b4, sub_801B954((ObjHead *)(&obj->headA)));
            sub_80207DC(obj, obj->posX, obj->posY, gObjActSavedF2A, gObjActSavedPal);
            gObjActStep = 0x1A;
            break;
        case 26:
            if (obj->headA.kindFlags & 0x800)
                break;
            gObjActStep = 9;
            break;
        case 9:
            sub_8045B90(obj, obj->pad_A1);
            ret = 1;
            break;
    }
    sub_803F658(obj);
    return ret;
}

#if 1
INCLUDE_ASM("asm/nonmatchings", sub_8039724);

#else

// @ 0x08039724
// 战斗对象"多段演出/目标池清理"状态机 (gObjActStep: 0 → 0x12..0x20 → 9)。
// case0 备份 posX/posY/palSlot/f_1E + sub_80444A4/sub_803F5B4 装配 + f_B4/f_B6 清零
//   + sub_8020974(0x3DA) 装载 + gTargetSlotCount=sub_80489E8(pool,gTargetSlotList,1,0x7F) 取敌侧候选;
// 0x12 等 headA.kindFlags&0x800 → Sfx(0x42); 0x13 等 &0x1000 → 释放调色板 + 清 0x1000
//   + sub_8020974(0x3DB,..,0x502) 换动画; 0x14 等 0x800 → 清 0x100 + Sfx(0x26);
// 0x15 等 Sfx_TrackBusy(1) ==0 → sub_8020974(0x3DC); 0x16 等 0x800 → Sfx(0x3A);
// 0x17 等 0x1000 → 释放调色板 + headA.kindFlags|=0x100 + sub_801A348 → 0x18;
// 0x18 sub_8019B98(4,3,0xE,0)!=0 → Sfx(0x6D) + gSceneFadeOut=0 + sub_801A2AC(0x1C42,0,0x10);
// 0x19 清步计数 + sub_804BDD8(0xE,1,1,-1,0xF); 0x1A 帧计数<=0x13 时 sub_801768C(0,0xC,0x14,t,2)
//   淡入否则置 0xC 收尾 → 0x1B; 0x1B 帧计数<=0x31 自增, 超时按 sub_80187B4()&0x220 决定是否
//   遍历 gTargetSlotList 把每个候选对象 slot(+0xBE)=0xFF、variantClass(+0xAB)=7 并 sub_80207A4 → 0x1C;
// 0x1C 帧计数<=0x13 时 sub_801768C(0xC,-0xC,0x14,t,2) 淡出, 超时 Sfx_StopTrack(1)+sub_801A2AC(0,0,0)
//   + obj->state&=0x2000 + sub_804BE90(0xE,1) + 清 DISPCNT bit9 → 0x1D;
// 0x1D sub_8020974(0x3DD,..,0x102); 0x1E 等 0x800 清 0x100 → 0x1F;
// 0x1F 等 0x1000 释放调色板 + sub_80207DC 收尾绘制 → 0x20; 0x20 等 0x800 → 9; 9 sub_8045B90 ret=1.
// 尾部 sub_803F658。bytecmp: 仅 34 个 bl 槽除外, 其余字节全等。
u32 sub_8039724(BattleObj *obj)
{
    u32 ret;
    u32 pool;
    int b4;
    u8 *ptr;
    u8 i;

    ret = 0;
    pool = GetObjPool();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        gObjActSavedX = obj->posX;
        gObjActSavedY = obj->posY;
        gObjActStepTimer = 0;
        sub_80444A4(obj);
        sub_803F5B4(obj);
        obj->f_B6 = 0;
        obj->f_B4 = 0;
        sub_8020974(&obj->headA, 0x3DA, 0x1B4, 0xD, 2);
        gTargetSlotCount = sub_80489E8(pool, gTargetSlotList, 1, 0x7F);
        gObjActStep = 0x12;
        break;
    case 18:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x42, 1, 0);
        gObjActStep = 0x13;
        break;
    case 19:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        b4 = obj->headA.palSlot;
        sub_804C3A4(b4, (u8)sub_801B954(&obj->headA));
        obj->headA.kindFlags &= 0xEFFF;
        sub_8020974(&obj->headA, 0x3DB, 0x1B4, 0xD, 0x502);
        gObjActStepTimer = 0;
        gObjActStep = 0x14;
        break;
    case 20:
        if (obj->headA.kindFlags & 0x800)
            break;
        obj->headA.kindFlags &= 0xFEFF;
        Sfx_Play(0x26, 1, 0);
        gObjActStep = 0x15;
        break;
    case 21:
        if ((u8)Sfx_TrackBusy(1) != 0)
            break;
        sub_8020974(&obj->headA, 0x3DC, 0x1B4, 0xD, 2);
        gObjActStep = 0x16;
        break;
    case 22:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x3A, 1, 0);
        gObjActStep = 0x17;
        break;
    case 23:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        b4 = obj->headA.palSlot;
        sub_804C3A4(b4, (u8)sub_801B954(&obj->headA));
        obj->headA.kindFlags |= 0x100;
        sub_801A348();
        gObjActStep = 0x18;
        break;
    case 24:
        if (sub_8019B98(4, 3, 0xE, 0) == 0)
            break;
        Sfx_Play(0x6D, 1, 0);
        gSceneFadeOut = 0;
        sub_801A2AC(0x1C42, 0, 0x10);
        gObjActStep = 0x19;
        break;
    case 25:
        gObjActStepTimer = 0;
        sub_804BDD8(0xE, 1, 1, -1, 0xF);
        gObjActStep = 0x1A;
        break;
    case 26:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0, 0xC, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1C42, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gSceneFadeOut = 0xC;
        sub_801A2AC(0x1C42, 0xC, 0x10);
        gObjActStep = 0x1B;
        break;
    case 27:
        if (gObjActStepTimer <= 0x31)
        {
            gObjActStepTimer += 1;
            break;
        }
        if (!(sub_80187B4() & 0x220))
        {
            for (i = 0; i < gTargetSlotCount; i++)
            {
                ptr = (u8 *)(pool + gTargetSlotList[i] * 0xC8);
                ptr[0xBE] = 0xFF;
                ptr[0xAB] = 7;
                sub_80207A4();
            }
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x1C;
        break;
    case 28:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0xC, -0xC, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1C42, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        Sfx_StopTrack(1);
        sub_801A2AC(0, 0, 0);
        obj->state &= 0x2000;
        sub_804BE90(0xE, 1);
        *(volatile u16 *)(0x80 << 0x13) &= 0xFDFF;
        gObjActStep = 0x1D;
        break;
    case 29:
        sub_8020974(&obj->headA, 0x3DD, 0x1B4, 0xD, 0x102);
        gObjActStep = 0x1E;
        break;
    case 30:
        if (obj->headA.kindFlags & 0x800)
            break;
        obj->headA.kindFlags &= 0xFEFF;
        gObjActStep = 0x1F;
        break;
    case 31:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        b4 = obj->headA.palSlot;
        sub_804C3A4(b4, (u8)sub_801B954(&obj->headA));
        sub_80207DC(obj, obj->posX, obj->posY, gObjActSavedF2A, gObjActSavedPal);
        gObjActStep = 0x20;
        break;
    case 32:
        if (obj->headA.kindFlags & 0x800)
            break;
        gObjActStep = 9;
        break;
    case 9:
        sub_8045B90(obj, obj->pad_A1);
        ret = 1;
        break;
    }
    sub_803F658(obj);
    return ret;
}
    #endif
// @ 0x08039C38
// 战斗对象"场景淡出/淡入+演出序号"状态机 (gObjActStep 0 → 0x12..0x29 → 9; 单参 obj, arg1=协作对象):
// case0  存 posX/posY, 初始化 (f_B6=f_B4=0), 起手调色/闪屏复位 + sub_801A2AC(0x1F4F,0,0x10) → 0x12。
// 0x12   sub_8019B98(4,2,0xE,1) 等待 → ctx->state|=0x400 + Sfx(0x59) + sub_804BDD8 → 0x13。
// 0x13/0x14 (0x1F4F 窗口) 与 0x16/0x17/0x18 (0x1F47 窗口) 为两段淡出/淡入插值, 用
//        gSceneFadeOut/In + sub_801768C/sub_801A2AC; 0x15/0x18 等待+复位。
// 0x18   等 0x800 落 → 0x8000 标记? 逐步推进到 0x1E (子动画 0x365/0x377) → 0x1F..0x29。
// 0x1C..0x23 双段 sub_8020CC4 锚点动画 + sub_801768C 位移; 0x24..0x29 等 gActWaitBusy 清零后
//        BgLoad_Reset → 9。
// case9  BgLoad_GetPos()>3 → sub_80209EC(obj); 返回 1。尾部 sub_803F658(obj)。
// 注: 跳转表含 38 项 (0..0x2A; 0x2A=case42 置于 case30 之后, 对应 ROM 物理布局);
// case37 用 gObjActParam % 4 索引 gUnk_0839DF67 (u8&3 语义等价, 但 % 形式才得目标寄存器分配);
// case22 的 sub_801768C 第 5 参为 1, case19/20/23 为 2 (勿统一)。
u32 sub_8039C38(BattleObj *obj, u8 *arg1)
{
    u32 ret;
    BattleObj *ctx;
    u8 sel;

    ret = 0;
    ctx = (BattleObj *)GetCtx_0248();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = obj->posX;
        gObjActSavedY = obj->posY;
        gObjActStepTimer = 0;
        sub_80444A4(obj);
        sub_803F5B4(obj);
        obj->f_B6 = 0;
        obj->f_B4 = 0;
        gObjActStep = 0x12;
        sub_80187C0(0x10);
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
        break;
    case 18:
        if (sub_8019B98(4, 2, 0xE, 1) == 0)
            break;
        ctx->state |= 0x400;
        Sfx_Play(0x59, 0, 1);
        gObjActStepTimer = 0;
        sub_804BDD8(0xE, 1, 1, -1, 0xF);
        gObjActStep = 0x13;
        break;
    case 19:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x14;
        break;
    case 20:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
            break;
        }
        sub_801A348();
        gObjActStep = 0x15;
        break;
    case 21:
        if (obj->slot == 0)
            sel = 5;
        else
            sel = 6;
        if (sub_8019B98(sel, 3, 0xF, 0) == 0)
            break;
        gObjActStepTimer = 0;
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F47, 0, 0x10);
        gObjActStep = 0x16;
        break;
    case 22:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 1);
            sub_801A2AC(0x1F47, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x17;
        break;
    case 23:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
            break;
        }
        gObjActStepTimer = 0;
        sub_801A2AC(0, 0, 0);
        gObjActStep = 0x18;
        break;
    case 24:
        if (gObjActStepTimer <= 0x31)
        {
            gObjActStepTimer++;
            break;
        }
        Sfx_StopTrack(0);
        sub_8019AD0(5, 0x110);
        gObjActStep = 0x19;
        break;
    case 25:
        if (!(FlashFlag_Get() & 0x4000))
            break;
        FlashFlag_Reset(0x4000);
        sub_8019AD0(0xA, 0x120);
        sub_804BE90(0xE, 1);
        BattleUiFlag_Set(0xA0);
        gObjActStep = 0x1A;
        break;
    case 26:
        if (!(FlashFlag_Get() & 0x4000))
            break;
        sub_80187D4(0x10);
        FlashFlag_Reset(0x4000);
        BattleFx_DispOff();
        sub_8018BF8();
        BattleUiFlag_Set(8);
        ctx->state &= 0xFBFF;
        gObjActParam = 0;
        gObjActStep = 0x1B;
        break;
    case 27:
        if (sub_803E58C(obj, arg1, 3) != 1)
            break;
        gUnk_0300086C = (obj->slot == 0) ? 0x365 : 0x377;
        sub_8020974(&obj->headA, gUnk_0300086C, 0x1B4, 0xD, 2);
        gObjActStep = 0x1C;
        break;
    case 28:
        if (obj->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x1D;
        Sfx_Play(0x9A, 0, 0);
        break;
    case 29:
        if (!(obj->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(obj->headA.palSlot, (u8)sub_801B954(&obj->headA));
        gUnk_0300086C++;
        sub_8020974(&obj->headA, gUnk_0300086C, 0x1B4, 0xD, 0x102);
        gObjActStep = 0x1E;
        break;
    case 30:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x6F, 1, 0);
        obj->headA.kindFlags &= 0xFEFF;
        sub_8020CC4(obj, 0x50, 0x64, 0x27C, 0xE, 0x381, 5);
        obj->headB.f_2B = obj->posX;
        obj->headB.f_2C = obj->posY;
        gObjActStepTimer = 0;
        gObjActStep = 0x2A;
        break;
    case 42:
        if (obj->headB.kindFlags & 0x800)
            break;
        Sfx_Play(0x55, 2, 0);
        gObjActStep = 0x1F;
        break;
    case 31:
        if (gObjActStepTimer <= 0x3B)
        {
            gObjActStepTimer++;
            break;
        }
        gUnk_0300086C++;
        sub_8020974(&obj->headA, gUnk_0300086C, 0x1B4, 0xD, 2);
        gObjActStep = 0x20;
        break;
    case 32:
        if (obj->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x6E, 1, 0);
        gObjActStep = 0x21;
        break;
    case 33:
        if (obj->headA.frameIdx <= 0x15)
            break;
        gObjActStepTimer = 0;
        gObjActStep = 0x22;
        break;
    case 34:
        if (obj->headA.kindFlags & 0x1000)
        {
            sub_804C3A4(obj->headA.palSlot, (u8)sub_801B954(&obj->headA));
            obj->headA.kindFlags |= 0x100;
        }
        if (gObjActStepTimer <= 0x13)
        {
            obj->headB.f_2B = sub_801768C(obj->posX, -0x48, 0x14, gObjActStepTimer, 2);
            gObjActStepTimer++;
            break;
        }
        Sfx_StopTrack(2);
        gObjActStepTimer = 0;
        sub_8044514(0x14);
        gObjActStep = 0x23;
        break;
    case 35:
        if (!(obj->headA.kindFlags & 0x1000))
        {
            gObjActStepTimer++;
            break;
        }
        sub_804C3A4(obj->headA.palSlot, (u8)sub_801B954(&obj->headA));
        obj->headA.kindFlags |= 0x100;
        sub_8020CC4(obj, 0x50, 0x64, 0x27C, 0xE, 0x382, 5);
        gObjActStep = 0x24;
        break;
    case 36:
        if (gActWaitBusy0 != 0 || gActWaitBusy1 != 0 || gActWaitBusy2 != 0)
            break;
        if (obj->headB.kindFlags & 0x800)
            break;
        gSceneFadeIn = obj->headB.f_2C;
        sub_8044514(0x50);
        gObjActParam = 0;
        gObjActStep = 0x25;
        break;
    case 37:
        gSceneFadeOut = gUnk_0839DF67[(u8)(gObjActParam % 4)];
        gObjActStep = 0x26;
        break;
    case 38:
        if (gObjActStepTimer <= 9)
        {
            obj->headB.f_2B = sub_801768C(0x38, gSceneFadeOut - 0x38, 0xA, gObjActStepTimer, 2);
            obj->headB.f_2C = sub_801768C(gSceneFadeIn, -gSceneFadeIn, 0xA, gObjActStepTimer, 2);
            gObjActStepTimer++;
            break;
        }
        gObjActStepTimer = 0;
        gObjActParam++;
        if (gObjActParam <= 7)
        {
            gObjActStep = 0x25;
            break;
        }
        obj->state &= 0xDFFF;
        gObjActParam = 0xC;
        gObjActStep = 0x28;
        break;
    case 40:
        if (sub_803E58C(obj, arg1, 3) != 1)
            break;
        gObjActStep = 0x29;
        break;
    case 41:
        if (gActWaitBusy0 != 0 || gActWaitBusy1 != 0 || gActWaitBusy2 != 0)
            break;
        BgLoad_Reset();
        gObjActStep = 9;
        break;
    case 9:
        if (BgLoad_GetPos() <= 3)
            break;
        sub_80209EC(obj);
        ret = 1;
        break;
    }
    sub_803F658(obj);
    return ret;
}

// @ 0x0803A478
// 战斗对象"场景混合/动画恢复"状态机 (动作 selector 0x31, gObjActStep 0 → 18..26 → 1 → 2 → 5 → 6 → 8 → 9):
// case0  清辅助对象链, 存 SavedPal/SavedF2A, 起手 sub_80187C0(0x10)+sub_801A348+混合控制 0x1F4F。
// case18 等 sub_8019B98(4,2,14,1), 抑制辅助对象更新 (context->state |= 0x400), Sfx 0x59, timer=0,
//        启动调色板动画 sub_804BDD8(0xE,1,1,-1,0xF) → 19。
// 19/20  第一段 BLD 混合系数插值 (0x1F4F 窗口, 0→0x10/0x10→0) → 21。
// 21     等 sub_8019B98(7,3,15,0), 清 timer/第一系数, 切 0x1F47 → 22。
// 22/23  第二段插值; 结束清混合寄存器 → 24。
// 24     timer<=49 计数, 超时 Sfx_StopTrack(0)+sub_8019AD0(5,0x110) → 25。
// 25/26  两阶段等 FlashFlag 0x4000; 26 恢复引擎标志/关特效/UI 标志/解除抑制 (state &= 0xFBFF) → 1。
// 1/2/5/6 动画 0x386 装载 + headA 标志轮询 + Sfx 0x3E + sub_801CBA4 恢复动画参数。
// 8      sub_801EEE4(obj,pool,1,2,999)==1 → BgLoad_Reset → 9。
// 9      BgLoad_GetPos()>3 → sub_80209EC(obj), 返回 2; 其余返回 0。
// 注: context 用 BattleObj* + context->state 访问是字节匹配必需 (u8*+裸偏移会退化寄存器分配,
//     case18 的 movs r4,#0 会排到 orrs 之后 —— 见 BLOCKED-803A478 union 交接)。
// case0 的两次 gObjActStep=18 保留 (ROM 原始形状)。
u32 sub_803A478(BattleObj *obj)
{
    u32 result;
    BattleObj *context;
    int palSlot;
    u32 timerStart;

    result = 0;
    context = (BattleObj *)GetCtx_0248();
    switch (gObjActStep)
    {
    case 0:
        sub_8020DE4();
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        gObjActStep = 18;
        sub_80187C0(0x10);
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F4F, 0, 16);
        gObjActStep = 18;
        break;
    case 18:
        if (sub_8019B98(4, 2, 14, 1))
        {
            context->state |= 0x400;
            timerStart = 0;
            Sfx_Play(0x59, 0, 1);
            gObjActStepTimer = timerStart;
            sub_804BDD8(14, 1, 1, -1, 15);
            gObjActStep = 19;
        }
        break;
    case 19:
        if (gObjActStepTimer <= 19)
        {
            gSceneFadeOut = sub_801768C(0, 16, 20, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, 16);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 20;
        }
        break;
    case 20:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(16, -16, 10, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            sub_801A348();
            gObjActStep = 21;
        }
        break;
    case 21:
        if (sub_8019B98(7, 3, 15, 0))
        {
            gObjActStepTimer = 0;
            gSceneFadeOut = 0;
            sub_801A2AC(0x1F47, 0, 16);
            gObjActStep = 22;
        }
        break;
    case 22:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeOut = sub_801768C(0, 16, 10, gObjActStepTimer, 1);
            sub_801A2AC(0x1F47, gSceneFadeOut, 16);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 23;
        }
        break;
    case 23:
        if (gObjActStepTimer <= 19)
        {
            gSceneFadeIn = sub_801768C(16, -16, 20, gObjActStepTimer, 2);
            sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            sub_801A2AC(0, 0, 0);
            gObjActStep = 24;
        }
        break;
    case 24:
        if (gObjActStepTimer <= 49)
            gObjActStepTimer++;
        else
        {
            Sfx_StopTrack(0);
            sub_8019AD0(5, 0x110);
            gObjActStep = 25;
        }
        break;
    case 25:
        if (FlashFlag_Get() & 0x4000)
        {
            FlashFlag_Reset(0x4000);
            sub_8019AD0(10, 0x120);
            sub_804BE90(14, 1);
            BattleUiFlag_Set(0xA0);
            gObjActStep = 26;
        }
        break;
    case 26:
        if (FlashFlag_Get() & 0x4000)
        {
            sub_80187D4(0x10);
            FlashFlag_Reset(0x4000);
            BattleFx_DispOff();
            sub_8018BF8();
            context->state &= 0xFBFF;
            BattleUiFlag_Set(8);
            gObjActStep = 1;
        }
        break;
    case 1:
        sub_8020974(&obj->headA, 0x386, 0x1B4, 13, 2);
        gObjActStep = 2;
        break;
    case 2:
        if (!(obj->headA.kindFlags & 0x800))
        {
            Sfx_Play(0x3E, 1, 0);
            gObjActStep = 5;
        }
        break;
    case 5:
        if (obj->headA.kindFlags & 0x1000)
        {
            palSlot = obj->headA.palSlot;
            sub_804C3A4(palSlot, sub_801B954(&obj->headA));
            obj->headA.kindFlags &= 0xEFFF;
            sub_801CBA4(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
            gObjActStep = 6;
        }
        break;
    case 6:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 8;
        break;
    case 8:
        if (sub_801EEE4(obj, GetObjPool(), 1, 2, 999) == 1)
        {
            BgLoad_Reset();
            gObjActStep = 9;
        }
        break;
    case 9:
        if (BgLoad_GetPos() > 3)
        {
            sub_80209EC(obj);
            result = 2;
        }
        break;
    }
    return result;
}

// @ 0x0803A8D0
// 战斗对象"场景混合/双头动画编排"长状态机 (gObjActStep 0 → 0x12..0x25 → 9, 单参 obj):
// case0  存 posX/posY, f_B6=0x39B, f_B4=0, sub_80187C0(0x10)+sub_801A348+混合控制 0x1F4F。
// 18     等 sub_8019B98(4,2,14,1), 抑制辅助对象更新 (context->state |= 0x400), Sfx 0x59,
//        timer=0, sub_804BDD8(0xE,1,1,-1,0xF) → 0x13。
// 0x13/0x14 第一段 BLD 插值 (0x1F4F); 0x16/0x17/0x18 第二段 (0x1F47); 0x19 收尾 BLD 清零。
// 0x1A/0x1B 两阶段 FlashFlag 0x4000 等待; 0x1B 解除抑制 (state &= 0xFBFF) + BgLoad_Reset。
// 0x1C 等 BgLoad_GetPos>3 → 0x1D sub_8020CC4(0x50,0x64,...,0x38D,5)。
// 0x1E 等 headB 0x800 落 → Sfx 0x97; 0x1F headB 0x1000 → 还原调色 + 锚点 0x38E/0x405。
// 0x20 等 headB 0x800 落 → Sfx 0x66 + timer=0; 0x21 timer<=0x1D 时 headB.f_2C 插值
//      sub_801768C(0x64,-0x82,0x1E,t,1), 否则锚点 0x399/0x405 + headB.f_2A=0。
// 0x22 等 headB 0x800 落 → sub_804BBDC(0,3,0xA,0xA,4,4,-1,2) + timer=0。
// 0x23 timer<=0x1D 则 ++; 否则 sub_804BD54(0,3) + 锚点 0x39A/5 + headB.f_2A=0。
// 0x24 等 headB 0x800 落 → sub_8044514(0x28) + sub_804BF14(0,3,0xF,0xF,6,4,4,-1,2) + timer=0。
// 0x25 timer<=3 则 ++; 否则 sub_804C728(0,3,0x20) → 9。
// 9     等 headB 0x1000 → 还原调色 + Sfx_StopTrack(1) + obj.state &= 0xDFFF 转 9; 每帧 timer++。
// 尾    三 gActWaitBusy 清零 → sub_80209EC(obj), 返回 1。尾部 sub_803F658(obj)。
// 注: context 用 BattleObj* + context->state (同 sub_803A478 的字节匹配关键);
//     0x21/0x23 的 sub_8020CC4 第 4 参是 0x1B4 (0xDA<<1), 0x1D 是 0x27C (0x9F<<2) —— 勿混。
u32 sub_803A8D0(BattleObj *obj)
{
    u32 result;
    BattleObj *context;
    int palSlot;
    u32 timerStart;

    result = 0;
    context = (BattleObj *)GetCtx_0248();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = obj->posX;
        gObjActSavedY = obj->posY;
        gObjActStepTimer = 0;
        sub_80444A4(obj);
        sub_803F5B4(obj);
        obj->f_B6 = 0x39B;
        obj->f_B4 = 0;
        gObjActStep = 0x12;
        sub_80187C0(0x10);
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
        break;
    case 18:
        if (sub_8019B98(4, 2, 0xE, 1))
        {
            context->state |= 0x400;
            timerStart = 0;
            Sfx_Play(0x59, 0, 1);
            gObjActStepTimer = timerStart;
            sub_804BDD8(0xE, 1, 1, -1, 0xF);
            gObjActStep = 0x13;
        }
        break;
    case 19:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 0x14;
        }
        break;
    case 20:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            sub_801A348();
            gObjActStep = 0x15;
        }
        break;
    case 21:
        if (sub_8019B98(8, 3, 0xF, 0))
        {
            gObjActStepTimer = 0;
            gSceneFadeOut = 0;
            sub_801A2AC(0x1F47, 0, 0x10);
            gObjActStep = 0x16;
        }
        break;
    case 22:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 1);
            sub_801A2AC(0x1F47, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 0x17;
        }
        break;
    case 23:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            sub_801A2AC(0, 0, 0);
            gObjActStep = 0x18;
        }
        break;
    case 24:
        if (gObjActStepTimer <= 0x31)
            gObjActStepTimer++;
        else
        {
            Sfx_StopTrack(0);
            sub_8019AD0(5, 0x110);
            gObjActStep = 0x19;
        }
        break;
    case 25:
        if (FlashFlag_Get() & 0x4000)
        {
            FlashFlag_Reset(0x4000);
            sub_8019AD0(0xA, 0x120);
            sub_804BE90(0xE, 1);
            BattleUiFlag_Set(0xA0);
            gObjActStep = 0x1A;
        }
        break;
    case 26:
        if (FlashFlag_Get() & 0x4000)
        {
            sub_80187D4(0x10);
            FlashFlag_Reset(0x4000);
            BattleFx_DispOff();
            sub_8018BF8();
            BattleUiFlag_Set(8);
            context->state &= 0xFBFF;
            BgLoad_Reset();
            gObjActStep = 0x1B;
        }
        break;
    case 27:
        if (BgLoad_GetPos() > 3)
            gObjActStep = 0x1C;
        break;
    case 28:
        sub_8020CC4(obj, 0x50, 0x64, 0x27C, 0xE, 0x38D, 5);
        gObjActStep = 0x1D;
        break;
    case 29:
        if (!(obj->headB.kindFlags & 0x800))
        {
            Sfx_Play(0x97, 1, 0);
            gObjActStep = 0x1E;
        }
        break;
    case 30:
        if (!(obj->headB.kindFlags & 0x1000))
            break;
        palSlot = obj->headB.palSlot;
        sub_804C3A4(palSlot, (u8)sub_801B954(&obj->headB));
        sub_8020CC4(obj, 0x50, 0x64, 0x27C, 0xE, 0x38E, 0x405);
        gObjActStep = 0x1F;
        break;
    case 31:
        if (obj->headB.kindFlags & 0x800)
            break;
        Sfx_Play(0x66, 1, 1);
        gObjActStepTimer = 0;
        gObjActStep = 0x20;
        break;
    case 32:
        if (gObjActStepTimer <= 0x1D)
        {
            obj->headB.f_2C = sub_801768C(0x64, -0x82, 0x1E, gObjActStepTimer, 1);
            gObjActStepTimer++;
        }
        else
        {
            sub_8020CC4(obj, 0x78, 0x50, 0x1B4, 0xE, 0x399, 0x405);
            obj->headB.f_2A = 0;
            gObjActStep = 0x21;
        }
        break;
    case 33:
        if (obj->headB.kindFlags & 0x800)
            break;
        sub_804BBDC(0, 3, 0xA, 0xA, 4, 4, -1, 2);
        gObjActStepTimer = 0;
        gObjActStep = 0x22;
        break;
    case 34:
        if (gObjActStepTimer <= 0x1D)
        {
            gObjActStepTimer++;
            break;
        }
        gObjActStepTimer = 0;
        sub_804BD54(0, 3);
        sub_8020CC4(obj, 0x50, 0x78, 0x1B4, 0xE, 0x39A, 5);
        obj->headB.f_2A = 0;
        gObjActStep = 0x23;
        break;
    case 35:
        if (obj->headB.kindFlags & 0x800)
            break;
        sub_8044514(0x28);
        sub_804BF14(0, 3, 0xF, 0xF, 6, 4, 4, -1, 2);
        gObjActStepTimer = 0;
        gObjActStep = 0x24;
        break;
    case 36:
        if (gObjActStepTimer <= 3)
        {
            gObjActStepTimer++;
            break;
        }
        sub_804C728(0, 3, 0x20);
        gObjActStep = 0x25;
        break;
    case 37:
        if (obj->headB.kindFlags & 0x1000)
        {
            palSlot = obj->headB.palSlot;
            sub_804C3A4(palSlot, (u8)sub_801B954(&obj->headB));
            Sfx_StopTrack(1);
            obj->state &= 0xDFFF;
            gObjActStep = 9;
        }
        gObjActStepTimer++;
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            sub_80209EC(obj);
            result = 1;
        }
        break;
    }
    sub_803F658(obj);
    return result;
}

// @ 0x0803AF60
// 战斗对象"场景混合/对象状态收集"状态机 (gObjActStep 0 → 0x12..0x1E → 9, 单参 obj):
// case0      存 posX/posY, timer=0, sub_80187C0(0x10)+sub_801A348+混合控制 0x1F4F。
// 18         等 sub_8019B98(4,2,14,1), ctx->state|=0x400, timer=0, Sfx 0x59,
//            sub_804BDD8(0xE,1,1,-1,0xF) → 0x13。
// 0x13/0x14   第一段 BLD 插值 (0x1F4F, 0→0x10 用 0x28 步 / 0x10→0 用 0x14 步)。
// 0x16/0x17/0x18 第二段插值 (0x1F47); 0x19 timer<=0x31 后 Sfx_StopTrack(0)+sub_8019AD0(5,0x110)。
// 0x1A/0x1B   两阶段 FlashFlag 0x4000; 0x1B 解除抑制 (state&=0xFBFF) + BgLoad_Reset。
// 0x1C        BgLoad_GetPos>3 → sub_8020CC4(0x80,0x50,0x1B4,0xE,0x3B5,5) + headB.f_2A=0。
// 0x1D        等 headB 0x800 落 → Sfx 0x58 + sub_804BF14(0,3,0xF,0xF,6,4,4,-1,2) + timer=0。
// 0x1E        timer<=3 后 sub_804C728(0,3,0x20) + timer=0。
// 9           timer<=0x1F 则 ++; 否则取对象池 sub_80489E8(pool,buf,0,0xFF), 对每个候选槽
//             slot->state|=0x10 且 slot->[0xBA]=0, 再 obj->state|=0x10 + obj->[0xBA]=0,
//             sub_80209EC(obj), 返回 2。
// 注: case9 的 `(u8*)pool + (buf[i]*0xC8) + off` 写法是字节匹配必需 (触发 movs r7,#0xC8
//     立即数而非字面池; 见 handoff 的 4B 卡点)。case0 不调 sub_80444A4/sub_803F5B4。
u32 sub_803AF60(BattleObj *obj)
{
    u32 result;
    BattleObj *context;
    BattleObj *pool;
    u8 buf[8];
    u8 count;
    u8 i;

    result = 0;
    context = (BattleObj *)GetCtx_0248();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = obj->posX;
        gObjActSavedY = obj->posY;
        gObjActStepTimer = 0;
        gObjActStep = 0x12;
        sub_80187C0(0x10);
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F4F, 0, 0x10);
        break;
    case 18:
        if (sub_8019B98(4, 2, 0xE, 1))
        {
            context->state |= 0x400;
            gObjActStepTimer = 0;
            Sfx_Play(0x59, 0, 1);
            sub_804BDD8(0xE, 1, 1, -1, 0xF);
            gObjActStep = 0x13;
        }
        break;
    case 19:
        if (gObjActStepTimer <= 0x27)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0x28, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 0x14;
        }
        break;
    case 20:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            sub_801A348();
            gObjActStep = 0x15;
        }
        break;
    case 21:
        if (sub_8019B98(9, 3, 0xF, 0))
        {
            gObjActStepTimer = 0;
            gSceneFadeOut = 0;
            sub_801A2AC(0x1F47, 0, 0x10);
            gObjActStep = 0x16;
        }
        break;
    case 22:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 1);
            sub_801A2AC(0x1F47, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 0x17;
        }
        break;
    case 23:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            sub_801A2AC(0, 0, 0);
            gObjActStep = 0x18;
        }
        break;
    case 24:
        if (gObjActStepTimer <= 0x31)
            gObjActStepTimer++;
        else
        {
            Sfx_StopTrack(0);
            sub_8019AD0(5, 0x110);
            gObjActStep = 0x19;
        }
        break;
    case 25:
        if (FlashFlag_Get() & 0x4000)
        {
            FlashFlag_Reset(0x4000);
            sub_8019AD0(0xA, 0x120);
            sub_804BE90(0xE, 1);
            BattleUiFlag_Set(0xA0);
            gObjActStep = 0x1A;
        }
        break;
    case 26:
        if (FlashFlag_Get() & 0x4000)
        {
            sub_80187D4(0x10);
            FlashFlag_Reset(0x4000);
            BattleFx_DispOff();
            sub_8018BF8();
            BattleUiFlag_Set(8);
            context->state &= 0xFBFF;
            BgLoad_Reset();
            gObjActStep = 0x1B;
        }
        break;
    case 27:
        if (BgLoad_GetPos() > 3)
        {
            sub_8020CC4(obj, 0x80, 0x50, 0x1B4, 0xE, 0x3B5, 5);
            obj->headB.f_2A = 0;
            gObjActStep = 0x1C;
        }
        break;
    case 28:
        if (!(obj->headB.kindFlags & 0x800))
        {
            Sfx_Play(0x58, 0, 0);
            sub_804BF14(0, 3, 0xF, 0xF, 6, 4, 4, -1, 2);
            gObjActStepTimer = 0;
            gObjActStep = 0x1D;
        }
        break;
    case 29:
        if (gObjActStepTimer <= 3)
            gObjActStepTimer++;
        else
        {
            sub_804C728(0, 3, 0x20);
            gObjActStepTimer = 0;
            gObjActStep = 0x1E;
        }
        break;
    case 30:
        if (obj->headB.kindFlags & 0x1000)
        {
            sub_804C3A4(obj->headB.palSlot, (u8)sub_801B954(&obj->headB));
            obj->state &= 0xDFFF;
            gObjActStep = 9;
        }
        gObjActStepTimer++;
        break;
    case 9:
        if (gObjActStepTimer <= 0x1F)
        {
            gObjActStepTimer++;
            break;
        }
        pool = (BattleObj *)GetObjPool();
        count = sub_80489E8(pool, buf, 0, 0xFF);
        for (i = 0; i < count; i++)
        {
            *(u16 *)((u8 *)pool + (buf[i] * 0xC8) + 0xB0) |= 0x10;
            *((u8 *)pool + (buf[i] * 0xC8) + 0xBA) = 0;
        }
        obj->state |= 0x10;
        *((u8 *)obj + 0xBA) = 0;
        sub_80209EC(obj);
        result = 2;
        break;
    }
    return result;
}

// @ 0x0803B484
INCLUDE_ASM("asm/nonmatchings", sub_803B484);
// @ 0x0803BBEC
// 战斗对象"场景混合/多段头动画"长状态机 (gObjActStep 0 → 0x12..0x2A → 9, 双参 obj+arg1):
// case0  存 posX/Y, sub_80444A4 + sub_803F5B4, f_B6=0x3D2, f_B4=0, sub_80187C0(0x10)
//        + sub_801A348 + 混合 0x1F4F → 0x12。
// 18     等 sub_8019B98(4,2,14,1), ctx->state|=0x400, timer=0, Sfx 0x59, sub_804BDD8 → 0x13。
// 0x13/0x14 第一段 BLD 插值 (0x1F4F, 0x28/0x14 步); 0x16 等 sub_8019B98(0xB,3,0xF,0);
// 0x17/0x18 第二段 (0x1F47); 0x19 timer<=0x31 后 Sfx_StopTrack(0)+sub_8019AD0(5,0x110)。
// 0x1A/0x1B 两阶段 FlashFlag 0x4000; 0x1B 解除抑制 (ctx->state&=0xFBFF) + BgLoad_Reset。
// 0x1C BgLoad_GetPos>3; 0x1D gObjActParam=0; 0x1E 等 sub_803E58C(obj,arg1,3)+动画 0x3CE。
// 0x1F..0x27 headB 双头动画编排 (锚点 0x3CF/0x3D0/0x3D1, sub_804BF14/C728, headB.frameIdx 按
//        sub_801B95C 步进); 0x28 timer<=0xF; 0x29 headA 0x1000 还原调色; 0x2A 三 busy 清零 + Param=0xC。
// 9      等 sub_803E58C(obj,arg1,3) → sub_80209EC(obj), 返回 1。
// 尾     若 headA.kindFlags & 0x1000 则 |= 0x100; sub_803F658(obj)。
// 注: case34 的 headB.frameIdx = sub_801B95C(&headB) + ((frameIdx+1)%4 - 5) 写法是字节匹配必需
//     (把 -5 作为独立项, 否则 GCC 折成 subs 而非 ldr 0xFFFB 立即数)。
u32 sub_803BBEC(BattleObj *obj, u8 *arg1)
{
    u32 result;
    BattleObj *context;

    result = 0;
    context = (BattleObj *)GetCtx_0248();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = obj->posX;
        gObjActSavedY = obj->posY;
        gObjActStepTimer = 0;
        sub_80444A4(obj);
        sub_803F5B4(obj);
        obj->f_B6 = 0x3D2;
        obj->f_B4 = 0;
        gObjActStep = 0x12;
        sub_80187C0(0x10);
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
        break;
    case 18:
        if (sub_8019B98(4, 2, 0xE, 1))
        {
            context->state |= 0x400;
            gObjActStepTimer = 0;
            Sfx_Play(0x59, 0, 1);
            sub_804BDD8(0xE, 1, 1, -1, 0xF);
            gObjActStep = 0x13;
        }
        break;
    case 19:
        if (gObjActStepTimer <= 0x27)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0x28, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 0x14;
        }
        break;
    case 20:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            sub_801A348();
            gObjActStep = 0x15;
        }
        break;
    case 21:
        if (sub_8019B98(0xB, 3, 0xF, 0))
        {
            gObjActStepTimer = 0;
            gSceneFadeOut = 0;
            sub_801A2AC(0x1F47, 0, 0x10);
            gObjActStep = 0x16;
        }
        break;
    case 22:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 1);
            sub_801A2AC(0x1F47, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 0x17;
        }
        break;
    case 23:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            sub_801A2AC(0, 0, 0);
            gObjActStep = 0x18;
        }
        break;
    case 24:
        if (gObjActStepTimer <= 0x31)
            gObjActStepTimer++;
        else
        {
            Sfx_StopTrack(0);
            sub_8019AD0(5, 0x110);
            gObjActStep = 0x19;
        }
        break;
    case 25:
        if (FlashFlag_Get() & 0x4000)
        {
            FlashFlag_Reset(0x4000);
            sub_8019AD0(0xA, 0x120);
            sub_804BE90(0xE, 1);
            BattleUiFlag_Set(0xA0);
            gObjActStep = 0x1A;
        }
        break;
    case 26:
        if (FlashFlag_Get() & 0x4000)
        {
            sub_80187D4(0x10);
            FlashFlag_Reset(0x4000);
            BattleFx_DispOff();
            sub_8018BF8();
            BattleUiFlag_Set(8);
            context->state &= 0xFBFF;
            BgLoad_Reset();
            gObjActStep = 0x1B;
        }
        break;
    case 27:
        if (BgLoad_GetPos() > 3)
            gObjActStep = 0x1C;
        break;
    case 28:
        gObjActParam = 0;
        gObjActStep = 0x1D;
        break;
    case 29:
        if (sub_803E58C(obj, arg1, 3) == 1)
        {
            sub_8020974(&obj->headA, 0x3CE, 0x1B4, 0xD, 2);
            gObjActStep = 0x1E;
        }
        break;
    case 30:
        if (!(obj->headA.kindFlags & 0x800))
            gObjActStep = 0x1F;
        break;
    case 31:
        if (obj->headA.frameIdx > 7)
        {
            sub_8020CC4(obj, obj->posX, obj->posY, 0x27C, 0xE, 0x3CF, 0x2005);
            gObjActStep = 0x20;
        }
        break;
    case 32:
        if (!(obj->headB.kindFlags & 0x800))
        {
            Sfx_Play(0x9A, 0, 0);
            gObjActStep = 0x21;
        }
        break;
    case 33:
        if (obj->headB.kindFlags & 0x1000)
        {
            sub_804C3A4(obj->headB.palSlot, (u8)sub_801B954(&obj->headB));
            obj->headB.kindFlags |= 0x100;
            gObjActStep = 0x22;
        }
        break;
    case 34:
        obj->headB.frameIdx = sub_801B95C(&obj->headB) + ((obj->headB.frameIdx + 1) % 4 - 5);
        if (obj->headA.frameIdx > 0x70)
        {
            sub_8020CC4(obj, (u8)(obj->posX + 0x1E), obj->posY, 0x27C, 0xE, 0x3D0, 0x2005);
            gObjActStep = 0x23;
        }
        break;
    case 35:
        if (!(obj->headB.kindFlags & 0x800))
        {
            Sfx_Play(0x5C, 1, 0);
            gObjActStepTimer = 0;
            gObjActStep = 0x24;
        }
        break;
    case 36:
        if (gObjActStepTimer <= 3)
            gObjActStepTimer++;
        else
        {
            sub_8020CC4(obj, (u8)(obj->posX - 0x78), (u8)(obj->posY - 0xF), 0x27C, 0xE, 0x3D1, 0x2405);
            gObjActStep = 0x25;
        }
        break;
    case 37:
        if (!(obj->headB.kindFlags & 0x800))
        {
            sub_8044514(0x28);
            sub_804BF14(0, 3, 4, 7, 0x15, 0x20, 0x20, -1, 2);
            gObjActStepTimer = 0;
            gObjActStep = 0x26;
        }
        break;
    case 38:
        if (gObjActStepTimer <= 3)
            gObjActStepTimer++;
        else
        {
            sub_804C728(0, 3, 0x10);
            gObjActStepTimer = 0;
            gObjActStep = 0x27;
        }
        break;
    case 39:
        if (gObjActStepTimer <= 0xF)
            gObjActStepTimer++;
        else
        {
            obj->state &= 0xDFFF;
            gObjActStep = 0x28;
        }
        break;
    case 40:
        if (obj->headA.kindFlags & 0x1000)
        {
            sub_804C3A4(obj->headA.palSlot, (u8)sub_801B954(&obj->headA));
            gObjActStep = 0x29;
        }
        break;
    case 41:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            gObjActParam = 0xC;
            gObjActStep = 0x2A;
        }
        break;
    case 42:
        if (sub_803E58C(obj, arg1, 3) == 1)
            gObjActStep = 9;
        break;
    case 9:
        sub_80209EC(obj);
        result = 1;
        break;
    }
    if (obj->headA.kindFlags & 0x1000)
        obj->headA.kindFlags |= 0x100;
    sub_803F658(obj);
    return result;
}

// @ 0x0803C328
INCLUDE_ASM("asm/nonmatchings", sub_803C328);
// @ 0x0803CE0C
// 0x0839D4CC 动作表项 0x38 (56) 演出状态机:
// case 0: 保存动画恢复参数 (SavedPal, SavedF2A), 请求演出 sub_8048B30(3, 0x1E, 0x39C), 设 0x10 引擎标志,
//         复位混合 (0x1F4F, 0, 0x10) 并切至 18。
// case 18: 等待 sub_8019B98(4, 2, 0xE, 1), 清 StepTimer, 抑制辅助对象更新 (context->state |= 0x400),
//          播放音效 0x59, 启动调色板动画 sub_804BDD8(0xE, 1, 1, -1, 0xF) 并切至 19。
// case 19: 双向淡出淡入插值阶段 1: gSceneFadeOut 在 [0..39] 插值 (0 -> 0x10, 0x28 步), 超时切 20。
// case 20: 双向淡出淡入插值阶段 2: gSceneFadeIn 在 [0..19] 插值 (0x10 -> 0, 0x14 步), 超时复位混合并切 21。
// case 21: 等待 sub_8019B98(8, 3, 0xF, 0), 清 StepTimer 与 gSceneFadeOut, 设混合控制 0x1F47 并切 22。
// case 22: 第二段混合插值阶段 1: gSceneFadeOut 在 [0..9] 插值 (0 -> 0x10, 10 步), 超时切 23。
// case 23: 第二段混合插值阶段 2: gSceneFadeIn 在 [0..19] 插值 (0x10 -> 0, 20 步), 超时清混合寄存器并切 24。
// case 24: StepTimer 计数至 49, 超时停止音轨 0, 启动 sub_8019AD0(5, 0x110) 并切 25。
// case 25: 等待 FlashFlag 0x4000, 复位标志, 启动 sub_8019AD0(10, 0x120), 停止调色板动画 sub_804BE90(0xE, 1),
//          置 BattleUiFlag(0xA0) 并切 26。
// case 26: 等待 FlashFlag 0x4000, 恢复引擎标志 0x10, 关闭特效/窗口, 置 BattleUiFlag(8),
//          解除辅助对象抑制 (context->state &= 0xFBFF), 复位 BgLoad 并切 27。
// case 27: 等待 BgLoad_GetPos() > 3 并切 28。
// case 28: 等待 sub_8047B1C(obj) 返回 1 并切 29。
// case 29: 恢复对象动画与调色板 sub_801CBA4(obj, 0, SavedF2A, SavedPal, 0) 并切 6。
// case 6: 等待 obj->headA.kindFlags 0x800 清除并切 9。
// case 9: 置对象 state 0x4000 并调用 sub_80209EC(obj), 返回 2 (演出完成)。
// 其它分支: 返回 0 (进行中)。
u32 sub_803CE0C(BattleObj *obj)
{
    u32 ret = 0;
    BattleObj *context = (BattleObj *)GetCtx_0248();

    switch (gObjActStep)
    {
    case 0:
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        sub_8048B30(3, 0x1E, 0x39C);
        sub_80187C0(0x10);
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F4F, 0, 0x10);
        gObjActStep = 18;
        break;

    case 18:
        if (sub_8019B98(4, 2, 0xE, 1))
        {
            gObjActStepTimer = 0;
            context->state |= 0x400;
            Sfx_Play(0x59, 0, 1);
            sub_804BDD8(0xE, 1, 1, -1, 0xF);
            gObjActStep = 19;
        }
        break;

    case 19:
        if (gObjActStepTimer <= 0x27)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0x28, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 20;
        }
        break;

    case 20:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            sub_801A348();
            gObjActStep = 21;
        }
        break;

    case 21:
        if (sub_8019B98(8, 3, 0xF, 0))
        {
            gObjActStepTimer = 0;
            gSceneFadeOut = 0;
            sub_801A2AC(0x1F47, 0, 0x10);
            gObjActStep = 22;
        }
        break;

    case 22:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 10, gObjActStepTimer, 1);
            sub_801A2AC(0x1F47, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 23;
        }
        break;

    case 23:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            sub_801A2AC(0, 0, 0);
            gObjActStep = 24;
        }
        break;

    case 24:
        if (gObjActStepTimer <= 49)
        {
            gObjActStepTimer++;
        }
        else
        {
            Sfx_StopTrack(0);
            sub_8019AD0(5, 0x110);
            gObjActStep = 25;
        }
        break;

    case 25:
        if (FlashFlag_Get() & 0x4000)
        {
            FlashFlag_Reset(0x4000);
            sub_8019AD0(10, 0x120);
            sub_804BE90(0xE, 1);
            BattleUiFlag_Set(0xA0);
            gObjActStep = 26;
        }
        break;

    case 26:
        if (FlashFlag_Get() & 0x4000)
        {
            sub_80187D4(0x10);
            FlashFlag_Reset(0x4000);
            BattleFx_DispOff();
            sub_8018BF8();
            BattleUiFlag_Set(8);
            context->state &= 0xFBFF;
            BgLoad_Reset();
            gObjActStep = 27;
        }
        break;

    case 27:
        if ((u8)BgLoad_GetPos() > 3)
        {
            gObjActStep = 28;
        }
        break;

    case 28:
        if ((u8)sub_8047B1C(obj) == 1)
        {
            gObjActStep = 29;
        }
        break;

    case 29:
        sub_801CBA4(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
        gObjActStep = 6;
        break;

    case 6:
        if (!(obj->headA.kindFlags & 0x800))
        {
            gObjActStep = 9;
        }
        break;

    case 9:
        obj->state |= 0x4000;
        sub_80209EC(obj);
        ret = 2;
        break;
    }

    return ret;
}
// @ 0x0803D20C
u32 sub_803D20C(BattleObj *obj)
{
    u32 ret = 0;
    BattleObj *context = (BattleObj *)GetCtx_0248();

    switch (gObjActStep)
    {
    case 0:
        gObjActSavedPal = obj->headA.palSlot;
        gObjActSavedF2A = obj->headA.f_1E;
        sub_8048B30(3, 0x1E, 0x3B6);
        sub_80187C0(0x10);
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F4F, 0, 0x10);
        gObjActStep = 18;
        break;

    case 18:
        if (sub_8019B98(4, 2, 0xE, 1))
        {
            context->state |= 0x400;
            gObjActStepTimer = 0;
            Sfx_Play(0x59, 0, 1);
            sub_804BDD8(0xE, 1, 1, -1, 0xF);
            gObjActStep = 19;
        }
        break;

    case 19:
        if (gObjActStepTimer <= 0x27)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0x28, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 20;
        }
        break;

    case 20:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            sub_801A348();
            gObjActStep = 21;
        }
        break;

    case 21:
        if (sub_8019B98(9, 3, 0xF, 0))
        {
            gObjActStepTimer = 0;
            gSceneFadeOut = 0;
            sub_801A2AC(0x1F47, 0, 0x10);
            gObjActStep = 22;
        }
        break;

    case 22:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 10, gObjActStepTimer, 1);
            sub_801A2AC(0x1F47, gSceneFadeOut, 0x10);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            gObjActStep = 23;
        }
        break;

    case 23:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer++;
        }
        else
        {
            gObjActStepTimer = 0;
            sub_801A2AC(0, 0, 0);
            gObjActStep = 24;
        }
        break;

    case 24:
        if (gObjActStepTimer <= 49)
        {
            gObjActStepTimer++;
        }
        else
        {
            Sfx_StopTrack(0);
            sub_8019AD0(5, 0x110);
            gObjActStep = 25;
        }
        break;

    case 25:
        if (FlashFlag_Get() & 0x4000)
        {
            FlashFlag_Reset(0x4000);
            sub_8019AD0(10, 0x120);
            sub_804BE90(0xE, 1);
            BattleUiFlag_Set(0xA0);
            gObjActStep = 26;
        }
        break;

    case 26:
        if (FlashFlag_Get() & 0x4000)
        {
            sub_80187D4(0x10);
            FlashFlag_Reset(0x4000);
            BattleFx_DispOff();
            sub_8018BF8();
            BattleUiFlag_Set(8);
            context->state &= 0xFBFF;
            BgLoad_Reset();
            gObjActStep = 27;
        }
        break;

    case 27:
        if ((u8)BgLoad_GetPos() > 3)
        {
            gObjActStep = 28;
        }
        break;

    case 28:
        if ((u8)sub_8047B1C(obj) == 1)
        {
            gObjActStep = 29;
        }
        break;

    case 29:
        sub_801CBA4(obj, 0, gObjActSavedF2A, gObjActSavedPal, 0);
        gObjActStep = 6;
        break;

    case 6:
        if (!(obj->headA.kindFlags & 0x800))
        {
            gObjActStep = 9;
        }
        break;

    case 9:
        obj->state |= 0x4000;
        sub_80209EC(obj);
        ret = 2;
        break;
    }

    return ret;
}
// @ 0x0803D60C
// 0x0839D4CC 动作表项 45 (0x2C) 演出状态机 (sub_803D20C/sub_803CE0C 的姊妹变体, 双参 obj+arg1):
// case0 备份 posX/posY/palSlot/f_1E, sub_80444A4 + sub_803F5B4 复位, f_B6/f_B4 清零,
//       场景 0x1F4F 淡出准备 (sub_80187C0(0x10) + sub_801A348 + sub_801A2AC);
// case18-23 四段场景插值: 0x1F4F 淡出 (sub_8019B98(4,2,0xE,1) + ctx->state|=0x400 + Sfx 0x59 +
//       sub_804BDD8(0xE,1,1,-1,0xF)) → 0x1F4F 淡入 → 0x1F47 淡出 (sub_8019B98(0xA,3,0xF,0)) → 0x1F47 淡入;
// case24-27 音乐切换 (Sfx_StopTrack + sub_8019AD0 0x110/0x120) 与 FlashFlag 0x4000 等待,
//       sub_804BE90 + BattleUiFlag_Set(0xA0)/Set(8) + ctx->state&=0xFBFF + BgLoad_Reset + BgLoad_GetPos>3;
// case28-31 装载 0x3BB, kindFlags 0x800/0x1000 等待, sub_804C3A4 调色板释放 + kindFlags|=0x100,
//       sub_801768C 把 obj->posX/posY 向 arg1 锚点插值 (用 sub_801EC3C(arg1,0/1)>>1 求半宽);
// case44 (0x2C, 表内错位槽) 装载 0x3BE → case36;
// case32-35 kindFlags&=0xFEFF + f_B4=5 + sub_8044514(0x14) → 0x3BD 装载 → sub_804B96C 调色板闪变 → sub_804C4D8;
// case36-38 sub_8020CC4 锚点动画 0x3C5 (arg1 半高偏移) + Sfx 0x58 + kindFlags 0x1000/0x100 收尾 + state&=0xDFFF;
// case39-43 FlashFlag 0x4000 等待 + sub_801CBA4 调色板恢复 + posX/posY 还原 + f_B6=0x3C6 +
//       sub_804BF14(0,3,0x1C,0xE,7,4,4,-1,2) 窗口 + sub_804C728(0,3,0x20) 收尾;
// case9 三等待闩 gActWaitBusy0/1/2 全清 → sub_80209EC(obj) 返回 1; 尾部统一 sub_803F658(obj)。
// 字节匹配要点: case31/36 的 sub_801EC3C 结果必须先 (u8) 截断再 >>1 (得 lsls #24 + lsrs #25),
// 否则 GCC 生成单条 lsrs #1 且字面池偏移错位。
u32 sub_803D60C(BattleObj *arg0, BattleObj *arg1)
{
    u32 ret;
    BattleObj *ctx;

    ret = 0;
    ctx = (BattleObj *)GetCtx_0248();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg0->posX;
        gObjActSavedY = arg0->posY;
        gObjActSavedPal = arg0->headA.palSlot;
        gObjActSavedF2A = arg0->headA.f_1E;
        gObjActStepTimer = 0;
        sub_80444A4(arg0);
        sub_803F5B4(arg0);
        arg0->f_B6 = 0;
        arg0->f_B4 = 0;
        gObjActStep = 0x12;
        sub_80187C0(0x10);
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
        break;
    case 18:
        if (sub_8019B98(4, 2, 0xE, 1) != 0)
        {
            ctx->state |= 0x400;
            gObjActStepTimer = 0;
            Sfx_Play(0x59, 0, 1);
            sub_804BDD8(0xE, 1, 1, -1, 0xF);
            gObjActStep = 0x13;
        }
        break;
    case 19:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x14;
        break;
    case 20:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        sub_801A348();
        gObjActStep = 0x15;
        break;
    case 21:
        if (sub_8019B98(0xA, 3, 0xF, 0) != 0)
        {
            gObjActStepTimer = 0;
            gSceneFadeOut = 0;
            sub_801A2AC(0x1F47, 0, 0x10);
            gObjActStep = 0x16;
        }
        break;
    case 22:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 1);
            sub_801A2AC(0x1F47, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x17;
        break;
    case 23:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        sub_801A2AC(0, 0, 0);
        gObjActStep = 0x18;
        break;
    case 24:
        if (gObjActStepTimer <= 0x31)
        {
            gObjActStepTimer += 1;
            break;
        }
        Sfx_StopTrack(0);
        sub_8019AD0(0x3C, 0x110);
        gObjActStep = 0x19;
        break;
    case 25:
        if (!(FlashFlag_Get() & 0x4000))
            break;
        FlashFlag_Reset(0x4000);
        sub_8019AD0(0x41, 0x120);
        sub_804BE90(0xE, 1);
        BattleUiFlag_Set(0xA0);
        gObjActStep = 0x1A;
        break;
    case 26:
        if (!(FlashFlag_Get() & 0x4000))
            break;
        sub_80187D4(0x10);
        FlashFlag_Reset(0x4000);
        BattleFx_DispOff();
        sub_8018BF8();
        BattleUiFlag_Set(8);
        ctx->state &= 0xFBFF;
        BgLoad_Reset();
        gObjActStep = 0x1B;
        break;
    case 27:
        if ((u8)BgLoad_GetPos() > 3)
            gObjActStep = 0x1C;
        break;
    case 28:
        sub_8020974((ObjHead *)(&arg0->headA), 0x3BB, 0x1B4, 0xD, 2);
        gObjActStep = 0x1D;
        break;
    case 29:
        if (arg0->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x1E;
        break;
    case 30:
        if (!(arg0->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
        arg0->headA.kindFlags |= 0x100;
        gObjActStepTimer = 0;
        gObjActStep = 0x1F;
        break;
    case 31:
        if (gObjActStepTimer <= 9)
        {
            arg0->posX = sub_801768C(gObjActSavedX,
                                     arg1->posX + ((u8)sub_801EC3C(arg1, 0) >> 1) - gObjActSavedX,
                                     0xA, gObjActStepTimer, 2);
            arg0->posY = sub_801768C(gObjActSavedY,
                                     arg1->posY - gObjActSavedY + 1,
                                     0xA, gObjActStepTimer, 2);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        sub_8020974((ObjHead *)(&arg0->headA), 0x3BC, 0x1B4, 0xD, 0x102);
        gObjActStep = 0x20;
        break;
    case 32:
        if (arg0->headA.kindFlags & 0x800)
            break;
        arg0->headA.kindFlags &= 0xFEFF;
        arg0->f_B6 = 0;
        arg0->f_B4 = 5;
        sub_8044514(0x14);
        gObjActStep = 0x21;
        break;
    case 33:
        if (!(arg0->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
        sub_8020974((ObjHead *)(&arg0->headA), 0x3BD, 0x1B4, 0xD, 0x102);
        gObjActStep = 0x22;
        break;
    case 34:
        if (arg0->headA.kindFlags & 0x800)
            break;
        sub_804B96C(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)),
                    0x1C, 0xE, 7, 0x20, 0x20, -1, 2);
        gObjActStepTimer = 0;
        gObjActStep = 0x23;
        break;
    case 35:
        if (gObjActStepTimer <= 0x1F)
        {
            gObjActStepTimer += 1;
            break;
        }
        sub_804C4D8(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)), 4);
        gObjActStep = 0x2C;
        break;
    case 44:
        if (gObjActStepTimer <= 3)
        {
            gObjActStepTimer += 1;
            break;
        }
        sub_8020974((ObjHead *)(&arg0->headA), 0x3BE, 0x1B4, 0xD, 0x102);
        gObjActStep = 0x24;
        break;
    case 36:
        if (arg0->headA.kindFlags & 0x800)
            break;
        sub_8020CC4(arg1, arg1->posX,
                    (u8)(arg1->posY - ((u8)sub_801EC3C(arg1, 0) >> 1)),
                    0x27C, 0xE, 0x3C5, 4);
        gObjActStep = 0x25;
        break;
    case 37:
        if (arg1->headB.kindFlags & 0x800)
            break;
        Sfx_Play(0x58, 0, 0);
        arg0->headA.kindFlags &= 0xFEFF;
        gObjActStep = 0x26;
        break;
    case 38:
        if (arg0->headA.kindFlags & 0x1000)
        {
            sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
            arg0->headA.kindFlags |= 0x100;
        }
        if (!(arg1->headB.kindFlags & 0x1000))
            break;
        arg1->state &= 0xDFFF;
        sub_8019AD0(5, 0x110);
        gObjActStep = 0x27;
        break;
    case 39:
        if (!(FlashFlag_Get() & 0x4000))
            break;
        sub_801CBA4(arg0, 0, gObjActSavedF2A, gObjActSavedPal, 0);
        arg0->posX = gObjActSavedX;
        arg0->posY = gObjActSavedY;
        gObjActStep = 0x28;
        break;
    case 40:
        if (arg0->headA.kindFlags & 0x800)
            break;
        FlashFlag_Reset(0x4000);
        sub_8019AD0(5, 0x120);
        gObjActStep = 0x29;
        break;
    case 41:
        if (!(FlashFlag_Get() & 0x4000))
            break;
        FlashFlag_Reset(0x4000);
        BattleFx_DispOff();
        arg0->f_B6 = 0x3C6;
        arg0->f_B4 = 0;
        sub_8044514(0x32);
        sub_804BF14(0, 3, 0x1C, 0xE, 7, 4, 4, -1, 2);
        gObjActStepTimer = 0;
        gObjActStep = 0x2A;
        break;
    case 42:
        if (gObjActStepTimer <= 3)
        {
            gObjActStepTimer += 1;
            break;
        }
        sub_804C728(0, 3, 0x20);
        gObjActStepTimer = 0;
        gObjActStep = 0x2B;
        break;
    case 43:
        if (gObjActStepTimer <= 0x1F)
        {
            gObjActStepTimer += 1;
            break;
        }
        gObjActStep = 9;
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            sub_80209EC(arg0);
            ret = 1;
        }
        break;
    }
    sub_803F658(arg0);
    return ret;
}
// @ 0x0803DECC
// 0x0839D4CC 动作表项 48 (0x30) 演出状态机 (sub_803D60C 的姊妹变体, 双参 obj+arg1):
// case0 备份 posX/posY + sub_80444A4/sub_803F5B4 复位, f_B6=1 / f_B4=0,
//       场景 0x1F4F 淡出准备 (sub_80187C0(0x10) + sub_801A348 + sub_801A2AC);
// case18-23 四段插值 (与 sub_803D60C 同构): 0x1F4F 淡出 (sub_8019B98(4,2,0xE,1) + ctx->state|=0x400 +
//       Sfx 0x59 + sub_804BDD8(0xE,1,1,-1,0xF)) → 0x1F4F 淡入 → 0x1F47 淡出 (sub_8019B98(0xB,3,0xF,0)) → 0x1F47 淡入;
// case24-27 音乐切换 (Sfx_StopTrack + sub_8019AD0(5,0x110)/(0xA,0x120)) 与 FlashFlag 0x4000 等待,
//       sub_804BE90 + BattleUiFlag_Set(0xA0)/Set(8) + ctx->state&=0xFBFF + BgLoad_Reset + BgLoad_GetPos>3 → gObjActParam=0;
// case28 sub_803E58C(obj,arg1,4)==1 → 装载 0x3D3, arg1->slot<=0xA 时 obj->headA.kindFlags|=0x20;
// case29-30 kindFlags 0x800 等待 → sub_8044514(0xBE) → 帧 0x32/0x50/0x68/0xA0 各播 Sfx 0x37,
//       kindFlags 0x1000 后 sub_804C3A4 释放调色板 + 装载 0x3D4 (0xF5<<2);
// case31-34 kindFlags&=0xFEFF + Sfx 0x1D + 记录 gObjActSavedArgY=posY-2,
//       两段 sub_801768C 位移: posY→pad_C1 半程 + posX 0→0xF0 (40 帧), 再回落 (10 帧),
//       末了 gObjActParam=0xC 并记录 gObjActMoveFromX/Y=posX/posY + sub_8044514(0x14);
// case35 三等待闩 gActWaitBusy0/1/2 全清 → case36; case36 sub_803E58C(obj,arg1,4)==1 → 9;
// case9 三等待闩全清 → sub_80209EC(obj) 返回 1; 尾部统一 sub_803F658(obj)。
// 字节匹配要点: (1) case37-47 虽为空体也必须列出, 否则 switch 上界变成 36、跳转表只有 37 项;
// (2) case30 的四个帧号须用 switch 而非 || 链, 否则比较序列形状不同。
u32 sub_803DECC(BattleObj *arg0, u8 *arg1)
{
    u32 ret;
    BattleObj *ctx;

    ret = 0;
    ctx = (BattleObj *)GetCtx_0248();
    switch (gObjActStep)
    {
    case 0:
        gObjActSavedX = arg0->posX;
        gObjActSavedY = arg0->posY;
        gObjActStepTimer = 0;
        sub_80444A4(arg0);
        sub_803F5B4(arg0);
        arg0->f_B6 = 1;
        arg0->f_B4 = 0;
        gObjActStep = 0x12;
        sub_80187C0(0x10);
        sub_801A348();
        gSceneFadeOut = 0;
        sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
        break;
    case 18:
        if (sub_8019B98(4, 2, 0xE, 1) != 0)
        {
            ctx->state |= 0x400;
            gObjActStepTimer = 0;
            Sfx_Play(0x59, 0, 1);
            sub_804BDD8(0xE, 1, 1, -1, 0xF);
            gObjActStep = 0x13;
        }
        break;
    case 19:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x14;
        break;
    case 20:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0xA, gObjActStepTimer, 2);
            sub_801A2AC(0x1F4F, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        sub_801A348();
        gObjActStep = 0x15;
        break;
    case 21:
        if (sub_8019B98(0xB, 3, 0xF, 0) != 0)
        {
            gObjActStepTimer = 0;
            gSceneFadeOut = 0;
            sub_801A2AC(0x1F47, 0, 0x10);
            gObjActStep = 0x16;
        }
        break;
    case 22:
        if (gObjActStepTimer <= 9)
        {
            gSceneFadeOut = sub_801768C(0, 0x10, 0xA, gObjActStepTimer, 1);
            sub_801A2AC(0x1F47, gSceneFadeOut, 0x10);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        gObjActStep = 0x17;
        break;
    case 23:
        if (gObjActStepTimer <= 0x13)
        {
            gSceneFadeIn = sub_801768C(0x10, -0x10, 0x14, gObjActStepTimer, 2);
            sub_801A2AC(0x1F47, gSceneFadeOut, gSceneFadeIn);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        sub_801A2AC(0, 0, 0);
        gObjActStep = 0x18;
        break;
    case 24:
        if (gObjActStepTimer <= 0x31)
        {
            gObjActStepTimer += 1;
            break;
        }
        Sfx_StopTrack(0);
        sub_8019AD0(5, 0x110);
        gObjActStep = 0x19;
        break;
    case 25:
        if (!(FlashFlag_Get() & 0x4000))
            break;
        FlashFlag_Reset(0x4000);
        sub_8019AD0(0xA, 0x120);
        sub_804BE90(0xE, 1);
        BattleUiFlag_Set(0xA0);
        gObjActStep = 0x1A;
        break;
    case 26:
        if (!(FlashFlag_Get() & 0x4000))
            break;
        sub_80187D4(0x10);
        FlashFlag_Reset(0x4000);
        BattleFx_DispOff();
        sub_8018BF8();
        BattleUiFlag_Set(8);
        ctx->state &= 0xFBFF;
        BgLoad_Reset();
        gObjActStep = 0x1B;
        break;
    case 27:
        if ((u8)BgLoad_GetPos() > 3)
        {
            gObjActParam = 0;
            gObjActStep = 0x1C;
        }
        break;
    case 28:
        if ((u8)sub_803E58C(arg0, arg1, 4) == 1)
        {
            sub_8020974((ObjHead *)(&arg0->headA), 0x3D3, 0x1B4, 0xD, 2);
            if (arg1[0xBE] <= 0xA)
                arg0->headA.kindFlags |= 0x20;
            gObjActStep = 0x1D;
        }
        break;
    case 29:
        if (arg0->headA.kindFlags & 0x800)
            break;
        sub_8044514(0xBE);
        gObjActStep = 0x1E;
        break;
    case 30:
        switch (arg0->headA.frameIdx)
        {
        case 0x32:
        case 0x50:
        case 0x68:
        case 0xA0:
            Sfx_Play(0x37, 0, 0);
            break;
        }
        if (!(arg0->headA.kindFlags & 0x1000))
            break;
        sub_804C3A4(arg0->headA.palSlot, (u8)sub_801B954((ObjHead *)(&arg0->headA)));
        sub_8020974((ObjHead *)(&arg0->headA), 0x3D4, 0x1B4, 0xD, 0x102);
        gObjActStep = 0x1F;
        break;
    case 31:
        if (arg0->headA.kindFlags & 0x800)
            break;
        Sfx_Play(0x1D, 2, 0);
        arg0->headA.kindFlags &= 0xFEFF;
        gObjActStepTimer = 0;
        gObjActStep = 0x20;
        gObjActSavedArgY = arg0->posY - 2;
        break;
    case 32:
        if (gObjActStepTimer <= 0x27)
        {
            arg0->pad_C1 = sub_801768C(0, arg0->posY >> 1, 0x28, gObjActStepTimer, 2);
            arg0->posX = sub_801768C(gObjActMoveFromX, 0xF0 - gObjActMoveFromX, 0x28, gObjActStepTimer, 2);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        sub_8020974((ObjHead *)(&arg0->headA), 0x3D5, 0x1B4, 0xD, 2);
        gObjActStep = 0x21;
        break;
    case 33:
        if (arg0->headA.kindFlags & 0x800)
            break;
        gObjActStep = 0x22;
        break;
    case 34:
        if (gObjActStepTimer <= 9)
        {
            arg0->pad_C1 = sub_801768C(arg0->posY >> 1, -(arg0->posY >> 1), 0xA, gObjActStepTimer, 2);
            arg0->posX = sub_801768C(0xF0, gObjActMoveFromX - 0xF0, 0xA, gObjActStepTimer, 2);
            gObjActStepTimer += 1;
            break;
        }
        gObjActStepTimer = 0;
        arg0->pad_C1 = 0;
        gObjActParam = 0xC;
        gObjActMoveFromX = arg0->posX;
        gObjActMoveFromY = arg0->posY;
        sub_8044514(0x14);
        gObjActStep = 0x23;
        break;
    case 35:
        if (gActWaitBusy0 != 0 || gActWaitBusy1 != 0 || gActWaitBusy2 != 0)
            break;
        gObjActStep = 0x24;
        break;
    case 36:
        if ((u8)sub_803E58C(arg0, arg1, 4) == 1)
            gObjActStep = 9;
        break;
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
        break;
    case 9:
        if (gActWaitBusy0 == 0 && gActWaitBusy1 == 0 && gActWaitBusy2 == 0)
        {
            sub_80209EC(arg0);
            ret = 1;
        }
        break;
    }
    sub_803F658(arg0);
    return ret;
}
// @ 0x0803E58C
INCLUDE_ASM("asm/nonmatchings", sub_803E58C);
// @ 0x0803ED34
INCLUDE_ASM("asm/nonmatchings", sub_803ED34);
// @ 0x0803F21C
INCLUDE_ASM("asm/nonmatchings", sub_803F21C);
// @ 0x0803F328
u8 sub_803F328(u8 arg0)
{
    u8 result;
    u8 v;
    int base;

    result = 0;
    switch (gObjActBranch)
    {
        case 0:
            result = 1;
            break;
        case 1:
            DialogCtx_Clear3();
            Bg0_InitClear();
            base = 0x02035AC0;
            v = 2;
            sub_80196D4(0, (u8 *)base, 0xB, 2, 2, 1, 2, 0xC, 4);
            gObjActStepTimer = 0;
            gObjActBranch = v;
            break;
        case 2:
            if (DialogCtx_GetField_C(0) == 4)
            {
                sub_803F21C((u8 *)0x02035AC0, arg0);
                sub_80187C0(0x400);
                gObjActBranch = 3;
            }
            break;
        case 3:
            if (!(sub_80187B4() & 0x400))
                gObjActBranch = 4;
            break;
        case 4:
            if (gObjActStepTimer <= 0x27)
                gObjActStepTimer += 1;
            else
            {
                DialogCtx_SetHead(0, 2, 2);
                gObjActBranch = 5;
            }
            break;
        case 5:
            v = DialogCtx_GetField_C(0);
            if (v == 0)
            {
                Disp_Bg1Off();
                gObjActBranch = v;
            }
            break;
    }
    sub_801933C();
    return result;
}
// @ 0x0803F444
INCLUDE_ASM("asm/nonmatchings", sub_803F444);
// @ 0x0803F5B4
void sub_803F5B4(BattleObj *obj)
{
    u32 pool;
    u8 i;
    u8 *buf;
    u32 result;

    pool = GetObjPool();
    sub_8020DF0(obj);
    gObjActGroupCount = sub_8020E5C();
    buf = (u8 *)sub_8020E54();
    gObjActGroupSlots = buf;
    gActEventCount = 0;
    for (i = 0; i < gObjActGroupCount; i++)
    {
        if ((gObjActGroupSlots[i] & 0xF0) == 0x10)
        {
            result = sub_804473C((BattleObj *)(obj), pool + (gObjActGroupSlots[i] & 0xF) * 0xC8);
            result += *(u16 *)(pool + (gObjActGroupSlots[i] & 0xF) * 0xC8 + 0xB2);
            *(u16 *)(pool + (gObjActGroupSlots[i] & 0xF) * 0xC8 + 0xB2) = result;
            gActEventCount++;
        }
    }
}
/*
void sub_803F5B4(u8 *arg0)
{
    u32 pool;
    u8 i;
    s32 ret;

    pool = GetObjPool();
    sub_8020DF0(arg0);
    gObjActGroupCount = sub_8020E5C();
    gObjActGroupSlots = sub_8020E54();
    gActEventCount = 0;
    for (i = 0; i < gObjActGroupCount; i++) {
        if ((((u8 *)gObjActGroupSlots)[i] & 0xF0) == 0x10) {
            ret = sub_804473C(arg0, pool + (((u8 *)gObjActGroupSlots)[i] & 0xF) * 0xC8);
            ret += *(u16 *)(pool + (((u8 *)gObjActGroupSlots)[i] & 0xF) * 0xC8 + 0xB2);
            *(u16 *)(pool + (((u8 *)gObjActGroupSlots)[i] & 0xF) * 0xC8 + 0xB2) = ret;
            gActEventCount++;
        }
    }
}
*/
// @ 0x0803F658
INCLUDE_ASM("asm/nonmatchings", sub_803F658);
