# HANDOFF: src/sio_battle.c → src/battle_fx.c 文件改名 (module 名去除伪 SIO)

- 日期: 2026-09-14, agent: zcode (认领 BattleFx_Reset/BattleFx_GetObjCount/BattleDrops_Clear, 已释放)
- 前置: RENAME-30000DDE-E30-20260914.md / STRUCTS-30000DDE-E30-20260914.md。
  该文件 3 函数 (BattleFx_Reset 0x0804F210 / BattleFx_GetObjCount 0x0804F244 /
  BattleDrops_Clear 0x0804F250) 与 SIO 联机无关 (前两份 handoff 的取证), 文件随语义改名。

## 改动

1. `src/sio_battle.c` → `src/battle_fx.c` (纯 mv, 内容零修改; `src/sio_link.c` 未动,
   其真伪 SIO 身份未经取证, 留待单独验证)。
2. `rm build/src/sio_battle.{o,s,i}` — Makefile 用 `$(wildcard src/*.c)` 收集,
   不删旧 .o 会双链接重定义符号。
3. `functions.tsv` 3 行 module 列: sio_battle → battle_fx。
4. `scripts/gen_debug_ld.py`: debug 链接脚本文件清单 src/sio_battle.o → src/battle_fx.o。

## 验证状态 (诚实记录)

- ✅ `build/src/battle_fx.o` 编译成功 (改名后首次编译)。
- ✅ ROM 字节不变性: 纯文件名改动不进入任何 .o 的 section 内容, 链接产物必然逐字节一致
  (agbcc -g 的调试信息不进入 objcopy -O binary 的 ROM)。
- ✅ 3 函数 fncheck OK (52B/12B/48B)。
- ⚠ **全量 `make`+`sha1` 复验被并发编辑阻塞**: zcode-pal-ae0 正在 iwram.h 把
  gPendingPortrait* 宏改为 typed extern (HEAD 有宏/工作区已删/anim_slot.c 未跟上),
  anim_slot.o 编译失败, 链接无法完成。**不是本改动引起** (证据: git diff iwram.h 中
  Portrait 三行删除与本文件其余改动无交集)。磁盘上的 ll.gba 是此前绿链的产物, 仍 OK。
- **后续**: 待 zcode-pal-ae0 完成后重跑 `make && sha1sum -c ll.sha1`, 预期绿 (869/1059)。

## 后续

- `src/battle_rewards.c` 文件名同样与证据不符 (实为 slot≥0x71 特殊对象目标选取, 见
  OBJANIMENTRY handoff), 可改名 battle_target_fx.c 之类 — 未动 (涉及 16 行 tsv module 列)。
- `src/sio_link.c` 与其它 sio_* 命名需独立取证。
