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

// @ 0x0804AD54
void sub_804AD54(u16 *ptr)
{
    *(ptr + 0x5B) = 0xB000;
}
extern u8 gBattleIntroCmd[];
extern u8 gBattleIntroPal[];

// @ 0x0804AD60
/* 战斗开场"演出对象装配": 把 gBattleIntroObj 这块 ObjHead 用 sub_801B81C 装配为
 * 战斗开场动画 (0xF0,0x50 起, 0x1B4 尺寸, 调色板 0xE, 命令流 gBattleIntroCmd /
 * 调色板 gBattleIntroPal), 分步装载首帧后清 kindFlags 的 0x800, 复位开场演出状态机
 * (gBattleIntroState/Timer/Phase=0), 并停止 BGM。由 battle 流程 state 7 调用。 */
void sub_804AD60(void)
{
    ObjHead *obj = (ObjHead *)gBattleIntroObj;
    u8 zero;
    u16 flags;

    sub_801B81C(obj, 0xF0, 0x50, 0xDA * 2, 0xE, gBattleIntroCmd, gBattleIntroPal, 0xA8 * 8, 1, 0x402);
    ObjGfxLoad_Step(obj);
    flags = 0xF7FF & obj->kindFlags;
    zero = 0;
    obj->kindFlags = flags;
    sub_801A684(obj);
    gBattleIntroTimer = zero;
    gBattleIntroState = zero;
    gBattleIntroPhase = zero;
    obj->f_2A = zero;
    Bgm_Stop();
}
// @ 0x0804ADE0
/* 开场 BGM 演出阶段 1: 复位状态机并置 gBattleIntroPhase=1 (进入 BGM 播放/等待段)。 */
void sub_804ADE0(void)
{
    gBattleIntroState = 0;
    gBattleIntroPhase = 1;
}
// @ 0x0804ADF8
/* 开场演出阶段 2: 启动屏幕淡入 (effect 2, 10 帧, 0x32), 复位状态机并置 gBattleIntroPhase=2。 */
void sub_804ADF8(void)
{
    ScreenFade_Start(2, 10, 0x32);
    gBattleIntroState = 0;
    gBattleIntroTimer = 0;
    gBattleIntroPhase = 2;
    gBattleIntroFadeFlag = 0;
}
// @ 0x0804AE2C
// 注: OAM 缓冲用强转常量 (非 gOamBuffer 符号) 才能让 agbcc 逐迭代重物化基址 (RULES 规则102)
#define OAM_BUF ((GameOamData *)0x030035C0)
extern u8 gObjSizeTable[];
/* 战斗转场"上/下擦除"逐帧更新 (gWipeCtl bits4-7 == 0x10 时):
 * 从 gWipeDesc 取受影响的 OAM 下标范围 [gWipeOamStart, gWipeOamEnd], 扫描这些 OAM 求出
 * 最小 VPos (gWipeMinY) 与最大 VPos+高度 (gWipeMaxY, 高度查 gObjSizeTable), 并逐项保存
 * 原始 HPos 到 gWipeSavedH; 每 5 帧把 gWipeProgress 推进 1 (遮挡高度), 当推进量接近
 * 扫描范围时关闭转场 (清 gWipeCtl bit0/1)。由 sub_801889C 逐帧调用。 */
void sub_804AE2C(void)
{
    u16 i;

    if ((gWipeCtl & 1) != 0 && (gWipeCtl & 0xF0) == 0x10)
    {
        gWipeOamStart = gWipeDesc->field_2D;
        gWipeOamEnd = gWipeDesc->field_2D - gWipeDesc->field_2E;
        gWipeMinY = 0xA0;
        gWipeMaxY = 0;
        i = gWipeOamStart;
        if (i > gWipeOamEnd)
        {
            do
            {
                if (gWipeMinY > OAM_BUF[i].fields.VPos)
                    gWipeMinY = OAM_BUF[i].fields.VPos;
                if (gWipeMaxY < gObjSizeTable[OAM_BUF[i].fields.Size + (OAM_BUF[i].fields.Shape << 2)] * 8
                        + OAM_BUF[i].fields.VPos)
                    gWipeMaxY = (u8)(gObjSizeTable[OAM_BUF[i].fields.Size + (OAM_BUF[i].fields.Shape << 2)] * 8
                        + OAM_BUF[i].fields.VPos);
                gWipeSavedH[i] = OAM_BUF[i].fields.HPos;
                i--;
            } while (i > gWipeOamEnd);
        }
        gWipeCtl |= 2;
        gWipeSubframe = (u8)((gWipeSubframe + 1) % 5);
        if (gWipeSubframe == 0)
            gWipeProgress++;
        if ((gWipeMaxY - gWipeProgress) < (gWipeMinY - 0x1E))
        {
            gWipeCtl &= ~1;
            gWipeCtl &= ~2;
        }
    }
}
#undef OAM_BUF
// @ 0x0804AF60
INCLUDE_ASM("asm/nonmatchings", sub_804AF60);
// @ 0x0804B080
INCLUDE_ASM("asm/nonmatchings", sub_804B080);
// @ 0x0804B1EC
/* 战斗转场效果复位: 清 gWipeCtl (停止并清类型)。 */
void sub_804B1EC(void)
{
    gWipeCtl = 0;
}
// @ 0x0804B1F8
/* 战斗转场效果启动: 绑定转场描述块 gWipeDesc, 置 gWipeCtl = 0x11 (运行中 + 类型 0x10),
 * 并复位子计数 gWipeSubframe / 推进量 gWipeProgress。 */
void sub_804B1F8(WipeDesc *arg0)
{
    gWipeDesc = arg0;
    gWipeCtl |= 0x11;
    gWipeSubframe = 0;
    gWipeProgress = 0;
}
extern u32 gUnk_0861AAA4[];
extern u32 gUnk_0861C764[];

// @ 0x0804B224
void sub_804B224(u16 *flags)
{
    if ((*flags & 0x80) != 0)
    {
        DmaCopy32(3, gUnk_0861AAA4, (void *)0x06012E80, 0x280);
        DmaWait(3);
        sub_804C2FC((u32)gUnk_0861C764, 0xF, 1);
        *flags &= 0xFF7F;
    }
}
// @ 0x0804B288
// 战斗动画子系统复位: 清 0x03000AE0/03000AE2/03000CE8 (u16) 与 03000AE4/03000AE5 (u8) 状态字,
// 随后用共享的 vu16 fill=0 做 4 次 DMA fill (控制字 0x81000100 = 使能+源固定+256 半字),
// 依次清 OBJ 调色板 (0x05000200)、BG 调色板 (0x05000000) 及两份镜像 0x02036AC0/0x02036CC0,
// 每次 fill 后 DmaWait。最后把两张 16 项调色板动画表 (gBgPalAnim/gObjPalAnim) 的每一项
// ctrl/palSlot |= 0xFF、period/counter/span/dir 清零、frameIdx (u16) 清零。
// 形状要点 (GCC2.9): ①fill 必须是 vu16 且复用同一栈槽; ②DmaSet 用宏展开 (局部 dmaRegs)
// 才能每个 fill 重新装载 0x040000D4; ③循环内先用 u8* 中间量 pA 锚定 gBgPalAnim 的池装载
// 位置, 再转 entry 指针, 否则该 ldr 会被提升到首个 DmaWait 之前。
INCLUDE_ASM("asm/nonmatchings", sub_804B288);
// @ 0x0804B3C0
/* opcode1 流式调色板动画一帧: 由 sub_804C45C/sub_804C6B0 的逐帧调度 (ctrl&0xF==1) 调用。
 * 先按 ctrl bit4 (0x10) 决定往返方向: 置位则 counter++ 到 period-1 后清 bit4, 否则 counter--
 * 到 0 后置 bit4。再把 period 右移折成插值移位量 entry->shift (每步 +1, 上限 7), 最后调
 * sub_804B56C 从镜像 (dest) 取 16 色, 用 dR/dG/dB 按 weight=(period-counter) 插值写 VRAM (src)。
 * 形状要点 (GCC2.9):
 *  ① flags 不能存变量 (读 entry->ctrl 直接参与测试/掩码), 否则 flags 抢 r3、count 退 r5, 与
 *     目标 (flags=r5, count=r3) 相反 —— 全局分配优先级差 (经验 245)。
 *  ② 掩码须 `int mask = ~0x10;` 变量, 否则常量被窄化成 movs#0xEF。
 *  ③ 循环变量必须 `unsigned short v` (非 u8/u16-int), 且 `v = entry->period; v >>= 1;` 分开写,
 *     才能在循环内保住 `(u8)(v>>1)` 的截断临时 (r0); 直接 `v = entry->period >> 1` 会让 GCC
 *     折叠成 in-place lsrs, 差 2 指令。
 *  ④ 指针算术按**字节偏移** (<<5 = 16色×2), 故把 src/dest 强转 u8* 再加偏移, 否则 u16* 会
 *     再 ×2 出 <<6。 */
void sub_804B3C0(PaletteAnimEntry *entry, u8 slot, u16 *src, u16 *dest)
{
    int mask = ~0x10;
    unsigned short v;

    if (entry->ctrl & 0x10)
    {
        entry->counter += 1;
        if (entry->counter >= entry->period - 1)
            entry->ctrl = entry->ctrl & mask;
    }
    else
    {
        entry->counter -= 1;
        if ((u8)entry->counter == 0)
            entry->ctrl = entry->ctrl | 0x10;
    }
    entry->shift = 0;
    v = entry->period;
    v >>= 1;
    if (v != 0)
    {
        do
        {
            entry->shift += 1;
            if ((u8)entry->shift > 7)
                break;
            v = (u8)(v >> 1);
        } while (v != 0);
    }
    sub_804B56C((u16 *)((u8 *)src + ((s8)entry->palSlot << 5)), (u16 *)((u8 *)dest + (slot << 5)),
                (u8)(entry->period - entry->counter), &entry->dR);
}
// @ 0x0804B458
void sub_804B458(PaletteAnimEntry *entry, u8 slot, u16 *src, u16 *dest)
{
    u8 width;
    u8 frames;

    entry->counter = (entry->counter + 1) % entry->period;
    if (entry->counter == 0)
    {
        width = entry->span & 0xF;
        frames = entry->span >> 4;
        if (entry->dir == 0)
            entry->frameIdx = (u8)((*(u8 *)&entry->frameIdx + 1) % frames);
        else if (entry->frameIdx == 0)
            entry->frameIdx = frames - 1;
        else
            entry->frameIdx--;
        sub_804C2A0(src + ((s8)entry->palSlot << 4), dest + (slot << 4), width, frames, entry->frameIdx);
    }
}
// @ 0x0804B4D0
/* opcode3 淡变调色板动画一帧: 由 sub_804C45C/sub_804C6B0 的逐帧调度 (ctrl&0xF==3) 调用。
 * 先把 period 折叠成插值移位量 entry->shift (对 period 连续右移、每步 +1, 上限 7),
 * 再调 sub_804B56C 从镜像 (dest) 取 16 色, 用 entry 的 dR/dG/dB 按 weight=(period-counter)
 * 和 shift 插值写入当前 BG/OBJ 调色板 (src); 随后推进 counter:
 *   ctrl bit6=0: 单向递减 counter, 减到 period-dir 为止;
 *   ctrl bit6=1: 递增 counter, 到 period 后按目标缓冲 (BG 镜像 0x02036AC0 / OBJ 镜像
 *                0x02036CC0) 调 sub_804BB64/sub_804C10C 结束该槽的淡变。
 * 形状要点 (GCC2.9):
 *  ① shift 计算必须写成 `if ((v >>= 1) != 0) do {...} while ((v >>= 1) != 0)`
 *     (= 顶测 while 被 expand_end_loop 滚到循环尾的形态, 前置 b 不能少), 且 period 缓存进
 *     局部 u8 v; 直接对 entry->period 做 >>= 会重读内存, v 不缓存则前置分支消失 (差 2 字节)。
 *  ② shift 字段必须按**裸字节**访问 `((u8 *)entry)[0xF]`, 写成 `entry->shift` 会让 GCC2 放弃
 *     上述循环翻转 (变成顶测 while, 差 ~1400 分); 结构体成员形式对其它字段无碍。
 *  ③ 指针按**字节**偏移: `(u8 *)src + (palSlot << 5)` (16色×2=32), 直接用 u16* 加会把偏移再
 *     ×2 出 <<6 且丢失 (s8)palSlot 符号扩展语义。 */
void sub_804B4D0(PaletteAnimEntry *entry, u8 slot, u16 *src, u16 *dest)
{
    u8 v = entry->period;

    ((u8 *)entry)[0xF] = 0;
    if ((v >>= 1) != 0)
    {
        do
        {
            if (++((u8 *)entry)[0xF] > 7)
                break;
        } while ((v >>= 1) != 0);
    }
    sub_804B56C((u16 *)((u8 *)src + ((s8)entry->palSlot << 5)), (u16 *)((u8 *)dest + (slot << 5)),
                (u8)(entry->period - entry->counter), &entry->dR);
    if (!(entry->ctrl & 0x40))
    {
        if (entry->counter > entry->period - entry->dir)
            entry->counter--;
    }
    else
    {
        if (entry->counter < entry->period)
            entry->counter++;
        else if (dest == (u16 *)0x02036AC0)
            sub_804BB64(slot, 1);
        else if (dest == (u16 *)0x02036CC0)
            sub_804C10C(slot, 1);
    }
}
// @ 0x0804B56C
INCLUDE_ASM("asm/nonmatchings", sub_804B56C);
// @ 0x0804B654
INCLUDE_ASM("asm/nonmatchings", sub_804B654);
// @ 0x0804B7B0
/* 停止 BG 调色板动画槽 [arg0, arg0+arg1): 与 sub_804B8E8 同表 (gBgPalAnim 0x03000AE8)
 * 同逻辑, 但表项访问为原始字节指针 (entry[0..3])。对每个非空 (ctrl != -1) 条目, 若未禁止
 * 颜色重置 (ctrl bit5=0x20) 则注销调色板槽 (sub_804C3A4), 调 sub_804C420 刷新该槽, 然后把
 * ctrl/palSlot 置 0xFF、period/counter 清 0。
 * 形状要点: 必须用 u8* entry 而非 PaletteAnimEntry* (结构体形式差 99B 寄存器分配);
 * "空" 判断经 u32 v=*(s8*)&entry[0] 与函数作用域 int empty=-1 比较 (令 -1 在循环内物化,
 * base 进 sl, 破解 sl/r8 分配, 同 sub_804B8E8/sub_804BD54); `v=0x20; v&=flags;` 复用 u32
 * v 以得 ands r0,r1; mask=0xFF 经 u8 临时 temp|=mask 写入 ctrl/palSlot。 */
void sub_804B7B0(u8 arg0, u8 arg1)
{
    u8 i;
    int empty = -1;
    u8 *entry;

    for (i = 0; i < arg1; i++)
    {
        u8 *base = (u8 *)gBgPalAnim;
        u8 mask = 0xFF;
        entry = base + (arg0 + i) * 16;
        {
            u8 temp;
            u8 flags = entry[0];
            u32 v = *(s8 *)&entry[0];
            if (v == empty)
                continue;
            v = 0x20;
            v &= flags;
            if (v == 0)
                sub_804C3A4(entry[1], 1);
            sub_804C420(arg0 + i);
            temp = entry[0];
            temp |= mask;
            entry[0] = temp;
            temp = entry[1];
            temp |= mask;
            entry[1] = temp;
            entry[2] = 0;
            entry[3] = 0;
        }
    }
}
// @ 0x0804B834
INCLUDE_ASM("asm/matchings", sub_804B834);
/* 建立 BG 调色板淡变槽 [arg0, arg0+arg1): 对每个未在淡变中的槽 (ctrl&0xF != 2) 调用
 * sub_804C3E4 备份当前 BG 调色板, 然后装配一个"淡变"条目 ——
 *   ctrl=0x22 (opcode=2 淡变 + bit5 禁止颜色重置), palSlot=槽号, period=arg2, counter=0,
 *   span=(arg4<<4)|(|arg3|&0xF), frameIdx=0, dir=arg3>>7 (arg3 为带符号方向, 负数取绝对值
 *   放低 4 位)。返回表中 arg0 槽的 palSlot, 供调用者作为"目标槽"使用。
 * 由 sub_80285A0 等演出状态机在启动槽位淡变时调用 (对应 sub_804B8E8 释放槽)。
 * 形状要点 (GCC2.9): ①表项访问必须用 u8* 原始字节 (结构体形式差 99B, 同 sub_804B7B0);
 * ②源用 do-while + 预置 i=0/if(i<arg1) 展开, 令 arg0→sl/arg1→r9 并让 abs 复用 arg3 的 r3;
 * ③span 低4位掩码先算成 u32 m 再 `(arg4<<4)|m`; ④返回值必须经 `u8 *slot=gUnk+off; slot[1]`
 * (直接 `gUnk[off+1]` 会把 +1 单独相加, 差 2 指令/6B)。
 * 注意 (2026-09-13): arg3 是有符号方向, 但本函数体内只用 `entry[8] = arg3 >> 7` 与
 * `t = arg3; if (t < 0) abs = -t;`, 声明成 s8 会改变调用方物化 (见 code_0.h 原型说明)。 */
#if 0
s8 sub_804B834(u8 arg0, u8 arg1, u8 arg2, s8 arg3, u8 arg4)
{
    u8 i;
    u32 idx;
    u32 base;
    u8 abs;
    u32 m;
    u8 zero;
    s8 t;
    u32 off;
    u8 *entry;
    u8 *slot;

    t = arg3;
    if (t < 0)
        abs = -t;
    else
        abs = t;
    i = 0;
    off = arg0 * 16;
    if (i < arg1)
    {
        zero = 0;
        base = (u32)gBgPalAnim;
        do
        {
            idx = arg0 + i;
            entry = (u8 *)(idx * 16 + base);
            if ((entry[0] & 0xF) != 2)
            {
                sub_804C3E4((u8)idx);
                entry[0] = 0x22;
                entry[1] = idx;
                entry[2] = arg2;
                entry[3] = zero;
                m = abs & 0xF;
                entry[4] = (arg4 << 4) | m;
                *(u16 *)(entry + 6) = zero;
                entry[8] = arg3 >> 7;
            }
            i++;
        } while (i < arg1);
    }
    slot = (u8 *)gBgPalAnim + off;
    return (s8)slot[1];
}
#endif
// @ 0x0804B8E8
/* 停止 BG 调色板动画槽 [arg0, arg0+arg1): 对每个非空 (ctrl != 0xFF) 且 opcode==3 (淡变)
 * 的条目, 若未禁止颜色重置 (ctrl bit5=0x20) 则注销其占用的调色板槽 (sub_804C3A4), 调
 * sub_804C420 刷新该槽的调色板, 然后把 ctrl/palSlot 置 0xFF (标记为空), period/counter 清 0。
 * 由 sub_80285A0 等状态机在演出收尾时调用 (释放 sub_804B834 建立的槽)。
 * 形状要点: 用 PaletteAnimEntry 成员访问; "空" 判断写成 `*(s8 *)&entry->ctrl == -1`
 * (而非先取 flags/v 临时再比), 否则 GCC2.9 会把 flags/v 分到 r1/r2 从而顶掉 arg0 临时寄存器,
 * 与目标差 ~12 字节 (已穷举)。同型: sub_804BD54/sub_804BE90 (OBJ 表)。 */
void sub_804B8E8(u8 arg0, u8 arg1)
{
    u8 i;
    PaletteAnimEntry *entry;

    for (i = 0; i < arg1; i++)
    {
        PaletteAnimEntry *base = gBgPalAnim;
        entry = &base[arg0 + i];
        {
            if (*(s8 *)&entry->ctrl == -1)
                continue;
            if (!(entry->ctrl & 0x20))
                sub_804C3A4(entry->palSlot, 1);
            sub_804C420(arg0 + i);
            entry->ctrl |= 0xFF;
            entry->palSlot |= 0xFF;
            entry->period = 0;
            entry->counter = 0;
        }
    }
}
// @ 0x0804B96C
INCLUDE_ASM("asm/nonmatchings", sub_804B96C);
// @ 0x0804BB64
/* 停止 BG 调色板动画槽 [start, start+count): 与 sub_804B8E8 逻辑相同但用 do-while 展开,
 * 且 index 为 u32。对每个 opcode==3 的条目注销调色板槽 (sub_804C3A4/sub_804C420) 并标记为空。 */
void sub_804BB64(u8 start, u8 count)
{
    u8 i;
    u32 index;
    PaletteAnimEntry *base;
    PaletteAnimEntry *entry;

    i = 0;
    if (i < count)
    {
        do
        {
            base = gBgPalAnim;
            index = start + i;
            entry = base + index;
            if ((entry->ctrl & 0xF) == 3)
            {
                if ((entry->ctrl & 0x20) == 0)
                    sub_804C3A4(entry->palSlot, 1);
                sub_804C420((u8)index);
                entry->ctrl |= 0xFF;
                entry->palSlot |= 0xFF;
                entry->period = 0;
                entry->counter = 0;
            }
            i++;
        } while (i < count);
    }
}
// @ 0x0804BBDC
INCLUDE_ASM("asm/nonmatchings", sub_804BBDC);
// @ 0x0804BD54
void sub_804BD54(u8 arg0, u8 arg1)
{
    u8 i;
    PaletteAnimEntry *entry;

    for (i = 0; i < arg1; i++)
    {
        PaletteAnimEntry *base = gObjPalAnim;
        entry = &base[arg0 + i];
        {
            if (*(s8 *)&entry->ctrl == -1)
                continue;
            if (!(entry->ctrl & 0x20))
                sub_804C5F8(entry->palSlot, 1);
            sub_804C674(arg0 + i);
            entry->ctrl |= 0xFF;
            entry->palSlot |= 0xFF;
            entry->period = 0;
            entry->counter = 0;
        }
    }
}
// @ 0x0804BDD8
INCLUDE_ASM("asm/matchings", sub_804BDD8);
/* 建立 OBJ 调色板淡变槽 [arg0, arg0+arg1): 对每个非空槽 (ctrl&0xF != 2, 即非进行中)
 * 调用 sub_804C638 备份当前 OBJ 调色板, 然后装配一个"淡变"条目 ——
 *   ctrl=0x22 (opcode=2 淡变 + 0x20 禁止颜色重置), palSlot=槽号,
 *   period=arg2, counter=0, span=(arg4<<4)|(|arg3|&0xF), frameIdx=0,
 *   dir=arg3>>7 (arg3 为带符号方向, 负数取绝对值放低4位)。
 * 返回 BG 调色板表中同槽的 palSlot, 供调用者作为"目标槽"使用。
 * 由 sub_80285A0 等演出状态机在启动槽位淡变时调用。
 * 注意 (2026-09-13): arg3 为有符号方向, 声明成 s8 会改变调用方物化 (见 code_0.h 原型)。 */
#if 0
s8 sub_804BDD8(u8 arg0, u8 arg1, u8 arg2, s8 arg3, u8 arg4)
{
    u8 i;
    u32 idx;
    u32 base;
    u8 abs;
    u32 m;
    u8 zero;
    s8 t;
    PaletteAnimEntry *entry;

    t = arg3;
    if (t < 0)
        abs = -t;
    else
        abs = t;
    i = 0;
    if (i < arg1)
    {
        zero = 0;
        base = (u32)gObjPalAnim;
        do
        {
            idx = arg0 + i;
            entry = (PaletteAnimEntry *)(idx * 16 + base);
            if ((entry->ctrl & 0xF) != 2)
            {
                sub_804C638((u8)idx);
                entry->ctrl = 0x22;
                entry->palSlot = idx;
                entry->period = arg2;
                entry->counter = zero;
                m = abs & 0xF;
                entry->span = (arg4 << 4) | m;
                entry->frameIdx = zero;
                entry->dir = arg3 >> 7;
            }
            i++;
        } while (i < arg1);
    }
    return gBgPalAnim[arg0].palSlot;
}
#endif
// @ 0x0804BE90
void sub_804BE90(u8 arg0, u8 arg1)
{
    u8 i;
    PaletteAnimEntry *entry;

    for (i = 0; i < arg1; i++)
    {
        PaletteAnimEntry *base = gObjPalAnim;
        entry = &base[arg0 + i];
        {
            if (*(s8 *)&entry->ctrl == -1)
                continue;
            if (!(entry->ctrl & 0x20))
                sub_804C5F8(entry->palSlot, 1);
            sub_804C674(arg0 + i);
            entry->ctrl |= 0xFF;
            entry->palSlot |= 0xFF;
            entry->period = 0;
            entry->counter = 0;
        }
    }
}
// @ 0x0804BF14
INCLUDE_ASM("asm/nonmatchings", sub_804BF14);
// @ 0x0804C10C
void sub_804C10C(u8 start, u8 count)
{
    u8 i;
    u32 index;
    PaletteAnimEntry *base;
    PaletteAnimEntry *entry;

    i = 0;
    if (i < count)
    {
        do
        {
            base = gObjPalAnim;
            index = start + i;
            entry = base + index;
            if ((entry->ctrl & 0xF) == 3)
            {
                if ((entry->ctrl & 0x20) == 0)
                    sub_804C5F8(entry->palSlot, 1);
                sub_804C674((u8)index);
                entry->ctrl |= 0xFF;
                entry->palSlot |= 0xFF;
                entry->period = 0;
                entry->counter = 0;
            }
            i++;
        } while (i < count);
    }
}
// @ 0x0804C184
void sub_804C184(void)
{
    sub_804C45C();
    sub_804C6B0();
}

// @ 0x0804C194
void *sub_804C194(u8 arg0)
{

    switch (arg0)
    {
        case 0:
            return (void *)0x03000AE8;
        case 1:
            return (void *)0x03000BE8;
    }
    // No return?
}

// @ 0x0804C1B4
void sub_804C1B4(u8 arg0, u8 arg1, u8 arg2)
{
    switch (arg0)
    {
        case 0:
            sub_804C364(arg1, arg2);
            break;
        case 1:
            sub_804C5B8(arg1, arg2);
            break;
    }
}

// @ 0x0804C1E4
void sub_804C1E4(u8 arg0, u8 arg1, u8 arg2)
{
    switch (arg0)
    {
        case 0:
            sub_804C3A4(arg1, arg2);
            break;
        case 1:
            sub_804C5F8(arg1, arg2);
            break;
    }
}
// @ 0x0804C214
u8 sub_804C214(u8 arg0, u8 arg1)
{

    u8 ret = 0;

    switch (arg0)
    {
        case 0:
            if ((gUnk_03000AE0 >> arg1) & 1)
            {
                ret = 1;
            }
            break;
        case 1:
            if ((gUnk_03000AE2 >> arg1) & 1)
            {
                ret = 1;
            }
            break;
    }

    return ret;
}
// @ 0x0804C250
void sub_804C250(u8 arg0, u8 arg1)
{
    switch (arg0)
    {
        case 0:
            sub_804C3E4(arg1);
            break;
        case 1:
            sub_804C638(arg1);
            break;
    }
}
// @ 0x0804C278
void sub_804C278(u8 arg0, u8 arg1)
{
    switch (arg0)
    {
        case 0:
            sub_804C420(arg1);
            break;
        case 1:
            sub_804C674(arg1);
            break;
    }
}

// @ 0x0804C2A0
void sub_804C2A0(u16 *arg0, u16 *arg1, u8 arg2, u8 arg3, u8 arg4)
{
    u8 i;

    for (i = 0; i < arg3; i++)
    {
        arg0[i + arg2] = arg1[arg4 + arg2];

        arg4 = (arg4 + 1) % arg3;
    }
}
// @ 0x0804C2F0
u16 sub_804C2F0(void)
{
    return gUnk_03000AE0;
}
// @ 0x0804C2FC
void sub_804C2FC(u32 arg0, u8 arg1, u8 arg2)
{
    u8 i;

    DmaCopy16(3, arg0, 0x5000200 + (arg1 << 5), arg2 * 0x20);
    DmaWait(3);

    for (i = 0; i < arg2; i++)
    {
        if (!((gUnk_03000AE0 >> (arg1 + i)) & 1))
        {
            gUnk_03000AE0 |= (1 << (arg1 + i));
        }
    }
}
// @ 0x0804C364
void sub_804C364(u8 arg0, u8 arg1)
{
    u8 i;

    for (i = 0; i < arg1; i++)
    {
        if (!((gUnk_03000AE0 >> (arg0 + i)) & 1))
        {
            gUnk_03000AE0 |= (1 << (arg0 + i));
        }
    }
}
// @ 0x0804C3A4
void sub_804C3A4(u8 arg0, u8 arg1)
{
    u8 i;
    for (i = 0; i < arg1; i++)
    {
        if (((gUnk_03000AE0 >> (arg0 + i)) & 1))
        {
            gUnk_03000AE0 &= ~(1 << (arg0 + i));
        }
    }
}

// @ 0x0804C3E4
void sub_804C3E4(u8 arg0)
{
    DmaCopy16(3, 0x05000200 + (arg0 << 5), 0x02036AC0 + (arg0 << 5), 0x20);
    DmaWait(3);
}

// @ 0x0804C420
void sub_804C420(u8 arg0)
{
    DmaCopy32(3, 0x02036AC0 + (arg0 << 5), 0x05000200 + (arg0 << 5), 32);
    DmaWait(3);
}

// @ 0x0804C45C
void sub_804C45C(void)
{
    u8 i;
    PaletteAnimEntry *entry;

    for (i = 0; i <= 15; i++)
    {
        entry = &gBgPalAnim[i];
        switch (entry->ctrl & 0xF)
        {
            case 1:
                sub_804B3C0(entry, i, 0x05000200, 0x02036AC0);
                break;
            case 2:
                sub_804B458(entry, i, (u16 *)0x05000200, (u16 *)0x02036AC0);
                break;
            case 3:
                sub_804B4D0(entry, i, 0x05000200, 0x02036AC0);
                break;
        }
    }
}
/* sub_804C4D8/sub_804C728 用 PaletteAnimEntry 结构体成员形式访问 (iwram.h)。
 * 必须用结构体成员形式: 写成 `u8 *ptr; ptr[0] |= 0x40;` 时 GCC2 会把 IOR 的
 * 目的寄存器选成常量那个 (`mov r0, ip; orrs r0, r1`), 而目标是
 * `adds r0, r1, #0; orrs r0, r7` (先拷 b 再或常量)。见规则 11 / 67。 */

// @ 0x0804C4D8
void sub_804C4D8(u8 arg0, u8 arg1, u8 arg2)
{
    u8 i;
    PaletteAnimEntry *entry;

    for (i = 0; i < arg1; i++)
    {
        entry = &gBgPalAnim[arg0 + i];
        if ((entry->ctrl & 0xF) == 3)
        {
            entry->ctrl |= 0x40;
            entry->period = arg2;
            entry->counter = 0;
        }
    }
}

// @ 0x0804C53C
u16 sub_804C53C(void)
{
    return gUnk_03000AE2;
}
// @ 0x0804C548
void sub_804C548(u32 src, u8 slot, u8 count)
{
    u8 i;

    DmaCopy32(3, src, 0x05000000 + (slot << 5), count * 0x20);
    DmaWait(3);
    for (i = 0; i < count; i++)
    {
        if (((gUnk_03000AE2 >> (slot + i)) & 1) == 0)
            gUnk_03000AE2 |= 1 << (slot + i);
    }
}
// @ 0x0804C5B8
void sub_804C5B8(u8 arg0, u8 arg1)
{
    u8 i;

    for (i = 0; i < arg1; i++)
    {
        if (!((gUnk_03000AE2 >> (arg0 + i)) & 1))
        {
            gUnk_03000AE2 |= (1 << (arg0 + i));
        }
    }
}

// @ 0x0804C5F8
void sub_804C5F8(u8 arg0, u8 arg1)
{
    u8 i;

    for (i = 0; i < arg1; i++)
    {
        if (((gUnk_03000AE2 >> (arg0 + i)) & 1))
        {
            gUnk_03000AE2 &= ~(1 << (arg0 + i));
        }
    }
}
// @ 0x0804C638
void sub_804C638(u8 arg0)
{
    DmaCopy32(3, 0x05000000 + (arg0 << 5), 0x02036CC0 + (arg0 << 5), 32);
    DmaWait(3);
}
// @ 0x0804C674
void sub_804C674(u8 arg0)
{
    DmaCopy32(3, 0x02036CC0 + (arg0 << 5), 0x05000000 + (arg0 << 5), 32);
    DmaWait(3);
}
// @ 0x0804C6B0
void sub_804C6B0(void)
{
    u8 i;
    PaletteAnimEntry *entry;

    for (i = 0; i <= 15; i++)
    {
        entry = &gObjPalAnim[i];
        switch (entry->ctrl & 0xF)
        {
            case 1:
                sub_804B3C0(entry, i, 0x05000000, 0x02036CC0);
                break;
            case 2:
                sub_804B458(entry, i, (u16 *)0x05000000, (u16 *)0x02036CC0);
                break;
            case 3:
                sub_804B4D0(entry, i, 0x05000000, 0x02036CC0);
                break;
        }
    }
}
// @ 0x0804C728
void sub_804C728(u8 arg0, u8 arg1, u8 arg2)
{
    u8 i;
    PaletteAnimEntry *entry;

    for (i = 0; i < arg1; i++)
    {
        entry = &gObjPalAnim[arg0 + i];
        if ((entry->ctrl & 0xF) == 3)
        {
            entry->ctrl |= 0x40;
            entry->period = arg2;
            entry->counter = 0;
        }
    }
}
// @ 0x0804C78C
void sub_804C78C(void)
{
    u8 values[16];
    u8 count;
    u8 i;
    BattleObj *obj;
    BattleObj *pool;

    sub_804DE8C();
    pool = (BattleObj *)GetObjPool();
    count = sub_80489E8(pool, values, 0, 0x43);
    for (i = 0; i < count; i++)
    {
        obj = &pool[values[i]];
        if (sub_8045F10(obj, 0x20) == 1)
        {
            switch (obj->slot)
            {
                case 0:
                case 1:
                    ((void (*)(BattleObj *, u8))sub_804CA2C)(obj, values[i]);
                    break;
                case 2:
                    ((void (*)(BattleObj *, u8))sub_804CAA0)(obj, values[i]);
                    break;
                case 3:
                    ((void (*)(BattleObj *, u8))sub_804CB18)(obj, values[i]);
                    break;
                case 4:
                    ((void (*)(BattleObj *, u8))sub_804CB8C)(obj, values[i]);
                    break;
                case 5:
                    ((void (*)(BattleObj *, u8))sub_804CC00)(obj, values[i]);
                    break;
                case 6:
                    ((void (*)(BattleObj *, u8))sub_804CC78)(obj, values[i]);
                    break;
                case 7:
                    ((void (*)(BattleObj *, u8))sub_804CCEC)(obj, values[i]);
                    break;
                case 8:
                    ((void (*)(BattleObj *, u8))sub_804CD60)(obj, values[i]);
                    break;
                case 9:
                    ((void (*)(BattleObj *, u8))sub_804CDD4)(obj, values[i]);
                    break;
                case 10:
                    ((void (*)(BattleObj *, u8))sub_804CE48)(obj, values[i]);
                    break;
            }
        }
    }
    sub_804EF50();
}
// @ 0x0804C890
/* 战斗对象"随机指定同伴目标" (道具/背包延迟写入的调用点)。
 * obj = 对象池基址 (GetObjPool(), 5 槽 × 0xC8)。
 * 流程:
 *   1. sub_804DE8C() 初始化背包延迟写入工作表 (gInvPageDeltas 快照当前页道具数量);
 *   2. 遍历池槽 0..4, 对 sub_8045F10(o, 0x20) == 2 的对象 (存活的我方战斗对象):
 *      sub_804C8E0(obj, i) 从其余对象中随机挑一个槽号 (排除自身), 写入 o->f_BD
 *      (目标对象池索引), 并把 o->fxKind 清 0;
 *   3. sub_804EF50() 把工作表中 itemId>0xDC 的真实道具数量写回背包数量表 gInventory。
 * 调用者: sub_802151C / sub_802192C 在战斗对象入场/行动分配前调用。 */
void sub_804C890(BattleObj *obj)
{
    u8 i;

    sub_804DE8C();
    for (i = 0; i <= 4; i++)
    {
        BattleObj *o = &obj[i];
        if (sub_8045F10(o, 0x20) == 2)
        {
            u8 r;
            u8 t = i;
            u8 *p;

            Rng_LcgNext();
            r = sub_804C8E0(obj, t);
            p = &o->f_BD;
            t = 0;
            *p = r;
            o->fxKind = t;
        }
    }
    sub_804EF50();
}
// @ 0x0804C8E0
u8 sub_804C8E0(BattleObj *obj, u8 arg1)
{
    u8 values[8];
    u8 count;
    u8 i;
    u8 j;
    u8 slot;

    slot = 0;
    count = sub_80489E8(obj, values, 0, 0x7F);
    for (i = 0; i < count; i++)
    {
        if (values[i] == arg1)
        {
            for (j = i; j < count - 1; j++)
                values[j] = values[j + 1];
            count--;
            break;
        }
    }
    if (count == 0)
    {
        if (slot == 0)
            slot = 1;
        else
            slot = 0;
        count = sub_80489E8(obj, values, slot, 0x6F);
        for (i = 0; i < count; i++)
        {
            if (values[i] == arg1)
            {
                for (j = i; j < count - 1; j++)
                    values[j] = values[j + 1];
                count--;
                break;
            }
        }
    }
    return values[((s32 (*)(void))Rng_LcgNext)() % count];
}
// @ 0x0804C9B4
void sub_804C9B4(void)
{
    u8 values[8];
    u8 count;
    u8 i;
    u8 value;
    BattleObj *pool;
    BattleObj *obj;

    pool = (BattleObj *)GetObjPool();
    count = sub_80489E8(pool, values, 0, 0x7F);
    for (i = 0; i < count; i++)
    {
        obj = &pool[values[i]];
        if (obj->slot == 9)
        {
            obj->fxKind = 0;
            pool = (BattleObj *)GetObjPool();
            count = sub_80489E8(pool, values, 1, 0x7F);
            value = values[((s32 (*)(void))Rng_LcgNext)() % count];
            obj->f_BD = value;
            break;
        }
    }
}
// @ 0x0804CA2C
void sub_804CA2C(BattleObj *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8(GetObjPool(), values, 1, 0x7F);
    obj->fxKind = 0;
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}

// @ 0x0804CAA0
void sub_804CAA0(BattleObj *obj)
{
    u8 values[24];
    u8 count;
    u8 value;
    BattleObj *base;

    base = (BattleObj *)GetObjPool();
    obj->fxKind = 0;
    count = sub_80489E8(base, values, 1, 0x17F);
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}

// @ 0x0804CB18
void sub_804CB18(BattleObj *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8((BattleObj *)GetObjPool(), values, 1, 0x7F);
    obj->fxKind = 0;
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}

// @ 0x0804CB8C
void sub_804CB8C(BattleObj *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8((BattleObj *)GetObjPool(), values, 1, 0x7F);
    obj->fxKind = 0;
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}

// @ 0x0804CC00
void sub_804CC00(BattleObj *obj)
{
    u8 values[24];
    u8 count;
    u8 value;
    BattleObj *base;

    base = (BattleObj *)GetObjPool();
    obj->fxKind = 0;
    count = sub_80489E8(base, values, 1, 0x17F);
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}

// @ 0x0804CC78
void sub_804CC78(BattleObj *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8((BattleObj *)GetObjPool(), values, 1, 0x7F);
    obj->fxKind = 0;
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}

// @ 0x0804CCEC
void sub_804CCEC(BattleObj *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8((BattleObj *)GetObjPool(), values, 1, 0x7F);
    obj->fxKind = 0;
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}

// @ 0x0804CD60
void sub_804CD60(BattleObj *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8((BattleObj *)GetObjPool(), values, 1, 0x7F);
    obj->fxKind = 0;
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}

// @ 0x0804CDD4
void sub_804CDD4(BattleObj *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8((BattleObj *)GetObjPool(), values, 1, 0x7F);
    obj->fxKind = 0;
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}

// @ 0x0804CE48
void sub_804CE48(BattleObj *obj)
{
    u8 values[16];
    u8 count;
    u8 value;

    count = sub_80489E8((BattleObj *)GetObjPool(), values, 1, 0x7F);
    obj->fxKind = 0;
    if ((s8)gObjTargetCache[obj->slot] < 0)
    {
        value = values[((s32 (*)(void))Rng_LcgNext)() % count];
        obj->f_BD = value;
        gObjTargetCache[obj->slot] = values[((s32 (*)(void))Rng_LcgNext)() % count];
    }
    else
    {
        obj->f_BD = gObjTargetCache[obj->slot];
    }
}
// @ 0x0804CEBC
void sub_804CEBC(void)
{
    u8 i;
    u8 *ptr;

    for (i = 0; i <= 10; i++)
    {
        gObjTargetCache[i] = 0xFF;
    }
}
// @ 0x0804CEE0
INCLUDE_ASM("asm/nonmatchings", sub_804CEE0);
// @ 0x0804D0F8
void sub_804D0F8(BattleObj *obj)
{
    u8 values[8];
    u8 count = 0;
    u8 i;
    u8 j;
    BattleObj *pool;

    if (*(u32 *)(obj->animPtr + 0x1C) == 0)
    {
        obj->fxKind = 0;
        pool = (BattleObj *)GetObjPool();
        count = sub_80489E8(pool, values, 1, 0x6F);
        if (count <= 1)
        {
            count = sub_80489E8(pool, values, 0, 0x6F);
        }
        for (i = 0; i < count; i++)
        {
            if (pool[values[i]].pad_AC[0] == obj->pad_AC[0])
            {
                for (j = i; j < count - 1; j++)
                    values[j] = values[j + 1];
                count--;
                break;
            }
        }
        obj->f_BD = values[(u32)(u8)Rng_LcgNext() % count];
    }
    else
    {
        obj->fxKind = 3;
    }
}
