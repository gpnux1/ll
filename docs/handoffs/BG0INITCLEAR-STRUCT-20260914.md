# Bg0_InitClear 结构体化重写 (2026-09-14, zcode-struct)

## 目标
0x08019148, 60B(注释槽前) / 132B 机器码, sio_link。原 status=1 (字节已匹配), 但 C 形态为
"4×s32 形参 + 一串手工 `d &= ~X; d |= Y;` 掩码", 可读性差且形参纯属凑形状。本次以结构体
重写, 保持字节不变。

## 语义 (E0/E1/E2)
1. `ewram = (u16*)0x02035AC0`; `vram = (u16*)0x06007000`; 循环 `i=0..0x3FF` 各写 0
   (0x400 个半字, 清两张 32×32 的图块图)。
2. `REG_DISPCNT |= DISPCNT_BG0_ON` (0x100)。
3. 配置 `REG_BG0CNT` (0x04000008):
   - CharBasep (字符基址) = 2 → 字符数据 0x4000 偏移 (VRAM 0x06004000)
   - ScBasep (屏幕基址) = 14 → 图块图 0xE800 (VRAM 0x06007000, 与上面清空区一致)
   - Priority=0 / Dummy_5_4=0 / Mosaic=0 / ColorMode=0 (16 色) / Loop=0 / Size=0
   - 最终寄存器值 = 0xE08 = `|8|0xE00`。

调用点 (E2, 全 ROM 仅 2 处, 均 `bl` 无参数): `sub_8048DA4` (battle_engine.c:1941) 与
`sub_803F328` case1 (event_hub.c:2841)。故签名应为 `void Bg0_InitClear(void)`。

## 关键: 末尾掩码链 = BgCnt 位域写
目标尾部 `movs r0,#4; rsbs; ands r3,r0; subs r0,#9; ands r3,r0; movs r0,#8; orrs; subs r0,#0x39; ands; ...`
是 GCC2.9 对**未初始化容器**逐字段赋值的产物: 每个 `x.field = C` 编译成
`v = (v & ~mask) | C` (常量经 `movs #(k+1); rsbs` 或 `subs` 链构造), 容器 `r3` 保持 SSA 不落地,
最终一次 `strh`。位域掩码/顺序与 `include/gba/types.h` 的 `BgCnt` 声明完全一致:
`Priority:2 / CharBasep:2 / Dummy_5_4:2 / Mosaic:1 / ColorMode:1 / ScBasep:5 / Loop:1 / Size:2`。

- 必须用**位域视图**而非裸 `u32 d`+手写掩码: 目标每条掩码值/指令形态来自位域 RMW, 手写
  `d &= ~3` 可对上部分但整体不可读且脆。
- 容器**不赋初值**: 写 `bg0cnt.word = 0` 会改变开头 (多余清零), 不匹配。
- 复用本文件(sio_link.c:567)已有的 `typedef union { BgCnt bg; u32 word; } BgUnion;`,
  与已匹配的 `sub_8018A58` (REG_BG2CNT) 同惯用法; battle_obj_core.c 另有一份同构 `BgUnion`。

## 命名依据
`BgCnt` 各字段名来自 include/gba/types.h (既有库头, 非本项目发明); CharBasep/ScBasep 取值
2/14 由目标常量 0xE08 反推 (CharBasep 占 bit2-3 → 2; ScBasep 占 bit8-12 → 14)。

## 验证
- bytecmp: `OK (132 bytes)` (符号全含, 无池重定位差)。
- fncheck: `OK (132 bytes @0x08019148, 0 池重定位已施加, 0 bl 槽忽略)`。
- `make` + `sha1sum -c ll.sha1`: 绿 (`ll.gba: 成功`), 进度 870/1059。

## 改动文件
- `src/sio_link.c`: Bg0_InitClear 函数体 (仅此函数区段)。
- `functions.tsv`: 0x08019148 行 note。
- 无头文件/原型变化 (code_0.h 已是 `void Bg0_InitClear();`)。

---

# 附: BgCnt 结构体使用点全扫描 (2026-09-14, 同日)

方法: `rg` 全 src/include + `rg --no-ignore` 全 asm (asm/ 被 gitignore, 必须加 --no-ignore)。
判据不是掩码常量 (`~0xC000`/`~0x2000`/`~0x1F00` 被 OAM/flag 等众多结构体复用), 而是
**是否写 BGxCNT 寄存器 (0x04000008/0A/0C/0E) 且掩码链 = BgCnt 字段序**。

## A. 已用 BgCnt/BgUnion 结构体 (3 处)
| 函数 | 文件 | 寄存器 | status |
|---|---|---|---|
| sub_8018A58 | src/sio_link.c:562-570 (Union), 602-612 | BG2CNT | 1 |
| sub_801A6F4 | src/battle_obj_core.c:156-159 (Union), 164-195 | BG1CNT + BG3CNT | 1 |
| Bg0_InitClear | src/sio_link.c:701-730 | BG0CNT | 1 (本次) |

注: `BgUnion { BgCnt bg; u32 word; }` 在 sio_link.c 与 battle_obj_core.c **各有一份重复 typedef**;
`DispUnion` 只在 sio_link.c。若将来共享化, 建议放 include 头, 但需全量重编验证。

## B. 手工 u32 + 裸掩码写 BGxCNT (可直接结构体化)
| 函数 | 文件 | 寄存器 | status | 备注 |
|---|---|---|---|---|
| Op_OpenWindow (0x8051A1C) | src/script_vm.c:887-922 | BG0CNT | 1 | ✅ **2026-09-14 已完成结构体化** (见下 §B1) |

字段序 (Op_OpenWindow): Priority=0 / CharBasep=2 / Dummy_5_4=0 / Mosaic=0 / ColorMode=0 /
ScBasep=31 / Loop=0 / Size=0 → 0x1F08, 与 Bg0_InitClear 完全同构 (仅 ScBasep 不同: 31 vs 14)。

### B1. Op_OpenWindow 结构体化 (2026-09-14 完成)
- 原: `u32 bgcnt;` + 9 行 `bgcnt &= ~3; bgcnt &= ~0xC; bgcnt |= BGCNT_CHARBASE(2); ...`
- 改: 在函数前加本地 `BgUnion`; 函数内 `BgUnion bg0cnt;` + 8 行 `bg0cnt.bg.<field> = C;`
  + `REG_BG0CNT = bg0cnt.word;`
- **与 Bg0_InitClear 的唯一差异**: ScBasep=31 填满 5 位掩码, GCC2 把
  `(v & ~0x1F00) | 0x1F00` 简化为 `orrs` 单独一条 (无前导 `ands`), 对应目标
  `movs r0,#0xf8; lsls r0,r0,#5; orrs r3,r0` —— 结构体写法自动产生, 无需特判。
- 验证: bytecmp `OK (208 bytes)`; fncheck `OK (208 bytes @0x08051a1c, 2 池重定位, 0 bl 槽)`;
  `make` + SHA1 绿 (870/1059)。
- 注意: 目标把 `u32 value = *pScriptCursor` 的读取排在末尾标量构造之后 (r4), 故源码须保留
  `value` 变量在 if 之后使用; 直接返回 `*pScriptCursor + 1` 会改变寄存器分配。

## C. 未匹配 ASM 中同构的 BGxCNT 配置 (结构体化是解 matching 的手段)
| 函数 | 文件 | 寄存器 | 字段配置 → 终值 |
|---|---|---|---|
| sub_8050720 (0x8050720) | script_vm | BG0CNT | P=0 CB=2 SB=31 Loop=0 Size=0 → 0x1F08 |
| sub_805144C (0x805144C) | script_vm | BG0CNT | 同上 (与 8050720/51BE4 尾段逐指令相同, 疑共享 helper) |
| sub_8051BE4 (0x8051BE4) | script_vm | BG0CNT | 同上 |
| sub_8018BF8 (0x8018BF8) | sio_link | BG3CNT | P=1 CB=0 SB=13 Loop=1 Size=0 → 0x2D01 |
| sub_8019B98 (0x8019B98) | battle_obj_core | BG1CNT/BG3CNT (按 index 选) | 有 0x1FF 图块掩码混入, 置信度较低 |

`sub_8018BF8` 的 BG3CNT 配置 (ScBasep=13/Loop=1) 与已匹配的 `sub_801A6F4` case8 同值, 是该
字段组合的第二处证据。

## D. 纯常量写 BGxCNT (无需结构体)
anim_slot.c (0x3C02/0x3D01/0x3F08/0x1E0F)、menu.c (0x1E01/0x1F09/0x3C02/0x1E09)、
sprite_engine.c (已用 BGCNT_SCREENBASE/BGCNT_CHARBASE 宏)。这些是直写常量, 位域无收益。

## E. 结论
- 结构体化还有 **1 处已匹配可安全复用**: `Op_OpenWindow` (B)。
- **4 处未匹配** (C) 含同构 BGxCNT 配置, 可作 matching 抓手, 但需先匹配字节, 不能只改形态。
- 全部候选当前**无人认领**; 本扫描未改任何文件。

