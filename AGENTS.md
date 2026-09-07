# AGENTS.md — 开工手册 (每个 agent 必读, 唯一操作权威)

> 项目: 《Lunar Legend (Japan)》GBA ROM 全量反编译。终验 = `make` 后 `sha1sum -c ll.sha1` 通过 (ROM 逐字节一致)。
> 本文件 = 怎么干活; 深度经验 (102 条规律/坑/失败存档) 在 `docs/EXPERIENCE.md`, 索引见文末 §9。
> ⚠ **匹配流程是铁律 6 (permuter 先到分数 0 才许动 src), 违反视为事故**。

## 0. 铁律 (多 agent 并行, 违反 = 事故)

1. **动手前先认领**: `DECOMP_AGENT=<你的名字> scripts/claim.sh <函数名>`; 已被占 (rc=1) 就换一个。
   做完 `--release`, 卡住 `--note "具体卡点"` (转挂起)。`scripts/claim.sh --list` 看全局。
2. **共享文件只追加/只改自己的行** (`functions.tsv` `linker.ld` `iwram.h` `code_0.h` `docs/*.md`);
   编辑前必须 `grep -n` 重读目标行原文, **绝不凭上一轮记忆拼 oldText**。
3. **做完一个函数立刻自证**: `python3 scripts/fncheck.py <func>` (字节级, 不需整 ROM 绿);
   ROM 红了先 `python3 scripts/fncheck.py --blame` 归属, 不是自己的锅就照常提交, **不回退别人的文件**。
4. **禁止**: `cp x.bak x` 整体回退共享文件; `rm -rf .scratch` (用 `.scratch/<agent>/`);
   goto / 固定寄存器 / 内联汇编凑匹配; 给普通 RAM 加 volatile; 手改 `code.s` 和 `asm/{matchings,nonmatchings}/` (全是生成物)。
 5. **开工先 `make` 看尾部有没有报错** (不要只 `sha1sum -c` — 编译失败时 ll.gba 是旧产物, 会误报绿)。
 6. **匹配必须先过 permuter 到分数 0, 未匹配前禁止改 `src/*.c`** — 流程 = 铁律, 不许跳过:
    1. `make ctx` + m2c 把 `asm/nonmatchings/<fn>.s` 转成 C 草稿;
    2. **先建 permuter 草稿**: `scripts/mkpermuter.py <fn>` 生成 base.c (不合入 src);
    3. 自己读反汇编+草稿,**人工分析/优化 C** (类型、访问、调度、语义), 改 base.c;
    4. `permuter.py` 跑分 → **分数=0 才算候选成立** (分数≠0 继续改/重跑);
    5. permuter 抛出的代码**可能不可读/有界外索引/偷改数据流** (经验 18/113), 必须**人工修正为人类代码**;
    6. 修正后**再次跑分, 分数仍=0** 才算真正匹配 (改人类代码可能改变字节, 必须复验);
    7. 全部 OK 才允许合入 `src/<module>.c` 替换 INCLUDE_ASM, 再走 §2 的 fncheck/make/sha1。
    违反: 直接改 src 靠 make 试错, 分数从未到 0 → 视为事故, 回退且不留痕。
 7. **完工必留痕** — 经验没写进文档 = 没做完 (commit message / 聊天不算)。每个 agent 匹配过程中踩的坑、
   发现的规律、遇到的问题, 按下面归位:

   | 内容 | 去处 | 时机 |
   |---|---|---|
   | 该函数一行结论 (✅要点 / ⏸卡点+候选) | `functions.tsv` note 列 + status 翻转 | 每次合入/挂起时, **必写** |
   | 逐函数详细故事、原型修正记录、死路清单 | `docs/progress.md` 追加 | 匹配过程有非平凡发现时 |
   | **新发现的代码生成规律** (可复现模式, 附案例函数地址) | `docs/EXPERIENCE.md` 追加编号经验 | 同一个坑/技巧第二次用到时 |
   | 并发冲突、构建/工具事故 | `docs/INCIDENTS.md` 追加一行 (时间\|事件\|根因\|对策) | 事故发生当场 |
   | 模块语义/数据结构/命名发现 | 对应 `docs/modules/MOD-*.md` | 分析或改名后 |

   写共享文档前 `grep -n` 重读目标区 (铁律 2); 只追加, 不重构别人的段落。
 8. **禁止 `git commit`** — 本流水线自动化提交, agent 只改文件 & 写共享文档, 从不主动 commit/push;
    git 提交由外部流程统一收尾 (重构冻结期/提交窗回归主线是唯一例外, 且由人工重建)。
    git add 仅用于在极少数需要者, 也只在重建/核验后人工执行; 严禁 `git commit`/`git push`。
    违反: 抢占并发窗口造成冲突, 视为事故回退。

## 1. 函数清单: functions.tsv (唯一权威)

```
status isa   module        addr        asm_lines name                 note
1      thumb  code_80002A0  0x080002a0  261       VBlank_UpdateGameScreen
0      thumb  code_8005020  0x080055e8  685       sub_80055E8          ⏸ 612指令+r8/sb; 语义已全解; 待攻
```

- **主键 = addr** (永不漂移); `name` 是缓存, 权威在 `ll.cfg` (改名后 `gen_asm.py --sync` 自动回写)。
 - `status`: 1=已匹配 0=未匹配。`module`: **C 文件** (`src/<module>.c` → 一个 `.o`),
   即翻译单元 (Translation Unit); 文档统一称"C 文件", 命令过滤用 `MODULE=xxx`。
- `asm_lines`: `asm/{matchings,nonmatchings}/<name>.s` 的**主体**行数, 已扣除 3 行头
  (`.syntax unified` / `thumb_func_start` / `标签 @地址`)、尾部空行 + `.syntax divided`、以及紧贴主体的
  尾部 `.align 2, 0`。主体内部的 `.align` 与字面池 `.4byte` 行仍计入 (多段池的函数 `.align` 会落在池前)。
  用于按 §2「优先 ≤60 行小函数」筛目标; 切片缺失为 0, `code.s` 变更后需重跑 `tsv_init.py`。
  注: 主体行数 ≠ 指令数, 每条字面池 `.4byte` 占 1 行但占 4 ROM 字节。
- **`note` (末列) = 完成/挂起明细**, 一行无制表符:
  完成 `✅ 日期 agent: 关键技术点(经验号); fncheck 结果` | 挂起 `⏸ 卡在哪; 最佳候选路径; 详情见 progress.md §xxx`。
  详细故事写 `docs/progress.md`, TSV note 只放一行。
  ⚠ note 可能含字面 `"` (引号) 与尾随 tab — **禁止用 csv.writer 重写 TSV** (会加 CSV 引号/丢尾 tab),
  按行 `split("\t", 5)` + `"\t".join` 处理。
- 常用查询:
  ```bash
  awk -F'\t' '$1==0' functions.tsv | wc -l              # 剩余总数
  awk -F'\t' '$1==0 && $3=="code_80264C0"' functions.tsv   # 某 C 文件 的未匹配
  awk -F'\t' '$1==0 && $5<=60 {print $4,$6}' functions.tsv  # ≤60 行小函数 (挑目标)
  grep -P '\t⏸' functions.tsv                           # 所有挂起项(带卡点)
  python3 scripts/tsv_init.py                            # 从 src 重新推导 (note 按 addr 保留)
  ```

## 2. 标准工作循环

```bash
export DECOMP_AGENT=<name>
scripts/claim.sh sub_XXXXXX                     # 1. 认领
# 2. 选目标: 优先 ≤60 行小函数 / 同构家族(ATLAS §3) / note 无 ⏸ 的; 读 asm/nonmatchings/<fn>.s
#    查符号: 池常量在 iwram.h/ewram.h+linker.ld? bl 目标在 code_0.h 有原型? 缺的先补 (§7)
scripts/fndiff.sh sub_XXXXXX                    # 3. 逐指令形状回环 (候选在 permuter/<fn>/base.c)
scripts/bytecmp.sh sub_XXXXXX <候选.c> "sym = 0x...;"...   # 4. 候选级字节判定 (score 会假高, 以它为准)
# 5. 合入 (仅在铁律 6 分数=0 后): 真 C 替换 src/<module>.c 的 INCLUDE_ASM 行; 原型按反汇编证据写全 (⚠ 别顺手改已有原型签名!)
#         同步: functions.tsv 该行 status 0->1 + 写 note; 新全局登记 iwram.h + linker.ld (§7)
python3 scripts/gen_asm.py                      # 6. 增量重建 asm/ (内容不变不 touch)
python3 scripts/fncheck.py sub_XXXXXX           # 7. 字节定论
timeout 900 make 2>&1 | tail -3 && sha1sum -c ll.sha1   # 8. 全量终验
#    用了 r8/sb/sl 且 make 红在别的函数 = GCC2 泄漏 → 拆 C 文件 (EXPERIENCE 坑1)
# 9. 铁律 8: 禁止 git commit/push — 只改文件&写共享文档, 提交由外部流水线统一收尾
scripts/claim.sh --release sub_XXXXXX           # 10. 释放; 按铁律 6 留痕: TSV status+note / progress / EXPERIENCE / INCIDENTS / modules
```

## 2b. permuter 用法 (匹配必经压分工具, 铁律 6 的主战场)

按铁律 6, 任何函数在动 `src/*.c` 前, 都必须在 permuter 里把分数压到 0。这是验证候选的**唯一门槛**,
不是可选的"压分/探索器"。m2c 出来的 C 只当草稿种子, 后面的分析/优化/修正都在 `permuter/<fn>/base.c` 里做。

四件套目录 `permuter/<fn>/` (缺 `target.o`/`compile.sh` 时 fndiff.sh 会自动生成):

| 文件 | 要求 |
|---|---|
| `base.c` | 单函数自包含 C: 类型 typedef 内联, **不能 include 项目头文件** (permuter 用系统 cpp -nostdinc) |
| `target.o` | 由 `asm/nonmatchings/<fn>.s` 拼 `macros/function.inc` 汇编而成 |
| `compile.sh` | 从任一现有套件 `cp` (与 Makefile 同一套 flag) |
| `settings.toml` | `func_name = "<fn>"` + `compiler_type = "gcc"` |

运行 (**本机沙箱拒绝 multiprocessing, 必须 -j 1**, 建议带 timeout):
```bash
cd permuter/<fn> && timeout 280 ../../.venv/bin/python ../../tools/decomp-permuter/permuter.py . -j 1
```
- 自己分析/优化 base.c (类型、访问、调度、语义), 再跑分。permuter 只探索**语句顺序/括号放置**,
  适合"调度槽位"与"home 互换"类卡点 (实测 sub_8014084 → 经验 87; sub_8007ADC 2685→27B 平台期)。
  **结构/表达式问题它救不了**, 卡 30 分钟就 `scripts/claim.sh --note "具体卡点"` 转挂起换目标。
- 中奖输出 `output-<score>-<n>/source.c`; **分数低 ≠ 对 / 分数=0 也不可直接合入**:
  ① 经验 18/113 可能偷改数据流/有界外索引, 人工核对每条访存并**修正为人类可读代码**;
  ② 修正后**再跑分, 仍=0** 才算真正匹配 (改人类代码可能改变字节, 必须复验);
  ③ 复验通过才 `scripts/fndiff.sh --promote <fn> <winner>.c` 固化, 然后才许按 §2 合入 src。
  全程字节定性以 `bytecmp.sh` / `fncheck.py` 为准 (score 会假高)。
- 挂起函数的最优候选写进 TSV note。套件回收: 只删 [tsv status=1 + src 真 C + 无人认领] 的。

## 3. 硬约束 (ROM 布局相关, 不可违反)

- 函数/数据**顺序即布局**: 不重排函数, INCLUDE_ASM 顺序 = ROM 顺序; linker.ld 符号按地址序插。
- 数据表声明**默认保持 1-D** (经验 99): 改多维会改变消费者索引算术; 逻辑维度写注释。
- 硬件寄存器一律 `include/gba/io.h` 的 `REG_*` 宏 (它们是 `(*(vu16 *)...)` volatile), 禁止裸地址或普通 u16 强转
  (`*(u16 *)0x04000054` ✗ → `REG_BLDY` ✓); **permuter base.c 同理**: 不能 include 头文件时, 把
  `typedef volatile unsigned short vu16;` + `#define REG_x (*(vu16 *)0x...)` 从 io.h 原样内联,
  否则是在给另一个程序打分 (经验 96)。DMA/串行口先查 `include/gba/macro.h`
  的 `DmaSet/DmaCopy*/CpuSet` 宏 (经验 55 — 目标里"连续同基址 str + 一次空读"就是宏展开形状)。
- ⚠ 寄存器号/通道别凭记忆: `0x040000B0`=DMA0SAD 不是 DMA3; `0x04000130`=REG_KEYINPUT 不是定时器。先 grep io.h。
- volatile 只给真实 IO; u16 寄存器字节读用 `*(vu8 *)&REG_x`。

## 4. 工具速查

| 命令 | 用途 |
|---|---|
| `scripts/claim.sh <fn>` / `--list` / `--release` / `--note "..."` | 认领锁 (.claims/) |
| `scripts/fndiff.sh <fn> [cand.c]` | 单函数逐指令形状 diff (+`--promote` 固化) |
| `scripts/fncheck.py <fn...>` | 已合入真身的字节定论; `--blame` 差异归属到 .o |
| `scripts/bytecmp.sh <fn> <cand.c> "sym = 0x...;"...` | 候选级字节判定 (部分链接施加池重定位) |
| `python3 scripts/audit.py` | 函数清单体检: TSV×ll.cfg 漂移 + status=1 全量字节核验 + note 覆盖 + C 文件 新鲜度 |
| `python3 scripts/gen_asm.py [--sync] [--dry-run]` | TSV+ll.cfg+code.s → asm/ (增量幂等) |
| `python3 scripts/tsv_init.py` | 从 src/*.c 重新推导 functions.tsv (note 保留) |
| `scripts/rename_fn.sh <old> <new>` | **全链改名一条命令** (ll.cfg→code.s→引用点→gen_asm→fncheck, 失败自动回滚) |
| `scripts/match_fn.sh <fn>` | **合入收尾一条命令** (校验真C→gen_asm→重编→fncheck→SHA1→打印收尾清单) |
| `scripts/mkpermuter.py <函数名或0x地址>` | 一键建 permuter 套件: 从 asm/{non,}matchings 拷 .s 出 target.o, 把 src 里的真 C 函数体原样拷成 base.c 种子; INCLUDE_ASM 占位自动跳过 |
| `make code.s` / `make asm` | 重出反汇编 / 增量重建 asm/ |
| `make verify` | 全量终验: make+SHA1+audit+status=1 全量字节核验 |
| `make remaining [MODULE=xxx]` | 未匹配报表 (scripts/remaining.py) |
| `python3 scripts/gen_reports.py` | 重生成 docs/reports/remaining.md (Markdown; 名称列 Ctrl+Click 跳 asm 切片, permuter 列跳套件目录, 行数<200 前缀 ▶; 全量清单看 functions.tsv) |
| `make ctx` | 重生成 m2c 上下文 (头文件改后) |
| `tools/gbadisasm/gbadisasm baserom.gba -c ll.cfg > code.s` | 重出反汇编 (改名管线第 2 步) |
| `arm-none-eabi-cpp ... && agbcc -dl` | RTL/寄存器分配转储 → gccdump.lreg (查 home, EXPERIENCE §诊断) |

## 5. m2c 初转 (大函数起手)

```bash
make ctx
.venv/bin/python tools/m2c/m2c.py -t gba --context ctx.c -f <func> /tmp/opencode/<func>.s
```
- 必须 `-t gba`; 喂 `asm/nonmatchings/<fn>.s` 单函数文件 (不能喂 code.s)。先转字面池写法:
  ```python
  import re
  src = open('asm/nonmatchings/<func>.s').read()
  src = re.sub(r'^\tldr (r\d+), _[0-9A-F]+ @ =(0x[0-9A-F]+)$', r'\tldr \1, =\2', src, flags=re.M)
  src = re.sub(r'^\t_[0-9A-F]+: \.4byte .*$\n?', '', src, flags=re.M)
  src = src.replace('\t.align 2, 0\n', '')
  ```
- `M2C_ERROR(...)` = 它没看懂的寄存器流, 是重要线索 (例: "死代码" flag 实为第三实参)。
- ≤60 行函数直接手读反汇编更快, 别开 m2c。

## 6. 编译选项 (固定, 勿改)

`-mthumb-interwork -Wimplicit -Wparentheses -O2 -fhex-asm -fprologue-bugfix` (agbcc, 统一用它, 禁 old_agbcc)。
怎么写都不匹配时可试带 -g 变体: `-mthumb-interwork -Wparentheses -Werror -O2 -g -fprologue-bugfix` (改变寄存器分配/保活)。

## 7. 符号登记与改名

**新 RAM/ROM 符号** (匹配函数时发现池常量):
- IWRAM/EWRAM: `iwram.h`/`ewram.h` 加 extern + `linker.ld` 对应 section 内 `. = 0x0000XXXX; sym = .;` **按地址序插** (插错 = `cannot move location counter backwards`)。
- ROM 绝对符号: `linker.ld` 的 `SECTIONS {}` **外面** `gUnk_08XXXXXX = 0x08XXXXXX;` (放里面会叠加段基址污染池值)。
- 同地址多类型视图 → 起别名符号 (也在 SECTIONS 外)。

**函数改名全链** (细节与坑见 EXPERIENCE.md「符号改名管线」):
```bash
sed -i 's/\bOldName\b/NewName/g' ll.cfg include/code_0.h src/*.c   # TSV 不用改 (addr 主键)
tools/gbadisasm/gbadisasm baserom.gba -c ll.cfg > code.s
python3 scripts/gen_asm.py --sync                                  # 重切 asm/ + 回写 TSV 缓存名
make ctx && timeout 900 make 2>&1 | tail -3 && sha1sum -c ll.sha1
```
- ⚠ **改名不得顺手改原型签名**: K&R `void f();` 改全原型会给调用点加截断、破坏已匹配字节 (2026-09-01 实际踩过, 5 字节)。
- 未匹配函数想用语义名: 先 `#define 语义名 sub_XXXX` 别名 (asm 侧符号不动), 匹配后再走真改名。
- 数据符号改名零链接风险 (asm 按硬码地址引用, 不按名字)。

## 8. 命名规范

语义名走 `ll.cfg` 真改名全链; RAM/ROM 符号 `g` 前缀, 函数 PascalCase;
模块前缀跟所在子系统 (Stats_/Chara_/Party_/AnimSlot_/Obj_/MapZone_/Op_/Save_/MenuUi_/Text_...) — 查 `docs/modules/MOD-*.md` 已有命名再定。

## 9. docs/ 索引 (按需读, 不要全量读)

| 文件 | 内容 | 什么时候读 |
|---|---|---|
| `docs/EXPERIENCE.md` | **102 条代码生成规律** + 寄存器诊断 + 失败案例存档 + 坑 + 改名管线全文 + 硬件寄存器全规范 | 写 C 前查同族规律; 卡寄存器必看经验 29/51/54/76/87/88/102 + 失败存档 |
| `docs/INCIDENTS.md` | 并发/构建事故记录 (追加式) | 发生并发冲突/想改共享文件前 |
| `docs/progress.md` | 逐函数匹配故事/原型修正表 (追加式) | 接手挂起函数前 |
| `docs/modules/README.md` | MOD-01..09 物理分段索引 + 分析状态 | 选目标前 |
| `docs/modules/MOD-*.md` | 各模块语义分析+函数清单 | 命名/定原型前查所在模块 |
| `docs/ATLAS.md` | 自动生成的模块聚类/同构家族 (§3) | 找同族套模板 (`atlas.py` 可重生成) |
| `docs/FUNCTIONAL_MAP.md` | F1-F9 功能架构视图 | 理解系统边界/命名域 |
| `docs/ROUTES.md` | 已打通语义路线 (道具→装备→存档) | 涉及背包/属性/存档命名 |
| `docs/PLAN_DATA.md` | 数据区 de-blob/符号化计划 | 做数据侧工作时 |
| `docs/RULES_HARDWARE_IO.md` | **GBA 硬件寄存器与 DMA 操作代码规范** (禁止裸指针, 必用 REG_* 与 DmaCopy 宏) | 涉及硬件 IO/DMA 访问时必查 |
| `docs/REFACTOR_PLAN.md` | 管线重构进度 (R0-R7) + 待拍板方案 | 继续重构时 |
