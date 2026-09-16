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
INCLUDE_ASM("asm/nonmatchings", sub_8032EA0);
// @ 0x080334B8
INCLUDE_ASM("asm/nonmatchings", sub_80334B8);
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
INCLUDE_ASM("asm/nonmatchings", sub_8035360);
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
INCLUDE_ASM("asm/nonmatchings", sub_8038920);
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
INCLUDE_ASM("asm/nonmatchings", sub_8039C38);
// @ 0x0803A478
INCLUDE_ASM("asm/nonmatchings", sub_803A478);
// @ 0x0803A8D0
INCLUDE_ASM("asm/nonmatchings", sub_803A8D0);
// @ 0x0803AF60
INCLUDE_ASM("asm/nonmatchings", sub_803AF60);
// @ 0x0803B484
INCLUDE_ASM("asm/nonmatchings", sub_803B484);
// @ 0x0803BBEC
INCLUDE_ASM("asm/nonmatchings", sub_803BBEC);
// @ 0x0803C328
INCLUDE_ASM("asm/nonmatchings", sub_803C328);
// @ 0x0803CE0C
INCLUDE_ASM("asm/nonmatchings", sub_803CE0C);
// @ 0x0803D20C
INCLUDE_ASM("asm/nonmatchings", sub_803D20C);
// @ 0x0803D60C
INCLUDE_ASM("asm/nonmatchings", sub_803D60C);
// @ 0x0803DECC
INCLUDE_ASM("asm/nonmatchings", sub_803DECC);
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
