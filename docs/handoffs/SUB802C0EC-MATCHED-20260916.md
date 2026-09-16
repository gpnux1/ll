# sub_802C0EC 工作包 (2026-09-16, qwen38-opencode) — 已匹配

## 机器契约
- address: 0x0802C0EC, 407 源码槽 / **932B** 机器码, module=battle_stage_actor, src=src/battle_stage_actor.c
- entry_kind: direct (战斗对象演出 handler; 与 sub_802C490 相邻, 与 sub_802B608/sub_802BB24/sub_802FE98 同族)
- 原型: `u8 sub_802C0EC(BattleObj *obj)` (单参 r0=obj→r7; 返回值走 sb, 默认 0)
- match_status: 1 (**fncheck OK** 932B, 54 池重定位已施加, 26 bl 槽忽略)
- callers/callees: 演出引擎 (sub_80444xx/BattleTask) 逐帧调用; callee: GetObjPool, sub_80187E8,
  sub_80444A4, sub_801CE80, sub_803F5B4, sub_803F658, sub_801B95C, sub_801768C, __modsi3,
  sub_8044514, Sfx_Play, Sfx_StopTrack, sub_801A348, sub_801A2AC, sub_8019B98, sub_804BDD8, sub_8020CC4, sub_804BE90
- persistent_state: 状态 PC = gObjActStep(0x03000820), 跨帧; 工作寄存器 gObjActSavedX/Y/F2A,
  gObjActStepTimer, gObjActMoveFromX, gSceneFadeOut/In, gActWaitBusy0/1/2

## 语义 (E0/E1)
战斗对象"变身/强化 + 左右往返位移 + 场景淡入淡出"多段演出状态机。`switch(gObjActStep)`
jump table 28 项, 有效 case {0,9,18,19,20,21,22,23,24}, 主干 `0 → 0x12 → … → 0x19 → 9`;
仅 case9 (等 gActWaitBusy0/1/2 全 0) 置 result=1。其余 case 1–8,10–17,25–27 落到 default 尾部。

| step | 动作 | 下一 step |
|---|---|---|
| 0x00 | gObjActSavedX=posX / SavedY=posY / SavedF2A=headA.f_1E; sub_80444A4 清同组伤害; sub_801CE80(obj,5,0x1B4,0xD,0) 切动画; sub_803F5B4 起手; StepTimer=0; f_B6=2; f_B4=0 | 0x12 |
| 0x12 | 等 headA.kindFlags 的 0x800 落; Sfx(0x5A) | 0x13 |
| 0x13 | headA.frameIdx==0x6E 时 Sfx(0x68); 等 kindFlags 到 0x1000 → Sfx_StopTrack(1) + sub_801CE80(obj,0,SavedF2A,r4,0) | 0x14 |
| 0x14 | 等 headA.kindFlags 0x800 → sub_801A348 + sub_801A2AC(0x1F47,0xF,8) + Sfx(0x64) | 0x15 |
| 0x15 | sub_8019B98(0xC,3,0xF,2)!=0 → sub_804BDD8(0xF,1,3,1,6) + StepTimer=0 + sub_8020CC4(obj,0x7D,0x91,0x27C,0xD,0x33B,5) + headB.f_2A=0 | 0x16 |
| 0x16 | 等 headB.kindFlags 0x800 | 0x17 |
| 0x17 | StepTimer<=0x31: headA.frameIdx=(idx+1)%sub_801B95C(&headA)+0x14; posX=sub_801768C(MoveFromX,0xF0-MoveFromX,0x32,t,2); t++; 超过: StepTimer=0, pad_C1=posY, posX=SavedX, state&=0xDFFF, sub_8044514(0x28) | (17) / 0x18 |
| 0x18 | StepTimer<=0x27: gSceneFadeOut=sub_801768C(0xF,-0xF,0x28,t,2); gSceneFadeIn=sub_801768C(8,8,0x28,t,2); sub_801A2AC(0x1F47,Out,In); t++; 超过: sub_801A2AC(0,0,0)+sub_804BE90(0xF,1)+REG_DISPCNT&=0xFDFF+sub_801CE80(obj,0,SavedF2A,r4,0)+headA.kindFlags|=0x100 | (18) / 0x19 |
| 0x19 | 等空闲 | — |
| 0x09 | gActWaitBusy0/1/2 全 0 → result=1 | — |

## 命名依据 (E2 多消费者)
全部符号取自既有 iwram.h/battle_object_engine.h 等公共头, 与已交付姊妹函数
**sub_802FE98 (0x0802FE98) / sub_802C490 (0x0802C490)** 共用同一套命名 (逐地址一致), 非本次新增。
- `gObjActStep/StepTimer/SavedX/SavedY/SavedF2A/MoveFromX` = 0x03000820/825/828/829/822/86E (ll.map 核对)
- `gSceneFadeOut/In` = 0x03000867/868; `gActWaitBusy0/1/2` = 0x03000844/845/856
- REG_DISPCNT 宏 (gba.h) = 0x080<<0x13, 与 target `movs r2,#0x80;lsls r2,r2,#0x13` 一致
- BattleObj 偏移核对: headA.kindFlags@0x24 / frameIdx@0x28 / f_1E@0x2A / headB.f_2A@0x66 /
  headB.kindFlags@0x54 / state@0xB0 / f_B4@0xB4 / f_B6@0xB6 / posX@0xBF / posY@0xC0 / pad_C1@0xC1

## 唯一决定形状的 lever (本次攻坚核心)
**case0 的 `movs r4,#0` 必须在 `movs r2,#0xda; lsls r2,r2,#1` (0x1B4) 之后发射。**
- 前任 (permuter/…/base.c 与 src 3047–3154 注释候选) 用**分离语句** `zero = 0;` 再接
  `sub_801CE80(obj,5,0x1B4,0xD,zero)`, agbcc -O2 把 `movs r4,#0` 提到 r2 计算**之前** →
  .text 排布错位 (实测 case0 处 movs r4 与 lsls r2 顺序反了), status 一直停 0。
- 修法: 把赋值**内联进调用实参** `sub_801CE80(obj, 5, 0x1B4, 0xD, zero = 0);`
  → GCC 在 arg4 (stack→`str r4,[sp]`) 的求值点物化 `movs r4,#0`, 恰落在 lsls r2 之后, 与 target 对齐。
- 之后 case13 / case18-else 的 `sub_801CE80(...,zero,0)` 复用同一 r4 (`adds r3,r4,#0`)。
  仅此一处即从指令流差异 (3 处 lsls/lsrs + 顺序) 降到 **0 差异**。

## bytecmp/fncheck 说明
- 单函数 **bytecmp 不可信于本函数**: 它对每个外部 `bl` 施加 abs 符号后 `arm-none-eabi-ld` 会插入
  `__sub_XXXX_from_thumb` interworking thunk, 使 linked .text 从 932 膨胀到 1224 且 bl 槽被填真实地址,
  而 target.o 不链接 bl 槽留 0 → 假性 104 字节 DIFF。这是工具对"大量外部调用"函数的固有局限。
- 改用: (1) 归一化指令流 diff = 0; (2) **fncheck** (真实 build .o, 施加 54 池重定位, 忽略 26 bl 槽) = OK。
  两者一致即字节正确。

## 剩余/风险
- 语义命名沿用同族已交付函数, 无新增结构体字段, 未触碰公共头/linker.ld/iwram.h (零跨包影响)。
- 未跑全量 make + sha1: 工作区存在其他 agent 的未完成 M 文件, 全量终验应由 owner 统一执行,
  避免把他人半成品当本包结果。本包单函数 fncheck 已达门槛。
