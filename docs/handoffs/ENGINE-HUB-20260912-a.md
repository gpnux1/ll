# Handoff ENGINE-HUB-20260912-a

- 模式: SEMANTIC_ANALYSIS
- 状态: SEMANTIC-DONE
- 创建时间: 2026-09-12T12:10:00+08:00
- 最后更新: 2026-09-12T12:10:00+08:00
- 原 agent: zcode-sem
- 当前 agent: (未接手)
- 包类型: ENGINE-HUB (高扇入枢纽 + 直接调用子图 + 状态块)
- 函数地址: 0x0803F658 (hub); 关联 0x0803F5B4 (actor-list 构建), 0x0803F444 (actor handler 分派器)
- 当前工程名: sub_803F658 / sub_803F5B4 / sub_803F444 (仅检索标签)
- 当前 C 文件: src/event_hub.c (仅物理归属标签)
- 当前函数状态: status=0 (hub); sub_803F5B4 / sub_803F444 均 status=1 (语义模式保持不变)
- 入口类型: object-table (经 sub_803F444 分派到 3 张 handler 表, 表项 handler 每帧末尾 `bl` hub)
- 证据基线: main 分支 dirty (AGENTS.md/EXPERIENCE.md/progress.md/functions.tsv 及新增 docs 未提交); 本包只读 ROM/ASM/src + 新增 docs/handoffs 与 functions.tsv note, src 零改动

## A. 任务边界

- 要解决的机器问题: 零信任复核 sub_803F658 的 fan-in 来源、入口类型、控制流结构、直接调用参数、全局状态读写和跨帧消费者; 判断它是否真是调度枢纽以及其状态块边界。
- 不包含的函数或问题: 不匹配任何函数; 不修改 src/ll.cfg/linker.ld/iwram.h; 不分析 3 张 handler 表内 101 个 handler 的各自语义 (只作为 hub 的调用者分组证据)。
- 依赖的地址工作包: 无强依赖。子图相关: sub_803F5B4 (0x0803F5B4)、sub_803F444 (0x0803F444)、以及 0x03000800 区状态块。
- 工作区/锁: 工作前 hub 未被占用 (`.claims/` 无 sub_803F658.lock); 已按 `DECOMP_AGENT=zcode-sem scripts/claim.sh sub_803F658` 认领; 交付后释放。

## B. E0/E1 原始事实

- ROM 地址和目标字节范围: hub 0x0803F658..0x0803FF53 (asm_lines=1127, ts=1127); 本包只读, 未 hexdump 逐字节。
- ASM 来源和重新生成命令:
  - 切片: `asm/nonmatchings/sub_803F658.s` (1132 行, 由 `python3 scripts/gen_asm.py` 生成)
  - 反汇编: `tools/gbadisasm/gbadisasm baserom.gba -c ll.cfg > code.s`
  - ll.cfg: `thumb_func 0x803f658 sub_803F658`
- 直接调用者: **101 个** (code.s 中 `bl sub_803F658` 计数 = 101)。全部形如 `adds r0, <reg>, #0; bl sub_803F658`, 即 hub 签名 = `void sub_803F658(u8 *obj)`, 每帧每个 actor 调用一次。
  - caller 地址集合 (E1, 完整): 0x080257D8, 0x08025994, 0x08025DA8, 0x080260BC, 0x080264C0, 0x0802698C, 0x08026F88, 0x0802723C, 0x0802761C, 0x08027760, 0x08027A20, 0x08027D9C, 0x08028098, 0x080282EC, 0x080285A0, 0x080287EC, 0x08028AD8, 0x08029268, 0x08029510, 0x08029784, 0x080299C8, 0x08029BF8, 0x0802A154, 0x0802A86C, 0x0802ADC4, 0x0802B0F0, 0x0802B608, 0x0802B8BC, 0x0802BB24, 0x0802BD94, 0x0802C0EC, 0x0802C490, 0x0802C714, 0x0802C9E8, 0x0802CE90, 0x0802D1FC, 0x0802D454, 0x0802D728, 0x0802DA78, 0x0802DE04, 0x0802DFDC, 0x0802E234, 0x0802E49C, 0x0802E6C8, 0x0802EAC4, 0x0802EDD8, 0x0802F100, 0x0802F480, 0x0802F6D8, 0x0802F9EC, 0x0802FE98, 0x0803029C, 0x08030664, 0x080309B0, 0x08032548, 0x0803272C, 0x08032948, 0x08032EA0, 0x080334B8, 0x08033988, 0x08033E2C, 0x08034F00, 0x08035130, 0x08035360, 0x0803586C, 0x08035B04, 0x08035D9C, 0x08036034, 0x080362CC, 0x08036564, 0x080368FC, 0x08036B30, 0x08037078, 0x08037388, 0x08037868, 0x08038C84, 0x08038E44, 0x08039024, 0x080393E0, 0x08039724, 0x08039C38, 0x0803A8D0, 0x0803BBEC, 0x0803C328, 0x0803D60C, 0x0803DECC, 0x08040690, 0x08040EE8, 0x08041308, 0x080416F0, 0x080419E0, 0x08041EDC, 0x080422B8, 0x08042784, 0x08042B90, 0x08042E70, 0x08043554, 0x08043938, 0x08043B5C, 0x08043F90, 0x0804E2AC
- 间接表地址、槽位和目标 (E0/E1):
  - **0x0839CD5C**: 89 个 thumb 指针 (`addr|1`), 其中 54 个是 hub caller。
  - **0x0839CEC4**: 14 个指针, 其中 12 个是 hub caller。
  - **0x0839D4CC**: 60 个指针, 其中 33 个是 hub caller。
  - 三表合计覆盖 101 个 hub caller 中的 **98 个** (缺 0x08040EE8, 0x08041308, 0x0804E2AC, 三者可能属于另一分派器/表)。
  - 消费这些表的函数 = **sub_803F444 (0x0803F444)**: 读 obj+0xbe (u8, 行为类别, 0..0x70+), 按 `(u8)obj+0xbc` 的值走 0/1 分支, 分别索引 0x0839CD5C / 0x0839CEC4 / 0x0839D4CC 并 `bl _call_via_r2` 调用表项。即 sub_803F444 = actor handler 分派器; 表项 handler 再 `bl sub_803F658`。sub_803F444 的调用者 = 0x0801BE34, 0x0801C484 (各 1 次)。
  - 主表的索引计算 (E1): `id = obj+0xa1 (u8); if (id <= 7) id = obj+0x99[id]; handler = table[id]` (0x0839CD5C 分支); 另一分支读 `*(u32*)(obj+0x88) + 0x1c / 0x1e` (u16 字段) 作为 id 索引 0x0839CEC4。
- 直接被调用者 (hub 的 14 个 callee, 含次数; 括号内为 E1 参数形状):
  - `GetObjPool()` → 0x02037028 (对象池基址; 1x)
  - `sub_801DB3C(pool_slot, 0, u16 *(u16*)(obj+0xb6))` (3x)
  - `sub_801A684(pool_slot+0x3c)` (1x)
  - `sub_80208A4(pool_slot)` (1x)
  - `sub_80207DC(pool_slot, u8, u8, u16, u8[stack])` (1x; 栈参 = `*(u8*)(obj+0xbf+1)`?)
  - `sub_801B954(pool_slot+0x3c)` → u8 (2x)
  - `sub_804C3A4(u8, u8)` (2x)
  - `sub_801768C(u8, s16 delta, 3, u8, s16[stack] step)` (2x; step 取 ±5 / 0xFFFB, r1=delta)
  - `Sfx_Play(*(u16*)0x03000886, *(u8*)0x03000888, 0)` 与 `Sfx_Play(0x1d, 3, 0)` (2x)
  - `Sfx_TrackBusy(0)` (1x), `Sfx_StopTrack(0)` (1x)
  - `sub_801ED40(pool_slot, 4)` (1x), `sub_801EE6C(pool_slot)` (1x)
  - `sub_80471AC()` → u8 (2x, 返回值 `&0xFF` 后判 0)
- 全局/结构体偏移、宽度、读写方向 (hub 内, 全部为池常量 `ldr =0x...`, E1):
  - `0x03000844` u8: **RW** — hub 主相位 (case 选择 + 回写); 9 case (0..8), 越界 (>8) 走 no-op。
  - `0x03000845` u8: **R** — 次相位 (0..3) 的 switch; hub 只读。
  - `0x03000856` u8: **R** — 第三相位 (0..3) 的 switch; hub 只读。
  - `0x0300083D` u8: **R** — actor 列表条目数 (循环上界)。
  - `0x03000840` u8*: **R** — 指向每 actor 一个字节的数组 (class/id 字节)。
  - `0x03000858` u8: **R** — 活跃计数 (与 0x0300085C 比较判完成)。
  - `0x03000855` u8: **R** — 当前选中 actor 的索引 (strb 于 case 1)。
  - `0x03000854` u8: **RW** — 进度 0..3 (case 后段自增/回绕)。
  - `0x03000848` u8[]: **R** — 每 actor 一个字节 (0x03000848+i), 传给 sub_801768C。
  - `0x03000857` u8: **R** — 计数器; `0x0300085A` u16(s16): **R** — 目标值 (比较/自增)。
  - `0x0300085C` u8: **RW** — 计数器 (case 后段自增)。
  - `0x03000869` u8: **R** — actor 扫描索引 (strb 自增)。
  - `0x03000825` u8: **R** — 共享每帧计数器 (obj+0xb4 比较)。
  - `0x03000886` u16: **R** — 音效 id; `0x03000888` u8: **R** — 音效参数 (传给 Sfx_Play)。
  - 对象池 (GetObjPool=0x02037028), **stride = 0xC8 (200 字节)**; `slot = 列表字节 & 0x0F`; 列表字节高半字节 `0x10` = "actor 存在", `0x00` = 空槽。
  - hub 内对象偏移 (obj = 传入指针): +0x24 u16(bit 0x800), +0x88 u8*(指针), +0xb0 u16(bit 0x1000/0x2000), +0xb4 u16, +0xb6 u16, +0xbc u8, +0xbe u8, +0xbf u8, +0x54 u16(bit 0x100/0x800), +0x65 u8, +0x3c, +0x2a u16, +0xb2 u16 (0x0803F5B4 写)。
  - 掩码常量 (E1): 0x0000F7FF=~0x0800, 0x0000FEFF=~0x0100, 0x0000EFFF=~0x1000, 0x0000DFFF=~0x2000, 0x0000FFFB=s16 -5。
- 跨帧生产者和消费者:
  - **生产者**: sub_803F5B4 (0x0803F5B4) 写入 0x0300083D(条目数), 0x03000840(列表指针), 0x03000858(活跃数); 它读取 `sub_8020DF0/sub_8020E5C/sub_8020E54` 的返回值。sub_803F5B4 自身被 98 个函数调用 (与 hub caller 高度重叠)。
  - **消费者**: hub 每帧读上述字段; 0x03000844 由 hub 自写、被 4 个函数直接 W (0x0804442C/0x08044514/0x08044574 区) 与 14 个函数 RW。
  - 0x03000825 被 **130+ 个函数** RW (全 actor handler 共享的每帧 tick 计数)。
- 状态块非 hub 侧访问者 (E1 全 ROM 扫描; 用于确认字段有多个独立消费者):
  - 0x03000845: 被 78 个函数 R (与 hub caller 重叠) + 14 个 RW + 3 个 W。
  - 0x03000856: 被 76 个函数 R + 16 个 RW + 3 个 W。
  - 0x03000857 / 0x0300085A: 各被 16 个函数 W (集中在 0x08040690..0x08044574 区)。
  - 0x0300085C: 被 17 个函数 W; 0x03000886/0x03000888 各被 16 个函数 W。

## C. 脚本上下文（不适用写 none）

- called_by_opcode: none — hub 由 actor handler 表项直接 `bl` 调用, 是引擎每帧服务, 非 opcode handler (未发现 opcode 表引用 hub 地址; E1)。
- handler 地址和原始表项: 不适用 (hub 不是 opcode handler; 其 caller 是 actor handler)。
- PC delta: none
- return 0: hub 返回 void (`pop {r0}; bx r0` 恢复 r0, 无显式返回值); 无布尔返回。
- return 1: none
- VM/跨帧状态写入: none (写的是 IWRAM 0x030008xx 状态块, 非脚本 VM 标志)
- 下一帧消费者: hub 自身 (下一帧 `bl`), 以及上述 0x03000844/45/56 的其他读写者。
- handler -> service 调用边: actor handler (表项, 0x0839CD5C/CEC4/D4CC) -> `bl sub_803F658` (hub)。分派器 sub_803F444 -> `bl _call_via_r2` (表项)。

## D. 语义模型

- 已确定（E0/E1/E2）:
  - E2: hub 的调用者集合 (101) 与 sub_803F5B4 的调用者集合 (98) 高度重叠, 且 98/101 出现在 3 张 ROM 函数指针表中; 说明这 101 个函数是同一批 "actor 处理器", 每帧先/后调用 actor-list 构建与 hub。hub 是**共享的每帧 actor 动作/效果收尾服务**, 不是被单一模块内部调用的小工具。
  - E2: hub 同时被已匹配 caller (如 sub_8044514/sub_8044574/sub_8040690) 与未匹配 caller (如 sub_80257D8) 调用, 签名统一为 `(u8 *obj)`; caller 侧 `adds r0,<reg>,#0` 传参, 返回值不被使用 (caller 随后返回自己的局部)。
  - E1: hub 内顺序执行 3 个 switch 状态机: phase A = 0x03000844 (case 0..8, hub 自写回), phase B = 0x03000845 (case 0..3, hub 只读), phase C = 0x03000856 (case 0..3, hub 只读)。phase A 是主"动作阶段"推进器。
  - E1: actor 迭代协议 = `for (i=0; i<0x0300083D; i++) { b=*(0x03000840+i); if ((b&0xF0)==0x10) { slot=b&0x0F; obj=pool+slot*0xC8; ... } }`; `0x03000840` 是指向 u8 数组的指针 (非数组本身, `ldr r0,[r0]` 后 `ldrb`), 数组与 hub 同处 IWRAM。
  - E1: case 1 在 `0x03000858==0` 时把 0x03000844 清 0 并跳到 epilogue (无 actor 则不推进) — 0x03000858 是活跃 actor 数。
  - E1: hub 读对象字段 +0x70 之外的 (0x03000825 vs obj+0xb4) 比较用于"计时到点"判断, 与 caller sub_80257D8 中 `obj+0xb4 - 5/-10` 的用法一致 (E2)。
  - E1: 无内存写越出 0x030008xx 状态块与对象池; 所有访存宽度为 u8/u16/s16; 无跨帧堆分配。
- 工作假设（必须标明未验证）:
  - hub 的 phase A 9 个 case 对应"actor 动作序列"的 9 个阶段 (如 开始→选目标→执行→收尾), 但**每个 case 的业务语义未逐条证实**; 当前只记录机器事实 (读写了哪些字段、调了哪些服务、写了哪个下一相位)。
  - 0x03000844/45/56 三个相位可能属于同一个"动作序列控制器"的三个并行维度, 也可能是三层嵌套; 需结合 caller (如 sub_80257D8 的 0x03000820 子状态) 才能定层。
  - sub_801768C 的栈参 (±5 / 0xFFFB) 是"每帧位移步长"; 未由 E3 运行时确认。
  - "0x03000840 指向 u8 数组" 与 "actor 列表" 的对应关系来自 `obj+0xB4`/`0x03000825` 与 sub_803F5B4 写入 `obj+0xb2` 的 E2 一致; 数组元素的确切语义 (类别字节 0x1X 的 X 是否恒等于对象池 slot) 未在 ROM 数据层确认。
- 与旧名称/旧分析的冲突:
  - TSV `module` 列标为 `event_hub`, iwram.h 用 `gUnk_03000844` 等中性名 — 与本包结论不冲突; 但 "event_hub" 是物理归属标签, 不是功能边界 (本包未按它命名)。
  - 未发现旧 docs 对 sub_803F658 的语义结论 (TSV note 为空)。本包结论均独立来自 code.s + ROM (E0/E1)。
- 尚未决定的问题:
  - 缺 0x08040EE8 / 0x08041308 / 0x0804E2AC 三个 caller 的表归属 (可能在第四张表)。
  - hub 三个相位的精确进入/退出条件 (哪些 caller 在什么条件下把 0x03000844 设为 1)。
  - 0x03000844 的 3 个直接 W 写者 (0x0804442C/0x08044514/0x08044574 区) 是否就是"发起动作"的入口。

## E. 语义分析结果

- 函数卡 (hub, 0x0803F658):

```text
address: 0x0803F658
current_name: sub_803F658 (仅检索标签)
physical_tu: src/event_hub.c (仅物理归属)
match_status: 0 (未匹配, 保持不变)
entry_kind: object-table (经 sub_803F444 分派到 handler 表, 表项 handler 每帧 bl hub)
callers: 101 个地址 (见 B 节; 98 个位于 0x0839CD5C/0x0839CEC4/0x0839D4CC)
callees: GetObjPool, sub_801DB3C(3x), sub_801A684, sub_80208A4, sub_80207DC,
         sub_801B954(2x), sub_804C3A4(2x), sub_801768C(2x), Sfx_Play(2x),
         Sfx_TrackBusy, Sfx_StopTrack, sub_801ED40, sub_801EE6C, sub_80471AC(2x)
indirect_tables: hub 自身不在表中; 其 callers 在 0x0839CD5C(89)/0x0839CEC4(14)/0x0839D4CC(60),
                 表消费者 = sub_803F444 (0x0803F444)
args: r0 = u8 *obj (actor 对象指针); 无其他入参 (返回后 caller 不消费 r0)
return: 无显式返回值 (void); epilogue 恢复 r0
memory_reads: 全局 0x03000825/3D/40(*)/44/45/48[]/54/55/56/57/58/5A/5C/69/86/88;
              obj+0x24,+0x2a,+0x54,+0x65,+0x88,*+0xb0,+0xb4,+0xb6,+0xbc,+0xbe,+0xbf; pool+slot*0xC8
memory_writes: 0x03000844 (相位), 0x03000845? 仅 epilogue 之外未见; 0x03000848/54/55/57/5C/69 的自增;
               obj+0x54/0xb0 的位清置 (掩码 0xF7FF/FEFF/EFFF/DFFF); 0x03000886/88 未写
persistent_state: 0x03000844 = 动作相位 (0..8); 0x03000845/0x03000856 = 次/三相位 (0..3);
                  0x03000858 = 活跃 actor 数; 0x03000854 = 子进度; 0x0300085C = 计数器
state_transitions: phase A 各 case 把 0x03000844 写为 0/2/3/4/5/6/7/8;
                   phase B → 0x03000845 值 0..3; phase C → 0x03000856 值 0..3
script_relation: none (引擎每帧服务)
semantic_model: 每帧 actor 列表扫描型"动作/效果"状态机收尾服务; 3 个顺序相位机
uncertainty: 各 case 业务语义、相位分层、缺失 3 caller 的表归属
```

- 调用子图 (以地址为节点):
  - 上游: `sub_801BE34(0x0801BE34), sub_801C484(0x0801C484)` → `sub_803F444(0x0803F444)` (分派器, handler 表消费者) → handler 表项 (0x0839CD5C/CEC4/D4CC) → 101 个 actor handler → `sub_803F658` (hub)。
  - 同批 handler → `sub_803F5B4(0x0803F5B4)` (actor-list 构建) → `sub_8020DF0 / sub_8020E5C / sub_8020E54`。
  - hub → `GetObjPool(0x08018864)` 取对象池 `0x02037028`。
  - hub → 对象/音效服务: sub_801DB3C, sub_801A684, sub_80208A4, sub_80207DC, sub_801B954, sub_804C3A4, sub_801768C, Sfx_Play, Sfx_TrackBusy, Sfx_StopTrack, sub_801ED40, sub_801EE6C, sub_80471AC。
- 状态块和生命周期:
  - 初始化/构建: sub_803F5B4 从对象池选择 class 0x1X 的 actor, 写 0x0300083D(数)、0x03000840(指针)、0x03000858(活跃数), 并累加 `obj+0xb2`。
  - 每帧更新: 每个 actor handler 末尾 `bl sub_803F658`; hub 按 0x03000844/45/56 推进相位, 迭代 0x0300083D 个 actor。
  - 收尾/清理: case 1 在 0x03000858==0 时 0x03000844=0 (无活跃 actor 时停机); phase B/C 在 progress>3 时回绕并切下一相位。
  - 跨帧消费者: hub 自身 + 16 个函数写 0x03000857/5A/5C/86/88 (疑似同一套动作序列的其它入口)。
- 变量/全局/结构体证据:
  - 对象池 stride 0xC8 (E1: `muls ...,#0xC8` 多处)。
  - 列表字节 = `(class<<4)|slot`, class 0x1 = 存在, slot 低 4 位索引对象池 (E1: `(b&0xF0)==0x10`, `b&0x0F` + `*0xC8`)。
  - 0x03000840 是**指针** (E1: `ldr r0,[r0]` 后 `ldrb`), 不是内联数组。
- 对已匹配函数的语义审计: 本包未修改/未重命名任何已匹配函数; 但确认 sub_803F5B4 与 sub_803F444 是 hub 子图的直接依赖 (若未来改名需保留其调用点证据)。
- 对未匹配函数的机器契约: hub 的签名 `void sub_803F658(u8 *obj)` 与 101 个调用点一致 (E2); 无返回值消费 (caller 不读 r0)。
- 下一条分析命令:
  ```bash
  sed -n '/thumb_func_start sub_803F658/,/thumb_func_start sub_803F5B4/p' code.s | head -40
  ```
  然后核对第四个 caller (0x0804E2AC) 的调用与表归属:
  ```bash
  rg -n 'sub_804E2AC|sub_804E76C' code.s
  ```

## F. 匹配结果（仅 MODE=MATCH）

- 全部字段: not applicable (MODE=SEMANTIC_ANALYSIS)。
- 未生成候选、未运行 bytecmp/fndiff/permuter、未改 src/status。

## G. 已尝试且排除（按 mode）

- 本包为语义模式, 无匹配实验。
- 排除的语义假设: "hub 是 opcode handler" (E1 无 opcode 表引用 hub, 且 caller 是 actor handler); "0x03000840 是内联 u8 数组" (E1 证否, 实为指针)。

## H. 共享影响

- 计划修改的 src 地址区段: 无 (语义模式不修改代码)。
- code_0.h 原型和全部调用点: 未改。若后继者匹配 hub, 原型应为 `void sub_803F658(u8 *);` 并需记录 101 个调用点 (均在 code.s 中 `bl sub_803F658`)。当前 include/code_0.h 是否已有该原型未在本包核改 (仅只读确认无冲突)。
- iwram.h/linker.ld 符号: 未改。相关符号 `gUnk_03000825/3D/40/44/45/56/57/58/5A/5C` 已在 iwram.h/linker.ld 登记 (历史标签)。0x03000848 / 0x03000869 / 0x03000886 / 0x03000888 在 link 脚本中未见对应符号 (hub 以裸常量引用), 若后继者要命名需按地址序插入。
- 受影响的已匹配函数: 无 (只读分析)。
- 是否需要强制重编: 否 (src 零改动)。
- 语义模式声明: 不修改代码和 status。

## I. 最后验证

- 语义模式：函数卡/调用图审查完成时间: 2026-09-12 ~12:10。
- fncheck（MATCH only）: not applicable。
- bytecmp（MATCH only）: not applicable。
- make（MATCH only 或明确外部要求）: 未运行 (语义模式不构建)。
- sha1sum -c ll.sha1（MATCH only 或明确外部要求）: 未运行。
- audit: 未运行。
- 验证时间和完整命令:
  - `python3 .scratch/zcode-sem/callgraph.py` → 1250 funcs / 3878 direct edges; hub fanin=101 (第 3 高, 最高未匹配)。
  - `grep -c 'bl sub_803F658' code.s` → 101。
  - `python3 .scratch/zcode-sem/dumptable2.py` / `tablemap.py` → 3 张表 0x0839CD5C/0x0839CEC4/0x0839D4CC 覆盖 98/101 caller。
  - `python3 .scratch/zcode-sem/ga2.py 0x03000844 ...` → 状态块读写者清单。

## J. 恢复记录

（后继者只追加）

- 2026-09-12 zcode-sem: 本包创建。hub fanin=101 为最高未匹配 (全局第 3, 前两名 Sfx_Play/GetObjPool 已匹配); 入口 = actor handler 表项每帧 bl; 结论已留函数卡。下一步建议: 分析 3 个直接写 0x03000844 的函数 (0x0804442C/0x08044514/0x08044574) 与 0x0804E2AC, 定位"动作发起"入口。
