# Lunar Legend 事件脚本 authoring 语言 v2 (提案)

> 本文是 `GAME_SCRIPT_AUTHORING_DESIGN.md` (v1) 的可读性修订版。
> 底层约束、字节级保证、raw 逃生口、entry 编号规则和 canonical 对齐
> 与 v1 完全一致；本文只改作者看到的语法。
> canonical 层与 opcode 薄宏仍见 `GAME_SCRIPT_LANGUAGE_DESIGN.md`。

## 0. v1 的可读性痛点 (为什么需要 v2)

对照 `GAME_SCRIPT_AUTHORING_DESIGN.md` §4/§9 的实际示例，可以列出 6 个
具体痛点：

| # | v1 写法 | 问题 |
|---|---|---|
| 1 | `text opening_song_line: ... end` | 有的块用 `:` 缩进、有的块用 `end` 关键字结束，两种块语法并存 |
| 2 | `scene.load_map map=burg_home move_set=1 spawn=(11, 4) facing=left` | `=` 同时是分隔符和赋值符；`spawn=(11,4)` 元组 + 别名 + 裸数字混排 |
| 3 | `wait scene_idle` 与 `wait.frames 30` | 同一个 `wait` 既是名词也是动词前缀，作者要背两套模式 |
| 4 | `jump_if event 0x0039 -> phase_6` | `jump_if` 是 VM opcode 名，作者却要在源码里做"跳转目标 + 条件"的双向解读 |
| 5 | `actor.control raw "4d 00 00 00 01 04 fd"` | raw 字节塞在字符串里，改一个 byte 要重新拼字符串 |
| 6 | `entry 01 new_game:` | 编号前缀 `01` 与 label 在同一 token 组里，256 编号容易被看成普通整数 |

v2 不改语义、不改字节、不改 opcode 数量，只改这些**作者每天都要看的东西**。

## 1. 五条设计原则

1. **一条语句 = 一条 VM opcode**：与 v1 完全一致；作者看到的顺序即字节顺序。
2. **块用缩进到底结束，无 `end` 关键字**：所有块 (`entry` / `text` / `raw` /
   `alias`) 统一语法，学习成本减半。
3. **位置参数优先**：VM 的参数宽度、端序、顺序已经固定，源码用位置参数就是
   "字节顺序的散文形式"；命名别名一律用行尾注释标记，不再用 `key=value` 语法。
4. **动词式命令**：`wait_scene_idle()`、`wait_frames(30)`、`bgm_stop()`——
   命令名与 VM opcode 表 1:1 对应，作者可以在两个文件之间直接跳读。
5. **raw 是等宽 byte 列表**：raw 段就是 `.byte` 风格的等宽字节列表，可以
   和 canonical `.s` 逐行对齐；不再把字节塞进字符串。

## 2. 文件布局与元数据

```text
data/scripts/
    001_opening.lls         # authoring 源
    names.toml              # 别名 → 数值 (与 v1 一致)
    text/                   # 独立 token 文件 (可选)
    raw/                    # 未识别的整段正文
```

文件头允许任意数量 `key: value` 元数据；编译器只识别下列 key：

```lls
mode: recover                     # 或 author
baseline: "data/raw_data/unk_862E2A0.bin"
preserve: true                    # 保留未修改 entry
set: 0x01                         # 可选；缺省时从文件名取
```

## 3. 词法

| 类别 | 例子 | 说明 |
|---|---|---|
| 数字 | `0x01` `0xFF` `0x0001` | 前缀 `0x` 显式 hex；纯十进制仅用于 frame 数等 |
| 标识符 | `new_game` `bgm_stop` | 小写 snake_case |
| label | `new_game` `resume_0a` | 与标识符同形，作用域是 entry |
| 字符串 | `"泉の方から聞こえる"` | 只用于文本 token body |
| 注释 | `# ...` | 单行；也允许 `;` 作为汇编兼容写法 |
| 关键字 | `entry` `goto` `if` `call` `return` `text` `raw` `alias` `mode` | 保留 |
| 块标记 | `:` 结尾 | 块头部结束符；后面所有缩进行都属于该块 |

## 4. 顶层结构

一个 `.lls` 文件由三类顶级构造组成，顺序任意：

```lls
; metadata (可选)
mode: recover
baseline: "data/raw_data/unk_862E2A0.bin"

; alias 定义 (可选, 别名 → 数值)
alias:
    burg_home: 0x0B
    nash: 2

; entry (可多个)
entry 0x01 new_game:
    ...

entry 0x0A resume_0a:
    ...

; text block (可多个, 被 dialog 指令引用)
text opening_song_line:
    portrait 55
    "これ ルーナの声じゃないか?"
    [newline]
    "行ってみよう アレス"
    [wait]
    [clear]

; raw 整段 (可选, 用于 legacy entry)
legacy 0x00 "raw/cutscene_00.bin":
    note: "未解析的开场过场, 原 ROM 偏移 0x862E2A8 .. 0x862E2A8+len"
```

`alias:` 块把别名一次声明在本文件作用域；跨文件复用仍走 `names.toml`。

## 5. 命令 (每命令 = 一条 VM opcode)

命令名使用 `namespace.op` 或纯动词，两种风格共存：

```lls
; --- audio ---
bgm_stop()                        # OP_BGM_STOP        0x08
bgm_play(0xFB, 0x0000)            # OP_BGM_PLAY        0x07  music=0xFB loop=0
bgm_volume(128)                   # OP_BGM_VOLUME      0x09
bgm_fade_in(8)                    # OP_BGM_FADE_IN     0x0A
bgm_fade_out(8)                   # OP_BGM_FADE_OUT    0x0B
sfx_play(219, 0)                  # OP_SFX_PLAY        0x0C

; --- script control ---
goto resume_0a                    # OP_SCRIPT_JUMP     0x01
call common_dialog                # OP_SCRIPT_CALL     0x02
call_alt sub_script               # OP_SCRIPT_CALL_ALT 0x0F
return()                          # OP_SCRIPT_RETURN   0x03
return_chunk()                    # OP_SCRIPT_RETURN_CHUNK 0x16
stream(0x00, 0x00)                # OP_SCRIPT_STREAM_LZ 0x15  set=0x00 entry=0x00
stop(1)                           # OP_SCRIPT_STOP     0x06

; --- wait ---
wait_frames(30)                   # OP_WAIT_FRAMES     0x26
wait_chars_stop()                 # OP_WAIT_CHARS_STOP 0x10
wait_scene_idle()                 # OP_WAIT_SCENE_IDLE 0x19
wait_sprite_load()                # OP_WAIT_SPRITE_LOAD 0x13
wait_camera_pan()                 # OP_WAIT_CAMERA_PAN 0x2B
wait_chara_anim(3)                # OP_WAIT_CHARA_ANIM 0x2E
wait_anim_slot(0)                 # OP_WAIT_ANIM_SLOT_IDLE 0x34
wait_menu_ready(1)                # OP_WAIT_MENU_READY 0x38
wait_logo_fade()                  # OP_WAIT_LOGO_FADE  0x4B

; --- scene / camera ---
load_map(0x01, 0x01, 0x0B, 0x04, 0x04)   # OP_LOAD_MAP 0x1B  npc_set, move_set, x, y, dir
scene_fade(0)                          # OP_SCENE_CHANGE_FADE 0x14
scene_plain(0)                         # OP_SCENE_CHANGE_PLAIN 0x18
camera_snap()                           # OP_CAMERA_SNAP 0x29
camera_follow()                         # OP_CAMERA_FOLLOW 0x2A
camera_pan(32, 88, 32)                  # OP_CAMERA_PAN 0x1A  frames, x, y

; --- chars ---
load_chara_gfx(3, 3, 0)                # OP_LOAD_CHARA_GFX 0x11  id, x, y
load_chara_pal(3, 3, 0)                # OP_LOAD_CHARA_PAL 0x12
restart_chara_anim(3, 1)               # OP_RESTART_CHARA_ANIM 0x2D  id, anim
set_chara_level(3, 5)                  # OP_SET_CHARACTER_LEVEL 0x4C
chara_control(0x4D, 0x03, 0x3E, 0x5A, 0x02)  # OP_CHARA_CONTROL 0x04  子命令

; --- flags / switch / item / money ---
if_event_jump(0x0A, 0x029E)            # OP_IF_EVENT_FLAG_JUMP 0x1E  (见 §6 分支)
set_event_flag(0x0001)                 # OP_SET_EVENT_FLAG 0x1F
clear_event_flag(0x0001)               # OP_CLEAR_EVENT_FLAG 0x20
set_switch(0x0014)                     # OP_SET_SWITCH 0x27
clear_switch(0x0014)                   # OP_CLEAR_SWITCH 0x28
if_party_member(2, 1) goto nash_present  # OP_IF_PARTY_MEMBER_JUMP 0x2F  (见 §6)
if_item_qty(0x2A) goto has_key         # OP_IF_ITEM_QTY_JUMP 0x3B
if_money(300, GE) goto can_buy         # OP_IF_MONEY_JUMP 0x49
give_take_item(0xDD, 2, GIVE)          # OP_GIVE_TAKE_ITEM 0x4E
silver_add_sub(300, ADD)               # OP_SILVER_ADD_SUB 0x4F

; --- party ---
party_add(2)                           # OP_ADD_PARTY_MEMBER 0x1D
party_remove(4)                        # OP_REMOVE_PARTY_MEMBER 0x1C
party_full_heal()                      # OP_FULL_HEAL_PARTY 0x39
equip_item(0x2A)                       # OP_EQUIP_ITEM 0x3A

; --- dialog ---
dialog_setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)   # OP_DIALOG_SETUP 0x23
dialog_message(0x00, opening_song_line)            # OP_DIALOG_MESSAGE 0x00
dialog_text(0x06, 0x07, 0x00, map_sign)            # OP_DIALOG_TEXT 0x17
dialog_choice(ask_leave, choice_yes, choice_no)    # OP_DIALOG_CHOICE 0x33
open_window()                            # OP_OPEN_WINDOW 0x24
close_window()                           # OP_CLOSE_WINDOW 0x25

; --- save / cutscene ---
save_ui_trigger()                       # OP_SAVE_UI_TRIGGER 0x3D
if_save_loaded goto after_load          # OP_IF_SAVE_LOADED_JUMP 0x3E
if_save_flag_jump(0x0001) goto save_done # OP_IF_SAVE_FLAG_JUMP 0x41
save_op(1)                              # OP_SAVE_OP 0x42
script_battle(0x01)                     # OP_SCRIPT_BATTLE 0x22
load_cutscene_anim(0x00, 0x00, 0x00, 0x00)  # OP_LOAD_CUTSCENE_ANIM 0x2C
chest_open(0x01)                        # OP_CHEST_OPEN 0x3C

; --- ui / anim slot ---
menu_load_anims(0x01)                   # OP_MENU_LOAD_ANIMS 0x35
menu_unlock()                           # OP_MENU_UNLOCK 0x36
menu_lock()                             # OP_MENU_LOCK 0x37
load_anim_set(0x00)                     # OP_LOAD_ANIM_SET 0x30
anim_slot_resume(0)                     # OP_ANIM_SLOT_RESUME 0x31
anim_slot_pause(0)                      # OP_ANIM_SLOT_PAUSE 0x32

; --- sys effect (0x4D 通用特效) ---
sys_effect(0xC9, 0x00)                  # OP_SYS_EFFECT 0x4D  sub=0xC9 (IntroBg_Load)
sys_effect(0xCA, 0x01)                  # SYSFX_WHITEOUT in
sys_effect(0x00, 0x02)                  # SYSFX_SHAKE 双 plane

; --- misc ---
nop(0x00)                               # OP_NOP 0x05
start_logo_fade()                       # OP_START_LOGO_FADE 0x4A
```

规则：

- 每个命令在注释里同时给出 **VM opcode 名 + 数值 + 参数含义**，方便和
  `include/script_vm.h` 交叉核对。
- 命令不接受 `key=value` 语法；参数一律位置传入，参数数量必须与 opcode
  完全一致。
- `goto` 的**无条件**形式不写 `if`；条件形式见 §6。
- 别名解析规则：如果参数是标识符且能在 `alias:` / `names.toml` 里找到，
  展开为数值；否则按数字字面量处理。

## 6. 条件分支 (`goto ... if ...`)

v1 的 `jump_if event 0x0039 -> phase_6` 改成 **"goto 目标 if 条件"**，
方向与阅读顺序一致；条件使用 VM 已经存在的形式，不引入布尔表达式。

```lls
; --- 单事件标志 ---
goto phase_6 if event 0x0039

; --- 多事件全部满足 (对应 OP_IF_ALL_FLAGS_JUMP 0x45) ---
goto resume_0a if all_events 0x0A, 0x029E
goto after_burial if all_events 0x05, 0x0401

; --- 多事件全部清空 (对应 OP_IF_ALL_FLAGS_CLR_JMP 0x46) ---
goto first_visit if no_events 0x0001, 0x0020

; --- 多事件任意满足 (对应 OP_IF_ANY_FLAG_JUMP 0x47) ---
goto has_route if any_events 0x0003, 0x0004

; --- 单个事件 flag 判定 (对应 OP_IF_EVENT_FLAG_JUMP 0x1E) ---
goto spring_song_seen if event 0x0001

; --- switch 判定 (对应 OP_IF_SWITCH_JUMP 0x21) ---
goto chest_done if switch 0x0014

; --- party / item / money / save ---
goto nash_present if party 2 count 1
goto has_key     if item  0x2A
goto can_buy     if money 300 GE
goto after_load  if save_loaded
goto save_done   if save_flag 0x0001
```

编译器只接受上表列出的形式；`event 0x0000` 这种 ROM 中的历史特殊值
必须原样保留并在 canonical listing 里带警告，不能被展开为"无条件"。

## 7. `text` 块

`text` 块定义对白 token 序列，被 `dialog_message` / `dialog_text` /
`dialog_choice` 引用；token 有 3 种写法：

```lls
text opening_song_line:
    portrait 55                        # 已确认的控制 token (等价 v1 的 0x0D01)
    "これ ルーナの声じゃないか?"         # 明文, 通过 charmap 展开为 u16 token 序列
    [newline]                          # 控制 token, 方括号内为语义名
    "泉の方から聞こえるぜ"
    [newline]
    "行ってみよう アレス"
    [wait]
    [clear]
```

方括号控制 token 与 v1 的裸标识符等价，但视觉上是明显的"非文本"标记：

| 方括号标记 | 数值 | 说明 |
|---|---|---|
| `[newline]` | `0x0000` | 换行 / 分句 |
| `[wait]` | `0x0700` | 等待按键继续 |
| `[clear]` | `0x0F00` | 清空当前文本框 |
| `[portrait N]` | `0x0D0N` | 显示头像 N |

明文与别名共存，也可以直接写裸 token：

```lls
text legacy_line:
    0x0D37                             # 未知 / 保留原值
    0x1234
    0x0700
    0x0F00
```

编译器生成 canonical 时必须**逐 token**列出 (十六进制 + 解码结果)，未知
token 不得丢弃或替换为 Unicode 字符。

## 8. `raw` 段

`raw` 是等宽 byte 列表，不再用字符串包 hex；每行最多 8 个字节，与
canonical `.s` 的 `.byte` 行严格对齐：

```lls
; 单条 VM 指令 (含 opcode)
raw 0x04:
    0x4D 0x03 0x3E 0x5A 0x02

; 保留一整段子协议 (含 opcode 0x04)
raw 0x04:
    0x4D 0x00 0x02 0x02 0x02 0x14 0xFD

; 完全未知字节
raw 0x00:
    0x4D 0x64 0x00 0x02 0x02 0x02 0x14 0xFD
```

编译器对 `raw` 只检查：

1. 每条 byte 是合法 u8 字面量。
2. 该 raw 段所在位置不能越过 entry 边界。
3. 若声明了外层 opcode，第一个字节必须与之匹配。

**不允许**编译器猜测 raw 段长度或补齐对齐；raw 就是 raw。

## 9. `legacy` 与 `alias` 块

```lls
; 把尚未解析的整段正文纳入某个 entry
legacy 0x00 "raw/cutscene_00.bin":
    note: "ScriptSet 000 E00, ROM 0x862E2A8..0x862E400"
    rom_start: 0x0862E2A8
    rom_end:   0x0862E400

; 入口别名: 让入口表的两个编号指向同一偏移
alias_entry 0x05 = 0x07

; 别名块 (数值 → 名字, 反向映射)
alias:
    burg_home:   0x0B
    nash:        2
    luna:        4
    ares:        5
    hard_reset:  0
    safe_stop:   1
```

## 10. 注释与 offset 追踪

作者写的注释是自由文本；编译器在生成 canonical `.s` 时会追加**逐字节
offset 注释**，与反汇编 listing 严格对齐：

```lls
; 作者源:
entry 0x01 new_game:
    goto resume_0a if all_events 0x0A, 0x029E
    goto resume_09 if all_events 0x09, 0x0054

; canonical .s 输出 (自动追加 offset):
ScriptSet001_E01:
    EV_IF_ALL       0x0A, 0x029E    ; offset=0x0000  from 001_opening.lls:5
    EV_IF_ALL       0x09, 0x0054    ; offset=0x0005  from 001_opening.lls:6
```

这样作者不需要在源码里维护 offset；只需要看 canonical listing 就能对齐
原始 ROM。lint 会提示"未等待的地图切换"、"永不终止的 entry"、"不可达
entry"、"`stream` 没有可见的 `return_chunk`"、"对白缺少 `[wait]`/`[clear]`
结束控制"，但**不会**自动改写源码。

## 11. 完整示例：ScriptSet 001 Entry 01

对应 `GAME_SCRIPT_AUTHORING_DESIGN.md` §9 的 v1 片段。v2 版本：

```lls
mode: recover
baseline: "data/raw_data/unk_862E2A0.bin"

alias:
    burg_home: 0x0B

entry 0x01 new_game:
    # 存档继续路线必须排在初始开场路线之前;
    # 顺序不能改, 否则命中条件会漂移。
    goto resume_0a if all_events 0x0A, 0x029E
    goto resume_09 if all_events 0x09, 0x0054
    goto resume_08 if all_events 0x08, 0x0039
    goto resume_07 if all_events 0x07, 0x0402
    goto resume_06 if all_events 0x06, 0x0038
    goto resume_05 if all_events 0x05, 0x0401
    goto resume_03 if all_events 0x03, 0x0010
    goto resume_02 if all_events 0x02, 0x0000  # ROM 特殊判定, 保留数值

    bgm_stop()
    stream(0x00, 0x00)                       # set=0x00 entry=0x00
    load_map(0x01, 0x01, burg_home, 0x04, 0x04)  # npc_set, move_set, x, y, dir
    chara_control(0x4C, 0x03, 0x3E, 0x5A, 0x02)  # luna  -> (62, 90) down
    chara_control(0x4C, 0x04, 0x41, 0x59, 0x00)  # ares  -> (65, 89) up
    chara_control(0x4C, 0x05, 0x47, 0x52, 0x02)  # 3     -> (71, 82) right
    chara_control(0x4C, 0x06, 0x46, 0x5C, 0x00)  # 4     -> (70, 92) up
    chara_control(0x4C, 0x07, 0x40, 0x5E, 0x00)  # 5     -> (64, 94) up
    chara_control(0x4C, 0x09, 0x42, 0x4E, 0x06)  # 9     -> (66, 78) left
    chara_control(0x4C, 0x0C, 0x38, 0x55, 0x04)  # C     -> (56, 85) down
    chara_control(0x4C, 0x0D, 0x3B, 0x51, 0x04)  # D     -> (59, 81) down
    chara_control(0x4C, 0x0E, 0x3B, 0x5C, 0x00)  # E     -> (59, 92) up
    chara_control(0x4C, 0x11, 0x4A, 0x57, 0x02)  # 11    -> (74, 87) right
    scene_plain(0)
    wait_scene_idle()
    stop(1)

entry 0x0A resume_0a:
    dialog_message(0x00, opening_song_line)
    return_chunk()

text opening_song_line:
    portrait 55
    "これ ルーナの声じゃないか?"
    [newline]
    "泉の方から聞こえるぜ"
    [newline]
    "行ってみよう アレス"
    [wait]
    [clear]
```

对照 v1 的对应片段 (`GAME_SCRIPT_AUTHORING_DESIGN.md` §9) 与 v2，作者看
得到的差别只有：

- `jump_if all [event 0x029e] -> resume_0a` → `goto resume_0a if all_events 0x0A, 0x029E`
- `scene.load_map map=burg_home move_set=1 spawn=(11, 4) facing=left` →
  `load_map(0x01, 0x01, burg_home, 0x04, 0x04)`
- `wait scene_idle` → `wait_scene_idle()`
- `actor.position id=3 tile=(62, 90) facing=down` →
  `chara_control(0x4C, 0x03, 0x3E, 0x5A, 0x02)` (直接给出底层字节，作者需要
  查一次 `CharaControl` 子命令编码表；后续在 `names.toml` 里给 `luna: 0x03`
  等别名，仍可用别名替换)
- `text ... end` → `text ...:` + 缩进到底结束

## 12. 命令-字节对照表 (给作者查阅)

编译器在首次生成 canonical 时输出 `docs/scripts/cmd_table.md`，列
VM opcode 名 / 数值 / 参数签名 / v2 命令名，例如：

| v2 命令 | VM opcode | 数值 | 参数 |
|---|---|---|---|
| `bgm_stop()` | `OP_BGM_STOP` | 0x08 | — |
| `bgm_play(a,b)` | `OP_BGM_PLAY` | 0x07 | u8 music, u16 loop |
| `load_map(a,b,c,d,e)` | `OP_LOAD_MAP` | 0x1B | u8 npc_set, u8 move_set, u8 x, u8 y, u8 dir |
| `chara_control(a,b,...)` | `OP_CHARA_CONTROL` | 0x04 | 子命令字节流 (可变) |
| `goto L if all_events a,b,...` | `OP_IF_ALL_FLAGS_JUMP` | 0x45 | u16 flags…, u8 entry |
| `goto L if event a` | `OP_IF_EVENT_FLAG_JUMP` | 0x1E | u8 idx, u16 flag, u8 entry |
| `wait_frames(n)` | `OP_WAIT_FRAMES` | 0x26 | u8 n |

## 13. 编译器与验证

工具组织与 v1 完全一致 (`tools/llscript/`)：

```text
llscript compile data/scripts/001_opening.lls --out build/scripts/001
llscript disasm build/scripts/001.bin
llscript verify --against data/raw_data/unk_862E2A0.bin build/scripts/001.bin
llscript lint  data/scripts/001_opening.lls
```

编译器必须执行的检查 (与 v1 §8 一致，本节只列 v2 特有新检查)：

1. 每条命令的参数数量必须与 §5 表中的签名完全一致；多了少了都报错。
2. `goto` 的目标必须是同一脚本集的 entry label；`goto L if ...` 的目标
   必须是 entry 编号或 label。
3. `text` 块内不允许混入 VM 命令；命令块内不允许混入 token。
4. `raw` 段的字节数必须完整、不能越界；若声明了外层 opcode，第一字节
   必须匹配。
5. 别名只能出现在**参数位置**；不能出现在命令名或 block 名里。
6. `entry` 编号必须唯一；省略编号、自动按顺序编号、字节地址跳转都是错误。
7. `mode: recover` 下未修改的 entry 必须与 baseline 完全一致。
8. `mode: author` 下所有运行时入口必须显式声明或写成 `alias_entry`/`legacy`；
   省略是错误，不是自动生成空脚本。

## 14. 与 v1 的迁移对照

| 概念 | v1 | v2 |
|---|---|---|
| 块结束 | `text ... end` / `entry ...:` | 统一 `:` + 缩进到底 |
| 参数 | `key=value` | 位置参数 `f(a, b, c)` |
| 别名 | `map=burg_home` | `load_map(..., burg_home, ...)` + 行尾注释 |
| 分支 | `jump_if event X -> L` | `goto L if event X` |
| 分支条件 | `[event X, event Y]` | `all_events X, Y` |
| 等待 | `wait scene_idle` / `wait.frames 30` | `wait_scene_idle()` / `wait_frames(30)` |
| 脚本 | `script.stream set=000 entry=00` | `stream(0x00, 0x00)` |
| 场景 | `scene.load_map map=X move_set=Y ...` | `load_map(X, Y, x, y, dir)` |
| 角色 | `actor.position id=3 tile=(62,90) facing=down` | `chara_control(0x4C, 0x03, 0x3E, 0x5A, 0x02)` |
| 对白 | `dialog.message style=0 text=L` | `dialog_message(0x00, L)` |
| 对白 token | `portrait 55` / `newline` / `wait_input` / `clear` | `portrait 55` / `[newline]` / `[wait]` / `[clear]` |
| raw | `actor.control raw "4d 00 00 00 01 04 fd"` | `raw 0x04: 0x4D 0x00 0x00 0x00 0x01 0x04 0xFD` |
| legacy | `legacy entry 00 from="raw/x.bin"` | `legacy 0x00 "raw/x.bin": ...` |
| alias | `entry 05 resume = 07` | `alias_entry 0x05 = 0x07` |
| offset | 无 | 编译器自动写入 canonical `.s` |

## 15. 不做的事

与 v1 完全一致：

- 不引入新的 VM opcode。
- 不把多个 VM 指令合并成不可追踪的高层宏。
- 不用 JSON/YAML 作为作者唯一入口。
- 不在 `scene_plain` 后自动插 `wait_scene_idle`。
- 不把未知数据强行命名成错误的语义。
- 不声称"恢复了原厂脚本语言"；这只是本项目的 authoring 重建格式。

## 16. 迁移顺序

1. 先在 v1 parser 里加 v2 语法入口，两语法共存 (v1 保持可用)。
2. 把 `GAME_SCRIPT_AUTHORING_DESIGN.md` §9 的开场流程按 v2 重写，作为
   round-trip 回归测试。
3. 把 `script_vm.h` 的 80 个 opcode 全部映射为 §5 中的 v2 命令名。
4. 生成 `docs/scripts/cmd_table.md`，供作者查阅。
5. 把 `names.toml` 的地图、角色、场景别名迁移为 v2 的 `alias:` 块。
6. 迁移完成后再考虑把 v1 语法标为 deprecated。

在 §14 全部对照表落地并通过 §10 offset 追踪验证前，v1 仍然是权威语法；
v2 是提案，不覆盖 v1 的字节级保证。
