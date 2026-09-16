#include "battle_types.h"
#include "battle_task_services.h"
#include "battle_flow_rules.h"
#include "battle_menu_windows.h"
#include "battle_object_engine.h"
#include "battle_palette_wipe.h"
#include "engine_core.h"
#include "scene_mgr.h"
#include "script_vm.h"
#include "sound.h"
#include "text_engine.h"
#include "data_805769C.h"
#include "gba/defines.h"
#include "gba/gba.h"
#include "gba/macro.h"
#include "globals.h"
#include "include_asm.h"
#include "iwram.h"
#include "m4a.h"
#include "save.h"

// @ 0x080170BC
void Sio_SetReady(void)
{
    if (gSioState[0] != 0)
    {
        gSioState[6] = 1;
    }
}
// @ 0x080170D0
void Sio_Shutdown(void)
{
    REG_IME = 0;
    REG_IE &= 0xFF3F;
    REG_IME = 1;

    REG_SIOCNT = 0x2003;
    REG_TM3CNT = 0xBFC0;
    REG_IF = 0xC0;
    gSioState[6] = 0;
}
// @ 0x08017120
u32 sub_8017120(int arg0)
{
    u32 status;

    if ((gSioSession.field_48 & 0x180) == 0x100)
        IntrWait(1, 0x80);
    gSioSession.field_48 = sub_8016D24((u8 *)&gSioSession + 0x18);
    if (arg0 != 0)
        Sio_SetReady();
    if (gSioSession.field_4C == 0)
    {
        if (gSioSession.field_48 & 0x100)
        {
            gSioSession.unk0 = 0x4E4C;
            gSioSession.field_4C = 1;
        }
    }
    else
    {
        status = gSioSession.field_48;
        if (status & 0x1000)
        {
            *(u16 *)((u8 *)&gSioSession + 0x18 + gSioSession.field_4D * 24) = 0;
            return 1;
        }
        if (status & 0x2000)
        {
            *(u16 *)((u8 *)&gSioSession + 0x18 + gSioSession.field_4D * 24) = status & 0x1000;
            return 2;
        }
        if (status & 0x8000)
        {
            if (((status << 28) >> 28) != ((status << 20) >> 28))
                return 3;
        }
    }
    Sio_BuildPacket((u8 *)&gSioSession);
    return 0;
}
// @ 0x080171E4
INCLUDE_ASM("asm/nonmatchings", sub_80171E4);
// @ 0x08017588
u32 Sio_IsHost(void)
{
    u32 ret;

    ret = 0;
    if (gSioState[1] == 2)
    {
        if (*(u16 *)((u8 *)&gSioSession + 0x18 + gSioSession.field_4D * 24) == 0x4E4C)
        {
            ret = 1;
        }
    }
    return ret;
}
// @ 0x080175C0
void sub_80175C0(void)
{
    s32 i;
    s32 zero;
    Unk_03004F20_entry *p;

    sub_8016C88();
    CpuFill32(0, &gSioSession, 0x60);
    zero = 0;
    p = &gSioSession.unk18[zero];
    i = 1;
    do
    {
        p->field_0 = zero;
        p->field_2 = zero;
        p++;
        i--;
    } while (i >= 0);
    sub_8017120(1);
}
// @ 0x08017600
void Sio_SetXferCtx(u32 *arg0, u32 *arg1, u32 arg2, u32 arg3)
{
    gSioXferCtx.field_4 = arg0;
    gSioXferCtx.field_0 = arg1;
    gSioXferCtx.field_8 = arg2 >> 4;
    gSioXferCtx.field_A = 0;
    gSioXferCtx.field_C = arg3;
}
// @ 0x0801761C
void Sio_ClearSlot(void)
{
    u8 index;

    index = gSioSession.field_4D;
    *(u16 *)((u8 *)&gSioSession + 0x18 + index * 24) = 0;
    Sio_Shutdown();
}
// @ 0x08017640
void sub_8017640(void *dst, void *src, s32 count)
{
    u8 *d;
    u8 *s;
    if (((u32)dst | (u32)src) & 3)
    {
        d = dst;
        s = src;
        count = count * 4;
        count--;
        while (count != -1)
        {
            *d++ = *s++;
            count--;
        }
    }
    else
    {
        count = count - 1;
        while (count != -1)
        {
            *(u32 *)dst = *(u32 *)src;
            dst = (u8 *)dst + 4;
            src = (u8 *)src + 4;
            count--;
        }
    }
}
// @ 0x0801768C
s16 sub_801768C(s16 arg0, s16 arg1, s16 arg2, s16 arg3, u8 mode)
{
  float new_var;
  s16 result;
  switch ((s8)mode)
  {
    case 0:
      result = arg1;
      break;

    case 1:
      new_var = 2.0f - (((float) arg3) / ((float) arg2));
      result = ((float) arg1) * (((float) arg3) / ((float) arg2));
      break;

    case 2:
      new_var = 2.0f - (((float) arg3) / ((float) arg2));
      result = ((float) arg1) * new_var;
      break;

    case 3:
      result = (double) (((float) arg1) * (((((-10.0f) * ((float) arg3)) / ((float) arg2)) + 20.0f) / 10.0f));
      break;

  }

  return arg0 + ((result * arg3) / arg2);
}

// @ 0x080177AC
INCLUDE_ASM("asm/nonmatchings", BattleTask_Run);
// @ 0x08017FA4
/* 战斗场景进入/重置 (BattleTransition_Enter 在切战斗前调, 参 = gUnk_030025B8/gEncounterEnabled;
 * BattleTask_Run case0 在 gGstate324 bit0 置位时以 (s8)gGstate32E 重入)。
 * 1) 非 bit0 时清 bit5(0x20)/bit9(0x200); 2) 按 s8 参数高位 (>0x39 / >0x1b) 重新置这两位的
 *    "大战/中战"分级; 3) 存 gGstate32E; 4) FlashFlag_Clear + 重置行动链 (0x03000318) /
 *    gUnk_03000240 / gGstate314 + 三个战斗子系统初始化; 5) 状态字 = (旧 & 0x261) | 8
 *    (只留 bit0/5/6/9, 强制置 bit3 = 冻结对象处理), 复位图块 DMA/战斗 UI;
 * 6) 末置 bit6 则清。 gGstate324 = 打包 u16 标志字 (非位域): 全访问点均 ldrh/strh 整字 +
 *    立即数掩码, 并有运行期掩码 API sub_80187C0(置)/sub_80187D4(清)。 */
#define BATTLE_FLAG_REENTER      0x0001
#define BATTLE_FLAG_FREEZE_OBJ   0x0008
#define BATTLE_FLAG_GRADE_HI     0x0020
#define BATTLE_FLAG_SCRIPT       0x0040
#define BATTLE_FLAG_GRADE_LO     0x0200
#define BATTLE_RESET_KEEP_MASK   0x0261

void sub_8017FA4(s8 encounterLevel)
{
    u16 mask;
    u8 level = encounterLevel;
    u16 state;

    if (!(gGstate324 & BATTLE_FLAG_REENTER))
    {
        if (gGstate324 & (int)BATTLE_FLAG_GRADE_HI)
            gGstate324 &= (u16)~BATTLE_FLAG_GRADE_HI;
        if (gGstate324 & BATTLE_FLAG_GRADE_LO)
            gGstate324 &= (u16)~BATTLE_FLAG_GRADE_LO;
    }
    mask = level;
    if ((s8)level > 0x39)
        gGstate324 |= BATTLE_FLAG_GRADE_HI;
    if ((s8)mask > 0x1B)
        gGstate324 |= BATTLE_FLAG_GRADE_LO;
    gGstate32E = level;
    FlashFlag_Clear();
    ListNode_Init(&gUnk_03000318);
    gUnk_03000240 = 0;
    gGstate314 = 0;
    sub_804ADE0();
    sub_804B1EC();
    sub_804B288();
    state = gGstate324;
    mask = BATTLE_RESET_KEEP_MASK;
    mask = state & mask;
    gGstate324 = mask | BATTLE_FLAG_FREEZE_OBJ;
    TileDma_Reset();
    BattleUiFlag_Clear();
    if (gGstate324 & BATTLE_FLAG_SCRIPT)
        gGstate324 &= (u16)~BATTLE_FLAG_SCRIPT;
}
/* 战斗场景 VBlank 流水线 (VBlankIntr 的 gVBlankPipelineMode==2 分支)。
 * 顺序: BG 滚动/战斗 FX/全局状态刷新 → 未冻结(bit4)时把 0x020352C0 的图块缓冲 DMA 到
 * 0x06006800 → 对话上下文 → 对象链 (0x03000318) 逐节点分发:
 *   · kind (kindFlags&0xF) 为 6/7 的对象先刷 0x020362C0 → 0x06007800;
 *   · 恒调 sub_801B8AC(headA, headA.f_2D); 若 state&0x2000 (跳跃) 且 headB 未禁用 DMA(0x800)
 *     再调 sub_801B8AC(headB, headB.f_2D)。
 * 之后: 结果回填 0x03000344 → OAM 刷屏 → 待传图块 (bit10) / 图块装载 (bit11) 收尾 →
 * 常量图块回刷 0x060125C0 → 战斗 UI 显示位生效。 */
// @ 0x08018070
void sub_8018070(void)
{
    UnkNode *node;
    BattleObj *obj;
    u8 ret;

    BgScrolls_WriteAll();
    sub_801889C();
    sub_804C184();

    if (!(gGstate324 & 0x10))
    {
        DmaCopy32(3, gUnk_020352C0, (void *)0x06006800, 0x800);
        DmaWait(3);
    }

    sub_804B224(&gGstate324);
    DialogCtx_Flush();

    gUnk_03000344 = 0x7F;
    ret = sub_8022458(0x7F);
    ret = sub_8049D58(ret);
    sub_801B7B8();

    node = gUnk_03000318.next;
    while (node->key <= 0xFE && !(gGstate324 & 8))
    {
        obj = (BattleObj *)node;
        if ((obj->headA.kindFlags & 0xF) == 6 || (obj->headB.kindFlags & 0xF) == 6
            || (obj->headA.kindFlags & 0xF) == 7 || (obj->headB.kindFlags & 0xF) == 7)
        {
            DmaCopy32(3, (void *)0x020362C0, (void *)0x06007800, 0x800);
            DmaWait(3);
        }
        ret = sub_801B8AC(&obj->headA, obj->headA.f_2D);
        if (obj->state & 0x2000)
        {
            if (!(obj->headB.kindFlags & 0x800))
                ret = sub_801B8AC(&obj->headB, obj->headB.f_2D);
        }
        node = node->next;
    }

    if (!(gGstate324 & 0x10))
        ret = sub_801D214(gObjPoolPtr, ret);

    sub_801B688(ret);
    sub_801B920();
    gUnk_03000344 = ret;

    DmaCopy32(3, gOamBuffer, OAM, 0x400);
    DmaWait(3);

    if (gGstate324 & 0x400)
    {
        if (sub_80527AC() < 0)
            gGstate324 &= 0xFBFF;
    }
    if (gGstate324 & 0x800)
    {
        BgTiles_LoadSet(0);
        gGstate324 &= 0xF7FF;
    }

    DmaCopy32(3, (void *)0x0861A7E4, (void *)0x060125C0, 0x2C0);
    DmaWait(3);

    sub_8018928();
}
// @ 0x080182A8
/* 按键连发状态刷新 (sub_80188BC / ScriptPump_Run 每帧尾调, 键参 r0, r1 未用):
 *   1) D-pad(0xF0) 按住时 gUnk_03000317 作 0..15 帧计数 ((n+1)%16), 松开归零;
 *   2) 清空 gGstate314 高字节 (每帧的"按下/连发"标志位);
 *   3) 逐键处理: D-pad 四键在"新按"或"连续按住 >14 帧"时置 held+pressed 双位并清计数
 *      (产生连发), 否则保持; A/B/L/R 只在"新按"时置双位并装 gKeyIgnoreTimer=10 帧
 *      (输入屏蔽, 供 sub_80188BC 跳过按键读取), 不连发。
 * 位含义用 iwram.h 的 KEYREPEAT_* 宏命名 (KeyRepeatState 位域联合的同源语义视图)。
 * 形状要点: 三组状态是独立绝对地址全局 (GCC2 逐次 ldr =0x03000314/316/317);
 * 计数用 `(n+1) % 16` (agbcc 展开成 asrs/lsls/subs, 目标正是该形态); 首块用 r2 的
 * 字节值参与 orr/and, 后续块由编译器各自 ldrh 重读。掩码清位须 `(u16)~宏` 折叠成
 * 池常量 0xFFxx, 直接用 `& 0xFFFE` 亦可 (字节等价); 结构体成员访问会改寻址, 禁用。 */
void sub_80182A8(u16 keys, u16 *arg1)
{
    (void)arg1;

    if (keys & 0xF0)
        gUnk_03000317 = (gUnk_03000317 + 1) % 16;
    else
        gUnk_03000317 = 0;

    gGstate314 = (u8)gGstate314;

    if (keys & 0x40)
    {
        if (!(gGstate314 & KEYREPEAT_UP_HELD) || gUnk_03000317 > 14)
        {
            gGstate314 |= KEYREPEAT_UP_HELD | KEYREPEAT_UP_PRESSED;
            gUnk_03000317 = 0;
        }
    }
    else
        gGstate314 &= (u16)~KEYREPEAT_UP_HELD;

    if (keys & 0x80)
    {
        if (!(gGstate314 & KEYREPEAT_DOWN_HELD) || gUnk_03000317 > 14)
        {
            gGstate314 |= KEYREPEAT_DOWN_HELD | KEYREPEAT_DOWN_PRESSED;
            gUnk_03000317 = 0;
        }
    }
    else
        gGstate314 &= (u16)~KEYREPEAT_DOWN_HELD;

    if (keys & 0x20)
    {
        if (!(gGstate314 & KEYREPEAT_LEFT_HELD) || gUnk_03000317 > 14)
        {
            gGstate314 |= KEYREPEAT_LEFT_HELD | KEYREPEAT_LEFT_PRESSED;
            gUnk_03000317 = 0;
        }
    }
    else
        gGstate314 &= (u16)~KEYREPEAT_LEFT_HELD;

    if (keys & 0x10)
    {
        if (!(gGstate314 & KEYREPEAT_RIGHT_HELD) || gUnk_03000317 > 14)
        {
            gGstate314 |= KEYREPEAT_RIGHT_HELD | KEYREPEAT_RIGHT_PRESSED;
            gUnk_03000317 = 0;
        }
    }
    else
        gGstate314 &= (u16)~KEYREPEAT_RIGHT_HELD;

    if (keys & 0x01)
    {
        if (!(gGstate314 & KEYREPEAT_A_HELD))
        {
            gGstate314 |= KEYREPEAT_A_HELD | KEYREPEAT_A_PRESSED;
            gKeyIgnoreTimer = 0xA;
        }
    }
    else
        gGstate314 &= (u16)~KEYREPEAT_A_HELD;

    if (keys & 0x02)
    {
        if (!(gGstate314 & KEYREPEAT_B_HELD))
        {
            gGstate314 |= KEYREPEAT_B_HELD | KEYREPEAT_B_PRESSED;
            gKeyIgnoreTimer = 0xA;
        }
    }
    else
        gGstate314 &= (u16)~KEYREPEAT_B_HELD;

    if (keys & 0x200)
    {
        if (!(gGstate314 & KEYREPEAT_L_HELD))
        {
            gKeyIgnoreTimer = 0xA;
            gGstate314 |= KEYREPEAT_L_HELD | KEYREPEAT_L_PRESSED;
        }
    }
    else
        gGstate314 &= (u16)~KEYREPEAT_L_HELD;

    if (keys & 0x100)
    {
        if (!(gGstate314 & KEYREPEAT_R_HELD))
        {
            gKeyIgnoreTimer = 0xA;
            gGstate314 |= KEYREPEAT_R_HELD | KEYREPEAT_R_PRESSED;
        }
    }
    else
        gGstate314 &= (u16)~KEYREPEAT_R_HELD;
}
// @ 0x080184A8
/* 对象链两趟扫描 (BattleTask_Run 尾部调用: sub_80184A8(行动链表头.next, state)):
 *   趟1 图形装载: 逐节点 (key<=0xFE), 当 headA.kindFlags&0x800 且本帧还没装过 → 
 *      ObjGfxLoad_Step(&headA); 当 state&0x2000(跳跃) 且 headB.kindFlags&0x800 且
 *      headA 无需装载 且没装过 → ObjGfxLoad_Step(&headB)。
 *   趟2 动作/OAM 刷新: kindFlags&0xF==3 时直接 f_2D=state; 否则先按 state&0x40 决定是否
 *      回填 headA.f_2B/f_2C (posX / posY-pad_C1), state=sub_804B080(obj,state,gGstate324);
 *      state&4 时 f_2D=state, 否则 f_2D=state=sub_801D378(obj,state); kind 6/6 装 0x020362C0;
 *      slot∈[12,112] 且 !(kindFlags&0x8000) 且 variantClass==4 → 置 0x8000、f_2F=sub_8020798();
 *      state=sub_801B878(&headA,state,&out), f_2E=f_2D-state; 若置过 0x8000 则清回;
 *      state&0x2000 且 headB 未禁 → headB.f_2D=state, state=sub_801B878(&headB,...), f_2E 同上。
 *   返回 0 (sb=r9 恒 0; out 栈字节=0 作 sub_801B878 的第三参输出)。
 * 形状要点: ①node(UnkNode*) 作循环变量 + obj(BattleObj*) 体变量 (同 sub_8018070 双变量族,
 *      `adds r4,r6` 拷贝); ②if(!(state&4)) 正排 (目标 bne 直跳 else 的 f_2D=state);
 *     ③趟2 每节点先 flag8000=0 再 obj=node。 */
u8 sub_80184A8(UnkNode *node, u8 state)
{
    BattleObj *obj;
    u8 gfxDone;
    u8 result;
    u8 out;
    u8 flag8000;

    gfxDone = 0;
    result = 0;
    out = result;

    sub_8018D9C();
    sub_801A2EC();

    while (node->key <= 0xFE)
    {
        obj = (BattleObj *)node;
        if ((obj->headA.kindFlags & 0x800) && gfxDone == 0)
        {
            ObjGfxLoad_Step(&obj->headA);
            gfxDone = 1;
        }
        if ((obj->state & 0x2000) && (obj->headB.kindFlags & 0x800) &&
            !(obj->headA.kindFlags & 0x800) && gfxDone == 0)
        {
            ObjGfxLoad_Step(&obj->headB);
            gfxDone = 1;
        }
        node = node->next;
    }

    node = node->next;

    while (node->key <= 0xFE)
    {
        flag8000 = 0;
        obj = (BattleObj *)node;
        if ((obj->headA.kindFlags & 0xF) != 3)
        {
            if (!(obj->state & 0x40))
            {
                obj->headA.f_2B = obj->posX;
                obj->headA.f_2C = obj->posY - obj->pad_C1;
            }
            state = sub_804B080(obj, state, gGstate324);
            if (!(obj->state & 4))
            {
                state = sub_801D378(obj, state);
                obj->headA.f_2D = state;
            }
            else
            {
                obj->headA.f_2D = state;
            }
        }
        else
        {
            obj->headA.f_2D = state;
        }

        if ((obj->headA.kindFlags & 0xF) == 6 || (obj->headB.kindFlags & 0xF) == 6)
            sub_801A270();

        if ((u8)(obj->slot - 12) <= 0x64 && !(obj->headA.kindFlags & 0x8000) &&
            obj->variantClass == 4)
        {
            obj->headA.kindFlags |= 0x8000;
            obj->headA.f_2F = sub_8020798();
            flag8000 = 1;
        }

        state = sub_801B878(&obj->headA, state, &out);
        obj->headA.f_2E = obj->headA.f_2D - state;

        if (flag8000 == 1)
            obj->headA.kindFlags &= 0x7FFF;

        if ((obj->state & 0x2000) && !(obj->headB.kindFlags & 0x800))
        {
            obj->headB.f_2D = state;
            state = sub_801B878(&obj->headB, state, &out);
            obj->headB.f_2E = obj->headB.f_2D - state;
        }

        node = node->next;
    }

    return result;
}
// @ 0x0801869C
void sub_801869C(void)
{
    if (gGstate324 & 0x20)
    {
        switch (gGstate32E - 0x3A)
        {
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
            case 15:
            case 16:
            default:
                Bgm_Play(2, 0);
                Bgm_FadeIn(0x14);
                break;
            case 12:
            case 13:
                Bgm_Play(3, 0);
                Bgm_FadeIn(0x14);
                break;
            case 14:
                Bgm_Play(4, 0);
                Bgm_FadeIn(0x14);
                break;
        }
    }
    else if (gGstate324 & 0x200)
    {
        Bgm_Play(2, 0);
        Bgm_FadeIn(0x14);
    }
    else
    {
        Bgm_Play(0, 0);
        Bgm_FadeIn(0x14);
    }
}
// @ 0x08018744
void sub_8018744(void)
{
    gKeyIgnoreTimer = 10;
}
extern u8 gUnk_080936A0[];

// @ 0x08018750
void sub_8018750(void)
{
    u16 offset;
    u16 count;

    offset = 0;
    count = 0;

    while (count <= 0x128)
    {
        if (gUnk_080936A0[offset] == 0xFF)
        {
            count++;
        }
        offset++;
    }

    gGstate340 = (u32)&gUnk_080936A0[offset];
}

// @ 0x0801878C
u32 sub_801878C(void)
{
    return gGstate340;
}
// @ 0x08018798
void sub_8018798(u8 index, s16 value)
{
    gGstate330[index] = value;
}

// @ 0x080187A8
u32 sub_80187A8()
{
    return gGstate32E;
}
// @ 0x080187B4
u16 sub_80187B4()
{
    return gGstate324;
}
// @ 0x080187C0
void sub_80187C0(u16 arg0)
{
    gGstate324 |= arg0;
}
// @ 0x080187D4
void sub_80187D4(u16 arg0)
{
    gGstate324 &= ~arg0;
}
// @ 0x080187E8
u16 sub_80187E8()
{
    return gGstate314;
}
// @ 0x080187F4
u16 sub_80187F4()
{
    return gGstate312;
}
// @ 0x08018800
void ListNode_Init(UnkNode *node)
{
    node->prev = node;
    node->next = node;
    node->key = -1;
}
// @ 0x0801880C
void ListNode_InitKey(UnkNode *node, u8 arg1)
{
    node->prev = 0;
    node->next = 0;
    node->key = arg1;
}
// @ 0x08018818
void ListNode_InsertSorted(UnkNode *head, UnkNode *new_node)
{
    UnkNode *cur = head->next;

    while (cur->key < new_node->key)
    {
        cur = cur->next;
    }
    new_node->next = cur;
    new_node->prev = cur->prev;
    cur->prev->next = new_node;
    cur->prev = new_node;
}
// @ 0x08018838
void sub_8018838(u32 arg0)
{
    gBattleRngSeed = arg0;
}
/*
    LCG（linear congruential generator）线性同余算法
*/
// @ 0x08018844
u32 Rng_LcgNext(void)
{
    u32 seed;
    seed = gBattleRngSeed * 0x41C64E6D + 0x3039;
    gBattleRngSeed = seed;
    return (seed / 0x10000) & 0x7FFF;
}

// @ 0x08018864
u32 GetObjPool()
{
    return 0x02037028;
}
// @ 0x0801886C
u32 GetCtx_0248()
{
    return 0x03000248;
}
// @ 0x08018874
u32 GetBuf_37410()
{
    return 0x02037410;
}

// @ 0x0801887C
void sub_801887C(void)
{
    if (!(gGstate324 & 8))
    {
        sub_80199E0();
        sub_804AF60();
    }
}
// @ 0x0801889C
void sub_801889C(void)
{
    BattleFx_UpdateTable();
    if (!(gGstate324 & 8))
    {
        sub_804AE2C();
    }
}
// @ 0x080188BC
void sub_80188BC(void)
{
    u16 keys;
    u16 tmp;

    if ((s8)gKeyIgnoreTimer <= 0)
        goto readkeys;
    gKeyIgnoreTimer--;
    tmp = gKeyIgnoreTimer;
    if ((s8)tmp > 0)
        goto clear;
readkeys:
    keys = (u16)~REG_KEYINPUT;
    gGstate312 = keys & ~gKeysHeld;
    gKeysHeld = keys;
    goto tail;
clear:
    gGstate312 = 0;
    gKeysHeld = 0;
tail:
    sub_80182A8(gKeysHeld, gGstate330);
}
// @ 0x08018928
void sub_8018928(void)
{
    if (gBattleUiFlags & 1)
    {
        REG_DISPCNT |= 0x100;
        gBattleUiFlags &= 0xFFFE;
    }
    if (gBattleUiFlags & 2)
    {
        REG_DISPCNT |= 0x200;
        gBattleUiFlags &= 0xFFFD;
    }
    if (gBattleUiFlags & 4)
    {
        REG_DISPCNT |= 0x400;
        gBattleUiFlags &= 0xFFFB;
    }
    if (gBattleUiFlags & 8)
    {
        REG_DISPCNT |= 0x800;
        gBattleUiFlags &= 0xFFF7;
    }
    if (gBattleUiFlags & 0x10)
    {
        REG_DISPCNT &= 0xFEFF;
        gBattleUiFlags &= 0xFFEF;
    }
    if (gBattleUiFlags & 0x20)
    {
        REG_DISPCNT &= 0xFDFF;
        gBattleUiFlags &= 0xFFDF;
    }
    if (gBattleUiFlags & 0x40)
    {
        REG_DISPCNT &= 0xFBFF;
        gBattleUiFlags &= 0xFFBF;
    }
    if (gBattleUiFlags & 0x80)
    {
        REG_DISPCNT &= 0xF7FF;
        gBattleUiFlags &= 0xFF7F;
    }
}
// @ 0x08018A58
/* 战斗背景加载: idx*12 查 0x087ED394 三元组 (瓦片/调色板/瓦片地图),
 * 装载到 VRAM 并配置 DISPCNT (开启 BG2/BG3/OBJ/WIN0) 与 BG2CNT (CharBase 0, ScreenBase 12, 循环)。
 * 初始化波浪滚动表与 BG 滚动备份。 */
typedef struct BattleBgGfx
{
    const u8 *tiles;    /* +0x00 瓦片数据 (LZ77 解压到 VRAM 0x06000000, CharBase 0) */
    const u8 *pal;      /* +0x04 调色板数据 (sub_804C548 加载到 BG 调色板槽 0) */
    const u8 *map;      /* +0x08 瓦片地图 (LZ77 解压到 VRAM 0x06006000, ScBase 12) */
} BattleBgGfx;

#define Unk_087ED394 BattleBgGfx

extern const BattleBgGfx gUnk_087ED394[];
extern const u8 gUnk_0861A4A4[];
extern const u8 gUnk_0809C834[];
extern u8 gUnk_02036EC0[];
extern u32 gWaveTablePtr;
extern u32 gWaveBgHofsTbl[4];
extern u32 gWaveBgVofsTbl[4];
extern u8 gWaveMode;

void sub_8018BF8();
void sub_804C548();
void BgLoad_Finish();

typedef union {
    DispCnt disp;
    u32 word;
} DispUnion;

typedef union {
    BgCnt bg;
    u32 word;
} BgUnion;

void sub_8018A58(u8 unused)
{
    DispUnion dispcnt;
    BgUnion bg2cnt;
    u32 idx;

    idx = sub_8018E34();
    LZ77UnCompVram((void *)gUnk_087ED394[idx].tiles, (void *)0x06000000);
    sub_804C548((u32)gUnk_087ED394[idx].pal, 0, 3);
    LZ77UnCompVram((void *)gUnk_087ED394[idx].map, (void *)0x06006000);

    DmaCopy32(3, &gUnk_0861A4A4, 0x06005000, 0x50 * 4);
    DmaWait(3);

    sub_8018BF8();

    dispcnt.disp.BgMode = 0;
    dispcnt.disp.Bmp_FrameNo = 0;
    dispcnt.disp.Obj_H_Off = 1;
    dispcnt.disp.ObjCharMapType = 1;
    dispcnt.disp.Lcdc_Off = 0;
    dispcnt.disp.Bg0_On = 0;
    dispcnt.disp.Bg1_On = 0;
    dispcnt.disp.Bg2_On = 1;
    dispcnt.disp.Bg3_On = 1;
    dispcnt.disp.Obj_On = 1;
    dispcnt.disp.Win0_On = 1;
    dispcnt.disp.Win1_On = 0;
    dispcnt.disp.ObjWin_On = 0;

    bg2cnt.bg.Priority = 3;
    bg2cnt.bg.CharBasep = 0;
    bg2cnt.bg.Dummy_5_4 = 0;
    bg2cnt.bg.Mosaic = 0;
    bg2cnt.bg.ColorMode = 0;
    bg2cnt.bg.ScBasep = 12;
    bg2cnt.bg.Loop = 1;
    bg2cnt.bg.Size = 0;

    REG_DISPCNT = dispcnt.word;
    REG_BG2CNT = bg2cnt.word;

    DmaFill16(3, 0, &gUnk_02036EC0, 0xB4 * 2);
    DmaWait(3);

    gWaveTablePtr = (u32)gUnk_02036EC0;
    sub_804C548((u32)gUnk_0809C834, 0xB, 3);
    gWaveMode = 0;
    gWaveBgHofsTbl[0] = REG_ADDR_BG0HOFS;
    gWaveBgHofsTbl[1] = REG_ADDR_BG1HOFS;
    gWaveBgHofsTbl[2] = REG_ADDR_BG2HOFS;
    gWaveBgHofsTbl[3] = REG_ADDR_BG3HOFS;
    gWaveBgVofsTbl[0] = REG_ADDR_BG0VOFS;
    gWaveBgVofsTbl[1] = REG_ADDR_BG1VOFS;
    gWaveBgVofsTbl[2] = REG_ADDR_BG2VOFS;
    gWaveBgVofsTbl[3] = REG_ADDR_BG3VOFS;
    gBgScrollBackup.bg0Vofs = 0;
    gBgScrollBackup.bg0Hofs = 0;
    gBgScrollBackup.bg1Vofs = 0;
    gBgScrollBackup.bg1Hofs = 0;
    gBgScrollBackup.bg2Vofs = 0;
    gBgScrollBackup.bg2Hofs = 0;
    gBgScrollBackup.bg3Vofs = 0;
    gBgScrollBackup.bg3Hofs = 0;
    BgLoad_Finish();
}
// @ 0x08018BF8
extern const u8 gUnk_083939F8[];
extern const u8 gUnk_0861A5E4[];
extern const u8 gUnk_0861C664[];
extern const u8 gUnk_08619FA4[];
extern const u8 gUnk_08619AA4[];
extern const u8 gUnk_08393A54[];
void *memcpy(void *, const void *, unsigned long);

/* 战斗背景素材装配: 先把 13 字节图形表 0x083939F8 拷到栈缓冲 buf, 再把 0x0861A4A4 的
 * 0x140 字节 tile 数据拷到 VRAM 0x06005000; 然后遍历 5 个队伍槽 gPartyMemberIds[0..4]
 * (0xFF = 空), 用 buf[id] 选 0x08619FA4 中每条 0x80 字节的角色图形依次拷到 0x06005140 起;
 * 接着装配 gUnk_0861A5E4 的 0x20/+0x20 (共 0x40) 字节与 0x08619AA4 中由 gUnk_08393A54[0x14]
 * 选中的 0x80 字节块; 载入两组调色板 (0x0861C664 的槽 3/9) 后清空 tilemap; 最后把 BG3CNT
 * 配成 Priority 1 / CharBase 0 / ScBasep 13 / Loop 1 (终值 0x2D01)。
 * 形状要点: ①槽基址经 u32 变量承接 (gPartyMemberIds 是 EWRAM 数组地址), 使 `j + slots`
 * 保持整数加法序 (目标 `adds r1,r2,r0`); ②gfx 那笔 DMA 包一层 do{...}while(0) 改变 agbcc
 * 块编号, 才让槽基址落 r8、gfx 落 ip (经验 113/2548 族的块编号杠杆)。 */
void sub_8018BF8(void)
{
    u8 buf[13];
    u8 k;
    u8 j;
    u32 slots;
    const u8 *gfx;
    const u8 *gfx2;
    BgUnion bg3cnt;

    memcpy(buf, (void *)gUnk_083939F8, 13);
    DmaCopy32(3, &gUnk_0861A4A4, (void *)0x06005000, 0x50 * 4);
    DmaWait(3);
    k = 0;
    j = 0;
    slots = (u32)gPartyMemberIds;
    gfx = gUnk_0861A5E4;
    gfx2 = gfx + 0x20;
    do
    {
        if (*(u8 *)(j + slots) != 0xFF)
        {
            DmaCopy32(3, gUnk_08619FA4 + (buf[*(u8 *)(j + slots)] << 7), (void *)(0x06005140 + (k << 7)), 0x20 * 4);
            DmaWait(3);
            k++;
        }
        j++;
    } while (j <= 4);
    do
    {
        DmaCopy32(3, gfx, (void *)0x060053E0, 8 * 4);
    } while (0);
    DmaWait(3);
    DmaCopy32(3, gfx2, (void *)0x06005400, 0x10 * 4);
    DmaWait(3);
    DmaCopy32(3, gUnk_08619AA4 + (gUnk_08393A54[0x14] << 5), (void *)0x06005440, 0x20 * 4);
    sub_804C548((u32)gUnk_0861C664, 3, 6);
    sub_804C548((u32)gUnk_0861C664 - 0x40, 9, 1);
    BlankTilemap();
    bg3cnt.bg.Priority = 1;
    bg3cnt.bg.CharBasep = 0;
    bg3cnt.bg.Dummy_5_4 = 0;
    bg3cnt.bg.Mosaic = 0;
    bg3cnt.bg.ColorMode = 0;
    bg3cnt.bg.ScBasep = 13;
    bg3cnt.bg.Loop = 1;
    bg3cnt.bg.Size = 0;
    REG_BG3CNT = bg3cnt.word;
}
/* 战斗场景 tilemap 缓冲(0x020352C0 + 错位视图 0x020352C2)的第 0x221/0x241 项:
 * 按 gGstate324 bit14 选 0x92A2..5(战斗 UI 边框)或 0x92C0(空), 见调用点 sub_8018928。 */
// @ 0x08018D9C
void sub_8018D9C(void)
{
    u16 idx;
    u16 *p;
    u16 *q;

    idx = 0x221;
    p = (u16 *)0x020352C0;
    q = (u16 *)0x020352C2;

    if (sub_80187B4() & 0x4000)
    {
        p[idx] = 0x92A2;
        q[idx] = 0x92A3;
        p[0x241] = 0x92A4;
        q[0x241] = 0x92A5;
    }
    else
    {
        p[idx] = 0x92C0;
        q[idx] = 0x92C0;
        p[0x241] = 0x92C0;
        q[0x241] = 0x92C0;
    }
}
extern u8 gUnk_083989B0[];
extern u8 gUnk_083989CB[];
extern u8 gUnk_083989DC[];

// @ 0x08018E34
u32 sub_8018E34(void)
{
    u8 ret;
    if (sub_80187B4() & 0x20)
    {
        ret = gUnk_083989CB[(u8)sub_80187A8() - 0x3a];
    }
    else if (sub_80187B4() & 0x200)
    {
        ret = gUnk_083989DC[(u8)sub_80187A8() - 0x1c];
    }
    else if (gEncounterEnabled != 0)
    {
        ret = gUnk_083989B0[gEncounterEnabled - 1];
    }
    else
    {
        ret = gUnk_083989B0[gEncounterEnabled];
    }
    return ret;
}
// @ 0x08018EA8
INCLUDE_ASM("asm/nonmatchings", sub_8018EA8);
// @ 0x08018FC0
INCLUDE_ASM("asm/nonmatchings", sub_8018FC0);
// @ 0x08019148
/* BG0 复位: 清零 EWRAM 0x02035AC0 与 VRAM 0x06007000 各 0x400 个半字, 开 BG0,
 * 再把 BG0CNT 配成 CharBase 2 / ScreenBase 0xE (最终值 0xE08)。
 * 末尾的位域读改写链由 BgCnt 逐字段赋值产生 (容器未初始化, 故 GCC2 只发 RMW 掩码)。 */
void Bg0_InitClear(void)
{
    u16 *ewram;
    u16 *vram;
    u16 i;
    BgUnion bg0cnt;

    ewram = (u16 *)0x02035AC0;
    vram = (u16 *)0x06007000;
    i = 0;
    do
    {
        ewram[i] = 0;
        vram[i] = 0;
        i++;
    } while (i <= 0x3FF);

    REG_DISPCNT |= DISPCNT_BG0_ON;

    bg0cnt.bg.Priority = 0;
    bg0cnt.bg.CharBasep = 2;
    bg0cnt.bg.Dummy_5_4 = 0;
    bg0cnt.bg.Mosaic = 0;
    bg0cnt.bg.ColorMode = 0;
    bg0cnt.bg.ScBasep = 14;
    bg0cnt.bg.Loop = 0;
    bg0cnt.bg.Size = 0;

    REG_BG0CNT = bg0cnt.word;
}
// @ 0x080191CC
INCLUDE_ASM("asm/nonmatchings", sub_80191CC);
/* 清空 gDialogCtx[0..2] 共 3 个表项。
 * 注意必须用结构体成员形式逐个写: 目标是对同一基址取 11 个 `strb [r0,#N]`
 * + 2 个 `strh [r0,#0xc/#0xe]` 位移寻址。改成 `u8 *b; b[N] = 0;` 会被 GCC2
 * 强度削减成 `adds` 连续递增, 逐指令全变(规则 11 / 67)。
 * 0xb (field_B) 不被清零; 0xe/0xf 是一条 u16 存零, 所以原代码在那里看的是 u16 字段。 */
// @ 0x08019304
void DialogCtx_Clear3(void)
{
    u8 i;
    Unk_03000348 *ptr;

    for (i = 0; i <= 2; i++)
    {
        ptr = &gDialogCtx[i];
        ptr->padding0[0] = 0;
        ptr->padding0[1] = 0;
        ptr->padding0[2] = 0;
        ptr->padding0[3] = 0;
        ptr->padding0[4] = 0;
        ptr->padding0[5] = 0;
        ptr->padding0[6] = 0;
        ptr->padding0[7] = 0;
        ptr->field_8 = 0;
        ptr->field_9 = 0;
        ptr->field_A = 0;
        ptr->field_C = 0;
        *(u16 *)&ptr->field_E = 0;
    }
}

// @ 0x0801933C
INCLUDE_ASM("asm/nonmatchings", sub_801933C);
// @ 0x080196D4
void sub_80196D4(index, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) u8 index;
u32 arg1;
u16 arg2;
u8 arg3;
u8 arg4;
u8 arg5;
u8 arg6;
u8 arg7;
u8 arg8;
{
    gDialogCtx[index].padding0[0] = arg5;
    gDialogCtx[index].padding0[1] = arg6;
    gDialogCtx[index].padding0[2] = arg7;
    gDialogCtx[index].padding0[3] = arg8;
    gDialogCtx[index].padding0[4] = 0;
    gDialogCtx[index].padding0[5] = 0;
    gDialogCtx[index].padding0[6] = 0;
    gDialogCtx[index].padding0[7] = 0;
    gDialogCtx[index].field_8 = arg3;
    gDialogCtx[index].field_9 = 0;
    gDialogCtx[index].field_A = arg4;
    gDialogCtx[index].field_B = 0;
    gDialogCtx[index].field_C = 1;
    gDialogCtx[index].field_E = arg2;
    gDialogCtx[index].field_10 = arg1;
}
// @ 0x08019748
void DialogCtx_SetPair(u32 arg0, u32 arg1, u32 arg2, u32 arg3, u32 arg4)
{
    u8 a;
    u8 b;
    u8 c;
    u8 d;
    u8 e;
    u8 *tbl;
    u8 *ptr;

    a = arg0;
    b = arg1;
    c = arg2;
    d = arg3;
    e = arg4;
    tbl = (u8 *)gDialogCtx;
    ptr = tbl + a * 0x14;
    ptr[0] = b;
    ptr[1] = c;
    ptr[2] = d;
    ptr[3] = e;
    ptr[4] = b;
    ptr[5] = c;
    ptr[6] = d;
    ptr[7] = e;
}
// @ 0x08019784
void BattleFx_UpdateTable(void)
{
    s16 i;
    s16 tmp;
    if (0x1000 & gFlashFlags)
    {
        switch (gFlashFlags & 0xF)
        {
        case 0:
            break;
        case 1:
            gWaveAngle = ((s16)gWaveAngle + gWaveAngleVel) % 360;
            for (i = 0, tmp = gWaveAngle; i <= 0x9F;)
            {
                gWaveRowOffset[i] = (s8)((s8 *)gWaveTablePtr)[(s16)tmp % 360];
                i++;
                tmp = (u16)(tmp + gWaveRowStep);
            }
            break;
        case 2:
        {
            int diff;
            if ((s16)gWaveAngle <= 0x10F)
            {
                gWaveAngle = (u16)(gWaveAngle + gWaveAngleVel);
                for (i = 0x50; i <= 0x9F; i++)
                {
                    diff = gWaveAngle - i;
                    tmp = (s16)diff >> 2;
                    if (tmp > 24)
                    {
                        tmp = 24;
                    }
                    if (tmp < 0)
                    {
                        tmp = 0;
                    }
                    switch (gFlashFlags & 0xF0)
                    {
                    case 0x10:
                        gWaveRowOffset[i] = tmp;
                        break;
                    case 0x20:
                        gWaveRowOffset[i] = 24 - tmp;
                        break;
                    }
                }
                for (i = 0; i <= 0x4F; i++)
                {
                    gWaveRowOffset[i] = gWaveRowOffset[0xA0 - i];
                }
            }
            else
            {
                gFlashFlags |= 0x4000;
            }
        }
        break;
        }
    }
    else if (0x2000 & gFlashFlags)
    {
        switch (gFlashFlags & 0xF)
        {
        case 0:
            break;
        case 1:
        {
            s16 j;
            for (j = gWaveAngle; j < ((s16)gWaveAngle + 18); j++)
            {
                ((s8 *)gWaveTablePtr)[j] = (int)((float)(int)gWaveAmp * gCosTable[j] - (float)(int)gWaveAmp * gSinTable[j] + (float)(int)gWaveAmp);
            }
            if (j <= 359)
            {
                gWaveAngle = j;
            }
            else
            {
                gFlashFlags = (gFlashFlags & ~0x2000) | 0x1000;
                gWaveAngle = 0;
            }
        }
        break;
        case 2:
            gFlashFlags = (gFlashFlags & ~0x2000) | 0x1000;
            gWaveAngle = 0;
            break;
        }
    }
}
// @ 0x080199E0
// 淡出步进: flags=gFlashFlags; 若 flags&0x1000 按低 nibble 分派。
// case1: 4 通道循环, bits=(u8*)0x030004D7, 第 i 位为 1 时把
//   gWaveRowOffset[*(vu16*)0x04000006 & 0xFF] 写入 gWaveBgHofsTbl[i], 其 >>1 写入 gWaveBgVofsTbl[i]。
// case2: REG_BLDY = gWaveRowOffset[*(u8*)0x04000006]; 再按 flags&0xF00 设 REG_BLDCNT
//   (0x100→0xBF, 0x200→0xFF)。case2 的 bldy/tbl/port 三指针预载 + i=0xFF 之间形成
//   arm_reorg 调度窗口, 使 movs r4,#0xff 落入 ldr→ldrb 延迟槽 (规则128/134 族)。
void sub_80199E0(void)
{
    u16 flags;
    u8 i;
    u8 *bits;
    vu16 *bldy;
    u16 *tbl;
    u8 *port;

    flags = gFlashFlags;
    if (flags & 0x1000)
    {
        switch (flags & 0xF)
        {
        case 0:
            break;
        case 1:
            for (i = 0, bits = (u8 *)0x030004D7; i < 4; i++)
            {
                if ((bits[0] >> i) & 1)
                {
                    *(u16 *)gWaveBgHofsTbl[i] = gWaveRowOffset[*(vu16 *)0x04000006 & 0xFF];
                    *(u16 *)gWaveBgVofsTbl[i] = gWaveRowOffset[*(vu16 *)0x04000006 & 0xFF] >> 1;
                }
            }
            break;
        case 2:
            bldy = (vu16 *)0x04000054;
            tbl = gWaveRowOffset;
            port = (u8 *)0x04000006;
            i = 0xFF;
            *bldy = tbl[*port];
            switch (flags & 0xF00)
            {
            case 0x100:
                REG_BLDCNT = 0xBF;
                break;
            case 0x200:
                REG_BLDCNT = i;
                break;
            }
            break;
        }
    }
}
// @ 0x08019AD0
void sub_8019AD0(u8 arg0, u16 arg1)
{
    u16 v;

    v = gFlashFlags & 0xFFF0;
    v &= 0xFF0F;
    v &= 0xF0FF;
    v |= 2;
    gFlashFlags = arg1 | v | 0x1000;
    gWaveAngle = 0;
    *(vu16 *)0x04000048 = 0x3F;
    *(vu16 *)0x04000040 = 0xF0;
    *(vu16 *)0x04000044 = 0x2A0;
    REG_DISPCNT |= 0x2000;
    gWaveAngleVel = arg0;
    gWaveAmp = 0;
    switch (gFlashFlags & 0xF0)
    {
    case 0x10:
        break;
    case 0x20:
        REG_BLDY = 0x18;
        break;
    }
    switch (gFlashFlags & 0xF00)
    {
    case 0x100:
        REG_BLDCNT = 0xBF;
        break;
    case 0x200:
        REG_BLDCNT = 0xFF;
        break;
    }
}

// @ 0x08019B98
/* 战斗背景组表条目 (gBgLoadTable): 一个"背景组"= 调色板 + 地图 + 若干图块块。 */
typedef struct
{
    u32 pal;       /* +0x00 调色板源 (case1 完成后 sub_804C548 装载) */
    u32 map;       /* +0x04 图块地图 LZ77 源 (case2 → 0x020362C0) */
    u16 chunkBase; /* +0x08 gBgLoadChunks[] 起始下标 */
    u16 chunkCount;/* +0x0A 本组的图块块数 */
} BgLoadEntry;

extern const BgLoadEntry gBgLoadTable[];
extern const u32 gBgLoadChunks[];

/* 战斗背景分步装载状态机 (由事件演出逐帧调用, 返回 1 表示完成):
 *   case0: 复位 → 1
 *   case1: 逐帧把 gBgLoadTable[arg0] 组的第 gUnk_03000514 块 LZ77 解压到
 *          VRAM (0x06000000 + arg1*0x4000, 块间步长 0x1000); 全部解完后装载
 *          该组调色板 (槽 arg2) → 2
 *   case2: 把该组地图 LZ77 解压到 WRAM 缓冲 0x020362C0 → 3
 *   case3/4: 给缓冲前/后半 (各 0x200 项) 的调色板索引位填 arg2&0xF → 4 / → 5
 *   case5: 逐帧 DMA 4 个 0x200B 块 (0x020362C0 → 0x0600_6800 + 块号*0x200, arg1==3
 *          时基址 0x7800); 4 块送完后按 arg1 (2→BG3 / 3→BG1) 配置 BGxCNT
 *          (Priority=arg3 / CharBase=arg1 / ScreenBase=0xD, arg1==3 时 0xF / Loop=1),
 *          返回 1 并复位状态机。
 * 形状要点: ①case1 表基址必须经 `tbl` 局部承接, 否则该池加载被调度到索引计算之后
 *   (目标顺序 ldr =table; 索引; ldrh count); ②case3/4 的调色板值须提到循环外,
 *   且循环体带花括号; ③case5 的 VRAM 目标用 `(i<<11)` 单独算, 而非内联三目。 */
u8 sub_8019B98(u8 arg0, u8 arg1, u8 arg2, u8 arg3)
{
    u8 result;
    u8 *vram;
    u16 i;
    u8 pos;
    const BgLoadEntry *tbl;
    const BgLoadEntry *entry;

    result = 0;
    vram = (u8 *)(0x06000000 + (arg1 << 14));

    switch (gUnk_03000512)
    {
    case 0:
        gUnk_03000514 = 0;
        gUnk_03000512 = 1;
        break;
    case 1:
        pos = gUnk_03000514;
        tbl = gBgLoadTable;
        entry = &tbl[arg0];
        if (pos < entry->chunkCount)
        {
            LZ77UnCompVram((void *)gBgLoadChunks[entry->chunkBase + pos], vram + (pos << 12));
            gUnk_03000514++;
        }
        else
        {
            sub_804C548(entry->pal, arg2, 1);
            gUnk_03000512 = 2;
        }
        break;
    case 2:
        LZ77UnCompWram((void *)gBgLoadTable[arg0].map, (void *)0x020362C0);
        gUnk_03000512 = 3;
        break;
    case 3:
    {
        u16 *p = (u16 *)0x020362C0;
        u16 pal = (arg2 & 0xF) << 12;
        for (i = 0; i < 0x200; i++)
            p[i] = (p[i] & 0xFFF) | pal;
        gUnk_03000512 = 4;
        break;
    }
    case 4:
    {
        u16 *p = (u16 *)0x020362C0;
        u16 pal = (arg2 & 0xF) << 12;
        for (i = 0x200; i < 0x400; i++)
            p[i] = (p[i] & 0xFFF) | pal;
        gBgLoadChunkIdx = 0;
        gUnk_03000512 = 5;
        break;
    }
    case 5:
        if (gBgLoadChunkIdx <= 3)
        {
            vu32 *dmaRegs;
            i = (arg1 == 3) ? 0xF : 0xD;
            dmaRegs = (vu32 *)REG_ADDR_DMA3;
            dmaRegs[0] = (vu32)(0x020362C0 + (gBgLoadChunkIdx << 9));
            dmaRegs[1] = (vu32)(((i << 11) + 0x06000000) + (gBgLoadChunkIdx << 9));
            dmaRegs[2] = 0x84000080;
            dmaRegs[2];
            while (dmaRegs[2] & 0x80000000)
                ;
            gBgLoadChunkIdx++;
        }
        else
        {
            BgUnion bgcnt;

            bgcnt.bg.Priority = arg3 & 3;
            bgcnt.bg.CharBasep = arg1 & 3;
            bgcnt.bg.Dummy_5_4 = 0;
            bgcnt.bg.Mosaic = 0;
            bgcnt.bg.ColorMode = 0;
            bgcnt.bg.ScBasep = (arg1 == 3) ? 0xF : 0xD;
            bgcnt.bg.Loop = 1;
            bgcnt.bg.Size = 0;
            if (arg1 == 3)
            {
                REG_BG1CNT = bgcnt.word;
                REG_DISPCNT |= 0x200;
            }
            else if (arg1 == 2)
            {
                REG_BG3CNT = bgcnt.word;
            }
            result = 1;
            gUnk_03000512 = 0;
            gBgLoadChunkIdx = 0;
        }
        break;
    }
    return result;
}
// @ 0x08019DF8
void BattleUiFlag_Clear()
{
    gBattleUiFlags = 0;
}
// @ 0x08019E04
void BattleUiFlag_Set(u16 arg0)
{
    gBattleUiFlags |= arg0;
}
// @ 0x08019E18
u16 BattleUiFlag_Get()
{
    return gBattleUiFlags;
}
// @ 0x08019E24
void BattleUiFlag_Reset(u16 mask)
{
    gBattleUiFlags &= ~mask;
}
// @ 0x08019E38
void Disp_ObjOff(void)
{
    REG_DISPCNT &= 0xF7FF;
}
// @ 0x08019E4C
void Disp_ObjOn(void)
{
    REG_DISPCNT |= 0x800;
}
// 清空 VRAM 上编号 0x2C0 的那块图块(0x06005800, 4bpp 8×8 = 32 字节),
// 并把 32×32 = 1024 项的 tilemap 缓冲区(0x020352C0)全部填成指向该空白图块。
// 项格式: bit0-9 图块号(0x2C0), bit10-11 清 0, bit12-15 = 3|(原值 bit14-15)。
// 注: attr 在原始代码里就是**未初始化**的局部 —— 目标第一条相关指令是
//     `ands r2, r0`(r2 从未被写入), 两个调用点也都直接 `bl sub_8019E60` 不传参。
//     写成参数或预先赋值都会多指令/少指令, 不匹配。
// @ 0x08019E60
void sub_8019E60(void)
{
    u32 attr;
    u16 *map;
    u8 *tile;
    u16 i;
    u32 tmp;

    map = (u16 *)0x020352C0;
    tile = (u8 *)0x06005800;
    for (i = 0; i <= 0x1F; i++)
    {
        tile[i] = 0;
    }

    attr &= ~0x3FF;
    attr |= 0x2C0;
    tmp = 0x400;
    attr &= ~tmp;
    attr &= ~0x800;
    attr &= ~0xF000;
    attr |= 0x3000;
    // 注: 外层 do {} while(0) 是 GCC2 调度屏障(规律25)。去掉后第二个循环的
    //     `movs r1,#0` 会从 `orrs r2,r0` 之前挪到之后, 差 4 字节。
    do
    {
        for (i = 0; i <= 0x3FF; i++)
        {
            tmp = attr;
            map[i] = tmp;
        }
    } while (0);
}
// @ 0x08019ECC
void Disp_Bg1Off(void)
{
    REG_DISPCNT &= 0xFEFF;
}

// @ 0x08019EE0
void DialogCtx_SetHead(u8 index, u8 arg1, u8 arg2)
{
    gDialogCtx[index].field_8 = arg1;
    gDialogCtx[index].field_9 = 0;
    gDialogCtx[index].field_A = arg2;
    gDialogCtx[index].field_C = 5;
}
// @ 0x08019F08
void sub_8019F08(u16 *tilemap, u16 addVal, u8 startCol, u8 startRow, u8 width, u8 height)
{
    u16 *p;
    u8 col;
    u8 row;

    p = &tilemap[startRow * 32 + startCol];
    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            p[col] = (p[col] & 0xFC00) + addVal;
        }
        p += 32;
    }
}
// @ 0x08019F78
void sub_8019F78(u16 *dest, int a1, s8 shift, int a3, u8 left, u8 top, u8 width, u8 height)
{
    u8 col;
    u8 x;
    u8 y;

    if (shift == 0)
        return;
    if (shift > 0)
    {
        col = left + width;
        for (x = 0; x < width; x++)
        {
            for (y = top; y < height + top; y++)
                dest[(y << 5) + (col + shift)] = dest[(y << 5) + col];
            col--;
        }
    }
    else
    {
        col = left;
        for (x = 0; x < width; x++)
        {
            for (y = top; y < height + top; y++)
                dest[(y << 5) + (col + shift)] = dest[(y << 5) + col];
            col++;
        }
    }
}
// @ 0x0801A05C
u8 DialogCtx_GetField_C(u8 index)
{
    return gDialogCtx[index].field_C;
}
/* BG map 矩形区域调色板覆盖: 以 (x,y) 为左上角、width×height 的半字区,
 * 每项 (tile & 0x0FFF) + palette<<12 (保留 tile 号, 替换高 4 位调色板号)。
 * 调用点: sub_8020D50.c 菜单条目高亮 (style+0xB 选调色板, x=8, y=(i-view)*2+8, w=9, h=2)。 */
// @ 0x0801A074
void BgMap_PalFillRect(base, palette, x, y, width, height) u16 *base;
u16 palette;
u8 x;
u8 y;
u8 width;
u8 height;
{
    u8 col;
    u16 *dst;
    u8 row;

    dst = base + ((16 * (y * 2)) + x);
    for (row = 0; row < height; row++)
    {
        for (col = 0; col < width; col++)
        {
            dst[col] = (palette << 12) + (dst[col] & 0x0FFF);
        }

        dst += 32;
    }
}
// @ 0x0801A0F0
void DialogCtx_Flush(void)
{
    if (gDialogCtx[0].field_C || gDialogCtx[1].field_C != 0 || gDialogCtx[2].field_C != 0)
    {
        DmaCopy32(3, 0x02035AC0, 0x06007000, 0x800);
        DmaWait(3);
    }
}
// @ 0x0801A13C
void FlashFlag_Clear()
{
    gFlashFlags = 0;
}
// @ 0x0801A148
u16 FlashFlag_Get()
{
    return gFlashFlags;
}
// @ 0x0801A154
void FlashFlag_Reset(u16 mask)
{
    gFlashFlags &= ~mask;
}
// @ 0x0801A168
void BattleFx_Init(u8 arg0, u8 arg1, u8 arg2, u8 arg3)
{
    gFlashFlags &= 0xFFF0;
    gFlashFlags |= 1;
    gFlashFlags |= 0x2000;
    if (gFlashFlags & 0x1000)
        gFlashFlags &= ~0x1000;

    gWaveAngle = 0;

    gWaveAngleVel = arg0;
    gWaveAmp = arg1;
    gWaveRowStep = arg2;
    gWaveMode = arg3;
}
// @ 0x0801A1DC
void BattleFx_Stop(void)
{
    gFlashFlags &= 0xFFF0;
    gFlashFlags &= 0xEFFF;
    if (gFlashFlags & 0x4000)
    {
        gFlashFlags &= 0xBFFF;
    }

    gWaveMode = 0;
}

// @ 0x0801A218
void BattleFx_DispOff(void)
{
    REG_DISPCNT &= 0xDFFF;
    REG_BLDY = 0;
    REG_BLDCNT = 0;

    gFlashFlags &= 0xFFF0;
    gFlashFlags &= 0xEFFF;

    if (gFlashFlags & 0x4000)
    {
        gFlashFlags &= 0xBFFF;
    }

    *(u8 *)0x030004D7 = 0;
}

// @ 0x0801A270
void sub_801A270(void)
{
    DmaFill16(3, 100, (void *)0x020362C0, 0x800);
    DmaWait(3);
}

// @ 0x0801A2AC
void sub_801A2AC(int arg0, int arg1, int arg2)
{
    u32 v = arg0 << 16;
    u8 b1 = arg1;
    u8 b2 = arg2;

    REG_BLDCNT = arg0;
    REG_BLDALPHA = b1 | (b2 << 8);
    v >>= 22;
    v &= 2;
    switch ((u16)v)
    {
    case 2:
    case 3:
        REG_BLDY = b1;
        break;
    }
}
extern u8 *gUnk_087EBDF0[];

// @ 0x0801A2EC
void sub_801A2EC(void)
{
    if (gBgLoadSlot <= 3)
    {
        LZ77UnCompVram(gUnk_087EBDF0[gBgLoadSlot], (void *)(0x06008000 + gBgLoadSlot * 0x1000));
        gBgLoadSlot++;
    }
}
// @ 0x0801A324
void BgLoad_Reset(void)
{
    gBgLoadSlot = 0;
    return;
}
// @ 0x0801A330
void BgLoad_Finish(void)
{
    gBgLoadSlot = 4;
}
// @ 0x0801A33C
u8 BgLoad_GetPos(void)
{
    return gBgLoadSlot;
}
// @ 0x0801A348
void sub_801A348(void)
{
    gUnk_03000512 = 0;
    gUnk_03000514 = 0;
}
// @ 0x0801A35C
void sub_801A35C(void)
{
    sub_8018BF8();
    sub_80187D4(0x10);
}

// @ 0x0801A36C
void BgScrolls_WriteAll(void)
{

    REG_BG0HOFS = gBgScrollBackup.bg0Hofs;
    REG_BG0VOFS = gBgScrollBackup.bg0Vofs;
    REG_BG1HOFS = gBgScrollBackup.bg1Hofs;
    REG_BG1VOFS = gBgScrollBackup.bg1Vofs;
    REG_BG2HOFS = gBgScrollBackup.bg2Hofs;
    REG_BG2VOFS = gBgScrollBackup.bg2Vofs;
    REG_BG3HOFS = gBgScrollBackup.bg3Hofs;
    REG_BG3VOFS = gBgScrollBackup.bg3Vofs;
}
typedef union
{
    u16 array[4][2];
} U0500_arr;

// @ 0x0801A3A8
void sub_801A3A8(u8 arg0, u16 arg1, u16 arg2)
{
    U0500_arr *u;

    u = (U0500_arr *)&gBgScrollBackup;
    u->array[arg0][0] = arg1;
    u->array[arg0][1] = arg2;
}