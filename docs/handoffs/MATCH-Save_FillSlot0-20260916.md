# Save_FillSlot0 工作包 (2026-09-16, match_save) — 已匹配

## 机器契约
- address: 0x08010BEC, 122 汇编行 / 224B 机器码, module=save, src=src/save.c
- entry_kind: direct (被 Save_Fsm 状态机的 0xF9, 0xFA, 0xFB, 0xFC 调用)
- 原型: `u8 Save_FillSlot0(u8 slot)` (单参 r0=slot; 返回值 r0: 0=校验成功, 1=签名错误或校验和不匹配且写0xFF, 0xFF=槽位超限>3)
- match_status: 1 (**fncheck OK** 224B @0x08010bec, 0 bl 槽忽略, sha1 全绿)
- callers: `Save_Fsm` (src/save.c:81, 86, 91, 96)
- callees: 无
- data_tables:
  - `gSaveSignature` (0x08098199, 12B `"LUNAR1_12_09"`)
  - `word_80981B0` (0x080981B0, 普通存档槽 0..2 数据块长度表，以 0 结尾)
  - `gUnk_080981E6` (0x080981E6, 中断/继续存档槽 3 数据块长度表 `{2, 0x5A, 8, 0}`)
- buffer: `0x02021000 + (slot * 0x2000)`

## 语义与逻辑 (E0/E1)
该函数是存档槽的数据校验函数（真实语义为 `Save_VerifySlotChecksum` / `Save_CheckSlot`）：
1. 检查 `slot > 3`，若超限返回 `0xFF`。
2. 校验槽首部 12 字节签名是否与 `"LUNAR1_12_09"`（`gSaveSignature`）完全一致。若不一致直接返回 1。
   - 注意：虽然 `src/menu.c` 中存在功能类似的 inline 函数 `Save_SigCheck`，但在 `Save_FillSlot0` 中内联展开会导致寄存器局部性错位（GCC2.9 会因内联函数返回类型提升打乱 r4/r5/r6/r7 的复用）；采用与 `sub_8015ED0` 相同的自然 `for` 循环与 `sigPtr = slotData; sigPtr++`，完美契合 GCC2.9 的寄存器生命周期分配。
3. 校验和计算：
   - 槽 0..2 遍历 `word_80981B0` 块长度表；
   - 槽 3 遍历 `gUnk_080981E6` 块长度表；
   - 从偏移 12 开始，按块长度逐字节累加 u8 校验和。
4. 比对计算得到的 `checksum` 与紧随各块末尾存储的校验和字节 `slotData[offset]`：
   - 若匹配：返回 0（存档有效）。
   - 若不匹配：将该槽起始字节写为 `0xFF`（破坏签名标记损坏），并返回 1。

## 验证结论
- `scripts/bytecmp.sh Save_FillSlot0`: `OK (224 bytes)`
- `python3 scripts/fncheck.py Save_FillSlot0`: `OK (224 bytes @0x08010bec)`
- `make compare`: `ll.gba: 成功` (sha1 一致)
