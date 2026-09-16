# HEADER-SPLIT-CODE0: remove include/code_0.h and add module headers

- 日期: 2026-09-16
- agent: opencode-header-split
- 状态: 已完成，全量 `make` + `sha1sum -c ll.sha1` 通过。

## 1. 结果

删除旧大杂烩头：

```text
include/code_0.h
```

新增/扩展为按当前 `src` 模块划分的 API 头：

```text
include/battle_types.h
include/engine_core.h
include/scene_mgr.h
include/sprite_engine.h
include/vram_transfer.h
include/map_view.h
include/anim_slot.h
include/player_stats.h
include/menu_ui.h
include/save.h
include/menu.h
include/text_engine.h
include/battle_task_services.h
include/battle_object_engine.h
include/battle_menu_windows.h
include/battle_stage_actor.h
include/battle_stage_transition.h
include/battle_stage_dialogue.h
include/battle_stage_effects.h
include/battle_stage_state.h
include/battle_flow_rules.h
include/battle_palette_wipe.h
include/battle_special_targets.h
include/battle_itemuse_rewards.h
include/battle_fx.h
include/script_vm.h
include/sound.h
```

## 2. `code_0.h` 的拆分策略

`include/code_0.h` 原本有三类内容：

1. 共享对象类型：`ObjHead`、`BattleObj`、`ObjAnimIdxBlk`、`ObjAnimEntry`、`struct ObjFadeSeq`、`gUnk_08393B28` extern。
   - 迁入 `include/battle_types.h`。
2. 函数原型和少量别名宏。
   - 按 `functions.tsv` 当前 `module` 列分组迁入对应模块头。
   - K&R 原型保持 code_0 原样，不补参、不改返回类型，避免改变已匹配函数 codegen。
3. `iwram.h` 间接包含 `menu.h` 造成的循环风险。
   - 新头文件大多会 include `iwram.h` / `battle_types.h`，因此把 `iwram.h` 对 `menu.h` 的包含移到 `iwram.h` 末尾。
   - `menu.h` 现在可 include `iwram.h` 与 `battle_types.h`，但实际包含链不会看到未定义类型。

## 3. 源文件 include

所有原 `#include "code_0.h"` 文件已替换为模块头集合：

- 每个 C 文件包含自身模块头（若存在）。
- 包含 `battle_types.h`。
- 根据文件中出现的函数名、别名宏和共享类型自动补其它模块头。
- 去重 include 行，避免同一 header 多次出现。

示例：

```c
#include "battle_types.h"
#include "battle_object_engine.h"
#include "battle_flow_rules.h"
#include "battle_palette_wipe.h"
...
```

## 4. 修正的模块划分判断

本次根据声明归属进一步确认：

- `battle_task_services.c` 不是纯 SIO；SIO 只占开头一段，主体是 BattleTask 运行服务。
- `battle_stage_*` 系列都属于同一套物件 stage 引擎，拆分按 handler 群而非旧 `event_*` 名。
- `battle_itemuse_rewards.c` 是道具使用/掉落/目标缓存，不是通用池。
- `battle_palette_wipe.c` 是 OBJ/BG palette 动画与 wipe，不是泛动画。

## 5. 验证

```bash
touch src/*.c
make
sha1sum -c ll.sha1
```

结果：

```text
ll.gba: 成功
匹配进度: 895/1059 (84.5%)
```

全量 SHA1 通过，说明：

- 删除 `code_0.h` 未改变 ROM 字节。
- 函数原型搬运未改变 codegen。
- 共享类型搬入 `battle_types.h` 未改变 struct 布局。

## 6. 已知 warning

编译仍有多处旧 warning，例如：

- `implicit declaration of Display_ShutdownSequence`
- `implicit declaration of System_SoftReset`
- `LZ77UnComp*` 指针/qualifier warning
- menu VRAM buffer macro redefinition

这些多数在拆头前也存在，因为 `code_0.h` 原本就没有这些原型；本次不新增函数原型，避免影响已匹配函数字节。后续如需清 warning，应逐函数 bytecmp/fncheck 验证。
