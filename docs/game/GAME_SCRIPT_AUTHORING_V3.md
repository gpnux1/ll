# Lunar Legend 事件脚本 authoring 语言 v3 (独立命名)

> 本文档的命名完全基于 `src/script_vm.c` 里每个 opcode handler
> 的实际源码语义独立推导。不继承 `GAME_SCRIPT_AUTHORING_DESIGN.md`
> (v1) 与 `GAME_SCRIPT_AUTHORING_V2.md` (v2) 的任何命令名。
> canonical 汇编层见 `GAME_SCRIPT_LANGUAGE_DESIGN.md`。

## 0. 分析基础

本文档的所有命令名都从下面这条源码链反推得到，不使用原文档的命名：

1. `gScriptOpcodeHandlers` 表 (ROM 0x0862D434) 按 opcode 编号索引
   到 handler 函数指针数组。
2. 每个 handler 的 `*pScriptCursor += N` 决定该指令的字节宽度。
3. 每个 handler 内部 `pBytecode[K]` 的下标 K 就是运行时参数位。
4. Handler 返回 `0` 意味着"等下一帧再来" (阻塞)；返回 `1` 意味着
   "本帧推进" (非阻塞)。

由此推导出的 opcode → 参数签名映射表见 §4。命名规则见 §5。

## 1. 三条设计原则

1. **命令名从 handler 语义重推**：不看 `OP_*` enum 名、不看原文档，
   直接读 `src/script_vm.c` 里 handler 函数体，问"这个函数做了
   什么动作"。
2. **参数宽度写死在 §4 表里**：`u8`/`u16` 由 handler 源码里的
   `pBytecode[K]` 和 `pBytecode[K] | (pBytecode[K+1] << 8)` 决定，
   不由作者选。
3. **阻塞 vs 非阻塞显式标注**：§4 表每行给出返回值语义，作者可以
   判断"这行之后要不要等"。原文档从来没把这一层信息暴露出来。

## 2. 语法速览

```lls
; 顶层 (任意顺序)
mode = recover                     ; 元数据 (key = value)
baseline = "path/to.bin"
alias name = 0xNN                  ; 本文件别名
entry 0xNN label:                  ; 256 项入口之一
    cmd(arg0, arg1, ...)           ; 单条 VM 指令 (函数式)
    if.cond(flag, target)          ; 条件跳转
text label:                        ; 对白 token 序列
    face(0xNN)                     ; 说话人头像
    "..."                          ; 明文, 通过 charmap 展开
    newline() wait() clear()       ; 控制 token
    0xNNNN                         ; 裸 token (未知/保留)
raw(0xNN, "...")                   ; 原始字节段 (首字节是 opcode)
legacy(0xNN, "path")               ; 整段 entry 纳入某编号
```

块结束用**缩进到底**判定，不用 `end` 关键字。所有命令统一用
`name(arg0, arg1, ...)` 函数式调用；无参命令写 `name()`。

## 3. 元数据与文件头

```lls
mode = recover
baseline = "data/raw_data/unk_862E2A0.bin"
```

| key | 含义 |
|---|---|
| `mode` | `recover` (保留 baseline 未编辑 entry) 或 `author` (显式声明所有入口) |
| `baseline` | 参考 bin 路径；`recover` 模式必填 |

## 4. opcode 注册表 (从 handler 源码反推)

列: `编号 | v3 命令名 | handler 函数 | 参数 (字节序) | 返回值 | 说明`

`返回值` 中 `0=阻塞` 表示 handler 返回 0，需要下一帧再次调用；
`1=推进` 表示 handler 返回 1，脚本指针前进。

### 4.1 脚本控制

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x01 | `jump` | `Op_ScriptJump` | u8 entry | 1 |
| 0x02 | `call` | `Script_Call` | u8 entry | 1 (栈深度 ≤ 7) |
| 0x03 | `return` | `Op_ScriptReturn` | — | 1 (有栈) / 0 (空栈) |
| 0x05 | `nop` | `Op_Nop` | u8 pad | 1 |
| 0x06 | `stop` | `Op_ScriptStop` | u8 mode | 0 (仅第一次) |
| 0x0F | `call.alt` | `Op_ScriptCallAlt` | u8 entry | 1 |
| 0x15 | `stream.lz` | `sub_80512C4` | u8 set, u8 entry | 0/1 |
| 0x16 | `stream.return` | `sub_80513A0` | — | 0/1 |
| 0x0E | `jump.random` | `Op_RandomJump` | u8 min, u8 max | 1 |

`jump.random` 在 `[min, max]` 范围里取随机 entry 跳转；`min == max`
等价于 `jump`。

### 4.2 条件跳转

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x1E | `if.event` | `Op_IfEventFlagJump` | u16 flag, u8 target | 1 |
| 0x21 | `if.switch` | `Op_IfSwitchJump` | u16 flag, u8 target | 1 |
| 0x2F | `if.party` | `Op_IfPartyMemberJump` | u8 id, u8 count, u8 target | 1 |
| 0x3B | `if.item` | `Op_IfItemQtyJump` | u8 item, u8 target | 1 |
| 0x49 | `if.money` | `Op_IfMoneyJump` | u8 target, u16 amount | 1 |
| 0x3E | `if.save_loaded` | `Op_IfSaveLoadedJump` | u8 target | 0 (忙) / 0 |
| 0x41 | `if.save_timer` | `Op_IfSaveFlagJump` | u8 id, u8 target | 1 |
| 0x45 | `if.all_set` | `Op_IfAllFlagsJump` | u8 n, u8 target, [u16 flag × n] | 1 |
| 0x46 | `if.all_clear` | `Op_IfAllFlagsClearJump` | u8 n, u8 target, [u16 flag × n] | 1 |
| 0x47 | `if.any_set` | `Op_IfAnyFlagJump` | u8 n, u8 target, [u16 flag × n] | 1 |

Flag id 判定: `≤ 0x1FF` 走 `EventFlags_Test`；`> 0x1FF` 走
`SwitchFlags_Test(id - 0x200)`。这个规则对 `if.all_set`/
`if.all_clear`/`if.any_set`/`flags.set_many`/`flags.clear_many` 全部
生效。

`if.item` 的判定是 `gUnk_03004980[item] > 0x62` (0x62 = 98)；这是
源码里的硬编码阈值，不是作者可配置的参数。

`if.party` 统计 `gPartyMemberIds[0..4]` 中等于 `id` 的成员个数，
`count == expected` 才跳。

### 4.3 标志设置

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x1F | `flag.set` | `Op_SetEventFlag` | u16 flag | 1 |
| 0x20 | `flag.clear` | `Op_ClearEventFlag` | u16 flag | 1 |
| 0x27 | `switch.set` | `Op_SetSwitch` | u16 flag | 1 |
| 0x28 | `switch.clear` | `Op_ClearSwitch` | u16 flag | 1 |
| 0x48 | `switch.clear_all` | `Op_ClearSwitchTail` | — | 1 |
| 0x43 | `flags.set_many` | `Op_SetFlagsList` | u8 n, [u16 flag × n] | 1 |
| 0x44 | `flags.clear_many` | `Op_ClearFlagsList` | u8 n, [u16 flag × n] | 1 |

### 4.4 等待 (阻塞族)

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x26 | `wait.frames` | `Op_WaitFrames` | u8 n | 0 (未够) / 1 (够) |
| 0x10 | `wait.chars` | `Op_WaitCharsStop` | — | 0 (仍在动) / 1 |
| 0x13 | `wait.sprite` | `Op_WaitSpriteLoad` | — | 0 (未加载) / 1 |
| 0x19 | `wait.scene` | `Op_WaitSceneIdle` | — | 0 (过渡中) / 1 |
| 0x2B | `wait.camera` | `Op_WaitCameraPan` | — | 0 (平移中) / 1 |
| 0x2E | `wait.anim` | `Op_WaitCharaAnim` | u8 id | 0 (未完成) / 1 |
| 0x34 | `wait.anim_slot` | `Op_WaitAnimSlotIdle` | u8 slot | 0 (忙) / 1 |
| 0x38 | `wait.menu` | `Op_WaitMenuReady` | u8 item | 0 (未就绪) / 1 |
| 0x4B | `wait.logo` | `Op_WaitLogoFade` | — | 0 (未结束) / 1 |

### 4.5 音频

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x07 | `bgm.start` | `Op_BgmPlay` | u8 track, u16 loop | 0 |
| 0x08 | `bgm.stop` | `Op_BgmStop` | — | 0 |
| 0x09 | `bgm.volume` | `Op_BgmVolume` | u16 value | 0 |
| 0x0A | `bgm.fade_in` | `Op_BgmFadeIn` | u8 speed | 0 |
| 0x0B | `bgm.fade_out` | `Op_BgmFadeOut` | u8 speed | 0 |
| 0x0C | `sfx.play` | `Op_SfxPlay` | u8 id, u8 track, u8 flag | 0 |
| 0x0D | `sfx.stop` | `Op_SfxStop` | u8 track | 0 |

`bgm.volume` 的值是 `pBytecode[2] | (pBytecode[3] << 8)`，占 2 字节
(u16)；`bgm.start` 占 3 字节 (u8 + u16)。这些宽度全部从源码读出。

### 4.6 场景与地图

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x14 | `scene.fade` | `Op_SceneChangeFade` | u8 mode | 1 |
| 0x18 | `scene.plain` | `Op_SceneChangePlain` | u8 mode | 1 |
| 0x1B | `map.load` | `Op_LoadMap` | u8 npc, u16 move, u8 x, u8 y, u8 dir | 0 |
| 0x29 | `camera.snap` | `Op_CameraSnap` | — | 1 |
| 0x2A | `camera.follow` | `Op_CameraFollow` | — | 1 |
| 0x1A | `camera.pan` | `Op_CameraPan` | u8 dur, u16 x, u16 y | 1 |

`map.load` 的字节宽度是 7 (opcode 1 + 6 参数字节)：`u8 npc` + `u16
move` (小端) + `u8 x` + `u8 y` + `u8 dir`。返回值 0 意味着"本帧开始
场景装载"，需要跟 `wait.scene` 才继续。

### 4.7 角色控制

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x04 | `chara.exec` | `sub_804F280` | 可变子命令 | 0/1 |
| 0x11 | `chara.load_gfx` | `Op_LoadCharaGfx` | u8 slot, u16 gfx | 0 |
| 0x12 | `chara.load_pal` | `Op_LoadCharaPal` | u8 slot, u16 pal | 0 |
| 0x2D | `chara.anim` | `Op_RestartCharaAnim` | u8 id, u8 anim | 0 |
| 0x4C | `chara.set_level` | `Op_SetCharacterLevel` | u8 id, u8 level | 1 |

`chara.load_gfx` 的 slot 特殊: `slot == 0xFF` 时把 gfx id 写入
`gMoveCmdSetId` 并调用 `BgScroll_LoadFromTable`，否则作为角色槽 GFX。
所以 `chara.load_gfx 0xFF gfx_id` 是"装载地图移动集"，不是角色。

### 4.8 队伍与道具

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x1C | `party.remove` | `Op_RemovePartyMember` | u8 id | 1 |
| 0x1D | `party.add` | `Op_AddPartyMember` | u8 id | 1 |
| 0x39 | `party.heal_all` | `Op_FullHealParty` | — | 1 |
| 0x3A | `item.equip` | `Op_EquipItem` | u8 a, u8 b, u8 c | 1 |
| 0x4E | `item.give` / `item.take` | `Op_GiveTakeItem` | u8 id, u8 qty | 1 |
| 0x4F | `silver.add` / `silver.sub` | `Op_SilverAddSub` | u8 sign, u16 amount | 1 |
| 0x3C | `chest.open` | `Op_ChestOpen` | — (用 gUnk_03004860) | 0 |

`item.give` 与 `item.take` 共用 opcode 0x4E，源码按 `qty > 100`
区分：`qty > 100` → `sub_800AA84` (消耗，扣 `qty - 100`)；`qty ≤
100` → `sub_800AA60` (获取，加 `qty`)。作者写 `item.take id 150`
表示消耗 50。

`silver.add` 与 `silver.sub` 共用 opcode 0x4F，`sign != 0` 走加，
`sign == 0` 走减。

### 4.9 对话与窗口

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x00 | `dialog.message` | `sub_8050720` | 可变 (见 §7) | 0/1 |
| 0x17 | `dialog.text` | `sub_805144C` | 可变 (见 §7) | 0/1 |
| 0x33 | `dialog.choice` | `sub_8051BE4` | 可变 (见 §7) | 0/1 |
| 0x23 | `dialog.setup` | `Op_DialogSetup` | u8 a1..a5, u8 mode | 1 (mode≠1) / 0 (mode==1) |
| 0x24 | `window.open` | `Op_OpenWindow` | — | 1 |
| 0x25 | `window.close` | `Op_CloseWindow` | — | 0 |

### 4.10 动画槽 / 过场

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x2C | `cutscene.load` | `Op_LoadCutsceneAnim` | u16 anim, u8 x, u8 y | 0 |
| 0x30 | `anim.load` | `Op_LoadAnimSet` | u8 slot, u8 set | 1 |
| 0x31 | `anim.resume` | `Op_AnimSlotResume` | u8 slot | 1 |
| 0x32 | `anim.pause` | `Op_AnimSlotPause` | u8 slot | 1 |

### 4.11 菜单

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x35 | `menu.load_anim` | `Op_MenuLoadAnims` | u8 range, u8 mode | 1 |
| 0x36 | `menu.unlock` | `Op_MenuUnlock` | u8 item | 1 |
| 0x37 | `menu.lock` | `Op_MenuLock` | u8 item | 1 |

### 4.12 存档

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x3D | `save.ui` | `Op_SaveUiTrigger` | u8 mode | 0 |
| 0x3F | `save.timer_add` | `Op_SaveTimerA` | u8 id | 1 |
| 0x40 | `save.timer_sub` | `Op_SaveTimerB` | u8 id | 1 |
| 0x42 | `save.mark` | `Op_SaveOp` | u8 flag | 1 |

### 4.13 战斗 / Logo / 系统特效

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x22 | `battle.start` | `Op_ScriptBattle` | u8 a, u8 b, u8 c | 0/1 |
| 0x4A | `logo.fade` | `Op_StartLogoFade` | — | 0 |
| 0x4D | `sys.effect` | `Op_SysEffect` | u8 sub, u8 arg | 0/1 |

`battle.start` 是状态机: `gAfterBattleCounter == 0` 时按 `b` 设置
`gBattleResultType`，`c` 设置 `gUnk_030025B8` (负数 +0xBA，正数
+0x1C)，切换 `gGameState = GAME_STATE_BATTLE_ENTER`；`== 3` 时按
`a` 判定是否跳表。返回值 0 意味着本帧还在状态机里，需要下帧再调。

`sys.effect` 是通用系统特效分发器，`sub` 是子命令号，`arg` 是子参数
(非 0 表示"执行/开启"，0 表示"复位/关闭/续行")。子命令清单:

| sub 值 | 语义 | arg 含义 |
|---|---|---|
| 0x00 | 视口抖动 | 1/2 = 单/双 plane 掩码；≥ 3 = 掩码 7 |
| 0x01 | 白闪 | — |
| 0x02 | BG 层显示 | mapId 0x63 → BG2，否则 BG3 |
| 0x03 | 相机缓动 | 开/关 |
| 0x04 | 调色板渐变序列 | 1..5 = 子步号 |
| 0x05 | 存档 UI | Save_Fsm(1) / SaveUi_OpenLoad |
| 0x06 | 等按 A | 仅复位族，触发 Abort |
| 0x07 | OBJ 调色板渐显 | 10 帧渐变 |
| 0x08 | 存档 UI 调色板 | 仅复位族，4 步装载 |
| 0x4D | 全体满血 | `FullHealCharacter(arg)` |
| 0x64 | BGM 续播 | 仅复位族 |
| 0xC8 | 地图背景全量装载 | `MapBg_LoadFull(arg+0x81)` |
| 0xC9 | 开场背景装载 | `IntroBg_Load(arg)` |
| 0xCA | 白化淡入/淡出 | 1 = 淡入, 2 = 淡出 |

### 4.14 其他

| 编号 | v3 命令 | handler | 参数 | 返回值 |
|---|---|---|---|---|
| 0x28 | (alias `switch.clear_all`) | `Op_ClearSwitchTail` | — | 1 |

## 5. 命名规则 (从 handler 语义推导)

命令名格式: `<namespace>.<verb>` 或 `<verb>.<object>`。namespace 是
handler 触及的主资源类别。

| 规则 | 说明 |
|---|---|
| 动词导向 | `jump` `call` `return` `stop` `wait` `load` `set` `clear` `add` `remove` `start` `stop` `fade` |
| 阻塞族统一 `wait.*` | 所有返回 0 等下一帧的 handler 命名 `wait.<what>` |
| 条件跳转统一 `if.<what>` | 所有条件跳转 handler 命名 `if.<condition>` |
| 批量族后缀 `_many` | `flags.set_many` `flags.clear_many` 表示一次多条 |
| 二选一命令用 `/` 分隔 | `item.give / item.take` `silver.add / silver.sub` 共用 opcode |
| `chara.exec` 保留 | 0x04 是可变长子协议，尚未反编译，用 `exec` 而非猜测名字 |

## 6. 完整示例：ScriptSet 001 Entry 01

对应 `NEWGAME_TITLE_TO_OPENING_FLOW.md` 描述的开场宿主入口。

```lls
mode = recover
baseline = "data/raw_data/unk_862E2A0.bin"

alias burg_home = 0x0B

entry 0x01 new_game:
    ; 存档继续路线 (顺序不能改，否则命中条件漂移)
    if.all_set(1, 0x0A, 0x029E)
    if.all_set(1, 0x09, 0x0054)
    if.all_set(1, 0x08, 0x0039)
    if.all_set(1, 0x07, 0x0402)
    if.all_set(1, 0x06, 0x0038)
    if.all_set(1, 0x05, 0x0401)
    if.all_set(1, 0x03, 0x0010)
    if.all_set(1, 0x02, 0x0000)     ; ROM 特殊值，保留

    bgm.stop()
    stream.lz(0x00, 0x00)
    map.load(0x01, 0x01, burg_home, 0x04, 0x04)
    chara.exec(0x4C, 0x03, 0x3E, 0x5A, 0x02)
    chara.exec(0x4C, 0x04, 0x41, 0x59, 0x00)
    chara.exec(0x4C, 0x05, 0x47, 0x52, 0x02)
    chara.exec(0x4C, 0x06, 0x46, 0x5C, 0x00)
    chara.exec(0x4C, 0x07, 0x40, 0x5E, 0x00)
    chara.exec(0x4C, 0x09, 0x42, 0x4E, 0x06)
    chara.exec(0x4C, 0x0C, 0x38, 0x55, 0x04)
    chara.exec(0x4C, 0x0D, 0x3B, 0x51, 0x04)
    chara.exec(0x4C, 0x0E, 0x3B, 0x5C, 0x00)
    chara.exec(0x4C, 0x11, 0x4A, 0x57, 0x02)
    scene.plain(0)
    wait.scene()
    stop(1)

entry 0x0A resume_0a:
    dialog.message(0x00, opening_song)
    stream.return()

text opening_song:
    face(0x37)
    "これ ルーナの声じゃないか?"
    newline()
    "泉の方から聞こえるぜ"
    newline()
    "行ってみよう アレス"
    wait()
    clear()
```

`chara.exec` 是 0x04 的子命令流，第一个字节是子命令号，其余是子参
数。目前源码 `sub_804F280` 未反编译，所以不猜测子命令名；只保留
字节序列。

## 7. 对白 token

`dialog.message` / `dialog.text` / `dialog.choice` 三个 handler 都
包含自定义 16 位 token 数组。本文档不深入 token 语义 (它们由
`charmap.txt` 和已确认的汉字表决定)，只定义 authoring 写法:

```lls
text label:
    face(0xNN)          ; u16 token: 0x0DNN (说话人头像)
    "..."               ; 明文，通过 charmap 展开为 u16 token 序列
    newline()           ; u16 token: 0x0000
    wait()              ; u16 token: 0x0700
    clear()             ; u16 token: 0x0F00
    0xNNNN              ; 裸 token (未知/保留)
```

`face` 是日式 RPG 对话系统的标准术语（RPG Maker / Pokémon / Dragon Quest
系列都用 face / faceplate 表示"说话人头像"）。早期文档用 `portrait` 偏向
美术/摄影语义，改回 `face` 更贴合作者日常用语。

编译器必须逐 token 输出到 canonical `.s`，十六进制值和解码结果并列。

## 8. raw 段

```lls
raw(0x04, "4d 00 02 02 02 14 fd")
raw(0x04, "4d c9 00")
legacy(0x00, "raw/cutscene_00.bin")
```

- `raw(opcode, "bytes")`：内联字节，opcode 必须是第一个字节
- `legacy(entry, "path")`：把整段 entry 纳入某个编号

编译器只检查 opcode 匹配和边界，不猜长度、不插等待、不做重定位。

## 9. 编译器检查项

1. 每条命令的参数宽度必须与 §4 表完全一致。
2. `if.*` / `jump.*` 的目标必须是同一脚本集的 entry label 或编号。
3. `text` 块内不允许 VM 命令；命令块内不允许 token。
4. `raw` 段不能越 entry 边界；首字节必须等于声明的 opcode。
5. `entry` 编号唯一；`mode = author` 下必须显式声明所有入口。
6. `mode = recover` 下未修改的 entry 必须与 baseline 完全一致。
7. `item.give` / `item.take` 的 qty 语义由 100 阈值决定，编译器必须
   在 lint 中提示 `qty > 100 = 消耗 qty-100`。
8. `chara.load_gfx` 的 slot 0xFF 走 `gMoveCmdSetId`，lint 应提示。

## 10. 迁移路径

v3 是独立命名空间，不覆盖 v1/v2。建议:

1. 先实现 §4 opcode 注册表为单一数据源 (TOML 或 Python 模块)。
2. 基于注册表生成 parser 的命令白名单，避免手写。
3. 用 `NEWGAME_TITLE_TO_OPENING_FLOW.md` 的开场流程做 round-trip
   测试 (§6 的示例)。
4. 反汇编器必须从同一注册表生成，保证 authoring → canonical →
   disasm 三方对齐。

在 §4 注册表全部通过逐字节验证前，v3 不覆盖 v1 的权威地位。
