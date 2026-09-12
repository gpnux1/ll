# STRUCT-BATTLEOBJ — battle_obj_core.c 结构体化签名改造 (2026-09-12, gpnux)

## 结果

`src/battle_obj_core.c` 中 30+ 个仍用 `u8 *` + 裸偏移的已匹配函数改为正确的
`BattleObj *` / `ObjHead *` 签名; 相关头文件与外部调用点同步。
**全量 make + sha1 绿, 800/1059 (75.5%)**。

## 结构体字段提升 (布局不变, 仅拆 pad, offset 命名)

`include/code_0.h` 的 `BattleObj` 补:
- `+0x6E u16 f_6E` (D12C/D19C 与 f_6C 判等)
- `+0xA2 u8 f_A2` (D12C 写子状态)
- `+0xB4 u16 f_B4`, `+0xB6 u16 f_B6` (CE80 写)
- `+0xC3 u8 f_C3` (2093C 写)

`f_6C`/`f_B2`/`f_C2` 为前一会话已提升, 本次沿用。全部为 **offset 命名**:
无跨模块语义证据, 只有一个 TU 内的写入者, 不做语义猜测。
(注: `f_6C` 在 battle_engine 有 `st->hp = obj[0x6C]` 的回存, 但那是战斗单位属性
拷贝, 不足以断言对象池字段就是 HP; 保留 offset 名。)

`Unk_801B81C` 局部视图 (battle_obj_core.c 内, 与 ObjHead 字段一一对应) **退役**,
B81C 直接用 `ObjHead *`。

## 签名变更清单

| 组 | 函数 | 变更 |
|---|---|---|
| ObjHead | sub_801B570 | `u8*` → `ObjHead*` |
| ObjHead | sub_801B81C | `u8*` → `ObjHead*` (字段访问换 ObjHead 名) |
| ObjHead | sub_801B878 / sub_801B8AC | `u8*` → `ObjHead*` |
| ObjHead | sub_801B8FC | `u8*` → `ObjHead*` |
| ObjHead | sub_8020974 | `u8*` → `ObjHead*` |
| BattleObj | sub_801CE80 / D12C / D19C / DB3C / DC20 / DD04 / DDB0 | `u8*` → `BattleObj*` |
| BattleObj | sub_801DEDC / DF90 | `u8*,u8*` → `BattleObj*,BattleObj*` |
| BattleObj | sub_801E4D4 | `u8*,u8*` → `BattleObj*,BattleObj*` |
| BattleObj | sub_801E040 | 局部 obj 类型 + `gUnk_030006F8[]` 元素 |
| BattleObj | sub_80207DC / 20840 / 208A4 / 20914 / 2093C | `u8*` → `BattleObj*` |
| BattleObj | sub_8020A0C / 20A7C / 20B90 / 20BC0 / 20BF0 / 20C2C / 20C58 | `u8*` → `BattleObj*` |
| 未匹配 | sub_801CA08 / CBA4 | `()` → `(BattleObj*, u8, u16, u8, u8)` |
| 未匹配 | sub_801D568 | `()` → `(BattleObj*)` |
| fnptr | `UnkFunc20C2C` | `u8*(*)` → `BattleObj*(*)` |

全局: `include/iwram.h` `gUnk_030006F8[]` `u8*` → `struct BattleObj *` (仅本 TU 消费)。

| BattleObj | sub_80209C8 / sub_80209EC | `u8*`/`MyStruct*` → `BattleObj*` (0x88 保持 `(u16*)` 半字写) |

`MyStruct` 局部视图退役 (sub_80209EC 直接用 BattleObj; 0x88 半字写用
`*(u16 *)&ptr->animPtr = 0` 保留, 直接 `ptr->animPtr = 0` 会变 u32 存储)。
注: sub_80209EC 的 `state |= 2` 需写成命名临时变量 `new_var = *st | 2; *st = new_var;`
才命中目标 ORR 寄存器序 (与同族 sub_80209C8 一致; 直接 `|=` 会让常量与载入值互换)。

| BattleObj | sub_801FEBC | `void *` → `BattleObj *` (本地 Unk_8020F4C 视图退役) |

sub_801FEBC: 原 `void *varg` + 本地 `Unk_8020F4C` 视图 (字段与 BattleObj 精确同构:
0xB0=state, 0x24=headA.kindFlags, 0x37/0x38=headA.f_2B/f_2C) 改为直接 `BattleObj *`,
视图退役。字节保持 132B OK。

**更正 (同日追加)**: 此前该函数保留了 `p = &arg0->state; *(u8*)(p-0x79)` 与
`*(u8*)((u8*)arg0+0x38)` 两处裸指针造型, 并注释称"匹配必需"。**该结论错误** —— 经
强制重编 A/B/C 对照 (raw 造型 / 仅 0x38 改字段 / 全字段) 三种写法产出**完全相同**的
132 字节, 现已全部清理为纯字段访问 `arg0->headA.f_2B` / `f_2C`。裸造型是 `void*`
时代的遗留, 改型后不再需要。
教训: 判"某造型是否必需"必须用 `rm -f <obj> && make` 强制重编做对照; 直接
`make <obj>` 会因时间戳粒度/并行构建给出假绿或假红 (本次就出现过同一变体两次结果
不一致)。详见 docs/EXPERIENCE.md。

**有意保留 `u8*`**: sub_8020B04 (arg0 透传给 sub_80462E4 的不透明过滤器, 其余调用者
也传 u8*, 无对象证据)。

## 两处"静默缩放"陷阱 (关键, 全量才暴露)

1. `gUnk_030006F8[0] + 0xBE` — 全局改型后指针算术按 sizeof(BattleObj)=0xC8 缩放,
   生成多余 `muls #0xC8` (sub_8020C2C 44B→48B)。**此 TU 内 fncheck 单函数全绿,
   但全量链接失败**: `.rodata` 紧跟 `.text`, 绝对边界 0x61C784 被撑破 4 字节 →
   `ld: cannot move location counter backwards (0x61C788 → 0x61C784)`。
   修复: `gUnk_030006F8[0]->slot`。
2. `arg1[0xBE]` (u8* 下标) 改型后按 0xC8 缩放且类型错误 — 编译器报
   "invalid operands to binary >", 能抓到。

**教训**: 结构体化改造中, `指针 + 偏移` 形式在改型后会**静默**生成错误取址;
单函数 fncheck 抓不到 (生成的代码自洽且"看起来"匹配), 只有全量 sha1 兜底。
改型后必须逐函数核对所有 `ptr + off` / `ptr[int]` / `gArr[i] + off` 表达式;
定位手段: 全项目 fncheck 找字节数异常者。

## 外部调用点修整 (纯 cast, 零代码生成影响)

- 去 cast: `battle_anim.c` (obj 已 ObjHead*), `sio_link.c` (`&obj->headX` 已 ObjHead*)
- 加 cast: `scene_obj_fx.c`, `cutscene_mgr.c`, `event_hub.c`, `battle_engine.c`
  (调用点持有 `u8*`/`void*`, 显式 cast 到 BattleObj*/ObjHead*)

## 验证

- 单函数: battle_obj_core 57/57 + battle_engine 62 + scene_obj_fx 27 + cutscene_mgr 3
  + event_hub 27 + battle_anim 54 + sio_link 71 **全部 fncheck OK**。
- 全量: `make` + `sha1sum -c ll.sha1` **成功**, 800/1059 (75.5%)。

## 可复现的坑 (给后续 agent)

- fncheck 对某些 TU (engine_core/battle_gfx_load) 因未解析符号会整片 FAIL, 属工具局限,
  非真回退; 判断真回退看**字节数**是否与参考一致。
- 结构体化是"语义正确、字节不变"的改造, 但任何漏改的指针算术都会改变字节;
  改完必须全量 sha1, 不能只看局部函数。
