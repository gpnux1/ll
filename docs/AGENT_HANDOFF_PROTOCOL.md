# 多 Agent 交接协议

> 版本：2026-09-12  
> 适用项目：Lunar Legend (Japan) GBA ROM 全量反编译  
> 目标：让后继 agent 能从地址、调用图、语义模型和可读 C 候选继续推进，最终完成 GCC2.9 风格字节匹配。

## 1. 交接对象

交接的对象不是整个 C 文件、旧模块或一份历史文档，而是：

```text
地址函数/同构族
 + 调用子图
 + 全局/结构体/状态模型
 + 当前人类可读 C 候选
 + 精确字节差异
 + 下一实验
```

语义分析不是最终交付；它必须服务于可读、合理命名且 byte-exact 的 C。

## 2. 事实优先级

- **E0**：原始 ROM 字节、原始数据表、函数指针/任务/对象表、脚本正文。
- **E1**：按当前 ROM 和地址生成的 ASM。
- **E2**：多个调用者/消费者对参数、返回值、字段和状态的一致证据。
- **E3**：受控运行、调试器、模拟器、脚本样本和跨帧观察。
- **H**：旧文档、旧命名和其他 agent 结论，只能帮助定位下一处 E0/E1 证据。

所有语义和命名都要说明证据等级。H 与 E0/E1/E2 冲突时，以原始证据为准。

## 3. 工作包边界

每个包必须以地址定义：

```text
package_id: ENGINE-HUB-20260912-a
function_addrs: 0x080xxxxx, 0x080yyyyy
entry_roots: direct/task-table/vblank/object-table/opcode/unknown
```

附加边界：

- callers/callees；
- 间接表地址、槽位和目标；
- 共享全局/结构体字段范围；
- 跨帧生产者和消费者；
- 当前 C 文件仅作为物理位置。

不能以“整个 event_hub.c”“battle 模块”“所有 script_vm 函数”作为唯一任务边界。

## 4. 每个包必须回答的问题

### 4.1 机器契约

```text
address:
current_name: 检索标签
physical_tu: 物理归属
callers:
callees:
indirect_tables:
args: 寄存器/栈位置、宽度、用途
return: 返回寄存器/标志、调用者用途
memory_reads:
memory_writes:
persistent_state:
state_transitions:
script_relation:
```

### 4.2 语义和命名

```text
function_role:
globals:
struct_or_object:
field_evidence:
state_lifecycle:
chosen_names:
name_confidence: E0/E1/E2/E3/H
old_name_conflicts:
unknowns:
```

### 4.3 C 和匹配状态

```text
readable_candidate:
candidate_path:
candidate_is_human_readable:
bytecmp:
fndiff/permuter:
first_unresolved_difference:
failed_experiments:
next_experiment:
```

如果还没有候选，明确写“候选尚未开始”，并说明是因为语义、接口还是数据模型未稳定。不要把语义分析写成终点。

## 5. 工作阶段

工作包可以处于以下阶段，阶段不是互相割裂的 agent 类型：

```text
NEW             尚未初勘
CONTRACT        已建立 ASM/调用图/参数机器契约
MODEL           已建立语义、全局、结构体和状态模型
CANDIDATE       已写出人类可读 C 候选
MATCHING        正在调整 GCC2.9 生成形状和做 bytecmp
MATCHED         C 可读、命名合理、byte-exact、fncheck 通过
BLOCKED         明确缺少外部证据或共享接口
READY-HANDOFF   当前阶段资料完整，可由后继者继续
```

不要把 `MODEL` 当成最终完成；它是通往 `CANDIDATE`、`MATCHING` 和 `MATCHED` 的中间阶段。

## 6. 开工和认领

```bash
export DECOMP_AGENT=<稳定名称>
git status --short --branch
scripts/claim.sh --list
rg -n '<地址或当前名>' functions.tsv ll.cfg
scripts/claim.sh <当前函数名>
```

认领前确认：

- 地址、ASM、status 和真实源码位置；
- 直接/间接调用入口；
- 当前 owner；
- 是否有已有 handoff。

一次只认领一个函数；只有真正共享候选和交付物的同构族才批量认领。认领不包括整个 C 文件、公共头文件、所有全局符号或全局 build。

## 7. 正确工作循环

1. 重新读取 ROM/ASM 和调用点，不信任旧命名。
2. 建立机器契约。
3. 沿调用图确认全局、结构体、状态和脚本/任务/VBlank关系。
4. 建立合理命名和证据等级。
5. 写人类可读 C 候选。
6. 用 GCC2.9 形状实验处理声明顺序、类型提升、home、寄存器、switch 和字面池。
7. `fndiff`/`bytecmp` 验证。
8. 候选可读、数据流正确且 byte-exact 后才合入真实 C。
9. `fncheck`，必要时全量 `make`/SHA1。
10. 更新 handoff、`functions.tsv` note 和必要的进度记录，释放锁。

permuter 只探索生成形状，不能替代语义分析、可读性检查或 bytecmp。

## 8. 卡住时必须保存的内容

- 当前机器契约；
- 当前语义模型和不确定性；
- 当前最好的人类可读候选；
- 全局/结构体/命名依据；
- 精确 bytecmp/fndiff 结果；
- 第一处未解决差异；
- 已排除实验及原因；
- 下一条实验；
- 共享原型、全局符号、linker 和源码影响。

连续两轮实验没有减少差异时，停止盲调，转向同一调用子图的姊妹函数或缺失的数据证据。

## 9. 交接文件

放在版本控制的：

```text
docs/handoffs/<package_id>.md
```

最小模板：

```markdown
# Handoff <package_id>

- 阶段: NEW / CONTRACT / MODEL / CANDIDATE / MATCHING / MATCHED / BLOCKED / READY-HANDOFF
- 创建时间:
- 最后更新:
- 原 agent:
- 当前 agent:
- 函数地址:
- 当前函数名: 检索标签
- 当前 C 文件: 物理标签
- 证据基线: git status/commit/dirty 说明

## A. 任务边界
- 目标:
- 不包含:
- 依赖地址:

## B. 机器契约
- callers:
- callees:
- tables:
- args/return:
- reads/writes:
- persistent state:
- script/task/vblank/object relation:

## C. 语义和命名
- function role:
- globals:
- struct/object fields:
- chosen names and confidence:
- old-name conflicts:
- unknowns:

## D. C 候选和匹配
- readable candidate:
- candidate path:
- candidate safety review:
- bytecmp:
- fndiff/permuter:
- first unresolved difference:
- failed experiments:
- next experiment:

## E. 共享影响
- src function region:
- code_0.h prototype/callers:
- iwram.h/linker.ld:
- affected matched functions:
- forced rebuild needed:

## F. 验证
- fncheck:
- make:
- sha1sum -c ll.sha1:
- audit:

## G. 恢复记录
- YYYY-MM-DD agent: 重核事实、当前阶段、下一动作。
```

不能只写“语义已分析”“属于某系统”“候选在 ignored 目录”“分数很低”。

## 10. 共享文件规则

- `src/*.c`：只改自己函数区段。
- `functions.tsv`：按地址独占行，编辑前重读并保留列数/尾 tab。
- `code_0.h`：原型变化记录全部调用点。
- `iwram.h`/`linker.ld`：按地址定点修改，记录所有消费者。
- `asm/`、`code.s`、`ll.map`：生成物，不手改。
- `docs/handoffs/`：追加恢复记录，不覆盖前任事实。
- `permuter/<fn>/`：一个 suite 同时只有一个实验。
- `.scratch/<agent>/`：私有临时目录，重要结论复制到 handoff。
- `build/`、`ll.gba`、`ll.elf`：共享状态，不并发 clean/make。

## 11. 后继者恢复顺序

1. 记录 `git status --short --branch`。
2. 按地址查 `functions.tsv`、`ll.cfg`、ASM、src 和锁。
3. 读取 handoff，重新核对 E0/E1/E2 事实。
4. 恢复当前阶段：先机器契约，再模型，再候选，再匹配。
5. 重现候选或分析命令。
6. 从第一处未解决差异继续，每次只改一个因素。
7. 追加恢复记录，不覆盖前任。

## 12. 完成定义

`MATCHED` 只有在以下条件全部满足时成立：

- 调用图和机器契约没有已知矛盾；
- 全局、结构体、参数和状态命名有证据支持；
- C 人类可读；
- 没有错误指针步长、无界索引、伪造数据流或禁止技巧；
- `bytecmp`/`fncheck` 字节完全一致；
- 必要时全量 `make` 和 SHA1 通过；
- handoff 和共享影响已记录；
- 锁已释放。

最终标准只有一个：**人能读懂，命名合理，语义可信，GCC2.9 风格编译后与原 ROM 完全一致。**
