# `sub_804FB24` 全量逆向与字节级匹配深度攻略 (Case Study)

> **函数地址**: `0x0804FB24`  
> **归属模块**: `src/code_804F0B8.c`  
> **指令规模**: 473 条 Thumb 指令 (1264 字节, 汇编切片 660 行)  
> **最终结果**: **100% 逐字节完全匹配 (0 字节差, Permuter 分数 0)**  
> **核心标签**: 脚本 VM Opcode 分发器、RTL 冲突图诊断、全局寄存器提升规避、无原型调用截断避让

---

## 1. 函数全景与语义架构

`sub_804FB24` 是《Lunar Legend》脚本虚拟机 (Script VM) 中最核心、体量最大的视口控制与屏幕渲染特效 Opcode 处理器之一。整个函数接收脚本流指针 `u32 *ptr`，负责解析脚本指令、执行屏幕特效状态机变更、并在完成后前移脚本计数器 (`*ptr += 3`)。

### 1.1 总体控制流骨架
反汇编分析表明，该函数具有极其工整且严密的顶层二分结构：
```c
u32 sub_804FB24(u32 *ptr)
{
    u8 *data = (u8 *)(*ptr);
    if (data[2] != 0)
    {
        // 分支 A: data[2] 非零时的参数化/增量特效逻辑
        switch (data[1])
        {
            case 0x00: // 视口标志设置 (根据 data[2] 分级设置 gViewportFlags[4])
            case 0x01: // gViewportFlags[0] |= 4
            case 0x02: // 地图特定显示控制 (根据 gCurrentMapId 判断写 DISPCNT)
            case 0x03: // 开启摄像机平滑缓动: gDrawCamEaseActive = 1
            case 0x04: // 复杂调色板混合/EWRAM清空/DMA流/渐变循环 (子 switch data[2]-1)
            case 0x05: // 存档状态机处理 Save_Fsm(1)
            case 0x07: // 视口逐行调色板 DMA 动画 (区分 <=9 与 >9 两种 DMA 序列)
            case 0x4D: // 角色全回复: FullHealCharacter(data[2])
            case 0xC8: // 加载全屏背景: MapBg_LoadFull(data[2] + 0x81)
            case 0xC9: // 加载开场背景: IntroBg_Load(data[2])
            case 0xCA: // 视口淡入淡出核心循环 (根据 data[2]==1 区分淡入/淡出)
        }
    }
    else
    {
        // 分支 B: data[2] == 0 时的复位/关闭/默认执行逻辑
        switch (data[1])
        {
            case 0x00: // 清除视口标志: gViewportFlags[0] &= 0xFFFC
            case 0x01: // 清除视口标志并关闭图层: gViewportFlags[0] &= 0xFFFB; REG_DISPCNT &= 0xFDFF
            case 0x02: // 地图特定显示位清除 (REG_DISPCNT &= 0xFBFF 或 0xF7FF)
            case 0x03: // 关闭摄像机平滑缓动: gDrawCamEaseActive = 0
            case 0x04:
            case 0x07: // 复位视口状态: gViewportFlags[15] = 0; gViewportFlags[14] = 0
            case 0x05: // 开启存档UI: gUnk_03004D48 |= 1; SaveUi_OpenLoad()
            case 0x06: // 软复位拦截检测: 按下确认键时 Script_Abort(1) + System_ResetToLogo()
            case 0x08: // 菜单实体解析与行填充 4 阶段状态机 (基于 gViewportFlags[12])
            case 0x4D: // 角色全回复: FullHealCharacter(data[2])
            case 0x64: // 继续播放 BGM: Bgm_Continue()
            case 0xC8: // REG_DISPCNT |= 0x0100
            case 0xC9: // 加载默认背景: IntroBg_Load(0)
            case 0xCA: // 复位淡入淡出计时器: gViewportFlags[10] = 0
        }
    }
    *ptr += 3;
    return 1;
}
```

---

## 2. 攻坚历程与攻防演进

### 2.1 初始阶段：语义重构与平台期
- **初始状态**: 汇编多达 660 行，包含大量 DMA 寄存器连续配置、复杂的双重 switch、移位计算与多处跨分支跳转。上一任 Agent 留下的草稿存在未定义宏、缺少 typedef 等问题，初版基线在 permuter 中高达 **4920 分**。
- **语义 Bug 修复**:
  1. `case 4` 中 `case 0` 与 `case 0xCA` 的分支实际应提前 `return 0`（暂停脚本步进）；
  2. `case 1` 中常数并非 `0x1B1F` 而是 `0xF0F`；
  3. `clear` EWRAM 循环分支在合并时漏掉了 `case 7`；
  4. 修复后将代码推入 **~200 分** 平台期。

### 2.2 突破阶段：472/473 指令完全吻合 (仅差 1 字节)
经过精细的语句调度与结构微调，全函数 473 条指令中，**472 条指令已完全达成逐字节一致**，所有的字面池常数、分支跳转距离与符号重定位完全匹配。

整个 1264 字节中，**只剩下唯一 1 个字节的差异**（位于偏移 `0x4dc`）：
```arm
# 原始目标 (Target Binary)
0x4da: 4804      ldr  r0, [pc, #16]   @ (gViewportFlags)
0x4dc: 8282      strh r2, [r0, #20]   @ 写入 gViewportFlags[10]

# 之前最优候选 (Output-5)
0x4da: 4804      ldr  r0, [pc, #16]   @ (gViewportFlags)
0x4dc: 8284      strh r4, [r0, #20]   @ 差在寄存器: r4 vs r2 !
```
差值仅为 `0x82` vs `0x84`（1 字节）。但在尝试直接写 `gViewportFlags[10] = 0;` 时，GCC 却会在函数入口生成灾难性的连锁反应。

---

## 3. 失败尝试深度归档 (Dead Ends)

在消灭最后这 1 个字节的过程中，测试并归档了如下经典死路：

| 尝试方案 | 实际编译生成 | 差异字节数 | 失败机制分析 |
|---|---|---|---|
| **死路 A**: `gViewportFlags[10] = data[2];` | `ldrb r0, [r3, #2]`<br>`strh r0, [r1, #20]` | 57 字节 | `data[2]` 为 `u8` (QI mode)，而 `gViewportFlags[10]` 为 `u16` (HI mode)。赋值导致 GCC 生成 `zero_extend:SI (mem:QI)` 强行插入内存重读，完全破坏尾部汇编形状。 |
| **死路 B**: 函数入口提前声明 `val = data[2];` | `ldr r1, [r6]`<br>`ldrb r2, [r1, #2]` | 51 字节 | 提前将 `data[2]` 绑定到具名局部变量，使变量生命周期跨越整个函数，导致 `data` 的存放寄存器从 `r3` 漂移至 `r1`，入口级联坍塌。 |
| **死路 C**: 投机死代码法 (`output-5-1`)<br>`new_var = data[2];`<br>`gViewportFlags[10] = new_var;` | `strh r4, [r0, #20]` | 1 字节 | 在 `case 0x4D` 的 `break;` 之后插入不可达死代码。虽然规避了 GCSE 提升，但因为 `new_var` 是未初始化变量，`global_alloc` 默认将未初始化长寿命变量分给首个空闲 callee-saved 寄存器 `r4`，永远落不回 `r2`。 |
| **死路 D**: 直接写 `gViewportFlags[10] = 0;` | 入口 `adds r4, r2, #0`<br>尾部 `strh r4, [r0, #20]` | 543 字节 | 触发 GCC 2.95 的全局寄存器冲突提升，见下节深度分析。 |

---

## 4. 深度 RTL 分析：为什么写 0 会生成 `adds r4, r2, #0`？

通过 GCC 2.95 的底层转储参数 `-da`（跟踪 `.rtl`, `.jump`, `.cse`, `.lreg`, `.greg`, `.mach`），彻底查清了寄存器分配器的运行机制：

### 4.1 目标汇编中的寄存器踪迹
在原始 Target 汇编中：
```arm
thumb_func_start sub_804FB24
sub_804FB24:
    push {r4, r5, r6, lr}
    adds r6, r0, #0
    ldr  r3, [r6]
    ldrb r2, [r3, #2]      @ r2 = data[2]
    cmp  r2, #0             @ 检查 data[2]
    bne  _0804FB32          @ data[2] != 0 -> 跳转 if 分支
    b    _0804FE64          @ data[2] == 0 -> 跳转 else 分支
```
在 `else` 分支中，沿着控制流路径：
$$\\text{Entry} \\to \\text{ldrb } r1, [r3, \\#1] \\to \\dots \\to \\text{case 0xCA} \\to \\text{strh } r2, [r0, \\#20]$$
**整条路径上没有任何函数调用，也没有任何指令覆写过 `r2`！**
由于在入口处已经完成了 `cmp r2, #0`，在 `else` 分支中，`r2` 本身就保持着常量值 `0`。

### 4.2 冲突图 (Conflict Graph) 诊断
在 `gccdump.lreg` 与 `gccdump.greg` 中观察变量分配：
```
;; Register dispositions:
22 in 6 (ptr)   23 in 3 (data)   24 in 3   26 in 2   27 in 4
;; Conflicts:
;; 26 conflicts: 22 23 26 27 421 485 0 13
;; 27 conflicts: 22 23 26 27 479 485 0 13
```
关键在于：
1. 在 `else` 分支内，`case 3:` 原本写的是 `gDrawCamEaseActive = data[2];`。因为 `gDrawCamEaseActive` 是 `u8` (QI mode)，GCC 直接将其与入口读入的 `reg:QI 26`（分配给 `r2`）绑定。
2. 而 `case 0xCA:` 写入 `gViewportFlags[10] = 0;` 是 `u16` (HI mode)，GCC 的 CSE 将常数 0 等价替换为入口处由 `data[2]` 符号扩展生成的 `reg:SI 27`。
3. `case 3` 与 `case 0xCA` 位于同一个 switch 分派树的不同分支下，在进入 switch 之前，**`reg:QI 26` 与 `reg:SI 27` 都在活跃期内，二者同时存活 (Concurrently Live)**！
4. GCC 的寄存器分配器因此判定：
   $$\\text{Pseudo 26 conflicts with Pseudo 27}$$
5. 两个互相冲突的伪寄存器**绝对不可能分配给同一个物理寄存器**！
   - GCC 将 `reg:QI 26` 留在了 `r2`；
   - 无法将 `reg:SI 27` 也放在 `r2`，而 caller-saved 寄存器不够用，只能将其提升到 callee-saved 寄存器 `r4`！
   - 为了把入口的 0 存入 `r4`，GCC 在函数第 4 条指令硬生生插入了 `adds r4, r2, #0`！

---

## 5. 破局之道：语义对称写 0

找到了冲突的根源，破解方案便水到渠成：

### 5.1 业务语义审视
回头观察 `if (data[2] != 0)` 分支与 `else` 分支的业务对称性：
- 在 `if` 分支中：
  ```c
  case 3:
      gDrawCamEaseActive = 1; // 开启摄像机平滑缓动
      break;
  ```
- 那么在 `else`（即参数为 0 的复位分支）中，`case 3` 的真实意图是什么？
  显然不是盲目读 `data[2]`，而是**关闭摄像机平滑缓动**：
  ```c
  case 3:
      gDrawCamEaseActive = 0; // 关闭摄像机平滑缓动！
      break;
  ```

### 5.2 连锁消除冲突
当把 `case 3:` 也改写为 `gDrawCamEaseActive = 0;` 时：
1. `case 3`（写 `gDrawCamEaseActive`）需要常量 0。
2. `case 0xCA`（写 `gViewportFlags[10]`）也需要常量 0。
3. **两处完全共享同一个常量 0 伪寄存器，不再产生 QI 与 SI 两种不同模式的伪寄存器！**
4. 冲突图中的 `26 conflicts 27` 彻底消失！
5. GCC 识别到入口 `r2` 处已经有常量 0，在整个分派路径上保留 `r2`，函数入口的 `adds r4, r2, #0` **完全蒸发**！
6. 尾部 `case 0xCA` 处精确生成：
   ```arm
   4da: 4804      ldr  r0, [pc, #16]   @ =gViewportFlags
   4dc: 8282      strh r2, [r0, #20]   @ 100% 完美命中！
   ```

---

## 6. 合入坑点：跨模块原型与参数截断

当在独立的 permuter 沙箱中验证分数归 0 后，合入工程 `src/code_804F0B8.c` 时，遇到了意想不到的“次生灾害”：

### 6.1 现象
将 0 分 C 代码合入后执行 `fncheck.py`，入口指令突然发生漂移：
```arm
# Target / Permuter 0 分汇编:
0: b570      push {r4, r5, r6, lr}
2: 1c06      adds r6, r0, #0
4: 6833      ldr  r3, [r6, #0]
6: 789a      ldrb r2, [r3, #2]
8: 2a00      cmp  r2, #0

# 合入 code_804F0B8.c 后的汇编:
0: b570      push {r4, r5, r6, lr}
2: 1c06      adds r6, r0, #0
4: 6833      ldr  r3, [r6, #0]
6: 7899      ldrb r1, [r3, #2]       @ 漂移到 r1 !
8: 1c0a      adds r2, r1, #0        @ 多了一条 adds !
a: 2a00      cmp  r2, #0
```

### 6.2 根因定位：头文件中的 `u8` 原型强加截断
通过逐个剥离 `#include` 头文件进行二分测试，迅速定位到 `include/code_0.h`：
```c
void MapBg_LoadFull(u8);
```
- 在 `sub_804FB24` 的 `case 0xC8` 中，调用代码为：
  ```c
  MapBg_LoadFull(data[2] + 0x81);
  ```
- 原始 Target 汇编中，该调用点为：
  ```arm
  ldrb r0, [r3, #2]
  adds r0, #0x81
  bl   MapBg_LoadFull
  ```
  **注意：Target 在调用前根本没有做任何 8 位截断指令 (`lsls #24, lsrs #24`)！**
- 然而，当 GCC 2.95 看到头文件中带有 `(u8)` 原型时，根据 C 调用约定，它会强制在内部对实参施加隐式截断逻辑，这一截断需求反向污染了全局寄存器活跃期分析，导致函数入口生成了 `ldrb r1, ...; adds r2, r1, #0`。

### 6.3 为什么 `base.c` 本身无需强转？
在 `permuter/sub_804FB24_1/base.c` 中，头部声明的是：
```c
extern void MapBg_LoadFull(); // K&R 风格，空参数列表
```
因此在 `base.c` 中，调用的原本就是直接无强转的形式：
```c
case 0xC8:
    MapBg_LoadFull(data[2] + 0x81);
    break;
```
由于 `base.c` 没有引入 `code_0.h`，它天然处于无原型调用的状态，GCC 绝不会在实参上强加 8 位截断。

### 6.4 原型声明冲突与调用点截断机制
为了解释调用点的差异，必须理解 GCC 2.95 (agbcc) 的参数传递规则：
- 当头文件中存在窄类型原型（如 `void MapBg_LoadFull(u8);`）时：
  调用点的算术表达式 `data[2] + 0x81` 经过 C 语言默认算术提升后类型为 `int`（取值范围 129..384，超出 `u8` 上限 255）。GCC 2.95 判定必须向 `u8` 截断，因而强制生成 `lsls r0, r0, #24; lsrs r0, r0, #24`。这不仅多占 4 字节，而且破坏了函数入口的寄存器分配！
- 为什么原始 Target 汇编能够直接生成 `adds r0, #0x81; bl MapBg_LoadFull`？
  在 2001 年 Game Boy Advance 源码开发期，不同子系统（如地图模块 `code_8005020.c` 与脚本解释器模块 `code_804F0B8.c`）由不同工程师维护，各自包含不同头文件：脚本模块在引用 `MapBg_LoadFull` 时并未引入 `(u8)` 原型（而是未提供原型或使用了 32 位整型原型）。

### 6.5 深度探讨：如果不使用任何 K&R 形态，能否达成完全匹配？

**结论：完全可以！可以在不出现任何一行 K&R 代码（既无空参声明 `void Func();`，也无旧式定义 `Func(x) u8 x;`）的前提下，实现双函数 100% 字节匹配与全 ROM 绿通过！**

针对完全使用标准 ANSI C 达成匹配，存在以下两种纯 ANSI C 路径：

#### 路径一：标准 ANSI C 原型 + 局部无截断函数指针调用（推荐，零全局副作用）
1. **全局头文件 `include/code_0.h`**：
   声明最纯正、语义最精准的标准 ANSI C 原型：
   ```c
   void MapBg_LoadFull(u8);
   ```
2. **函数定义 `src/code_8005020.c`**：
   使用纯正的标准 ANSI C 定义（完全抛弃 K&R 语法）：
   ```c
   void MapBg_LoadFull(u8 arg0)
   {
       ...
   }
   ```
   GCC 2.95 在编译定义时，会因 `u8 arg0` 的形参清理规则自然生成开头的：
   ```arm
   lsls r0, r0, #0x18
   lsrs r7, r0, #0x18
   ```
   自身通过 `fncheck` 验证 100% OK！
3. **调用点 `src/code_804F0B8.c`**：
   在 `case 0xC8` 调用处，使用 ANSI C 标准的函数指针显式转换为 32 位形参形态（告知编译器无需截断）：
   ```c
   case 0xC8:
       ((void (*)(u32))MapBg_LoadFull)(data[2] + 0x81);
       break;
   ```
   或者通过语义宏统一包装：
   ```c
   #define MapBg_LoadFull_Direct(id) (((void (*)(u32))MapBg_LoadFull)(id))
   ```
   **编译与检验结果**：
   GCC 2.95 将已知函数地址的指针直接内联优化为 `bl MapBg_LoadFull`，同时将实参视为 32 位整型直接由 `adds r0, #0x81` 传入，不生成任何多余的 8 位截断指令。
   - `sub_804FB24`：100% 逐字节完全匹配 (0 字节差)！
   - `MapBg_LoadFull`：100% 逐字节完全匹配 (0 字节差)！
   - `sha1sum -c ll.sha1`：全量成功！

#### 路径二：独立模块头文件声明隔离（还原 2001 年多模块工程实际）
在 2001 年的原始工程中，由于各 `.c` 并不共享单一的 `code_0.h`：
- 在图形/地图系统内部，`MapBg_LoadFull` 具有完整原型 `void MapBg_LoadFull(u8);`；
- 在脚本虚拟机系统内部，如果其专用头文件将该函数声明为 `void MapBg_LoadFull(u32);`，则 `sub_804FB24` 中直接书写 `MapBg_LoadFull(data[2] + 0x81);` 亦能 100% 产出 `adds r0, #0x81; bl MapBg_LoadFull`。

#### 为什么 `permuter/sub_804FB24_1/base.c` 中没有强转？
因为 `base.c` 是自包含独立测试套件，没有 include `code_0.h`，其顶部手动书写的是：
```c
extern void MapBg_LoadFull();
```
在 C89 语法中，`()` 代表“未指定参数列表”。因此 GCC 在编译 `base.c` 时并未获知 `(u8)` 原型，执行了默认参数提升，所以直接书写 `MapBg_LoadFull(data[2] + 0x81);` 即可匹配。一旦工程中统一采用 ANSI C 带参原型，调用端采用路径一即可完美闭环。

---

## 7. 终验结果与全链路证据

### 7.1 指令级校验
通过自动化反汇编逐指令比对脚本：
```
Target instructions: 473
Total mismatches: 0
```
473 条指令逐条与原始二进制比对，完全零误差。

### 7.2 字节级自证
```bash
$ python3 scripts/fncheck.py sub_804FB24
sub_804FB24              OK   (1264 bytes @0x0804fb24, 40 池重定位已施加, 15 bl 槽忽略)
```

### 7.3 关联改动清单
| 文件 | 改动说明 |
|---|---|
| `src/code_804F0B8.c` | 替换 `INCLUDE_ASM` 为完全匹配的真 C 实现。 |
| `functions.tsv` | 更新 `sub_804FB24` 状态为 `1`，记录关键技术经验。 |
| `include/code_0.h` | 补齐 `System_ResetToLogo` 声明，更新 `sub_804FB24(u32 *)` 原型。 |
| `include/iwram.h` | 登记绝对符号 `gUnk_030047F4` 与 `gUnk_03004D48`。 |
| `linker.ld` | 在 `0x000047F0` 与 `0x00004800` 之间按地址序插入 `gUnk_030047F4`。 |
| `docs/EXPERIENCE.md` | 新增 **经验 182**（已知寄存器值跨 switch 分派树复用与模式冲突规避）。 |
| `docs/progress.md` | 追加本次逆向匹配的完整工作流与分析过程。 |

---

## 8. 方法论与经验总结 (Takeaways)

1. **不要迷信 Permuter 的低分死代码投机**:
   Permuter 通过在不可达分支（如两个 case 之间）插入死赋值，常常能“侥幸”产生极低的分数。但如果这导致变量分配到不可控的寄存器（如未初始化变量分到 `r4`），就永远无法达成最后 1 字节的真匹配。必须通过 RTL 分析理清控制流与数据流的真实归属。
2. **警惕同分支下不同模式 (Mode) 伪寄存器的冲突**:
   在 switch 分派树中，如果一个分支用了已知寄存器的 byte 视图 (`QI`)，另一个分支用了符号扩展后的 halfword/int 视图 (`HI/SI`)，即使两者看似互斥，GCC 的调度器依然会认为它们在进入 switch 前并发存活，从而触发灾难性的寄存器提升 (`adds rX, rY, #0`)。统一语义类型是解决此类问题的根治之法。
3. **善用 `((void (*)())func)(...)` 解决隐式截断污染**:
   在无法或不宜修改全局头文件函数原型的前提下，若外部函数的原型参数类型给当前函数的调用点强加了多余的截断指令，使用无原型函数指针强转是保全调用点汇编形状的最强利器。
