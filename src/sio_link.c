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
INCLUDE_ASM("asm/nonmatchings", sub_8017FA4);
// @ 0x08018070
INCLUDE_ASM("asm/nonmatchings", sub_8018070);
// @ 0x080182A8
INCLUDE_ASM("asm/nonmatchings", sub_80182A8);
// @ 0x080184A8
INCLUDE_ASM("asm/nonmatchings", sub_80184A8);
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
    gUnk_03000316 = 10;
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
u16 Rng_LcgNext(void)
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

    if ((s8)gUnk_03000316 <= 0)
        goto readkeys;
    gUnk_03000316--;
    tmp = gUnk_03000316;
    if ((s8)tmp > 0)
        goto clear;
readkeys:
    keys = (u16)~REG_KEYINPUT;
    gGstate312 = keys & ~gUnk_03000310;
    gUnk_03000310 = keys;
    goto tail;
clear:
    gGstate312 = 0;
    gUnk_03000310 = 0;
tail:
    sub_80182A8(gUnk_03000310, gGstate330);
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
INCLUDE_ASM("asm/nonmatchings", sub_8018A58);
// @ 0x08018BF8
INCLUDE_ASM("asm/nonmatchings", sub_8018BF8);
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
u8 sub_8018E34(void)
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
void Bg0_InitClear(s32 a, s32 b, s32 c, s32 d)
{
    u16 *ewram;
    u16 *vram;
    u16 i;
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
    d &= ~3;
    d &= ~0xC;
    d |= 8;
    d &= ~0x30;
    d &= ~0x40;
    d &= ~0x80;
    d &= 0xFFFFE0FF;
    d |= 0xE00;
    d &= ~0x2000;
    do
    {
        d &= ~0xC000;
    } while (0);
    REG_BG0CNT = d;
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
            gUnk_03000386 = ((s16)gUnk_03000386 + gUnk_030004D4) % 360;
            for (i = 0, tmp = gUnk_03000386; i <= 0x9F;)
            {
                gUnk_03000390[i] = (s8)((s8 *)gUnk_030004D0)[(s16)tmp % 360];
                i++;
                tmp = (u16)(tmp + gUnk_030004D6);
            }
            break;
        case 2:
        {
            int diff;
            if ((s16)gUnk_03000386 <= 0x10F)
            {
                gUnk_03000386 = (u16)(gUnk_03000386 + gUnk_030004D4);
                for (i = 0x50; i <= 0x9F; i++)
                {
                    diff = gUnk_03000386 - i;
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
                        gUnk_03000390[i] = tmp;
                        break;
                    case 0x20:
                        gUnk_03000390[i] = 24 - tmp;
                        break;
                    }
                }
                for (i = 0; i <= 0x4F; i++)
                {
                    gUnk_03000390[i] = gUnk_03000390[0xA0 - i];
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
            for (j = gUnk_03000386; j < ((s16)gUnk_03000386 + 18); j++)
            {
                ((s8 *)gUnk_030004D0)[j] = (int)((float)(int)gUnk_030004D5 * gCosTable[j] - (float)(int)gUnk_030004D5 * gSinTable[j] + (float)(int)gUnk_030004D5);
            }
            if (j <= 359)
            {
                gUnk_03000386 = j;
            }
            else
            {
                gFlashFlags = (gFlashFlags & ~0x2000) | 0x1000;
                gUnk_03000386 = 0;
            }
        }
        break;
        case 2:
            gFlashFlags = (gFlashFlags & ~0x2000) | 0x1000;
            gUnk_03000386 = 0;
            break;
        }
    }
}
// @ 0x080199E0
// 淡出步进: flags=gFlashFlags; 若 flags&0x1000 按低 nibble 分派。
// case1: 4 通道循环, bits=(u8*)0x030004D7, 第 i 位为 1 时把
//   gUnk_03000390[*(vu16*)0x04000006 & 0xFF] 写入 gUnk_030004D8[i], 其 >>1 写入 gUnk_030004E8[i]。
// case2: REG_BLDY = gUnk_03000390[*(u8*)0x04000006]; 再按 flags&0xF00 设 REG_BLDCNT
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
                    *(u16 *)gUnk_030004D8[i] = gUnk_03000390[*(vu16 *)0x04000006 & 0xFF];
                    *(u16 *)gUnk_030004E8[i] = gUnk_03000390[*(vu16 *)0x04000006 & 0xFF] >> 1;
                }
            }
            break;
        case 2:
            bldy = (vu16 *)0x04000054;
            tbl = gUnk_03000390;
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
    gUnk_03000386 = 0;
    *(vu16 *)0x04000048 = 0x3F;
    *(vu16 *)0x04000040 = 0xF0;
    *(vu16 *)0x04000044 = 0x2A0;
    REG_DISPCNT |= 0x2000;
    gUnk_030004D4 = arg0;
    gUnk_030004D5 = 0;
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
INCLUDE_ASM("asm/nonmatchings", sub_8019B98);
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

    gUnk_03000386 = 0;

    gUnk_030004D4 = arg0;
    gUnk_030004D5 = arg1;
    gUnk_030004D6 = arg2;
    gUnk_030004D7 = arg3;
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

    gUnk_030004D7 = 0;
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
INCLUDE_ASM("asm/nonmatchings", sub_801A2AC);
// void sub_801A2AC(u16 arg0, u8 arg1, u8 arg2)   // ⏸ 逻辑正确(v3), 卡在寄存器分配: 见 progress.md
// {
//     REG_BLDCNT = arg0;
//     REG_BLDALPHA = arg1 | (arg2 << 8);
//     if (((arg0 >> 6) & 2) == 2)   // 目标其实是 switch(&2){case2,case3} 的 range-check 形状
//         REG_BLDY = arg1;
// }
extern u8 *gUnk_087EBDF0[];

// @ 0x0801A2EC
void sub_801A2EC(void)
{
    if (gUnk_030004F8 <= 3)
    {
        LZ77UnCompVram(gUnk_087EBDF0[gUnk_030004F8], (void *)(0x06008000 + gUnk_030004F8 * 0x1000));
        gUnk_030004F8++;
    }
}
// @ 0x0801A324
void BgLoad_Reset(void)
{
    gUnk_030004F8 = 0;
    return;
}
// @ 0x0801A330
void BgLoad_Finish(void)
{
    gUnk_030004F8 = 4;
}
// @ 0x0801A33C
u8 BgLoad_GetPos(void)
{
    return gUnk_030004F8;
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

    REG_BG0HOFS = gUnk_03000500.field_0;
    REG_BG0VOFS = gUnk_03000500.field_2;
    REG_BG1HOFS = gUnk_03000500.field_4;
    REG_BG1VOFS = gUnk_03000500.field_6;
    REG_BG2HOFS = gUnk_03000500.field_8;
    REG_BG2VOFS = gUnk_03000500.field_A;
    REG_BG3HOFS = gUnk_03000500.field_C;
    REG_BG3VOFS = gUnk_03000500.field_E;
}
typedef union
{
    u16 array[4][2];
} U0500_arr;

// @ 0x0801A3A8
void sub_801A3A8(u8 arg0, u16 arg1, u16 arg2)
{
    U0500_arr *u;

    u = (U0500_arr *)&gUnk_03000500;
    u->array[arg0][0] = arg1;
    u->array[arg0][1] = arg2;
}