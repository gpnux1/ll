# TitleMenu_ProcessFrame 分析

## 1. 函数定位

| 项目 | 内容 |
|---|---|
| 函数 | `TitleMenu_ProcessFrame` |
| 地址 | `0x08011454` |
| C 文件 | `src/menu.c` |
| 调用者 | `Task_TitleMenuFrame` |
| 下级更新器 | `TitleMenu_UpdateUi` (`0x08012790`) |
| 作用 | 标题开场、PRESS START、主菜单、读档、Option、图鉴和卡片交换的总状态机 |

调用链为：

```text
AgbMain
  -> Task_DispatchGameState
    -> Task_TitleMenuFrame
      -> TitleMenu_ProcessFrame
        -> TitleMenu_UpdateUi
```

`TitleMenu_ProcessFrame` 负责大阶段和资源切换；`TitleMenu_UpdateUi` 负责当前页面的
逐帧输入、文字、游标、音效和页面内部小状态。两者不能合并理解为一个简单的标题
绘制函数。

## 2. 截图与运行状态

| 截图 | phase | page (`gMenuCursorGrp`) | 解释 |
|---|---:|---:|---|
| `Screenshots/ll_000.png` | 8 | 0 | 标题画面和 `PRESS START BUTTON` 提示 |
| `Screenshots/ll_001.png` | 0 | 0 | 无操作超时后的剧情演示/attract mode |
| `Screenshots/ll_002.png` | 9 | 1 | `NEW GAME`、`LOAD GAME`、`OPTION` 主菜单 |
| `Screenshots/ll_003.png` | 14 | 2 | 三个存档槽的 `LOAD` 页面 |
| `Screenshots/ll_004.png` | 20 | 3 | `OPTION` 页面：BGM、SE、CARD、GALLERY |

这里的 phase 是 `gScenePhase`，page 是 `gMenuCursorGrp`。phase 是整个标题/子菜单
状态机的位置，page 是 `TitleMenu_UpdateUi` 的页面分派值，二者是不同层次。

## 3. 开场 Logo 到标题

`TitleMenu_ProcessFrame` 的 phase 0 至 4 依次完成：

| phase | 名称 | 行为 |
|---:|---|---|
| 0 | `TITLE_PHASE_INIT` | 停止 BGM，读取存档槽 0，把 `gTitleIntroState` 设为 GAME ARTS Logo。 |
| 1 | `TITLE_PHASE_LOGO_LOAD` | forced blank 下加载 GAME ARTS 或 ESP Logo 的图块、调色板和 tilemap。 |
| 2 | `TITLE_PHASE_LOGO_FADEIN` | 等待屏幕转场完成，转入 Logo 保持阶段。 |
| 3 | `TITLE_PHASE_LOGO_HOLD` | 保持约 64 帧，然后启动淡出。 |
| 4 | `TITLE_PHASE_LOGO_FADEOUT` | 第一个 Logo 结束后加载 ESP；第二个 Logo 结束后进入 title scene load。 |

phase 5 加载真实 Lunar 标题场景，包括地图 0x0C、标题背景、菜单文字、OBJ 资源、
调色板、消息页索引和 BGM。phase 6/7 完成淡入和标题保持，phase 8 才进入
PRESS START 输入态。

## 4. `ll_000.png`：PRESS START

phase 8 对应 page 0。`TitleMenu_UpdateUi` 每帧：

1. 使用 `gSceneLoadToggle` 的 bit 5 控制 `PRESS START BUTTON` 的显示/隐藏。
2. 检查 `gNewKeysRaw & (A_BUTTON | START_BUTTON)`。
3. 命中后播放确认音效，切换到 `gScenePhase = TITLE_PHASE_MAIN_MENU`，调用
   `sub_800E668(1)` 进入 page 1。
4. 每次 page 0 更新都消耗一个 `gTitleAttractCountdown` 单位；
   `gSceneLoadToggle` 的低 6 位只控制 UI 更新节奏。

标题场景加载时：

```text
gTitleAttractCountdown = 0x10
```

phase 8 每个游戏主循环帧消耗 1 个单位，初值 16，因此无操作约 16 帧后进入
phase 21；按 GBA 约 59.7 Hz 计算约 0.27 秒。`gSceneLoadToggle` 的低 6 位与
bit 5 只分别参与 UI 更新和 `PRESS START` 显隐，不能拿来给倒计时乘以 64。
计时器由原来的地址占位名
`gTitleFadeStep` 改为 `gTitleAttractCountdown`，旧名保留为同址兼容宏。

## 5. `ll_001.png`：attract/剧情演示

phase 21 是 `TITLE_PHASE_ATTRACT_START`。它关闭标题显示并选择
`GAME_STATE_NEW_GAME`，但不清除 `gTitleIntroState`。因此 `NewGame_Init` 执行：

```c
ScriptSet_Load(0, 0, 1);
```

这与玩家选择 NEW GAME 时的脚本集 1 不同。脚本集 0 负责截图 `ll_001.png` 所示的
剧情介绍/演示内容，所以该画面不是标题菜单的另一个 tilemap 页面。

演示期间 `Task_MapExplore` 继续检查输入。当 `gTitleIntroState != 0` 且检测到
有效按键时，脚本被 `Script_Abort(1)` 中止，重新设置 `gGameState` 为标题菜单并
从 phase 5 重载标题场景。也就是说，attract 中按键的语义是“跳过演示回标题”，
不是进入新游戏地图。

## 6. `ll_002.png`：主菜单

phase 9 对应 page 1。主菜单游标由 `gMenuCursorSel` 控制，实际可选项为：

| 游标 | 文字 | 目标 phase |
|---:|---|---:|
| 0 | `NEW GAME` | `TITLE_PHASE_START_NEW_GAME` |
| 1 | `LOAD GAME` | `TITLE_PHASE_LOAD_MENU_OPEN` |
| 2 | `OPTION` | `TITLE_PHASE_OPTIONS_OPEN` |

按 `A` 后先执行淡出或页面初始化，转场完成后再由 `TitleMenu_ProcessFrame` 进入
目标页面。按 `B` 会清理主菜单文字并回到 phase 8/page 0。

选择 NEW GAME 后，`gTitleIntroState` 被清零，随后进入 `NewGame_Init`。这一步是
普通游戏与 attract 演示的关键区别：

```text
普通 NEW GAME -> ScriptSet_Load(1, 0, 1)
attract mode -> ScriptSet_Load(0, 0, 1)
```

## 7. `ll_003.png`：LOAD 页面

page 2 使用 phase 11 至 14：

| phase | 名称 | 行为 |
|---:|---|---|
| 11 | `TITLE_PHASE_LOAD_MENU_OPEN` | 进入 page 2，初始化 Save FSM 和目标槽，清窗口 tilemap。 |
| 12 | `TITLE_PHASE_LOAD_READ_SLOTS` | 等待 `Save_Fsm(0)` 读取三个槽。 |
| 13 | `TITLE_PHASE_LOAD_MENU_FADEIN` | 启动淡入，完成后进入稳定显示。 |
| 14 | `TITLE_PHASE_LOAD_MENU` | 每帧更新读档 UI；Save FSM 为 `0xFE` 时重绘三个槽。 |

每个槽由 `Save_FillSlot3` 填充。当前槽使用 palette 13，其余槽使用 palette 11。
按 `A` 选择槽时会先检查 `0x02021000 + slot * 0x2000` 的存档签名
`LUNAR1_12_09`；无效槽不会进入游戏加载。

有效槽的后续路径是：

```text
确认槽
  -> TITLE_PHASE_SUBMENU_CLOSE
  -> screen fade out
  -> Save_FillSlot2(gSaveCurSlot)
  -> TITLE_PHASE_START_LOADED_GAME
  -> NewGame_Init / 地图恢复流程
```

按 `B` 则通过 `TITLE_PHASE_RETURN_TO_MAIN_MENU` 回到 page 1。

## 8. `ll_004.png`：OPTION 页面

page 3 对应 phase 18 至 20。进入时清空窗口缓冲、初始化 Option 的声音测试值，
然后调用 `OptionsMenu_DrawEntries` (`0x08013934`) 绘制四项：

| 游标 | 文字 | 数据/行为 |
|---:|---|---|
| 0 | BGM | 有效存档时显示 BGM 曲目编号和播放状态。 |
| 1 | SE | 有效存档时显示音效编号。 |
| 2 | CARD | 进入卡片图鉴/卡片交换路径。 |
| 3 | GALLERY | 进入画廊场景和卡片详情路径。 |

无有效存档时，前两项由 `OptionsMenu_DrawEntries` 绘制为 `???`，这对应截图中的
两个黄色问号。该函数通过 `gOptionsMenuLabels` 访问 `0x087EB278` 的菜单文字表。

Option 声音测试变量：

| 新名 | 地址 | 作用 |
|---|---:|---|
| `gSoundTestSfxId` | `0x03000230` | 当前 SE 编号/选择值 |
| `gSoundTestBgmId` | `0x03000232` | 当前 BGM 编号 |
| `gSoundTestPlayingBgmIdPlusOne` | `0x03000233` | 正在播放的 BGM 编号加一，0 表示停止 |

这三个变量原先被误命名为 `gCardAlbumCursor`、`gCardAlbumPage` 和
`gCardCursorY`。卡片图鉴初始化时仍会使用相同地址区域的不同生命周期，不能仅
根据旧地址名推断它们永远属于图鉴。

## 9. Gallery、Card 和 SIO 路径

phase 22 至 26 是 Gallery/卡片详情，不是 Option 本身：

| phase | 新名 | 行为 |
|---:|---|---|
| 22 | `TITLE_PHASE_GALLERY_SCENE_LOAD` | 按卡片 ID 查背景场景，加载 Gallery tilemap 和卡片详情文字。 |
| 23 | `TITLE_PHASE_GALLERY_SCENE_FADEIN` | 等待详情场景淡入。 |
| 24 | `TITLE_PHASE_GALLERY_EXIT_FADEOUT` | 等待离开详情场景的转场完成。 |
| 25 | `TITLE_PHASE_GALLERY_SCENE_LOOP` | 更新详情场景。 |
| 26 | `TITLE_PHASE_GALLERY_RETURN_TO_OPTIONS` | 重建 Option 页面并返回 phase 19。 |

`0x08089B90` 既被存档地图解锁逻辑使用，也被卡片/Gallery 背景索引使用，因此
源码保留 `gCardBgMapTable` 作为共享 ROM 表视图，没有错误地拆成两个物理数组。

phase 27 以后是卡片交换：初始化 SIO、淡入、连接、发送卡片 ID、轮询结果、保存
接收卡片。`CardExchangePacket` 的低字节和高字节分别命名为 `cardIdLo` 和
`cardIdHi`，保持目标代码的 `strb/ldrb` 访问形状。

## 10. 输入和转场门控

`TitleMenu_ProcessFrame` 在 `gScreenTransitionState != 0` 时清零
`gNewKeysRaw` 和 `gHeldKeysRaw`。因此：

- Logo 淡入淡出期间按键不会被页面提前消费。
- PRESS START/主菜单/Option/Load 切换期间一次按键只属于当前页面。
- 页面文字和 tilemap 通常先写入 WRAM 缓冲，再由 VBlank pipeline 上传到 VRAM。
- `TitleMenu_UpdateUi` 的末尾还更新 UI 精灵和 SIO 链路任务。

## 11. 代码命名结果

| 原名 | 新名 |
|---|---|
| `sub_8011454` | `TitleMenu_ProcessFrame` |
| `sub_8012790` / `SaveUi_LoadScreen` | `TitleMenu_UpdateUi` |
| `sub_8013870` | `TitleMenu_DrawOptions` |
| `sub_8013934` | `OptionsMenu_DrawEntries` |
| `gUnk_03000234` / `gTitleFadeStep` | `gTitleAttractCountdown` |
| `gUnk_03000230` / `gCardAlbumCursor` | `gSoundTestSfxId` |
| `gUnk_03000232` / `gCardAlbumPage` | `gSoundTestBgmId` |
| `gUnk_03000233` / `gCardCursorY` | `gSoundTestPlayingBgmIdPlusOne` |

所有新名称均保留旧名同址兼容宏或链接别名，避免影响尚未完全语义化的调用点。
