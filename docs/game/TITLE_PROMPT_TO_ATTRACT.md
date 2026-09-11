# 标题界面等待与剧情演示入口

## 1. 范围

本文分析 `Screenshots/ll_000.png` 的 `PRESS START BUTTON` 状态到
`Screenshots/ll_001.png` 剧情演示开始的路径。标题资源加载、Logo 和主菜单的整体
流程分别见 [`BOOT_TO_TITLE_FLOW.md`](BOOT_TO_TITLE_FLOW.md) 与
[`TITLE_MENU_PROCESS_FRAME.md`](TITLE_MENU_PROCESS_FRAME.md)。

分析对象：

| 对象 | 地址/位置 | 作用 |
|---|---|---|
| `TitleMenu_ProcessFrame` | `0x08011454`, `src/menu.c` | 标题大 phase 状态机；phase 8 负责进入逐帧 UI。 |
| `TitleMenu_UpdateUi` | `0x08012790`, ASM | page 0 的输入、提示绘制、计时和 UI 精灵更新。 |
| `Task_TitleMenuFrame` | `0x080031E4` | 每帧调用标题状态机、调色板效果和 OAM 队列。 |
| `NewGame_Init` | `0x08001538`, `src/scene_mgr.c` | attract 和正式 NEW GAME 共用的初始化入口。 |
| `Task_MapExplore` | `0x08001D08`, `src/scene_mgr.c` | 脚本地图帧任务；负责演示结束/跳过后的回标题判断。 |

## 2. 逐帧入口

启动初始化完成后，普通主循环每帧执行：

```text
AgbMain
  -> gMainLoopCallbacks[gMainLoopMode]
  -> Task_DispatchGameState
  -> gGameStateCallbacks[gGameState]
  -> Task_TitleMenuFrame
  -> TitleMenu_ProcessFrame
  -> TitleMenu_UpdateUi
  -> PaletteEffects_Update
  -> OAM_FlushFromQueue
  -> VBlankIntrWait
```

`Task_TitleMenuFrame` 没有帧跳过、定时器换算或“每 64 帧调用一次”的逻辑；它每次
被普通主循环选中都会直接调用 `TitleMenu_ProcessFrame`。因此 page 0 的倒计时单位
就是游戏主循环帧。

## 3. 进入 PRESS START

`TitleMenu_ProcessFrame` 在 phase 5 装入 Lunar 标题场景时执行：

```c
gTitleAttractCountdown = 0x10;
gScenePhase = TITLE_PHASE_TITLE_FADEIN;
```

phase 6 等待标题渐入完成，phase 7 保持标题演出。phase 7 的计数结束时：

```c
gScenePhase = TITLE_PHASE_PRESS_START;
gSceneLoadToggle = 0;
```

phase 8 的 C 代码先执行 `gSceneLoadToggle++`，然后故意无 `break` 落入 phase 9
分支，调用 `TitleMenu_UpdateUi()`。因此 phase 8 不是一个独立的稳定画面任务，而
是从标题保持阶段进入 page 0 的第一帧；后续帧通常已经以 phase 9 进入同一个 page 0
处理器。

## 4. 两个独立计数器

反汇编中容易把两个计数器混为一个。它们的职责不同：

| 变量 | 地址 | 每帧行为 | 用途 |
|---|---:|---|---|
| `gSceneLoadToggle` | `0x03002C4C` | phase 8/9 入口递增 | bit 5 控制 `PRESS START BUTTON` 显隐；低 6 位为 UI/OAM 更新节奏。 |
| `gTitleAttractCountdown` | `0x03000234` | page 0 每次更新减 1 | 减到 0 后写 phase 21，启动标题退出转场。 |

page 0 的关键 ASM 顺序是：

```text
检查 gNewKeysRaw 的 A/START
若无确认输入，绘制或隐藏 PRESS START
用 gSceneLoadToggle 的低 6 位决定是否刷新 UI 精灵
gTitleAttractCountdown--
若结果非 0，刷新 UI 精灵并返回
若结果为 0，gScenePhase = TITLE_PHASE_ATTRACT_START
进入标题退出/淡出辅助路径
```

`gSceneLoadToggle & 0x3F == 0` 只决定是否调用 UI 精灵刷新辅助块；它不包围
`gTitleAttractCountdown--`。因此正确时间是：

```text
初值 0x10 = 16 个 page-0 更新
16 个主循环帧 / 59.7 Hz ~= 0.268 秒
```

`gSceneLoadToggle` 的 bit 5 和低 6 位负责提示显隐、UI 精灵更新等视觉节奏，不能
拿来把倒计时解释成 `16 * 64` 帧。此前文档中的约 17.1 秒结论是混淆了两个不同
地址的计数器，现以目标 ASM 为准。

## 5. 玩家按键分支

page 0 首先检查 `gNewKeysRaw` 的 `A_BUTTON | START_BUTTON`：

```text
按 A 或 START
  -> 播放确认音效
  -> 初始化主菜单游标为 page 1
  -> gScenePhase = TITLE_PHASE_MAIN_MENU
  -> TitleMenu_UpdateUi 继续绘制 NEW GAME / LOAD GAME / OPTION

无 A/START
  -> 保留 page 0
  -> 更新 PRESS START 显隐和 UI 精灵
  -> 消耗一个 attract 倒计时单位
```

输入先在 `TitleMenu_ProcessFrame` 中受 `gScreenTransitionState` 门控。转场非零时，
它清除 `gNewKeysRaw` 和 `gHeldKeysRaw`；因此 Logo/标题渐入、退出淡出期间的按键
不会被 page 0 当成确认输入。进入稳定 phase 8/9 后，`ReadKeys` 写入的边沿按键
才会到达 `TitleMenu_UpdateUi`。

## 6. 倒计时归零后的 phase 21

倒计时归零并不是在 `TitleMenu_UpdateUi` 内直接调用 `NewGame_Init`。它分成两个
主循环帧级阶段：

```text
page 0 更新
  -> gScenePhase = TITLE_PHASE_ATTRACT_START (21)
  -> 标题 UI 辅助路径启动 ScreenFx_SetMode(4)

下一次 Task_TitleMenuFrame
  -> TitleMenu_ProcessFrame 仍调用 TitleMenu_UpdateUi
  -> 等待 gScreenTransitionState 清零
  -> Display_ShutdownSequence()
  -> gGameState = GAME_STATE_NEW_GAME
  -> gVBlankPipelineMode = 1
  -> 保留 gTitleIntroState != TITLE_INTRO_DISABLED
```

phase 21 仍在 `TitleMenu_ProcessFrame` 的共享分支中。转场完成前，标题函数会继续
清除输入并等待；完成后才设置 `GAME_STATE_NEW_GAME`。设置游戏状态发生在 CPU
逻辑帧末尾，所以当前帧仍由标题任务收尾；下一个普通主循环帧才会索引
`gGameStateCallbacks[GAME_STATE_NEW_GAME]`。

## 7. `NewGame_Init` 的 attract/正式新游戏分流

下一帧 `Task_DispatchGameState` 通过 `gGameStateCallbacks[GAME_STATE_NEW_GAME]`
调用 `NewGame_Init`。它先重建初始玩家、队伍、道具、金钱、相机和脚本 VM，然后
依据 `gTitleIntroState` 选择脚本集：

| 入口 | `gTitleIntroState` | 脚本调用 | 等待 VBlank 次数 | 语义 |
|---|---:|---|---:|---|
| attract 超时 | 非 0（保留的标题演示标志） | `ScriptSet_Load(0, 0, 1)` | 4 | 演示/剧情介绍脚本。 |
| 玩家选 NEW GAME | 0 (`TITLE_INTRO_DISABLED`) | `ScriptSet_Load(1, 0, 1)` | 16 | 正式新游戏开场脚本。 |

两条路径之后都执行：

```c
ScriptPump_JumpToEntry(1, 2);
gGameState = GAME_STATE_MAP_EXPLORE;
gScenePhase = 0;
```

因此 `gTitleIntroState` 不是单纯的 Logo 当前序号；它同时是“下一次
`NewGame_Init` 是否走演示脚本”的持久分流标志。普通 NEW GAME 和 attract 必须在
标题层分别清零或保留它。

## 8. 脚本地图任务如何产生 ll_001

`NewGame_Init` 返回后，下一帧状态为 `GAME_STATE_MAP_EXPLORE`，进入
`Task_MapExplore`。该任务每帧先检查 `Script_GetFlags() & 1`：

```text
脚本运行位为 0
  -> gDialogueActive = 0
  -> 若 gTitleIntroState != 0 且无淡出/转场
       gGameState = GAME_STATE_TITLE_MENU
       gScenePhase = 5
       回标题场景

脚本运行位为 1
  -> gDialogueActive = 1
  -> 若 gTitleIntroState != 0 且无淡出/转场且 gNewKeysRaw & 0x30F
       Script_Abort(1)
       gGameState = GAME_STATE_TITLE_MENU
       gScenePhase = 5
       回标题场景
```

演示脚本运行时，`gDialogueActive=1`，脚本 VM、地图和精灵任务共同生成
`ll_001.png` 的剧情介绍画面。演示中的输入由 `0x30F` 掩码筛选；命中后终止演示
并回到标题资源加载 phase 5，不会进入正式新游戏。

脚本自然结束时，运行位清零，`Task_MapExplore` 同样回到 phase 5。于是
`ll_001.png` 之后的两个返回条件是：玩家跳过演示，或演示脚本自行播完；两者都
回到标题 scene load，再重新经过标题渐入和 PRESS START。

## 9. 显示与声音时序

标题退出帧由 `TitleMenu_ProcessFrame` 将 `gVBlankPipelineMode` 从 4 切到 1，
并调用 `Display_ShutdownSequence`。当前标题 CPU 缓冲的最终 OAM 仍由
`Task_TitleMenuFrame` 收尾；下一次 VBlank 使用 mode 1 的游戏画面上传路径。

`NewGame_Init` 在脚本装载后通过 `VBlankWaitExit_PumpSound` 等待解压/显示同步，
随后 `ScriptPump_JumpToEntry` 开始入口脚本。attract 分支只等待 4 次，正式新游戏
等待 16 次，说明两者使用了不同的脚本/加载量，但不能仅据此推断剧情时长。

## 10. 结论与交接重点

1. `PRESS START` 的显示状态由 `gSceneLoadToggle` 控制，自动演示触发由
   `gTitleAttractCountdown` 控制；两者不可合并。
2. 目前从目标 ASM 可确定自动演示触发为 16 个 page-0 更新帧，约 0.27 秒；
   若实机录屏显示更长等待，应优先检查模拟器暂停/录制帧率或外层输入工具，而
   不是修改该计数器语义。
3. phase 21 只负责从标题状态切到 `GAME_STATE_NEW_GAME`，演示画面本身由脚本集
   0 和 `Task_MapExplore` 产生。
4. attract 路径保留 `gTitleIntroState`，普通 NEW GAME 清零；这是两条入口最终
   加载脚本集 0/1 的决定性条件。
5. `TitleMenu_UpdateUi` 仍是 ASM 实现；后续若将 page 0 反编译成 C，必须保持
   `gSceneLoadToggle` 的 bit 逻辑、倒计时无条件递减和 phase 21 的转场顺序。

## 11. 验证记录

本次分析同步修正了源码注释和 `docs/game` 既有说明，未改变目标函数指令：

```text
TitleMenu_ProcessFrame 4314 bytes: OK
OptionsMenu_DrawEntries 472 bytes: OK
make verify: ROM SHA1 OK, 752/752 matched functions OK
```
