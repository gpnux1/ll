# C 文件模块划分分析

## 1. 结论

当前 C 文件名和边界并不完全等于游戏的业务模块。反编译项目还受到 agbcc
翻译单元级寄存器分配、函数顺序和 linker 排布影响，所以“语义上应该拆分”与
“现在可以安全拆分”必须分开记录。

本轮结论：

| C 文件 | 当前判断 | 建议 |
|---|---|---|
| `src/engine_core.c` | 基本准确 | 保留，承载硬件启动、显示、VBlank、按键和主循环。 |
| `src/scene_mgr.c` | 基本准确 | 保留，承载地图/战斗/场景状态和状态转换。 |
| `src/menu.c` | 文件名准确但范围很宽 | 暂时保留；它是标题、读档、Option、图鉴、卡片交换及部分 HUD/背包代码的连续菜单代码区。 |
| `src/sprite_engine.c` | 语义范围过宽 | 后续可拆出 `game_state.c` 和 `actor_runtime.c`，当前不直接移动函数。 |
| `src/data_87E83F0.c` | 数据混合 | 暂时保留；回调表和多个只读数据块尚未全部独立结构化。 |
| `src/data_805769C.c` | 数据混合但已逐步去 blob | 继续按地址和消费者拆数据，不按猜测重排。 |

## 2. `engine_core.c`

职责边界清晰：

- `__start` 之后的 `AgbMain` 主循环接口。
- `System_Init` 冷启动初始化。
- IRQ 向量、IWRAM `IntrMain`、VBlank/HBlank 管线。
- `System_SoftReset`、显示关闭/重启和 palette/OAM/VRAM 清理。
- 按键采样和主循环时序。

启动链相关入口：

```text
System_Init
  -> gMainLoopMode = MAIN_LOOP_GAME
  -> gGameState = GAME_STATE_TITLE_MENU
  -> gScenePhase = TITLE_PHASE_INIT

AgbMain
  -> gMainLoopCallbacks[gMainLoopMode]()
```

将 `AgbMain` 移入单独 `main.c` 在语义上可行，但会改变包含头文件和同一翻译单元
的寄存器分配环境，必须等启动链依赖函数全部有稳定匹配结果后再做。

## 3. `scene_mgr.c`

`scene_mgr.c` 的核心是游戏状态和场景状态，而不是单纯的“地图管理”：

- `NewGame_Init`：新游戏/attract 的公共初始化。
- 地图进入、地图恢复、战斗进入和战斗结束状态。
- `gGameState` 的地图、战斗、文本和标题切换。
- `gScreenTransitionState` 完成后的场景转移。
- `gTitleIntroState` 对 attract 演示和普通游戏的分流。

`NewGame_Init` 是 `menu.c` 与 `scene_mgr.c` 的边界函数。标题菜单只设置游戏状态
和 intro 标记，具体脚本集、队伍、初始地图和 VM 初始化由 `scene_mgr.c` 完成。

## 4. `menu.c`

`menu.c` 当前覆盖地址连续、调用关系紧密的一整组 UI：

```text
标题开场/主菜单
  -> PRESS START
  -> NEW GAME / LOAD GAME / OPTION
  -> LOAD 槽
  -> BGM/SE/CARD/GALLERY Option
  -> 卡片图鉴/详情/SIO 交换
  -> 背包、HUD、部分存档和数字绘制辅助
```

重要函数边界：

| 地址 | 函数 | 语义 |
|---:|---|---|
| `0x08011454` | `TitleMenu_ProcessFrame` | 标题及所有子菜单的大状态机。 |
| `0x08012790` | `TitleMenu_UpdateUi` | page 0 至 page 5 的逐帧 UI 更新器。 |
| `0x08013870` | `TitleMenu_DrawOptions` | 标题 tilemap 和主菜单初始文字绘制。 |
| `0x08013934` | `OptionsMenu_DrawEntries` | Option 四项及 BGM/SE 数值绘制。 |
| `0x08013C00` | `sub_8013C00` | 读档/场景选择页的复杂文字和数据绘制。 |
| `0x08015F14` | `SaveUi_DrawSlots` | 三个存档槽绘制。 |
| `0x08016BB0` | `SaveUi_OpenLoad` | 读档 UI 的底层入口。 |

把整个文件改名为 `title_menu.c` 会丢失背包/HUD/存档辅助代码的真实归属；改名为
`save_menu.c` 则会错误地忽略标题和 Option。当前 `menu.c` 是最不误导的文件名。

## 5. `sprite_engine.c` 的隐含边界

`sprite_engine.c` 前段主要是精灵图形、动画槽和角色显示运行时；但地址
`0x08003088..0x08003208` 附近还包含：

- `Task_DispatchGameState`。
- `Task_TitleMenuFrame`。
- 地图/文本/标题状态的薄包装任务。

这段代码按职责更适合独立为：

```text
sprite_engine.c : 精灵图形、动画、OAM 和角色显示
game_state.c    : 主循环状态分发、标题/地图/文本状态任务
actor_runtime.c : Actor/Chara 更新和运行时对象逻辑
```

但是当前不直接拆分，原因有三点：

1. agbcc 的寄存器分配会受到同一翻译单元中前后函数和全局引用影响。
2. linker 中 `.text` 的对象顺序必须保持，不能因文件拆分让函数顺序漂移。
3. 该地址段前后仍有未完全反编译函数，移动已匹配函数可能让后续匹配全部重新验证。

## 6. 数据 C 文件边界

### `data_87E83F0.c`

其中包含 `gMainLoopCallbacks`、`gGameStateCallbacks` 以及大量地址相邻只读数据。
回调表的语义已经清楚，但相邻数据块未全部解析。可以在未来建立：

```text
game_state_data.c : gMainLoopCallbacks / gGameStateCallbacks 及相关状态表
menu_text_data.c  : 标题、Option、Load 和卡片菜单字符串表
```

迁移时必须保持 ROM 地址和 `.rodata` 顺序，不能按 C 文件名重新排序。

### `data_805769C.c`

该文件已经按消费者逐步拆出地图边界、存档表、菜单调色板、脚本和动画数据。
仍然混有变长记录、压缩块和多种表，不适合一次性改成大结构体数组。默认策略：

- 一维表优先保持一维声明。
- 只在存在明确消费者索引公式时引入结构体。
- 重叠地址视图使用别名，而不是复制成两个物理数组。
- 每个数据块用地址注释，验证 `nm` 地址与 ROM 原址一致。

## 7. 推荐的后续拆分顺序

```text
1. 先完成 game_state.c 候选函数的 score 0 和 fncheck 资产
2. 固定 sprite_engine.c 中前后函数的 linker 顺序
3. 只移动连续地址段，逐个函数验证
4. 再把标题/Option 数据表迁入 game_state_data.c 或 menu_text_data.c
5. 最后处理 actor_runtime.c 和其调用者
```

不建议在未匹配函数仍大量存在时按目录名重构。对于本项目，字节匹配、地址布局
和可复现构建优先于目录层面的理想化模块设计。
