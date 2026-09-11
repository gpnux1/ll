# MOD-07 对象行为 + 战斗逻辑 (0x08044394-0x0804F0B8)

> 分析人: plan (2026-08-31, 骨架 — 源码 1606 行尚未逐函数通读)。
> 226 函数: 106✅ / 120❌。

## 2026-08-31 调用图修正

**头 8 个函数 (0x08044394-0x080446BC) 重新归属为 F5 对象系统状态服务** (见 FUNCTIONAL_MAP 修正):

| 地址 | 状态 | 语义名 | 证据 |
|---|---|---|---|
| 0x08044394 | ✅C | `ObjStats_Reset` | 被 8042784 (MOD-05/06 区) 调 |
| 0x08044414 | ✅C | (状态服务) | 被 801E040 (对象系统) 调 |
| 0x08044420 | ✅C | (状态服务) | 被 801BE34/801C484 (对象命令执行) 调 |
| 0x0804442C | ✅C | `GlobalState_Reset` (progress.md 已有) | callers=3 含对象命令执行 |
| 0x08044514 | ✅C | `GlobalState_Init` (progress.md 已有) | **callers=78** (0x08025xxx 对象生成器) |
| 0x0804473C | ✅C | (状态服务) | 被 MOD-06 hub sub_803F5B4 调 |
| 0x080448A8 | ✅C | 技能恢复量分发 (2026-09-10 zcode) | (s8)obj[0xBC] 选 stats[0x2F] 或 stats[0x30+obj[0xC2]], switch 0..54 → sub_8044A40 +0xF/+0x1E 档位, 写 gUnk_03000908 (新登记); 见 progress.md 2026-09-10 |

## 已确证 (progress.md 既有记录)

- 804C2FC = 对象/精灵生成 (被 801A684/801DAA0/8020F08 调) → 属对象系统
- 804C4D8 = 对象 RMW 位标志 (iso 家族)
- 804CEE0 / 804DD70 = 对象每帧分发双路 (8020C58 按 gUnk_03000324 bit5 选择)
- 8045F10 = 对象动画状态检查 (8020A7C 循环调)
- 80462E4 = 对象槽号收集 (8020B04 调)
- 80489E8 = 事件扫描 (8020AB0/801D468 调)
- 8048A88 = 快排, 8048ACC = 排序入口 (801D468 调 — 对象排序!)
- 804DD90 = 通道检查 (804F0B8/8045EB8 调)
- gPartyMemberIds/gBattleFormationIds/gInventory 等队伍数据在此区读写 (804F768 脚本 opcode 委托)

## ⭐ 战斗主循环证据 (2026-08-31 用户确认 + 调用图)

`gMainTasks[] = {Task_DispatchGameState, sub_80177AC}` (data_87E83F0.c) — **sub_80177AC = BattleTask_Run**。
战斗进入路径: gUnk_03001AC0=1 (Scene_ExitToMenu/sub_8001828 设置) → AgbMain 派发。

BattleTask_Run 调用 46 函数, 按段分布:
- 0x08017-0x08018 (MOD-03 文件): 战斗 UI 命令族 (8017FA4/80184A8/801869C/80188BC/8018A58) + 有序链表 (8018800/801880C/8018818)
- 0x08020 (MOD-05): 对象系统 (80207B4=对象命令分发/8020F08=特效生成/8020F4C=对象初始化/802103C/8021064=tile动画/802151C/8021700=对象生成步进/802192C)
- 0x08044-0x08049 (本模块): **战斗逻辑** (804442C/804448C=状态服务, 80457AC=伤害/属性计算(调 800A048+8018864), 8048DA4=对话窗口战斗文本, 8048FB8=对象检查, 8049C1C=回合逻辑(调 801A884=对象命令!), 8049DF8=407行 大型战斗流程(调 801FA10/8020FB8/8050434))
- 0x0804A-E: 战斗演出/生成 (804A148/804A368=对象生成, 804AD60=对象命令, 804ADE0/ADF8, 804B288, 804DE20, 804EEC4)
- 801FF40 (MOD-04): 随机事件 (Rng_LcgNext + 80489E8 事件扫描)

→ **战斗 = 战斗任务循环 + 对象 VM 执行战斗脚本**。0x080444xx 状态服务为两者共享 (维持 F5 归属)。

## 剩余推断 (待源码通读确认)

0x080447xx-0x0804Axx = 对象行为/生成参数; 0x0804Bxx-0x0804Cxx = 对象精灵/生成;
0x0804Dxx-0x0804F0B8 = 战斗数据+脚本辅助 (gPartyMemberIds 族 + 804DD90 通道)。
**原"战斗系统"的推断弱化**: 未发现战斗主循环; 本区更可能是"对象行为+游戏数据管理"。

## 0x0804AE2C OAM 预扫描区 (2026-09-06, franklin, ✅)

`sub_804AE2C` (0x0804AE2C, 已匹配) + 姐妹 `sub_804AF60`/`sub_804B080` 组成
"战斗演出 OAM 槽 预扫描/回写" 状态机, 控制块在 0x03000AD8-0x03000ADE:

| 地址 | 语义 | 说明 |
|---|---|---|
| 0x030009D0 | 场景结构指针 | 演出系统帧状态 |
| 0x030009D8[128] | u16 槽 HPos 载出缓冲 | sub_804AE2C 写入, sub_804AF60 回写 OAM CharNo |
| 0x03000AD8 | u8 帧计数器 | 每帧 (+1)%5, 到 0 时 ADD++ |
| 0x03000AD9 | u8 槽区间高界 | 取自 *(s+0x2D) |
| 0x03000ADA | u8 槽区间低界 | *(s+0x2D) - *(s+0x2E) |
| 0x03000ADB | u8 扫描最小 VPos | 初 0xA0, 取 min |
| 0x03000ADC | u8 扫描最大 VPos | 初 0, 取 max(表[Size+(Shape<<2)]*8+VPos) |
| 0x03000ADD | u8 超帧计数器 | keep-alive 检查 (ADC-ADD) < (ADB-0x1E) |
| 0x03000ADE | u16 使能位 | bit0=使能 bit1=本轮已扫描; init=0x11, 清~3 关 |

数据表 0x08393A24 = 4x4 [Shape][Size] 尺寸表 (每槽 VPos 偏移预测), 与
sub_801A884 (0x0801A884, 未匹配) 同表同索引算法 `(b3>>6)+((b1>>6)<<2)`。
匹配要点见 progress.md sub_804AE2C 条目 (OAM 缓冲强转常量 / bitfield 访问 / 整式内联)。

## 2026-09-10 battle_rewards.c 掉落判定族全匹配 (8 函数)

`src/battle_rewards.c` D4xx-DExx 的 8 个同类函数全部字节级匹配 (permuter 5 分 = 池计分, fncheck 全 OK):

| 地址 | 语义 | 关键 idiom |
|---|---|---|
| 0x0804D4FC | 单掉落 (阈值 count*10) | flag 三件套 (EXPERIENCE 211) |
| 0x0804D5B4 | 团队战奖励 (count==2 扫队友 0xC8 步长表) | (count=1) 载体式 (EXPERIENCE 213); 中途 obj[0xBC]=3 → 第一 switch 落 default 读未初始化 sb = ROM 真实 UB |
| 0x0804D840 | 单掉落 (阈值 <=0x45) | flag 三件套原型 (EXPERIENCE 211) |
| 0x0804D8F4 | 幸运掉落 (金钱 < 售价/10*4 → 必掉) | gold 基址=obj+0x6E; lucky(victory) 走 case1 else 分支 |
| 0x0804DA04 | 单掉落 (D1B4 家族 push{r4,r5,r6}) | (s8)obj[0xBC] 数组读归一化 + kind 局部 |
| 0x0804DB64 | 双色掉落 (kind=Rng%5, ==2 归 0) | kind 独立局部 + 内存读比较 |
| 0x0804DC24 | D840 常量变体 (0x64/0x3B) | 三件套原样套用 |
| 0x0804DCD8 | 单掉落 (强制 lucky) | break 后 while(value)break; 调度屏障 (EXPERIENCE 212) |

家族共性: count=sub_80489E8(arg1, values, slot?, 0x6F) 收集掉落候选 → rng%0x65 判定 obj[0xBC]
(0=普通表/1=特殊表) → case 索引 obj[0xC2] 进 gUnk_08393B28_entries[].field_8[kind] → 尾 switch
entry->field_10 决定 obj[0xBD] = values[Rng%count] 或 0。entry 结构 20B (pad_0[0x10] + field_10 u16)。

## 0x0804473C 语义补注 (2026-09-10 wb, 已匹配 permuter score=0)

状态服务总入口, hub `sub_803F5B4` (MOD-06) 遍历物件池逐项调用, 参数2 = `pool + (slot_id & 0xF) * 0xC8` (0xC8 字节池 entry):

| 条件 | 去向 |
|---|---|
| `arg0[0xBE] > 0xA` | `sub_8044A40` (兜底服务) |
| `(s8)arg0[0xBC] == 0` | `sub_8044A40` |
| `(s8)arg0[0xBC] == 1` | 按 `sub_8048764(arg0)` (0..59) 细分: `0,1,2,38,39,40,42,48,53,55,58,59`→A40 / `17-20`→返回0 / 其余→`sub_8044F4C` |
| `(s8)arg0[0xBC] == 2` | `sub_8045098` |
| 其余 (负/ >2) | 返回未初始化 result (switch 无 default) |

全部经共享 `u16 result` 汇聚, 末尾一次 `return result;`。`arg0[0xBC]` = 服务类型, `arg0[0xBE]` = 服务上限/等级阈值。

## 0x0804473C / 0x08044F4C 语义补注 (2026-09-11 wb, 均已匹配 permuter score=0)

`sub_804473C` (状态服务分派) 按 `(s8)arg0[0xBC]` 三分派, 把 `sub_8044F4C` (伤害结算) 当三分支之一调用。

`sub_8044F4C(arg0=攻方, arg1=守方 pool entry)` 的字段语义:
- `arg1+0xB0` (u16) = 守方状态标志位: `0x1000` = 已结算/免疫(清位后 return 0), `0x10` = 走另一返回路径;
  `arg1+0xB2` (u16) = 结算输出槽 (被 `sub_803F5B4` 再累加 `+0xB2` 处的值)。
- `arg0+0xB0` (u16) = 攻方标志位, `0x4000` = 暴击/翻倍 (用后清位)。
- `arg0[0xAA]` = 技能倍率; `sub_8047024(obj, 8 / 9)` = 攻/守方某项数值 (8=攻, 9=守);
  `sub_80472E8(arg0, (u8)sub_8048764(arg0), 0)` = 技能威力; `sub_8047DC8/7D28` = 属性克制查询 (返回 0/1/2)。

## BattleTask_Run 21-case 状态机全解 (2026-09-11, zcode-main, ⏸ 未匹配)

`gUnk_03000240` = 战斗主循环状态 (0..0x15), 每帧派发 (见 progress.md §BattleTask_Run):
| 状态 | 语义 | 次态 |
|---|---|---|
| 0 | 初始化: DMA3 清对象池 0x02037028(0xC00B) → gObjPoolPtr; Sound_VSyncOn; sub_8020F4C(ctx 0x03000248)+ctx[0x37]=0xAF/0x38=0x14; gGstate330[0..5]=-1; 扫 gUnk_080936A0 数 0x128 个 0xFF 存 gGstate340 | 1 |
| 1 | 等 FlashFlag&0x4000 → BattleFx_DispOff; 或 gGstate324&8 清对象 +0x24 的 0x200 位 | 2 |
| 2 | gRandCursor→gBattleRngSeed; sub_802151C 判定 | 3 / 5 |
| 3 | sub_8021700()==1 | 4 |
| 4 | 对话: DialogCtx.field_C==6/7 忽略, 否则 sub_802192C→SetHead; 无对话 Disp_Bg1Off | 5 / 0x14 |
| 5 | sub_80207B4 分派 0..4; 末尾按 gGstate324&0x4000 清位 (gGstate314&0xF000==0x2000) | 0xD/6/8/0x13 |
| 6→7 | sub_804AD60 | 7 |
| 7 | sub_8049C1C (回合逻辑) | 0x11 |
| 8 | sub_804ADF8; gGstate32E==0x3C | 0x13 / 9 |
| 9 | 等 gUnk_0300032C>0x31 (u16 计数) | 0xA |
| 10 | sub_8049DF8 (大型战斗流程) 返回 1→重置战斗, 2→System_ResetToLogo+回标题 | 0 |
| 13→14 | sub_804A148; sub_804A368==1 | 0xE→0xF |
| 15 | sub_801FF40 取槽 (gGstate324&2 或 &4) → sub_802103C 应用 | 0x10 / 2 / 0x13 |
| 16 | ctx+0xB0 & 0xF0 ∈{0x40,0x50} | 2 / 0x13 |
| 17→18 | sub_8048DA4; sub_8048FB8 | 0x12→0x13 |
| 19 | 等 0x32 帧 → Bgm_FadeOut(0x14)+sub_8019AD0(0xA,0x110)+sub_80457AC | 0x15 |
| 20 | sub_80401AC()==1 → 按 gGstate324&0x1000 | 5 / 0x13 |
| 21 | 收尾: Sound_GetFlags&4==0 → REG_DISPCNT|=0x80, gGameState=6, gMainLoopMode=0, gVBlankPipelineMode=0, Bgm_Stop, Sound_VSyncOff | 0 |
尾部(所有 case 共用): 遍历 12 个 0xC8 对象建行动链 0x03000318 (ListNode_InitKey/InsertSorted, key=obj[0x38]);
`!(*ctx+0xB0 & 0x400)` 时 ctx 也挂链; state=sub_801D984(state); sub_80184A8(链头.next, state)。

**新增 RAM 符号**: `gObjPoolPtr` @0x03000244 (u8* 对象池指针), `gUnk_03000317` @0x03000317 (u8),
`gUnk_0300032C` @0x0300032C (u16 状态等待计数, 目标 += 用 u16 截断)。
