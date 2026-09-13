# STRUCT-ALL-U8-BATTLEOBJ — 全仓库 `u8 *` 参数批量升级为 `BattleObj *`（完成，SHA1 绿）

- 包 ID：`STRUCT-ALL-U8-BATTLEOBJ-20260913`
- 触发：`STRUCT-802761C-BATTLEOBJ-20260913` → `STRUCT-CALLEES-802761C-20260913` 之后，用户要求"所有 u8* 都换"
- 状态：**已完成**。70 个函数的第一参数由 `u8 *` 改为 `BattleObj *`，全部字段访问结构体化；调用点全部加显式 cast。
- 验证：`fncheck` 75/75 OK；`make clean && make` + `sha1sum -c ll.sha1` 绿（812/1059，76.7%）
- 日期：2026-09-13

---

## 1. 范围与方法

### 判定（E1：函数体自身证据）
不靠调用点猜，而是**在函数体内找 BattleObj 独有偏移**：
- `0xBE` slot、`0xB0` state、`0x88` animPtr、`0xBC` fxKind、`0xBD` f_BD、`0xC2` animSubIdx、`0xAB` variantClass、`0x6C/0x6E` hp/maxHp、`0x54` headB.kindFlags、`0x24` headA.kindFlags、`0xB2` dmgAmount
- 命中 `0xBE`，或命中集合与独有掩码交集 ≥2，或 `{0x24,0x54}` / `{0x24,0xB0}` 同现 → 判为 BattleObj

结果：**97 个函数 / 9 个文件**（第一遍 70 个，第二遍按 headA/headB 信号补 27 个）。

| 文件 | 个数 |
|---|---|
| src/battle_engine.c | 20 |
| src/battle_rewards.c | 15 |
| src/scene_interact.c | 10 |
| src/event_actor.c | 9 |
| src/event_hub.c | 5 |
| src/obj_pool.c | 4 |
| src/obj_state.c | 4 |
| src/battle_anim.c | 3 |
| src/cutscene_mgr.c | 3 |

### 变换（两层，均字节校验）
1. **签名**：`T f(u8 *arg, ...)` → `T f(BattleObj *arg, ...)`
2. **字段结构体化**（65 个函数）：脚本自动改写 7 种访问形态
   - `arg[N]`（u8）→ `arg->field`
   - `*(u16 *)((u8 *)arg + N)` → `arg->field`（按位宽匹配 u8/u16/u32，防误配）
   - `*(u16 *)&*((u8 *)arg + N)`、`*(u16 *)&arg[N]` → `arg->field`
   - `(u16 *)((u8 *)arg + N)` → `&arg->field`
   - `arg + N` → `&arg->field`（已知偏移）或 `(u8 *)arg + N`（未知）
   - **危险形 `*(u16 *)(arg + N)` 必须按字节语义转换**（`arg` 现为 BattleObj*，`+N` 会按 0xC8 步进）
3. **5 个函数只做签名转换**（`sub_8045F94`/`sub_8046060`/`sub_804D310`/`sub_804D708`/`sub_804DABC`）：字段化会改变局部寄存器分配（实测 `strb` 源 r3→r1、u8 提升差异），故保留显式 `((u8 *)arg)[N]` / `*(u16 *)((u8 *)arg + N)` —— 语义等价、类型安全、字节不变。
4. **头文件原型同步**（70 条），否则 "conflicting types" 编译失败。
5. **调用点 cast**：25 处（`sub_8045F94`×9、`sub_804E76C`×7、`sub_804612C`×2、`sub_804DD70`×2 等）加 `(BattleObj *)`，消除 `incompatible pointer type` 警告。

---

## 2. 关键坑（复现者必读）

1. **`arg + N` 是最大的静默地雷**。`u8 *` 时 `arg + 0x24` 是字节偏移；改成 `BattleObj *` 后变成 **0xC8×24 字节**。转换器必须覆盖 `*(u16 *)(arg + N)` 这一形态，否则编译通过、SHA1 红且差异巨大。
2. **`(u16 *)arg + 0x36` 不是字节偏移**，是 u16 索引（=字节 0x6C）。转换器需识别 `(T *)arg + N` 并跳过，否则错改。
3. **`/tmp` 里的 heredoc 脚本不持久**。用 Write 工具落盘再跑；后台任务若无输出，先确认脚本文件存在。
4. **字段化不必定字节中性**。u8 提升/位宽会在少数函数改变寄存器或指令条数（如 `D310/D708` 140→142B）；**必须逐函数 fncheck**，失败即退回"签名+显式字节 cast"方案。
5. **头文件不同步 = 编译失败**，不是警告。

---

## 3. 验证

```
python3 scripts/fncheck.py <70 个函数 + sub_802761C/sub_80444A4/sub_803F5B4/sub_80462E4/sub_803F658>
→ 75/75 OK

make clean && make && sha1sum -c ll.sha1
→ ll.gba: 成功（匹配进度 812/1059）
```

`sub_8046F0C` 顺带完成 case1-14 的字段命名（`*((u16*)obj+0x36)` = `hp`、`*((u16*)obj+0x3F)` = `statMods[0]` 等），fncheck OK 278B。

---

## 4. 未做 / 后续

- 仍有大量函数以 `u8 *` 接收 BattleObj（未在本包判定集内，多为证据不足或非本族），后续可逐个判定。
- `event_hub.c` 里 `sub_8045B90`、`sub_8020974`、`sub_801CBA4` 等**被调**函数仍为 K&R 原型（`()`），其调用点警告是**既有的**，本包未动。
- 已匹配函数 `sub_804D310/D708/DABC` 采用"签名+显式字节 cast"折中；若后续要让它们字段化，需先解决局部寄存器分配（参见各自 TSV note）。

---

## 5. 第二遍（补漏 27 个）

首遍只认 `0xBE`/多字段共现，漏掉了**只用 `headA` 字段**（0x24/0x2A/0x35）的函数。补判据：
函数体内出现 `headA`/`headB` 引用，或 `(BattleObj *)` 强转自身参数。

补转 **27 个**（event_hub.c 20、battle_engine.c 5、obj_pool.c 1、obj_state.c 1），
其中 `sub_8044F4C` 的第二参 `arg1` 也是 BattleObj（体内 `arg1+0xB0`/`+0xB2` 字节语义），一并转换。
`sub_8038568` 第二参同理（`+0x6E` = maxHp）——**该处漏转曾导致 `linker.ld cannot move location counter backwards`（尺寸-4B）**，是"只转第一参"的典型陷阱。

## 6. 最大陷阱：多参函数只转第一参

`sub_8038568(BattleObj *arg, BattleObj *arg1)` 若只把 `arg` 的访问字段化而漏掉 `arg1`：
`*(u16 *)(arg1 + 0x6E)` 会变成 **0x6E×0xC8 字节**，编译通过但链接器钉扎点漂移。

**判据脚本**（每轮转换后必跑）：
```python
# 对每个 BattleObj* 形参，检查是否残留未加 (u8*) 的裸偏移
if re.search(r'\b%s\s*\+\s*0x[0-9A-Fa-f]+'%arg, body) and '(u8 *)' not in pre: 报错
```

## 7. 最终状态

- 转换 **97 个函数**，9 个文件；`fncheck` 97/97 OK。
- `make clean && make` + `sha1sum -c ll.sha1` 绿（813/1059，76.8%）。
- 剩余 32 个 `u8 *` 首参函数**均无 `headA`/`headB` 信号**，非 BattleObj（多为字节缓冲/池基址），不应转换。
