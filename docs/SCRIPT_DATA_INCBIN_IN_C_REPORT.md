# 脚本数据 INCBIN 入 C 文件落地报告

> **工程**: 《Lunar Legend (Japan)》GBA ROM 逆向与源码全量反编译  
> **作者**: Antigravity  
> **日期**: 2026-09-07  
> **状态**: 100% 逐字节匹配 (747/1059, 70.5%), SHA1 验证通过  
> **约束说明**: 独立新建逆向分析文档，不修改历史旧文档；严格禁止 git commit。

---

## 1. 需求与工程方案

用户明确要求：**“脚本数据incbin到c文件里而不是汇编”**。

在 GBA C 语言反编译标准流水线中，通过 `tools/preproc` 支持在 C 语言文件中书写：
```c
const u8 gScriptSet_001[] = INCBIN_U8("data/raw_data/unk_862E2A0.bin");
```
这不仅将全部数据从底层汇编文件转移到高层 C 源文件体系中，而且通过具名符号实现了 C 符号级的可见性，便于未来直接在 C 语言中建立符号引用。

---

## 2. 实施细节

### 2.1 新建 C 数据文件 ([`src/data_script.c`](file:///home/gpnux/decomp/ll/src/data_script.c))
创建了包含 83 项脚本与操作码资源数据块的 C 语言文件，涵盖：
- 前期脚本块：`gScriptSet_140`、`gScriptSet_141`、`gScriptSet_142`
- 操作码分发表：`gScriptOpcodeHandlers` (`0x0862D434`)
- 图块配置集表：`gTileGfxSets` (`0x0862D574`)
- 共享空脚本集：`gScriptSet_Empty` (`0x0862D858`)
- **开场 Burg 村剧情脚本集**：`gScriptSet_001` (`0x0862E2A0`，由 `NewGame_Init: ScriptSet_Load(1, 0, 1)` 装载)
- 各迷宫、城镇、剧情事件脚本集：`gScriptSet_000` 至 `gScriptSet_133`

### 2.2 链接脚本段对齐 ([`linker.ld:685-688`](file:///home/gpnux/decomp/ll/linker.ld#L685-L688))
在链接脚本的 `.rodata` 段中，精准配置段锚点：
```ld
        src/data_805769C.o(.rodata);
        data/data.o(.rodata);
        . = ORIGIN(rom) + 0x61C784;
        src/data_script.o(.rodata);
        src/m4a_tables.o(.rodata);
```

### 2.3 产物尺寸核验
- `src/data_script.o` 编译后 `.rodata` 大小实测为 `600,720` 字节（`0x92A90`）。
- 与 ROM 中 `0x086AF214 - 0x0861C784 = 0x92A90` **分毫不差**。

---

## 3. 验证结果

- **工程审计 (`audit.py`)**: 1059 个函数，**747/747 status=1 字节核验全量通过，改名漂移为 0**。
- **全量构建终验 (`sha1sum`)**: **`ll.gba: 成功`**（ROM 逐字节 100% 完全一致）。
