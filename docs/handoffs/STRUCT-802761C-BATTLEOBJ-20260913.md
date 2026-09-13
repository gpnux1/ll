# STRUCT-802761C — 战斗对象类型确认：`sub_802761C` 的 `obj` 是 `BattleObj *`（已落地，fncheck 324B 绿）

- 包 ID：`STRUCT-802761C-BATTLEOBJ-20260913`
- 地址：`0x0802761C`（`functions.tsv` 第 574 行，module `event_actor`）
- 状态：**已匹配（324 B）**，本次为**语义/类型升级**（不改字节）
- 日期：2026-09-13
- 涉及文件：`src/event_actor.c`（函数签名+字段结构体化）、`include/code_0.h`（原型注释）、`functions.tsv`（note）

---

## 1. 结论

**用户怀疑成立：`sub_802761C(u8 *obj)` 的 `obj` 就是 `BattleObj *`。** 已改为 `u8 sub_802761C(BattleObj *obj)`，裸偏移全部换成结构体字段，`fncheck` 仍 **OK 324 B**，全量 `make` + `sha1sum -c ll.sha1` 绿（812/1059）。

这是**类型/命名**层面的更正，不是新匹配。

---

## 2. 证据链（E1 为主，三重独立印证）

### 2.1 入口是函数指针表，索引由 `slot` 决定 → 传参必是池对象

`sub_803F444`（`0x0803F444`）按对象 `slot`（+0xBE）分流，`slot > 0x70`（特效/特殊槽）分支：

```
0803f564: cmp  r2, #0x70            ; r2 = obj->slot
0803f566: bls  → 返回 0
0803f568: r0 = obj; r0 += 0x88      ; obj->animPtr
0803f56c: r2 = *r0                  ; animPtr
0803f56e: r0 = r2 + 0x2f; r5 = *r0  ; 默认索引 = animPtr[0x2F]
0803f574: r0 = obj->fxKind (+0xBC)  ; s8
0803f57e: 若 fxKind == 1:           ; 变体: 索引 = animPtr[0x30 + obj->animSubIdx]
0803f586:   r1 = obj + 0xC2; r0 = r2 + 0x30; r0 += *r1; r5 = *r0
0803f594: r1 = 0x0839CD5C
0803f596: r0 = r5 << 2
0803f59a: r2 = *(r1 + r0)           ; 表项
0803f59c: r0 = obj (r4); r1 = arg1 (r6)
0803f5a0: bl  _call_via_r2          ; 间接调用
```

`0x0839CD5C` 是一张 89 项（`0x0839CD5C..0x0839CEBC`）的 code 指针表；`sub_802761C` 在 **idx 9**（`0x0839CD80 = 0x0802761D`，含 Thumb 位）。

调用者 `sub_801BE34`（`0x0801BE34`）与 `sub_801C484`（`0x0801C484`）这样取对象：

```
adds r1, r0, #0xbd
ldrb r2, [r1]           ; obj->f_BD = 目标池索引
movs r1, #0xC8
muls r1, r2, r1
adds r1, r6, r1         ; r1 = pool + f_BD*0xC8   ← 0xC8 = BattleObj 步长
bl   sub_803F444
```

`0xC8` 正是 `BattleObj` 的大小、`slot`(+0xBE)/`fxKind`(+0xBC)/`animPtr`(+0x88)/`animSubIdx`(+0xC2) 全部吻合 → **传进去的就是 `BattleObj`**。

### 2.2 函数体偏移逐一落在 BattleObj / ObjHead 字段上

| 原写法 | 结构体等价 | BattleObj 证据 |
|---|---|---|
| `obj[0xBF]` | `obj->posX` | +0xBF 屏幕 X（E3：出场 ← 阵型表 `tbl[id*4+2]`；`801D568` 锚点；`802B608` 拷到 `0x03000828`） |
| `obj[0xC0]` | `obj->posY` | +0xC0 屏幕 Y（同上 `tbl[id*4+3]`） |
| `*(u16*)(obj+0x2A)` | **`obj->headA.f_1E`** | headA 在 +0x0C，+0x0C+0x1E = **0x2A**；ObjHead 注释：`f_1E "重置时拷自 +0x24"`。⚠ **不是 `f_1A`**（0x26），首轮误配即差 2 B（见 §4） |
| `obj[0x35]` | `obj->headA.palSlot` | headA+0x29 = **0x35**；ObjHead 注释：`palSlot "调色板槽 (sub_804C2FC 实参)"`，与本函数把它传给 `sub_801CE80`（调色板/动画切换）一致 |
| `*(u16*)(obj+0x24)` | `obj->headA.kindFlags` | headA+0x18 = **0x24**；ObjHead 注释：`kindFlags "0x200=跳过帧构建; 0x800=DMA 禁用; 0x8000=激活后清除"` → **`0x1000` 是同一标志字节的位，本函数用作"本对象演出完成"** |
| `(u8*)obj` 传给 `sub_80444A4` | 保持 `u8*` | `sub_80444A4` 内调 `sub_80462E4(arg0, ids, 0x6F)` 收集同组对象，参数确为 `u8*` |

### 2.3 `sub_803F5B4`（本函数调用）反证

`sub_803F5B4` 内：`sub_8020DF0(obj)`、`sub_8020E54()` 写 `0x0300083D/0x03000840`，并对命中项 `sub_804473C(obj, pool + (v&0xF)*0xC8)` 后写 `target[0xB2] += ...`。**`0xB2` = `BattleObj.dmgAmount`**，且 `submit`/`+=` 与 `sub_8020AE4`（`ObjList_IncAffine`）同型。同族的 `sub_80444A4` 直接把 `pool + ids[i]*0xC8 + 0xB2` 清零 —— 即"清伤害累积"。这解释并印证了本函数 case0 的 `sub_80444A4(obj)`（清同组伤害）+`sub_803F5B4(obj)`（按等级给同组加值）。

---

## 3. 落地改动

`src/event_actor.c`：

```c
u8 sub_802761C(BattleObj *obj)
{
    ...
    case 0:
        gUnk_03000828 = obj->posX;
        gUnk_03000829 = obj->posY;
        gUnk_03000822 = obj->headA.f_1E;
        gUnk_03000824 = obj->headA.palSlot;
        sub_80444A4((u8 *)obj);
        sub_801CE80(obj, 5, 0x1B4, 0xD, 0);
        sub_803F5B4(obj);
        ...
    sub_803F658(obj);
    if (obj->headA.kindFlags & 0x1000)
    {
        u16 masked = obj->headA.kindFlags & 0xEFFF;
        zero = 0;
        obj->headA.kindFlags = masked;
        sub_801CE80(obj, zero, gUnk_03000822, gUnk_03000824, zero);
    }
```

`include/code_0.h`：`u8 sub_802761C();` → `u8 sub_802761C(BattleObj *);` + 语义注释。

`functions.tsv` 第 574 行 note 已更新（status 保持 1）。

**验证**：
```
python3 scripts/fncheck.py sub_802761C   → OK (324 bytes @0x0802761c, 14 池重定位已施加, 8 bl 槽忽略)
timeout 900 make && sha1sum -c ll.sha1   → ll.gba: 成功 (匹配进度 812/1059)
```

---

## 4. 工具与坑

- 隔离比对脚本（不碰共享文件）：`/tmp/sc761C.sh <cand.c>` + `.scratch/syms761C.ld`。候选用 `permuter/sub_802761C/compile.sh`。
- **坑 1：+0x2A 是 `headA.f_1E` 不是 `headA.f_1A`**。headA 基址 +0x0C，`f_1A` = +0x26，`f_1E` = +0x2A。误用 `f_1A` 出 `ldrh r0,[r5,#0x26]`（目标 `#0x2A`），差 2 B。**ObjHead 里 +0x1A/0x1E 两个"未验证"字段，语义需按 headA 基址 +0x0C 重新数一遍。**
- **坑 2：`__umodsi3` 需入 syms**（`= 0x08055DD9`），否则 ld 报 undefined。
- 结构体化后字节完全不变（本轮首次编译即 2 B 差，仅因坑 1；改对后直接 OK 440→324 B 真身）。

---

## 5. 家族影响（后续可做，本包未动）

`0x0839CD5C` 表共 **89 项**，指向 `0x080257D8` 起的一整族对象演出函数（`sub_80257D8 … sub_804E2AC`，见 `docs/handoffs/ENGINE-HUB-20260912-a.md` 的完整 caller 集合）。这些函数**同样是 `sub_803F444` 经同一张表调用的**，因此**同一批函数的入参也应统一为 `BattleObj *`**。

现状：该区间内目前**只有 `sub_802761C` 是 status=1**（其余全为 status=0 的 `INCLUDE_ASM`）。因此不存在"改了要同步已匹配兄弟"的字节风险；后续匹配这批函数时，可直接采用 `BattleObj *` 签名，无需再从 `u8 *` 迁移。

已完成的 `src/event_actor.c` / `src/event_hub.c` 里若干函数（如 `sub_8032548` 族）若也来自本表，签名仍写作 `u8 *`；**但改它们属于独立工作包**（需逐个核对偏移与调用点字节），本包不做，避免无谓churn。
