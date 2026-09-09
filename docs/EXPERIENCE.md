# EXPERIENCE.md — 经验库 (agbcc 代码生成经验 / 坑 / 失败案例)

> ⚠ **本文是「经验参考」, 不是「必须遵守的硬约束」。**
> 下面每一条编号条目都是过去匹配某个具体函数时总结出的**经验性倾向** —— 用来在卡住时提供思路、
> 加速尝试。**不是硬约束**。遇到与实际情况冲突、或换个上下文就不适用时, 以当前反汇编和
> permuter/fncheck 的实测结果为准, 别硬套。
> 硬约束(必须遵守的铁律/禁令)全部在 `AGENTS.md` (仓库根) —— 那边是「流程」, 本文是「经验」。
> 内部的「经验 N」只是本文件内的**编号索引**(方便交叉引用), 不表示「这条必须被遵守」。

> **开工手册在仓库根 `AGENTS.md`** (铁律/函数清单/工作循环/permuter/工具)。本文只放**深度经验**:
> 写 C 前查同族经验、卡寄存器时查诊断与失败存档、改名/注册符号前查管线细节。持续追加。
> 历史来源: 原 `RULES.md` (2026-09-01 拆分; 2026-09-08 更名为 `EXPERIENCE.md`, 内部编号
> 索引「规则 N」统一改为「经验 N」, 因为这不是必须遵守的规矩, 而是卡壳时提供的思路)。

## agbcc (GCC 2.x) 代码生成经验

以下是已验证过的经验, 按 C 写法反推 agbcc (GCC 2.x) 汇编形态:

> 每条对应一次或多次具体的匹配成功/失败案例, 属于**经验性倾向**而非普适铁律。
> 换个函数/换个上下文可能失效 —— 卡壳时先查同族案例, 别直接照抄。

> ⚠ **编号修正 (2026-09-01)**: 合并两份经验清单时 **72 / 78 / 86 / 87 / 88 各撞过一次号**。
> 已把每组里**靠后**的那条改编为 **97 / 98 / 99 / 100 / 101** (首现者保留原号,
> 所以指向首现者的旧引用全部仍有效)。标题行带“原误编为 N”后缀的就是改过的那条。
> 已知引用同步: AGENTS.md §3 “1-D”→经验99; progress.md 的 sub_8048818→经验97、
> “两个新规律”→经验100/101。若再在旧文档里看到歧义的 规72/78/86/87/88, 按上下文案例函数名判定。

1. **调用方侧的 u8 截断** (`lsls r0,#0x18; lsrs r0,#0x18`) = 被调函数首参类型是 u8
2. **加法操作数不交换**: 想要 `adds r4, r0, r1` (乘积在左) 写 `(u32)(i * 0xC8 + (u32)ptr)`,
   想要 ptr 在左写 `ptr + i * 0xC8`
3. **分支极性**: `if (!(x & 0x20)) A; else B;` → `ands; cmp; bne→B` 直落 A
4. **continue 形式**: `if (...) continue;` → `beq 循环增量处`, 比嵌套取反 if 贴合目标
5. **RMW 位标志**: `newval = CONST | read; write = newval;` (命名临时变量)
   - CONST 在左 → `adds r0, r2, #0` + `orrs r0, r1`
   - `|=` 会把读操作规范化到 r0, 有时需要这个形态 (对比选择)
6. **extern 具名符号防常量折叠**: 表基址/外设地址要在 linker.ld 注册成符号
   (GCC 不对符号基址做常量折叠, 池里保留完整地址; 字面常量会被折叠如 0x0839CE7C-0x71*4 → 0x0839CCB8)
7. **全局当数组直用**: `extern u8 arr[]; arr[i]` 比局部指针变量更贴近原始代码的寄存器分配
8. **单表达式 vs 多语句**: 间接调用 `tbl[idx](ptr)` 单表达式让 GCC2 先展开被调地址(表池在前);
   拆成两个池加载顺序相反的语句会颠倒
9. **flag 分支形状**: `val = load; flag = 1; if (val <= N) flag = 0;` →
   `ldrb; movs r2,#1; cmp; bhi; movs r2,#0` (1-初始化+条件清零, 不是 bge 反转)
10. **命名局部变量影响寄存器分配**: 临时变量的声明位置/命名会影响 home 寄存器
    (例: newval 命名临时变量强制结果落 r0, load 落 r1; `u8 val` 具名局部改变 flag 分支前的指令顺序)
11. **结构体成员访问 vs 裸指针偏移**: `ptr->field_BB = 0` 逐成员独立寻址
    (每成员 `mov rX, ip; adds rX, #off`, GCC 不做相邻偏移 CSE), 而 `*(u8*)(arg0+0xBB)` 系列会被
    CSE 成 `adds r1, #1` 连续递增。目标汇编出现 `mov ip, r0` 缓存 + 每成员 fresh 寻址 = 原代码是结构体成员访问
12. **闩自增调度**: `for(;;i++) { call(..., i+1, 1); }` GCC2 调度器自动把闩自增
    (`adds r4,#1; lsls; lsrs`) 吊到 bl 之前 —— 遇到"自增在调用前"的目标代码先试最自然形式,
    不要手工 `++i`/拆语句 (会被合并成经临时寄存器的形态)
13. **RMW 拆两条赋值**: `x = x & MASK; x = CONST | x;` 两条语句的调度与 `x = (x & MASK) | CONST`
    一条语句不同 (中间会插入其他语句的常量物化)。单语句不匹配时试拆分
14. **头文件类型冲突**: 定义带 struct 指针形参而头文件是 `void*` 时, 定义改用 `void *arg0` +
    函数体首行 `Struct *ptr = (Struct *)arg0;` —— cast 局部不影响代码生成 (已验证)
15. **表基址池加载位置决定 tbl 局部取舍**: 目标汇编里 `ldr rX, =表基址` 出现得早(首个实参求值前)
    → 用 `tbl` 局部变量且首语句赋值; 出现得晚(贴近首次使用) → 不用局部, 直接重复写 `表[arg].field` 表达式
    (CC4 案例=后者; sub_80210C0 案例=前者)。注意 tbl[0].field_X 无下标运算时 tbl 局部不会被传播优化掉
16. **switch 分发形状**: 少量 case 时 GCC2 生成 `cmp; beq case0; cmp; beq case1; b default`
    (beq 正跳转进 case 体), 而 if/else-if 生成 bne 跳过形状 —— 目标是 beq 链时改写 switch
    (sub_802093C 案例)。case 体只算地址/赋公共变量, store 放 switch 后, 载入值用命名临时
    (new_var) 才能落在独立寄存器
17. **三个连续 `sym = 0` 字节存储的寄存器轮换未解 → 已解**: sub_8020B54 目标把三个地址分配成
    r5/r6/r4 (池序不变), 任何语句顺序变体都得到 r4/r5/r6; agbcc local_alloc 按 QTY_CMP_PRI
    (=floor_log2(n_refs)*n_refs*size/life) 排序, 全 0 权重按 qty 序。
    ✅ **2026-09-02 解**: `do { gUnk_03000716 = 0; } while (0);` 屏障 (经验 113) 打破等优先级 tiebreak,
    最后一条 strb 的 qty 因屏障多一条 insn 的 life 变化, 权重不再相等, 分配轮换归位。
18. **permuter 高分解可能语义错误**: 置换可能把索引换成别的变量/把加法挪过分支,
    分数低但行为错 (案例: sub_8053138 的 70 分版用检查字节当索引、sub_805321C 的 45 分版
    在 if 路径引用未初始化 r6)。凡 permuter 改动过数据流的解, 合入前必须人工核对每条访存
19. **script 处理器族的值链寄存器分配**: `val = *ptr; val += N;` 分支两侧公共尾存时,
    目标要求 ldrh 直写 val 的寄存器 (r0) 且池常量落 r1; 现有写法 (数组索引/ofsPtr/+=)
    GCC2 都生成临时寄存器 —— 该类问题仍未解。sub_801A684 后来确认并非同一根因，已由经验 83 收尾
20. **循环内常量存储的外提**: `ptr->field_BC = 0` 的 0 常量会被 loop.c 当循环不变量外提,
    寄存器强制用 call-saved r7 (多 push)。long long 局部变量 (zero=0 后存其低字节) 可阻止外提,
    但 movs 的调度位置仍差 1 条 —— 卡住 sub_804C890 (75分)
21. **结构体数组寻址的两段形式**: 全局结构体内嵌数组 (如 gUnk_03004F20 的 0x18 偏移数组)
    用成员形式 `s.arr[i].f` 时 GCC 会把成员偏移折进 strh/ldrh 的立即数 (strh [r0,#0x18]);
    目标是两段寻址 (adds r2,#0x18 再 adds r0,r0,r2) 时要写显式字节指针:
    `*(u16 *)((u8 *)&sym + 0x18 + index * 24)` (案例 sub_801761C/sub_8017588)
22. **if/else-if 链 vs switch**: 目标是 `cmp;beq;cmp;beq;b` + 函数体放在链条之后(体外) = 原代码是
    `switch(x) { case N: ... break; }`; 若是内联布局(第一个 body 紧跟)则是 if/else-if
23. **不要把重读的全局缓存成局部**: 目标在函数调用后重读 `gUnk_XXX`(ldrb) 说明原代码没有局部缓存,
    每次直接写全局访问即可 (缓存反而多占寄存器导致 push {r4-r7})
24. **mask 链必须逐条语句**: `x &= ~3; x &= ~0xC;` 各自一条 —— 写成单表达式
    `x & ~3 & ~0xC` 会被 GCC2 代数折叠成 `x & ~0xF`, 指令序列完全不同
25. **`do { stmt; } while (0)` 屏障**: 包住某条语句可阻止 GCC2 与相邻语句合并/调度
    (permuter 常用此招)。**实测必要**: sub_8019148 的末位 `d &= ~0xC000` 去掉屏障后,
    GCC2 将它与前一条 `&= ~0x2000` 折叠成池常量, 指令序列改变 —— 屏障不可省略
26. **赋值表达式作存储地址**: `*(u16 *)(new_var = 0x04000008) = d;` ——
    命名变量+赋值表达式的形态改变 GCC2 对存储地址伪寄存器的处理
27. **初始化顺序即指令顺序**: `i = 0` 的 movs 出现在目标两个池加载之后 = 原 C 是
    `ptr = ...; i = 0;` 顺序, 声明处初始化(`u16 i = 0;`)会让 movs 提前
28. **寄存器差异不计入 asm-differ score 的假象**: score=0 不代表字节一致!
    终验用 `arm-none-eabi-objcopy -O binary --only-section=.text xx.o xx.bin && cmp xx.bin yy.bin`
29. **反向假象: 单函数 .o 的 score 会被"未重定位的字面池"污染**。
    真 C 编出的 .o 里字面池是 `.word 0` + `R_ARM_ABS32 gUnk_XXX` 重定位项,
    而 gbadisasm 生成的 target.o 池里是硬码值 —— 于是**逐指令全对也会报 score=400**
    (池被 objdump 解码成假指令)。正确做法: 先用绝对符号脚本部分链接, 再比字节:

    ```bash
    printf 'SECTIONS { .text 0 : { *(.text) } }\ngUnk_03004614 = 0x03004614;\n' > .scratch/abs.ld
    arm-none-eabi-ld -T .scratch/abs.ld -o .scratch/linked.o .scratch/t.o
    arm-none-eabi-objcopy -O binary --only-section=.text .scratch/linked.o .scratch/mine.bin
    arm-none-eabi-objcopy -O binary --only-section=.text permuter/<func>/target.o .scratch/tgt.bin
    cmp -l .scratch/mine.bin .scratch/tgt.bin   # 只剩 bl 编码差 = 已匹配
    ```

30. **GCC2 的 flatten_expr 会把加法链里的常量/符号地址归到最左项**。
    写 `dest = A + (B + K) + C` 无论怎么加括号都得到 `(A + K) + B + C`（K=常量或符号地址同理）。
    目标若是 `A + (B + K)` 的分步形式（先算 B+K 再与 A 相加）,
    **必须把 `B + K` 拆成独立语句的临时变量**, 单表达式无解 (sub_8007A1C)。

31. **赋给指针局部 vs 整数局部 会差一条 mov**。`dst = (u8*)((f0-1)<<15)` 当 f0 是 CSE 临时且已死时,
    GCC2 直接 `subs r0,#1; lsls r4,r0,#15`; 而 `bankOff = (f0-1)<<15`（u32 局部）会出
    `mov r1,r0; subs r1,#1; lsls r1,r1,#15`。目标里多出的那条 `adds rX,rY,#0` 就是
    “这里原本存的是整数临时变量”的确证 (sub_8007A1C)。

32. **字面量 vs extern 符号不只防常量折叠, 还会改变寄存器分配**。
    sub_8007A1C 里图块缓存基址写 `(u32)gUnk_02006000`（需在 linker.ld 注册）会多占一个 callee-saved
    变成 `push {r4-r7}`; 改回字面量 `0x02006000` 后与目标逐字节一致。
    即“能宏化/符号化”不等于“应该”, 以字节为准。

33. **有时需要一条“死 store”才能复现目标的寄存器分配**。
    sub_8007A1C 必须在声明处写 `u8 rows = gUnk_030046A0[arg0].field_9;`（后面会重赋值,
    该 store 被 GCC2 删除不发任何指令）, 否则字面池加载位置从偏移 4 跑到 8。
    症状：“逐指令序列归一化后完全相同, 但 push 多一个寄存器 / 池加载位置差两格” 时,
    试加或减一个死初始化、或改临时变量的类型（u32/指针）。

34. **循环体内两条独立 `adds` 的顺序由调度决定**。
    `*dest++ = *src++` 出 `adds r4,#2; adds r1,#2`, 目标要 `src` 先自增 ——
    写成 `*(u16*)dest = *(u16*)src; src += 2; dest += 2;` 仍会被调度回去,
    是 **permuter 的语句置换**找到的解 (sub_8007A1C)。

    `R_ARM_THM_CALL` 的 bl 偏移依赖最终链接地址, 单 .o 里必然不同, 不算真差异。
    一切以 `make` + SHA1 为准。(案例: sub_8052BA0 首试即此情况, score 假高 400, 实际字节一致)
35. **`strh` 紧跟 `ldrh` 再 `bl` = 原代码是两次独立访问**: 目标
    `strh r1,[r2]; ldrh r0,[r2]; bl sub_8008B14` 对应 `gU = expr; f(gU);` 两条语句,
    没有局部缓存也没有寄存器直传 —— 写成 `val = expr; gU = val; f(val);` 会少一条 ldrh。
    (案例 sub_8052BA0)
36. **同一函数内 `+` 与 `|` 可以混用且必须按目标拼**: `data[2] + (data[3]<<8)` 生成 `adds`,
    `data[2] | (data[3]<<8)` 生成 `orrs` —— 两者语义等价但字节不同, 必须逐分支照抄目标助记符。
    (案例 sub_8052BA0: 赋值侧用 `+`, 实参侧用 `|`)
37. **switch 不写 `default:` 才复现比较链分发**: 目标是 `cmp#1;beq; cmp#1;bgt; cmp#0;beq; b 尾`
    这种二分形状时, 写 `switch (x) { case 0: ... case 1: ... case 2: ... }` 且**不要加 default 标签**;
    加了 default 会多出一个空基本块, 分发顺序改变。(案例 sub_8052C24)
38. **每个 case 里重复写公共赋值, 不要外提**: 三个 case 各自 `ldr r1,=sym; movs r0,#0xff; strb`
    = 源码里每个 case 都写了 `gU = 0xFF;`。提到 switch 之后只会存一份, 少两份池加载 → 不匹配。
    (案例 sub_8052C24)
39. **跨调用存活的变量自然落被调保存寄存器**: 目标 `push {r4,r5,lr}` + `bl` 后仍用 `ldrb r0,[r4,#1]`
    说明 `data` 被分配到 r4 —— GCC2 local-alloc 会自动避开 call-clobbered 的 r0-r3, **不需要**
    手工加 `register` 或拆表达式。同理, 调用之后的 `*ptr += N` 必然重读 `ldr r0,[r5]`
    (r3 已被调用展平), 写 `data += N; *ptr = ...` 反而不匹配。(案例 sub_8052BA0/sub_8052C24)
40. **switch 的 case 贯穿(fall-through)会直接反映在布局上**: 目标里 case0 body 末尾**没有** `b 尾`
    而是直接接到 case1 body(物理相邻) = 源码 case0 少写一个 `break`。
    反过来, 两个 body 之间有 `b 尾` 就是有 break。建议加 `/* fall through */` 注释。
    (案例 sub_8052C90: case 0 → `sub_8009B44();` 贯穿到 case 1 → `sub_80089E0(3);`)
41. **只出现 `lsls r0,#0x18` 而没有配对的 `lsrs`** = 源码是 `(u8)ret != 0` 这类**只测零**的用法
    (左移保留零性, GCC2 省掉回移)。仍说明被调函数返回 u8, 但写法上不要手加 `(u8)` 强转。
42. **只用于分发/判断一次的指针不要当长命局部**: 目标 `ldr r0,[r4]; ldrb r0,[r0,#1]` 把值留在 r0
    (无独立 home) = 该值在源里只被读一次; 若目标把它放在 r2/r4 并在多处引用, 才需要命名局部。
    同一句 `data = (u8 *)*ptr;` 两种情况 GCC2 自己会区分, 不必改写。
    (对比: sub_8052C90 用 r0 一次性 vs sub_8052D4C 用 r4 多处引用)
    (案例 sub_8052D4C: `if (sub_8001030(...) != 0)`)
43. **两个都跳调用的指针同时住进 r4/r5 时的先后**: 被调保存寄存器从 r4 往上分配,
    **n_refs 大的先拿 r4**。sub_8052D4C 里 `data`(4 refs: 1 定 + 3 用) 拿到 r4,
    `ptr`(3 refs) 拿到 r5 —— 与目标一致, 无需干预。
44. **被调函数已匹配但没进头文件时的补法**: 先确认定义在调用点之前(否则之前是隐式 int 声明),
    再按定义的真实类型补上完全一致的原型 —— 同类型不会改变已有代码生成。
    (案例: 新增 `u8 sub_8001030(u16);` / `u8 sub_80010AC(u16);` 到 code_0.h, code_0.c 不受影响)
45. **`adds r0,#1; ldrb rX,[r0]` 连续走位 ≠ 数组下标**: 目标里地址逐次 +1 地读一串字节
    (且最后一个可能是 `ldrb rX,[r0,#1]`), 必须写成前递指针 `a1 = *(++p); ... ; a6 = *(p + 1);`;
    写成 `data[1]..data[6]` 会得到 `ldrb rX,[rBase,#N]` 偏移寻址, 字节不同。(案例 sub_80528C8)
46. **`return x != 1` 会编成无分支序列** `eors/negs/orrs/lsrs`; 想要目标的
    `cmp #1; beq L; movs #1; b end; L: movs #0` 分支形, 必须写成两条语句:
    `if (x == 1) { return 0; } return 1;`。(案例 sub_80528C8)
47. **GCC2 动用 r8 时的固定序言/尾声**: `push {r4,r5,r6,lr}; mov r6, r8; push {r6}; sub sp, #N`
    对应尾声 `add sp, #N; pop {r3}; mov r8, r3; pop {r4,r5,r6}; pop {r1}; bx r1`。
    能否复现取决于**活跃值个数刚好溢出 r0-r7**: sub_80528C8 需要 ptr + 6 个 u8 局部
    (其中 2 个溢到栈、1 个进 r8); 少一个局部就不进 r8, 多一个就多 push 一个寄存器。
    写法上就是普通的多局部变量, 不要手工干预。
48. **一串无分支的 `池加载 + ldrb + strb` 平行赋值**: 若 ptr 在整个过程中从未占用 scratch 寄存器,
    GCC2 会把它一直留在 r0 **不产生入口复制**(目标开头没有 `adds rX, r0, #0`)。
    反推时别因为没看到复制就以为写错了。(案例 sub_8052CF0)
49. **池常量先查具名符号**: 0x03001944 已有 `gMainGameState`。用名字写(更可读且防常量折叠);
    确实没符号时才新增 linker.ld 条目。
50. **并发陷阱: `make` 会把其他智能体改到一半的源文件一起编进来**。
    本轮 sub_80528C8 合入后 SHA1 报 760 万字节差异, 实际原因是 code_1.c / code_1b.c /
    code_8005020.c 在 22:00-22:11 被别人改动且缺 linker.ld 符号, 与本次合入无关。
    判定三步: ① `ls -l --time-style=+%H:%M:%S src/*.c` 看哪些文件 mtime 晚于你上次绿灯;
    ② 用 ll.map 把差异地址归属到 .o; ③ 只重编自己的对象
    `make build/src/<你的文件>.o` + 部分链接 cmp 自证清白。
51. **验证 r8/高位寄存器是否泄漏的新方法**: 把当前 C 文件 备份一份, 将目标函数换回
    `INCLUDE_ASM("asm/matchings", <func>);` 重编, 然后逐函数比对两个 .o 的反汇编字节:
    除目标函数外全部一致 = 无泄漏。比“等整个 ROM 绿”快得多且不受其他智能体干扰。
52. **单函数回环: 单独编 `sub_xxx.o` → dump 汇编 → 用 tools/asm-differ 对 `sub_xxx.s`**。
    这是**不依赖整 ROM 的唯一逐指令验证手段**, 多智能体并行时必用。一条命令:

    ```bash
    scripts/fndiff.sh <func> [候选.c]        # 默认用 permuter/<func>/base.c
    scripts/fndiff.sh sub_8052C24
    #    参考: asm/nonmatchings/sub_8052C24.s
    #    == sub_8052C24 : permuter/sub_8052C24/base.c ==
    #    TARGET ... CURRENT (630)
    ```

    它做的四步: ① 无 `permuter/<func>/compile.sh` 时自动生成(与 Makefile 同一套 flag:
    cpp → preproc → agbcc -O2 -fhex-asm -fprologue-bugfix → as); ② 无 `target.o` 时从
    `asm/nonmatchings/<func>.s` 拼 `macros/function.inc` 汇编出来; ③ 编候选到
    `.scratch/fndiff/<func>/mine.o`; ④ `diff.py -o -f mine.o -F target.o <func>`。
    已匹配的函数会自动回退用 `asm/matchings/<func>.s` 作参考, 所以也能当回归测试用。

    **score 解读方法**: 每个未重定位的字面池约值 400 分(经验 29), 所以
    `score ≈ 400 × 池个数 + 10 × 真差异行数`。典型: 无池函数应为 0;
    1 个池 400; 4 个池 1600。**score 不等于 0 不代表没匹配, 必须配合 fncheck 定性。**

    **匹配完成后必须固化胜出版本**(否则下次人跑 base.c 会得到完全不同的 score, 误以为没匹配):

    ```bash
    scripts/fndiff.sh --promote <func> permuter/<func>/<winner>.c   # 回写 base.c + 清理中间变体
    ```

    本轮实测: 4 个函数的 `base.c` 停在首个失败尝试上, score 与实际差 5-6 倍。
53. **两个验证工具分工**: `fndiff.sh` 看**逐指令形状**(快, 能定位到哪一行不对);
    `fncheck.py` 看**字节级定论**(自动施加池重定位 + 排除 bl 槽)。两个都不需要整 ROM 绿。
    标准顺序: fndiff 迭代到形状一致 → fncheck 确认 OK → 合入 → make 全量终验。
54. **⭐ 函数"返回类型非 void 但体内没有 return" 会把 r0 整个函数锁死 → 所有寄存器上移一位**
    (案例 sub_8008124, 破解过程见 progress.md)。
    症状: 指令序列逐条已完全一致, 但目标里 **r0 一次都没出现**, 且第 8 个横跨全函数的值被塞进 `ip`
    (Thumb-1 的 `ldr Rt,[pc,#imm]` / `strb Rt,[Rb]` 只认 r0-r7, 所以表现为
    `ldr r4,=sym; mov ip,r4` + 使用时 `mov r1,ip; strb r3,[r1]`)。
    机理: 非 void 且无 return → 没有任何指令写 r0 → flow 认为 r0 从入口一直活到结尾,
    `find_free_reg` 从 r0 往上扫时全程被拒, 于是临时量落 r1、主变量从 r2 起分配。
    写法: `u32 sub_8008124(void) { ...; gUnk_03004640 = i; }` —— **不要**补 `return 0;`
    (会多一条 `movs r0,#0` 并释放 r0, 分配立刻退回原样), 也**不要**改成 void。
    头文件里保持 K&R 式 `u32 sub_8008124();` 即可, 调用方 `f();` 不受影响。
    判据: 全 ROM 只有 sub_8008124 一个 nonmatching 函数完全不碰 r0 —— 见到 r0 缺席就想这条。
55. **⭐⭐ 遇到“无法解释的连续相同 `ldr`”或“相邻 I/O 寄存器共基址访问”, 先查 `include/gba/macro.h`**。
    不要自己拼 `REG_DMA3SAD/DAD/CNT` 或手造结构体 —— SDK 宏展开后就是原代码的形状:
    ```c
    #define DmaSetUnchecked(n, src, dest, control) {
        vu32 *dmaRegs = (vu32 *)REG_ADDR_DMA##n;   /* ← 共基址 + 偏移 0/4/8 */
        dmaRegs[0] = (vu32)(src);
        dmaRegs[1] = (vu32)(dest);
        dmaRegs[2] = (vu32)(control);
        dmaRegs[2];                                /* ← 就是那次“值未用的 volatile 空读”! */
    }
    #define DmaWait(n) { vu32 *dmaRegs = (vu32 *)REG_ADDR_DMA##n;
                         while (dmaRegs[2] & (DMA_ENABLE << 16)); }
    #define DmaCopy32(n, s, d, size) DmaSet(n, s, d,
        (DMA_ENABLE|DMA_START_NOW|DMA_32BIT|DMA_SRC_INC|DMA_DEST_INC)<<16 | ((size)/(32/8)))
    ```
    所以目标里的 `str r0,[r2,#0/#4/#8]` + 两条重复 `ldr r0,[r2,#8]` + `ands r0,#0x80000000`
    全部自然得到。sub_80527AC 的正解就是两行:
    `DmaCopy32(3, 0x0203DE00, 0x0600B800, gUnk_03000F24 * 64); DmaWait(3);` ——
    参考同族已匹配写法 sub_801A0F0。反例: 自己拼寄存器会多/少指令;
    改用 types.h 的 `DmaCnt` 位域 `->Enable` 会变成 `lsls #0x18; lsrs #0x18` 字节抽取, 也不对。
    同理先查: `CpuSet/CpuFastSet/CpuFastCopy`、`DmaFill*/DmaClear*`、`IntrEnable`、`SPI*` 等。
56. **`asm/matchings/<func>.s` 是"已匹配"的权威记录, 不要手工删改**。
    `scripts/gen_asm.py` 依据 `functions.tsv` + `ll.cfg` + `code.s` **增量重建** (内容不变不 touch);
    所以: 改完 TSV 必须跑 gen_asm; 不要直接编辑这两个目录里的文件;
    `fndiff.sh` 对已匹配函数回退用 `asm/matchings/<func>.s` 做参考, 因此它同时是回归测试。
57. **单函数对比的地址必须取 `code.s`, 不能取 `ll.map`**。
    并行开发中别人改完函数会改变各 .o 尺寸, 整个 ROM 布局跟着漂移。
    本轮实测: `sub_8052580` 在 ll.map 里是 `0x0805257c`, 而原始地址是 `0x08052580`(差 4 字节);
    拿漂移后的地址去比 baserom 会把一个**完全正确**的函数误报成 FAIL。
    正确优先级: `code.s`(原始反汇编, 永远等于 baserom) > `linker.ld`(数据符号 base+offset)
    > `ll.map`(仅兜底)。`scripts/fncheck.py` 已按此修正。
    推论: **ROM 红不等于你的函数错** —— 先 `fncheck.py <自己的函数>` 定性,
    再看 `--blame` 的首个差异地址属于谁。
    `--blame` 已能自动识别这种位移: 先采样定位主偏移量, 再把假差异剥掉 ——
    本轮实测 `差异 7381151 字节 → 真实内容差异 5997 字节`, 归属从“data.o 5.9M”
    变成可解读的 `code_1b.o 1108 / code_1c.o 257 / code_0.o 112`。
58. **目标里某寄存器第一次出现就是 RMW（`ands r2,r0` / `adds r3,r2,#1`）且从头没有被写入**
    = 源码是**未初始化的局部变量**（真 bug，不是参数）。
    判别方法: 看调用点 —— 若调用前没有给 r0-r3 赋值的动作（直接 `bl f`）, 就不是参数,
    而是原代码写了个没赋值的局部。GCC2 不删这个读也不优化它。
    改写成参数会多一条 `mov`, 预先赋值会多一条 `movs` —— 都不匹配。(案例 sub_8019E60)
59. **`do { for(...){...} } while(0);` 屏障会改变下一条指令的调度槽位**（规律25 的第二个实例）。
    sub_8019E60 里第二个循环的 `movs r1,#0` 目标位置在 `lsls r0,r0,#6` 与 `orrs r2,r0` 之间;
    不加屏障则 GCC2 把它留到 `orrs` 之后（差 4 字节）。同时需要两处“多写的临时变量”
    （`tmp = 0x400; attr &= ~tmp;` 与循环体内 `tmp = attr; map[i] = tmp;`）——
    只加其中一处仍差 4-8 字节; 把 tmp 拆成两个不同名变量也会退回 4 字节。
    这类“调度槽位”问题优先交给 permuter（本轮 base=60 → 找到 score=0）。
60. **定位 ROM 首差异时，要区分 BL 偏移和真正的代码错误**。本轮 `cmp -l` 首个差异在
    `0x080003D4`，表面上是 `sub_80002A0` 调用 `sub_805008C` 的 BL 偏移；继续比较
    `ll.map` 与 `code.s` 的函数起始地址后，发现真正原因是前序 `sub_804AC60` 少了 4 字节，
    使后续函数整体前移。排查顺序应为：`cmp -l` 找首字节 → 反汇编确认是否为 BL →
    对比该调用目标的地址 → 用 `fncheck.py` 验证调用者和被调函数，避免误改首个出现差异的函数。
61. **全局符号的有符号视图会直接改变 GCC2 的代码长度**。`gUnk_030009C5` 若声明为 `u8`，
    `sub_804AC60` 会省掉 `lsls/asrs #24`，函数短 4 字节；声明为 `s8` 后，agbcc 生成目标所需的
    `ldrb; lsls #24; asrs #24`，函数长度和后续布局恢复一致。只有一个使用点时，优先修正
    `iwram.h` 的符号类型；修改后必须检查所有引用并重新跑 `make` + SHA1，避免因类型变化影响其他函数。
62. **固定寄存器扩展不是纯 C 匹配方案**。`sub_80531A8` 的目标只与候选 C 相差
    `ptr/data` 的 `r1`/`r2` home；`register ... asm("r1")` 可以强行得到 60 字节一致，
    但这属于编译器扩展，会掩盖 GCC2 原始寄存器分配规律。项目要求保持可移植的纯 C 时，
    应保留语义正确的 C 草稿、函数状态继续标为 `[0]`，并在进度文档记录寄存器差异，
    不要为了让 SHA1 变绿而引入固定寄存器或内联汇编。
63. **头文件原型冲突要先排除并发竞态**。若 `make` 报“conflicting types”，但预处理后的声明和定义
    完全一致，可能是另一个智能体正在写共享头文件。先用 `make -B build/src/<module>.o` 单独重编，
    再运行 `fncheck.py`；确认对象可编译后才修改原型，避免把并发瞬态误当成代码错误。
64. **"两个分支结果相同"的空转 if 必须原样保留**（案例 sub_8052758）：
    目标 `cmp r0,#0; beq 尾; movs r0,#0` 且两条路径最终值完全一样 = 原代码写了
    `if (arg0 != 0) { arg0 = 0; }`。简化成 `arg0 = 0;` 只剩一条 `movs` → 少 6 字节。
    这类"语义冗余但代码存在"的形态是原作者的 bug/半成品，反编译时**不能顺手清理**。
65. **`u8` 形参永远拿不到 `x < 0` 这条指令**（案例 sub_804F0B8）。
    GCC2 对 u8 提升后的 `x < 0` 会报 "comparison is always false due to limited range"
    并**整条删除**（实测 `int f1(u8 a){ if(a<0) ... }` 直接变成 `movs r0,#0`）。
    所以目标里只要有 `cmp rX,#0; blt`，该变量就**不能**声明成 u8。
    但入口又有 `lsls rX,#0x18; lsrs rX,#0x18` 的零扩展 —— 两者共存的唯一写法是：
    **形参声明 `s32`，并在函数体第一句显式 `arg1 = (u8)arg1;`**（零扩展由这句产生）。
    实测 `s8`/`char` 形参会在每个有符号比较前多插一对 `lsls/asrs`；
    `s32 v = arg1;` 再比较则会被 cprop 把零扩展传过去、同样折叠掉 `<0`。
66. **一个函数可能需要多个 `do {} while(0)` 屏障，位置不同效果不同**（案例 sub_804F0B8）。
    该函数需要**两个**：一个包 `if (a==0 && b==0) return 0;`，另一个包三条 `arg1` 测试。
    只留后者差 7 字节（ret 落到 r4 而非 r5），只留前者差 48 字节，全去掉差 51 字节。
    屏障同时影响“寄存器 home 分配”和“`<0` 折叠是否发生”，不只是调度。
    探序方法：把“语句顺序 × 屏障位置”当成搜索空间写脚本穷举（本轮 6×4 组合命中），
    比手工试错快得多。
67. **多实参 K&R 调用: 把重复的基址表达式提成“声明处初始化”的局部指针**（案例 sub_8020974）。
    同一个 10 参调用 `sub_801B81C(...)`, 写法不同会产生两种完全不同的调度：
    - 内联写 4 次 `gUnk_08393B28[arg1].field_X` → GCC2 把地址 CSE 成 `adds r,#4` 连续递增，
      并把两个 `ldrb` 提前算好后塞进 r8/r9 → 多出一对 `push/pop {r5,r6}` + `mov r8,r3` 等 6 条指令。
    - 改成调用前先 `Unk_08393B28 *entry = &gUnk_08393B28[arg1];` → 基址先进 r4，
      4 个成员各自 `[r4] / [r4,#4] / [r4,#8] / [r4,#0xa]` **带位移独立寻址**，
      两个 `ldrb` 被推迟到 r1/r2 空出来之后，完全不碰高位寄存器 → 逐指令全等。
    判据：目标里出现 `ldr rX,[r4]; ldr rX,[r4,#4]; ldrh rX,[r4,#8]; ldrh rX,[r4,#0xa]`
    这种“同一基址 + 不同位移”= 原代码有一个结构体指针局部；
    若是 `adds r,#4` 连续递增 = 内联重复下标。另见经验 11。
    ⚠ 局部指针类型必须在函数之前：若 typedef 在文件更后面，**整块前移**即可（纯搬迁，
    不改任何函数的代码生成），别为了避开而再造一个别名 struct。
68. **`u8` 计数器的包含式上界会把截断放在循环闩上**（案例 sub_8016978）。
    `for (i = 0; i <= 0xF; i++)` 且 `i` 为 `u8` 时，GCC2 生成
    `adds r0,r1,#1; lsls r0,#0x18; lsrs r1,r0,#0x18; cmp r1,#0xF; bls loop`。
    这里比较的是截断后的新 `i`，同时命中分支的 `return i + 1` 也会生成同样的 u8 截断。
    如果目标尾部有这组 `lsls/lsrs`，不要把计数器改成 `s32`，也不要先把循环重写为
    `< 16` 后再假定代码生成等价；先保留源码的窄类型和包含式边界逐指令验证。
    本例还有两个地址池，但 `fndiff` 只报 score=400：`gInventory` 是已注册符号、候选 `.o`
    中需要重定位，ROM 表 `0x0839CFAA` 是硬编码常量。**score 不能按池数量机械估算**，
    应看 `objdump -r` 或直接以 `fncheck` 的“已施加 N 个池重定位”作为定论。
69. **连续同体的 switch case 会保留两端比较，布尔范围表达式可能被折叠**（案例 sub_801B878）。
    目标 `cmp kind,#8; bgt fallback; cmp kind,#6; blt fallback` 对应
    `switch (kind) { case 6: case 7: case 8: ...; default: ...; }`，其中 `kind` 为 `s16`。
    写成 `kind >= 6 && kind <= 8` 或 `kind > 8 || kind < 6` 时，GCC2 会规范化成
    `kind -= 6; kind <= 2`；拆成嵌套 if 又可能得到等价但不匹配的 `cmp #5; ble`。
    看到“先上界、后下界、区间内共用一个块”的形状，应优先尝试连续 case，而不是继续排列 if。
70. **调用前未改写的参数寄存器可能是在隐式转发实参**（案例 sub_801B878）。
    调用点以 `r2=sp` 传第三参数，包装函数只设置 `r0/r1` 就 `bl sub_801A884`，说明入口 `r2`
    必须原样活到 fallback 调用。若错误地把包装函数或被调函数声明成两参数，C 语义看似相同，
    但分配器会把 `arg0` 放进 r2，既覆盖真实第三实参，也产生 r2/r3 home 差异。
    应把原型写全为 `u8 f(u8 *arg0, u8 arg1, u8 *arg2)` 并显式传 `arg2`；ABI 会自然省掉
    对 r2 的重复赋值。反推原型时不仅看 callee 前紧邻的 mov，还要回溯入口寄存器是否一路未被改写。
71. **相同的入口 `u8` 截断不代表形参本身就是 `u8`**（案例 sub_8019748）。
    目标五个参数都出现 `lsls/lsrs #0x18`，但直接定义成五个 `u8` 形参会生成
    `push {r4,r5,lr}`，在索引乘 20 之后才把全局基址载入 r0，score=629。
    命中写法是五个 `u32` 形参先分别赋给五个独立 `u8` 局部，再用窄局部计算和存储；
    这样 GCC2 会把基址提前放进 r6，生成 `push {r4,r5,r6,lr}`，第 5 参数也因此从
    `[sp,#0x10]` 读取。这里局部收窄既负责截断，也改变 local-alloc 的生命周期与寄存器压力。
    受控实验还表明 `ptr = (u8 *)global + index * 0x14` 本身即可命中，不必机械拆出
    `u8 *tbl = (u8 *)global`；判断因果时一次只改变形参类型、局部变量或地址表达式中的一个因素。
72. **语义相同的 switch fall-through 与显式 return 仍会生成不同布局**（案例 sub_801B8AC）。
    case 6 调用函数后与 case 7/8 都返回 `arg1`；写成贯穿时 GCC2 会合并返回块并把 case 6 主体外置，
    比目标少 4 字节。目标在两处各有 `adds r0,r4,#0; b end`，因此 case 6 必须显式 `return arg1`，
    case 7/8 再写另一份 return。反推 switch 时要按物理重复块保留源级重复，不能只按语义合并。
97. **一个内存装载值要喂给两个用途时，把原变量声明成宽类型（u32）可让 load 直接落进它的 home 寄存器**。  *(原误编为 72, 2026-09-01 修重号)*
    案例 sub_8048818：目标为 `ldrb r2,[r0]`（formation → r2）`adds r0,r2,#0`（拷给 idx）
    `cmp r2,#0` / `subs r0,r2,#1` —— 测试和减法都**读 formation(r2)**，结果写 idx(r0)。
    写成 `u8 formation` 时 GCC2 把 load 放进临时量再**拷两份**
    （`adds r3,r0,#0; adds r2,r0,#0`，多 1 条指令，score 845）；
    改成 `u32 formation` + `u8 idx` 后 load 直接进 r2，只剩一次拷贝 → 命中。
    副作用正好对：`idx = formation`（u32→u8）无需截断，`idx = formation - 1` 才需要
    `lsls/lsrs #0x18` —— 与目标完全一致。
    与经验 71 同一类：**类型宽度是分配器输入，不只是语义标注**。
    另：本函数是接力完成的 —— 智能体 B 推到 25 分并在 progress.md 留下
    “permuter 用 new_var(u32) 分离调用实参”的线索，本条就是按该线索收尾。
    **所以挂起函数的备注要写具体（到“哪个量是什么类型”这一层），下一个人能直接接。**
73. **短路条件顺序可直接决定循环旋转，具名 ROM 符号还能阻止 `base+1` 池折叠**（案例 sub_80166A4）。
    目标在调用后先检查 `++i > 7`，未超限才读取并后增 `src`，对应
    `while (i <= 7 && (ch = *src++) != 0)`；把哨兵条件单独放在 while 顶部会生成另一种布局。
    若表基址写成硬编码 `0x08095028`，GCC2 会为首次 `src++` 另造 `0x08095029` 池；
    声明 `extern const u8 gUnk_08095028[]` 并在 linker.ld 注册绝对符号后，才会保留
    单一基址池和目标的 `adds src,#1`。
74. **`fndiff` 与 `fncheck` 测的不是同一个东西，结论冲突时以 `fncheck` 为准**。
    `fndiff.sh` 编的是 `permuter/<func>/base.c`（**候选**），`fncheck.py` 读的是
    `build/src/*.o`（**已合入 src/ 的真身**）。两者不一致 = 合进去的版本呷 permuter 里那个不一样
    （常见于：合入后又改了 src、或别人合入了旧版本、或正在迭代）。
    实测: sub_8045EB8 `fndiff=5` 但 `fncheck=FAIL` —— permuter 里 08:58 的新候选已对，
    但 src/code_1b.c 里合入的是旧版本。
    → **合入后必须跑一次 `fncheck`**（它才是对 src 的校验），别拿 fndiff 的分数当合入结论。
    ⚠ **但 `fncheck` 对仍是 `INCLUDE_ASM` 的函数会报假 OK**：它只比对 `build/*.o` 在该地址的字节，
    而 asm 占位编出来的就是原始反汇编 → 必然一致。实测 `sub_8009370`/`sub_8018E34`/`sub_804BE90`
    三个**未匹配**的挂起项 fncheck 全报 OK。所以判“是否真已匹配”要看
    `functions.tsv` 的 status 列（或 `audit.py` 的 status=1 字节核验），不能只看 fncheck。
75. **函数清单会漂移，定期跑 `python3 scripts/audit.py`**。
    它交叉核对 `functions.tsv` × `functions.tsv note 列` × `build/*.o` 字节，
    并列出各 C 文件的 mtime（10 分钟内被人改过的自动标“避开”）。
    本轮实测发现 **58 个函数已经匹配但函数清单还写“待开始”** —— 不查就是 58 个重复劳动。
    `--fix` 可自动校正（仅改已字节验证通过的行）。
76. **`x |= 0xFF` 这类“或全 1”字面量会被 GCC2 直接折叠掉整个 RMW**（案例 sub_804BE90）。
    u8 变量 `ptr[0] |= 0xFF;` 因 `ldrb` 已零扩展、值域已知 ≤0xFF，GCC2 折成
    `movs r0, #255; strb r0, [r4]` 两条。实测 5 种拼法全部折叠：
    `|=0xFF` / `=0xFF|ptr[0]` / `|=(u8)-1` / `|=~0` / `*ptr = *ptr|0xFF`。
    目标若是 `ldrb r0,[r4]; mov r1,r8; orrs r0,r1; strb r0,[r4]` 的真 RMW（且 r8 在 preheader 预置），
    **则掩码必然是一个变量**：写 `u8 mask; ... mask = 0xFF;` 放在循环内，
    GCC2 会当循环不变量提到 preheader 并因要跨 `bl` 存活而分配 r8 —— 形态自然全对。
    推广：目标里“常量先 `movs` 进寄存器再参与运算”且该寄存器是高位被调保存寄存器时，
    先想“这是个跨调用存活的变量”，而不是常量物化。
77. **经验 2（加法操作数顺序）不适用于指针加法**（案例 sub_804BE90）。
    `base + n*16` / `(u8*)((u32)n*16 + (u32)base)` / `&arr[n*16]` /
    `(u8*)((u32)base + n*16)` 四种写法得分 **完全相同**（均 65/132），
    `adds r4, r0, r1` vs `adds r4, r1, r0` 改不动 —— GCC2 对 `PLUS: pointer+int`
    做了规范化，指针固定放第二操作数。所以“乘积在左/在右”的拼法只适用于
    **纯整数加法**（如 sub_8020C58 的 `i * 0xC8 + (u32)ptr`），不要在指针地址上耗时间。
78. **结构体成员形式还会改变"可交换运算目的寄存器"的选择**（案例 sub_804C4D8，是经验 11 的新推论）。
    同一个 `x |= 0x40`：
    - `u8 *ptr; ptr[0] |= 0x40;` → GCC2 生成 `mov r0, ip; orrs r0, r1`，**目的寄存器是常量那个**；
    - `Unk_03000AE8 *entry; entry->field_0 |= 0x40;` → `adds r0, r1, #0; orrs r0, r7`，
      **目的寄存器是读回来的值**（= 目标形态）。
    机理：local-alloc 的 `combine_regs` 把 dest 与"在本 insn 死亡"的那个源操作数绑定；
    结构体形式让成员值成为独立 qty，改变了谁在此死亡。
    实测代价：非结构体的 13 种拼法（`|=CONST` / `=CONST|x` / `=x|CONST` / newval 两步 / 具名 b /
    flag 变量 / `do{}while(0)` 屏障）全停在 66~90 分，结构体形式**一击全等**。
    → **凡是 RMW 位标志的形态对不上，先换成结构体成员写法再试**，别在 `|` 的操作数顺序上穷举。
    类型冲突时不必改共享头：本地 `typedef struct {...} Unk_XXX;` +
    `(Unk_XXX *)&gUnk_XXX[idx * 16]` 转型即可（已验证字节不变）。
98. **尾部地址临时量能反向改变整个函数的寄存器 home**（案例 sub_8016758）。  *(原误编为 78, 2026-09-01 修重号)*
    当 switch 的比较链、case 顺序和指令数都已一致，却有两组稳定的寄存器互换时，
    不要只在 switch 内反复换类型。该函数把尾部单表达式 `x * 2` 提成具名
    `int xOffset` 后，入口保留的 `kind` 从 r3 归位 r1、动画 bit 从 r1 归位 r3，
    尾部也从“地址 r0 / 值 r1”归位为“地址 r1 / 值 r0”。原因是局部量改变了
    伪寄存器生命周期与分配优先级，即使它在控制流合流之后才出现，也会影响全函数分配。
    这类修改应一次只增加一个临时量，并同时观察入口和尾部，而不是只看改动附近。
79. **普通 RAM 不得用 `volatile` 当调度工具；先检查会被 DSE 删除的冗余写入**（案例 sub_801B81C）。
    `volatile` 只用于真实 IO 寄存器，不能为了阻止 GCC2 重排而加在普通对象或局部变量上。
    本例自然 setter 已逐指令相同，只是 `field_14` 的 store 被推迟跨过三个栈参数 load，score=130；
    指针局部、赋值表达式、声明顺序和 `do {} while (0)` 均无效。零分写法是在后面的 `field_10`
    写入后再次写 `field_14 = arg6`：第二次写被死存储删除，不增加目标指令，却改变调度依赖，
    使第一次 store 紧跟 arg6 的 load。遇到这种单纯调度差异，应对可疑冗余语句做消融实验；
    命中后保留并注释，不能按 C 语义“清理”掉。该规律与经验 64 的原作者冗余代码同类。
80. **固定内偏移可能被折进绝对池；异步共享字段的 volatile 必须有硬件语义证据**（案例 sub_8016E30）。
    直接反复写 `*(u32 *)(gUnk + 0x1C)` 时，GCC2 会把池变成 `gUnk+0x1C`，再为访问
    `gUnk[2/3/0xB]` 生成反向减法；先写 `u8 *state = gUnk` 才保留目标基址池和
    `[r4,#offset]`。本例的字段由串行 IRQ 路径同时访问，目标也在连续存储之间反复 `ldr`，
    因而将“指针字段本身”声明为 volatile 有真实异步语义，不违背经验 79；普通 RAM 不可照搬。
    此外，`i = 0` 与 `packet = field` 的源码顺序会直接决定调用后的 `movs`/`ldr` 顺序。
81. **相邻硬件寄存器若目标共享一个基址，就用 `REG_ADDR_*` 建寄存器块指针**（案例 sub_8016F30）。
    分别写 `REG_SIODATA8` 与 `REG_SIOCNT` 会让 GCC2 为 0x0400012A/0x04000128 各建一个池；
    目标却是一次 `ldr r2,=REG_ADDR_SIOCNT`，随后 `[r2,#2]` 和 `[r2]`。用
    `vu16 *sio = (vu16 *)REG_ADDR_SIOCNT` 既保留硬件命名，也复现共享基址。
    同函数还证明常量的局部类型会改变池加载 home：直接存 0xFEFE 会多一次 r1→r0 复制，
    先赋给 `u16 sioData` 才让 `ldr r0,=0xFEFE` 直达目标；共享零值则需独立 `u32 zero`。
82. **空 switch case 会固定比较树，即使它与 default 语义相同也不能删除**（案例 sub_801D12C）。
    外层显式 `case 4: break` 让 GCC2 先生成 `cmp #4; beq`，再分派 0..2 与 5；删掉后
    会压缩成 `0..2` 范围测试。第二个内层 switch 的 `case 0: break` 同样迫使目标生成
    `cmp #7; bgt`、`cmp #1; bge`、`b default`，删掉会改成升序范围检查并少 2 字节。
    另一个易误判点是类型：虽然值来自 `ldrb`，保存到 `s16` 后才会得到目标的 `bgt/bge`；
    用 `u8` 会变成 `bhi/bls` 并触发更激进的值域折叠。
83. **两个相同常量若只差物化顺序，可让一个恒等式延迟到 RTL 再折叠**（案例 sub_801A684）。
    本函数的语义已经完全一致，最后只差目标 `movs r1,#0; movs r0,#0` 与候选的反序。
    两个普通局部直接写 `zero8 = 0; zero16 = 0;` 时，GCC2 会把字节零传播到远处的 `strb`，
    先为两次 `strh` 生成半字零。把字节零写成无符号恒等式
    `zero8 = off0 & ~off0` 后，combine 仍把它化成 0，但保留较早的伪寄存器生成位置；
    随后的 `zero16 = 0` 因而生成第二条 `movs`，寄存器和顺序同时归位。
    该技巧不读取额外内存、不增加最终指令，也不需要 `volatile`。普通 RAM/局部变量仍禁止
    使用 `volatile` 调度；只有真实 IO 寄存器可以使用。
84. **循环比较的左右操作数位置会决定入口测试能不能常量代入**（案例 sub_80532DC）。
    目标入口测试是 `cmp r0,#0; bls`（即把 `i=0` 代入后的 `n > 0`），而底部测试是
    `cmp r7,r4; bhi`（r7=n 在左）。对应源码必须写 **`for (i = 0; n > i; i++)`**（界在左）。
    写成常见的 `i < n` 则得到 `cmp r4,r0; bcs`，combine 不会把 `i=0` 代入，差 22 字节。
    同理，循环上界若是表达式（如 `t >> 1`），先存进局部再用，否则 CSE 不成立。
85. **并发修改头文件原型会打断已匹配函数**（案例 sub_804F0B8）。
    他人把 `sub_804DD90` 从 `u8 (u8,u8)` 改回 K&R `u32 ()`（因为全原型会让另一个函数
    把 `0x6C+0x21` 折叠成 `0x8D`），导致本函数调用点少了返回值的 u8 截断（`lsls r0,#0x18`）。
    修法：不改共享原型，在**调用点显式截断** `if ((u8)sub_804DD90(a, 6) != 0)` ——
    按规律 41，只测零时 GCC2 生成 `lsls #0x18; cmp #0`（无配对 lsrs），与目标一致。
    推论：依赖“头文件原型带米截断”的匹配很脆，**调用点自己写截断更鲁棒**。
86. **`(u8)(x << 4)` 想要 `lsls #0x1c` + `lsrs #0x18`，必须先落进一个 u8 临时变量**
    （案例 sub_80140D0 / sub_8014124，半字节合并）。
    写成一条表达式 `byte = (u8)(nib << 4) | (byte & 0xF)` 时，agbcc 认为整条式子最后要
    截断，于是把 `nib << 4` 留在 SImode，改为在 `orrs` 之后补 `lsls #0x18; lsrs #0x18`
    ——指令条数相同但落点不同，差 260 分。拆成
    `hi = nib << 4; byte = hi | (byte & 0xF);` 后，u8 赋值强制在此处截断，combine 把
    `(x << 4) & 0xFF` 合成 `lsls #0x1c; lsrs #0x18`，并且因为两个操作数都已是 u8，
    末尾不再补截断，与目标逐条相同。
    推论：**目标末尾有没有 `lsls #0x18; lsrs #0x18` 就是判据** —— 有则合并结果需要截断
    （某个操作数是 int，如 `nib - 1`），没有则截断必须提前绑到某个子表达式上。
    同一函数的两条分支可以一条有、一条没有，别强行对称。
87. **同一个局部变量兼职两个无关值，可以同时买到寄存器 home 和 `adds rX, rY, #0` 拷贝**
    （案例 sub_8014084，permuter 找到）。
    目标入口是 `ldr r1, =0x03004DE4; strh r0, [r1]; movs r2, #0; ldr r4, =0x03004D60;
    adds r3, r1, #0`：计数器地址存在**两个**伪寄存器里，靠一条拷贝相连，而循环里
    `ldrb r1, [r0]` 又把 r1 抢回去装载入的字节。直觉写法（宏解引用两次）只产生一个
    地址伪寄存器，cse1 把循环内的常量并进它，于是它整段活着占住 r1，把字节挤到 r2、
    把 `i` 挤到 r3 —— 全函数指令形状全对，只差 3 个寄存器 home 加这条拷贝（660 分）。
    解法是让**一个 `u32` 变量先装地址、循环里再装载入的字节**：
    ```c
    u32 val;
    val = (u32)&gUnk_03004DE4;
    *(u16 *)val = 0;              /* 地址用途，占 r1 */
    for (...) {
        val = gUnk_03004D60[i >> 1];   /* 同一变量改装字节，r1 被复用 */
        if ((mask & val) != 0) gUnk_03004DE4++;   /* 独立常量 → 第二个伪寄存器 */
    }
    ```
    地址伪寄存器就此在入口块内死亡（归 local-alloc），循环里的计数器地址是另一个跨块
    伪寄存器（归 global-alloc），loop 把它提到 preheader、cse2 把冗余常量加载化成
    `adds r3, r1, #0`。三处寄存器 home 一次性归位。
    **这是 rule 29「变量复用」的最强形态**：不是为了省寄存器，而是为了制造伪寄存器的
    生死边界。凡是「指令全对、只差几个 home」的函数都值得试一遍。
88. **local-alloc 的 qty 优先级表只覆盖块内伪寄存器，跨块的 home 争议归 global-alloc**
    （工具：`scripts/qtydump.sh`，补丁 `scripts/patches/agbcc-qty-dump.patch`）。
    `local_alloc()` 只给**完整活在一个基本块内**的伪寄存器发号，`block_alloc()` 里
    按 `QTY_CMP_PRI = floor_log2(refs)*refs*size/life*10000` 排序（同分按 qty 号小者优先）。
    循环计数器、跨分支的指针、函数级长命值都由 global.c 分配，**不会出现在这张表里**。
    实测 sub_8014084 / sub_80531A8 的 qty 表只有 life 2-10、pri 1-6 万的短命量，
    真正决定 home 的 `i`/数组基址/载入字节一个都不在。
    所以：qty 表用来解释「同一基本块内两个临时谁先拿到低号寄存器」；
    看到跨块的 home 差异就别查它了，直接按 rule 87 改伪寄存器的生死边界。
    补丁只在 `block_alloc()` 末尾读数组打印（`qty_birth/death/n_refs` 是
    `local_alloc()` 的 alloca，`qty_order` 是 `block_alloc()` 的 alloca，
    到 `dump_local_alloc` 时全部悬空），由环境变量 `AGBCC_QTY_DUMP` 开关，
    编译成**独立**二进制 `tools/agbcc/bin/agbcc_qtydump`，构建管线始终用 `bin/agbcc`。
    已验证 10 个 C 文件的 .s 与原编译器逐字节相同。

99. **⭐⭐ 数据表的 C 声明维度会强制改变消费者的索引算术 —— 默认保持 1-D**。  *(原误编为 86, 2026-09-01 修重号)*
    把 `const u8 t[]` 改成 `const u8 t[4][4][2]` 不是"纯文档变更": 一旦消费者改写为
    `t[a][b][c]`, GCC2 会重算地址表达式, 生成完全不同的指令。实测同一语义三种写法:
    ```c
    /* A: 1-D + 预乘掩码 —— ROM 实际形态 */
    t[((attr0 >> 11) & 0x18) + ((attr1 >> 13) & 6)]      → lsr #0xb; mov #0x18; and; lsr #0xd; mov #6; and; add
    /* B: 3-D 声明 + t[shape][size][0] */
    t[(attr0 >> 14) & 3][(attr1 >> 14) & 3][0]            → lsr #0xe; lsl #0x1 / lsl #0x3,
                                                            且 attr1 先于 attr0 读, push 列表也变了
    ```
    两者指令数、顺序、寄存器分配全不同 → **必然不匹配**。
    判据: ROM 里出现 `& 0x18` / `& 6` / `& 0xC0` 这类**预乘字节偏移掩码**
    (而不是 `& 3` + 独立 `lsl`) = 原代码把表当**扁平字节数组用字节偏移索引**,
    声明就必须是 `u8[]`。
    → **逻辑维度写注释, 不要改类型**。要改类型必须先确认全部消费者已是真 C 且逐个 fncheck。
    (另见经验 21/67: 结构体成员形式同样会改变寻址形态, 同一类陷阱。)

100. **⭐⭐ 不要用 `goto` 复刻"跳过某个判断"的控制流 —— 给同一个 flag 赋值, 让 GCC2 的 jump-threading 自己生成绕过块**。  *(原误编为 87, 2026-09-01 修重号)*
    案例 `Stats_BuildSkillList` (原 sub_800A048)。ROM 的控制流是:
    ```
    cmp r0, #1
    bne _FLAGCHECK      ; 不满足 → 落到 flag 检查 (flag=0 会 continue)
    b   _ACCEPT         ; 满足   → **直接跳过** flag 检查进接受块
    _FLAGCHECK:
    adds r1, r2, #1
    cmp r5, #0
    beq _CONT
    _ACCEPT:
    ```
    看上去非 `goto` 不可。但项目禁用 goto。正确写法是把**两条路都写成 `flag = 1`**:
    ```c
    flag = 0;
    if (tbl[i*5] == 0xFF) {
        if (gPartyMemberIds[0] == 1)
            flag = 1;            /* ← 不是 goto, 也不是 continue */
    } else if (tbl[i*5] <= lvLimit)
        flag = 1;
    if (flag == 0)
        continue;
    *skills++ = i + 1;
    count++;
    ```
    GCC2 的 CSE/flow 能证明 `flag==1` 时后面的 `cmp r5,#0; beq` 必不跳，于是**自己把
    接受块线进那条路径**，包出的 `b _ACCEPT` 与 ROM 逐字节一致 (bytecmp: OK 156B)。
    实测对比 (同一函数): 用 `goto accept` 的写法残留 13 字节差且长度呷对；
    而上面的 flag 写法 **0 字节差**。→ **遇到"看起来要 goto/汇编才能凑出的控制流",
    先试把分支归约成对同一个变量的赋值**。

101. **u8 形参上的 `+1` 会生成移位域加法, 不是 `adds #1`** (经验 30 的新变体)。  *(原误编为 88, 2026-09-01 修重号)*
    `lvLimit = lv + 1;` 其中 `lv` 是 **u8 形参** 时, GCC2 先做零扩展 `lsls r1,#0x18`,
    然后把 `+1` 折进已移位的域里: `movs r0,#0x80; lsls r0,r0,#0x11` (即 1<<24) →
    `adds r1,r1,r0` → `lsrs r1,r1,#0x18`。写成 `u8 lvLimit = lv + 1;` 或先赋局部再加
    都得同样的形态; 但写成 `int` 局部或 `(u8)(lv+1)` 强转会退回 `adds #1`。→ 目标里看到
    `movs rX,#0x80; lsls rX,rX,#0x11` 就是 **1<<24**, 背下来能省一轮试探。

89. **u16→s16 的字段类型直接决定 ldrh/ldrsh, 错了整体位移**。`StaticMapObject.x/y/z` 与
    `ChestObject.x/y` 实为 **u16** (目标 `ldrh` 零扩展); 声明成 s16 会生成 `ldrsh` (符号扩展),
    单函数看着只差 2 字节, 但 `-8/+8` 的尺寸漂移让后续所有函数位移 → ROM 大面积红。
    → 见到 `ldrsh` 先查 struct 字段类型, 不要急着调 C 写法。实测×2 (2026-09-01)。

90. **被调返回值按 u8 原型读会多出 lsls 截断**。`if (Sprite_EnqueueRender(...) != 0)` 目标是
    直接 `cmp r0,#0` (无 `lsls r0,#0x18`) —— code_0.h 的 `u8` 返回原型对**个别调用点**是错的。
    解法: 本 C 文件 内声明 s32 原型 + 链接期同址别名 `Sprite_EnqueueRender_S32 = Sprite_EnqueueRender`
    (linker.ld), 不改共享头文件 (会牵连其他 C 文件 的已匹配调用点)。

91. **独立数据符号 vs 基址±偏移决定字面池布局**。目标用 `gfx=r4` 缓存 + `gfx+0x144` / `gfx-0x180`
    (movs+lsls 构造偏移) 时, C 必须写成 **同一指针变量的 ±偏移**; 换成独立符号 (gUnk_08087500 等)
    会多出 4 个池条目, 函数尺寸虽同但池位置不同 → 后续全部位移。→ Logo_LoadAssets 实测。
    反向: 目标用独立池常量时, 不要自作聪明合并成基址算术。

92. **在 C 文件 里定义"占位数组"会占 .data 导致 rom 溢出**。`u8 gUnk_08095028[][8] = {{}};` 让
    .data 长 8 字节, 链接报 `region rom overflowed by 8`。占位/前向引用一律 `extern const` +
    linker.ld 绝对符号, 数据本体留给数据区计划 (PLAN_DATA)。

93. **跨 C 文件 补原型必须同步 code_0.h 的空括号声明**。`void sub_X();` (K&R 未指定参数) 与真 C 的
    `void sub_X(u8)` 冲突 (`can't match an empty parameter name list declaration`)。
    实装函数时先 grep code_0.h, 把 `()` 升级为带参原型 —— 只影响本 C 文件 代码生成, 已匹配调用点
    无 C 引用时安全。

94. **注释块嵌套事故重演防范**: 在 `/* */` 草稿内追加 `/* ... */` 小节注释会提前闭合外层,
    后半草稿变成 live 代码 (`syntax error before /`)。改草稿前 `grep -n '/\*|\*/' <file>` 配对,
    改完立刻单文件编译。→ INCIDENTS.md 事故表 2026-09-01 再现。

95. **json.dump 整文件重写会打乱共享清单格式**。`scripts/data.json` 用 `json.dump(indent=1)`
    重写产生 4 万行假 diff。共享机器文件只做**定向字符串替换** (assert count==1) 保格式。

96. **硬件寄存器一律走 `REG_*` 宏 (volatile 类型), 真 C 与 permuter base.c 都不许用普通 u16 裸地址**。
    io.h 的宏本质是 `#define REG_BLDY (*(vu16 *)0x04000054)` —— `vu16 = volatile unsigned short` (types.h)。
    - 真 C 里写 `*(u16 *)0x04000054 = v;` (普通 u16) = **禁止**: volatile 不是装饰, 它改变代码生成
      (多次访问不被重排/合并、RMW 不被折叠、"值未用的空读"不被死代码消除), 非 volatile 版和 ROM 对不上。
    - **permuter 跑分同理**: base.c 不能 include 项目头, 必须把 io.h 的定义原样内联进去:
      ```c
      typedef volatile unsigned short vu16;
      #define REG_KEYINPUT (*(vu16 *)0x04000130)
      ```
      用普通 `u16` 定义寄存器宏, permuter 是在给**另一个程序**打分 —— 分数与真实构建脱钩 (假高分/假平台期)。
      先例: `permuter/sub_8050014/base.c`、`sub_8016F30/base.c`。
    - 宏名/地址必须从 io.h 抄 (寄存器号陷阱见 AGENTS.md §3: `0x04000054`=REG_BLDY 不是定时器);
      u16 寄存器的字节访问用 `*(vu8 *)&REG_x` (目标 ldrb/strb 时)。
    - 扫描违例: `grep -rnE '\(\s*(vu?16|u16)\s*\*\s*\)\s*0x04' src/*.c permuter/*/base.c` 应只命中 io.h 宏展开形态。详见专用规范文档 `docs/RULES_HARDWARE_IO.md`。
102. **⭐⭐ 数据符号的“拼写形式”会改变寄存器 home：强转宏 (`const_int`) ≠ extern 数组 (`symbol_ref`)**
    （案例 `Inv_FindHeldItemOnPage` / 原 sub_80169EC）。
    为了绕过未登记的 ROM 符号, 常见写法是 `#define gTable ((const u8 *)0x0839CFAA)`。
    这让 GCC2 看到一个 **`const_int` 地址常量**, 与真 `extern const u8 gTable[]` 产生的
    **`symbol_ref`** 走不同的 local-alloc 路径 —— 实测同一个函数里表基址被分到 **r2**,
    而 ROM 用的是 **r0**; 指令条数与形状全对, 只差 4 字节 (score 20)。
    改成在 `linker.ld` 登记绝对符号 + 头文件 `extern` 声明后逐字节命中 (score 20 → 0)。
    **推论**: 遇到“形状全对只差几个寄存器 home”时, 先查参与计算的地址常量是不是被写成了强转字面量;
    把旧宏换成真符号后**必须重跑 `audit.py`** —— 同文件里靠这个宏才匹配的兄弟函数可能反过来被带偏
    (实测 `Inv_FindFirstHeld`/`Inv_FindPrevHeld` 两种写法均 OK, 但不可假定普适)。
    另注意: 只有“多个 qty 抢寄存器”的函数才敏感; 单 qty 的函数两种写法同果。
     关联: 经验 87 (一个变量兼职两个值)、经验 88 (qty 优先级表只覆盖块内伪寄存器)。

103. **循环体用到的字面量 0, 先存入独立变量再参与赋值, 会让 agbcc 把 `movs rX,#0` 提到循环前 preheader 的最前面**（案例 `sub_80175C0`）。
     同一函数, 循环体 `p->field_0 = 0; p->field_2 = 0;` 直接写字面量时, 生成 `adds r4,#0x18; movs r0,#1; movs r1,#0` (零常量最后加载);
     改成 `zero = 0; p = &gSioSession.unk18[zero]; ... p->field_0 = zero; p->field_2 = zero;` 后,
     生成 `movs r1,#0; adds r4,#0x18; movs r0,#1` (零常量最先加载), 逐字节命中 (64B)。
     **推论**: 循环前 preheader 里几条独立 `movs`/`adds` 的先后顺序由“哪条指令先被数据流引用”决定;
     想指定某常量最先加载, 就把该常量当作变量先初始化、并让循环地址也经由它计算。
     关联: 经验 29 (字面池重定位误判)、经验 76 (home 互换) —— 都是“调度槽位/寄存器 home”类卡点,
     这类卡点用 permuter 探索语句顺序即可, 结构无需重排。

104. **memcpy/拷贝族函数的寄存器 home 由“参数/局部变量声明形式”决定，且“复合赋值 vs 两步赋值”改变 LSL 槽**（案例 `sub_8017640`）。
     目标形状: `adds r3,r0; adds r4,r1`（dst→r3、src→r4），字节路径 `lsls r2,r2,#2; subs r2,#1`（变换直接在 count 上做）。
     - **寄存器 home**: 形参写成 `void *dst, void *src` + 函数顶部先声明 `u8 *d; u8 *s;`（在 if 外集中声明）, prologue 才是 `dst→r3, src→r4`;
       一次性在分支内 `u8 *d = dst` 或直接用 `u8 *dst` 形参, 编译器生成反过来的 `adds r4,r0; adds r3,r1`（dst→r4、src→r3）, 全函数镜像偏移。
     - **LSL 槽**: `count = count * 4; count--;`（两句）生成 `lsls r2,r2,#2; subs r2,#1`（原地改 r2, 匹配）；
       `count = count * 4 - 1;`（一句）生成 `lsls r0,r2,#2; subs r2,r0,#1`（借用 r0, 多 2 字节）。
     - 两个分支各自循环用 `while (count != -1)` + 循环尾 `count--` 才产生 `movs r0,#1; negs r0,r0; cmp r2,r0` 的 -1 哨兵预检结构。
      **推论**: 拷贝族函数卡“寄存器镜像”时, 先试“形参 void* + 顶部集中声明指针局部变量”; 卡“差一条移位”时, 把复合赋值拆成两句。
     关联: 经验 87 (一个变量兼职两个值)、经验 103 (调度槽位)。

105. **u8 参数减 1 后当表索引时, 必须显式 `(u8)(x - 1)` 强转, 否则 agbcc 会把 -1 折叠进表基址产生错误序列**（案例 `sub_80166FC`）。
     目标序列 `subs r0,#1; lsls r0,r0,#24; lsrs r0,r0,#21`（(u8)(idx-1)<<3 的表指针计算）。
     写成 `gUnk_08095828[charId - 1]`（charId 为 u8 形参）时, 编译器把 -1 的移位量折进基址,
     生成 `lsls r1,r0,#3`（无 subs）, 整体错位 67 字节; 改成 `gUnk_08095828[(u8)(charId - 1)]`
     后逐字节命中。
     **推论**: 遇 u8 索引表达式被“聪明地”常数折叠时, 用显式 u8 回绕强转阻断折叠, 逼出 ROM 的
     subs+lsls+lsrs 三步截断序列。
     另注意同一函数里 `dest = x*2 + 0x02005800 + y*64` 的**加数书写顺序**决定 `lsls r1,r5,#1`
      (x*2 先入 r1) 还是 `lsls r2,r2,#6` (y*64 先) —— 与目标不一致就整体镜像, 顺序调整即可。
     关联: 经验 104 (寄存器 home)、经验 30 (移位域加法)。

106. **"global-alloc 域" 表基址提升进 r8 / RAM 基址循环内现取的决策, 可以被 C 结构改变, 不必改编译器**（案例 `sub_8009370`, 首个被攻破的 "global-alloc 域三连"）。
     目标形状: preheader `ldr r7,=0x0808A234; mov r8,r7`（ROM 表基址进 r8）且 `0x03000010` 循环头现取;
     循环体条件 `movs r0,#4; ands r0,r2`（AND 结果落常量寄存器 r0, b 保持 QI/subreg）。
     - **保持窄类型不落局部**: 条件必须直接写 `gUnk_03000010[i] != 0 && (gUnk_03000010[i] & 4) == 0`
       （两次直接下标访问）。若先 `b = gUnk_03000010[i]` 再 `b & 4`, GCC2 把 b zero_extend 成 SI,
       生成 `ands r2,r0`（结果落 b 寄存器, 差 2 字节）。
     - **DMA 源拆三行**: `off = (...<<5)+2; base = (u8*)gMenuEntityPaletteTable; src = (u32)(base + off);`
       再 DmaSet, 才能让 ROM 表基址提升进 r8 并保住 `adds r0,#2`; 直接写 `gMenuEntityPaletteTable + off`
       会把 +2 折进池常量（丢 adds）, 且基址不被提升。
     - **推论**: 遇挂起项 note 写着 "global-alloc 域 / 别再穷举 C 写法" 时, 仍应先穷举
       "窄类型不落局部变量" 与 "指针/偏移拆分变量" 两类结构 —— 提升决策往往由中间变量的
       存在与类型决定, 不是铁板一块。打 agbcc global.c 转储补丁 (路径 a) 是最后手段。
     关联: 经验 29 (字面池重定位)、经验 76/87/88 (寄存器 home 与 qty)、经验 104 (拷贝族 home)。

107. **s32/u16 减 1 后再按 s8 比较时, 若直接 `if ((s8)g > 0)` 编译器复用寄存器移位 (lsls rX,rX,#24) 而非重读内存 (movs+r0,ldrsb)**（案例 `sub_80188BC`）。
     目标序列: `subs r0,r2,#1; strb r0,[r1]; movs r0,#0; ldrsb r0,[r1,r0]; cmp r0,#0; bgt clear`
     —— 减后**重读内存**取带符号字节。直接写 `gUnk_03000316--; if ((s8)gUnk_03000316 > 0)`
     会被 GCC2 改成 `lsls r0,r0,#24; cmp r0,#0` (复用寄存器, 差 4 字节); 插入中间变量
     `tmp = gUnk_03000316; if ((s8)tmp > 0)` 后强制重读, 逐字节命中。
     **推论**: 想让减/改后的全局值被"当作新读"产出 ldrsb/ldrsh 序列, 就经一个局部变量中转,
     阻断寄存器数据流的 reuse; 这也是可控 "home 互换" 的一招。
     **⚠ 反例教训 (2026-09-02 踩坑)**: 本函数曾用 **readkeys/clear/tail 三标签 `goto`** 强控块布局来
     匹配目标 (readkeys 带 `b tail`, clear fall-through)。`goto` 违反铁律 4 / 经验 100 —— 经验 100 已
     证明这类"跳过块"控制流用 **flag 赋值让 GCC2 jump-threading 自生成绕过块** 同样逐字节命中且更规范。
     **正确做法**: 遇到疑似非 goto 不可的控制流, 先按经验 100 把分支归约成对同一变量的赋值
     (如 `flag = 1;`), 再交给 flow/CSE; 只有当 flag 写法也无法复刻时才考虑 goto, 且须在
     functions.tsv note 里标注"用了 goto 凑形, 待 flag 重构"。不要把 goto 当推荐手段。
     关联: 经验 104 (home 由声明形式决定)、经验 103 (调度槽位)、经验 100 (禁 goto)。
108. **`for(i=0;i<N;i++){if(x==t[i])break;} return i;` 线性查找: GCC2 把首迭代(i=0)peel 到循环外, 循环体先 `i++` 再 `cmp #N-1/bhi` 收尾**（案例 `sub_804F050`）。
     目标形状: `movs r1,#0; ldrb r3,[base]; cmp arg,r3; beq ret; loop: adds r0,r1,#1; (u8 截断 lsls/lsrs); cmp r1,#0xf; bhi ret; ldrb r0,[r1,base]; cmp arg,r0; bne loop; ret: mov r0,r1`。
     —— 即 `t[0]` 的比较被提到循环前单独做一次, 循环内从 i=1 起, 退出条件写成 `i > N-1`(`bhi`) 而非 `i >= N`; 无命中时返回 N。
     **要点**: ① 参数与 `i` 都必须是 `u8`, 才会产出 `lsls #0x18; lsrs #0x18` 字节截断 (arg0 入口也截断一次); 写成 `int i` 会丢这两条移位。
     ② 表基址用**真 extern 数组** `gInvPageItemIds[i]` 索引, 不要 `((const u8*)0x0839CFAA)[i]` 强转宏 —— 后者换寄存器分配 (同 `code_8010F10.c` 注释, 经验见 data_805769C.h)。
     ③ peel 是编译器自动做的, **不要**手写 `if(x==t[0])return 0;` 再进循环去"复刻"它, 直接写朴素 for+break 即命中。
     **变体 (案例 `sub_804EF90`)**: 界是变量 (`i < gUnk_03000DDC`) 时, peel 出的首块仍先 `ldrb count; cmp i,#0; bhs ret`, 循环体每轮**重读** count (全局不缓存); 带 `ret` 累加器 (默认 0xFF, 命中 `ret=i`) 时返回值走 r5 而非直接 r1, 但 peel/bhi 骨架不变。
     ⚠ 表若是已登记的结构体数组 (如 iwram.h `Unk_03000DEntry gUnk_03000D88[]`), **直接复用该类型**, 别在函数里再 `typedef`+`extern` 一份同名/同址符号 —— 与头文件声明冲突编译即红; 4 字节元素 → 索引算术是 `lsls r0,r2,#2`。
      关联: 经验 99 (数据表保持 1-D extern)。
109. **同一 ROM 表需要"第二种类型视图"时, 用同址别名符号声明, 绝不用 cast**（案例 `sub_8048BAC`）。
      表已按 `u8[]` 登记且被别的**已匹配**函数占用 (如 `gUnk_0839CC4C` 被 `sub_8048B88` 以 `gUnk_0839CC4C[i*4]` 用), 而本函数需要 `struct[]` 视图 (字段偏移当 ldrb displacement、且基址在分支内**早加载**) 时:
      写 `((Struct *)gArr)[i].field` → GCC2 **先算下标再取基址** → 基址晚加载进 r1、arg0 挤进 r1、`adds r0,#2` 折进下标, 三处全错 (实测连 FAIL 3 次: 直址下标 `gArr[i*4+2]` 把 +2 折进 index; 局部 `p=gArr+i*4;p[2]` 与 cast 都 base-late)。
      **正解**: linker.ld 的 SECTIONS **外**加同址别名 `gUnk_0839CC4C_entries = 0x0839CC4C;` + 本 C 文件 `extern Struct gUnk_0839CC4C_entries[];`, 用**真 extern 结构体数组** `gUnk_0839CC4C_entries[i].field` 索引 → 基址在块首 `ldr r1,=addr`、arg0 落 r2、`.field` 折成 `ldrb [r0,#off]`, 一次命中。
      关联: 经验 15 (基址池加载位置↔tbl 局部)、经验 108② (真 extern 数组 vs 强转宏换寄存器)、§7 别名符号 (同址多视图)。

110. **多个 RAM 地址复位 (各 `=0`) 时, 把其中两个写成链式赋值 `B=(C=0)` 可以消掉一个地址伪寄存器, 改变 r8/ip 的 home 争议**（案例 `sub_804AB40`, 2026-09-02）。
     目标形状: 4 个 strb 用 4 个独立高位 home (r8/sb/r7/ip) + 池序 [94A,B,C,D]; 朴素 4 行散写时
     mine 是 3 个独立 home + 池序正确但 A/D 的 home 与目标互换 (17 字节差)。
     - **链式赋值 = 地址伪寄存器压缩器**: `gX = (gY = 0)` 使 gX/gY 的地址共享同一伪寄存器
       (内层地址值是外层赋值的数据流来源), 4 散写 4 伪寄存器 → 2 散 + 1 链 3 伪寄存器。
     - **存储序 = 分配序**: strb 顺序跟随 allocno 分配顺序而非语句顺序; 全链 `D=(C=(B=(A=0)))`
       反而生成 stores=[A,B,C,D] 逆链序 — 半链形态的序最难猜, 逐个 bytecmp 穷举。
     - **do-while + 守卫** (`if (i < N) do {...} while (cnt < N)`) 才是"顶测 i/底测 count"的正确源结构,
       for/while 双条件都会多测一次; 与经验 21/108 同族 (首迭代结构由编译器拆)。
     - **终址复用常量伪寄存器**: 尾段 `&tbl[i]` 直接用真 extern 数组名 (不落 ptr 变量) 时,
       它与循环不变量共享 home, 省一条 `adds rX,rY,#0`。
     ⚠ 本案例最终 17 字节差 (9 条 home 指令) 仍未归零: 目标 home 序 [D→ip,C→r7,B→sb,A→r8]
     隐含 qty 创建序 D→C→B→A, 但能产生该序的 C 写法全都同时破坏循环体 home —— 已实测穷尽
     链/排列/划分 300+ 变体 + permuter 10 万次, 属于 global-alloc 域深层问题 (同"三连"家族)。
     关联: 经验 87 (变量兼职制造伪寄存器生死边界)、经验 106 (global-alloc 域可被 C 结构改变)、
     经验 104 (home 由声明形式决定)。

111. **同一全局在两个互斥分支里各读一次时, 分支内 `u8 x = g; if (x == N)` 局部化可打破跨分支 CSE, 改变全局-alloc 的 home 分配**（案例 `sub_80094FC`, 2026-09-02）。
     目标: `>6` 测一次、`==7`/`==2` 各自**重读**一次 (3 条 ldrb), 计数器 c 落 **r1**、state 落 r0。
     直写 `if (g > 6) {...if (g == 7)...}` 时 GCC2 把 state 的 3 次读 **CSE 成 1 个长命伪寄存器**
     (掩码结果 reg51, 用 3 次跨块) → 占掉 r1, 计数器 c 被迫落 r0 (`adds r0,#1` 就地增),
     与目标差 103 字节。把 ==7/==2 的读改成**分支内局部** `u8 s2 = gUnk_03004910; if (s2 == 7)`
     后, 分支内读是独立短命伪寄存器, CSE 不再跨块合并 → c 落 r1 (`adds r1,r0,#1`)、state 重读 r0, 逐字节命中。
     **诊断法**: `gccdump.greg` 看 `X preferences: 0` —— 该伪寄存器被硬分 r0 (案例是 c);
     冲突表里 `X conflicts: ... 0 ...` 即与硬 r0 冲突 (case 是分支内读挤掉它)。
     **判据**: 目标里"同一地址连续 3 条 ldrb"而朴素 C 只有 1 条 = 原代码存在分块读。
      注意: 这是经验 23 (勿缓存重读全局) 的反向实例 —— 不是缓存进跨块局部, 而是**分块短命局部**
      恰好阻止 CSE。两者都只为复现 GCC2 的分配行为。
      **第二次案例 (sub_8025650, 2026-09-04)**: 同一手法还破 **store→load 转发** —— 全局刚被
      `strh` 写、分支内再读时, GCC2 会把读转发成移位寄存器值 (省掉 ldrh); 分支内 `u16 c = g;`
      局部化后强制真重读。且转发被破后 subs 结果落回 r0、被紧随的位测试 `movs r0` 破坏,
      连带 global-alloc 的两个地址 home (arg0↔r7 / &g↔r6) 一次归位 —— 10 字节平台期一步清零。
      判据补充: 目标 `strh rX,[rY]; ...; ldrh rZ,[rY]` (存后跨分支重读) 而候选是
      `lsls/lsrs` 双移位 = 候选发生了转发, 用分块局部重读。
      关联: 经验 88 (跨块 home 归 global-alloc, 按伪寄存器生死边界改)、经验 87、经验 97、经验 114 (u8 形参配空原型须 K&R 定义)。

112. **switch 守卫的分式拆分让独立 load 落进 `ands` 与 `subs` 的调度空隙**（案例 `sub_800A534`, 2026-09-02）。
     目标形状: `ldrb [tbl,#8]; ands r0,r1; ldrb r2,[tbl,#6]; subs r0,#1; cmp r0,#6; bhi` ——
     val(=tbl[6]) 的装载被调度在 `ands`(取低4位) 与 `subs`(-1) 之间。
     直觉写法 `v = (tbl[8]&0xF)-1; val = tbl[6]; if (v<=6) switch(v)` 生成
     `ands; subs; ldrb`(val 装载落在 subs 之后, 差 8 字节), 且直接交换语句序
     (`val` 先) 会搅乱 tbl/val 的寄存器 home(差 24 字节)。
     **正解**: 把 `-1` 从 v 的赋值里拆到守卫与 switch 的表达式:
     ```c
     v = tbl[8] & 0xF;          /* 只有 ands */
     val = tbl[6];              /* 独立 load 落在中间 */
     if (v - 1 <= 6)            /* subs 在此处生成 */
         switch (v - 1) { ... } /* CSE 复用同一个 subs 结果 */
     ```
     调度器于是把 ldrb 塞进 ands→subs 之间, 逐字节命中。
     **配套 (val 的 home 之争)**: 若不处理, val(=tbl[6]) 的伪寄存器生命周期跨过整个
     switch (load→add), 全局分配优先级反而低于 tbl 基址 → val 落 r3/基址落 r2, 差 7 字节。
     正解 = 第一分支内 **重读** `u8 bonusVal = tbl[6];`(被 CSE 合并成 val 的副本, 不增指令),
     使 val 的首次使用提前到 switch 之前 (生命周期变短) → 优先级反超基址 → val 落 r2/基址落 r3。
     **不要用** `register` 关键字硬定 home (编译器扩展, 本项目禁), 重读语句是纯 C。
     这属于"把一条表达式按 RTL 层次拆成多语句"的调度槽位技巧, 与经验 13/25/83 同类 ——
     判定: 目标出现 `ands ...; ldrb ...; subs ...`(装载夹在掩码与减之间) 就用分式拆分。
     关联: 经验 25 (屏障定槽)、经验 103 (常量顺序由引用序决定)。

 113. **目标有跳表但所有 case 都指向同一块 (如全 `return 10`) 时, 必须每个 case 独立写语句**（案例 `sub_8009F70`, 2026-09-02）。
      写成 `case 0: case 1: ... case 10: return 10;`(合并 case 标签) 会被 GCC2 折叠成
      范围测试 `cmp #10;bgt; cmp #0;blt; movs #10`, 丢 `mov pc,r0` 跳表。
      每个 case 单独 `return 10;` 才生成 11 项跳表、全部指向同一块 —— 与 sub_8048BD0
      (≥2 个不同目标天然出跳表) 不同, 单目标全靠独立语句强出。
     判定: 目标 `lsls r0,#2; ldr r1,=表; ldr r0,[r0]; mov pc,r0` 且跳表项全同址。

 114. **K&R 旧式定义是"空形参声明 + 全原型编译"的调和剂**（案例 `sub_8009F70`, 2026-09-02）。
      头文件 `u16 f();`(空形参, 项目既有约定, 见 §7 改名警告) 下, 定义写全原型
      `u16 f(u8,u8,u8)` 触发 GCC2 `default-promotion` 冲突报错; 就算能编译, 全原型会让
      已匹配调用方的寄存器分配漂移 (sub_8048818 formation r2→r3, 差 12B)。
      **定义改用旧式**: `u16 f(a,b,c) u8 a; u8 b; u8 c; { ... }` —— 与 `()` 声明兼容,
      且生成的 u8 入口掩码 (lsls/lsrs) 与全原型完全一致 (bytecmp 216B 同)。
      关联: AGENTS.md §7「改名不得顺手改原型签名」; 判定: 改全原型后某调用方 fncheck FAIL。

 115. **移除-移位循环: `count--` 后置 + 循环界写 `count - 1`, 让截断延迟到循环后**（案例 `sub_804C8E0`, 2026-09-02）。
      目标形状 (数组移除元素后 `break`):
      ```
      adds r0, r1, #0       @ j = i  (for 初始化先于界计算)
      subs r4, #1           @ count-1 (不截断!)
      adds r3, r4, #0       @ bound = count-1 的副本 (循环界用 r3, r4 作备)
      cmp r0, r3; bge skip
      do { values[j]=values[j+1]; j=(u8)(j+1); } while (j < r4);
      lsls r0, r3; lsrs r4  @ count = (u8)count  (延迟截断写回)
      ```
      直觉写法 `count--; for (j=i; j<count; j++) values[j]=values[j+1];` 会**立即**截断
      (`subs+lsls+lsrs` 紧跟 --), 循环界直接用截断后 count, 少了 bound 副本, 且 obj 被挤到
      r7 而移位基址用 r3 —— 全部错位。
      **正解**: `for (j=i; j<count-1; j++) values[j]=values[j+1]; count--;`
      —— for-init(j=i) 先出, 界表达式 count-1 算进 r4 并复制 r3 作界, count-- 延迟到循环后
      才以 (u8) 写回; 此形态下编译器把 obj 分配 r8 (r7 要让给移位基址 `mov r7, sp`)。
       判定: 目标移位循环用 `mov r7, sp` 基址 + bound 副本 + 循环后 `lsls/lsrs` 截断。
       关联: 经验 12 (调度器吊闩), 经验 10/33/47 (伪寄存器生命周期 ↔ 写法)。

 116. **`for` 循环写成 `count > i` (界在左) 才触发 GCC 的循环旋转**（案例 `sub_804FA04`, 2026-09-02）。
      `for (i=0; i<count; i++)`(标准写法) 生成**未旋转**形态: 顶部守卫 `cmp i,count; bcs skip`,
      循环回边 `cmp i,count; bcc` (i 在左)。目标若为**旋转**形态 (guard `cmp count,#0; bls skip`
      测试界本身, 回边 `cmp count,i; bhi`, 循环体无前跳即 do-while 风格), 写成 `count > i`:
      `for (i=0; count>i; i++)` 会触发循环旋转, guard 变 `cmp count,#0` —— 逐字节命中。
      判定: 目标循环入口前有 `cmp <界>,#0; bls/bhi` 且回边是 `<界> <循环变量>` 方向。
       关联: 经验 115 (界表达式顺序), 经验 3 (分支极性)。

116. **三个及以上等权重 RAM 地址复位 (`=0`) 卡寄存器轮换时, 用 `do { X = 0; } while (0);` 屏障包住其中一个, 打破 local_alloc 的平手 tiebreak**（案例 `sub_8020B54`, 2026-09-02）。
     QTY_CMP_PRI 全等 (n_refs=2/size=4/life=30) 的三个地址伪寄存器, 平手时按 qty 号小者先拿
     r4/r5/r6, 目标却是 r5/r6/r4 轮换 —— 之前穷举 40+ 写法 (语句序/链式/指针/类型) 全撞 6B 地板。
     给**最后一条**存储加 `do { } while (0)` 屏障后, 该 qty 因屏障多包一条 insn, life 微变,
     权重不再全等, qsort 次序翻转, 三条 ldr + 三条 strb 全部归位, 逐字节命中 (fncheck OK, 60B)。
     **注意**: 屏障必须只包**最后一个** `=0` (包中间的会把中间 qty 单独拎出, 反而破坏存储序)。
     判定: 目标三条 strb 顺序 714,715,716 而 C 得 714,716,715 (链式) / r4,r5,r6 (散写) 时,
     先试"最后一个存储加 do-while 屏障", 比链式赋值更精准 —— 链式还会搅乱池序 (经验 110)。
     关联: 经验 17 (同题已解)、经验 25 (do-while 屏障定槽)、经验 110 (链式=地址伪寄存器压缩)。
     **⚠ 脆弱性 (sub_801DDB0 2026-09-04)**: 此 do-while 屏障 tiebreak 对**同一 C 文件 内新增的全局符号引用**敏感 ——
     给姊妹函数引入 `extern T gUnk_0839B2D4[]` 后, sub_8020B54 的 714/715 载入 r5/r6 互换 (agbcc local_alloc
     平手受符号表/伪寄存器计数影响)。swap 源序修 load 却破 store 序 (二者与源序耦合), 屏障只会把目标推到 r4, 无解。
     即: 改本 C 文件 任何函数前, 若涉及新增符号引用, 先 fncheck sub_8020B54 确认没被扰动。
 117. **跨块 home 争议 = global-alloc 的 `allocno_compare` 排序, 用 `-da` 的 `.flow` 转储量化**（案例 `sub_8045F10`, 2026-09-02, 未破）。
      经验 88 说"跨块的 home 归 global-alloc, 别查 qty 表", 但没说怎么算 —— 补上:
      源码 `tools/agbcc/gcc/global.c:605 allocno_compare()`, 与 QTY_CMP_PRI **同形**:
      `pri = floor_log2(n_refs) * n_refs / allocno_live_length * 10000 * size`, **降序**发号,
      同分按 allocno 号小者优先; 再由 `find_reg()` 按 r0→r15 升序取第一个不冲突的硬寄存器
      (两轮: pass0 只准复用"已被别的 allocno 占用"的槽, pass1 才准开新槽)。
      取数: `agbcc -da` 会吐 `x.i.flow`, 里面 `Register N used R times across L insns; set S times; user var`
      就是 (n_refs, live_length), 且这是 **global-alloc 前**的真值 (`-dl` 的 lreg 是分配后的, 数字会漂)。
      用法: 目标 home 与候选 home 只差一次**相邻交换**时, 先算出两者 pri, 再看要把哪个 allocno
      的 refs 抬到多少 / live 压到多少 —— 比盲改写法快一个数量级。
      实测 `sub_8045F10`: obj(3 refs/13 live)=0.231 > result(4/42)=0.190 > dirMask(2/37)=0.054
      → 候选发号 obj=r2, result=r3, dirMask=r4; 目标要 dirMask=r2, 即 pri(dirMask) 必须 > 0.231
      (refs≥5, 或 refs=4 且 live≤34)。
      ⚠ **抬 refs 这条路基本是死的**: `x = x` / `(void)x` / `x |= 0` / `x &= 0xFFFF` / `x << 0` /
      `x * 1` / 重复子表达式 `(e && e)` `(e | e)` `(e + e)` / `if (x) { x = x; }` **全部在 tree/CSE
      阶段被折掉**, flow 里 refs 纹丝不动 (与经验 88 对 local-alloc 的结论一致)。
      能抬 refs 的只有"语义上真读写该变量"的语句, 而那必然留下指令 —— 所以 `case: dirMask++;
      dirMask--;` ×2 能做到逐字节 0 分, 但那是**伪造语句**, 铁律 4 禁止, 不得合入
      (它唯一证明的是: 目标里 dirMask 这个 allocno 的 pri 确实高于 obj)。
关联: 经验 87 (一个变量兼职两个值 = 改伪寄存器生死边界)、经验 88、经验 102。
 120. **⭐ 函数末尾"多一条把值装进 r0 却没人用"的 load = 原代码有 `return <表达式>`, 被 K&R `void f();` 原型掩盖**（案例 `sub_8016E80`, 2026-09-02, 破 34B→0）。  *(原编 118, 与并发 agent 的 s8 截断条目撞号, 2026-09-02 修重号)*
      目标尾部 `ldrb r0,[r1,#2]; ldrb r2,[r1,#3]; orrs r0,r2; strb r0,[r1,#2]; ldrb r0,[r1,#3]` ——
      最后那条 `ldrb` 读出的值既不被读也不参与返回路径, 直觉上像编译器残渣, 其实是
      `return gSioState[3];` 的物化 (调用方 `bl` 后立刻重载 r0, 所以"返回值被忽略")。
      判据: **epilogue (`add sp/pop/bx`) 之前**出现一条目的寄存器为 r0、且其值在程序里
      再未被使用的 load/算术指令 → 先按"真返回值"补 `return`, 而不是当死代码删。
      配套坑: 头文件里 `void f();` 是**旧式非原型声明**, 定义写成 `u8 f(u8 *)` 不会报冲突,
      但反过来若先按 void 定义, 就永远少这一条指令 (本例差 2 条 = 4 字节)。
      同函数另两处可复用的 home 手法 (经验 87 的同一思路, 案例同):
      ① **交换双缓冲的临时量复用循环指针变量** (`packet = *(u16 **)(state+0x2C); ... *(u32 *)(state+0x28) = (u32) packet;`)
        —— 单开一个 `u32 temp` 会多出一个 allocno, BB0 的 state/temp home 整体错位 (实测差 34B→8B);
      ② **循环里换用第二个指针变量** (`st = state;` 放在 `i = 0;` 之后) 才生成目标那条
        `adds r7, r5, #0`; 全程只用 `state` 则不生成 (实测差 132B→8B)。
      ③ `i = 0;` 必须是循环外的独立语句 + `for (; i <= 1; i++)` 空 init, 否则 `movs rN,#0`
        会落到那条拷贝之后。
      ④ 收尾的 `CpuSet` 立即数 0x04000006 / 0x05000006 分别是 `CpuCopy32(src,dst,24)` 与
        `CpuFill32(0,dst,24)` 的宏展开 (后者自带 `vu32 tmp` 栈槽 = `sub sp,#4` + `str r0,[sp]`),
        别手写裸 CpuSet (经验 55)。
      关联: 经验 54 (非 void 无 return 锁死 r0 —— 本条是它的镜像)、经验 87、经验 117。
 118. **s8 返回值截断位置: `s8 tmp = result; if (tmp >= 0)` 使 `lsls/lsrs #0x18` 排在 `cmp` 之前**（案例 `sub_804F10C`, 2026-09-02）。
      目标: `call → lsls r0,#0x18; lsrs r1,#0x18; cmp r0,#0; blt; adds r7,r1,#0`
      朴素写 `if (result >= 0) { found = result; break; }` 产 `lsls r0,#0x18; cmp r0,#0; blt; lsrs r7,#0x18`
      截断 (u8)result 在 cmp 之后才出货。加 `s8 tmp = result;` 后 agbcc 把截断结果暂存 r1 (tmp 伪寄存器),
      `cmp r0,#0` 仍用原始 r0, 截断从 "if-body 内的赋值" 提前到 "tmp 的赋值" —— 因 tmp 的赋值在 if 判别之前,
      编译器把截断指令提前。同理, `int idx = values[i] * 0xC8` 把乘法从子表达式提升为独立伪寄存器,
      避免内联求值时 `pool + values[i] * 0xC8` 的乘-加序列被调度器重排。
      关联: 经验 71 (局部收窄改变生命周期)、经验 106 (中间变量决定 global-alloc)、经验 115 (截断延迟到循环后)。
 119. **SIOCNT/SIODATA8 这类"控制+数据同基址"硬件寄存器, 想要 ROM 的 `ldr rN,=0x04000128; strh rX,[rN,#2]`
      形状 (基址池字面量三处共享一个), 必须按两-u16 结构视图访问: `((SioMultiCnt *)REG_ADDR_SIOCNT)->Data`;
      io.h 分开的 `REG_SIOCNT`/`REG_SIODATA8` 各开一个地址字面量 → 池差 4B。且必须用**非 volatile** 的
      `SioMultiCnt`(types.h 已带): 换 `vSioMultiCnt`(volatile) 会把 `.Error` 读拆成半字访存, 破坏目标
      `ldr word + lsls #0x19/lsrs #0x1f` 位域提取形状 (差 137B)。配套②: 同址 RAM 的"结构化视图"应注册成
      **独立别名符号** (如 gUnk_03004DF0, 与 gSioState 同址双符号), 不要用 `#define OBJ (*(struct*)0xADDR)`
      宏 —— 宏让每处成员访问自带常量+偏移池加载, 池字面量激增 (差 160B); extern 对象统一 `ldr rN,=0xADDR; [rN,#off]`。
      另: 访问走全局对象名而非"局部指针=cast(gSioState)" 也影响分配 (局部指针版差 185B, 需对象语义)。
      （案例 `sub_8016FC0`, 2026-09-02; 参考同为 agbcc 的已匹配参考 C, 一次合入。）

 121. **⭐ 循环里的"值未用的死读" (`ldrb r0,[r0]` 结果即弃) 在 agbcc 下只有 volatile 读能保形**（案例 `sub_80454A4`, 2026-09-02, 用户拍板用 volatile 合入）。
       目标第一个循环: `if ((u8)(obj[0xAB]-7) > 1) idx = obj[0xBE];` 编译出的 `ldrb r0,[r0]`
       结果从未被用 (下一条指令就是 `adds r0,r6,#1` 覆盖 r0)。穷举 40+ 非 volatile 形态全部被 DCE 删除:
       `(void)obj[0xBE];` / `idx = obj[0xBE];`(局部死)/ `idx=..; idx^=idx` / `obj[0xBE] += 0` /
       `obj[0xBE]=obj[0xBE]` / switch 空 case / `if(x==0xFF) continue;` / 死 store `obj[0xBB]=obj[0xBE]`
       (有 strb 不符) / 局部数组 store (有 strb 不符) / 函数调用 (有 bl 不符) / 位域 / do-while 屏障——
       唯一逐字节一致的是 `idx = ((volatile u8 *)obj)[0xBE];` (bytecmp 除 6 个 bl 槽全同)。
       **推理**: 死读在 ROM 里真实存在 → 原代码必然是 volatile 读 (agbcc 唯一天然产此形态的写法),
       volatile 在此是**忠实还原原代码**, 不是经验 79 禁止的"纯调度 hack"。故本项目破例:
       当"目标含非 IO 死读"且穷举证明只有 volatile 保形时, 允许在**单点读**上加 volatile 并注明依据,
       不同于 code_80002A0.c OAM 先例 (那例 volatile 用于阻止两条 live load 的 CSE 合并, 属调度 hack, 保留 INCLUDE_ASM)。
        **判定**: ① 死读结果真的不流向任何后续指令 (看目标汇编下一指令); ② 穷举 ≥20 种非 volatile 形态确认
        DCE 全删; ③ 加 volatile 后 fncheck 全绿。三条件齐才破例, 否则仍按经验 79 保留 INCLUDE_ASM。
 122. **所有局部变量声明必须集中在函数开头 (C89 风格), 禁止块内/中途声明**（2026-09-03 全项目规范）。
        agbcc (egcs 1.1 系) 是 C89 编译器: 在 for 循环体 / if 分支内声明变量 (`u8 *obj = ...;`
        `u16 idx = ...;` 等) 会改变伪寄存器 (qty/allocno) 的创建顺序与生命周期边界, 进而改变
        global-alloc 的 home 分配和 local-alloc 的调度槽位 —— 同一语义不同声明位置, 字节结果不同。
        判定: ① 函数体内所有 `u8/u16/u32/指针` 声明一律上提到函数开头统一声明; ② 中途才需要的
        临时量也先声明 (可留空初始化); ③ 循环变量 i/j 也在开头声明。
         关联: 经验 10/33/47 (伪寄存器生命周期↔声明), 经验 106 (中间变量决定 global-alloc)。
 123. **ROM 池地址常量的物化顺序, 用 `int` 局部 (赋裸地址值) 而不是 `u8*` 局部/内联 cast 控制**（案例 `sub_803F328`, 2026-09-03）。
        目标在 `bl Bg0_InitClear` 后先 `ldr r1,=0x02035AC0`(池加载) 再 `movs r4,#2`(v=2);
        写成 `sub_80196D4(0,(u8*)0x02035AC0,...)`(内联 cast) 或 `base=(u8*)0x02035AC0;`(指针局部)
        都让 GCC 先物化 `movs r4,#2` 再加载池 → 0x4c 处 4B 逆序。
        **正解**: `int base; ... base = 0x02035AC0; ... sub_80196D4(0,(u8*)base,...)` ——
        `int` 局部赋值裸地址值, 池加载伪寄存器先于 `v=2` 的 movs 被 global-alloc 排到前面
        (经验 44 的"池加载早=用局部"镜像: 不是表基址而是任意 ROM 地址常量同样适用)。
        传入处再 `(u8*)base` cast。fncheck 284B OK。
 124. **switch 的 case 块源码顺序 = ROM 块发射顺序 (GCC2 保序), 尾块异常合并时先对齐 case 顺序**（案例 `sub_8032D74`, 2026-09-03）。
        ROM 顺序 0→19→20→6→9 的 switch 按"习惯的数值序" 0,6,9,19,20 书写时, 相距最远的
        case0/case19 各自的 `gUnk_03000820 = X; break;` 尾被跨块 tail-merge 成共享
        `strb r0,[rN]; b end` (bytecmp 155B 差); 按 ROM 序重排后每处存储就近内联, 差异立降。
        判定: 目标里同一个"写状态全局"的 strb 在多个 case 各自独立出现 (共享一个池字但
        不共享指令), 而候选把它们合并进共享尾块 → 首先检查 case 声明顺序是否与 ROM 一致。
        关联: 经验 16/37 (switch 分发), 近亲验证 sub_8042AB4/sub_80405A4 (均已匹配, 源序=ROM序)。
 125. **三目 if-conversion 的基值取 else 分支**: `cond ? A : B` (A>B) 生成 `base=B; cond真: add (A-B)`;
        要得到 `base=A; cond真: sub` 必须写成 `!cond ? B : A` 或 `cond==0 ? B : A`（案例 `sub_8032D74`, 2026-09-03）。
        目标: `ldr r2,=0x371; cmp r0,#0; bne skip; subs r2,#0xF` — 基值 0x371 是 cond(obj[0xBE]!=0) 为**真**时的值,
        即 C 写法是 `obj[0xBE] == 0 ? 0x362 : 0x371` (cond 为假取 0x362, 为真取基值 0x371)。
        写成 `obj[0xBE] != 0 ? 0x371 : 0x362` 会得 `base 0x362; beq skip; adds #0xF`, 分支极性+算术全反。
        判定: 目标是 base=大值+subs → 三目真值分支写大值; 目标是 base=小值+adds → 真值分支写小值。

 126. **单侧区间守卫写成 `if (x < lo) return` 被 agbcc 常量归约成 `cmp #0; ble` (丢 #lo); 要得到 `cmp #lo; blt` 须写成"行内单侧 switch" (case 只列区间内值)**（案例 `sub_801D19C`, 2026-09-03）。
      目标末尾: `cmp r0, #7; bgt ret; cmp r0, #1; blt ret; movs r2, #1` —— 把 1..7 视为"改 v=1", 0 与 8+ 都回 v。
      写成 `if (ab > 7) break; if (ab < 1) break; v = 1;` 时, agbcc 把 `ab < 1` 归约成 `ab <= 0` →
      `cmp r0, #0; ble` (差 2 字节); 改成嵌套 `switch (ab) { case 1..7: v = 1; }` 后才得 `cmp #7; bgt; cmp #1; blt`, 逐字节命中。
      连带的两个前置: ① 外层 if 的 >0xA 路径**不写 return** (rule 54: 无 return 锁 r0=obj, 临时量上移 r1/r2) —
         补 `return (u32)obj` 会多 r0 重载/回退; ② `ab` 声明为 `int` (有符号) 才会出 `bgt/blt`, `u8` 会出 `bhi/beq`。
       关联: 经验 54 (无 return 锁 r0)、经验 11 (结构体成员访问)、经验 16 (switch 分发形状)、经验 22。

  127. **"结构体成员两段 RMW + 读取内嵌赋值 + 链式赋值"三件套, 同时买下 zero 提前物化、subs 复用调度、diff 落 r1 三个 home**（案例 `sub_801FEBC`, 2026-09-03, 挂起→0）。
       目标 (0xB0 位段改 + 滑动参数组 0x03000618-624 写入 + 条件位 + sub_801FA10 收尾):
       ① `movs r3,#0` 出现在 `ands r0,r3` 之后(死槽)、`orrs r0,r6` 之前 —— 这是 `gUnk_...1A = 0`
         的 0 提前物化进 value 死寄存器 r3 (后用 `strh r3,[0x0300061A]`)。要复现必须:
         - **两条语句拆 RMW** (经验 13): `arg0->field_B0 = arg0->field_B0 & 0xFF0F;`
           `arg0->field_B0 = 0x20 | arg0->field_B0;`
         - **结构体成员访问** (经验 11, `mov ip,r0` 缓存 + 每成员 fresh 寻址);
         单条 `x=(x&M)|C` 或裸指针 cast 均不产生空隙。
       ② `subs r4,#0x79` (B0 指针复用出 0x37) 若写成独立语句 `p -= 0x79;` 会被调度到
         下一存储地址 `ldr r1,=...` **之前** (差 8B); 把递减**内嵌进读取赋值表达式**
         `g = *(u8 *)(p = (u16 *)((u8 *)p - 0x79));` 后 subs 才落到 `ldr` 之后紧贴 `ldrb` (0 差)。
         判定: 目标 `ldr r1,=addr; subs rX,#K; ldrb; strh` 序列 + 指针递减被复用时, 用赋值表达式内嵌。
       ③ diff 值 (0xB4 - byte) 要落 r1 且地址先置: 写成 `gUnk_03000620 = (dh = 0xB4 - *p);`
         (链式赋值, 经验 110 同思路) 才让 qty 创建序 diff→地址, home 归位 r1; 拆两行则 diff 落 r3、
         地址加载后置 (差 8B)。判定: 目标 `movs r1,#K; subs r1,r1,r0; strh r1,[r3]` + 尾部 `cmp r1,#0`。
       ④ 常数 0x20 两处使用 (B0 改 + 条件位) → agbcc 保活 r6 跨全函数, 别用变量名干扰它。
       附带: 参数用 `void *varg` + 首行 cast 局部 (经验 14), 与 code_0.h K&R/void* 声明不冲突。
        关联: 经验 13 (两段 RMW)、经验 11 (结构体成员)、经验 110 (链式赋值=地址伪寄存器压缩)、经验 117 (global-alloc home)。
128. **独立载入被 GCC2 提前填进 load-use 延迟槽时, 把依赖计算拆成"宽类型独立语句"把独立载入推回原位**（案例 `sub_80498E0`, 2026-09-03, 34B地板→0）。
       目标 `ldrb r0,[r0]`(byte) → `lsls r0,r0,#3`(byte*8) → `ldrb r1,[r3]`(frame): frame 是独立载入,
       GCC2 总把它提前填进 byte 载入的延迟槽 (得 `ldrb;ldrb;lsls`, 差 4B)。穷举 frame 局部/内联/volatile/
       独立变量名全撞 4B 地板。解法 = 把 `byte*8` 拆成**独立语句** `u16 ofs = byte * 8;` 再 `tile=tbl[ofs+frame]`:
       独立语句让 shift 紧贴 byte 载入, frame 读取(下一语句)落到 shift 之后 → 逐指令一致。
       **必须 u16/int**: 若 `u8 ofs = byte*8` 会多出 u8 截断 (`lsls#27;lsrs#24` 两条, 反增 8B)。
       判定: 目标"载入→移位→独立载入→加"序列, 而 C 得"载入→独立载入→移位→加"时, 移位拆宽类型独立语句。
        关联: 经验 33 (死 store 改 home)、经验 25 (do-while 定槽)、经验 116 (屏障破 tiebreak)。
129. **循环内常量 0 被 LICM 外提到 callee-saved 时, 让该 0 复用"call 实参寄存器变量"兼两职: 既作实参占低号 caller-saved, 又置 0 复用同寄存器**（案例 `sub_804C890`, 2026-09-03, 34B地板→0）。
       目标 `o[0xBC]=0` 的字面量 0 被 LICM 提到 r7 (多 push), 且字面量 0 触发 `subs r1,#1` 地址折叠。
       解 = 声明 `u8 t=i;` 把循环变量 i 复制进 t 作 `sub_804C8E0(obj,t)` 实参 (t 落 r1), 再 `t=0; o[0xBC]=t;`
       —— t 兼职"实参"与"零", 零复用 r1 (call 后死寄存器) 不触发外提, 且变量(非字面量)避免 subs 折叠。
       再叠经验 87: 首地址 `p=o+0xBD` 提成指针变量 (life 变长→global-alloc 优先级降→让出 r1 给零、自取 r2),
       零 life 短→优先级高→拿 r1, 逐指令命中。判定: 目标"call 后 movs rX,#0 + 该 rX 是刚死的实参寄存器"时,
       用实参变量复用置 0; 首地址寄存器被零抢占时, 地址提指针变量延长 life。
       关联: 经验 87 (变量兼职买 home)、经验 33 (死 store)、经验 88 (global-alloc life 定优先级)。

130. **基址用"命名符号(重定位)"还是"裸字面量"会同时改变调度与折叠 —— 目标基址 ldr 排在 index 计算之前时, 必须用命名符号**（案例 `Sprites_LoadMapNPCs`, 2026-09-04, 真C落地）。
       目标形如 `ldr r1,=base; <index 计算 r0>; adds r0,r0,r1; ldrb/ldr [r0,#off]` (基址先载入)。
       裸地址 `(T*)0xbase`: GCC2 把基址 ldr 排到 index **之后**, 且把 `base + (idx-1)*scale` 代数折叠成
       `(base-scale) + idx*scale`(读错槽!) —— permuter 修不动(非语句顺序)。
       命名符号 `extern T arr[]`/`extern const T arr[]`: 基址是重定位(非常量)→无法折叠→`(idx-1)*scale` 显式
       subs+lsls, 且基址 ldr 自然提前(匹配调度), 寄存器分配也对齐。判定: 目标"基址 ldr 在 index 前 + 带 -1 缩放下标"
       → 用命名符号, 别用裸地址; 草稿里 `arr[idx-1]` 保持不拆分(命名符号下本就不折叠, 拆成 `idx=idx-1` 反致 home 互换)。
       关联: 经验 32 (字面量 vs extern 改寄存器分配)、经验 7 (原型截断)。

131. **多栈参"提升进高位寄存器 + 一个溢出 [sp]"的触发器 = 用 `arr[idx].field=` 成员直写(非指针局部), 让每次访存重算地址 → CSE 出 off/base 占满 r4-r6 → 最后使用的栈参无低寄存器可用而 spill, 其余四个依次提升 r8/sb/sl/ip**（案例 `sub_80196D4`, 2026-09-04, 破解 2026-09-02 挂起）。
       症状: 9 参函数目标 prologue = `push{r4-r7,lr}; 存r8/sb/sl; sub sp,#4; ldr r4,[sp,#0x24]; str r4,[sp]; ldr r7,[sp,#0x28]; mov r8,r7; ...mov ip,r4`
       (第5参→[sp]局部槽, 第6-9参→r8/sb/sl/ip)。
       破法: **不要**写 `p = base + off; p[0]=arg5; ...` (指针局部) —— 那样 off 在 arg5 之后才出生, local-alloc 按优先级让 arg5 抢到低寄存器 r4, 提升错位。
       改写 `gDialogCtx[index].padding0[0]=arg5; gDialogCtx[index].field_8=arg3; ... gDialogCtx[index].field_10=arg1;` (每行重算 `index*20`),
       GCC2 CSE 把 `index*20`(off) 与基址符号(base) 提成两个**贯穿全函数**的 qty, 加上零常量(6 个 strb 复用)共占满 r4/r5/r6;
       于是 5 个栈参里"使用最晚"的那个(arg4, 寿命最长→优先级最低→最后分配)拿不到任何低寄存器 → spill 到 `sub sp,#4` 的局部槽,
       其余 arg5-8 依次提升进 r8/sb/sl/ip。判定: 目标"栈参提升+一个 spill"→ 把指针局部拆回成员直写, 让地址重算撑高 off/base 的 n_refs 与寿命。
       ⚠ 参数类型必须保留窄类型(u8/u16): 全 u32 会让栈参直接 `ldrb [sp,#off]` 就地读(无提升)。窄类型 ANSI 定义与 K&R 空原型 `f();` 冲突报
       "default promotion can't match empty parameter list" → 用 **K&R 旧式定义** `f(a,b) u8 a; u32 b; {...}` 保留 code_0.h 的 `void f();`(调用者无截断, 不能改原型)。
       关联: 经验 88 (global-alloc life 定优先级)、经验 106 (提升决策可被 C 结构改变)、经验 130 (命名符号防折叠)。

132. **switch 型属性 getter: 不写显式 default `return` → 默认路径 fall-through 返回 stale r0(=函数内某早先值), 这迫使该值的 home 留在 r0, 连带把后续地址计算的 temp/base/fid 挤到 r1/r2/r3 命中目标**（案例 `sub_80455A0`, 2026-09-04）。
       目标形如 `bl GetX; adds r2,r0,#0; ldr r3,=sym; ...; cmp r5,#7; bhi <epilogue>` 且 epilogue 直接 `pop;bx`(默认返回 r0=GetX 结果, 无 mov/无截断)。
       写法: `base=GetX(); ...switch(stat){case 0..7: return arr[f].fieldN;} ` **结尾不写 return**。
       - 写了 `return base;` → GCC2 在默认路加截断/`mov r0,r2`(多指令)。
       - 不写 → GCC2 把 base 的 home 定在 r0(调用返回寄存器, 因它"活到"默认出口), 地址计算另起 r1/r2/r3, index 段逐指令对齐。
       ⚠ 配套: 数组 stride 必须用**真结构尺寸**(PlayerStats=0x40→`lsls #6`); 内联草稿若按实际字段大小(0x1C)会算成 ×28 的 `lsl#3;sub;lsl#2`。
       返回类型 u16 与 u32 在"无 return 语句"时字节一致(无值可截断), 取语义类型。
       关联: 经验 97 (u32 formation 使 load 落 r2)、经验 44 (base 先物化再 movs)。


133. **"某 call 结果只用一次却需留在 r0(无拷贝) + 让紧随的全局基址 ldr 落 caller-saved 寄存器在 call 之后载入" → 把该 call 结果存进一个用后即死的独立局部变量**（案例 `sub_8045860`, 2026-09-04）。
       症状: 目标 `bl GetX; ldr r2,=globalBase; ...add rX,rX,r0...`(call 结果 r0 直接用, 全局基址在 call 后载入 caller-saved r2)。
       - 内联 `f = base[*(GetObjPool()+...)]` → GCC2 把 `ldr r4,=base` **提到 call 前**(选 callee-saved 以存活), bl 偏移差 2 字节。
       - 复用同名变量 `pool=GetX(); ...; pool=GetX()` → 首次结果被 `add r2,r0,#0` 拷贝(因 pool home 需跨后续)= 多一条指令。
       - **独立变量** `u32 p1=GetX(); f=base[*(p1+...)];`(p1 之后不再用)→ p1 留 r0 无拷贝, base 落 r2 在 call 后载入, 全段对齐。
       配套: 多个"生命周期不重叠"的同型循环计数器若目标共用一个寄存器, 就**合并成单变量**多次复位(别拆成 i/j 抢不同 home)。
       关联: 经验 132 (stale-r0 默认返回)、经验 97 (临时量类型决定 load home)。

134. **目标把某常量(如 `movs rN,#0`)塞进某 `ldr` 的延迟槽, 而标准 C 把它 hoist 到更早处 → 在该常量"应在位置"之前插一条空循环 `do{}while(0)` 作 arm_reorg 调度屏障**（案例 `sub_80457AC`, 2026-09-04, 新工具)。
       症状: 目标 `ldrh r0,[..]; movs r6,#0; strh r0,[..]` —— 零填在 ldrh 与 strh 之间(load-delay 槽); 你的等价 C 里这条零被 arm_reorg 提到更早 (如上一条 store 前), 差一截字节。
       穷举语义等价写法(三元/if/变量/块作用域/`if(0);`/标签/空语句)全不改此 hoist。
       解: 在目标零"应出现位置"对应的 C 语句**之前**插 `do {} while (0);` (或 `while(0);`/`for(;;)break;`)。空循环产生 NOTE_INSN_LOOP_BEG/END,
       改变 arm_reorg 填延迟槽的候选集, 把零压回 ldrh 槽。纯 no-op, 不动数据流/语义 (≠ 经验 18/113 的偷改数据流 hack); 先例 sub_8045EB8 的"死语句留 movs rN,#0"。
       ⚠ 仅当 fncheck 证实差异常量位置且无干净解时用; 加注释说明是调度屏障。
       关联: 经验 89 (字段类型决定 ldrh/ldrsh)、坑11 (死语句痕迹)。


## 寄存器分配定量诊断 (agbcc -dl 转储) —— 破解"怎么写都不换寄存器"类卡壳
agbcc (egcs 1.1 系) 自带 RTL 转储开关, 对定位寄存器 home 问题极其有用:

```bash
arm-none-eabi-cpp -nostdinc -I tools/agbcc/include -iquote include x.c -o x.i
tools/preproc/preproc x.i | tools/agbcc/bin/agbcc -mthumb-interwork -Wimplicit \
    -Wparentheses -O2 -fhex-asm -fprologue-bugfix -dl -o x.s
grep '^Register ' gccdump.lreg          # 每个伪寄存器: used N times across M insns / pref / dies in K places
grep '^;; Register .* in' gccdump.lreg  # 最终硬寄存器分配结果
```

其他可用字母: `-dj`(jump 后) `-dc`(combine 后) `-df`(flow 后) `-dg`(global-alloc 后)。

### QTY_CMP_PRI 模型 (源码 tools/agbcc/gcc/local-alloc.c:1435)

```
pri = (int)(((double)(floor_log2(n_refs) * n_refs * size) / (death - birth)) * 10000)
```

- 分配两轮: ①有硬寄存器建议(copy-sugg)的 qty 先试建议寄存器; ②其余按 pri 降序
- **pri 相同则 qty 号小者优先** (`qty_compare_1` 返回 `q1 - q2`) —— 参数/最先定义的局部变量 qty 号最小, 平手时它先拿寄存器
- `find_free_reg` 从 r0 往上扫, 取第一个在 [birth, death) 区间未被占用的寄存器
- `n_refs` 由 flow.c 统计, **包含 REG_EQUIV / REG_EQUAL 注释里出现的寄存器**
- 空语句 `;`、`(void)0;`、`ptr = ptr;` 这类"想刷引用数"的写法会在 CSE 阶段就消失, **不影响 n_refs**
- 推论: 想让一个长生命周期变量拿到 r1, 它的 pri 必须 ≥ 同函数内所有会抢 r1 的短命临时量。
  典型短命临时量 = 字面池加载(2 refs / 4 insns) → **pri = 5000**; 移位/加法中间量(2 refs / 3 insns) → 6666
- 实测: `n_refs=4 / life=24` → 3333 抢不过 5000; 要翻盘需 `n_refs ≥ 6`(12/24=5000 平手, qty 号小者胜)
  或 `life ≤ 16`
- ⚠ 转储文件写在**当前工作目录**(`gccdump.lreg`), 跑完记得 `rm -f gccdump.lreg`, 别污染仓库根目录
- **定位“多占一个寄存器”的具体手法**: 先在 `^Register ` 摘要里找到那个不该长命的 qty
  (看 `used N times across M insns`, M 异常大就是它), 再到同一文件的 RTL 体里
  `grep -n "reg/v:SI <N>"` 看它到底在哪条指令被多引用了一次 ——
  本轮就靠这一步发现 sub_80529B8 的尾部 `*ptr += 2` 被 CSE 复用成了 `data + 2`
  (目标其实是重新 `ldr r0,[r3]`), 即“`data` 变量本身不应当存在”。
  这比盲猜写法快得多, 也能直接告诉你该删哪个局部变量。

## 失败案例存档 (已穷举过、别再重复的方向)

### ⭐ global-alloc 域三连（sub_8009370 / sub_8018E34 / sub_804BE90）—— `qtydump.sh` 已定性，别再穷举 C 写法

2026-09-01 用 `scripts/qtydump.sh`（打过诊断补丁的 `tools/agbcc/bin/agbcc_qtydump`，
只 dump **local-alloc** 的 qty 优先级表）扫了这三个挂起项的最优候选，结论是**它们不在 local-alloc 域**：

| 函数 | 字节差(bytecmp) | local-alloc 表里的 qty | 争议值是否出现在表里 |
|---|---|---|---|
| `sub_8009370` | 78 / 216 | 16 个，**全部**分到 r0；含循环体在内的 3 个块是 **0 qty** | ✗ 表基址/数据指针是跨块长寿命值 |
| `sub_8018E34` | 37 / 152 | 17 个，最长 `life` 仅 12，只用到 r0/r1 | ✗ 尾段"表基址进 r0 还是 r1"不在表内 |
| `sub_804BE90` | 64 / 168 | 14 个，最长 `life` 仅 8；2 个块 0 qty | ✗ "表基址 vs `-1` 谁进 sl"不在表内 |

**机制（已核对 toplev.c 的 pass 顺序：local-alloc 先跑、global-alloc 后跑）**
1. local-alloc 只管**完全活在单个基本块内**的伪寄存器，从 r0 往上发寄存器；
   它拒绝 r0 的唯一原因是 `regs_live_at` 里 r0 已被标记存活 —— 而那个存活信息来自 **flow 对硬寄存器的判定**
   （入口参数寄存器、返回值寄存器、调用点），**不是来自 C 层的表达式形状**。
2. 争议的那些长寿命值（跨分支/跨调用的基址、循环不变量）由**后跑的 global-alloc** 分配，
   它只能拿 local-alloc 剩下的，所以 `qtydump.sh` 这张表里**根本看不到它们**。
3. 推论：**穷举等价 C 写法改不动这类卡点**——改的是伪寄存器图，但被改的那一层不是决策层。

**不要再重复的方向**（三个函数各自实测过，全部无效）：语句顺序/括号位置、具名临时变量、
`&`/`|` 操作数左右互换、指针局部 vs 内联表达式、`do{}while(0)` 屏障、空语句刷 n_refs、
声明处初始化 vs 分离赋值、permuter（只探索语句顺序与括号，正好是这一层）。

**⚠ 已攻破 (2026-09-05): `sub_8018E34` 不属于"改不动"的一档。** 原候选全部是**早返**
（`if(...) return ...;` 各自带一份 `ldrb + b 尾`），于是尾段 4 条路径各自的 `ldrb` 分处不同块，
表基址被 local-alloc 塞进 r0、x 被迫重读。改用**具名 `u8 ret` 变量 + if/else-if 链 + 末尾单 `return ret`**
后，4 条路径汇聚成**单个出口块**（目标 `_08018E9A` 唯一的 `ldrb r0,[r0]`），尾段基址进 r1、
x 不重读——整函数逐字节命中（fncheck OK, 116B）。这也把判定反了过来：跨块长寿命值的 home
争议仍归 global-alloc，但"单出口 vs 多出口"的块结构是 C 层可控的，会整体改变 local/global
两层的分配结果。**教训：挂起项的候选若全是早返风格，先把分支归约成"单赋值变量 + 单 return"再定性。**

**两条真正可行的下一步**
- **(a) 补 global-alloc 转储**：照 `scripts/patches/agbcc-qty-dump.patch` 的路子给
  `tools/agbcc/gcc/global.c` 打补丁，dump 每个全局伪寄存器的候选/优先级/最终硬寄存器；
  这才是这三个函数的决策层。
- **(b) 先查"硬寄存器存活"差异**（成本极低，且已被验证有效）：
  **sub_8008124 就是这么破的**（经验 54）—— 返回类型非 void 且体内无 return ⇒ 没有任何指令写 r0
  ⇒ flow 认为 r0 从入口活到结尾 ⇒ local-alloc 全程不敢用 r0 ⇒ 所有寄存器整体上移一位。
  所以遇到"目标 r0 用量异常少 / 某常量被提升进高位寄存器"这类症状，
  **先怀疑函数签名（形参个数与类型、返回类型、有无 return），而不是函数体写法**。

### sub_80531A8 —— ptr/data 的 r1↔r2 互换, 已确认非写法问题

目标指令序列已 100% 复现(含池加载位置/分支极性/合并尾存), 只剩两个 home 寄存器号互换。
已穷举的方向全部无效, **不要再试**:

| 尝试 | 结果 |
|---|---|
| 620 种等价写法(6 种表访问 × 6 种取值 × 3 种条件结构 × 声明顺序/cast/空语句) | `n_refs` 恒为 4, `life` 恒为 24, 输出 `add r2, r0, #0` 不变 |
| permuter 12k 次迭代 | 停在 score=70 (7 行纯寄存器差异) |
| 编译 flag 变体: -O1 / -O2 / -Os / -g / -fforce-mem / -fno-gcse / 去掉 -fprologue-bugfix | 全部仍为 `add r2, r0, #0` —— **不是 flag 问题** |
| `;` / `(void)0;` / `ptr = ptr;` 想刷 n_refs | 在 CSE 阶段就消失, n_refs 不变 |
| `u32 *p1 = ptr;` 别名 (能把 n_refs 抬到 5) | **触发 GCC2 CSE 误编译**: 直接吞掉 `ldr rX,[ptr]`, 把 data 当 ptr 用 —— 绝对不可用 |
| 引入 `tbl`/`ofsPtr` 局部拉长池临时量生命周期 | 池加载会被提前到 `ldrb` 之前, 指令顺序反而错 |

定量结论: 要翻盘必须 `n_refs(ptr) ≥ 6` 或 `life(ptr) ≤ 16`, 而目标自身的指令数就把 life 钉在 24。
→ 真实原始写法必然带某种我们还没识别出的 ptr 引用形式; 下一步应当给 agbcc 打补丁
打印完整 qty 优先级表(而非只看 flow 摘要), 或拿同族函数(sub_8053138/sub_805321C)
交叉比对找共性。

### sub_8049AD8 —— diff 区 int 值 home r0 + 纯拷贝到 r2, 干净 C 不可复现 (2026-09-04)

目标 diff 区 `subs r0,r0,r4 / adds r2,r0,#0 / strh r0,[r1] / lsls r0,r0,#0x10 / cmp`:
int `diff=b-a` home 在 **r0** (供 strh+cmp), 另 `adds r2,r0,#0` **纯拷贝**到 r2 供 call 第3参
(obj reload 在分支后复用 r0 → 须提前存 r2)。这是 LRA 的 live-range-split (短命 r0 + 长命 r2 拷贝)。
标准 agbcc 从等价 C **产不出**: separate store → int diff home r2 (`subs r2`, 无拷贝);
combined store `diff=(*X=b-a)` → 得 `subs r0`+`strh r0` 但赋值表达式类型 u16 → call 处
`lsrs r2,r0,#0x10` 截断而非纯拷贝 (差 6 真实字节)。**与 sub_80531A8 同族: home/拷贝是 global-alloc 决策, 非写法层。**

已穷举无效 (别再重复): ~55 变体 = {int/u16/u32/long diff} × {combined/separate/chained/comma/
u16*ptr/extern store} × {obj-var/inline obj} × {int-split/a251/inline idx} × {显式拷贝变量 save/dc/
callval/raw=reassign} × do-while 屏障; permuter 5 次 (base/t6/t1/output-10/e70 种子)。
permuter 最优 output-10 差 2 真实字节但靠 `diff=a; b-diff` 偷改数据流 (不可读, 清洗即退回 subs r2)。
候选存 `permuter/sub_8049AD8/candidates/`。下一步同 sub_80531A8: 补 global-alloc 转储看 diff 的候选/优先级,
或找能触发"短命值进 r0 + 长命拷贝进 r2 且不被 CSE 合并"的引用形式。

### sub_804621C —— 4 个跨调用值进高位寄存器 (目标 r7-r10 vs 我方 r6-r9), qtydump 已定性 0 qty (2026-09-06)

对象池找槽/过滤助手: `u8 sub_804621C(u8 *arg0, u8 *arg1, u8 arg2)`。结构全解, 语义/长度/栈帧
全对齐 (buf[0xC]=sp-12, filtered 先算, case/loop 形态正确), 剩 ~2000 分纯分配:
- **qtydump.sh 定性: local-alloc 表块块 0 qty** —— arg1/arg2/filtered/pool 四个跨 `bl sub_8045F10`
  调用的长寿命值全归 global-alloc 决策层, **C 写法改不动** (同 ⭐ global-alloc 族)。
- 症状: 目标 `mov sb,r1 / mov sl,r2 / mov r8,filtered / pool→r7` (push {r5,r6,r7} 三高),
  我方 `arg1→r8 / arg2→r9 / filtered→r7 / pool→r6` (push {r6,r7} 两高) —— 全体寄存器整体上移一位。
- 已知无效 (~30 变体, 别再重复): buf[7]/[0xC]/[12]; filtered 先/后算; pool 作 u8*/u32; 参数别名
  (o0/out/mask); 循环用 goto/while/do-while/comma-init/`i!=0xC`; 声明顺序全排列; permuter 2 轮
  (最优 ~2500 靠 `new_var3=filtered` 刷 n_refs, 仍差)。
- 附带死结: 循环 C `cmp r4,#0xc; bcc` 目标不折叠, 但 agbcc 对 `u8 i<12` 一律折叠成 `cmp #0xb; bls`
  (30+ 循环写法全折) —— 即使分配解决这 1 字节也需另找出路。
- 候选存 `permuter/sub_804621C/base.c`。下一步: 补 global-alloc 转储 (global.c 打补丁) 看四个值的
  候选/优先级, 或交叉比对同族 sub_8048458 (同 GetObjPool+sub_8045F10+0xAC 过滤结构) 的原始分配。

### sub_804DCD8 / sub_804D798 —— obj[0xC2]/obj[0xBD] 存的 addr/val 寄存器 split, 同 global-alloc 族 (2026-09-04)

姊妹族 sub_804D1B4/D260/D708/D798/DCD8 (概率判定+掉落道具)。目标把 `obj[0xC2]=0`/`obj[0xBD]=0` 这类
"存常量到 obj+off" 排成 **复用 r1 存值 + 把地址拷到 r2** (`adds r2,r1,#0; movs r1,#0; ...; strb r1,[r0]`),
matched 的 sub_804D260 也是 addr=r1/val=r0。但 straight-line 写法 (sub_804DCD8 的 `obj[0xBC]=1; obj[0xC2]=0;`)
让 agbcc 给 addr=r0/val=r2, 少那条拷贝 → 函数短 4B。命名符号/switch 折叠/零变量/交换顺序/~15 变体 + permuter
全产不出那条跨块 r1 复用拷贝 (global-alloc live-range-split, 同 sub_80531A8/sub_8049AD8 决策层)。
**⚠ 致命副作用**: status=0 函数若落地这个 4B 短版, 会把后续函数 (sub_804DD70) 整体位移 -4, 连锁改前面函数
(VBlank_UpdateGameScreen@0x080003d4) 的 bl 目标 → 全 ROM 红; 而 **fncheck 会假报 OK** (比对前缀, 漏报尺寸短)。
定论必须看 `ll.map` 函数实际尺寸/下一函数地址 或 make+SHA1, 别信 fncheck 的 "OK N bytes"。

### sub_80525E8 —— 脚本装载器的"早载存址 &E6C 落 r1"二难, 同 global-alloc 族 (2026-09-04)

> **⚠ 2026-09-07 语义更正 (证伪)**: 本条与 #113 相关段落所称 "LZ_BGM 装载" 是**错误定性** —— 该函数实为
> **脚本集装载器 (新名 `ScriptSet_Load`)**, 与 M4A 音乐无关。证据链 (解压目标=EWRAM 脚本区 0x02016000、
> 消费者全是 gScriptCursor/入口表、gCurrentSongId 实为 gEnvScriptSetId) 见 progress.md 2026-09-07 段与
> 本文件 #186。**寄存器分配/二难部分的分析仍然有效** (字节结论未变), 但所有 "songId/BGM" 字样应读作
> "setId/脚本集"。

`sub_80525E8(songId,entry,mode)` (LZ_BGM 装载, 姊妹 sub_80513A0): LZ 解压 + switch 设 `gUnk_03000E6C`。
目标 case2 (mode==2) 骨架: `ldr r1,&E6C; lsls; ldr r2,tbl; adds; ldrh; ldr r3,base; adds; str r0,[r1]` ——
**存址 &E6C 早载(0x8e)且落 r1**, 且 default/case1 与 case2 的 `str r0,[r1]` 由 cross-jump 合并成共享尾。
穷举 ~28 变体 (switch 序/scriptPtr 尾存/v 命名临时/tblv/base/np/b2/dst 指针/ydst 统一跨块指针/volatile/
冗余二次 store/do-while 屏障/-g flag) + permuter 4 轮(含 37825 迭代, 停 280)。**产物已 184B 尺寸一致**,
只剩 case2 三处 load 顺序/寄存器 (早载&r1 ↔ 晚载&r2 不可兼得)。
定量: 早载 &E6C 伪寄存器 2refs/14–16insns → pri≈5700–8500 抢不过表基址临时 (2refs/4insns → 20000),
目标需 &E6C n_refs≥5 才拿得到 r1, C 层产生不了; late 载则 &E6C 拿 r1 但 load 位置差 4 条。
→ **cross-jump 合并公共 str 的前提=两支路 store 寄存器号一致; 而"早载即低 pri"使存址抢不到首选寄存器,
存在"早载+rN 正确"不可兼得的二难**。与 sub_80531A8/sub_804DCD8 同决策层。最佳候选见
`permuter/sub_80525E8/base.c` (O1/np+b2+v) + functions.tsv note。

### 其他已确认的死路

- `old_agbcc`: 项目统一用 agbcc, 不要混用
- `objcopy --rename-section` 生成 baserom.o: 会清零内容, 用坑 5 的 `ld -r` 方案
- 裸地址访问硬件寄存器: 一律用 `include/gba/io.h` 的 `REG_*` 宏(volatile 不影响代码生成)

## 符号改名管线 (函数/数据通用) —— 数据侧批量命名必用

名字有三个独立载体, **漏一个就会链接失败或函数清单丢状态**:

| 载体 | 作用 | 是否进 git |
|---|---|---|
| `ll.cfg` | gbadisasm 的名字源, 决定 `code.s` 里所有标号与 `bl` 目标符号 | ✅ |
| ~~`functions.yaml`~~ | **已删除 (2026-09-01)**; 函数清单 = `functions.tsv` (addr 主键, status 列) | ✅ |
| `include/*.h` + `src/*.c` | 原型/定义/调用点/`INCLUDE_ASM` 行 | ✅ |
| `code.s` → `asm/{matchings,nonmatchings}/*.s` | **全部重生成, 禁止手改** | ❌ (gitignore) |

固定流程 (实测 3.8s 重生成 + 完全可复现):

```bash
# 0) 先确认重生成是幂等的 (没改过 ll.cfg 时应该一字不差)
tools/gbadisasm/gbadisasm baserom.gba -c ll.cfg > /tmp/code.s.new && cmp code.s /tmp/code.s.new

# 1) 机械改名 (函数名出现在 ll.cfg / 头文件 / src 的 INCLUDE_ASM 行; functions.tsv 不用改,
#    gen_asm 按 addr 从 ll.cfg 取当前名, --sync 回写缓存列; 推荐直接用 scripts/rename_fn.sh)
sed -i 's/\bOldName\b/NewName/g' ll.cfg include/code_0.h src/*.c

# 2) 重生成反汇编 + 重切 asm 目录 (bl 目标会跟着换, 无需手改任何 .s)
tools/gbadisasm/gbadisasm baserom.gba -c ll.cfg > code.s
python3 scripts/gen_asm.py --sync

# 3) 数据符号还要改 iwram.h/ewram.h + linker.ld 的偏移行
#    (linker.ld 里是 `. = 0x0000XXXX; sym = .;`, 按地址排序原地改名即可, 不要移动行)

# 3b) ✅ Makefile 已补上 INCLUDE_ASM → asm/*.s 的依赖 (2026-09-01), 改名后会自动重编
#     引用方 C 文件。以前需要 `touch src/*.c`, 现在不需要了。
#     (回归测试: touch asm/matchings/<某函数>.s → 应看到它所在 C 文件 重新 agbcc)
#     ✔ `gen_asm.py` 是**增量**的: 只有内容变化的 .s 才被 touch, 改名后只重编真正受影响的 C 文件。

# 4) 刷新 m2c 上下文 + 终验
make ctx && timeout 900 make 2>&1 | tail -3 && python3 scripts/audit.py
```

**为什么安全**: `asm/*.s` 里的 `bl` 全部已被符号化 (实测 code.s 6360 个 `bl` 零个数字目标),
所以改名后重切就自动一致。数据符号则根本不会被按名字引用 —— gbadisasm 把数据地址写成
`.4byte 0x080576D0` 硬码, **所以数据符号改名永远不影响 asm**, 只影响 C 侧。

**两个坑**:
1. 改函数名时若该函数在 `src/` 里已是真 C, `gen_asm.py` 仍会生成 `asm/matchings/<新名>.s`
   (没人 include), 无害; TSV 以 addr 为主键, **改名不会丢状态** (旧 yaml 按名索引的孤儿问题已根除)。
2. **改局部变量名安全, 改声明顺序/个数不安全** (规 10/33/47: 伪寄存器生命周期会变)。
   只改拼写、加注释 → 字节不变; 拆合并语句 → 必须 fncheck 定性。

## 硬件寄存器操作经验 (硬件语义硬约束 + 代码生成经验混合)

> 注: 本节里**关于 volatile / 走 REG_* 宏 / 不写裸地址**的部分, 与 AGENTS.md §3 一致, 属于**代码生成上
> 有硬约束的**(违反会直接产生不同字节) —— 那几处「禁止/必须」是描述**代码怎么写才能匹配**, 不是
> 「你必须遵守的规矩」。其他条目是踩坑记录, 属于经验性提示。

- **寄存器访问一律用 `include/gba/io.h` 的 `REG_*` 宏**, 禁止裸地址:
  ```c
  REG_DISPCNT |= DISPCNT_BG0_ON;   // ✓ (*(vu16*)0x04000000, volatile)
  *(u16 *)0x04000000 |= 0x100;     // ✗ 裸地址
  ```
- 位标志用 io.h 的 `DISPCNT_*` / `BGCNT_*` / `DISPSTAT_*` 常量 (如 DISPCNT_BG0_ON=0x100,
  BGCNT_SCREENBASE(n), BGCNT_CHARBASE(n), BGCNT_WRAP=0x2000)
- volatile 限定符(宏自带)与裸指针代码生成**完全一致**(已验证 sub_8019148), 无需担心
- 非 I/O 的数据地址(EWRAM 缓冲 0x02xxxxxx / VRAM 图形数据 0x06007000 等)不是寄存器,
  若 defines.h 无对应宏则保留字面量
- 已知寄存器: REG_DISPCNT=0x04000000 (DISPCNT_BG0_ON=0x0100 开 BG0, OBJ_ON=0x1000),
  REG_BG0CNT=0x04000008 (bit0-1 优先级, bit2-3 CBB 字符块, bit4-5 SBB 屏幕块,
  bit6 256色, bit7 尺寸, bit13 wraparound)
- **DMA 通道号别凭记忆写**: 0x040000B0=DMA0SAD (DMA0 首通道!), DMA3SAD=0x040000D4。
  实测踩坑: sub_8008978 用 DMA0, 误写 REG_DMA3SAD 导致池值变 0x040000D4 (SHA1 差 1 字节)
- **遇到 `0x04xxxxxx` 先查 `include/gba/io.h` 的偏移表再猜功能**, 别凭数量级臆想。
  典型误判: `0x04000130` 很容易当成定时器(定时器在 0x100-0x10E), 实际是 **REG_KEYINPUT**。
  旁边的 `mvns` 就是铁证 —— 按键是低有效, 原代码必定写 `~REG_KEYINPUT` 取反成按键掩码;
  定时器取反没意义。配套形状: `bics r0, r3` = `keys & ~旧值` → 新按下边沿检测的标准三件套
  (新边沿存 F2E、现状存 F2C、然后存后读回当实参)。→ sub_8050014
- u16 寄存器的字节读取: `(u8)REG_VCOUNT` 会生成 ldrh+截断(3条), 目标常是 ldrb(1条) ——
  用 `*(u8 *)&REG_VCOUNT` 或裸字节指针; 参照目标汇编选择
- volatile 读的死读取语句(`dma[2];`)可用于对齐"无条件寄存器回读"的目标形态
  —— 且这形态就是项目自带的 **`DmaSet` 宏** (include/gba/macro.h: DmaSetUnchecked 的
  `dmaRegs[0/1/2] = ...; dmaRegs[2];` 展开)。DMA 设置代码一律用
  `DmaSet(通道, src, dest, control)`, 不要手写寄存器数组
- **DMA 通道与地址陷阱**: REG_ADDR_TM0CNT_L = 0x04000100 (定时器在 0x100 段)!
  0x04000040 不是任何标准寄存器 (io.h 无宏) → 目标 DAD 写 0x04000040 时只能用字面量。
  教训: 宏名的语义不能想当然, 必须 grep 核对 REG_OFFSET_* 的实际值

## linker.ld 符号注册

- IWRAM 内偏移: `. = 0x00000XXX; gUnk_03000XXX = .;` (在 iwram section 内, 按地址排序插入,
  **插错位置会报 cannot move location counter backwards**)
- 新增符号后链接报 undefined reference = 忘了在 linker.ld 注册 (IWARM 偏移或 SECTIONS 外绝对符号二选一)
- ROM 绝对地址表: `gUnk_08XXXXXX = 0x08XXXXXX;` (**必须在 `SECTIONS {}` 外面**)
- 同一地址需要不同类型视图时用别名: `gUnk_03000730_arr = 0x03000730;` (同样放 SECTIONS 外)

## 踩过的坑

### 1. GCC2 跨函数状态泄漏 (最重要!)

同一 C 文件内, **使用 r8/sb/sl (高位寄存器) 的函数会改变后续所有函数的寄存器分配**。

症状: 新函数单测 score=0, 合入后 make SHA1 失败, 但差异在**别的函数**。
定位: `cmp -l ll.gba baserom.gba` 找差异字节 → 查 ll.map 归属函数。

解法: **拆分 C 文件**。已拆两次: `src/code_1.c` + `src/code_1b.c` (在 sub_8020CC4 后),
`src/code_1b.c` + `src/code_1c.c` (在 sub_804F0B8 stub 处), linker.ld 中按顺序拼接。
以后合入使用 r8/sb/sl 或间接调用 (_call_via_rX) 的函数时, 如 SHA1 失败且差异在别处, 继续拆文件。
拆分锚点必须是 INCLUDE_ASM stub 行 (保持 ROM 顺序), includes 块照搬到新文件头部。

### 2. linker.ld 别名符号放错位置

别名符号放在 section 内部会让重定位叠加段基址
(0x03000730 → 0x06000730, 池值被污染, ROM 只差 1 字节)。
**所有绝对赋值符号一律放 SECTIONS {} 外面**。

### 3. 头文件旧声明 `void func()`

遇到带 u8/u16 形参的真定义会报 conflicting types。
按反汇编证据升级为全原型 (参数类型从调用方代码生成推断)。
同时 functions.tsv 的 status 才不会放错目录。

### 4. 编辑失误: 保留旧 INCLUDE_ASM 行

替换 INCLUDE_ASM 为真 C 时容易把原行留在新串里 → 链接期 symbol already defined。
合入前 grep 确认: `grep -n "<func>" src/*.c` 应只有一处定义。

### 5. objcopy --rename-section 清零内容 (本机 binutils 怪癖)

baserom.o 的正确生成方式 (不用 objcopy rename):

```bash
arm-none-eabi-objcopy -I binary -O elf32-littlearm baserom.gba /tmp/b1.o
printf 'SECTIONS { .text 0x08000000 : { *(.data) } }\n' > /tmp/wrap.ld
arm-none-eabi-ld -r -T /tmp/wrap.ld /tmp/b1.o -o baserom.o
```

(b1.o 的 .data 已含正确内容; 若直接 objcopy rename 或调整地址, 内容会被清零)

### 6. permuter 用法细节

- `-j` 必须带参数: `-j 4`
- base.c 不能 include 项目头文件 (permuter 用系统 cpp -nostdinc), 类型 typedef 内联
- 参考目标文件名必须固定: base.c / target.o / compile.sh / settings.toml
- compile.sh 必须可执行
- 临时变量/结构体大小错了会白跑很久, 先用 diff.py -o 确认基本形状再开 permuter

### 7. setup.sh 相关

- `scripts/gen_asm.py` / `tsv_init.py` 无第三方依赖, 系统 python3 即可 (旧 split_asm 的 pyyaml 依赖已随 yaml 函数清单一起移除)
- `data/raw_data/*.bin` 由 `scripts/dumpraw.py` 从 baserom.gba 提取 (2026-09-01 起 **setup.sh 已自动跑**;
  之前不跑, 新克隆 `make` 必报 `Failed to open "data/raw_data/byte_XXXX.bin"` —— `src/data_805769C.c` 等三处 INCBIN)
- **`scripts/fndiff.sh` 硬编码 `.venv/bin/python`**, 所以 `.venv` 是必需品而不是可选项; 但函数清单侧脚本
  (gen_asm/audit/fncheck/tsv_init/gen_reports) 系统 python3 就够。asm-differ 要 `colorama watchdog
  Levenshtein cxxfilt`, permuter 要 `toml`, m2c 只要 `graphviz` —— setup.sh 现在自建 venv 并装这些
  (`--skip-venv` 可跳)。**pip 失败只警告不中断**, 因为 make 不依赖它。
- ⚠ 改 setup.sh 的 agbcc 段时注意: `tools/agbcc/install.sh` **无条件** cp `agbcc`/`old_agbcc`/`agbcc_arm`,
  所以 `./build.sh || true` 这种"容忍 agbcc_arm 失败"的写法是假的 —— 会死在 install.sh 上且报错指不到真因。
  现在 build.sh 失败即停并打 `.scratch/agbcc-build.log` 尾部; 另外 `old_agbcc` 也要查 (m4a/m4a_tables/agb_sram 用它)。
- functions.tsv 的 status 与 src 中 INCLUDE_ASM 目录引用必须一致 (推导依据见 `tsv_init.py`, 怀疑漂移时重跑它交叉核对:
  引用 "asm/matchings" 的函数 yaml 必须是 [1], 否则 split 后文件缺失编译失败

### 8. 旧 ll.gba 让 sha1sum 误报绿灯 (本轮实际踩到)

`make` 如果**编译阶段就失败**(例如头文件原型与真 C 冲突), 它不会重写 `ll.gba`,
磁盘上留下的是上一次成功构建的产物 —— 此时单独跑 `sha1sum -c ll.sha1` 仍然报"成功",
很容易误判为"基线是绿的"。本轮接手时 `src/code_1.c` 就因为
`sub_8020A0C`/`sub_8020A7C`/`sub_8045F10` 三个旧 `void ()` 声明编译不过,
而 ll.gba 显示零字节差。

→ **开工先 `make` 看尾部有没有报错**, 不要只 `sha1sum -c`。
→ 修真 C 时同步升级头文件原型(按反汇编证据定参数/返回类型), 改完再 make。

### 9. 单函数 .o 对比时字面池未重定位 (score 假高)

见经验 29。典型现象: diff.py 逐行全绿但 score=400~600,
差异行集中在池常量地址(被 objdump 解码成 `movs r0, r0` 之类的假指令)。
用部分链接 + cmp 一次就能定性, 别去调写法。

→ **permuter 想让老实的 score=0 也走同一条路**: 直接在 `permuter/<fn>/compile.sh`
  末尾 (`.align 2, 0` 之后) 给函数用到的每个绝对符号追加
  `printf ".equ gUnk_0300XXX, 0x0300XXXX\n" >> "$T/f.s"`, 汇编器会把它化成绝对地址,
  字面池就从 `R_ARM_ABS32 + 符号名` 变成 `.word 0x...`, 与 target.o 的池按键一致,
  permuter 即可真实到 0 (实测 sub_804DE8C, 2026-09-04)。注意此招只为给 permuter
  打分清零, **合入 src 仍用真 extern**, 最终定性仍以 fncheck/bytecmp 为准。

### 10. 分节注释 `/* ==== 标题 ====` 漏写 `*/` → 吞掉紧跟的那一行 extern

`iwram.h` 里追加的 `/* ==== 视口/摄像机滚动 ... ====` 没有闭合, C 注释在**下一个** `*/`
处才结束 —— 也就是下一行 `extern s16 gCameraMinY;        /* 0x0300464C ... */` 的**行尾注释符**
把它关掉, 于是 `extern` 声明整行被注释掉, 使用者报 `'gCameraMinY' undeclared`。
症状: 只有紧跟分节标题的**第一行**声明消失, 后面的都正常 → 极易误判成"谁删了声明"。

→ 分节标题一律写 `/* ==== 标题 ==== */`; 排查用注释配对扫描:
```bash
python3 -c "import glob
for f in glob.glob('include/*.h')+glob.glob('src/*.c'):
    s=open(f,errors='replace').read(); i=0
    while True:
        a=s.find('/*',i)
        if a<0: break
        b=s.find('*/',a+2)
        if b<0: print(f,'UNTERMINATED line',s[:a].count(chr(10))+1); break
        i=b+2"
```
→ 同一坑的另一面: 往已被 `#include` 的头文件里搬 typedef/extern 时, 必须**同时删掉** .c 里的
本地重复声明 (agbcc 对 typedef 重复报 `conflicting types` 且是**错误**), 且要逐字段核对
两边布局一致再删, 否则改了类型 = 改了 codegen。

### 11. 目标里多出一个"死" callee-saved 初始化 (`movs r7, #0` 且全函数不再读 r7)

全 ROM 只有两例 (sub_8045EB8 / ChestObjects_LoadForMap)。egcs 会删掉"写了从没读"的
局部变量初始化 (实测 `u8 x = 0;` 完全不用 / `x++` / `if (x) {}` / `volatile u8 x = 0;` /
结构体局部 `s.a = 0;`+`if (s.a)` **全部被删**, 一个都不留), 所以这个 `movs r7, #0`
**必然对应源码里真实存在的一次读**, 只是那次读被 CSE 折成了零指令:

    *flags |= extra;      /* extra 恒为 0 → OR 0 被折叠, RMW 整条消失 */

关键在**顺序**: CSE 折叠读点发生在 flow 算活跃性**之后**, flow 仍把 `extra` 记成 live,
于是定义它的 `movs r7, #0` 活了下来, 并逼出第 4 个 callee-saved 寄存器 (push 变
`{r4,r5,r6,r7,lr}`)。同理 `*flags += x;`、`*flags = *flags | x;` 这类"值恒等"的
读点都可以当吊闩用 (与经验 89 的"故意保留死代码"是一对反向技巧: 89 是**造**一条假读,
这里是**留**一条真读到被折叠的表达式上)。

排查配方 (sub_8045EB8 实测, 88B 全绿):
1. 先按语义写最小版 (基址指针 + `u8 i = 0` + 目标原型), 确认只差 push/pop 与那条 `movs rN,#0`;
2. 补一个末位声明的局部 (`u8 extra = 0;`) + 函数末尾一条恒等读 → 一次命中;
3. 声明顺序 = 伪寄存器顺序: 目标里 `p, i, flags, extra` 分别落 r6/r5/r4/r7,
   把 extra 提前会整体错位 (见经验 51/54 的 home 讨论)。

## 工具与命令补充 (速查见 AGENTS.md §4)
| 工具 | 位置 | 用途 |
|---|---|---|
| m2c | tools/m2c (submodule) | asm→C 初转 |
| asm-differ | tools/asm-differ (submodule) | 汇编 diff (单函数 -o / 全 ROM -e) |
| decomp-permuter | tools/decomp-permuter (submodule) | 置换搜索压分 |
| diff_settings.py | 项目根 | asm-differ 配置 (base=baserom.o, my=ll.elf) |
| `agbcc -dl` | tools/agbcc/bin | **RTL/寄存器分配转储** → `gccdump.lreg` (查 qty 优先级与 home 寄存器) |
| `permuter/try.sh` | permuter/ | `./permuter/try.sh <func> <file.c>` → 编译 + 报 diff.py score |
| `.scratch/abs.ld` 部分链接法 | 临时 | 排除未重定位字面池假象, 做单函数字节级终验 (经验 29) |
| .venv | 项目根 | python 环境 (yaml/pycparser/graphviz/colorama/levenshtein/toml/pynacl) |
| **scripts/fndiff.sh** | 项目根 | **单函数回环**: 单独编 <func>.o → dump 汇编 → `tools/asm-differ` 对 `asm/{non,}matchings/<func>.s`; `--promote` 固化胜出版本 (经验 52) |
| **scripts/fncheck.py** | 项目根 | **单函数字节级定论**(自动施加池重定位+排除 bl 槽, 不需整 ROM 绿) + `--blame` 把 ROM 差异归属到 .o。<br>2026-09-01 修了三个假阴性: ① asm-match 函数无 `.size` 属性被误报 NOT BUILT (现用同节下一个符号/节大小定长);<br>② 指向段符号 `.text` 的 R_ARM_ABS32 报未解析 (现用 `函数ROM地址 - 节内偏移` 推段基址, 不取会漂移的 ll.map);<br>③ 无缓存导致慢。现 **588/588 已匹配函数全部可验**, 0 假 FAIL |
| **scripts/bytecmp.sh** | 项目根 | **候选级**字节判定: 编 `permuter/<func>/*.c` → 部分链接(施加池重定位) → 与 target.o 逐字节 cmp。<br>补上 fndiff(形状) 与 fncheck(已合入真身) 之间的空档。⚠ fndiff 的 score 会假阳性<br>(实测一个少一条 `ldrh` 的破代码也报 score=400), 候选阶段必须走 bytecmp |
| **scripts/typecov.py** | 项目根 | 逐函数变量类型解析, 把字段访问归到**真实结构体** —— 解决"按字段名 grep 被多个结构体同名字段污染" |
| **scripts/rename_scoped.py** | 项目根 | 按类型限定地改结构体字段名, 不碰其它结构体的同名字段; 编译器会反过来验证解析对不对 |
| **scripts/claim.sh** | 项目根 | **函数认领锁**(原子, 防两人做同一个) + `--list`/`--table` |

## 快速命令参考

```bash
# 环境激活 (fish)
source .venv/bin/activate.fish

# 多智能体协作三件套
export DECOMP_AGENT=B
scripts/claim.sh <func>                            # 认领 (失败则换一个)
make build/src/<你的文件>.o && python3 scripts/fncheck.py <func>   # 单函数自证
python3 scripts/fncheck.py --blame                 # ROM 红了看是谁的 .o
scripts/claim.sh --release <func>

# 全量构建+验证
make

# 重新提取 raw_data (换 baserom 后)
.venv/bin/python scripts/dumpraw.py

# 重新生成 asm 目录 (functions.tsv 改后; 增量, 幂等)
python3 scripts/gen_asm.py
# 从 src 重新推导函数清单 (交叉验证/初始化)
python3 scripts/tsv_init.py

# 重新生成 m2c 上下文 (头文件改后)
make ctx

# 找某函数地址
grep "<func>" ll.map

# 找 ROM 差异字节
cmp -l ll.gba baserom.gba

# 按 nonmatching 函数大小排序挑软柿子
for f in asm/nonmatchings/*.s; do echo "$(wc -l < $f) $f"; done | sort -n | head

# 单函数 .o 字节级终验 (排除未重定位字面池假象, 经验 29)
./permuter/<func>/compile.sh <候选.c> x .scratch/t.o
printf 'SECTIONS { .text 0 : { *(.text) } }\ngUnk_XXX = 0x03XXXXXX;\nsub_YYY = 0x08XXXXXX;\n' > .scratch/abs.ld
arm-none-eabi-ld -T .scratch/abs.ld -o .scratch/linked.o .scratch/t.o
arm-none-eabi-objcopy -O binary --only-section=.text .scratch/linked.o .scratch/mine.bin
arm-none-eabi-objcopy -O binary --only-section=.text permuter/<func>/target.o .scratch/tgt.bin
cmp -l .scratch/mine.bin .scratch/tgt.bin     # 只剩 bl 槽 = 已匹配

# 查寄存器分配为何不给某变量 (破解 home 问题)
arm-none-eabi-cpp -nostdinc -I tools/agbcc/include -iquote include <file>.c -o .scratch/x.i
tools/preproc/preproc .scratch/x.i | tools/agbcc/bin/agbcc -mthumb-interwork -Wimplicit \
    -Wparentheses -O2 -fhex-asm -fprologue-bugfix -dl -o .scratch/x.s
grep '^Register ' gccdump.lreg; grep '^;; Register .* in' gccdump.lreg; rm -f gccdump.lreg
```

135. **尾部多 store 的"常量+拷贝中转"链: orr 结果先落 int 临时再 store, 且最后一个 0 与 orr 常量共用 r3** —— `moveBits(u16)=0x200|y; walkOfs=0; stateFlags(int)=moveBits; *(u16*)(obj+0xB0)=stateFlags; obj[0xA8]=walkOfs;` 让 GCC2 把 0x200-常量伪 (r3, 死于 adds r0,r3 拷贝) 的颜色让给 0-pseudo (def 落 b8, 被 r3-read@b6 反依赖挡住), orr 累加器=常量拷贝侧 r0 (const-as-accumulator)。
   若 moveBits 直 store (无 int 中转), 0-pseudo 恒落 r1@ba (差 4B); 若 0x200 临时复用 u32 zero, orr 翻成 `orrs r1,r0` (dest=y 侧)。
   第 5 实参 0 与末尾 `obj[0xA8]=0` 的 0 必须是**两个独立变量** (同一变量则 strb 源复用 arg5 的 r7)。
   （案例 `sub_8046060`, 2026-09-04, permuter 破局: 在手工 u1.c 上加 `stateFlags=moveBits` 中转一句 → 70→0; 姊妹 sub_8045F94 同族仅差 CBA4 第2实参 0xA/4, 2026-09-04 gpnux 用同一模板直接得分 0, 无需再做 permuter 探索 — 同族先例成立时直接套用）

136. **fndiff 中 `ldrb r1,[r0]` (载入值进 r1 而非常规 r0) + 后续 `adds r0,r1,#0` 拷回 → 载入值还要作第 2/后续实参用, 不是纯 home 偏好**: 先查调用点语义 (该值是否兼作下一 bl 的实参), 别急着当寄存器分配死磕。（案例 `sub_804666C` 第三循环 `sub_80466F0(obj, indices[j])`, 2026-09-04; 曾被误判为 2B home 差挂起 2 天）

137. **u16 回绕减法: 目标把 `arg1-2` 编成 `ldr rX,=0xFFFE; adds rY,rA,rX; lsls#16; lsrs#16` (加补码+掩码) 而非 `subs #2` → 源必须写 `(u16)(arg1 + 0xFFFE)`**, 写 `(u16)(arg1 - 2)` 恒出 subs+掩码 (差 4B)。probe 验证: `(u16)(arg1-2)` → `subs r0,r1,#2;lsls;lsrs`; `(u16)(arg1+0xFFFE)` → 目标形状。同类 `(arg1 + 0xFFFE) != x` 直接比较 (无掩码) 又是另一形状, 必须带 (u16) 强转。改错 30 分钟级别卡点, 先试 +0xFFFE。
   （案例 `sub_804612C`, 2026-09-04, gpnux; 连带发现 sub_801DD04 是 3 参 (obj, idx, val), 池扫下标 i 就是 idx 实参）

138. **K&R 空原型 `void f();` 会让 3 参调用编错实参 (第 3 参 r2 丢失) — 匹配前先查被调子函数 (如 sub_801DD04) 的真实签名, 按反汇编证据写全原型**; 合入时 src 里的空原型若与真 C 定义冲突 (make 报 conflicting types), 升级为全原型 `(u8*, u16, u16)` (无调用点则安全)。案例 `sub_804612C`, 2026-09-04。

139. **位域结构体局部被 GCC2 寄存器提升 (整值留寄存器、末尾一次 str) 而目标要求"每写一个字段就 str 回栈、但不重载"时, 用 `union { struct S s; u32 w[N]; } u;` 包裹可强制内存驻留 + 逐字段写回**（案例 `sub_8025518`, 2026-09-04, 未破但结构归位 4510→945）。
     判定: 目标对同一 `[sp]` 连续 `ands/orrs; str [sp]` 多条且**首条之后不再 ldr [sp]** (值在寄存器链 r4→r2→r4… 上前进, 但每步都镜像写回内存)。
     朴素 `struct S s; s.f0=..; s.f1=..; out[0]=*(u32*)&s;` 会被提升: GCC2 把整值留一个寄存器, 中间 str 判死删掉, 只末尾写一次。
     **正解**: 声明 `union { struct S s; u32 w[2]; } u;`, 字段一律 `u.s.fX = ..` 写, 末尾用 `out[0]=u.w[0]; out[1]=u.w[1];` 读回。
     u32 数组视图让 GCC2 认定内存可能被观察 → 放弃提升, 对**首个被访问的字 (word0)** 逐字段 str 回 [sp], 且 CSE 把上一步结果转发给下一步 (不重载), 与目标一致。
     ⚠ 不对称: 同一 union 里**后写的 word1 仍可能被留在寄存器只 str 一次** (GCC2 在 word0 temp 释放后压力下降), 目标若对 word1 也逐字段 str 则纯 C 难强制 —— 这是 sub_8025518 的残留卡点之一。
     配套: 单字段算术 (如 `g2 = a6 + 0xd`) 拆成 `new_var = a6 + 0xd; u.s.g2 = new_var;` 可让 a6 保持入口的 `<<24` 未归一形态, 复现目标的 `+0xd0000; lsrs #0x10` 移位域算术。
      ⚠⚠ permuter 对此族压到 <945 全靠 `new_var=u.w[0]; out[0]=new_var;` (new_var 取 u16) **截断 word0 高 16 位**省指令 —— 经验 18/113 偷改数据流, 非法, 必须人工回退到全字 `out[0]=u.w[0]`。
     关联: 经验 87 (双职变量买 home)、经验 88/117 (跨块 home 归 global-alloc)、经验 99 (数据默认 1-D)。

140. **⭐⭐ 传 `-1` 给 u16 形参会被 GCC2 折成池常量 `ldr rN,=0xFFFF`; ROM 若是 `movs rN,#1; rsbs rN,rN,#0` (s32 -1 进 callee-saved 跨多次调用) → 形参必须是有符号 `s16`**（案例 `sub_8024820`, 2026-09-04）。
     目标形状: `push {r4,lr}; movs r4,#1; rsbs r4,r4,#0; movs r0,#0; adds r1,r4; bl sub_8018798; movs r0,#1; adds r1,r4; bl` —— s32 -1 物化一次进 r4, 跨 `bl` 复用, 每次调用在 u16 边界截断 (被调方 `strh r1,[r0]`)。
     我写 `sub_8018798(0,-1)` (原形参 `u16 value`) 时 GCC2 在调用点把 -1 类型转换成 `(u16)-1 = 0xFFFF` 再传给 u16 形参 → 每个用 -1 的块产生 `ldr r4,=0xffff` + 4 字节池条目, 且 `int`/`s16` 局部变量缓存还会多出 `lsls r4,#16; lsrs r4,#16` 显式截断 (都差 1 x 真实指令数)。
     正解: 把被调方形参声明成**有符号** `s16 value` 后, -1 保持 s32 直入寄存器 → `movs r4,#1; rsbs r4,r4,#0`, 逐字节命中 (bytecmp 0 real diff, fncheck OK)。
     **安全判据**: 改形参 u16→s16 安全 iff 被调函数体对该参数只做"写 `strh`/读"这类不依赖无符号语义的窄存取 (i.e. 编译后字节不变; 本例 `gGstate330[index]=value` 两型一样)。改之前务必单独编译被调函数两型比对字节, 并 grep 确认没有其它已匹配调用者会被影响。
     关联: 经验 117 (常量物化/负常量 arm_split_constant)、经验 87 (寄存器 home: 跨 bl 存活的常值取 callee-saved)、经验 96 (permuter 打分同一程序)。

141. **⭐⭐ 汇编跳转表有 N 个 case → C 代码必须显式写 N 个 case，且按 0..N-1 顺序排列** —— GCC2 对 `switch` 的优化取决于 case 的完整性和顺序。缺 case 会让 GCC2 生成不同的比较树/跳转表结构，导致字节不匹配。（案例 `sub_8047FCC` 58 项跳转表，2026-09-04, gpnux；permuter 探索卡在 S-bit 差异，实际正确写法全 case 显式列出即得分 0）

142. **case 0 用 `return ret` 而非 `break` 可改变控制流结构** —— 当目标汇编的 case 0 处理路径与其余 case 不同（如直接跳到尾部），用 `return` 而非 `break` 可让 GCC2 生成匹配的目标代码。（案例 `sub_8047FCC` case 0 用 `return ret` 匹配目标早期返回路径，2026-09-04）

143. **⭐⭐ home 互换卡点: permuter 的"多余赋值"是有效杠杆, 但产物必须人工去类型作弊**
    （案例 `sub_800FA24`, 2026-09-04, opencode）。
    目标 `chara=&gPartyStats[i]` 指针 home 是 r1, 候选恒 r2 → 整个 HP/MP 体字节全互换
    (bytecmp 114B)。20+ 纯 C 变体 (u8/u16/u32 索引、ptr 算术拼法、声明序、const_int vs
    symbol_ref) 全无效。permuter 中奖输出 output-400: 在 MP 分支插入
    `new_var = (gUnk_030001AF) ? (gUnk_030001AF) : (999);` 的**多余赋值** —— 新增局部量
    改变伪寄存器生死边界, chara 一次归位 r1 (bytecmp 114→33)。
    但 permuter 把 `new_var` 声明为 `u8` → 999 被截断成 231 (`movs r0,#0xe7`), 是经验 18 的
    语义作弊; 目标从字面池 `ldr r0,=999`。人工修正为 `u16 amt; amt=gUnk_030001AF;
    if (amt==0) amt=0x3E7; chara->mp+=amt;` → 保留 999 + 保留 home 翻转, bytecmp 只剩 4 个
    bl 槽假差, fncheck OK 264B。
    **流程**: ① permuter 中奖产物若改变数据流 (截断/改常量), 先记录"它加了哪个多余赋值"的
    机制, 而非直接采信源码; ② 人工把类型改宽 (u8→u16/u32) 并重新跑 bytecmp —— 类型宽度决定
    字面量是否物化成池常量, 是 home 翻转与语义正确之间的唯一变量; ③ 改类型后 home 仍保留才
    算真解。这是经验 87 (变量兼职) 的"外部新增局部量"变体: 不需复用现有变量, 凭空加一个
    语义无痛的赋值语句即可改变伪寄存器生死边界。

144. **CSE 会把"开关分发等值寄存器"替换进 case 体内的同值常量**（案例 `sub_801A6F4`, 2026-09-05, 挂起）。
    分发 `cmp rN,#8; beq body8` 会让 CSE 在 body8 里把常量 8 替换成 type 寄存器本身
    (RTL `(ior (reg 185) X)` REG_DEAD 185, `-dc` dump 可见), type 活范围被拉长 60+ insn →
    全局分配优先级暴跌 → 落 r5 (目标 r1), 级联整函数寄存器错位。
    - **把 case 体走 NE 分支** (`if (type != 8) {} else { body8 }`) 时 CSE 记录的是 `≠8` 的
      qty_comparison (NE), 不再替换常量 → type 归位 r1、常量正常物化 `movs rX,#8`。
      代价: 空 then 的写法常被 GCC 反转成 `beq` (重新引入替换), 反转与否取决于外层 if 结构
      (`else if(type>7){...}` 嵌套可保住 NE, `else if(type<=7)`/`else if(type!=8)` 直链会被反转)。
    - 死 store `type=0;` (经验 33 手法) 在此无效: type 在分发后 tree 层已无引用, store 在
      RTL 生成前就被删, 杀不掉 CSE 等价。
     - 反推: 目标 case 体里出现"本可复用的常数被重新物化" (如 `movs rX,#8` 而非用分发寄存器),
       说明原源码的分发比较让 CSE 记录成了 NE/或 type 寄存器被 case 体首语句抢先占用。

145. **⭐⭐ `for (i = 0; ...)` 自带初值时, 循环变量**不得**在声明处再初始化一次** —— 重复的
      `u16 i = 0;` 会翻转邻近变量的寄存器 home（案例 `sub_8052F44`, 2026-09-05, 2 轮即命中）。
      目标 prologue:
      ```
      adds r5, r0, #0    ; ptr  -> r5
      ldr  r3, [r5]      ; data -> r3
      movs r4, #0        ; count-> r4
      movs r2, #0        ; i    -> r2
      ```
      - 写 `u16 i = 0;` + `for (i = 0; ...)` → `data`/`count` 的 home 在 **r3↔r4 互换**,
        全函数 `ldrb r0,[r3,#1]` / `cmp r4,r0` 级联错位, bytecmp 差 12B (104B 函数)。
      - 写 `u16 i;` (不初始化) + `for (i = 0; ...)` → 逐字节命中。
      原因: 声明处 `= 0` 让 `i` 的常量 0 在 tree 层多一个存活点, 与 `for` 的初始化合并方式改变,
      全局分配器给出的 home 顺序随之重排。
      - **同类判别法**: 若目标 prologue 里"指针装载 + 几个 `movs #0`"的**顺序**与你的 C 声明顺序
        一致但 home 全错, 先删掉循环变量的声明处初始化再试, 比改类型/加屏障快。
      关联: 经验 71 / 97 (类型宽度是分配器输入), 经验 104 (home 由声明形式决定),
      经验 115 (截断延迟), 经验 143 (home 互换家族)。

146. **`lsls/lsrs` 的移位量直接读出变量宽度: `#0x18` = u8, `#0x10` = u16** —— 反推循环计数器和
      累加器类型的第一手判据（案例 `sub_8052F44`, 2026-09-05）。
      本函数两个变量宽度不同, 各截断一次:
      ```
      ; 循环计数 i (u16):
      adds r0, r2, #1
      lsls r0, r0, #0x10     ; ← u16
      lsrs r2, r0, #0x10
      cmp  r2, #4
      ; 累加器 count (u8):
      adds r0, r4, #1
      lsls r0, r0, #0x18     ; ← u8
      lsrs r4, r0, #0x18
      ```
      - 两宽度混用时**不要假设它们是同一类型**。前两轮尝试把 `i` 也写成 u8, 卡在 score 1000 / 535
        判为"不可达 0"; 实际只差 `u16 i` 一个词。
      - 同一 C 文件 已匹配的 `Op_RemovePartyMember` 就是 `u16 i` → `lsls/lsrs #0x10`, 可作对照模板。
      - 注意 `cmp r2,#4; bhi` 是**退出式上界** (i > 4 退出), 对应 C 的 `i <= 4`;
        配合经验 108 的 peel 手法 (首迭代被提到循环外单独处理)。
      关联: 经验 68 (u8 计数器的包含式上界), 经验 108 (for+break 线性查找的 peel),
      经验 41 (只出现 lsls 无配对 lsrs = 只测零)。

147. **⭐⭐ GCC2 会把 `A + (B + CONST)` 重结合成 `(A + CONST) + B`; 解法是故意写成
      `(A) + CONST + (B)` 三向左结合** —— 目标要「常量绑到右侧移位项」时的唯一可写形态
      （案例 `sub_801A3C4` case 1 dest, 2026-09-05）。
      目标反汇编 (0x0801A35E..0x64):
      ```
      lsls r1, r1, #5      ; A = f_24<<5  (r1)
      lsls r2, r2, #0xc    ; B = f_22<<12 (r2)
      ldr  r3, =0x06010000
      adds r2, r2, r3      ; 常量先加到 r2 (=B)
      ; 尾部 adds r1, r1, r2
      ```
      即最终表达式 `A + (B + CONST)`, 但**按字面这么写会被 GCC2 打散**:
      - `(ROW<<5) + ((COL<<12) + CONST)` → 重结合成 `((ROW<<5)+CONST) + (COL<<12)`,
        产出 `adds r1,r1,r3`(常量绑 r1=A) + 独立 `lsls r2,#0xc` → **1 条差**。
      - 三向左结合 `(ROW<<5) + CONST + (COL<<12)` → 逐字节命中 ✓。
      - 指针算术 `((char*)ROW<<5) + CONST + (char*)(COL<<12)`、`=0x06000000 + 0x10000`、
        `u32`/`int`/`long`/`u64` cast、把 CONST 提到左/右、`((CONST)+(ROW<<5))+(...)`、
        `(ROW<<5) + (CONST+(COL<<12))` 等 **21 个变体全部失败**, 只有"常量夹在两个移位项中间"命中。
      - 关键判别: `fndiff` 显示 `adds r1,r1,r3` (常量绑 A) vs 目标 `adds r2,r2,r3` (常量绑 B)
        且目标里 `lsls r2,#0xc` 无独立副本 → 常量与 B 融合。别去猜, 直接枚举常量位置。
      关联: 经验 71 (GCC2 RTL 树形状敏感), 经验 104/143 (home 由表达式形状决定)。

148. **INCLUDE_ASM → 真 C 也会扰动同文件姊妹函数的 local_alloc tiebreak, 不限于 r8 使用者**
      —— 坑1 的"r8/sb/sl 触发"条件**过窄**, 需修正（案例 `sub_801A3C4` 转真 C 扰动
      `sub_8020B54`, 2026-09-05）。
      术语: **C 文件 = 翻译单元 (Translation Unit)** = 一个 `.c` 源文件及其编译产物 `.o`,
      即 `functions.tsv` 的 `module` 列。本项目文档统一称"**C 文件**"; 旧文档与命令残留的
      "TU" 均指此。GCC 每次编译一个 C 文件, 其内部 `local_alloc` 寄存器分配的平手裁决状态
      只在**同一 `.c` 文件内**的函数间共享; 不同 `.c` 之间互不可见。
      现象: `sub_801A3C4` 在本 `.c` 文件内的位置在 `sub_8020B54` 之前, 且其目标反汇编**无任何高位寄存器**
      (r8/sb/sl/c 均未使用), 但一旦从 `INCLUDE_ASM` 换成**任意**真 C body（连
      `if (obj->f_18 & 1) obj->f_22 = obj->f_22 + 1;` 这种单行也触发）, `sub_8020B54` 在
      0x08020B58/5A 的 `ldr r5,[pc,#36] / ldr r6,[pc,#40]` 会互换, 连带 0x08020B74/76 的
      `strb r0,[r5] / strb r0,[r6]` 互换 → ROM 4 字节差。
      已排除的替代原因:
      - 只加 `linker.ld` 绝对符号 `gUnk_087EBE00 = 0x087EBE00;` **不扰动** (ROM 仍绿)。
      - 用裸地址 `(*(const u8 * const *)0x087EBE00)` 替代具名符号**同样扰动**, 但自身 fncheck FAIL
        (池布局变化) → 说明触发点是"该位置存在真 C 函数定义"本身, 与符号命名/字面池形态无关。
      - `scripts/fncheck.py --blame` 报"完全一致"时差异必然来自同一 `.c` 内的其他函数, 不会归属到本 .o。
      解法: **拆分 C 文件** (即把一个大 `.c` 拆成两个, 坑1 通用解法, code_1/code_1b/code_1c 先例)。
      锚点必须是 `INCLUDE_ASM` stub 行以保持 ROM 顺序; 拆完两个 `.o` 各自独立 `local_alloc`,
      tiebreak 互不干扰。
      ⚠ 实测结论 (未落库, 供后续拆文件决策): 该 `.c` 在 `sub_801A3C4` 之后还有数百个函数,
      拆文件需同步改 `linker.ld` 的 `src/code_801A3C4.o(.text);` 为两个 `.o` 顺序拼接, 并更新
      `functions.tsv` 的 module 列与 `gen_asm.py` 的 module 映射; 触碰共享文件且有其他 agent 并行,
      风险高于收益时可按坑1 的既有取舍**保留同文件姊妹函数的 INCLUDE_ASM 形态**。
      关联: 坑1 (GCC2 跨函数状态泄漏), 经验 25+110 (do-while(0) 屏障打破 tiebreak)。

149. **DmaFill16/DmaWait 的通道号必须从字面池地址反推, 不要猜** —— 0x040000D4 = `REG_OFFSET_DMA3`,
      不是 DMA0 (2026-09-05, `sub_801A3C4` case 6/7/8)。
      - 目标字面池里 `0x040000d4` 直接就是寄存器地址, 对着 `include/gba/io.h` 查表:
        `REG_OFFSET_DMA3 0xd4`。写 `DmaFill16(0, ...)` 会编出池值 `0x040000B0` (DMA0SAD) →
        `fncheck` FAIL。
      - 同区相邻: `0x040000B0`=DMA0SAD, `0x040000BC`=DMA1, `0x040000C8`=DMA2, `0x040000D4`=DMA3。
        `0x040000AC`=DMARCNT10, `0x040000B4`/`0x040000C0`/`0x040000CC`=RCNT23。
      - DMA 控制字 `0x81002000` / `0x81000010` 的 `& 0xFFFF` 即 size/2 (0x4000 / 0x20),
        与 `macro.h` 的 `(flags)<<16 | size/2` 吻合, 可直接核。
      - 形状判别: "连续同基址 str + 一次空读" = `DmaFill16` 宏展开, 后面紧跟一次
        `ldrb` 读 RCNT = `DmaWait`; 见经验 55。
      关联: 经验 55 (CpuSet/DmaCopy 宏展开形状), AGENTS.md §3 (寄存器号别凭记忆)。

150. **⭐⭐ local_alloc 双轮分配模型 (sugg 轮 → pri 轮) 与"refs 堆叠"新杠杆** (2026-09-05, 案例sub_80525E8/sub_80526A0, 读 tools/agbcc/gcc/local-alloc.c 源码+追踪补丁实证)。
      `block_alloc` 分两轮发号: **第一轮 sugg**——凡 `qty_phys_num_sugg/copy_sugg≠0` 的 qty
      先行 `find_free_reg(just_try_suggested=1)`, 只在被建议的硬寄存器集合里挑;
      **第二轮 pri**——其余按 QTY_CMP_PRI 降序 (同分 qty 号小者先), r0→r15 首个空窗。
      - **sugg 唯一来源 = `combine_regs`**, 且要求指令 RTL 里**出现硬寄存器**: 纯 move
        (`set 伪 ← 硬寄存器`) 记 copy_sugg, 其余记 sugg。实测全函数只有 prologue 零扩展
        `lsls/lsrs` 产生 3 次 combine (r0/r1/r2 → songId/entry/mode 临时)。
        **普通算术/访存块内 RTL 不含硬寄存器** (movs/lsls 常量物化是 reload 期拆分),
        所以纯 RAM 访问块内 sugg 恒为 0 —— 查 sugg 路线前先确认块内有没有硬寄存器。
      - **qty_n_refs 按 qty 累计** (块内所有 tie 住伪寄存器的提及数之和, 实测 songId mega-qty
        refs=11): 累计来自**零扩展链/子reg 的硬寄存器 combine + 伪-伪 tie** (prologue 特有),
        ⚠ **C 层拷贝链堆 refs 不可行** —— `p=&G; q=p; r=q; *r=v;` 的拷贝在 CSE 就被折叠
        (实测 v19: 到 local-alloc 只剩一个 `reg/v` 伪寄存器, refs 回到 2), 不要再试。
      - **诊断路径**: qtydump 表 (ord/refs/birth/death/pri/sugg/reg 七列) + `-dl` 的
        "Register N used R times across L insns in block B" + LREG 文件里的 REG_EQUIV 注释,
        三者对齐后可在纸上完整模拟 block_alloc 的发号序 —— sub_80525E8 的 case2 目标序
        [pairs→r0×3, E69a→r0, 0x400→r1, **E6C→r1**, tbl→r2, base→r3, E70a→r2] 要求
        E6C 的 pri ≥ 10000 (即 refs≥7 或 sugg{r1}), 且 base 窗口 (28,30) 内 r2 被占 ——
        两条在现有 C 空间均无解, 属 agbcc 行为墙 (详见 progress.md 2026-09-05 段)。
      - **判据**: 目标里"长命地址伪寄存器拿了低位寄存器 (r0/r1) 而短命临时拿高位"时,
        先查该地址是否有多引用/拷贝链 (refs 堆叠), 再查块内是否有硬寄存器 combine (sugg 路),
        两者皆无 → 是墙, 不要再穷举语句序。
      关联: 经验 53/88 (QTY_CMP_PRI 与作用域), 经验 117 (global-alloc 同形公式), 经验 43 (refs 定 home)。

151. **位域提取必须写显式移位, 写 ands 会走错指令路径** (2026-09-05, 案例 sub_8017120 nibble 比较)。
     目标 `lsls r1, r2, #0x1c; lsrs r1, #0x1c; lsls r0, r2, #0x14; lsrs r0, #0x1c; cmp` —— C 层写
     `(status & 0xF) != ((status >> 8) & 0xF)` 产出 `movs r2,#0xF; adds r1,r3,#0; ands` (多一条且
     寄存器 home 不同); 写显式移位 `((status << 28) >> 28) != ((status << 20) >> 28)` 则逐字节命中,
     且 status 的 home 顺势落到 r2。背景: andsi3 expander 的 extzv 路径只接 i≥9 的掩码, 0xF 走
     force_reg+ands; 而显式双重移位让 combine 走位域提取。副作用: 上界/掩码类表达式先看目标 asm
     是 shifts 还是 ands, 再决定 C 形态。
     关联: 经验 117 (常量物化), 经验 54。

152. **循环不变量提取表达式的括号分组控制 LICM 上提位置** (2026-09-05, 案例 sub_8019F78)。
     `dest[(y << 5) + (col + shift)] = ...` 的括号把 col+shift 钉成子树, GCC 在 x-guard 之后、内层
     循环 preheader 处计算; 不分组 (`(y << 5) + col + shift` 左结合) 会把 sp 重读 + 加法提升到
     `mov r2, sl` 之前, 错位 2 条。两个对称循环体都要分组。同类: 循环上界 `height + top` 的操作数序
     直接决定 `adds r1, r4, r2` 的编码序 (fold 对 VAR+VAR 不重排, 源码序=编码序)。
     关联: 经验 29 (池假高), 经验 117。

153. **float→小整数的转换路径由"赋值目标类型"决定: s16 直接赋值走 __fixsfsi, (u16) 强转走
     __fixunssfsi** (2026-09-05, 案例 sub_801768C)。目标 `bl __fixsfsi; lsls #16; lsrs` —— C 写
     `result = (u16)(float-expr)` 会产出 fixunssfsi (字节错); 写 `s16 result; result = (float-expr);`
     则 fixsfsi + lsls/lsrs 命中。同函数: ①case1 里的死赋值 `new_var = 2.0f - ...` 是分配承重代码,
     删除会使 case2 的 subsf 寄存器序崩 (permuter 引入的伪死代码不可随手清理, 先 bytecmp);
     ②尾部 `result * arg3` 与 `arg3 * result` 的操作数序决定扩展指令序列 (commutative 不自由)。
     关联: 经验 29, 经验 87。

154. **结构体相邻小字段合并为宽字段可修复"基址+位移"寻址** (2026-09-05, 案例 sub_8017120/
     gSioSession.field_48..4B)。C 写 `*(u32 *)((u8 *)&gSym + 0x48)` 时 GCC 常把 +0x48 折进池条目
     (ldr [r0] 而非 ldr [r4,#0x48]), 连锁导致后续所有地址算术错位 (subs #0x30 之类)。先把 4 个
     无独立引用的 u8 合并成 u32 (grep 确认无 field_49/4A/4B 引用), 让 C 直接 `gSym.field_48`。
     顺带: linker.ld 插入符号必须按地址序 (0x4D0 插到 0x4D7 之后会 location counter backwards;
     本日两次踩坑, 插入前先 grep 目标行号)。
     关联: 经验 99 (数据表 1-D), 经验 7。

155. **未初始化局部变量 = 跨调用寄存器垃圾的合法 C 表达** (2026-09-05, 案例 sub_8018A58)。
     `bl sub_8018BF8` 之后直接 `dispcnt &= ~7` 使用 r6 (无任何初始化/load) —— C 层是未初始化局部:
     GCC2 分配寄存器后不生成任何 set, 值=调用者上下文遗留。成立条件: 掩码链覆盖结果全部 16 位
     (每位置或清), 垃圾位不外泄。同理 sub_8018A58 的 dispcnt/bldcnt 双变量。遇到目标函数"来历不明
     的寄存器链"先验证: ①callee 是否恢复 r6/r7 (pop 还原=调用者遗留) ②所有位是否被链覆盖。
     关联: 经验 55 (DMA 宏形状), 经验 149。

156. **bytecmp 的差字节基线 = 4×bl 条数** (2026-09-05, 案例 sub_8017120/801768C)。
     target.o 由 gbadisasm 的 .s 汇编, bl 槽位是 0xF7FF FEFF 占位; 候选经 abs.ld 部分链接后 bl 被解析
     (或生成 veneer 追加在 .text 后)。判据: cmp -l 的差异位置按 4 字节组对齐且组数=bl 数 → 零真实
     差异; 组数 > bl 数 → 每多一组一个真实差异。另: abs.ld 给 bl 目标赋真地址 (0x08xxxxxx) 会因
     基址 0 超 ±4MB 触发 veneer 追加, 不影响 .text 前 0x??? 字节的比较。
     关联: 经验 29 (fndiff 池假高), bytecmp.sh 头注。

157. **局部数组按 4 字节向上取整分配栈槽, 所以"最小语义尺寸"与"凑数尺寸"常生成完全相同的字节** (2026-09-05, 案例 sub_801D468)。
     `u8 a[5]; u8 b[7]; u8 c[12];` 与 `u8 a[8]; u8 b[8]; u8 c[12];` 都产出 `sub sp, #0x1c`
     (5→8, 7→8, 12→12, 共 28)。实测边界: 5/6/12=23→#0x1c, 5/8/12=25→#0x1c, 5/5/12=22→#0x1c,
     但 4/7/12=23→**#0x18** (首个槽 4 不需要取整, 省了 4 字节)。
     用途: ① 反推原 C 数组尺寸时**不要**用 `sub sp` 除槽数硬算, 只能得到"取整后的尺寸";
     ② 候选写不出来精确尺寸时用最小语义值 (例: mode0 写 5 项就写 [5], 别凑 8) —— 字节通常已相同,
     别为对齐尺寸浪费时间。判定法: 两种写法各跑一次 fndiff, 只要逐指令全对且 `sub sp` 一致即为等价。
     关联: 经验 27 (初始化顺序即指令顺序), 经验 47 (r8 序言/栈槽交互), 经验 124。
158. **分支内"表地址计算先于实参装载"时, 把表址提成显式指针局部放 call 前** (案例 sub_801DB3C, 2026-09-05)。
    目标 `lsls r5,r4,#1; adds r5,r5,r4; lsls r5,r5,#2; ldr r0,=tbl; adds r5,r5,r0` 在 arg0-arg3 装载**之前**。
    内联写法 `sub_801B81C(..., tbl[idx].field_0, ...)` 让 GCC2 把地址计算推迟到 `str sp` 装载之间,
    挤爆低号寄存器 → 强制 r8/r9/sl 溢出 (入口 `mov r7,sl; push {r5,r6,r7}` 多 6 条 + 中间 3 处往返)。
    解 = 独立语句 `t = &tbl[idx];` 放 call 前 → GCC2 把地址计算排到分支最前, 表指针稳定占 r5,
    参数全落低号寄存器, 逐指令命中。关联: 经验 47 (r8 序言), 经验 128 (宽类型独立语句)。
159. **if/else 的直落块 = 原 C 的 if 体** (案例 sub_801DB3C, 2026-09-05): 目标 `cmp; bhi→B块; A块直落; b join`
    表明 A 是 if 体、B 是 else 体。写 `if (be <= 0xA) {A} else {B}` (A 的条件为真走直落) 才命中;
    反写 `if (be > 0xA) {B} else {A}` 会得到镜像布局 `bls→A块; B块直落`。判定: 目标分支指令 (bhi/bls)
    指向哪块, 哪块就是 else; 直落块写进 if 体。关联: 经验 3 (分支极性), 经验 9 (flag 分支形状)。

160. **⭐⭐ 改共享原型的 signedness 会静默改坏"远端"已匹配调用点 — 定义侧与调用点侧的类型需求可以不同, 解法 = 定义侧宽类型 + 显式窄转换** (2026-09-05, 案例 sub_801768C; 已修复整 ROM 位移 8B)。
    事故: 提交 d4fcb74 把 `sub_801768C` 从 INCLUDE_ASM 换真 C 时, 顺手把共享原型
    `u16 sub_801768C(s16, s16, u8, u8, u8)` 改成 `s16 (s16, s16, s16, s16, s8)`。第 5 参改 s8 后,
    另一个 TU (`src/code_80264C0.c`) 里两个**早已匹配**的调用点 `sub_80405A4` / `sub_8042AB4`
    传 `gUnk_03000820` (u8) 给 s8 形参, GCC2 被迫补一条 `ldrsb r2,[r6,r2]` 符号扩展 (原来直接复用
    顶部 `ldrb` 的零扩展值), 各 +4B → 0x08040690 起全部符号位移 +8B → sha1 大面积红。
    正确形态: 原型 `s16 sub_801768C(s16, s16, s16, s16, u8)` (前 4 参仍必须 s16: 定义体走有符号
    float 转换, 改 u8 会转成 `__fixunsdfsi`, 288B 直接膨胀到 404B), 定义侧形参声明 `u8 mode` 但
    开关写 `switch ((s8)mode)` —— 显式转换重新触发 `lsls r4,r4,#24; asrs r4,r4,#24` 截断
    (u8 会发 `lsrs` 逻辑扩展, 与 ROM 差 1 字节)。三个 TU 全部字节命中 (sub_801768C 288B /
    sub_80405A4 236B / sub_8042AB4 220B / SoundMain_Frame 196B)。
    排查法 (整 ROM 位移定位): `fncheck --blame` 先报 "整体位移 -8"; 再用
    `arm-none-eabi-nm -n ll.elf` 逐个符号对比 `functions.tsv` 的 addr, 找第一个
    `actual > expected` 的符号 = 尺寸漂移起点 (本次 sub_80405A4 之后), 漂移量阶梯 (+4 → +8)
    直接指出是几个函数各多/少几个字节。别只靠 fncheck 的 FAIL 名单: 位移受害的 11 个函数
    (HBlankWave_BuildTables/Rand_TableNext/... ) 全表 FAIL 但字节本身没错, 是 bl 目标/池值连带偏移。
    教训: ① 动共享原型 (尤其第 N 参 signedness / 宽度) 后必须全量回归同调函数, 而不是只 fncheck 自己那个;
    ② 原型必须兼容**定义体**和**所有调用点**, 二者冲突时用"宽形参 + 函数体内显式窄转换"而不是 K&R 裸声明
    (K&R 会让 sound.c 的调用点丢掉 `lsls/asrs` 符号扩展, 同样改坏字节)。
     关联: 经验 140 (u16 形参装 -1 的池常量陷阱), 经验 148 (INCLUDE_ASM→真 C 扰动姊妹函数),
     经验 156 (bytecmp 差字节基线 4×bl), INCIDENTS 2026-09-05 23:5x。
161. **⭐⭐ 位域链赋值 (union.fields) 触发 GCC2 的逐位字节级 RMW, 输出 `movs rX,#N; negs rX,rX; ands` 的 ~mask 链 + sl/ip/r9 常量提升** (2026-09-06, 案例 sub_801D984)。
     写 `o->fields.AffineParamNo_L = 0; fields.HFlip = 0; fields.VFlip = 0; fields.Size = 1;` 时
     GCC2 对同字节内相邻位域生成**一条** 加载→连续 ands→orr→存储 的 RMW, 掩码按
     `~字段位` 的补码物化 (`movs #0xf; negs` = ~0x0E, `movs #0x11; negs` = ~0x10,
     复用 `subs r1,#0x10` = ~0x20), 0x3F/0x40 这类小常量被提到循环前放 sl/ip。
     对比: 裸字节 `t &= ~0x0E; t &= ~0x10; ...` (u8 局部) 会被 GCC2 折叠成 8 位掩码
     `movs rX,#0xf1` (经验 76 同类), 得不到目标形态。**当目标出现 ~mask 的 neg 连发 + 高位
     常量提升时, 先试 union fields 位域写法而不是裸掩码**。GBA OAM 位域 (GameOamData) 的
     完整半字 RMW (HPos/CharNo 跨字节字段) 也是位域赋值自动出的 (掩码 0xFFFFFE00 走池, 不折叠)。
162. **8 位存储目标会让 GCC2 在 expand 时把加法操作数换成 subreg 再被 expand_binop 交换, 产生 3-op `adds rd,rm,rd`; 用 u32 临时量接住和可保住 2-op `adds rd,rm`** (2026-09-06, 案例 sub_801D984)。
     `o->fields.VPos = field_3 + r7;` (VPos 是 :8 位域) → GCC2 先把 field_3 的零扩展字节抽成
     `(subreg:SI (reg:QI))` 再进 plus, expand_binop 见 op1 是 REG 而 op0 是 subreg 就交换
     → RTL `(plus r7_var field3)` → 分配后 `adds r0,r7,r0` (3-op, 0x1BC0), 与 ROM 的
     `adds r0,r7` (2-op, 0x19C0) 差 1 字节。解法: `u32 t = field_3 + r7; o->fields.VPos = t;`
     —— 先算完整 SImode 和, 截断留到位域存储, expand 时 op0/op1 都是 REG 不交换。
     教训: "存到小字段"的加法若 2-op/3-op 对不上, 优先把和提成 u32 临时。
163. **⭐⭐ 头部"取值→判空→条件减一"的寄存器 home 与分支极性: `if/else` 显式结构 (非三元) 一次给对** (2026-09-06, 案例 sub_8021184, 终结 3 天挂起)。
     `idx = val ? val - 1 : val` 三元写法 → `cmp r0,#0; bne→then` (else 直落);
     目标要 `cmp r0,#0; beq→else` (then 直落) + 四寄存器 home 全占 (b→r3, idx→r2, ptr→r1, val→r0)。
     写法: 声明 `u8 b; u8 idx; u8 *ptr;` → `b = (u8)arg0;` → `ptr = arg1 + 0xBE;` →
     `if (*ptr != 0) idx = *ptr - 1; else idx = *ptr;` —— val 读入 r0 参与 cmp 与 subs,
     else 分支 `ldrb r2,[r1]` 重读。案例 3/6/7 主体此前已逐行对 (除寄存器号), 破解只在头部三段。
     关联: 经验 3 (分支极性), 经验 159 (if/else 直落块 = if 体)。穷举了 三元/内联自增/
     ptr 别名/volatile/参数形态 ~600 种 (前 agent 记录), 只有这套组合中奖。
164. **⭐⭐ permuter 套件 RAM 池字"假分"的另一方向: extern 引用 → 候选侧 reloc-0 假差异; bytecmp 补符号桩判真, permuter 要真 0 须把套件 target.s 对应池字面量改成符号引用** (2026-09-06, 案例 sub_80526A0, 假分 25 → 真 0)。
     候选 C 引用 `extern u32 gUnk_03000E6C;` 等 RAM 符号时, 候选 .o 池字 = 0 + R_ARM_ABS32 重定位,
     而套件 target.o (asm 切片汇编) 池是字面量 `.4byte 0x03000E6C` → permuter/bytecmp 把这几个
     池字全判差异 (sub_80526A0 实测指令流 100% 一致仍记 25 分)。scorer 只在**目标侧有符号重定位**
     时才豁免 (`field_matches_any_symbol + old_line.has_symbol`), 故:
     ① bytecmp 判真: 逐个补绝对地址桩 `"gUnk_03000E6C = 0x03000E6C;"` → 部分链接施加池重定位后
     cmp, OK 136B 即候选字节成立;
     ② permuter 过门槛: 把套件 target.s 里 C 侧以 extern 引用的池字面量改成 `.4byte gUnk_03000E6C`
     (保持未定义 → 重定位) 重汇编 target.o, 两侧注解同名 → base score 真 0。C 侧字面量地址的池字
     (`(u16*)0x02016000` 等) 保持字面量不动。
     关联: 经验 29 (fndiff 池重定位假高 —— 本条是 extern 假低方向), 经验 96 (base.c 内联地址), AGENTS §2b。
165. **存全 1 常量到 `extern u16 数组[]` 元素 (`arr[i] = -1;`) 走 GCC2 窄化 store 的 ldrh/orr/strh 展开; 一切可树折叠的同语义写法折叠成直接 strh —— orr 是分配承重指令** (2026-09-06, 案例 sub_80526A0)。
     目标清槽循环 `ldrh r1,[r0]; orrs r1,r3; strh r1,[r0]` 只能用 `extern u16 gX[]; gX[i] = -1;`
     复现 (bytecmp OK 136B)。以下全部折叠成直接 `strh 0xFFFF` (函数 132B, push 丢 r6, case2/循环
     home 连锁位移, 假差 64B): 字面量强转视图 `((u16*)0x03000ED8)[i]`、`((u16*)&标量)[i]` (u8 decl)、
     数组指针解引用 `(*(u16(*)[8])0x03000ED8)[i]`、局部指针 `p[i]`、以及 `|=` 写法 (GCC 恒折常量)。
     同函数配套字形约束: ① 地址当**值**用必须字面量 —— `tbl = (u16*)gUnk_02016000` (符号) 池载入
     不下沉进 case2 (提前到 switch 前), 差 105B; base 写 `(u32)gUnk_02016200` 差 9B (case2 E6C home);
     地址当 **lvalue** (gUnk_03000E6C/E70/E72) 用 extern 无碍。② `u16 *tbl` 提升语句本身分配承重
     (删差 9B)。③ m2c 草稿的 `arg0++,arg0--; arg1++,arg1--;` no-op 纯属多余, u8 参数 prologue 截断
     lsls/lsrs 自然生成, 去掉零影响。
     同址多类型视图注册: 别名符号必须写成 SECTIONS 外 `gScriptLocalSlots = 0x03000ED8;` (fncheck
     只解析绝对赋值与 `. = 0x..; sym = .;` 两种行式, 段内裸 `sym = .;` 不解析 → 未解析符号 FAIL)。
     关联: 经验 29/96/118/150。
166. **"分支尾同形加法"会被 GCC2 跨跳合并 (cross-jump); 用提升表指针变量 + 字面量基址在前的和防合并** (2026-09-06, 案例 sub_804FA94, 修订: 初版结论 "long long 截断暂存承重" 已被更自然写法取代)。
     ① 现象: `if/else` 两分支都以 `*ptr = X + Y` 结尾时 (跳转路径 `表值+基址` / 推进路径 `*ptr+t+3`),
     GCC 把跳转路径的基址加法合并进公共尾部的 `adds r0,r0,r1` —— 基址落 r1 且路径内少一条 adds
     (差 5B)。目标要求基址落 r2、加法留在路径内。
     ② 解法 (sub_80526A0 套路): **提升 `u16 *tbl;` 变量 + 字面量基址在前的和** ——
     `tbl = (u16 *) 0x02016000;` 提到 if 前 (放循环前后皆可), 路径内写 `*ptr = 0x02016200 + tbl[data[2]];`
     (基址字面量在前, 符号形式 `(u32)gUnk_02016200 + tbl[...]` 不行)。0 字节命中。
     ③ 曾用 `long long dest; dest = 和; *ptr = dest;` 同样命中 (64-bit 暂存高位半占住 r1 迫基址落
     r2, RTL 实证 reg/v:DI 29 "8 bytes" 块内占 {r0,r1}), 但不必要且丑 —— 需要占位屏障时优先想
     "提升指针变量 + 和序调整", 具名宽类型是兜底。
     ④ bytecmp 伪影两层 (判变体时先校准): (a) bl 差 4B/条 —— 候选侧过部分链接 bl 被补丁, target 侧
     objcopy 直出是占位 (经验 156); (b) 函数桩地址给太远时 ld 插 veneer, mine.bin 尾部多字节 ——
     都是装置伪影。变体间对比用**同一已知好版本的 mine.bin 作基准**可零伪影 (套件 target.s 符号化后
     bytecmp 的 target 基线池字会变 0, 不能直接当基准)。
     ⑤ 比较类表达式的操作数序勿翻: `n > i`/`v > 0x1FF` 对应 `cmp r7,r4`/`cmp r1,sb`。⑥ 未初始化
     局部在目标里同样无初始化指令时, C 也保持无初值, 别"修"它。
     关联: 经验 156/164, 经验 29, 经验 152 (子树分组控制调度)。
160. **链表摘链必须用 u32 字指针, 结构体指针会被别名分析"优化"掉目标重载** (2026-09-06, 案例 sub_801DD04)。
     `node->prev->next = node->next` 的 struct-typed 访问让 GCC2 判定"存储与后续读取无别名",
     CSE 掉目标里的两次重载 (ldr [r1]/[r3] 各出现两次)。改用 u32 视图:
     `((u32 *)prev)[2] = *np; ((u32 *)next)[1] = *pp;` —— u32 存储与 u32 读取同别名集 → 保留重载,
     且 `[r,#8]` 偏移寻址保留 (ARRAY_REF 常量下标折进寻址)。配套: `u32 prev = *pp;` 早读语句
     (在另一地址计算前) 定调度序; `*pp = 0` 的零寄存器自动复用 prev 的死寄存器。
161. **RAM 字面常量基址的 +off 会折进池 (0x030006A0+4 → 池 0x030006A4), 注册符号 + 中间变量才能保住运行时加** (2026-09-06, 案例 sub_801DD04/801DF90)。
     `u8 *p = (u8 *)0x030006A0 + 4;` 单语句 → 池 0x...A4 (树级折叠); 拆两条语句也救不了字面量;
     必须 iwram.h/linker.ld 注册符号 (经验 73 的 RAM 版) 且基址先存变量: `arr = gSym; p = (u8 *)arr + 4;`
     —— DECL 初始化为符号引用, expand 不折, 产出 `adds rX, r0, #4`。经验 6 的补充: 符号+常量
     直接表达式 (`&gSym[idx].prev`) 仍会折成 `.word gSym+4` addend, 必须经变量中转。
162. **K&R 空参原型 `void f();` 之后的定义不允许带默认提升的形参 (u8/s16), 且中间补全原型也被 egcs 拒绝** (2026-09-06, 案例 sub_801FA10)。
     `void f(); void f(u8*, u8){}` → "argument type has a default promotion can't match empty
     parameter name list"; 插入中间原型 `void f(u8*, u8);` 同样报错。解法 = 升级 code_0.h 原型为全原型,
     前提是所有调用点传常量/已截断实参 (零代码影响), 改完必须 fncheck 全部调用方。
     定义侧用 int 形参虽合法但改变 `arg & 0xF` 的 ands 操作数序 (QI 语义丢失), 不可取。
163. **statement 拆分锁调度序**: 独立的地址算术/常量物化会被 GCC2 调度器自由重排, 把中间值提成
     具名局部语句可钉死顺序 (2026-09-06, 案例 sub_801DF90/801DEDC)。
     `off = arg0[0xC2] * 2; anim += 8; *(u16 *)(anim + off)` 三语句产出目标的
     `lsls; adds r1,#8; adds r1,r1,r0` 序; 单表达式会被重结合成 `ldrh [r0,#8]` 偏移折叠。
     `anim += 8` 语句 (变量回写) 是"运行时加"的最强锚点。反向: u16 变量的第二次赋值
     (`idx = idx + x`) 会在 case 内物化截断 (lsls/lsrs), 目标截断在合并点时须保持单表达式赋值。
164. **TU 状态泄漏的可迭代性**: 同一 C 在 standalone (permuter) 与 in-TU (真身) 的寄存器分配可以
     双向不同 —— standalone 正确而 in-TU 出 tie (sub_801DF90: standalone score 105 可复现,
     in-TU 剩 2 处调度 tie), 也可能反过来 (sub_801FA10: standalone 不出 z=0 的 r5, in-TU 一次成型)。
     对策: ①候选先 standalone 打磨形状, 再 in-TU fncheck 定案; ②in-TU tie 不必硬磕 ——
     每次落地其他函数都会重洗 TU 状态, 挂起候选换天重试即可; ③重试成本 = 一次 40s 编译 + fncheck。
165. **`~x` 掩码的 GCC2 展开是 `movs #(x+1); negs`, 所以掩码常量必须逐位核对** (2026-09-06, 案例 sub_801D378)。
     目标 `movs r2,#4; rsbs r2,r2,#0` = -(4) = ~3 (清除 bits 0-1), 不是 ~4!
     若 C 写 `& ~4` 会编成 movs#5+negs (差一位)。RSBS(0) = 取负 = -(x), ~x = -(x+1)。
     同族: `& ~0x1FF` 经池 (0xFFFFFE00), `& 0x1FF` 直接池; u8 变量上的 `& ~4` 会被折叠成
     8-bit 立即数 ands (0xFB) —— 目标保持 32-bit 运行时形式时必须用 int 临时承接 (t = oam[1];
     t &= ~3; oam[1] = t;)。掩码跨块复用 (r6/r5/r4/r3) 来自同常量表达式的高序 CSE。
166. **arg1 两分支 + default 的分派必须写 switch —— if/else-if 会内联首块** (2026-09-06, 案例 sub_801EC3C)。
     目标 `cmp/beq; cmp/beq; b default; <case体表体外置>` 是 switch 的形状; if/else-if 会把
     第一个 then 块内联进派发表后。同理: 单一共享 `return result` (无早退) 让 result 保持寄存器
     home → 声明处初始化 (u8 result = 0x20;) 提升到函数顶 → 高优先级长寿命量挤走参数寄存器,
     `push {r4,lr}` + callee-saved 自然出现。**case 体按源顺序排放** (case 7 在 case 8 前),
     相同掩码链的 case 体 (case 4/5/8) 自动共享比较尾 (b ED0A)。
167. **整数算术避免指针规范化**: `i * 0xC8 + (u32)arg1` 经 (u32) 转整数加法, keeps op0=mult →
     `adds rd, r_mult, r_base`; 直接 `arg1 + i * 0xC8` 会被 fold 成 ptr+int (op0=ptr) 翻转
     `adds rd, r_base, r_mult` (2026-09-06, 案例 sub_801DEDC/DF90)。
168. **半字 `|= K` 的 orrs 寄存器互换用命名临时修 (同族两处同时命中)**: 目标 `ldrh r1; movs r0,#K; orrs r0,r1; strh r0`(载荷在 r1、常量在 r0), 直接 `x |= K`/显式 `x = x|K`/`x = K|x` 全给镜像形状 (load r0/const r1); 函数顶声明 `u16 t;` + `t = x|K; x = t;` 一次修正 case0 与 case2-else 两处 (2026-09-06, 案例 sub_8030D9C; 同族 sub_8030F30/80310C4/8030C08 同构可复用)。另: 跳表函数 permuter 真 0 的完整路径 = 按 164 把套件 target.s 的 4 个 gUnk 池字面量换符号引用 (跳表 .4byte 标签引用两侧同 reloc 无需动), 9676 迭代底分 55 → 0。
169. **全局直读 + 常量写在前面 → gcc 常量进累加器; 局部快照 → 变量进累加器** (2026-09-06, 案例 BattleFx_UpdateTable/0x08019784, 关联经验 3)。
     目标 `adds r0,r2; ands r0,r3` (const r2 拷进 r0 做累加器) 与 `adds r0,r3; ands r0,r2`
     (var 做累加器) 的差别来自 expand_binop 的操作数: `(and REG CONST_INT)` 时 commutative swap
     恒把 CONST 排右 (op1), op1=REG 前置 → var 进累加器; 但若常量因 preserve_subexpressions_p
     (-O2 恒真) + rtx_cost>2 被 force_reg, 或 op0 是内存操作数/已被物化进目标寄存器, swap 条件
     `GET_CODE(op0)==CONST_INT` 不成立 → const 留在 op0 → **const 进累加器**。
     C 层面控制要点: **不要 `u16 flags = gFlashFlags; if (flags & 0x1000)`** (var 是 REG, const 被 fold
     排右 → var 累加器); 要写成直读全局且常量在前: `if (0x1000 & gFlashFlags)` → const 累加器 ✓。
     `x |= 0x4000`(内存 x) 同理给 const-acc; 有 3+ 处 and/or 同此规律时一并生效。
     连带: 别用 16 位局部做掩码快照再参与所有 and/or, 会统一翻成 var-acc。

170. **`int diff = a - i; v = (s16)diff >> 2;` 中间量锁定 (s16)i 的符号扩展; 单表达式会被截断折叠** (2026-09-06, 案例 BattleFx_UpdateTable/0x08019784)。
     `v = (s16)((u16)g386 - (s16)i) >> 2` 的 gcc: 结果立即 (s16) 截断 → (s16)i 的符号扩展被
     值编号折叠成零扩展 (`subs r0,r0,r3` 直接读 u16 home); 目标要 `lsls r1,r3,#16; asrs r1,r1,#16;
     subs r0,r0,r1`。拆成 `int diff = g386 - i; v = (s16)diff >> 2;` 后 diff 是完整 32 位变量,
     符号扩展必须物化 → 目标形状。

171. **同一 16 位局部在互斥 switch case 间复用 → gcc 分到同一寄存器 (r6)** (2026-09-06, 案例 BattleFx_UpdateTable/0x08019784)。
     目标 case1 的 angle、case2 的 v、0x2000-case1 的 amp_float 全用 r6; 把 C 里各自的
     `u16 angle;` `s16 v;` 改成一个函数级共享 `s16 tmp;` 四处通用 → 三处分配同时对齐
     (score 8650→7755)。gcc2.95 对不重叠生命期的 16 位局部倾向复用寄存器, 显式共享变量
     更可靠 (负数比较需要 s16, 用 (u16)/(s16) cast 在定点转型)。

172. **switch 分派树形态 (bgt-X-分离块 vs ble) 是 later block relayout 的产物, 两个同形 switch 会出不同形态; 别为形态差异改 C 结构** (2026-09-06, 案例 BattleFx_UpdateTable/0x08019784, 关联经验 166/168)。
     同一份 `switch (x) { case 0: break; case 1:...; case 2:...; }` 在某处编成
     `cmp#1;beq;cmp#1;bgt X;b 退出;X:cmp#2;beq;b 退出`, 另一处编成
     `cmp#1;beq;cmp#1;ble 退出;cmp#2;beq;b 退出` — 先 RTL 同形, 后段块布局翻转。
     判定: 只要 target 两处形态不同而 C 同构, 就别硬凑 case 0/default 差异; 先把其他
     分配差修完再看。真正要控制的是 **if/else 哪支直落**: `if (g386<=0x10F){循环}else{置位}`
     (循环在真支) 出 `bgt` 到置位块 + 循环直落; 反写 `>0x10F` 循环在假支出 `ble` 到循环。

173. **s16 参数转换链 (`lsls/lsrs` 截断) 会被 combine 吸收进首用点, 破坏入口参数序; 用"结果复用参数作累加器 + `case 0: break`"钉回入口** (2026-09-06, 案例 sub_8051AEC, 同族 sub_801768C)。
     症状: 5 参插值函数 (`s16 f(s16,s16,s16,s16,u8)`) 反汇编入口参数转换序 = 参数序
     (arg0→sl, arg1→r0, arg2→r6, arg3→r5), 但把 `result = arg1;` 写成具名局部时,
     combine 会把 arg1 的转换链 (`ashift+lshiftrt`) 吸收进该赋值语句 (body 内), 序变成
     arg0, arg2, arg1, arg3 → score 40~500 平台期 (permuter 只给 `arg1 = arg1;` 假招,
     需人工找真形)。正解: **不建 result 局部, 直接往参数上写**: `case 1: arg1 = (float)arg1 * ...; break;`
     + 显式 `case 0: break;` (让 switch 值域含 0, 分派树才是 `cmp #1; beq; cmp #1; ble;
     cmp #2; beq; b` 而不会退化成 `cmp #2` 直链)。default/0 路径 arg1 原样 → r0 直达尾部,
     天然等价无初始化。判定: 目标尾部 `muls r0, r1, r0` (arg3 * result) 且 default 无 mov。
     注意 5 参 u8 mode 若声明 s8, 调用点会多 `ldrsb` (经验 173 同条), 必须 u8 + switch 内 `(s8)`。

174. **"幽灵栈帧" (`sub sp,#8`/`add sp,#8` 全函数零 `[sp]` 访问) 的真因 = 未使用的 `u8 values[8];` 局部数组** (2026-09-06, 案例 sub_8048C80, 挂起→0)。
     同文件姊妹函数 (sub_804C9B4/D1B4/D310/C8E0/666C 等) 都有真使用的 `u8 values[8]` 传给 sub_80489E8,
     本函数是模板复刻残留 —— 数组虽零引用, GCC2 仍为其分配 8B 栈帧。等价 C 不写数组则帧消失 (差 4B/帧位移级联)。
     排查手法: 在已匹配函数里 grep `sub sp, #8` + 零 `sp]` 访问, 找到 C9B4 一看源码即中。
     关联: 经验 71 (类型宽度是分配器输入), 规则 76。

175. **求和链的 adds 操作数序由"结果 dest 是否已是操作数"决定; 两次独立 SET 才能保住 `adds rd, rn, rm` 的 rn=基址序** (2026-09-06, 案例 sub_8048C80)。
     目标 `adds r4,r4,r1; adds r4,r2,r4` (先 sw+v1 再 base+sw): 写 `threshold = base + (sw + v1);`
     (dest=threshold 不在操作数里) 逐字节命中; 写 `sw = base + sw;` (dest=sw 是操作数) 会触发
     dest-first 重排成 `adds r4,r4,r2`。左结合 `base + sw + v1` 树序也不对 (先算 base+sw)。
     关联: 经验 146/147 (重结合)。

176. **`(s8)x == -1` 走 cmpsi 的 cmn 路径 (`movs rN,#1; cmn`), 后续 pass 把 cmn 转成 cmp -1 时 -1 常量落在哪由分配决定; 目标"循环内 movs#1+rsbs 重物化"要求 +1 伪寄存器不被 loop 提升** (2026-09-06, 案例 sub_804BD54/B7B0/B8E8/BE90 家族, 未解)。
     实测 (t1-t6): cast/switch单case/struct-s8字段/s8指针/`(s8)x+1==0` 全部把 -1 提升到循环外
     (占掉一个 callee-saved, 挤掉 base 的 home 致 base 每轮重载池, 级联 ~66B)。
     -g 变体同。突破口猜测: 让 cmn 操作数 +1 与调用实参 1 共享伪寄存器 (多 basic 块使用 →
     reg_in_basic_block_p 阻止提升), 或找到让 reload 而非 expand 物化的写法。
     关联: thumb.md cmpsi expand (agbcc 源码 -255..0 走 cmn), 经验 29。
177. **死赋值拉长伪寄存器寿命 → 翻转 global-alloc 的 allocno 顺序 (r6/r7 home 互换)** (2026-09-06, 案例 Op_AddPartyMember/sub_804F7F8, 2030分→全指令命中)。
     症状: 全函数只剩两条指令差 —— 目标 `adds r6,r0,#0`(ptr→r6)+`adds r7,r4,#0`(id拷贝→r7),
     我方恰好 r6/r7 互换 (ptr→r7、拷贝→r6), 连带 `strb r7,[r1]`→r6、`mov ip,r1`→`adds r7,r1,#0`。
     根因 = global-alloc 按 `pri = floor_log2(n_refs)*n_refs/live_length*10000*size` 降序发号
     (EXPERIENCE 117 方法): newId (2 refs/27 insns, pri 0.0741) 恰好排在 ptr (4 refs/118 insns, 0.0678)
     前一位 → newId 先拿 r6, ptr 只剩 r7。两者只差 9%。
     正解: 在 part-1 循环里给 newId 加一条**死赋值** `newId = gPartyMemberIds[i];`
     (下一迭代即被覆盖、part-2 重新赋值, 无任何读取) —— newId 的 allocno 寿命横跨全函数,
     pri 掉到 2/118 级, ptr 升回首位拿 r6。死赋值在 cse2 后被删除, 输出零指令代价。
     permuter 的变异形式 `(newId = gPartyMemberIds[i]) > data[1]` 同效 (读取走 CSE 临时, set 仍死);
     ⚠ 若把比较也写成读 newId (`if (newId > m)`), set 变活 → newId 有了 part-1 home → 反而更糟。
     判定: 仅剩两条 home 互换 + agbcc -da 的 greg dump 里两 allocno 的 pri 差 <10% 时, 找"能否给
     低 pri 侧加死赋值拉寿命"。注意 -da 的 greg "across N insns" 数的是 **RTL insn 数** (post-pass),
     不是 thumb 指令数; PROMOTE_MODE 把 u8/u16 局部全提升 SImode, **换变量类型 (u8/u16/s16) 不能改
     size 旋钮** (greg sorted order 里 size 恒为 1)。
     关联: 经验 117 (allocno_compare 取数法), 经验 17/116(第二个) (do-while 屏障 —— 本例试过屏障
     也翻得动 newId 但把 ptr 挤到 r8, 级联更大, 不如死赋值外科)。
178. **⭐⭐⭐ 直写 `ptr[0] = x + 大常量` 的 strh 目标会触发 GCC2 HImode 符号扩展, 常量进池变负数; 经 u32 中间变量转存才能保持 SImode 正数形态** (2026-09-06, 案例 sub_805063C)。
     - 现象: `ptr[0] = (d & 0xFF) * 2 + 0xB000;` → 池字 `.word -0x5000` (0xFFFFB000);
       同值经 u32 临时 `v = (d & 0xFF) * 2 + 0xB000; ptr[0] = v;` → 池字 `.word 0xb000` 且
       常量走 `movs #0xb0; lsls #8` 物化。strh 存储结果相同, **字节不同**。
     - 机理: 直接对 HImode 目标的 store, GCC2 把 RHS 放进 HImode 语境, 常量按 16 位符号扩展
       (0xB000 → -0x5000), 符号扩展后的常量不再是 byte<<shift 可移位形式 → 只能池加载;
       SImode 正常算术里的常量保持正数 → thumb.md define_split (i=0..24 扫描 val ⊆ 0xFF<<i,
       低位优先) 拆成 movs+lsls。u16 中间变量**不行**(仍负), 必须 u32/int。
     - 推论: 池字面量的正负与 movs+lsls/ldr 形态是字节级判据; fndiff 报 "常量区大面积差异"
       时先查是不是这个。关联: 经验 29 (池重定位假分)。
179. **多处使用的大常量会被 GCSE 合并成共享伪寄存器, 从而阻止 combine 折叠; 源码侧用"拆语句"配合** (2026-09-06, 案例 sub_805063C 搜索分支)。
     - 目标形状: store1 = `0xB000 + t`, store2 = `t + 1 + 0xB000` —— 后者不被折叠成 0xB001 池,
     因为 0xB000 已是寄存器 (r8), combine 折不动寄存器。源码写 `v = 0xB000 + x; ptr[0] = v;
     v = x + 1; v += 0xB000; ptr[0x20] = v;` (0xB000 出现两次) 即可让 GCC 自建共享寄存器;
     但 `v = x + 1 + 0xB000;` 单表达式会被折叠/重结合成 x + 0xB001 池 —— **+1 和 +大常量必须
     拆成两句** (`v = x + 1; v += 0xB000;`), 且物化点要落在循环头前的 preheader。
     - 若手写 `u32 base = 0xB000;` 变量且赋值在循环前, 效果等同; 但变量存活范围跨循环时会
     在 global-alloc 里被挤到高位寄存器 (见 180), 不如让 GCC 自己合并。
180. **⭐⭐ 兄弟函数的"表指针/off 局部变量"风格不能无脑移植到含循环的函数: pre-cse 存活范围跨循环 → global-alloc 级联到高位寄存器** (2026-09-06, 案例 sub_805063C vs sub_804ABF8)。
     - sub_804ABF8 (无循环) 的 `base=表; off=counter*2+arg1*18;` 局部风格形状完美 (表地址最先
     加载 —— 表变量赋值语句在最前); 移植到 sub_805063C 后, off/tbl 在循环体内被引用
     (pre-cse RTL), REG_LIVE_LENGTH 拉长 → allocno pri (global.c allocno_compare:
     floor_log2(nrefs)*nrefs/LL*10000*size) 掉级 → 分到 ip/r8 级联 (prologue 多存一个), 4565 分。
     - 正解: head 用局部变量可以 (表地址最先加载的形状只有它能出), 但**循环体内的读取必须全内联**
     (自己的 counter-load/a18/table), 不引用这些局部 —— post-cse 后 tbl/off 只剩 head/store1
     路径的引用, 不与循环寄存器冲突。全内联 + 头部局部 = 585 分 (差 35B)。
     - 附: 同一常量 0xB000 目标里出现 `movs 0xB0; lsls 8` 与 `movs 0xB; lsls 12` 两种物化 ——
     后者是物化被 GCSE/LICM 提升出循环后**重拆分**(0xB0→0xB<<4) + combine 合并移位(4+8=12) 的
     双拆分痕迹, 复刻该形态需让 base 赋值在循环内被提升, 本轮未复现 (赋值在循环内外都只有单拆分)。
     关联: 经验 117 (allocno 取数法), 经验 177 (死赋值拉 pri)。

181. **⭐⭐⭐ `(s8)x == -1` 循环内 -1 常量提升问题的完整解法: 中间 u32 变量 + 无符号比较** (2026-09-06, 案例 sub_804BD54/B7B0/B8E8/BE90 四孪生, 从"不可解"到仅差 12B)。
     直写 `if ((s8)x == -1)` 时 GCC2 把 -1 QI 化 + 符号扩展对 (movs#1+negs 链, life=4, savings=2),
     loop.c 判定 `13×savings×life(104) ≥ insn_count(45)` → 整链提升到循环外, 挤掉 base 的 callee-saved
     寄存器 → base 每轮重载池 (级联 66B)。
     **正解三件套**:
     ① `u32 v = *(s8 *)(entry + 0);` —— 经中间 u32 变量: convert_move(MEM→SI, signed) 在 expand 期
        直接发 extendqisi2 (ldrsb), 值立即为 SI;
     ② `if (v == -1)` —— v 为 u32, 比较无符号, 常量 INTVAL=4294967295 > 0 (64位 HOST) →
        cmpsi 走 PATH A `force_reg(-1)` → 该 pseudo life=1 savings=1 → 13 < 45 不可提升 →
        movs+negs 留在循环内 = 目标形状;
     ③ 读 entry[0] 入 flags 局部须在 -1 检查**之前** (目标 ldrb 在 ldrsb 前)。
     另: `entry = base + index * 16` 的 base 会被 update_equiv_regs 等价替换 (REG_EQUAL(symbol) note)
     导致 base 每轮重载池; 写成 `entry = index * 16 + base` 使替换后 (plus (symbol) (reg)) 不过
     general_operand 校验 → base 保住 callee-saved 寄存器。
     诊断工具: `agbcc -dL` (loop dump 直接打印每个 movable 的 savings/life/desirable!) +
     `-ds` (cse 后 RTL 查 -1 链形态)。关联: 经验 176 (本文的失败存档), 经验 29, 规则 76。

182. **⭐⭐ 分支内部寄存器已知值 (如 `r2==0`) 跨 switch 分派树复用: 避免不同类型 (QI vs SI) 伪寄存器共存导致的全局寄存器提升 (r4 冲突)** (2026-09-07, 案例 sub_804FB24 完全匹配 1264B)。
     - 现象: `if (data[2] != 0) ... else { switch(data[1]) { case 3: ...; case 0xCA: gViewportFlags[10] = 0; } }`
       在 `else` 分支入口 `r2` 已知为 0 (来自 `cmp r2, #0`)。但在尾部 `case 0xCA` 处预期 `strh r2, [r0, #20]` 却变成了 `strh r4, [r0, #20]`，并在函数入口多出 `adds r4, r2, #0`（整体差异暴增至千字节）。
     - 根因分析:
       在 `else` 分支内，`case 3:` 写入 `gDrawCamEaseActive = data[2];`，由于 `gDrawCamEaseActive` 是 `u8` (QI mode)，GCC 直接复用了入口加载的 `reg:QI 26`（分配在 `r2`）；而 `case 0xCA:` 写入 `gViewportFlags[10] = 0;` 是 `u16` (HI mode)，GCC 的 CSE 将常数 0 等价映射到入口符号扩展后的 `reg:SI 27`。
       由于 `case 3` 与 `case 0xCA` 位于同一个 switch 分派树下，`reg:QI 26` 与 `reg:SI 27` 在整个分派树上**同时存活**，在 `global_alloc` 冲突图中 `26 conflicts 27`！GCC 无法将二者分在同一个物理寄存器，只能将 `reg:SI 27` 提升至 callee-saved 寄存器 `r4`，并在入口生成 `adds r4, r2, #0`。
     - 解法:
       使两处写入在语义上统一使用常数 0：将 `case 3:` 同样写为 `gDrawCamEaseActive = 0;`（与 `if` 分支的 `gDrawCamEaseActive = 1;` 语义高度对称）。此时两处均复用相同的 0 寄存器，消除了 `QI` 与 `SI` 伪寄存器的并发冲突，GCC 自然在整个分派路径上保持 `r2` 为 0，并在尾部精确生成 `strh r2, [r0, #20]`，入口 `adds r4, r2, #0` 完全消失！
     - 伴随坑点: 跨模块调用未截断实参时（如 `data[2] + 0x81` 传给 `MapBg_LoadFull`），若头文件声明了原型 `(u8)`，GCC 2.95 会强行插入截断 `lsls #24, lsrs #24` 并破坏调用函数入口的寄存器生命周期。
       完全使用纯标准 ANSI C 时的最优解：全局头文件与定义均保持标准的 `void MapBg_LoadFull(u8)` 与 `(u8 arg0)`（绝不引入过时的 K&R 语法），在调用点使用 `((void (*)(u32))MapBg_LoadFull)(data[2] + 0x81)` 告知编译器无需截断实参。GCC 将直接优化为 `bl MapBg_LoadFull`，达成纯 ANSI C 下双函数 100% 逐字节匹配。


183. **字面量 vs 链接器绝对符号: 地址常量会触发 GCC2 循环不变量提升, 数据池重定位则安全互换** (2026-09-07, 案例 Op_SysEffect 重命名复验)。
     - 现象: 把 `palDst = (u16 *)0x02020000;` 换成 `(u16 *)gCutsceneGfxBuf` (linker.ld 绝对符号) 后
       fncheck 差 +0x180..0x18b: GCC2 把符号地址视作地址常量, 识别出 `palDst` 跨清零循环不变,
       生成 `adds r4,r1,#0` 保存 + DMA 源直接 `str r4`(不再重读池), 而原版字面量重读两次池。
     - 规律: **指针赋值/DMA 源若要与"整型字面量"等价, 必须继续写字面量** (同 sub_80526A0 的
       `0x02016200` 坑, 经验 165 家族); 而 **只进字面池、不做指针运算的常量** (如 DMA 源表基址
       gFlashFxPaletteTable/gObjPalFadeInSteps/gObjPalFadeInFinal) 换成链接器符号后池值相同,
       fncheck 重定位施加后字节一致, 可放心符号化提升可读性。
     - 验证手段: 改名/符号化后必须 fncheck 逐字节复验 (本例 12B 差异即由其捕获)。
184. **已匹配函数的可读性重构安全清单** (2026-09-07, 案例 Op_SysEffect): 以下改动字节零影响, 可直接做并 fncheck 复验:
     ① 函数/参数/局部变量改名; ② case 标签换枚举常量 (含 `case 枚举 - 1` 复合常量表达式);
     ③ `arr[N]` 下标换枚举; ④ 全局符号改名 (iwram.h+linker.ld 同步, asm 硬码地址不受影响);
     ⑤ 纯数据池常量换链接器符号 (见 183 的边界)。⚠ 任何涉及"指针运算/地址常量"的符号化都要先过 fncheck。

185. `-g` 与 `-O2` 可叠加且不碰 ROM, 但 Makefile 不跟踪 flag 变化 (2026-09-07, script_vm.o 全链实测)
  现象: Makefile CC1FLAGS 加 `-g -O0` 后链接报 `linker.ld:682 cannot move location counter backwards (087f8cbc→087e83f0)`; 改回 `-O2 -g` 不 clean 复用旧 .o 报同样错误, 极具误导性。
  根因: ①`-O0` 让 .text 暴涨 (script_vm.o 16752→22828B, +36%), 把 `.rodata` 后续的硬地址锚点 `. = ORIGIN(rom)+0x7E83F0` (linker.ld .rodata 区) 顶过线, 计数器回退即链接报错 — 该错误的第一嫌疑永远是某 TU 体积变化; ②`-g` 只是往 .s 注入 `.LM*`/`.LFB*`/`.LI*` 标签与 `.debug_*` 节, 代码指令逐条相同 (规范化标签后 diff=0), `.debug_*` 不参与 ROM 布局; ③GNU make 对命令行变量变化不敏感, 改 CC1FLAGS 必须先 `make clean`, 否则旧 .o 混编 (两次地址一字不差即为旧物复用铁证)。
  规律: 实测 CC1FLAGS 加 `-g` 为 `-O2 -g` 时代码 0 字节漂移, 全量 make+sha1 绿 (746/1059) — 需要调 agbcc 内部转储时安全; 但每函数字节匹配后仍须 fncheck 复验。调 `-O0`/`-O1` 只能 per-function 覆盖 (照 m4a.o/agb_sram.o 的 per-target CC1FLAGS 模式), 全局改必炸布局。

186. **⭐⭐⭐ 脚本装载三件套: ScriptSet_Load/ScriptPump_JumpToEntry/ScriptPump_ServiceFrame 与"环境脚本集"状态机** (2026-09-07, agent opencode-scriptset, 更正 #1155 的 "LZ_BGM" 旧注)
  - `ScriptSet_Load(setId, entry, mode)` (0x080525E8): 从 ROM 指针表 gUnk_087ED6D4[setId] (363 项,
    0x0862D8A4..0x0861C784, 最大块 0x78010B) 取 LZ 块解压到 **EWRAM 脚本区 0x02016000** (入口表
    u16[256] + 代码区 0x02016200)。REG_DISPSTAT bit7 (VBlank 中) → 同步解压; 否则挂
    gUnk_03000E70 bit9(0x200) 交 VBlank 泵逐帧解压。mode1: gScriptCursor=0x02016200;
    mode2: 记挂 gScriptPendingEntry+E70 bit10(0x400), PC 预跳 base+entryTbl[entry]。
  - `ScriptPump_ServiceFrame` (0x0805008C, VBlank 调用): bit9 分支调 LZ_UncompressChunk 续解压,
    完成后若 bit10 置位则 gScriptCursor = 0x02016200 + entryTbl[gScriptPendingEntry] 并清标志。
  - `Script_SetEnvSet` (0x08008DCC, 12B `strb r0,[0x03004850]; bx lr`): 把 gScriptReturnSetId
    (0x03000E68, ScriptSet_Load 记挂的集号) 写入 gEnvScriptSetId (0x03004850, 原名 gCurrentSongId)。
    Op_ScriptReturn/Op_ScriptStop 脚本退场时调用 → 环境脚本集还原。
  - **状态机闭环**: gEnvScriptSetId 的全部 3 个消费者 (Scene_Reload 0x08001A54 / Scene_RestoreAfterBattle
    0x08001CEC / MapScene_Load 0x080071D4) 都是 `ScriptSet_Load(gEnvScriptSetId, 0, 1)` 重装环境脚本集;
    gMapScriptSetId (0x03004634, 原 gUnk_03004634) 来自 MapSceneDescriptor.scriptSetId (+0x0B, 原
    sceneFlag 注释有误), 是"进图时装哪个脚本集"。**确认变量语义必须追全部消费者, 而不是看赋值者**:
    本案 "Bgm_Request 只写 gCurrentSongId" + "NewGame_Init 里 =1" 曾导致两代 agent 都定性成 BGM。
  - 真正的 BGM 在 gPlayingSongId / Bgm_Play→m4aSongNumStart (sound.c:116), 与脚本集 id 是两套编号
    空间 (碰巧 NewGame_Init 两边初值都是 1, 加重了误判)。

187. DEBUG 构建 (-O0/-O1 + gdb 符号) 与字节匹配 ROM 布局共存方案 (2026-09-07, make DEBUG=1 全链实测)
  需求: 调试要 -O0 (断点/单步/变量), 但 -O0 让 .text 膨胀 (script_vm.o 16752→22828B, +36%), 全局 -O0 装不进 8MB。
  死路清单: ①改 CC1FLAGS 不 `make clean` → 旧 .o 复用, 报错一模一样极具误导 (经验 185); ②删 linker.ld 的
  .rodata ROM 锚点 (`. = ORIGIN(rom)+0x7E83F0`) → 数据 blob 整体后移, 但 asm 字面池硬编码 0x08xxxxxx 共
  1386 处 (代码区 1115 处) + 数据 blob 内函数指针 379 处 + 锚区 blob 内指针全部悬空, ROM 必崩; ③把膨胀代码
  塞"数据区尾部间隙" → 间隙实测仅 256B (8MB 完全塞满); ④`*(.text.__stub)` 想收 ld 自动 veneer →
  链接器后处理 stub 区 (.text.__stub) 不响应段内通配, 必须靠布局余量让 stub 自然落位。
  可行方案 (三件套, 全部 Makefile DEBUG=1 分支自动化):
    1. linker_debug.ld = scripts/gen_debug_ld.py 从 linker.ld 生成: rom region 8M→16M; .text 段剔除
       GAP 名单文件 (默认 event_hub event_actor, DBG_GAP 可改) 并**删掉段内补洞锚**; .rodata 显式
       `ORIGIN(rom)+0x5769C` 定位 (保数据起点, ld 的 stub 落在 GAP 挪走留的洞内); .text_gap 段
       ORIGIN(rom)+0x900000 收 GAP 文件。IWRAM/EWRAM 锚点不动。
    2. CC1FLAGS: DEBUG=-O1 (全局 -O0 实测 .text+rodata 溢出 8MB 6.9KB; -O1 仅 +720B 且 rodata 孤儿
       少)。per-target old_agbcc 覆盖 (m4a*.o/agb_sram.o) 自带独立 CC1FLAGS, 不受影响; 命令行传
       CC1FLAGS= 会覆盖 per-target `:=`, 别用。
    3. scripts/fix_debug_rom.py 链接后修补: GAP 搬家致 .text0 内其后的文件也前移, 所以用
       nm(ll.elf)×nm(ll_debug.elf) 同名符号地址 diff 全集做 旧→新 映射 (thumb 形态 |1), 4B 对齐扫
       ROM 替换裸立即数 (INCBIN blob 内的函数指针无重定位)。⚠ 扫描必须 4B 对齐步进: 逐字节会撞上
       图像/调色板数据里非对齐巧合值 (实测误改 460 处, 如 01000100→01003d00)。C 数组指针
       (m4a_tables 等) 链接器自动重定位无需处理。终验: 数据区"未解释残留"=0。
  铁律: 修补只影响 ll_debug.gba, 正式 ll.gba 布局/SHA1 不受任何影响 (make 仍绿)。16MB "ROM" 仅模拟器
  可跑 (真实卡带上限 32MB 但寻址镜像只到 8M/16M/32M 视总线), 调试用足够。
  产物: build_debug/ + ll_debug.{gba,elf,map} — ll_debug.elf 带 -g DWARF, gdb/mgba 可直接加载断点。

188. **超大状态机 switch 有符号比较 (bgt vs bhi) 与 DmaCopy16 控制字反推** (2026-09-07, sub_8011454 4314B 标题/存档大状态机实战)
  - **Signed Switch 调度**: 汇编如果出现 `cmp r0, #0xE; bgt default_or_exit`，说明是带符号上限比较。如果状态变量是 `u32`，GCC2.95 默认发射无符号 `bhi`；写为 `switch ((s32)state)` 即可严格对齐 `bgt`。
  - **DmaCopy16 字节尺寸换算**: GBA `DmaCopy16(ch, src, dst, size)` 的控制字为 `0x80000000 | (size / 2)`。反汇编中若见寄存器复用常量 `0x80000020`，其半字计数值为 0x20，对应的真实传输字节数为 `0x20 * 2 = 0x40` (64 字节)；草稿反推时若误填为 0x20 会导致控制字变成 `0x80000010`。
  - **具名结构体访问消减别名屏障 (Load-Delay Slot)**: `((u8 *)&var)[0]` 强转会导致 GCC2.95 的别名分析退化，阻碍常量/寄存器赋值向 load-delay slot 的调度填充；声明为真正的具名结构体 `var.field_0` 可解除别名假设，让指令调度槽 100% 契合目标代码。
189. **⭐⭐⭐ permuter score 假高第三类: CFG 折叠把不可达分支"激活"语义** (2026-09-08, 案例 sub_8050434, 3275 分候选全错)。
     目标汇编里 `if (v<0x6E3) collect` 的互补分支 `b.n next` (0x84) 是**不可达死代码** (到达它时
     v<0x6E3 前提已吸收了 0x6E4/0x6E5 检查)。permuter 重排出 `if (v<0x6E3) goto collect;`+
     collect 标签紧贴的 CFG (score 3275 vs 语义正确版 4120), 汇编文本差异小但把
     v∈[0x6E3,0x6E6] 从"跳过"变成"进入 collect" — **文本近似分 rewarding 语义漂移**。
     教训: promote 前必须对 if/else if 落穿链逐分支核对真值表 (尤其"互补条件+标签紧贴"的形状);
     score 更低 ≠ 更接近, 每轮 promote 后用 bytecmp + 手读 CFG 复核语义不变式。
     关联: 经验 18/113 (permuter 偷改数据流), AGENTS.md 铁律 6⑤。
190. **venv python 被桌面 AppImage 挟持的识别与绕过** (2026-09-08, 环境)。
     症状: `.venv/bin/python` 的 `sys.executable` = ZCode AppImage, venv site-packages 不进
     sys.path (`import toml` 失败), 但 `python3 -m pip install` 报 "No module named pip"。
     识别: `.venv/bin/python3 -c "import sys; print(sys._base_executable)"` 指向 AppImage 即中。
     绕过: `PYTHONPATH=$PWD/.venv/lib/python3.14/site-packages /usr/bin/python3 <tool>` (系统
     python3.14 正常, venv 的包用 PYTHONPATH 注入)。另: Python 3.14 起 multiprocessing 默认
     start method = forkserver (context.py gh-84559), permuter worker cwd 不再继承主进程,
     compile.sh 必须自带 `cd "$(dirname "$0")/../.."` (fndiff 生成的模板有, 手拷的没有 —
     症状: permuter 报 `tools/preproc/preproc: 没有那个文件或目录` 而 compile.sh 手跑正常)。

### 2026-09-08 codex-menu 会话增量 (menu.c 批次)

118. **gWindowBgBuf 引用形态决定 8B 差**: `extern u8 gWindowBgBuf[]` 符号引用与 `((u16*)0x02005800)`
     字面量在 gcc2 里寄存器分配不同 (360B vs 368B)。permuter base.c 的 typedef/宏必须与项目头**完全同型**
     (经验 96 延伸: 类型不只是"能编过"而是直接决定分配)。案例 sub_80154E8。

119. **gcc2 的 switch 生成块序 = 源码 case 声明序** (非二分平衡树)。想让树的分支点与目标一致,
     先数目标 asm 的**块地址序** (case 体地址从小到大), 再按此序声明 case。案例 sub_801417C
     (F5,28,1,8,2/4/6,7,5,1E,1F,20,21/22,23,F0,F1-F8,F9,FA → score 25975→19765)。

120. **if/else 嵌套层级决定块布局**: 平铺 `if(A) else if(B) else(C)` 生成块序 [A][B][C];
     嵌套 `if(!A){ if(B) C else D } else {A体}` 生成 [B][C][A体]。目标 asm 的跳转方向
     (beq 目标在后方 = 该分支体在后) 反推嵌套层级。案例 sub_8010F10。

121. **字符串表绘制 helper 必须内联展开**: agbcc 不内联 static 函数 (即使 -O2, 单调用点也不内联),
     目标里 N 份重复循环体 = 源码 N 份展开 (或用宏)。先试 helper → fndiff 直接多出 bl+独立栈帧。

122. **u16 shifted-home 家族确认不可达** (补强 sub_8049AD8 判例): 当目标把 u16 变量的
     `V<<16` 形态存入高位寄存器 (r8/sb/sl/ip) 并在使用点 `lsrs #0x10` 重载时, 等价 C 的 gcc2
     只生成真值 movs — 触发条件是目标把该变量放高位寄存器, 而 mine 无法控制分配落位。
     案例 sub_8013934 (cols/rows/6<<16), InvUi_Main (0xB0<<13), sub_80146A8 (fill 循环)。
     已穷举: 声明序、类型宽度、register、(u16) 强转、变量别名、permuter ×4 — 均无效。
     遇到此形状 (movs rX,#V; lsls rX,#N 合成到高位 + lsrs #0x10 重载) 直接判挂起, 别再死磕。

123. **permuter 偷改数据流新模式**: `((0x02005800 & 0xFF) & 0xFF) & 0xFF` — 把基地址折叠成 0
     (fndiff 分数反而更低, 8285 vs 12325!), 语义全错。清洗准则: 任何 `& 0xFF` 链作用于
     常量基址的一律回写原式。另: `if (未初始化变量)` 分支 (5675 版) 直接拒绝。

189. **DmaCopy16 0x20 字节 = 控制字 0x80000010, 别与 DmaCopy32 混淆** (2026-09-08, sub_80501B8)
  - FlushTileDma 家族 (sub_80501B8 生产者 → sub_80527AC 消费者) 的字库字形拷贝是 **16 位 DMA 传输 0x20 字节**:
    `(DMA_ENABLE|DMA_START_NOW|DMA_16BIT|DMA_SRC_INC|DMA_DEST_INC)<<16 | 0x20/2` = `0x80000010`。
    草稿反推时若见 `0x80000010` 写成 DmaCopy32(...,0x20) 会得 0x80000008 (32位计数 0x20/4=8), 差 2。
    判定口诀: 控制字低 16 位计数值 = 字节数/2 → DmaCopy16; = 字节数/4 → DmaCopy32。
  - 同族两个 `DmaCopy16` + 各自 `DmaWait` 的源码形状会生成: 两次 `str [base,#0/#4/#8]` 共基址 +
    每次拷贝后一次 `ands r0,#0x80000000` 忙等 — `DmaSetUnchecked` 的尾部空读 `dmaRegs[2]` 与
    `DmaWait` 的循环读是两条独立语句, 别合并。

190. **0x100 步进 switch 比较树的源码顺序敏感性** (2026-09-08, sub_80501B8)
  - `switch (hi)` 值域 0x100..0xF00 时 GCC2 发射二分比较树; **case 顺序决定树的分裂点**:
    实测 `case 0x800`(带附加条件体) 放在最前与 `case 0xF00` 相邻 vs 隔位, 差 ~5 处分支目标偏移。
    目标形状: 0x800 与 0xF00 体都在比较树尾部 (cmp+beq 内联), 其余 12 个 case 纯 `beq` 直跳共享 break。
    源码按数值序排列 case (0x100<0x200<...<0x800<0x900<...<0xF00) 且 0x800 体在最前, 才能命中。
  - `default` 块的位测试掩码若被 permuter 提成 `new_var` 变量, 属合法调度 (非经验 18 违例),
    但人工化时要还原为直接常量 (查它是否只读一次)。

191. **⭐⭐⭐ 破解'基址进 r8 + 循环后 mov r4, r8 迭代'的终极方案: extern 数组符号阻止 update_equiv_regs 常量传播折叠** (2026-09-08, 案例 sub_8013870 完全匹配 196B, 攻克历经三代 agent 的 53B 卡点)。
  - 现象: 目标形状 prologue `ldr r2,=0x02005800; movs r1,#0; ldr r0,=0x08098622; mov r8,r0; ...` (ClearBuffer 占满 r4-r6, 0x08098622 提升进 r8), 清屏后紧跟 `mov r4, r8; b loop_head` (r4 作为字符串迭代器循环推进)。
  - 根因分析:
    若写纯字面量 `(const u8 *)0x08098622`:
    ① 若在清屏前赋值 `src = 0x08098622`, 因 src 循环内有 14 次引用, global-alloc 中 pri≈12353 率先抢占 r4, 将 ClearBuffer 的变量挤乱 (v16 级联 675 分);
    ② 若拆成基址+迭代器 `table = 0x08098622; ClearBuffer(); src = table;`, 因 table 仅赋值 1 次、读取 1 次 (REG_N_REFS==2), GCC2 的 local-alloc `update_equiv_regs` 检测到其值为已知常量, 判定 `rtx_equal_p` 成立, 直接将 `src = table` 替换为 `src = 0x08098622`, 彻底删除 table 伪寄存器并在清屏后发出 `ldr r4, =0x08098622` (v13 级联 53B);
    ③ 前人曾尝试 `int new_var = 0xFE` 顶赋 (cand_490_best.c) 强行占领 r8, 虽使清屏块命中, 但残留 3 处无法消除的伪指令差。
  - 正解:
    声明为外部符号 `extern const u8 gTitleMenuDesc[];` (在 linker.ld 赋予 `0x08098622`):
    在源码中写:
    `ClearBuffer((u16 *)0x02005800, 0x1E, 0x14);`
    `table = gTitleMenuDesc;`
    `src = table;`
    此时:
    1. `gTitleMenuDesc` 是符号引用而非立即数, `update_equiv_regs` 无法将其作为已知整型常量进行常量传播折叠;
    2. GCC 的 loop/GCSE 将其识别为全函数不变的地址, 自动将其提升至 prologue (排在 ClearBuffer 初始化的 r2, r1 之后, 完全吻合目标指令顺序!);
    3. 由于 ClearBuffer 内部占满了低寄存器 r4-r7, table 自动被分配到首选的高位寄存器 r8;
    4. 清屏结束后, 语句 `src = table;` 在进入迭代循环前精确发射为 `mov r4, r8`, 逐字节 100% 完美匹配!
  - **再次同构印证 (sub_8013934, 472B, menu.c)**:
    在 `sub_8013934` 中，清屏小框 `ClearBuffer((u16 *)buf, 3, 2)` 后紧随 `sub_800EAE4` 绘制数字，目标在清屏前 `ldr r0, =ptr; mov sl, r0;`，清屏后 `mov r2, sl; ldrb/ldrh r1, [r2]; bl sub_800EAE4`。
    前人曾误报为 "LRA live-range-split 不可达" 假案。使用同款 `table = &gCardAlbumPage; val_ptr = table;`，外部符号完全阻止了 `update_equiv_regs` 的折叠，高位寄存器从 `r8` 自然变为 `sl`，一举消除所有差异完美匹配！

192. **GCSE PRE 的 phi 拷贝与 LIM 循环不变量提升的对抗** (2026-09-08, InvUi_Main, 未匹配挂起)
  - **LIM 触发条件再认识**: `while ((ch = *p++) != 0) {body}` 的 loop 头 = 测试块时 gcc loop.c 识别为循环,
    循环内不变表达式 (`pos2<<6`) 会被提升到循环前并抢占高位寄存器 (经验 122/antigravity 卡点)。
    do-while 改写 (`do {...} while(1)` / `do {...} while(cnt<8)`) **不奏效** — gcc 会把 break 链还原成
    相同的测试前置结构。真正破法: 让循环前的表达式依赖被循环体"消费"的中间变量内联展开
    (`y2 = rowi*2+7` 内联进循环体后, 循环块拓扑改变, LIM 不再识别)。
  - **GCSE PRE 的 phi 拷贝形态**: 同一表达式 (`rowi*2`) 在两分支各算一次、汇合点又用一次时,
    gcse PRE 会插入归一拷贝 (`adds rX, rY, #0`)。若 C 在两分支各写一个局部变量、汇合点用
    **原始表达式重算** (而非变量), PRE 才会出现; 若汇合点直接用变量, CSE 把两分支定义合并成
    单伪单 home, 拷贝消失。注意 `rowi*2` / `rowi<<1` / `rowi+rowi` 在 expand 阶段全部归一为
    ashift — 想靠"换写法"阻止 CSE 是徒劳的。
  - **数组下标 vs 指针算术改变伪结构**: `a[i]` 与 `*(a+i)` 在 gcc 前端同 RTL, 但**数组形式会让
    gcc 先 ldrb 下标表达式再 ldr 基址** (顺序相反), 影响附近伪寄存器的分配编号。
    实测 InvUi_Main else 分支 `rowi == *(gMenuCursorStack + gMenuCursorGrp)` 使 count/icon
    从 r1 挪到 r2 (正确位置), 数组形式则错位。遇到"逐指令形状全对但寄存器号系统性差 1"时先试这个。
  - **global-alloc home 差 1 链 = C 不可达**: 一旦出现 y2:r3 vs 目标 r2、PRE phi:r4 vs r3 这类
    全链错位, 穷举类型宽度/声明顺序/变量复用/表达式写法均无效 (home 由冲突图着色决定)。
    遇此形态直接转挂起, 在 note 记录 "home 差 1 链", 别再烧 iterations。
  - **对照工具陷阱**: gbadisasm 输出用 `sb/sl/fp` 别名而 objdump 用 `r9/r10/r11` (sb==r9!),
    池加载一个用标签一个用 `[pc,#n]`, 条件后缀 blo/bcc、bhs/bcs 互为别名 — 逐指令 diff 前必须
    先做这三类归一, 否则 ~15% 的"差异"是伪差异, 会把排查方向完全带偏。

193. **⭐⭐⭐ `(u16)(x+K)*2` 的移位编码由目标变量类型决定: u32 中转出 `lsls#0x10/lsrs#0xf`, u16 直接收窄出 `#0x11/#0x10`** (2026-09-08, 案例 sub_805063C, 破"GCC2 版本差异"误判)。
  - 目标 `adds r0,i,#0xE0; lsls#0x10; lsrs#0xf` (= (u16)(i+0xE0)*2, 17位中间值) 曾被三任 agent
    判为 "HImode 移位, 当前 agbcc 无法复现"。真相: **t 声明为 u32** 且写 `t = (u16)(i + 0xE0) * 2;`
    即可 — combine 把 u16 截断+乘2 重结合成 `x<<17>>16`; t 为 u16 时是 `x<<17>>17>>1` (#0x11/#0x10)。
  - 推广: 遇到 "窄类型截断+缩放" 的移位编码差异, 先查**接收变量**的类型宽度, 再查表达式写法
    (`*2` vs `<<1` 也不同字节: 乘法走 muls/移位走 lsls)。判"编译器版本差异"前必须穷尽类型矩阵。

194. **⭐⭐ 恒假/恒真条件复用返回值寄存器做循环守卫: `if (ret < n)` (ret=0)** (2026-09-08, 案例 sub_805063C 搜索循环)。
  - 目标循环守卫是 `cmp r9, r3` (r9=sb=ret 的高寄存器 home!) 而非独立的计数器比较 — 说明原 C 的
    循环守卫直接复用了**返回值变量**。C: `ret = 0; ... if (ret < n) { do {...} while (i < n); }`。
    这是 sb/r9 高寄存器 home 的来源: ret 的唯一长寿命引用就是这个守卫。
  - 识别特征: prologue `movs r0,#0; mov r9,r0` 且 r9 在循环头出现一次 cmp — 别把 ret 当纯返回值,
    它可能是循环边界复用的变量 (原作者风格: 零寄存器浪费)。

195. **⭐⭐ 直写分支掩码 `0xFF & v` 常量前置的又一实证 + 双 web 拆分时刻决定 ands 是否破坏 home** (2026-09-08, 案例 sub_805063C)。
  - `p[0] = 0xB000 + ((0xFF & v) << 1)` (常量在源文本前) 出 `movs r1,#0xFF; adds r0,r1,#0; ands r0,r4`
    (常量拷贝进 dest, v 保持); `v & 0xFF` 则出 `ands r_v, r_c` 原地破坏 — 经验⑥ (BattleFx) 的第三次复现。
  - **双 web 拆分时刻**: 目标的两个 v web = [r4 原值(直写/循环用) + r2 比较副本(if 用)] — 副本在
    ldrh 后立即创建; 若 C 里写 `w = v` 且 if 用 w, 副本 web 变成 [r1 + r4], ands 落在 r1 (if web 的
    home) — 位置对但**哪边被 ands** 反了。破法: 让 if 的条件表达式直接引用一个"经过副本的"变量,
    而 ands 消费原变量 (源码上同一变量, 靠 GCC web 拆分)。尚未穷尽: 3 变量互拷矩阵 (v=w, w=v, n=v 组合)。

196. **⭐⭐⭐ GCC 2.95 "Ghost 寄存器" (prologue/epilogue push/pop 了某 callee-saved 寄存器但在体内完全未读写) 的生成机制与解法** (2026-09-08, 案例 sub_8015E1C / MenuUi_HideAll, 攻克全 ROM 仅有的两个 Ghost 寄存器谜团)。
  - **现象**: 函数体逐指令 100% 匹配, 唯独 prologue `push {..., rX, lr}` / epilogue `pop {..., rX}` 多包含一个 callee-saved 寄存器 (r4 或 r6), 而在整个函数体内根本找不到一条读写 rX 的指令。
  - **全 ROM 普查定论**: baserom.gba 全量 1200+ 函数中, 出现 Ghost 寄存器的函数**全 ROM 恰好只有两个**:
    ① `MenuUi_HideAll` (`0x0801667C`, 40B, 出现 ghost r4);
    ② `sub_8015E1C` (`0x08015E1C`, 108B, 出现 ghost r6)。
  - **GCC 2.95 编译机制全链路溯源**:
    1. **RTL 生成阶段**: 循环体内的某种复合操作 (如 `MenuUi_HideAll` 的 `p->statusFlags |= 0x40` 经 `store_fixed_bit_field` 产生 0 掩码; 或 `sub_8015E1C` 内联 `PutGlyph` 时的 `tileId = ((charCode & 0xFF00) >> 7) + 0x280` 产生 16 位常量 `0xFF00` 的 subreg:SI 伪寄存器) 产生了中间伪寄存器 P。
    2. **`loop` pass (早于 `combine`)**: `loop` pass 检测到 P 的赋值是循环不变量, 将其提升 (hoist) 到循环前置块 (loop preheader)。
    3. **`combine` pass**: 随后 `combine` 发现循环体内使用 P 的地方因数据宽度或常量特性 (如 `(u8)x & 0xFF00` 恒为 0) 可完全折叠消除, 从而删除了循环体内所有对 P 的引用。
    4. **`global_alloc` pass**: preheader 中的 P 依然存活且其生命期跨越整个循环。因所有 caller-saved (r0-r3) 以及已被占用的 callee-saved 寄存器 (如 sub_8015E1C 的 r4:arg3, r5:palAttr) 与其冲突, `global_alloc` 将下一个空闲的 callee-saved 寄存器分配给 P (MenuUi_HideAll 分到 r4, sub_8015E1C 分到 r6)。
    5. **`reload` pass**: 寄存器重载阶段调用 `mark_home_live(regno)` 标记 `regs_ever_live[regno] = 1`。
    6. **`flow2` pass (reload 后的死代码消除)**: 发现 preheader 中对 P 的定义无任何后继活跃消费者, 将其作为死代码标记为 `NOTE_INSN_DELETED` (不生成汇编指令)。
    7. **`final` pass (代码生成)**: 根据 GCC 内部机制 (`flow.c:1635`), reload 期间记录的 `regs_ever_live` 在 flow2 结束后被原样恢复, prologue / epilogue 输出保存/恢复列表时读取该数组, 导致被删除的寄存器依然被 push/pop, 但体内无任何指令！
   - **解法**:
     不要尝试手写死代码 (手写死代码在 `cse` 阶段即被折叠, 活不到 `loop` 和 `global_alloc`)。寻找真实语义中被在循环内调用的内联函数或宏 (`Text_PutGlyph`)。将其形式声明为 `static inline`, 内部保留自然的字形编码扩展逻辑 (`((charCode & 0xFF00) >> 7) + 0x280`), 编译器自发在 preheader 产生 0xFF00 并在 flow2 静默消除, 完美零误差生成 Ghost 寄存器。

197. **⭐⭐ 常量`+0xFFFFB001` 会被 GCC2 折叠成池 0x0000B001, 写成 `x - 0x4FFF` (≡ +0xFFFFB001 mod 2^16) 保住 0xFFFFB001 池条目** (2026-09-08, 案例 sub_8024618, permuter 640→520)。
  - 现象: 目标 else 分支第二存 `ldr r3,=0xFFFFB001`, 我方写 `(u16)(x + 0xFFFFB001)` 时 GCC2 把
    u16 截断吸收进加法 → 池只剩 0x0000B001 (复用 constVal 条目), 差 1 池字 + 对齐 2 行。
  - 机理: `(u16)(x + K)` 的截断让 GCC2 只保留 K 的低 16 位参与常量池查找; 0xFFFFB001 与 0xB001
    低 16 位相同 → 命中已有条目。写成 `(u16)(x - 0x4FFF)` 后 0x4FFF=20479 是独立正常量,
    GCC2 仍以 32 位语义物化 0xFFFFB001 (池条目保留), 且因 u16 截断二者结果逐位相等。
  - 判定: `- 0x4FFF` 比 permuter 的 `- -0xFFFFB001` 可读; 语义合法 (u16 截断吸收), 必须加注释说明。
  - 关联: 经验 18/113 (permuter 偷改数据流 — 本条是"合法常量改写"), 经验 164 (套件池字)。

198. **⭐⭐ 让"扫描指针的基址/索引"两个 allocno 跨循环存活, 可把另一个指针从 r7 顶进 r8** (2026-09-08, 案例 sub_8024618, dest→r8)。
  - 目标第二循环同时存活 r4(i), r5(ptr), r6(constVal), r7(常量 scratch), r8(dest) 五个 callee-saved。
  - 若写 `ptr[i]` (ptr=data+idx), data/idx 在建 ptr 后即死, dest 只顶到 r7 (约 500 分差);
  - 改写 `data[idx+i]` 让 data 和 idx 都跨循环存活 → 压力 +1 → dest 进 r8。
    代价: 循环底条件 `ptr[i]` 一条 `adds rX,ptr,i` 变成 `adds rX,idx,i; adds rX,rX,data` 两条 (约 100 分),
    净赚约 400 分。同时 constVal 被顶到 r7 (目标 r6), 是剩余 home 差异主因。
  - 判定: 同类"目标要用高位寄存器 (r8) 存指针"卡点时, 优先试"把两三个扫描/索引变量保持存活",
    而不是加 `new_var` 这类只在单分支赋值的指针 (会制造 UB, 见 经验 18)。

199. **⭐⭐ 分支前读外设的局部指针在 else 块复用为参数指针，可引导 local-alloc 将后续 RMW 位标志地址分配到 r0** (2026-09-09, 案例 sub_80512C4 / sub_80513A0)。
  - 现象：在 `if ((*ioReg & 0x80) == 0x80) { ... } else { ...; gFlags |= 0x200; }` 中，目标生成 `ldr r0, =gFlags; ldrh r1, [r0]; ...; orrs r1, r2; strh r1, [r0]`（地址在 r0，值在 r1）。
  - 瓶颈：直接写裸宏 `if (REG_DISPCNT & 0x80)` 时，`0x02016000` 作为字面量实参传给 `LZ_InitContext`，产生独立临时 pseudo-reg，导致 else 块内 `gFlags` 的 RMW 操作被分配为 `ldr r2, =gFlags; ldrh r0, [r2]`（地址在 r2，值在 r0），卡在 Score 90 平台期。
  - 机理：声明局部指针 `vu16 *ioReg;`，分支前 `ioReg = (vu16 *)0x04000000;` 测条件，在 else 块内复用 `ioReg = (vu16 *)0x02016000; LZ_InitContext((u8 *)ioReg, ...);`。`ioReg` 的生命周期消除单独实参量，重塑 basic block 的 local quantity 排序，使 `&gFlags` 优先获得 `r0`。
  - 关联：经验 78（结构体成员与目的寄存器选择）、经验 87（局部变量兼职无关值买寄存器分配）、经验 98（局部变量反向影响寄存器分配）。
200. **⭐⭐ 将双槽条件检查提取为传对象指针的 static inline 函数 + 首句 ret=0，可同时破除 r4/r5/r6 寄存器分配轮换与入口初始化调度错位** (2026-09-09, 案例 sub_804E7EC, 破除 2026-09-02 挂起至今的 525 分卡点)。
  - 现象：函数入口执行 `movs r6, #0`（初始化返回值），随后将对象读至 `r5`，将第二个字段 `obj[0x92]` 读至 `r4`（目标 `r4 = v92`, `r5 = obj`, `r6 = ret`）。
  - 瓶颈：
    ① 若写单函数：`obj` 引用达 4 次，GCC 2.95 `global-alloc` 优先级公式 $\text{pri} \propto \lfloor\log_2(\text{refs})\rfloor \times \text{refs}$ 导致 `pri(obj) > pri(v92)`，`obj` 恒抢占 `r4`，与 `v92` 错位；
    ② 若提取 inline 函数且形参传 `(v91, v92)`：实参求值优先发生，导致 `ret = 0` 的发射延迟到 `ldrb r4, [r1]` 之后（0xe 处，目标在 0x4 处）；
  - 破局：声明 `static inline u8 CheckObj(u8 *obj)`，在内联函数首句声明 `u8 ret = 0;`，随后读取 `u8 v91 = obj[0x91]; u8 v92 = obj[0x92];`，内部辅以 `do { if (v91 == 0 && v92 == 0) return 0; } while (0);`。
  - 机理：内联函数的首行初始化在 AST 中先于局部字段读取展开，使 `movs r6, #0` 稳居第 4 字节；同时对象指针在子内联块内部求值，使跨调用的 `v92` 生命周期与频次恰好满足 `r4` 的分配阈值，三寄存器完美归位！
  - 关联：经验 117（global-alloc 优先级公式）、经验 177（内联函数隔离变量优先级）、`sub_804F0B8`（同模块兄弟函数写法）。
201. **⭐⭐ 复合赋值 `v = CONST; v &= var; if (v == 0)` 迫使常量居第一操作数，精准生成 `movs r0, #CONST; ands r0, r1` (打破 GCC 将常量规范化到右操作数的默认行为)** (2026-09-09, 案例 `sub_804B8E8` / `sub_804B7B0` / `sub_804BD54` / `sub_804BE90` 家族)。
  - 现象：在 ARM Thumb-1 中，`ands` 只有双操作数形式 `and %0, %0, %2`。目标汇编需要 `movs r0, #0x20; ands r0, r1; cmp r0, #0`（常量装入 r0 并作为目的寄存器）。
  - 瓶颈：若写常规写法 `if ((flags & 0x20) == 0)` 或 `if (0x20 & flags)`，GCC 前端在 AST 阶段会将常量规范化到右侧 `(and flags 32)`。在 reload 阶段由于 `flags`（在 r1）是第一操作数，reload 默认将目的寄存器绑定为 r1，生成 `ands r1, r0; cmp r1, #0`，永久相差 4 字节且破坏后续寄存器值。
  - 机理：显式声明临时变量 `u32 v = 0x20;`（先物化到 r0），然后执行复合赋值 `v &= flags;`。这在 RTL 中强行将 `v`（r0）指定为赋值目的操作数和第一操作数 `(set (reg:SI 0) (and:SI (reg:SI 0) (reg:SI 1)))`，迫使 GCC 输出 `ands r0, r1`！配合循环外 `int empty = -1;` 与 `*(s8 *)&entry[0] == empty`，成功将表基址提升进 `sl`，掩码提升进 `r8`，完全破解 2026-09-06 挂起至今的 battle_anim 核心四同构家族。
  - 关联：经验 54（RTL 交换性与操作数顺序）、经验 76（掩码折叠）、经验 87（临时变量兼职）、`FAMILIES.md`（sub_804B7B0 4x 家族）。
