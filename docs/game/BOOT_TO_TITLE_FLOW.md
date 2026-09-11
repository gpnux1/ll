# Reset 到标题界面的执行流程

> 范围: GBA 硬件复位入口 `__start` 到标题菜单开始逐帧更新。
> 依据: `asm/crt0.s`、ROM 反汇编、已匹配 C、状态回调表和 IWRAM 地址。
> 日期: 2026-09-08。

## 1. 总体调用链

```text
GBA reset vector 0x08000000
  __start / _080000C0 (ARM)
    设置 IRQ 栈 0x03007FA0、System 栈 0x03007F00
    临时令 BIOS IRQ 向量 0x03007FFC -> ROM IntrMain
    bx AgbMain
      System_Init
        RegisterRamReset(3)
        安装 IWRAM 中断表与 IWRAM IntrMain
        gMainLoopMode = MAIN_LOOP_GAME
        gGameState = GAME_STATE_TITLE_MENU
        gScenePhase = TITLE_PHASE_INIT
        初始化显示、运行时、声音和菜单槽
      while (1)
        gMainLoopCallbacks[gMainLoopMode]()
          Task_DispatchGameState
            ReadKeys
            RenderQueue_Clear
            gGameStateCallbacks[gGameState]()
              Task_TitleMenuFrame
                TitleMenu_ProcessFrame
                PaletteEffects_Update
                OAM_FlushFromQueue
        VBlankIntrWait
          VBlankIntr: 根据 gVBlankPipelineMode 上传 VRAM/OAM/调色板
        SoundMain_Frame
```

这是两级调度，不是一个平面状态机：`gMainLoopMode` 在普通游戏循环和战斗循环之间切换；只有普通游戏循环才继续用 `gGameState` 索引 14 项状态回调表。

## 2. 复位入口

`__start` 位于 `0x08000000`，先跳过 ROM Header 到 `0x080000C0`。入口仍在 ARM 状态，依次做三件事：

1. CPSR 切到 IRQ mode (`0x12`)，设置 IRQ SP=`0x03007FA0`。
2. CPSR 切到 System mode (`0x1F`)，设置主 SP=`0x03007F00`。
3. 将 ROM 内 `IntrMain` 临时写到 BIOS IRQ 指针 `0x03007FFC`，再 `bx AgbMain` 进入 Thumb C 世界。

这个临时 IRQ 指针随后会被 `System_Init` 替换为 IWRAM 副本，避免正常运行时从慢速 Game Pak ROM 执行中断分发器。

## 3. System_Init 冷启动

`System_Init` (`0x08001128`) 的顺序由目标汇编固定：

| 顺序 | 操作 | 含义 |
|---|---|---|
| 1 | `RegisterRamReset(3)` | BIOS 清 EWRAM/IWRAM；BIOS 保留顶部栈/向量区。 |
| 2 | `REG_WAITCNT = 0x4014` | 开 ROM prefetch，WS0 non-sequential=3 cycles、sequential=1 cycle。旧报告中的 `0x4317` 不正确。 |
| 3 | `gMainLoopMode = MAIN_LOOP_GAME` | 选择普通游戏顶层循环。 |
| 4 | `Palette_FillWhite()` | 先把 512 色调色板填白，避免初始化过程显示随机内容。 |
| 5 | DMA `gIntrTable -> 0x03001950` | 把 IRQ 回调表复制到 IWRAM。 |
| 6 | DMA `IntrMain -> gIntrMainBuf` | 复制 `0x400` halfwords，即 `0x800` bytes ARM IRQ 分发代码。 |
| 7 | `INTR_VECTOR = gIntrMainBuf` | BIOS IRQ 指针切到 IWRAM。 |
| 8 | 状态初始化 | `gGameState=GAME_STATE_TITLE_MENU`，`gScenePhase=0`。 |
| 9 | IRQ 配置 | IE 开 VBlank/HBlank/GamePak，DISPSTAT 开 VBlank/HBlank IRQ，IME=1。 |
| 10 | 首次 `VBlankIntrWait()` | 等待安全显示时点。 |
| 11 | `REG_DISPCNT = 0x37C0` | Mode 0、OBJ 1D、forced blank，开 BG0/1/2、OBJ、WIN0。 |
| 12 | `System_SoftReset(0)` | 清 VRAM/OAM/PLTT、精灵/传输队列、事件和开关状态。 |
| 13 | `Sound_Init()` | 初始化 M4A 音频运行时。 |
| 14 | `MenuSlot_ResetAll()` | 清 10 个菜单光标/滚动槽。 |
| 15 | `gScenePhase = 0` | 明确从标题 phase 0 开始。 |

`AgbMain` 进入无限循环后，每帧固定为“CPU 逻辑 -> 等 VBlank -> 音频主任务”。标题期间 `Task_TitleMenuFrame` 还会在 CPU 逻辑末尾推进调色板特效并生成 OAM；真正向硬件 OAM/VRAM 的上传发生在 VBlank IRQ。

## 4. 标题状态机

启动路径只经过 `TitleMenu_ProcessFrame` 的 phase 0..9：

| Phase | 枚举 | 作用和转移 |
|---:|---|---|
| 0 | `TITLE_PHASE_INIT` | 停 BGM、读存档槽 0、令 `gTitleIntroState=GAME_ARTS_LOGO`，转 1。 |
| 1 | `TITLE_PHASE_LOGO_LOAD` | forced blank 下清运行时，按 `gTitleIntroState` 解压 GAME ARTS 或 ESP Logo，启用 VBlank pipeline 4，启动 palette effect 3，转 2。 |
| 2 | `TITLE_PHASE_LOGO_FADEIN` | 等 `gScreenTransitionState==0`，转 3，并设 64 帧停留计数。 |
| 3 | `TITLE_PHASE_LOGO_HOLD` | 倒计时 64 帧，启动 palette effect 4，转 4。 |
| 4 | `TITLE_PHASE_LOGO_FADEOUT` | 等转场结束；Logo 1 后回 phase 1 加载 ESP，Logo 2 后转 phase 5。 |
| 5 | `TITLE_PHASE_TITLE_LOAD` | 加载地图场景 0x0C、标题 BG/OBJ/调色板，调用 `TitleMenu_DrawOptions`，建立消息页索引，启动渐入并转 6。 |
| 6 | `TITLE_PHASE_TITLE_FADEIN` | 等公共 palette transition 完成，再推进 56 帧 alpha 混合。 |
| 7 | `TITLE_PHASE_PRE_PROMPT_HOLD` | 继续 120 帧标题演出；标题已经可见，但尚未开放菜单输入。 |
| 8 | `TITLE_PHASE_PRESS_START` | 递增提示闪烁计数器，并由 `TitleMenu_UpdateUi` 处理提示页输入和 16 帧 attract 倒计时。 |
| 9 | `TITLE_PHASE_MAIN_MENU` | `TitleMenu_UpdateUi` 每帧处理 `NEW GAME`、`LOAD GAME`、`OPTION`。 |

“到达标题界面”有两个可用定义：phase 5 完成后资源已经装入并开始显示；phase 9 才是玩家可以稳定操作的标题菜单。调试断点若要观察首帧图像，应放在 phase 5 的 `Display_RestartAfterLoad` 后；若要观察输入，应放在 `TitleMenu_UpdateUi`。

## 5. VBlank 与输入门控

标题 Logo 和标题菜单把 `gVBlankPipelineMode` 设为 4。`VBlankIntr` 的 mode 4 每次 VBlank：

- 调 `m4aSoundVSync`；
- flush VRAM 请求；
- 写 BG1/2/3 scroll；
- 写 BLDCNT/BLDALPHA；
- DMA CPU OAM buffer 到硬件 OAM；
- DMA 两块 UI tilemap buffer；
- 更新窗口/精灵寄存器。

`gScreenTransitionState` 非零表示 `ScreenFx_SetMode` 启动的窗口/调色板转场仍在进行。`TitleMenu_ProcessFrame` 在这些帧把 `gNewKeysRaw` 和 `gHeldKeysRaw` 清零，因此 Logo、渐变和子菜单过渡不会误吃按键。`PaletteEffects_Update` 最终把该状态清零，标题 FSM 才继续。

## 6. 本轮语义命名

| 旧名 | 新名 | 证据 |
|---|---|---|
| `gMainTasks` | `gMainLoopCallbacks` | `AgbMain` 直接索引；仅含普通游戏与战斗两个顶层循环。 |
| `gMainTaskSlot` | `gMainLoopMode` | 值 0/1 选择上述表，不是任务队列槽。 |
| `gUnk_087E83F8` | `gGameStateCallbacks` | `Task_DispatchGameState` 用 `gGameState` 索引 14 项回调。 |
| `gMainGameState` | `gGameState` | 当前普通游戏状态枚举。旧链接名保留为同址别名。 |
| `gSceneSubState` | `gScreenTransitionState` | `ScreenFx_SetMode` 写入，palette/window effect 完成时清零；它不是场景子状态。 |
| `gCutsceneActive` | `gTitleIntroState` | 启动时依次为 1/2 选择两个 Logo，3 表示 title attract/demo 路径，0 表示正常游戏。 |
| `Scene_ExitToMenu` | `BattleTransition_Enter` | 淡出后调用战斗初始化，`gMainLoopMode=MAIN_LOOP_BATTLE`。 |
| `SaveUi_LoadScreen` | `TitleMenu_UpdateUi` | 唯一调用者是标题总状态机，实际为标题/读档/卡片 UI 的逐帧更新器。 |
| `sub_8013870` | `TitleMenu_DrawOptions` | 清标题 tilemap、遍历 `gTitleMenuDesc` 并绘制标题选项。 |

`CardExchangePacket` 明确了 SIO 一次传输的 32-bit 布局；匹配代码仍以两个 byte field 访问卡片 ID 的低/高字节，保留原始 `strb/ldrb` 形状。

## 7. C 文件边界复核

当前与启动链相关的文件名中，`engine_core.c`、`scene_mgr.c`、`menu.c` 都可成立：

- `engine_core.c`: IRQ、VBlank、显示停启、软复位、按键、`System_Init`、`AgbMain`。
- `scene_mgr.c`: `gGameStateCallbacks` 中的地图/战斗进入/战后恢复等主要状态。
- `menu.c`: 地址连续的标题、读档、卡片图鉴、联机交换、背包和 HUD 菜单代码；它比 `save_menu.c` 或 `title_menu.c` 更符合实际覆盖面。

`sprite_engine.c` 的 `0x08003088..0x08003208` 是例外：这段是状态分发和薄包装任务，职责上应为 `game_state.c`，其前后才是精灵/角色运行时。现在不物理移动，因为从中间切开翻译单元会改变 agbcc 的跨函数寄存器分配状态，并要求重新验证后续整段函数。安全的后续拆分顺序应是：先保证该地址段所有函数均有 score 0 资产，再拆成 `sprite_engine.c` / `game_state.c` / `actor_runtime.c` 三个连续对象，并保持 linker 顺序和逐函数字节一致。

`src/data_87E83F0.c` 也包含回调表和大量无关只读数据，名称只是地址标签。数据去 blob 后可把前两张表迁入 `game_state_data.c`，但在其余数据尚未结构化前直接改 C 文件边界只会制造链接布局风险，因此本轮仅语义化表符号。
