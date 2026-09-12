# AGENTS.md — GBA 反编译工作规则

> 项目：Lunar Legend (Japan) GBA ROM 全量反编译。
> 最终目标：写出语义合理、命名可读、符合 GBA 时代 GCC2.9 生成习惯，并与原 ROM 字节完全一致的 C 代码。
> `make` 后 `sha1sum -c ll.sha1` 是全量终验；单函数以 `fncheck.py`/`bytecmp.sh` 为准。

## 0. 先理解项目目标

本项目不是两个互相独立的项目：

- 不是只做“语义分析”；
- 不是只做“字节匹配”；
- 不是只做脚本 VM；
- 不是只把旧的 `sub_XXXXXX` 改成好看的名字。

正确闭环是：

```text
ROM/ASM/调用图取证
    -> 全局、结构体、参数、状态和功能建模
    -> 人类可读的 C
    -> GCC2.9 生成形状调整
    -> bytecmp/fncheck 字节确认
    -> 合入并全量验证
```

语义分析的价值是让最终 C 正确、可读、可维护；字节匹配是交付门槛；GCC2.9 形状调整不能反过来破坏语义和命名。

## 1. 零信任命名，保留语义目标

以下内容只能作为检索线索，不能单独当作事实：

- `src/*.c` 文件名和 `functions.tsv` 的 `module` 列；
- 函数名、全局变量名、结构体名、字段名和注释；
- `FAMILIES.md`、`FUNCTIONAL_MAP.md`、`ROUTES.md`、`docs/modules/*`；
- `progress.md`、`EXPERIENCE.md` 和其他 agent 的历史结论；
- 旧脚本 authoring 表、opcode 表和人工 listing。

这些资料可以帮助定位代码，但所有真正的命名和语义都要回到：

- E0：原始 ROM 字节、原始数据表、函数指针表、任务表、对象表、脚本字节；
- E1：按当前 ROM 和地址生成的 ASM；
- E2：多个调用者/消费者对参数、返回值、字段和状态的一致证据；
- E3：受控运行、调试器、模拟器或脚本样本的交叉验证。

已有函数即使 `status=1`，也只代表字节匹配，不代表它的名字、原型、结构体或功能解释一定正确。

## 2. 并行协作铁律

1. 开工前认领：
   ```bash
   export DECOMP_AGENT=<稳定名称>
   scripts/claim.sh <当前函数名>
   ```
   已被占用就换目标。完成或暂停后释放；不要释放其他 agent 的锁。
2. 认领粒度是地址函数或明确的同构族，不是整个 C 文件、module 或功能模块。
3. 共享文件只改自己的局部：`functions.tsv` 按地址独占行；源码只改自己函数区段；共享头文件、`iwram.h`、`linker.ld` 记录全部影响。
4. 编辑共享文件前重新读取目标原文；禁止整文件 checkout、stash、回退、`cp backup` 覆盖或 `git add -A`。
5. 禁止固定寄存器、内联汇编、goto 凑形状、无界索引、错误指针步长和伪造数据流。
6. 不手改 `asm/`、`code.s`、`ll.map` 等生成物。
7. 禁止 agent `git commit`/`git push`。
8. `.claims/`、`permuter/`、`.scratch/` 中的内容可能被忽略；重要结论必须写入版本控制的 `docs/handoffs/<package_id>.md`。
9. `build/`、`ll.gba`、`ll.elf` 是共享状态；不要并发 `make clean`，不要把别人的构建结果当成自己的验证。

## 3. 标准工作循环

### 3.1 认领和初勘

```bash
export DECOMP_AGENT=<稳定名称>
git status --short --branch
scripts/claim.sh --list
rg -n '<地址或当前名>' functions.tsv ll.cfg
```

读取当前地址的 ASM、真实源码占位、直接调用者、被调用者、函数指针表/任务表/对象表引用。地址是主键；当前名字和 C 文件只是缓存标签。

### 3.2 建立机器契约

每个目标函数都记录：

```text
address:
current_name: 仅检索标签
physical_tu: 仅物理归属
match_status: 0/1
entry_kind: direct / task-table / vblank / object-table / opcode / unknown
callers:
callees:
indirect_tables:
args: 寄存器/栈位置、宽度和用途
return: 寄存器/标志和调用者用途
memory_reads: 地址/偏移、宽度、条件
memory_writes: 地址/偏移、宽度、条件
persistent_state: 跨帧地址和生命周期
script_relation: 无 / 被脚本触发 / 改变脚本状态 / 未知
```

### 3.3 建模和命名

沿调用图分析已匹配和未匹配函数，确认：

- 函数职责和边界；
- 全局变量生命周期和所有消费者；
- 对象/结构体字段偏移、宽度、初始化、更新和清理；
- 参数、返回值、状态码和状态转移；
- 脚本入口与真正引擎消费者的关系；
- 哪些命名有 E2/E3 支持，哪些仍需 `field_XX`/地址名。

先保证语义和命名合理，再追 GCC2.9 生成形状。至少多个独立消费者支持后才提升公共结构体字段；不能因为一个函数的一次访问就定型。

### 3.4 写可读 C 候选

在 `permuter/<fn>/base.c` 或自己的 `.scratch/<agent>/` 中写出人类可读的候选：

- 合理的变量、全局、结构体和字段命名；
- 与调用图、ASM 和数据流一致；
- 保守表达未确定字段；
- 不改变原始访问宽度、指针步长、返回值和副作用。

如果命名/结构体改动会影响已匹配函数，先记录所有消费者并单独验证，不要顺手全局重构。

### 3.5 调整 GCC2.9 形状

语义正确后，才研究：

- 局部变量声明顺序和生命周期；
- `u8/u16/s32` 类型及整数提升；
- 条件极性、switch 源码顺序、共享尾块；
- 指针算术与字节偏移；
- 结构体字段访问和裸偏移的编译差异；
- home/栈槽、callee-saved 寄存器、S-bit 和字面池。

每次只改变一个因素。permuter 是搜索工具，分数不是结论；候选必须保持可读和语义安全。

### 3.6 字节确认和合入

```bash
scripts/fndiff.sh <当前函数名> <候选.c>
scripts/bytecmp.sh <当前函数名> <候选.c> "sym = 0x...;"
```

只有目标函数字节完全一致，并且人工检查通过，才允许替换真实 `src` 中的 `INCLUDE_ASM`。然后：

```bash
python3 scripts/gen_asm.py
python3 scripts/fncheck.py <当前函数名>
```

必要时运行：

```bash
timeout 900 make 2>&1 | tail -3
sha1sum -c ll.sha1
```

头文件、原型、公共结构体、`iwram.h` 或 `linker.ld` 变化后必须强制重编相关翻译单元，避免旧 `.o` 假绿。

## 4. 脚本的正确位置

游戏大量逻辑由脚本触发，但脚本不是替代引擎分析的主线。脚本用于确认：

- 哪个 handler 进入某引擎服务；
- 参数来自哪些脚本字节；
- 返回 0/1 是等待、异步请求还是继续；
- 哪个跨帧状态由下一帧消费。

被脚本调用的地图、角色、对白、DMA、存档、音频和战斗函数仍要按自己的调用图、状态块和数据结构分析。不要因为函数不在脚本 VM 中就把它的语义放弃。

## 5. 卡点处理

连续两轮实验没有减少差异时，停下来判断：

1. 调用约定是否未确认；
2. 全局/结构体字段是否只有单处猜测；
3. 是否缺少间接表或跨帧消费者；
4. 是否是 GCC2.9 的声明顺序、类型提升、home 或寄存器分配问题；
5. 是否应该先处理同一调用子图的姊妹函数。

保存当前最好的人类可读候选，记录精确差异、已排除实验和下一步。语义交接是中间状态，不是最终目标。

## 6. 完工留痕

- 完成匹配：更新 `functions.tsv` 对应地址 status/note，写 `docs/handoffs/<package_id>.md`，必要时追加 `docs/progress.md` 和经验。
- 暂时卡住：status 保持 0；note 写主卡点、候选路径、精确差异和下一实验；handoff 写完整机器契约和命名/结构体证据。
- 文档中明确区分：已验证事实、强推断、工作假设、与旧命名的冲突。
- 最终回复必须说明：当前 C 可读性、全局/结构体命名依据、bytecmp/fncheck、剩余差异、下一步和锁状态。

## 7. 工具边界

- `scripts/claim.sh`：认领和释放。
- `scripts/fndiff.sh`：候选指令形状比较。
- `scripts/bytecmp.sh`：候选字节比较。
- `scripts/fncheck.py`：合入后单函数字节检查。
- `scripts/mkpermuter.py`：建立候选套件。
- `make ctx`/m2c：阅读和候选初稿辅助。
- `gen_asm.py`/`make asm`：生成 ASM，不手改生成物。
- `make` + SHA1：全量终验。

所有工具都服务于同一个闭环，不代表可以跳过语义、命名和可读性检查。

## 9. 文档索引

以下文档只提供工作流程或历史线索，不替代 ROM/ASM/调用点证据：

- `docs/NEXT_PHASE_ROADMAP.md`：75% 之后的语义驱动 GCC2.9 字节匹配路线；
- `docs/AGENT_HANDOFF_PROTOCOL.md`：地址工作包、函数卡、候选、匹配和多 agent 交接格式；
- `docs/AGENT_START_PROMPT.md`：可直接复制给 agent 的统一启动提示词；
- `docs/EXPERIENCE.md`、`docs/progress.md`、`docs/modules/*`、`docs/FAMILIES.md` 等：历史经验和分析线索，使用时必须重新验证；
- `docs/INCIDENTS.md`：历史并发和构建事故线索。

如果旧文档、旧命名或其他 agent 结论与当前 ROM/ASM、调用点或数据访问冲突，以当前原始证据为准。
