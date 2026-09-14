# handoff: 别名 #define 全量清除 (2026-09-14, agent=zcode-dlg-9xx)

## 任务
用户指令: 去掉所有 `#define gUnk_03000DDE gItemUseFxState` 形式的别名 define, 全仓直接使用语义名。

## 范围与结果
共 **55 个别名 define** 全部删除 (iwram.h 50 / script_vm.h 2 / data_805769C.h 2 / anim_slot.h 1):
- **gUnk→语义 (34)**: logo (3)、sound test/卡片 (6)、menu (2)、ItemUseFx (3)、BattleFx (3)、
  BattleDrops (2)、ScriptVm (5)、DialogWindow (2)、CardExchange/TitleFade (2)、Save (3)、
  AnimSlots (1)、ROM 数据表 3 (gScriptSetTable/gMapNpcSlotGroups/gSkillLearnTable)。
- **语义→gUnk 反向 (12)**: 相机平移 6 (gCameraPan*)、待载头像 3 (gPendingPortrait*)、
  窗口过渡 2 (gWindowTransitionProgress/Snapshot)、gScriptVmFlags (反向桥)。
  统一以语义名为准 (与用户意图一致: 语义命名胜出)。
- **语义→语义 (9)**: gCardAlbumCursor/gCardAlbumPage/gCardCursorY/gTitleFadeStep/
  gMainGameState/gMainTaskSlot/gCutsceneActive/gWindowTransitionScanlineTable/gSceneSubState —
  右名为准 (多数 0 使用, 纯死别名)。

## 修改面
- 头文件: 删 define + extern 声明改用语义名 (iwram.h 含 11 处反向符号的声明改名)。
- 源码: engine_core/save/menu/text_engine/battle_engine/player_stats/sprite_engine/
  scene_mgr/anim_slot/script_vm/data_87E83F0/data_805769C 内旧名 token (含注释) → 语义名。
- linker.ld: 删 41 个 gUnk 旧标签 (语义名标签已存在的同址重复), 4 个改名
  (gPendingPortraitGfx/Palette/Slot, gWindowTransitionProgressSnapshot — 原本只有旧名);
  另修正 sio_battle→battle_fx 的段指派 (并行会话文件改名遗留)。
- 预处理等价性: 旧 token 本经 define 展开为语义名, 删 define + 直用语义名后
  预处理输出不变 → 代码零字节影响 (全量 sha1 证实)。

## 事故记录 (rule: 修改 linker.ld 必须人工逐行核对)
第一版脚本重建 linker.ld 时正则 key 写错 (捕获后缀而非完整符号名), 误删 324 行 gUnk 标签;
恢复时又错位了 5 处 (0x188/0x198 互换、0x1B4 gItemUseCtx 后置、0x200 gScreenIdleIconCursor
后置、0x9BE-0x9C8 块插入 0x949-0x97D 之间、AE0-CE8 块乱序), 链接报
"cannot move location counter backwards"。已按用户指令**手动逐处修复**
(对照 git HEAD 原序, Edit 逐处移动), 只读单调性校验通过 (唯一"倒退"= EWRAM→IWRAM 段边界重置)。
并行会话同时在改 sio_battle.c→battle_fx.c 造成的一次 ld "cannot find src/sio_battle.o"
竞态, 经用户提示在 linker.ld:758 手改为 `src/battle_fx.o(.text);`。

## 验证
- 全量 `make` + `sha1sum -c ll.sha1`: **ll.gba OK** (869/1059, 82.1%)。
- src/include/linker.ld 中 45 个旧别名符号 0 残留; `#define g\w+ g\w+` 别名 0 残留。
- ll.map: 4 个改名标签落位 (0x3004668/0x3004694/0x30047cc/0x300483c)。
- 代码侧未动任何函数逻辑, 无需 fncheck (预处理等价)。

## 备注
- iwram.h 中存在少量同地址双标签 (如 0x230 gCardAlbumCursor/gSoundTestSfxId 并存,
  并行会话所加), 无害, 未动。
- sio_battle.c→battle_fx.c 为并行会话所做改名, 本任务只修 linker.ld 段指派适配。
