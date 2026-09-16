# MATCH-BattleDrops_Roll-20260916.md

## 1. 基础信息
- **函数名**: `BattleDrops_Roll` (旧名: `sub_804E9DC`)
- **ROM 地址**: `0x0804E9DC`
- **物理 TU**: `src/battle_itemuse_rewards.c`
- **函数大小**: 552 字节 (271 行 ASM)
- **匹配状态**: 100% Match (fncheck OK 552B, SHA1 全绿, 898/1059)
- **日期**: 2026-09-16
- **认领 Agent**: `antigravity` (接管自 `claude_1` / `zcode`)

---

## 2. 机器契约 (Machine Contract)
- **输入**:
  - `u32 *arg0`: 战后掉落表指针输出槽 (`*arg0 = (u32)gBattleDrops`)
- **输出**:
  - `u8`: 产生的有效掉落物品条目数 (`gBattleDropCount`)
- **寄存器使用**:
  - `r4`: 循环索引 / 临时运算寄存器
  - `r5`: 对象池指针 (`pool` / `objs`) / 标志
  - `r6`: 收集槽位数量 / 外部循环索引
  - `r7`: 最终掉落物品 ID / 临时值
  - `r8`: `&gBattleDropCount` 缓存指针 (高寄存器)
  - `sb` (`r9`): 幸运加成增量 (`bonus`, 0xF 或 0)
  - `sl` (`r10`): 怪物列表指针 (`list`, 来自 `sub_8020E68()`)
- **外部依赖与调用 (Callees)**:
  - `GetObjPool()` (`battle_task_services.h`): 获取战斗对象池基址 (调用 2 次，分别赋给局部变量)
  - `sub_8020E68()` (`battle_object_engine.h`): 获取当前敌方怪物槽位配置列表 (`list[0]` 为数量，条目位于 `list + 1 + i * 4`)
  - `sub_80489E8(...)` (`battle_flow_rules.h`): 筛选参战槽位。通过模块内 `static inline Battle_CollectSlots` 封装调用
  - `sub_804E76C(...)` (`battle_itemuse_rewards.h`): 检查对象装备属性 (参数 `5, 4`，检测幸运系装备)
  - `Rng_LcgNext()`: 随机数发生器，`(s32)Rng_LcgNext() % 100` 产生有符号模运算 `__modsi3`
  - `Inventory_AddItem(u8 itemId, u8 count)` (`player_stats.h`): 背包入库函数
- **全局状态与数据表**:
  - `gBattleDrops` (`0x03000E08`, `BattleDropEntry[]`): 战后掉落记录表
  - `gBattleDropCount` (`0x03000E30`, `u8`): 战后掉落有效条目数
  - `gUnk_0839D9B8` (`0x0839D9B8`, `[][5]`): 普通怪物掉落概率表 (101 项 × 5 字节，步长 5)
  - `gUnk_0839DBB1` (`0x0839DBB1`, `[][4]`): 特殊/高阶怪物掉落概率表 (步长 4)

---

## 3. 业务逻辑与语义建模 (Semantic Modeling)
本函数为战斗胜利后的**战利品掉落计算与结算核心驱动**:
1. **重置计数与初始化**:
   - `gBattleDropCount = 0`。
   - 分别调用 `GetObjPool()` 获取全局对象池指针，调用 `sub_8020E68()` 获取敌方怪物列表。
2. **幸运装备检测 (Bonus Roll)**:
   - 收集参战友方对象槽位 (`Battle_CollectSlots`)。
   - 遍历各参战角色槽位，调用 `sub_804E76C(obj, 5, 4)` 检查装备被动效果字段是否包含幸运属性 (如盗贼之心、龙之戒等)。
   - 若命中任意幸运装备，则将 `bonus` 设为 `0xF` (15)，掉落物品 ID 将自动提升 15 号进入稀有档；否则 `bonus = 0`。
3. **逐怪物掉落掷取**:
   - 遍历敌方列表 `i = 0 .. list[0] - 1`，跳过 `pool[i * 0xC8 + 0x493] == 7` 的怪物。
   - 获取怪物槽号 `v = list[1 + i * 4]`。
   - 若 `v <= 0x70`: 查普通表 `gUnk_0839D9B8[v - 0xC]` (步长 5)。
   - 若 `v > 0x70`: 查特殊表 `gUnk_0839DBB1[v - 0x71]` (步长 4)。
   - 第一道门槛: `Rng % 100 <= 59` (60% 基础掉落门)。
   - 第二道阈值: 再次掷骰 `Rng % 100 < tbl_thresh`。若通过则计算掉落物品 `val = base_item_id + bonus`。
4. **去重合并与记录**:
   - 在 `gBattleDrops` 现有记录中线性查找是否存在同种 `itemId`。
   - 若存在则其 `count++`；若不存在则新建记录 `itemId = val, count = 1` 并递增 `gBattleDropCount`。
5. **入库与返回**:
   - 遍历 `gBattleDrops` 中所有产生的掉落项，逐项调用 `Inventory_AddItem(itemId, count)` 直接送入玩家背包。
   - 写入 `*arg0 = (u32)gBattleDrops`，返回 `gBattleDropCount`。

---

## 4. 关键卡点突破与 GCC 2.9 编译器机理 (Root Cause & Solution)
### 历史卡点
历史多轮探索（claude_1 等）中，指令形状与分支完全对齐，但始终困扰于：
- 寄存器角色全局错位 (`r4` 与 `r5` 互换，导致全局分配多处偏移，分数为 6030)。
- 关键差异点定位在 `sub_80489E8` 的参数发射阶段：
  - 目标 ROM:
    ```asm
    mov r1, sp
    movs r2, #0
    ldr r3, =0x1FF
    bl sub_80489E8
    ```
  - 候选 C 直接调用时:
    ```asm
    ldr r3, =0x1FF
    mov r1, sp
    movs r2, #0
    bl sub_80489E8
    ```
  `ldr r3, =0x1FF` 恒定被提前发射到 `mov r1, sp` 之前。

### 根本原因剖析 (GCC 2.91 RTL / CodeGen 机制)
在 GCC 2.91 中，当常量 `0x1FF` 作为函数调用的第四个参数传入时：
1. 因为 `0x1FF` 无法通过一条 `movs` 立即数生成（`rtx_cost > 2`），在直接函数调用上下文下，`expr.c` 的 `precompute_register_parameters` 将其视为昂贵表达式，强制使用 `copy_to_mode_reg` 提前生成伪寄存器，从而将 `ldr r3, =0x1FF` 调度到了参数压入/寄存器分配流程的前列。
2. 这一参数发射顺序的颠倒直接改变了基本块内伪寄存器的生存期边界与活跃变量分析结果，引发后续 `r4`/`r5` 的寄存器分配交换。

### 破局解法 (Static Inline Wrapper)
原开发团队在 `src/battle_itemuse_rewards.c` 内部定义了槽位收集辅助函数：
```c
static inline u8 Battle_CollectSlots(BattleObj *objs, u8 *values, u8 mode, u16 flags)
{
    return sub_80489E8(objs, values, mode, flags);
}
```
- 通过 `static inline` 函数包装，调用点处的参数先绑定到内联函数的形参声明 (`PARM_DECL`)。
- 在内联展开时，GCC 2.91 能够将常量直接下推到 `load_register_parameters` 阶段，按函数参数的自然顺序依次生成：
  `mov r1, sp` -> `movs r2, #0` -> `ldr r3, =0x1FF` -> `bl sub_80489E8`。
- 该发射顺序完美复原后，整个函数的全局寄存器分配链完全归位，**实现 100% 字节一致 (552 字节 0 差异)**。
- 姊妹证据：同文件中紧邻的下一个未匹配函数 `sub_804EC04` 中两次调用 `sub_80489E8` 具有完全相同的指令序列，确证了该 static inline helper 是原团队在模块内的通用惯用写法。

---

## 5. 验证结果
- `scripts/fndiff.sh BattleDrops_Roll`: 0 diffs
- `scripts/fncheck.py BattleDrops_Roll`: OK (552 bytes @0x0804e9dc, 0 池重定位, 18 bl 槽忽略)
- `scripts/fncheck.py` 对 `src/battle_itemuse_rewards.c` 全部 24 个已匹配函数复验：全部通过 (24/24 OK)
- `make` + `sha1sum -c ll.sha1`: 全工程构建成功，SHA1 验证通过 (进度提升至 898/1059, 84.8%)
