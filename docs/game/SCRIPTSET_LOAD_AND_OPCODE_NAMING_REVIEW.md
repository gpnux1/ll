# ScriptSet_Load 参数与脚本 Opcode 命名复核

## 1. 结论

`ScriptSet_Load(1, 0, 1)` 不能解释成“装载脚本集 1 的入口 0”。它的三个参数在
当前 ROM 中实际是：

| 参数 | 当前原型名 | 已确认语义 |
|---|---|---|
| 第 1 个 | `setId` | `gScriptSetTable[setId]` 的脚本集编号；同时写入当前脚本集状态，脚本结束时用于恢复环境脚本集。 |
| 第 2 个 | `entry` | 只有 `mode == 2` 时才立即/异步选择入口表项；`mode == 1` 时完全不读。 |
| 第 3 个 | `mode` | `1`（以及 default）从解压区正文基址开始；`2` 使用第 2 个参数选择入口，并设置异步解压完成后的入口跳转。 |

因此新游戏路径是：

```text
ScriptSet_Load(1, 0, 1)
    -> 载入 ScriptSet 001，脚本 PC = 0x02016200
ScriptPump_JumpToEntry(1, 2)
    -> 用入口表 [1]，脚本 PC = 0x02016200 + entryTable[1]
    -> ScriptSet 001 Entry 01 = 0x0202
```

第二个参数的 `0` 是 mode 1 下的占位值，不表示 Entry 00。

## 2. `ScriptSet_Load` 的证据

`src/script_vm.c:890` 的实现先执行：

```c
gScriptReturnSetId = setId;
lzData = (struct LzHeader *)gScriptSetTable[setId];
```

随后根据 `REG_DISPCNT & 0x80` 选择一次性解压或流式解压。入口相关分支是：

```c
switch (mode)
{
case 1:
default:
    gScriptCursor = 0x02016200;
    break;
case 2:
    gScriptPendingEntry = entry;
    gScriptVmFlags |= 0x400;
    gScriptCursor = 0x02016200 + jtbl[entry];
    break;
}
```

`ScriptPump_ServiceFrame` 在流式解压完成时再次使用
`gScriptPendingEntry` 跳到入口表项；所以 mode 2 的 `entry` 同时适用于当前已经可用
的入口表和异步完成后的入口跳转。

`gScriptReturnSetId` 这个旧名也不够准确。它不是一个只在“返回”时使用的参数，而是
当前活动脚本集编号：`sub_80512C4` 保存父脚本集后写入子集编号，
`sub_80513A0` 恢复父集编号，`Op_ScriptStop`/`Op_ScriptReturn` 结束脚本时才把它交给
`Script_SetEnvSet`。在没有完成全链改名之前，文档中建议使用候选名
`gActiveScriptSetId`，源码保留旧符号。

脚本集数据表给出的两个相关对象是：

```text
ScriptSet 000: ROM 0x0862D8A4, 解压大小 0x13F3 (5107)
ScriptSet 001: ROM 0x0862E2A0, 解压大小 0x73F1 (29681)
```

## 3. 真实的开场 Entry 01

对 `data/raw_data/unk_862E2A0.bin` 解压后，入口表前 12 项为：

```text
Entry 00 = 0x0200
Entry 01 = 0x0202
Entry 02 = 0x0280
Entry 03 = 0x031E
Entry 04 = 0x0390
Entry 05 = 0x03F0
Entry 06 = 0x0438
Entry 07 = 0x04E6
Entry 08 = 0x053A
Entry 09 = 0x05BE
Entry 0A = 0x0624
Entry 0B = 0x067E
```

ScriptSet 001 Entry 01 的实际字节和 handler 行为对应如下：

```text
0202  45 02 0A 9E 02     IfAllFlagsJump([0x029E], Entry 0A)
0207  45 02 09 54 00     IfAllFlagsJump([0x0054], Entry 09)
020C  45 02 08 39 00     IfAllFlagsJump([0x0039], Entry 08)
0211  45 02 07 02 04     IfAllFlagsJump([0x0402], Entry 07)
0216  45 02 06 38 00     IfAllFlagsJump([0x0038], Entry 06)
021B  45 02 05 01 04     IfAllFlagsJump([0x0401], Entry 05)
0220  45 02 03 10 00     IfAllFlagsJump([0x0010], Entry 03)
0225  45 02 02 00 00     IfAllFlagsJump([0x0000], Entry 02)
022A  35                  BgmStop
022B  15 00 00           CallScriptSet(set=0, entry=0)
022E  0E 01 01 00 0B 04  RequestMapLoad(mapNpc=1, moveSet=1,
     04                  spawn=(11,4), facing=4)
0235  04 05 4C 03 3E 5A  CharaControl(SetPosDir, chara=3,
     02                  pos=(62,90), dir=2)
...   ...                 (同样的 9 条角色定位命令)
027B  0C 00               StartPlainScreenEffect(mode=0)
027D  0D                  WaitScreenEffectIdle
027E  06 01               StopScript(resetPc=1)
```

这段字节流直接否定了旧 authoring 表中“`0x07` 是 BGM、`0x1B` 是地图、`0x18` 是
plain scene”的编号。开场使用的 BGM stop 是 `0x35`，地图请求是 `0x0E`，plain
screen effect 是 `0x0C`。

当前 `docs/scripts/script_set_001.txt` 的文件头仍显示解压大小 5107、Entry 01
为 `0x0CE1`，所以它实际上是 ScriptSet 000 的 listing。原因是
`scripts/disasm_script.py:617` 固定把 `unk_862D8A4.bin` 输出为
`script_set_001.txt`；该文件名不能作为 ScriptSet 001 的证据。

## 4. 唯一可信的 opcode -> handler 对照

`data/raw_data/gScriptOpcodeHandlers.bin` 是 80 个 little-endian Thumb 函数指针。
函数指针最低位为 Thumb 标记，去掉最低位后得到以下真实映射：

| opcode | handler | 当前函数名 | 以行为为准的稳定命令名 |
|---:|---|---|---|
| 00 | 08050720 | `sub_8050720` / `Op_DialogMessage` | `ShowDialogMessage` |
| 01 | 08052858 | `Op_ScriptJump` | `JumpToEntry` |
| 02 | 08052878 | `Script_Call` | `CallEntry` |
| 03 | 080511A0 | `Op_ScriptReturn` | `ReturnEntryOrStopScript` |
| 04 | 0804F280 | `sub_804F280` / `Op_CharaControl` | `CharaControl`（子协议） |
| 05 | 080528C4 | `Op_Nop` | `ReservedNoAdvance`（不能当普通 nop） |
| 06 | 08051230 | `Op_ScriptStop` | `StopScript` |
| 07 | 08052B80 | `Op_WaitCharsStop` | `WaitCharactersStopped` |
| 08 | 08052BA0 | `Op_LoadCharaGfx` | `LoadCharaGfxOrMoveSet` |
| 09 | 08052BE0 | `Op_LoadCharaPal` | `LoadCharaPalette` |
| 0A | 08052C04 | `Op_WaitSpriteLoad` | `WaitSpriteLoad` |
| 0B | 08052C24 | `Op_SceneChangeFade` | `StartScreenFadeEffect` |
| 0C | 08052C90 | `Op_SceneChangePlain` | `StartPlainScreenEffect` |
| 0D | 08052CD0 | `Op_WaitSceneIdle` | `WaitScreenEffectIdle` |
| 0E | 08052CF0 | `Op_LoadMap` | `RequestMapLoad` |
| 0F | 08052D4C | `Op_IfEventFlagJump` | `IfEventFlagJump` |
| 10 | 08052D8C | `Op_SetEventFlag` | `SetEventFlag` |
| 11 | 08052DAC | `Op_ClearEventFlag` | `ClearEventFlag` |
| 12 | 08052DCC | `Op_IfSwitchJump` | `IfSwitchFlagJump` |
| 13 | 08052E0C | `Op_SetSwitch` | `SetSwitchFlag` |
| 14 | 08052E2C | `Op_ClearSwitch` | `ClearSwitchFlag` |
| 15 | 080512C4 | `sub_80512C4` | `CallScriptSet` |
| 16 | 080513A0 | `sub_80513A0` | `ReturnScriptSet` |
| 17 | 0805144C | `sub_805144C` / `Op_DialogText` | `ShowDialogText` |
| 18 | 08052E4C | `Op_CameraSnap` | `CameraSnap` |
| 19 | 08052E6C | `Op_CameraFollow` | `CameraFollow` |
| 1A | 0804F64C | `Op_CameraPan` | `CameraPan` |
| 1B | 08052E80 | `Op_WaitCameraPan` | `WaitCameraPan` |
| 1C | 0804F768 | `Op_RemovePartyMember` | `RemovePartyMember` |
| 1D | 0804F7F8 | `Op_AddPartyMember` | `AddPartyMember` |
| 1E | 08052E9C | `Op_LoadCutsceneAnim` | `LoadCutsceneAnimation` |
| 1F | 08052EC0 | `Op_RestartCharaAnim` | `StartCharaScriptAnimation` |
| 20 | 08052F20 | `Op_WaitCharaAnim` | `WaitCharaAnimation` |
| 21 | 08052F44 | `Op_IfPartyMemberJump` | `IfPartyMemberPresentJump` |
| 22 | 0804F8D8 | `Op_ScriptBattle` | `StartBattleAndRouteResult` |
| 23 | 080528C8 | `Op_DialogSetup` | `ConfigureDialogWindow` |
| 24 | 08051A1C | `Op_OpenWindow` | `OpenDialogWindow` |
| 25 | 0805291C | `Op_CloseWindow` | `CloseDialogWindow` |
| 26 | 080529B8 | `Op_WaitFrames` | `WaitScriptFrames` |
| 27 | 08052FAD | `Op_LoadAnimSet` | `LoadAnimationSlotSet` |
| 28 | 08052FC9 | `Op_AnimSlotResume` | `ResumeAnimationSlot` |
| 29 | 08052FE5 | `Op_AnimSlotPause` | `PauseAnimationSlot` |
| 2A | 08053001 | `Op_WaitAnimSlotIdle` | `WaitAnimationSlotIdle` |
| 2B | 08053025 | `Op_MenuLoadAnims` | `ParseMenuEntityRange` |
| 2C | 08053041 | `Op_MenuUnlock` | `UnlockMenuEntity` |
| 2D | 0805305C | `Op_MenuLock` | `LockMenuEntity` |
| 2E | 08053078 | `Op_WaitMenuReady` | `WaitMenuEntityIdle` |
| 2F | 0805309C | `Op_FullHealParty` | `FullHealParty` |
| 30 | 080530B4 | `Op_EquipItem` | `EquipItem` |
| 31 | 080530D4 | `Op_GiveTakeItem` | `GiveOrTakeItem` |
| 32 | 08053104 | `Op_SilverAddSub` | `AddOrSubtractSilver` |
| 33 | 08051BE4 | `sub_8051BE4` / `Op_DialogChoice` | `ShowDialogChoice` |
| 34 | 08052A14 | `Op_BgmPlay` | `PlayBgm` |
| 35 | 08052A38 | `Op_BgmStop` | `StopBgm` |
| 36 | 08052A50 | `Op_BgmVolume` | `SetBgmVolume` |
| 37 | 08052A70 | `Op_BgmFadeIn` | `FadeInBgm` |
| 38 | 08052A8C | `Op_BgmFadeOut` | `FadeOutBgm` |
| 39 | 08052AA8 | `Op_SfxPlay` | `PlaySfx` |
| 3A | 08052ACC | `Op_SfxStop` | `StopSfxTrack` |
| 3B | 08053138 | `Op_IfItemQtyJump` | `IfItemStackFullJump` |
| 3C | 0805316C | `Op_ChestOpen` | `OpenChest` |
| 3D | 0805318C | `Op_SaveUiTrigger` | `OpenSaveUi` |
| 3E | 080531A8 | `Op_IfSaveLoadedJump` | `WaitSaveUiResultJump` |
| 3F | 080531E4 | `Op_SaveTimerA` | `IncrementSaveTimer` |
| 40 | 08053200 | `Op_SaveTimerB` | `DecrementSaveTimer` |
| 41 | 0805321C | `Op_IfSaveFlagJump` | `IfSaveTimerClearJump` |
| 42 | 08053254 | `Op_SaveOp` | `SetSaveFlag` |
| 43 | 08053270 | `Op_SetFlagsList` | `SetFlagList` |
| 44 | 080532DC | `Op_ClearFlagsList` | `ClearFlagList` |
| 45 | 0804F974 | `Op_IfAllFlagsJump` | `IfAllFlagsSetJump` |
| 46 | 0804FA04 | `Op_IfAllFlagsClearJump` | `IfAllFlagsClearJump` |
| 47 | 0804FA94 | `Op_IfAnyFlagJump` | `IfAnyFlagSetJump` |
| 48 | 08053348 | `Op_ClearSwitchTail` | `ClearSwitchRange` |
| 49 | 08053360 | `Op_IfMoneyJump` | `IfSilverGreaterThanJump` |
| 4A | 080533A0 | `Op_StartLogoFade` | `StartLogoEffect` |
| 4B | 080533B4 | `Op_WaitLogoFade` | `WaitLogoEffect` |
| 4C | 080533D4 | `Op_SetCharacterLevel` | `SetCharacterLevel`（候选，callee 仍未匹配） |
| 4D | 0804FB24 | `Op_SysEffect` | `SystemEffect` |
| 4E | 08052AE8 | `Op_RandomJump` | `RandomEntryJump` |
| 4F | 08052B34 | `Op_ScriptCallAlt` | `CallEntry`（与 0x02 的机器码/行为相同） |

表中 `0805xxxx` 是去掉 Thumb 标志后的函数地址；handler 表本身保存的是地址 + 1。

## 5. 当前名字中已经证明会误导的项目

### 5.1 `Op_LoadMap` 不是同步加载

源码只写入 `gMapNpcSetId`、`gMoveCmdSetId`、出生点和朝向，然后把
`gGameState` 改成 `GAME_STATE_SCENE_LOAD`，最后返回 0。真正的地图资源装载在下一
个游戏状态回调完成。因此 authoring 名应使用 `request_map_load`，并把“后续是否等待”
作为显式脚本时序，而不是把这条指令描述成同步 `load_map`。

### 5.2 `Op_SceneChangeFade` / `Op_SceneChangePlain` 不是地图切换

两个函数都只调用 `Palette_Backup`（部分 mode）和 `ScreenFx_SetMode`，并写入
`gScreenTransitionState`。它们不选择地图，也不调用地图装载器。建议 authoring 层使用
`screen.fade_start` 和 `screen.plain_start`；`wait.screen_idle` 对应 0x0D。

### 5.3 `Op_ScriptStreamLZ` / `Op_ScriptReturnChunk` 是脚本集调用/返回

0x15 保存父脚本集和宿主 PC，解压子脚本集并跳到子入口；0x16 弹出这些宿主信息并
恢复父脚本集。以新游戏开场为例，`0x15 00 00` 明确是“调用 ScriptSet 000 Entry
00”，不是普通的数据流操作。建议 authoring 名为 `script.call_set` /
`script.return_set`；保留 `LZ` 只作为实现注释。

### 5.4 `Op_LoadCharaGfx` 有第二种完全不同的参数语义

当 `data[1] == 0xFF` 时，`data[2..3]` 被写入 `gMoveCmdSetId`，并调用
`BgScroll_LoadFromTable`；否则才调用 `SetSlotGfxId` 设置角色槽图形。因此
`chara.load_gfx` 对 `0xFF` 情况会误导作者，至少应在语法层拆成
`actor.load_gfx` 和 `scene.load_move_set` 两个显式形式，底层仍共用 opcode 0x08。

### 5.5 `Op_RestartCharaAnim` 实际会重建精灵节点

函数先释放旧的 sprite chain，再分配节点，最后调用 `Chara_StartScriptAnim`。它不只
是“从当前帧重启动画”。更准确的 authoring 名是 `actor.start_script_anim`，旧名可
保留为源代码迁移别名。

### 5.6 `Op_IfPartyMemberJump` 实际是存在性判断

循环遇到第一个匹配成员就 `break`，所以 `count` 只有 0 或 1，不会统计同一成员的
多个实例。建议脚本语法写成 `if party_present(member) -> entry` 或
`if party_absent(member) -> entry`，不要向作者暴露一个看似可任意计数的参数。

### 5.7 `Op_MenuLoadAnims` 实际是菜单实体描述区间解析

函数调用的是 `MenuEnt_ParseRange`，不是动画资源加载器。该命令可以保留低层别名
`menu.parse_range`；`menu.load_anim` 不应作为 canonical 命令名。

### 5.8 存档四个 handler 的旧名过于宽泛或错误

| 旧名 | 实际行为 | 建议 authoring 名 |
|---|---|---|
| `Op_SaveUiTrigger` | `SaveUi_Open(param)`，进入 `GAME_STATE_TEXT` | `save.open_ui` |
| `Op_IfSaveLoadedJump` | 等 `gSaveBusyA` 清零；`gSaveBusyB != 0` 时跳入口，否则跳过 | `save.wait_result_jump` |
| `Op_SaveTimerA/B` | 对半字节计数器 `gSaveTimers` 加一/减一（每项封顶 5） | `save_timer.inc/dec` |
| `Op_IfSaveFlagJump` | `SaveTimer_Get(id) != 0` 时跳过，为 0 时跳转 | `if save_timer_clear` |
| `Op_SaveOp` | `SaveFlag_Set(id)` | `save_flag.set` |

“save loaded”“save flag”“save operation”都不能作为这些 handler 的精确语义。

### 5.9 物品和金钱条件的边界必须写进语言

`Op_IfItemQtyJump` 判断的是 `gInventory[item] > 0x62`。由于普通物品加法上限是 99，
这实际上是“物品堆已经达到 99”的分支，不是任意数量比较。

`Op_IfMoneyJump` 使用严格的 `gSilverAmount > threshold`，不是 `>=`。authoring
语法必须叫 `greater_than`，不能写成 `at_least`。

`Op_GiveTakeItem` 用数量大于 100 的哨兵编码消耗操作：`qty <= 100` 为获得，
`qty > 100` 为消耗 `qty - 100`。 `Op_SilverAddSub` 则用 sign 字节区分增加/减少。
这两个底层编码都应该由编译器显式验证，不能让作者直接猜哨兵值。

## 6. 暂不应正式改名的项目

- `Op_ScriptCallAlt` 与 `Script_Call` 的 ASM 完全相同。0x4F 在脚本中确实被用于另一
  组公共动作，但目前没有业务层差异证据；可在 authoring registry 中把二者都映射到
  `call_entry`，源码中先保留 `Alt` 地址名。
- `Op_Nop` 的 handler 只有 `bx lr`，既不推进 PC，也没有明确的返回值。当前已解析的
  ScriptSet 000/001 中没有实际 opcode 0x05。它应作为 `reserved/unknown`，不要提供
  普通 `nop()`，否则很容易生成停在同一字节的脚本。
- `Op_SetCharacterLevel` 的 wrapper 参数确实传给 `sub_800A3C8(id, level)`，但 callee
  尚未匹配完成；“等级”由数据流强烈支持，仍应在源码改名前保持候选状态。
- `Op_SysEffect` 包含抖动、白闪、BG 层、调色板序列、存档 UI、地图背景和白化等多个
  子命令。它适合保留为 `system.effect(subop,arg)`，不要拆成会隐藏原始 0x4D 的高层
  语句，直到每个子命令都有独立 round-trip 测试。

## 7. 对 authoring 设计的直接约束

1. opcode registry 必须从 `gScriptOpcodeHandlers.bin`、handler 源码/ASM 和已验证脚本
   样本三方生成或校验；`include/script_vm.h` 和旧 authoring 文档不能作为编号来源。
2. canonical 名称要描述实际副作用：`request_map_load`、`screen.fade_start`、
   `script.call_set`、`save.wait_result_jump`，而不是把下一帧状态机动作伪装成同步调用。
3. 所有可能返回 0 的 handler 要在 registry 中标为阻塞/暂停，并保留脚本指针推进量；
   编译器不自动插入 `wait`。
4. `CharaControl` 是 opcode 0x04 下的独立子协议。已经确认的 `0x4C` 子命令可以写
   成 `actor.position`；其他子命令暂时使用完整 raw 字节，不能与顶层 opcode 0x4C
   混淆。
5. 对 `0x31`、`0x32`、`0x3B`、`0x3E`、`0x41`、`0x49` 等历史哨兵/状态机命令，语法
   层应该使用结构化参数，但 canonical listing 必须同时打印原始字节。
6. ScriptSet 000 与 ScriptSet 001 必须分别生成 listing。任何文件名、entry 偏移或
   opcode 名称与解压头不一致时，工具应直接报错，不应静默覆盖另一个脚本集。

## 8. 本轮状态

本轮只完成取证和命名审查，没有修改 `src/`、`ll.cfg`、函数真名或链接布局。尤其没有
把候选名直接替换进已经匹配的函数，以免在证据不足时引入全链改名和字节回归。

证据来源：

- `src/script_vm.c:890-975`：脚本集加载、入口选择、VM 状态位。
- `src/script_vm.c:1028-2009`：已匹配 opcode handler 的实际副作用。
- `asm/nonmatchings/sub_80512C4.s` / `sub_80513A0.s`：嵌套脚本集保存/恢复机制。
- `data/raw_data/gScriptOpcodeHandlers.bin`：80 项真实 handler 指针表。
- `data/raw_data/unk_862E2A0.bin`：ScriptSet 001 的压缩数据和 Entry 01。
- `docs/game/NEWGAME_TITLE_TO_OPENING_FLOW.md`：新游戏调用链和状态时序。
