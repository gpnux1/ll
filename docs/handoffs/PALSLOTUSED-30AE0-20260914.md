# 类型分析 handoff: gUnk_03000AE0 / gUnk_03000AE2 → gObjPalSlotUsed / gBgPalSlotUsed (2026-09-14, agent=zcode-pal-ae0)

## 任务
分析 IWRAM 全局 `gUnk_03000AE0` (u16) 与 `gUnk_03000AE2` (u16) 的数据类型语义并合理重命名。

## 改名结果
| 旧名 | 新名 | 语义 |
|---|---|---|
| `gUnk_03000AE0` | **`gObjPalSlotUsed`** | OBJ 调色板动画系统 16 槽占用位图 (bit0-15, 1=占用) |
| `gUnk_03000AE2` | **`gBgPalSlotUsed`** | BG 调色板动画系统 16 槽占用位图 (bit0-15, 1=占用) |

定义点: linker.ld (IWRAM 标签), 声明: iwram.h。两 bitmap 分别是 0x03000AE8 / 0x03000BE8
两张 `PaletteAnimEntry[16]` 调色板动画表的槽占用标记, 与各自系统的 bank/mirror 配对:

| bitmap | 表 | 调色板 bank | 镜像 | 逐帧驱动 |
|---|---|---|---|---|
| gObjPalSlotUsed (AE0) | 0x03000AE8 | 0x05000200 (OBJ palette RAM) | 0x02036AC0 | sub_804C45C |
| gBgPalSlotUsed (AE2) | 0x03000BE8 | 0x05000000 (BG palette RAM) | 0x02036CC0 | sub_804C6B0 |

## 位图语义 (E2, 消费者交叉验证)
bit n 置位 = 对应 bank 的调色板槽 n 被战斗调色板动画子系统占用 (已装载或保留):
- **置位**: 装载并占用 `sub_804C2FC` (DMA 到 0x05000200+slot<<5 后置位; battle_obj_core.c:153/959/1119/2672/2728 用 headA.palSlot/槽 0xF/i+1 装敌我精灵调色板), `sub_804C548` (0x05000000 侧; sio_link.c:569/606 装槽 0-2/0xB-D), 纯占用 `sub_804C364`/`sub_804C5B8` (经分派器 sub_804C1B4 sel=0/1; 804BBDC mode3 分配路径)。
- **清位**: `sub_804C3A4`/`sub_804C5F8` (经 sub_804C1E4); 释放者: event_hub.c:92/118/189/... 以 `headA.palSlot` 注销精灵调色板槽, sub_804B8E8/BB64/BD54/BE90/C10C 淡变收尾, sub_804B288 复位连同两 bitmap 一起清零。
- **读**: `sub_804C2F0`/`sub_804C53C` (整字返回), `sub_804C214` (单 bit 查询 sel=0/1), 及未匹配 asm 的找首个空闲槽循环 (`asrs+ands`, 上限 0xF): sub_804B654/804B96C 查 AE0, sub_804BBDC mode3/804BF14 查 AE2 (BLOCKED-804BBDC-20260913.md 的"调色板槽占用位图"结论一致)。

## 连带纠正: gBgPalAnim/gObjPalAnim 与 GBA 硬件 BG/OBJ 互换
改名过程中确认两张表的旧名标签与硬件内存图互换, 已一并纠正 (纯标识符互换):
- 0x03000AE8: 旧 `gBgPalAnim` → **`gObjPalAnim`** (驱动 0x05000200 = OBJ palette RAM)
- 0x03000BE8: 旧 `gObjPalAnim` → **`gBgPalAnim`** (驱动 0x05000000 = BG palette RAM)

证据:
- **E0 (硬件内存图)**: GBA BG palette = 0x05000000..0x050001FF, OBJ palette = 0x05000200..0x050003FF; OAM attr2 调色板号只寻址 OBJ bank。AE8 系所有 DMA 目的都是 `0x05000200 + slot<<5` (sub_804C2FC/804C3E4/804C420/804C45C), BE8 系全部是 `0x05000000 + slot<<5` (sub_804C548/804C638/804C674/804C6B0)。
- **E2 (消费者语义)**: AE8 系槽号全部来自/去往精灵头 `headA.palSlot` (code_0.h:481 "palSlot +0x29 调色板槽 (sub_804C2FC 实参)"; event_hub.c 以 headA.palSlot 释放; battle_obj_core.c:2672 装敌我精灵调色板到槽 i+1; sub_804B224 同屏 0x06012E80 OBJ 贴图区拷贝 + 槽 0xF 调色板装载) — 精灵调色板必须位于 OBJ bank。BE8 系消费者为 sio_link 装载 BG 侧调色板、sub_804BBDC 闪白/演出淡变 (BG bank 淡变)。
- iwram.h 旧注释自身已写明 AE8"目的 0x05000200", 与其"BG"标签自相矛盾 — 系早期标签笔误并被后续注释沿用。

影响面: battle_anim.c (数组标识符 + 7 处注释 bank 措辞: B7B0/B834/B8E8/BB64 注释"BG 调色板"→OBJ, BDD8 注释"OBJ"→BG, B8E8 注释尾"(OBJ 表)"→"(BG 表)", B288 注释头补 bitmap 新名), iwram.h (extern ×4 + 块注释纠正带 ⚠ 说明), linker.ld (4 个标签)。标识符交换按地址映射保持, 所有既有引用自动落位正确; `#if 0` 候选 (sub_804B834/sub_804BDD8) 同步换名。

## 验证
- `make build/src/battle_anim.o` 通过 (警告均为既有整型→指针转换, 非本次引入)。
- `fncheck` 21 个受影响/相邻函数全 OK 字节不变: B7B0/B8E8/BB64/BD54/BE90/C10C/C45C/C4D8/C6B0/C728/C2F0/C2FC/C364/C3A4/C53C/C548/C5B8/C5F8/C214/C1B4/C1E4 (INCLUDE_ASM 的 B654/B96C/BBDC/BF14/B288/B834/BDD8 仅注释/候选变动, 不参与编译)。
- 全量 `make` + `sha1sum -c ll.sha1`: **ll.gba OK**, 进度 869/1059 (82.1%); ll.map 已再生成, 新符号名落位 0x03000ae0/0x03000ae2。

## 锁状态
21 把认领 (上列函数) 已全部释放。未触碰 claude804AF60 (sub_804AF60) 与 void-main (sub_805063C)。

## functions.tsv
sub_804C2F0 / sub_804C53C 行已追加改名注记 (其余函数见本 handoff)。

## 工作假设 (未定)
- 位图 bit 的确切措辞"占用"覆盖两种来源 (装载置位与保留置位), 两种场景行为一致 (找空槽时都视 1 为不可用), 未再细分。
- sub_804C728/C4D8 (淡变周期批量设置) 不触碰位图; 与位图的关系仅经由各自表。
