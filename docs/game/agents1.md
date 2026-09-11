# AGENTS1.md — 游戏流程分析与可读性重构交接手册

## 0. 文档定位

本文件服务于“已经匹配好的函数分析、语义命名、注释补全和模块重构”工作。
项目根目录的 `AGENTS.md` 仍是反编译匹配流水线的最高约束，尤其是函数认领、
permuter、`fncheck.py`、构建和禁止提交等规则。本文件不替代它，也不允许跳过
字节级验证。

本工作流的目标不是重新反编译所有函数，而是：

1. 从入口和调用者证据理解已经匹配的函数。
2. 把 `sub_XXXXXXXX`、`gUnk_...`、`field_...` 改成可验证的语义名称。
3. 在不破坏 ROM 字节和地址布局的前提下补充注释、枚举和结构体。
4. 重新评估 C 文件边界，提出可以交给后续 agent 的物理拆分方案。
5. 把分析结果统一写入 `docs/game/*.md`。

用户明确要求：本工作流新写的游戏分析不要修改已有 `docs/*.md`、
`docs/modules/*.md` 或其他历史报告；新内容写入 `docs/game/`。已有文档只能作为
参考，不能当作未经复核的事实。

## 1. 适用范围与规则优先级

本手册适用于 `functions.tsv` 中 `status=1`、源码已经替代 `INCLUDE_ASM` 且能通过
`fncheck.py` 的函数。工作对象包括调用链、状态机、数据结构、命名、注释以及
C 文件职责。以下任务不属于本手册：

| 情况 | 处理方式 |
|---|---|
| 函数仍为 `status=0` 或源码仍是 `INCLUDE_ASM` | 回到根 `AGENTS.md` 的匹配流程，必须先用 permuter 得到 score 0。 |
| 已匹配函数只做注释、局部变量或等价符号清理 | 可按本手册操作，但仍须函数级和全 ROM 验证。 |
| clean code 改变表达式、控制流、类型、原型或内联关系 | 先在源文件外验证候选字节一致，再落入 `src/`。 |
| 物理移动函数、拆分 C 文件或拆分数据对象 | 视为高风险布局变更，单独认领、单独验证、单独写交接报告。 |
| 希望顺手匹配调用链中的未匹配函数 | 另起匹配任务，不能把分析任务当作绕过 permuter 的理由。 |

根 `AGENTS.md` 对并发、生成物、硬件寄存器、ROM 布局、构建和禁止提交的约束
继续有效。本手册只替换“任务目标和分析方法”，不放宽以下底线：

- 不手改 `code.s`、`asm/matchings/` 或 `asm/nonmatchings/`。
- 不提交、不 push，不回退其他 agent 的修改。
- 不用 `volatile`、固定寄存器、goto 或内联汇编包装可读性问题。
- 不改变函数和数据的链接顺序，除非当前任务就是经过验证的物理拆分。
- 不把旧文档中的推测直接升级为符号名；必须重新回到源码、汇编和调用者取证。

## 2. 证据等级与命名门槛

语义判断至少要同时回答“谁调用、读写什么、状态如何转移”。证据按可靠程度分为：

| 等级 | 证据 | 可以支持的改动 |
|---|---|---|
| A | 调用者/被调者闭环、明确状态转移、数据地址与画面行为一致 | 正式函数名、全局名、枚举成员名和流程结论。 |
| B | 多个静态调用点一致，访存宽度/偏移明确，但缺少运行画面或完整生产者 | 保守语义名、结构字段名，并在文档注明边界。 |
| C | 单一调用点、字符串邻接、截图相似或地址邻接 | 只记录候选名，不执行全链改名。 |
| D | 旧文档、旧文件名、旧 `gUnk` 别名或主观猜测 | 仅作为检索线索，不能作为结论。 |

截图用于确认玩家可见状态，不能独立证明函数职责。地址连续性用于判断原始链接
布局，不能独立证明模块归属。一个变量在两个生命周期中有不同用途时，应记录
同址视图和生命周期，不能只选择其中一个用途覆盖全部消费者。

分析状态机时，至少记录：状态存储变量、每个已确认值、进入条件、逐帧行为、
退出条件、下一状态、输入门控和产生的副作用。分析结构体时，至少记录：基址、
访问宽度、字段偏移、数组步长、生产者、消费者和生命周期。

## 3. 标准工作循环

### 3.1 开工基线

先确认工作树和构建基线，区分自己的任务与并发修改：

```bash
export DECOMP_AGENT=<agent-name>
timeout 900 make 2>&1 | tail -3
git status --short
scripts/claim.sh --list
awk -F'\t' '$6=="<FunctionName>" {print}' functions.tsv
scripts/claim.sh <FunctionName>
```

只有 `status=1` 的函数才进入本流程。认领使用 `ll.cfg`/`functions.tsv` 当前的规范
名；若任务中改了函数名，结束时仍要释放最初的 claim key。纯只读分析可以不占用
源码函数，但动 `src/`、原型、全局声明、链接符号或函数真名之前必须认领。

基线 `make` 已红时先运行 `python3 scripts/fncheck.py --blame`。若差异属于其他
agent，记录现状并限制验证范围，不回退；若正好落在待分析对象上，先停止源码
清理，避免在未知基线上叠加变化。

### 3.2 建立证据包

不要从函数体孤立猜名字。每个分析单元至少检查以下内容：

1. `functions.tsv` 的地址、状态、C 文件和历史 note。
2. 已匹配 C 与 `asm/matchings/<FunctionName>.s`，确认 C 注释没有背离指令事实。
3. 所有直接调用者、被调者、函数指针表和状态分发表。
4. 读写全局的其他生产者/消费者，以及 linker 中的真实地址和对象宽度。
5. 相关资源表、字符串、存档布局、脚本入口和硬件寄存器。
6. `Screenshots/` 中对应画面；需要时按状态逐帧对照，不凭单张图推导时间顺序。

常用只读命令：

```bash
rg -n "<symbol>" src include linker.ld ll.cfg functions.tsv
rg -n "bl <FunctionName>|=<address>" asm code.s
sed -n '<start>,<end>p' src/<module>.c
python3 scripts/fncheck.py <FunctionName>
```

先在 `docs/game/<TOPIC>.md` 写出调用链、状态表和未决问题，再决定是否改名。
文档中的每个重要结论应能回指到地址、函数、字段偏移或截图状态之一。

### 3.3 按风险分层清理

| 层级 | 改动 | 落盘前要求 |
|---|---|---|
| L0 | 新增 `docs/game/*.md` | 检查路径、链接和事实一致性。 |
| L1 | 注释、局部变量名、参数名、无语义变化的格式整理 | 认领函数；改后 `fncheck`。 |
| L2 | 枚举常量、结构体字段、表达式、控制流、static inline 辅助 | 在临时候选中先证明目标函数字节不变。 |
| L3 | 函数真名、全局真名、原型、链接符号 | 检查全部消费者；执行全链同步和受影响函数验证。 |
| L4 | 拆函数、移动函数、改 C 文件/对象边界、移动 `.rodata` | 先写边界方案；按整个受影响翻译单元验证。 |

L1 不应改变声明类型、语句顺序、作用域、常量写法或表达式括号。L2/L3 的“更
漂亮”不构成修改理由，必须带来准确语义或消除真实误导。若候选改变了任何目标
字节，应先退出源码、在临时副本定位原因；不能把已匹配函数留成回归状态。

### 3.4 代码生成敏感改动

枚举、结构体、原型和辅助函数都可能改变 agbcc 的整数提升、参数截断、别名分析、
寄存器 home 或内联结果。此类改动先复制目标函数为源文件外候选，用
`scripts/bytecmp.sh` / `scripts/fndiff.sh` 验证，确认字节完全一致后再用
`apply_patch` 修改正式源码。若修改后的可读版本不能维持 score 0，保留原 C，
把语义放进注释和 `docs/game/`，不要牺牲匹配度。

特别注意：

- K&R 风格的 `void Fn();` 不能仅因已推断参数就直接改成完整原型；调用点可能
  因新原型产生截断或符号扩展。
- 枚举常量可以命名状态值，但 byte-sized IWRAM 状态仍应保持 `u8` 存储；不能
  默认把全局改成编译器宽度的 `enum` 对象。
- 结构体必须按 `ldrb/ldrh/ldr` 和实际步长设计，不能添加隐式 padding 或把
  byte field 合成 bitfield。
- 抽取 helper 只有在原编译器继续完全内联且所有调用函数字节不变时才成立；
  新增真实 `bl` 的“拆函数”不是等价 clean code。

### 3.5 收尾

完成后更新当前主题的 `docs/game/*.md`，写明确定事实、保留疑点、改名映射、
受影响文件和验证命令。不要向旧 `docs/progress.md`、`docs/modules/*.md` 或其他
历史文档追加本流程结果。最后释放 claim：

```bash
scripts/claim.sh --release <original-claim-name>
```

## 4. 当前标题流程基线

标题相关的核心调用链：

```text
__start
  -> AgbMain
    -> System_Init
    -> gMainLoopCallbacks[gMainLoopMode]
      -> Task_DispatchGameState
        -> gGameStateCallbacks[gGameState]
          -> Task_TitleMenuFrame
            -> TitleMenu_ProcessFrame
              -> TitleMenu_UpdateUi
```

五张截图的对应关系：

| 文件 | phase | page | 画面 |
|---|---:|---:|---|
| `Screenshots/ll_000.png` | 8 | 0 | Lunar 标题和 `PRESS START BUTTON` |
| `Screenshots/ll_001.png` | 0（attract） | 0 | 无操作超时后的剧情介绍 |
| `Screenshots/ll_002.png` | 9 | 1 | `NEW GAME`、`LOAD GAME`、`OPTION` |
| `Screenshots/ll_003.png` | 14 | 2 | 三个存档槽的 `LOAD` 页面 |
| `Screenshots/ll_004.png` | 20 | 3 | `OPTION`：BGM、SE、CARD、GALLERY |

启动链见 [`BOOT_TO_TITLE_FLOW.md`](BOOT_TO_TITLE_FLOW.md)，标题状态机见
[`TITLE_MENU_PROCESS_FRAME.md`](TITLE_MENU_PROCESS_FRAME.md)。

关键事实：

- `gScenePhase` 是标题/子菜单总状态机；`gMenuCursorGrp` 是
  `TitleMenu_UpdateUi` 的页面分派值，不能混用。
- phase 8/page 0 检查 `A_BUTTON | START_BUTTON`。
- `gTitleAttractCountdown` 初值为 16，page 0 每个主循环帧消耗一次，16 帧后进入
  phase 21；按 GBA 约 59.7 Hz 计算约 0.27 秒。`gSceneLoadToggle` 的低位只负责
  提示闪烁和 UI 更新节奏。
- attract 保留 `gTitleIntroState != TITLE_INTRO_DISABLED`，所以
  `NewGame_Init` 装载 script set 0；普通 NEW GAME 清除该标记并装载 script set 1。
- `Task_MapExplore` 在 attract 期间发现输入会 `Script_Abort(1)` 并回到标题，
  不会把演示输入误认为玩家正式开始游戏。

## 5. 已采用的安全语义命名

### 函数

| 地址 | 旧名 | 当前名 | 证据 |
|---:|---|---|---|
| `0x08001128` | `System_Init` | `System_Init` | 冷启动、IRQ、显示、声音和菜单槽初始化 |
| `0x0800128c` | `AgbMain` | `AgbMain` | 主循环入口 |
| `0x080031e4` | `Task_TitleMenuFrame` | `Task_TitleMenuFrame` | 标题状态帧任务 |
| `0x08011454` | `sub_8011454` | `TitleMenu_ProcessFrame` | 标题/读档/Option/图鉴/SIO 总状态机 |
| `0x08012790` | `sub_8012790` / `SaveUi_LoadScreen` | `TitleMenu_UpdateUi` | page 0–5 的逐帧 UI 更新器 |
| `0x08013870` | `sub_8013870` | `TitleMenu_DrawOptions` | 标题 tilemap 和主菜单文字 |
| `0x08013934` | `sub_8013934` | `OptionsMenu_DrawEntries` | Option 四项和 BGM/SE 数值 |
| `0x0800e668` | `sub_800E668` | 暂保旧名 | 菜单页面切换和 UI 游标插值，需全局调用者复核后再改 |

函数改名必须同时更新 `ll.cfg`、调用点、原型、生成的 asm 切片和
`functions.tsv` 缓存名。优先使用项目提供的 `scripts/rename_fn.sh`，不要手改
`code.s` 或 `asm/{matchings,nonmatchings}` 生成物。改名前先检索新名是否已经
存在，并保存旧地址用于核对；改名后地址必须保持不变。

### 全局变量

| 地址/旧名 | 当前名 | 证据和注意事项 |
|---|---|---|
| `gMainTasks` | `gMainLoopCallbacks` | `AgbMain` 的顶层循环回调表 |
| `gMainTaskSlot` | `gMainLoopMode` | 普通游戏/战斗顶层循环选择值 |
| `gUnk_087E83F8` | `gGameStateCallbacks` | `gGameState` 索引的状态回调表 |
| `gMainGameState` | `gGameState` | 普通游戏状态 |
| `gSceneSubState` | `gScreenTransitionState` | 屏幕转场进行标志，不是场景子状态 |
| `gCutsceneActive` | `gTitleIntroState` | Logo 序号和 attract 分流标志 |
| `0x03000234` | `gTitleAttractCountdown` | PRESS START 无操作计时器，旧名 `gTitleFadeStep` |
| `0x03000230` | `gSoundTestSfxId` | Option 声音测试值，类型为 u16 |
| `0x03000232` | `gSoundTestBgmId` | Option BGM 选择值 |
| `0x03000233` | `gSoundTestPlayingBgmIdPlusOne` | 正在播放的 BGM 编号加一，0 表示停止 |

同址旧名宏/链接别名应保留，直到所有调用者和相关数据生命周期都确认完毕。对于
一个地址被不同子系统以不同视图访问的情况，应记录“视图”而不是强行声明两个
物理数组。例如 `0x08089B90` 既参与存档解锁扫描，也参与卡片背景索引，不能
未经数据边界证据就拆成独立的 `gGallerySceneIds`。

### 类型

- `enum MainLoopMode`：普通游戏与战斗顶层循环。
- `enum GameState`：地图、标题、文本、战斗等游戏状态。
- `enum TitleIntroState`：GAME ARTS Logo、ESP Logo、attract、disabled。
- `enum TitleMenuPhase`：phase 0–38 的标题/读档/Option/图鉴/SIO 状态。
- `enum TitleMenuPage`：`gMenuCursorGrp` 的 page 0–5。
- `CardExchangePacket`：SIO 卡片 ID 的低字节、高字节及保留半字。

枚举值必须与汇编比较常量一一对应；不能为了好看重新编号。结构体字段必须有
访存偏移证据，不能仅凭字段名称或一张截图猜测。

## 6. C 文件边界判断

### 当前文件的真实职责

| 当前文件 | 实际覆盖 | 语义评价 |
|---|---|---|
| `src/engine_core.c` | AgbMain、System_Init、IRQ/VBlank、显示、软复位、按键、主循环 | 名称偏窄但职责连续，候选新名 `system_main.c` |
| `src/scene_mgr.c` | NewGame_Init、地图进入/恢复、战斗状态、场景转换、地图探索 | 不只是 scene manager，候选新名 `game_state_scene.c` |
| `src/menu.c` | 标题、读档、Option、图鉴、卡片交换、背包/HUD、数字/文字辅助 | `menu.c` 比 `save_menu.c` 或 `title_menu.c` 更诚实，但仍可细分 |
| `src/sprite_engine.c` | 精灵图形、动画槽、OAM、角色渲染，夹有状态分发薄包装 | 地址 `0x08003088..0x08003208` 应独立为 game state 区 |
| `src/data_87E83F0.c` | 状态回调表和相邻只读数据 | 数据职责混合，不能只按现有文件名理解 |
| `src/data_805769C.c` | 地图/存档/菜单/动画/压缩资源和多种表 | 按地址块和消费者逐步拆，不一次性重构 |

### 推荐的目标模块

这是语义目标，不代表可以立即移动源码：

```text
system_main.c       : AgbMain、System_Init、IRQ/VBlank、显示和按键
game_state.c        : Task_DispatchGameState、Task_TitleMenuFrame、状态回调包装
scene_flow.c        : NewGame_Init、地图/战斗/场景状态转换
title_menu.c        : TitleMenu_ProcessFrame、TitleMenu_UpdateUi、标题/读档/Option
menu_common.c       : 公共游标、文字、数字、窗口和菜单实体辅助
gallery_card.c      : 卡片图鉴、详情、SIO 卡片交换
sprite_runtime.c    : 精灵图形、动画、OAM 和 Actor 显示运行时
game_state_data.c   : gMainLoopCallbacks、gGameStateCallbacks 和相关表
menu_text_data.c    : 标题、Option、Load 和 Gallery 文字表
```

### 为什么当前不直接拆

物理拆分翻译单元可能改变：

1. agbcc 的全局寄存器分配、callee-saved 寄存器和局部变量 home。
2. `src/*.o` 内函数排列、`.rodata` 顺序和 linker 地址。
3. 尚未匹配函数依赖的原型、静态辅助函数和字面池位置。

因此当前做法是先在原文件内完成函数级语义改名和注释，再用独立文档保存目标
边界。只有一个连续地址段的函数全部有 score 0 候选、并且调用者影响已验证后，
才进行物理移动。

## 7. 安全拆分流程

任何后续 agent 想把函数移动到新 C 文件，都按以下顺序执行：

1. 阅读本文件和 `docs/game/C_FILE_MODULE_BOUNDARIES.md`，确认目标函数的地址
   连续性、调用者和数据依赖。
2. 记录拆分前 `make`、目标函数 `fncheck.py`、ROM SHA1 和对象大小。
3. 在新文件保留原函数顺序、原型风格、编译选项和必要的静态辅助函数。
4. linker 中只调整对象归属，不重排 ROM 地址；必要时显式固定 `.text` 顺序。
5. 先编译目标对象并用 `fncheck.py --blame` 确认差异归属。
6. 对所有受影响的已匹配函数逐个 `fncheck.py`，再运行 `make verify`。
7. 失败时记录“哪一个翻译单元边界导致哪一组寄存器/地址变化”，不要整体回退
   其他 agent 的工作。
8. 在 `docs/game/` 新增交接记录，包含旧文件、新文件、函数地址、验证结果和
   是否保留兼容别名。

禁止为了目录整洁把未匹配函数直接从 `INCLUDE_ASM` 批量迁移；禁止把多个逻辑
视图的数据复制成两个定义；禁止用 `volatile`、固定寄存器、goto 或内联汇编
伪造匹配。

“拆 C 文件”和“拆一个大函数”是两种风险。前者主要影响对象顺序、字面池和
翻译单元代码生成；后者还会引入 ABI、调用开销和新的符号。大状态机优先通过
枚举、局部变量、分段注释和流程文档做逻辑拆分。只有目标汇编原本就存在调用，
或抽出的 `static inline` 能维持所有字节时，才物理拆成 helper。

## 8. 命名与注释规则

### 函数

- 使用 PascalCase，前缀体现子系统：`TitleMenu_`、`Save_`、`Scene_`、
  `Map_`、`Sprite_`、`Script_`、`Bgm_`。
- 名称只描述已经有调用者/访存证据的语义。
- 只知道“更新某个页面”时用 `Update`，只知道“绘制文字”时用 `Draw`，不要把
  一次性初始化函数命名成 `Update`。
- 对仍有歧义的函数保留 `sub_`，在本文件记录候选名和证据。

### 全局变量

- RAM 全局以 `g` 开头；数组元素宽度和生命周期要写进注释。
- 硬件寄存器使用 `REG_*`，不能用裸地址替代。
- 对同址不同用途使用 `gXxxView`、`gXxxSnapshot` 或兼容宏，而不是制造重复
  的存储定义。

### 局部变量

优先使用 `cursor`、`optionIndex`、`slotIndex`、`label`、`tilemapCursor`、
`messageCursor`、`countdown` 等表达数据流的名字。循环变量只有在用途确实相同
时才复用 `i`；不同循环用 `slotIndex`、`pageIndex` 等区分。

同一个反编译临时变量承载多个互不相关的寄存器生命周期时，不要强行取一个宽泛
业务名。可以在不改变作用域和代码生成的前提下按现有声明拆名；若拆声明导致
寄存器分配变化，则保留变量并在每个使用区块前解释当前含义。

### 枚举与结构体

- 枚举名描述状态空间，成员名描述值的业务含义，显式保留目标数值。
- 未知值不要补齐“看起来合理”的名字，使用 `UNKNOWN_<value>` 并记录消费者。
- 结构体类型名描述对象而非地址；尚不清楚整体用途时允许保留地址型临时名。
- 字段名必须和宽度匹配；`u8` 标志不能仅因相邻地址存在就升级为 `u16`。
- 数组维度、元素步长和终止符必须由访问算术或资源格式证明。

### 注释

注释解释“为什么、何时、下一步”，不逐行翻译 C。优先注释以下内容：

- 状态进入条件、等待条件和退出目标。
- 同址多生命周期变量、非直观的加一编码、bit mask 和哨兵值。
- CPU 缓冲与 VRAM/OAM 硬件上传之间的时序。
- 截图能确认的用户可见结果，以及仍然只是推测的边界。
- 为保持原始代码生成而保留的反直觉表达式或 K&R 原型。

避免注释“给变量赋值”“调用函数”这类源码已经表达的事实。注释中的文档链接
统一指向 `docs/game/` 下真实存在的文件，并在改名后搜索旧符号，避免留下失效
名称。

## 9. 验证标准

分析和 clean code 也必须留下可重复的验证：

```bash
timeout 900 make
python3 scripts/fncheck.py <changed-function> [...]
python3 scripts/fncheck.py --blame
make verify
sha1sum -c ll.sha1
git diff --check -- docs/game src include linker.ld functions.tsv ll.cfg
```

`make` 必须先确认编译成功，不能只看旧的 `ll.gba` 是否 SHA1 通过。验证范围按
改动风险扩展：

| 改动 | 最低验证 |
|---|---|
| 仅 `docs/game/*.md` | `git diff --check`，并核对链接和符号仍存在。 |
| 注释、局部变量名 | 改动函数 `fncheck.py`，然后 `make verify`。 |
| 函数/全局改名 | 改名函数及全部 C 调用者 `fncheck.py`，然后 `make verify`。 |
| 原型、枚举存储、结构体或 inline helper | 所有消费者所在对象的已匹配函数，`fncheck.py --blame`，然后 `make verify`。 |
| C 文件或数据物理拆分 | 新旧翻译单元中全部已匹配函数、对象/段布局、`make verify` 和 SHA1。 |

失败时先判断差异是否由本任务产生。自己的改动导致红灯就回到本次改动前的局部
内容或修正候选；不能用整体文件恢复覆盖并发变化。其他 agent 导致的差异应在
交接报告中写明 `fncheck --blame` 结果，自己的目标函数仍需单独证明。

当前已知安全结果：

- `TitleMenu_ProcessFrame`：4314 bytes，`fncheck` 通过。
- `OptionsMenu_DrawEntries`：472 bytes，`fncheck` 通过。
- `make verify`：752/752 个已匹配函数通过，ROM SHA1 通过。

注意：`TitleMenu_UpdateUi` 目前仍保留 asm 实现，虽然已经完成语义命名和调用
路径分析；不要因为名称已经可读就误将它标记为 C 已匹配。

## 10. 文档与交接清单

后续 agent 接手一个标题/菜单问题时，应在报告中回答：

- 触发画面对应哪个 `gScenePhase` 和 `gMenuCursorGrp`？
- 输入来自 `gNewKeysRaw` 还是 `gHeldKeysRaw`？是否被转场门控清零？
- 画面是直接写 VRAM，还是先写 EWRAM/WRAM tilemap 缓冲再由 VBlank DMA？
- 该选择修改了哪个全局、触发了哪个 screen effect、下一帧进入什么 phase？
- 是否涉及存档有效性、脚本集、BGM/SFX 或 SIO 状态？
- 新名字是确定语义、同址视图，还是暂定候选？证据在哪里？
- 改动后目标函数和所有受影响对象是否 `fncheck`、`make verify`、SHA1 全绿？

最终报告应只引用 `docs/game/*.md` 的新分析文档，不要求后续 agent 阅读历史
进度日志才能理解结论。

新主题文档建议采用以下结构：

```text
# 主题
1. 范围、地址、当前 C 文件
2. 上游入口与完整调用链
3. 状态机/时序表
4. 输入、渲染、音频、存档或脚本副作用
5. 全局变量与结构体字段证据
6. 旧名 -> 新名映射及置信度
7. C 文件边界判断
8. 未决问题和禁止过度推断项
9. 修改文件与验证结果
```

交接时明确区分三类结果：已经写入源码的正式名、只在文档出现的候选名、仍保留
地址名的未知对象。报告还要说明哪些函数虽然完成语义分析但仍是 `status=0`，
防止后续 agent 把“已命名”误认为“已匹配”。

## 11. 当前文档索引

| 文档 | 内容 |
|---|---|
| [`BOOT_TO_TITLE_FLOW.md`](BOOT_TO_TITLE_FLOW.md) | Reset、IRQ/System_Init、两级主循环到标题可操作状态。 |
| [`TITLE_MENU_PROCESS_FRAME.md`](TITLE_MENU_PROCESS_FRAME.md) | 标题 phase/page、attract、Load、Option、Gallery 和 Card/SIO。 |
| [`TITLE_PROMPT_TO_ATTRACT.md`](TITLE_PROMPT_TO_ATTRACT.md) | PRESS START 的逐帧倒计时、phase 21、attract 脚本集 0 和回标题条件。 |
| [`C_FILE_MODULE_BOUNDARIES.md`](C_FILE_MODULE_BOUNDARIES.md) | 当前 C 文件职责、候选模块和物理拆分风险。 |

后续流程分析在 `docs/game/` 新建独立主题文件，并把入口补进本索引；不要把所有
子系统继续堆入一个总文档。

## 12. 本轮新增分析

| 文档 | 内容 |
|---|---|
| [`NEWGAME_TITLE_TO_OPENING_FLOW.md`](NEWGAME_TITLE_TO_OPENING_FLOW.md) | 从标题 `NEW GAME` 选择、`NewGame_Init`、ScriptSet 001/000 嵌套开场，到 Burg 村地图装载和首次可移动帧。 |
| [`GAME_SCRIPT_LANGUAGE_DESIGN.md`](GAME_SCRIPT_LANGUAGE_DESIGN.md) | 事件脚本重建方案：采用接近 GBA 时代工具链的汇编数据源、薄 opcode 宏、独立文本 token 和逐字节验证，不假称恢复原厂 DSL。 |
| [`GAME_SCRIPT_AUTHORING_DESIGN.md`](GAME_SCRIPT_AUTHORING_DESIGN.md) | 面向人类编写的 `.lls` authoring 层：显式 entry、分支、等待、对白 token、资源别名、raw/legacy 和 canonical 逐字节验证。 |
| [`SCRIPTSET_LOAD_AND_OPCODE_NAMING_REVIEW.md`](SCRIPTSET_LOAD_AND_OPCODE_NAMING_REVIEW.md) | 复核 `ScriptSet_Load(1,0,1)` 三参数、ScriptSet 001 Entry 01 实际字节，以及 80 项 handler 与当前 `Op_*` 命名的偏差。 |
