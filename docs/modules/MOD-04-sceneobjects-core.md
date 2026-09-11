# MOD-04 战斗对象核心 (0x0801A3C4-0x08020D50)

> 分析人: plan (2026-08-31)。源文件原为单一 `src/code_801A3C4.c`; 2026-09-06 zcode-ll2 为解除 TU 状态泄漏对 sub_8020B54 local_alloc tiebreak 的扰动 (经验 148), 拆分为 `src/code_801A3C4.c`(仅 sub_801A3C4) + `src/code_801A5EC.c`(sub_801A5EC..sub_8020CC4), linker.ld 两 .o 顺序拼接, functions.tsv module 列已同步。
> 2026-09-11 gpnux-rename: 调用图证实本系统**战斗专用** (`gMainLoopCallbacks[1]=BattleTask_Run` 战斗主循环 →
> sub_8018070/80184A8/801B964 → ObjGfxLoad_Step/命令分发, 地图侧无调用), 文件改名
> `src/battle_gfx_load.c` (ObjGfxLoad_Step) + `src/battle_obj_core.c` (其余), TSV module 列已同步;
> 旧注"地图 NPC/宝箱"有误 — 0xC8 池 (0x02037028) 是战斗对象池, IWRAM 0x03000254(+0x14)/0x03000918 是同头部独立实例。
> **核心发现: 战斗对象 (0xC8 字节) 有自己的脚本 VM** — 对象头部 (ObjHead, 0x30 字节) +0x0/+0x4 是两个命令流基址,
> +0x8/+0xC 是跳转偏移表; 行为按 `kindFlags & 0xF` 的 kind 值分发 (类似 MOD-08 的 opcode 表, 但按对象类型)。

## 对象结构 (0xC8 字节, 池 0x02037028, 关键字段)

+0x0/4 脚本代码指针 | +0x8/C 命令流 | +0x10/14 脚本基址对 | +0x18 kind(bits0-3) | +0x1A/1C/1E 计数
+0x24 标志(bit11=移动完成) | +0x28/29 复制槽 | +0x3C 精灵设置区(Unk_801B81C) | +0x66 样式
+0x6C/6E 移动坐标 | +0x88 数据指针 | +0xA2 移动状态 | +0xA3 移动参数 | +0xAB 方向
+0xB0 状态位(0x80=移动,0x2000=跳跃,bit1=锁定) | +0xB2 步长 | +0xBB/BC/BD 辅助
+0xBE 槽号(≤0xB 有效) | +0xBF/C0 朝向/参数 | +0xC3 事件值

## 真 C 函数 (30 个中 22 个已读)

| 地址 | 状态 | 语义名 | 语义 |
|---|---|---|---|
| 0x0801A684 | ✅C | `Obj_ResetScript` | 解析脚本头 (双偏移→+0/4, +4→8/C), 清计数, 按 kind 6-8 分发 |
| 0x0801B760/790/7B8 | ✅C | `ObjFlag_Set/Test/ClearAll` | gUnk_03000518 位图 (0x80B) + 03000598 |
| 0x0801B81C | ✅C | `ObjSprite_Setter` (已匹配) | 10 参精灵设置器 (field_10-2F, 见 MOD-05 跳跃/滑动调用) |
| 0x0801B878 | ✅C | `Obj_CmdDispatch` | kind 6/7/8→801AD0C, 其他→801A884 (对象命令分发) |
| 0x0801B8AC | ✅C | `Obj_FrameDispatch` | kind 6→801B570, 7/8→空, 其他→801B0B8 (每帧分发) |
| 0x0801B570 | ✅C (匹配 2026-09-11) | `Obj_AnimTilesDMA` (sub_801B570) | kind6 对象帧构建: 逆序遍历命令流计数, 801B8E8 查跳转项 → 帧表 (+n0*4 跳过) → 逐条目 DMA3 拷 EWRAM 0x0202B2C0 帧图块 → OBJ VRAM 0x0600C000+tile*0x20 (tile 从 1 起, 尺寸=0x08393A30[大小×4+格式]<<4); f_18&0x800 关 DMA 只累计 tile; 详见 progress.md |
| 0x0801B8E8 | ✅C | `Obj_FindJumpEntry` | 命令流跳转表查找 (value 比较步进) |
| 0x0801B8FC | ✅C | `Obj_FindJumpEntryAt` | 同上, 从 +0xC 表按 arg1 索引 |
| 0x0801B920 | ✅C | `OamAffine_Copy32` | gUnk_030034C0[32]→OAM 仿射参数 |
| 0x0801B954/95C | ✅C | `Obj_GetByte2/GetU16At1` | 指针槽字段读取 |
| 0x0801D12C | ✅C | `Obj_SetMoveState` | 0xA2 = f(0xAB 方向, 坐标相等判定) 移动状态机 |
| 0x0801D468 | ✅C | `sub_801D468` (匹配 2026-09-05) | 战斗对象列表装配: 80489E8(mode0/1, 0xE3) 分段扫槽号 + 8048ACC 各段快排 → 拼接; gGstate324&0x1000 置位则跳过段0并清位; 槽号×0xC8 填 gUnk_03000638[], 03000669=0 / 03000668=总数 |
| 0x0801DE44 | ✅C | `ResetSceneObjects` (已有名) | 清生成队列+按出生点表逐个 801D710 初始化 |
| 0x08020798/07A4 | ✅C | `Obj_Get744/Dec630` | 计数器 getter/递减 |
| 0x0802093C | ✅C | `Obj_ReadEventVal` | 按对象 0xBC 选 0x88 指针表偏移 → 0xC3 |
| 0x08020974 | ✅C | `ObjSprite_SetById` | gUnk_08393B28[arg1] 表 → ObjSprite_Setter |
| 0x080209EC | ✅C | `Obj_ResetWalkOfs` | 0x88=0, 0xB0\|=2 (槽≤6) |
| 0x08020A0C | ✅C | `ObjSprite_SetJump` | gUnk_0839B2A4 表 + arg1<<5 → 跳跃精灵 |
| 0x08020A7C | ✅C | `Party_AllAnimIdle` | 5 对象 8045F10(0x114) 检查 |
| 0x08020AB0 | ✅C | `ObjGroup_AnyEvent` | 80489E8 事件扫描 + 8044498 |
| 0x08020AE4 | ✅C | `ObjList_IncAffine` | 链表 (0x03000690) 遍历, 各对象 +0xB2 递增 |
| 0x08020B04 | ✅C | `ObjGroup_Spawn` | 80462E4 收集槽号 → 801D568 逐个初始化 |
| 0x08020B48 | ✅C | `Obj_Get718` | gUnk_03000718 getter |
| 0x08020B90 | ✅C | `Obj_Register` | gUnk_030006F8[714++]=obj; 槽>0xB → 718=obj (主角对象) |
| 0x08020BC0 | ✅C | `Obj_WalkCooldown` | 0x6C -= 0xB2 (≤0 归零返回 1) |
| 0x08020BF0/20C2C | ✅C | `Obj_FrameDispatchByKind` | 主角对象按 0xBE 分派 gUnk_0839CE7C 表 (0x71-0xFD 两段) |
| 0x08020C58 | ✅C | `Objs_PerFrameUpdate` | 遍历 0300062C 计数, 跳过 0xFF/状态 8/5, 按 gUnk_03000324 bit5 分派 804CEE0/804DD70 |
| 0x08020CC4 | ✅C | `ObjSprite_SetFromTable` | gUnk_08393B28[arg5] → ObjSprite_Setter + 0xB0\|=0x2000 |
| 0x0801D984 | ✅C | `Obj_WriteOamBuf` (匹配 2026-09-06) | OAM 缓冲自绘: 按 gUnk_0300068C 循环把 gUnk_03000670[i] 写入 gOamBuffer[r6] (r6 递减, 返回递减后槽号); 入口 068C≠0 时用 068E/068D 经 sub_801768C 插值算 VPos 偏移; 全字段走 GameOamData.fields 位域 |
| 0x0801FEBC | ✅C | `Obj_SetSlideParams` (匹配 2026-09-03) | 滑动参数组: 0xB0=(0xB0&0xFF0F)\|0x20, 写 0x03000618-624 (61C/61E=field_37/38, 620=0xB4-37, 622=0xF-38), 若 0xB4-37>0 则 0x24\|=0x20, sub_801FA10(obj,1) |

## 对象行为表 (发现, 待逐个分析)

- `gUnk_0839CE7C[]`: 对象 kind → 行为函数表 (主角 0x71-0xB 段 + 0xFD 段)
- `gUnk_08393B28[]` / `gUnk_0839B2A4[]`: 精灵配置表 (field_0/4/8/A 四元组)
- `gUnk_0839CFAA[16]`: 出生点类别表 (MOD-03 Inv_FindFirstHeld 也引用)
- 0x03000690: 对象链表头 (field_8=首节点); 030006F8[]: 对象指针注册表; 03000714/718: 注册数/主角
- 0x03000638: `u8 *gUnk_03000638[12]` 战斗对象指针列表 (由 801D468 按槽号×0xC8 填充);
  0x03000668: `u8 gUnk_03000668` 列表条目数; 0x03000669: `u8 gUnk_03000669` 当前遍历下标
  (由 801D468 归零, 801BE34/801C484 状态机逐条消费)。三者 2026-09-05 匹配 sub_801D468 时登记

## 未匹配 (53 个)

801A3C4, 801A5EC, 801A6F4, 801A884, 801AD0C, 801B0B8, 801B964, 801BE34, 801C484,
801CA08, 801CBA4, 801CE80, 801CF90, 801D19C, 801D214, 801D378, 801D568, 801D710,
801DAA0, 801DB3C, 801DC20, 801DD04, 801DDB0, 801DEDC, 801DF90, 801E040, 801E1D8,
801E30C, 801E4D4, 801E690, 801E848, 801EA70, 801EC3C, 801ED40, 801EE6C, 801EEE4, 801F3FC,
801F76C, 801F884, 801FA10, 801FAB8, 801FF40, 80200E8, 8020228, 802031C, 8020648,
80207DC, 8020840, 80208A4, 8020B54。
(801D710=对象初始化, 801BE34/801C484=对象命令执行分支, 80207DC=生成初始化 — 重点)

## 调用图证据 (2026-08-31)

- 与 MOD-05 同一系统: 物理跨 code_801A3C4.c / code_8020D50.c 两个文件, 调用边密集
- 依赖 0x08044394-0x080446BC 的**全局状态服务** (804442C/8044514/804448C 被 801BE34/801C484/80177AC 调用;
  sub_804442C/8044514 = 全局状态复位/初始化, callers 78-84 个)
- 对象行为函数族实际分布在 0x08044C-0x0804F (804C2FC=对象生成, 804CEE0/804DD70=每帧分发,
  8045F10=动画检查, 80462E4=槽号收集) — 物理在 MOD-07 区但语义属本系统

## ObjHead 结构体 (0x30 字节公共头部, 2026-09-11 定名, 原 Unk_801A5EC)

| 偏移 | 类型 | 名 | 语义 |
|---|---|---|---|
| +0x00/04 | u32 | cmdBase0/1 | 命令流基址 (内含跳转表, Obj_FindJumpEntry 查询; 801B570 读 +0x04 首半字=流长) |
| +0x08/0C | u32 | jumpTable0/1 | u16 偏移表 (基址=cmdBase0/1) |
| +0x10 | u32 | scriptPtr | 脚本头: 半字[0]/[2] → 重算 cmdBase/jumpTable (sub_801A684) |
| +0x14 | u32 | palBitsPtr | 调色板装载指针 (→ sub_804C2FC) |
| +0x18 | u16 | kindFlags | bits0-3=kind; 0x200=跳过帧构建; 0x800=DMA 禁用只累计 tile; 0x8000=激活标志 (装载完清, kind9 除外) |
| +0x1C | u16 | frameIdx | 当前帧/跳转查找游标 |
| +0x20 | u16 | gfxTotal | 分步装载总数 (ObjGfxLoad_Step 上限) |
| +0x22 | u16 | gfxPos | 当前装载片号 |
| +0x24 | u16 | vramBank | case1 OBJ VRAM 槽 (<<5) |
| +0x26 | u16 | gfxBaseIdx | gUnk_087EBE00 索引基 (表实 1382 项, linker.ld 旧注[9]误) |
| +0x29 | u8 | palSlot | 调色板槽 (→ sub_804C2FC) |
| +0x1A/1E/28/2A-2F | — | f_* | 未验证 (重置/复制语义已见) |

**ObjGfxLoad_Step kind 目的地**: 1=OBJ VRAM 0x06010000+vramBank/pos, 2=0x020212C0, 3=0x02020E00,
4=0x0202B2C0 (case6 先清 0x0600C000 0x4000 落入), 5=0x020302C0, 7=0x0600C020 (先清 0x20B),
8=0x06008020 (先清 0x20B), 9=0x02037C28 (循环不清 0x8000, 常驻循环动画)。

## gUnk_03000248 = IWRAM 独立战斗对象实例 (2026-09-11 zcode 分析)

`0x03000248` 是**单个 0xC8 字节战斗对象**, 与池 0x02037028 的 12 个槽完全同布局。
判定证据 (三条独立):
1. **地址边界**: linker.ld 里 `0x03000248`(gUnk_03000248) 到下一符号 `0x03000310` 恰差 **0xC8**。
2. **同一初始化器**: BattleTask_Run case0 调 `sub_8020F4C(0x03000248)` —— 该函数按 `Unk_8020F4C`(0xC8 对象) 初始化
   (+0xBB/+0xBC/+0xB0/+0xBE/+0x36), 随后 BattleTask_Run 写 `+0x37=0xAF/+0x38=0x14` (对象滑动区间字段)。
3. **同字段访问**: `sub_802151C(0, ctx)` 里 `mov r0, ctx; movs r1,#2; bl sub_801FA10` —— sub_801FA10 把
   参数当 0xC8 对象 (`arg0[0xB0]` 低 4 位=kind, `sub_801B81C(arg0+0xC, …)`);
   `sub_802192C(pool, ctx, stateptr)` 对 ctx 也读 `+0xBE` (槽号); BattleTask_Run 尾部对 ctx 访问 `+0xB0/+0x38`
   并当 `UnkNode` 头 (key@+0, prev@+4, next@+8) 挂到行动链 0x03000318 (`ListNode_InitKey(ctx, 0)`)。

**结构体视图** (已加入 `include/code_0.h` 作 `BattleObj`):
```
+0x00 UnkNode node   (12B: key@0, prev@4, next@8; key 由 +0x38 值填充 → 行动链按位置排序)
+0x0C ObjHead headA  (0x30B: 主图形/脚本头; kindFlags @ +0x24 = A+0x18)
+0x3C ObjHead headB  (0x30B: 次图形/脚本头; kindFlags @ +0x54 = B+0x18)   ← 双头!
+0x6C .. +0xAF       坐标/移动/精灵区 (MOD-04 表: +0x6C/6E 坐标, +0xA2/A3 移动)
+0xB0 u16 state      (bits0-3=kind, bits4-7=子态 0x10/0x20/0x60, 0x400=不入行动链, 0x2000=跳跃)
+0xB2 步长 | +0xBB/BC/BD 辅助 (BD=sub_802103C 的 arg1)
+0xBE u8 slot        (槽号 ≤0xB; 0xFF=空)
+0xBF/C0 朝向/参数 | +0xC1..C7 事件值
```
**⭐ 双 ObjHead 发现 (2026-09-11 zcode, code.s 0x080180F8+)**: `sub_8018070` 的每帧对象循环
对**同一对象 r4** 先后调 `sub_801B8AC(r4+0xC, r4[0x39])` 和 `sub_801B8AC(r4+0x3C, r4[0x69])`,
并行读 `[r4,#0x24]`(headA.kindFlags) 与 `[r4+0x54]`(headB.kindFlags); `sub_801B81C` 也分别以
obj+0xC / obj+0x3C 装配两头 (`sub_8020A0C` 专装配 headB)。推测为「本体 + 叠加(武器/特效)」两套
脚本-图形通道, 各自独立 kind/帧游标。ObjHead 大小恰 0x30, 故 +0x0C/+0x3C 紧邻。
语义命名 `BattleObj` 需要人类确认前先只在文档/头文件注释使用; 本轮**不改任何已匹配函数的既有字段名**。

## 2026-09-11 claude-arg0: ObjHead 指针字段改型 + 脚本头布局实证

- `scriptPtr: const u16*` — 脚本头 (ROM gUnk_08393B28[idx].field_0):
  `u16[0]`=4 (cmdBase0 字节偏移, 恒定), `u16[1]`=cmdBase1 字节偏移 (0x14..0x224)。
- `cmdBase0/1: u16*` — 命令流首; `cmdBase1[0]` = jumpTable1 项数; jumpTable 紧随 +4。
- `jumpTable0/1: u16*` — u16 偏移表, **表值 = 字节偏移** (相对各自 cmdBase)。
- 命令条目 = u16 对 (值, 帧号), sub_801B8E8 以 4B 步进按帧号查找。
- `palBitsPtr: const u8*` — sub_804C2FC 的 DMA 调色板源。
- 消费链: sub_801A684 (重算四指针) → sub_801B570 (帧构建, jumpTable1 逆序) →
  sub_801B954 (cmdBase0+2 类型字节) / sub_801B95C (cmdBase1+2 u16)。

## 2026-09-12 nova: sub_8018070 匹配 → 双 ObjHead 分发实证 + 新符号 gUnk_03000344

- **双 ObjHead 分发规则已由真 C 确认** (此前只是 code.s 形态推测): 行动链 `gUnk_03000318` 上每个
  BattleObj, 每帧恒调 `sub_801B8AC(&obj->headA, obj->headA.f_2D)`;
  仅当 `obj->state & 0x2000` (跳跃) 且 `!(obj->headB.kindFlags & 0x800)` (headB 未禁用 DMA) 时,
  才追加 `sub_801B8AC(&obj->headB, obj->headB.f_2D)`。
  另: `headA/headB.kindFlags & 0xF` 为 6 或 7 时先刷 `DmaCopy32(3, 0x020362C0, 0x06007800, 0x800)`。
- **新 IWRAM 符号 `gUnk_03000344` (u8)**, 位于 gGstate340(0x340) 与 gDialogCtx(0x348) 之间:
  sub_8018070 入口写 0x7F 哨兵、出口写本帧分发返回值; 全 ROM 仅此处读写 (已在全部 asm 中确认)。
- `sub_801B8AC` 的第二实参是 `ObjHead.f_2D` (headA→obj[0x39], headB→obj[0x69]), 与既有字段表一致。
- 详见 `docs/progress.md §sub_8018070` (含「循环需 node+obj 双变量」「kind 短路链必须内联」两个
  GCC2 代码生成硬约束, 对同类链表遍历函数可直接复用)。
