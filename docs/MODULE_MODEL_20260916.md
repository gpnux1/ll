# 2026-09-16 基于 src 代码的模块模型与 C 文件改名

> 本文件按“函数体实际读写的状态/硬件/表”重新归类，不把旧函数名当证据。  
> 本次只改物理文件名、linker 对象名和 `functions.tsv` 的 `module` 列，不移动函数，因此 ROM `.text` 顺序不变，全量 SHA1 保持一致。

## 1. 原则

1. `src/*.c` 文件名必须表达该地址段内主要消费的数据对象与职责，而不是沿用旧 `sub_XXXXXX` 文件命名。
2. 不移动函数。原因是 GBA `.text` 地址由 `linker.ld` 中对象顺序 + 对象内函数顺序共同固定；物理移动会改变函数地址或迫使重新拆分 linker 段，需要单独按函数逐批验证。
3. 一个物理文件若包含多个地址簇，以主体簇命名，并把混入事实写进文档；后续如需拆物理 TU，应在 linker 中保持对象顺序逐段拆分。
4. 本模型优先使用地址段中反复出现的状态变量、DMA/硬件寄存器、池地址、ROM 表，而不是函数名。

## 2. 本次改名映射

| 旧文件 | 新文件 | 地址段 | 改名依据 |
|---|---|---|---|
| `src/sio_link.c` | `src/battle_task_services.c` | `0x080170BC..0x0801A3A8` | 前段确有 SIO，但主体是 BattleTask 运行服务：`BattleTask_Run`、对象链表、`GetObjPool`、LCG RNG、`gGstate324` 状态字、按键/连发、DialogCtx、flash/wave、BG 装载、tilemap 文本数字辅助。旧名只覆盖开头少数函数。 |
| `src/battle_obj_core.c` | `src/battle_object_engine.c` | `0x0801A3C4..0x080210C0` | 核心消费 `ObjHead.scriptPtr/palBitsPtr/gfxBaseIdx/gfxTotal`、`gUnk_08393B28`、`gUnk_087EBE00`、`BattleObj.animPtr`、伤害数字队列、对象入队。比 `core` 更准确地表达 BattleObj/ObjHead 引擎。 |
| `src/scene_obj_fx.c` | `src/battle_menu_windows.c` | `0x08021130..0x080260BC` | 函数体集中写 `gMenuSlotStates`、`gMenuObjLoadSlots/Count/Idx/Phase`、`gMenuWindowPhase/Flags`、`gMenuList*`、`gMenuSel*`、tilemap/OAM。不是泛场景 FX，而是战斗菜单窗口/对象装载。 |
| `src/event_actor.c` | `src/battle_stage_actor.c` | `0x080264C0..0x080313EC` | 主体消费 `gObjActStep/Timer/SavedF2A/SavedPal/SavedX/SavedY/MoveFromX/Y/GroupSlots`，调用对象动画装配、SFX、淡出、调色板层；这是 actor 舞台演出 handler。 |
| `src/scene_interact.c` | `src/battle_stage_transition.c` | `0x08031580..0x080323B4` | 函数体集中消费 `gSceneTransStep`、`gSceneFadeOut/In`、`gObjActStepTimer`，用 textbox 与插值推进场景/战斗转换阶段。不是普通地图交互。 |
| `src/event_hub.c` | `src/battle_stage_dialogue.c` | `0x08032548..0x0803F658` | 大量函数为 NPC/对话/文字选择/剧情演出 handler，使用 `gObjActStep`、keys、DialogCtx 相关服务和目标槽表；同时包含 stage 收尾服务 `sub_803F5B4/sub_803F658`。 |
| `src/cutscene_mgr.c` | `src/battle_stage_effects.c` | `0x0803FF54..0x08043F90` | 实际是战斗对象效果/事件阶段 handler 群，操作 `gObjActStep/Timer/SavedPal/F2A/X/Y`、对象动画、palette release。不是独立 cutscene manager。 |
| `src/obj_state.c` | `src/battle_stage_state.c` | `0x08044394..0x08044738` | 小型服务簇：stage 状态复位、SFX latch、wait busy 计时、伤害命中数、group slots、`gObjSlotFxCmd` 写命令。它是 stage 引擎工作区服务，不是通用 obj state。 |
| `src/battle_engine.c` | `src/battle_flow_rules.c` | `0x0804473C..0x0804AD24` | 覆盖效果计算、HP/MP/Skill/EXP/LevelUp、目标槽筛选、状态异常、回合行动槽、战斗对话结果屏、能力值重算、BGM 演出请求。属于战斗规则与流程，不只动画。 |
| `src/battle_anim.c` | `src/battle_palette_wipe.c` | `0x0804AD54..0x0804D0F8` | 主要全局为 `gObjPalAnim`、`gBgPalAnim`、`gObjPalSlotUsed`、`gBgPalSlotUsed`、`gWipeCtl/Desc/Progress/Oam*`、OBJ/BG palette DMA (`0x05000000/0x05000200`)。旧名 `anim` 太泛，实际是调色板/擦除/开场视觉系统。 |
| `src/battle_rewards.c` | `src/battle_special_targets.c` | `0x0804D1B4..0x0804DCD8` | 16 个函数都是 slot≥0x71 特殊对象目标选择 handler：经 `gUnk_0839CE38[slot-0x71]` 分派，掷 `fxKind`，查 `gUnk_08393B28`，按 `targetMode` 写 `f_BD`。与战后奖励无关。 |
| `src/obj_pool.c` | `src/battle_itemuse_rewards.c` | `0x0804DD70..0x0804F17C` | 包含特殊对象分派、装备/道具槽搜索、`gInvPageDeltas`/`gObjInvBackup`、ItemUseFx 状态机、tilemap 道具栏绘制、BattleDrops 掉落、战斗 FX object 清理。不是简单对象池。 |

## 3. 仍不理想的文件

这些文件不是本次改名重点，或主体名仍可接受：

- `src/engine_core.c`：系统启动/VBlank/按键/主循环/LZ/HBlank 等，仍偏宽，但 `engine_core` 比任意子集更准确。
- `src/scene_mgr.c`：游戏/场景状态和转场薄层，名称可接受。
- `src/sprite_engine.c`：包含 sprite/OAM/actor/task dispatch，可未来拆 `map_actor_runtime` 与 `game_state_dispatch`，但移动函数需要单独验证。
- `src/vram_transfer.c`：实际同时处理 VRAM、palette、sprite queue、actor pending load；可未来改 `gfx_transfer.c`，本次不动。
- `src/anim_slot.c`：动画槽 + map assets + intro/logo/portrait/chest；名称偏窄，但主体确实有 `gAnimSlots` 簇。
- `src/player_stats.c`：前半是 palette/menu entity/static object，后半是 party stats/inventory/equip；建议未来拆两个连续段，本次不改，避免过度命名。
- `src/text_engine.c`：文本渲染 + message + item/skill menu + SIO packet，名称偏窄，但不明显误导。
- `src/menu.c` / `src/menu_ui.c`：两个菜单域重叠；现有 `menu.c` 更像 title/option/save/HUD，`menu_ui.c` 更像状态/背包/HUD 控件。暂保留。

## 4. 验证

```bash
make
sha1sum -c ll.sha1
```

结果：

```text
ll.gba: 成功
```

全量 ROM 字节未变；本次是文件/模块标签重划分，不是函数位移。


## 2026-09-16 anim_slot.c 拆分

`src/anim_slot.c` 原来混合了动画槽、地图区域、选项场景、头像/视口、屏幕 FX、精灵表/BG、宝箱等代码，文件名不合理。按连续 ROM `.text` 地址拆为：

```text
src/anim_slot_core.c            0x08007964..0x08007ADC
src/map_zone.c                  0x08007ADC..0x08007D5C
src/option_scene_loader.c       0x08007D5C..0x08008620
src/map_portrait_viewport.c     0x08008620..0x080088B4
src/screen_fx_loader.c          0x080088B4..0x08008A3C
src/sprite_bg_sheet_loader.c    0x08008A3C..0x08008CC0
src/map_misc_runtime.c          0x08008CC0..0x08008F28
src/chest_objects.c             0x08008F28..0x0800908C
```

原 `include/anim_slot.h` 改名为 `include/map_scene_runtime.h`，并生成各模块头。函数体未移动、函数名未改；仅按原地址连续切片，`linker.ld` 保持对象顺序。
