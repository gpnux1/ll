# 游戏事件脚本 authoring 语言设计

## 1. 目标

`NEWGAME_TITLE_TO_OPENING_FLOW.md` 已经证明：开场流程不是 C 函数直接完成的，而是
由 Script VM 按帧解释一条压缩后的变长字节流。现有的反汇编 listing 适合取证，却不
适合人类编写剧情。问题不在于缺少更多 opcode 名字，而在于作者需要同时处理入口表、
数值 ID、对白 token、阻塞时序和未知子协议。

本设计定义一个真正可编辑的 authoring 层，目标是：

1. 剧情作者能看懂流程、修改对白、移动角色、添加分支和等待。
2. 编译结果仍然是当前 VM 能执行的字节流，不增加运行时解释器能力。
3. 每一条 authoring 语句都有确定的 VM 展开；不自动插入等待、跳转或副作用。
4. 未知 opcode、未知 token 和未解出的角色子命令可以原样保留。
5. 生成物能反向输出 canonical listing，并可逐字节验证原始解压数据。

这是一种本项目的重建/编辑格式，不声称恢复了原厂工具或原厂 DSL。

## 2. 三层表示

脚本不再要求同一个文件同时承担“给人读”和“证明字节”的职责：

```text
data/scripts/001_opening.lls       人类 authoring 源
             |
             v
      typed lowering + validator    无优化、保留源位置
             |
             +--> 001_opening.s     canonical 汇编风格 listing
             +--> 001_opening.bin   解压后的入口表 + VM 字节流
             +--> 001_opening.lz    可接回 ROM 的压缩对象
```

### 2.1 Authoring 层 (`.lls`)

`.lls` 使用缩进和短命令表达剧情顺序。命令名是语义名，参数仍然显式写出；资源名
只是数值 ID 的别名，不能隐藏一个以上的 opcode。

### 2.2 Typed lowering

编译器内部只保存以下几种记录：`Entry`、`Instruction`、`TextToken`、`RawBytes`。
不建立会重排控制流的优化 AST。每条记录带有源文件、行号、entry 编号和预计字节
长度，错误信息可以直接指回作者写的那一行。

lowering 只做四件事：解析参数、解析资源别名、把标签解析为 entry 编号、把命令展开
为固定字节。它不做常量折叠、不合并等待、不消除 NOP、不重排命令。

### 2.3 Canonical 层 (`.s`)

`.s` 仍是审核和逐字节比对的权威输入。入口表始终有 256 项；多个 entry 可以指向
同一个偏移。无法确认的内容用 `.byte` 或 `RAW` 表达，不能为了可读性猜参数宽度。

## 3. 文件格式

一个可独立编译的脚本集包含一个主文件和可选资源目录：

```text
data/scripts/
    001_opening.lls
    names.toml              # 可读别名到数值 ID 的映射
    text/                    # 对白 token 源
    raw/                     # 未识别 token 或命令的字节片段
```

脚本集编号由文件名或构建清单决定，不在正文中用会影响链接的声明重复指定。资源
别名文件只提供名字到数字的映射；它不定义新的游戏逻辑。

## 4. `.lls` 语法

### 4.1 顶层和 entry

```lls
script_set 001

entry 01 new_game:
    audio.stop_bgm
    script.stream set=000 entry=00
    scene.load_map map=burg_home move_set=1 spawn=(11, 4) facing=left
    scene.plain mode=0
    wait scene_idle
    script.stop mode=1

entry 13 spring_song:
    jump_if event 0x0001 -> spring_song_seen
    audio.volume 128
    audio.sfx id=219 arg=0
    wait.frames 550
    dialog.message style=0 text=opening_song_line
    audio.volume 255
    actor.control raw "4d 00 00 00 01 04 fd"
    wait actors_stopped
    flag.set event 0x0001
    script.stop mode=1

entry 1d spring_song_seen:
    dialog.message style=0 text=already_heard_line
    actor.control raw "4d 00 00 00 01 04 fd"
    wait actors_stopped
    script.stop mode=1
```

`entry` 的第一个参数是实际写入入口表的 `u8` 编号，第二个参数是可读标签。标签
只用于本文件内引用，不会写入运行时字节。entry 编号必须唯一；省略编号、自动按
出现顺序编号或用任意字节地址跳转都属于错误。

### 4.2 分支

分支使用 VM 已经存在的条件，不提供任意布尔表达式。这样作者看到的条件与运行时
行为一致：

```lls
jump_if event 0x0039 -> phase_6
jump_if switch 0x0014 -> chest_done
jump_if all [event 0x0401, event 0x0010] -> after墓地
jump_if all_clear [event 0x0001, switch 0x0020] -> first_visit
jump_if any [event 0x0003, event 0x0004] -> has_route
jump_if party member=2 count=1 -> nash_present
jump_if item id=0x2a -> has_key
jump_if money at_least=300 -> can_buy
```

编译器只接受 VM 已确认的条件形式。`all` 和 `all_clear` 的列表顺序保持不变；
`event 0x0000` 这种历史脚本中的特殊值必须原样编码，并在 listing 中保留警告，
不能被解释成 authoring 层的“无条件”。

分支目标必须是同一脚本集的 entry 标签。若需要共享正文，使用显式入口别名：

```lls
entry 05 resume = 07
```

别名会让入口表的 `05` 和 `07` 指向同一偏移，不会生成额外跳转；`07` 可以在文件
后面定义，但必须最终存在。

### 4.3 流程和阻塞

所有可能返回 `0`、等待下一帧的操作都使用 `wait` 或带 `wait_` 前缀的命令明确写出：

```lls
wait.frames 30
wait scene_idle
wait actors_stopped
wait sprite_load
wait camera_pan
wait chara_anim id=3
wait anim_slot id=0
wait menu_ready mode=1
```

`dialog.message`、`dialog.text` 和 `dialog.choice` 本身按照当前 VM 的行为阻塞；作者
不需要额外添加一个猜测性的等待，但必须明确写出对话之后的流程。编译器不会在
`scene.load_map` 后自动插入 `wait scene_idle`，只会给出可选的 lint 警告。

脚本结束、返回和嵌套脚本也必须区分：

```lls
script.stop mode=1       # 结束当前脚本并把控制权交给地图/玩家
script.return            # 0x03，返回普通脚本调用者
script.return_chunk      # 0x16，返回 ScriptStreamLZ 保存的宿主脚本集
script.call entry=common_dialog
script.jump entry=phase_2
script.stream set=000 entry=00
```

### 4.4 场景、角色和摄像机

已确认的参数使用结构化写法，保持底层宽度和顺序可见：

```lls
scene.load_map map=burg_home move_set=1 spawn=(11, 4) facing=left
scene.fade mode=0
scene.plain mode=0
camera.snap
camera.follow
camera.pan frames=32 target=(88, 32)

actor.position id=3 tile=(62, 90) facing=down
actor.load_gfx id=3 gfx=(3, 0)
actor.load_palette id=3 palette=(3, 0)
actor.restart_anim id=3 anim=1
actor.control raw "4d 00 00 00 01 04 fd"
```

`actor.position` 只对应已经确认的 `CharaControl` 子命令。其他角色动作仍使用
`actor.control raw`；raw 字节必须包含完整子命令，不允许编译器猜测长度。

### 4.5 音频、队伍、道具和标志

```lls
audio.play_bgm id=0x00fb arg=0
audio.stop_bgm
audio.fade_in speed=8
audio.fade_out speed=8
audio.sfx id=219 arg=0

party.add member=2
party.remove member=4
item.give id=0xdd count=2
item.take id=0xdd count=1
money.add amount=300
money.sub amount=50

flag.set event 0x0001
flag.clear event 0x0001
switch.set 0x0014
switch.clear 0x0014
```

数值别名可以来自 `names.toml`，例如 `map=burg_home` 或 `member=nash`；编译输出
必须同时显示别名和数值，避免作者无法核对 ROM 参数。

## 5. 对白和 token

对白不能直接当作 C 字符串。authoring 层提供可读 token 标记，但每个标记仍然只展开
为一个已知 token：

```lls
text opening_song_line:
    portrait 55
    "これ ルーナの声じゃないか?"
    newline
    "泉の方から聞こえるぜ"
    newline
    "行ってみよう アレス"
    wait_input
    clear
end
```

对于带样式、坐标或长度字段的两种 VM 对话指令，正文分别写成：

```lls
dialog.message style=0 text=opening_song_line
dialog.text x=6 y=7 text=map_sign
dialog.choice yes=choice_yes no=choice_no text=ask_leave
```

文本编译器从 `charmap.txt` 和已确认的汉字表生成 16 位 token。`portrait`、`newline`、
`wait_input`、`clear` 等控制标记有固定数值；未知值直接写：

```lls
text legacy_line:
    token 0x0d37
    token 0x1234
    token 0x0700
    token 0x0f00
end
```

未知 token 不得静默丢弃或替换成 Unicode 字符。生成的 `.s` 必须列出每个 token 的
十六进制值和解码结果。

## 6. Raw 和 legacy 区域

Raw 是正式语法，不是编译失败后的临时补丁：

```lls
raw bytes "04 4d 00 02 02 02 14 fd"
raw command "4d c9 00"
legacy entry 00 from="raw/cutscene_00.bin"
```

规则：

1. `raw bytes` 按写入顺序原样复制，不加对齐、不插等待、不做资源重定位。
2. `raw command` 的字符串包含 opcode 本身，表示一条完整 VM 指令；编译器只检查
   边界，不会从内容中猜测或补齐长度。
3. `legacy` 可以把尚未解析的整段正文纳入某个 entry，旁边必须有来源偏移和长度。
4. 一旦某段 raw 的格式被确认，才可以把它提升为一个一对一的命令；提升必须保留
   展开后的 byte dump 回归测试。

## 7. 基线模式和新脚本模式

同一语言支持两种用途，避免“编辑原 ROM”和“写新事件”互相污染。

### 7.1 Recovery 模式

用于从现有脚本集迁移和逐字节重建：

```lls
script_set 001 mode=recover
baseline "data/raw_data/unk_862E2A0.bin"
preserve_unedited_entries
```

编译器先读取 baseline 的 256 项入口和正文；`.lls` 中出现的 entry 覆盖对应正文，
未出现的 entry 保留原始字节。这样新作者不必立即理解 1,410 条指令，但每个修改的
entry 都会重新生成并参与验证。若一个 entry 长度变化导致后续偏移变化，编译器必须
重新生成完整入口表，不能尝试原地覆盖。

### 7.2 Author 模式

用于完整新脚本集或已经完成 canonical 化的脚本：

```lls
script_set 001 mode=author
```

Author 模式要求所有运行时入口都显式声明，或显式写出 `alias`/`legacy`。省略的
entry 是错误，而不是自动生成空脚本；这能避免把一个漏写的入口误变成可执行的
`STOP`。

## 8. 编译器验证和输出

建议工具位于 `tools/llscript/`：

```text
llscript compile data/scripts/001_opening.lls --out build/scripts/001
llscript disasm build/scripts/001.bin
llscript verify --against data/raw_data/unk_862E2A0.bin build/scripts/001.bin
llscript lint data/scripts/001_opening.lls
```

编译器使用一份单独冻结的 opcode registry，至少记录 opcode 数值、参数宽度和端序、
是否可能阻塞、以及 canonical 展开格式。registry 由 VM handler、反汇编器和已验证的
脚本样本交叉确认后维护；不能只从某个头文件或旧报告自动推导。authoring parser、
canonical 宏和反汇编器都从这份 registry 生成或校验，发现同一 opcode 的元数据冲突时
必须停止编译，而不是静默选择一个名字。

编译必须执行以下检查：

1. entry 编号在 `0..255`，标签唯一，所有分支目标存在。
2. 每个 opcode 的参数宽度、端序、范围和总长度正确。
3. 对话 token 可以由 `charmap.txt` 或显式 `token` 完整编码。
4. `raw` 段不会越过 entry 边界；entry 正文不会引用不存在的相对偏移。
5. 输出 canonical listing，包含源行号、解压偏移、opcode、参数和展开字节。
6. recovery 模式下，未修改 entry 的解压字节保持完全一致。

Lint 只报告风险，不改写源文件：未等待的地图切换、永不终止的 entry、不可达 entry、
`script.stream` 没有可见的 `script.return_chunk`、对白缺少结束控制 token，以及
使用未命名数字资源。Lint 不能把这些情况自动“修好”，因为原始脚本确实可能依赖
跨 entry 或跨场景的状态。

压缩验证分开处理：

1. 先比较解压后的入口表和 VM 字节流。
2. 再比较 LZ token 的贪心策略和 tie-break，确认压缩区逐字节一致。
3. 最后接回 `src/data_script.c`，运行 `make`、目标 `fncheck.py`、`make verify` 和
   `sha1sum -c ll.sha1`。

## 9. 开场流程的完整 authoring 片段

下面这段对应 `NEWGAME_TITLE_TO_OPENING_FLOW.md` 的宿主 Entry 01。它把流程意图写在
命令和注释里，但没有隐藏任何会影响 VM 时序的指令：

```lls
script_set 001 mode=recover
baseline "data/raw_data/unk_862E2A0.bin"

entry 01 new_game:
    # 存档继续路线必须先于初始开场路线判断。
    jump_if all [event 0x029e] -> resume_0a
    jump_if all [event 0x0054] -> resume_09
    jump_if all [event 0x0039] -> resume_08
    jump_if all [event 0x0402] -> resume_07
    jump_if all [event 0x0038] -> resume_06
    jump_if all [event 0x0401] -> resume_05
    jump_if all [event 0x0010] -> resume_03
    jump_if all [event 0x0000] -> resume_02  # 原格式特殊判定，保留数值

    audio.stop_bgm
    script.stream set=000 entry=00
    scene.load_map map=burg_home move_set=1 spawn=(11, 4) facing=left
    actor.position id=3 tile=(62, 90) facing=down
    actor.position id=4 tile=(65, 89) facing=up
    actor.position id=5 tile=(71, 82) facing=right
    actor.position id=6 tile=(70, 92) facing=up
    actor.position id=7 tile=(64, 94) facing=up
    actor.position id=9 tile=(66, 78) facing=left
    actor.position id=12 tile=(56, 85) facing=down
    actor.position id=13 tile=(59, 81) facing=down
    actor.position id=14 tile=(59, 92) facing=up
    actor.position id=17 tile=(74, 87) facing=right
    scene.plain mode=0
    wait scene_idle
    script.stop mode=1
```

`script.stream` 是嵌套 ScriptSet 000 的一次 opcode，不是一个会自动等待并返回的
高层函数。`Scene.load_map` 和 `scene.plain` 的执行顺序保持不变；作者若要改变顺序，
会在源码中明显看到这个时序变化。

## 10. 迁移顺序

1. 先实现 parser、`entry`、`raw`、`script.stop`、`script.jump/call/return` 和 byte
   dump；用 ScriptSet 001 做 round-trip。
2. 实现 flags、scene、wait、audio、party/item 等已经确认参数宽度的命令。
3. 实现独立 text token 文件和 `dialog.message/text/choice`，未知 token 继续走
   `token 0xNNNN`。
4. 对已确认的 `CharaControl SetPosDir` 提供 `actor.position`；其他子协议保留 raw。
5. 写 lint 和 source map，再接入 LZ pack/verify。
6. 最后才把更多脚本集从 `.bin` 迁移为 `.lls`；每迁移一个 set 都保存解压级和 ROM 级
   验证结果。

当前不做的事情：不在运行时加入新的 opcode，不把多个 VM 指令合成不可追踪的宏，不
用 JSON/YAML 作为作者唯一入口，不把未知数据强行命名成错误的语义。高层编辑器或
图形化工具可以以后调用 `.lls` 编译器，但不能绕过 canonical listing 和逐字节验证。

## 11. 结论

可读性来自稳定的命令分组、资源别名、对白 token 标记、entry 标签和 lint；可信度来
自显式参数、显式等待、256 项入口表、raw 逃生口和 canonical byte dump。`.lls` 解决
“人类怎么写”，`.s` 解决“我们是否仍在写同一套 VM 数据”，两者不互相冒充。
