# Lunar Legend 事件脚本重建方案：汇编数据源，不另造现代 DSL

> 面向剧情作者的可读 `.lls` authoring 语法见
> [`GAME_SCRIPT_AUTHORING_DESIGN.md`](GAME_SCRIPT_AUTHORING_DESIGN.md)。本文继续作为
> canonical 字节层和 opcode 薄宏的约束；`.lls` 不替代它，而是必须能无歧义地 lowering
> 到它。

## 1. 方向修正

上一版虽然去掉了花括号和对象，但本质上仍是一个现代“脚本语言”：有 `.SET`、
`.ENTRY`、具名分支、命令抽象和作者友好的字符串语法。它适合写一份新的游戏脚本，
却不像 2000 年前后 GBA 项目里真正会交给构建系统的东西。

本项目现在不应声称“恢复了原厂脚本语言”。从 ROM 能确认的是：游戏内只有一个紧凑的
事件 VM 字节流，前面是 256 项 `u16` 入口表；看不出原开发团队使用的源文件究竟是
事件编辑器、表格导出器、C 宏、汇编宏，还是几者的组合。

因此采用两层、但不引入现代 AST 的重建方案：

1. **canonical 层**：用 ARM/GNU 汇编风格的 `.s` 文件描述入口表和字节流。每一条
   宏调用固定展开成已知 opcode；未知内容直接写 `.byte`。目标是逐字节重建。
2. **authoring 层**：以后可以有一个很薄的文本/表格转换器，把对白、角色动作和
   场景资料生成 canonical `.s`。它不是运行时语言，也不是本项目首先要恢复的部分。

作者看到的不是 `scene.load_map()`，而是接近下面这种“事件数据源”：

```asm
        .include "script_vm_macros.inc"

ScriptSet001_EntryTable:
        .hword  ScriptSet001_E00 - ScriptSet001_Body
        .hword  ScriptSet001_E01 - ScriptSet001_Body
        .hword  ScriptSet001_E02 - ScriptSet001_Body
        /* ... 共 256 项，保持 ROM 中的原顺序 ... */

ScriptSet001_Body:
ScriptSet001_E00:
        EV_STOP         1

ScriptSet001_E01:
        EV_IF_ALL       0x0A, 0x029E
        EV_IF_ALL       0x09, 0x0054
        EV_BGM_STOP
        EV_STREAM       0x00, 0x00
        EV_LOAD_MAP     0x01, 0x01, 0x0B, 0x04, 0x04
        EV_CHARA_RAW    0x4C, 0x03, 0x3E, 0x5A, 0x02
        EV_SCENE_PLAIN  0x00
        EV_WAIT_SCENE
        EV_STOP         1
```

`EV_*` 不是新的运行时语法，而是 `.byte`、`.hword` 和文本 token 的薄宏。展开结果
必须能和 `docs/scripts/script_set_001.txt` 的反汇编逐条对照。

## 2. 为什么汇编数据源更合理

当前已确认的格式有几个强约束：

- 脚本集解压后先放 256 个入口偏移，入口编号是一个 `u8`。
- 指令是可变长度字节串，参数宽度有 `u8`、little-endian `u16` 和 token 数组。
- 条件分支跳到入口编号，不是任意字节地址。
- `STOP`、`WAIT`、对话和 LZ 流式装载会改变逐帧执行状态。
- 一部分 opcode 的参数尚未完全解释，角色控制还包含自己的子协议。

这些特征更像“数据记录 + 小型解释器”，而不是需要表达嵌套作用域的语言。现代
`if/else`、对象、资源名和自动等待会隐藏原始字节边界。

入口编号也不能被普通源码标签替代。`IF_*` 写入的是 `Entry_XX` 的编号；多个入口
可以故意共享同一偏移。因此 canonical 文件应保留真实的 256 项表，而不是只输出
“有内容的入口”：

```asm
        .hword  ScriptSet000_E00 - ScriptSet000_Body
        .hword  ScriptSet000_E01 - ScriptSet000_Body
        .hword  ScriptSet000_E02 - ScriptSet000_Body
        /* E03 .. EFF 仍然必须存在 */

ScriptSet000_Body:
ScriptSet000_E00:
        EV_STOP         1
```

源码标签只是 listing 的符号，不参与游戏逻辑；分支仍明确写入口号：

```asm
        EV_IF_EVENT     0x11, 0x0039
        EV_JUMP         0x1D
```

## 3. 文件和工具组织

脚本集编号由文件名和构建清单决定，不在每个文件里增加高层 `.SET` 声明：

```text
data/scripts/
    script_vm_macros.inc       ; 固定 opcode 薄宏
    script_000_entries.inc     ; 256 项入口表
    script_000_body.inc        ; Entry_00 .. Entry_FF 正文
    script_001_entries.inc
    script_001_body.inc
    text/                       ; 文本 token 源或生成的 .inc
tools/llscript/
    pack.py                     ; 入口表 + 正文 + LZ header
    disasm.py                   ; 原始脚本区 -> 汇编风格 listing
    verify.py                   ; 解压后逐字节比较
    text_tokens.py              ; 日文与特殊 token 转换
```

外层 LZ header、入口表相对基址和链接地址由专用打包步骤处理，不能把普通 ARM 汇编
器的默认重定位结果直接当作 `gScriptSetTable` 的 ROM 对象。`pack.py` 可以读取汇编
预处理后的 `.byte`/`.hword`，但输入仍保持可审查的汇编宏风格，而不是不透明的
JSON/YAML 中间格式。

## 4. 宏的风格：只做固定展开

宏库应像旧式汇编器的 opcode include，而不是类库。每个宏只负责写 opcode、写固定
宽度参数并检查参数数量；不负责等待、资源查找、路径规划或控制流重写。

```asm
        .macro EV_STOP mode
        .byte   0x06, \mode
        .endm

        .macro EV_BGM_STOP
        .byte   0x08
        .endm

        .macro EV_WAIT frames
        .byte   0x26, \frames
        .endm

        .macro EV_STREAM set_id, entry
        .byte   0x15, \set_id, \entry
        .endm

        .macro EV_RETURN_CHUNK
        .byte   0x16
        .endm

        .macro EV_LOAD_MAP npc_set, move_set, x, y, direction
        .byte   0x1B, \npc_set, \move_set, \x, \y, \direction
        .endm

        .macro EV_SCENE_PLAIN mode
        .byte   0x18, \mode
        .endm

        .macro EV_WAIT_SCENE
        .byte   0x19
        .endm
```

上面是宏形状示例；每个宏的实际参数顺序必须由 `scripts/disasm_script.py`、
`include/script_vm.h`、opcode handler 和原始数据共同确认。未确认的命令不能因为
名字好看就加入宏库。

推荐命名：

- `EV_`：已确认的 VM opcode 薄宏。
- `TXT_`：文本 token 生成器，不模拟对话逻辑。
- `RAW_`：保留原始字节的工具宏，也可直接使用 `.byte`。
- `DBG_`：只在 listing/验证阶段使用，不写入 ROM。

不要提供 `PLAY_CUTSCENE`、`TALK_TO`、`SPAWN_CHARACTER` 这种跨越多个 opcode 的
高层宏，除非已经拿到原始展开字节并为其写了回归测试。它们会隐藏关键的等待、场景
切换和角色子命令。

## 5. 文字和对白：独立的 token 源

对白不是普通 ASCII 字符串。`OP_DIALOG_MESSAGE (0x00)` 和 `OP_DIALOG_TEXT (0x17)`
都包含游戏自定义的 16 位 token，还混有换行、等待、清屏、头像和未识别控制码。

canonical 层应允许命名一段 token 数据，而不是依赖编译器 locale：

```asm
        EV_DIALOG_MESSAGE 0x00, msg_cutscene_00
        EV_DIALOG_TEXT    0x06, 0x07, 0x00, msg_opening_00

msg_cutscene_00:
        .hword  0x0D01          /* portrait 01 */
        .hword  0x0000          /* charmap token */
        .hword  0x0700          /* WAIT */
        .hword  0x0F00          /* CLEAR */
```

也可以让 `text_tokens.py` 从文本文件生成 `.inc`：

```text
; text/0001.txt
; portrait=01
ドラゴンマスター か、、、[WAIT][CLEAR]
```

但生成器输出必须能回到 `.hword` token listing。未知 token 保留为数字，不能因为
无法识别就丢弃。

## 6. Opening / cutscene_00 的 canonical 外观

ScriptSet 001 的新游戏宿主入口应接近真实字节顺序：

```asm
ScriptSet001_E01:
        EV_IF_ALL       0x0A, 0x029E
        EV_IF_ALL       0x09, 0x0054
        EV_IF_ALL       0x08, 0x0039
        EV_IF_ALL       0x07, 0x0402
        EV_IF_ALL       0x06, 0x0038
        EV_IF_ALL       0x05, 0x0401
        EV_IF_ALL       0x03, 0x0010
        EV_IF_ALL       0x02, 0x0000
        EV_BGM_STOP
        EV_STREAM       0x00, 0x00
        EV_LOAD_MAP     0x01, 0x01, 0x0B, 0x04, 0x04
        EV_CHARA_RAW    0x4C, 0x03, 0x3E, 0x5A, 0x02
        EV_CHARA_RAW    0x4C, 0x04, 0x41, 0x59, 0x00
        EV_SCENE_PLAIN  0x00
        EV_WAIT_SCENE
        EV_STOP         1
```

这里没有 `@ENTRY_0A`、没有 `LOAD_MAP { ... }`、没有自动插入的等待，也没有把角色
位置提升为对象构造。每一行都对应一个可定位的原始 opcode。

ScriptSet 000 的开场前段也保留底层顺序：

```asm
ScriptSet000_E00:
        EV_SYS_RAW      0x4D, 0xC9, 0x00
        EV_LOAD_MAP     0x82, 0x00, 0x00, 0x00, 0x00
        EV_OPEN_WINDOW
        EV_DIALOG_SETUP 0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00
        EV_SCENE_PLAIN  0x00
        EV_WAIT_SCENE
        EV_BGM_PLAY     0xFB, 0x0000
        EV_DIALOG_TEXT  0x06, 0x07, 0x00, msg_opening_00
        EV_LOAD_MAP     0x0B, 0x00, 0x0F, 0x2E, 0x06
        EV_CUTANIM_RAW  0x00, 0x00, 0x00, 0x00
        EV_WAIT_CHARS
        EV_DIALOG_MESSAGE 0x00, msg_cutscene_00
        EV_RETURN_CHUNK
```

`EV_SYS_RAW`、`EV_CUTANIM_RAW` 和角色命令名字只是保守占位，不是已经完成的语义
命名。未确认时优先保留完整字节：

```asm
        .byte   0x04, 0x4D, 0x00, 0x02, 0x02, 0x02, 0x14, 0xFD
```

## 7. `RAW` 的规则

`RAW` 不是失败退路，而是重建工作的一等构造：

```asm
        .byte   0x4D, 0x64, 0x00           /* 完整 VM 指令 */
        EV_CHARA_RAW  0x4D, 0x00, 0x02, 0x02, 0x02, 0x14, 0xFD
        .incbin       "text/generated/0001.tokens"
```

规则如下：

1. 未知序列原样保留，不猜参数宽度。
2. `EV_CHARA_RAW` 若约定不含外层 `0x04`，必须在名称和文档中明确；完整指令直接
   `.byte 0x04, ...`。
3. `RAW` 不插入等待、不做对齐、不进行资源重定位。
4. 每个被提升的宏都要保留展开后的 byte dump 测试。

## 8. 工具链：先做 pack/disasm/verify

第一阶段不需要 lexer、parser、AST 或解释器。优先实现：

```text
tools/llscript/
    pack.py
    disasm.py
    verify.py
    text_tokens.py
```

验证分三级：

1. **指令级**：宏展开与反汇编 listing 的 opcode、参数和长度一致。
2. **解压级**：生成的解压区与 `data/raw_data/unk_*.bin` 逐字节一致。
3. **ROM 级**：接回数据对象后再跑 `make`、相关 `fncheck` 和 `sha1sum -c ll.sha1`。

LZ 压缩 token 即使解压内容相同也可能不同，所以先把“解压数据完全一致”作为
canonical 门槛；确认需要压缩区逐字节一致后，再复原原压缩器的贪心策略和 tie-break。

## 9. 关于原厂语法的结论

目前不能从 `script_vm.c` 或脚本字节流证明开发团队使用了哪一种源语法。能够确认
的只有运行时格式和若干 opcode。合理的历史假设是：事件编辑器/表格负责内容，导出器
或宏汇编器负责生成入口表 + 字节流；但这是假设，不是 ROM 取证结论。

所以本项目的文件应称为 **Lunar Legend event data reconstruction source**，而
不是 “Lunar Script language”。只有找到开发残留、工具格式或同一团队其他版本后，
才可以讨论是否恢复原厂脚本语言。

## 10. 实施顺序

1. 先把 `ScriptSet 001 / Entry 01` 做成 `.s` 风格 canonical listing。
2. 实现 `EV_STOP`、`EV_RAW`、`EV_STREAM`、`EV_RETURN_CHUNK` 和入口表验证。
3. 加入场景、地图、BGM、等待和旗标 opcode，并逐条做 byte dump 测试。
4. 用独立 token 文件迁移 Opening 的 `OP_DIALOG_TEXT (0x17)`。
5. 用同一 token 机制迁移 `cutscene_00` 的 `OP_DIALOG_MESSAGE (0x00)`。
6. 角色控制和过场动画在语义闭环前全部保留 raw；不要为了“好看”提前抽象。
7. 最后才考虑 authoring 层；它必须能反向导出完全相同的 canonical 源和解压字节。
