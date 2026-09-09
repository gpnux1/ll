# 数据区域全量导出计划 (de-blob + 符号化 + 类型化)

> 依据: `docs/INCIDENTS.md`(协作铁律/方案 B) + `docs/EXPERIENCE.md`(验证分层) +
> `docs/modules/README.md`+`FUNCTIONAL_MAP.md`。本文只管 **ROM 数据区**, 代码匹配见 AGENTS.md。
> 生成日期: 2026-09-01。所有数字均为本机实测 (`ll.map` / `scripts/data.json` / `baserom.gba` 扫描)。

---

## 0. 结论先行: 要解决的不是"导出", 是"两个不透明 blob"

| 事实 | 实测值 |
|---|---|
| ROM 总大小 | 8,388,608 B |
| `.text` (代码, 已走匹配流程) | 0x08000000–0x0805769C = 358,556 B (**4.3%**) |
| **数据区** | 0x0805769C–0x08800000 = 8,030,052 B (**95.7%**) |
| `scripts/data.json` 清单 | **4296 项 / 8,030,124 B / 0 gap / 0 overlap** —— 已经完美铺满数据区 |
| `data/raw_data/*.bin` | **4296 个 .bin 已全部 dump** (`scripts/dumpraw.py`, 已 gitignore, 可重生成) |
| 已逐项符号化的 | **仅 161 项 / 196,024 B (2.4%)** → `src/data_805769C.c` |
| **不透明 blob** | `data/data.s` = **6,453,768 B (占整张 ROM 77.0%)** + `data/data1.s` = 92,844 B |
| 声音区 | 1,281,512 B —— **已经逐项 granular** (`sound/*.inc` + 273 个入库 .bin), 不在本计划主攻范围 |
| 真 C 表 | `src/data_87E83F0.c` 4,452 B (主任务表/指针表) + `m4a_tables` 728 B + `agb_sram` 28 B |
| `linker.ld` 里手贴的绝对 ROM 符号 | **28 个** (`gUnk_0839CE7C = 0x0839CE7C;`) —— 这正是 blob 的症状: blob 内部的符号链接器看不见, 只能硬写地址 |

**所以本计划的主体是**: 把 6.55 MB 的 `.incbin "baserom.gba", off, len` 巨块, 换成
**4296 个有名字、有类型、可被链接器解析的符号**, 并且**每一步都保持 SHA1 绿**。

### 0.1 资产图其实已经机械可恢复 (这是本计划可行的关键证据)

对 `baserom.gba` 做了一遍全量指针扫描:

| 可达性来源 | 项数 | 说明 |
|---|---|---|
| `code.s` 里直接出现地址 | 278 | 只有 6.5% 的数据被代码直接引用 |
| 被**其它数据项里的指针**指向 | 3,912 | 指针表/资源索引 |
| **合计可达** | **4,186 / 4,296 = 97.4%** | |
| 仍孤立 (需再挖一层或人工) | 110 项 / 21,027 B | |
| 识别出的"纯指针表" (≥90% 表项是已知数据地址) | **53 张 / 15,980 B** | Tier-1 的首批目标 |

→ 不需要靠"匹配函数时顺便发现数据"这种慢路径; **资产图可以先生成, 再按图命名**。

### 0.2 编码分布 (决定"能不能解压再重压"的取舍)

| 类型 | 项数 | 字节 | 处置 |
|---|---|---|---|
| LZ77 (0x10) | 2,291 | 3,471,658 | **保持压缩原样 incbin**, 只做识别/命名, 不重压 |
| raw | 1,589 | 4,474,280 | 直接 incbin |
| RLE8 (0x01) / RLE16 (0x02) | 324 / 41 | 39,617 / 39,521 | 同上 |
| delta (0x03) | 30 | 4,973 | 同上 |
| fill (全 00/全 FF) | 3 | 44 | 可当 padding 合并 |
| tiny (<4 B) | 18 | 31 | 合并进邻项或直接 `.byte` |

---

## 1. 硬约束 (违反任何一条 = ROM 立刻红)

1. **数据区是字节紧排的**: 4296 项 0 gap 0 overlap。实测 **244 项起始地址非 4 对齐**、
   **227 项长度不是 4 的倍数** (例: `gObjPalFadeInSteps`(原 unk_8289B6E) size=48318)。
   → **任何逐项导出都不得插入对齐填充**。
2. `linker.ld` 的 `.rodata` 段带 `SUBALIGN(4)`: 它按**输入 section** 对齐。
   → 一个 region 文件 = 一个 `.rodata`, 其**起始锚点必须 4 对齐** (可用切点 4053/4297, 足够)。
   段内逐项**不要**写 `.align`。
2b. ⚠ **切点必须选在 4 对齐的 item 边界上 —— 不仅看起点, 更看终点**。
   实测踩到两次: 搬出区的结束地址非 4 对齐 → `SUBALIGN(4)` 给下一个输入段补 1~2 字节
   → **整张 ROM 平移** (一次 7.19 MB 差异, 一次直接 SHA1 失败)。
   推论: **一个逻辑对象能不能单独搬出, 取决于它的长度 mod 4 与后面邻接项**。
   若不为 0, 要么连带后续项一起搬到一个 4 对齐边界, 要么把它留在 blob 里。
   (实例: 168 B 的 `gChoiceGroupPairTable` 结束于 0x080882E2 (mod4=2), 必须连 286 B 的
    `gUnk_080882E2` 一起搬, 才能落到 0x08088400。)
3. 函数顺序不变的老规矩同样适用于数据: **地址顺序 = 布局顺序**, 生成文件禁止按名字排序。
3b. ⚠ **同样按地址序发射, 不能按"引用它的指针表"的顺序发射。**
   实测踩到: `off_87E96C8` 的 84 个表项不是地址序的（[3]=0x08087792 而 [4]=0x0808777e）,
   按表序声明会让 `.rodata` 排布乱掉 → 152 字节差异。**表索引只能写注释。**
4. ⚠ **每个搬出区段的结束地址必须 4 对齐**, 否则 `SUBALIGN(4)` 会给下一个输入段补位,
   导致整张 ROM 位移（实测: 结束于 0x0808823A → 补 2 字节 → **7.19 MB 差异**）。
   做法: 只在 4 对齐的 item 边界切块, 或把尾部零头若 2 B 当作填充数组一并搬出。
5. 非 const 的数组定义会掉进 `.data` 而不是 `.rodata` → 破坏布局。
   从 blob 搬出的项一律 `const`; 若已有 `extern u8 x[]`（非 const）声明在跳 C 文件,
   不必去改它 —— 链接期不检查 const 一致性, 且那边代码生成不变。
4. 分析/生成阶段**不改** `src/*.c` 的代码语义; 数据符号化只增删 `.rodata` 输入项与 `linker.ld` 条目。
5. 每一步的验收只认一个东西: `make && sha1sum -c ll.sha1` 绿
   (且必须看 `make` 尾部无报错 —— 坑 8)。

---

## 2. 架构决策: C 定义 vs `.incbin`, 三层模型

### 2.1 为什么不能"全部写进 C"

`src/data_805769C.c` 现在的写法 `const u32 x[] = INCBIN_U32("...")` 有三个硬伤, 放大到 4296 项会崩:

- **长度必须整除元素宽度**: 227 项 `size % 4 != 0` → `INCBIN_U32` 直接报错/补零;
- **编译成本**: `INCBIN_*` 由 `tools/preproc` 展开成海量 `.byte`, 单文件 4296 项会让
  `build/src/*.s` 膨胀到数百 MB, `make ctx` / m2c 上下文也跟着炸;
- **对齐**: C 数组天然带元素对齐要求, 与 §1.1 的紧排冲突。

### 2.2 三层模型 (采纳)

| 层 | 内容 | 载体 | 谁写 |
|---|---|---|---|
| **Tier 0 机械层** | 全部 4296 项, 每项一个全局标号 + `.incbin` | **`data/rodata/<region>.s`** (生成) | `scripts/data_gen.py` |
| **Tier 1 结构层** | 53 张纯指针表 + 表间引用 | 同上, 但生成 `.4byte <目标符号>` 重定位而非裸字节 | 生成器 + manifest |
| **Tier 2 语义层** | 已知布局的表 (角色基础数据/字库/精灵动画集/脚本头) | **`src/data_*.c` + `include/data_*.h`** 真 C 结构体 | 人工/AI 逐个提 |

**Tier 0 用 asm `.incbin` 而不是 .bin 文件**, 直接写:

```asm
	.global gUnk_0839CE7C
gUnk_0839CE7C:            @ 0x0839CE7C, size 0x1C4
	.incbin "baserom.gba", 0x39CE7C, 0x1C4
```

好处: **零新增二进制文件**、按构造字节正确、符号真实存在 (链接器可解析) →
`linker.ld` 里那 28 个手贴绝对符号**全部可以删掉**, 代码侧改用 `extern const u8 gUnk_0839CE7C[]`
拿真重定位 (顺带满足经验 6/32 的"防常量折叠"诉求, 不再靠硬写地址)。

Tier 2 里**已识别的图形/脚本资产**再升级成命名 `.bin` 资产文件 (pokeemerald 风格:
`data/graphics/<name>.4bpp.lz` / `.gbapal` / `.bin.lz`), 由 manifest 驱动, 与
`data/lunar_logo/` 现有做法一致。

### 2.3 目录布局

```
data/
  rodata/                 # Tier0/Tier1 生成物 (git 跟踪, 但只由生成器写)
    r0805_00.s .. r0805_FF.s     按 0x10000 分块, 与 docs/modules/README.md 的 MOD 地址视图对齐
  manifest/               # ⭐ 人工/AI 成果的唯一载体 (追加式, 抗并发)
    core.yaml             # 地址→{name,kind,type,decl,asset,note}
    agent-A.yaml agent-B.yaml ... # 每人一份, 生成器按地址合并 (functions.tsv 一行一函数+merge=union (已实装))
  raw_data/               # 已存在, 保持 gitignore
```

**关键纪律**: 生成文件 (`data/rodata/*.s`, `include/rodata.h`) **禁止手改**;
所有命名/类型/资产化的意图只写进 `data/manifest/*.yaml` (追加式, 天然可 `merge=union`)。
这样重新生成不丢工作, 多人不撞车。

---

## 3. 阶段计划

### Phase D0 — 基础设施 (0.5 天, 不改 ROM)

- [ ] `scripts/data_gen.py`: 读 `scripts/data.json` + `data/manifest/*.yaml` →
      输出 `data/rodata/*.s` + `include/rodata.h` + `build/data_gen_report.json`。
      切块方式: 只在 **4 对齐的 item 边界**断开; 每块头部写 `. = ` 锚点由 linker.ld 负责。
- [ ] `scripts/datacov.py`: 覆盖率/一致性审计 (见 §5), 输出 `docs/reports/data_coverage.csv` (目录重建)。
- [ ] `scripts/lz.py`: LZ77/RLE/delta 解码 + **round-trip 重压校验** (只用于识别, 不改字节)。
- [ ] `scripts/data_graph.py`: 指针表可达性 → `docs/reports/data_refs.csv`
      (`child_addr, child_size, parent_addr, offset, kind`), 复用 §0.1 的扫描逻辑。
- [ ] `.gitattributes` 追加 `merge=union` 到 `data/manifest/*.yaml` + `linker.ld`
      (AGENTS.md 铁律2 + merge=union, 现在就有 4 行)。
- [ ] **补可复现性缺口**: `scripts/data.json` 目前**无生成器** (gbadisasm 不产 JSON,
      它是 commit 538ab6c 一次性提交的)。要么写 `scripts/data_scan.py` 从
      `ROM + ll.cfg + 代码边界`重算 tiling 并与 data.json 对账, 要么在文档里明确
      "data.json 是人工维护的权威清单, 改动需过 datacov 校验"。**先选后者, 成本低。**

**D0 验收**: `python3 scripts/datacov.py` 能报出 §0 的全部数字; `make` 仍绿 (未动构建)。

### Phase D1 — 拆 blob (主体, 按 region 逐个搬, 每步绿)

顺序**从收益最高/风险最低的一端开始**:

| 步 | 区间 | 字节 | 理由 |
|---|---|---|---|
| D1.1 | 0x087E9554–0x08800000 (`data/data1.s`) | 92,844 | 最小, 193 项, 先验证整套流水线 |
| D1.2 | 0x0805769C–0x0808760C | 196,024 | 已有 161 项在 `data_805769C.c`, 迁成 Tier0 并**保留** Tier2 的真 C |
| D1.3 | 0x086AF214–0x087E82D4 (声音区) | 1,282,240 | 只做**对账**: 确认 `sound/*.inc` 与 data.json 的 255 项边界一致, 不重写 |
| D1.4 | 0x0808760C–0x086AF214 (`data/data.s` 主块) | 6,453,768 | 3671 项, 按 0x10000 分 ~100 个 region 文件, **一次一个文件**合入 |

每一步的固定动作:

```bash
python3 scripts/data_gen.py --region 0x087E9000-0x08800000 --emit data/rodata/r087E9.s
# 1) 在 linker.ld 的 .rodata 里, 用 `. = ORIGIN(rom)+0x7E9554;` 锚点 + 新文件替换旧 blob 片段
# 2) 只删被本 region 覆盖的那段 .incbin, 不碰别的
timeout 900 make 2>&1 | tail -3 && sha1sum -c ll.sha1
python3 scripts/datacov.py --bytes      # 逐符号字节校验
git add -A && git commit -m "data: de-blob 0x087E9554-0x08800000 (193 syms)"
```

**D1 验收**: `data/data.s` / `data/data1.s` 里**不再有任何 `.incbin "baserom.gba"`**;
`datacov.py` 报 "blob 0 B"; 符号数 4296; SHA1 绿。

### Phase D2 — 清掉 `linker.ld` 的 28 个手贴绝对符号

blob 拆完后, `gUnk_0839CE7C` 等已经是真符号 → 删 `SECTIONS {}` 外的绝对赋值,
改成引用生成符号。**这一步会改变重定位形态**, 因此必须逐符号 `make` 验证
(经验 32: 符号化/字面量化本身会改变代码生成 —— 这里改的是数据侧引用, 但池值必须不变)。

**D2 验收**: `grep -cE "^[A-Za-z_][A-Za-z_0-9]* = 0x08" linker.ld` → 0; SHA1 绿。

### Phase D3 — Tier 1: 指针表 → 重定位 (把资产图变成可编译事实)

对 §0.1 的 53 张纯指针表 + `off_*` 140 项:

```yaml
# data/manifest/core.yaml
- addr: 0x087E8430
  name: gSpriteGfxPtrTable
  kind: ptr_table
  element: "const u32"
  targets_are: asset
```

生成器把该表从 `.incbin` 改写成:

```asm
	.global gSpriteGfxPtrTable
gSpriteGfxPtrTable:
	.4byte unk_80A1314
	.4byte unk_80A16C8
	...
```

→ 表项与被指向资产**在链接期绑定**。此后任何一次资产重定位/改名都由链接器保证一致,
而且 `ll.map` 里能直接读出"表 → 资产"的引用关系。

**D3 验收**: 53 张表全部重定位化; `data_graph.py` 的孤立项从 110 降到 <20。

### Phase D4 — Tier 2: 语义化命名与结构体 (与代码匹配并行, 由 `docs/modules/` 承接)

按 `FUNCTIONAL_MAP.md` 的 F 视图分工, 每块数据挂到一个功能模块, 结论写进
`docs/modules/F*-*.md` 的数据小节 (沿用 docs/modules/ 模板), 命名走
`data/manifest/<agent>.yaml`:

| 优先 | 目标 | 依据 |
|---|---|---|
| 1 | 字库 `gUnk_08095028` (已注册) | `sub_80166A4` 已匹配, 布局已知 |
| 2 | 角色基础数据 `gCharaBaseData` = 0x087EA580 | 已有别名符号 |
| 3 | 脚本区 0x0861xxxx–0x0862xxxx (含 `gUnk_0862D434` 调度表) | MOD-08/F8 主循环+脚本 VM 已 74/102 匹配 |
| 4 | 精灵动画集指针表 `gUnk_087EA1A0` / 图块表 `gUnk_087E8430` | `LoadSpriteAnimSet`/`UpdateSpriteAnim` 已匹配 |
| 5 | 文本/消息表 (`gMsgTable` 的 ROM 侧) | F4a 文本渲染已部分匹配 |
| 6 | 地图/场景块 (0x0822xxxx 附近 400 KB 大项) | MOD-06/F6 定性后 |

**判据**: 只有当某个数据块被**至少一个已匹配真 C 函数**以类型化方式引用时, 才从 Tier0
提升到 Tier2 真 C —— 避免凭猜定义结构体导致后续函数匹配返工。

### Phase D5 — 收尾

- `scripts/lz.py --roundtrip` 全量跑一遍: 对 2291 个 LZ77 项做"解压→重压→与原字节比对",
  把**可无损重压**的资产列入可外提清单 (仅记录, 默认仍用原字节 incbin);
- `docs/reports/data_coverage.csv` + `data_refs.csv` 进 git (docs/reports/ 重建后入库);
- 更新 `README.md` / `AGENTS.md` §4 工具表。

---

## 4. 要写的工具 (全部新文件, 不碰现有流程)

| 工具 | 职责 | 关键设计 |
|---|---|---|
| `scripts/data_gen.py` | data.json + manifest → `data/rodata/*.s` + `include/rodata.h` | **幂等**; 只在 4 对齐边界切块; 按地址排序; 输出报告含每块锚点 |
| `scripts/datacov.py` | 覆盖率/一致性/逐符号字节审计 | `--bytes` 用 `ll.map`+`baserom` 对每个符号做区间 cmp; `--fix` 只改 manifest 不改生成物 |
| `scripts/data_graph.py` | 指针表可达性、孤立项、表→资产边 | 输出 CSV 进 `docs/reports/` (重建) |
| `scripts/lz.py` | LZ77/RLE8/RLE16/delta 解码 + 重压 round-trip | 只读; 用于识别与命名建议 |
| `scripts/audit.py` (扩展) | 加 `--data`: 检查 blob 残留、生成物是否被手改、manifest 与生成物漂移 | 与现有函数清单审计同一入口 |

---

## 5. 验证分层 (对齐 EXPERIENCE.md 的 fndiff/fncheck 思路)

| 层 | 命令 | 抓什么 |
|---|---|---|
| L1 形状 | `python3 scripts/datacov.py` | gap/overlap/项数/blob 残留字节 |
| L2 字节 | `python3 scripts/datacov.py --bytes` | 单个符号的字节与 baserom 不符 (对齐/长度/顺序错) |
| L3 全 ROM | `make && sha1sum -c ll.sha1` | 最终裁决 |
| L4 归属 | `python3 scripts/fncheck.py --blame` | 红的时候判断是数据段还是别人的代码段 |

**L2 是新增的关键一层**: 现在 blob 一坏就是 6.4 MB 全坏, 无法定位; 逐项符号化后
`--bytes` 能直接指出"哪个符号的第几字节不对", 把调试粒度从"整张 ROM"降到"单个资产"。

---

## 6. 风险与对策

| 风险 | 概率 | 对策 |
|---|---|---|
| 插入对齐 → 整体位移, ROM 红几 MB | **高** | §1.1/1.2 硬约束; 只在 4 对齐边界切块; 生成器禁止输出 `.align`; L2 逐符号 cmp |
| `SUBALIGN(4)` 与新 region 起始地址冲突 | 中 | 每 region 前显式 `. = ORIGIN(rom) + <anchor>;`, 锚点由生成器算好并写进报告 |
| 生成物被手改, 下次重生成丢失命名 | **高** | 命名只进 `data/manifest/*.yaml`; `audit.py --data` 校验生成物 hash |
| 多人同时改 `linker.ld` 的 .rodata 列表 | 中 | `.gitattributes merge=union` + 每人只管自己 region 的那一行; 定期 `audit.py` |
| Tier1 重定位化后, 表项值与原字节不符 (指针对象算错) | 中 | 生成前用 `data_graph.py` 校验 `rom[off:off+4] == target_addr`; 不匹配就退回 `.incbin` |
| 声音区被误重写 | 低 | D1.3 明确"只对账不重写"; `sound/` 保持现状 |
| 4296 个符号让 `ll.map`/`ctx.c` 膨胀 | 低 | 符号只进 `include/rodata.h` (纯 extern), 不进 `ctx.c` 的展开链; 必要时分片头文件 |
| 命名工作与函数匹配抢人力 | 中 | D4 明确"由已匹配函数驱动", 不单独开一条战线; D0–D3 是纯机械+脚本, 可并行 |

---

## 7. 里程碑与度量

| 里程碑 | 验收 | 当前 |
|---|---|---|
| **DS0** 工具就绪 | `datacov.py` 能复述 §0 全部数字; `data_gen.py` 干跑 diff 为空 | 0% |
| **DS1** blob 清零 | `data/*.s` 中**未被注释**的 `.incbin "baserom.gba"` 行数 → 0 (当前活跃 4 行:
  `data.s` 2 + `data1.s` 1 + `sound_data.s` 1); 符号数 4296 | **2.4%** (161/4296) |
| **DS2** 绝对符号清零 | `linker.ld` SECTIONS 外 `= 0x08...` 条目 = 0 | 28 个待清 |
| **DS3** 资产图入库 | 53 张表重定位化; 孤立项 <20 | 0 张 |
| **DS4** 语义命名 ≥200 | manifest 里 `name != gUnk_*` 的项 ≥200, 且每项有 `note` 指向引用它的已匹配函数 | 5 (`data_805769C.c`) + `gIntrTable`/`gMainTasks`/字库等零星 |
| **DS5** 数据可读性终态 | 数据区 ≥60% 字节有类型化 C 声明或命名资产文件; SHA1 保持绿 | ~2.5% |

**全程红线**: 每一步合入后 `make` + SHA1 必须绿, 否则**只回退本 region 的那一个文件**
(铁律 3: 绝不整体回退共享文件)。

---

## 8. 实例：数据表维度判定模板 (0x0805881C–0x0805888C 一批 5 张表)

> 方法 = 数据侧的"逐指令形状回环": 不猜名字, 而是从**索引表达式反推维度**。
> 工具: `grep -n "0x<ADDR>" code.s` 找引用点 → `.scratch/funcof.py <line>` 定位宿主函数
> → 读索引算式里的 `lsls/ands/adds #K` 得到行跨距与维宽。

| 地址 | 现名 | 字节 | **维度** | 索引证据 |
|---|---|---|---|---|
| 0x0805881C | `gWalkMoveDirLut` | 16 | **1-D `u8[16]`** | `table[moveFlags]`, moveFlags = D-pad 4-bit 码 (UP=1/DOWN=2/LEFT=4/RIGHT=8), 0..15 全覆盖 |
| 0x0805882C | `gWalkAnimFrameMapping` | 8 | **2-D `u8[2][4]`** | 一处 `table[phase]` (phase=(animTimer>>3)&3 → 0..3), 一处显式 `table[phase + 4]` → **行跨距 4** |
| 0x08058834 | `gWalkAnimDimTable` | 48 | **3-D `u8[4][4][2]` = 前 32 B**; 后 16 B 零引用应拆出 | `idx = ((attr0>>11)&0x18) + ((attr1>>13)&6)` = shape*8 + size*2; 取 `[i]`/`[i+1]` → **末维宽 2**; 写入 `gCurSpriteW`/`gCurSpriteH` |
| 0x08058864 | `gWalkDirectionMapping` | 24 | **2-D `u8[3][8]`** (第 3 行零引用) | `table[charaObj->facingDir]` 与 `table[facingDir + 8]` → **行跨距 8**, facingDir ∈ 0..7 |
| 0x0805887C | `gSpriteTileCountTable` | 16 | **2-D `u8[4][4]`** | `table[(oam[1]>>6)*4 + (oam[3]>>6)]` = shape*4 + size → 行跨距 4, 正好 16 B |

### 拆表证据 (0x08058834 实为 32 + 16)

前 32 字节按 `[4][4][2]` 展开就是标准 GBA OBJ 尺寸表, **shape 3 (prohibited) 呷到 1×1 兼底**:

```
shape0 方形 : (8,8)  (16,16) (32,32) (64,64)
shape1 横向 : (16,8) (32,8)  (32,16) (64,32)
shape2 纵向 : (8,16) (8,32)  (16,32) (32,64)
shape3 非法 : (1,1)  (1,1)   (1,1)   (1,1)
```

最大可达索引 = 3*8 + 3*2 + 1 = **29** → 前 32 B 全部可达; 而 0x08058854–0x08058863 这 16 字节
在 code.s 里 **0 个引用点** (已按基址+偏移两种形式扫过) → `scripts/data.json` 把它跟尺寸表当成一项
是**分块粒度问题**, 不是同一逻辑表。拆成 `gObjSizeTable[4][4][2]` + `gUnk_08058854`(16 B) 后
地址不变 (u8 数组无对齐要求)。

### 命名修正建议 (与上表同一批)

| 地址 | 建议 | 理由 |
|---|---|---|
| 0x0805881C | 保留 `gWalkMoveDirLut` | 1-D LUT, 名字已准确 (值 = 1-based 8 方向, 顺时针从 UP; 对立方向组合→0 不动) |
| 0x0805882C | 保留名, **类型改 `const u8[2][4]`** | 两行 = 两套帧图, 列 = 4 个动画相位 |
| 0x08058834 | **改 `gObjSizeTable`**, 类型 `const u8[4][4][2]`, 拆掉后 16 B | 旧名 `WalkAnim` 错: 它是通用 OBJ shape×size 尺寸表, 被 `Sprite_EnqueueRender` 与 `Anim_BuildOamChain` 共用, 与行走无关 |
| 0x08058864 | **改 `gFacingDirAttrTable`**, 类型 `const u8[3][8]` | 旧名 `DirectionMapping` 误导: 它不映射方向, 而是**按 facingDir 查属性** —— 行0 当枚举用 (`==3`/`==1` 决定 attr1 的 HFlip 位), 行1 当**位标志**用 (bit0 → 瓦片偏移 0x20, bit1 → HFlip) |
| 0x0805887C | 保留名, **类型改 `const u8[4][4]`** | 名字准确 (每 shape×size 需多少块 OBJ), 只是没体现二维 |

**改名安全性**: 5 张表在 `asm/` 里全部**不按名字引用** (gbadisasm 写硬码 `.4byte 0x08058834`),
所以改名只影响 C 侧 → 零链接风险 (见 EXPERIENCE.md「符号改名管线」)。

## 9. 一句话给决策

数据侧的地基 (完美 tiling 的 4296 项清单 + 全部 .bin) **已经在仓库里躺着了**,
95.7% 的 ROM 面积目前只是被两行 `.incbin "baserom.gba", ...` 糊住。
先花半天做 D0 的两个脚本, 然后按 region 机械拆块 —— 拆完的那一刻起,
`linker.ld` 的 28 个手贴地址可以删、数据调试粒度从整张 ROM 降到单个资产、
而且 97.4% 的数据项已经能通过指针表自动挂上名字。**这是当前性价比最高的一条战线。**
