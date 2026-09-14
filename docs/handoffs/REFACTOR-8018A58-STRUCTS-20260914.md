# Handoff: sub_8018A58 (0x08018A58, 184 行汇编 / 416 字节) — 结构体语义重构 ✅

日期: 2026-09-14 | agent: antigravity | 状态: **完成重构合入** (status=1, fncheck OK 416B, 全量 make + sha1 保持一致, 870/1059)

## 1. 重构背景与破局

历史代码中，`sub_8018A58` 虽然字节匹配，但存在严重的“伪造掩码凑形状”问题：
- 声明了 `u32 dispcnt; u32 bldcnt;` 并使用长达 25 行的硬编码位运算 (`dispcnt &= ~7; ... bldcnt |= 3; ...`)。
- 历史注释甚至猜测其写入 `0x0400000C` 是“疑似本想写 BLDCNT (0x04000050) 的死写入”。

**语义取证与建模确认**:
1. `0x0400000C` 是 GBA 硬件的 `REG_BG2CNT`，与前面的 `REG_DISPCNT` (0x04000000) 构成战斗场景主 BG 层的显示启用。
2. 历史代码中 25 行看似杂乱的 `&=` 与 `|=` 掩码链，其常量序列与字段次序**100% 逐字段精确对应** `DispCnt` 与 `BgCnt` 位域：
   - `dispcnt`: `BgMode=0`, `Bmp_FrameNo=0`, `Obj_H_Off=1`, `ObjCharMapType=1`, `Lcdc_Off=0`, `Bg0_On=0`, `Bg1_On=0`, `Bg2_On=1`, `Bg3_On=1`, `Obj_On=1`, `Win0_On=1`, `Win1_On=0`, `ObjWin_On=0`。
   - `bg2cnt`: `Priority=3`, `CharBasep=0`, `Dummy_5_4=0`, `Mosaic=0`, `ColorMode=0`, `ScBasep=12`, `Loop=1`, `Size=0`。
3. 这些参数与前文代码高度互证：
   - 前文 `LZ77UnCompVram(tiles, 0x06000000)` 正好解压到 CharBase 0；
   - 前文 `LZ77UnCompVram(map, 0x06006000)` 正好解压到 ScreenBase 12 (12 * 0x800 = 0x6000)。
4. `gUnk_087ED394` 三元组表建立明确结构体 `BattleBgGfx`：
   - `tiles`: 瓦片图形数据
   - `pal`: 调色板数据
   - `map`: 瓦片地图数据

## 2. 结构化重构形态

使用标准 `DispUnion` / `BgUnion` 联合体进行自然位域赋值：
```c
    dispcnt.disp.BgMode = 0;
    dispcnt.disp.Bmp_FrameNo = 0;
    dispcnt.disp.Obj_H_Off = 1;
    dispcnt.disp.ObjCharMapType = 1;
    dispcnt.disp.Lcdc_Off = 0;
    dispcnt.disp.Bg0_On = 0;
    dispcnt.disp.Bg1_On = 0;
    dispcnt.disp.Bg2_On = 1;
    dispcnt.disp.Bg3_On = 1;
    dispcnt.disp.Obj_On = 1;
    dispcnt.disp.Win0_On = 1;
    dispcnt.disp.Win1_On = 0;
    dispcnt.disp.ObjWin_On = 0;

    bg2cnt.bg.Priority = 3;
    bg2cnt.bg.CharBasep = 0;
    bg2cnt.bg.Dummy_5_4 = 0;
    bg2cnt.bg.Mosaic = 0;
    bg2cnt.bg.ColorMode = 0;
    bg2cnt.bg.ScBasep = 12;
    bg2cnt.bg.Loop = 1;
    bg2cnt.bg.Size = 0;

    REG_DISPCNT = dispcnt.word;
    REG_BG2CNT = bg2cnt.word;
```
不仅代码语义 100% 还原 GBA 硬件控制，而且 GCC 2.95 编译生成的 48 条 Thumb 指令与原 ROM 完全逐字节一致。

## 3. 验证与交付

- `src/sio_link.c`: 完成结构体重构。
- `functions.tsv`: 更新第 388 行 note。
- 验证: `fncheck.py sub_8018A58` OK (416B)，`make` + `sha1sum -c ll.sha1` 终验通过。
- 姊妹函数启示: 邻接函数 `sub_8018BF8` (line 389) 中同样包含针对 `BG3CNT` 的所谓“掩码链”，同样可通过 `BgUnion` 位域结构体进行彻底清理与匹配攻坚。
