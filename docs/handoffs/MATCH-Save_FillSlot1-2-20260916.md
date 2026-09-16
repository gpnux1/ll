# Save_FillSlot1 / Save_FillSlot2 / Save_FillSlot3 工作包 (2026-09-16, match_save) — 已匹配

## 机器契约

### Save_FillSlot3 (0x08010E58)
- address: 0x08010E58, 91 汇编行 / 184B 机器码, module=save, src=src/save.c
- entry_kind: direct (SaveUi_DrawSlots, src/menu.c；亦在 TitleMenu_ProcessFrame 路径由 sub_8010F10 消费其结果)
- 原型: `void Save_FillSlot3(u8 slot)`
- match_status: 1 (**fncheck OK** 184B @0x08010e58, 0 bl 槽)
- callers: `SaveUi_DrawSlots` (src/menu.c:1819, 槽 0..2)；`TitleMenu_ProcessFrame` 路径 (src/menu.c:594)
- callees: `Save_SigCheck` (static inline, 0x08010E82..0x08010ECC 展开为内联检查)
- 全局输出:
  - `gUnk_03000204[slot]` (0x03000204, u8) = p[0x1A] (失败写 0xFF)
  - `gUnk_03000208[slot]` (0x03000208, u8) = p[0x14] (失败写 0)
  - `gUnk_03000210[slot]` (0x03000210, u32) = p[0xC] | p[0xD]<<8 | p[0xE]<<16 | p[0xF]<<24 (失败写 0)
- buffer: `0x02021000 + (slot * 0x2000)` (= GET_SAVE_SLOT)

## 机器契约

### Save_FillSlot1 (0x08010CCC)
- address: 0x08010CCC, 94 汇编行 / 180B 机器码, module=save, src=src/save.c
- entry_kind: direct (Save_Fsm 状态机 0xF9..0xFC 的逆操作；写槽)
- 原型: `void Save_FillSlot1(u8 slot)` (单参 r0=slot; 无返回)
- match_status: 1 (**fncheck OK** 180B @0x08010ccc, 0 bl 槽)
- callers: 无直接调用者 (与 Save_FillSlot0 同一状态机入口族；实际由加载路径经 Save_Fsm 选路)
- callees: 无
- data_tables:
  - `gSaveSignature` (0x08098199, 12B `"LUNAR1_12_09"`)
  - `word_80981B0` (0x080981B0, 数据块长度表, u16, 以 0 结尾)
  - `gSaveSlotSourceTable` (0x087EB180, u32* 数据块源指针表, 与长度表同序)
- buffer: `0x02021000 + (slot * 0x2000)` (= GET_SAVE_SLOT)

### Save_FillSlot2 (0x08010D80)
- address: 0x08010D80, 100 汇编行 / 216B 机器码, module=save, src=src/save.c
- entry_kind: direct (TitleMenu_ProcessFrame case TITLE_PHASE_START_LOADED_GAME, src/menu.c:542)
- 原型: `void Save_FillSlot2(u8 slot)`
- match_status: 1 (**fncheck OK** 216B @0x08010d80, 3 bl 槽忽略)
- callers: `TitleMenu_ProcessFrame` (src/menu.c:542)
- callees: `SwitchFlags_ClearRange` (0x0800110c), `Followers_SyncToTail` (0x080043d4), `Display_ShutdownSequence` (0x080008cc)
- data_tables: 同 Save_FillSlot1 (`gSaveSignature` 不由本函数使用)
- buffer: `0x02021000 + (slot * 0x2000)`

## 语义与逻辑 (E0/E1)

三者是同一”存档槽内存序列化/反序列化”家族，共享 `word_80981B0` 长度表 + `0x087EB180` 源指针表：

1. **Save_FillSlot0 (校验)** — 已匹配 (前包)：核对槽首 12B 签名与校验和。
2. **Save_FillSlot1 (写)** — 真实语义 `Save_SerializeSlot`：
   - `slot > 3` 直接 return；
   - `slot < 3` 时 `gActiveSaveSlot = slot` (槽 3 是中断存档，不改选中槽)；
   - 槽数据 0xC 前 12B 先全写 `0xFF`；
   - 按 `word_80981B0` 逐块，从 `gSaveSlotSourceTable[i]` 拷 blockSize 字节到 `slotData[0xC..]`，同时把每字节累加成 u8 校验和 (`checksum += slotData[offset]`)；
   - 块尾写 `slotData[offset] = checksum`；
   - 最后用 `for (offset = 0; offset < 12; offset++) slotData[offset] = gSaveSignature[offset]` 回填签名 (与 Save_LoadContinue 的”签名后写”顺序一致)。
3. **Save_FillSlot2 (读)** — 真实语义 `Save_DeserializeSlot`：
   - `slot > 3` return；`slot < 3` 时 `gActiveSaveSlot = slot`；
   - 反方向：按 `word_80981B0` 逐块，从 `slotData[0xC..]` 拷 blockSize 字节回 `gSaveSlotSourceTable[i]`；
   - 循环用 `do { src = tbl[i]; i++; while(len) 拷; i = (u16)i; len = tbl[i]; } while(len)` (与已匹配的 `Save_SyncShadow` 同构；`i` 为 u32，仅循环底 `(u16)i` 归一化，这是匹配关键)；
   - 依次 `SwitchFlags_ClearRange(); gAfterBattleCounter = 0; Followers_SyncToTail(); gGameState = 12; gVBlankPipelineMode = 1; Display_ShutdownSequence();`
   - 最后用 u16 指针循环写 0x400 个 u16 到 `0x02005800` (=0x800 字节) 清窗缓冲，再写 DMA3 寄存器组 `0x040000D4`：`[0]=0x02005800`、`[1]=0x0600F800`、`[2]=0x80000400`，末读 `[2]`。

## 匹配要点 (GCC2.9 形状)

- **Save_FillSlot1**：唯一难点是 12B 填充循环必须与后面两个循环**共用同一个 `offset` 变量** (声明为 u16)。若另立 `i` 变量，第一个循环会分到 r1 而非目标 r4，差 8 字节。
- **Save_FillSlot2**：
  - 块拷贝循环必须写成 `do { } while` + `i = (u16)i;` 归一化 + `i` 为 **u32** (照抄 `Save_SyncShadow`)；用普通 `while ((dataSize = tbl[i]) != 0)` 会多出每次迭代的 `lsls/lsrs` 归一化，差 ~百字节。
  - 清 0x02005800 的循环用独立 u16 计数器 (`offset` 复用)，生成 `strh r4, [r1]; adds r1, #2; ... cmp r3, r2; bls`。
  - DMA 寄存器必须写成 `{ vu32* dmaRegs = (vu32*)0x040000D4; dmaRegs[0]=...; dmaRegs[1]=...; dmaRegs[2]=...; dmaRegs[2]; }`，目标才产出 `ldr r1,=DMA基; ldr r0,=val; str; ldr r0,=val; str [r1,#4]; ...` 的形态 (与 battle_palette_wipe.c 的既有惯用一致)。
  - `gGameState` (=0x03001944) 是 `gMainGameState` 的同址别名 (linker.ld:539-540)，源码用 `gGameState` 即可编译通过。
- **Save_FillSlot3**：
  - 签名检查必须**内联展开** `Save_SigCheck` (static inline u8 返回 0/1)，不能另立循环变量。
  - 但 `p` 局部指针必须**声明在函数体首**且**不传入签名检查**；签名检查用 `GET_SAVE_SLOT(slot)` 直接传，`p` 只在成功分支赋 `GET_SAVE_SLOT(slot)` 供尾部读取。若把 `p` 传进检查 (`Save_SigCheck(p)`)，寄存器分配从 r4 挪到 r5，偏 20+ 字节。
  - 尾部三个字段读取各自重算基址会多出 `subs/ldr` 序列；复用同一个 `p` 才生成目标的 `mov r0, ip; adds r1, r4, r0; ldrb r0, [r2,#0x1a]` 形态。

## 符号与数据命名 (E0/E1)

- `0x03004D50` 已是 linker 符号 `gActiveSaveSlot` (linker.ld:730)，本包把源码从临时的 `gUnk_03004D50` 改为规范名。
- `0x087EB180` 新增 linker 别名 `gSaveSlotSourceTable` (u32 源指针表，与 `word_80981B0` 长度表配对；26 项，尾项 size=0 且指针=0x03004D48 作终止哨)。
- `0x03002C48` = `gAfterBattleCounter`；`0x03001944` = `gGameState`/`gMainGameState`；`0x0300259C` = `gVBlankPipelineMode` (均为既有 linker 符号)。
- `0x040000D4` = `REG_ADDR_DMA3` 寄存器基址 (VRAM 0x02005800 的搬运描述符)。

## 验证结论

- `scripts/bytecmp.sh Save_FillSlot1`: `OK (180 bytes)`
- `scripts/bytecmp.sh Save_FillSlot2`: `DIFF: 12 bytes / 264` → 仅 3 个 `bl` 槽位 (链接期重定位)，属预期
- `scripts/bytecmp.sh Save_FillSlot3`: `OK (184 bytes)`
- `python3 scripts/fncheck.py Save_FillSlot0 Save_FillSlot1 Save_FillSlot2 Save_FillSlot3`: 四者均 `OK`
- `make` + `sha1sum -c ll.sha1`: `ll.gba: 成功` (全量 SHA1 一致, 匹配进度 908/1059)

## 改动文件

- `src/save.c`: Save_FillSlot1/2/3 从 `INCLUDE_ASM` 换成可读 C；新增 extern 与 static inline `Save_SigCheck`。
- `include/save.h`: Save_FillSlot1/2 原型补 `(u8)` 参数。
- `include/engine_core.h`: 补 `Display_ShutdownSequence()` 声明。
- `include/iwram.h`: `gUnk_03000204/208/210` 改为数组声明 (按 slot 索引)。
- `linker.ld`: 新增 `gSaveSlotSourceTable = 0x087EB180`。
- `functions.tsv`: 三个地址 status=1 + note。
