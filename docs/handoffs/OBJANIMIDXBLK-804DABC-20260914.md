# 类型分析 handoff: Unk_804DABC_Ptr → ObjAnimIdxBlk (2026-09-14, agent=zcode-rename-804dabc)

## 任务
分析 `Unk_804DABC_Ptr`（src/battle_rewards.c 局部 typedef）的数据类型语义并合理重命名。

## 改名结果
- `Unk_804DABC_Ptr` → **`ObjAnimIdxBlk`**（BattleObj.animPtr 动画索引块视图；与并行会话已命名的 `ObjAnimEntry`（gUnk_08393B28 表项类型, code_0.h）同族）。
- 成员 `field_8[4]` → **`subIdx[4]`**（动画副索引表，即 `obj->animSubIdx` 所索引的表）。
- 布局与访问形状完全不变：`{u8 pad_0[8]; u16 subIdx[4];}`，+8 处 u16 数组成员形式保留
  （glm-batch 历史注记：若 +8 被折进 ldrh 位移会破坏字节）。

## 证据链（类型语义）

**已验证事实 (E0/E1)：**
1. 消费形式：`entry = &gUnk_08393B28[((ObjAnimIdxBlk *)(obj->animPtr))->subIdx[obj->animSubIdx]]`
   （battle_rewards.c 内 8 处 + sub_804DABC 经 `*(u32*)(obj+0x88)` 的裸偏移 1 处）。
2. `animPtr` 数据块来源（E1, asm/nonmatchings/sub_802031C.s:11-16）：
   slot≥0x71 特殊对象 `animPtr = &gUnk_0839ABCC[field_BE*0x40]`（0x40 字节/项）。
3. ROM 数据验证（E0, baserom @0x0839C80C 起 = 表内 field_BE=0x71 项）：
   +0/+2/+4/+6/+8..+0xE 均为小整数（如 0x71 项: 693,694,698,697; +8..E={695,695,695,695}），
   全部落在 gUnk_08393B28 的 ~992 项范围内；对照 +0x10 起为 0/1/数值参数，形状截然不同。
4. 索引指向的表项（E0, 693-708）field_0/field_4 均为合法 ROM 指针 (0x0855xxxx/0x0858xxxx)
   —— 确为图形帧/资源表项。

**强推断 (E2，多消费者一致)：**
5. +0x08 u16[4] = 动画副索引表：sub_801CE80 case5 以 `*(u16*)(p+8+arg5*2)` 消费同一区域；
   sub_801DF90/E4D4/E690 以 `animPtr+8+animSubIdx*2` 消费；battle_rewards 本族以
   `subIdx[animSubIdx]` 消费。`obj->animSubIdx` 取值域（Rng&3 / Rng%3 / Rng%5(==2→clamp) /
   1&Rng）≤3，与 4 项容量吻合。
6. +0x02 = fxKind==0 路径的默认表项索引（本族与 sub_801CE80 case1、sub_801DF90 case0 一致）。
7. 选中 `ObjAnimEntry` 后其 `targetMode`(+0x10) 决定 f_BD 赋值：0=随机存活候选
   (values[Rng%count])，1=0。本族经 sub_804DD70 分派表 gUnk_0839CE38[slot-0x71] 进入
   （表项为 addr|1 Thumb 指针，E0 已验证），调用者 sub_801EA70/sub_8020C58（并行会话结论，未重验）。

**工作假设（未定）：**
- +0x00/+0x04/+0x06 三个索引的具体语义按 sub_801CE80 kind0/1/2/6 推断，本视图未建模（保留 pad_0[8]）。
- gUnk_0839ABCC 尚无符号名；若后续该块全布局入库 code_0.h，可把 ObjAnimIdxBlk 一并上收。

## 连带修复
并行会话（agent=zcode，sub_804D1B4 持有者）已将 code_0.h 的表项类型改为 `ObjAnimEntry`
（成员 field_10→targetMode），但 battle_rewards.c 16 处 `switch (entry->field_10)` 未跟上，
TU 编译失败。本次按 header 既定方向机械补齐 16 处 → `entry->targetMode`（纯标识符，无形状影响）。
文件在本次工作期间被并行修改过一次（首次编辑触发 stale 检测，已重读后基于最新原文编辑）。

## 验证
- `make build/src/battle_rewards.o` 通过（仅剩并行会话遗留的 sub_80489E8 arg1 指针类型警告，非本次引入）。
- `fncheck` 全部 15 个 C 函数 OK（sub_804D798 为 INCLUDE_ASM 不涉及）：
  D1B4/D260/D310/D3A0/D44C/D4FC/D5B4/D708/D840/D8F4/DA04/DABC/DB64/DC24/DCD8 字节不变。

## 锁状态
已认领并将于本 handoff 提交后释放：sub_804DABC、D4FC、D5B4、D840、D8F4、DA04、DB64、DC24、DCD8
（共 9 把）。未触碰 zcode 的 sub_804D1B4 区段源码（仅文件级机械成员改名波及其行，属 header 改动的必然后果）。

## functions.tsv
sub_804DABC 行已追加改名注记（历史注记保留原文）。
