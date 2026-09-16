# SPLIT-ANIM-SLOT: anim_slot.c 按地址段拆分并重命名模块

- 日期: 2026-09-16
- agent: opencode-module-rename
- 状态: 已完成，全量 `make` + `sha1sum -c ll.sha1` 通过。

## 1. 问题

`src/anim_slot.c` 文件名不合理：40 个函数里只有动画槽核心函数与 `AnimSlot` 直接相关，同一地址段还混入：

- map zone 查找/触发；
- option scene / logo / choice menu；
- dialog portrait / viewport / intro bg；
- screen fade / wave / screen fx；
- BG tiles / sprite sheet reload；
- camera/script env/BG palette 辅助；
- chest objects。

## 2. 拆分方案

按连续 `.text` 地址拆成 8 个 C 文件，保持原函数顺序和 linker 对象顺序：

| 新文件 | 地址段 | 函数 |
|---|---|---|
| `src/anim_slot_core.c` | `0x08007964..0x08007ADC` | `AnimSlot_Parse`, `AnimSlot_ParseLoop`, `AnimSlot_Step` |
| `src/map_zone.c` | `0x08007ADC..0x08007D5C` | `sub_8007ADC`, `sub_8007BD0` |
| `src/option_scene_loader.c` | `0x08007D5C..0x08008620` | `MapBg_LoadInterior`, `Logo_LoadAssets`, `ChoiceMenu_BuildList`, `BattleIntro_Cursor`, `ChoiceMenu_HandleInput` |
| `src/map_portrait_viewport.c` | `0x08008620..0x080088B4` | `DialogPortrait_Set`, `Viewport_UpdateEffects`, `IntroBg_Load` |
| `src/screen_fx_loader.c` | `0x080088B4..0x08008A3C` | `ScreenFade_Start`, `AnimSlot_BankReload`, `Win0H_WaveDmaByVCount`, `ScreenFx_SetMode` |
| `src/sprite_bg_sheet_loader.c` | `0x08008A3C..0x08008CC0` | `AnimSlots_Release`, `AnimSlots_StepAll`, `BgTiles_LoadUiSet`, `BgScroll_LoadFromTable`, `PlayerSheets_Load`, `AnimSlot_LoadSet`, `AnimSlot_Pause`, `AnimSlot_Resume`, `AnimSlot_Active`, `ReloadSpriteSheet`, `ReloadAllSpriteSheets` |
| `src/map_misc_runtime.c` | `0x08008CC0..0x08008F28` | `ChoiceMenu_ResolveDest`, `DialogPortrait_FlushPending`, `Camera_GetDrawOffset`, `Script_SetEnvSet`, `BgPal_ResetFirst`, `AnimSlot_PlayOnce`, `BgMap_FillRow`, `MapBg_FlushPending` |
| `src/chest_objects.c` | `0x08008F28..0x0800908C` | `ChestObjects_LoadForMap`, `ChestObject_BuildSprite` |

## 3. 头文件

- `include/anim_slot.h` 改名为 `include/map_scene_runtime.h`，保留 AnimSlot struct 与旧 API 原型。
- 为每个新 C 模块生成：
  - `include/anim_slot_core.h`
  - `include/map_zone.h`
  - `include/option_scene_loader.h`
  - `include/map_portrait_viewport.h`
  - `include/screen_fx_loader.h`
  - `include/sprite_bg_sheet_loader.h`
  - `include/map_misc_runtime.h`
  - `include/chest_objects.h`

`iwram.h` 和原 include `anim_slot.h` 的调用点全部改为 `map_scene_runtime.h`。

## 4. 构建系统同步

- `linker.ld`: `src/anim_slot.o(.text)` 替换为 8 个对象，顺序保持 ROM 地址序。
- `functions.tsv`: 旧 `anim_slot` 行全部按地址改入新 module。
- `scripts/gen_debug_ld.py`: TEXT_ORDER 同步。
- 重新生成 `linker_debug.ld`。

## 5. 验证

```bash
rm -f build/src/anim_slot.o build/src/anim_slot.s build/src/anim_slot.i
make
sha1sum -c ll.sha1
```

结果：

```text
ll.gba: 成功
匹配进度: 895/1059 (84.5%)
```

函数名本次未改；后续可按模块继续分析函数命名。
