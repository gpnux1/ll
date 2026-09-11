# LzHeader 结构命名、gScriptVmFlags 状态位图解码与脚本集数据 C 数组落盘报告

> **工程**: 《Lunar Legend (Japan)》GBA ROM 逆向与源码全量反编译  
> **作者**: Antigravity  
> **日期**: 2026-09-07  
> **状态**: 100% 逐字节匹配 (747/1059, 70.5%), SHA1 验证通过  
> **约束说明**: 独立新建逆向分析文档，不修改或追加历史旧文档；严格不执行 git commit。

---

## 1. 目标 1: Unk_LzData 规范命名为 LzHeader

### 1.1 物理结构剖析与字段解码
在 GBA 的流式 LZ77 解压缩子系统（`LZ_InitContext` 与 `LZ_UncompressChunk`）中，原 `Unk_LzData` 位于 `include/iwram.h:28`：

```c
struct LzHeader
{
    u32 uncompressedSize; /* 解压后的目标总字节数 (例如 0x73F1B) */
    u32 size;             /* 压缩字面数据负载 (Literal Data) 大小; ptr + size 即为 Flags 标志流起始 */
    u8 data[1];           /* 变长压缩数据负载 */
};
```

### 1.2 规范化与向后兼容
- 重命名为 **`LzHeader`**（标准 LZ77 资源流块头）。
- 在 [`include/iwram.h`](file:///home/gpnux/decomp/ll/include/iwram.h#L28-L34) 中同时导出：
  ```c
  typedef struct LzHeader LzHeader;
  typedef struct LzHeader Unk_LzData; // 保持向后兼容
  ```
- 同步更新 [`src/script_vm.c`](file:///home/gpnux/decomp/ll/src/script_vm.c#L889) 中的使用点。

---

## 2. 目标 2: gUnk_03000E70 含义推测与位图全解析

### 2.1 物理本质定性
位于 IWRAM `0x03000E70` 的 16 位全局变量是 **脚本虚拟机运行状态与异步系统服务请求位图标志（Script VM Status & Async Service Flags）**。

### 2.2 全部活动位语义解码表

| Bit 位 | 掩码十六进制 | 规范宏命名 | 物理语义与协作子系统 |
|---|---|---|---|
| **bit 0** | `0x0001` | `SCRIPT_VM_FLAG_RUNNING` | **脚本正在执行中**。由 `ScriptPump_JumpToEntry` 置位，脚本退场或调用 `Script_Abort` 时清除。外部通过 `Script_GetFlags() & 1` 判断主脚本是否忙碌。 |
| **bit 4** | `0x0010` | `SCRIPT_VM_FLAG_WINDOW_BG_REQ` | **窗口/对话框 BG0 拷贝请求**。由 `Op_OpenWindow` 操作码置位，主循环 VBlank 泵 `ScriptPump_ServiceFrame` 消费并清零 BG0 滚动后清位。 |
| **bit 5** | `0x0020` | `SCRIPT_VM_FLAG_DIALOG_STATE` | **对话框等待/翻页标志**。对话框文本排版推进与等待玩家按键交互时交替翻转。 |
| **bit 6** | `0x0040` | `SCRIPT_VM_FLAG_TILE_DMA_REQ` | **图块 DMA 传输待刷新请求**。由 `Op_LoadTileGfx` 置位，在 VBlank 期间执行 `FlushTileDma()` 成功后清位。 |
| **bit 8** | `0x0100` | `SCRIPT_VM_FLAG_RELOAD_BG_SET` | **关窗后地图图块重载请求**。由 `Op_CloseWindow` 操作码置位，消费后调用 `BgTiles_LoadSet(0)` 恢复地图底图。 |
| **bit 9** | `0x0200` | `SCRIPT_VM_FLAG_LZ_STREAMING` | **异步流式 LZ 解压进行中**。由 `ScriptSet_Load` 在非强制黑屏时置位，此时**主脚本泵暂停 opcode 执行**，每帧由 VBlank 泵调用 `LZ_UncompressChunk()` 分块推进。 |
| **bit 10** | `0x0400` | `SCRIPT_VM_FLAG_ENTRY_JUMP_REQ` | **异步 LZ 解压完成后 PC 跳转入口标志**。当流式解压完成且该位置位时，脚本泵自动将 PC 指针跳到 `0x02016200 + entryTbl[gScriptPendingEntry]`，并清位。 |

### 2.3 规范化落地
在 [`include/script_vm.h`](file:///home/gpnux/decomp/ll/include/script_vm.h#L159-L170) 中定义全部 `SCRIPT_VM_FLAG_*` 宏常量，并提供别名：
```c
#define gScriptVmFlags gUnk_03000E70
```
在 [`linker.ld:57`](file:///home/gpnux/decomp/ll/linker.ld#L57) 中添加符号别名：
```ld
gScriptVmFlags = 0x03000E70;
```

---

## 3. 目标 3: gUnk_087ED6D4 数据从 Blob 剥离并落入 C 文件

### 3.1 数据特征与布局
- **地址与长度**:
  - 起始地址: `0x087ED6D4` (ROM 偏移 `0x7ED6D4`)
  - 结束地址: `0x087EDC80` (ROM 偏移 `0x7EDC80`)
  - 大小: `363` 项 32 位绝对指针（共 1,452 字节，`0x5AC`）
- **数据内容**:
  全部 363 项均为指向 ROM 剧情/地图事件脚本 LZ 压缩包的绝对指针（`0x0862D8A4` ~ `0x087E82B8`）。

### 3.2 剥离工程实施步骤
1. **新建 C 文件**: [`src/data_087ED6D4.c`](file:///home/gpnux/decomp/ll/src/data_087ED6D4.c)
   定义 `const u32 gUnk_087ED6D4[363] = { ... };`。
2. **重构底包汇编分段**:
   - 原 `data/data1b.s` 负责 `0x7EA580` ~ `0x800000` 的 blob。
   - 截断 `data/data1b.s` 为：
     ```assembly
     gUnk_087EA580: @ 087EA580
         .incbin "baserom.gba", 0x7EA580, 0x7ED6D4 - 0x7EA580
     ```
   - 新建 `data/data1c.s` 承接后续部分：
     ```assembly
     gMPlayInfos2: @ 087EDC80
         .incbin "baserom.gba", 0x7EDC80, 0x800000 - 0x7EDC80
     ```
3. **链接脚本锚定**:
   在 [`linker.ld:687-691`](file:///home/gpnux/decomp/ll/linker.ld#L687-L691) 中：
   ```ld
           src/data_087EA1A0.o(.rodata);
           data/data1b.o(.rodata);
           . = ORIGIN(rom) + 0x7ED6D4;
           src/data_087ED6D4.o(.rodata);
           data/data1c.o(.rodata);
   ```

---

## 4. 验证结果汇总

- **函数单体字节级核验 (`fncheck.py`)**:
  `ScriptSet_Load OK (184 bytes @0x080525e8, 7 池重定位已施加, 3 bl 槽忽略)`
- **工程全量函数体检 (`audit.py`)**:
  1059 函数，747 已匹配（70.5%），312 未匹配，改名漂移 0，**通过 747/747 全绿**。
- **全量终验 (`sha1sum`)**:
  `ll.gba: 成功`（逐字节一致，SHA1 哈希无变动）。
