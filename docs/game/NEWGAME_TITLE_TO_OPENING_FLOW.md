# NEW GAME 选择后的开场执行流程

## 1. 范围与结论

本文从标题主菜单选择 `NEW GAME` 开始，追踪到开场脚本结束、玩家第一次获得地图
行走控制为止。分析对象均为当前 ROM 中已经有逐字节证据的函数；未匹配函数只作为
调用链节点记录，不把“已命名”误认为“已匹配”。

关键结论是：`NEW GAME` 不会直接调用 `MapScene_Load(1)`。实际流程分成三层：

```text
标题菜单 phase 9/page 1
  -> phase 10，等待标题转场
  -> gGameState = GAME_STATE_NEW_GAME
  -> NewGame_Init
  -> ScriptSet_Load(1, 0, 1)，启动 ScriptSet 001 Entry 01
  -> Entry 01 嵌套 ScriptSet 000 Entry 00，播放开场过场
  -> ScriptSet 000 返回宿主 PC
  -> Entry 01 才执行 Op_LoadMap(map=1, spawn=(11,4), dir=4)
  -> 配置 Burg 村 NPC，等待场景稳定
  -> Op_ScriptStop，Task_MapExplore 开放玩家行走
```

因此，`NewGame_Init` 是“新游戏运行时状态和脚本入口初始化器”，不是首张地图的
完整装载器；首张可探索地图的选择由 ScriptSet 001 的宿主脚本完成。

## 2. 标题选择与状态切换

### 2.1 输入层

标题主菜单是 `gScenePhase = TITLE_PHASE_MAIN_MENU (9)`、
`gMenuCursorGrp = TITLE_PAGE_MAIN_MENU (1)`。游标 0 对应 `NEW GAME`。页面逐帧
处理器 `TitleMenu_UpdateUi` 仍是 ASM，因此“按 A 后如何设置 phase 10”的局部寄存器
细节没有在本轮重构；但总状态机和后续状态转移已经由匹配的
`TitleMenu_ProcessFrame` 确认。

按下 `A` 后，页面处理器把标题状态推进到：

```text
gScenePhase = TITLE_PHASE_START_NEW_GAME (10)
```

标题转场未完成时，`TitleMenu_ProcessFrame` 只继续调用 `TitleMenu_UpdateUi`，并等待
`gScreenTransitionState` 归零。转场完成后才执行以下三项：

```c
gGameState = GAME_STATE_NEW_GAME;
gVBlankPipelineMode = 1;
gTitleIntroState = TITLE_INTRO_DISABLED;
```

这三项分别表示切到主世界状态表的第 0 项、使用游戏画面的 VBlank pipeline，以及
明确关闭标题 attract 分流。当前标题任务帧不会直接调用 `NewGame_Init`；因为
`gGameState` 是在状态回调已经执行之后写入的，下一次主循环才会进入新游戏回调。

### 2.2 调度层

`AgbMain` 的普通游戏循环调用 `gMainLoopCallbacks[gMainLoopMode]`。此时
`gMainLoopMode = MAIN_LOOP_GAME (0)`，所以进入 `Task_DispatchGameState`：

```c
ReadKeys();
RenderQueue_Clear();
gGameStateCallbacks[gGameState]();
```

`gGameStateCallbacks[0]` 是 `NewGame_Init`；回调表顺序由
`src/data_87E83F0.c` 的数组初始化直接给出：

| `gGameState` | 回调 | 作用 |
|---:|---|---|
| 0 | `NewGame_Init` | 新游戏冷启动 |
| 1 | `Task_MapExplore` | 地图脚本/对话/行走逐帧任务 |
| 2 | `SceneTransition_Load` | 脚本 `LOAD_MAP` 请求的场景装载 |
| 11 | `Task_TitleMenuFrame` | 标题菜单逐帧任务 |

## 3. `NewGame_Init` 的冷启动内容

函数地址为 `0x08001538`，定义在 `src/scene_mgr.c`。它先清理跨场景状态，再建立
新游戏的初始队伍、属性、资源和脚本 VM：

| 类别 | 写入/调用 | 语义 |
|---|---|---|
| 场景状态 | `gSceneEntryFlag = 0xFF`、`gScenePhase = 0`、`gSceneLoadToggle = 0` | 清除上一次场景进入/标题子状态 |
| 对话与转场 | `gDialogueActive = 0`、`gWarpAnimState = 0`、`gScreenIdleEventFlags[0..1] = 0` | 清除剧情、传送和已看场景图标运行态 |
| VBlank/地图参数 | `gVBlankPipelineMode = 1`、`gMapNpcSetId = 0x82`、出生点/移动集清零 | 设置游戏画面 pipeline 和默认临时参数 |
| 队伍 | `gPartyMemberIds[0] = 0`，其余槽置 `0xFF` | 只让主角进入初始队伍 |
| 战斗阵型 | `gBattleFormationIds[0] = 0`，其余槽置 `0xFF` | 与初始队伍同步清空战斗成员 |
| 属性 | `Party_InitStats()`；对 11 个槽执行 `Stats_RebuildEquipBonuses`、`Stats_RecalcEquip` | 建立基础属性和装备加成缓存 |
| 初始资源 | `gInventory[0xDD] = 2`、`gSilverAmount = 300`、`gGameTimer = 0` | 初始道具、金钱和游戏计时器 |
| 镜头 | `gCameraTargetX = 0x60`、`gCameraTargetY = 0x50` | 脚本场景正式装载前的初始镜头值 |
| 脚本 VM | `Script_ResetVM()` | PC、调用栈、局部槽和对话阶段复位 |

随后打开窗口 0 的硬件显示配置，并依据 `gTitleIntroState` 选择脚本集：

```c
if (gTitleIntroState == TITLE_INTRO_DISABLED) {
    gMapScriptSetId = 1;
    gEnvScriptSetId = 1;
    ScriptSet_Load(1, 0, 1);
    for (i1 = 0; i1 < 16; i1++)
        VBlankWaitExit_PumpSound();
} else {
    ScriptSet_Load(0, 0, 1);
    for (i1 = 0; i1 < 4; i1++)
        VBlankWaitExit_PumpSound();
}
ScriptPump_JumpToEntry(1, 2);
gGameState = GAME_STATE_MAP_EXPLORE;
gScenePhase = 0;
```

普通 `NEW GAME` 已经在标题层把 `gTitleIntroState` 清零，所以这里确定走
`ScriptSet_Load(1, 0, 1)`，而不是 attract 使用的 ScriptSet 000 入口路径。
`ScriptSet_Load` 的 `mode=1` 先把 PC 指向脚本区基址；随后
`ScriptPump_JumpToEntry(1, 2)` 才把 PC 定位到 ScriptSet 001 的 Entry 01：

```text
脚本压缩数据: ROM 0x0862E2A0 (gScriptSet_001)
解压工作区:   0x02016000 入口表，0x02016200 字节码区
Entry 01:     0x02016202
```

`ScriptPump_JumpToEntry` 还会把 8 个脚本局部槽写成 `0xFFFF`，置脚本运行标志
bit 0，并清除对话阶段。`NewGame_Init` 返回时，`gGameState` 已被改为
`GAME_STATE_MAP_EXPLORE`，所以下一帧不会再次执行初始化。

## 4. ScriptSet 001 Entry 01：宿主脚本路由

ScriptSet 001 的入口表和反汇编记录在 `docs/scripts/script_set_001.txt`。Entry 01
位于解压缓冲偏移 `0x0202`，先按事件旗标选择存档继续路线；全新游戏因相关旗标均
为 0，落入初始开场路径：

```text
0x0202  IfAllFlags(0x029E) -> Entry 0A
0x0207  IfAllFlags(0x0054) -> Entry 09
0x020C  IfAllFlags(0x0039) -> Entry 08
0x0211  IfAllFlags(0x0402) -> Entry 07
0x0216  IfAllFlags(0x0038) -> Entry 06
0x021B  IfAllFlags(0x0401) -> Entry 05
0x0220  IfAllFlags(0x0010) -> Entry 03
0x0225  IfAllFlags(0x0000) -> Entry 02
0x022A  Op_BgmStop
0x022B  Op_ScriptStreamLZ(setId=0, entry=0)
0x022E  Op_LoadMap(mapNpcSet=1, moveCmdSet=1, spawn=(11,4), dir=4)
...
0x027B  Op_SceneChangePlain(mode=0)
0x027D  Op_WaitSceneIdle
0x027E  Op_ScriptStop(mode=1)
```

`IfAllFlags(0x0000)` 在此处不是“无条件跳转”的证明；它是脚本格式中对事件旗标
表的特殊判定，反汇编器将其解释为初始开场的阶段路由。全新流程最终执行
`0x022A` 之后的顺序段。

### 4.1 嵌套 ScriptSet 000

`Op_ScriptStreamLZ(setId=0, entry=0)` 是一个脚本 VM 的嵌套装载 opcode，不是普通
函数调用。其 ASM 证据显示它会：

1. 保存当前宿主 PC 的返回位置和宿主脚本集号；
2. 从脚本集指针表 `gScriptSetTable[0]` 取得 ScriptSet 000；
3. 将 ScriptSet 000 解压到同一个 `0x02016000/0x02016200` 工作区；
4. 把 PC 改到嵌套集的 Entry 00，并设置流式解压/入口跳转标志。

因此，`ScriptPump_Run` 在执行到 `0x022B` 时返回，不会在同一轮继续执行
`0x022E`。非强制黑屏时，VBlank 的 `VBlank_UpdateGameScreen` 调用
`ScriptPump_ServiceFrame`，负责每帧推进 LZ 解压；解压完成后，脚本泵才执行
ScriptSet 000 Entry 00。

ScriptSet 000 Entry 00 的前段证据为：

```text
0x0200  Op_ClearSwitchTail
0x0201  Op_LoadMap(mapNpcSet=130, moveCmdSet=0, spawn=(0,0), dir=0)
0x0208  Op_OpenWindow
0x0209  Op_DialogSetup(...)
0x0210  Op_SceneChangePlain(mode=0)
0x0212  Op_WaitSceneIdle
0x0213  Op_BgmPlay(...)
0x0217  Op_DialogText(...)
...
```

这说明普通新游戏首先播放 ScriptSet 000 的开场过场；它不是标题 attract 所用的
“同一段代码换一个标志”，而是从 ScriptSet 001 入口显式嵌套进去的另一段脚本。
ScriptSet 000 的后续还会装载过场地图、角色动画、对白和场景淡出。其尾部在
`0x0CE0` 执行 `Op_ScriptReturnChunk`，按此前保存的宿主上下文恢复 ScriptSet 001
的 PC。宿主 PC 恢复到 `0x022E`，所以返回后才开始 Burg 村地图装载。

## 5. `Op_LoadMap` 到场景装载

`Op_LoadMap` 位于 `0x08052CF0`，当前为已匹配的 `src/script_vm.c` 函数。它从
opcode 后的 6 个字节写入场景请求参数：

| 脚本参数 | 全局 | Entry 01 的值 |
|---|---|---:|
| 地图/NPC 场景编号 | `gMapNpcSetId` | 1 |
| 移动命令集 | `gMoveCmdSetId` | 1 |
| 出生格 X | `gSpawnTileX` | 11 |
| 出生格 Y | `gSpawnTileY` | 4 |
| 出生朝向 | `gSpawnFacingDir` | 4 |

然后它把 `gGameState` 写为 `GAME_STATE_SCENE_LOAD (2)`、把
`gVBlankPipelineMode` 写为 1、把脚本 PC 前进 7 字节并返回 0。返回 0 的重要性在
于：脚本解释循环立即停下，把地图切换交给下一次主世界状态调度。

下一帧 `Task_DispatchGameState` 进入 `SceneTransition_Load`：

```text
Display_ShutdownSequence
临时保存 pipeline，加载期间暂设 pipeline=0
gCameraTarget = (gSpawnTileX * 8, gSpawnTileY * 8) = (88, 32)
清理精灵池、渲染队列、VRAM/调色板传输、动画槽和静态对象
Party_FollowAnim / Party_FollowStep
MapScene_Load(1)
MapScene_InitSprites(1)
Sprites_LoadMapNPCs(1)
BgScroll_LoadFromTable(1)
ChestObjects_LoadForMap(1)
StaticObjGfx_LoadPair(gMapObjGfxSetId)
StaticObjs_Spawn(gMapObjGfxSetId)
恢复 pipeline=1
若脚本不在运行，才从当前地图环境入口重新启动脚本
设置窗口/转场并恢复 GAME_STATE_MAP_EXPLORE
Display_RestartAfterLoad
```

本次首张地图请求时脚本仍处于运行态（嵌套开场脚本或宿主恢复后的脚本），因此
`SceneTransition_Load` 的 `if ((Script_GetFlags() & 1) == 0)` 分支通常不会重置
当前脚本 PC。这是“地图装载”和“剧情继续”能够串接的关键：地图装载只准备运行时
资源，脚本本身在下一帧继续从 `Op_LoadMap` 后的位置执行。

`MapScene_Load(1)` 从 `gMapSceneDescriptors[1]` 读取场景描述符。当前结构化表给出
该项的关键字段：

```text
bgLoadMode=1, bgScrollMode=7, menuEntitySetId=2,
npcSlotGroupId=1, scriptSetId=1,
collisionTileMax=0x025F, tilemapId=0, tileSetId=0, bgPaletteId=0
```

`MapScene_Load` 还会把描述符的 `scriptSetId` 写入 `gMapScriptSetId`；只有脚本运行
标志为 0 时才调用 `ScriptSet_Load(gMapScriptSetId, 0, 1)` 并更新
`gEnvScriptSetId`。所以开场脚本正在执行时，地图环境脚本集字段会被记录，但不会
覆盖当前的剧情脚本上下文。

## 6. Burg 村开场配置与玩家控制

宿主 Entry 01 在 `Op_LoadMap` 返回后继续执行 10 条 `CharaControl SetPosDir`，配置
初始场景 NPC：

```text
chara 3  -> (62,90), dir 2
chara 4  -> (65,89), dir 0
chara 5  -> (71,82), dir 6
chara 6  -> (70,92), dir 0
chara 7  -> (64,94), dir 0
chara 9  -> (66,78), dir 4
chara 12 -> (56,85), dir 2
chara 13 -> (59,81), dir 2
chara 14 -> (59,92), dir 0
chara 17 -> (74,87), dir 6
```

接着执行：

```text
Op_SceneChangePlain(0)
Op_WaitSceneIdle
Op_ScriptStop(1)
```

`Op_SceneChangePlain` 只启动场景调色/窗口效果；`Op_WaitSceneIdle` 在
`gScreenTransitionState == 0` 前逐帧停住脚本；`Op_ScriptStop(1)` 清除脚本运行位。
停止时脚本 VM 还会通过 `Script_SetEnvSet(gScriptReturnSetId)` 恢复环境脚本集，清理
脚本角色临时数组，并保留结束后的 PC 语义。

之后 `Task_MapExplore` 每帧先处理脚本运行位：

- 脚本运行位为 1 时置 `gDialogueActive = 1`，禁止玩家移动；
- 脚本自然停止后，`gDialogueActive = 0`；因普通 NEW GAME 的
  `gTitleIntroState == TITLE_INTRO_DISABLED`，不会走 attract 的回标题分支；
- `gInputLockFrames` 在场景装载和 `Scene_EnterMap` 中设置为 2，随后逐帧递减，并
  在锁定期间清零新按键/持续按键；
- 无对话、无转场、无菜单占用且输入锁清除后，D-pad 才进入 `MovePlayer`；A 键则
  通过 `CheckFacingEvent` 查找面向事件并跳转脚本入口。

所以“看到 Burg 村”与“第一次能走路”不是同一帧：至少要经过场景装载、渐显等待、
脚本停止和两帧输入锁。若开场过场尚未结束，`Task_MapExplore` 的脚本运行位仍会
把玩家输入当作剧情对话状态处理，而不是地图移动。

## 7. 状态时序表

| 时点 | `gGameState` | `gScenePhase` | 脚本状态 | 关键副作用 |
|---|---|---:|---|---|
| 选择后转场完成 | `NEW_GAME` (0) | 标题 phase 10 | 未进入新游戏回调 | 清零 `gTitleIntroState`，pipeline=1 |
| 下一主循环帧 | `NEW_GAME` (0) | 0 | 尚未启动新游戏脚本 | `NewGame_Init` 重建队伍/资源/VM |
| `NewGame_Init` 返回 | `MAP_EXPLORE` (1) | 0 | ScriptSet 001 Entry 01 已定位 | `gScriptCursor=0x02016202`，脚本运行位=1 |
| Entry 01 执行嵌套加载 | `MAP_EXPLORE` (1) | 0 | ScriptSet 000 流式解压/执行 | `Op_ScriptStreamLZ(0,0)` 暂停宿主 PC |
| 嵌套开场过场 | `MAP_EXPLORE`/脚本驱动 | 0 或场景 opcode 所需状态 | 脚本运行位=1 | 多个过场地图、对白、动画和音乐 |
| 返回宿主并执行 `Op_LoadMap` | `SCENE_LOAD` (2) | 0 | PC 前进到 `0x0235` | 请求 map 1、move set 1、出生 `(11,4)` |
| `SceneTransition_Load` 完成 | `MAP_EXPLORE` (1) | 0 | 脚本继续 | 加载 map 1 背景、NPC、碰撞、精灵和静态对象 |
| Entry 01 尾部 | `MAP_EXPLORE` (1) | 0 | `Op_WaitSceneIdle` 后停止 | 配置 NPC，结束脚本并恢复环境脚本集 |
| 首次可移动帧 | `MAP_EXPLORE` (1) | 0 | 脚本运行位=0 | 输入锁清零后 `MovePlayer` 接管 D-pad |

## 8. 证据边界与未决问题

- `TitleMenu_UpdateUi` 仍为 `INCLUDE_ASM`；本轮只确认其输出状态与总状态机消费，
  不给 page 1 内部局部寄存器分配起新语义名。
- ScriptSet 000 Entry 00 的开场过场已经通过解压后的字节码确认，但 map 130、135、
  136 等编号的剧情地点名称仍应以场景资源和截图进一步交叉验证；本文只使用脚本
  opcode 中的数值编号。
- `Op_ScriptStreamLZ` 的宿主返回数组 `gUnk_03000EA0/gUnk_03000EC0` 尚未全部
  结构化命名；本文只根据 ASM 确认“保存宿主脚本集与返回 PC”的行为。
- `MapScene_Load` 当前为 `status=0`，虽然其 ASM 已确认描述符字段和脚本集装载
  分支；不要因为文档使用 `MapScene_Load` 语义名就把它视为已匹配 C 函数。
- 本轮没有修改 `src/`、`ll.cfg`、`functions.tsv` 或链接布局，也没有改变 ROM。

## 9. 相关文件与验证

| 文件 | 用途 |
|---|---|
| `src/menu.c` | 标题总状态机 `TitleMenu_ProcessFrame` |
| `src/scene_mgr.c` | `NewGame_Init`、`SceneTransition_Load`、`Task_MapExplore` |
| `src/sprite_engine.c` | `Task_DispatchGameState`、标题任务收尾 |
| `src/script_vm.c` | 脚本入口、`ScriptSet_Load`、`ScriptPump_Run`、`Op_LoadMap` |
| `src/data_87E83F0.c` | `gGameStateCallbacks` 状态回调表 |
| `docs/scripts/script_set_001.txt` | ScriptSet 001 Entry 01 的反汇编 |
| `data/raw_data/unk_862D8A4.bin` | ScriptSet 000 原始压缩数据 |

本轮只新增分析文档；基线 `make` 已成功并通过 `sha1sum -c ll.sha1`，工作树中其余
并发修改保持不动。
