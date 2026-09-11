# 模块语义化物理重构规范与边界定性报告: code_8044394.c & code_804F0B8.c

> 本文档为独立反编译与架构重构文档，由 Antigravity 独立创建与维护。
> 严禁向项目历史共享文档（`docs/progress.md`, `docs/EXPERIENCE.md`, `docs/INCIDENTS.md` 等）追加或覆盖内容。
> 终验标准: `make` 构建成功且 `sha1sum -c ll.sha1` 100% 逐字节完全匹配（746/1059, 70.4%）。

---

## 1. 重构背景与原工程问题剖析

在原始工程的粗暴物理划分中，存在两块极为混乱的代码膨胀体：
1. **`src/code_8044394.c`**:
   - 跨度从 `0x08044394` 一路延续至 `0x0804F088`，包含整整 226 个函数，代码汇编行数超过 18.7k 行。
   - 内部杂糅了完全不同职责的子系统：全局实体状态服务、战斗伤害计算数值引擎、战斗流程与回合主控、战斗演出与 OAM 扫描、战后掉落概率抽取、以及场景对象池管理。
   - 由于单 TU 过大，在 GCC 2.95 (agbcc) 编译环境下存在极高的高位寄存器 (r8/sb/sl) 跨函数泄漏污染风险（见经验 1 与经验 1612）。
2. **`src/code_804F0B8.c`**:
   - 在此前拆出 `src/sio_battle.c` 与 `src/script_vm.c` 后，该文件仅突兀地遗留了 3 个对象池操作例程 (`sub_804F0B8`, `sub_804F10C`, `sub_804F17C`)。
   - 其调用目标 `sub_804DD90`, `sub_804E76C` 均位于 `code_8044394.c` 的尾部，人为割裂了对象池的内聚性。

---

## 2. 5 段精细化语义拆分架构

针对上述问题，按照函数聚类特征、调用图谱（Call Graph）与数据结构消费边界，完成了严谨的 5 路语义化物理拆分：

| 模块源文件 | 地址物理范围 | 函数总数 | 已匹配 | 未匹配 | 核心子系统与职责 |
|---|---|:---:|:---:|:---:|---|
| [`src/obj_state.c`](file:///home/gpnux/decomp/ll/src/obj_state.c) | `0x08044394` - `0x08044738` | 20 | 20 (100%) | 0 | **实体全局状态与重置服务 (F5)**<br>包含 `ObjStats_Reset`、`GlobalState_Reset`、`GlobalState_Init`（callers=78，被对象生成器群调用）以及 5 个末尾虚表对齐桩。 |
| [`src/battle_engine.c`](file:///home/gpnux/decomp/ll/src/battle_engine.c) | `0x0804473C` - `0x0804AD24` | 99 | 52 | 47 | **战斗核心规则、数值计算与回合主控 (F7)**<br>包含命中/闪避判定、技能与法术习得、伤害结算（`sub_80457AC`）、行动顺序快排（`sub_8048A88`/`sub_8048ACC`）、以及 407 行大型战斗主流程（`sub_8049DF8`）。 |
| [`src/battle_anim.c`](file:///home/gpnux/decomp/ll/src/battle_anim.c) | `0x0804AD54` - `0x0804D0F8` | 68 | 51 | 17 | **战斗演出、法术动画与 OAM 扫描系统**<br>包含战斗演出 OAM 预扫描状态机（`sub_804AE2C`）、OAM 槽回写（`sub_804AF60`）、精灵动画播放步进、以及特效槽位管理。 |
| [`src/battle_rewards.c`](file:///home/gpnux/decomp/ll/src/battle_rewards.c) | `0x0804D1B4` - `0x0804DCD8` | 16 | 8 | 8 | **战后奖励结算与掉落物品抽取**<br>包含基于伪随机数发生器的孪生结算族（`sub_804D1B4` 等）、道具掉落概率判定与奖励派发。 |
| [`src/obj_pool.c`](file:///home/gpnux/decomp/ll/src/obj_pool.c) | `0x0804DD70` - `0x0804F17C` | 26 | 19 | 7 | **场景对象池管理与槽位过滤助手**<br>完美吸收原 `code_8044394.c` 尾部 23 个函数及原 `code_804F0B8.c` 的 3 个函数（`CheckObjectKindSlot`、`sub_804F10C`、`sub_804F17C`），统一围绕 `GetObjPool()` 展开检索与管理。 |

---

## 3. 链接器与符号表精确对齐

在 [`linker.ld`](file:///home/gpnux/decomp/ll/linker.ld) 的 `.text` 节中，新生成的 5 个 `.o` 按照物理严格连续的地址序排放：

```ld
        src/code_80264C0.o(.text);
        src/obj_state.o(.text);
        src/battle_engine.o(.text);
        src/battle_anim.o(.text);
        src/battle_rewards.o(.text);
        src/obj_pool.o(.text);
        src/sio_battle.o(.text);
        src/script_vm.o(.text);
        src/sound.o(.text);
```

这一顺序与 ROM 镜像中 0x08044394 至 0x0804F280 的二进制布局 100% 吻合，确保链接器重定位完全不发生任何漂移。

---

## 4. 全量编译与逐字节自证结果

```bash
$ timeout 900 make 2>&1 | tail -5 && sha1sum -c ll.sha1
arm-none-eabi-objcopy -O binary ll.elf ll.gba
/usr/bin/sha1sum -c ll.sha1
ll.gba: 成功
匹配进度: 746/1059 (70.4%)
ll.gba: 成功

$ python3 scripts/audit.py
=== functions.tsv 函数清单 ===
共 1059 函数 | 已匹配 746 (70%) | 未匹配 313
改名漂移: 0
note 覆盖: 挂起 55/313 | 完成 212/746
=== status=1 字节核验 ===
通过 746/746

$ python3 scripts/fncheck.py sub_8044394 sub_8044738 sub_804AD24 sub_804AD54 sub_804D0F8 sub_804D1B4 sub_804DD70 sub_804F0B8 sub_804F17C
sub_8044394              OK   (128 bytes @0x08044394)
sub_8044738              OK   (4 bytes @0x08044738)
sub_804AD24              OK   (48 bytes @0x0804ad24)
sub_804AD54              OK   (12 bytes @0x0804ad54)
sub_804D0F8              OK   (188 bytes @0x0804d0f8)
sub_804D1B4              OK   (170 bytes @0x0804d1b4)
sub_804DD70              OK   (32 bytes @0x0804dd70)
sub_804F0B8              OK   (84 bytes @0x0804f0b8)
sub_804F17C              OK   (148 bytes @0x0804f17c)
```

全量 746 个已匹配函数 100% 通过字节核验，`sha1sum` 逐字节完全一致，重构零漂移完成。
