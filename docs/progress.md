# 真 C 匹配进度

真 C 实现（非 INCLUDE_ASM）已匹配函数清单。ROM SHA1 保持通过。
最后更新: 见 git log。当前总量: 586/1067 = 54.9%。

## 本轮会话小结（遇到的问题 → 定位 → 解决）

| # | 问题 | 定位方法 | 解决 / 结论 |
|---|---|---|---|
| 1 | 接手时 `make` 编译失败，但 `sha1sum -c ll.sha1` 报"成功" | 看 make 尾部输出；磁盘上的 ll.gba 是旧产物，编译失败时不会重写 | 按反汇编证据升级 `code_0.h` 三个旧 `void ()` 原型（sub_8020A0C / sub_8020A7C / sub_8045F10）。教训→ EXPERIENCE 坑 8 |
| 2 | 逐指令完全一致，diff.py 却报 score=400 / 630 | `objdump -r` 看到 `R_ARM_ABS32 gUnk_XXX`；差异行全集中在池常量地址（被解码成 `movs r0,r0` 假指令） | **单函数 .o 的字面池未重定位导致的假象**。用绝对符号脚本部分链接 + `objcopy` + `cmp` 定性：只剩 bl 槽差异 = 已匹配。→ 经验 29 |
| 3 | sub_80531A8 怎么写都是 ptr/data 的 r1↔r2 互换（620 变体 + permuter 12k 次全失败） | 挖出 `agbcc -dl` 转储（→ `gccdump.lreg`）+ 读 `tools/agbcc/gcc/local-alloc.c:1435` 的 QTY_CMP_PRI 公式 | 根因定量定位：ptr = 4refs/24insns → pri 3333，抢不过池临时量 2refs/4insns → 5000。要翻盘需 n_refs≥6。**未破解**，已归档到 EXPERIENCE.md"失败案例存档"，后人不必重复穷举 |
| 4 | 别名技巧 `u32 *p1 = ptr;` 能把 n_refs 抬到 5，但生成的代码把 `ldr` 吞了 | 对比 `-dl` 输出与 .s | 判定为 GCC2 CSE 误编译（data 被当成 ptr），**禁止使用**，已列入死路清单 |
| 5 | 改 (已删)ASSIGNMENTS 时上一轮读到的整行消失 | `grep` 重读 → 发现已被别的智能体改掉（sub_80444A4 已合入） | 每次 edit 前先重读目标行原文，不凭上一轮内容拼 oldText。→ EXPERIENCE.md"并发编辑注意" |
| 6 | `-dl` 把 gccdump.lreg 拉到仓库根目录 | `git status` | 用完 `rm -f`；已写进文档提醒 |
| 7 | 合入用 r8 的 sub_80528C8 后 `make` SHA1 报 760 万字节差异 | `ls -l --time-style` + ll.map 归属 → code_1.c/code_1b.c/code_8005020.c 在 22:00-22:11 被其他智能体改到一半且缺 3 个 linker.ld 符号; 差异归属里没有 code_1c.o | 把目标函数换回 `INCLUDE_ASM("asm/matchings", ...)` 重编, 逐函数比对两个 code_1c.o → 1209 个其他函数完全一致 = **无 r8 泄漏**; 回退临时加的 3 个符号。→ 经验 50/51 |
| 8 | sub_80528C8 首试 score=3064, 差得远 | 逐行对比: 字节走位形式 / return 形式 / 局部变量个数 | 三处同时修正(`*(++p)` 链 + 两条 return + 6 个局部) → 降到只剩 1 个 bl 槽。→ 经验 45/46/47 |
| 9 | 别名/临时改 linker.ld 可能踩到别人的并发编辑 | 发现 `.scratch/` 里已有别的智能体的文件 | 改用 `.scratch/<自己的函数名>/` 子目录; 共享文件改前先 `grep -n` 重读, 改完立即 `make` 验证 |
| 10 | `cmp -l` 首差异看似位于 `sub_80002A0` 的 BL 指令 | 对比 `ll.map`/`code.s` 函数地址并用 `fncheck.py` 定位前序尺寸漂移 | 根因是 `sub_804AC60` 的 `gUnk_030009C5` 错用 `u8`，函数少 4 字节；改为 `s8` 后生成 `ldrb + lsls/asrs #24`，`sub_804AC60` 变为 96 字节，ROM SHA1 恢复通过。详见 EXPERIENCE 经验 60-61 |
| 11 | `sub_80531A8` 的 C 草稿始终是 `ptr→r2/data→r1`，目标却是 `ptr→r1/data→r2` | 620+ 种纯 C 变体、`agbcc -dl` 和 permuter 对照；固定寄存器扩展虽能命中但按项目规范禁止 | 保留 `INCLUDE_ASM`，不使用 `register ... asm("r1")` 或内联汇编伪造匹配；将该寄存器 home 卡点归档到 EXPERIENCE 经验 62 |
| 12 | `sub_800AADC`/`sub_800AAF8`/`sub_800AB18` 曾在并发构建中报 conflicting types | 单独预处理和 `make -B build/src/code_8005020.o` 重编确认声明/定义一致 | 判定为共享头文件并发编辑造成的瞬态；三个函数均通过 `fncheck`，遇到同类报错先按 EXPERIENCE 经验 63 排查竞态 |
| 13 | `sub_8048C30`/`sub_8048C80` 纯 C 候选接近但未完全匹配 | `fncheck` + 目标/候选逐条反汇编；C80 仅剩加法操作数顺序，C30 卡在指针 home 与 `cmp #0; blt` 形态 | 撤回未完全匹配候选，保留 `INCLUDE_ASM`，避免 ROM 变红；已确认 C80 需要宽返回 RNG + 栈帧占位，C30 目标表为 0x0839D5BC、步长 6 |
| 14 | `sub_8053138` 首版把 `gUnk_03004980` 池加载排在 `ldrb data[1]` 之前 | 单函数反汇编对照；将 `data[1]` 先存入 `index` 局部 | 真 C 已匹配，`fncheck`：52 字节、3 个池重定位、零差异；目标顺序为 `ldrb → 池加载 → adds` |
| 15 | `sub_805321C` 初版 C 让 `ptr/data` 落在 `r5/r4`，目标为 `r4/r5`，且目标真分支重读 `*ptr` | 去掉独立参数局部，改用 `sub_8015F50(data[1])`；真分支改为 `*ptr += 3` | 真 C 已匹配，`fncheck`：56 字节、2 个池重定位、1 个 BL 槽忽略 |
| 16 | `sub_8053270` 循环语义已还原但寄存器 home 整体错位 | 对照 m2c 的 do-while 结构和 0x1FF/-0x200 常量；当前目标 `ptr/data/limit = r6/r5/r7`，候选仍为 `r7/r6/r5` | 暂保 `INCLUDE_ASM`，避免未匹配 C 破坏 ROM；后续从局部变量优先级继续攻 |
| 17 | `sub_8052F44` 候选 C 的 `ptr/data/count/index` home 与目标不一致 | 已按要求建立 `permuter/sub_8052F44` 并尝试运行；当前环境的 multiprocessing forkserver 被沙箱拒绝（`PermissionError`） | 暂保 `INCLUDE_ASM`；目标语义已确认是队伍 ID 匹配后按 `data[2]` 选择跳转表或 `+4` |
| 18 | `sub_8044514` 初版 `fncheck` 在 `+0x30` 起出现差异 | 逐条对照目标存储序列，发现遗漏 `gUnk_03000857 = 0`；补注册 IWRAM 符号并重编 | 真 C 已匹配；`fncheck`：96 字节、9 个池重定位、零差异。初始化顺序必须保持为 `0x844/845/856` → 条件设置 `0x85A` → `0x857/85C/886/888` |
| 19 | `sub_8044574` 与 `sub_8044514` 逻辑相似但入口有三个截断参数 | 对照入口 `lsls/lsrs`：参数确认为 `s16, u16, u8`；复用状态初始化顺序，末尾改写 `arg1/arg2` | 真 C 已匹配；`fncheck`：108 字节、9 个池重定位、零差异。相似函数应先核对调用方截断，不能直接复制原型 |
| 20 | `sub_8045328` 主体候选已对齐，但结果标志的 `movs r2,#0` 调度提前 2 条指令 | 建立 `permuter/sub_8045328` 四件套；`-j1` 沙箱可运行，最佳 score=60。尝试结果局部、条件反转和 `new_var` 均无法让 GCC2 延后该赋值 | 暂保 `INCLUDE_ASM`，避免未匹配 C 破坏 ROM；最佳候选保留在 permuter 输出目录，后续从局部变量生命周期继续攻 |
| 21 | `sub_804F250` 参考代码中的结构体字段注释与实际偏移命名不一致 | 直接核对目标 `lsls index,#2` 和连续 `strb [base] / [base,#1]`，确认每项 4 字节且只清前两个字节 | 真 C 已匹配；`fncheck`：48 字节、2 个池重定位、零差异。新增 `Unk_03000E08[]` 与 `gUnk_03000E30` 符号 |
| 22 | `sub_8016978` 已被自动脚本标为 `[1]`，但分配表仍写“待开始”，且 `fndiff` 报 score=400 | 重读源码/YAML/分配表，逐条核对目标汇编，再跑 `fndiff`、`fncheck` 和全量 `make` | 状态冲突是文档陈旧；score=400 是 `gInventory` 池重定位假差异。`fncheck`：50 字节、1 个池重定位、零差异；SHA1 通过。分配表已同步 |
| 23 | `sub_801B878` 的自然范围判断被 GCC2 合并，且首版把对象指针放进 `r2`，破坏第三实参 | 逐版跑 `fndiff`：范围 `if` 被化成 `kind -= 6; kind <= 2`；补第三参数后 `arg0` 从 `r2` 回到目标 `r3`；连续同体 `case 6/7/8` 最终得到目标比较链 | 真 C 已匹配。第三实参是 `u8 *`，fallback 调 `sub_801A884(arg0,arg1,arg2)` 时入口 `r2` 必须原样保活；`switch` 才生成 `cmp #8; bgt; cmp #6; blt`。`fndiff=0`，`fncheck`：50 字节、2 个 BL 槽忽略，SHA1 通过 |
| 24 | `sub_801B878` 全量终验一度通过，随后最新共享树因 `sub_8048818` 原型冲突无法重编 `code_1b.o` | 检查 claim 表确认 `sub_8048818` 正由其他智能体处理；不修改其半成品，改跑 `make -B build/src/code_1.o`、`fndiff` 与 `fncheck` | 最新 `code_1.o` 强制重编成功，目标仍为 `fndiff=0` / `fncheck=OK`。共享树瞬态冲突按经验 63 归属，不回退或代改其他智能体的文件 |
| 24 | `sub_8019748` 的直接 `u8` 五形参版本有相同入口截断，却少 push `r6`，池加载推迟，score=629 | 与已命中的 `u32` 五形参 + 五个 `u8` 收窄局部版本做受控 `fndiff`；另验证无 `tbl` 局部的 `base.c` 仍命中 | 决定因素是宽形参与独立窄局部造成的伪寄存器生命周期，不是必须拆 `tbl`。胜出版本 `fndiff` 仅剩池假差异；`fncheck`：60 字节、1 个池重定位、零差异；SHA1 通过 |
| 25 | `sub_801B8AC` 首版把 case 6 贯穿到 case 7/8，语义相同但少 4 字节 | `fndiff` 显示 GCC2 合并了两处 `return arg1`，将 `sub_801B570` 块外置；在 case 6 内显式返回 | 真 C 已匹配；case 6 与 case 7/8 必须各保留一份返回赋值。`fndiff=0`，`fncheck`：58 字节、2 个 BL 槽忽略 |
| 26 | `sub_80166A4` 的自然哨兵循环先是控制流不对，改对短路顺序后又多出 `0x08095029` 池 | 用 `i <= 7 && (ch = *src++) != 0` 复现“先计数、再读字节”的旋转循环；把硬编码 `0x08095028` 改为 extern 表符号 | 真 C 已匹配。具名符号阻止 GCC2 将首次 `src++` 折成新常量池；`fncheck`：86 字节、1 个池重定位、1 个 BL 槽忽略 |
| 27 | `sub_8016758` 的比较链和 case 指令数已对，但 `kind/bit` 为 `r3/r1` 而目标是 `r1/r3`，尾存的地址/值寄存器也互换 | 保持 switch 不动，仅把尾部 `x * 2` 提成 `int xOffset`；该局部改变伪寄存器生命周期，使前后两组 home 一起归位 | 真 C 已匹配。`fndiff=400` 仅是池重定位假差异；`fncheck`：124 字节、1 个池重定位、0 个 BL 槽忽略、零差异 |
| 28 | `sub_801B81C` 的自然 10 参数 setter 只差一次 `field_14` 写入被 GCC2 延后到后三个栈参数加载之后，`fndiff=130` | 对 14 种普通局部/表达式形态做消融，再由 permuter 找到零分候选；逐项移除 `long long`、临时变量和重复写，确认唯一决定因素是 `field_10` 后第二次 `field_14 = arg6` | 真 C 已匹配。第二次写会被优化删除、不会增加指令，但改变第一次写的调度；必须保留原作者冗余。普通 RAM 不得用 `volatile` 伪造顺序，`volatile` 仅用于 IO。`fndiff=0`，`fncheck`：90 字节、零差异。全量构建当前另有从 `0x080003D4` 开始的共享树 `+4` 位移，早于本函数，不回退他人改动 |
| 29 | `sub_8016E30` 首版把 `gUnk_03004DF0+0x1C` 折进池，并缓存串行 IRQ 共享的目的指针；修正后又只差调用后 `ldr`/`movs` 反序 | 先设具名 `state` 基址；对确由串行 IRQ 异步共享的指针字段使用 volatile，而非给普通数据加 volatile；把 `i = 0` 明写在 `packet` 读取之前 | 真 C 已匹配。入口 r0 确认为 24 字节源指针；`fncheck`：80 字节、1 个池重定位、1 个 BL 槽忽略、零差异 |
| 30 | `sub_8016F30` 首版多入口复制、零值重复物化，bit 6 使用不同指令链；分别写 `REG_SIODATA8`/`REG_SIOCNT` 又拆成两个 IO 地址池 | `mode` 改 `u32`；独立 `u32 zero` 供 `strb/str` 共用；bit 6 写 `(v<<25)>>31`；`u16 sioData=0xFEFE` 让池值直落 r0；以 `REG_ADDR_SIOCNT` 建共享寄存器块指针 | 真 C 已匹配。`fndiff=800` 只是两个符号池；`fncheck`：144 字节、2 个池重定位、0 个 BL 槽忽略、零差异 |
| 31 | `sub_801D12C` 的自然 if 版本被 GCC2 压缩范围判断，首版少 16 字节；改成嵌套 switch 后第二条路径仍少一个无条件分支 | `obj[0xAB]` 用 `s16 value` 保留 `bgt/bge`；外层显式空 `case 4`；两个内层 switch 都列出 1..7 和 8，第二个再显式保留空 `case 0` | 真 C 已匹配。空 case 会改变比较树，不能按语义删除；`fndiff=0`，`fncheck`：110 字节、零池、零 BL、零差异 |
| 32 | `sub_801A684` 从 540 分压到 10 分后，只剩两条独立 `movs #0` 反序 | `do {} while (0)` 先恢复前缀值链；再用 `agbcc -dl` 确认半字零的伪寄存器先于字节零生成。把字节零改写为 `off0 & ~off0`，让 combine 在 RTL 阶段折叠 | 真 C 已匹配。最终仍是两个普通零值且无额外指令；不使用 `volatile`。`fndiff=0`，`fncheck`：110 字节、零差异。→ 经验 83 |
| 33 | 想用 qty 优先级表解释 `sub_8014084` 的三个寄存器 home，表里却找不到 `i`/数组基址/载入字节 | 给 agbcc 打诊断补丁（`scripts/patches/agbcc-qty-dump.patch`），在 `block_alloc()` 末尾读 `qty_order` 打印；读 `local-alloc.c` 确认 `qty_birth/death/n_refs` 是 `local_alloc()` 的 alloca，到 `dump_local_alloc` 时已悬空 | **QTY_CMP_PRI 只管块内伪寄存器**：跨基本块的值（循环计数器、跨分支指针）由 global.c 分配，根本不进这张表。实测 sub_8014084/sub_80531A8 的表里只有 life 2-10 的短命量。工具编成独立二进制 `bin/agbcc_qtydump`（10 个 C 文件 的 .s 与原编译器逐字节相同），构建管线不受影响。→ 经验 88 |
| 34 | `sub_80140D0` 逐条全对，只差奇路径把 `(u8)` 截断补在 `orrs` 之后（630 分） | 对照两条分支：偶路径无末尾截断、奇路径有，说明截断落点不同而不是漏指令 | 把 `(u8)(nib << 4)` 拆进 u8 临时 `hi`，combine 合成 `lsls #0x1c; lsrs #0x18`，且两操作数皆 u8 后末尾不再截断。首试即降到只剩假池差异。同招直接拿下 `sub_8014124`（挂起项，原 1385 分）。→ 经验 86 |
| 35 | `sub_8014084` 指令形状全对，只差 `i`/字节/指针三个 home 加一条 `adds r3, r1, #0`（660 分；手写 12 个变体全部更差） | 交给 permuter（59k 次迭代，最好 score 5）；读中奖源码发现它把一个 `unsigned int` 变量先当地址、循环里再当载入字节 | 计数器地址因此在入口块内死亡（local-alloc），循环里的地址是另一个跨块伪寄存器（global-alloc），loop 提到 preheader + cse2 把冗余常量加载化成拷贝。三处 home 一次归位。**「指令全对只差几个 home」应优先试变量兼职**。→ 经验 87 |

新增匹配（本会话共 12 个，涉及 code_1c.c 与 code_1b.c）：
**sub_8052BA0**(37) / **sub_8052C24**(57) / **sub_8052C90**(38) / **sub_8052D4C**(37) /
**sub_8052DCC**(37) / **sub_8052808**(43) / **sub_8052878**(42) / **sub_8052CF0**(45) /
**sub_80528C8**(47) / **sub_8044514**(47) / **sub_8044574**(53) / **sub_804F250**(1)。其中 7 个首试即中, 5 个经 1-4 次写法修正; 每个都用
"部分链接 + cmp" 确认到只剩 bl 槽或零差异。
新沉淀代码生成规律：经验 35-51（见 EXPERIENCE.md）。


| 函数 | 位置 | 区域 | 文件 | 关键技术点 |
|---|---|---|---|---|
| sub_8020AE4 | 0x08020AE4 | 0x8020簇 | code_1.c | 首个演示; m2c 直转 + while 循环链表遍历 |
| sub_8020C2C | 0x08020C2C | 0x8020簇 | code_1.c | extern 符号防折叠(gUnk_0839CE7C); fnptr 落 r1 因 r0 被实参占用; 单表达式展开顺序 |
| sub_8020C58 | 0x08020C58 | 0x8020簇 | code_1.c | continue 形式; 乘积在左; `if(!(...))` 分支极性 |
| sub_8020CC4 | 0x08020CC4 | 0x8020簇 | code_1.c | 7 参(4 寄存器+3 栈); 表步长 0x14; 无 tbl 局部重复表达式由 CSE 处理; newval 临时变量; **触发 GCC2 泄漏 → 拆分 code_1b.c** |
| sub_8020D50 | 0x08020D50 | 0x8020簇 | code_1b.c | u8 截断=被调首参类型; 修正 sub_804BBDC/sub_801D19C 头文件原型 |
| sub_8020DA0 | 0x08020DA0 | 0x8020簇 | code_1b.c | 同族 0xB0/0xBE 字段簇; 一次合入通过 |
| sub_8020DF0 | 0x08020DF0 | 0x8020簇 | code_1b.c | m2c 破案"死代码"=第三实参; 全局当数组直用; **linker 别名符号必须在 SECTIONS 外** |
| sub_8020F08 | 0x08020F08 | 0x8020簇 | code_1b.c | 最自然 for 形式即可: GCC2 自动把闩自增吊到 bl 前; gUnk_087ED6A8 新符号(SECTIONS 外) |
| sub_8020F4C | 0x08020F4C | 0x8020簇 | code_1b.c | 结构体成员访问形式(ip 缓存+逐成员寻址); 新增 7 个 IWRAM 符号(0x618-0x624) |
| sub_8020FB8 | 0x08020FB8 | 0x8020簇 | code_1b.c | RMW 拆两条赋值(permuter 找到); void* 形参+内部 cast 解决头文件类型冲突; struct 补 field_37/38 |
| sub_802103C | 0x0802103C | 0x8020簇 | code_1b.c | (前人已写好真C, 仅同步 yaml [0]→[1]) |
| sub_8021064 | 0x08021064 | 0x8020簇 | code_1b.c | 7 项结构体数组清零(0x670, 步长4); 表符号 gUnk_0861C664 步长 0x20; 开局 score=0 |
| sub_8021700 | 0x08021700 | 0x8020簇 | code_1b.c | **if/else-if 要写成 switch**(GCC2 对 switch 用"链条+体外放置"布局, 与 if/else-if 内联布局不同); 不要缓存全局到局部(目标每次重读 gUnk_03000812); Unk_8020F4C 扩展为完整 0xC8 对象结构 |
| sub_804DD70 | 0x0804DD70 | 0x804D区 | code_1b.c | 开局 score=0; 与 sub_8020C2C 同款(0x71 索引表 0x0839CE38, fnptr 落 r2 因 r0/r1 被实参占用); 表类型 void(*)(u8*, u32); **合入触发第二次泄漏 → 拆分 code_1c.c** |
| sub_8019148 | 0x08019148 | 0x8019区 | code_1.c | 4 参只用 r3; 清屏循环(EWRAM/VRAM 指针局部); **mask 链必须逐条语句**(单表达式会被 GCC2 折叠); **`do{}while(0)` 屏障**阻止末位 mask 合并(permuter 发现); **i=0 必须在指针赋值之后**(否则 movs 提前); 语义=清调色板/OAM缓冲+开BG0显示+配置REG_BG0CNT; 已改用 REG_DISPCNT/REG_BG0CNT 宏(volatile 不影响代码生成, 已验证 0 字节差) |
| sub_8021184 | 挂起 | 0x8020簇 | - | 见"待研究" |
| sub_80210C0 | 0x080210C0 | 0x8020簇 | code_1b.c | 10 参调用+表 gUnk_0839B2A4; **tbl 局部存活判据**: 目标基址池加载位置很早→用 tbl 局部(首语句); `0xDA<<1` 字面量形式; (arg1<<5) 的 u8 截断舞步 GCC2 自动生成 |
| sub_8020EAC | 0x08020EAC | 0x8020簇 | code_1b.c | 与 sub_8020E90 同族: u8* 裸索引 arg0[0xBE] + gUnk_03000748; 两次读被 CSE 成单次 ldrb; 原型 void* → u8* |
| sub_802093C | 0x0802093C | 0x8020簇 | code_1.c | **switch 分发形状**(beq 正跳转链, 不是 if/bne); 分支内只算地址+尾部公共存储; 载入值用命名临时 new_var 才落 r1; field_BC 是 s8(lsls/asrs); *(u32*)(arg0+0x88) 是指针 |
| sub_8020AB0 | 0x08020AB0 | 0x8020簇 | code_1.c | 首试 score=0; u8 buf[8] 传 sp; sub_80489E8(sub_8018864(), buf, 0, 0x6B); `ret != 0` → negs/orrs/lsrs #0x1f; if(==0) return 1 极性 |
| sub_80489C8 | 0x080489C8 | 0x8048簇 | code_1b.c | 首试 score=0; min(diff, arg1): s32 diff = *(u16*)(a+0x72) - *(u16*)(a+0x70); blt 有符号比较; 原型 void() → u16(u8*, u16) |
| sub_8048B88 | 0x08048B88 | 0x8048簇 | code_1b.c | `if (<=10) { return 表[idx*4]; } return 0;` 极性(GCC 把 return 体内联、load 块外置); gUnk_0839CC4C 注册 linker.ld(SECTIONS 外) 防常量折叠; 字节视图 u8[](idx*4), 与 8BAC 的 struct 视图(+2)并存 |
| sub_8048A68 | 0x08048A68 | 0x8048簇 | code_1b.c | **s16 命名局部 a/b 才能阻止 GCC 把 ldrsh 合并成 ldrh**(表达式内联会被优化); diff 声明 s16 → lsls#0x10+cmp/ble(无 asrs, combine 折叠); 原型 u8(u8*) |
| sub_8048A88 | 0x08048A88 | 0x8048簇 | code_1b.c | 快排递归; (s8) 参数(lsls/asrs)但 val 声明 u8(lsrs); 首试 score=0; sub_8046E18=partition |
| sub_8048ACC | 0x08048ACC | 0x8048簇 | code_1b.c | 排序入口; gUnk_030008F0(u8)/gUnk_030008EC(u32) 新注册 iwram+linker.ld; val=(s8)(arg1-1) 但 val2=u8; 池差异渲染仅 permuter 环境现象 |
| sub_801B8FC | 0x0801B8FC | 0x801B簇 | code_1.c | 表查找: val=*(u16*)(arg1*2+*(u32*)(arg0+0xC)) 先求值; current=(u16*)(base+val+2); while(*(current+1)<=arg2) current+=2; 首试 score=0; (u8)arg1*2 = lsls#0x18+lsrs#0x17 |
| sub_801B81C | 0x0801B81C | 0x801B簇 | code_1.c | 10 参数对象 setter：写入 2 个 u32、5 个 u16、3 个 u8 字段并清零 `field_22/field_1C`。窄形参签名决定栈参数装载与 `arg9` 的 u16 截断；`field_14 = arg6` 必须在 `field_10` 后冗余再写一次，第二次写虽被 DSE 删除，却使第一次 store 在调度中紧跟 arg6 的 load。普通 RAM 不使用 volatile。`fndiff=0`，`fncheck` 90 字节一致 |
| sub_801B878 | 0x0801B878 | 0x801B簇 | code_1.c | `kind=(*(u16 *)(arg0+0x18)&0xF)`；case 6/7/8 调 `sub_801AD0C(arg0)` 后原样返回 `arg1`，其余转发三参数到 `sub_801A884`。关键：`kind` 用 `s16` 得有符号分支；连续同体 `switch case` 才保留上下界比较；第三参数使 `r2` 跨 fallback 保活、迫使 arg0 落 r3。`fndiff=0`，`fncheck` 50 字节一致 |
| sub_801B8AC | 0x0801B8AC | 0x801B簇 | code_1.c | kind 6 调 `sub_801B570` 后返回 arg1；kind 7/8 直接返回 arg1；其余调用 `sub_801B0B8`。case 6 不能贯穿到 7/8，否则 GCC2 合并返回块。`fndiff=0`，`fncheck` 58 字节一致 |
| sub_801A684 | 0x0801A684 | 0x801A簇 | code_1.c | 初始化两组数据偏移指针和状态字段；低 4 位类型为 6..8 时调 `sub_801A6F4`，否则转发到 `sub_804C2FC`。整段初始化的 `do {} while (0)` 固定前缀值链；`zero8 = off0 & ~off0` 让 GCC2 先物化字节零，再物化半字零。普通 RAM 未使用 volatile。`fndiff=0`，`fncheck` 110 字节一致 |
| sub_8053104 | 0x08053104 | 0x8053簇 | code_1c.c | script 处理器; if(data[1])→800AAA4 else 800AAC0; 实参 data[2]+(data[3]<<8) 用加法不是或; 首试 score=0; gUnk_02016000/02016200 注册 ewram 段 |
| sub_8052808 | 0x08052808 | 0x8052簇 | code_1c.c | **首试即字节一致**(只剩 1 个 bl 槽); 地址表达式必须写成 `(u32)(arg0*18) + (u32)gUnk_0862D574 + gUnk_03000F2A*2` —— 先加 base 再加 arg*18 的写法会让 GCC2 先算指针部分, 指令顺序颠倒; `arg*18` = `lsls#3; adds; lsls#1`; 尾部 `gUnk_03000E70 |= 0x40` 直接匹配; 新增 gUnk_0862D574(linker.ld SECTIONS 外) + gUnk_03000F2A |
| sub_8052878 | 0x08052878 | 0x8052簇 | code_1c.c | **零字节差异**(本函数无 bl); 关键: 必须引入 `u16 ofs` 临时量才能把两个 `*ptr = ` 存储拆成两条(目标未合并), 且表访问写 `*(u16 *)((u32)gUnk_02016000 + data[1]*2)` 才能把池加载压到 `ldrb/lsls` 之后; 新增 gUnk_03000E78(u8) / gUnk_03000E80(u32[]) |
| sub_8052CF0 | 0x08052CF0 | 0x8052簇 | code_1c.c | **零字节差异**; 7 个平行 `池加载+ldrb+strb` 赋值串; ptr 全程留在 r0 不产生入口复制(经验 48); `gUnk_03004614 = data[2] + (data[3]<<8)` 用加法; 0x03001944 要用已有的具名符号 `gMainGameState`(经验 49); score 假高 2610 = 7 个未重定位池 |
| sub_8048818 | 0x08048818 | 0x8048簇 | code_1b.c | **接力完成**（智能体 B 推到 25 分 + 留下“new_var(u32)”线索，本侧用 `u32 formation` 收尾）; 目标 `ldrb r2,[r0]` + `adds r0,r2,#0` + `cmp r2,#0`/`subs r0,r2,#1` 要求测试与减法都**读 formation(r2)**、结果写 idx(r0); 写成 `u8 formation` 时 GCC2 会把 load 放进临时量再**拷两份**(`adds r3,r0; adds r2,r0`, score 845), 改 `u32 formation` + `u8 idx` 后 load 直接落 r2 → 命中(经验 97); 语义: 编队号 1-based → `idx = formation?:formation-1` 转 0-based 索引进 `PlayerStats.lv`, 而第一个实参仍传 1-based 原值; `*0xC8` 用 `muls`(非 2 幂), `*0x40` 用 `lsls #6`; 头文件两个旧 `void()` 原型需同步升级(`sub_8048818`/`sub_8009F70`) |
| sub_804DE20 | 0x0804DE20 | 0x8048簇 | code_1b.c | **关键: 原代码没有中间变量, 把嵌套表达式重复写三遍** —— `if (gUnk_03004980[gUnk_0839CFAA[i]] != 0) { D48[DDD].field_0 = gUnk_0839CFAA[i]; D48[DDD].field_1 = gUnk_03004980[gUnk_0839CFAA[i]]; DDD++; }`。引入 `u8 a = gUnk_0839CFAA[i];` 反而让 GCC2 把 `0839CFAA`/`03004980` 两个字面池**提前到第一个循环之前**(score 2020/2695); 全部内联后自然落回循环前的正确位置 (score 1600 = 4 池 × 400, fncheck 零差异)。结构体数组必须用 `Unk_03000DEntry`(4 字节) 而不是 `u8[]` 手拼 `i*4`/`i*4+1` —— 后者 GCC2 会把 `+1` 折进下标变成 `adds r0,#1; adds r0,r0,base`, 目标是 `[r0,#0]`+`[r0,#1]`(经验 21 同类)。第二个循环里 `DDD` 被前面的 `strb` 隔开 → GCC2 必须重读并重算地址(别名屏障), 与目标一致 |
| sub_8050014 | 0x08050014 | 0x8050簇 | code_1c.c | **首试即逐指令全对**(score 2400 = 7 个未重定位池, fncheck 零差异); **主循环 + 按键处理**: `0x04000130` 是 `REG_KEYINPUT`(不是定时器!), `mvns` = `~REG_KEYINPUT` 把低有效键值取反成按键掩码; `bics r0, r3` = `keys & ~gUnk_03000F2C`(新按下边沿), 注意 F2E 先算后 F2C 才赋值; 存完再 `ldrh` 读回当实参(经验 35); **间接调用** `gUnk_0862D434[*(u8 *)gUnk_03000E6C](&gUnk_03000E6C)` 单表达式 → 自然得到 `bl _call_via_r1` 且表基址池在前(经验 8); 表元素类型 `u16 (*)(u32 *)` → 返回后 `lsls/lsrs #0x10` 截断; `while (...) {}` 空体自然得底部测试循环(无入口跳转); 双条件 `if ((E70&1) != 0 && (E70&0x200) == 0)` 得两个早退到同一尾部; `0x200` 编成 `movs #0x80; lsls #2` |
| sub_8052EC0 | 0x08052EC0 | 0x8052簇 | code_1c.c | **首试即逐指令全对**(score 800 = 2 池 × 400, fncheck 零差异); 结构体乘法: `*40` = `(x<<2+x)<<3` → `gUnk_03002E80[i].sprNodeIdx`(CharacterObject 尺寸 0x28), `*20` = `(v<<2+v)<<2` → `&gSpriteNodePool[v]`(SpriteNode 尺寸 0x14); **先查已有具名符号/结构体再手拼偏移**: 0x03003AC0 就是 `gSpriteNodePool`, 0x03002E80 就是 `CharacterObject[]` 且首字段叫 `sprNodeIdx`(不是 field_0); `u8 sub_8004BFC()` 返回后 `lsls/lsrs #0x18` 截断入 r2; `if (v <= 0x6F)` → `cmp #0x6f; bhi skip`; data(r4)/base(r6) 跳调用存活→被调保存寄存器 |
| sub_8052580 | 0x08052580 | 0x8052簇 | code_1c.c | **首试即逐指令全对**(score 3600 = 9 个未重定位字面池 × 400, fncheck 零差异); 无参初始化函数: 9 个平行 `池加载+存储` + 一个 `for(i=0;i<=7;i++) gUnk_03000E80[i]=0;` 清零循环; 细节: `i` 是 u8 → 自增被编成 `adds r0,r1,#1; lsls #0x18; lsrs r1,#0x18` 且**吊在比较之前**(经验 12); `movs r2,#0` 出现两次(CSE 未合并循环外的零常量); 尾部无 `movs r0,#N` → 定为 `void`; 新增 5 个 IWRAM 符号(E6C/E72/ECA/ECB/ECC) |
| sub_80528C8 | 0x080528C8 | 0x8052簇 | code_1c.c | **首试 3064 → 四改后只剩 1 个 bl 槽**; 三个关键写法: ① 字节走位必须 `a1=*(++p); ...; a5=*(++p); a6=*(p+1);`(经验 45, 数组下标不匹配); ② 6 个 u8 局部才能凑出 r8 序言(经验 47); ③ `if (a6==1) return 0; return 1;` 而不是 `return a6!=1;`(经验 46); 6 参调用(4 寄存器+2 栈); **已验证无 r8 泄漏**(经验 51 方法) |
| sub_8052BA0 | 0x08052BA0 | 0x8052簇 | code_1c.c | script 处理器; **首试即字节一致**（仅 bl 重定位待链接）; data[1]==0xFF 分支写 gUnk_03004614 后**再读回**传参（原代码就是两次独立访问, 非寄存器直传）; 同函数内两种拼法共存: 赋值侧 `data[2] + (data[3]<<8)` → `adds`, 实参侧 `data[2] | (data[3]<<8)` → `orrs`; 调用后 `*ptr += 4` 必须重读 `*ptr`（r3 被调用展平）; push {r4,lr} + pop{r4}/pop{r1}/bx r1 = -fprologue-bugfix 形态 |
| sub_8052C24 | 0x08052C24 | 0x8052簇 | code_1c.c | script 处理器; **首试即字节一致**; `switch (data[1])` 无 default 标签即可（GCC2 生成 `cmp#1;beq; cmp#1;bgt; cmp#0;beq` 二分比较链, 非跳转表）; case 体按 0/1/2 源序外置, 最后一个 case 自然落入公共尾部; `gUnk_030025F4 = 0xFF` 在**每个 case 里重复写**（提出到 switch 外会变少一份池加载）; data 跳调用存活→分配器呷 r4(被调保存寄存器); 尾部 `*ptr += 2; return 1;` |
| sub_8052C90 | 0x08052C90 | 0x8052簇 | code_1c.c | script 处理器; **首试 score=0**（本函数无字面池, 所以 score 直接可信）; `switch (data[1])` 同样二分比较链; **case 0 缺 `break` 贯穿到 case 1**（目标里 case0 body 末尾无 `b 尾` 且与 case1 物理相邻）; 分发值只读一次→留在 r0 无独立 home, ptr 跳调用→r4 | 
| sub_8052D4C | 0x08052D4C | 0x8052簇 | code_1c.c | script 处理器; **首试即字节一致**（只剩 1 个 bl 槽）; `if (sub_8001030(data[1] \| (data[2]<<8)) != 0)` —— 目标只有 `lsls r0,#0x18` 无配对 lsrs = 只测零; 真分支算 EWRAM 脚本地址, 假分支 `*ptr += 4`; 两分支各自赋 `*ptr` 但 GCC2 合并为单一尾存; ptr→r5 / data→r4 (data n_refs 大先拿 r4); 新增 `u8 sub_8001030(u16)` 原型 |
| sub_8052DCC | 0x08052DCC | 0x8052簇 | code_1c.c | 与 sub_8052D4C **完全同构**, 仅把 sub_8001030 换成 sub_80010AC(另一张标志位图); 直接 sed 改名即首试字节一致; 新增 `u8 sub_80010AC(u16)` 原型 |
| sub_8017588 | 0x08017588 | 0x8017簇 | code_1.c | 双层条件极性: ret=0; if(gUnk_03004DF0[1]==2) if(*(u16*)(base+0x18+field_4D*24)==0x4E4C) ret=1; 全局基址+内偏移两段寻址(先 adds r2,#0x18 再 adds r0,r0,r2); u16 数组视图 |
| sub_8016978 | 0x08016978 | 0x8016簇 | code_1.c | 扫描 `gUnk_0839CFAA[0..15]` 对应的库存槽，首个非零项返回 1-based 位置，否则返回 0。`u8 i` 很关键：循环闩的 `i + 1` 生成 `lsls/lsrs #0x18` 截断，随后用截断后的 r1 做 `cmp #0xF; bls`；改成 `s32 i` 或 `< 16` 可能改变尾测形状。`fndiff` score=400 只是假池差异，`fncheck` 50 字节零差异 |
| sub_801761C | 0x0801761C | 0x8017簇 | code_1.c | 参考代码数组视图是错的(field_0[index+1] 基于 0 偏移); 正解: *(u16*)(base+0x18+index*24)=0, 两段寻址 (ldr 后 adds r2,#0x18 再合并); ldrh strh + sub_80170D0 尾调 |
| sub_80166A4 | 0x080166A4 | 0x8016簇 | code_1.c | `arg0==0` 早退；否则从 `gUnk_08095028[arg0*8]` 取最多 8 个非零字符，写到 `0x02005800 + arg2*64 + arg1*2`。短路条件次序决定目标循环旋转；ROM 表必须具名以避免额外的 base+1 池 |
| sub_8016758 | 0x08016758 | 0x8016簇 | code_1.c | 从 `gUnk_03000198` 取动画位 `(state>>3)&1`，按 kind 0..3 选择图块 0x826/0x26/0x428/0x28，default 为 0x3F，再加 0xB240 写到 `0x02005800+x*2+y*64`。尾部必须先写 `int xOffset = x * 2`，否则 GCC2 会同时交换 switch 活跃值和尾存地址/值的寄存器 |
| sub_8016E30 | 0x08016E30 | 0x8016簇 | code_1.c | 以 state[0xB] 和 state[2]^state[3] 构造包头，清校验字段，`CpuSet` 从入口 src 复制 24 字节到包偏移 4；累加 14 个 u16 后写 `~sum-0x10`，置 state[4]=1。具名 state 防止 +0x1C 折进绝对池；该指针字段由串行 IRQ 异步共享，volatile 重读有实际语义 |
| sub_8016F30 | 0x08016F30 | 0x8016簇 | code_1.c | VBlank 串行泵：活动且 ready 时交换 0x24/0x28 双缓冲，按需交换 0x1C/0x20，记录 SIOCNT bit6，发送 0xFEFE 并启动 SIO/Timer3；非活动时置中断检查 bit7。宽 mode 避免入口复制，独立 zero 和 u16 sioData 决定 r0 复用；SIO 必须共享 0x04000128 基址 |
| sub_801D12C | 0x0801D12C | 0x801D簇 | code_1.c | 当 obj[0xBE]<=10 时更新 obj[0xA2]：输入状态 0..2 下，obj[0xAB] 的 1..7→1、8→2、0 且两个 u16 坐标相等→3；输入 5 只做前两种映射。`s16 value` 决定有符号比较；外层空 case4 和第二内层空 case0 决定目标比较树 |
| sub_80445E8 | 0x080445E8 | 0x8048簇 | code_8044394.c | 遍历 gUnk_03000840[i]&0xF 索引池槽, 命中写 gUnk_03004F90。**关键坑**: ① `off + (u32)base`(off 在前) 才出 `adds r0,r0,r3`(目标), 反序出 `adds r0,r3,r0`(差 1B); ② 0xF 必须内联不可抽 mask 变量(抽了子 sp 槽与调度多差 112B); ③ off 用 u32 中间变量(经验 122: 声明在函数开头)。fncheck OK 152B|
| sub_8045EB8 | 0x08045EB8 | 0x8048簇 | code_1b.c | ✅ 2026-09-02 gpnux (88B, fncheck OK)。语义: obj+0x8D..0x92 六个角色编号逐个查 `sub_804DD90(id, 1)` (= gUnk_087EA580[id*12+10] 指向的 AI 字节 bit6-7 分类), 命中 1/2/3 → obj+0xB8 的 u16 标志置位 1/2/4。三处定死形状: ① **`u8 sub_804DD90(u8, u8)` 原型**才产生 `lsls/lsrs #0x18` 返回值截断 (宽原型/K&R 不截, 且会把 0x6C+0x21 折成 0x8D); ② 取号必须走 `Sub6C{pad[0x21], ids[6]}` 结构视图 → 目标 `adds r0,r6,#0; adds r0,#0x21; adds r0,r0,r5` 三条; 写成 `obj+0x8D` 少一条, 写成 `base[0x21+i]` 变 `add r0,r5,r6` 两条 (取址树须是 (p+0x21)+i); ③ **多出来的 `movs r7, #0` 不是死代码, 而是一次被 CSE 折叠的真读**: 函数末尾 `*flags |= extra;` (extra 恒 0) → RMW 折成零指令, 但 flow 已判 extra live, 定义指令存活并逼出第 4 个 callee-saved。实测: 完全没用的 `u8 x = 0;` / `x++` / `if (x) {}` / `volatile u8 x = 0;` / 结构体局部 `s.a=0;`+`if (s.a)` / `for (x=0; x<0; x++) {}` 的初始化**全被 egcs 删干净**, 恒等读是唯一通路 (新规律见 EXPERIENCE 坑11)。声明顺序 p/i/flags/extra = r6/r5/r4/r7, 把 extra 提前会整体错位。前一轮 `new_var=i` 冻结条件变量的方向错误, 已弃用 |
| sub_8045F10 | 0x08045F10 | 0x8048簇 | code_8044394.c | ✅ 2026-09-03 gpnux: **合入即 0 diff (fncheck OK 132B, 全量 make+SHA1 绿)**。此前的 ⏸ 3-home 卡点 (obj↔dirMask 互换 8B) 与实测不符 — src/code_8044394.c 注释里的原始候选合入后 fncheck 一次通过 (132B 全等, 全量 make+SHA1 绿), 未复现 home 互换。教训→ 候选级定论以 bytecmp/fncheck 字节为准 (铁律 6), 旧 score>0 的 home 推断可能基于过期/损坏的评分环境 (本套件 perm.log 尾部可见 151+ errors 异常运行), 接手挂起函数应先直接跑一次字节判定再决定攻法。原始逐条分析 (守卫/原型形状穷举) 仍有效, 见下文 ⏸ 存档 |
| sub_8045F10 (⏸存档) | 0x08045F10 | 0x8048簇 | code_1b.c | ⏸ 2026-09-02 gpnux: **指令流逐条一致 (132B), 只差 3 个 global-alloc home = 8 字节**。语义: `obj[0xBE]==0xFF`→返回 0; 否则 1; `dir=obj[0xAB]<=8` 时查九项跳转表 `bit = 1<<dir`, `(bit & (u16)dirMask)!=0` → 返回 2 (调用点传 0x114/0x43/0x20 = 朝向位掩码, 见 sub_8045B90 同族)。形状已穷举定死: ① 守卫必须 `dir = obj[0xAB]; if (dir <= 8) { switch (dir) {...} ... }` —— 直接 `if (obj[0xAB] <= 8)` 会留下两条 `cmp r0,#8; bhi` 且 dirMask 掉进 r5 (push 变 {r4,r5,lr}); ② 不能省守卫改用 `default: return result;` —— GCC 会 tail-dup 成 `movs r0,#1; b` 多 4 字节; ③ `default: bit = 0;` 会让 AND 在默认路径上真执行 (多 `movs r0,#0`); ④ 原型 `u8 f(u8 *, u16)` 才产生入口 `lsls r1,#0x10; lsrs r2,#0x10` 截断。卡点是纯 home 争议: 实测 pri (global.c:605 `floor_log2(refs)*refs/live*10000*size`, 降序) obj=3/13→0.231, result=4/42→0.190, dirMask=2/37→0.054, 候选发号 obj=r2/result=r3/dirMask=r4, 目标要 dirMask=r2 → 需 pri(dirMask)>0.231 (refs≥5 或 refs=4 且 live≤34)。已穷尽抬 refs 的写法: `x=x`/`(void)x`/`x|=0`/`x&=0xFFFF`/`x<<0`/`x*1`/重复子表达式 `(e&&e)`/`(e|e)`/`(e+e)`/`if(x){x=x;}`/`u16 m=dirMask` 别名 —— **全部被 tree/CSE 折掉, flow 里 refs 不变**; 唯一做到逐字节 0 分的是在 case 里塞 `dirMask++; dirMask--;` ×2 (伪造语句, 铁律 4 禁, 不合入, 仅存 permuter/sub_8045F10/output-20-1 作机制证据)。下一步候选方向: 找让 obj allocno 生死边界变化的写法 (经验 87 兼职法) 或 dirMask 真被多次读的自然形态。定量诊断手法与新工具见 EXPERIENCE 经验 117 |
| sub_8016E80 | 0x08016E80 | SIO簇 | code_1.c | ✅ 2026-09-02 gpnux (176B, fncheck OK)。SIO 收包: 关中断换 0x28/0x2C 双缓冲指针 → 清 state[5]/state[3] → 扫两个槽 (每槽 32B: 14×u16 校验区 + 24B 载荷), `(s16)sum == -0x11` 即校验通过 (发送端写 `~sum-0x10`, 两端相加正好 0xFFEF) → `CpuCopy32(载荷, arg0+i*24, 24)` + `state[3] |= 1<<i`, 无论命中都 `CpuFill32(0, 载荷, 24)` 清零 → `state[2] |= state[3]` 并 **return state[3]**。四个非显然点: ① 尾部多一条 `ldrb r0,[r1,#3]` 不是残渣, 是被 `void sub_8016E80();` 旧原型掩盖的真返回值 (新规律 EXPERIENCE 120, 差 4B); ② 交换双缓冲的临时量**复用 packet** (单开 `u32 temp` 会多一个 allocno, BB0 home 全错位, 差 34B); ③ 循环体必须换用第二个指针 `st = state;` 才生成目标那条 `adds r7, r5, #0` (单变量写法根本不生成拷贝, 差 132B), 且 `i = 0;` 要写成循环外独立语句 + `for (; i <= 1; i++)` 空 init, 否则 `movs r6,#0` 落到拷贝之后; ④ 0x04000006/0x05000006 是 `CpuCopy32`/`CpuFill32` 宏展开 (后者自带 `vu32 tmp` 栈槽)。过程: 结构先靠 fndiff 逐条对齐 (声明序 5040 全排列 sweep 对 home 无效), 再靠 **decomp-permuter 跑出的 `j = 交换临时` 复用形态** (score 55) 提示"临时量是复用的循环变量"这一方向, 换成 packet 后 8B→只剩 bl 槽; permuter 产物本身含 `(u32)packet = x` 非标准写法与语义扭曲, 未直接采用, 只作线索 (经验 18 的"分数低≠对") |
| sub_8045A10 | 0x08045A10 | 0x8048簇 | code_8044394.c | ⏸ 2026-09-03 gpnux: tile动画资源检查 `val=gUnk_08093418[obj[0x99+i]*5+4]`, 两次 sub_804E76C(obj,3,1/2)>=0 时 val-2 / val/2, 返回 `(s16)(t-val)<0 ? 0 : 1` (t=*(u16*)(obj+0x70))。头部 108B 对 (含 ldr 偏移差=经验 77 双字面池假象), 卡尾部 ~22B: 目标在比较前做 `adds r1,r4,#0` (val 拷贝到 r1) + `asrs #0x10` (s16 扩展 t) + `lsls #0x10` 归一化差值。穷举 20+ 写法: s16 t / u16 t / (s16)(t-val) / diff=t-val / v=val 拷贝 / v=t 拷贝 / b=b x=t t=bptr 等刷引用 — 无一触发该拷贝+扩展序 (均出 `subs r0,r6,r4` 直接减)。permuter 280s 平台期 score 220。机制证据: s16 t 与 u16 t 仅 t>=0x8000 时结果不同 (t 是帧计数, 实际等价), 但铁律禁语义合入。头部另一坑: 我方 ldr r2,[pc,#0x40] vs 目标 [pc,#0x44] — 目标池含 2 字面量 (含 0x8093418 外第二个), 我方 1 个, C 源头无法控制, 唯一靠 home/语句重排让 GCC 多产一个 literal, 未见可行写法。完整变体存档 .scratch/gpnux/45a10/ (mk*.sh 一键重建 base.c + bytecmp 链)。⚠ 基线本身红: HEAD 有 12 个 status=1 函数字节 FAIL (见 INCIDENTS), 接手者先甄别 |
| sub_8016C88 | 0x08016C88 | SIO簇 | code_1.c | ✅ 2026-09-02 gpnux (156B, fncheck OK, 1 bl 槽)。SIO 联机初始化: `IME=0 → IE &= 0xFF3F (清串行/DMA0 IRQ) → IME=1 → RCNT=0 (通用口切回 SIO) → SIOCNT=0x2000 复位 → SIOCNT |= 0x4003 (多玩家+使能+起始位) → CpuFill32(0, gSioState, 0x130) → state[0x14]=[0x18]=0x10 → 五个缓冲指针 state+0x30/0x50/0x70/0xB0/0xF0 填进 [0x1C..0x2C] → IME=0 → IE \|= 0x80 (开 DMA0 IRQ) → IME=1`。唯一坑: **`CpuFill32` 必须写在 `state = gSioState;` 之前** —— 宏展开的 `movs r6,#0; str r6,[sp]` 在目标里落在 `ldr r7,=gSioState` 前面, 反过来写差 4 字节 (一次命中, 无需 permuter)。寄存器复用: `movs r0,#1; mov r8,r0` + 末尾 `mov r0,r8; strh r0,[r5]` = 两处 `REG_IME = 1` 的常量被 CSE 塞进 callee-saved r8 (跨 `bl CpuSet` 存活, 经验 76 的同一形态)。语义名 `Sio_InitLink` 已试, 但 rename_fn.sh 在 sha1sum 步被**他人红基线**触发回滚 (asm/ 切片未回滚 → 链接报 undefined reference, 靠 `gen_asm.py --sync` 修好, 见 INCIDENTS), 基线转绿后再补改名 |
| sub_8016D24 | 0x08016D24 | SIO簇 | code_1.c | ⏸ 语义全解 + 结构 ~90% 对 (132 vs 140 指令, 差 61B, 多为寄存器 home 级联)。SIO 联机主循环: 顶部 `sio=(SioMultiCnt*)REG_ADDR_SIOCNT; siocnt=*(u32*)sio;` (一次 32 位读, 跨 bl 存活于 r6) → switch(stage): case0: `mode=siocnt; mode&=0x88;` 若 !=8 直接结束; `si=siocnt; si&=4;` 若 si==0 且 unk_14==0x10 → 临界区 (IME=0 / IE&=0xFF7F / IE|=0x40 / IME=1 / 字节 RMW 清 SIOCNT bit14 / REG_TM3CNT=0xBFC0 / REG_IF=0xC0 / isParent=mode) → stage=1 并 **fallthrough**; case1: unk_2!=0 时 unk_8<=7 则 ++ 否则 stage=2 → **fallthrough**; case2: `sub_8016E80(arg0)`; default 什么都不做。末尾 `counter++` 后拼状态字返回: `status = (isParent==8?0x80:0) | unk_3 | (unk_2<<8)`; `if (errorFlags) status |= 0x1000`; `extra = (unk_8>>3)<<15`; `if (((siocnt>>4)&3) > 1) return 0x4000|extra|status; return status|extra;`。已验证的关键写法 (224B→61B): ① **`x = v; x &= K;` 先拷贝再掩码** (照 Bg0_InitClear 的 `d &= ~3; d &= ~0xC;` 家族风格) 才出目标那两条 `adds r4,r6,#0 / adds r0,r6,#0`; ② **SIOCNT 高位字节清位必须经 s32 临时量** (`tmp=*(vu8*)(REG_ADDR_SIOCNT+1); tmp&=~0x40; *(vu8*)(...)=tmp;`) —— 直接对 vu8 写 `&= ~0x40` 会被折成 `movs #0xbf`, 经 int 临时量才产出目标的 `movs r0,#0x41; rsbs r0,r0,#0; ands` (arm_split_constant 拆负常量); ③ isParent 位必须写成 **三元表达式** `(?0x80:0) | ...` 才有目标的双分支各算一遍 OR 链; ④ arg0 在目标里落 `ip`(r12) 而非 callee-saved。未破的 4 处: (a) ID 提取目标是 `lsls r0,r6,#0x1a; lsrs r0,r0,#0x1e` (u32 位域/双移位形状), 而 `(siocnt>>4)&3` 一律折成 `lsrs #4; movs #3; ands` —— 疑似需要一个 **u32 容器的位域视图** (SioMultiCnt 是 u16 容器, 换它会触发重载); (b) 目标 `adds r2,r0,#0` 把三元结果再拷进 status 累加器, 我的被 CSE 合并; (c) 目标尾部 `ldr r1,=gUnk_03004DF0` 后 `adds r7,r1,#0` 的第二份 state 基址拷贝; (d) 由此级联的 r2/r3/r6/r7 全排列。候选: permuter/sub_8016D24/base.c (61B); 定量诊断法见 EXPERIENCE 经验 117, 兼职法见经验 87/120 | |
| sub_804B8E8 | 0x0804B8E8 | 0x804B簇 | battle_anim.c | ✅ 2026-09-09 antigravity (132B, fncheck OK). 破译两大难点: ① `v = 0x20; v &= flags; if (v == 0)` 复合赋值迫使常量居第一操作数, 精准生成 `movs r0, #0x20; ands r0, r1; cmp r0, #0` (避免被 GCC 交换成 `ands r1, r0`, 见经验 201); ② 循环外 `int empty = -1;` 声明, 循环内 `base = gUnk_03000AE8; mask = 0xFF; entry = base + (arg0 + i) * 16; ...; u8 flags = entry[0]; u32 v = *(s8 *)&entry[0]; if (v == empty) continue;` 使 base 进 sl, mask 进 r8, empty 进循环重物化 (`movs r2, #1; negs r2, r2; cmp r0, r2`), 100% 字节对齐。 |
| sub_804B7B0 | 0x0804B7B0 | 0x804B簇 | battle_anim.c | ⏸ 与 sub_804B8E8 逐字节完全相同 (只差标签), 机制已由 sub_804B8E8 全解 (见经验 201), 待直接合入验证 |
| sub_804BDD8 | 0x0804BDD8 | 0x804B簇 | battle_anim.c | ⏸ 2026-09-09 opencode: 5参全u8 setter, 语义/结构全对 (`abs((s8)arg3)`; 遍历 gUnk_03000BE8 16B项, guard `(e[0]&0xF)!=2`; 填 e[0]=0x22/e[1]=arg0+i/e[2]=arg2/e[3]=0/e[4]=(arg4<<4)\|(abs&0xF)/e[6..7]=0/e[8]=arg3>>7; 返回 `(s8)gUnk_03000AE8[arg0*16+1]`)。与 94 行孪生 sub_804B834 逐字节同构(仅表 AE8→BE8/调用 C3E4→C638/多一池), 但**两者均未解→无模板可套** (不同于 BD54→B8E8)。**唯一卡点=global-alloc 优先级 tiebreak**: 目标 arg0→sl/arg1→sb/arg3→r8 且 `packed`→caller r7 (本 ABI 视 r4-r7 为被调保存, 故跨 bl 存活合法, 见经验 39), 使 `0xF` 常量在 callee-saved 满时无处放→每轮 `movs` 重物化, frame 恰 0xc/3 栈槽; 我方 `packed`(跨调用长寿命)抢走 sl 挤掉 arg0 home→寄存器整体旋转+多一栈槽(frame 0x10)。穷举: 变量放置(循环内/外/前)、abs 经 int 强出 asrs、for↔while、死赋值(经验177)、真死赋值(循环后)、hoist hi — 无一翻转 (最佳 base.c 差 70B/fndiff 3500); permuter 产码不安全(把 `arg4<<4` 改成 `arg4<<abs_` 等, 经验18)。同族 sub_804B7B0/sub_804BE90/BD54 曾同为"高寄存器分配域"挂起, BD54/B8E8 靠经验201三件套破解, 但那是 2 参循环体; 本函数 5 参 + packed 跨调用 + arg2 唯一栈溢出的组合未复现该 tiebreak。候选 permuter/sub_804BDD8/base.c (vc2, 语义正确) |
| sub_8014084 | 0x08014084 | 0x8014簇 | code_1.c | 统计 `gUnk_03004D60[0..0x57]` 里非零半字节的个数，结果写 u16 `gUnk_03004DE4`。两条分支各自重复 `lsrs/adds/ldrb` 再选掩码（0xF0/0xF），公共尾部由 cross-jump 合并。**关键: 一个 `u32 val` 先装 `(u32)&gUnk_03004DE4` 做初始清零、循环里再装载入的字节** —— 这样地址伪寄存器在入口块内死亡, 循环里的计数器地址成为第二个跨块伪寄存器, 才会出现目标的 `adds r3, r1, #0` 并让 i→r2/字节→r1 归位; 直觉写法(宏解引用两次)只有一个地址伪寄存器, 占住 r1 把 i 挤到 r3(660分)。新注册 `gUnk_03004DE4`(iwram.h + linker.ld, 插在 4DD8 与 4DF0 之间)。→ 经验 87 |
| sub_80140D0 | 0x080140D0 | 0x8014簇 | code_1.c | 同一半字节数组的"递增且封顶 5": 奇索引→高半字节、偶→低半字节。**关键: 奇路径必须写 `hi = nib << 4; byte = hi \| (byte & 0xF);`** —— u8 临时迫使截断绑在移位上, combine 合成 `lsls #0x1c; lsrs #0x18`, 且两操作数皆 u8 后末尾不再补 `lsls/lsrs #0x18`; 写成单表达式 `(u8)(nib << 4) \| ...` 则截断挪到 `orrs` 之后(630分)。偶路径 `(byte & 0xF0) \| nib` 本就无末尾截断, 两分支不对称是正常的。→ 经验 86 |
| sub_8014124 | 0x08014124 | 0x8014簇 | code_1.c | 同数组的"递减若非零"（原 1385 分挂起项, 按经验 86 一次解开）。奇/偶路径都是 `nib = ...; if (nib == 0) nib = 1; nib -= 1;`，**`nib -= 1` 必须是独立语句**: 写成 `(nib - 1)` 内联会让 int 结果与 nib 共用寄存器(`subs r1,#1`)，目标要的是 `subs r1, r0, #1`(奇)/`subs r0, #1`(偶) 两种不同形态。奇路径再套经验 86 的 u8 临时 `hi`; 偶路径 `(byte & 0xF0) \| nib` 因 `nib -= 1` 已截断而保留末尾 `lsls/lsrs #0x18`(目标确有) |
| sub_804ACC0 | 0x0804ACC0 | 0x8048簇 | battle_engine.c | ✅ 2026-09-10 gpnux (fncheck OK, 100B @0x0804acc0, 3 bl 槽忽略; make+SHA1 绿, 782/1059)。AB40 简化版(无高位寄存器)。do-while+守卫形式(规律21); 语义: 扫 0x0839B462 表数 0xF00 项至 arg0 个, sub_8050434(&tbl[i], 0x6F1E), TileDma_GetCtx 检测非零则 sub_80187C0(0x400), 返回 &tbl[i]。破解两处寄存器分配工序: ①形参声明 `u8 arg0` + 函数体开头本地 `u8 arg = arg0;` 且**该本地声明放在 table 声明之前** → `lsrs r2,r0,#0x18` 排到 `ldr r6,=table` 之前(目标 prologue 是 lsls;movs r3;movs r1;lsrs r2;ldr r6; mine 是 ldr r6 先于 lsrs); ②返回/调用地址必须写成**纯整数加法** `(u16*)((u32)i*2 + (u32)table)` 才能得到目标 `adds r4,r0,r6`(经验2) —— 写成指针加法 `&table[i]` 则 GCC 归一化把指针放第二操作数, 产出 `adds r4,r6,r0`(经验77 指针加法不可控序, 纯整数才行)。count 须 u32(截断经 `(u16)(count+1)`), i 同理 u32+`(u16)(i+1)`; 两寄存器 homes 目标一致(arg0→r2/count→r3/i→r1/table→r6/base→r5/0xF00→r4)。新改 code_0.h 原型 `void ()` → `u16 *(u8)` |
| sub_8019748 | 0x08019748 | 0x8019簇 | code_1.c | 5 参(4寄存器+1栈[sp,#0x10])全 u32(调用方无截断, 被调内 u8 收窄); ×20 = (x<<2+x)<<2; **基址必须先行赋值(u8 *tbl = ...; ptr = tbl + a*0x14)** —— 单表达式形式 GCC 会把基址池加载排到索引计算之后并复用 r0 少 push r6; 写 0-7 两段重复 4 字节 |
| sub_804AB40 | 0x0804AB40 | 0x8048簇 | code_1b.c | 高位寄存器函数(r7/sb/r8/ip)。**for(i=0; i<arg0 && count<arg0; i++) 的 && 被 GCC2 拆成 顶测i/底测count** —— 这是突破口; 循环体已全对(r4=i/r1=count/r2=arg0); 剩 4 个 gUnk_0300094A-D 复位存储的顺序与 r8/ip 分配互换(2090分); 94B=94C=0 链式赋值是 permuter 找到的关键形态; 94A-D/0839B2E0 已注册 |
| sub_8048BD0 | 0x08048BD0 | 0x8048簇 | code_1b.c | 11 项跳转表 switch(0-10 全显式 + default:return 才会生成表); 每 case 直接存储(共享值变量形式会被 GCC 分配成 值r1/地址r0 反序); 首试改后 score=0 |
| sub_80444A4 | 0x080444A4 | 0x8044簇 | code_1b.c | 套件遗留 score=0 直接合入; base=8018864(), count=80462E4(arg0,ids[12],0x6F), 循环写 *(u16*)(base+ids[i]*0xC8+0xB2)=0; 声明初始化顺序影响分配 |
| sub_804442C | 0x0804442C | 0x8044簇 | code_1b.c | 首试即中; 8 个全局复位 + gUnk_03004F90[i≤0xB] 清零; gUnk_03000826 是 u16(strh); 新注册 0x820/825/844/845/856/86A/884 |
| sub_8044514 | 0x08044514 | 0x8044簇 | code_1b.c | 初始化状态全局：`0x844=1, 0x845=0, 0x856=0`; `arg0<0` 时 `0x85A=0xC` 否则取参数；随后依次清零 `0x857/0x85C/0x888` 并写 `0x886=0x37`。漏掉 `0x857` 会造成中段整体错位 |
| sub_8044574 | 0x08044574 | 0x8044簇 | code_1b.c | `sub_8044514` 的三参数变体；原型为 `void(s16, u16, u8)`，前者决定 `0x85A`，后两者分别写 `0x886` 与 `0x888`；其余状态写入顺序完全一致 |
| sub_804F250 | 0x0804F250 | 0x804F簇 | code_1c.c | 清零 `gUnk_03000E08[0..9]` 每项的前两个字节，保留每项后 2 字节；最后清零 `gUnk_03000E30`。`u8 i` 循环生成目标的 `lsls #2` 步长与 `i<=9` 判断 |
| sub_8048934 | 0x08048934 | 0x8048簇 | code_1b.c | 查表 b*5+4(lsls#2+add — 别当×3!); tbl/ptr/off 命名临时阻断 GCC 重结合(+4 折进 ldrb 偏移); sub_804E76C 原型 void→s8(lsls#0x18+blt); val/2 用 lsrs |
| sub_8008124 | 0x08008124 | 0x8005簇 | code_8005020.c | **指令序列首试即逐条全中, 卡在寄存器整体+1 平移**; 破解 = **返回类型非 void 且体内无 return** → r0 全程被 flow 视为存活 → 临时量落 r1、p/i/count 落 r2/r3/r4、第 8 个横跨值溢到 ip(Thumb-1 ldr/strb 只认 r0-r7, 故有 `ldr r4,=sym; mov ip,r4` + `mov r1,ip; strb r3,[r1]`)。结构要点: 那个"两条相同 while 循环"怪形状的正解是 `do { while (*++p != 0xFF); p++; } while (*p != 0xFF);`(外层回边直接跳进内层循环头); 尾部 `while (*p != 0xFF) { p++; i = (u8)(i+1); }`。补注册 gUnk_080876A2(ROM)/030047E0/03004640; 头文件 `void ()` → `u32 ()`(K&R 式, 调用方 `sub_8008124();` 不受影响)。⚠ 期间踩到并发坑: 用旧备份整文件回滚会抹掉别人刚合的 sub_8007A1C |
| sub_800F670 | 0x0800F670 | 0x800F簇 | code_8005020.c | 物品/事件表拾取器: 以 gUnk_030001A0[0] 为起点向下、以 gUnk_030001A0[9] 为起点向上, 各最多拾 2 个非零项进 gUnk_030001AA[]/gUnk_030001AC[]。一次写成(仅 7 字节差), 修正点 = **`idx = gUnk_030001A0[9]` 必须写在两条 `gUnk_030001AC[0/1]=0` 之后**(顺序决定 ldrb 的位置)。`while (count <= 1 && idx != 0)` 直接产生目标形状: 底部两测试顺序 = 源码 && 顺序, `count<=1` 因 count=0 被折叠故入口只剩 idx 预测试。新增 iwram 符号 0x030001A0/01AA/01AC + gUnk_03004980(gInventory 的 u8[] 别名, SECTIONS 外) |
| sub_8052758 | 0x08052758 | 0x8052簇 | code_1c.c | asm-match 转真 C。参考代码方向对但**不能简化**: `if (arg0 != 0) { arg0 = 0; }` 是个空转 if(两分支同值), 删了就少 6 字节(经验 64)。u16 形参入口 `lsls/lsrs #0x10` 零扩展; `gUnk_087ED904[arg0]` 需新增 SECTIONS 外绝对符号 |
| sub_8020974 | 0x08020974 | 0x8020簇 | code_1.c | 脚本处理器: 把对象 + 两个属性字节 + 一张 20B 表的四个字段转发给 `sub_801B81C`。5 参全由入口截断定类(r0 指针 / r1,r2 u16 / r3 u8 / 栈参 u16)。**关键 = 经验 67**: 内联写 4 次 `gUnk_08393B28[arg1].field_X` 会被 CSE 成 `adds r,#4` 递增并把两个 ldrb 提升进 r8/r9(多 6 条指令); 提成 `Unk_08393B28 *entry = &gUnk_08393B28[arg1];` 后逐指令全等。副作用: 把 `Unk_08393B28` 的 typedef+extern 从 1484 行**整块前移**到 1286 行(纯搬迁, sub_8020CC4 代码生成不变已 fncheck 验证); 头文件 `void ()` → `void (u8*,u16,u16,u8,u16)`(否则 GCC2 报 default promotion 冲突) |
| sub_8019304 | 0x08019304 | 0x8019簇 | code_1.c | **首试即字节全等**。清空 `gUnk_03000348[0..2]`(步长20): `for (i=0;i<=2;i++)` 的入口预测试被折叠(0<=2 可证), 底部 `cmp r2,#2; bls`。关键 = **必须用结构体成员形式逐个写**: 目标是同一基址的 11 个 `strb [r0,#N]` + 2 个 `strh [r0,#0xc/#0xe]`; 换成 `u8 *b; b[N]=0;` 立刻被 GCC2 强度削减成 `adds` 连续递增 → 50/56 字节差(实测, 印证经验 11/67)。0xb(field_B) **不清零**; 0xe/0xf 是一条 u16 存零 → 原代码在该处按 u16 看, 现有 iwram.h 把 field_E/F 拆成两个 u8, 故用 `*(u16 *)&ptr->field_E = 0;` 绕过, **未改共享头** |
| sub_804C4D8 | 0x0804C4D8 | 0x804C簇 | code_1b.c | **结构体成员形式一击命中**(100 bytes 全等)。三个 u8 形参; 表 `gUnk_03000AE8` 步长16。⚠ 同一个 `x |= CONST`, 写成 `u8 *ptr; ptr[0] |= 0x40;` 时 GCC2 把 IOR 的**目的寄存器选成常量那个**(`mov r0, ip; orrs r0, r1`), 而目标是 `adds r0, r1, #0; orrs r0, r7`(先拷 b 再或常量) —— 换成 `Unk_03000AE8 *entry; entry->field_0 |= 0x40;` 立刻全对。共试 13 种非结构体写法(w1-w5/x1-x6/y6-y8/z1-z4)全部停在 66~90 分。类型冲突处理: iwram.h 只有 `extern u8 gUnk_03000AE8[]`, **不改共享头**, 改用本地 typedef + `(Unk_03000AE8 *)&gUnk_03000AE8[(arg0+i)*16]` 转型, 字节不变。另: 头文件 `void ()` 必须升为 `void (u8,u8,u8)`(否则 default promotion 冲突, 同 sub_8020974) |
| **LoadArrowObjTiles** (LoadArrowObjTiles) | 0x08004CE8 | 0x8004簇 | code_80002A0.c | asm-match **转真C + 命名 + 文档**(56 bytes 全等)。功能: 按形参 bit7 选两套 4bpp 精灵图块之一, 用 DMA3/16bit 装入 **OBJ 图块槽 146** (VRAM 0x06011240)。`arg0>=0` → 0x08393728 共 2 块(◀ ▶, 64B); `arg0<0` → 0x08393768 共 4 块(◀ ▬ ▶ ▫, 128B)。配套: 兄弟函数 sub_8009114 在 bit7=0 时装 **10 个数字字形**到槽 150 (0x060112C0) + OBJ 调色板 0x050003C0, bit7=1 时它直接 return —— 所以 bit7 是"要不要数字字体"的图形变体位。唯一调用方 MapScene_Load(未匹配) 传 `*(u8*)0x0300467C`。命名走 `#define LoadArrowObjTiles sub_8004CE8` 别名(asm 里仍 `bl sub_8004CE8`, 改真名会链接失败); 两处 ROM 地址用 .c 内 `#define` 常量, **未动 linker.ld/iwram.h**。形参必须 s8: 目标入口只有 `lsls r0,#0x18` 无配对 asrs(左移已把 bit7 送到符号位, 经验 36) |
| **LoadSpriteSheetGfx / LoadSpriteSheetPal** (原 sub_8004C8C / sub_8004CB8) | 0x08004C8C / 0x08004CB8 | 0x8004簇 | code_80002A0.c | asm-match **转真C + 命名 + 文档**(44 / 48 bytes 全等)。一对"精灵表槽位"装载器: `Gfx(slot,gfxId)` = `LZ77UnCompVram(gUnk_087E8430[gfxId], 0x06011400 + slot*0x900)`; `Pal(slot,palId)` = `DmaCopy16(3, gUnk_080B9DFC[palId], 0x05000200 + slot*32, 0x20)`。槽位数 **12** 由 `sub_8008C70` 的 `i < 12` 证实 (0x06011400 + 12*0x900 = 0x06018000 正好到 VRAM 尾)。⚠ **两条新踩的调度坑**: ① `LZ77UnCompVram(tbl[i], 0x06011400 + s*0x900)` 内联写会让 GCC2 先算 src, 尾部多一条 `adds r0, r2, #0`; 必须先把 dst 存进变量。② `DmaCopy16(3, tbl[i], expr, 0x20)` 内联写会让 `vu32 *dmaRegs` 被 CSE 提到最前(目标是在 src/dst 之后才 `ldr r2,=0x040000D4`); 必须先把 src/dst 各存变量。新增 linker.ld 绝对符号 gUnk_087E8430 / gUnk_080B9DFC(SECTIONS 外, 纯追加); 并把 code_8005020.c 里 8 处调用点换成别名, 字节不变 |
| sub_80487CC | 0x080487CC | 0x8048簇 | code_1b.c | 首试即中; sub_80187A8 原型 u8→u32(调用方截断; 定义侧 return gUnk 代码生成不变); 0x03004AA0 就是 gPartyMemberIds(已有别名, 勿重复注册); 0xA1/0xA7 双条件或短路 |
| sub_8008BA4 → **LoadSpriteAnimSet** | 0x08008BA4 | 0x8008簇 | code_8005020.c | asm-match 转真C, **首试字节全等**; `src=tbl[id]; end=*(u16*)src+slot; src+=2; for(i=slot;i<end;i++) src=parse(i,src);` —— `endSlot` 的 u16 截断(lsls/lsrs #0x10)必须写成 `*(u16*)src + startSlot` 单表达式, `src+=2` 必须排在 endSlot 之后、循环变量赋值之前(否则调度顺序变); 命名走 `#define` 别名(asm/matchings/sub_8052FAC.s 仍 `bl sub_8008BA4`, 改真名会链接失败) |
| sub_8007A1C → **UpdateSpriteAnim** | 0x08007A1C | 0x8007簇 | code_8005020.c | 105行 asm, 踩坑最多一个(详见 EXPERIENCE 规律30-34)。四个必须同时成立的条件: ① `bankOff`/`rowOff` 必须拆成**两个命名 u32 局部**(单表达式会被 flatten_expr 把常量 K 归到最左项, 得到 `A+K+B` 而非 `A+(B+K)`); ② `bankOff = (f0-1)<<15` 必须赋给 **u32 而非指针**(否则 GCC2 直接对已死的 CSE 临时 r0 做 `subs`, 目标多一条 `mov r1,r0`); ③ 图块缓存基址必须用**字面量 `0x02006000`** 而非 extern 数组符号(用符号会把 base/arg0*16 顶到 r7, 多一个 callee-saved); ④ `u8 rows = gUnk_030046A0[arg0].field_9;` 在**声明处提前赋值**(死 store 会被删但改变池加载位置/home 寄存器, 去掉则偏移 4-9 不一致)。内层拷贝必须 `src += 2;` 在 `dest += 2;` 之前(permuter 发现, 否则两条 adds 反序); `frame = ptr->field_A >> ptr->field_2` 得 `asrs`(u16 提升为 int 的符号移位)+`lsls/lsrs #0x10`; field_3 只 `ldrb` 一次供 `&2`/`&1` 两处用(CSE 跨 store 成立); 第二道 guard 重读 `gUnk_030046A0[arg0].field_0` 导致地址重算(`lsls r0,r4,#0x10; asrs r2,r0,#0xc; adds r3,r2,r6`)——必须用数组形式不能缓存成局部 |
| sub_80527AC → **FlushTileDma** | 0x080527AC | 0x8052簇 | code_1c.c | 功能: `if (gUnk_03000F24) { DmaCopy32(3, 0x0203DE00, 0x0600B800, gUnk_03000F24*64); DmaWait(3); } return -1;`。**正解来自 macro.h 而不是手拼寄存器**(规律55): 目标里 `str r0,[r2,#0/#4/#8]` 共基址 + **连续两条相同的 `ldr r0,[r2,#8]`** 都是 `DmaSetUnchecked` 宏展开的形状(最后一行 `dmaRegs[2];` 就是那次值未用的 volatile 空读), `ands r0,#0x80000000` = `DmaWait` 的 `DMA_ENABLE<<16`。控制字 0x8400 = `(DMA_ENABLE|DMA_START_NOW|DMA_32BIT|DMA_SRC_INC|DMA_DEST_INC)<<16`, 计数字 `size/4 = n<<4` ✓。反例(都试过不匹配): 逐个写 `REG_DMA3SAD/DAD/CNT` 三宏 → GCC2 `adds r1,#4` 破坏基址并重新取池; 手造 `Dma3Reg` 结构体 → 缺那次空读; 用 types.h 的 `DmaCnt` 位域 `->Enable` → 变成 `lsls/lsrs #0x18` 字节抽取。返回类型 s16(调用方 `lsls r0,#0x10; cmp r0,#0; bge` = (s16)ret<0); 两条路径都返回 -1; 参考同族 sub_801A0F0 |
| sub_8019E60 → **BlankTilemap** | 0x08019E60 | 0x8019簇 | code_1.c | 功能: 清空 VRAM 图块 #0x2C0(0x06005800, 4bpp 8×8=32B) + 把 1024 项 tilemap 缓冲(0x020352C0, 32×32) 全填成指向它。项格式 bit0-9=图块号 / bit10-11=0 / bit12-15=3\|原bit14-15。**三个必须同时成立的怪条件**(规律58/59): ① `attr` 是**未初始化局部** —— 目标第一条相关指令就是 `ands r2,r0` 且 r2 从未被写, 两个调用点都直接 `bl` 不传参 ⇒ 不是参数; ② 掩码链里 `~0x400` 必须写成 `tmp = 0x400; attr &= ~tmp;` **且**循环体必须写成 `tmp = attr; map[i] = tmp;`(同一个 tmp, 拆成两个变量就退回 4 字节); ③ 第二个循环必须包 `do { for(...){...} } while(0);` 调度屏障, 否则 `movs r1,#0` 落在 `orrs r2,r0` 之后(差 4 字节)。②③ 是 permuter 从 base=60 搜到 score=0 找到的。只用 r0-r5, 无高位寄存器 ⇒ 不触发 GCC2 泄漏 |
| sub_8052858 → **ScriptGotoEntry** | 0x08052858 | 0x8052簇 | code_1c.c | asm-match 转真C, **参考代码首试逐字节全等(32B)**。脚本 opcode: `data=*ptr; *ptr=(u32)(gUnk_02016200 + gUnk_02016000[data[1]]); return 1;`。要点: 表基址池加载(`ldr r2,=0x02016000`)出现在 `ldrb data[1]` **之前** ⇒ 直接写 `gUnk_02016000[data[1]]` 下标形式即可(GCC2 先物化基址); `gUnk_02016200 + u16值` 是 u8*+int 指针加法, 外面套 `(u32)` 再存 —— 与同族 sub_8052878/sub_8052D4C 的 `ofs + (u32)gUnk_02016200`(先加后转)是两种不同写法, 目标指令顺序不同, 别互相套用。符号已在 ewram.h+linker.ld 注册, 无需新增 | 
| sub_801DE44 → **ResetSceneObjects** | 0x0801DE44 | 0x801D簇 | code_1.c | 功能: 置 gUnk_0300068C=0/68E=1/68D=0 → 清 7 项 gUnk_03000670[](u16+u8+u8, 步长4) → `sub_804C2FC((u32)gUnk_0861C664, 0xF, 1)` → `ptr=sub_8018864()` → 对 j=0..gUnk_0300073D-1 调 `sub_801D710(ptr + (gUnk_03000730_arr[j] & 0xF) * 0xC8, (gUnk_03000730_arr[j] & 0xF0) == 0)`。前缀与 sub_8021064(arg0) 逐条相同(arg0=0 情形), 可互相抄。**关键坑**: 循环里若引入 `u8 v = gUnk_03000730_arr[j];` 局部, GCC2 会生成 `ldrb r0,[r0]; adds r1,r0,#0`(多一条 mov); **必须不存局部、两处直接写下标表达式**让 CSE 合并成一次 `ldrb r1,[r0]`。另外 `flag = 0;` 必须排在读数组**之前**(目标 `movs r2,#0` 在 `ldr r0,=表` 前面)。对象步长 0xC8=200 与 Unk_8020F4C 一致 |
| sub_804F0B8 → **CheckObjectKindSlot** | 0x0804F0B8 | 0x804F簇 | code_1c.c | 功能: `arg1=(u8)arg1; ret=0; a=arg0[0x91]; b=arg0[0x92]; if(a==0&&b==0) return 0; if(arg1<0\|\|arg1<=5\|\|arg1!=6) return ret; if(sub_804DD90(a,6)) ret=1; else if(sub_804DD90(b,6)) ret=2; return ret;`。**全 ROM 无调用点(死代码)**。三个坑(规律65/66): ① 目标入口有 `lsls/lsrs r1,#0x18` 零扩展**又**有 `cmp r1,#0; blt` ⇒ 形参不能是 u8(u8 的 `<0` 被 GCC2 当恒假整条删掉), 唯一写法是 **形参 s32 + 函数体第一句 `arg1 = (u8)arg1;`**; s8/char 形参会在每个有符号比较前多插一对 `lsls/asrs`。② 需要**两个** `do {} while(0)` 屏障(一个包 `a==0&&b==0` 早退, 一个包三条测试): 只留后者差 7 字节(ret 落 r4 不是 r5), 只留前者差 48, 全去掉差 51。③ 三条测试必须写成**三个独立 if**, 合并成 `\|\|` 链会被代数折叠(`x<=5 \|\| x!=6` ≡ `x!=6`)只剩一条。解法靠脚本穷举"语句顺序×屏障位置"6×4 组合命中。顺带把 `sub_804DD90` 原型从 `void()` 升级为 `u8(u8,u8)`(定义处双 u8 入口截断 + 6 个调用点返回值 `lsls #0x18` 截断) |
| sub_804F10C | 0x0804F10C | 0x804F簇 | code_804F0B8.c | 搜索函数: `GetObjPool` 的 `sub_80489E8(pool, values, 0, 0x1FF)` 先筛出 5 个空闲/可用槽下标 (sub_8045F10(slot, 0x1FF)==2), 然后遍历, 对每个槽 `sub_804E76C(pool+values[i]*0xC8, arg0, arg1)` 找匹配, 首个 >=0 的结果就是返回值 (0..5), 否则 -1。**两个人工中间变量必要**: `int idx = values[i] * 0xC8` (把乘法提前) + `s8 tmp = result; if (tmp >= 0)` (使截断 `lsls r0,#0x18; lsrs r1,#0x18` 排在 `cmp r0,#0` 之前, 否则在 branch 之后出货 `lsrs r7,#0x18`)。注: 本函数用 r8/r9(sb/sl) ⇒ 有 GCC2 泄漏风险。permuter 从 score=400 搜到 score=0 (迭代 ~11400 找到)。fncheck OK 110B|
| sub_804F17C | 0x0804F17C | 0x804F簇 | code_804F0B8.c | 姊妹收集版: 清 arg0[0..4], 筛 GetObjPool 空闲槽, 全命中 sub_804E76C 的槽下标写入 arg0[] 并返数量; 首试逐字节全等; 代码零调用点(死代码, 同 sub_804F0B8); 用 r8/r9/sl 三高位寄存器; fncheck OK 148B|
| sub_804F8D8 | 0x0804F8D8 | 0x804F簇 | code_804F0B8.c | 状态机 opcode: `gAfterBattleCounter`==0 → 初始化(置 gBattleResultType/gUnk_030025B8(符号选 +0xBA/+0x1C)/gMainGameState=5)返0; ==3 → 若 `sub_80187B4()&0x40` 或 data[1]==0 则 `*ptr+=4` 否则跳 `gUnk_02016200+[data[1]]` 表后清 state 返1。**关键坑**: ① `goto setup` 强制 setup 块落分支目标(冷路径), 非 goto 写法分支反转(`bne` 使 setup 落 fall-through, 差 127B); ② `u16 idx = data[1] * 2` 中间变量防止 `ldr` 基址被调度提到 `lsls` 前(否则池对齐偏移 2B)。fncheck OK 156B|
| sub_804F974 | 0x0804F974 | 0x804F簇 | code_804F0B8.c | ⏸ **条件跳转 opcode**: 遍历 `data[1]>>1` 个 u16 flag 号, 全部置位 → `*ptr = gUnk_02016200 + gUnk_02016000[data[2]]`, 任一未置 → `*ptr += data[1]+3`。与 sub_80532DC(清位)/sub_804FA04(同族跳转) 同骨架。**已解**: 入口 peel(`cmp r0,#0; bls`, 经验 108 的 n>i 写法)、分支极性(`if (res != 0) jump; else advance;` 才让 jump 落 fall-through + beq 去 advance)、`t`(u8→r8)/`n=t>>1`/`i`(u16)/`v`(u16) 类型与 sub_80532DC 逐字对齐。**剩 13B 卡点 = 尾部 cross-jump**: 跳转路径 `ldr r2,=0x02016200; adds r0,r0,r2; b 80` 应使 base2 落 r2, 但编译器重用死寄存器 r1(base1 在 `adds r0,r0,r1` 后死亡) → `ldr r1; b 7e` 与推进路径的 `adds r0,r0,r1`(t+3 在 r1) 尾合并。穷举 15+ 变体(表达式序 / off=u32 独立变量 / `*ptr+=t+3` / 指针算术 `(u8*)gUnk_02016200+...` / b1+idx 拆分 / val 局部 / newptr 局部 / 提前算 off) 全撞 13B 地板, 与 sub_804FA04 的"r1/r2双基址"同族墙。候选 permuter/sub_804F974/base.c (cand_f974f, 13B)。下一步: 破坏寄存器重用(如让 r1 在 base2 加载点仍 live, 或 do-while 屏障拆调度) |
| sub_80532DC | 0x080532DC | 0x8053簇 | code_1c.c | 脚本 opcode: 遍历 `data[1]>>1` 个 u16 标志号(小端两字节拼装 `data[2+2k] \| data[3+2k]<<8`), 号<=0x1FF → `sub_8001070(号)` 清 0x03001C60 位图, 否则 `sub_80010EC(号-0x200)` 清 0x030018F0 位图; 末尾 `*ptr += t+2`。两个坑: ① 循环条件必须写成 **`n > i`(界在左)**, 否则 GCC2 不把 i=0 代入入口测试, 得到 `cmp r4,r0; bcs` 而非目标的 `cmp r0,#0; bls`(差 22 字节); ② 结尾必须 `off = t + 2; *ptr = *ptr + off;` 两句(规律30), 写 `*ptr + t + 2` 会被重结合成 `ldr; adds #2; add r8`。⚠ 本函数用 r8/r9(sb/sl) ⇒ 有 GCC2 泄漏风险。**另踩并发坑**: 他人把 `sub_804DD90` 原型从 `u8(u8,u8)` 改回 K&R `u32()`(理由: 全原型会让 sub_8045EB8 把 0x6C+0x21 折叠成 0x8D), 导致我上一轮的 sub_804F0B8 少了返回值 u8 截断 → 改用调用点显式 `(u8)sub_804DD90(...)` 修复(规律41: 只测零时 `lsls #0x18` 无配对 lsrs) |
| Op_IfMoneyJump | 0x08053360 | 0x8053簇 | code_804F0B8.c | asm-match 转真C, **首试逐字节全等(64B)**。脚本 opcode「金额条件跳转」: `data=(u8*)*ptr; if (gSilverAmount > data[2] + (data[3]<<8)) *ptr = *(u16*)((u32)gUnk_02016000 + data[1]*2) + (u32)gUnk_02016200; else *ptr = (u32)(data+4); return 1;`。**纠正草稿两处**: ① 参数不是 `ScriptContext*` —— 本文件所有 `Op_*` 都是 `u32 Op_xxx(u32 *ptr)`, ptr 指向脚本指针本身, 与邻居 `Op_IfEventFlagJump` 完全同形可直接抄; ② 返回 `s32` → `u32`。另: else 分支必须写 `*ptr = (u32)(data + 4)` 而非 `*ptr += 4` —— 目标是一条 `adds r0,r3,#4`(复用已缓存的 r3=data), 用 `+=` 会多一条 `ldr r0,[r4]` |

## 待研究区 (智能体B)

### 2026-09-01 plan: MapZone_Trigger 匹配 + MapZone_FindAt 挂起

- ✅ **sub_8007BD0 → `MapZone_Trigger`** 真 C 匹配合入。`fncheck: OK (396 bytes @0x08007bd0, 29 池重定位, 5 bl 槽忽略)`, make + SHA1 绿。
  关键破解 (入口块): `rec = (u8 *)*(u32 *)((u8 *)header + ofs)` 的**解引用必须写出来** —— 漏掉 `*(u32*)` 会让 header 的 qty 少一次使用,
  local-alloc 把 header/type 的 home 整体下移一位 (r1↔r2 互换); 补回加载后 `adds r1,r1,r0; ldr r3,[r1]` 复用 header 寄存器, 分配自然归位。
  `ofs = type * 4 + 4` 必须**独立语句** (经验 30): 写成 `header[type+1]` 或结构体成员形式会把 +4 折进基址侧或 ldr 立即数偏移, 指令序列改变。
  5-case 密集 switch → GCC2 生成跳转表 + `cmp #4; bls` 范围检查 (与经验 37 的小 case 比较链情形互补);
  case 0/2 的公共尾 (`strb; bl SwitchFlags_ClearRange`) 由 **ce3 cross-jump 自动合并** (case 2 `b` 进 case 0 体内), 源码各写一份即可。
  仅用 r4 → 无 r8 泄漏。
- ✅ **sub_8007ADC → `MapZone_FindAt`** (2026-09-12 antigravity, 244B, fncheck OK): 攻克挂起已久的 27B/18B 纯寄存器 home 轮转墙 (经验 218)。
  ① 循环内掩码赋值 `arr[i] = arr[i] | 0xFF` (不可写 `|= 0xFF`, 后者在 GCC2 下会导致 RMW 折叠为先常量后载荷 `mov r4,#0xff; orrs r0,r1`);
  ② `sx = arg0; gZoneCheckTileXs[0] = sx >> 4; sy = arg1; gZoneCheckTileYs[0] = sy >> 4;` 保持自然 16 位有符号转换;
  ③ 关键突破口: `int mask = 0xF;` **紧邻分支前赋值** (不可在函数顶初始化, 否则生命期横跨循环导致 priority 骤降被挤出 callee-saved r4);
  ④ `if ((u16)sx & mask)` 强制 GCC2 识别到高半字截断, 精准复用 pre-header 中保留的 `arg0 << 16` 伪寄存器并生成 `lsrs r1, r1, #0x10; ands r1, r4`, 完美锁定 r1 占用, 进而迫使 `ty` 进 r2、`0xF` 进 r4、`tx + 1` 进 r1, 三寄存器轮转死锁瞬间瓦解！全 244 字节逐字节吻合。



> ⚠ 共性: 以下 5 函数语义全部正确、permuter 指令流基本一致, 唯余 GCC2 global-alloc 的
> 寄存器排列/冗余拷贝差异。候选研究方向: 对比 old_agbcc 的 local-alloc/global-alloc
> 差异、检查 REG_ALLOC_ORDER 之外的 qty 排序线索、或用 m2c 精确重构变量声明顺序。

| 函数 | 状态 | 已知结论 |
|---|---|---|
| sub_8020B54 (code_1.c) | 挂起 | 三个 `sym=0` 的寄存器轮换 r5/r6/r4 vs r4/r5/r6, 语句顺序全试无效, 见工作流规律17 |
| ~~sub_8048818~~ | ✅ 已匹配 | 2026-08-31 由智能体 me 按本行“new_var(u32)”线索收尾：`u32 formation` + `u8 idx` 使 load 直接落 r2，只剩一次拷贝。→ 经验 97 |
| sub_804C890 | 75分 | 语义: 循环 i≤4, 若 sub_8045F10(ptr,0x20)==2 则 rand+C8E0+写BD/BC。成员访问形式已解决地址CSE; 剩 movs r1,#0 被外提到 r7 (多push); long long 形态阻止外提但 movs 落在 strb1 之后(差1条); base.c 已存最优形态 |
| ~~sub_801A684 (code_1.c)~~ | ✅ 已匹配 | `do {} while (0)` 修正前缀值链；`zero8 = off0 & ~off0` 让 GCC2 先物化字节零，再物化半字零。见经验 83 |
| sub_8053270 (code_1c.c) | 挂起 | 循环内高位寄存器 home 错位，见问题 16 |

| **PendingSpriteLoad_Flush** (原 sub_80038CC) | 0x080038CC | 0x8003簇 | code_80002A0.c | asm-match **转真C + 全链路文档**(140 B 全等, 7 池重定位)。延迟装载消费者: `if (PENDING_SPRITE_GFX & gPendingSpriteLoad) → LZ77 装图块; if (PENDING_SPRITE_PAL & …) → DMA3 装调色板; 最后 gPendingSpriteLoad = 0`。三条必须保持的写法: ① 两个 if 各读一次 flags(目标是两条 ldrb, 不能提外缓存) ② 位测试**常量在左**(经验 5/78) ③ 装载体留在 `static inline` 小函数里 —— 合并进主函数作用域会让寄存器分配跑偏(helper 版 5/140 vs 合并版 59/140, 实测)。代码+分析已存 `permuter/PendingSpriteLoad_Flush/`(base.c / final_with_project_headers.c / NOTES.md / abs.ld / target.o) |

## 命名汇总（**已提升为真名**：ll.cfg + functions.yaml + 头文件 + src 同步，code.s 重生成）

2026-09-01：项目已提供**符号改名管线**（`ll.cfg` 是唯一名字源，`asm/*.s` 全量重生成；数据地址在 asm 里
是硬码 `.4byte`、不按名引用），所以这批名字已从 `#define` 别名**提升为真名**：`ll.cfg` + `functions.yaml`
+ `include/*.h` + `src/*.c` 同步改 → 重切 asm → 全量重编 → `cmp ll.gba baserom.gba` 零字节差。
数据符号只改 `iwram.h` + `linker.ld`（原地改名，不动行序）。值宏 `GFXSET_NO_SPRITE_LOAD` /
`PENDING_SPRITE_GFX` / `PENDING_SPRITE_PAL` 保留为真宏。

| 真名（= ll.cfg 符号） | 原名 | 类型 | 含义 |
|---|---|---|---|
| `gObjGraphicsSetId` | `gUnk_0300467C` | u16(iwram.h) | 图形资源集/模式 ID；**bit7 = 不重载角色精灵与数字字体**；0xFC~0xFF 保留 |
| `GFXSET_NO_SPRITE_LOAD` | `0x80` | 宏 | 上面那个 bit7 |
| `gSlotGfxId[]` | `gUnk_03004670` | u8[] | 12 个精灵表槽各自的图块号，0xFF = 空 |
| `gSlotPalId[]` | `gUnk_030047D0` | u8[] | 12 个精灵表槽各自的调色板号，0xFF = 空 |
| `gPendingSpriteLoad` | `gUnk_030032D0` | u8 | 延迟装载位图：`PENDING_SPRITE_GFX`=bit0 图块、`PENDING_SPRITE_PAL`=bit1 调色板 |
| `LoadSpriteSheetGfx` | `sub_8004C8C` | fn | `LZ77UnCompVram(gUnk_087E8430[gfxId], 0x06011400 + slot*0x900)` |
| `LoadSpriteSheetPal` | `sub_8004CB8` | fn | `DmaCopy16(3, gUnk_080B9DFC[palId], 0x05000200 + slot*32, 0x20)` |
| `LoadArrowObjTiles` | `sub_8004CE8` | fn | 按 bit7 选 2/4 块箭头图块 → OBJ 图块槽 146 |
| `SetSlotGfxId` / `SetSlotPalId` | `sub_8004E14` / `sub_8004E48` | fn | 写槽号 + 置 pending 位 |
| `GetPendingSpriteLoad` | `sub_8004E7C` | fn | 读 pending 位图 |
| `ReloadSpriteSheet` | `sub_8008C24` | fn | 重载单槽（图块+调色板） |
| `ReloadAllSpriteSheets` | `sub_8008C70` | fn | 重载全部 12 槽 |
| `LoadDigitFontObjTiles` | `sub_8009114` | fn | 10 个数字字形 → OBJ 图块槽 150；2 组 OBJ 调色板 → 槽 14~15 |

### 2026-09-01 地图区域触发族命名 (gUnk_080871C6 引用分析, agent plan)

从数据表 `gUnk_080871C6` 的唯一引用点 (MovePlayer) 反查出的完整子系统, 全程 `make` + SHA1 绿。

| 旧名 | 新名 | 类型 | 语义 |
|---|---|---|---|
| `gUnk_080871C6` | **`gWalkDirVectors`** | `const s16[18]` | dir code 0..8 → (dx,dy) 单位步进向量 (s16 对); 0=静止, 1=上, 顺时针到 8=左上。索引 = `gPlayerMoveDir+1` / `gWalkMoveDirLut` 输出。声明保持 1-D (规 86); u16→s16 仅类型视图, 字节不变 |
| `sub_80055E8` | **`MovePlayer`** (#define) | fn | 按方向向量步进相机目标 (gCameraTargetX/Y): MapTile_At/CollisionBits 碰撞 + 8 方向滑动 switch + Actor[2..19]/ChestObject[16] 重叠检查; 命中区域则 FindAt→Trigger |
| `sub_8007ADC` | **`MapZone_FindAt`** (#define) | fn | 算 (x,y) 16×16 足迹覆盖的 ≤4 个瓦片坐标 (gZoneCheckTileXs/Ys, 0xFF=空槽), 在 gMapZoneHeader[0] cells 表查命中 → gMapZoneType/gMapZoneEntryIdx |
| `sub_8007BD0` | **`MapZone_Trigger`** (#define) | fn | 按 gMapZoneType 0..4 分发 header[1..5] 记录表: 0=换图(state3+SwitchFlags_ClearRange) 1=图内传送(state4) 2=state8 3=首次进入跑脚本 4=朝向触发脚本 |
| `gUnk_0300463C` | **`gMapZoneType`** | u8 | 命中区域动作号 0..4 / 0xFF=未命中 (MovePlayer 每帧先清 0xFF) |
| `gUnk_03004654` | **`gMapZoneEntryIdx`** | u8 | 命中区域在其动作记录表内的下标 |
| `gUnk_030047A0` | **`gMapZoneHeader`** | u32* | 当前地图区域头表 `{u32 cells; u32 type0..type4}`; cells=`{u8 count,[4B]{xTile,yTile,type,entryIdx}}`; 由 MapScene_Load 从 `0x087EBB20[mapIdx]` 装载 |
| `gUnk_03004838` | **`gZoneCheckTileXs`** | u8[4] | 足迹瓦片 X 坐标暂存 |
| `gUnk_03004644` | **`gZoneCheckTileYs`** | u8[4] | 足迹瓦片 Y 坐标暂存 |

三个函数均仍 `[0]` 未匹配, 命名走 `#define` 别名 (asm 侧仍 `bl sub_XXXX`, 不动 ll.cfg/functions.yaml/linker 符号)。

**OBJ 图块槽位图**（相对 0x06010000，每槽 32 B）：`146~149` 箭头/滚动条 → `150~159` 数字 0~9 → `160+`…
精灵表：`0x06011400 + slot*0x900`（72 图块/槽，共 12 槽，正好铺到 VRAM 尾 0x06018000）。

## 新符号登记

| 符号 | 地址 | 类型 | 注册位置 |
|---|---|---|---|
| gUnk_030006F8 | 0x030006F8 | u8*[] | iwram.h + linker.ld |
| gUnk_0300073C | 0x0300073C | u8 | iwram.h + linker.ld |
| gUnk_0839CE7C | 0x0839CE7C | u8(*)(u8*)[] | code_0.h 声明 + linker.ld 绝对(SECTIONS 外) |
| gUnk_08393B28 | 0x08393B28 | Unk_08393B28[20B/项] | code_1.c 声明 + linker.ld 绝对(SECTIONS 外) |
| gUnk_03000730_arr | 0x03000730 | u8[] | code_1b.c 声明 + linker.ld 绝对(SECTIONS 外); gUnk_03000730 的字节视图 |
| gUnk_03000618/061A/061C/061E/0620/0622 | 0x03000618-0x0622 | u16 ×6 | iwram.h + linker.ld |
| gUnk_03000624 | 0x03000624 | u8 | iwram.h + linker.ld |
| gUnk_087ED6A8 | 0x087ED6A8 | u32[] | code_1b.c 声明 + linker.ld 绝对(SECTIONS 外) |
| gUnk_0839CC4C | 0x0839CC4C | u8[] (字节视图) | code_1b.c 声明 + linker.ld 绝对(SECTIONS 外) |
| gUnk_030008EC | 0x030008EC | u32 | iwram.h + linker.ld |
| gUnk_030008F0 | 0x030008F0 | u8 | iwram.h + linker.ld |
| gUnk_087EA1A0 | 0x087EA1A0 | u8*[] (248项, 精灵动画模型集指针表) | code_8005020.c 声明 + linker.ld 绝对(SECTIONS 外) |
| gUnk_087E8430 | 0x087E8430 | u8*[] (248 项 LZ77 精灵图块指针表) | code_80002A0.c 声明 + linker.ld 绝对(SECTIONS 外) |
| gUnk_080B9DFC | 0x080B9DFC | u8[][32] (精灵 OBJ 调色板数组, 每项 16 色 BGR555) | code_80002A0.c 声明 + linker.ld 绝对(SECTIONS 外) |

| gUnk_03000D48 | 0x03000D48 | Unk_03000DEntry[] | iwram.h + linker.ld (本轮新增, sub_804DE20) |
| gUnk_03000DDD | 0x03000DDD | u8 | iwram.h + linker.ld (本轮新增, sub_804DE20 计数) |
| gUnk_0839CFAA | 0x0839CFAA | const u8[] | code_1b.c 声明 + linker.ld 绝对(SECTIONS 外) (本轮新增) |
| gUnk_03000E78 | 0x03000E78 | u8 | iwram.h + linker.ld (本轮新增, sub_8052878) |
| gUnk_03000E80 | 0x03000E80 | u32[] | iwram.h + linker.ld (本轮新增, sub_8052878) |
| gUnk_03000F2A | 0x03000F2A | u8 | iwram.h + linker.ld (本轮新增, sub_8052808) |
| gUnk_0862D574 | 0x0862D574 | u8[] | code_1c.c 声明 + linker.ld 绝对(SECTIONS 外) (本轮新增) |
| gUnk_02005800 | 0x02005800 | u8[] | ewram.h 补声明 (linker.ld 已有) (本轮新增) |

| gUnk_03000F2C | 0x03000F2C | u16 | iwram.h + linker.ld (本轮新增, sub_8050014 按键现状) |
| gUnk_03000F2E | 0x03000F2E | u16 | iwram.h + linker.ld (本轮新增, sub_8050014 新按下边沿) |
| gUnk_03000ED8 | 0x03000ED8 | u8 | iwram.h + linker.ld (本轮新增, sub_8050014) |
| gUnk_0862D434 | 0x0862D434 | u16(*)(u32*)[] | code_1c.c 声明 + linker.ld 绝对(SECTIONS 外) (本轮新增, 主循环调度表) |
| gUnk_03000E6C | 0x03000E6C | u32 | iwram.h + linker.ld (本轮新增, 脚本指针; sub_8052580 写入 / sub_8050014 读取) |
| gUnk_03000E72 | 0x03000E72 | u8 | iwram.h + linker.ld (本轮新增) |
| gUnk_03000E74 | 0x03000E74 | u8 | iwram.h + linker.ld (本轮新增) |
| gUnk_03000ECA/ECB/ECC | 0x03000ECA-0x03000ECC | u8 ×3 | iwram.h + linker.ld (本轮新增, sub_8052580) |

## 头文件原型修正记录

| 函数 | 原声明 | 修正为 | 依据 |
|---|---|---|---|
| sub_8020C2C | void () | u8 (void) | 返回值 lsls/lsrs #0x18 |
| AddInventoryItem→sub_800AA60 | - | u8 视图见 code_0.h | ROM 符号名必须真定义(asm 块 bl 引用) |
| sub_8020CC4 | void () | void (void*, u8, u8, u16, u8, u16, u16) | 栈参布局 |
| sub_804BBDC | void () | u8 (u8, u32×7) | 调用方 u8 截断 |
| sub_801D19C | void () | u16 () | 返回值使用 |
| sub_804BD54 | void () | u32 (u8, u32) | 同上 |
| sub_8046480 | void () | u32 (u8*, u8*, u8) | 返回值宽度决定截断位置 |
| sub_8020EAC | u8 (void *) | u8 (u8 *) | 同族 sub_8020E90 头文件即 u8*; 字节访问 +0xBE |
| sub_802093C | void () | void (u8 *) | 调用方 r0 传结构体指针, 返回值忽略 |
| sub_8020AB0 | void () | u8 (void) | 调用方 lsls/lsrs #0x18 截断返回值 |
| sub_80489E8 | void () | u8 (u8*, u8*, u8, u16) | r2 lsls#0x18 / r3 lsls#0x10 截断; r1 是 u8* 输出缓冲; 返回计数 |
| sub_80489C8 | void () | u16 (u8*, u16) | 被调方对返回值 lsls/lsrs #0x10 截断; 实参 r1 入口 u16 零扩展 |
| sub_8048A68 | void () | u8 (u8 *) | 调用方 lsls #0x18 后测试非零 |
| sub_8048A88 | void () | void (u8*, s8, s8) | 入口 lsls/asrs #0x18 符号扩展; 递归实参 (s8) 截断 |
| sub_8048ACC | void () | void (u8*, u8, u8) | 入口 lsrs #0x18 零扩展; arg2 存 u8 全局 |
| sub_801B8FC | void () | u16* (u8*, u8, u16) | arg1 lsls#0x18 (u8); arg2 lsls/lsrs #0x10 (u16); 返回指针 |
| sub_801B81C | void () | void (u8*, u8, u8, u16, u8, u32, u32, u16, u16, u16) | 10 参数对象 setter；arg6/arg5 为 u32，arg3/7/8/9 为 u16，arg1/2/4 为 u8 |
| sub_801A884 | void () | u8 (u8*, u8, u8*) | sub_801B878 fallback 保留入口 r2 并转发；调用后按 u8 截断返回值 |
| sub_801AD0C | void () | void (u8*) | sub_801B878 的 case 6/7/8 路径只传对象指针，返回值未使用 |
| sub_801B878 | void () | u8 (u8*, u8, u8*) | 入口截断 arg1；第三参数在 r2 原样转发给 sub_801A884；返回值为 u8 |
| sub_801B0B8 | void () | u8 (u8*, u8) | sub_801B8AC fallback 的双参数调用与 u8 返回截断 |
| sub_801B570 | void () | void (u8*) | sub_801B8AC case 6 只传对象指针，返回值未使用 |
| sub_801B8AC | void () | u8 (u8*, u8) | 入口截断 arg1，所有路径返回 u8；无第三实参保活 |
| sub_8046E18 | void () | u8 (u8*, s8, s8) | 调用方 (8A88/8ACC) 返回值 lsrs #0x18 截断 |
| sub_8008BA4 | void () | void (u8, u8) | 入口 `lsls r0/r1,#0x18`+`lsrs #0x18` 双 u8 截断; arg0 另接 `lsrs #0x16` = u8*4 进指针表 |
| sub_8008124 | void () | **u32 () 保持 K&R 空参, 且定义内不写 return** | 目标完全不使用 r0 = 非void返回+无 return 把 r0 锁死(经验 54); 写成 `u32 sub_8008124(void)` 定义 + 头文件 `u32 sub_8008124();`, 调用方 `sub_8008124();` 代码生成不变 |

| sub_8045F10 | void () | u8 (u8 *, u16) | 调用方 `lsls/lsrs #0x18` 截断返回值; 入口 `lsls r1,#0x10; lsrs r2,#0x10` = arg1 u16 |
| sub_8009F70 | void () | u16 (u8, u8, u8) | 多个 asm 调用方 `bl` 后直接 `strh r0,[..]` / `strb r0,[..]` 用返回值; 本函数尾部 `lsls/lsrs #0x10` 截断 → u16 |
| sub_8048818 | void () | u16 (u8, u8) | 入口 `lsls/lsrs #0x18` ×2 = 两个 u8 形参; 尾部 `lsls/lsrs #0x10` = 返回 u16 |
| sub_8001030 | (无声明) | u8 (u16) | 定义在 code_0.c:679, 为 code_1c.c 新补 |
| sub_80010AC | (无声明) | u8 (u16) | 定义在 code_0.c:705, 为 code_1c.c 新补 |
| sub_8052808 | void () | u32 (u8) | 入口 `lsls/lsrs #0x18` = arg0 u8; 返回 0/1 |
| sub_8052878 | void () | u32 (u32 *) | 入口 `ldr r2,[r3]` 指针用法 |
| sub_80528C8 | void () | u32 (u32 *) | 同上 |
| sub_8020A0C | void () | void (void *, u8) | code_1.c 真定义形参类型 (修 conflicting types) |
| sub_8020A7C | void () | u8 (u8 *) | 同上 |
| sub_8001030 | (无声明) | u8 (u16) | 定义在 code_0.c:679 但未进任何头文件; 为 code_1c.c 新补声明(同类型, code_0.c 代码生成不变) |
| sub_80010AC | (无声明) | u8 (u16) | 同上, 定义在 code_0.c:705 |

## 已完成 (智能体B 最近批次)

### 2026-09-01 数据表维度 + 结构体校验批次 (0x0805881C 起 5 张表 / gCutsceneAnimConfigTable / CharacterObject / SpriteNode)

**结论 1: 数据表一律保持 1-D 声明**（实测推翻了我自己提的升维方案）。
声明成多维会**强制改变消费者的索引算术**，与 ROM 不一致。对照实验（同一语义三种写法）：

| 写法 | 生成的指令序列 | 与 ROM |
|---|---|---|
| `t[((a>>11)&0x18) + ((b>>13)&6)]` (1-D) | `lsr #0xb; mov #0x18; and` → `add r1,r1,r0` | ✅ **ROM 就是这个形态** |
| `t[shape][size][0]` (3-D) | 变成 `lsr #0xe; lsl #0x1` / `lsl #0x3`，**且 attr1 先于 attr0 读**，还多/少一个 `push {r4,lr}` | ❌ |

→ 维度信息只写注释，不改类型。已归档到 EXPERIENCE.md。

**结论 2: 5 张表的真实维度**（从索引算式反推，非猜测）：
`gWalkMoveDirLut` = 1-D `u8[16]`（D-pad 4-bit 码）; `gWalkAnimFrameMapping` = 2-D `u8[2][4]`（显式 `+4`）;
`gWalkAnimDimTable` = 3-D `u8[4][4][2]` 前 32 B（`shape*8 + size*2`，**后 16 B 零引用应拆出**）;
`gWalkDirectionMapping` = 2-D `u8[3][8]`（显式 `+8`，索引 = `CharacterObject.facingDir`）;
`gSpriteTileCountTable` = 2-D `u8[4][4]`（`shape*4 + size`）。
命名建议（`gObjSizeTable` / `gFacingDirAttrTable`）**未执行**，因为消费者还在 INCLUDE_ASM 且需逐函数验证。

**结论 3: gCutsceneAnimConfigTable 全部声明已逐项验证**（481 条 × 8B）：
`scriptIdx == gfxIdx` 在 **481/481** 成立; `field_6` 在 **481/481 全为 0** → 改名 **`pad_6`**;
`palIdx` ∈ 0..62 ✓; `loopFlag` ∈ {0x00, 0x80} → 实为 bit7 位标志; `scriptIdx` ∈ 0..477 且**不等于**条目下标。
消费者 `CutsceneAnim_Load` 参数已命名：`animId / slot / slotSel`（`slotSel ≥ 100` 是十进制编码：减 100 存槽号 + 置 flags bit6）。

**结论 4: 两个结构体尺寸正确，但有两处实质错误已修**：
`sizeof(CharacterObject)=0x28` / `sizeof(SpriteNode)=0x14` / `sizeof(CutsceneAnimConfig)=8`
—— 已用 agbcc 实编译对账 asm 里的 `idx*40` / `idx*20` 步长 ✓。
- **`SpriteNode.flags` 旧注释是错的**（"bit 0=active, bits 1-7=chain count"）。
  实测：**bits 0-6 = 链的 OBJ 段数**（`flags & 0x7F`），**bit 7 = 隐藏/跳过渲染**
  （`sub_8004F64` 的 `(s8)flags < 0` 直接返回 next；`|= 0x80` / `&= ~0x80` 成对出现），`flags == 0` = 空闲池块。
- **`CharacterObject.gap1C[8]` 不是空隙**：`sub_804F280` 里基址 `=0x03002E80` + `idx*40` 后
  有 `strh [r0,#0x1C]` / `strh [r0,#0x1E]` / `strh [r0,#0x20]` 三处半字写
  → 已拆成 `u16 field_1C/1E/20/22`（尺寸不变，零 codegen 影响，SHA1 保持绿）。

**未做（证据不足，已记录障碍）**：`CharacterObject` 的 `field_1/2/10/12/18/E` 想改名时，
发现按字段名 grep 会被**多个结构体的同名字段污染**（`chara->` 在不同函数里指向不同类型：
既有 0x28 的 CharacterObject，也有带 `equip_atc/base_atc/skills` 的 RPG 属性结构体）。
→ 必须先做逐函数的变量类型解析（或给两个结构体分名）才能安全改名，不能靠全局 sed。

**并发事故修复**：`src/data_87E83F0.c` 因另一个 agent 删注释块正文但留下孤立 `/*` →
草稿定义变 live → `multiple definition` + `.rodata` 溢出 ROM 232 B。
修法 = `git show HEAD:` 回底 + 只重新贴回其**有效** hunk（`gMainTasks`/`gUnk_087E83F8` 的 16 个语义函数名，
已逐个核实均在 ll.cfg + functions.yaml + 原型三处齐全）。详见 INCIDENTS.md。


### 2026-09-01 数据侧命名: 逐扫描线水波效果族 (gWaveSineTable 起头)

从 `src/data_805769C.c` 第一个未命名项开始做的改名批次, 全程 `make` + SHA1 保持绿:

| 旧名 | 地址 | 新名 | 依据 |
|---|---|---|---|
| `gUnk_080576D0` | 0x080576D0 | **`gWaveSineTable`** | 128 项正弦表, `(u8)(int)(100*sin(2*PI*i/128))` 逐项验证 0 误差; 半周期反对称 |
| `HBlankSinTable_Init` | 0x08000C98 | **`HBlankWave_BuildTables`** | 旧名不准: 它不初始化正弦表(那是 ROM 常量), 而是用正弦表**构建**两张逐行偏移表 |
| `HBlank_ApplyLineScroll` | 0x080005A8 | **`HBlankWave_ApplyLineScroll`** | 统一 `HBlankWave_` 前缀 |
| `HBlank_WaveDma` | 0x08008978 | **`Win0H_WaveDmaByVCount`** | **旧名 + 旧注释均误**: 目的端 0x04000040 = `REG_WIN0H` (不是声音 FIFO, FIFO A/B 在 0xA0/0xAC); 且**全 ROM 无调用点 = 死代码** |
| `gUnk_03004560` | 0x03004560 | **`gWin0HWaveTable`** | 上一条的 DMA 源表 |

引用链 (全部实测):
`gWaveSineTable` 在整个 ROM 里只出现 **1 次** (0x08000D50 字面池) → 唯一读者 `HBlankWave_BuildTables`
→ 填 `gHBlankWaveH`@0x03001B60 / `gHBlankWaveV`@0x030019C0 (各 255 项)
→ `HBlankWave_ApplyLineScroll` 在 H-Blank 里按 `(gHBlankWaveRow + VCOUNT) & 0xFF` 逐行写 `REG_BG1HOFS/VOFS`
→ 相位由 `VBlank_UpdateGameScreen` / `VBlank_UpdateScreenSimple` 每帧推进 `gHBlankScrollCounter`。

**新发现的代码生成约束 (补规 61 同类)**: `gWaveSineTable` 必须声明为 **u8**。
消费者依赖 `ldrb`(零扩展) + 无符号除; 改成 s8 会变 `ldrsb` 从而改变生成字节。
副作用是原作者的 mode 1/3 (tableMask=0x7F) 把负半周 (0xFC..0xFF) 当成 252..255 的大正数参除,
算出的是跳变而非正弦 —— **这是原 ROM 行为, 不能顺手修正** (同规 64)。

### 2026-09-01 数据侧命名: WIN0H 虹膜过渡轮廓

`gUnk_080870EC` 已确认为 129 项、范围 0..128 的单调轮廓曲线。它近似四分之一正弦，
但并非标准浮点正弦逐项取整，因此命名为 **`gWindowTransitionCurve`** 而不是 sine table。
唯一消费者 `sub_8005020` 用它把 0..240 的过渡进度映射为逐扫描线半宽，钳制到 120 后
打包成 `WIN0H = left | right << 8`，生成 81 项表并由 DMA0 送入 `REG_WIN0H`，效果是从
屏幕中心展开/收拢的虹膜式场景切换。

| 旧名 | 地址 | 新名 | 语义 |
|---|---|---|---|
| `gUnk_080870EC` | 0x080870EC | `gWindowTransitionCurve` | 129 项过渡轮廓曲线 |
| `gUnk_03004604` | 0x03004604 | `gWindowTransitionProgress` | 0..240，模式 1 每帧 +16、模式 2 每帧 -16 |
| `gUnk_03004668` | 0x03004668 | `gWindowTransitionProgressSnapshot` | 建表前保存本帧进度 |
| `gWin0HWaveTable` | 0x03004560 | `gWindowTransitionScanlineTable` | 81 项 packed WIN0H 边界 |
| `sub_8005020` | 0x08005020 | `VBlank_UpdateSpriteAndWindow` | VBlank 资源传输 + 虹膜过渡更新 |
| `sub_80051D0` | 0x080051D0 | `ScreenTransition_UpdateBlend` | BLDCNT/BLDALPHA/BLDY 过渡更新 |


| sub_800A86C | ✅ 一次命中 | 参考草稿转正 (9999999 上限 + gUnk_08092248 累减); 需要 s32 typedef |
| sub_800A8A0 | ✅ 一次命中 | 参考草稿转正 (08092248 累加) |
| sub_800A8D0 | ✅ 一次命中 | 参考草稿转正 (08093418 步长5双字段搜索); 修正头文件 void→u8×3 |

| sub_8008978 | ✅ 一次字节命中 | 声音DMA0旋转(VCOUNT同步); **0x040000B0=DMA0SAD 不是DMA3**(io.h宏核对纠正); 寄存器读用宏+字节读用 *(u8*) 强转; void+有值return的松散返回; **最终形态用 DmaSet 宏**(macro.h 自带, 含 dmaRegs[2] 回读); 0x04000040 非 TM0 寄存器(io.h 核对), 保留字面量 |

| sub_80209EC | ✅ 一次命中 | 参考草稿转正; MyStruct(0x88/0xB0/0xBE) 位标志 |
| sub_8020A0C | ✅ 一次命中 | 与 sub_80210C0 孪生(逐字节相同的函数体), 解法直接复用 |
| sub_8020A7C | ✅ 一次命中 | 参考草稿转正 (0xC8 步长×5 项 sub_8045F10 检查) |
| sub_8020B04 | ✅ 一次命中 | 44A4 模式变体(ids[12]/0x7F/sub_801D568); 修正 sub_8045F10 头文件 void→u8 (也解开了另一智能体 AB0 的阻塞) |

| 函数 | 状态 | 备注 |
|---|---|---|
| sub_8020B90 | ✅ 一次字节命中 | 参考草稿转正; `gUnk_03000718 = (u32)arg0` 形式 |
| sub_8020BC0 | ✅ 一次字节命中 | 参考草稿转正 (0x6C/0xB2 s16/u16 差值扣减) |
| sub_8020BF0 | ✅ 一次字节命中 | 参考草稿转正; 修正 sub_801E848 头文件 void→u8 |
| sub_8020B54 | ⏸ 放弃 | 见下方挂起区 (寄存器轮换 r5/r6/r4) |

## 待研究 / 挂起

### sub_80529B8 (0x080529B8) — 指令序列已全对, 多占一个寄存器 (score 1510)

- 最佳候选 `permuter/sub_80529B8/v2.c`（= v3/v8/v10 同分）：
  ```c
  u8 *data; u8 idx; u8 ret;
  data = (u8 *)*ptr;      /* ldr r0,[r3]  —— 目标里这是临时量, 落在 r0 */
  ret = 0;                /* movs r4,#0 */
  idx = data[1];          /* ldrb r6,[r0,#1] */
  if ((gUnk_03000E70 & 0x20) == 0) { gUnk_03000E74 = 0; gUnk_03000E70 |= 0x20; }
  else if (gUnk_03000E74 < idx) { gUnk_03000E74++; }
  else { gUnk_03000E74 = 0; gUnk_03000E70 &= ~0x20; *ptr += 2; ret = 1; }
  return ret;
  ```
  语句顺序必须 `data / ret / idx` 才能得目标的 `ldr; movs; ldrb` 三段式
- **指令逐条一致**, 只差: 目标 `push {r4,r5,r6,lr}` + {ptr→r3, ret→r4, E70addr→r5, idx→r6, data→r0};
  我 `push {r4,r5,r6,r7,lr}` + 多一个寄存器(data 拿了 r6)
- **根因（用 `-dl` 查到）**: `data` 的 qty = `3 refs / 24 insns` —— 因为尾部 `*ptr += 2` 被 CSE
  复用成了 `data + 2`（RTL 行 `(plus:SI (reg/v:SI 23) (const 2))` + REG_DEAD reg 23）。
  目标尾部是 `ldr r0,[r3]; adds r0,#2; str r0,[r3]` —— **重新加载了 `*ptr`**,
  说明原代码里没有一个能活到尾部被 CSE 传播的 `data` 变量
- 已试: v2(分离赋值)/v3(声明迵初始化)/v4(无 data 变量)/v5(u32 base)/v6(u32 ret)/v7-v10
  → 全部 1510 或更差; 无 data 变量时反而变成 `ldr; ldrb` 相邻(顺序错)
- 下一步: 找一个能“阻止 CSE 把 data 传播到尾部”的写法（例如中间插入会刷新 mem 等价项的
  存储、或目标确实用了不同的基量）

**2026-09-03 已解 (gpnux, 92B exact)**:
- 根因确认 = 经验 33 手法直接命中: 在 `idx = data[1]` 后加一条死 store `data = 0;`。
  agbcc 会删除该 store 不发任何指令, 但它先杀死了 `data` 伪寄存器的 liveness,
  CSE 无法再把它传播到尾部 `*ptr += 2` → 尾部被迫 `ldr r0,[r3]; adds r0,#2; str r0,[r3]`
  重加载, 与目标逐指令一致。多占的 r7 消失 (push {r4,r5,r6,lr})。
- 语句顺序保持 progress 早期结论 `data / ret / idx` 三段式; 末尾 if/else 链用单行花括号。

### sub_8052AE8 (0x08052AE8) — 字节池临时量 home + 双池加载 (score 1075)

- 语义已破解, 最佳候选 `permuter/sub_8052AE8/w2.c`：
  ```c
  data = (u8 *)*ptr;
  ofs = gUnk_02016000[data[1]];
  if (data[1] < data[2]) {
      u8 diff = (u8)(data[2] - data[1]);
      ofs = gUnk_02016000[(u8)(data[1] + (sub_8018844(ofs) % (diff + 1)))];
  }
  *ptr = ofs + (u32)gUnk_02016200;
  ```
- **已解决的两点**：① `__modsi3` vs `__umodsi3` → 需要无符号操作数,
  用宽返回声明 `extern u32 sub_8018844_wide() __asm__("sub_8018844");` 同时解决
  目标里**没有** u16 返回截断(`lsls/lsrs #0x10`)的问题；② diff 必须在调用**前**算好并活在
  被调保存寄存器里 → 必须拆成独立语句 `diff = (u8)(data[2]-data[1]);` 在调用之前
- **卡点**（与经验 19/35-39 同族）：目标把 `gUnk_02016000` 基址**从同一个池字加载两次**
  (r1 在 0x06, r0 在 0x30), 且 `ofs` 直接落在 r0 无需 `adds r0, rX, #0` 预备;
  我的写法只得到一次加载(基址被 CSE 保留, 被迫占用 r7 → 多一个 `push {r4,r5,r6,r7}`)
- 已试: w1/w2/x1-x5/y1-y8/z5-z8 共 15+ 种拼法, 均停在 1075
- 下一步: 目标基址落在 call-clobbered 的 r1 上 → 它的 qty 必须不跳调用;
  即两个 `gUnk_02016000` 引用在 RTL 里是两个独立 qty。CSE 为何没合并需要查
  (cse.c 里本版本无 `plus_low_order_part`, 怀疑与 `no_more_replacements`/`reg_tick` 有关)

### sub_80531A8 (0x080531A8) — 寄存器 home (ptr/data 的 r1↔r2 互换)

- 语义已完全破解, 生成的**指令序列与目标逐条一致**(含池加载位置), 只差 home 寄存器编号:
  - 目标: `ptr→r1`, `data(=*ptr)→r2`, 两个 EWRAM 池临时量→r2
  - 现状: `ptr→r2`, `data→r1`, 池临时量→r1
- 最佳候选写法 = `permuter/sub_80531A8/base.c` (permuter score=70 = 7 行纯寄存器差异)
  关键: 表地址必须写成 `*(u16 *)((u32)gUnk_02016000 + data[1] * 2)` 这种内联形式 ——
  若先引入 `tbl`/`ofsPtr` 局部, 池加载会被提前到 `ldrb` 之前(与目标不符)
- **根因已定量定位**(用新学的 `agbcc -dl` 转储, 见 EXPERIENCE.md):
  - ptr qty: `used 4 times across 24 insns` → pri = floor_log2(4)*4/24 = **3333**
  - 池加载临时量: `2 times across 4 insns` → pri = **5000** → 先分配, 抢走 r1(r0 已被 block4 其他临时量占)
  - 要让 ptr 拿 r1 需 pri ≥ 5000, 即 `n_refs(ptr) ≥ 6` 或 `life(ptr) ≤ 16`
- 已试: ~620 种等价写法(6 种表访问 × 6 种取值 × 3 种条件结构 × 声明/转换/空语句/别名变体)
  → `n_refs` 恒为 4, `life` 恒为 24; permuter 12k 迭代同样停在 70
- **别名技巧不可用**: `u32 *p1 = ptr;` 能把 n_refs 抬到 5, 但会触发 GCC2 CSE 误编译
  (直接吞掉 `ldr rX, [ptr]`, 把 data 当 ptr 用), 生成的代码错
- 下一步候选: ① 找 n_refs=6 的真实原始写法(可能有未识别的 ptr 引用形态);
  ② 给该函数单开编译单元试 `-O1`/`-g` 变体(实测本函数 -g/-O1 均不改变分配);
  ③ 与 sub_8053138/sub_805321C 同族(见经验 19), 建议合并攻坚；sub_801A684 已由经验 83 独立收尾

### sub_8021184 (0x08021184) — 寄存器 home 之谜 (✅ 已匹配 2026-09-06 opencode)
- 逻辑已完全清楚: switch((s8)arg0) case 0/3/6/7, gUnk_03000788 行数组(步长5)混合
  s8/u8 字段访问, gUnk_0300076A/76C/770/781/782/808/809/80A 符号已注册(iwram.h+linker.ld)
- permuter 四件套已建好 (target.o 可用), 当前最佳 score=905 (permuter) / 1490 (手工)
- **卡点**: 头部块的目标分配是 b(=arg0截断)→r3, idx→r2, ptr→r1, val→r0 (r0-r3 全占);
  我的所有变体都是 b→r2, idx→r1(与 ptr 复用), r3 空闲 —— 差一个寄存器的让位
- 已试: vu8* 参数 / 混合 volatile / ptr 局部 / u8 b 命名局部 / 三元运算符 / permuter 4min
- **下一步猜测**: ① 头部可能还有第 4 个存活值未识别; ② else 重读的来源可能是其他别名符号;
  ③ 或原 C 的 if/else 结构不同(如嵌套 if)。用 m2c 输出对照过, 语义一致, 纯分配问题
- 案例 3/6/7 的写法已验证正确(除寄存器号外逐行一致), 主体 C 可从 permuter/sub_8021184/base.c 继续

**✅ 破解 (2026-09-06 opencode)**: 头部三段一次性写对 (经验 163):
```c
u8 b; u8 idx; u8 *ptr;
b = (u8)arg0;
ptr = arg1 + 0xBE;
if (*ptr != 0) idx = *ptr - 1; else idx = *ptr;
switch ((s8)b) { ... }
```
关键: ① `b = (u8)arg0` 显式截断; ② `ptr = arg1 + 0xBE` 直接改指针并连读三次 `*ptr`
(val 被 CSE 进 r0, else 分支重读 `ldrb r2,[r1]`); ③ **`if/else` 显式结构而非三元** ——
三元会编成 `bne→then` (else 直落), 目标要 `beq→else` (then 直落) + b/r3,idx/r2,ptr/r1,val/r0
四寄存器 home 全对。bytecmp OK 304B; 原型 `void sub_8021184();` → `(u8, u8*)`。
调用方全部未匹配 (sub_802151C/2192C/23820/24940), 无字节风险。

### PartyForm_ApplyBonus (原 sub_800AC08) — ✅ 已匹配 (2026-08-31, 见表格区新条目; 以下为旧记录存档)
- 逻辑已清楚: 4 个 0x087EA580 表项(步长12)的 +4 字段高半字节一致性检查,
  一致时按其值(>>4==0xE/0xF)写 gUnk_03004AAC/03004A80 = 0x3C/0x3F 或 0x22/0x2D
- permuter/sub_800AC08/ 套件保留 (target.o 可用), 最佳 score=67:
  tbl 局部 + p0/p1/p2/p3 指针局部形态 (v4)
- **卡点**: 目标指针 home = p0→r5, p1→r4, p2→r7, p3→r6(复用池寄存器), pool→r6;
  我的变体 pool→r6 ✓ 但 p0→r4, p1→r5 —— p0/p1 的 home 交换未解
- 已试: 直接数组访问 / 指针局部 / tbl 局部+tbl+a 形式 / p0/p1 声明交换 / permuter 3min(反优化)

### ⭐ 三个挂起项已用 qtydump 定性 = global-alloc 域（2026-09-01）

`scripts/qtydump.sh`（诊断补丁版 `agbcc_qtydump`，只 dump **local-alloc** 的 qty 优先级表）扫了
`sub_8009370` / `sub_8018E34` / `sub_804BE90` 的最优候选，三个的争议值**都不在表里**：

| 函数 | bytecmp | 表内 qty | 最长 life | 0-qty 的块 | 争议值 |
|---|---|---|---|---|---|
| sub_8009370 | 78/216 | 16（全部分到 r0） | 30 | 3（含循环体） | 表基址 / 数据指针（跨块） |
| sub_8018E34 | 37/152 | 17（只用 r0/r1） | 12 | 1 | 尾段基址进 r0 还是 r1 |
| sub_804BE90 | 64/168 | 14 | 8 | 2 | 表基址 vs `-1` 谁进 sl |

已核对 `toplev.c`：**local-alloc 先跑、global-alloc 后跑**。所以
① 这些跨块长寿命值由 global-alloc 决定，local-alloc 表看不到它们；
② local-alloc 敢不敢用 r0，取决于 flow 对**硬寄存器**存活性的判定（入口参数/返回值寄存器/调用点），
   而不是 C 层表达式形状。⇒ **穷举等价 C 写法改不动这一类**，三个函数的"已试无效"清单见各自小节。

可行的两条路（已写进 EXPERIENCE.md「失败案例存档」开头）：
- (a) 给 `tools/agbcc/gcc/global.c` 也打一个转储补丁（照 `scripts/patches/agbcc-qty-dump.patch` 的路子），
      这才是这三个的决策层；
- (b) **先查函数签名**（形参个数/类型、返回类型、有无 return）—— 成本极低且已被验证：
      `sub_8008124` 就是靠"非 void 返回 + 体内无 return"锁死 r0 一击破解的（经验 54）。

### sub_8009370 (0x08009370) — 挂起 (ptr 基址未被 CSE 保留 → 寄存器 home 级联)
- **指令序列已 95% 复现** (w11, 79/184 字节差, 且差值全集中在 3 处), 语义完全清楚:
  `if (gUnk_03004910) sub_80094FC(); else { sub_8003264(); for (i=0;i<=3;i++) { b=gUnk_03000010[i];
  if (b!=0 && (b&4)==0) DmaSet(3, &gMenuEntityPaletteTable[(gUnk_03000038[i][gUnk_03000020[i]>>gUnk_03000018[i]] << 5)+2], gUnk_03000028[i], 0x80000010); } }`
- 关键结构已拿下: `s16 i` + `for (i=0;i<=3;i++)` 自然产生 `lsls/asrs #0x10` 与 `+0x10000; >>16` 的 s16 归一化舞步;
  `DmaSet` 宏的 `dmaRegs[2];` 死读也对上了
- **卡点 1 (根因)**: 目标把 `0x0808A234` 基址提升进 r8 (`ldr r7,=sym; mov r8,r7` 在入口),
  而 `0x03000010` 反而在循环内现取。我直接写 `gMenuEntityPaletteTable` 时 GCC2 会把 `+2` 折进符号地址
  (池变 `gMenuEntityPaletteTable+0x2`, 丢一条 `adds r0,#2`); 改用局部 `u8 *ptr = gMenuEntityPaletteTable;` 可保住 `+2`,
  但 CSE 又把 ptr 当 `unique_reg_constant` 代入使用点 → 基址没提升, 反而挤掉了 0x03000010 的位置。
  即: **需要一个"能撑过 CSE 常量代入"的基址局部变量**。
- **卡点 2**: `movs r0,#4; ands r0,r2` (目标, 结果落常量的寄存器) vs 我的 `ands r2,r0` (结果落 b 的寄存器)。
  全 ROM `ands rX,#imm` 立即数形式 **0 次**, `ands rX,rY` 2650 次 —— 所以 4 必然先物化进寄存器;
  目标说明 RTL 里常量在前 (`(and (reg4) (regb))`), 而 GCC2 的 `swap_commutative_operands_p` 会把 const_int 换到后面。
  猜测: 卡点 2 是卡点 1 的级联后果, 修好提升就自然对。
- **已试无效**: ptr 放函数顶/else 顶/bl 前 (bl 前会多 `sub sp,#4` 溢出到栈)、基址写成 `base+2+idx` 等 5 种结合顺序
  (全部被折叠)、b 改 u32/s8、`4&b`/`!(b&4)`/`(b&4)==0` 三种写法、flag 变量形式
- 套件保留 `permuter/sub_8009370/` (target.o 可用, 最佳 = w11.c)

> ⚠ `permuter/` 在 `.gitignore` 里，最优候选**不随仓库分发**，故把源码内联在此备查。
```c
/* permuter/sub_8009370/base.c —— bytecmp 结果见上，勿直接合入 */
typedef unsigned char u8;
typedef unsigned short u16;
typedef signed short s16;
typedef unsigned int u32;
typedef signed int s32;
typedef volatile unsigned int vu32;

extern u8 gUnk_03004910;
extern u8 gUnk_03000010[4];
extern u8 gUnk_03000018[4];
extern u16 gUnk_03000020[4];
extern u32 gUnk_03000028[4];
extern u8 *gUnk_03000038[4];
extern u8 gMenuEntityPaletteTable[];
extern void sub_80094FC(void);
extern void sub_8003264(void);

void sub_8009370(void)
{
    s16 i;
    u8 b;
    u8 *ptr;

    if (gUnk_03004910 != 0)
    {
        sub_80094FC();
    }
    else
    {
        sub_8003264();
    ptr = gMenuEntityPaletteTable;

        for (i = 0; i <= 3; i++)
        {
            b = gUnk_03000010[i];
            if (b != 0 && (b & 4) == 0)
            {
                vu32 *dmaRegs = (vu32 *)0x040000D4;
                dmaRegs[0] = (u32)(ptr + ((*(u8 *)(gUnk_03000038[i] + (gUnk_03000020[i] >> gUnk_03000018[i])) << 5) + 2));
                dmaRegs[1] = gUnk_03000028[i];
                dmaRegs[2] = 0x80000010;
                dmaRegs[2];
            }
        }
    }
}
```

### sub_8018E34 (0x08018E34) — ✅ 已匹配 (2026-09-05)
- **破局关键**: 旧候选全是**早返** (`if(...) return ...;`), 各分支自带 `ldrb + b 尾`。
  改成**具名 `u8 ret` + if/else-if 链 + 末尾单 `return ret`** 后, 4 条路径汇成**单个出口块**
  (目标唯一的 `_08018E9A` `ldrb r0,[r0]`), 尾段表基址进 r1、x 不重读, 整函数逐字节命中
  (**fncheck OK 116B**)。打破了 "global-alloc 域三连" 里对本函数的"改不动"判定 → EXPERIENCE §失败案例存档已加反例。
- 命名与符号: 0x03004820 即已注册 `gEncounterEnabled` (u8), src 沿用该名; 三张 ROM 表
  `gUnk_083989B0/CB/DC` (0x083989xx) 新注册 linker.ld 外层绝对符号区。`code_0.h` 原型
  `void sub_8018E34()` → `u8 sub_8018E34()` (唯一调用者 sub_8018A58 未匹配 INCLUDE_ASM, 改返型零风险)。
- 语义: 依 `gGstate324`(u16 输入) 位 0x20/0x200 与 `gEncounterEnabled` 查三张菜单/对话图标表,
  返回 u8 图标 ID; sub_8018A58 把它乘 12 作 0x087ED394 压缩图表索引 (LZ77 加载)。

> 挂起期旧分析存档 (早返版 37/152, 尾段 base→r0 vs base→r1 之争, 勿再试):
> - 语义 (四路查表, 返回 u8):
>   ```c
>   u8 sub_8018E34(void) {
>       if (sub_80187B4() & 0x20)  return gUnk_083989CB[(u8)sub_80187A8() - 0x3a];
>       if (sub_80187B4() & 0x200) return gUnk_083989DC[(u8)sub_80187A8() - 0x1c];
>       if (gUnk_03004820 == 0)    return gUnk_083989B0[gUnk_03004820];
>       return gUnk_083989B0[gUnk_03004820 - 1];
>   }
>   ```
> - 已一次命中的细节: `movs r1,#0x20; ands r1,r0` (目的=常量寄存器) ✓;
>   `movs r1,#0x80; lsls r1,#2` 物化 0x200 ✓; `(u8)sub_80187A8()` 的 `lsls/lsrs #0x18` ✓;
>   `subs r0,#0x3a` / `subs r0,#0x1c` 无后置截断 ✓; `== 0` 先写才得到目标的 `beq` 极性
>   (写成 `!= 0` 会得到 `bne`, 45 分 vs 38 分)
> - 旧差异: 尾段目标 `ldr r1,=base; subs r0,#1; adds r0,r0,r1` —— 表基址进 **r1**, x 留 **r0** 不重读;
>   早返版 `ldr r0,=base; ldrb r1,[r1]; subs r1,#1; adds r1,r1,r0` —— 基址抢 r0, x 被重读;
>   即纯 local-alloc 对 base qty 的寄存器选择与 CSE 是否判定 x 在 cmp 后死亡互为因果。
> - 旧已试无效 (~20 种): `!=0`/`==0` 极性、if-else 显式 else、三元、具名 `u8 x` 局部、
>   `(u32)base + x - 1`、`base[-1 + x]`、`(u8)(x-1)` 截断、`u8 *addr` 统一出口(106, 大幅变差)、
>   permuter 150s(最优 100)。经验 77 (指针加法操作数顺序) 再次验证改不动。

### sub_804BE90 (0x0804BE90) — 挂起 (表基址与 -1 谁进 sl)
- 语义已完全破解, **指令序列 90% 一致** (最佳 65/132 字节差, 差值集中在 4 处):
  ```c
  void sub_804BE90(u8 arg0, u8 arg1) {
      for (i = 0; i < arg1; i++) {
          ptr = &gUnk_03000BE8[(arg0 + i) * 16];
          if ((s8)ptr[0] != -1) {
              if (!(ptr[0] & 0x20)) sub_804C5F8(ptr[1], 1);
              sub_804C674(arg0 + i);
              ptr[0] |= mask; ptr[1] |= mask; ptr[2] = 0; ptr[3] = 0;
          }
      }
  }
  ```
- **已确证的关键结论**: `ptr[0] |= 0xFF` 用**字面量**时 GCC2 必然折叠成 `movs r0,#255; strb r0,[r4]`
  (试了 `|=0xFF` / `=0xFF|ptr[0]` / `|=(u8)-1` / `|=~0` / `=*ptr|0xFF` 共 5 种拼法, 全部折叠)。
  目标却是真 RMW (`ldrb r0,[r4]; mov r1,r8; orrs r0,r1; strb`) 且 r8 = 预置的 0xFF
  → **原代码这里用的是一个变量**。用 `u8 mask;` 且在循环内赋值 `mask = 0xFF;`
  → GCC2 自动把它当循环不变量提到 preheader 并分配 r8 (跨两次 bl 存活), 形态即与目标一致 ✓
- **剩余卡点**: 目标把**表基址 0x03000BE8** 提到 preheader 放进 `sl`、而把 `-1` 留在循环内每次重算
  (`movs r2,#1; rsbs r2,r2,#0`); 我的所有变体恰好相反 (`sl` = -1, 基址每轮 `ldr r1,[pc,#84]`)。
  即两个循环不变常量谁被提升的选择不同, 连带 `mov r0,r9` vs `mov r2,r9` 的 scratch 编号差异。
- **已试无效**: 基址写成 `base + n*16` / `(u32)n*16 + (u32)base` / `&arr[n*16]` / `(u8*)((u32)base + n*16)`
  四种结合顺序 —— **分数完全不变**, 说明指针加法的操作数顺序被 GCC2 规范化, 经验 2 不适用于指针;
  另试: mask 提到循环外(72)、`u8 *base` 局部变量(64, 被 CSE 代回)、`u8 n = arg0+i` 具化(101~107, 更差)、
  `do{}while(0)` 屏障包 mask(65)、循环内第二次引用 `gUnk_03000BE8[...]`(65/81)
- **有效线索**: 在循环体内**额外加一次** `gUnk_03000BE8[...]` 引用 (b3 实验, 语义不等价) 把 65 → 57,
  说明"基址被 CSE 共享 → 提升"方向是对的, 缺一个语义等价的第二次引用形式
- 套件保留 `permuter/sub_804BE90/` (base.c = 最佳 w3)

> ⚠ `permuter/` 在 `.gitignore` 里，最优候选**不随仓库分发**，故把源码内联在此备查。
```c
/* permuter/sub_804BE90/base.c —— bytecmp 结果见上，勿直接合入 */
typedef unsigned char u8;
typedef signed char s8;

extern u8 gUnk_03000BE8[];
extern void sub_804C5F8(u8, u8);
extern void sub_804C674(u8);

void sub_804BE90(u8 arg0, u8 arg1)
{
    u8 i;
    u8 *ptr;
    u8 mask;

    for (i = 0; i < arg1; i++)
    {
        ptr = &gUnk_03000BE8[(arg0 + i) * 16];
        mask = 0xFF;
        if ((s8)ptr[0] != -1)
        {
            if (!(ptr[0] & 0x20))
            {
                sub_804C5F8(ptr[1], 1);
            }
            sub_804C674(arg0 + i);
            ptr[0] |= mask;
            ptr[1] |= mask;
            ptr[2] = 0;
            ptr[3] = 0;
        }
    }
}
```

### sub_80051D0 (0x080051D0) — 挂起 (movs r4,#0xff 位置)
- 逻辑已完全清楚 (m2c 核对): 画面淡入淡出控制 —— REG_BLDCNT/REG_BLDALPHA/REG_BLDY
  (0x04000050/52/54 = BLDCNT/BLDALPHA/BLDY!), 0x0300465C 状态 + VCOUNT 差值的 s16>>4 定点,
  gUnk_03004A80/0x03004AAC 写 0xBE/0x3F 或 0xFF/0x22
- permuter/sub_80051D0/ 套件保留 (target.o 可用), 最佳 score=18:
  permuter 结构 (顶部 if(new_var){=0xFF}else{=0xFF} 屏障 + 0x465C 两读由 CSE 合并)
- **卡点**: 目标的 movs r4,#0xff 在 0xc (check 判断之后、vcount/0x47A8 两池加载之间);
  我的变体的 movs 要么在 0x2 (函数入口提升) 要么在尾部 store 处 —— 就差一个位置
- 已试: value 局部(函数顶/if内/赋值分离) / both-branches 屏障(if内) / permuter 4min
- 宏已按规范用 REG_BLDCNT/REG_BLDALPHA/REG_BLDY + vu8 字节读; 语义 C 完整无误
- **下一步猜测**: 原代码的 value 赋值可能嵌在更深层的结构里 (如三层 if 或逗号表达式),
  使 GCC2 的 local_alloc 给它 r4 (callee-saved, 带 push {r4,lr})

### sub_8020B54 (0x08020B54) — 放弃 (寄存器轮换)
- 逻辑简单: 清 gUnk_030006F8[0..6] + gUnk_03000714/715/716 = 0 (符号已注册)
- **卡点**: 目标三地址分配 714→r5, 715→r6, 716→r4 (第三个回卷到 r4);
  语句序 714/715/716 得 r4/r5/r6; 716,714,715 序寄存器对上了(r4=716,r5=714,r6=715)
  但存储/池顺序又不符。6 种排列全试未命中
- permuter/sub_8020B54/ 套件保留 (target.o 可用), 智能体A 在 EXPERIENCE 经验17
  有独立记录(QTY_CMP_PRI 排序假设)
- 若破解: 参考 GCC2 local_alloc 的 QTY_CMP_PRI = floor_log2(n_refs)*n_refs*size/life

### sub_8021788 (0x08021788) — 已匹配 ✅ (2026-09-04 opencode)
- 语义: `switch(gUnk_03000816)` case 0/1/2。case 0: `gUnk_03000818 & 0x1000` 为真 →
  `DialogCtx_GetField_C(0)==0` 则清 018 位, 否则 `v=DialogCtx_GetField_C(0); if (v==4 &&
  (v & gUnk_0300076C)==0)` 调 `sub_802181C(0x02035AC0, 0x18, 2, arg0)`; case 1: 置 018 位
  + 016=0 (fallthrough 到 case 2 的 `gUnk_03000816 = 0`); case 2: 016=0。
- **卡点破解 (`ands r1, r0` 结果落 res 寄存器)**: 把 res 声明成 **u32** (宽返回) 后, 再给 AND
  的左操作数加 **`(unsigned short)` 强转**: `if (((unsigned short)v & gUnk_0300076C) == 0)`。
  GCC2 就会让 AND 结果落回 res 的寄存器 (r1, `ands r1, r0`) 而非 gUnk 装载的寄存器 (r0,
  `ands r0, r1`)。已穷举失败的变体: res u8/u16/u32、`&&`链/嵌套 if、res 左/gUnk 左、
  compound `res&=`、mask 局部、结构体成员视图、u32 mask 提前装载 —— 全都 `ands r0, r1`。
  **机理**: u16 强转把 res 变成 HImode 子寄存器 subreg, 改变了 expand 里 `andsi3` 的
  目标寄存器选择 (local-alloc 把 dest 与"在本 insn 死亡"的源绑定, subreg 形式让 res 的 qty
  优先保留 home r1)。→ 经验 78 (可交换运算目的寄存器) 的又一个触发开关: **目的寄存器选错时,
  给其中一个操作数加窄类型强转试试**。
- 宽返回: 本 C 文件 调用点需要 `u32` 返回 (目标无 lsls/lsrs #0x18 截断), 用
  `extern u32 DialogCtx_GetField_C_wide(u8) __asm__("DialogCtx_GetField_C");` 别名解决
  (不与 code_0.h 的 u8 声明冲突; code_1.c 的 u8 调用方不受影响)。
- `sub_8021788` 头文件原型从 K&R `void f();` 改为 `void sub_8021788(u8 arg0);` —
  仅有的调用方 `sub_802192C` 仍是 asm, 无字节影响 (经验 7 的顾虑只针对已匹配调用方)。
- 结果: bytecmp 仅 3 个 bl 槽位不同 (bytecmp 排除类别; 多余 36 字节是 abs-symbol 测试的
  linker veneer, 非真代码); fncheck OK 148 bytes。
- permuter 套件保留 (base.c = 最终真身), output-20-1 是 u16 强转形态。

### sub_8004F64 (0x08004F64) — 挂起 (CSE 折叠掉一次 `*oamIdx` 重读, 纯 C 无解)

- 语义已完全破解 (精灵链节点 → OAM 缓冲渲染 + 游标推进), 参考草稿见
  `src/code_8004F64` 上方的注释块 (已按项目真实类型 `SpriteNode`/`gOamBuffer.attrs[]` 重写)。
- **已确证的一条**: `(s8)node->flags < 0` 一击命中 `movs r0,#0; ldrsb r0,[r4,r0]`。
  该形态出自 `tools/agbcc/gcc/thumb.md` 的 `*extendqisi2_insn`: 当地址是**裸寄存器**
  (无 PLUS) 且目的寄存器 ≠ 基址寄存器时, 它输出 `mov %0,%2; ldrsb %0,[%1,%0]` (ops[2]=const0)。
  → 以后凡是"`movs rX,#0` + `ldrsb/ldrb [rY,rX]`"就是**偏移 0 的符号扩展字节读**, 不用猜。
- **卡点**: ROM 在 0x4F6E/0x4F76/0x4F86 读了三次 `*oamIdx`, 而游标自增用的是**第一次**的值 (r6)。
  即需要"一个 u16 局部量 + 两处必须重读的取地址"。任何纯 C 写法都会被 GCC2 CSE 把
  局部量折进第一处取地址 (`lsls r2,r6,#3`), 少一条 `ldrh` → 44/68 字节差, 并连带 r5/r6 互换。
- **已穷举无效** (60+ 形态, 全部用 `scripts/bytecmp.sh` 字节级判定, 不信 fndiff score):
  下标写 `*ptrIndex`/`ptrIndex[0]`/`*(u16*)p`/`((T*)p)->v`(标量成员/union/位域/大结构)/
  `(u32)`/`(s16)`/`const u16*` 视图; 局部量 `u16`/`u32`; 自增写 `*p=idx+1`/`++idx;*p=idx`/
  `(*p)++`/`*p+=1`/`*p=*p+1`; 比较写 `idx>0x7F`/`idx<128`/`*p>0x7F`; 赋值进 if 条件;
  `&&` 合并条件; 声明处初始化; `idx=idx;`/`do{}while(0)`/空语句屏障; 恒等 store `*p=*p`。
  编译选项变体 `-O1`/`-Os`/`-O2 -g`/`-fno-gcse`/`-fno-cse-follow-jumps` 全部 45 字节差 (不变)。
- **根因 (读 cse.c 定位)**: 阻止 `mem==reg` 代入只有两条路 —— `do_not_record` (volatile /
  PRE_INC/CALL/ASM) 或在中间插入一次真 store 触发 `invalidate()`。本函数两者都不存在。
  `MEM_IN_STRUCT_P`/alias set 都**不**参与 `lookup()` 的比较 (只比 mode + `exp_equiv_p`),
  所以"用结构体视图读、用裸指针读"骗不过它 (实测)。
- **唯一逐字节一致的写法**: 两处下标写 `*(volatile u16 *)oamIdx` (bytecmp: OK 68 bytes)。
  **不合入** —— 违反经验 79: 调用方 `sub_80032BC` 传的是它自己的栈上 `u16 index`, 没有
  异步共享语义, 这里加 volatile 纯粹是代码生成工具。按经验 62 的先例保留 INCLUDE_ASM。
- **新工具**: `scripts/bytecmp.sh <func> <候选.c> <abs符号...>` —— 对**候选文件**做
  部分链接 + `cmp` 字节级判定。补上了 fndiff/fncheck 之间的空档:
  `fndiff` 的 score 会假阳性 (实测一个 `idx` **未初始化**、少一条 `ldrh` 的破代码也报 score=400),
  `fncheck` 只能验已合进 `src/` 的真身。候选阶段要用 bytecmp。

### 2026-09-01 Actor 字段推测命名 (逐函数变量类型解析批次)

**先解决方法论问题**: 上一轮发现"按字段名全局 grep 会被多个结构体的同名字段污染"
(`field_1/2/10/12/17/18/1A/24/E` 共 16 个字段名与其它 16 个结构体重名)。本轮写了两个工具彻底解决:

| 工具 | 作用 |
|---|---|
| `scripts/typecov.py <类型>...` | 逐函数解析形参/局部声明/`v=&GLOB[i]`/`v=GLOB+n`/`v=GLOB` 传播, 把字段访问归到**真实类型** |
| `scripts/rename_scoped.py <类型> '<json映射>' [--apply]` | 只改类型解析确认属于目标结构体的那些访问点, 不碰其它结构体同名字段 |

**验证方式 = 编译器本身**。改完使用点但忘改结构体定义 → `structure has no member named`
直接报出漏改/误改的行。本轮共拓出 3 个工具 bug:
1. `strip_c` 删注释不保留行数 → 行号漂到完全不相关的行 (必须用等量换行回填)
2. `gSpriteNodePool[charaObj->field_18]` 被外层正则整体匹配后**吞掉了内层的 Actor 访问**
   → 变量字段正则不能带前导 `\[[^\]]*\]`
3. `ptr2E80 = gActors;` (裸全局赋值) 不在传播模式里 → 需加 `^([A-Za-z_]\w*)$`

**语义来源 = `Chara_ProcessCmdStream` (live C 命令解释器)**, 不是猜的:
`temp = cmdStream + cmdPc; cmd = *temp++` + 各命令 `cmdPc += 2/3/4` + `0xFE 归零重播`。

| 旧 | 新 | 硬证据 |
|---|---|---|
| `field_24` | **`cmdStream`** | `u8*`, 非空=有脚本; 与 cmdPc 配对出现在 `cmdStream + cmdPc` |
| `field_17` | **`cmdPc`** | `= 0` (0xFE 归零), `+= 2/3/4` (按命令长度跳过操作数) |
| `field_E` | **`targetFacing`** | 永远 `&= 7`; `++/--` = 左转/右转; `facingDir = targetFacing` 成对; `Chara_SetPosDir` 同时写两者 |
| `field_1A` | **`z`** | `Chara_GetDrawZ` 返回它 (bit0 时叠加摄像机偏移), 作为 `Sprite_EnqueueRender` 第 4 实参 |
| `field_18` | **`subSprNodeIdx`** | `= Sprite_AllocNode()`; `&gSpriteNodePool[subSprNodeIdx]` 取 flags/next; 释放后置 0 |
| `field_12` | **`stateFlags`** | bit0 影响 z (GetDrawZ), bit4/bit5 由命令 2/0xFD 置, bit6 = 玩家移动 (D-pad), `&= 0x7B`/`&= 0x7F` 清位 |
| `field_1` | **`renderFlags`** | bit0 门控渲染 (`sprNodeIdx && (renderFlags&1)`); 三个 init 路径均置 2; 传给 EnqueueRender 第 5 参 |
| `field_2` | **`gfxSetId`** | 与 `paletteId` **同值初始化** (箭头=9 / NPC=5 / 特效=0xA), 用作瓦片基址 ×72 |
| `field_10` | **`stepTimer`** | `Chara_StepMove` 返 1 且有脚本时每帧 `++`; 命令里置 `op+1` / init 1 |

**保留匿名的 14 个及理由** (不凭猜取名): `field_A/B/C/D`、`field_F`、`field_11`、`field_13`、
`field_19` 在 live C 里**只写不读** (读者在未匹配的 asm 里); `field_14` 仅知是 u16 计时器
(`> 0xFE` 判完, 哨兵 0xFF); `field_1C/1E/20/22` 只有 `sub_804F280` 的 `strh` 写与
`CutsceneAnim_PlayFrame` 的 `ldrh` 读 (0x20), 其中 0x1C = `z << 4`、0x1E = 差值×16/表值
→ 看着像**仿射/缩放参数**, 但需先匹配 `sub_804F280` 才能定名。

**重要修正**: 上一轮我当作证据用的 `Chara_InitFromDesc` / `UnkStruct` 那段**在注释块里**
(另一个 agent 的草稿), 不是 live 代码。本轮已改用只抽 live 行的提取器重做,
上表每一条都有 live 证据。`gfxSetId` 的同值初始化证据改自 live 的
`Chara_InitDialogArrow` / NPC init / `gEffectActor` init 三处。

**尺寸复核**: `sizeof(Actor) = 0x28 (40)` 不变, `make` + SHA1 绿。

### 2026-09-01 `sub_8008CC0` → `ChoiceMenu_ResolveDest` 真 C 化 + 选项数据库语义闭环

`fncheck: OK (88 bytes @0x08008cc0, 4 池重定位已施加, 0 bl 槽忽略)`，`bytecmp` 也是 OK。
全量回归 **590/590 OK**，SHA1 绿。

**它是什么**：把“当前选项号”解析成**目的地像素坐标**，写 `gChoiceDestX`/`gChoiceDestY`。
`gChoiceDestTable @0x08087648` 是**分组变长表**：每组 `[count][count × {x,y}]`，组间无填充。
实测解出 5 组、count = 5/7/9/9/5、共 **35 个目的地**，消耗 75/76 字节（末 1 字节 0 终止）；
值域 x∈12..200 / y∈32..128 → **240×160 屏幕的像素坐标**（不是格坐标）。

**整条链现在完全清楚了**（这是本轮真正的产出，`ChoiceMenu_HandleInput` 的剩余部分因此可直接写）：
```
gChoiceDataBase (0x080876A2, 分层记录流, 0xFF 分隔)
  └─ sub_8008124 按 gChoiceGroupIdx → gChoiceSubIdx 定位
       → gChoiceListPtr / gChoiceListLen / gChoiceCursor=0
ChoiceMenu_HandleInput(keys)   R|DOWN 前扫 / L|UP 后扫, 选项字节低 nibble≤8 直用、
                               >8 则查 gChoiceGateEventFlags[lo-9] 过 EventFlags_Test, 取高 nibble
  └─ 按 A → Scene_EnterDoor → ChoiceMenu_ResolveDest(lo nibble)
       → gChoiceDestX / gChoiceDestY
       → gCameraTargetX/Y = 目的地; gActors[1].x = destX, .y = destY - 8
```
即：**这是一个“选目的地”的传送/入口菜单**，`gActors[1]` 是随行的第二名角色。

**代码生成要点**（已逐字节验证）：
- 必须写成 `skipLen = *ptr << 1` 的**先读后自增**结构，目标才是 `ldrb; lsls #0x19; lsrs #0x18` + `adds r1,#1`
- 循环内 `ptr += skipLen; i++; skipLen = *ptr<<1; ptr++;` 的**顺序不能调**
- 取项写 `ptr + (idx << 1)` 再 `ptr[0]`/`ptr[1]`，不能合并成 `ptr[idx*2]`

**修了两个自己埋的雷**：
1. 上轮生成“段2”数据符号时用了 `'gUnk_%06X' % (addr-0x08000000)`，**少了地址前导 0**，
   产出 `gUnk_087648` / `gUnk_087694`（不符 `gUnk_0808XXXX` 约定）。
   后果：我上轮“旧名已清”的 grep 检查是**假阴性** —— 它查的是 `gUnk_08087694`，
   而实际符号是 `gUnk_087694`，所以那轮声称的“`gChoiceGateEventFlags` 已改名”根本没生效。
   → 教训：**改完必须用“新名存在 + 旧名不存在”双向 grep 断言，且模式要覆盖命名变体。**
2. `src/code_8005020.c` 被 agent K 并发编辑（02:17），其新写的 `case 2:` 分支引用了
   我上轮已改掉的旧名 `gUnk_030047BC`/`gUnk_030047E0` → 树红。修法是**把他们的引用改到新名**
   （标识符级，保留其工作），而不是回退他们的代码。



### 2026-09-01 `sub_80169EC` → **`Inv_FindHeldItemOnPage`** + 清掉 `gInvPageItemIds` 的强转宏 hack

`fncheck OK (40 bytes @0x080169ec, 2 池重定位, 0 bl 槽)`；`make` + SHA1 **绿**。

**语义**（四个共用方交叉定死, 不是猜的）：返回技能/道具菜单第 `page` 页对应的道具 id,
玩家一个都没持有则返回 `0xFF`（`page > 15` 同样返回 0xFF）。

| 共用 `gUnk_0839CFAA` 的函数 | 作用 |
|---|---|
| `Inv_FindFirstHeld` | 第一个有货的页号 (返回 `i+1`) |
| `Inv_FindPrevHeld` | 从 `gSkillMenuPage - 1` 往回找有货的页号 |
| **`Inv_FindHeldItemOnPage`** | 本页的道具 id, 无货返回 0xFF |
| `sub_804DE20` | 把 16 页压缩成 `{id, count}` 列表写 `gUnk_03000D48` |
| `sub_804F050` | 反向查找: 道具 id → 页号 |

表本身 16 字节 `{0xDD..0xE4}` (连续 8 个) + `{0x19,0x1A,0x1F,0x2D..0x31}`。
按项目"证据不足就不起语义名"的约定, 只按结构命名 `gInvPageItemIds`, 不断言它到底是技能书还是别的。

**两个值得记的坑**

1. **`#define` 强转宏会改变 GCC2 的寄存器分配。** 原 `code_8010F10.c` 里用
   `#define gUnk_0839CFAA ((const u8 *)0x0839CFAA)` 绕过未注册的符号。这个写法让 GCC2 把地址当
   `const_int`, 于是表基址被分到 **r2**; 而 ROM 里是 **r0**。改成真 `extern const u8 arr[]`
   (`symbol_ref`) 后立刻逐字节命中。分数轨迹: 强转宏 20 → 真 extern **0**。
   已把这条写进 `data_805769C.h` 的表注释, 并把 `code_8010F10.c` 里**两处**旧宏一并清掉、
   `code_8044394.c` 里两处文件内 `extern` 声明收敛到头文件 —— 实测那两个已匹配函数
   (`Inv_FindFirstHeld` / `Inv_FindPrevHeld`) 换写法后仍 OK, SHA1 未红。

2. **`page` 必须就地复用存道具 id**, 不能另起一个 `u8 item` 局部。多一个 qty 就会把表基址
   挤到 r2 (同上, score 20 / 只差 4 字节)。写成 `page = table[page];` 反而既是原码又更像人话。

**工具假阴性记录**: `fndiff` 单文件编译时 `extern` 数组在字面池里是**重定位而非硬码值**,
所以对着 ROM 的硬码池会报 score 35~425 的假差异。判别法: 看指令编码是否一致
(`objdump` 比 `4804 1808 7801 ...`), 或直接用 `fncheck`(它走真实链接)。

### 2026-09-01 `sub_8016368` → **`Text_PutGlyph`**（用户给出匹配版, 我做语义分析与命名）

`fndiff score = 0` + `bytecmp OK (100B)` + `fncheck OK (100 bytes @0x08016368, 0 池重定位, 0 bl 槽)`。
至此 `ChoiceMenu_HandleInput` 这条链上的 asm-match 被调函数**全部转成真 C**。

**语义（靠三处交叉证据定死, 不是猜的）**

| 结论 | 证据 |
|---|---|
| 目标是**窗口/菜单瓦片图**, 行距 32 个 u16 | `Text_TileAt()` = `(u16*)0x2005800 + y*32 + x`;`Text_ClearRect()` 用 `temp_buf + 0x20` 换行 |
| 一个字占**上下两格** = 8×16 字形 | 本函数写 `tilemap[0]` 与 `tilemap + 0x20`, 而 0x20 正是下一行同列 |
| 写入值是标准 BG 图块项 | `(palette << 12) + tileId` —— 位 0-9 瓦片号 / 位 12-15 调色板号 |
| `charCode == 0` 是**空白字形**(瓦片 1) | `Text_ClearRect` 手写的 `0xB001` 恰等于 `Text_PutGlyph(p, 0, 0xB)` 的结果 |
| `0x280` 是**扩展字模块基址** | 并列证据: `Text_WriteChars` 用 `+ 0x200` 作 8×8 块基址; 基本块 `2*code` 最大 506 < 640 不重叠 |
| 调色板参数 = 文字配色 | `MenuUi_DrawItemList` 光标行走 `0xD`、普通行走 `0xB` |

**命名**：`Text_` 家族已有 `Text_ClearRect` / `Text_DrawChar` / `Text_TileAt` / `Text_WriteChars` /
`Text_FillHidden` / `Text_WriteOrClear`, 本函数是它们共同的最底层原语。选 **`Text_PutGlyph`**
而非 `Text_DrawGlyph` —— 因为旁边就站着一个 `Text_DrawChar`(它其实是"画 8 字名字串"的高层函数),
`Draw` 会撞车。参数 `tilemap` / `charCode` / `palette`, 局部 `palAttr`(调色板位) / `tileId`(瓦片号)
—— 原名 `tm_entry_h` / `tm_entry_l` 是误导的: 两个值不是高/低半, 而是**属性位与索引**。

**"更像人写"的实测边界**（逐条隔离, 每条都跑 fndiff）

免费的: 改名 ✓ / `0xFF & x` → `x & 0xFF` ✓ / `<< 0xC` → `<< 12` ✓ / `char_code << 1` →
`charCode * GLYPH_TILES` ✓ / 加 `#define` 常量 ✓
**不免费的**（承重结构, 已在函数头注释里逐条标注实测分数）:
- `if/else if/else` 平铺 → 905（必须保留嵌套, 目标的空白字形体在末尾是远跳）
- `palette << 12` 外提 → 2610（必须每分支各写一遍）
- `tilemap[0]` / `tilemap[32]` 下标 → 1610（必须 `*p=` + `p += 0x20`）
- `>> 7` 改 `>> 8 << 1` 或 `* 2` → 5（GCC2 移位域折叠）
- 底格 `+= 1` 与 `|= 1` 统一写法 → 破坏（目标一个 adds 一个 orrs）

顺带发现并修掉一个**用户给的匹配版里的真 bug**：把 `((code & 0xFF00) >> 7) + 0x280`
"美化"成 `((...) >> 7) * 2 + 0x280` 会把扩展字模索引翻倍（`>>7` 已经是 `2h`）。
它只报 score 5 而非语义错误, 靠肉眼才看得出来 —— 已在新版里去掉。

**工具修复**：`scripts/fndiff.sh` 原来按**参考名**在候选 `.o` 里找符号, 一旦候选已改名就查不到,
diff.py 输出空 → 报 `CURRENT (5000)`, 看起来像"完全不匹配", 实际是符号名对不上。
现在会自动取候选对象里唯一的全局 `T` 符号、`objcopy --redefine-sym` 临时改回参考名再比,
并打印 `(候选对象里符号叫 X, 已临时改回 Y 比对)`。回归验证: 未改名候选与 `Msg_BuildSegmentIndex` 仍为 0。

### 2026-09-01 `ChoiceMenu_HandleInput` 的剩余 asm-match 被调函数: 匹配了 1 个, 1 个待定

用户要求“先匹配 sub_8008254 内未匹配的子函数”。先把候选范围定清楚：
`ChoiceMenu_HandleInput` 的 6 个被调函数里, `Chara_SetCmdPtr`/`Chara_StartMoving`/
`EventFlags_Test`/`sub_8008124` **已是真 C**, 只剩两个仍是 `INCLUDE_ASM`：

| 函数 | 行数 | 结果 |
|---|---|---|
| `sub_80164C0` | 32 | ✅ **已匹配并转真 C** → `Msg_BuildSegmentIndex` |
| `sub_8016368` | 56 | ⚠ 未收敛 (最好 score 2330), 仍是 INCLUDE_ASM |

**`Msg_BuildSegmentIndex` @0x080164C0** —— `fndiff score = 0` + `bytecmp OK (56B)`
+ `fncheck OK (56 bytes, 0 池重定位, 0 bl 槽)`。它给主文本池 `0x080936A0`（6536 B, 0xFF 分隔）
建**每 64 段一个跳转项**的索引写到 `0x030001D0`（64 × u32 = 256 B）。

三个必须踩中的代码生成点（每个都花了一轮实测）：
1. `n` 必须是 **u32/int** —— 写成 u16 会给 `n++` 加上 `lsls/lsrs #0x10` 截断（695 → 270）
2. 首次写入必须写成 `*(u32 *)0x030001D0 = p;` 而**不能**复用 `index` 变量 ——
   否则 GCC2 把池载入直接落到 r3, 少掉目标的 `ldr r0; str [r0]; adds r3,r0,#0` 基址复制（270 → 60）
3. `n = 0;` 必须**夹在**首次写入与 `index = (u32 *)0x030001D0;` 之间 —— 决定那条 `adds` 的位置（60 → 0）

**`sub_8016368` 未收敛的原因**（已定位, 待后续）：它是“写一个 8×16 字形 = 两个瓦片
(`dest[0]` 与 `dest[32]`)”的底层写入器, 被 4+ 处调用。三个分支各自内联重算
`palette << 12`（不能提到顶部, 提到顶部反而从 2435 变 2610）；转义分支的 tile 是
`((code & 0xFF00) << 9) + 0x2800000` 再 `>> 16` 的**移位域**形式（经验 30）。
当前最好 2330, 差异集中在 `code` 为何同时住在 r1 与 r5 两个寄存器（目标在入口就
`adds r5, r1, #0` 复制）—— 属于 progress.md “寄存器 home/排列墙” 那一类。

**本轮同时匹配了 `Msg_DrawPoolSegment`**（原 `sub_8016460`, fncheck OK 96B）——
它才是 `ChoiceMenu_HandleInput` 直接依赖的那个串渲染器。关键规律：`*p` 必须写**三次**
（循环测 / ==0xFE 测 / 实参）且 `dest++` 写在实参位置, 提成局部变量会少一条 ldrb。

**并发状态**：`src/code_8005020.c` 在 02:38:18（35 秒前）被 agent `plan` 改动,
正在转 `sub_8009A7C`/`sub_8009AC4` 但还没升级 `code_0.h` 原型 → 全量 `make` 暂时红。
**不是本次改动引入的**, 已用 `make -B build/src/code_8010F10.o` + `fncheck` 独立验证本步四个函数均 OK。
我没有去改他们的半成品（铁律 1/3）。



### 2026-09-01 `sub_8008254` 尝试匹配: 未收敛, 但语义已全解 + 命名已落盘

**匹配结果：未完成**（第一版候选 `fndiff score = 40640`，只覆盖约 25%）。
函数 474 行 asm / 948 B，剩下三段各需先逆清一张 ROM 表的格式。
**因为项目铁律要求每步 SHA1 绿，部分匹配不可提交** → 保留 `INCLUDE_ASM`，
把语义分析与命名落盘（这才是让实际匹配可行的前置）。

**定名**：`sub_8008254` → **`ChoiceMenu_HandleInput`**（分层选项数据库的光标输入处理器）。

**语义链（由已匹配的 `sub_8008124` 反推完全闭合）**：
```
gChoiceDataBase @0x080876A2  = 分层记录流 (0xFF 分隔)
  ├─ gChoiceGroupIdx (0x47BC) 跳过 N 组“双 0xFF 字段”记录
  └─ gChoiceSubIdx   (0x47E0) 再跳过 M 组“单 0xFF 字段”记录
       → gChoiceListPtr (0x462C) = 选项列表起始
         gChoiceListLen (0x4640) = 列表长度 (到 0xFF 为止的字节数)
         gChoiceCursor  (0x466C) = 光标, 重置 0
```
`ChoiceMenu_HandleInput(keys)`：
- `gActors[0].stateFlags & 0x80` → 玩家移动中则直接返回
- `keys & 0x50`（R|DOWN）前向扫描 / `keys & 0xA0`（L|UP）后向扫描（到 0 回绕到 len-1）
- **选项字节打包**：低 nibble ≤ 8 → 直接就是选项值；> 8 → 是事件门控项，
  查 `gChoiceGateEventFlags[(lo-9)]`（u16[7] @0x08087694）过 `EventFlags_Test`，
  未触发则跳过；触发则取**高 nibble** 作为值
- 光标变了 → `Sfx_Play(0,0,0)` + 写 `gChoiceSel`，再遍历 `gUnk_08087648`（变长表：
  `[count][count*2 字节]`）取第 `gChoiceSel` 项 → 写 `gUnk_03004824`/`gUnk_030047B8`
  与 `gActors[0].field_2E/field_30`
- 光标没变且按了 A（bit0）→ 从 `gUnk_080882E2`（8 字节记录、0xFF 终止）取
  `record[gChoiceSubIdx]` 的 5 个字段写入 `gMapNpcSetId`/`gUnk_0300468C`/`gSpawnTileY`
  /`gSpawnFacingDir`/`gMoveCmdSetId`(u16=字节4|字节5<<8)，然后
  `Chara_SetCmdPtr(0, gCharaCmdStreams[idx])` + `Chara_StartMoving(0)` +
  `gBlendCoefficients = 0x1F00` + `gMainGameState = 0xA`
- 共用特例：`if (gChoiceGroupIdx == 0 && packedPair == 1)` 走另一分支（就是 `gUnk_0808823A` 项 0 的唯一用处）
- 尾部：清 `0x02005C84[0..15]` 与 `0x02005CC4[0..15]` 两个 u16 数组，再
  `sub_8016460(0x02005C82, record[...], 11)`

**本轮新命名的符号**（全部标识符级，codegen 中性，SHA1 保持绿）：

| 旧 | 新 | 地址 |
|---|---|---|
| `gUnk_0300462C` | `gChoiceListPtr` | 0x0300462C |
| `gUnk_03004640` | `gChoiceListLen` | 0x03004640 |
| `gUnk_0300466C` | `gChoiceCursor` | 0x0300466C |
| `gUnk_0300469C`（原本**未注册**） | `gChoiceSel` | 0x0300469C |
| `gUnk_030047BC` | `gChoiceGroupIdx` | 0x030047BC |
| `gUnk_030047E0` | `gChoiceSubIdx` | 0x030047E0 |
| `gUnk_08087694` | `gChoiceGateEventFlags` | 0x08087694 (u16[7]) |
| `gUnk_080876A2` | `gChoiceDataBase` | 0x080876A2 |

**踩坑（已记）**：改未匹配函数名同样必须跑完整四载体管线 + 重生成 code.s + split_asm;
漏了重生成那步会报 `can't open asm/nonmatchings/<新名>.s`，而且会先以“C 语法错”的
假象出现（cascade 自上一个失败步骤），容易误判成改名改坏了代码生成。

**下一步（如果要完成匹配）**：先逆清 `gUnk_08087648`（76 B 变长表）与
`gUnk_080882E2`（286 B 记录表）的确切格式 —— 这两张表不弄清就没法写出正确的
确认分支；建议先匹配 `sub_8008124` 的调用方（`Scene_EnterDoor`）来定位记录语义。



### 2026-09-01 `gUnk_0808823A` / `gUnk_0808823B` 分析 + 修正上轮的对齐截断错误

**先说结论：`gUnk_0808823B` 不存在**（data.json 的 `byte_808823B` 是它的分块产物），
而 `gUnk_0808823A` 我上一轮当成"2 字节对齐填充"是**错的** —— 它是一个 84 项表的项 0。

**真实结构**：`0x0808823A..0x080882E2` = **84 项 × 2B**，每项 = (组号, 有序数字对)：

| 组 | 项数 | 无序对数 | 内容 |
|---|---|---|---|
| 0x00 | 8 | 4 | 01 10 \| 23 32 \| 24 42 \| 34 43 |
| 0x01 | 22 | 11 | 01 10 \| 12 21 \| 23 32 \| 45 54 \| 02 20 \| 03 30 \| 13 31 \| 06 60 \| 16 61 \| 26 62 \| 36 63 |
| 0x02 | 26 | 13 | 01 10 \| 12 21 \| 13 31 \| 14 41 \| 15 51 \| 16 61 \| 24 42 \| 25 52 \| 26 62 \| 45 54 \| 46 64 \| 56 65 \| 78 87 |
| 0x03 | 20 | 10 | 01 10 \| 02 20 \| 03 30 \| 12 21 \| 13 31 \| 23 32 \| 45 54 \| 67 76 \| 68 86 \| 78 87 |
| 0x04 | 8 | 4 | **与组 0x00 完全相同** |

合计 42 个无序对，数字范围 0..8，每个无序对都以两个方向各出现一次 → **有向边表**形状。

**引用情况（全 ROM + 全数据段扫过）**：
- 代码里只有 **1 处**：`0x08008510` 的字面池 = `0x0808823A`，在 `sub_8008254` 里读 `[r2]`/`[r2,#1]`，
  即**只用项 0** 做特例判断：`if ([0x030047BC] == 0 && packedPair == 1)`，
  其中 `packedPair = ((x & mask) << 4) | y` —— 与"有序数字对"的编码完全一致（这是命名依据）。
- 项 1..83 **无任何按地址引用**；数据段里也 **0 个指针**指向本表区间。
- 假设（未证实）：5 组 = 5 个区域，每组是区域间连通关系的有向边表 → 需先匹配 `sub_8008254`。
  所以**没给语义名**，只保留 `gUnk_0808823A` + 完整结构注释。

**做的修正**：把项 0 截断的 `gUnk_0808823A = {0x00,0x01}` 换成完整 168 B 表。
但 `0x0808823A + 168 = 0x080882E2`，**非 4 对齐** → 又踩一次 SUBALIGN 补位（SHA1 失败）。
解法：把紧邻的 `byte_80882E2`（286 B，被 `sub_8008254` 当表基址引用 4 次）一并搬出，
它结束于 **0x08088400（4 对齐）** → blob 起点移到 0x08088400，绿。
`gUnk_080882E2` 前几条看着像 `{u8 a,b,c,d; u32 val}` 记录（val = 1,2,22,132,5,6148…），
但 286 不是 8 的倍数且 val 后期不单调 → **不拆**，整体搬出作不透明块。

**累计去 blob**：`data.s` 6,453,768 → 6,450,708（−3572 B）；`data1.s` 92,844 → 92,136（−708 B）；
共 **4280 B** 变成真 C 符号。`fncheck 589/589 OK`。

**教训（已补进 PLAN_DATA §1）**：对齐约束不仅适用于"切块边界"，也适用于
**“一个逻辑对象能不能单独搬出来”** —— 如果它的长度 mod 4 ≠ 0 且后面紧跟的也不是 4 对齐，
就必须连带后续项一起搬到一个 4 对齐边界，或者反过来把它留在 blob 里。



### 2026-09-01 `gUnk_080871EA` → `gScrollEaseDeltas` + 视口滚动系统命名

**唯一引用者** = `sub_8005C70`（957 行 asm, 未匹配）→ 已改名 **`Viewport_UpdateScroll`**。
它的被调函数**全都已匹配**（只有 `__divsi3` 与 `[1]` 的 `sub_80086FC`），所以没有可顺带匹配的函数。

**`gScrollEaseDeltas` @0x080871EA** —— 22 项 u16（按 s16 读）的**逐帧缓动增量表**：
```
0, -6, 0, 0, +16, +16, +16, +8, +8, +6, +6, +6, +6, +4, +4, +4, +4, +4, +4, +4, +8, +8
```
用法（jump table case 3，即 `gCameraDrawMode == 4`）：
```c
if (gDrawCamEaseActive) {
    gDrawCamX++;                              // 兼作帧计数器
    gDrawCamY += gScrollEaseDeltas[gDrawCamX]; // 累加到绘制 Y
    if (gDrawCamX > 21) gDrawCamEaseActive = 0;   // 22 项刚好走完
}
```
增量从 ±16 逐帧衰减到 4 再回到 8 → **减速曲线**，项数与 `cmp #0x15`(21) 完全吻合。

**`Viewport_UpdateScroll` 语义地图**（触及 25 个 IWRAM 全局，本轮命名了 7 个）：

| 地址 | 新名 | 依据 |
|---|---|---|
| 0x0300464C | `gCameraMinY` | `gCameraPosY = gCameraTargetY - 0x50` 后被夹到 `[本值, gMapHeightPx-160]` |
| 0x03004650 | `gCameraMinX` | 同上，X 轴：`[本值, gMapWidthPx-240]` |
| 0x03004680 | `gDrawCamEaseActive` | 缓动进行中标志，计数器过 21 时清零 |
| 0x03004684 | `gDrawCamY` | `Chara_GetDrawY` 用 `>>4` / `-0x20` / `-gCameraPosY` 三种方式读它 |
| 0x030047C0 | `gDrawCamX` | `Chara_GetDrawX` 用 `-256` / `-gCameraPosX` 读它；缓动时兼作帧计数器 |
| 0x030047C4 | `gMapWidthPx` | 上界 = 本值 `- 0xF0`(240 = 屏宽) |
| 0x030047EC | `gMapHeightPx` | 上界 = 本值 `- 0xA0`(160 = 屏高) |

结构：`gCameraSnapFlag != 0` 时走“吸附+缓动”分支（用 `gUnk_030047B4` 作总步数、
`gUnk_03004844` 作当前步，`__divsi3` 做线性插值），否则直接把 `gCameraTargetX/Y - (112,80)`
夹到地图边界。后半是一个 8 case 跳转表（按 `gCameraDrawMode-1`）重算
`gBG2ScrollX/Y`、`gBG3ScrollX/Y` 的 **4 位小数部分**（全都 `& 0xF` → 16 分像素/格），
末尾按 `gViewportFlags` 的 bit0/bit1 叠加 `0x03004802`/`0x03004804` 两个增量。

**未命名保留**：0x0300461C/0x03004630/0x030047DC/0x03004830（插值的起/止坐标对）、
0x03004664、0x03004844、`gUnk_030047B4` —— 它们只在未匹配的插值路径里出现，
需先匹配 `Viewport_UpdateScroll` 才能定名。

**本轮踩的坑（都是"新符号注册"的坑，已记）**：
1. 往 `linker.ld` 插新条目时按地址序找插入点，**必须限定在 iwram 段内搜** ——
   第一次插到了 ewram 段（会变成 0x0200464C）。
2. 找插入点的正则不能以 `\.` 结尾：现有行有**行尾空格**（如 `gHBlankEffectMode = .;    `），
   会整行匹配不上 → StopIteration。
3. 往 `iwram.h` 加 extern 前先查是否**已有声明**（`gUnk_030047B4` 在 667 行已是 `u8`），
   重复声明不同型 = `conflicting types`。
4. 改未匹配函数名同样要走完整管线（ll.cfg + 头文件 + src + 重生成 code.s + gen_asm.py --sync；
   TSV 按 addr 键控免改），否则报 `can't open asm/nonmatchings/<旧名>.s`。见 AGENTS.md §7。



### 2026-09-01 去 blob 首例: 0x0808760C 区段 → 真 C + 指针表重定位化

**分析结果**：`0x0808760C` 是 `data/data.s` 里 `rom_data` 巨块（6.45 MB）的起点，
`code.s` 对它**零直接引用** —— 它靠两张指针表间接可达：

| 表 | 地址 | 项数 | 消费者 | 目标语义 |
|---|---|---|---|---|
| `off_87E9554` | 0x087E9554 | 88 | `sub_8008620`(未匹配) | LZ77 资源（已在 `data_805769C.c` 里） |
| `off_87E96B4` | 0x087E96B4 | 5 | `TextBlocks_Render` | **队伍成员名字文本块** |
| `off_87E96C8` | 0x087E96C8 | 84 | `sub_8008254` → `Chara_SetCmdPtr` | **NPC 行为命令流** |

用 `charmap.txt` 解码表1 → **ホンメル / カタリナ / マリウス / スタジウス**（角色名）。
表2 用 charmap 会解出假名，**那是巧合** —— 它是 4 字节一组的命令流
（`[cmd][op1][op2][op3]`，`0xFF` 结尾；cmd=0 走 `Chara_ProcessCmdStream` 的 default，
`op1 & 7` = 8 方向）。差点误命名成"文本"。

**搬出来的内容**（共 3828 B 从 blob 变成真 C 符号）：
- `src/data_805769C.c` += 0x0808760C..0x0808823C（3120 B）：
  `gCharNameTextBlock_{Homel,Catarina,Marius,Stadjus,4}` + 3 项未定性数据
  + `gCharaCmdStream_87742..88226`（84 个）+ 2 B 对齐填充
- `src/data_87E83F0.c` += 0x087E9554..0x087E9818（708 B）：三张表改成
  `(u32)&目标符号` 形式 → **177/177 表项变成真重定位**，不再硬写地址
- `data/data.s` blob 起点 0x8760C → **0x8823C**；`data/data1.s` 起点 0x7E9554 → **0x7E9818**
- `linker.ld` 删掉绝对符号 `gUnk_080876A2 = 0x080876A2;`（现在是真定义了）
- `include/data_805769C.h` += 179 个 extern

**两个踩坑（都是 PLAN_DATA §1 预言的对齐问题）**：
1. **按表序发射 → 地址乱掉**。`off_87E96C8` 的项**不是地址序**的（[3]=0x08087792 而 [4]=0x0808777e）。
   `.rodata` 跟随**声明顺序**排布，所以必须按地址序发射，表索引只能写注释。症状：152 字节差异。
2. **搬出区结束于非 4 对齐地址 → 全局位移**。0x0808823A 非 4 对齐，`SUBALIGN(4)` 给下一个
   输入段补 2 字节 → **7.19 MB 差异**。修法：把填充 2 字节一起搬出来（`gUnk_0808823A = {0x00,0x01}`）。
   → 已补进 PLAN_DATA §1：**切块边界必须选在 4 对齐的 item 边界上**。

另：`gUnk_080876A2` 不能定义成非 const（会掉进 `.data` 破坏布局），定义成 `const u8` 并
**不动** `code_8005020.c` 里那个 `extern u8` 声明 —— 跳 C 文件 的 const 不一致链接期不检查，
且那边代码生成不变。实测 SHA1 绿。



### 2026-09-01 工具/构建缺陷修复批次

四个真实缺陷，全部修完并回归验证：

**① `fncheck.py` 对 asm-match 函数误报 `NOT BUILT`**
根因: `thumb_func_start` 宏不发 `.size` 属性 → 符号表里 size==0, 而旧代码用 `if size:` 直接拒。
修: size==0 时用同 section 下一个 FUNC 的起点定长; 本节最后一个则用节大小兑底
(注意 readelf -SW 要取**第 3 个十六进制列**才是 Size, 第一次错取 Offset 导致 size 为负 → 空 blob **假通过**,
比 NOT FOUND 更危险)。
附带修: `R_ARM_ABS32` 指向**段符号 `.text`** 时报未解析 → 用 `函数ROM地址 - 它在节内偏移` 推段基址
(不取 ll.map, 避开布局漂移)。再加对象级缓存。
回归: **588/588 已匹配函数全部可验, 0 个假 FAIL** (之前只能验真 C 的那部分)。

**② Makefile 不跟踪 `INCLUDE_ASM` 展开出的 `.include` 依赖**
`$(C_BUILDDIR)/%.o: $(C_SUBDIR)/%.c` 看不到 `.include "asm/matchings/<func>.s"`,
所以改 ll.cfg 函数名 + 重切 asm/ 后, 引用方 C 文件 的 .o **不重编** → 链接期 undefined reference 旧名。
修: 解析期用 grep 把依赖补上 (`ADD_ASM_DEPS`), 并用 `$(wildcard)` 过滤掉注释里已不存在的 .s。
踩过的坑: sed 捕获组含了 `asm/` 前缀 → 产出 `asm/asm/...` 被 filter 全清, 规则等于没加;
靠 `make -pn | grep '^build/src/.*\.o:'` 数依赖条数 + `touch` 一个 .s 看是否重编才确认生效。

**③ `functions.yaml` / `ll.cfg` 有 7 个陈旧键 (split_asm 一直在报 `Skipping unknown function`)**
`sub_800065C`→`VBlankIntr`、`sub_800121C`→`ReadKeys`、`AddInventoryItem`/`RemoveInventoryItem`
(真 C 定义在 `code_8005020.c`, 名字是 `sub_800AA60`/`sub_800AA84`, yaml 里挂错了模块)。
`nullsub_3/4/6` 则是 **ll.cfg 陈旧**: 真 C 已叫 `DummyIntr3/4/5`。
修完 split_asm 零报错; 改法用逐行 sed/插入不重排 yaml (避开共享文件重排风暴)。
踩过的坑: 插入时没发现 `sub_800AA60/AA84` 已存在于目标模块 → 造出重复键;
而 `yaml.safe_load` 会**静默后写覆盖**, 用“重复键: 无”误导了我一次 ——
查重复必须用文本级 `grep -oE '^  \w+:' | sort | uniq -d`。

**④ 函数清单漂移**: `audit.py --fix` 校正 `sub_8014488` 一行, 现在 0 漂移。

**另记**: 本轮观察到 `build/src` 在两次命令之间被清空过 (16 个 .o → 0), 说明仍有并发进程在跑 clean。
→ 验证与构建必须写在**同一条命令**里, 否则 fncheck 会因对象缺失而全体误报。
根治方案仍是 REFACTOR_PLAN 待拍板(worktree) (每人一个 git worktree, `build/` 独立)。



### 2026-09-01 `sub_800BF5C` → `PartyUi_InitEntities` + 两个构建级发现

`fncheck: OK (156 bytes @0x0800bf5c, 3 池重定位已施加, 1 bl 槽忽略)`，`make` + SHA1 绿。

**功能**: 重建 HUD 队伍精灵实体表 `gUnk_03000058` (UISpriteEntity[15], 步长 0x14)。
前 5 项 = 队伍成员 (若 `gPartyMemberIds[i] != 0xFF`): `x = i*40+0x48`、`y = 8`、
`statusFlags = 0x80`、`field_10 = i*48+0x200` (基础图块起始 ID)、`oamSlotId = i+0x71`；
其余/空槽清零; 15 项都重置 `animTimer`/`lerpFrame`。`mode == 0` 时额外
`sub_800EB98(0)` + `实体[5].statusFlags |= 8`。调用点: sub_800ACC8 传 0, sub_801417C 传 1。

**草稿的两处重复发明** (已改回现有符号): `gPartyCharacterIds` → 实际已存在 `gPartyMemberIds`;
`gUISpriteEntities` → 实际已存在 `gUnk_03000058`。另外草稿用 `gUnk_030000BC` 符号,
**必须改成 `gUnk_03000058[5]`**: 目标是 `ldr r0,=0x03000058; adds r0,#0x64` (复用同一池项),
用独立符号会多一个字面池项 (实测多 20 字节)。

### 构建级发现 1: Makefile 不跟踪 `.include` 的 asm 依赖 → 改名后 .o 不重编

`$(C_BUILDDIR)/%.o: $(C_SUBDIR)/%.c` 只依赖 .c, 而 `INCLUDE_ASM` 展开成
`.include "asm/matchings/<func>.s"`。所以 **ll.cfg 改名 + 重生成 code.s + split_asm 后**,
引用该函数的其它 C 文件 的 .o **不会重编**, 链接期报 `undefined reference to 旧名`。
本轮实际踩到 (code_8044394.o / code_804F0B8.o)。
→ **改名流程必须加一步 `touch src/*.c`** (已补进 EXPERIENCE.md 管线)。

### 构建级发现 2: 另一个 agent 的半成品改名 (只改了 ll.cfg)

`sub_8008788` 在 `ll.cfg` 已被改成 `IntroBg_Load`, 但 `functions.yaml` / `code_0.h` /
`src` 的 INCLUDE_ASM 行都没跟上 → `split_asm.py` 在 code.s 里找不到旧名 →
不生成 `asm/matchings/*.s` → 汇编报 `can't open asm/matchings/sub_8008788.s`。
正是本轮写进 EXPERIENCE.md 的"名字有三个独立载体, 漏一个就挂"。已补全四个载体。

### 本轮自己引入又修好的回归

往 `include/iwram.h` 里合并重复 typedef (`Unk_03000058` → `UISpriteEntity`, 两者字段完全相同)
时, 误以为"类型名只在头里用"—— 因为 `grep ... | head` 把结果截断了。
实际 `src/code_8010F10.c:671/700` 有 `Unk_03000058 *p;` 两处 → 编译报 undeclared。
已改为 `UISpriteEntity *p;` (纯类型名替换, codegen 中性)。
**教训**: 判定"某标识符还有哪些引用点"时不能带 `head` 截断, 必须看全量。



### 2026-09-01 `sub_800BEE4` → `Msg_RenderLine` (INCLUDE_ASM 转真 C)

该函数原本已是 asm-match (`functions.yaml` = [1])，本轮把旁边的**注释草稿转正**:
`fndiff score = 0` (指令逐条全等)，`fncheck: OK (120 bytes @0x0800bee4, 1 池重定位, 4 bl 槽忽略)`，
`make` + SHA1 绿。进度仍计 590/1067 (本来就算匹配)，但真 C 化才是难点。

**功能**: 把一条字节编码消息解码成一整行瓦片写入 `gMsgLineBuf` (u16[29] @0x02005C44)。
`0xFF` 结束、`0xFE` 转义前缀 (后跟高位字节 → `(hi<<8)|0xFE`)、其它直接当 16 位码；
输出 = `[0xC8 左边框] [N 个内容码] [0xC9 右边框] [0 补齐]`，共 29 项 (count 到 0x1C)。
边框/补齐固定用调色板 0xB，内容用调用者传的 palette。
调用者 `Msg_ShowById` 先在 `gMsgTable` 块里跳过 target 个 0xFF 定位消息再交给本函数。

**新注册符号**: `gMsgLineBuf` (linker.ld ewram 段按地址序插在 gWindowBgBuf 之后 + ewram.h)。
实测字面量 `0x02005C44` 与命名符号两种写法**字节完全一致** (差异只在 4 个 bl 槽)。

**草稿本来就对的四处关键写法** (值得记住):
- `count` 是 **u16** → 自增生成 `adds; lsls #0x10; lsrs #0x10` (u16 截断，经验 68 同类)
- 转义分支用 `|` 不用 `+` → 目标是 `orrs r1, r0` (经验 36)
- 两个分支**各自重复**一次 `sub_8016368(dst++, ch, palette)`，不能外提 (经验 38)
- `dst++` 在目标里是 `adds r0,r5,#0; adds r5,#2` (先传后推)

**踩坑**: 原型 `void f();` 与新定义 default-promotion 冲突 (同 sub_8020974/Stats_BuildSkillList)，
升为 `void (u8 *, u8)` 后 SHA1 仍绿 —— 调用点实参本来就是 u8，没多出截断。

### 2026-09-01 匹配 `Stats_BuildSkillList` (原 sub_800A048, 89 行 asm)

`fncheck: OK (156 bytes @0x0800a048, 2 池重定位已施加, 0 bl 槽忽略)`, `make` + SHA1 绿。

**功能**: 从 `gUnk_08093418` (48 项 × 5B) 筛出行, 把**行号+1** 填进 `PlayerStats.skills[8]`,
不足 8 个用 0xFF 补齐。入选条件: `[i*5+1]>>4 == groupId` (`groupId<=1` 归 0), 且
(`[i*5]==0xFF && gPartyMemberIds[0]==1`) 或 (`[i*5] != 0xFF && [i*5] <= lv+1`)。
4 个调用点: `Chara_ClearTempStatus` / `sub_800A1B4` / `sub_800A3C8` / `sub_80457AC`。

**两个新规律 (已入 EXPERIENCE 经验 100/101)**:
- **禁用 goto 时, 把分支归约成对同一个 flag 赋值, 让 GCC2 jump-threading 自己生成绕过块**。
  ROM 里 `pid==1` 那条路 `b _ACCEPT` 直接绕过 `cmp r5,#0`, 看上去必须 goto;
  实测 `goto` 写法残留 13 字节差且长度不对, 而 `if (pid==1) flag = 1;` 写法 **0 字节差**。
- **u8 形参上的 `+1` 生成移位域加法**: `movs r0,#0x80; lsls r0,r0,#0x11` = **1<<24**,
  然后 `adds r1,r1,r0; lsrs r1,r1,#0x18`。看到这条序列就知道是 `(u8)(param + 1)`。

**本轮走过的坑**:
- 先写了 `want = arg2;` 引入多余局部量 → 多占一个 callee-saved (`push {r5,r6,r7}` vs `push {r6,r7}`)
  呷对。**直接改写形参 `arg2` 本身**才能复现 `lsrs r7,r2,#0x18` 把形参归到 r7。
- `pid != 1` 写成 `continue` → ROM 是**落到 flag 检查块**再 `beq continue`, 字节不同。
- 给表改名 `gSkillTable` 后发现同一张表已被别人的真 C (`ItemFindSlot`/`ItemGetValue`)
  以"按 id 相等查"的方式使用, 与本函数"按等级 ≤ 查"矛盾 → **语义未定, 已回退表名**,
  只在注释里记录实测布局与矛盾点。教训: 给共享数据改名前必须先扫全部现有引用者。
- 本函数用 r8/sb → 有经验 51 泄漏风险; SHA1 保持绿 = 无泄漏 (已验证)。
- 原型 `void f();` 与新定义冲突 (default promotion), 按经验 44 升为 `void (u8 *, u8, u8)`;
  同时发现同一 C 文件 里已有 `extern u8 gUnk_08093418[];` (非 const), 必须用**完全一致的声明**
  避免 conflicting types (实测 const/非const 对字节无影响, 但重复声明必须同型)。



## 提示

### code_1.c 短函数批处理（2026-08-31）

新增 `scripts/auto_match_code1.py`，按 `functions.yaml` 自动筛选 `code_1.c` 中
汇编文件少于 80 行的 `[0]` 函数，顺序执行 `fncheck` / `fndiff`，失败或 m2c
无法转换时写入 `.scratch/auto_match_code1/status.tsv` 并继续下一个，不会把失败
候选合入源码。运行 `.venv/bin/python scripts/auto_match_code1.py --apply` 后，
以下 4 个原本已有真 C 且字节级通过的函数已更新为 `[1]`：

* `sub_8016978`（32 行；后续人工复核：`fncheck` 50 字节零差异，`make` + SHA1 通过）
* `sub_80169AC`（38 行）
* `sub_801A0F0`（42 行）
* `sub_801A1DC`（31 行）

其余 39 个目标均已自动尝试；`sub_8020B54` 最佳候选 score=30，
`sub_80175C0` score=405；`sub_801A684` 当时 score=540，后续已由经验 83 收尾；其他目标因编译错误或
m2c 生成 `goto`/`M2C_ERROR` 暂挂。批处理不会使用 `register ... asm("rN")`，
也不会生成 `goto` 代码。

- **0x8052 / 0x8053 script 处理器族模板**（本簇已连中三个，直接套用）:
  ```c
  u32 sub_805XXXX(u32 *ptr)
  {
      u8 *data;

      data = (u8 *)*ptr;
      /* 可选: 一个无参调用 sub_8009B44() —— data 会落 r4 */
      /* 分发: if (data[1] == 0xFF) ... 或 switch (data[1]) { case 0/1/2 } */
      /* 16 位实参: data[2] + (data[3] << 8)  或  data[2] | (data[3] << 8) —— 照抄目标助记符 */
      *ptr += N;      /* N = 指令字节长度 */
      return K;       /* 0 或 1, 照抄目标 */
  }
  ```
  要点: 调用后的 `*ptr += N` 一定重读（写 `data += N` 不匹配）；
  目标里每个分支重复的 `ldr/movs/strb` 不要外提；≤60 行直接手读反汇编比 m2c 快。
- 0x8020 区域函数簇共享 `0xB0`(u16 状态位)/`0xBE`(u8 类型) 字段结构, 步长 0xC8,
  遇到同字段访问可直接套用已有写法
- 下一个候选: 0x8052 簇剩余 (sub_8052AE8 41 / sub_8052878 42 / sub_8052808 43 / sub_8052CF0 45
  / sub_80528C8 47 / sub_8052EC0 50 / sub_8052580 50 / sub_80529B8 51) ——
  **本簇已连中 5/5, 且 sub_8052D4C/DCC 证明同构函数可直接 sed 改名复用, 优先成对处理**
- 注意: code_1.c / code_1b.c 是独立编译单元, GCC2 泄漏被隔断; 新函数用 r8/sb/sl 时
  若 SHA1 挂且差异在别的函数 → 继续拆文件
| sub_8003958 | 0x08003958 | MOD-01 | code_80002A0.c | ⏸ 挂起 (10270分, 候选已固化 permuter/sub_8003958/base.c) | 角色寻路指令写入器: `void(u8 idx, u8 tx, u8 ty)` — 把最多 2 条 4 字节移动命令写入 `gUnk_0203EE00[idx*9]` (9B/角色: cmd{0,dir,1,amt}×2 + 0xFF 终止符), 并把指针存入 `CharacterObject.field_24`。tx/ty==0xFF 时参考 `gUnk_03002E80[0]`(主角!) 的 x/y; 否则 tx*8/ty*8。dx=obj->x-refx(u16 回绕), 负值取 `~dx+1`(实测 `-dx` 生成 negs, 必须 `~x+1` 才是 mvns+adds); |dx|>=|dy| 先写 X(dir: dx<0→2, ≥0→6) 后写 Y(dy<0→4, ≥0→0), 反之先 Y 后 X。**已破解**: ① 第一条命令是字面量地址写 `*(u8*)(0x0203EE01+off)=dir` (pool 0x0203EE01/02 为证, 数组下标 `gUnk_0203EE00[off+1]` 会先算 off+1 得两步 adds, 不折叠); ② 第二条命令走 ptr 变量 (目标 `ldr r1,[sp]` 步进式, ptr 溢出到 [sp]); ③ 分支极性 `if (n1 >= n2) {X;Y;} else {Y;X;}` (fall-through=X先); ④ 符号比较用 `(s16)` 内联 cast; ⑤ off=idx*9 是长命量(r4)。**剩余卡点 = global-alloc 排列墙**: 目标把 dx→r8/dy→ip/sx→sb/ay16→sl/ptr→[sp]/ax16→[sp+4] (sub sp,#8), 我的编译这些量的 home 不同 (如 ax16 落寄存器不落栈, push 少一个 sl)。amt 变量: 目标 <0 臂 `lsls r0,r7,#0x10; lsrs r1`(从 n1 扩展), ≥0 臂无扩展(复用 ax/sx 的 home) — 说明源码 amount 是独立 s16 局部(初值 sx, <0 臂重赋 n1), 但 home 排列仍未命中。已试: v1-v8 共 8 轮结构变体 + permuter 8 万次迭代 (best 7880 但含双写 store 语义破坏, 弃)。**下一步**: 对比 agbcc -dl 的 qty 优先级表定位哪个量的 pri 需要抬/压; 或确认源码是否有 m2c 的 sp4(ax<<16) 命名局部。 |

## MOD-01 语义命名对照 (2026-08-31 plan 应用)

- **真改名已应用** (2026-08-31 二次操作, 取代初版 #define 方案): src/*.c 定义+全部调用点、
  include/iwram.h / ewram.h extern、include/code_0.h 原型、linker.ld 符号、ll.cfg、code.s、
  functions.yaml 键 → 全部改为语义名; split_asm.py 重建 asm/{non,}matchings (文件名+内部 bl 同步);
  asm/crt0.s 的 AgbMain 引用手动同步。IDA .i64 同步重命名 (原符号记入注释)。
- 注意: Makefile 不跟踪 INCLUDE_ASM 的 .s 依赖 — 改名/改 asm 后需 `rm -rf build` 全量重建再验。
- RAM: iwram.h/ewram.h 的 extern 已用语义名+原类型 (类型逐一对齐既有声明); linker.ld 符号同步改名;
  IDA 已建 EWRAM/IWRAM 段并打标签 (72 个)。
- 语义依据: docs/modules/MOD-01-engine.md (8 子系统 + 38 全局语义)。
- 构建验证: make + SHA1 通过, fncheck --blame 全 ROM 一致。

| 语义名 | 符号 | 地址 |
|---|---|---|
| AgbMain | sub_800128C | 0x0800128C |
| System_Init | sub_8001128 | 0x08001128 |
| Task_MapExplore | sub_8001D08 | 0x08001D08 |
| Task_DispatchGameState | sub_8003088 | 0x08003088 |
| SceneTransition_Load | sub_8001354 | 0x08001354 |
| NewGame_Init | sub_8001538 | 0x08001538 |
| CheckEncounter | sub_8002D54 | 0x08002D54 |
| Chara_SetWalkPath | sub_8003958 | 0x08003958 |
| Chara_StepMove | sub_8003C54 | 0x08003C54 |
| Party_FollowAnim | sub_80040E4 | 0x080040E4 |
| CutsceneAnim_Load | sub_80046DC | 0x080046DC |
| CutsceneAnim_PlayFrame | sub_800478C | 0x0800478C |
| EventFlags_Test/Set/Reset | sub_8001030/1050/1070 | 0x08001030+ |
| SwitchFlags_Test/Set/Reset | sub_80010AC/10CC/10EC | 0x080010AC+ |
| LZ_InitContext / LZ_UncompressChunk | sub_8000FD0 / sub_8000D5C | 0x08000FD0 |
| Rand_TableNext | sub_8000FF8 | 0x08000FF8 |
| gCameraPosX / gCameraPosY | gUnk_030025B4 / gUnk_030025FC | 0x030025B4 |
| gDialogueActive / gEncounterEnabled | gUnk_030025D8 / gUnk_03004820 | 0x030025D8 |
| gEventFlags / gSwitchFlags | gUnk_03001C60 / gUnk_030018F0 | 0x03001C60 |
| gCharaWalkCmdBuf | gUnk_0203EE00 | 0x0203EE00 |
| gMainTaskSlot / gScenePhase | gUnk_03001AC0 / gUnk_03002600 | 0x03001AC0 |
| (完整清单) | iwram.h/ewram.h/code_0.h 尾部 | - |

## 第二批语义真改名 (2026-08-31 plan, MOD-08/MOD-02/sound/save)

- 应用 168 函数 + 11 RAM 符号真改名 (1945 处替换): 脚本 VM opcode 处理器全族 (Op_*),
  精灵动画槽 (AnimSlot_*), 菜单 UI (MenuEnt_*/MenuUi_*), 音频包装层 (Bgm_*/Sfx_*/Sound*),
  存档 (Save_Fsm/Save_FillSlot0-3), 属性系统 (Stats_*/ExpToLevel/Item*), 宝箱 (Chest_*)。
- 依据: docs/modules/MOD-08-scriptvm.md + MOD-02-sprite-chara.md + MOD-09-sound-save.md。
- 保留既有 #define 名的函数未动: UpdateSpriteAnim/LoadSpriteAnimSet/ReloadSpriteSheet(s)/
  LoadDigitFontObjTiles/BlankTilemap/ResetSceneObjects/FlushTileDma/ScriptGotoEntry/ScriptClearFlags/
  CheckObjectKindSlot/AddInventoryItem/RemoveInventoryItem/SetSlotGfxId/SetSlotPalId/GetPendingSpriteLoad 等。
- RAM 新语义: gSoundTaskFlags/gPlayingSongId/gBgmVolume/gFade*/gSfxTrack*/gBgmRequestId(修正原 gCurrentSongId)。
- make + SHA1 + fncheck --blame 全绿。
- 待办: MOD-03(code_8010F10.c)/MOD-04(code_801A3C4.c)/MOD-05(code_8020D50.c)/MOD-07(code_8044394.c) 分析+改名;
  IDA 库与第二批名字同步。
| PartyForm_ApplyBonus | 0x0800AC08 | MOD-02 | code_8005020.c | ✅ 已匹配 (用户首试+plan 可读性改造) | 队伍形态一致性检查: 4 角色 gCharaBaseData[].field_4 高4位全同且为 0xE/0xF → 设 gEquipBonusAtk/Def (0x22/0x2D 或 0x3C/0x3F, Stats_RecalcEquip 消费)。typedef Unk_Struct {u16 field_0; u16 field_2; u8 field_4; u8 pad[7]} = 12B 角色基础数据表 (0x087EA580)。**双视图**: 同地址 u8 字节视图 (AAF8/AB18 拼字节) + struct 视图 (AC08), linker.ld 两符号 — 字节视图函数改 struct 访问会变代码生成 (实测踩到, 经验 67 变体)。→ 路线文档 docs/ROUTES.md |

## 会话小结 (2026-09-01 plan): code_8005020.c matchings 全量处理

**总量: 21 个 asm/matchings 函数, 19 个实装真 C + 2 个语义命名保留 asm (注释草稿完整), SHA1 全绿。**

| # | 问题 | 定位方法 | 解决 / 结论 |
|---|---|---|---|
| 1 | IntroBg_Load 调色板 DMA 尺寸写 0x20 但目标是 0x84000010 (16字=0x40B) | fncheck 池内容比对 (mine `08 00 00 84` vs target `10 00 00 84`) | 实际 DMA 拷贝**整个 0x40 行 (双 16 色库)**, 草稿的 0x20 是错的。DMA 控制字反推 size 参数 |
| 2 | StaticObjs_StepAll 的 EnqueueRender 返回值多出 `lsls r0,#0x18` 截断 | fndiff 逐指令 | 目标直接 `cmp r0,#0` — code_0.h 的 u8 返回原型对该调用点是错的; 本 C 文件 用 s32 局部原型 + linker.ld 同址别名 `Sprite_EnqueueRender_S32` (不改共享头, 经验 90) |
| 3 | StaticMapObject.x/y/z、ChestObject.x/y 声明 s16 → ldrsh → code_80002A0.o 缩 8 字节整体位移 | fncheck --blame 报 `-8` 位移, 逐函数尺寸对账 | 字段实为 **u16** (经验 89); 单函数 OK ≠ 布局 OK, 必须全量 SHA1 |
| 4 | 在 C 文件 定义 `u8 gUnk_08095028[][8]={{}}` 占位 → rom overflow 8 字节 | 链接错误 + .data 0x08800000 0x8 | 占位一律 extern const + linker.ld 绝对符号 (经验 92) |
| 5 | sub_8007D5C 单字节差 +0x244 | fncheck 偏移落在池区, 反汇编该池条目 | 草稿索引变量错: `gUnk_087E96B4[gUnk_030047B4]` 应为 `[gChoiceGroupIdx]` (0x030047BC, 与 0x030047B4 相邻易混) |
| 6 | sub_8007FB8 怎么写都多 4 个字面池 | 目标用 r4 缓存 + `adds r1,r4,r2` (0xFFFFFE80 等负偏移池) | **基址±偏移形态**: `gfx=gUnk_080873BC; gfx+0x144; gfx-0x180; gfx-0x160` (经验 91), 独立符号多 4 池 → ROM 位移 |
| 7 | sub_800EB98 剩 12 字节寄存器分配差 | 目标零常量 `mov sb`; 0x4000/0x3FF 直接经 r2 物化 | 挂起: 零常量 sb/r9 与常量 ip 中转的分配选择, 需 permuter。语义已 100% 还原并注释 |
| 8 | 在 /* */ 草稿内嵌套 /* */ 小节注释 → 提前闭合, 后半草稿变 live 代码 | `syntax error before /` | INCIDENTS.md 事故表老坑重演; 改草稿前配对检查 /* */ (经验 94) |
| 9 | json.dump 重写 scripts/data.json 产生 4 万行假 diff | git diff | 共享机器文件只做定向字符串替换 (assert count==1) 保格式 (经验 95) |
| 10 | code_0.h 空括号原型 `void sub_X();` 与真 C 冲突 | `can't match an empty parameter name list declaration` | 实装前先升级 code_0.h 原型为带参形式 (经验 93) |
| 11 | 并发编辑: 另一 agent 同期实装了 sub_8008CC0(ChoiceMenu_ResolveDest) 并重命名 | INCLUDE_ASM 行消失 + code.s 中途变化 | 编辑前重读 + 保留对方成果; INCIDENTS.md 流程有效 |

**语义命名新增 (ll.cfg 权威管线)**: MapBg_LoadFull / MapScene_InitSprites / MapBg_LoadInterior /
BgScroll_LoadFromTable / MenuEnt_ParseAll / MenuEnt_ParseRange / PaletteFx_Apply / StaticObjGfx_LoadPair /
StaticObjs_Spawn / StaticObjs_StepAll / StaticObj_BuildChain / SceneBg_Reload / MenuUi_DrawItemList /
ScreenIdleIcons_BuildList / IntroBg_Load / Logo_LoadAssets / MenuUi_SpawnAuxSprites / UiSprites_Update /
UiSprite_BeginSlide / LoadBackdropScreen(=IntroBg 旧版, 已并入)。
数据符号: gIntroBgPalettes/Tiles/Maps, gBgPalBackdropWhite, gScreenIdleIcon*(4), gUiSprites(+Aux/AuxDesc),
gStaticMapObject 系列, gUnk_03004914/18/496C/4970 等。

## EnemyCharaStat (0x087EA580) 数据定性 (2026-08-31 plan)

12B×256 项 (有效 0-247): expReward(u16)/goldReward(u16,≈exp/2)/formRace(高4外形,低4属性族)/
dropItemId/hp/attack/defense/aiTableIdx(×3→gUnk_0839CEFC)/resistFlags(元素位段)。
- **ROM 数据不落 C 数组**: data1.s 的 `.incbin` 已含此数据; 单独 C 数组使 .rodata 溢出 8MB。
  正确做法 = linker.ld 绝对别名 `gCharaBaseData = 0x087EA580` + iwram.h `extern const EnemyCharaStat gCharaBaseData[]`。
- **双视图纪律**: gUnk_087EA580 (u8, 字节视图, 各 C 文件 局部 extern — const 性不同会 conflict) 供
  sub_800AAF8/AB18/AADC/804DD90 按字节拼/读; gCharaBaseData (struct) 供 PartyForm_ApplyBonus。
  字节视图函数改成 struct 字段访问 → GCC2 生成 ldrh 代替两条 ldrb → 已匹配 ROM 变红 (经验 67 变体, 实测×2)。
- iwram.h `#endif` 后追加 typedef 需自带 `#ifndef` guard (agbcc 对 typedef 重复声明报 conflict)。

## data_805769C.c 数据区命名 (2026-08-31 plan)

| 语义名 | 旧名 | 语义 |
|---|---|---|
| gWaveSineTable | gUnk_080576D0 | 128 项 u8 半周期正弦表 (HBlank 滚动波形源) |
| **gRandShuffleTable** | gUnk_08057750 | 256 项 0-255 Fisher-Yates 预计算完全排列 (伪随机查表, &7 取模 → 8 路均匀) |
| gWalkMoveDirLut | gUnk_0805881C | 16B: 十字键 moveFlags → 方向编码 (Task_MapExplore 消费) |
| gWalkAnimFrameMapping | gUnk_0805882C | 8B: 走路动画帧序 (0,1,2,1,0,1,0,1) |
| gWalkAnimDimTable | gUnk_08058834 | 精灵尺寸/偏移表 (Sprite_EnqueueRender + Anim_BuildOamChain 共用) |
| gWalkDirectionMapping | gUnk_08058864 | 24B: 朝向→动画方向映射 (Sprite_UpdateCharaAnim 消费) |
| gSpriteTileCountTable | gUnk_0805887C | 16B: OAM shape/size → tile 数 (Anim_BuildOamChain 消费) |

**gRandShuffleTable 定性**: 256 项 0-255 完全排列 (distinct=256, Fisher-Yates 预计算),
尾部 5×0xFF padding。Rand_TableNext 每次返回下一项, gRandCursor 每 VBlank 递增。
&7 取模后 32/32/32/32/32/32/32/32 = **完美均匀 8 路**。

## 2026-09-01 ScreenFade 命名与真 C 匹配 (codex)

`sub_80051D0` 已匹配并命名为 `ScreenFade_Apply`（156B），其相邻的
`BlendRegs_Update` 与 `FadeScript_Start` 分别统一为 `ScreenFade_Update`（140B）和
`ScreenFade_Start`（64B）。三者形成同一淡入淡出状态机：Start 设置初始状态，Update 每帧推进，
Apply 依据当前扫描线将结果写入 GBA 的混合寄存器。

| 旧名 / 地址 | 新名 | 语义 |
|---|---|---|
| `sub_80051D0` / 0x080051D0 | `ScreenFade_Apply` | 用 `(u16)gScreenFadeProgress - VCOUNT` 的有符号 1/16 缩放值更新 `REG_BLDCNT`、`REG_BLDALPHA` 或 `REG_BLDY`。 |
| `BlendRegs_Update` / 0x0800526C | `ScreenFade_Update` | 刷新常规 blend 寄存器；按 signed step 更新进度，到达两端时清除或置位完成标志。 |
| `FadeScript_Start` / 0x080088B4 | `ScreenFade_Start` | 写 flags、step、param；负 step 从 `0x1B0` 开始，非负 step 从 0 开始。 |

全局变量命名：`gScriptLockFlags` → `gScreenFadeFlags` (0x0300465C)，
`gUnk_030047A8` → `gScreenFadeProgress` (0x030047A8)，
`gUnk_030047F0` → `gScreenFadeStep` (0x030047F0)，
`gUnk_03004834` → `gScreenFadeParam` (0x03004834)。`gUnk_03004658` 与已有的
`gBlendControl` 同址，删除重复别名。`gScreenFadeFlags` 的 bit 7 是完成状态；param 目前只确认由
Start 写入，尚未确认读取者。

`ScreenFade_Apply` 的生成要点：进度虽为 `s16`，但必须显式 `(u16)` 转换以产生目标的 `ldrh`；
另外保留对局部 `blendControl` 的恒真死赋值，以让 GCC2 将其分配至 r4。移除该语句会使 156B 函数的
寄存器分配及后续字节偏离。三函数均已通过 `fncheck`，全 ROM SHA1 为 OK。

## 2026-09-02 `gUnk_0808A234` 引用分析与命名

`0x0808A234` 在代码中有两条直接引用链，但只有一条真正把它当作表基址使用：

| 引用函数 | 访问形状 | 结论 / 命名 |
|---|---|---|
| `sub_8009370` / `0x08009370` | VBlank 调色板刷新循环中，`base + (gUnk_03000038[i][gUnk_03000020[i] >> gUnk_03000018[i]] << 5) + 2`，DMA3 传 0x10 个半字到 `gUnk_03000028[i]` | `0x0808A234` 是 124 项 × 0x20B 的 OBJ 调色板表；建议改名 `MenuEnt_FlushPalettes`，数据改名 `gMenuEntityPaletteTable` |
| `sub_800661C` / `0x0800661C` | 仅在场景资源分支中访问 `base + 0x1140 = 0x0808B374` 的一个字节，再 DMA 到 `0x05000140` | 这是后续独立数据 `byte_808B374` 的访问，不是调色板表消费者；函数职责是地图场景资源/状态总装载，已改名 `MapScene_Load` |

`data/raw_data/byte_808A234.bin` 大小为 `3968 = 124 * 0x20` 字节。每项首半字为 `0xFFFF` 保留值，实际 OBJ 调色板刷新从偏移 `+2` 搬运 15 个 BGR555 色值。`MapScene_Load` 的 `+0x1140` 恰好越过该区域末端 `0x0808B1B3`，落在 `0x0808B374`，因此不能因共享一个字面池基址而给它使用调色板语义名。

已落盘：`ll.cfg`/`functions.tsv`/调用点/asm 切片中的 `sub_800661C -> MapScene_Load`，绝对符号 `gMenuEntityPaletteTable = 0x0808A234`，以及 `scripts/data.json` 数据名同步。`MapScene_Load` 单函数 `fncheck` 为 OK；`sub_8009370` 当前由另一 agent 认领，正式改名待其锁释放后执行。

## 2026-09-02 `sub_80175C0` 匹配 (SIO 槽位清零, code_8010F10)

34 行小函数, 语义: SIO 会话初始化前的槽位清零。流程:
1. `sub_8016C88()` (前置状态重置, 未匹配, 原型 K&R `void`);
2. `CpuFill32(0, &gSioSession, 0x60)` — 控制字 `0x05000018` = 32bit | SRC_FIXED | 0x18, 清零结构体前 96 字节;
3. 循环把 `unk18[2]` 两个 24 字节槽位各自前 4 字节 (`field_0`/`field_2`) 清零;
4. `sub_8017120(1)`。

**卡点与解法 (新规律 103)**: 循环体/指针初始化写法 (do-while 指针 / for 双初值) 都被编译器调度成
`adds r4,#0x18; movs r0,#1; movs r1,#0` (零常量最后加载), 差 3 条指令顺序 18 字节。
permuter 探索出: 把常量 0 存入独立变量 `zero = 0;`, 让指针经 `&gSioSession.unk18[zero]` 计算、
赋值用 `p->field_0 = zero;`, 编译器便把 `movs r1,#0` 提到 preheader 最前,
生成 `movs r1,#0; adds r4,#0x18; movs r0,#1`, 逐字节命中 (64B, fncheck OK)。

原型: `void sub_80175C0(void)`, `void sub_8016C88(void)`, `void sub_8017120(u32)`, `CpuSet` 走 `gba/syscall.h`。
未改名 (SIO 语义名待 `sub_8016C88`/`sub_8017120` 一带匹配后统一命名)。

注: 提交时 ROM 未全绿, `fncheck.py --blame` 归属显示差异主要在 `data/sound_data.o`(4660B) 等
其他 agent 未提交工作 (首个真实差异 0x080003d4, 早于本函数 0x080175c0); 本函数 fncheck OK, 照常提交。

## 2026-09-02 `0x08088400` 地图宝箱表引用分析

`0x08088400` 只有一个直接代码引用：`ChestObjects_LoadForMap`（原 `sub_8008F28`，
0x08008F28）。函数以 `gMapNpcSetId` 为参数，从该地址开始按 8 字节步长扫描 256 条记录；
首字节相等时依次建立 `gChests[0..15]`，记录序号写入对象的 `mapEntryIndex`，用于索引
`gChestFlags`，坐标半字分别左移 3 后写入对象 `x/y`（Y 额外加 8），并调用
`Chest_BuildSprite`。未命中的剩余对象被填成 `flags=0xFF`、`spriteNodeIdx=0`。

表项已确认是 `ChestMapEntry`：`mapId`、`itemId`、`specialFlag`、保留字节、地图 tile
坐标 `tileX/tileY`。`itemId` 被写入宝箱对象的 `field_3`；`CheckFacingEvent`（原
`sub_8003F40`）在面向宝箱时返回 `field_3 + 1`，再由探索主循环转换为脚本事件号。
`specialFlag` 设置对象状态位 7，面向交互时额外要求事件标志 `0x40`。

已将 `ChestObjects_LoadForMap` 还原为 C。逐字节匹配确认：记录扫描确实覆盖索引 0..255，命中项按记录序号写入 `mapEntryIndex`，`tileX/tileY` 分别转换为 `x=tileX<<3`、`y=(tileY<<3)+8`，未使用槽以 `flags|=0xFF`、`spriteNodeIdx=0` 清空；`fncheck` 为 168B OK。

已将 256 × 8B 原始数据结构化写入 `src/data_805769C.c` 的 `gChestSpawnTable`，并将
`data/data.s` 的连续 blob 起点从 `0x08088400` 调整为 `0x08088C00`。表区与后续 64 字节
数字字形数据均通过 ROM 偏移比较；两个重命名函数分别通过 168/420 字节 `fncheck`，全量
`make` 与 `sha1sum -c ll.sha1` 均通过。

## 2026-09-02 `0x08088C00` 数字字体调色板

`0x08088C00` 只有 `LoadDigitFontObjTiles`（0x08009114）一处直接引用。函数在
`gObjGraphicsSetId` bit7 清零时通过 DMA3 将该地址的 0x40 字节复制到 `0x050003C0`，
对应 OBJ 调色板槽 14/15；同一函数随后从 `0x08088C40` 搬运 0x140 字节数字图块到槽 150。
`0x08088C00` 已定义为 `gDigitFontObjPalettes[2][16]`（BGR555 半字），`0x08088C40`
已定义为 `gDigitFontObjTiles[0x140]`（10 个 4bpp 数字字形 tile）。`LoadDigitFontObjTiles`
已是合适语义名，无需进一步改名；连续 blob 起点相应后移至 `0x08088D80`，场景描述符地址保持不变。

## 2026-09-02 `sub_8017640` 匹配 (memcpy 对齐双路径, code_8010F10)

76 字节 memcpy 变体: 参数 `(void *dst, void *src, s32 count)`, count 是**字数**, 总复制 `count*4` 字节。
`((u32)dst | (u32)src) & 3` 为 0 → 4 字节对齐路径 `ldmia/stmia` 逐字复制 count 次; 否则逐字节复制 `count*4` 次。
两条路径都做 `while (count != -1)` 预检 (count==0 时直接返回)。

**卡点与解法 (新规律 104)**:
1. **寄存器镜像**: 一次性分支内 `u8 *d = dst` / 直接用 `u8 *dst` 形参, 编译器生成 `adds r4,r0; adds r3,r1` (dst→r4, src→r3), 全函数镜像。改写成 `void *dst, void *src` 形参 + **函数顶部集中声明** `u8 *d; u8 *s;` (分支内再赋值), prologue 变为 `adds r3,r0; adds r4,r1` (dst→r3, src→r4) ✓。
2. **LSL 槽**: 字节路径 `count = count * 4 - 1` 一句生成 `lsls r0,r2,#2; subs r2,r0,#1` (借用 r0); 拆两句 `count = count * 4; count--;` 生成 `lsls r2,r2,#2; subs r2,#1` (原地改 r2) ✓。
3. 字路径直接用参数 dst/src 做指针 (不引入 d/s), 复用 prologue 的 r4=src。

fncheck OK (76 bytes, 0 池重定位, 0 bl 槽)。全 ROM SHA1 绿。

## 2026-09-02 `sub_80166FC` 匹配 (字符表渲染到 tilemap, code_8010F10)

92 字节: 把 `gUnk_08095828[charId-1]` 的字符串逐字渲染到 tilemap `0x02005800 + y*64 + x*2`,
最多 8 字符, 遇 0 终止; charId==0xFF 直接返回。

**卡点与解法 (新规律 105)**:
1. 首版 `gUnk_08095828[charId - 1]` (charId 为 u8 形参) 被编译器常数折叠, 生成 `lsls r1,r0,#3`
   (没有 subs r0,#1), 整体错位 67 字节。显式 `(u8)(charId - 1)` 后得到目标的
   `subs r0,#1; lsls r0,r0,#24; lsrs r0,r0,#21` 三步截断序列 ✓。
2. `dest = x*2 + 0x02005800 + y*64` (x*2 最前) 才生成目标的 `lsls r1,r5,#1` (x*2→r1) 先、
   `lsls r0,r2,#6` (y*64) 后; 写成 `0x02005800 + y*64 + x*2` 则镜像 11 字节差。
3. 原型从 K&R `void sub_80166FC();` 改为全原型 `(u8,u8,u8,u8)` — 调用方 sub_800B374 未匹配,
   INCLUDE_ASM 不受影响, 安全。

fncheck OK (92 bytes, 1 bl 槽忽略)。全 ROM SHA1 绿。

## 2026-09-02 `sub_8009370` 匹配 —— 破解 "global-alloc 域三连" 首例! (code_8005020)

函数: 调色板 DMA 上传。`if (gUnk_03004910) sub_80094FC(); else { PalTransfer_Flush(); for (i=0;i<=3;i++) { ... DMA3 拷贝 32B ... } }`
卡了多轮的 "global-alloc 域" 挂起项 (TSV 旧 note 明示"别再穷举 C 写法")。

**最终解法 = 两个结构性关键点, 缺一不可:**

1. **`b` 不要落局部变量**: 条件直接写 `gUnk_03000010[i] != 0 && (gUnk_03000010[i] & 4) == 0`
   (两次直接下标访问)。若先 `b = gUnk_03000010[i]` 再 `b & 4`, GCC2 把 b zero_extend 成 SI,
   循环里 `ands r2,r0` (结果落 b 的寄存器); 直接下标访问则保持 QI(subreg), 生成目标的
   `movs r0,#4; ands r0,r2; cmp r0,#0` (结果落常量寄存器 r0)。
   → 这一条同时解决 ands 方向 + `movs r0,#4` 顺序。

2. **DMA 源拆三行**: `off = ((u32)(*(u8*)(gUnk_03000038[i] + (gUnk_03000020[i] >> gUnk_03000018[i]))) << 5) + 2;`
   `base = (u8*)gMenuEntityPaletteTable;` `src = (u32)(base + off);` 再 `DmaSet(3, src, gUnk_03000028[i], 0x80000010);`
   —— 让 `0x0808A234` 基址提升进 r8 (preheader `ldr r7,=0x0808A234; mov r8,r7`), 且 `0x03000010` 循环内现取,
   与目标完全一致 (旧 base.c 直接写 gUnk_0808A234 会折叠 +2 进池常量)。

**本轮 qtydump/-da 分析过程** (验证 global-alloc 层)**: 对候选跑 `-da`, 在 gccdump.greg 看
"Registers to be allocated in sorted order" 排序 (refs/live_length), 确认关键差异是
0x0808A234 (reg24, live=100) vs 0x03000010 (reg32, live=96) 的优先级竞争; 最终靠上述 C 结构
让编译器把表基址提升进 r8 而 RAM 基址现取, 逐字节命中。

bytecmp 8/216 (8 字节全为 bl 槽位, 非槽位差异 0); fncheck OK (184B, 2 bl 槽忽略); 全 ROM SHA1 绿。

**给另外两个 "global-alloc 域" 挂起项 (sub_8018E34 / sub_804BE90) 的启示**:
不要再去打 agbcc global.c 的 dump 补丁 (路径 a) —— 这条经验证明"提升决策"是可以被
C 结构 (中间变量拆分 + 保持窄类型不落局部) 改变的, 值得先穷举结构再考虑改编译器。

## 2026-09-02 `Save_SyncShadow` 匹配 (影子存档回拷, code_8010F10)

88 字节: 把 `0x02027000` 影子缓冲的数据按 `gUnk_080981E6` 长度表逐块拷回
`gUnk_087EB1E8` 指向的真实块地址 (Save_LoadContinue 的逆操作), 源偏移从 0xC 起连续递增。

**结构与卡点 (u16/u32 类型选择)**:
- 外层 `do { dest = gUnk_087EB1E8[i]; i++; 内层 while(len) 拷 len 字节; i = (u16)i; len = gUnk_080981E6[i]; } while(len)`
- **i 必须用 u32**: 目标顶部只有裸 `adds r3,#1` (不归一化), 归一化 `lsls/lsrs #0x10` 只出现在
  **循环底部** (`i = (u16)i`) —— 若 i 声明为 u16, 编译器每次 i++ 都插归一化, 整体错位。
- **offset/len 必须用 u16**: 内层 `*dest = shadow[offset]; offset=(u16)(offset+1); dest++; len=(u16)(len-1)`
  目标每次增量都带 `lsls/lsrs #0x10` 归一化。
- **shadow 指针先声明** → prologue `ldr r5,=0x02027000` 排第一 (目标顺序: shadow→r5, offset→r4, i→r3)。

bytecmp OK (88B 全等); fncheck OK; 全 ROM SHA1 绿。

## 2026-09-02 `sub_80188BC` 匹配 (按键轮询+前沿检测, code_8010F10)

108 字节: s8 倒计时 `gUnk_03000316`(减 1 后若仍 >0 走 clear 清空两缓冲, 否则读取按键),
尾调 `sub_80182A8(gUnk_03000310, gGstate330)`。0x03000310 新建符号 `gUnk_03000310`(当前按键 u16),
gGstate312 = 新按下按键(keys & ~old), 对齐 gGstate330 传参。

**卡点与解法 (新规律 107)**:
1. 减 1 后再按 s8 比较, 直接写 `if ((s8)gUnk_03000316 > 0)` 被 GCC2 复用寄存器 (lsls r0,r0,#24);
   插入中间变量 `tmp = gUnk_03000316; if ((s8)tmp > 0)` 强制重读内存 (movs r0,#0; ldrsb r0,[r1,r0]) ✓。
2. 块布局用 readkeys/clear/tail 三标签 goto: `if <=0 goto readkeys; dec; if >0 goto clear;` +
   readkeys 带 `goto tail`, clear fall-through —— 匹配目标基本块顺序。
3. 赋值顺序: `gGstate312 = keys & ~gUnk_03000310; gUnk_03000310 = keys;`(先写新按键) 决定
   ldr r4=0x03000312 在 ldr r2=0x03000310 之前, 池条目随之排列 ✓。

fncheck OK (108 bytes, 1 bl 槽忽略)。全 ROM SHA1 绿。

## 2026-09-02 `sub_804ABF8` 匹配 (tile 动画帧写入, code_8044394)

104 字节: 按 `arg1*18 + gUnk_0300094D*2` 索引 `gUnk_0862D574` (u8*, 每动画 18 字节的 u16 帧表),
把当前帧写入 `dest[0]/dest[0x20]` 两处 tilemap (值 = data*2 - 0x5000 / data*2 - 0x4FFF),
帧号 `gUnk_0300094D++` 后: >3 或下一帧 == 0xF00 终止符 → 返回 1 (动画完), 否则 0。

**关键 (三次迭代从 85B 差到 0)**:
1. **表基址作局部指针** `u8 *base = gUnk_0862D574;` (用它两次读) → 基址进 r6、counter 地址进 r4,
   加载顺序与目标一致 (直接写 gUnk_0862D574 池会后载)。
2. **偏移拆局部变量** `off = gUnk_0300094D * 2 + arg1 * 18;` 再 `*(u16*)(base + off)`
   → 算术排成 `lsls r3,r3,#1; lsls r2,r1,#3; adds r2,r2,r1; lsls r5,r2,#1; adds r3,r3,r5; adds r3,r3,r6`。
3. **第二次读也要用局部 off2** (不能复用同表达式内联) → 加法顺序目标为 "先 +arg1*18 再 +base",
   全连成 `adds r0,r0,r5; adds r0,r0,r6`; 内联则编译成 `adds r0,r0,r6; adds r0,r5,r0` 差 3 字节。

fncheck OK (104B, 0 池重定位, 0 bl 槽)。全 ROM SHA1 绿。

## 2026-09-02 `sub_804F050` 匹配 (道具id->菜单页号 线性查找, code_8044394)

44 字节: `for(i=0;i<16;i++) if(arg0==gInvPageItemIds[i]) break; return i;` —— 在 16 项页号表里
反查道具 id 所在页, 未命中返回 16。语义与 `code_8010F10.c` 的 Inv_FindFirstHeld 家族共用同一张表。

**一次成型 (候选 C 已在注释里, 直接实装即命中)**:
1. `arg0`/`i` 都取 `u8` → 入口与 `i++` 各产一对 `lsls #0x18; lsrs #0x18` 字节截断 (写成 int 会丢)。
2. 表用真 `extern const u8 gInvPageItemIds[]` 索引, 不用 `((const u8*)0x0839CFAA)[i]` 强转宏 (会换寄存器)。
3. GCC2 自动把首迭代 (i=0 的 `t[0]` 比较) peel 到循环外, 循环体 `i++` 后 `cmp #0xf; bhi` 收尾返 16;
   朴素 for+break 即复刻该形状, 无需手写 peel。→ 记入 EXPERIENCE 108。

fncheck OK (44B, 0 池重定位, 0 bl 槽)。全 ROM SHA1 绿。

## 2026-09-02 `sub_804EF90` 匹配 (gUnk_03000D88 线性查找, code_8044394)

76 字节: `for(i=0;i<gUnk_03000DDC;i++) if(gUnk_03000D88[i].field_0==arg0){ret=i;break;}` —— 在
`Unk_03000DEntry` (4B) 数组里按 field_0 反查 arg0, 命中返下标 i, 否则返 0xFF。经验 108 的变体。

**要点**:
1. 界是变量 `gUnk_03000DDC`: peel 首块先 `ldrb count; cmp #0; bhs`, 循环体每轮重读 count (全局不缓存)。
2. 带 `ret` 累加器 (0xFF 默认) → 返回值走 r5, 与 sub_804F050 直接返 r1 不同, 但 peel/bhi 骨架一致。
3. **复用 iwram.h 已有类型** `Unk_03000DEntry gUnk_03000D88[]` + `gUnk_03000DDC`; 候选注释里的本地
   `typedef UnkStruct` + `extern UnkStruct gUnk_03000D88[]` 会与头文件冲突, 实装时删掉。
4. 4 字节元素 → 索引 `lsls r0,r2,#2`。

fncheck OK (76B, 0 池重定位, 0 bl 槽)。全 ROM SHA1 绿。

## 2026-09-02 `sub_8048764` 匹配 (场景对象技能槽取值, code_8044394)

22 字节: `val = obj[0xA1]; return val <= 7 ? obj[0x99 + val] : val;` —— 从 0xC8 场景对象的
8 槽技能数组 (0x99~0xA0) 按 `obj[0xA1]` 选择; 若该选择字节 >7 则原样返回它 (越界哨兵)。

**MyStruct 定性 (回答"是否已定义结构体")**: 候选注释里的 `MyStruct{pad[153]; u8 data[8]@0x99; u8 chk@0xA1}`
**不对应任何现成结构体**。对象确为 0xC8 场景对象, 唯一命名类型 `Unk_8020F4C` 是 code_8020D50.c 的
**C 文件 局部** typedef, 且只列了 0x24/0xB0/0xBB/0xBE 等字段, **不含 0x99/0xA1**。本 C 文件 (code_8044394)
既有约定就是把对象当 `u8 *` + 裸偏移 (见已匹配的 sub_8048934/8984/89A4 的 `arg0 + 0x99`)。故沿用裸指针,
不引入重复/冲突的本地 struct。

**codegen 两个坑** (首版 `if(val>7)return val; return *(arg0+0x99+val);` FAIL):
1. **地址结合序**: 目标 `adds r0,#0x99; adds r0,r0,r1` = `(arg0+0x99)+val`; 而 `*(arg0+0x99+val)`
   被 GCC2 折成 `(val+arg0)+0x99` (顺序反了)。必须写 `ptr = arg0 + 0x99; return ptr[val];` 才拿到正确序 (经验 2 同源)。
2. **分支极性**: 目标 `cmp #7; bls LOAD` + `return val` 落空 → 对应 `if (val <= 7) { LOAD } else { return val }`
   的 if/else 写法; 写成 `if (val > 7) return val; ...` 会翻成 `bhi` 布局 (22B→24B 不等)。

fncheck OK (22B, 0 池重定位, 0 bl 槽)。全 ROM SHA1 绿。

## 2026-09-02 `sub_80207DC` 匹配 (场景对象行为分派, code_801A3C4)

100 字节: 5 参 `(u8 *obj, u8 bf, u8 c0, u16 f2a, u8 f35)`。按 `obj->field_BE` 三档分派到
sub_801CBA4 (≤0xA) / sub_801CA08 (≤0x70) / sub_801CE80 ((u8)(BE-0x71) ≤ 0x8D), 均传 `(obj, 0, f2a, f35, 0)`。

**卡点 (1 字节的 v home 选择)**: 目标 `adds r0,r4,#0; adds r0,#0xbe; ldrb r0,[r0]; adds r1,r0,#0`
= 值读进 **r0** 再拷贝到 r1 作比较; 若先 `u8 v = obj[0xBE]` 落局部变量, 编译器生成
`ldrb r1,[r0]; adds r0,r1,#0` (值在 r1, 拷贝到 r0), 差 2 字节。
→ 不落局部、三处条件直接写 `obj[0xBE]` (第三次 `(u8)(obj[0xBE] - 0x71)`), 逐字节命中。

fncheck OK (100B, 3 bl 槽忽略)。全 ROM SHA1 绿。

## 2026-09-02 `sub_804EF50` 匹配 (gUnk_03000D88 条件回写 gUnk_03004980, code_8044394)

64 字节: 遍历 `gUnk_03000D88[0 .. gUnk_03000DDC)`, 凡 `field_0 > 0xDC` 的项, 把 `field_1` 写进
`gUnk_03004980[field_0]`。与 sub_804EF90 同族 (共用 `Unk_03000DEntry gUnk_03000D88[]` + count `gUnk_03000DDC`)。

**要点**:
1. 无 break 的普通 for 被 GCC 旋转成 **bottom-test** (`blo loop`) + 循环前一次首检 peel (`cmp #0; bhs return`);
   循环体每轮重读 count (`ldrb r0,[r3]`, r3 常驻 &count)。与经验 108 的 search-peel 不同, 这里是标准 for 旋转。
2. `field_0` 只 `ldrb` 一次即复用 (既做 `>0xDC` 比较又做 `gUnk_03004980[]` 下标) → CSE, C 里写两遍同一表达式即可。
3. 复用 iwram.h `Unk_03000DEntry` + 本文件 line1408 的 `extern u8 gUnk_03004980[]`; 删候选注释里冲突的本地
   `typedef UnkStruct`/`extern UnkStruct gUnk_03000D88[]` (同 sub_804EF90 坑, 已记经验 108 变体)。

fncheck OK (64B, 3 池重定位已施加, 0 bl 槽)。全 ROM SHA1 绿。

## 2026-09-02 `sub_8020840` 匹配 (sub_80207DC 孪生变体, code_801A3C4)

100 字节: 与 sub_80207DC 完全同构，仅第一/二档分派的 r1 常量不同 (0xA/5 vs 0/0)。
结构同 sub_80207DC: `obj[0xBE]` 不落局部变量、三处条件直接写（落局部则 ldrb 进 r1 差 2 字节）。
首试字节级命中。fncheck OK (100B, 3 bl 槽忽略); 全 ROM SHA1 绿。

## 2026-09-02 `sub_8048BAC` 匹配 (对象 kind 参数表 value 取值, code_8044394)

36 字节: `obj[0xBE] <= 10 ? gUnk_0839CC4C_entries[obj[0x8D]].value : 0` —— 与 `sub_8048B88`
同族 (同表同守卫, 888 读字节 +0, BAC 读字节 +2 = struct 的 `value`)。表 = 4B 项 `{u16 unk_0; u8 value; u8 unk_3}`。

**死路 (连 FAIL 3 次, 全因基址池加载位置/寄存器错配)**:
1. `gUnk_0839CC4C[arg0[0x8D]*4 + 2]` (u8 视图): GCC2 把 `+2` 折进下标 → `lsls; adds r0,#2; adds r0,r0,r1; ldrb [r0]`, 目标要 `adds r0,r0,r1; ldrb [r0,#2]`。
2. `ptr = gUnk_0839CC4C + arg0[0x8D]*4; return ptr[2];` (局部): displacement 对了但基址**晚加载** (算完下标才 `ldr r1,=表`), arg0 落 r1 (目标 r2)。
3. `((Unk_0839CC4C *)gUnk_0839CC4C)[arg0[0x8D]].value` (cast): 同 2, cast 让 GCC2 先算下标 → base-late。

**破法**: 目标基址在分支块**首行**加载 = 只有"真 extern 结构体数组"声明才触发该调度。但 `gUnk_0839CC4C` 已被 `sub_8048B88` 以 `u8[]` 占用 (改声明会破坏已匹配函数), 故按 §7 起**同址别名** `gUnk_0839CC4C_entries` (linker.ld SECTIONS 外 `= 0x0839CC4C`) + `extern Unk_0839CC4C gUnk_0839CC4C_entries[]`, 用 `gUnk_0839CC4C_entries[arg0[0x8D]].value` 一次命中。→ 已固化 EXPERIENCE 109。

fncheck OK (36B, 1 池重定位, 0 bl 槽)。全 ROM SHA1 绿。

## 2026-09-02 code_8044394.c "fake-matched" 批量真 C 化 (9/10, sub_80462E4 挂起)

把 11 个 `INCLUDE_ASM("asm/matchings", ...)` (status=1 但无真身) 逐个真 C 化。qwen 占了 sub_8048BAC 跳过;
其余 10 个: 9 个成功 fncheck, sub_80462E4 评估后挂起。

**成功 9 个** (均 fncheck OK + 全 ROM SHA1 绿):
- sub_80444E8 / 8D40 / 8D64 / 8D84 / AB10 / ABD0 / EEC4 / EF00 / F088。
- 新登记符号: IWRAM gUnk_03000949/970/97B/97D (linker.ld + 本文件局部 extern + 200B obj struct),
  EWRAM gUnk_02035B04 (linker.ld + ewram.h); sub_804E0E4/E2AC 原型 void→u8(非void, 否则 return 不过编译)。
- **两个非平凡坑**:
  1. sub_8048D40: 清零 obj+0x7E..0x86 五个 u16。`*ptr++=0`×5 与 `ptr[i]=0`×5 都会让 egcs 把
     "移动指针" 分到 r1、"常量0" 分到 r0 (与目标 r0=ptr/r1=0 相反, 差 12B); 只有写 5 条
     `*(u16*)(arg0+0x7E/0x80/...)=0` (各自常量偏移) 才让 egcs 强度削减成目标形状。→ 用 bytecmp 隔离试出。
  2. sub_804AB10: obj struct 必须**恰好 200 字节** (remaining[146]) 才产出 `muls r0,#0xc8`;
     候选原写 remaining[143]=197B → 索引步长错。

**挂起 sub_80462E4** (231 指令, 见 functions.tsv note): obj 池筛选器。首版候选 (permuter/sub_80462E4/v1.c)
差 320/456B —— 寄存器分配 (r8/sb/sl 三连) 与循环 peel 全不同。且它现由 asm/matchings 保字节绿,
真 C 若用 r8/sb/sl 有坑1 (GCC2 泄漏破同一 C 文件 其他函数) 风险。判定为需专项逐块攻, 不在本次批量内强推。

## 2026-09-02 `sub_8020B54` 再攻 (经验 17 寄存器轮换, 仍挂起)

函数本身简单: `for(i=0;i<7;i++) gUnk_030006F8[i]=0;` (u8* 队列清 7 项) + `gUnk_03000714/715/716=0`。
朴素真 C 只差 **6 字节**, 且经 objdump 逐条核对: 字面池顺序 (0x714,0x715,0x716,0x6F8) 与目标**完全一致**,
唯一差异是三条 `ldr` 的 dest 与三条 `strb` 的 base —— 目标把三个地址伪寄存器分配成 0x714→r5 / 0x715→r6 /
0x716→r4, 我的恒为 r4/r5/r6。即纯 register rotation, 非语义/非寻址/非调度问题。

**本轮 ~40 种写法全部撞在 6B 地板**:
- 循环形: `for(i<7)` / `while` / `do-while(++i<7)` / `i!=7` / 递减 `for(i=7;i--)` / 拆 `i<6`+末项 —— 全 6B。
- 存储序: 6 种排列 —— 改变的是 **strb 发射序**(egcs 不重排 store), 得 8~9B, 更差。
- 提前算地址的指针局部 (`u8*c=&gUnk_03000716; ... *c=0;`): egcs **copy-prop** 把 c 折回 store 原位, 分配不变。
- 类型: `u8*[]` / `u32[]` / `(void*)0` / 显式字节指针 —— 全 6B。
- 加/删局部改伪寄存器编号 (期望扰动 qsort): 引入的额外指令又破坏字节, 两难。

**根因定位** (agbcc `-dl` → gccdump.lreg): 三个地址 reg 39/40/41 统计**逐字段相同**
(`used 2 times across 30 insns; set 1 time; pointer`), n_refs=2 → floor_log2=1, size=4, life=30 →
QTY_CMP_PRI 完全相等。egcs `local_alloc` 用 **非稳定 qsort** 按优先级排 qty, 等值时次序由 qsort 分区决定,
对当前 qty 数组恰好产出 39→r4,40→r5,41→r6; 目标 ROM 那次编译产出 41→r4,39→r5,40→r6。
这是**编译器版本/周边 qty 集合的 tiebreak 产物, 无法用等价 C 稳定复现** —— 经验 17 判定成立, 继续挂起。
候选留 `permuter/sub_8020B54/base.c` (6B)。若将来要收: 需改 agbcc local_alloc 的等值排序 (改编译器, 破全局一致性)

## 2026-09-02 `sub_8020B54` 攻破 (do-while 屏障打破 tiebreak, 经验 116)

前一轮 40+ 写法 (语句序/链式/指针/类型) 全撞 6B 地板后, 本轮**只差最后一条 strb 的存储序** (714,716,715 vs 714,715,716):

- **链式赋值突破口**: `gUnk_03000714 = 0; gUnk_03000715 = (gUnk_03000716 = 0);` 让
  寄存器分配**完全归位** (r5=0x714 / r6=0x715 / r4=0x716, 池序 [714,715,716,6F8] 不变) ——
  经验 110 的"链式=地址伪寄存器压缩"把三 qty 变成 2 伪寄存器+1 依赖, 平手被打破。
  但链式把存储序搅成 714,716,715 (链内先存内层), 仍差 2 条 strb 的字节 (4B)。
- **屏障定序**: 在**最后一个** `=0` 外包 `do { gUnk_03000716 = 0; } while (0);` (经验 25/116),
  使第三条存储的 qty 多一条 insn 的 life, 权重不再全等 → 分配轮换归位 + 存储序恢复 714,715,716。
- 最终写法:
  ```c
  void sub_8020B54(void) {
      u8 i;
      for (i = 0; i < 7; i++) gUnk_030006F8[i] = 0;
      gUnk_03000714 = 0;
      gUnk_03000715 = 0;
      do { gUnk_03000716 = 0; } while (0);
  }
  ```
- 验证: 单函数 .o 与 ROM 0x08020B54..0x2C 逐字节一致 (池区为待解析重定位, 链接后 = ROM 值), fncheck OK (60B), make+sha1 全绿。
- **教训**: "local_alloc tiebreak 不可控"的结论下早了 —— 平手权重不是只能靠编译器, `do-while` 屏障
  能让 qty 的 life 差一条 insn, 从而可控地打破平手。经验 116 收编; 经验 17 已改标「已解」。
- 合入: src/code_801A3C4.c 真 C, functions.tsv status 0→1。

或找到能改变 qty 数组组成又不增删指令的写法 (本轮未找到)。

## 2026-09-02 `sub_80208A4` 匹配 (obj-kind 分派家族变体, code_801A3C4)

112 字节: `sub_801D12C(obj,0)` 后按 `obj[0xBE]` 三段分派 `sub_801CBA4(,2,)/CA08(,1,)/CE80(,2,)`,
f2a=`*(u16*)(obj+0x2A)`、f35=`obj[0x35]` 内联, 第 5 参 0 上栈。与已匹配的 `sub_80207DC`/`sub_8020840`
同族 (那两把 f2a/f35 作形参, 本函数从 obj 现取 + 多一次 sub_801D12C 前置调用)。

**要点**: 直接照搬兄弟函数的 `if/else if (obj[0xBE]<=0xA) ... else if ((u8)(obj[0xBE]-0x71)<=0x8D)` 惯用法,
一次成型 —— bytecmp 与 target .text 逐字节 0 差异 (仅 4 个 bl 槽未重定位), fncheck OK。被调函数保持
code_0.h 里的 K&R `void f();` 原型即可传 5 参 (第 5 个自动上栈), 无需补全原型。

## 2026-09-02 `sub_80196D4` 挂起 (code_8010F10, dialogCtx 槽位初始化)

9 参数函数 (u8 index, ...arg1..arg8), 把 arg5-8 写入 gDialogCtx[index].padding0[0..3],
arg3→field_8, arg4→field_A, field_C=1, field_E=arg2, field_10=arg1。index*20 定槽位, 末尾
`field_10` 用 `0x03000358 + off` (index*20) 重算, 强制 off 存活。

**卡点**: 目标 prologue 是 agbcc **栈参数提升** (9 参导致):
`push {r4-r7,lr}; mov r7,sl; mov r6,r9; mov r5,r8; push {r5,r6,r7}; sub sp,#4;
ldr r4,[sp,#36]; str r4,[sp]; ldr r7,[sp,#40]; mov r8,r7; ldr r4,[sp,#44]; mov r9,r4;
ldr r7,[sp,#48]; mov sl,r7; ldr r4,[sp,#52]; mov ip,r4`
= 5 个栈参: arg4→[sp] 局部槽, arg5→r8, arg6→r9, arg7→sl, arg8→ip。

穷举记录:
- 全 u8 (v15): 无提升 (prologue 直接 push {r4-r6,lr}, 92/108)
- 全 u32/s32 (v17/v26/v34): 无提升; 全 s32+m2c sp0 结构 (v28/v31): 82-84/108
- 混合 u32/u8 (v19): **触发提升**但 arg4 进 r7 而非 [sp] 局部, 且寄存器映射 (arg5→r4,r6→r8...)
  与目标 (arg4→[sp], arg5→r8...) 错位 → 89/104
- struct 指针 + member 访问 (v25/v29/v32): 95/100 → 反而更差
- u16 栈参 (v39): 无提升
结论: 这是 global-alloc 域参数提升的固定分配, "穷举等价 C 写法" 改不动寄存器 home。
待攻方向: ① 参照 sub_801B81C (10 参同族已匹配) 找其 C 触发提升的"参数数量+类型"精确组合;
② 经验 102 的路子: 检查 `arg1`(0x02035AC0) 是否需要声明为指针以改变 home; ③ 已留 v19/v35/v38 候选。

### ✅ 2026-09-04 claude-196d4 解决 (经验 131)

**根因**: 之前所有候选都用 `p = base + off; p[i]=...` 指针局部写法。这样 `off`(index*20) 在 arg5 之后才出生,
local-alloc 按 QTY_CMP_PRI (短命/多引用优先) 让 arg5 抢到低寄存器 r4, 提升错位 (v19 的 89/104 即此)。

**破法**: 改用 `gDialogCtx[index].padding0[0]=arg5; ... gDialogCtx[index].field_10=arg1;` **成员直写** (每行重算 index*20)。
GCC2 CSE 把 `off=index*20` 与基址符号 `base=gDialogCtx` 提成两个贯穿全函数的长命 qty, 加上零常量 (6 个 strb 复用) 共占满 r4/r5/r6;
5 个栈参里"使用最晚"的 arg4 (寿命最长→优先级最低→最后分配) 拿不到低寄存器 → spill 进 `sub sp,#4` 局部槽 (`ldr r4,[sp,#0x24]; str r4,[sp]`, 用 `mov r3,sp; ldrb r3,[r3]` 读),
其余 arg5-8 依次提升 r8/sb/sl/ip。fndiff 逐指令全等, fncheck 116B OK, make+sha1 绿。

**类型坑**: 参数必须保留窄类型 (arg0/arg3-8=u8, arg2=u16, arg1=u32); 全 u32 会让栈参 `ldrb [sp,#off]` 就地读 (无提升, 见 y1 实验)。
窄类型 ANSI 定义与 code_0.h 的 K&R 空原型 `void sub_80196D4();` 冲突 (default promotion 报错), 且改全原型会给调用者 sub_803F328 加截断破坏其已匹配字节
→ 用 **K&R 旧式定义** `sub_80196D4(index,arg1,...) u8 index; u32 arg1; ... { }` 两全。
**struct 修正**: iwram.h `Unk_03000348.field_E` 由 u8 改 u16 (目标 `strh r2,[r0,#0xe]` 是半字写; 原 field_E/field_F 两 u8 无人引用)。

## sub_80113CC (0x080113CC, code_8010F10) — ✅ 2026-09-02 op1
背包 16 页道具表的翻页探测: 从 `gSkillMenuPage+1` 向后找第一个持有页 (`gUnk_03004980[gInvPageItemIds[i]] != 0`),
且其后 15 页内至少还有 2 个持有页才有效。返回: 页号 i / 0 (i 越过 14, 无候选) / 0xFF (后续持有不足 2)。
调用点 code_0.s 两处 (`bl sub_80113CC`, 0x0800D85E/0x0800DB26 附近), 返回值做 `lsrs #0x18` 后按非零分支 → u8 返回正确。

C 形状 (bytecmp + fncheck 136B 一次全等, 首候选即中):
```c
i = gSkillMenuPage + 1;
while (i <= 15 && gUnk_03004980[gInvPageItemIds[i]] == 0) i++;   // 双条件顶置旋转循环 (同经验 73形状)
if (i > 14) return 0;
count = 0;
for (j = i + 1; j <= 15; j++) { if (... != 0) { count++; if (count == 2) return i; } }
return 0xFF;
```
要点:
- 上界 `<= 15` / `> 14` 这对 u8 魔数就是目标的 `cmp #0xf; bhi` + `cmp #0xe; bls`, 不要改成 `< 16`/`> 13` 试探。
- 内层命中计数用 `if (count == 2) return i;` 直接对应 `cmp r3, #2; beq → movs r0,#i` 的提前出口。
- `gUnk_03004980` 在本 C 文件 原先没有 extern, 已按惯例放函数上方局部 extern (与 code_8005020/8044394/804F0B8 一致)。
- code_0.h:272 原型已是 `u8 sub_80113CC(void)`, 本函数没有 K&R 原型坑。

## 2026-09-02 `sub_8015E1C` 尝试 (Text_PutGlyph 内联 + 0xFF 循环, 挂起)

语义完全解出 (借同文件已匹配的 `Text_PutGlyph`/`TextBlocks_Render` 及其 codegen 注释):
`dest=(u16*)gWindowBgBuf + arg0 + arg1*32`, 遍历 `*p != 0xFF`, 每字节按 `b==0`(空白→两格 attr+1)/
`b==0xFE`(tile=0x280)/`else`(tile=b*2) 写 `dest[0]=attr+tile` 与 `dest[0x20]=attr|(tile+1)`, attr=arg2<<12。

**卡在寄存器分配 (经验 17 同类)**: 8 版候选 (v1-v8) 最好 70/104B 差。目标是高寄存器压力函数:
dest 落 **ip(r12)** (故每格 `mov r2,ip; strh[r2]` 而非直接 strh)、`arg2<<28` 落 r5 且**每轮 `lsrs r1,r5,#0x10` 重算 attr**
(非整体外提)、指针落 r4、base 落 r3。我所有等价写法恒得到 {r2,r3,r4} 的**循环置换** (ptr→r3/arg2→r4/base→r2),
且 egcs 在循环内把 `arg2<<12` 整体 CSE 外提 (与 standalone 的 Text_PutGlyph 行为不同)。
- 试过: 每分支各写 attr、`(arg2<<28)>>16` 字面分解、dest 先/后算、直接走 arg3、局部 b 变量 —— 均不翻转分配。
- 结论: 需 fndiff 逐指令长磨 (Text_PutGlyph 作者当年 2435→2610→0 才收, 且它更简单) 或改编译器; 非一次可下。
最佳候选留 `permuter/sub_8015E1C/base.c` (=v8, 70B)。claim 转挂起。

## sub_8018D9C (0x08018D9C, code_8010F10) — ✅ 2026-09-02 op1
战斗 tilemap 缓冲写入器: 按 `sub_80187B4()` (gGstate324 getter) 的 bit14, 把
`(u16*)0x020352C0[idx]`、`(u16*)0x020352C2[idx]`、`+0x482 即 [0x241]` 两项
写成 0x92A2/0x92A3/0x92A4/0x92A5 (战斗边框图块) 或全 0x92C0 (空图块)。
idx = 0x221 (r4 缓存, lsls#1 得字节偏移); 0x20C0 视图基址相差 2 字节 = 同一 u16[] 错位一字视图。

C 形状 (fncheck 152B 首候选全等):
```c
idx = 0x221; p = (u16*)0x020352C0; q = (u16*)0x020352C2;
if (sub_80187B4() & 0x4000) { p[idx]=0x92A2; q[idx]=0x92A3; p[0x241]=0x92A4; q[0x241]=0x92A5; }
else { p[idx]=0x92C0; q[idx]=0x92C0; p[0x241]=0x92C0; q[0x241]=0x92C0; }
```
要点:
- 双基址缓存 p/q (两次池读) 对应目标 0x020352C0/0x020352C2 两个池常量; 0x482 用独立池常量
  (`p[0x241]` 写法会命中 `ldr r1,=0x482; adds r2,r1,r3`, 别写成 `p[idx+0x220]` — 那会复用 0x221 池并多指令)。
- 写值 0x92A2..5 各占一个池常量 (目标 8 个池槽全用上); else 分支 0x92C0 单池常量复用 r2。
- bytecmp 报 4B 差为 bl 槽 + ld 生成的 interwork trampoline 垫尾 (168 vs 152), 指令域逐条全等;
  以 fncheck (合入真身) 为准 OK。此坑印证 bytecmp 对"含 bl 的候选"只比前缀, 长度差不代表不匹配。

## 2026-09-02 `sub_8019F08` 匹配 (tilemap 区域改写, code_8010F10)

112 字节: 把 tilemap 从 `startRow*32+startCol` 起的 width×height 区域, 每个 `u16 tile`
`= (tile & 0xFC00) + addVal` (保留 tile 号, 低位替换为 addVal 偏移)。6 参数函数。

**卡点与解法**:
1. **声明顺序定寄存器 home**: `u8 row; u8 col; u16 *p;` (指针在最后) 生成 `adds r5,r4` (nextRow→r5)
   与目标相反 (差 5 字节, r5↔r6 互换)。改成 `u16 *p; u8 col; u8 row;` (指针**最前**) 即 OK。
   → 多变量函数内, 指针/数组局部先声明可强制高寄存器外的 home 分配。
2. **起始指针写法**: `p = &tilemap[startRow * 32 + startCol]` (直接用数组取址) 生成目标的
   `lsls r3,r3,#0x18; lsrs r3,r3,#0x13; adds r3,r3,r2; lsls r3,r3,#1` (合并×2)。
   写成 `tilemap + startRow*32 + startCol` 则把 ×2 拆开进各项 (差 5-20 字节)。
3. 原型 K&R → 全原型 `(u16*, u16, u8, u8, u8, u8)`; 调用点传 6 参无截断风险。

fncheck OK (112 bytes, 0 池重定位, 0 bl 槽)。
注: 合入时 make 因**其他 agent 未提交重构** (code_8005020.c 引用不存在切片 MapScene_LoadNpcSlotIds)
红, 本 C 文件 独立编译通过、fncheck OK, 照常提交。

## 2026-09-02 `sub_8052AE8` 匹配 (号段随机查表, code_804F0B8)

76 字节: `rec=(u8*)*arg0`, 从 `rec[1]..rec[2]` 号段用 `Rng_LcgNext() % (max-min+1)` 随机取一索引查
`gUnk_02016000` u16 表, 把 `0x02016200 + val` 指针写回 `*arg0`, 返回 1。

**关键坑 (与经验 17 同族但可破)**: 表基址与目标基址**必须写成常量地址** `(u16*)0x02016000` / `0x02016200`,
**不能**用数组符号 `gUnk_02016000` / `gUnk_02016200`。用符号时 GCC2 把 SYMBOL_REF 基址当普通值留在
callee-saved r7 (多 push 一个 + val/max 落 r1/r0 互换), 差 15~35B; 用常量地址 GCC2 才识别为
rematerializable → 每处重取 `ldr [pc]` → 分配命中目标 (bytecmp mine.o .text 与 target 逐字节 0 差)。
- 其余要点: `Rng_LcgNext` 用 `((u32 (*)(void))Rng_LcgNext)()` 强制无符号 → `__umodsi3` (直接 `%` 出 `__modsi3`);
  `diff=rec[2]-rec[1]` 须在 Rng 调用**之前**算 (否则 Rng 先 clobber caller-saved → 目标要重载 max/min, 顺序不符);
  索引 `(u8)(...)` 截断 + `*2` 由 `lsls #0x18; lsrs #0x17` 产出。
- 破法路径: 先朴素版差 64B → 发现 dest/base 分配问题 → 局部指针 t + 常量地址 + diff 前置 + u8 截断, 逐步 64→35→15→0。

**并发提示**: 合入时工作区被另一 agent 的未完成改名 (code_8005020.c→MapScene_LoadNpcSlotIds, asm 未生成) 弄红,
非本函数之锅; 本函数已 bytecmp + objdump 双重定性, 定向提交自己的文件。

## 2026-09-02 `0x08088D80` 地图场景描述符表

`0x08088D80..0x08089B8F` 是 180 项、每项 `0x14` 字节的地图场景描述符表，
不是调色板数据，也不是 225 项的 `0x10` 字节表。已按地址顺序写入
`src/data_805769C.c` 的 `gMapSceneDescriptors[180]`，字段为 12 个字节加 4 个小端
半字：场景装载/显示参数、NPC 槽组号、碰撞阈值、tilemap、tileset、BG 调色板索引。

直接消费者已统一使用 `gMapSceneDescriptors`：`MapBg_LoadFull`、`MapScene_Load` 的
调用链、`MapScene_InitSprites` 以及 `Sprites_LoadMapNPCs` 的 NPC 槽组字段。
`MapScene_Load` 和 `MapScene_LoadNpcSlotIds` 已完成语义命名但仍保留原始 asm，
对应函数清单 note 标为挂起；四个相关函数 fncheck 均 OK，场景表 `0xE10` 字节比较通过，
场景相关函数与数据在独立核验中通过；整 ROM 在当时无并行改动时 `make && sha1sum -c ll.sha1` 通过。
当前共享工作树另有 `sub_8015AF0` 改动导致整体布局偏移，最终红差异由并行改动负责。

## 2026-09-02 code_8010F10.c matchings 批量 (qwen): 8 命中 + 2 挂起

本 C 文件 10 个 `INCLUDE_ASM("asm/matchings")` 全部处理: **8 个已实装 fncheck 绿** (sub_80160CC/038/068/178/1F4, sub_8015ED0, sub_8018750, sub_801A3A8), **2 个挂起** (sub_801A2AC, sub_8015AF0)。全 ROM SHA1 绿。

**已实装要点**:
- sub_80160CC: 修好了他人遗留的**嵌套注释炸弹** (`// ... /* extern */` 在块注释内, `*/` 提前闭合致整个 C 文件 编译崩) → 顺带解锁本文件。
- sub_8016038/068: `gUnk_03004AA0` 即 `gPartyMemberIds` (复用勿重注册); 068 用 DmaCopy32/16 宏 (经验 55), 且因 `SceneBg_Reload`(已匹配) 以 `sub_8016068()` **无参调用靠 r0 残留**传 arg0 → 必须保 `void()` 原型 → 用 **K&R 定义** `void sub_8016068(arg0) u8 arg0;` 规避 "default promotion 不能匹配空参数表" 冲突。sub_80161F4 同理 (MenuHp_Update 传3参, K&R 定义)。
- sub_8016178: rows/cols 夹取后向 VRAM 0x02005800 填 0xB001 边框。
- sub_8015ED0: 比较 `0x02021000+arg0*0x2000` 前 12B 与 `gSaveSignature` (=0x08098199, 复用)。
- sub_8018750: `gUnk_03000340` 即 `gGstate340` (复用)。
- sub_801A3A8: 关键 —— iwram.h 的 `Unk_03000500` 是 struct 无 array 成员; 用 `u16*` 或 `u16(*)[2]` 转型会被 GCC2 **CSE 成单基址+displacement** (`strh [r0,#2]`), 而目标要**两个独立地址 + r4** (`array[arg0][0]`/`[1]` 各算一次)。解法 = 本地 `typedef union { u16 array[4][2]; }` 转型 → 命中。新登记 ROM 符号 gUnk_0809E4E4/08098308/080936A0。

**挂起 1 — sub_801A2AC** (BLEND 寄存器设置): 逻辑 = `REG_BLDCNT=arg0; REG_BLDALPHA=arg1|(arg2<<8); if((arg0>>6)&2 落在[2,3]) REG_BLDY=arg1;`。range-check 形状来自 `switch((arg0>>6)&2){case 2:case 3:}` (v3 逻辑完全正确)。**卡点**: 目标 `strh r0,[r1]` 把 arg0 留在 r0、`arg0<<16` 放 r3; GCC2 对我方任意写法都 `lsls r0,r0,#16` 先 clobber r0 再 `lsrs r3,r0,#16` 恢复 → 寄存器错位。RTL 转储显示 arg0 的伪寄存器在 <<16 后即 REG_DEAD。permuter 语句序探索平台期 score=240 (非0)。候选文件 `permuter/sub_801A2AC/` (base.c=v3)。待攻方向: 换 arg0 用法让 GCC2 保留 r0 (如把 BLDCNT store 与 mode 计算解耦到不同中间量), 或深挖 -dl 调度。

**挂起 2 — sub_8015AF0** (背包 UI 光标 tile 写入): 无候选, 逻辑已全解 (见上 TSV note)。两处 tilemap 写 (0x020059AA / 0x02005BEA) + `gUnk_08093550[gSaveUiParam*8 + gUnk_03000228 + 4]` 查表。**卡点**: GCC2 把 store 基址 `ldr r2,=0x020059AA` 的调度位置 —— 目标插在 `(bit|0x826)` 之后, 我方版本提前物化基址 → +0xd 起错位。需先登记 gUnk_03000228(IWRAM)/gUnk_08093550(ROM) 符号 (本次为尝试已加又回退, 保持绿)。待攻: 逐条对齐两条 store 的基址/常量物化顺序。

## 2026-09-02 `sub_80446BC` 匹配 (obj kind 音效触发, code_8044394)

108 字节: obj kind(`arg0[0xBE]`)>11 且 `gUnk_03000884==0` 时, 按 `(s8)arg0[0xBC]` 选表列
(==1 → 列 2/3, 否则 0/1), 查 `gUnk_0839DBF6[kind-0xc][col]` 作阈值, `arg0[0x28] >= 阈值` 则
`Sfx_Play(表值, 2, 0)` 并置 `gUnk_03000884=1`。表是 `u16[][4]` (行字节偏移 (kind-0xc)*8 复用一次)。

**关键坑**: `arg0[0xBC]` 的判定目标出**两条** cmp (`==0` beq / `==1` bne), 单写 `if (bc==1)` 只出一条
(差在缺 `cmp #0`)。用 `switch((s8)arg0[0xBC]){case 0:break; case 1:...}` (或 `if(bc!=0){if(bc==1)..}`)
才复现两条。另: `gUnk_03000884` 用**命名符号**才对 (裸地址 `*(u8*)0x03000884` 反让 GCC2 把地址留 r6 多 push, 与 sub_8052AE8 相反 —— 那处裸地址才对, 视压力而定)。
新登记 ROM 绝对符号 `gUnk_0839DBF6 = 0x0839DBF6` (linker.ld) + 本文件 `extern u16 gUnk_0839DBF6[][4];`。
bytecmp 4B(仅 bl Sfx_Play 槽) → fncheck OK 108B, 全 ROM SHA1 绿。

## 2026-09-02 `sub_801EE6C` 匹配 (战斗单位字段处理, code_801A3C4)

120 字节: 读取 (u8*ptr) 字段。`ptr[0xBE] > 0x0B && ptr[0xAB] == 4` 时 v=gUnk_03000744 否则
v=ptr[0x35]; 然后 `sub_801B954(&ptr[0xC])` 结果 u8 给 `sub_804B7B0(v, u8)`; 清 `ptr[0x24]` 的
bit15 (0x8000 → 0x7FFF); 若 `ptr[0xBE]==0x77` 则 `sub_804B834(ptr[0x35], 1, 3, -11, 5)`。

**卡点与解法**: sub_804B834 第 4 参 `-11` 若声明为 u8 会直接写 `movs r3,#0xf5`, 目标要
`movs r3,#0xb; negs r3` → 必须声明为 s32 (带符号才会生成 negs)。bytecmp 后仅剩 3 个 bl
重定位槽 (伪差), 逐字节命中。

fncheck OK (120 bytes, 3 bl 槽忽略)。全 ROM SHA1 绿 (57.0%)。

## sub_8016B30 (0x08016B30, code_8010F10) — ✅ 2026-09-02 op1
道具/状态挂载上限检查: `count = (i = 0); charaId = gPartyMemberIds[0]; for (; i < 5; i++)` 内
`charaId = gPartyMemberIds[i]; 0xFF→break; 非0→charaId-- (表内 1 基转 0 基!); ==arg0→continue(排除本人);
gPartyStats[charaId].field_unk[2]==2 && field_unk[3]==arg1 → count++`。
尾: `count >= gInventory[arg1] ? 0 : 1` (gInventory = gUnk_03004980 别名, 下标=arg1)。
唯一调用点 sub_800B374 (0x0800BD9C, 仍未匹配): r1 = gPartyStats[?].field_unk[3]。

匹配要点 (fncheck 128B 全等, permuter 揭示初始化形状):
1. **`count = (i = 0);` 链式赋值是本函数胜负手** — `count = 0; i = 0;` 两条语句会被 GCC2
   "第二个拷贝第一个"(adds r3,r4,#0), `for (i = 0, count = 0; ...)` 逗号形式会把两个 movs
   **推迟到首条 ldrb 之后**; 只有链式赋值(先 i 后 count 的求值序)能让
   `movs r3,#0; movs r4,#0` 按目标落在 push/参数截断之后、首个池加载之前。经验 27 的反例补充。
2. `charaId--` 前必须有 `if (charaId != 0)` 守卫 (0 保持 0), 对应目标的 cmp/skip 三条指令。
3. 目标字面池放在 `movs r0,#1; b` 与 `movs r0,#0` 返回路径**之间** — GCC2 对该控制流的
   自然布局, 无需手工干预; varG 全指令全等但池在尾部 = 8B 假差, permuter 语句重排后归位。
4. code_0.h 原型 `void sub_8016B30()` → `u8 sub_8016B30(u8,u8)`: GCC2 拒绝空参数表 + 带参定义
   ("can't match an empty parameter name list declaration"), 只能补全原型; 唯一调用者未匹配, 零风险。

## 2026-09-02 `0x08089B90` 场景选择解锁标志表与 `SaveUi_LoadScreen` 命名

`0x08089B90..0x08089BC3` 是 52 字节的 u8 表，不属于前面的 180 项地图场景描述符，
也不属于后面的 `0x08089BC4` BG 滚动参数表。`SaveUi_LoadScreen`（原
`sub_8012790`，`0x08012790`）在场景选择 UI 的多个方向键分支中以选项下标索引该表，
将表项传给 `SaveFlag_Get` 判断场景是否解锁；确认后把选项下标加 `0x82` 传给
`MapScene_Load`。表中前 49 项是解锁标志编号排列，末尾 3 个零是尾部填充。

已将该数据按原地址顺序写入 `src/data_805769C.c` 的 `gSaveMapUnlockFlags[52]`，
在 `include/data_805769C.h`、`linker.ld` 与 `scripts/data.json` 登记，并将 `data/data.s`
的 blob 起点后移到 `0x08089BC4`。`sub_8012790` 已通过改名管线统一为
`SaveUi_LoadScreen`（`ll.cfg`、调用点、asm 切片和原型均同步），函数仍保留原始 asm，
因此函数清单 status 保持 0；`fncheck` 为 4320 字节 OK，整 ROM `make` + SHA1 通过。

## sub_804AB40 (0x0804AB40, code_8044394) — ⏸ 2026-09-02 opencode-1 (17字节差)
扫 ROM 表 0x0839B2E0 数 0xF00 项至 arg0 个; 复位 gUnk_0300094A-D 四字节;
sub_8050434(&tbl[i], 0x6F1E); sub_80187C0(0x400); 返回 &tbl[i]。姊妹函数 sub_804ACC0 同构 (表 0x0839B462)。

**已解** (从前人的 2090 分压到 bytecmp 17 字节差):
1. 循环 = **do-while + 守卫**: `if (i < arg0) do {...} while (count < arg0)` — 顶测 i 一次、底测 count,
   才能得到目标形状 (守卫 bcs + 底 bcc), 写 for/while 都会多测一次 (经验 21 同族, ACC0 note 已提)。
2. 终址必须用 `&gUnk_0839B2E0[i]` (常量伪寄存器与循环 HOT 的拷贝共用 home, 免 r6)。
3. 94 赋值 = **两散 + 一链**: `94A=0; 94B=(94C=0); 94D=0;` — 链使 B/C 地址共享伪寄存器,
   把散写从 4 个地址伪寄存器压到 3 个, r8/ip 的 home 争议因此消掉一半。

**剩 17 字节** = 9 条指令: 入口 4 条 (A/D 的 ldr/mov home 在 r8⇄ip 互换) + 尾段 5 条
(目标 stores=[A,B,C,D] 且 processing=[D,C,A,B]; mine stores=[A,C,B,D] processing=[A,C,D,B])。
**已穷尽** (300+ 变体, bytecmp 实测): 全链 4/3+1/1+3/双链 × 全排列、24 纯排列、8 有序集合划分、
ptr 变量 (vh/wk16 变大 168B)、ret 前置/后置、i/count 声明序、d=0 半独立、permuter 两轮 (~10万次)。
机制推论: 存储序 = 分配序, 目标 [D,ip][C,r7][B,sb][A,r8] 隐含 qty 创建序 D→C→B→A;
但能产生该序的所有 C 写法都同时破坏循环体 home — 是 local-alloc/global-alloc 交互的深层问题
(同 EXPERIENCE.md "global-alloc 域三连" 一类, 穷举 C 写法改不动)。
**最优候选**: permuter/sub_804AB40/base.c (= /tmp/opencode/ab40/vj.c 结构)。下一步候选:
(a) global-alloc 转储 (EXPERIENCE 88 延伸); (b) 用 -g 变体编译试; (c) 等 ACC0 先解 (同构家族互抄)。

## sub_80094FC (0x080094FC, code_8005020) — ✅ 2026-09-02 opencode (逐字节 OK)
调色板特效逐帧驱动: 若 gUnk_03004914 置位, 按 gUnk_03004918&3 选 4 个暂存区之一
(0x0203E600/700/800/900), DMA3 拷 0x80 半字到调色板 RAM 0x05000000+idx*0x100;
清标志、计数器+1。mode==2/7 (白闪) 且计数器超阈值 (0x40/0x20) 时重新断言 WIN0 窗口并复位
gUnk_03004910/gSceneSubState; 计数器到 4 时窗口全开 (WIN0V=0x100, WININ/WINOUT=0x3F)。

**匹配关键 (从 515 分压到 0, 非 volatile 的合法路径)**:
1. switch 必须显式 case 0/1/2/3 + `default: break` (default 不赋值 src, 是死路径)。
2. **零常量复用**: `src=0` 后三处复位用 src → r4 兼作零常量 (与 src 的 r4 同 home),
   否则零落 r3 (经验 87 变量兼职两值的变体)。
3. **打破跨分支 CSE (核心, 见 EXPERIENCE 111)**: 直写 `gUnk_03004910` 三次会被 CSE 成一个
   长命伪寄存器占 r1, 计数器 c 被迫落 r0 (`adds r0,#1`) 差 103B; 把 ==7/==2 读改成
   分支内 `u8 s2 = gUnk_03004910;` 后, 读变短命 → c 落 r1 (`adds r1,r0,#1`)、state 重读 r0, 归零。
4. WININ/WINOUT=0 必须用字面量 (独立 r1 零), 不能复用 r4 零 —— 目标两处零不同 home。
   permuter 3000 轮只会退化成 `volatile gUnk_03004910` (字节对但违反经验 79, 且会打爆
   PaletteFx_Apply/sub_8009370 等已匹配调用方), 弃用; 手动按 greg 诊断找到 1→3 的合法路径。

**验证**: fncheck OK (260B, 0 池重定位); 整 ROM make + SHA1 绿。

## sub_800A534 (0x0800A534, code_8005020) — ✅ 2026-09-02 opencode (逐字节 OK 304B)
装备加成结算: 按 gUnk_087EA580 的 12B 角色条目, 把 +8 防御字节的低4位-1 / 高4位-1
分别选一个装备加成栏 (0=AtkBase 1=Def2 2=Agl 3=Men 4=Res 5=Noa 6=Luc) 累加
+6(HP) / +7(攻击); ID 在 [0x22,0x2B] 或 [0x37,0x3E] 时 Noa 额外 +1。

**匹配历程 (505分 → 0分)**:
1. switch 用 `if (v <= 6) { switch(v) }` 守卫 + jump table 直接命中结构; `u32 v`
   才有无符号 bhi (u8 会加 lsls/lsrs 截断, s32 变 bgt)。`default: break` 写法会
   落进 add, 错。
2. 13 字节残留 = 纯 home (tbl r2↔r3, val r3↔r2) + val 装载/subs 顺序。
   先试 `register` 定 val→r2/v→r0 + 第一分支 `bonusVal = val` 副本补齐 home;
   应 reviewer 要求弃 register (编译器扩展), 改成第一分支内 **重读**
   `u8 bonusVal = tbl[6]` (CSE 合并成 val 副本, 不增指令) —— 同样把 val 生命周期
   缩短到 switch 之前, 使 val 全局分配优先级反超基址 → val 落 r2/基址落 r3, 仍是 0 分。
3. 最后 8 字节 = 调度顺序: 目标 `ands; ldrb val; subs` vs mine `ands; subs; ldrb`。
   **正解 (经验 112)**: `v = tbl[8]&0xF; val = tbl[6]; if (v-1 <= 6) switch(v-1)`
   —— 把 -1 拆到守卫表达式, val 装载落进 ands→subs 空隙, 逐字节命中。

**验证**: fncheck OK (304B, 16 池重定位); 整 ROM make + SHA1 绿 (610/1065)。

## sound.c 三函数 INCLUDE_ASM→真C (SoundTracks_Frame / Sfx_Play / Sfx_PlayFade) — ✅ 2026-09-02 sound-agent
三函数在 TSV 早已 status=1 (asm/matchings 直通), 本次把占位 INCLUDE_ASM 换成真 C 实装,
字节级与 baserom 完全一致 (fncheck OK: 232B/116B/120B, bl 槽忽略)。

**要点**:
- SoundTracks_Frame 是 4 音轨 SFX 状态机: 每帧查 active 位, 音轨 status==0 时按 loop/fade
  位重启或淡入。编译器用了 r8/sb/sl 高寄存器 (循环 i, &active 常量, 1 常量) — 写对
  `(u16)status == 0` (ldrh) 与 `i <= 3` (u8) 后天然复现, 无需特殊处理。
- Sfx_Play / Sfx_PlayFade 同构: MPlayStart + active 位置位 + song id 登记 +
  loop/fade 位条件清位再或入; PlayFade 尾调 m4aMPlayFadeOutTemporarily(bgm player)。
- 新登记符号: `gSfxTrackSongIds`@0x03000F48 (u16[4], linker.ld IWRAM + iwram.h),
  `gSongHeaderTable`@0x087ED910 (ROM 绝对符号, BGM/SFX 共用歌曲头指针表)。
- 首次 make 曾红 (+32 位移) 系并发 agent 编辑 code_8044394.c/code_8005020.c 所致;
  与本改动无关 (stash 本改动后依旧红, 对方提交 efd9039 后恢复)。fncheck 定论为准。

**验证**: 三函数 fncheck OK; `make`+SHA1 在无并发干扰时全绿。

## sub_804C728 (0x0804C728, code_8044394) — ✅ 2026-09-02 sound-agent (逐字节 OK 100B)
0x03000BE8 16B entry 表批量更新: 对 [arg0, arg0+arg1) 每项, 若 `(field_0 & 0xF) == 3`
则 `field_0 |= 0x40; field_2 = arg2; field_3 = 0`。

**要点**:
- 与已匹配 sub_804C4D8 完全同构 (r8/ip/sb 高寄存器逐条一致), 仅基址 0x03000BE8 vs 0x03000AE8;
  直接套用其结构体成员写法即可, 天然规避经验 11/67 (IOR 目的寄存器选错)。
- 零 bl 调用, bytecmp 100B 完全一致 (含池重定位)。
- code_0.h 原型 `()` → `(u8,u8,u8)` (同 sub_804C4D8 约定; 无 C 调用点, asm 调用点字节固定不受影响)。

**事故**: 第一次 edit 后 source 被并发 agent 恢复为 INCLUDE_ASM (编辑前读到的是其改动前版本),
第二次重读+重放编辑成功。教训: 多人共改 code_8044394.c 时 edit 后立即 grep 确认。

**验证**: fncheck OK 100B; 全 ROM make+SHA1 绿 (612/1065)。

## sub_804C78C (0x0804C78C, code_8044394) — ✅ 2026-09-02 sound-agent (fncheck OK 260B)
SFX 调度器: sub_804DE8C() → 遍历 obj 池 (GetObjPool + sub_80489E8 收集 count 个 id) →
对每个 obj=pool+id*0xC8, 若 sub_8045F10(obj,0x20)==1 则按 obj[0xBE] (0..10) 跳表分派
到 sub_804CA2C / CAA0 / CB18 / CB8C / CC00 / CC78 / CCEC / CD60 / CDD4 / CE48 → sub_804EF50()。

**要点**:
- 11 项跳表 switch 由编译器天然生成 (fndiff score 0, 含跳表数据字); bytecmp 因 VMA0 链接
  对嵌入式跳表条目误报, 以 fndiff/fncheck 为准。
- 目标分派函数在 code_0.h 均为 1 参原型 `(u8 *)`, 但调用点传 2 参 (冗余 values[i])。
  用函数指针强转 `((void (*)(u8 *, u8))f)(obj, values[i])` 复现调用点字节 (同文件
  sub_804C890 既有约定)。
- **并发事故**: 本次改动的文件里, 另一 agent 正并行把 sub_804C890 从 INCLUDE_ASM 转真C
  (工作区未提交), 其 WIP (两种写法) 均编译出 76B≠baserom 80B, 导致整 ROM 红。
  处置: 用 `git checkout HEAD --` + 只插自己函数的方式构建验证 HEAD+自己=绿,
  提交时只 `git add src/code_8044394.c` (文件已 checkout 到仅含自己改动),
  事后把对方 WIP 副本拷回工作区, 不回退不代修。

**验证**: fncheck sub_804C78C OK 260B (12 池重定位, 15 bl 槽); HEAD 全 ROM make+SHA1 绿。

## sub_804C9B4 (0x0804C9B4, code_8044394) — ✅ 2026-09-02 sound-agent (fncheck OK 120B)
SFX 换歌: 遍历 obj 池收集 id, 对 obj=pool+id*0xC8, 若 obj[0xBE]==9 则清零 obj[0xBC],
重取池 (GetObjPool + sub_80489E8(values,1,0x7F)), obj[0xBD]=values[Rng_LcgNext()%count], **break 整个循环**。

**要点**:
- 直接 `obj[0xBD] = values[...]` 时编译器把 LHS 地址 (obj+0xBD) 提前算进 r4, 与目标
  RHS-先算 (r1 存值, 再算地址) 不符 (r4/r5 分配互换, 差 4B)。
  **用 `value` 临时变量先把 RHS 算完再赋值**, 编译器便按目标顺序生成 (score 0)。
- 跳表区: 目标分派函数在 1 参原型下传冗余第2参 (见 sub_804C78C 记录)。
- 整文件被并发 agent 反复 checkout/编辑, 提交前用 `git checkout HEAD --` + 只插自己函数
  构建验证 (HEAD+自己=绿), 提交只 `git add` 该文件, 事后恢复他人 WIP。

**验证**: fncheck sub_804C9B4 OK 120B (6 bl 槽); 与 sub_804C78C 同 build 时全 ROM 绿。

## sub_8009F70 (0x08009F70, code_8005020) — ✅ 2026-09-02 opencode (fncheck OK 216B)
属性成长查询: `(职业 classId, 等级 lv, 属性序号 statIdx)` → 该等级属性值 (u16)。
调用点 sub_800A3C8 (队伍角色逐属性, c=0..8) 与 sub_8048818 (战斗对象 formation, 返回
`gPartyStats[idx].lv` 属性)。三张数据表: gClassStatCurveTable (9×8 职业×属性→曲线号 t,
0x080921F0) + gStatGrowthCurveTables (每曲线 100B 逐级增量, 0x080923D8)。

**要点 (全部逐字节验证)**:
- **跳表复现**: 首格 `statIdx>=8 && classId<=10` 内 switch(classId) 若把 11 个 case 写成
  `case 0: case 1: ... case 10: return 10;` 会被 GCC2 折叠成范围测试 (cmp/bgt/blt + 单
  return), 丢跳表。**必须每个 case 独立写 `return 10;`** 才生成 11 项跳表 (全部指向同一块)。
  (对比 sub_8048BD0: 只要有两个不同目标就会出跳表; 本函数 11 个目标相同, 靠独立语句强出。)
- **分步索引**: `stride = t*100` 命名变量提前算 → `movs r0,#100; adds r5,r1; muls r5` 落
  在 sum/i 初始化之前 (经验 30 分步形式); 直接写 `tbl2[t*100+i]` 会把 tbl2 基址提前
  hoist 到 r7 (多 push, 差 r4/r2 分配)。
- **循环守卫**: `while (i <= lv)` 比 do-while 更能复现入口 `cmp r2,r1; bhi` 守卫 (r2=sum
  恰为 0); 配合 u16 累加/自增 (lsls/lsrs 掩码)。
- **首格条件**: 必须写 `statIdx >= 8` (u8 归一化成 `cmp r4,#7; bls`); 写 `== 8` 变
  `cmp #8; bne` 不匹配。
- **定义必须 K&R 旧式**: 头文件保持 `u16 sub_8009F70();` (空形参), 定义用
  `u16 sub_8009F70(a,b,c) u8 a; u8 b; u8 c; { ... }`。理由: ① 全原型与 `()` 声明触发
  GCC2 default-promotion 冲突报错; ② 就算改成全原型能编译, 会让已匹配的调用方
  sub_8048818 的 formation 从 r2 漂到 r3 (经验 7 的坑, 差 12B)。K&R 定义字节与原型完全一致。

**验证**: fncheck OK 216B (14 池重定位); 全 ROM make+SHA1 绿 (615/1065)。

**事故**: 验证 `git checkout HEAD --` 复原 sub_8048818 时, 连带把并发 agent gpnux
(sub_804C890, 进行中) 在 src/code_8044394.c 的未提交真 C 转回 INCLUDE_ASM。其 WIP
(76B≠baserom 80B) 造成整 ROM 红, 复原后 ROM 反而转绿; WIP 副本完好保存在
permuter/sub_804C890/{base.c,v2.c}, functions.tsv 仍 status=0。见 INCIDENTS.md 新增行。


## sub_804C8E0 (0x0804C8E0, code_8044394) — ✅ 2026-09-02 sound-agent (fncheck OK 210B)
obj 池槽位操作: 从 sub_80489E8 收集的 values 中删除 arg1 (移位), 槽空则切换 slot (0→1)
再试一次, 返回 values[Rng_LcgNext()%count]。

**匹配历程 (3200→2780→0)**:
1. `slot = !slot` 编译成 r0 计算+拷贝; 改显式 `if (slot==0) slot=1; else slot=0` (2780)。
2. 残余结构差在移位循环: 直接 `count--; for(j=i; j<count; j++)` 会立即截断 count
   (subs+lsls+lsrs), 目标把 count-1 复制到 r3 作循环界、r4 作备、延迟到循环后 `count=(u8)r3` 截断。
   **正解 (经验 115)**: `for(j=i; j<count-1; j++) values[j]=values[j+1]; count--;`
   —— count-- 后置、循环界用 count-1, 编译器便按目标调度, obj 顺带落到 r8
   (需要 r7 作移位基址), 全程 score 0。
- 两段扫描-移除逻辑完全同构 (槽0→槽1), 直接复制结构。

**验证**: fncheck OK 210B (4 bl 槽); 全 ROM make+SHA1 绿 (615/1065)。

## sub_804D0F8 (0x0804D0F8, code_8044394) — ✅ 2026-09-02 sound-agent (fncheck OK 188B)
obj 槽位填充: 守卫 `*(u32*)(*(u32*)(obj+0x88)+0x1C)==0` 时, obj[0xBC]=0, 取池收集
values (slot=1, 若 count<=1 换 slot=0 重收), 移除首个 `obj[0xAC]` 匹配的池条目
(移除-移位循环, 经验 115), obj[0xBD]=values[(u32)(u8)Rng%count]; 否则 obj[0xBC]=3。

**要点**:
- 直写 score 215, 唯一差异是目标把 `movs r4,#0` 外提到守卫指针解引用之前
  (r4=count home, 供 obj[0xBC]=0 使用), 我的 0 从守卫已知零值 r1 复用。
  **正解: 声明 `u8 count = 0;`** —— 初始化把 0 装进 count 的 home 寄存器并提前调度, score 0。
- 取模是 `__umodsi3` (无符号): 需 `(u32)(u8)Rng_LcgNext() % count` 形式
  (`(u8)` 截断 + `(u32)` 强制无符号), 否则 u16%u8 提升成有符号出 __modsi3。
- count<=1 的槽切换: `cmp r4,#1; bhi` (u8 无符号>1 跳过换槽)。

**验证**: fncheck OK 188B (5 bl 槽); 全 ROM make+SHA1 绿 (616/1065)。

## sub_804D1B4 (0x0804D1B4, code_8044394) — ✅ 2026-09-02 sound-agent (fncheck OK 170B)
obj 槽位概率填充: count=sub_80489E8(arg1,values,0,0x6F); obj[0xBC] = (Rng%0x65 < count*15)?
1:0; switch((s8)obj[0xBC]) 选 0x08393B28 表条目 (case0: obj[+0x88] 指针 [2]; case1: 清
obj[0xC2]+[8]), switch(entry->field_10): case0 → obj[0xBD]=values[(u8)Rng%count];
case1 → obj[0xBD]=0。

**匹配历程 (3730→3195→1080→620→400→0)**:
1. `(u32)Rng_LcgNext()` 会给 u16 返回加 lsls/lsrs 规范化 (0x3730 差); 改
   `((u32 (*)(void))Rng_LcgNext)()` 直取 u32 → 首处模不再截断 (0x3195)。
2. `obj[0xBC] = cond?1:0` 三元表达式把地址/常量外提, 多占 r7/r8 (3195);
   **if/else 双语句** → 目标"分支内 fresh 地址"复现 (1080)。
3. entry 指针带 +0x10 (cast) 会被编译成 `adds r0,#0x10; ldrh [r0,#0]`, 目标要
   `ldrh [r6,#0x10]` 折叠 → **新登记同址别名 gUnk_08393B28_entries + 结构体
   field_10 成员访问** (EXPERIENCE 109: 禁 cast 用别名), 折叠命中 (400→0)。
4. 尾部 obj[0xBD] 用 value 临时 (RHS 先于地址, 经验 115 配套)。

**验证**: fncheck OK 170B (5 bl 槽, 1 池重定位); 全 ROM make+SHA1 绿 (617/1065)。

## sub_8013870 (0x08013870, code_8010F10) — ⏸ 2026-09-02 opencode (53字节差)
文本块绘制: ClearBuffer 内联填 0xB001 空白 (0x02005800, 0x1E 宽 × 0x14 高, 行距 0x40B),
再按 TextBlocks_Render 格式画 0x08098622 的字符串, 尾部 Text_TileAt(0xC,7)+Text_WriteChars(0x08098858)。

**已解**:
1. 填零 = 调用 inline `ClearBuffer((u16*)0x02005800, 0x1E, 0x14)` (SaveUi_Open 同款, 逐字节同构:
   入口测试 + `0x1E0000>>16` 物化内界 + 值逐外层迭代装载)。
2. 字符串段与 footer **逐字节一致** (与 TextBlocks_Render 结构相同, 含 0xFE 转义)。

**卡点 = 填零循环的全局寄存器 home 轮换 (经验 17/88类)**: 目标六值分配
{src:r8, 值:r4, 行距:r5, h:r6, w:r9, 0x1E0000:ip}, prologue 存 r8+r9 两个高位。
- v16/src 顶处赋 (早载): 字符串全对, 但填零 src→r4/值→r5/行距→r6/h→r9/w→ip/0x1E0000→r8 环形置换, 差 53B。
- v13/src 后赋 (晚载): 填零 home 全对 (值→r4 等), 但 src 晚载使 prologue 少存一个高位、串段整体平移 4B, 差 ~178B。
- 需要"src 早载入 r8 且不占 r0-r7"的两全分配, 穷举 20+ 变体 + permuter 三轮 (~5万次) 未果。

**候选**: permuter/sub_8013870/base.c (= output-555, 53B) + cand_v13_fillmatch.c / cand_v16_strmatch.c。

## sub_8011268 (0x08011268, code_8010F10) — ⏸ 2026-09-02 opencode (205字节差, 纯寄存器分配)
技能菜单物品页绘制: ClearBuffer(0x02005AA0, 8, 6) + ClearBuffer(0x02005AB6, 2, 6) 清两块,
再按 gSkillMenuPage 从首页起逐页 Inv_FindHeldItemOnPage 找持有物品, 最多画 3 件
(名字 8 字形 + gInventory[item] 数量 sub_800EAE4)。

**已解** (329B → 205B):
1. slot 前置量 `slotX = slot*2` 先存再用 → 计数 dest = 0x02005AB8 + slotX*64 (不能写
   `(slot-1)*0x80`, 会被折叠成 0x02005A38+slot*128 差基址)。
2. x 位置是 u8(slotX+0xA), 位移在用到处 `x<<6`, 不能提前移位截断。
3. 字形循环两写法等价 (do-while 底读 / for i<8 顶读+ch==0 break, 均 205B), MenuUi_DrawItemList
   (已匹配孪生) 用的是 for 顶读版。
4. page 用 u16 + 调用处 (u8)page 截断才无额外掩码。

**卡点 = 主循环寄存器 home 全面偏离**: 目标 page→r4/slot→r5/item→r7/i→r6 (低位 callee-saved),
高位 r8/r9/sl/sb 只装 page+1/&gSkillMenuPage/slot+1/slot*2 临时量; 我方 page→r8/slot→r9 反向占高位,
palette 用 r9(spill) 而非目标 r3+栈槽。穷举声明序/类型/循环形/permuter 均停在 205B, 属经验 17/88 深分配问题。

**候选**: permuter/sub_8011268/base.c (= v9, 205B) + cand_v7_205B.c。

## 2026-09-02 `sub_80256E4` 匹配 (tilemap 调色板覆写循环, code_8020D50)

136 字节: 遍历 row 从 `gUnk_03000781` 起, 条件 `row < (s8)gUnk_03000781+3 && row < gUnk_03000770`,
每行调 `BgMap_PalFillRect(base, palette, 8, (row-start)*2+8, 9, 2)`。palette 依据
`(gUnk_03000784 >> row) & 1` 和 `row == (s8)gUnk_03000782` 取 0xc/0xb/0xd。

**卡点与解法**: 
1. palette 掩码 `movs r1,#1`(与初值 1 复用) 的调度位置: 目标在 `ldr/ldrh/asrs` 后才 movs。
   用 permuter 探索出 **中间变量 `bits = gUnk_03000784 >> row;`** 前置位提取, 再 `if (bits & 1)`
   使掩码后置 → 逐字节命中 (score 20, bytecmp 仅剩 bl 槽)。
2. 起始 `for` 三条件 (初值/上界/步进) 直接书写即可, 不需 do-while (for 无入口旋绕)。
3. 新符号 `gUnk_03000784` (u16, 位掩码表) 登记 linker.ld + iwram.h。
4. BgMap_PalFillRect 保持 K&R 无原型调用 (被调截断由定义侧提供)。

fncheck OK (136 bytes, 1 bl 槽忽略)。全 ROM SHA1 绿 (58.3%)。

## sub_804FA04 (0x0804FA04, code_804F0B8) — ⏸ 挂起 (loop rotation 已解, 尾部寄存器分配)
条件跳转 script opcode: 数据块 = [1]字节数(>>1 个条件), 每条件 u16 id 于 data+3+i*2;
id≤0x1FF 测 EventFlags_Test(id), >0x1FF 测 SwitchFlags_Test(id-0x200); 若任一命中 → 
`*ptr += b+3`(跳过本命令), 若全 0 (或 count==0) → `*ptr = gUnk_02016200+gUnk_02016000[data[2]]`
(跳转表)。返回 1。

**匹配历程 (3215→2395→1895→1475)**:
1. id 需**逐字节装载** `data[3+i*2] | data[4+i*2]<<8` (偏移 3+2i 为奇数, u16 cast 会 ldrh 误对齐)。
2. 结果块序: 目标 `if (result == 0) {notfound} else {found}` (notfound 直落), 反写会调块。
3. **循环旋转 (经验 116)**: `for(i=0; i<count; i++)` 生成未旋转 `cmp i,count;bcs`; 写成
   `for(i=0; count>i; i++)` 触发旋转 → `cmp count,#0;bls` + do-while 回边, guard 逐字节命中 (1895→1475)。
4. result 不初始化 (count==0 路径读 r1 垃圾, 目标如此, 是原始 UB 伪影)。
5. **残余卡点 (~30B, 尾部两体)**: not-found 体目标 lsl 先于 base1 ldr 且 base2 用 r2 (我 ldr 先 + r1 复用);
   found 体目标先算 `b+3`(r8→r1→adds#3) 再读 *ptr (我 (*ptr+3)+b)。穷举 temp/slot/显式括号/换 if 序/-g/
   permuter 全撞 1475-2050。属调度+寄存器分配耦合, 候选 permuter/sub_804FA04/base.c。

**验证**: 无 (fncheck 未达 0)。

## 2026-09-02 `DialogPortrait_Set` (0x08008620, code_8005020)

按 `scripts/data.json` 地址回溯确认：`0x087E9554` 是 88 项头像图形指针表，
`0x0808716D` 是头像到调色板的 89 项索引，`0x080798A8` 是 16 色头像调色板，
`0x087E9818` 是 4 个对话框 tilemap 目标指针。函数的 `portraitId=1..0x58`
选择资源并设置待上传的图形/调色板，按 `position&2` 选择 `0xE280` 或 `0xF2C0`
的 tile 起点，写入 8×8 tilemap（行跨度 0x30 字节）；`portraitId=0` 则清空对应窗口。
反汇编未匹配段已替换为 `DialogPortrait_Set`，220B `fncheck` 通过。

同区域字节边界复核发现：`0x08058864` 的方向映射实际为 24B，`0x0805887C`
从下一字节开始是独立的 16B OAM tile 数表。修正 `gWalkDirectionMapping` 的 C 定义，
移除多出的尾部 `0`，使源码与 `data.json` 的 24B/16B 分割一致；`make` 与 SHA1 均通过。

## 2026-09-02 宝箱对象与选择组表命名收尾

按 `data.json` 地址和消费者访问方式复核后，宝箱运行时记录统一命名为
`ChestObject`（独立的 0x08 字节记录），数组为 `gChestObjects[16]`；可脚本寻址、
0x28 字节的活动实体继续统一使用 `Actor`。对应的地图 ROM 记录命名为
`ChestSpawnEntry`，装载、精灵构建和开启函数分别为 `ChestObjects_LoadForMap`、
`ChestObject_BuildSprite`、`ChestObject_Open`。

`scripts/data.json` 中原本从 `0x0808823A` 拆出的 1B/167B 两项实际是同一张
84 项 × 2B 的 `gChoiceGroupPairTable`；合并后其末端落在 `0x080882E2`，再与后续
286B 不透明表连续搬移至 0x08088400，避免 `.rodata` 的 `SUBALIGN(4)` 插入填充。
三个宝箱函数的 `fncheck` 均通过，构建和 `sha1sum -c ll.sha1` 均通过。

## 2026-09-02 CheckFacingEvent 草稿复核

`CheckFacingEvent`（0x08003F40）检查玩家朝向矩形内的特殊事件、`Actor[2..18]`
与 16 个 `ChestObject`，命中时返回交互 ID 加一；特殊事件则治疗队伍、安排角色
切换并返回零。m2c 草稿与旧草稿的语义及大部分指令形状已验证。当前最接近候选是
`permuter/CheckFacingEvent/output-1070-1/source.c`，入口和 Actor 循环基本同构，但宝箱
路径及全局分配仍不一致，未通过字节验证，因此恢复为 `INCLUDE_ASM("asm/nonmatchings", ...)`。

## 2026-09-02 sub_8016FC0 匹配 (Multi-SIO 串行 IRQ, code_8010F10)

252B 一次性合入 (零迭代), 依据是用户提供的"已匹配参考 C"(同为 agbcc 产物), 直接移植即 byte 相同。
语义: 读 `REG_SIOMLT_RECV`(0x4000120, 64 位) 到栈 recv[4] → 取 SIOCNT Error 位入 `errorFlags`;
收到 0xFEFE 同步头且接收列计数 `unk_18 > 0xD` 时复位(`unk_18=-1`)并交换 unk_28↔unk_24 接收双缓冲,
若 unk_4 挂起再交换 unk_20↔unk_1C 发送双缓冲并清零 unk_4/unk_14, 然后关 IME 置 0x3007FF8 bit7 再开;
随后 `unk_14<0xE` 时把 `((u16*)unk_20)[unk_14]` 写入 SIODATA8(0x12A), 计数推进到 0xF;
`unk_18>=0` 时把 recv[0..1] 按行写进 `unk_24[var][unk_18]`(每行 0x20B=16 u16), 列 0xD 置 unk_5;
`isParent` 时 TM3CNT_H=0 关节拍, 且 `unk_14<0xF && isParent` 时拉 SIOCNT bit7 并启 TM3 (0xC0)。
最后置 `sioInterrupted=1`。

关键代码生成规律 (入 EXPERIENCE 119):
- SIOCNT/SIODATA8 必须按 `((SioMultiCnt*)REG_ADDR_SIOCNT)->Data` 双 u16 结构视图写, 才出
  `ldr rN,=0x04000128; strh rX,[rN,#2]` 且基址池字面量与 Error 读/终段 OR 三处共享同一 0x04000128;
  io.h 分开的 REG_SIOCNT/REG_SIODATA8 → 池多一字面量差 4B。
- 必须非 volatile `SioMultiCnt`: `vSioMultiCnt` 把 .Error 位域读拆成半字访存, 破坏目标
  `ldr word; lsls #0x19; lsrs #0x1f` (差 137B)。
- `.Error` 位域非 volatile 读正好被 agbcc 扩成 word load + 双移位, 与相邻已匹配的
  `(*(vu32*)REG_ADDR_SIOCNT << 25) >> 31` 同形。
- 寄存器结构视图用**同址别名符号**: 新增 `gUnk_03004DF0`(类型 `Unk_03004DF0`, 见 iwram.h)
  与 `gSioState`(u8[], 老函数用) 同址 0x03004DF0。绝不用 `#define OBJ (*(struct*)0xADDR)` 宏
  (差 160B, 池字面量激增); 局部指针 `= (struct*)gSioState` 也不行 (差 185B)。

## 2026-09-02 `Stats_RebuildEquipBonuses` 匹配 (sub_800A664, code_8005020)

函数按 1-based 角色编号定位 `PlayerStats`，清零九项全局装备加成，依次调用
`sub_800A534` 重算六个装备槽；随后读取前四个装备对应的 `gUnk_087EA580`
表项，若高 nibble（形态类别）一致，则对 `0xE`/`0xF` 形态写入团队攻防加成。
目标与候选的主体指令逐条一致，工程 `fncheck` 结果为 310B、11 个池重定位、
6 个 `BL` 槽忽略。候选过程中修正了 `PlayerStats` 步长为 `0x40`，并沿用已匹配
`PartyForm_ApplyBonus` 的 `u8 val = entry[4] & 0xF0` 形状，得到目标的
`lsls #0x18`/`lsrs #0x1C` 和 `0xE` 优先分支布局。

## 2026-09-02 `sub_80454A4` 匹配 (队伍 EXP 发放, code_8044394)

**语义**: 给参战队伍成员发 EXP。遍历对象池前 5 项 (0xC8/项), 跳过
`obj[0xAB] ∈ {7,8}` 与 `obj[0xBE]==0xFF` 的空槽; 对有效对象按
`sub_80487A4(i)` (→ `gBattleFormationIds[obj[0xBB]]`) 映射到 `gPartyStats[idx]`,
`idx` 非 0 时减 1; 若 `lv <= 0x61` 则 `exp += amount`; `exp >= next_exp` 时升级:
`result |= 1<<i`, `next_exp = LevelToExp(ExpToLevel(exp))`, `lv = ExpToLevel(exp)`;
再压 `exp` 上限 `> 0x98967F → exp = LevelToExp(0x61)`。返回 u8 升级位掩码。

**匹配要点**:
1. **首循环死读**: 第一个 `for(i=0..4)` 只做 `if ((u8)(obj[0xAB]-7) > 1) idx = obj[0xBE];`
   且 `idx` 结果被丢弃 (目标 `ldrb r0,[r0]` 后直接 `adds r0,r6,#1` 覆盖)。穷举 40+ 非 volatile
   形态全部被 DCE 删掉; 唯一逐字节一致是 `((volatile u8 *)obj)[0xBE]`。用户拍板: 因死读在 ROM
   真实存在 = 原代码必然是 volatile 读, 属忠实还原, 允许破例 (经验 121, 与 OAM 先例区分)。
2. **`new_var = arg0;` 放在第二个循环前** (permuter 找到, score 5): 让 `arg0` 先落 r4
   (入口 `lsrs r4,r0,#0x10`), 到循环 2 前 `mov sl,r4` 再进 sl —— 复现目标 prologue
   `mov r7,sl; mov r6,sb; mov r5,r8; push {r5,r6,r7}` 的三高位保存。
3. **直写 `gPartyStats[idx].field` 而非 stats 指针** (经验 11 形态): 目标逐字段独立寻址
   `adds r0,r3,#0; adds r0,#0x38; adds r4,r2,r0`, 用指针会折成 `[r4,#0x38]` 单基址。
4. **`LevelToExp(ExpToLevel(exp))` 中间必须落 u8 临时变量** (`u8 newLevel`), 否则少
   `lsls/lsrs #0x18` 截断 (差 4B)。
5. 升级条件 `exp >= next_exp` 用 `bcc` (无符号小于) 而非 `blo`, 已按目标对齐。
6. `result |= (u8)(1 << i)` + 返回前 u8 截断 (`lsls/lsrs`), 匹配 r8 逐位累积。

**收尾**: fncheck OK 252B @0x080454A4 (1 池重定位, 6 bl 槽忽略); 原型 `void sub_80454A4()`
→ `u8 sub_80454A4(u16)` (code_0.h, 经验 93; 无 C 调用方, 安全)。全 ROM SHA1 仍红 =
并发 agent 的 in-progress 改动 (+4 整体位移, sound_data 等), 非本函数。

## 2026-09-02 `sub_80488CC` 匹配 (Actor 技能槽查询, code_8044394)

函数按技能 ID 查询 Actor 的 8 个技能槽，返回可用槽索引；普通技能 ID
扫描 `obj+0x99`，特殊 ID（大于 `0x2F`）则从 `obj+0x88/0x8A` 读取结果。
命中普通技能后调用 `sub_8045A10` 检查技能资源是否足够，不足时返回 `0xFF`。
关键写法是将首槽读值保存为独立的 `first`，再建立 `skills = obj + 0x99`，从而
复现目标的 `r5/r6/r7` 寄存器分配。源码和原型已合入，`fncheck` OK（104B）。

## 2026-09-02 数据地址复核与状态命名

按 `scripts/data.json` 复核选项菜单相邻数据后，确认 `0x0808823A..0x080882E2`
是连续的 84 项 × 2B `gChoiceGroupPairTable`。虽然只有项 0 有独立字面池引用，
`ChoiceMenu_HandleInput` 随后以表基址每次前进 2B 扫描项 1..83，不能把后续项当作
未使用数据。`0x080882E2..0x08088400` 重命名为 `gChoiceMapSpawnRecordStream`：
这是按五组目的地选择地图出生位置的 8B 记录流，不是任意 blob；5 组记录数为
5/7/9/9/5，组终止字节为 `0xFF`，整体末尾再以 `0x00` 收尾。

为调色板特效状态补充统一名称：`gPaletteFxMode` (`0x03004910`)、
`gPaletteFxPending` (`0x03004914`) 和 `gPaletteFxTimer` (`0x03004918`)。
其中 Pending 由 `PaletteFx_Transform` 置位、由 `PaletteFx_Step` 清除；本轮只改名和
注释，不改变 ROM 数据及函数机器码。旧的 `gUnk_08087648`、`gUnk_08088D80` 仅剩
linker 绝对声明，已移除，源码分别使用 `gChoiceDestTable`、`gMapSceneDescriptors`。

## 2026-09-02 sub_805008C 匹配记录 (脚本泵逐帧后台服务, 300B exact)

函数语义: `ScriptPump_Run` 的帧级姊妹服务, 由 `VBlank_UpdateGameScreen` 末尾
(`gLogoEffectState == 0` 时) 每帧调用。前半在 VM 活动 (`E70&1` 且非 `&0x200`) 时
处理窗口 BG: 当前 opcode ∈ {0x00, 0x17} 且无 bit4 请求时, 主动做一次
`REG_BG0HOFS/REG_BG0VOFS = 0` + `DmaCopy16(3, 0x02005800 → 0x0600F800, 0x800)`
(窗口缓冲整屏上屏); 若 bit4 置位 (Op_OpenWindow 请求) 则再做一次。后半为四个
独立的请求位消费者: bit6 → `FlushTileDma() < 0` 时清位; bit8 → `BgTiles_LoadSet(0)`
后清位; bit9 → `LZ_UncompressChunk() == 0` (流式解压完成) 时, 若 bit10 置位则
`gUnk_03000E6C = gUnk_02016200 + gUnk_02016000[gUnk_03000E69]` (脚本 PC 跳入解压
缓冲的入口表项) 并清 bit10, 再清 bit9。

非平凡发现:
- **0x04000010 是 `REG_BG0HOFS` 不是 BG2PA**。目标里 `ldr =0x04000010; strh; adds
  r0,#2; strh` 极易凭记忆误判成 BG2PA/PB (0x04000020); 实为 BG0HOFS/VOFS。
  与 `VBlank_UpdateScreenSimple` 开头八连清同族, 寄存器地址必须查 io.h。
- **条件值 u16 局部 + 同寄存器复用**: 目标在 bit4 测试处有 `ands r0,r2; lsls #16;
  lsrs #16` (uxth) 且随后 `strh r1` 复用同一寄存器, 说明源码把条件值存进了 u16
  局部并在写 IO 时复用 (`u16 bgRequest = gUnk_03000E70 & 0x10; ... REG_BG0HOFS =
  bgRequest;`)。写成常量 `REG_BG0HOFS = 0` 大概率也可 (该分支 r1 恒 0), 但 u16
  局部形状与目标逐指令一致, 一次通过。
- 新符号登记: `gUnk_03000E69` (u8, IWRAM, 脚本槽/场景索引, 选 gUnk_02016000[]
  入口偏移表项) + `LZ_UncompressChunk` 跨 C 文件 原型 (此前无任何声明, 定义在
  code_80002A0.c)。
- 本轮全 ROM SHA1 红, 经 `fncheck --blame` + worktree 对照归责: 并行 agent 的
  数据侧 WIP (data_805769C de-blob / 调色板符号改名) 使数据布局 +2/+4 位移,
  代码区差异均为池常量中指向被移数据的指针 (fncheck 池重定位归一后单函数全 OK),
  与本函数无关, 照常提交。另: HEAD 上 functions.tsv 的 sub_804F974 note 曾断行
  成无列首行, 会让 gen_asm.py 解析崩溃 (fresh checkout 无法构建), 本次提交附带
  修复该行。

## sub_804FA04 (0x0804FA04, code_804F0B8) — ⏸ 续攻记录 (zai 接管, 30B→11B, 剩 3 真实字节)
接管 sound-agent 的挂起认领。基线 1475 (fndiff)/30B (bytecmp) → **11B, 其中 8B 是两个 bl 槽,
真实差异仅 3 字节** (0x68/0x6b/0x6c), 全部集中在 not-found 体尾部三指令的寄存器指配。

**关键突破 — 共享 newval 形态 (permuter/sub_804FA04/base.c)**:
```c
u32 newval;                      // 函数顶声明
if (result == 0)
{
    u32 value = *(u16 *)((u32)gUnk_02016000 + data[2] * 2);
    newval = value + (u32)gUnk_02016200;
}
else
{
    u32 t = b + 3;
    newval = *ptr + t;
}
*ptr = newval;                   // 共享 store → sum 成跨块全局伪寄存器
```
found 体与 ROM 逐字节一致 (`mov r1,r8; adds r1,#3; ldr r0,[r6]; adds r0,r0,r1`)。

**本轮确认的编译器机制 (对同族 sub_804F974/sub_804FA94 同样适用)**:
1. agbcc **没有调度 pass** (无 sched dump); 指令顺序 = expand/regmove 顺序, 常量池装载
   由 CSE 生成伪寄存器装载 (insn 紧邻消费者), 位置天然正确 — 位置从来不是问题。
2. **cross-jump 在 reload 之后运行** (toplev.c: 全局分配/reload → thread_prologue_epilogue →
   jump_optimize(JUMP_CROSS_JUMP)) — 按硬寄存器合并尾指令。not-found 尾 `adds r0,r0,rX`
   与 found 体 `adds r0,r0,r1` 同寄存器即被合并; 目标 ROM base2=r2 故不合并。
3. local-alloc `find_free_reg` 按数字序扫 r0..r7, 窗口 = [2*出生指令, 2*死亡指令), 按
   QTY_CMP_PRI (refs*size/寿命) 排序分配。M 形态分配序: slot→r0, LC1(短窗)→r0,
   value 被挤→r1 — 这就是 3 字节差距的来源。
4. **两难**: 共享 newval (M 形) → LC1→r0/value→r1 (value 错位); 两个体各自 store (S 形,
   cross-jump 合并出公共 str) → value→r0 ✓ 但 LC1→r1 → 与 found 体 t=r1 尾合并。
   目标要求 value→r0 **且** LC1→r2 同时成立。
5. r2 需要窗口 [LC1 出生, 尾加法] 内 r0+r1 双占。r0=value 可解; **r1 占用源不明**:
   块内唯一 r1 占用者是 LC0 (0x02016000 装载), 死于 first-add, 窗口 [4,6) 不覆盖
   LC1 窗口 [10,12)。穷举 35+ 形态 (基址重叠存活期 N8/N9、base 复用 W4、do-while 屏障
   D1-D3、数组下标 W6-W8、u16 value P3、两 store S1-S4、pi 风格 off P11、+= 形态 W3/S4)
   全部收敛 11B 或 13B; permuter 2.4 万次迭代 (M1 种子) 无突破。已匹配同族
   Op_IfSaveLoadedJump (r1 被 ptr 占用才得到 r2/r2) 与 Op_IfSaveFlagJump (r1 空闲得
   r1/r1) 对照: 本函数 ptr 必须跨调用 → r6, r1 无活过值 — 与 ROM 的 r1/r2 指配矛盾,
   怀疑原始 C 有一个此处不可见的 r1 存活量 (或 regmove 的隐性合并)。
6. `-g` 变体不可用: 同一 C 文件 的 sub_80532DC/Op_IfSaveFlagJump 等已按默认 flags 逐字节匹配。

**下一步建议**: ① 用 gccdump 逐 pass 比对 regmove 输出 (regmove 在 -O2 因
-fexpensive-optimizations 实际开启, 可能产生模型外的 qty 合并); ② 检查 regclass.c 的
reg_pref 对 LC1 伪寄存器的建议值来源; ③ 同族 sub_804F974/sub_804FA94 解析后对照。

## 2026-09-03 sub_80488CC 合入遗漏修复 (zai)

开场 make 即红 (缺 asm/nonmatchings/sub_80488CC.s): 函数清单 status=1、切片在 matchings/,
但 src/code_4394 的 INCLUDE_ASM 从未被真 C 替换 —— 胜出候选只存在于提交说明里。
按 matchings/.s + progress 语义描述重建: 关键结构 = ① if/else 让普通路径 fall-through
(else 尾置特殊分支, bhi 跳末尾); ② `first = *(obj+0x99)` 独立先读、`skills = obj+0x99`
后建 (CSE 出 `adds r0,#0x99; ldrb r2,[r0]; adds r5,r0,#0`); ③ 循环体读值必须用**另一个
变量** `val`, 复用 first 会把循环 load 落 r2 (目标要 r0 scratch), 差 7B; ④ sub_8045A10
返回值按 u8 用 (void 原型改 u8 后 `lsls r0,#0x18; cmp r0,#0` 形状自现)。
bytecmp 指令级一致 (bl 槽远地址 veneer 为 bytecmp 伪影, 以 `sym=0x近地址` 消除)。
fncheck OK 102B。

## 2026-09-03 sub_803F328 合入 (opencode)

对话框状态机 (gUnk_0300086A, 0x0803F328), m2c 转出即近似 100%: jump table 0-5 case,
只有 0x4c 处 4B 逆序 (`ldr r1,=0x02035AC0` 应在 `movs r4,#2` 前)。
三条路对比解决:
- 内联 `sub_80196D4(0,(u8*)0x02035AC0,...)`: 先物化 movs 再池加载 → 逆序。
- `base=(u8*)0x02035AC0` 指针局部: 依旧逆序。
- `int base; base=0x02035AC0;` 再传 `(u8*)base`: ✅ 池加载先于 movs。
写入经验 123。sub_80196D4 是 9 参 (0,base,0xB,2,2,1,2,0xC,4) K&R, 实参见
sp 布局 r0/r1/r2/r3 + 5 栈槽。sub_803F21C(0x02035AC0,arg0)。C89 顶部集中声明
(经验 122)。fncheck OK 284B, 全量 sha1 仅剩 sub_802761C (他人进行中)。

## 2026-09-03 sub_802761C 匹配 (gpnux)

对话框状态机 (gUnk_03000820, 0x0802761C, code_80264C0), 324B exact。

**关键词**: 经验 16/37 (switch 不带 default, 空 case 1-7 迫使决策树分发 `cmp#7;bgt;cmp#1;bge;cmp#0;beq`);
zero 变量复用 arg2/arg5 (单个 `movs r1,#0` 同时服务 `r1` 和 `str r1,[sp]`);
**调度槽位**: 尾部 `obj[0x24] &= 0xEFFF; zero=0; call;` 的 `movs r1,#0` 排在 `strh` 之后,
但目标在 `ands r0,r1` 后立即物化。用 `masked = load & 0xEFFF; zero=0; store = masked;` 分解,
让 RTL 顺序变成 `ands r0,r1; movs r1,#0; strh r0` 与目标一致。permuter 跑 4400+ 代 score=130
未突破 (纯调度非语句排序问题)。

**结构**: case 0 设置 4 个全局状态 + 调 sub_80444A4/sub_801CE80/sub_803F5B4; case 9 随机选
10% 概率为 obj pool 槽设 collision flag; 尾部统一调用 sub_803F658 并检查 0x1000 标志位清除。

## 2026-09-03 sub_8032D74 匹配 (claude-c)

NPC 对话/交互状态机 (gUnk_03000820, 0x08032D74, code_80264C0), 298B exact, sub_802761C 近亲
(同入口形状: ldr 状态指针 r2 保活, case 6 复用 `strb r0,[r2]`)。

**两个关键点**:
1. **case 块源码顺序 = ROM 块顺序** (GCC2 保序发射, 近亲 sub_8042AB4/80405A4 均如此)。
   本函数 ROM 顺序 0→19→20→6→9, 按常规 0,6,9,19,20 书写时 case0/case19 的
   `gUnk_03000820=X; break;` 尾被跨块 tail-merge 进共享 `strb; b end` (bytecmp 155B 差);
   按 ROM 序重排后三处存储全部内联, 差异立降。**遇到 switch 尾块异常合并, 先对齐 case 顺序**。
2. **三目方向不可交换**: GCC2 if-conversion 机制 = 基值取 else 分支值, cond 为真时加
   (true-else) 差值。写 `obj[0xBE] != 0 ? 0x371 : 0x362` 得 `base 0x362; beq skip; add #0xF`;
   目标要 `base 0x371; bne skip; subs #0xF` → 必须写 `obj[0xBE] == 0 ? 0x362 : 0x371`。

**bytecmp 伪影实证**: 候选引用 4 个未匹配函数时, 若在 abs.ld 里定义
`sub_8048B30 = 0x08048B31;` 会因超 bl 范围出 veneer (字节 00F0 4DF8), 与 target 的
占位 F7FF FFFE 差 16B; **不定义这些符号则 ld 报 undefined**, 都不能到 OK —
这是"未匹配被调者"的固有伪影 (经验 29 注记), 以 fncheck (自动忽略 bl 槽) 为准。
permuter base score=40 (=4 bl 槽×10), 同属该伪影。

## 2026-09-03 sub_801869C 匹配 (gpnux)

BGM 选曲状态机 (gGstate324/gGstate32E, 168B exact)。**关键**:跳表 0-16 需列全 17 个 case(经验 37),否则 GCC 生成决策树而非跳表。`default:` 必须显式写出,否则 `bhi`(越界)直接跳 epilogue 跳过 Bgm_Play。Bgm_Play 放在各分支内(switch 内每个 case 组 + else-if + else),编译器自动合并 track=2/3/4 路径为公共块, track=0 单独复制。去掉 track 变量后仍匹配(GCC 把同一 case 组的常量调用直接定位到 r0)。

## 2026-09-03 sub_801A2AC 挂起更新 (gpnux)

再次尝试 25 个变体 (v1-v18, s1-s4, d1-d3, 含 `u32 ext` 显式零扩展、store 用副本/mode 用 arg0 等排列), 全部差 47 字节, 卡在同一个寄存器分配: 目标 `lsls r3,r0,#0x10` 把 arg0<<16 移入 r3、arg0 保留在 r0 供 `strh r0`(BLDCNT store); 我方任意写法 GCC2 都 `lsls r0,r0,#16` 就地 clobber r0 再 `lsrs r3,r0,#16` 恢复, 导致 store 用 r3、地址用 adds 递推而非独立池加载、mode 寄存器错位。根因: GCC2 的 CSE 把 arg0 的零扩展版(为 `>>6` 准备)与 store 值合并, 使 arg0 伪寄存器在 `<<16` 后即 REG_DEAD (联用 -dl 可确认)。permuter 平台期 240 未突破。最接近候选: v9 (u32 arg0, `REG_BLDCNT=(u16)arg0`, `switch ((arg0>>6)&2)`), 保留 r0 但缺 `lsls r3,r0,#0x10` 序列和独立池加载。

## 2026-09-03 sub_801D19C 匹配 (opencode)

音效/状态机 getter (sub_801D12C 的"取值"版, 120B 逐字节命中, sha1 全绿 648/1064)。

**流程**: 先按常规 if/switch 写出 (case 0/1/2 嵌 ab-switch, case 5 用 `if (ab>7)/(ab<1)` 区间守卫),
permuter 平台期 875~895 (未破, 最"佳" 555 是 `v=(u32)obj` 的 rule 87 伪造, 语义全崩, 弃用)。

**两个真正的坑 (经验 126 / 54)**:
1. **>0xA 路径不写 return (rule 54)**: 最初我把 guard 写成 `if (obj[0xBE] > 0xA) return (u32)obj;`
   (check-first) 或 wrapping 的 `return (u32)obj;` 在末尾 —— 两种 agbcc 都 0xbe 就地用 r0 (`adds r0,#0xbe`)
   再重载 obj, 与目标差 90+ 字节。改回 **经验 54 式**: `if (obj[0xBE] <= 0xA) { ...; return v; }` 中
   **>0xA 路径直接函数末尾掉出 (无 return)**, agbcc 锁 r0=obj (obj 就地留在 r0), 临时量上移 r1/r2
   (`r1=obj+0xbe`, `r2=kind`), 目标前 16 条指令全部归位。
2. **case5 区间守卫必须写成行内单侧 switch**: `switch (ab) { case 1..7: v = 1; break; }` 才出
   `cmp #7; bgt; cmp #1; blt`; 写成 `if (ab>7) break; if (ab<1) ...` 被 agbcc 归约成 `cmp #0; ble` (差 2 字节)。

**附带要点**: `ab` 声明为 `int` (有符号) 才出 bgt/blt; `u8` 出 bhi/beq。返回类型用 `u16` (与 header 一致,
不改 code_0.h 签名即可, 但 header 原是空参 `u16 sub_801D19C();`, 加了全原型 `(u8*, u8)` 后编译通过 —
调用方只传 (u8*, u8) 无截断, sha1 仍绿)。`fncheck` OK, `make` + `sha1sum -c` 通过。

## 2026-09-03 sub_801FEBC 匹配 (opencode)

场景对象滑动参数组设置 (与 MOD-05 `sub_8020FB8` Obj_StartSlide 同族, 132B 逐字节命中, sha1 全绿 649/1064)。
TSV 原挂起 note: "agbcc寄存器home深度分配; 目标value→r3/zero→r3/ptr+0x37→r4 subs复用; 穷举C不可破"。

**三个真正的坑 (经验 127 / 11 / 13 组合)**:

1. **zero 提前物化进 r3 需要"两条 RMW 拆写 + 结构体成员访问"**: 目标是
   `ldrh r3,[r4]; ldr r0,=0xFF0F; ands r0,r3; movs r3,#0; movs r6,#0x20; orrs r0,r6` ——
   value(0xB0 值)先进 r3、`ands` 后 r3 恰好死亡, agbcc 把 `gUnk_0300061A=0` 的 0 物化进死槽 r3,
   再用 `strh r3,[0x0300061A]`。只有写成 **两条语句** `arg0->field_B0 = arg0->field_B0 & 0xFF0F;
   arg0->field_B0 = 0x20 | arg0->field_B0;` (经验 13) 且用**结构体成员访问** (经验 11, mov ip,r0 缓存)
   才产生该空隙; 单条 `x = (x&0xFF0F)|0x20` 或裸指针 cast 均不产生 (差 35+ 字节)。

2. **ptr+0x37 的 subs 复用 + subs 调度位置**: 目标 `subs r4,#0x79` 复用 r4(=obj+0xB0) 得 obj+0x37。
   独立语句 `p -= 0x79;` 虽产生 subs, 但被调度到 `ldr r1,=0x0300061C` **之前** (差 8B);
   把递减**内嵌进读取表达式** `gUnk_0300061C = *(u8 *)(p = (u16 *)((u8 *)p - 0x79));`
   (C 的赋值表达式) 后, subs 才落到 `ldr` 之后紧贴 `ldrb` —— 与目标一致 (0 差)。

3. **diff 值落 r1 (而非 r3) 靠链式赋值**: `gUnk_03000620 = (dh = 0xB4 - *(u8 *)p);`
   写成 `dh = ...; gUnk_03000620 = dh;` 两行时 dh 落 r3、地址加载后置, 目标要 dh→r1 且
   `ldr r3,=0x03000620` 先置 —— 链式赋值 (经验 110 同思路) 才让 qty 创建序 diff→地址, home 归位。
   判定: 目标 `movs r1,#0xb4; subs r1,r1,r0; strh r1,[r3]` + 尾部 `cmp r1,#0` = 链式赋值标志。

**附带要点**:
- 参数必须按头文件写 `void *varg` + 函数体首行 `Unk_8020F4C *arg0 = (Unk_8020F4C *)varg;`
  (经验 14), 直接写 `Unk_8020F4C *arg0` 与 code_0.h 的 `void *` 声明冲突 (agbcc 报 conflicting types)。
- struct 用模块已有 `Unk_8020F4C` (0xC8 场景对象, 与 src/code_8020D50.c 同布局), 本文件也定义了
  (各 C 文件 独立 typedef, 不共享头)。
- `if (dh > 0) arg0->field_24 |= 0x20;` 复用 r6(=0x20, 已为 B0 物化) → 常数 0x20 两次使用正是
  r6 保活跨全函数的原因。
- `sub_801FA10(arg0, 1)` 收尾 (对象滑动状态机复用入口, 与 Obj_StartSlide 一致)。
- fncheck OK (132B, 1 bl 槽忽略); 字节定性以 bytecmp/fncheck 为准 (permuter base score 35 是 bl 链接
  artifact 假高, .o 层 raw 比对 0 指令差异)。

## 2026-09-03 sub_80498E0 匹配 (gpnux, 120B exact)

磁盘动画帧写入器 (gUnk_030009BF/9C0/94D + 表 gUnk_08095028)。原挂起 note 称"34B 地板需长磨"。

**两个卡点逐一破解**:
1. **callee-saved 寄存器轮换** (ptr→r4/BF→r5/table→r6): 用 `const u8 *tbl = gUnk_08095028;` 局部指针首载 (table 基址物化进 r6 第一条), 且 byte 表达式内联 `(u8*)gUnk_030009C0` (不声明 ptr 局部) → GCC2 自然把 ptr→r4、&gBF→r5。声明顺序 tbl 先、byte 表达式内联 gBF/gC0 是关键。
2. **第二处 frame 载入的调度地板 (4B)**: 目标 `ldrb r0,[r0]`(byte)→`lsls r0,r0,#3`(byte*8)→`ldrb r1,[r3]`(frame)。GCC2 总把 frame 载入提前填进 byte 载入的延迟槽。解法 = 把 `byte*8` 拆成独立语句 `u16 ofs = byte * 8;` (u16 避免 u8 截断的 lsls#27/lsrs#24), 再 `tile = tbl[ofs + frame]`。独立语句让 shift 紧跟 byte 载入, frame 读取落到 shift 之后 → 逐指令一致。

新符号 gUnk_030009BF(s8)/gUnk_030009C0(u32) 登记 iwram.h+linker.ld; code_0.h 原型 void→u32 (调用点忽略返回值, 安全)。

## 2026-09-03 sub_8013870 更新 (opencode, 仍挂起)

文本块绘制 = ClearBuffer 内联填 0xB001 (0x14×0x1E, 行距 0x20 u16) + TextBlocks_Render 式串 + Text_WriteChars 尾。
前 agent 已确认 v13(src 后赋) / v16(src 顶赋) 两个极近候选。本次深化:

**v13 = 整函数寄存器全对, 只有 prologue 差**: 目标 prologue 顺序 buf(r2)→y(r1)→src(r8)→0x1E0000(ip)→w(r9)→h(r6),
v13 是 buf→y→0x1E0000(ip)→w(r8)→h(r6), src 在 fill 后 `ldr r4,=0x08098622` 晚载。
差 8 字节 prologue + 全函数偏移级联 (bytecmp 53B 多是分支偏移错位)。**v13 的 fill/串循环寄存器与目标逐条一致**。

**permuter 从 v13 出发跑到 490 = cand_490_best.c**: 关键技巧 `int new_var = 0xFE;` 顶赋、
转义比较用 `charCode == new_var` → new_var 成为**低 pri 长活值** (lreg: 4 次/158 insn, pri≈506, 经验 117)
落 r8, 把 w 挤到 r9 → **填零块逐字节命中**。剩 ~21 真实字节 (bl 槽占 16B 另计):
1. prologue r8 装的是 0xFE 而非 src (值/顺序不同);
2. 串的 src 用 `ldr r4,=0x08098622` 而非 `mov r4,r8`;
3. 转义比较 `cmp r1,r8` (寄存器) 而非目标 `cmp r1,#0xfe` (立即数)。

**根因 (经验 117定量)**: 目标把 src 基址留在 r8、迭代用 r4 拷贝 (mov r4,r8, r8 仅用一次)
→ src 必须是**低 pri allocno** (少用/长活) 才落 r8; 但 src 作迭代器 (14 次/34 insn, pri≈12353)
必落 r4 (v16 整块轮换), 单独拆成基址+迭代器 (p=src) 又被常量传播折叠 (probe J/p)。
穷举 ~35 变体 (src 位置/类型、声明序、int 别名、命名 ROM 符号、双向迭代、-g flag) 均未破。

**下一位接手方向**: 经验 117 算 fill 各 allocno 的 pri, 让 fill 值先占 r4 把 src 逼到 r8;
或拆"基址+迭代器"时用非折叠用法 (如比较/寻址引用) 保 src 存活。候选: base.c=v13 式, cand_490_best.c 最近。

## 2026-09-03 sub_801DAA0 匹配 (agent_sub801DAA0_0447, 156B exact)

场景倒计时状态机 (演武/开场 demo 计时): `gUnk_0300068E` = 阶段 (0..0x22), `gUnk_0300068D` = 慢计数。
阶段<3: 计数=(c+1)%(10-阶段), 计满→阶段++; 3..0x22: 阶段++; >0x22: 返回 1 并整体重置
(gUnk_0300068C=0, 阶段=1, 计数=0, 清 7 项 gUnk_03000670 表, `sub_804C2FC(gUnk_0861C664,0xF,1)` + `sub_804C3A4(0xF,1)`)。

**流程**: m2c 弃用 (手读 79 行 asm), 手写人类 C 一次成稿 → 首跑 permuter 平台期 30 (迭代 1000+ 不动)。
卡点定性 = 经验 29: 单函数 .o 字面池未重定位, score 永不归 0 (本函数 2 个池块 6 个字面量)。
**字节定论走 bytecmp.sh**: 156 指令字节全等, 3 个 bl 对 = 可重定位占位 (0000 F016 vs FFF7 FEB4,
偏移全 0xFFFF), 尾部 +52B = abs.ld 段对齐 (ld 侧), mine.bin 无一真实差异 → 候选成立 (先例 EXPERIENCE 124 sub_8052BA0)。

**合入踩坑 2 个** (均为共享文件布局问题, 非 C 问题):
1. extern/typedef (Unk_8021064, gUnk_03000670, gUnk_0861C664) 声明在文件 460 行处 (sub_801DE44 前),
   新函数在 421 行 → "used prior to declaration" / "undeclared"。解法: 把 typedef+extern 上移到 124 行
   (sub_801BE34 的局部 typedef 区), 462 行处改为只留 `gUnk_03000730_arr`。
2. code_0.h 原型 `void sub_801DAA0()` → 返回值被 6 个调用点消费 (`lsls r0,#0x18; cmp r0,#1`),
   改 `u32 sub_801DAA0()` (K&R 无参形不改调用点形状; 6 个 caller 现全在 asm 侧, 零字节风险)。

fncheck OK (156B, 6 池重定位); 全量 make + SHA1 绿。
**语义发现**: 调用族 = sub_801BE34 (同一 C 文件) + sub_804A368 + sub_801EEE4 + sub_801F3FC (后三个同一 C 文件),
全部 `cmp r0,#1` 判"计时走完", 是场景切换边沿标志。建议后续匹配该族时考虑语义名 PollSceneTimer 族。

## 2026-09-03 sub_804C890 匹配 (gpnux, 78B exact) — 破 34B 调度地板

磁盘动画帧写入循环 (5 个 0xC8 槽, sub_8045F10 判 2 → Rng+C8E0 写 o[0xBD]/清 o[0xBC])。原挂起 note "movs r1,#0 被外提到 r7; long long 阻外提仍差1条; 75分"。

**破解链 (permuter score 0)**:
1. **零外提到 r7**: `o[0xBC]=0` 字面量 → GCC2 把 `movs r1,#0` LICM 外提到 r7 (多 push)。用**零变量** `t=0` 且 t 先作 C8E0 实参 (`t=i`) → t 占 r1 (call 实参寄存器), `t=0` 复用 r1, 不触发外提。
2. **subs 折叠**: 字面量 0 让 GCC2 把 o+0xBC 折成 `(o+0xBD)-1` 的 `subs r1,#1`。用零变量 t 避免折叠 (改从 r4 重算)。
3. **首地址 r1 vs r2 (6B 地板)**: 目标首地址 o+0xBD 在 r2、零在 r1 (life 短→优先级高→拿低号 r1)。解 = `p=o+0xBD` 指针变量提前算地址 (life 变长→降优先级→拿 r2), 且 `t=0` 在地址后、store 前 → 零落 r1@30。
最终: `u8 r,t=i,*p; Rng_LcgNext(); r=sub_804C8E0(obj,t); p=o+0xBD; t=0; *p=r; o[0xBC]=t;` → 0 字节。

**并发提示**: 本文件 code_8044394.c 同时有另一会话对 sub_8045A10 (status=0) 的未完成真 C (extern 错位/尺寸变), 致全 ROM 红+位移; 我的函数 fncheck 隔离 OK, 未回退他人编辑。

## 2026-09-04 sub_8049AD8 挂起 (opencode) — diff 区 LRA live-range-split 不可复现

**语义 (已全解)**: battle stat 成长检查。`s=(u8)(arg0+1)`, `n=13-s`, 遍历 `i=0..n-1`:
`idx=(u8)(i+251+s)` (= i+arg0-4, 即 8 个属性槽下标); `a=sub_80455A0(obj,idx)` 当前值,
`b=sub_8048818(obj,idx)` 目标值, `diff=b-a`; 存 `gUnk_0300095A=diff`; 若 `(s16)diff>0`
则 `sub_8045688(obj,idx,(u8)diff)` 加值并 `return s+i` (命中槽), 否则继续; 全不命中 `return 13`。
调用者 sub_80494F0 用返回值判 12/13 = "无成长"。obj = `gUnk_03000949` (战斗对象索引)。
sub_80455A0/sub_8045688 仅被本函数调用 (可安全定原型 `u16(u8,u8)` / `void(u8,u8,u8)`)。

**已解决的部分 (permuter/手写到 bytecmp 差 6 真实字节)**:
- 序言/`s`/`n`/循环闩/返回 全绿。
- idx 区: 标准 flag 下 `idx=i+251+s` 出 base-s 且 obj 载入在 idx 之后 → 用 `obj=*(u8*)0x3000949;`
  独立首语句 (强制 obj-load-first) + `t=i+251; idx=t+s;` (int 中间量强制 base-i) 命中。
  (注: `-g` 变体下 inline 写法即自然命中 obj-load-first+base-i, 但本 C 文件 用标准 flag。)
- b 截断: 需 `u16 b=sub_8048818(...)` 独立变量 (否则 subs 前无 `lsls/lsrs #0x10`)。

**卡点 (diff 区, 目标 `subs r0 / adds r2,r0,#0 / strh r0 / lsls r0 / cmp`)**:
目标把 int `diff` home 在 **r0** (供 strh+cmp 用), 并 `adds r2,r0,#0` **纯拷贝**到 r2 供 call 第3参
(obj reload 在分支后复用 r0, 故须提前存 r2)。这是 LRA 的 live-range-split (短命 r0 + 长命 r2 拷贝)。
标准 agbcc 从干净 C **产不出这个 split**:
- separate store (`diff=b-a; *X=diff;`): int diff 被 home 到 **r2** (call 寄存器), `subs r2`, strh/cmp 用 r2, 无拷贝。
- combined store (`diff=(*X=b-a)`): 得 `subs r0`+`strh r0`+cmp r0 (home r0 ✓), 但赋值表达式类型是 u16
  → diff 值被 u16 化 → call 处 `lsrs r2,r0,#0x10` (截断) 而非 `adds r2,r0,#0` (拷贝)。差 6 真实字节 (0x54-0x59)。
- 试过 ~55 变体 (int/u16/u32/long diff × combined/separate/chained/comma/ptr/extern × obj-var/inline ×
  int-split/a251/inline idx × 显式拷贝变量 save/dc/callval/raw × do-while 屏障 × 重赋值技巧), 全部
  int-diff→subs r2 或 u16-diff→subs r0+截断, 无一得 subs r0+纯 adds r2+strh r0。
- permuter 5 次跑 (base/t6/t1/output-10/e70 种子): 最优 output-10 = bytecmp 差 **2 真实字节**
  (`strh r2`/`lsls r0,r2` vs 目标 `strh r0`/`lsls r0`), 且它靠 `diff=a; b-diff` 重赋值偷改数据流 (不可读,
  违反铁律6步5), 清洗成人类代码即退回 subs r2。scan 全部 output 无一含目标 diff 序列。

**结论**: 该 diff 区是 agbcc 标准 flag 下的 LRA split 边缘案例, 干净 C 不可复现 (疑原始编译器/版本 RA 选择不同)。
按 §2b 转挂起。候选存 `permuter/sub_8049AD8/candidates/`: `best_readable_t6.c` (6 真实字节, 可读) 与
`best_permuter_output10.c` (2 真实字节, 不可读)。接手者: 若发现能触发 int-diff home r0 + 纯拷贝的写法
(参考 sub_804C890 用指针变量调 life 拿 r2 的反向思路: 让 store/cmp 的值 life 短→r0, call 的拷贝 life 长→r2,
且阻止二者 CSE 合并), 即可破。gUnk_0300095A 尚未登记 (linker.ld/iwram.h), 匹配时补。

## 2026-09-04 sub_80392C0 匹配 (agent_glm, 288B exact)

**语义**: 对话/事件对象状态机, 对 `gUnk_03000820` (u8 @03000820) switch, 有效 case 0/19/20/6/9 (21 项跳转表):
- case 0: 存 obj[0x35]→03000824, obj[0x2A]→03000822, `sub_8048B30(0, 0x1E, 0x3CB)`, 置 0x13;
- case 19: `sub_8047B1C(obj)==1` 则置 0x14;
- case 20: `sub_801CBA4(obj, 0, 0822, 0824, 0)` (第5参栈传), 置 6;
- case 6: `obj[0x24] & 0x800 == 0` 则置 9 (该写复用入口物化在 r2 的状态指针 `strb r0,[r2]`);
- case 9: `sub_8045B90(obj, obj[0xA1])`, 返回 2。

**同族模板**: 与 sub_8032D74 (claude-c 已匹配, 经验 124/125 出处) 除 case 0 第三参
(此处恒量 0x3CB, 彼处 obj[0xBE] 三目) 外逐指令相同。直接移植其源码 + 经验 124
(case 块按 ROM 序 0,19,20,6,9 书写), permuter 首评 `base score = 0` (0 errors)。

**新坑/技巧 (重要, 可复用)**:
1. **permuter base.c 中 IWRAM 全局用 `__asm__(".set gUnk_03000820, 0x03000820")` + `extern` 声明**,
   不要用 `#define gUnk (*(u8*)0x03000820)` 绝对宏:
   - 绝对宏版: case 6 的状态写被 GCC 重新物化 (`ldr r1,=0820; strb r0,[r1]`), 入口也不保
     r2=指针 → 344B/288B, 16+ 差异;
   - .set+extern 版: 真实符号语义 (SYMBOL_REF), GCC 跨跳转表分发把入口地址物化保到 r2,
     case 6 直接 `strb r0,[r2]` — 与目标及真实构建 (iwram.h+linker.ld) 完全同形,
     且池常量汇编期解析为绝对值, 无未定义符号重定位假差异, 分数可干净到 0。
   - 若用纯 extern 无 .set: target.o 池是绝对常量, candidate 是 R_ARM_ABS32 符号,
     scorer 豁免条件 (old_line.has_symbol) 不满足 → 每个池字 +1 假分, 0 不可达。
2. **bytecmp.sh 尾部 +64B 假象**: 部分链接对 bl 的 4 个互工作目标插 Thumb→ARM veneer
   (脚本赋值符号无 mapping symbol, ld 恒当 ARM, 加奇地址 Thumb 位也无效), 且 bl 槽位
   本身必然是链接值 vs 未链接占位 — 按 fncheck 为准 (288B OK)。

**原型**: code_0.h `void sub_80392C0()` → `u8 sub_80392C0()` (同族 sub_8032D74 先例;
无 C 调用者, 返回 0/2 与 sibling 同), 定义 `u8 sub_80392C0(u8 *obj)` 合法兼容 K&R 声明。

**验证**: fncheck OK 288B; make 全量 + sha1sum -c ll.sha1 绿 (匹配进度 654/1064)。

## 2026-09-04 Sprites_LoadMapNPCs 真C落地 (opencode, 116B exact) — 命名符号复现调度

原 status=1 但仅 `INCLUDE_ASM("asm/matchings")` + 注释掉的草稿 C。任务=把草稿变成真身。
语义: 若 `gObjGraphicsSetId & 0x80` 返回; `id=gMapSceneDescriptors[arg0].npcSlotGroupId`;
id==0 返回; `count=gUnk_08091948[(id-1)*18]`; `ptr2=gUnk_087EA394[id-1]`;
`for(i=2;i<count+2;i++) Chara_InitFromDesc(i, ptr2++)` (ptr2 每次 +0x10)。

**关键坑: 裸地址 vs 命名符号改变调度+折叠**。
- 用裸地址 `*(u8*)0x08088D80` 等: GCC2 把基址 `ldr` 排到 index 计算**之后**(目标在之前),
  且把 `(temp_r3-1)*4 + 0x087EA394` 代数折叠成 `temp_r3*4 + 0x087EA390`(读错槽!),
  permuter 也修不动(非语句顺序问题)。
- 改用**命名符号** `gMapSceneDescriptors[]`/`gUnk_08091948[]`/`gUnk_087EA394[]`(重定位):
  基址 ldr 自然排在 index 前(匹配目标调度), 且符号非常量→无法折叠 -4→`(id-1)*4` 显式 subs+lsls,
  寄存器分配也对齐(arg0=r2, id=r3)。→ bytecmp 0 真实字节(仅 bl 槽, 全链接后一致)。
- 草稿的 `ptr2 = gUnk_087EA394[temp_r3 - 1]` 保持**不拆分**(命名符号下本就不折叠; 拆成
  `temp_r3=temp_r3-1` 反而把 arg0/id 挤到 r3/r2 互换)。

**配套改动**: 新登记 `gUnk_08091948`/`gUnk_087EA394` 为 ROM 绝对符号(linker.ld SECTIONS 外, 地址序);
`Chara_InitFromDesc` 全局原型 `void()`→`void(u8,void*)`(触发调用点 arg0 的 u8 截断; 唯一其他调用者
sub_804F280 是 asm 不受影响)。fncheck OK 116B; make + SHA1 绿。

## 2026-09-04 sub_804DCD8 复核仍挂起 (opencode) — 差4B global-alloc split, 落地会破ROM

复核姊妹族 sub_804D1B4/D260/D708/D798 同族函数。语义: `count=sub_80489E8(arg1,values,0,0x6F)`;
`if(Rng%0x65 < count*10) obj[0xBC]=1 else obj[0xBC]=0`; 然后**强制** `obj[0xBC]=1; obj[0xC2]=0`;
`entry=&gUnk_08393B28_entries[*(u16*)(*(u32*)(obj+0x88)+8)]`; `switch(entry->field_10)`:
case0 `obj[0xBD]=values[Rng%count]`, case1 `obj[0xBD]=0`。

**真身差 4 字节 = 2 条指令** (我的 148B vs 目标 152B):
1. region1: 目标 `strb r0,[r1]`(cond) 后 `adds r2,r1,#0; movs r1,#0; movs r0,#1; strb r0,[r2]`(obj[0xBC]=1)
   `adds r0,r4; adds r0,#0xC2; strb r1,[r0]`(obj[0xC2]=0) —— 把 &obj[0xBC] 从 r1 拷到 r2, **复用 r1 存 0**。
   我的 straight-line `obj[0xBC]=1; obj[0xC2]=0;` 给 `strb r0,[r1]`(=1 用 r1) + `movs r2,#0; strb r2,[r0]`(0 用 r2), 少那条 adds 拷贝。
2. case1 obj[0xBD]=0: 目标 addr=r1/val=r0 (同 sub_804D260 matched 的 addr=r1/val=r0), 我给 addr=r0。

**已试全失败**: 命名符号(gMapSceneDescriptors 等)/switch((s8)obj[0xBC])折叠/`u8 z=0` 变量/交换 obj[0xBC]与obj[0xC2]顺序/~15 变体 + permuter(地板 score240=148B; 偶有 152B 输出但靠 `obj[0xBC]=(count=0)` clobber count 的 hack, 破坏 case0 语义不可读)。
根因 = **global-alloc live-range-split**(r1 跨块复用给值 + 拷地址到 r2), agbcc 从干净 C 产不出。与姊妹 sub_804D798 的 "&obj[0xC2] 调度差" 同坑(见其 note)。

**⚠ 重要教训**: 此函数 status=0, 落地 148B 版会让 sub_804DD70 从 0x0804dd70 位移到 0x0804dd6c(-4), 连锁改 VBlank_UpdateGameScreen@0x080003d4 的 bl 目标 → 全 ROM 红。**fncheck 报 "OK 152B" 是假绿**(它比对前缀, 漏报函数尺寸短了 4B)。定论必须看 `ll.map` 里函数实际尺寸/下一函数地址, 或 make+SHA1。已 git checkout 回退 src 保绿。
候选存 permuter/sub_804DCD8/(base.c=148B 干净版)。下一步同 sub_80531A8: 给 agbcc global.c 打补丁 dump 全局分配优先级, 或找触发 r1 跨块复用+地址拷贝的引用形式。

## sub_80455A0 (0x080455A0, code_8044394) — ✅ 2026-09-04 claude-455a0
战斗基础属性 getter: `f = gBattleFormationIds[obj[objIdx*0xC8+0xBB]]` (1-based 编队号), `if(f) f--` (转 0-based),
`switch(stat 0..7)` 返回 `gPartyStats[f].{max_hp,max_mp,base_atc,base_def,base_agl,base_men,base_res,base_noa}`。
仅被挂起的 sub_8049AD8 调用 (progress 2704 已注可安全定 `u16(u8,u8)`)。

**起手即中** (借同文件已匹配模板 `sub_8048818` 的索引式):
1. **struct 必须 0x40 stride**: 内联草稿若把 PlayerStats 写成实际字段大小(0x1C) → `gPartyStats[f]` 缩放成 `lsl#3;sub;lsl#2`(×28), 目标是 `lsls r1,r2,#6`(×64)。补 `u8 pad2[0x24]` 到 0x40 即对 (合入用真 PlayerStats)。
2. **返回类型 u32/int + switch 不写显式 default return**: 目标 `cmp r5,#7; bhi <epilogue>` 默认路径**直接 fall-through 返回 r0=base**(GetObjPool 的调用返回值, 未被地址计算 clobber), epilogue 无截断/无 mov。
   - 若写 `return base;` 显式默认 → GCC2 加 `lsls r0,#0x10; lsrs r0,#0x10`(u16 截断) 多 2 指令 (差)。
   - 不写默认 return → GCC2 让 base 的 home 落 r0(调用返回寄存器), 地址计算改用 r1(temp)/r2(base 拷贝 `adds r2,r0,#0`)/r3(fid) → **index 段逐指令命中目标**。
   写成 u16 返回 + 无 return 与 u32 字节相同(无 return 语句可截断), 取 u16 更语义。
3. `*0xC8` 走 `muls`(非 2 幂), 与 sub_8048818 同。

fndiff 指令域逐条全等; bytecmp 报 4B 差 = bl(GetObjPool) 槽 + 尾 trampoline 垫 (已知含 bl 候选伪影, 同 sub_8018D9C); fncheck 232B OK, make+sha1 绿。
关联经验 97 (u32 formation 使 load 落 r2)、经验 132 (switch getter 无默认 return → 默认 fall-through 返 stale r0 决定 index 分配)。

## sub_8045860 (0x08045860, code_8044394) — ✅ 2026-09-04 claude-45860
收集战斗对象"可学技能列表": `buf[0..7]` 先填 0xff, `f = gBattleFormationIds[obj[objIdx*0xC8+0xBB]]` (1-based→`if(f)f--`),
`while (obj[0xAA] < gPartyStats[f].lv) { obj[0xAA]++; id = ItemFindSlot(obj[0xAA], gBattleFormationIds[obj[objIdx*0xC8+0xBB]]); if (id!=0xff && sub_8048868(objIdx,id)) { if (c<=7) buf[c++]=id-1; } }` 返回计数 `c` (s8)。
仅被挂起的 sub_80494F0 调用 (调用者把返回值当 s8: `strb` 后 `asrs` 比较 0<c<=7) → 原型 `s8 sub_8045860(u8, u8*)`。

**三个分配触发器** (逐步 8→76→89→91→fncheck OK):
1. **首个 GetObjPool 结果存进独立变量 `p1`** (不是内联、也不是复用 `pool`):
   - 内联 `formation=...[*(GetObjPool()+...)]` → GCC2 把 `ldr r4,=gBattleFormationIds` **提到 call 之前** (r4 callee-saved 存活), 但目标是 call **之后**载入 fid 到 caller-saved r2 → bl 偏移差 2 字节。
   - 复用同一 `pool` 变量 (两次赋值) → 首次结果被 `add r2,r0,#0` **拷贝** (pool home=r2 跨 loop1), 目标无拷贝。
   - 独立 `p1` (用后即死) → 留 r0 无拷贝, 且 fid 自然落 r2 在 call 后载入 → index 段逐指令命中。
2. **两个循环计数器合并为单个 `s8 c`** (loop1 的 i 与 loop2 的 j 在目标共用 r5, 生命周期不重叠): 分开声明 `s8 i,j` 会让它们抢不同寄存器 (r1/r7) 错位; 单变量 `c` 复位两次 → 同落 r5。
3. **返回 s8 + while 循环无显式 default return** (同经验 132 精神)。

`*0xC8` 走 `muls` (非 2 幂), obj base 的 `objectIndex*0xC8` 与 formation 的**各算一次** (目标 r1 早、r3 晚, 非共享变量)。
fndiff 指令域全等 (mul 操作数序/`add rX,#imm` 写法是反汇编伪差, 字节同; 唯 `adds r3,r7,#0` vs `mov r3,r7` 需真字节, 实测合入后 fncheck OK 说明真编译下即 adds 形式)。
**并发提示**: 合入后 make 红, `fncheck --blame` 归属 6 字节全在 code_801A3C4.o (另一 agent 正在匹配 sub_801DDB0/sub_8020B54, 0x08020b59=sub_8020B54), 本函数区 0x45860-0x45940 零差异 → 非我之锅, 照常提交不回退。
关联: 经验 132 (switch/循环 getter 的 stale-r0 默认返回)、经验 97 (u32 临时量影响 load home)、经验 130 (命名符号)。

## 2026-09-04 sub_801DDB0 挂起 (opencode) — byte-exact 真身会扰动姊妹 sub_8020B54 的脆弱 tiebreak

语义: `sub_801B81C(arg0+0x3C, 0x78, 0x50, 0xDA<<1, 0xE, tbl->field_0, tbl->field_4+(arg1<<5), (u16)(0x549+tbl->field_8), tbl->field_A, 0x402)` (tbl=gUnk_0839B2D4, 10参调用); 然后 `arg0[0x66]=3; arg0[0x54]|=0x80; arg0[0xB0]|=0x2000; REG_DISPCNT|=0x8000; REG_WINOUT|=0x1400` (0x4a=WINOUT 非 BLDCNT!)。姊妹 sub_8020A0C 同族。

**byte-exact 真身已找到** (fncheck OK 148B, 见 candidates/byte_exact_named_symbol.c):
- arg7 `(u16)(0x549+tbl->field_8)` 目标要 const-as-accumulator (`ldr r2,=0x549; adds r1,r2,#0; ldrh r2,[r4,#8]; adds r1,r1,r2`)。裸地址 `(T*)0x0839B2D4` 给 field_8-first(144B, 少那条拷贝); **命名符号 `T *tbl = gUnk_0839B2D4`** 的重定位载入改变调度→命中 const-first (同 Sprites_LoadMapNPCs 经验 130)。
- arg0[0x54]/[0xB0] 的 `|=const` 目标要 const-as-accumulator (`ldrh r1; movs r0,#const; orrs r0,r1`); 用 `nv = const | x; x = nv` 临时变量形式命中 (同 sub_8020A0C 的 newval)。

**⚠ 致命冲突**: 引入 `gUnk_0839B2D4` 符号引用后, **扰动同一 C 文件 内姊妹 sub_8020B54 的 local_alloc 平手** (它带 rule116 do-while hack): 其 `gUnk_03000714/715` 地址载入的 r5/r6 互换 (目标 714→r5/715→r6, 变成 714→r6/715→r5)。
- 裸地址版 sub_801DDB0 → sub_8020B54 正确 (r5 first); 命名符号版 → sub_8020B54 翻转。即 agbcc 的 local_alloc 平手受**新符号引用**影响 (全局符号表/伪寄存器状态泄漏)。
- 重调 sub_8020B54 失败: swap 源序 (715;714) 修好 load 却把 store 序换成 715,714 (目标 714,715); do-while 屏障会把被屏障者推到 r4 (v1 屏障714→714落r4, v2 屏障715→715落r4), 无法只修 714/715 的 r5/r6。load/store 与源序耦合, 无解。
- 叠加: 当前 code_0.h 有**并发** agent 的 sub_80455A0/sub_8045860 原型改动, 已使 code_8044394.o 红 (非我), 无法验证全绿。

**结论**: sub_801DDB0 本身可 byte-exact, 但落地会回归姊妹 sub_8020B54 (脆弱 hack 受新符号扰动)。需协调: 要么找不引入新符号的 arg7 const-first 写法 (裸地址+屏障已试无效), 要么把 sub_8020B54 的 tiebreak 重做到对新符号表鲁棒。已回退未落地, 保 sub_8020B54 绿。

## sub_8045688 (0x08045688, code_8044394) — ✅ 2026-09-04 claude-45688
sub_80455A0 (getter) 的 **setter 对应版**: `sub_8045688(u8 objectIndex, u8 stat, u8 val)`。
索引式与 getter 完全相同 (`base=GetObjPool(); formation=gBattleFormationIds[obj[objIdx*0xC8+0xBB]]; if(f)f--;`),
`switch(stat 0..7)` 对 `gPartyStats[f]` 做 `+= val`:
0→max_hp, 1→max_mp, 2→base_atc&atc, 3→base_def&def, 4→base_agl&agl, 5→base_men&men, 6→base_res&res, 7→base_noa&noa。
(2-7 同时加 base 属性(u8@0x16+) 与当前属性(u16@0x6+, 唯 7 的 noa 是 u8@0x10))。

**验证经验 132 机制**: getter 因默认 `return base` 迫使 base→r2 拷贝 + arg0→r7; 本 setter **无默认返回 base**, 故 `base` 直接用 r0 (`adds r1,r1,r0` 无拷贝), arg0→r4。同一索引式两种分配差异的根因就是有无 default-return, 印证经验 132。
struct 须 0x40 (stride `lsls #6`)。原型 `void()`→`void(u8,u8,u8)` 全 (仅挂起的 sub_8049AD8 调用, 安全)。
fndiff 指令域全等 (跳转表数据/`@=rN` 注释为伪差); bytecmp 4B 差 = bl(GetObjPool) 槽伪影; fncheck 292B OK, make+sha1 绿 (并发 code_801A3C4 已被对方收尾)。
关联: 经验 132 (getter/setter 索引式与 default-return 决定 base home)。

## sub_80457AC (0x080457AC, code_8044394) — ✅ 2026-09-04 claude-457ac
同步 5 个编队槽的战斗对象 → gPartyStats: `for(i=0;i<=4;i++){ f=gPartyMemberIds[i]; if(f==0xff)continue; if(f)f--; st=&gPartyStats[f]; ob=pool+i*0xC8; st->hp=obj[0x6C]?:1; st->mp=obj[0x70]; st->equip_slot5=obj[0x91]; st->equip_slot6=obj[0x92]; if(obj[0x91]==0xb3||obj[0x92]==0xb3) st->field_unk[1]=0; else st->field_unk[1]=obj[0x88]; Stats_BuildSkillList(&st->skills[0],st->lv,gPartyMemberIds[i]); }`。
索引式与 sub_80455A0/868 同族 (base=GetObjPool 直用 r0)。

**卡点 = arm_reorg 延迟槽调度**: 目标 `movs r6,#0` (fu[1]=0 的零) 落在 **mp `ldrh` 的延迟槽** (0x46, mp load 与 mp store 之间)。
标准 C 下 GCC2 把这条零 hoist 到 **hp store 之前** (0x30) → 差 0x3e 起 8 字节。
穷举无效: 三元/`if(h==0)h=1`/readall/两if/变量z/块作用域/`if(0);`/标签/空语句 —— 全把零留在 0x30 或更糟。
**解**: hp 语句后插一条**空循环** `do {} while(0);` (或 `while(0);`/`for(;;)break;`) → 产生 NOTE_INSN_LOOP_BEG/END,
改变 arm_reorg 的延迟槽填充决策, 零被压到 mp ldrh 槽 (0x46), 逐字节命中。
纯 no-op, 不改数据流/语义 (区别于经验 18/113 的偷改数据流 hack); 先例 = sub_8045EB8 的"死语句留 movs rN,#0"注释。
**符号坑**: 0x03004AA0 的符号是 `gPartyMemberIds` (小写 s, iwram.h:803), 非 code_8005020.c 注释里的 `gPartyMemberIDs` (大写, 未登记→未解析)。
fncheck 180B OK, make+sha1 绿。新经验 134。

## 2026-09-04 `sub_8045940` 挂起 (code_8044394, 战斗技能按类别过滤收集)
`u8 sub_8045940(Obj*obj,u8*buf)`: 清零 buf[0..7]; 遍历 8 槽 i, `sub_80488CC(obj,obj->skills[i])==0xff` 则跳过;
`switch(obj->cat 0..7)` 按类别对 skill 值做范围判定, 通过则 `buf[count++]=i`; 返回 count。
skills 在 obj+0x99, cat 在 obj+0xBE。跳转表 cases 0,1,6,7→accept, 2→{8,0xd}, 3→[0xe,0x10], 4→[0x16,0x18]∪[0x1a,0x1c], 5→[0x1e,0x1f]∪{0x21}。

**已突破** (候选存 permuter/sub_8045940/base.c, LCS 57/83):
1. **结构体视图** `typedef struct{u8 pad0[0x99];u8 skills[8];u8 pad2[0x1D];u8 cat;}Obj;` + `obj->skills[i]` → GCC2 把 `obj+0x99` 当成员地址 hoist 进 **r7** (目标正是 r7), buf→r6 ✓。裸 `obj[0x99+i]`/`*(obj+0x99+i)` 都不 hoist (GCC2 重结合成 (obj+i)+0x99)。
2. **count 设 int** → 优先级 size×4 压过 i, count 拿到 r4、i 拿 r5 (修好 i/count 互换)。
3. case 内比较写成目标顺序 `if(v>hi)break; if(v<lo)break; accept;` (非 `>=&&<=`) → 避免 combine 的 `(v-lo)<=(hi-lo)` 减法技巧, 得两条独立 cmp。

**剩余两难 (卡点)**:
- **signed vs unsigned cmp**: 目标 ldrb(u8 载入) 后用 `bgt/blt/ble`(有符号, 常量 14/22/26/30); 我方 u8 值被 GCC2 转 `bhi/bls`(无符号, 常量 13/21/25/29)。s8 字段可转有符号但多出 `lsl/asr` 符号扩展 (目标无)。未找到 ldrb+有符号+无扩展 的写法。
- **count 类型两难**: count 设 int 修好互换但 `count++` 无 u8 截断 (目标 `add r0,r4,#1;lsl;lsr r4` 是 u8 截断 → count 实为 u8); count 设 u8 有截断但 i 抢走 r4 (互换复现)。需 count=u8 且优先级>i 的写法。
- permuter 卡 ~3995 (结构分配问题它不解决)。

**下一步**: ① 试 obj->skills 经 `int` 中间量但保持 per-case 重载 (目标每 case ldrb 重读, 因 call clobber); ② 查 count=u8 时如何降 i 优先级 (loop1 用独立 j 会破坏 obj→r2, 需另法); ③ 对照 sub_80488CC 的 skill 类型。基线红为并发 (见 INCIDENTS), 本函数以 fncheck 自证。

## sub_804DE8C (2026-09-04, gpnux)

道具页收集函数: 清 gUnk_03000DC8[0..4] + gUnk_03000D88[0..0xf] 的 field_0/1,
置 gUnk_03000DDC=0, 再遍历 gInvPageItemIds[0..0xf], 对 gUnk_03004980[id]!=0 的项把
(id, 数量) 写进 gUnk_03000D88[gUnk_03000DDC] 并实时 gUnk_03000DDC++。
与子函数 sub_804DE20 (写 gUnk_03000D48/gUnk_03000DDD) 成对, 一条候选 C 直接全绿。
关键点: permuter 对含绝对 RAM 符号的函数因字面池 R_ARM_ABS32 未重定位 score 假高(
经验 29/§9), 在 compile.sh 末尾追加 .equ 把 globals 化成绝对地址后 score 真到 0;
合入 src 仍用真 extern, fncheck/bytecmp 136B 通过, make+sha1 全绿。

## sub_8046060 (0x08046060, code_8044394) — ✅ 2026-09-04 opencode-46060
对象行动状态推进 (M19/ATLAS 同构族 M19 2函数, 姊妹 sub_8045F94 仅 CBA4 第2实参 0xA vs 4, 其余 104 条指令逐条相同):
`if(obj[0xBE]>0x70 && arg1!=8) return; if(obj[0xAB]>=arg1) return;` (方向槽只增不减),
`zero=0; obj[0xAB]=arg1;` 1..7 → `sub_801D12C(obj,1)` + 按 `obj[0xBE]<=0xA` 分派 `sub_801CBA4(obj,0xA,f2A,f35,0)` / `sub_801CA08(obj,0,f2A,f35,0)`;
8 → `sub_801D12C(obj,2)` (离场); 尾部 switch (case1 fallthrough): `obj[0xB8]|=0x10` → `obj[0xB0]|=0x200` → `obj[0xA8]=0`。
两个调用点都在 sub_801B964 (BattleTask_Run 启动的编队展开循环), 传 arg1=2。

**匹配路径** (permuter 分数 1465→840→270→130→80→85→70→0):
1. 直接借姊妹 permuter/sub_8045F94 的 base.c 种子, 改 CBA4 实参 + 从真反汇编重建 target.s (label 全量重映射比 sed 单行改更稳)。
2. 起手候选与姊妹同款结构但偏移 (arg1>=1/<=7 形式) → 270 分; 改成姊妹的 switch-case1..7 形式 → 130。
3. **主要卡点 = 尾部三个 store 的分配**: 目标是 `movs r3,#0x80; lsls r3,r3,#2; adds r0,r3,#0; movs r3,#0; orrs r0,r1; strh r0; …; strb r3` —
   0x200 常量临时与最终 0 **共用 r3** (生命区不重叠 b2-b6 vs b8-c2), orr 用 r0=常量拷贝作累加器 (const-as-accumulator)。
4. 手工逼近路径: ①第5实参的 0 与 `obj[0xA8]=0` 的 0 必须是**两个独立变量** (合并则 GCC2 把 strb 源复用 arg5 的 r7); ②`moveBits(u16) = 0x200|y` 直接 store → orrs r0,r1 ✓ 但独立 0 变量恒落 r1@ba; ③把 0x200 临时复用 `zero` (u32) → orr 翻成 r1,r0 ✗ (int 温度); ④`walkOfs=0` 放在 moveBits 计算**之后**、store 之前, 让 0-pseudo 后分配 → r1 依旧。
5. **破局 = permuter 在 u1.c 基础上加一句 `stateFlags = moveBits;` (int 中转拷贝后再 store)**: r0 的 orr 结果经 int 临时中转, GCC2 把 0-pseudo 的颜色让给 r3 (blocked by r3-read@b6, def 落 b8)。人类化: moveBits(u16)算 → walkOfs=0 → stateFlags(int)=moveBits → store stateFlags → obj[0xA8]=walkOfs。复验 score=0 + fndiff 逐指令全等。
6. code_0.h 原型 `void sub_8046060()` → `void sub_8046060(u8 *, u16)` (调用点 BattleTask 尾截断传参, 无字节影响)。

fndiff 0 / permuter 复验 0 / fncheck OK 204B / make+sha1 全绿。
关联: 经验 87 家族 (0 常量伪寄存器复用), 经验 18 (const-as-accumulator: orrs r0,r1 的 dest=常量拷贝侧)。
姊妹 sub_8045F94 卡在 orrs r1,r0 翻转 (两块同时), 其 score-30 候选已含 `zero=0x200|y; v=zero` 的复用形态 — 建议接手者从 permuter/sub_8045F94/output-30-1 起步, 试着把 case1 的 `v=new_var` 中转改成直接 store。

## sub_8045F94 (2026-09-04, gpnux)

接手 opencode 认领; 与已匹配姊妹 sub_8046060 完全同族, 仅 sub_801CBA4 第 2 实参 = 4
(vs 0xA)。直接套用经验 135 模板 (moveBits(u16)=0x200|y; walkOfs=0; stateFlags(int)=moveBits;
*(u16*)(obj+0xB0)=stateFlags; obj[0xA8]=walkOfs), fndiff score 0, permuter base=0, 无需探索。
中途 bytecmp 报 16 字节差 = bl 槽 (sub_801D12C/CBA4/CA08) 在 .text 0 基址部分链接时距离过远
产生长分支 veneer, 是工具假象, 以 fncheck 为准 (204B OK)。
顺带把 code_0.h 里 sub_8045F94 的 K&R 空原型升级为 (u8*, u16) 全原型 (无调用点, 安全),
make+SHA1 全绿。

## sub_804666C (0x0804666C, code_8044394) — ✅ 2026-09-04 opencode-46060
行动点收集+处理: `sub_804DE8C()` 重置 (0x03000DC8/0D88 道具区) → 收集 `sub_8045F10(obj+slot*0xC8, 0x43)==2`
的槽号到 buf[5] → 逐个 `sub_80466F0(base+idx*0xC8, idx)`。与已匹配 sub_8046C50 完全同构
(0x6C50 对每个槽写 obj[0xBC]=4, 本函数改为调 sub_80466F0; 且头部多一个 sub_804DE8C 调用)。

**2026-09-02 挂起卡点的真因**: 旧 note 归因为"第三循环 ldrb r1,[r0];adds r0,r1,#0 的寄存器 home 差 2B" —
走偏了。真因是**调用实参**: 目标 `ldrb r1,[r0]` 把 indices[j] 载入 r1 后**没有清零**, r1 直接作为
sub_80466F0 的第 2 实参 (r1=idx), 即 `sub_80466F0(obj, idx)` 而非 `sub_80466F0(obj, 0)`。
改一个实参后 fndiff 一次命中 0 分 (含那 2 字节)。sub_80466F0 内部 arg2→sl, 最终传给
sub_804DF74(gUnk_030008A8[r5], obj, index) 作 gUnk_03000DC8 的槽索引 — 语义自洽 (按对象槽号写状态槽)。

base.c 即人类代码 (索引式 idx 复用); permuter score=0 首轮即中; fncheck OK 132B; make+sha1 绿。
教训: **挂起 note 的"差 N 字节=纯寄存器 home"结论要先复核语义可能性 (实参/返回值) 再死磕分配** —
ldrb 进 r1 而非 r0, 最常见的原因就是 r1 的值在后面还要用 (这里就是实参)。

## sub_804612C (2026-09-04, gpnux)

对象状态推进, sub_8046060 家族. 语义: 方向槽 obj[0xAB]==arg1-2 或 arg1==0xB 才进入;
arg1==0xB 且方向==8 直接返回. 方向==8 = 入池: 在 GetObjPool 池槽里找 field_BE 与
obj[0xBE] 匹配的下标 i (i<=4), 调 sub_801DD04(obj, i, *(u16*)(obj+0x6C)+arg2);
否则清 obj[0xAB]=0 后按 obj[0xBE]<=0xA 分派 sub_801CBA4(obj,0,...)/sub_801CA08.
尾部 obj[0xB8]&0x10 → &0xFFEF, 再 sub_801D12C(obj,0).
关键点 (fndiff 2200→0 全路径):
① `(u16)(arg1 + 0xFFFE)` 显式 u16 回绕减 2 写法, 目标用 add-0xFFFE+掩码而非 subs #2
   (probe6/11 的 (u16)(arg1-2) 出 subs+掩码, 差 4B; 必须先试 +0xFFFE 形式, 记入 EXPERIENCE 136);
② sub_801DD04 是 3 参 (obj, idx, val) — 池扫循环变量 i 就是 idx 实参, 首次挂起因原型只给 2 参;
③ pool 指针复用为 &obj[0xB8] (经验 87 一变量两值) 买尾部 home: 单独 b8 指针或 flags 变量都差
   30B (指针 home 换 r2/r3); ④ 5th 实参 0 与 obj[0xAB]=0 用独立 zero 变量 (经验 135 同族);
permuter base=0 + fndiff 0, fncheck OK 240B. 尾部 b8 读写经 pool 复用后逐字节命中.

## sub_8024618 (2026-09-04 opencode; 2026-09-08 matcher 续攻)

⏸ 未匹配 (字节 111/240)。permuter 最佳 score: 2475 → **520** (2026-09-08, 目标 1000 已达)。

### 2026-09-08 续攻记录 (matcher)

从 2475 一路压到 520 的每一步都来自**纯 C 结构改造**, 没有用 register asm / 内联汇编。
候选套件留在 `permuter/sub_8024618/`, base.c = 最终 520 版本 (人类可读, 含注释)。

**关键突破 (按贡献排序):**

1. **块顺序 = 零路径当分支尾**: 写 `if (ptr[i] != 0) { 非零 } else { 零 }` 让 GCC2 产出目标
   的 `cmp r0,#0; beq zero_label` (非零路径 fall-through)。倒着写 (零路径在前) 寄存器流完全不同。
2. **`if (ptr[i] == 0)` 直接比较 (不落 `b` 局部)**: 零路径才不折叠 `constVal + ptr[i]`, 保留两次
   `ldrb` 重读 (与目标一致)。先 `b = ptr[i]` 再比较会让 GCC2 把第一存折叠成纯 constVal (丢指令)。
3. **循环用 `data[idx+i]` 而不是 `ptr[i]`**: 让 data 和 idx 两个 allocno 跨第二循环存活,
   寄存器压力 +1 → **dest 从 r7 顶进 r8** (约 500 分)。代价: 循环底 `ptr[i]` 的 `adds rX,rY,#0`
   变成 `adds rX,idx,i; adds rX,rX,data` 两条 (约 100 分), 净赚约 400 分。这是 `ptr[i]` 派
   (v16, 1665 分) 与 `data[idx+i]` 派 (520 分) 的本质分野。
4. **`out` 指针只在非零分支赋值使用**: `out = (u16 *)(dest + i*2); tmp = 2b+0xFFFFB000; *out = tmp;`
   第一存经 tmp (u32 变量) 才不会把 0xFFFFB000 折叠掉; 零路径两个存都用内联地址。
   注意 permuter 曾产出"new_var 只在 else 赋值、零路径也读它"的 UB 候选 (score 765/1175 类),
   全部**不合格** — 零路径首迭代 new_var 未初始化 → 写错地址。人工核对访存是必须的。
5. **第二常量写 `x - 0x4FFF`** (而非 `x + 0xFFFFB001`): 直接 `+0xFFFFB001` 会被 GCC2 折叠成
   池 0x0000B001 (u16 截断吸收高 16 位), 池少 0xFFFFB001 条目 (640 分); `- 0x4FFF` (= +0xFFFFB001
   mod 2^16, 语义等价因 u16 截断) 让 GCC2 保持 0xFFFFB001 池条目 → 520 分。附注释说明。
6. **prologue 顺序**: `limit` 计算必须在 `dest` 之前 (目标先 lsls/lsrs 再 ldr/adds/mov r8);
   arg1+0x99 要经 `lp` 临时变量强制 `(arg1+0x99)+limit` 顺序 (否则 GCC2 重组为 arg1+limit+0x99)。

**剩余 520 分构成**: regalloc 40 字段 (200) + reorder 2 (120) + insert 1 (100) + delete 1 (100)。
主要是 home 分配仍差: 目标 data→r1/idx→r4/limit→r5/constVal→r6, 我方 data→r5/idx→r6/
constVal→r7 (data/idx 存活的副作用)。要归零需目标式 "data 早死 + r7 作 const scratch" 的
分配, 但那条路 (ptr[i] 派) 实测 1665 分, 得不偿失。判定: 520 是纯 C 结构能到的局部最优附近,
再往下要动 agbcc global-alloc 排序。

**已排除**: register asm (经验 62 禁)、goto (经验 100 禁)、permuter 的 UB 偷改 (constVal=i、
跨分支用未赋值 new_var — 全部语义破坏, 分数低但字节不真)。

### 语义 (旧记录保留)

### 语义
- `arg0`: u16* tilemap 基址; `dest = (u8*)arg0 + 0x442` 为目标写入指针
- `arg1`: u32 数据指针; `arg2`: u32, 取高字节做 limit
- 若 limit≤7: limit = *(u8*)(arg1 + 0x99 + limit)
- sub_8019F08(arg0, 1, 1, 17, 28, 2): 在 tilemap 上 (1,17) 画 28×2 矩形, addVal=1
- data = sub_801878C() (=gGstate340, 指向 gUnk_080936A0+offset)
- 第一循环: 遍历 data 直到遇到 limit 个 0xFF, 记录 idx
- 若 data[idx] != 0xFF, 进入第二循环, 逐字节写入 dest:
  - 字节==0: dest[i*2] = dest[i*2+0x40] = 0xB001 + byte (=0xB001)
  - 字节!=0: dest[i*2] = (byte*2)+0xB000; dest[i*2+0x40] = (byte*2)+0xB001
  - 直到 data[idx+i]==0xFF

### 关键匹配尝试
- 常量 0xFFFFB000/0xFFFFB001: 必须用 u32 局部变量加载 (直接表达式会被编译器优化为 0xB000/0xB001, 池常量不对)
- limit 提取: `tmp2 = arg2 << 24; limit = tmp2 >> 24` 产生目标的两条 `lsls #24; lsrs #24`
- r8 home 寄存器: 必须声明 constB000/constB001/constVal 三个 u32 变量, 才能让编译器溢出 r4-r7 使用 r8
- 第一循环: `if (idx < limit) { do { ... } while (ffCount < limit); }` 结构比 `while (ffCount < limit)` 好 635 分

### 剩余差异 (GCC2 寄存器 home 分配, 非纯 C 可解)
| 变量 | 目标 | 我 | 
|---|---|---|
| dest | r8 | r6 |
| limit | r5 | r4 |
| data | r1 | r2 |
| ffCount | r3 | r1 |
| idx | r4 | r3 |
| constVal(0xB001) | r6 | r7 |
| constB000(0xFFFFB000) | r7 | r8 |
| constB001(0xFFFFB001) | r3 | ip(r12) |

### 2026-09-10 续攻 (zcode-8024618): 520 → 25 (字节 208 中仅剩 5 差)

**结论: 从 111/240 字节差一口气压到 5 字节差, 104 条指令形状全部一致。**
剩 5 字节 = 全局-alloc 两个相邻 home 互换, 已定性为 C 层不可达 (经验 119/88 域)。

**⚡ 关键突破: clamp 分支的两臂写成完全相同 (identical-arm merge)。**
自然读法是 `if (limit <= 7) { limit = ...; }` 然后顺序放后续 setup。改成

```c
if (limit <= 7)
{
    lp = arg1 + 0x99;
    limit = *((u8 *)(lp + limit));
    sub_8019F08(arg0, 1, 1, 17, 28, 2);   /* 两臂重复 */
    data = (u8 *)sub_801878C();
    idx = 0;
    ffCount = 0;
}
else
{
    sub_8019F08(arg0, 1, 1, 17, 28, 2);   /* 与上面逐字相同 */
    data = (u8 *)sub_801878C();
    idx = 0;
    ffCount = 0;
}
```

GCC2 把两臂当同一基本块合并 (不生成分支), 但 **allocno 创建顺序被错开** —— 这一步就把
`dest` 顶进 r8 (经验 47: 活跃值恰好溢出 r0-r7 才动高位寄存器), 并让
`idx→r4 / limit→r5 / ffCount→r3` 全部命中目标。此前所有"数据流改造"派 (per-at-i 存活等) 都做不到。

**循环 2 形状 (与上面合并结构配套才成立):**
`ptr = data; ptr = ptr + idx;` + `while (ptr[i] != 0xFF)` —— 注意不是 `ptr = data + idx`,
也不是 `data[idx + i]`。目标此处的 `adds rX,rY,rZ` 两条加法序列正是 `ptr + idx` 落成两段。

**唯一剩余的 5 字节 (未归零):**
循环 1 有一份循环上限副本 (目标 `adds r2,r0,#0` 落 **r2**, `data` 落 **r1**; 我方 r2/r1 互换)。
根因是 **global-alloc 优先级** (经验 88/119): 该副本 allocno refs=3 / live=4 →
`pri = 1*3/4*1e4 = 7500`; `data` (跨两个循环) refs=5 / live=27 → `pri = 2*5/27*1e4 ≈ 3703`。
副本先拿 r1。要翻转必须 `data` refs≥8 或 live≤13, 或副本 live≥8 —— 本轮已穷举
~900 个结构化变体 (两臂内容/{声明顺序 200 组}/{循环 1 八种写法}/{u16·u32·s16 类型置换}/
变量复用/{循环上限副本变量 n, bound 内外} /permuter 从 520→35→25) **均不可达**。
抬 refs 的路已由经验 119 判死 (`(void)x; x=x; x|=0` 等全被 tree/CSE 折掉)。

**不要再试的方向** (本轮实测无效): 声明顺序、`do{...}while(0)` 屏障、`for(;;)break`、
`data[idx+i]` vs `ptr[i]`、`constVal` 具名/字面量、`-0x5000/-0x4FFF` vs `+0xFFFFB000/+0xFFFFB001`
(两者等价, 但字面量写法会丢池条目)、`idx`/`i` 复用为循环 2 计数器。

### 2026-09-10 二次续攻 (zcode-8024618): 25 → **10 (仅剩 2 字节差)**

**进展**: 继合并两臂突破 (25/差 5) 后, permuter 找到 `long long lp` 变体 — 把 `lp`
声明为 **long long** (DImode) 而非 u32:
`long long lp; lp = arg1 + 0x99; limit = *((u8 *)(lp + limit));`
score 25→10, 字节差 5→**2**, 104 条指令只剩 1 条**操作数顺序**不同。

**末 2 字节的 RTL 级定性 (不可归零的原因)**:
- 目标 0x1c: `adds r0, r0, r5` (RTL `(plus:SI lp limit)`, 顺序 = C 书写序)
- 我方 0x1c: `adds r0, r5, r0` (RTL `(plus:SI limit (subreg:SI (reg/v:DI 34)))`)
- 根因: `lp` 为 long long 时 GCC 先产 `(zero_extend:DI lp)` (insn 41), 加法变成
  **SI×DI 混合** `(plus:SI limit (subreg:SI lp_DI))` — limit (纯 REG 'o' 类) 排前,
  lp 的 DI subreg 排后 → 操作数交换。改 32 位 lp (int/u32) 则加法顺序正确 (纯 SI plus)
  但**全局 home 全错** (回到 5 字节差) — 正是 DI 链把全局格局重排到只剩 2 字节, 二律背反。
- 已穷举无效: `(u32)lp`/`(int)lp` 强转、`lp[limit]`/`*(u8*)lp+limit` 指针形式、
  +0/×1/取负/括号屏障、limit u32 中转、声明位置全排列 — 全部停 2 字节或恶化到 5/130+。
- permuter 最终轮 (从 10 分 base) 9852 迭代确认 10 为该结构最优。

### permuter 套件
`permuter/sub_8024618/base.c` — 当前 = **2 字节差**版本 (score 10, 人类可读, 无 UB),
关键三要素 = 合并两臂 (EXPERIENCE 210) + `ptr=data; ptr=ptr+idx;` + `long long lp`。
状态维持 [0]。

## sub_8025650 (0x08025650, code_8020D50) — ✅ 2026-09-04 opencode (接管自 gpt)

### 语义
写 tile 边框: obj=arg0, 若 arg1!=0 则全局计数器 gUnk_0300076E=(+1)%16;
arg1&1==0 → obj[0x22C]=obj[0x36C]=0xB000; 否则按 arg1&0x10 / arg1&0x20 分别写
(gUnk_0300076E>>3)+0xB9DE / +0xB1DE 或 0xB001 到 obj[0x22C] / obj[0x36C]。
⚠ %16 必须写 `% 16`(asrs/lsls/subs 序列), 写 `& 0xF` 字节不同。

### 卡点与破法 (历史 10B 平台 → 一次归零)
前两轮 (v5/v6) 卡在 10 字节: arg0↔r6 与 &gUnk↔r7 home 互换 + subs 落 r1 非 r0 +
0x32 处 `ldrh r0,[r6]` 重读被 GCC2 CSE 转发成 `lsls/lsrs` 双移位 (0x5c 处因中间有
p[15] 指针写打破转发反而是对的)。
**正解 = 经验 111 分支内局部化重读**: 把两处 `gUnk_0300076E` 读改成块内短命局部
`u16 c = gUnk_0300076E;` / `u16 c2 = ...`。连锁效果: subs 结果落 r0 → 被
`movs r0,#1` 位测试破坏 → CSE 等价类失效 → 0x32 真重读; 同时 global-alloc 的
arg0/cnt home 一并归位 (r7/r6)。三处 10 字节一次全消, bytecmp OK 148B。
合入时 u8 形参 + 头文件空原型 `()` 触发 "default promotion can't match" 编译错误
→ 按经验 114 改 K&R 旧式定义, 字节不变, fncheck OK, SHA1 绿。

## sub_8025518 (0x08025518, code_8020D50) — ⏸ 2026-09-04 opencode (挂起)

### 语义 (已全解)
`u16 sub_8025518(u32 *out, u8 a1, u8 a2, u16 a3, u8 a4, u8 a5, u8 a6)`：
把 7 个入参按位域打包进 2 个 u32 写到 out[0]/out[1]，返回 `(u16)(a3 + gUnk_08393A30[a4*4 + a5])`。
word0 位域: [0-7]=a2, [8-9]=0, [10-11]=0, [12]=0, [13]=0, [14-15]=a4&3, [16-24]=a1, [25-27]=0, [28]=0, [29]=0, [30-31]=a5&3。
word1 位域: [0-9]=a3&0x3FF, [10-11]=0, [12-15]=(a6+0xd)&0xf, [16-31] 不写(保留未初始化栈值)。
表 gUnk_08393A30 = 6行×4列 u8 (ROM 0x08393A30, 待登记 linker.ld)。

### 结构突破 (经验 118)
朴素 struct 局部会被 GCC2 寄存器提升 (整值留 r4, 末尾一次 str)。
目标每写一个字段就 `str [sp]` 且**不重载**(CSE 把上一步寄存器转发给下一步) = 内存驻留位域结构体。
**正解 = union 包裹**: `union { struct S s; u32 w[2]; } u;` 字段用 `u.s.fX=` 写、末尾用 `u.w[0]/u.w[1]` 读回。
union 的 u32 数组视图让 GCC2 放弃提升、对 word0 逐字段写回 [sp]，与目标一致。
g2 再拆 `new_var = a6+0xd; u.s.g2 = new_var;` 让 a6 保持 <<24 延迟归一 (匹配目标的 `+0xd0000; >>16` 移位算术)。
→ permuter 分从 4510 降到 945 (干净候选 cand945.c)。

### 卡点 (global-alloc, 纯 C 未破)
1. **arg4/mask home 互换**: 目标 arg4→sl(r10)、共享掩码 0xFFFFF3FF→r8；候选 arg4→r8、掩码→sl。
   arg5→sb 两边一致。两者各 2 次引用、arg4 活更长，却拿到"较差"硬寄存器 —— 跨块 global-alloc tiebreak (经验 88/117)。
2. **word1 未逐字段写回**: 候选把 word1 累积在寄存器只 str 一次，目标 g0/g1/g2 各 str [sp+4] 一次。
   多出的 temp 可能正是翻转 #1 的关键，但无法用纯 C 强制 word1 也逐字段落内存 (word0 能、word1 不能, GCC2 不对称)。
3. permuter 压到 <945 的输出**全部**靠 `new_var = u.w[0]; out[0] = new_var;` (new_var 是 u16) 截断 word0 高位来省指令 —— 违反经验 18/113 (偷改数据流), 非法, 已弃。

### 下一步候选路径
- 用 `-da` 的 gccdump.greg 比对 arg4 与掩码 allocno 的 pri (经验 117), 找能让 arg4 反超掩码拿到 sl 的生死边界改动;
- 或试 word1 字段间插入对 u.w[1] 的"短命读"逼出逐字段 str (需不增净指令);
- 接手先 `scripts/fndiff.sh sub_8025518 permuter/sub_8025518/cand945.c` 复现 1755 基线。

## sub_80513A0 (0x080513A0) — ⏸ 挂起

**状态**: permuter 最优 score=60，不可达 0；语义完全正确，但 GCC2 指令选择差异不可控。

**函数逻辑** (LZ 解压上下文初始化):
1. `gUnk_03000ECA--` 递减索引
2. `gUnk_03000F30 = gUnk_03000EC0[idx]` 缓存
3. `gUnk_03000E68 = gUnk_03000EC0[idx]` 再次读取(不优化冗余)
4. `lzData = gUnk_087ED6D4[idx]` 获取 LZ 数据指针
5. 读 `REG_DISPCNT` (0x04000000) 检查 FORCED_BLANK (bit7):
   - 若置位: `LZ_InitContext(dest, lzData, uncompSize)` + `LZ_UncompressChunk()`
   - 否则: `LZ_InitContext(dest, lzData, 0x400)` + `gUnk_03000E70 |= 0x200`
6. `gUnk_03000E6C = 0x02016200` (立即覆写)
7. `gUnk_03000E6C = gUnk_03000EA0[idx]`
8. 返回 0 (void 函数但 GCC2 生成 `movs r0, #0`)

**最佳 C 实现** (permuter output-60-1):
- 返回类型 `u32` 而非 `void` (匹配 `movs r0, #0` 尾声)
- 使用局部变量 `new_var` 缓存索引值
- 使用 `gUnk_02016000` / `gUnk_02016200` 符号而非直接地址常量
- 0x400 表达为 `0x80 * 8`

**剩余 60 字节差异** (全为 GCC2 S-bit 指令选择):
| 目标 | 生成 | 差异类型 |
|---|---|---|
| `subs r0, #1` | `sub r0, r0, #0x1` | S-bit |
| `adds r0, r0, r2` | `add r0, r0, r2` | S-bit |
| `ands r0, r1` | `and r0, r0, r1` | 2-reg vs 3-reg + S-bit |
| `adds r1, r3, #0` | `add r1, r3, #0` | S-bit |
| `lsls r2, r2, #3` | `lsl r2, r2, #0x3` | S-bit |
| `orrs r1, r2` | `orr r1, r1, r2` | 2-reg vs 3-reg + S-bit |
| `movs r0, #0` | `mov r0, #0x0` | S-bit |
| `lsls r0, r0, #2` | `lsl r0, r0, #0x2` | S-bit |

**结论**: 这些 S-bit 差异是 GCC2 编译器内部指令选择决策，无法通过 C 源代码控制。
permuter 仅能改变语句顺序/括号放置，无法影响指令选择。此函数标记为挂起。

**新增符号**:
- `gUnk_03000F30` → `include/iwram.h`
- `gUnk_087ED6D4` → `linker.ld`

## sub_8047FCC (0x08047FCC) — ⏸ 挂起

**状态**: permuter 最优 score=75，不可达 0；语义完全正确，但 GCC2 指令选择差异不可控。

**函数逻辑** (8位输入映射):
1. 掩码到 8 位: `lsls r0, r0, #0x10; lsrs r0, r0, #0x10` (16位掩码)
2. 初始化 `r2 = 0`
3. 比较 `r0 <= 0x39` (57)
4. 若 <= 57: 使用 58 项跳转表 (case 0-57)
   - case 0, 1, 10 → r2 = 0
   - case 3 → r2 = 0xfd
   - case 11 → r2 = 0xfe
   - case 26 → r2 = 7
   - case 27 → r2 = 0x1e
   - 其余 → r2 = 0xff
5. 符号扩展 r2 到 r0 并返回

**最佳 C 实现** (permuter output-75-1):
```c
s32 sub_8047FCC(u8 arg0) {
    s8 r2 = 0;
    long long new_var;
    new_var = r2;
    if (arg0 > 0x39) { return new_var; }
    switch (arg0) {
        case 0: case 1: case 10: r2 = 0; break;
        case 3: r2 = 0xfd; break;
        case 11: r2 = 0xfe; break;
        case 26: r2 = 7; break;
        case 27: r2 = 0x1e; break;
        // 所有其他 case → r2 = 0xff
    }
    return r2;
}
```

**剩余 75 字节差异** (全为 GCC2 指令选择):
| 目标 | 生成 | 差异类型 |
|---|---|---|
| `lsls r0, r0, #0x10` | `lsl r0, r0, #0x18` | 掩码移位 (16位 vs 8位) + S-bit |
| `lsrs r0, r0, #0x10` | `lsr r2, r0, #0x18` | 目标寄存器 (r0 vs r2) + S-bit |
| `movs r2, #0` | `mov r0, #0x0` | 寄存器 (r2 vs r0) + S-bit |
| `cmp r0, #0x39` | `cmp r2, #0x39` | 寄存器 |
| `lsls r0, r0, #2` | `lsl r0, r2, #0x2` | 源寄存器 (r0 vs r2) + S-bit |
| `adds r0, r0, r1` | `add r0, r0, r1` | S-bit |
| `movs r2, #0xfd` | `mov r0, #0xfd` | 寄存器 + S-bit |
| `lsls r0, r2, #0x18` | `lsl r0, r0, #0x18` | 源寄存器 (r2 vs r0) + S-bit |
| `asrs r0, r0, #0x18` | `asr r0, r0, #0x18` | S-bit |

**结论**: 掩码移位量 (0x10 vs 0x18) 和寄存器分配 (r2 vs r0) 是 GCC2 内部指令选择决策，
无法通过 C 源代码控制。S-bit 差异同理。此函数标记为挂起。

## sub_801F76C (0x0801F76C) — ⏸ 挂起

**状态**: permuter 最优 score=2070，不可达 0；语义完全正确，但 GCC2 指令选择差异不可控。

**函数逻辑** (arg0[0xbc] 状态判定):
1. 加载 `v1 = (s8)arg0[0xbc]` (符号扩展)
2. 若 v1 == 1:
   - 加载 `v2 = arg0[0xa1]`
   - 若 v2 > 7: 使用 arg0[0xa1] 本身
   - 否则: 使用 `arg0[0x99 + v2]`
   - `v2 -= 6`
   - 若 v2 > 45: 返回 0
   - 否则用 46 项跳转表 (case 0-45):
     - case 0, 2, 3, 4, 6, 7 → 返回 1
     - 其余 → 返回 0
3. 若 v1 == 2: 返回 1
4. 否则: 返回 0

**最佳 C 实现** (permuter output-2070-1):
- 使用 `r3` 作为返回变量 (匹配目标的 `movs r3, #0` 初始化)
- 完整列出 46 个 case (匹配目标的 46 项跳转表)
- 使用 `else if (r0 > 1)` + 内部 `if (r0 == 2)` 结构 (匹配目标的双层比较)

**剩余 2070 字节差异** (全为 GCC2 指令选择):
| 目标 | 生成 | 差异类型 |
|---|---|---|
| `adds r2, r0, #0` | `add r1, r0, #0` | 寄存器 (r2 vs r1) + S-bit |
| `movs r3, #0` | `mov r2, #0x0` | 寄存器 (r3 vs r2) + S-bit |
| `adds r0, #0xbc` | `add r0, r0, #0xbc` | S-bit (同一操作数) |
| `ldrb r0, [r0]` | `ldrb r0, [r0, #0]` | 寻址模式 (无位移 vs 显式 #0) |
| `lsls r0, r0, #0x18` | `lsl r0, r0, #24` | S-bit |
| `asrs r0, r0, #0x18` | `asr r0, r0, #24` | S-bit |
| `cmp r0, #1; beq` | `cmp r0, #0x1; bne` | **分支极性反转** (beq vs bne) |
| `cmp r0, #1; bgt` | (无) | 目标有第二次比较, 生成无 |
| `adds r0, r1, #0` | `add r0, r1, #0` | S-bit |
| `cmp r0, #0x7; bhi` | `cmp r0, #0x7; bgt` | 分支条件 (bhi vs bgt, 等价但不同助记符) |

**结论**: 分支条件极性 (beq vs bne) 是 GCC2 内部指令选择决策，无法通过 C 源代码控制。
这是最关键的差异 —— 目标用 `beq` (相等则跳) 进入主逻辑, 生成用 `bne` (不等则跳) 跳过主逻辑。
虽然功能等价，但指令编码不同。此函数标记为挂起。

## sub_8047FCC (0x08047FCC) — ✅ 2026-09-04 gpnux

**状态**: 已匹配，fncheck OK 112B，全量 make+SHA1 绿。

**函数逻辑** (8位输入映射):
- 掩码到 8 位
- 若 arg0 > 0x39: 返回 0
- 否则用 58 项跳转表 (case 0-57):
  - case 0 → 返回 0 (用 `return ret` 而非 `break`)
  - case 1, 10 → 返回 0
  - case 3 → 返回 0xfd
  - case 11 → 返回 0xfe
  - case 26 → 返回 7
  - case 27 → 返回 0x1e
  - 其余 → 返回 0xff

**关键写法**:
1. **`s32` 返回类型** (不是 `void` 或 `u32`) — 匹配目标 `movs r0, #0` 尾声
2. **`u16` 参数类型** (不是 `u8`) — 匹配目标 `lsls r0, r0, #0x10; lsrs r0, r0, #0x10` 16位掩码
3. **`s8 ret = 0` 前置** — 匹配目标 `movs r2, #0` 初始化
4. **`case 0: return ret;`** 而非 `break` — 创建早期返回，匹配目标跳转表结构
5. **全 58 case 显式列出**，无 `default` 标签 — 匹配目标 58 项跳转表

**教训**:
- 之前的 permuter 探索卡在 S-bit 差异上，但实际上正确的 C 写法可以完全匹配
- `case 0: return ret;` 是关键 —— 早期返回改变了控制流结构
- 参数类型 `u16` vs `u8` 决定了掩码移位量 (0x10 vs 0x18)

## 2026-09-04 sub_8034440 匹配 (opencode, 364B exact) — switch 状态机, 池重定位假差

`switch (gUnk_03000820)` 状态机 (case 0/1/2/5/6/8/9), 同族 sub_8032D74/sub_80392C0。
返回 0/2 (case 9 置 2), 与 sibling 语义一致。

**流程**: src 已有被注释的完整草稿 → mkpermuter 建套件 → 直接 bytecmp 判字节。
- fndiff 逐指令全等 (唯一差异 = 字面池数据 `lsrs/lsls` 假差 + bl 槽, 均重定位产物);
- bytecmp 36 字节差 = 恰好 9 个 bl 槽 × 4B (脚本把 .text 链到 0, bl 出范围插 veneer), 非 bl 零真实差异 (先例 EXPERIENCE 124 / progress 2671);
- fncheck OK 364B (11 池重定位已施加, 9 bl 槽忽略); 全量 make + SHA1 绿 (668/1064)。

**原型修正 2 处** (都只影响未匹配函数的调用点形状, 零字节风险):
1. code_0.h `void sub_801EEE4()` → `u8 sub_801EEE4()` (调用点 `== 1` 消费返回值; 反汇编尾部返回 0/1)。
2. code_0.h `void sub_8034440()` → `u32 sub_8034440()` (返回 0/2, 定义处 `u32` 冲突编译红)。

**教训**: permuter 分数对本函数不收敛到 0 属正常 — 池常量 (gUnk_03000820/822/824 等) 在
单函数 .o 里是未重定位符号, scorer 无豁免 → 每池字假分。此类函数直接 bytecmp 判定即可
(与 progress 2671 sub_801DAA0 同路径)。

---

## 2026-09-04 sub_8053270 (Op_SetFlags) 匹配

### 背景
sub_80532DC(0x080532DC,清位)的姊妹函数(0x08053270,置位)。二者同族同骨架:
`push{r4,r5,r6,r7,lr}; mov r7,sb; mov r6,r8; push{r6,r7}` prologue, 循环
`data[1]>>1` 次,u16 拼装 `data[2+2k] | data[3+2k]<<8`, ≤0x1FF 走 EventFlags_*,
>0x1FF 走 SwitchFlags_*(v-0x200), 尾部 `off = t + 2; *ptr = *ptr + off;`。

### 卡点回顾 (progress 25 问题16)
"循环内高位寄存器home错位(ptr/data/limit)" — 候选曾把 r6/r5/r7 错排成 r7/r6/r5。
实际根因: 直接抄 sub_80532DC 的已匹配变量声明与调度顺序, 一次跑分即 0。
之前误以为要"攻寄存器分配", 实际是"没找到同族已匹配的模板"。

### 解
- `permuter/sub_8053270/base.c` 与 sub_80532DC 逐字对齐, 仅把 `_Reset` 换成 `_Set`。
- 关键不变量(来自 sub_80532DC 的经验, progress 152):
  1. 循环条件写 `n > i`(界在左), 否则 GCC2 不出 `cmp r0,#0; bls`。
  2. `off = t + 2; *ptr = *ptr + off;` 两句独立(规律30), 合并会被重结合。
- permuter 首次跑分 = 0 (无需迭代)。fndiff 逐指令全对齐, 108 字节。
- 合入 src/code_804F0B8.c (替换 INCLUDE_ASM), 走标准收尾:
  gen_asm → fncheck OK → make → sha1sum OK (ll.gba 通过)。
- functions.tsv status 0→1, note 更新。
- code_0.h 已有 `u32 sub_8053270(u32 *);` 原型, 无需改。

### 教训
同族"清/置"位对几乎总可复用同一模板, 差异在 `*Flags_Set` vs `*Flags_Reset`。
先 `grep` 邻近函数是否已匹配 (子_80532DC 已在同一 C 文件), 直接抄骨架比 permuter 硬攻高效。

## sub_8052F44 (0x08052F44) — ⏸ 2026-09-04 gpnux

**状态**: 挂起，permuter 最优 score=1000，不可达 0。

**函数逻辑** (队伍ID匹配+跳转表):
- 输入: `u32 *ptr` 指向脚本数据指针
- `data = *ptr` 解引用得到数据指针
- 检查 `gPartyMemberIds[0] == data[1]`，若匹配则 count=1
- 否则循环 i=1..4 检查 `gPartyMemberIds[i] == data[1]`，匹配则 count++ 并 break
- 若 `count == data[2]`: `*ptr = gUnk_02016200 + gUnk_02016000[data[3]]`
- 否则: `*ptr = *ptr + 4`
- 返回 1

**卡点**:
- GCC2 寄存器分配差异: 目标 `push {r4,r5,r6,lr}` vs 我方 `push {r4,r5,r6,r7,lr}` (多一个 r7)
- 数据加载时机: 目标 `ldr r3,[r5]` 在 prologue 后立即加载，我方延迟到条件检查前
- 移位量差异: 目标用 `lsls #0xc` (左移12位) 做 8 位掩码，我方用 `lsls #0x18` (左移24位)
- 循环结构: 目标的 for 循环展开方式与我方不同

**最佳 C** (permuter output-1000-1):
- 引入 `new_var = data` 副本可能有助于控制寄存器分配
- 使用 `((0, gUnk_02016000))[new_var[3]]` 非常规语法

**结论**: 纯 C 无法控制 GCC2 的寄存器分配和指令选择，标记为挂起。

## sub_800BFF8 (0x0800BFF8) — ⏸ 2026-09-04 gpnux

**状态**: 挂起，permuter 最优 score=650，不可达 0。

**函数逻辑** (坐标转换+tiles计算):
- 输入: `s16 value, u16 *dest, u32 base`
- 若 value==0: tiles = [base+0x27F, base+0x27F, base+0x25A]
- 否则:
  - 计算 hundreds: while (value >= 0) { value -= 100; hundreds++; }
  - 若 hundreds==0: hundreds = 0x25
  - tiles[0] = base + 0x25A + hundreds
  - 计算 tens: while (value >= 0) { value -= 10; tens++; }
  - 若 tens==0 且 tiles[0]==0xB27F: tens = 0x25
  - tiles[1] = base + 0x25A + tens
  - tiles[2] = base + 0x25A + value
- 输出: *dest = tiles[2]; dest--; *dest = tiles[1]; dest--; *dest = tiles[0]

**卡点**:
- GCC2 循环展开方式差异
- 寄存器分配差异
- 常数计算顺序差异

**最佳 C** (permuter output-650-1):
- 引入 `new_var = base16` 中间变量
- 使用 `(new_var + 0x25A) + hundreds` 结构

**结论**: 纯 C 无法控制 GCC2 的循环展开和寄存器分配，标记为挂起。

## 2026-09-04 sub_800FA24 匹配 (opencode, 264B exact) — permuter 机制 + 人工去作弊

物品治愈函数 (switch(gUnk_030001AE): HP/MP 分支)。返回 0x27(无队伍成员)/0x24(已满)/0x23(成功)。

**卡点**: 指令流逐条一致, 但 `chara = &gPartyStats[partyIdx]` 指针 home 是 r2 (应 r1),
级联整个 HP/MP 体 (bytecmp 114B)。穷举 20+ 变体 (u8/u16/u32 索引、ptr 算术拼法、
声明序排列、const_int vs symbol_ref) 全部停在 114。属于 global-alloc home 互换家族。

**破解链**:
1. permuter 中奖 output-400: 在 MP 分支加 `new_var = (gUnk_030001AF) ? (gUnk_030001AF) : (999);`
   的**多余赋值** → 新增局部量改变伪寄存器生死边界 → chara 归位 r1 (bytecmp 114→33)。
2. 但 permuter 把 `new_var` 声明为 **u8** → 999 被截断成 231 (`movs r0,#0xe7`), 语义作弊
   (经验 18/113)。目标是从字面池 `ldr r0,=999`。
3. 人工修正: `u16 amt; amt = gUnk_030001AF; if (amt == 0) amt = 0x3E7; chara->mp += amt;`
   → 保留 999 语义 + 保留 home 翻转。bytecmp 只差 4 个 bl 槽 (16B, 重定位假差), fncheck OK 264B。

**原型修正**: code_0.h `void sub_800FA24()` → `u8` (返回 0x23/0x24/0x27, 调用点
sub_800C2F8 里 `strh r0,[gUnk_030001C8]` 消费返回值)。定义处 K&R 兼容。

**新符号登记** (iwram.h + linker.ld 按地址序): gUnk_030001AE (HP/MP 分支选择),
gUnk_030001AF (治愈量), gUnk_030001B0 (=0x10 置位), gUnk_030001C8 (u16 清零/返回值)。

**教训**: permuter 的"多余赋值"是 home 互换类卡点的有效杠杆 (经验 87 兼职法的变体),
但产物必须逐条核对数据流 (经验 18); 类型 (u8/u16) 不同直接决定字面量是否被截断,
人工改类型后必须重跑 bytecmp 确认 home 仍保留。

## 2026-09-04 sub_800FA24 匹配 (opencode, 264B exact) — permuter 新机制 + 人工去作弊

**语义**: 物品使用执行器 (恢复 HP/MP)。`gUnk_030001AE==1` → 恢复 HP (`hp += gUnk_030001AF`,
上限 max_hp); 否则恢复 MP (`mp += gUnk_030001AF ? gUnk_030001AF : 999`, 上限 max_mp)。
返回 0x27 (无队伍成员) / 0x24 (已满) / 0x23 (成功)。

**卡点**: 指令流逐条一致, 但 `chara = &gPartyStats[partyIdx]` 的指针 home 恒 r2 (目标 r1),
级联整个 HP/MP 体 (bytecmp 114B)。穷举 20+ 变体 (u8/u16/u32 索引、ptr 算术拼法、声明序、
const_int vs symbol_ref) 全部停在 114 —— 属于 global-alloc home 互换家族。

**破解链**:
1. permuter 中奖输出 output-400: 在 MP 分支插入 `new_var = (gUnk_030001AF) ? ... : (999);`
   的**多余赋值** → 新增局部量改变伪寄存器生死边界 → chara 一次归位 r1 (bytecmp 114→33)。
2. 但 permuter 把 `new_var` 声明为 **u8** → 999 被截断成 231 (`movs r0,#0xe7`), 语义作弊
   (经验 18/113 先例)。目标从字面池 `ldr r0,=999`。
3. 人工修正: `u16 amt; amt = gUnk_030001AF; if (amt == 0) amt = 0x3E7; chara->mp += amt;`
   → 保留 999 语义 + 保留 home 翻转。bytecmp 只差 4 个 bl 槽 (重定位假差, 16B);
   **fncheck OK 264B**; 全量 make + SHA1 绿。

**新经验**: EXPERIENCE 143 (permuter "多余赋值" home 杠杆, 但产物须人工去类型截断作弊)。

**符号登记**: gUnk_030001AE (HP/MP 分支选择), gUnk_030001AF (恢复量), gUnk_030001B0 (=0x10),
gUnk_030001C8 (u16 清零 + 返回值消费处)。全部新登记 iwram.h + linker.ld (地址序插入)。

## 2026-09-04 sub_804E6DC 匹配 (agent1, 144B exact) — 经验 108变体 + *12表步长

**语义**: 对象字段检查 + 表查找。`obj[0xBE]<=10` 且 `obj[0x8D..0x94]` 不全为零时,
线性查找 `gUnk_087EA580[data[i]*12+5] == value` (data=obj+0x8D, i=0..5), 命中返 i 否则返 -1 (0xFF)。
姊妹 sub_804E76C 同结构但调 `sub_804DD90(values[i], arg1) == arg2` 间接判定。

**卡点**: 前手挂起(permuter 1650 不可达0)。根因=base.c 公式写错 `*5+5` (应为 `*12+5`):
目标 `lsls #1; adds; lsls #2` = `*12`, 前手产出 `lsls #2; adds` = `*5`。修正后 score 2705→400。

**破解链**:
1. 修正 `data[i]*12+5` → score 2705。
2. 改用 `if (... != 0 || ...)` 替代 `if (... == 0 && ...)` 守卫 → score 400 (与目标一致)。
3. 改用 `result=0xFF; for(...) { if(...) { result=i; break; } }` 累加器形式 (替代 `return i` 内联返回)
   → 触发 GCC2 peel 首迭代 + 循环体 `i++` 前缀 (经验 108 变体, 同 sub_804EF90 的 ret 累加器)。
   permuter 4000+ 迭代未出 0 (池重定位假差 1×400), bytecmp 施加 `gUnk_087EA580=0x087EA580` 后 **OK 144B**。

**要点**: ① 表步长 `*12` 非 `*5` (12B 条目, offset 5); ② 守卫用 `||` 短路非零检测 (非 `&&` 全零检测);
③ 累加器+break 形式触发 peel, `return i` 内联返回不触发; ④ `u8 result` + `result=-1` (s8 扩展返 0xFF)。

**fncheck OK 142B** (临时复制 matchings→nonmatchings 解 sub_8047FCC INCLUDE_ASM 残留构建阻塞后通过)。
bytecmp 144B 亦定论字节一致。

关联: 经验 108 (线性查找 peel + u8 截断 + ret 累加器变体)、经验 29 (池重定位假差)。

## 2026-09-04 sub_80525E8 挂起 (agent opencode) — LZ_BGM 装载入口, global-alloc 墙 (同 sub_80531A8)

**语义**: `sub_80525E8(songId, entry, mode)` — 歌曲/音效 LZ 数据装载入口 (VBlank 泵 LZ 解压, 见文档注释)。
- `gUnk_03000E68 = songId;` + 查 `gUnk_087ED6D4[songId]` 得 `struct Unk_LzData *`。
- `REG_DISPCNT & 0x80` (forced blank): `LZ_InitContext(gUnk_02016000, lz, uncompSize); LZ_UncompressChunk();`
  否则: `LZ_InitContext(gUnk_02016000, lz, 0x400); gUnk_03000E70 |= 0x200` (走 VBlank 泵)。
- `switch(mode)`: case1/other → `gUnk_03000E6C = (u32)gUnk_02016200`;
  case2 → `gUnk_03000E69 = entry; gUnk_03000E70 |= 0x400; gUnk_03000E6C = (u32)(gUnk_02016200 + gUnk_02016000[entry])`。

**已达成 (bytes-exact)**:
- 全函数 .text 尺寸 **0xB8 = 184B 一致**。
- 0x00–0x8c (LZ 分支 + E69/E70 写入) 与 switch 分发 `cmp r4,#1; beq; cmp r4,#2; beq` 逐指令一致。
- default/case1 块 `ldr r1,&E6C; ldr r0,=0x02016200; b 9c` 与共享尾 `str r0,[r1]` (cross-jump) 一致。
- 26/32 条 case2 指令一致; 工厂尺寸/池对齐全对。

**剩余差异 (仅 case2 0x8e–0x9c 窗口, ~12B)**:
- 目标: `[ldr r1,&E6C][lsls][ldr r2,tbl][adds][ldrh][ldr r3,base][adds][str r0,r1]`
- 最佳候选 O1 (permuter output-280-1): `[lsls][ldr r1,tbl][adds][ldr r1,base][ldrh][adds][ldr r1,&E6C][str r0,r1]`
- 候选已含 cross-jump + &E6C→r1 + 共享 store, 只差三处 load 的**顺序/寄存器** (早载 &E6C@8e vs 晚载@9a; tbl/base r1/r1 vs r2/r3)。

**已穷举 (全部无效)**:
- switch 大小写序 / m2c 的 `if(mode==1||mode!=2)` / Store-After-Switch (`scriptPtr` 尾存)
- 直接存 vs `u32 v` 命名临时 vs `u16 tblv`/`u32 base`/`u16 *np`/`u8 *b2` 指针形式 (20+ 组合)
- 函数级/块级 `u32 *dst` 统一跨块指针 (ydst2-8)、`volatile u32 *dst`、冗余二次 store (DSE)
- `do{}while(0)` 屏障 (调度不变)、`-g` flag 变体 (不变)、permuter 4 轮 (含 37825 迭代) 停在 **280**
- 根因定量: case2 早载 &E6C 伪寄存器 = 2refs/14–16insns → pri≈5700–8500, 永远抢不过表基址临时 (2refs/4insns → 20000);
  目标要求 &E6C n_refs≥5 才可能拿 r1, C 层产生不了 (同 sub_80531A8/sub_804DCD8 global-alloc 决策层)。
  cross-jump 是 reload 后运行, 无法回授分配建议。

**最佳候选**: `permuter/sub_80525E8/base.c` (O1/np+b2+v 形式; fndiff 280 = 池偏置假差 + 12 真字节)。
**建议下一步**: ① 找能让 &E6C n_refs≥5 的引用形式 (如让地址在 LZ 区或 default 块被"值"用一次 → REG_EQUIV 计数);
② 或按 EXPERIENCE sub_80531A8 思路给 agbcc 打补丁打印完整 qty 优先级表; ③ 交叉比对同族 sub_80513A0 (同样 LZ+E6C 结构, 挂起)。
README 补充: 本函数与 sub_80513A0 是姊妹装载器, 解一个的另一半概率大。

## 2026-09-04 `sub_8015E1C` 二次尝试 (agent1, 挂起, 70B→65B)

前手 2026-09-02 已全解语义, 卡经验 17类寄存器置换 (r2↔r3, tile/dest/hoist 三处), 8 版候选恒差 70/104B。

**本轮改进** (1105→960 fndiff, 70→65B bytecmp):
1. **去 p 变量, 直接用 arg3**: `while ((b = *arg3) != 0xFF)` + `arg3++` —— 省掉 `u8 *p` 的声明与赋值,
   让源指针直接落 r4 (与目标一致, prologue offset 2 对齐)。
2. **链式赋值 `dest[0] = (dest[0x20] = attr + 1);`** (空白分支): 让两格存储共用同一值寄存器,
   匹配目标的 blank case 结构 (r3/r2 两 home 各存一次同值)。

**残留差异** (经验 17 类, 非本次可解):
- prologue: arg2→r3 (我) vs r2 (目标), base→r2 (我) vs r3 (目标) —— r2↔r3 置换
- 循环体: dest+64 hoist 到 r3 (我) vs 内联 recompute 到 r2 (目标)
- tile→r2 (我) vs r3 (目标), dest→r6 (我) vs r2 (目标)
- advance: r6=2 (我) vs r0=2 (目标)

**结论**: permuter 620 分版 (链式赋值) 是当前最优候选 (fndiff 960, bytecmp 65/104B)。
后续需 fndiff 逐指令长磨 (参考 Text_PutGlyph 作者 2435→2610→0 的过程) 或改编译器 qty 分配。
最佳候选: `permuter/sub_8015E1C/base.c`。

## 2026-09-05 sub_801A6F4 挂起 (gpnux, 结构100%解, 卡 GCC2 CSE 常量替换)

**语义**: 精灵对象调色板装载 + BG 配置。`switch/if (type)` (type = `f_18 & 0xF`):
type 6/7 → 构建 BG1CNT (优先级来自 `f_2A&3`, charbase 3, screenbase 0xF00, 清 0x4000/0xC000)
→ `REG_BG1CNT = 值` → `DmaFill16(3,0,0x06007800,0x800)` + 内联 wait (`if(status<0) do{}while(status&0x80000000)`)
→ `REG_DISPCNT |= 0x200`; type 8 → 逐字节 RMW 构建 BG3CNT (b0: `(b0&~3)|(f_2A&3)` / `(b0&~0xC)|8` / `&~0x30` / `&~0x40` / `&0x7F`; b1: `(b1&~0x1F)|0xD`→`|0x20`→`&0x3F`)
→ `REG_BG3CNT = 值` → `DmaFill16(3,0,0x06006800,0x800)` + wait; 公共尾: `sub_804C548(f_14, f_29, (u8)sub_801B954(arg0))` (调色板 DMA 到 `0x05000000+slot*0x20`)。

**已定死的形状** (逐条验证过):
- 入口 `mov ip, r0` (arg0 全程 ip), `ldrh r0,[r0,#0x18]; movs rX,#0xf; ands rX,r0`。
- **type 必须是 `s16`** 才有目标的分发 `cmp #6;bge / cmp #7;ble / cmp #8;beq` (有符号); `u16`→`bhi`、`int`→`bgt#5`。
- 目标分发 = `switch` 或 `if(type>=6){ if(type<=7){} else if(type==8){} }` (两者同形)。
- BGCNT 构建全部是**逐条独立语句** (合并单表达式会折叠常量, 如 `0xF00|0x2000`→`0x2F00`)。
- type-8 每条 b0 写入是完整 32 位合并 `bgcnt = (…) | (bgcnt & 0xFFFFFF00)`; `~0xC`/`~0x30`/`~0x40` 由
  `movs #0xd/#0x31/#0x41; negs` 物化 (= 源码常量是 `~0xC` 不是 `~0xD`!); `~0x1F` 才是 `movs #0x20; negs`。
- type-8 b1 必须拆成多条语句 (单表达式会被 GCC 折叠 `0xD|0x20`→`0x2D`), 用临时或直接成员访问。
- DMA wait 用 `if((s32)status<0){ do{status=dmaRegs[2];}while(status&mask); }` (同 Op_OpenWindow)。
- 尾调用 `sub_804C548(u32 src, u8 slot, u8 count)` (code_8044394.c:2119), `sub_801B954((void**)arg0)`。

**卡点 = GCC2 CSE 把 type 寄存器替换进 stmt2 的常量 8** (RTL `(ior:SI X (reg 185))`, REG_DEAD 185):
- `case 8:`/`type==8` 分发记录 `beq body8` → CSE 建立 `reg_type == 8` 等价 (qty_comparison_const)。
- 随后 stmt2 的 `| 8` 被 CSE 换成 type 寄存器 (`orrs r2, r5`), type 活范围延到 case-8 体内 (66 insns)
  → 优先级骤降 → type 落 r5 (目标 r1) → 级联全函数寄存器错位 (~2535 分)。
- 已试无效: 死 store `type=0` (被 tree DCE 提前删)、barrier 各种位置 (仅 stmt2 barrier→2155)、
  显式 `(u8)type` (2720)、类型 s8/u8/u16/u32/int、struct/裸指针、`-g` 变体 (同 2535)。

**半个突破口** (结构 `permuter/sub_801A6F4/base.c` 保持 switch 版):
```c
if (type < 6) {} else if (type > 7) { if (type != 8) {} else { /*body8*/ } } else { /*body67*/ }
```
这个 m2c 结构让最后的 `cmp #8` 记录 **NE** 而非 EQ → 无替换, type 落 r1, stmt2 正确物化 `movs rX,#8`,
case-8 体寄存器分配几乎全对 (2250 分)。代价: ① 外层 `if(type<6){}` 被规范成 `cmp #5;bgt` (目标 `bge #6`);
② body8/body67 布局互换 (body8 内联、body67 置后)。`else if(type<=7)` 变体会重新引入替换 (3410)。
下一步方向: 找到同时满足 `bge #6` 分发 + body67 内联 + body8 走 NE 分支的结构; 或按经验 117
定量法抬 type 的 qty 优先级让它长活也落 r1 (它被 r1 上是 m2c 证明可行的)。

## sub_8052F44 (0x08052F44) — ✅ 2026-09-05 opencode (接手 sen1 挂起项, 104B 逐字节)

**状态**: 已匹配。bytecmp OK 104B → 合入 src → fncheck OK (102B + 3 池重定位) → TSV status 0→1。
sha1 当时红在 `code_801A3C4.o` (另一 agent sensenova 正在改的 sub_8020B54, 4B), 与本函数无关。

**最终 C**:
```c
u32 sub_8052F44(u32 *ptr)
{
    u8 *data = (u8 *)*ptr;
    u8 count = 0;
    u16 i;

    for (i = 0; i <= 4; i++)
        if (gPartyMemberIds[i] == data[1]) { count++; break; }
    if (count == data[2])
        *ptr = *(u16 *)((u32)gUnk_02016000 + data[3] * 2) + (u32)gUnk_02016200;
    else
        *ptr += 4;
    return 1;
}
```
语义 = 脚本 opcode: 统计队伍中 ID==data[1] 的成员数, 等于 data[2] 则跳脚本表项 data[3]。

**接手时的错误诊断** (前两轮 sen1 结论"纯 C 不可控, 需原版编译器"是错的):
1. **`i` 必须是 `u16`, 不是 `u8`** —— 目标循环体 `adds r0,r2,#1; lsls r0,r0,#0x10; lsrs r2,r0,#0x10`
   是 **u16** 截断; 累加器 `count` 才是 u8 (`lsls/lsrs #0x18`)。两变量宽度不同,
   前两轮全部假设成 u8 → 卡在 score 1000 / 535 误判"不可达 0"。同一 C 文件 已匹配的
   `Op_RemovePartyMember` 就是 `u16 i` → `#0x10`, 直接可作对照模板。
   → 已写成经验 146。
2. **`i` 不得在声明处初始化** —— 写 `u16 i = 0;` + `for (i = 0; ...)` 会让 `data`/`count` 的
   寄存器 home 在 r3↔r4 互换, 全函数 `ldrb r0,[r3,#1]` / `cmp r4,r0` 级联错位, 差 12B。
   删掉声明处的 `= 0` 即逐字节命中。→ 已写成经验 145。
3. **跳转表写法直接抄已匹配同族** —— 尾部 12 条指令与 `Op_IfEventFlagJump` / `Op_IfSwitchJump`
   完全同形 (`*(u16 *)((u32)gUnk_02016000 + data[3] * 2) + (u32)gUnk_02016200`),
   不必从零推导。这是本函数的最高杠杆一步。

**permuter 用法说明**: 本套件 permuter 最优稳定在 **score 15 = 3 个符号字面池**(permuter 不能施加
重定位, 池里的 `gPartyMemberIds`/`gUnk_02016000`/`gUnk_02016200` 在目标里是硬编码常量)。
**score 15 对本函数即"等价于 0"**, 以 `bytecmp.sh`(施加 abs.ld 重定位后)为准 → OK 104 bytes。
与经验 68 同类: score 不能按池数量机械估算。

**教训**: 挂起项的 note 写"需原版编译器"之前, 应先把**类型宽度**穷举一遍 (`lsls` 移位量是免费判据)。
本次两个卡点都是"一个词"级别的差异, 却在 TSV 里被记录成"不可达 0"。

## 2026-09-05 `sub_801A3C4` 匹配 (sensenova, 552B 逐字节) — GCC2 常量重结合 + 姊妹函数扰动

**结论**: ✅ 真 C 落库, `fncheck.py sub_801A3C4` OK (552 bytes, 19 池重定位已施加, 6 bl 槽忽略)。
ROM 全量 sha1 通过, 匹配进度 676/1064。代价: 本 C 文件 姊妹 `sub_8020B54` 退回 `INCLUDE_ASM`
(见下 §姊妹扰动), TSV 该行 status 1→0。

### 卡点 1: case 1 目标地址的常量绑定 (耗时最长)

case 1 编译 `LZ77UnCompVram(gUnk_087EBE00[f_26+f_22], (void *)((f_24<<5) + 0x06010000 + (f_22<<12)))`。
目标反汇编要求常量先加到**右侧**移位项 (`lsls r2,#0xc; ldr r3,=const; adds r2,r2,r3`),
但按直觉写的 `(A<<5) + ((B<<12) + CONST)` 被 GCC2 重结合成 `((A<<5)+CONST) + (B<<12)`,
产出 `adds r1,r1,r3` (常量绑左侧) → 1 条差。

枚举 21 个变体 (常量左/中/右、指针算术、`u32`/`int`/`long`/`u64` cast、拆成 `0x06000000+0x10000`、
`(CONST + (A<<5)) + ...` 等) 全部失败; 唯一命中是**三向左结合** `(A<<5) + CONST + (B<<12)`,
即"常量夹在两个移位项中间"。已写成经验 147。

判别要点: 别靠猜。`fndiff` 里对比"常量绑哪个寄存器" (`adds r1,r1,r3` vs `adds r2,r2,r3`),
并检查 `lsls r2,#0xc` 是否有独立副本 —— 无副本 = 常量与该项融合。

### 卡点 2: DMA 通道是 3 不是 0

目标字面池 `0x040000d4` = `REG_OFFSET_DMA3`, 写 `DmaFill16(0, ...)` 会编出 `0x040000B0` → FAIL。
case 6/7/8 三处 (控制字 `0x81002000`/`0x81000010`, size 0x4000/0x20) 全改为通道 3。
已写成经验 149。

### 卡点 3: 源码 case 顺序与 fallthrough

源码 case 顺序必须 1,2,3,6,4,5,7,8,9; **case 6 无 break**, 落进 case 4 的 `LZ77UnCompWram`
(0x0202B2C0)。case 7/8 的 `DmaWait` 后不 break。

### 卡点 4: 不能有本地变量

目标 prologue 栈只用 4 字节 (`push {r4, lr}` + `add r0, sp, #4`), 任何 `u32 tmp` 都会撑大栈帧。

### 卡点 5: 池引用符号必须具名

`extern u8 *gUnk_087EBE00[];` 放在函数上方。改 `linker.ld` 的 `SECTIONS {}` **外面**加
`gUnk_087EBE00 = 0x087EBE00;` (绝对符号区, 与 `gUnk_087EBDF0` 同区)。
实测: 只用裸地址 `(*(const u8 * const *)0x087EBE00)` 会改变池布局 → 自身 fncheck FAIL,
所以池常量必须具名符号化。

### §姊妹扰动: `sub_8020B54` 退回 INCLUDE_ASM

本C 文件 (`src/code_801A3C4.c`) 里 `sub_801A3C4` 位于 `sub_8020B54` (0x08020B54) 之前。
把前者从 `INCLUDE_ASM` 换成**任意**真 C body 都会扰动后者在 0x08020B58/5A 的
`ldr r5` / `ldr r6` tiebreak (互换), 连带 0x08020B74/76 的 `strb r0,[r5]`/`[r6]` 互换 → ROM 4 字节差。
该扰动即使 body 只是 `if (obj->f_18 & 1) obj->f_22 = obj->f_22 + 1;` 也触发。

已排除: 只加 `linker.ld` 符号不扰动; `sub_801A3C4` 目标反汇编无 r8/sb/sl → **不是**坑1 记载的
高位寄存器触发, 触发点是"该位置存在真 C 函数定义"本身。已写成经验 148 并指出坑1 的触发条件过窄。

决策 (用户拍板): 保留 `sub_801A3C4` 真 C, `sub_8020B54` 真身保留为注释 + 退回
`INCLUDE_ASM("asm/nonmatchings", sub_8020B54)`。净增 492 字节真实 C
(552 新 - 60 退), 但函数计数 676 持平。

**后续可选**: 拆分 C 文件可解 —— `sub_801A3C4` 之后有 `INCLUDE_ASM(sub_801A6F4)` 作锚点,
拆成两个 .c 各自独立 local_alloc。需改 `linker.ld` 的 `src/code_801A3C4.o(.text);` 为两个 .o
顺序拼接, 并同步 `functions.tsv` module 列。收益 +1 函数, 成本 = 触碰共享文件 + 数百行 TSV module 列
+ 与其他 agent 并发冲突风险。

### 验证工具踩坑

单 C 文件 实验必须先 `rm build/src/code_801A3C4.{o,s,i}` 再 `make build/src/code_801A3C4.o`,
否则 make 报 "无需做任何事" 并静默用过期 `.o`, 得出错误的 CLEAN 结论 (本次连踩 3 次)。
Makefile 只有 `%.o: %.c` 规则且没有 asm 依赖边, `.o` 存在时不会重编。
用绝对路径 `make build/src/code_801A3C4.o` 无规则匹配 (C_OBJS 是相对路径), 必须用相对路径。

## 2026-09-05 sub_80525E8 二次攻深 (agent claude-80525E8, 仍挂起 — 机制已推到证明级)

**新增硬进展**: 指令序已 100% 复现 (v01 形态 = 现 permuter base.c, 与目标仅差 case2 尾 5 条指令的
寄存器号: E6C r2→r1 / tbl r1→r2 / base r1→r3, 12 字节); 前任"指令序差异"结论修正 —— 问题纯化为
**case2 块内的寄存器 home 分配顺序**。

**local-alloc 机制新认知 (读源码 + 补丁实证, 见 EXPERIENCE 118)**:
- `block_alloc` 双轮: 先 sugg 轮 (有 `qty_phys_num_sugg/copy_sugg` 的 qty 抢先 `find_free_reg`,
  限定只用被建议的硬寄存器), 再 pri 轮 (QTY_CMP_PRI 降序, 同分按 qty 号)。
- sugg 唯一来源 = `combine_regs`, 且需要**指令 RTL 里出现硬寄存器** (纯 move: `set 伪←硬寄存器` 
  记 copy_sugg)。combine 追踪补丁实测: 全函数仅 3 次 combine, 全在 prologue 零扩展
  (r0→songId临时, r1→entry临时, r2→mode临时, 全 msc=1); **case2 块内 RTL 无任何硬寄存器**,
  任何 C 写法都造不出 (无 asm/无调用/无 volatile-IO), sugg 路对 E6C 封死。
- pri 公式确认 (2×缩放窗口): pri = floor_log2(refs)*refs*10000/(death-birth)。
  case2 块候选表: pair1/2/3 (refs4/life6→13333, 全落 r0) > E69a/0x400/tbl/base
  (refs2/life2→10000) > E70a (refs3/life8→3750) > E6Ca (refs2/life14→1428)。
- **目标分配要求**: E6C 必须在 tbl/base 之前拿 r1 ⇒ 需 pri(E6C) ≥ 10000 ⇒ **refs ≥ 7** (fl2(7)*7=14
  → pri 恰=10000 成平手, qty 号 E6C(4) < tbl(6) < base(8) 靠前先发) — 而 E6C 地址伪寄存器
  只有 set+store 2 处引用, C 层造不出 7; 指针拷贝链 (p=&E6C;q=p;r=q;*r=v) 借 tie 合并可堆 refs,
  但 2 拷贝=6→8571 (不够), 3 拷贝=8→17143 (越过 13333 会抢 r0), **无整数解命中 (10000,13333)**。
- **base→r3 是第二堵墙**: 要求 r2 在 base 窗口 (28,30) 被占, 而 r2 占用者只有 E70a(死于16)/tbl(死于24),
  E6C 在 r1, pairs 在 r0 — 无解。除非 tbl 的死延到 base 之后 (指令序固定, 不可能)。

**实验清单 (全部未命中)**: 18 个 C 变体 (指针/字面地址/store后置/加法交换/命名临时/entryTbl 形态/
链式/双指针/u8指针/earlyaddr 等, .scratch/claude-80525E8/v/), -g 变体 (分配不变), -O1 对照
(0x400 形状还变差), permuter 一轮 12500+ 迭代 (新低 290, output-290-1), agbcc combine 追踪补丁
(工具链已复原, 追踪二进制未保留)。

**同类函数图谱**: sub_805008C ✅匹配 = 同款 `[ldr tbl][ldr E6C][读E69+链][str r0,[r2]]` leftover 形态
(E6C→r2, 因其 E6C-load 距 store 仅 7 且 entryTbl 先占 r1); sub_80512C4 ⏸ (同款 gap=5, E6C→r3);
sub_80526A0 ⏸ (姊妹, 目标要 E6C→r0 最先拿, 同一墙); MenuUi_HideAll ✅ 展示了 refs=4 指针
(p 增量复用) 抢 r1 的合法路径 — 本函数 E6C 只有一次 store, 无法复制。

**下一步候选** (按性价比): ① 若将来给 agbcc 打 global.c/局部分配全转储, 可对**目标**反推其 qty 表
验证 sugg{r1,r2,r3} 三连假设 (本轮推断: 三池载均带 sugg 可完美解释目标分配序); ② 研究 sub_80512C4
(结构更小); ③ 接受墙, 保持 INCLUDE_ASM。

## 2026-09-05 zcode-ll: code_8010F10 批量匹配 (≤200行目标)

### ✅ sub_8017120 (196B, 一次通过)
SIO 会话轮询。要点:
1. **gSioSession.field_48 四个 u8 合并为 u32** (field_48..4B 无单独引用, 检索确认后合并) —— 否则
   `(u8*)&gSioSession + 0x48` 会让 GCC 把偏移折进池条目 (ldr [r4,#0x38] 之类错位寻址), 合并后才是
   基址 r4 + 位移 [r4,#0x48]。
2. **nibble 比较必须写显式移位** `(status << 28) >> 28 != ((status << 20) >> 28)` —— 写
   `(status & 0xF) != ((status >> 8) & 0xF)` 会产出 movs #0xF + ands (慢一拍且寄存器序不同);
   显式移位让 combine 走位域提取路径, 且顺带把 status 的 home 推到 r2。
3. 原型 void→u32 (sub_8016D24/sub_8017120): 本库调用方忽略返回值时改返回类型字节不变 (已 fncheck
   sub_80175C0 无回归)。
4. 第二个 store 写 `status & 0x1000` (不是 0!): GCC CSE 复用第一次 ands 的 r3, 此时值在路径上已知为
   0 但 GCC 不折叠 —— 恰好命中目标 `strh r3`。

### ✅ sub_8019F78 (228B, 一次通过)
32 宽 u16 缓冲区水平滚动 (8 参, r1/r3 未用)。
1. **y 循环上界必须写 `height + top`** —— 写 `top + height` 产出 `adds r1, r2, r4`, 目标是
   `adds r1, r4, r2` (height 在前); fold 对 VAR+VAR 不重排, 源码序即编码序。
2. **下标必须括号分组** `(y << 5) + (col + shift)` —— 不分组时 GCC 把 col+shift 提升出循环体到
   sp 重读之前 (LICM), 位置错 2 条。正向/负向两处都要分组。
3. shift 的 sp 槽重读位置在 x-guard 之后、内层循环 preheader —— 括号分组自然命中。

### ✅ sub_801768C (288B)
浮点插值 switch (mode): case0=arg1; case1=arg1*(arg3/arg2); case2=arg1*(2.0-arg3/arg2);
case3=arg1*((-10*arg3/arg2+20)/10); 尾=arg0+result*arg3/arg2。
1. **原型实为全 s16 + s16 返回** (code_0.h 原为 u16(s16,s16,u8,u8,u8)) —— asm 的 asrs 符号扩展证明。
   已改原型; 3 个已匹配调用者字节不变 (本库调用方不做小类型扩展, -fprologue-bugfix 下 callee 全责)。
2. **case 赋值不能写 (u16) 强转** —— float→u16 会走 __fixunssfsi, 目标是 __fixsfsi (s16 result 直接赋值)。
3. **case1 里必须保留死赋值 `new_var = 2.0f - (...)`** (permuter 引入) —— 删掉后 float 临时 home 变,
   case2 的 subsf 寄存器序崩。看似死代码实为分配承重。
4. **尾部乘法必须 `result * arg3`** —— 操作数顺序决定扩展序列 (result 的 lsls+asrs 先于 arg3 的 asrs)。

### ✅ sub_8018928 (304B, 一次通过)
gBattleUiFlags → REG_DISPCNT 的 BG 位转移。自然直写零迭代 —— 8 个重复 if 块, 位 0x01-0x08 置
0x100<<n、位 0x10-0x80 清对应位。REG_DISPCNT 地址由 movs+lsls 合成, 常量 0xFFFE 等走池, 无需干预。

### ⏸ sub_8017FA4 (permuter 35)
91 行。全函数仅 0x261 块两条 load 的寄存器 home 互换 (value/const 谁拿 r0)。
穷举 ~25 变体 + permuter 17k 迭代: 具名 temp 变体 (value→r0/const→r1) 的 ands dest 落 value home,
匿名访问变体 homes 反转但 dest 落 const home —— 目标是两者的交叉 (value r0 + dest=const home),
QTY_CMP_PRI 模型推不出整数解。候选 permuter/sub_8017FA4/base.c (output-35-1)。

### ⏸ sub_8013B0C (permuter 485)
127 行。仪表绘制, 语义全解 (见 TSV)。攻坚点: ①目标池有 4 个独立 0xB001 条目 (每 else 臂各自
materialize, 无 CSE) ②0x261 块 0x204 临时寄存器复用链 ③`base + 0xD000 + flag` 不可重结合 →
需 shadow 变量 (0xD000 存变量) 阻止 GCC 合并常数对。候选 output-485-1。

### ⏸ sub_80191CC (permuter 5100)
168 行。32 宽图块矩形边框 (语义全解见 TSV)。剩余 = 变量声明序/spill 模式: 目标 param/y0 spill 到
sp、x0→r8/x1→sb/y1→sl、5 个图块值占 sp[8..0x18] (槽序=声明序 corner,top,bottom,left,inner);
我的候选 x0 落 ip、y1 被 spill。下一步: 按目标 spill 集合反推声明序。

### ⏸ sub_8018A58 (53 diff)
184 行。背景加载, 语义全解 (见 TSV)。关键发现: **bl sub_8018BF8 之后直接使用 r6/r7** —— 是
未初始化局部变量 (掩码链覆盖全部 16 位, 垃圾值无影响), 按未初始化局部写 C 即可。剩余 = 池条目序 +
掩码链寄存器分配微调。

## 2026-09-05 zcode-ll: 接管四个被占用函数 (用户授权)

### ⏸ sub_801A2AC (47B → 13B, 接管自 sen1)
30 行 BLDCNT/BLDALPHA/BLDY 设置。突破:
1. **arg0 实为 int 非 u16** —— u16 会引入入口截断 (lsls/lsrs #16), 目标直接 `lsls r3, r0, #16` (无截断)。
2. **switch 值必须 (u16) 提升后才比较** —— `switch ((u16)(((u32)v >> 22) & 2))`: u32 让 >>22 是逻辑移位,
   (u16) 提升为 int 让 `cmp #3; bgt` 是有符号比较。
3. **v 的赋值链** `v = arg0 << 16` (具名变量独立寄存器, 可被调度提升到首条; 就地 clobber 会让 store
   侧多做一次 u16 提取)。
剩余 13B = v-shift 的调度位置 (arg1-ext 之前 vs 之后) + 链 home 是否整体用 r3。fndiff 0 diff。

### ✅ sub_8019AD0 (200B, 一次通过, 接管自 gpnux)
闪光效果设置。要点:
1. **三重掩码必须三条独立语句** `v = flags & 0xFFF0; v &= 0xFF0F; v &= 0xF0FF;` —— 合在一个表达式
   会被常数折叠成 0xF000 (movs+lsls 合成), 目标是三个独立池常量 ands。
2. **两段 dispatch 都是 switch**: `switch (flags & 0xF0) {case 0x10: break; case 0x20: BLDY=0x18;}` ——
   空 case 0x10 与 default 合流产生 `beq end; cmp; bne end` 形状; if/else-if 形状不同。
   第二段 `switch (flags & 0xF00) {case 0x100: BLDCNT=0xBF; case 0x200: =0xFF;}` 同理。
3. 0x04000048/0x40/0x44 三个 raw 写的地址距关系 (-8/+4) 由 GCC 自动合成。
4. 原型改为 void(u8, u16) (空原型与带默认提升参数的定义在 GCC2 冲突)。

### ⏸ sub_80199E0 (78B, 接管自 agent1)
淡出步进。语义全解 (见 TSV)。要点:
1. **空 case 0** 使 dispatch 出现 `cmp #1; ble end` 下落 narrowing。
2. **bits 指针 (0x030004D7) 必须在 case 1 内赋值** —— 函数顶赋值不可跨分支移动 (执行次数变化);
   for 逗号初始化 `for (i = 0, bits = ...; ...)` 可控 [i=0] 与 [bits] 的先后。
3. 剩余 78B = `movs r4, #0xFF` 的调度位置: 目标在 case2 入口 (0x200 臂的 store 值提前材料化),
   我方沉到 0x200 臂内。fndiff 16 (归一化匹配)。
4. 新符号 gUnk_03000390 (u16[], 调色板式查值表)。

### ⏸ sub_8018EA8 (15 diff / permuter 2860, 接管自 opencode)
3 位数图块显示。语义全解 (见 TSV): clamp 999 → 三位数分解 (udiv100/divsi10) → 前导零消隐
(switch(i) 内 case0/case1, 0x40=空白图块, 空白条件 d1==0&&d0>0xA) → 图块写
(*tile = (*tile & ~0x3FF) | ((0x280+digit) & 0x3FF); 高字节链式掩码 (~4/~8/0xF) | 调色板)。
攻坚点: ①高字节掩码链的 C 形态 (三段 ands 未折叠说明是分离链) ②数字写序 ③arg2*32 的 ip 使用。
新符号 gUnk_020352C0 (u16[], 图块映射)。

## 2026-09-05 sub_8010170 / sub_8010300 / sub_80104F8 三连攻 (agent claude-3fn, 全部挂起 — 语义全解, 字节差在分配/池层)

**共同背景**: 三函数均在 code_8005020 (菜单/道具 TU), 有用户草稿或语义注释; 全部符号已存在语义名
(gPartyStats/gPartyMemberIDs/gMenuCursorGrp/Sel/Stack/gScreenIdleIconIds/Cursor/gSpawnTileX/Y/
gSpawnFacingDir/gMapNpcSetId/gMoveCmdSetId/gWarpAnimState), 新登记 linker.ld 4 符号:
gUnk_030001C4/C5/C6 (0x1C4-6, u8)、gUnk_03002C44 (0x2C44, u8)、gUnk_080981EE (ROM 6B/项出生参数表)。

### sub_8010170 (装备更换) — 差 6 字节
- **permuter 发现关键技巧**: 在 `if (item != 0)` 前插入死赋值 `oldEquip = 0;` 并写成
  `if (item != oldEquip)` → item-ext 伪寄存器 refs 4→...、copy pseudo refs 2→4 (set+cmp+arg+store),
  pri = 8*10000/46 = 1739 越过 slot 的 968 → copy→r4、slot→r7 与目标一致, score 2840→60。
  (全局分配 pri = fl2(refs)*refs/L*10000, 见 EXPERIENCE 117/150; 死赋值改变 refs/L 归属是 C 层可用的杠杆。)
- 剩余 6 字节 = 尾部 `oldEquip = *equipSlot` 的寄存器 (mine r2 / 目标 r1) — global-alloc 边际
  (read pri≈5000 与 charaInfo/stack 平手区), 25+ 变体 + permuter 4 万迭代未破。
- **教训**: 草稿的 `oldEquip = item;` 死拷贝不是垃圾代码, 它是制造 r4 拷贝的源结构;
  但 `*equipSlot = item` 与 `= oldEquip` 语义等价而分配不同, 两种都要试。

### sub_8010300 (道具使用入口) — 80 字节
- 语义: 0→msg27; 0x3E→旗帜 0x03002C44&0x80 判定 (else 结构: 非 0x3E 才走 MP 消耗段, 0x3E+旗帜直跳
  itemTable); charaId/memberId 双变量 (memberId 喂 ItemGetUsePower, 调整后 charaId 查 gPartyStats);
  power>mp→0 写 gUnk_030001C4; 0→msg1d; 表 gUnk_08093418[(itemId-1)*5] 取 [1]&0xf/[3]; 0x26→
  WarpTable_Check; 末尾统计 hp<max_hp 人数 (i:u16, count:u8)。
- 已修: 0x3E 的 else 结构、memberId/charaId 拆分 (memberId != 0 判定+双写)。
- 剩余: MP 检查的 `movs r1,#0` 被 GCC2 提前到 ldrh 之前 (mine) vs 目标在 cmp/bls 之后; 三元/if-else/
  if-倒置同形 (GCC2 统一 if-convert); + 池级联与循环寄存器 (bls/bha 方向、r4/r5)。

### sub_80104F8 (传送/出生参数装载) — 182 字节
- 语义: kind = gScreenIdleIconIds[gScreenIdleIconCursor - 0xb + gMenuCursorSel]; 0→msg27;
  8→EventFlags_Test(0x10D)==0→msg1a; 0x18→EventFlags_Test(0xFF)!=0→msg1a; 否则线性搜索
  gUnk_080981EE 6B/项表 (首字节==kind), 把 5 字节拆入 gMapNpcSetId/gSpawnTileX/gSpawnTileY/
  gSpawnFacingDir/gMoveCmdSetId(两字节拼 u16: tbl[i]+(tbl[i+1]<<8)), gWarpAnimState=1,
  gUnk_03004D4C=0x34, SwitchFlags_ClearRange(1), return 1。
- 已修: **5 个 dst 指针变量预载** (p1..p6 在搜索循环前初始化 — 目标把 7 个基址全部预进寄存器
  r5/r3/r7/ip/r8/sb/sl); i=0 与指针 init 同组。
- 剩余: (a) 池倾倒级联 — 目标 4 个池 ([1F0,187,200,C8]/[10D,C8]/[1EE..260C]/[4D4C]), mine 3 个
  (dump3 发生在函数尾而非循环回边, 4D4C 混进 P3); (b) tbl/kind 的 r5/r6 互换; (c) u16 第二字节
  地址被 CSE 折叠成 ldrb r0,[r0,#1] (目标重算地址); (d) 首池 187/200 序。
- **教训**: `while (cond) i+=6` 与 `for(;cond;i+=6)` 同形; 池倾倒点 = 无条件分支处,
  表内常量的"首次引用顺序"决定各池内容 — 与 C 语句顺序强耦合。

**工具备忘**: bytecmp 的 bl/池字节差要先用真实函数地址 (ll.cfg) 填 sym 才有意义; fndiff 的
mine.o 未解析池渲染成 0x0000 是假差; GCC2 池倾倒级联分析 = 对比 .word 布局 (mine vs 目标 .s)。

## sub_8045A74 (战斗目标筛选) — 2026-09-05 match_bot
- 语义: 从 `list[0..count-1]` (u8 对象槽号) 里按 obj 槽 (stride 0xC8) 的字段阈值筛选,
  命中的索引压缩写回 list, 返回命中数。arg3 先 /=10; t1=(u16)(field_6e/10 * arg3),
  t2=(u16)(field_72/10 * arg3); mode 0: field_6c<t1, mode 1: field_70<t2, mode 2: 两者都满足。
  buf[5] 清零 0..4, 槽位复用 j (r6) 做零循环与主循环计数器。
- 关键点 (新增规律候选, 见 EXPERIENCE): **两段式 t 计算** — 写 `t1=(u16)(field_6e/10); t2=(u16)(field_72/10);
  t1=(u16)(t1*arg3); t2=(u16)(t2*arg3);` (分四条语句) 时 agbcc 才把两次 `bl __udivsi3` 与两次 muls 批量调度
  (arg3 只 `ldr r3,[sp,#0x14]` 一次), 寄存器分配才收敛 (j=r6, t1=r5, t2=r1, list[j]=r7)。
  单条内联表达式 `(u16)((u16)(f/10)*arg3)` 会触发立即乘法内联 (r7), 整函数寄存器位移一档 (score 1150→0)。
- 首次候选 (inline 表达式) 1150; 拆两条 t1/t2 语句后立即 0; fndiff score 0 + 双侧同时链接 bytecmp 296B 全等;
  fncheck OK (282B @0x08045a74, 3 bl 槽忽略)。同 C 文件 code_8044394.c, 原型 code_0.h 由 void 改全签名 (无调用者, 安全)。

## sub_8013B0C (文本行动画/仪表) — 2026-09-05 sense
- 语义: gUnk_03004DBC(帧计数)++后, 从 Text_TileAt(10,2) 起向左写 5 块 u16 图块。
  arg0==0xB0 时走动画帧路径: v=(c>>4)&3, 每块 ((v+i)&3+0xC)<<12+0x204 (i=0..4);
  否则走仪表路径: v=(c>>2)&3 (==3 则 v=1, flag=0x400), base=(u16)(0x204+v), shadow=0xD000,
  首块固定 0xB001, 后 4 块 arg0>0x8C/0x69/0x46/0x23 时 base+shadow+flag 否则 0xB001。
- 关键点: permuter 从 485→score5 的关键突破 = `v=(c>>4)&3` 入变量声明 (而非 store 表达式内联),
  使 agbcc 把 mask 3 放 r1 (而非 r0), 消除多余 `movs r0,#3`。迭代 1 用 `shadow=v` 强制 r1 复用
  (避免 `v+0` 被 CSE 折叠), 迭代 4 复用 shadow 保持寄存器家一致性。
- 路径 B 的寄存器家 (v=r1, 0x204=r3) 由 GCC2 自动分配 (与路径 A 的 v=r3, mask=r1 相反);
  原型 code_0.h 从 `void sub_8013B0C();` 修正为 `void sub_8013B0C(u16);` (唯一调用者 SaveUi_LoadScreen
  传 u16 实参, 无其他调用者, 安全)。
- fncheck OK (244B @0x08013B0C, 1 池重定位, 1 bl 槽忽略); 全量 make 成功但 SHA1 差 77B 属 code_801A3C4.o (非本次)。

## sub_801D214 (场景/对象 Tile DMA 上传) — 2026-09-05 agent
- 语义: 遍历 arg0 指向的对象表 5 项 (stride 0xC8), 若 (obj+0xBE)!=0xFF 且 (obj+0xB0)&0x20==0 则
  在 0x030035C0 缓冲按 idx*8 写两条 u32 "tile attr" 记录 (第一条 mask+字段位组合, 第二条含 i 与 r6
  累加), idx--。之后配置 DMA3 (SAD=0x02021040, DAD=0x06012880, CNT=0x840000A0), 忙等 CNT bit31 清,
  再调 sub_804C2FC(0x0861C744, 6, 1), 返回剩余 idx (即 count - 写入数)。
- 卡点 (score=5540, 未收敛):
  1) **prologue 3-extended-reg 保存**: 目标 `push{r4,r5,r6,r7,lr}; mov r7,sl; mov r6,sb; mov r5,r8; push{r5,r6,r7}; sub sp,#0x20`
     保存 r8/sb/sl 三个扩展寄存, 而 agbcc 只愿意保存 1-2 个。触发条件不明 — 尝试把 dst=0x0861C744 和
     mask=0xFFFFF3FF 提为局部变量并放到循环前, 但仍不足。可能目标原 C 有更多"跨循环存活"的局部量。
  2) **参数寄存器分配**: 目标 sb=r0 (arg0), r8=sign-ext(count), r7=0 (loop ctr); 我们的 GCC 分配 r8=arg0,
     r9=count (未 sign-ext), r7=0 (后置初始化)。`int idx = count;` (count 是 s8) 在 GCC2 里被 CSE 折叠,
     不 emit `lsls r1, r1, #0x18; lsrs r1, r1, #0x18` 的 sign-ext 序列。
  3) **tile1/tile2 累加器的初始值**: 目标在第一次迭代时 r5 = caller's r8, r6 = caller's sb — 都是
     未定义 (UB) 值。虽然 &0xFFFFFF00 / &0xFFFFFC00 只保留高位, 但仍需 GCC 生成"读未初始化寄存器"的
     指令序列, 标准 C 用局部变量难以精确复刻 (初始化 tile1=0 会改变字节)。
  4) **尾部 while(DMA_CNT & 0x80000000)** 分支模式: 目标用 `cmp r0, #0; bge exit; loop { ldr/and/cmp/bne }`,
     我们的 while 循环生成不同的 branch 布局。
- 尝试过的路径: (a) 手工按 m2c 草稿写 base.c 分数 ~6690; (b) 加 dst/mask 局部变量降到 7150;
  (c) 用 cnt_reg = (vu32*)0x040000D8 局部变量 + 分离 stmt 降到 5540 (最佳);
  (d) 合并多语句成单表达式反而升高到 9895 (说明目标 C 是分语句非折叠式)。
- 最佳候选: `permuter/sub_801D214/output-5540-1/source.c` (218 errors, score=5540)。
- 建议路径: 需要研究 prologue 3-extended-reg 保存的触发条件 (可能是某个 C 结构让 GCC 分配更多 live
  局部量), 以及如何让 GCC 生成"读未初始化寄存器"的模式 (可能需要 GCC 特定 opt-level 或特殊变量声明)。

## sub_801D468 (战斗对象列表装配) — 2026-09-05 opencode (✅匹配)
- 语义: 从对象池 (GetObjPool=0x02037028) 按段扫描"命中 flags 0xE3"的槽号:
  段0 = 槽 0..4 (`sub_80489E8(pool,buf,0,0xE3)`, mode0 写 5 字节), 段1 = 槽 5..11
  (`sub_80489E8(pool,buf,1,0xE3)`, mode1 写 7 字节); 两段各自 `sub_8048ACC(buf,n,7)` 快排后
  拼接进 `slots[12]`, 再整体快排。若 `gGstate324 & 0x1000` 置位则**跳过段0**(只拷段1)并
  `sub_80187D4(0x1000)` 清位 — 0x1000 是"前半段已展示过"的一次性标记。
  最后 `gUnk_03000638[i] = pool + slots[i]*0xC8` 填 12 项指针表, `03000669=0` (游标归零),
  `03000668=j` (条目数)。消费者: sub_801BE34 / sub_801C484 (场景对象命令执行分支状态机)。
- 关键点 1 — **`j = 0` 必须放在 `bl GetObjPool` 之前**: 目标 `sub sp,#0x1c` 之后第一条就是
  `movs r5,#0` (在第一次 bl 之前)。写在四个 bl 之后 → agbcc 把 `movs r5,#0` 排到 0x48
  (score 1260, 差 1 条位置); 提到函数头第一条 → 逐指令全对。这就是经验 27 (初始化顺序即指令顺序)
  的直接应用。
- 关键点 2 — **栈缓冲尺寸取最小语义值即可, agbcc 按 4 字节向上取整**: `slots1[5]/slots2[7]/slots[12]`
  与 `slots1[8]/slots2[8]/slots[12]` 生成**完全相同的字节** (都 → `sub sp,#0x1c`, 因为 5→8, 7→8)。
  验证: 5/6/12=23 和 5/8/12=25 也仍是 #0x1c; 只有 4/7/12=23 掉到 #0x18 (5→8 的取整边界)。
  所以优先写语义最小值 (mode0 写 5 项, mode1 写 7 项), 不要用凑数的 8。
- 关键点 3 — **数组元素类型 `u8 *` vs `u32` 都命中**: `gUnk_03000638[i] = pool + slots[i]*0xC8`
  声明为 `u8 *[12]` 或 `u32[12]` 生成相同字节 (指针宽度一致)。选 `u8 *[12]` 与消费方
  (sub_8045F10 取 u8*) 及 code_801A3C4.c 内 `gUnk_030006F8` 风格一致。
- 关键点 4 — **permuter 压不下去 15 分地板**: 函数含 3 个字面池 (0x03000638/0669/0668 都是真 extern
  符号, 候选 .o 是 R_ARM_ABS32 重定位, target.o 是硬码), 这是经验 29 的标准形态。
  fndiff 报 1200 = 3 池 × 400, **逐指令序列 100% 一致** (含池加载位置/分支极性/尾声);
  双侧同时链接 stubs 后 `.text` 276 字节**逐字节全等** (bl 槽编码也一致) → 判定匹配, 合入。
- 新增符号: `gUnk_03000638[12]` / `gUnk_03000668` / `gUnk_03000669` (iwram.h + linker.ld,
  按地址序插在 0x03000630 与 0x03000670 之间)。未改任何已有原型签名。
- 验证: `scripts/fncheck.py sub_801D468` OK (256B @0x0801d468, 3 池重定位, 8 bl 槽忽略);
  全量 `make` + `sha1sum -c ll.sha1` 通过; `scripts/audit.py` 685/685 status=1 字节核验通过。

## sub_801DB3C (code_801A3C4, 2026-09-05, pi)

场景对象"绘制/播报"入口: 按 arg2 (u16) 选表调 sub_801B81C, 尾设 `obj[0x66]=3; obj[0xB0]|=0x2000`。
两个分支都是 `sub_801B81C(obj+0x3C, obj[0xBF], obj[0xC0]-Δ, 常量, 0xE, 表[arg2]...4)`:
- arg2<=2: 12B 表 gUnk_0839B2B0 (0x0839B2B0, 新符号, 12 字节步长 = r4*3*4), Δ=0x10 或
  `(u8)sub_801EC3C(obj,1)>>1` (obj[0xBE]>0xA 时), arg6 加 `arg1<<5`, arg7=(u16)(0x543+field_8), arg3=0x2B4。
- arg2>2: 20B 表 gUnk_08393B28 (0x08393B28, 步长 r4*5*4), 不减 Δ, arg3=0x300。

关键点 1 — **`sub_801EC3C` 返回类型**: 目标 call 后是 `lsls r0,#0x18; lsrs r3,#0x19` =
  `(u8)func() >> 1` (先截断后移位), 说明原型是**宽返回 + (u8) 显式截断**; 若原型写 u8 则只有
  `lsrs #1` 一条, 不匹配。code_0.h 原来 `void sub_801EC3C();` 是错的 (它实际返回字节值:
  (x&0x1F)<<3 / 0x20 / 小常量), 已改 `u32 sub_801EC3C(u8 *, u8)`。无已匹配调用方, 改原型零风险。

关键点 2 — **显式指针局部 (经验 158)**: 第一版把 `gUnk_0839B2B0[arg2].field_*` 内联进 call 实参,
  GCC2 把表址计算推迟到 str sp 之间, 挤爆低号寄存器 → 动 r8/r9/sl (入口多 3 push, 中间 3 处 mov 往返,
  score 4740)。改成 `t1 = &gUnk_0839B2B0[arg2];` 独立语句放 call 前 → 表址计算排到分支最前、
  表指针稳占 r5、参数全落低号寄存器, 除 bl 槽外逐字节一致。

关键点 3 — **if/else 布局 (经验 159)**: `if (be > 0xA) {call} else {movs#0x10}` 生成镜像布局
  (bls 跳 movs 块); 目标直落块是 `movs r3,#0x10`, 分支目标是 call 块 → 原 C 必是
  `if (be <= 0xA) delta = 0x10; else delta = call;`。

关键点 4 — **尾块 OR**: `newval = 0x2000 | *(u16*)(obj+0xB0); *(u16*)(obj+0xB0) = newval;`
  (与 sub_80210C0 同款) 生成 `ldrh r1; movs r3,#0x80; lsls r3,r3,#6; adds r0,r3; orrs r0,r1; strh r0`。
  直接 `|=` 也同形, 但具名 newval 是已验证形态, 沿用。

结构/符号变更: 新增 linker.ld ROM 符号 `gUnk_0839B2B0 = 0x0839B2B0` (12B 表, 在 gUnk_0839B2A4 之后);
  Unk_08393B28 typedef+extern 从 0x08020974 前上移到 sub_801DB3C 前 (纯声明前移, 零语义变化);
  新增 12B struct `Unk_0839B2B0`。code_0.h: `sub_801DB3C` 补全原型 (u8*,u8,u16), `sub_801EC3C` 改 u32。
验证: permuter 硬地址版 score=0; 字节判定 bytecmp 除 3 个 bl 槽外逐字节全等; fncheck OK (228B);
  make + sha1sum 通过; audit 685/685 核验通过。

## sub_8047D28 (2026-09-05, flash150) — 类型分派 + 4 位 mask 判定, 首次尝试近满分, 卡 1 字节后破

**语义**: `u8 sub_8047D28(u8 *obj, u8 mask)`。按 obj[0xBE] 类型取 16 位 flags 对
(type≤0xA: sub_804E76C(obj,2,6/7)>=0 → flags=3/0xC; (u8)(type-0xC)<=0x64: obj[0x88]→u16 表
+0x16/+0x18; type>0x70: 同表 +0x2A/+0x2C), 然后对 mask 的 4 个位逐位判定: 位落在 flags1
(=0x16/0x2A, 先判定) → 返 1, 落在 flags2 (=0x18/0x2C, 后判定) → 返 2, 否则 0。

**攻坚记录**:
1. 首版 (局部变量 `type` + if/else-if 链) fndiff 仅 1 字节差: 目标 `adds r1,r0; cmp r1,#0xa`
   (首条比较用**副本**), 我的 `cmp r0,#0xa` (直接用 ldrb 结果)。fncheck FAIL @+0x1b。
   ⚠ 教训: fndiff 的 grep 计数会被跳转箭头 `~>` 污染, 真实差异要看分数/十六进制;
   另一次 "0 diff" 实为编译失败无输出 (ptr8 未声明), 差点误判。
2. 试错排除: 反转嵌套 (if type>0xA 在前) 差异扩大到 13 处; u32 type 使 +0x16..+0x27 全红;
   条件操作数序/声明顺序均无效 (GCC2 对可交换比较规范化)。
3. **正解: 三个条件全部内联读 `obj[0xBE]`, 不落局部变量**。内联后 GCC2 的 cse/副本传播把
   ldrb 结果复制进 r1 并让**首条比较也引用副本** (目标形态); 局部变量形式则让 type 驻留
   r0 (var 的 pseudo 与 ldrb 同 qty), 副本 r1 只服务末条比较。
4. 次要点: `i = 0` 语句必须在 `bit = 1` 之前 (目标 movs r2 先于 movs r3); 循环用
   `i = 0; bit = 1; for (; i <= 3; i++)` 形式 (入口测试被常量折叠, 与 do-while 同形)。
5. 合入副作用: code_0.h `void sub_8047D28();` 与定义 (u8 返回 + u8 参数) 冲突
   (K&R 整型提升) — 无其他 C 调用点, 改为 `u8 sub_8047D28(u8 *, u8);` 安全。

验证: fncheck OK (158B + 2B padding)。ROM 整体红为其他 agent 并发在途修改
(blame: code_80264C0.o/sound_data.o/code_804F0B8.o 位移), 非本函数问题。

## sub_8048F0C (2026-09-05, flash150) — 状态机 switch, case 集合形状决定分发链

**语义**: `void sub_8048F0C(void)`。按 gUnk_0300097B 状态机: case1 播放动画
(sub_804B96C 9 参调用, 栈传 0x1F/4/4/-1/2) + Sfx_Play(0x18,0,0) + state=2;
case2/3 是 gUnk_0300097C 计数器自增, 超 3 / 超 0xF 时重置, case2 超限还调
sub_804C4D8(gUnk_0300097D, 1, 0x10) 并 state=3; case3 重置时 state=0。

**关键发现**: switch 分发链形状由 **case 集合**决定 —
- 只写 case 1/2/3 → GCC2 平衡树 (root=2, cmp#2/beq; cmp#2/bgt; cmp#1…), 与目标不符;
- 补上**空体 `case 0: break;`** → case 集 {0,1,2,3}, case 0 标签=default 被剪枝,
  得到目标的线性链 (cmp#1/beq; cmp#1/ble-default; cmp#2/beq; cmp#3/beq; b-default)。
  原代码大概率显式写了 state 0 的空 case。

**其他**: case1 的 -1 栈参数由 `movs r1,#4 … subs r1,#5` 寄存器复用产生 (字面 -1 直接写即可,
GCC2 自己选 4-5); case2/3 的 `*state = 3/0` 两处 store 被 GCC2 尾合并为共享 `strb r0,[r5]`
(case2 先 movs r0,#3 再 b, case3 令 r0=0 直接落入) — 自然 C 即可复现。
新符号: linker.ld/iwram.h 登记 gUnk_0300097C = 0x0300097C (97B/97D 已有)。
验证: fndiff 75 条指令全同 (分数 2400 为池未重定位假高); fncheck OK 170B (6 池重定位)。

## sub_8049B70 (2026-09-05, flash150, 挂起) — 瓦片槽分配器, 结构全对, 差 12 条指令的分配彩票

**语义**: `u16 sub_8049B70(u8 *arg0)`。tile = *gUnk_0300096C; count = (u16)TileDma_GetCtx(&local);
tile ≤ 0xDF: v = tile & 0xFF (快路径); 否则 while (i < count && tile != ((u16*)local)[i]) i++,
v = i + 0xE0 (搜索路径, u16 截断+*2 被 combine 融合成 lsls#0x10/lsrs#0xf);
两路径都写 *(u16*)arg0 = 0xFFFFB000 + v*2 与 *(u16*)(arg0+0x40) = 0xFFFFB001 + v*2;
尾部 gUnk_0300096C++ 后判 *gUnk_0300096C == 0xF00 返 1/0。

**卡点** (fndiff 1865, ~12 条指令差, 全在快路径的分配):
- 目标: `ldr r2,=0xFFFFB000; adds r0,r2,#0; adds r0,r1,r0; strh [r5]` — addr 驻 r0,
  **池→addr 有副本**, 且 0xFFFFB001 是**第二条池条目** (ldr r3);
- 我的: `ldr r0,=pool; adds r2,r1,r0` (3-reg 直达, 无副本), addr 驻 r2, 且 B001 被
  `adds r0,#1` 从 B000 复用 (池常量 CSE)。
- 直接表达式 (无 addr 变量) 则触发**常量折叠**: 0xFFFFB000 → movs #0xB0 + lsls #8
  (strh 截断使高 16 位可弃), 与目标的池条目形态不符 → addr 变量是必需的 (阻断折叠)。
- 已穷举: int/u16 addr、addr1+addr2 双变量、操作数序 (0xB000+v*2 vs v*2+0xB000)、
  u16* arg0 数组形式、count u16/u32 — 均无效。怀疑需 GCC2 的 expand target-hint
  行为差异 (副本+就地加 = var 的 home 作 expand target), 待 qtydump 定量。
- 新符号: iwram.h/linker.ld 登记 `u16 *gUnk_0300096C = 0x0300096C`。

## sub_804DABC (2026-09-05, flash150, 挂起) — 对象随机属性初始化, 结构 100% 对齐仅差 4B

**语义**: count = sub_80489E8(arg1, values[8], 0, 0x6F); RNG%101 决定 obj[0xBC] (钻石菱形
双分支 + 无条件 obj[0xBC]=1 尾随存储); kind = RNG&3 → obj[0xC2] (==1 归 0);
entry = gUnk_08393B28_entries[*(u16*)(*(u32*)(obj+0x88) + obj[0xC2]*2 + 8)];
switch (entry->field_10): case0 → obj[0xBD] = values[(u32)(u8)Rng % count], case1 → 0。

**已破的关键**:
1. **count = entry[2] 语句必须放在 if/else 之后** — GCC2 cse 会把该 load **预插入两个分支尾**
   (increment 路径 adds r3,r0,#0 副本; decrement 路径 ldrb r3 直插), join 处 v = count>>1
   直接用副本; 放前面则 load 留在 join, 分支尾副本消失。
2. v 必须是 **int + 循环内 (u8)(v>>1)**: count 的值经副本后零扩展溯源丢失, 截断得以保留
   (直接 entry[2]>>1 则 provenance 完整, 截断被折叠)。
3. 分支极性: 写 `<= 0x45 → 1` (目标 bhi → 0 路径)。
4. kind 局部变量让 `adds r2,r4,#0; adds r2,#0xc2` (地址计算) 落到 ands 之后。
5. 加法**左结合**决定形态: `*(u32*)(obj+0x88) + obj[0xC2]*2 + 8` (8 在最后) 才出
   `adds r1,#8; adds r1,r1,r0; ldrh [r1]` 链; 8 在中间会被重关联折叠进 ldrh 位移。

**残留** (4B): 掩码路径 `movs #0x11; negs` 之后目标 `ands r0,r5` (dest=常量寄存器),
我的 `adds r0,r1,#0; ands r2,r0` (多一条 copy, dest=flags-home); orrs 同理。
or 路径的常量 0x10 在我的编译里被 CSE 到测试的 movs (目标重新物化)。
穷举过: 操作数序/两步赋值 newvar/u32 cast/RMW/struct 成员 (经验 78 各形态)。
姐妹函数 sub_804D260 (已匹配) 的 idioms 全部适用。

## sub_804B3C0 (2026-09-05, flash150, 挂起) — 调色板往返滚动, 差掩码路径 ~18B

**语义**: 调色板槽 16B 表 case 1 动画: flags & 0x10 决定 ++/-- 方向, 到界翻转方向位;
v = entry[2]>>1 经 do-while (≤8 步, (u8)entry[0xF] 计数) 算步数; 尾调 sub_804B56C
(src + (s8)entry[1]*16, dest + slot*16, (u8)(entry[2]-entry[3]), entry+0xC)。

**已破**: flags/count 须 int (u8 折叠 ~0x10 → 0xEF 立即数); v 须 int 且循环内
(u8)(v>>1) (v=entry[2]>>1 的溯源经副本丢失才保得住截断); count=entry[2] 在 if/else
后 (同 DABC 的 cse 预插入)。

**残留**: 掩码路径目标 `movs #0x11; negs; ands r0, r5` (dest=常量寄存器, 无多余 copy),
我的恒多一条 `adds r0,r1,#0` 且 ands/orrs dest 绑到 flags home; or 路径的 0x10 常量
被 CSE 复用测试的 movs (目标重新物化)。permuter 420 分平台期 (盲改语句序无效)。

## sub_804AE2C (2026-09-06, franklin, ✅ 已匹配) — 战斗演出 OAM 预扫描 (min/max/HPos 聚合)

**语义**: (gUnk_03000ADE&1)==1 且 (gUnk_03000ADE&0xF0)==0x10 时, 每帧对 gOamBuffer
槽区间 [gUnk_03000ADA, gUnk_03000AD9] (从 (gUnk_030009D0+0x2D)/(+0x2E) 计算) 做预扫描:
- gUnk_03000ADB = min(槽VPos, 初值 0xA0); gUnk_03000ADC = max(表[Size+(Shape<<2)]*8+VPos)
- gUnk_030009D8[i] = 槽 HPos (CharNo 载出, sub_804AF60 再回写 OAM)
- gUnk_03000ADE|=2; gUnk_03000AD8=(+1)%5; 到 0 时 gUnk_03000ADD++;
- (ADC-ADD)<(ADB-0x1E) 时 gUnk_03000ADE &=~1; &=~2 (bit0+bit1 关闭)。

**关键发现 (全部字节级可复现, fncheck OK, ROM sha1 绿)**:

1. **OAM 缓冲必须写成强转常量而非 extern 符号** (经验 102 的逆方向!):
   `gOamBuffer` 是 extern 数组时 agbcc 把基址 0x030035C0 缓存进高位寄存器 (ip),
   D8/表地址下移 → 全排列错; 写 `#define OAM_BUF ((GameOamData*)(0x030035C0))`
   (const_int) 则基址在循环内逐迭代 `ldr r1,=0x030035C0` 重物化 = 目标形态。
   表 0x08393A24 与 gUnk_030009D8 保持 extern (symbol_ref) 才被 hoist 进 sl/ip。
   验证: 强转后只差 bl __modsi3 槽位 (fncheck 忽略), 4 池重定位全施加。

2. **字段访问走 bitfield**: HPos (9bit 低位) → `ldrh [r4,#2]; lsls#0x17; lsrs#0x17`;
   Shape/Size (2bit 高位) → `ldrb [r4,#1|3]; lsrs#6`。写 `&=0x1FF` 会出常量池 ands。
   => 必须用 GameOamData.fields.* (iwram.h 已有结构)。

3. **max 的 ADC 要最先读**: 条件整式内联 (不单独 `v = ...` 再 if) 才把 `ldrb [r6]`(ADC)
   排到 size 之前 (`gUnk_03000ADC < 表[...]*8 + VPos` 直接写进 if, RHS 用同式复制,
   cse 合成单 v)。`int v` 局部 / s16 / u16 全不匹配 (s16 会加 sign-ext 两条)。

4. **头部不要 p 局部**: `gUnk_03000AD9 = *(u8*)(gUnk_030009D0+0x2D);` 直接写
   才让 `ldr r4,=0x03000AD9` 先于 `ldr r0,=gUnk_030009D0` (目标地址物化顺序)。

5. **末尾 &= 拆两条**: `&=~1; &=~2` 出 `ldr=FFFE; ands; ldr=FFFD; ands` (两条池常量);
   `&=~3` 会折叠成单 `ldr=FFFC; ands` (目标不符)。

6. 索引表达式必须内联在表下标里 (纹理带 new_var 且 orrs 而非 adds, 目标要 adds)。

**教训**: 这种"每个指令形状都对、只差整组寄存器 home"的函数, 别急着在语句序上死磕;
先跑 permuter 探平台 (~1125), 然后按"每个常量的拼写形式 (extern vs 强转)"逐个试 —
拼写一变, 整个 global-alloc 的缓存/重物化决策跟着变 (经验 102 双向适用)。

## sub_801CE80 (code_801A3C4, 2026-09-05, gpnux, ✅ 已匹配)

场景对象行为分派: `switch(kind)` 跳表 7 分支, 各 case 从 `p=*(obj+0x88)` 取 u16 索引进 20B 表
gUnk_08393B28 (0x08393B28, 步长 r4*5*4), 末尾统一 `flag|=0x20` (obj[0xBE]==0x78) 后调
`sub_801B81C(obj+0xC, obj[0x37], obj[0x38], f2a, f35, entry[0..A], flag)`。

**攻克点** (逐条映射到目标字节):
1. **case 共享模式** — 目标把 7 分支收敛到 3 个尾块:
   - 0/2 → entry 计算 + flag=0x409 (块 A, case 2 下落)
   - 1/5 → entry 计算 + flag=2 + 写 obj+0xB4/0xB6 (块 B, case 5 下落, case 1 跳入)
   - 6 + 3/4 → flag=0x401 (case 6 先自算 entry 再落进共享 flag 块; case 3/4 不碰 r4!)
   **初判失误**: 曾以为 case 1 是 flag=0x409 (与 case 0/2 同族), 目标实为 `ldrh r1,[r2,#2]; b 块B`
   → case 1 也写 B4/B6。source case 顺序须写 0,6,3,4,1,2,5 才能复现目标的块布局。
2. **case 3/4 与 default 的 entry 未初始化是 ROM 真 UB** — 跳表直落 flag 块, 末尾仍 `ldr r3,[r4]`
   读调用方遗留的 r4。C 里不初始化 entry 恰好复现 (agbcc 把 entry 分到 r4 不预写)。
3. **case 5 索引计算防折叠** — 直接写 `*(u16*)(p+8+arg5*2)` 被 agbcc 折叠成 `ldrh [r0,#8]`;
   目标要 `adds r0,r2,#0; adds r0,#8; adds r0,r0,r1; ldrh r1,[r0]` (5 条)。拆成
   `u16 off=arg5*2; u8 *p8=p+8; idx=*(u16*)(p8+off);` — off 先算(p8 后算)顺序也对上。
4. **B4/B6 写序** — 目标是 `ldrh [r4,#c]; 算 dest1; strh; ldrh [r4,#e]; 算 dest2(从 ip 重取); strh`
   (fresh dest, 不用 adds #2)。单临时 v 或双临时先载都会让 agbcc 走 `adds rX,#2` 复用;
   正确写法是 **v1 载→store1→v2 载→store2 交错**, 两个独立临时夹在 store 之间。
5. **permuter 分数** — base.c 里 `__asm__(".set gUnk_08393B28,0x08393B28")` + `.set sub_801B81C,0x0801B81C`
   (sub_80392C0 同款) 把字面池假差异消掉 → base score = 0。
6. **bytecmp 4B 基线** — 除 bl 槽 (mine 走 veneer `00f0 07f8` vs target 占位 `fff7 feff`) 外全等;
   符合经验 156 基线。fncheck OK 272B @0x0801CE80 (11 池重定位, 1 bl 忽略)。
7. 原型 `void sub_801CE80();` → `(u8*, u8, u16, u8, u8)`, 4 个调用点均 5 参无截断。
   `Unk_08393B28` typedef+extern 从文件后部 (原 620 行) 上移到本函数前 (同文件, 下游 sub_8020974 等不受影响)。

**TSV 教训**: 改 functions.tsv 时用 `split('\t',5)` 得 6 段 (name+note 合并进末段), 我误把 `f[5]` 整体换成 note → name 列被吞, gen_asm 报 drift 才发现。正确: `split('\t',5)` 后末段是 `name+'\t'+note`, 改 note 要 `f[5] = name + '\t' + note`; 或直接 `split('\t')` 取全 7 列再 join。round-trip 本身 `split('\t',5)+join` 无损耗 (AGENTS.md §1 没错)。

## sub_801D984 (OAM 缓冲自绘, 2026-09-06, opencode, ✅ 已匹配)

**语义**: 战斗/场景对象把 `gUnk_03000670[i]` (Unk_8021064: u16+u8+u8) 的内容逐字段写入
`gOamBuffer[r6]` (GameOamData, 8B/条), r6 每轮递减, 返回递减后的槽号。入口 `gUnk_0300068C != 0`
时用 `gUnk_0300068E/8D` 经 `sub_801768C` 插值算一个 r7 加到 VPos。

**匹配路径 (3 次结构翻转)**:
1. 初版裸字节 `o[3] &= ~0x0E; ...` + u32 局部 → 结构对但 0xFFFFFE00/0xFFFFFC00 折叠成 16 位
    (`movs #0xfe; lsls #8`), 且 gOamBuffer 基址被提到 r8 而目标在池内逐轮 ldr。
2. 换成 GameOamData **fields 位域赋值** (经验 161) → 全对! 字节 0 全位域 = 平 store,
   字节 1 五字段合并 = `mov r0,ip; strb` (全字节确定折叠), 字节 3/5 位域 RMW = ~mask neg 链,
   HPos/CharNo 半字 RMW 掩码走池, 0x3F/0x40/0xFFFFFE00 自动提升到 sl/ip/r9。
3. 剩 1 条指令: VPos 的 `adds r0,r0,r7`(2-op) vs `adds r0,r7,r0`(3-op)。根因 = expand 对
   ":8 位域存储" 把 field_3 抽成 subreg → expand_binop 交换操作数。解法 `u32 t = field_3 + r7;
   o->fields.VPos = t;` (经验 162) — 和先算全宽, 截断留给存储。

**定论**: fndiff 分数受池重定位假高 (经验 29), bytecmp 差字节 = 4×bl = 8B (经验 156),
fncheck OK 284B @0x0801D984 (6 池重定位, 2 bl 忽略)。原型 `void sub_801D984();` → `u8 sub_801D984(u8);`
(唯一调用方 BattleTask_Run 未匹配, 无字节风险)。新全局登记: 用已有 gOamBuffer/gUnk_03000670/
gUnk_0300068C/D/E, 无需新增。

## sub_80526A0 (脚本 VM 启动/跳转, 2026-09-06, claude06, ✅ 已匹配, 接手 opencode 9月5日陈旧锁)

**突破点: 前任的 score 25 是假分**。output-25-1 候选的指令流经 fndiff 逐条比对与目标 100% 一致,
仅 4 个 IWRAM 池字 (03000E6C×2/E70/E72/ED8) 在候选侧是 0 —— extern 引用进 .o 是 reloc+0,
而 target.o 是字面量, 纯假差异 (经验 164)。bytecmp 补 6 个 `sym = 0xADDR;` 桩后 **OK 136B**。

**去掉 m2c no-op (本次任务)**: `arg0++,arg0--; arg1++,arg1--;` 删除后 bytecmp 仍 OK —— 纯属多余,
u8 参数的 prologue 截断 (lsls/lsrs #24) 自然生成。vB (同时去掉 new_var 提升) 差 9B, 提升语句本身
承重 (case2 E6C→r0 的 home 依赖它, 与 sub_80525E8 的"墙"同源; 本函数靠提升语句即解, 无需 refs 堆叠)。

**因子隔离 (bytecmp 单变量)**:

| 变体 | 改动 | 结果 |
|---|---|---|
| vH | E6C 用 iwram.h 的 `extern u32` + base 写字面量 0x02016200 | OK 136B |
| vI | ED8 用字面量强转视图 `((u16*)0x03000ED8)[i]` | 差 64B (132B, orrs 没了) |
| vJ | base 用符号 `(u32)gUnk_02016200` | 差 9B (case2 E6C home) |
| vG | tbl 用符号 `(u16*)gUnk_02016000` | 差 105B (池载入不下沉进 case2) |
| vL/vM | ED8 经局部指针 `p[i]` | 差 105B |
| vN1 | 数组指针解引用 `(*(u16(*)[8])0x03000ED8)[i]` | 差 64B |

**`= -1` 的 orr 展开** (经验 165): 只有 `extern u16 数组[]` 元素下标能出 ldrh/orrs/strh;
iwram.h 里 ED8 只有 `extern u8` 标量 → 注册别名 `gScriptLocalSlots` (u16[] 视图,
linker.ld SECTIONS 外绝对赋值行式, fncheck 只认该行式)。

**permuter 真 0**: 套件 target.s 把 5 个 C 侧 extern 引用的池字面量改成符号引用重汇编
target.o → base score = 0 (人工可读 base.c 直接命中, 无需 permuter 变体)。

**合入收尾**: src/code_804F0B8.c 真 C 替换 INCLUDE_ASM; TSV status 0→1 + note;
match_fn.sh 首跑 FAIL (gScriptLocalSlots 未解析 —— 段内裸 `sym = .;` 行式不在 fncheck 解析集) →
linker.ld 改 SECTIONS 外 `gScriptLocalSlots = 0x03000ED8;` → **fncheck OK 136B (5 池重定位)**,
make + sha1 通过。原型未动 (code_0.h:1028 `void sub_80526A0(u8,u8)` 本就匹配)。

**顺手修复**: functions.tsv 954 行 sub_804FA04 的 ⏸ note 被此前某次批量回写清掉 (NF<7),
audit.py 对 note=None 无容错整表崩 —— 从 HEAD 恢复 note + audit.py 加 `(r["note"] or "")`
容错 (见 INCIDENTS 2026-09-06)。

## sub_804FA94 (脚本条件跳转·任一置位版, 2026-09-06, claude06, ✅ 已匹配, 接手 claude 9月4日陈旧锁)

**与 sub_80526A0 同一剧本: 前任 score 10 是假分**。output-10-1 的 fndiff 指令流与目标 100% 一致,
差异 = 2 个 EWRAM 池字 (02016000/02016200) 的 extern reloc-0 假分 (经验 164)。bytecmp 显示
"DIFF 8 bytes / 176": 8B = 两条 bl 的链接伪影 (经验 156/166), 多出的 32B = 函数桩地址给太远
ld 插的 veneer —— 装置伪影, mine.o 的 .text 实际 144B 与目标同长。

**人工形态一次过**: 套件 target.s 两个池字面量符号化 + base.c 仅把 `new_var` 改名 `dest` →
**base score = 0** (首跑命中, 无需 permuter 变体)。

**承重约束** (经验 166): ① 跳转路径须 **tbl 提升变量 + 字面量基址在前的和**: `tbl = (u16 *)0x02016000;`
提到 if 前, 路径内 `*ptr = 0x02016200 + tbl[data[2]];` —— 否则 GCC 把基址加法跨跳合并进公共尾部
(基址落 r1, 目标要 r2, 差 5B; 直写/符号形式/u32 暂存全试过都合并)。初版用 `long long dest` 截断
暂存同样命中 (64-bit 高位半占 r1 迫基址落 r2), 后按反馈改为此自然写法, 0 字节命中且无具名宽类型。
② t 存 r8 全程 / 0x1FF 存 sb —— r8+sb 双高寄存器函数, 合入后 make 全绿无泄漏。
③ `n > i` / `v > 0x1FF` 操作数序勿翻转。④ `res` 无初值 = 原始行为 (零循环路径读 r1 残值,
目标同样无初始化指令)。

**原型**: `void sub_804FA94();` → `u32 sub_804FA94(u32 *);` —— 全 ROM 无直接调用方
(经 gUnk_0862D434 函数指针表调用), 零字节风险 (同 sub_801D984 先例)。

**收尾**: TSV status 0→1, match_fn.sh 一次通过: **fncheck OK 144B (2 池重定位, 2 bl 忽略)**,
make + sha1 通过, audit 690/1059 (65%)。镜像姊妹 sub_804F974 (全置位版?) 仍挂起 ——
预期同根因, 套件 target.s 池符号化 + 找承重分配语句应可复制本路径。

## sub_801CBA4 (⏸ 2026-09-05 sense)

**状态**: 挂起。permuter base score=2225 (从 7635 经多轮优化压下), 但未达 0, 不能合入 src。

**签名** (从参数截断宽度推导): `void sub_801CBA4(u8 *obj, u8 kind, u16 f2a, u8 f35, u8 arg5)`。
prologue 首页寄存器: r6=obj, r8=kind(u8), sl=f2a(u16), sb(=r9)=f35(u8), r2=arg5(u8 临时),
r4=entry, r5=anim, r7=flag。

**优化历程**:
1. **case 重排** (0,10,1,2,3,4,6,7,8,9): 7635→2990 (−4645)。GCC 按 C 源码顺序发射 case 块,
   目标物理块序为 CC20(0) CC74(10) CCC0(1) CD14(2) CD18(3) CD30(4) CD5A(6) CD7C(7) CDC4(8) CDDC(9) CDEA(tail)。
2. **case 1 的 v1/v2 临时**: 2990→2450。function-scope `u16 v1; u16 v2;` 强制目标的重算地址 store 序
   (load→addr→store, load→addr→store), 与已匹配姊妹 sub_801CE80 同风格。
3. **tail 操作数序** (`val*4 + obj[0xBE]*16`): 2450→2450 (同分, 但 asm 更近)。
4. **case 0 eac 临时** (`v1 = sub_8020EAC(obj); if (v1 == 1)`): 2450→2450 (防 case0/case7 eac 合并)。
5. **ab=4 尾技巧**: 2450→2225。在 tail 中 `ab = 4;` 然后 `val * ab` 代替 `val * 4`。
   permuter 发现: 用局部变量代替常量改变寄存器分配, 使 tail 指令序更近目标。语义等价 (ab 在此处仅存 4)。

**卡点** (2225 分构成):
- **r5/r6 互换** (89 register diffs × 5 = 445 pts): 目标 obj=r6/anim=r5, mine obj=r5/anim=r6。
  纯分配器选择。9+ 次尝试失败 (声明序 anim↔entry、flag 置首、register 修饰、anim=0、额外长活局部、
  entry 改 u8*、register u8 *obj、ab 改 u16)。姊妹 sub_801CA08 用 ip(r12) 放 obj, sub_801CE80 用 ip,
  说明分配器选择随函数结构变化, 难以外部控制。
- **24 处结构差异** (~24×100 = 2400 pts):
  - case 0 `ab` 二次截断: 目标 `lsls r0,r1,#0x18; lsrs r0,r0,#0x18; cmp r0,#0` (ab 已 ldrb 过仍截断)。
    试 `(u8)ab==0`、`ab` 改 u16、`int ab` 等均不产生该截断。原因不明 (可能原码显式 `(u8)` cast)。
  - case 0 `idx = field_12` 共享: 目标 CD9E 为 case0(CC50→CD9E)/case7(CD96→CD9E) 共享块,
    mine 两处内联。GCC -O2 tail-merge 未合并 (可能因 case0 eac 检查嵌套在 bc 检查内)。
  - case 4 死截断: 目标 `bl sub_8020EAC; lsls r0,r0,#0x18; lsrs r0,r0,#0x18; b CD9A` (结果截断后丢弃)。
    10+ 种 C 变体 (u8/int 返回、显式 cast、赋值给临时、嵌套 if、`(void)` 强转) 均不产生。
    已确认本编译器始终消除死截断 (/tmp/t1-t6.c 验证), 疑编译器版本差异。
  - tail 指令序: 目标先 `ldr r2,=base` 再 `lsrs r0,#0x16`, mine 相反。试多种表达式序均未改善。
  - `.short 0x0000` 对齐填充与 literal pool 布局差异。

**下一步** (若再攻):
1. 尝试 `-g` 编译变体 (`-mthumb-interwork -Wparentheses -Werror -O2 -g -fprologue-bugfix`) 改变分配/保活。
2. 尝试将 case 0 的 eac 检查改为与 case 7 结构完全一致 (均顶层 `if (cond) if (eac==1) ... else ...`) 以触发共享。
3. 接受 r5/r6 差 (445 pts) 后, 专注消除 24 处结构差异中的可解部分 (tail 序、.short 填充)。

**permuter 套件**: permuter/sub_801CBA4/ (base.c score=2225, compile.sh/target.o/settings.toml 完整)。
最佳输出目录 output-2225-1 (与 base.c 同分, permuter 未能超越人工优化)。
注: 拆分后本函数归属 C 文件由 code_801A3C4 迁至 code_801A5EC (2026-09-06 zcode-ll2)。

## 2026-09-06 zcode-ll2: code_801A3C4.c 拆分 + 4 函数匹配 (≤200行批次)

**核心突破: 拆分 C 文件解除 TU 状态泄漏扰动 (经验 148 的解法落地)**
- `src/code_801A3C4.c` 拆为 `A=code_801A3C4.c{sub_801A3C4}` + `B=code_801A5EC.c{sub_801A5EC..sub_8020CC4}`。
  B 的编译器状态 = 2026-09-02 获胜态 (sub_801A3C4 转真 C 之前), 拆分本身零扰动 (audit 691/691 全绿)。
- sub_8020B54 ✅ 直接复用 09-02 真身 (do-while 屏障), fncheck OK 60B。
- sub_801DDB0 ✅ 落地 09-04 byte-exact 候选 (gUnk_0839B2D4 命名符号), 扰动消失, fncheck OK 148B。
- linker.ld 插入 `src/code_801A5EC.o(.text);` 于 code_801A3C4 之后; functions.tsv module 列经 tsv_init 重推导。
- **注意**: 若未来 B 文件再加真 C 函数, 需重验 sub_8020B54/sub_8020B90..CC4 的 tiebreak (尾部函数敏感)。

**✅ sub_801FA10 (168B, permuter score 0)**
- 语义: `*(u16*)(obj+0xB0)` 低 4 位替换为 arg1, switch(val&0xF) case1/2 调 sub_801B81C 传 ROM 表常量。
- 关键: ①`z=0` 局部变量跨 case 活 (r5), 字面 0 不行; ②`val = *(u16*)(...) & 0xFFF0` 与 `val |= arg1 & 0xF`
  拆写, z=0 夹在中间 (决定 movs r5,#0 的位置); ③code_0.h 的 K&R `()` 原型升全原型
  `void sub_801FA10(u8 *, u8);` —— u8 形参带默认提升, C89 禁止空参表声明后定义 u8 形参 (int 形参
  会改变 ands 操作数序), 两调用点 (FEBC/D50:175) 均传常量 → 零字节影响, fncheck 验证。

**✅ sub_801DD04 (172B)**
- 语义: 对象排序链表 (0x030006A0, 16B 节点 {key,prev,next,data}) 摘除 idx 节点 + 清对象字段 +
  按 field_BE 三分派 (≤0xA→CBA4, ≤0x70→CA08, 其余→CE80) + sub_801D12C(obj,0)。
- ①注册 `gUnk_030006A0` 符号 (iwram.h typedef + linker.ld 0x6A0) —— 字面常量基址的 +4 会被折进
  池 (0x030006A4), 符号基址+中间变量才保留运行时 `adds` (经验 73 的 RAM 版); ②摘链核心必须用
  **u32 字指针** `((u32 *)prev)[2] = *np;` —— 结构体字段存储与指针标量读取在 GCC2 别名集下判无冲突
  会省掉目标中的两次重载; ③`u32 prev = *pp;` 早读 (在 np 计算前) 定调度序。
- permuter 单函数编译无法复现 TU 态 (score≥960), 以 fncheck 为准 (经验 74)。

**⏸ sub_801DEDC (97行) / sub_801DF90 (95行) — 候选成型, 仅剩 TU 态调度 tie**
- 姊妹函数: 按 (s8)obj[0xBC] 选 0x08393B28 表项, 按 entry->field_10 二次分派 (与 DD04 同尾)。
- DF90: **standalone 可复现** (permuter/sub_801DF90/output-105-1, score 105 = 纯池重定位罚分);
  in-TU 仅剩 2 处调度 tie (count 区 movs r5,#7 位置; `adds r1, r0, r6` 操作数序)。
  关键技巧: case1 `off = arg0[0xC2] * 2; anim += 8; *(u16 *)(anim + off)` 语句拆分 (锁 +8 运行时加)。
- DEDC: 结构 100% 解 (off/idx 拆分 + entry 移出 switch 得到共享 ×20/截断), 剩 val home tie (r3 vs r0, 12B)。
  候选: permuter/sub_801DEDC/candidates/{v1_val_r3_tie.c, v2_valfirst.c}。
- **重启路径**: 任意其他函数落地改变 TU 状态后, 直接重试候选 (每变一次 tie 重洗)。

**事故**: TSV 是 7 列, 用 `split("\t",5)` 把 note 写进了 name 列 (AGENTS 示例歧义), 已修复并记 INCIDENTS。

## 2026-09-06 zcode-ll2 (续): EC3C 匹配 + DEDC/DF90 复活 + D378/20228 候选成型

**✅ sub_801EC3C (260B, 跳转表)** — 关键顺序:
1. arg1 分派必须是 **switch**(case 体表体外置), if/else-if 会内联首块;
2. **单一共享 `return result;`**(无早退) → result 保住寄存器 home (r1) → `u8 result = 0x20;`
   提升到函数顶 → `push {r4, lr}` 自然出现 (arg0 被挤出低寄存器);
3. **case 体按源顺序排放**: case 7 必须在 case 8 前 (跳转表体顺序 = 源序);
4. `result = x * 8` 的 u8 赋值合并 ×8 与截断 → `lsls r0,#0x1b; lsrs r1,r0,#0x18`;
5. 注册 gUnk_08393A3C/A40 符号。

**✅ sub_801DEDC (180B) / sub_801DF90 (176B) 复活** — 上一轮 tie 经状态变化+微调后全解:
- entry 计算放 case 内 + `(u16)` 强转 → cross-jump 合并尾部 (DED);
- `kindBE` 临时 + if 形式 + `i * 0xC8 + (u32)arg1` 整数算术 (避免指针规范化翻转操作数序) (DED/DF9);
- case1 `off = arg0[0xC2] * 2; anim += 8; *(u16 *)(anim + off)` 语句拆分锁 +8 运行时加 (DF9)。

**⏸ sub_801D378 (120行) / sub_8020228 (120行) — 结构 100% 解, 剩分配 tie**
- D378: `int t` 临时 + 位掩码链 (掩码 ~3/~0xC/~0x10/~0x20/~0xE/~0x1FF — GCC2 `~x` → `movs #(x+1); negs`
  实证! ~4 会编成 movs#5+negs, 目标是 movs#4+rsbs → 掩码常量必须逐位核对) + tile 三元 + `return --idx`。
  头部已对齐 (push/idx→r8/oam→r7); 剩 sl-vs-sb 高位寄存器选择 + idx 读取形状 tie。候选:
  permuter/sub_801D378/candidates/v1_t_temp.c。
- 20228: 表拷贝全解 (u16 读→u8 存 0xAA/0xA9、双读 field_2/field_4 是别名阻塞 CSE 的正确形状);
  剩 muls 区双拷贝 (目标 in-place r1)。候选: permuter/sub_8020228/candidates/v1_tbl_else.c。
  已升 code_0.h: `void sub_8020228(u8 *, u8 *, u8);` / `u8 sub_801D378(u8 *, u8);`。
- 重试路径: 落地其他函数改变 TU 状态后直接换入候选重编 (每次 ~40s)。

**本批新增匹配**: sub_801FA10 / sub_801DD04 / sub_801DEDC / sub_801DF90 / sub_801EC3C (+复用 8020B54/801DDB0)
进度 691 → 699 / 1059 (66.0%)。剩余 ≤200 行: DC20/B570/ED40/E1D8/200E8/20648/20228⏸/D378⏸/F76C⏸/D214⏸/A6F4⏸。

## 2026-09-06 zcode-ll2 (续2): DC20 匹配 + ED40/D378/20228 深度攻坚 + 一次自伤事故

**✅ sub_801DC20 (228B, 一次成型+一处修正)** — 对象入排序链表 (sub_801DD04 的逆操作):
buf[8] 栈缓冲 + sub_80489E8 收集同 kind 对象 + gUnk_030006A0[buf[i]].data=arg0 +
ListNode_InitKey/InsertSorted + sub_8045F94(arg0,8) + 0xB2=0 + sub_804E7EC + ≤6 分支 + 0x030006F0++。
唯一修正: **比较操作数交换** (`arg0[0xBE] == pool[...]` — GCC2 先求值左操作数, 目标先算 &arg0[0xBE])。
已升 code_0.h: `void sub_801DC20(u8 *, u8);`。

**⏸ sub_801ED40 (144行)** — RGB 颜色组合((x&0xFFFFFF00)|0x1F 三通道逐字节)+0x03000765=arg0[0x35]+1+
三分支 sub_804B654 调用族(栈参 -1/2、(s8)(v+1)/3、8/3)+ (s8) 符号检查 + 共享 `|= 0x8000` 尾。
已注册 gUnk_03000765。剩: 我方 3 高位寄存器 vs 目标 0 (arg1 的 home: sl vs r6) — 分配 tie。
候选: permuter/sub_801ED40/candidates/v1_p35_removed.c (移除 p35 命名变量后)。

**⏸ sub_801D378 (120行)** — OAM 条目位域写入, 掩码实证修正 (经验 165):
目标实际掩码 = ~3/~0xC/~0x10/~0x20/0x3F|0x40 (oam[1]) 与 ~0xE/~0x10/~0x20/0x3F|0x40 (oam[3])。
三轮形状: t临时 → 掩码修正 → 无t临时(每语句重读 OAM, GCC2 cse 转发刚存值)。
现剩: attr 的 home (目标 sb 高位 vs 我方 r6 低位) — 高位寄存器数量 2 vs 1 的分配 tie。
候选: permuter/sub_801D378/candidates/v2_notemp.c。

**⏸ sub_8020228 (120行)** — 表初始化全解 (0x083987EC×0x2C 条目, u16读→u8存, 别名阻塞 CSE 的双读);
剩 muls 区双拷贝 (目标 `muls r1, r0, r1` in-place)。候选: candidates/v1_tbl_else.c。
已注册 gUnk_083987EC; 已升 code_0.h 原型。

**⚠ 事故**: 挂起 ED40 时 index 切片误删 sub_801EE6C(真C)+sub_801EEE4(INCLUDE_ASM) → ROM 布局后移 sha1 红。
从 /tmp/full_bak.c 备份恢复, 全量验证 700/700 全绿。已记 INCIDENTS (切片回退校验+即时全量验证教训)。

**进度 700/1059 (66.1%)**。本批累计 8 个新匹配: FA10/DD04/DEDC/DF90/EC3C/DC20 (+复用 8020B54/801DDB0)。
剩余 ≤200 行 11 个: B570/E1D8/200E8/20648 (未动) + D378/20228/ED40 (候选待重试) + F76C/D214/A6F4 (老挂起)。

## 2026-09-06 zcode-ll2 (续3): DC20 匹配 + 200E8 候选 + 又一次切片事故(已修复)

**✅ sub_801DC20 (228B)** — 见上节。

**⏸ sub_80200E8 (158行)** — 语义全解 (candidates/v1_z_var.c): spawn 拷贝
(0x6E/0x72/0x6C/0x70 u16、0x74-0x7C 字节和、0xA9/0xAA/0xAB、0x7E-0x84 清零、
0x8D-0x92、sub_8048B5C、0x99 区 8 项条件循环 v!=FF&&v!=26→v-1 else |=FF、
sub_8045BF4/5EB8/D12C、C3/C4=0x10)。剩: 目标 RHS 先载(疑似结构体视图)、
push r7 (arg2 home)。下一步 = 定义 src/obj 结构体视图重写。已升 code_0.h 原型。

**⚠ 事故 2 (重犯)**: 挂起 200E8 的切片又吞了 sub_8020228 的 INCLUDE_ASM (链接期暴露)。
已修复并强化 INCIDENTS 教训: 挂起 = sed 单行替换 INCLUDE_ASM 行, 禁止跨行区间切片。

**进度 700/1059 (66.1%)**。剩余 ≤200 行 11 个:
B570(143)/E1D8(151)/20648(176) 未动; D378(120)/20228(120)/ED40(144)/200E8(158) 候选待状态窗口;
F76C(104)/D214(161)/A6F4(195) 老挂起。

## 2026-09-06 zcode-ll2 (续4): E1D8 候选成型

**⏸ sub_801E1D8 (151行)** — 语义 100% 解 (candidates/v1.c): 对象倒计时处理 —
对 gUnk_03000730[i] 激活项: (s16) 比较obj[0x6C]-obj[0xB2] 剩余量, >0 减算(ldrh 无符号)否则清零入队
(gUnk_030006F8[gUnk_03000714++]=obj, >0xB 则 gUnk_03000718=obj, result++); 未完成项经
gUnk_03004F90[k] 条件调 sub_8020E90 + sub_8045F94(obj, 表字节); 返回 result!=0。
已升 code_0.h: `u8 sub_801E1D8(void);`。补上了此前缺失的 `sub_80445E0()` 调用。
剩: pool/i/result 的寄存器 home 分配 tie (与目标完全互补: pool r9↔r7, i/result r8↔sl 互换)。

**≤200 行账本 (10 轮迭代后)**: ✅8 匹配 (8020B54/DDB0/FA10/DD04/DEDC/DF90/EC3C/DC20) +
⏸5 候选 100% 成型 (D378/20228/ED40/200E8/E1D8) + ⏸3 老挂起 (F76C/D214/A6F4)。
进度 700/1059 (66.1%)。全量 make+sha1+audit 全绿。

## 2026-09-06 claude-8030D9C: sub_8030D9C 匹配 (701/1059)

**✅ sub_8030D9C (174行, code_80264C0)** — 语义: 对象 BGM/音效演出状态机, switch 分派
IWRAM `gUnk_03000889` (22 态, case 0..21 跳表, 4..20=default):
- case0: 复位 gUnk_03000825 计数, gUnk_03000867=0x10, gUnk_03000868=0, obj+0x24 |= 0x10,
  sub_801A2AC(0x710,0x10,0) + Sfx_Play(0x5A,0,0), 进态 1。
- case1 (淡入 ≤9 步): gUnk_03000868 = sub_801768C(0,0x10,0xA,计数,2), 每歩
  sub_801A2AC(0x710,gUnk_03000867,gUnk_03000868); 步满→清计数进态 2。
- case2 (淡出 ≤0x13 步): gUnk_03000867 = sub_801768C(0x10,-0x10,0x14,计数,2), 每歩
  sub_801A2AC(0x710,gUnk_03000867,0x10); 步满→清计数, obj+0x24 |= 0x200, 进态 3。
- case3: Sfx_StopTrack(0), 进终态 0x15。case 0x15: sub_801A2AC(0,0,0), obj[0xBE]=0xFF,
  obj+0x24 &= 0xFFEF, 返回 1 (完成)。
- 原型升级 code_0.h: `void sub_8030D9C();` → `u8 sub_8030D9C(u8 *);` (无静态调用点, 池表引用)。

**关键坑**: (1) case1/2 的 `bhi else` 在 `ldr r4,=0x030008xx` 之前, 走 else 时 r4/r2 仍是
函数入口的 obj/&state —— C 侧天然成立, 不要被"else 分支引用了 case 内才加载的 r4"误导读成
0x0300088B。(2) obj+0x24 的两处 `|=` (0x10/0x200) 目标都是"载荷 r1/常量 r0"形状, `|=`
与两种显式 or 写法全给镜像; 命名临时 f2a/f2b (函数顶声明, C89) 一处改两处中 →
经验 168。(3) 跳表 .4byte 标签引用两侧都是 reloc-0+addend, 本来就对; 仅 11 个字面池假分
(fndiff 400/个=4400, permuter 5/个=55) → 按经验 164 把套件 target.s 的 gUnk 池字面量
换符号引用后 permuter 真 0 (9676 迭代无更低)。

**字节定论**: 双侧同 abs.ld 部分链接后 472/472 逐字节一致; fncheck OK (404B, 23 池重定位
施加, 7 bl 槽忽略); 全量 make + SHA1 绿。

**同族提示**: sub_8030F30 与本函数逐指令全同 (仅标签/池址差), sub_80310C4 仅差末位掩码池,
sub_8030C08 少 case0x15 尾部 8 行 —— 皆可用本 base.c 套模板快速出候选 (注意各自套件也要做
池符号化)。

## 2026-09-06 claude-8030D9C: 同族 15 连匹配 (716/1059)

**✅ A型×5** sub_8030F30/80310C4/8031258/80313EC/8031580 — 与 sub_8030D9C **逐指令+逐池值全同**
(仅标签差); 跳表 case 映射规范化比对确认一致。直接复用 sub_8030D9C 的真 C (换名)。

**✅ B型×9** sub_8031714/80318A8/8031A3C/8031BD0/8031D64/8031EF8/803208C/8032220/80323B4 —
唯一差异 = case0x15 内 `flags &= 0xFFEF` 块在 `obj[0xBE] = 0xFF` **之前** (BASE 是之后)。
C 侧仅交换两语句顺序。

**✅ C型×1** sub_8030C08 (173行) — case3 内联全部收尾: `Sfx_StopTrack(0); sub_801A2AC(0,0,0);
flags &= 0xFFEF; gUnk_03000889 = 0x15;` 直落终态; case0x15 只剩 `obj[0xBE] = 0xFF; result = 1`。

**方法论 (批量同构族)**: ① 规范化 diff (剥标签/@注释) 分型; ② 跳表单独验证 (body diff 对
`.4byte _xxx @ case N` 的目标标签是瞎的, 必须 base 相对偏移比对); ③ 每函数独立套件 + target.s
池符号化 (经验164) 后 permuter 验 0 (A/B/C 三型 base.c 一次成型, 15/15 直接 0, 无需迭代);
④ 15 个真 C 由 sub_8030D9C src 文本程序化派生 (保证文本级一致)。
**坑**: 批量合入别忘 code_0.h 的 K&R 原型 → `u8 fn(u8 *)` 全链升级 (漏 15 个, make 红;
另 `make | tail && sha1sum` 管道吞退出码, 旧 ROM 误报绿 —— 以 make-exit=${PIPESTATUS[0]} 为准)。

**字节定论**: 15×fncheck OK (各 404B, 23 池重定位施加, 7 bl 槽忽略); 全量 make exit=0 +
SHA1 绿; 进度 716/1059 (67.6%)。code_80264C0 剩 45 个未匹配。
## 2026-09-06 claude-8030D9C: sub_80345AC (717/1059)

**✅ sub_80345AC (161行, code_80264C0)** — sub_8034440 (0x08034440, 2026-09-04 匹配) 的同族克隆:
switch(gUnk_03000820) 状态机 case0/1/2/5/6/8/9, 全部指令/池/跳表逐条相同, **唯一差异 = case8 的
`sub_801EEE4(arg, GetObjPool(), 0, 0xB, 0x1E)`**(8034440 是 `(..., 0, 0x32)`)。直接复制其真 C
改两参 + 改名。语义: 事件演出状态机 (case0 备份 obj[0x35]/obj+0x2A → case1 演出请求 0x386/0x1B4
→ case2 移动完成(bit11)后 Sfx_Play(0x3E) → case5 bit12 到位后 sub_804C3A4+清bit12+sub_801CBA4
复位 → case6/8 等待窗口 (0xB 槽, 0x1E 帧超时?) → case9 sub_8045B90 收尾返回 2)。
原型: code_0.h `void` → `u32 sub_80345AC();` (照 sub_8034440 K&R 先例, 仅返回类型)。
permuter 0 (10 gUnk 池字符号化) + fndiff 0; fncheck OK 364B (与 8034440 完全对应);
make exit=0 + SHA1 绿。

## 2026-09-06 claude-8030D9C: sub_8034718 (718/1059)

**✅ sub_8034718 (170行, code_80264C0)** — sub_8034440 同族但**双参**: `u32 sub_8034718(u8 *arg,
u8 *arg1)` (r1 入口存 r2, 仅 case0x12 使用)。switch(gUnk_03000820) 19 态, case0/1/2/5/6/9/0x12
有体。与 8034440 差异: ① case0 无 sub_8020DE4, 改调 `sub_8048B30(0, 0x1E, 0x3AD)`; ② case6
跳 0x12 (非 8); ③ case0x12 = `if (sub_80476DC(arg, arg1) == 1) 进 9` (8034440 的 case8 是
sub_801EEE4); ④ case9 同款收尾。case 源顺序必须 0x12 在 9 前 (目标布局)。

**参数定型法**: sub_80476DC 未匹配, 但其体内 r6(=第二参)全程当对象指针用
([r6+0xBF]/[r6+0x54]/[r6+0x2A]/sub_801CBA4(r6,...)) → arg1 是 u8*; 三个调用点
(8034718/0x8037006/0x8037DA2 同族) 全是转发, 无独立佐证。

**头原型**: `void sub_80476DC()` → `u8` (返回值 lsls/lsrs 截断后 cmp #1 被消费;
同 sub_801EEE4 void→u8 先例, 另两个调用点还在 INCLUDE_ASM 里不受影响)。
permuter 0 (10 gUnk 池字符号化) + fndiff 0; fncheck OK 400B (20 池重定位); make exit=0 + SHA1 绿。

## 2026-09-06 glm5-8037E14: sub_8037E14 (719/1059)

**✅ sub_8037E14 (198行, code_80264C0)** — sub_8034440 同族状态机: `u32 sub_8037E14(u8 *obj)`,
switch(gUnk_03000820) 24 态, case 0/9/18/19/20/21/22/23 有体。流程: case0 sub_8020DE4 → 0x12 →
case18 `sub_8020974(obj+0xC, 0x3B9, 0x1B4, 0xC, 2)` → 0x13 → case19 !(keys&0x800) 时
Sfx_Play(0x3E,1,0) → 0x14 → case20 (keys&0x1000) 时 `sub_804C3A4(obj[0x35], sub_801B954(obj+0xC))`
+ `keys &= 0xEFFF` + `sub_8020974(obj+0xC, 0x3BA, 0x1B4, 0xC, 2)` → 0x15 → case21 → 0x16 →
case22 `sub_801EEE4(obj, GetObjPool(), 0, 0, 0x3C)==1` 时 sub_801CBA4 → 0x17 → case23 → 9 →
case9 sub_8045B90 收尾返回 2。

**头原型**: code_0.h `void sub_8037E14()` → `u32` (返回 r7, 照 sub_8034440 K&R 先例;
src 无调用点, 零风险)。

**踩坑复训**: 首次合入把 case 自作主张排成升序 (0,9,18..23) → fncheck FAIL 462B
(经验 124: case 块源码顺序 = ROM 块发射顺序, GCC2 保序)。恢复已验源序 (0,18..23,9) 即 OK。
候选阶段 bytecmp 差 40B 全部按 4 字节组落在 10 个 bl 槽 + mine.bin 尾部 148B 纯 veneer
(基址 0 超 ±4MB), 与既有判据一致, 非真实差异。

permuter 真 0 (10 个 gUnk 池字符号化, 经验 164/168 复用) + base.c 一次过无需探索;
fncheck OK 466B (35 池重定位, 10 bl 槽忽略); make exit=0 + SHA1 绿。

## 2026-09-06 glm5-8037FE8: sub_8037FE8 (720/1059)

**✅ sub_8037FE8 (198行, code_80264C0)** — sub_8037E14 同族克隆, 全流程同上一条; 仅 3 处常量差:
case18/20 `sub_8020974(obj+0xC, 0x3B9/0x3BA, 0x1B4, 0xD, 2)` (第4参 0xC→0xD), case22
`sub_801EEE4(obj, GetObjPool(), 1, 0, 0x28)` (r2 0→1, 栈参 0x3C→0x28)。
code_0.h `void`→`u32` (无调用点, 照先例)。
套件搭法已熟: target.s 由 nonmatchings asm 换名生成 + 10 gUnk 池字符号化 (经验164/168)。
permuter base score=0 一次过; bytecmp 对字面量 target.o 纯 10 bl 槽差 (⚠ 套件 target.o 是
符号化版时, gUnk 池字会另计 30B 假差异, 判读要么换字面量版要么按 4 字节组归类);
fncheck OK 466B; make exit=0 + SHA1 绿。

## 2026-09-06 glm5-80381BC: sub_80381BC (721/1059)

**✅ sub_80381BC (198行, code_80264C0)** — sub_8037FE8 同族克隆; 仅 case22 两处常量差:
`sub_801EEE4(obj, GetObjPool(), 1, 0xC, 0x1E)` (r3 0→0xC, 栈参 0x28→0x1E)。
code_0.h `void`→`u32` (无调用点)。permuter base score=0 一次过; fncheck OK 466B; SHA1 绿。

**小坑自伤**: bytecmp 用了上一函数遗留的 /tmp 字面量 .s 改名拼 target (内容仍是 8037FE8 的
字节), 多出 2B 差 — 恰为两函数仅有的常量差, 反向印证 base.c 正确; 用 80381BC 自己的
nonmatchings asm 重拼后纯 10 bl 槽差。教训: 字面量 target 必须从本函数 asm 现拼, 勿跨函数复用 /tmp 中间物。

## 2026-09-06 glm5-8038390: sub_8038390 (722/1059)

**✅ sub_8038390 (199行, code_80264C0)** — sub_80381BC 同族克隆; 仅 case22 一处差:
`sub_801EEE4(obj, GetObjPool(), 0, 0, 0x3E7)` (r2/r3 归 0, 栈参 999 超 movs 立即数域
自动走字面池, C 直接写 0x3E7 即可)。code_0.h `void`→`u32` (无调用点)。
permuter base score=0 一次过; bytecmp 纯 10 bl 槽差 (target 472B 多一池字);
fncheck OK 470B; SHA1 绿。

## 2026-09-06 claude-8030D9C: 8034BFC/80348A8 + 前批三函数 (727/1059)

**✅ sub_8034D94 (161行)** = sub_8034440 克隆, case8 sub_801EEE4 参数 (0,0x32)→(1,0x3C)。
**✅ sub_8035130 (243行)** = sub_8034F00 同构, case0 常量 0x38B/0x23→0x38C/0x14 (0x38C 可
movs+lsls 合成故无池字, 0x38B 奇数需池 —— 编译器常量合成差异佐证两函数源码同构)。

**✅ sub_8034F00 (243行, 21态)** — 尾部统一 sub_803F658(arg); case19/20 共享 gUnk_03000825++
尾块 (state 存储跳转线程化合并)。三个新坑:
① **常量半字 store 必须 struct 字段形式**: `*(u16*)(arg+0xB6) = 0x38B` 会多一条
   `adds r0, r2` 拷贝; `((struct ObjFadeSeq *)arg)->f_b6 = 0x38B` 才直载 r0 (最小实验证明
   struct 剪裁改变 store RTL; 库内先例 Sprite_SetupDialogArrow `sprNode->attr2 = 0x892`)。
② **case20 的 CSE**: `if (... && (v56 = gUnk_03000856) == 0) { sub_801CBA4(..., v56); }`
   赋值入条件让测试值存活作第 5 参, 免重载 (permuter 发现, 人工化保留)。
③ **case 源顺序 = 目标布局序 0,1,2,5,18,19,20,6,9** (GCC2 不重排 case 体)。
另 case0 的 `zero = 0; ... gUnk_03000825 = zero;` 命名临时把零物化提前 (同 8034440 case5 先例)。

**✅ sub_8034BFC (184行, 10态)** — case8: 局部 `u8 buf[8]` + `sub_80489E8(pool, buf, 1, 7)`
返回数 + `for (i=0; i<count; i++) if (((u32 (*)(void))Rng_LcgNext)() % 0x64 <= 0x27)
sub_8045F94(pool + buf[i]*0xC8, 3)` (旋转循环形态, count 是寄存器局部)。一次成型。

**✅ sub_80348A8 (382行, 30态最大)** — case0 把 sub_80489E8 结果写进**新登记全局**
`gUnk_0300083C` (输出表 = 新登记 `gUnk_03000830[]`, 0x830..0x83B); case24/26 用 7 参
sub_8020CC4 (0x37F/0x380, 0x114/0x14 差异); case27 遍历 gUnk_03000830[] 摘除对象
(ptr[0xBE]=0xFF, ptr[0xAB]=7, sub_80207A4()); case28 计数 >0x27 后清 0x54&=0xFEFF 进 0x1D;
case29 清 0xB0&=0xDFFF 落 9 (case6 的 else 经跳转线程化共享同一 state=9 存储)。
坑① **case25 极性**: 目标是 bit11 **清零**→Sfx_Play(0xA5,0,0)+进 0x1A (与 case2 相反,
与 0x54 字段语义"演出对象在场"相关)。坑② **case27 循环**: 直接 `*(u8*)(pool+...+0xBE)`
会得到旋转循环+无拷贝算术; 改**命名指针** `ptr = (u8*)(pool + gUnk_03000830[i]*0xC8);
ptr[0xBE]=0xFF; ptr[0xAB]=7;` 后 GCC2 出非旋转 for (init; b guard; body; incr; guard) +
`adds r2,r1; adds r2,#0xbe` / `adds r1,#0xab` 增量寻址 —— 命名指针对象改变了循环旋转决策。

**字节定论**: 5×fncheck OK (D94 364B / F00+5130 各 560B / BFC 408B / 48A8 852B);
两批 make exit=0 + SHA1 绿; 进度 722→727/1059 (68.6%)。code_80264C0 剩 38 个未匹配。

## 2026-09-06 glm5-8038568: sub_8038568 (728/1059)

**✅ sub_8038568 (206行, code_80264C0)** — 同族**双参**版: `u32 sub_8038568(u8 *arg, u8 *arg1)`
(照 sub_8034718 命名)。入口 arg1→r2、&gUnk_03000820→r3 (case21/23 `strb [r3]` 消费)。
case22 有本族首个真除法: `t1 = *(u16 *)(arg1 + 0x6E) / 3;` + `sub_801EEE4(arg, GetObjPool(),
0, 0xA, t1)` — `bl __udivsi3` + lsls/lsrs 0x10 截断。

**关键**: t1 必须是**命名局部变量先除后调** (照 sub_8045A74 的 `t1=(u16)(*(u16*)(o+0x6e)/10)`
先例)。首版把 `(u16)(.../3)` 内联进实参表 → GCC 把 GetObjPool 排到除法前, 且寄存器分配整体
漂移 (arg1 驻 r5、&gUnk 驻 r2), base score 380; 改命名局部后 arg1 驻 r2、&gUnk 驻 r3, 全对齐,
score 0。bytecmp 需给 `__udivsi3 = 0x08055d61;` 符号 (11 个 bl 槽)。
code_0.h `void`→`u32` (无调用点)。fncheck OK 482B; SHA1 绿。

## 2026-09-06 glm5-803874C: sub_803874C (729/1059)

**✅ sub_803874C (198行, code_80264C0)** — sub_8038390 同族克隆; 仅 case22 两处常量差:
`sub_801EEE4(obj, GetObjPool(), 0, 0xB, 0x1E)` (r3 0→0xB, 栈参 0x3E7→0x1E)。
code_0.h `void`→`u32` (无调用点)。permuter base score=0 一次过; bytecmp 纯 10 bl 槽差;
fncheck OK 466B; SHA1 绿。

**本日战果**: 一口气收割 0x08037E14-0x0803874C 六连族 (8037E14/8037FE8/80381BC/8038390/
8038568/803874C, 全部 729/1059)。族规律: 同一 24 态 switch(gUnk_03000820) 骨架, 差异仅在
case18/20 的 sub_8020974 第4参 (0xC/0xD)、case22 的 sub_801EEE4 五参组合、以及 8038568 的
双参+除法变体。经验 164/168 (target.s gUnk 池字符号化取真 0) 六连复用, 成熟管线。

## 2026-09-06 claude-8030D9C: 克隆族收割批 (716→738/1059, 69.7%)

相似度扫描 (.scratch/claude-8030D9C/famsim.py: 同模块规范化指令序列两两比对 + vs 已匹配模板)
发现 13 个克隆簇 + 36 个模板相似对。本批收割:

**✅ sub_804D708 (code_8044394)** = sub_804D310 纯克隆 (2行=标签), 池 0x08393B28 符号化。142B OK。
**✅ 262行三连体 sub_802F480/802DFDC/80309B0 (code_80264C0)** — 24态 BGM 演出 (尾部
sub_803F658+计数无条件); case0 备份 0xBF/C0 + sub_801CE80(obj,5,0x1B4,0xC,1) + 0xB6=2/0xB4=0
(struct 形式); case19 sub_8020CC4(0xB9,0x78,0x2E0,0xE,0x2F9,5); case20 keys=0x54&0x800 复用;
case21 flags=(&0xEFFF)|0x100; case9 三重测试 ret=1。**一次成型全 0** (三员逐指令全同)。
老挂起 80309B0 借此终结。
**✅ 294行五连体 sub_803586C/6034/5B04/5D9C/62CC** — F00 家族扩展 (0.873): case0 常量
0x392-0x396 (5D9C 用 movs+lsls 合成 0x394), case1/5 第4参 0xC, case9 改对象遍历
(gUnk_03000840[i]&0xF0==0x10 + Rng%0x64≤{0x27,0x13} → sub_8045F94(pool+(x&0xF)*0xC8, {2..6})),
case序 0,1,2,5,18,19,20,6,9。664B×5 OK。
**⏸ 67行对 sub_804BD54/804BE90 + 变体 804B7B0/804B8E8** — 高寄存器分配域 (arg0→sb/
table→sl/mask→r8): 最优 C 停 990, permuter 12k 迭代 → 420 (底分 5, do-while(0) 屏障 +
重算地址形态, 存 permuter/sub_804BD54/candidates/best420.c); 三变体 base.c 就绪待攻。
**⏸ 91-98四连体 sub_804D840/804DC24** — D3A0 同族: case1 的 obj[0xC2]=Rng&obj[0xBC]
复用 switch 值 (r4) + entry 索引 +8 不折叠; value 中转变量版最优 700 分 (base.c 留档);
D4FC/DB64 (17/26行差) 未动。**⏸ 444行对 803D20C/803CE0C (14行差) / 611行对 8029BF8/802A86C
(16行差) / sub_80368FC (F00变体 36行差) 未动。**
**⚠ 事故**: 收尾发现 src/code_80264C0.c 被截空 (与 glm5 并行写竞态), 从 HEAD 基线重放
全部 21 个合并恢复 (含 glm5 的 8037E14/7FE8/81BC/8390/8568/874C), 见 INCIDENTS.md。
**字节定论**: 22×fncheck OK (本批); make exit=0 + SHA1 绿; audit 738 全过。
**✅ sub_8051AEC (107 行, MOD-08 脚本 VM, 浮点插值家族)** — 2026-09-06 opencode:
- 家族同源: sub_801768C (src/code_8010F10.c) 完全同构 (同 5 参签名/同 float 表达式/同尾除), 先读其匹配 C 定框架。
- 本体: `s16 f(s16 base, s16 amp, s16 total, s16 step, u8 mode)`; mode1 = `amp * (step*10/total)/10`,
  mode2 = `amp * (((-10*step)/total + 20)/10)`, 返回 `base + step*result/total`。**结果直接写回参数 arg1** (累加器),
  无独立 result 局部 → default/case0 路径 arg1 原值直达尾部 r0。
- 卡点链: ① 先写 `s16 result = arg1;` 具名局部 → score 500, 入口参数转换序错乱 (arg2 在 arg1 前);
  ② permuter 跑到 280 平台期, 给 `arg1 = arg1;`/`new_var = arg3` 假招 (score 40 仍差入口序);
  ③ RTL 转储定位: s16 参数截断链 (ashift+lshiftrt) 被 **combine 吸收进首用点** (body 内 `result=arg1`),
  而 arg2 链多消费者留入口 → 序变 arg0,arg2,arg1,arg3; ④ 正解 = 复用参数 + `case 0: break;`
  (case 值域含 0 → 分派树 `cmp#1;beq;cmp#1;ble;cmp#2;beq;b` 与目标一致, 否则退化 `cmp#2` 直链)。
- 原型修正: code_0.h `void sub_8051AEC();` → `s16 sub_8051AEC(s16, s16, s16, s16, u8);` (无调用点, 零风险)。
- fncheck OK (248B @0x08051aec, 0 池重定位); 全量 make+SHA1 绿。进度 700/1059 (66.1%)。

## sub_805063C (0x0805063C) — tile 对写入器, 未收敛 (⏸)

**语义** (全解): 脚本 opcode 级 tilemap 写入器。参数 (u16 *dest, u8 charIdx, u8 x, u8 y)。
p = dest + (x + y*32) (u16); v = *(u16*)(0x0862D574 + x_fr*2 + charIdx*18), x_fr=gUnk_03000F2A。
- v ≤ 0xDF: 直接瓦片 id。p[0]=0xB000+((v&0xFF)<<1); p[0x20]=0xB001+((表重读&0xFF)<<1)
  (表重读必须写在 p[0] 之后, 否则 GCC2 CSE 掉第二次读)。
- 0xDF < v ≤ 0xEFF: 在 gUnk_03000EE8[0..gUnk_03000F24-1] 搜 v (找到即 break), 得 i。
  T=(i+0xE0)<<1; p[0]=0xB000+T; p[0x20]=0xB000+(T+1)  [目标: (T+1)+0xB000, 0xB000 在 r8]。
- v > 0xEFF: ret=1。末尾 gUnk_03000F2A++。

**已攻克的结构点** (均有实测支撑):
1. 外层 if 必须倒写 `if (v <= 0xEFF) {...} else {ret=1;}` 才得目标块序 [Z][Y][W][ret1]。
2. 内层也倒写 `if (v <= 0xDF) {Z} else {Y}`。
3. 首读用 `off` 变量 `off = gUnk_03000F2A * 2 + charIdx * 18;` 才避免 GCC2 因式化为 (9ci+x)*2。
   目标序: base→r7, x→r3, 2x, 18ci→r6, 求和。
4. Z 常量 SImode: `(v & 0xFF) << 1` 必须先落进 u32 变量 (z1/z2) 再 +0xB000/0xB001,
   否则 GCC2 收窄成 HImode → 池 0xFFFFB000/0xFFFFB001。
5. Y 的 0xB000 用变量 base2 (=0xB000, 在循环前赋) → GCC2 载入寄存器 (ip 或 r8)。
6. store2 用 VAF 招式 `(u = t + 1) + base2` → 得目标 `adds r0,#1; add r0,ip/r8` (免 fold/免 orr)。
7. 循环必须显式 do-while 包 `if (i < n)` 才得目标"不剥 q[0]"形态 (for/while 都会被 GCC2 剥首元素)。

**卡点 (未收敛, 目标 vs 我, fndiff 最好 3170/5445, bytecmp 差 114/228B)**:
A. `T=(i+0xE0)<<1` 的移位编码: 目标 `lsls #0x10; lsrs #0xf` (HImode 移位), 我怎么写都
   `lsls #0x11; lsrs #0x10` (SImode 移位+截断)。g1 最小复刻 sub_8049B70 同结构也一样
   #17/#16 —— 目标/sub_8049B70 的 #0x10/#0xf 编码在当前 tools/agbcc 无法复现 (试过
   (u16)cast、*2、u16 中间变量、-g flag、old_agbcc, 全部 #17/#16)。疑 GCC2 构建版本差异。
B. 寄存器 home: 目标 v 住 r4 且 Z 分支不破坏 (用 r0 做 ands), 我 v 被 `ands r4,r2` 覆盖;
   目标入口 `adds r2,r4,#0` (v 复制去 cmp) 与循环前 `adds r4,r2,#0` (复制回) 两条 home 复制,
   我没有。目标 ret→sb(r9), 0xB000→r8, x-addr→ip, base→r7; 我 ret→r8/r9 依结构漂移。
   唯一把 prologue (mov r7,r9; mov r6,r8; push {r6,r7}) + base r7 + x-addr ip + 0xB000 r8
   全对齐的是 permuter 把 y 改成 unsigned int 的 3170 版 —— 但那毁掉 y 的 u8 截断
   (目标 `lsls #0x18; lsrs #0x13`)。鱼与熊掌未兼得。

**最佳候选**: permuter/sub_805063C/base.c (v_IA, 4990) 与 permuter/sub_805063C/output-3170-4/
source.c (3170, y 改 int 不可读)。permuter 产物 <2905 的都是 score 假高 (经验 29, 池未重定位)。

**✅ BattleFx_UpdateTable (sub_8019784, 296 行, MOD-03 淡出波表生成器)** — 2026-09-06 opencode:
- 语义: 逐帧调用 (sub_801889C); 按 gFlashFlags 分派生成 gUnk_03000390[256] 淡出波表。
  `if (flags&0x1000) { case1: 角度步进 g386%360, 逐项填 (s8)g4D0[g386%360]; case2: 斜坡 + 0x50..0x9F 逐项 ±, 0x10/0x20 方向, 镜像抄 160-i }
   else if (flags&0x2000) { case1: 相位写回数据表 amp(cos-sin)+amp; case2: 转 0x1000+清 g386 }`。
- 匹配全程卡点链 (全部有 RTL 转储佐证):
  ① 结构骨架先按 sub_80199E0 族 + gFlashFlags 语义重建 (score 14065 → 8650, 无结构差);
  ② **switch 分派树**: 第一 switch 用 `case 0: break;` 显式 (才能得 `cmp#1;beq;cmp#1;bgt` 树), 第二 switch 同形但目标出 `ble` 版 → 全是 later block-relayout, 结构 C 相同即可 (别在两个 switch 里写不同形式);
  ③ **16 位局部共享**: angle(case1)/v(case2)/amp_float(0x2000 case1) 全用同一个 `s16 tmp` → 同寄存器 r6, 一改全消 (8650→7755);
  ④ **int diff 中间量锁定符号扩展**: `tmp = (s16)((u16)g386 - i) >> 2` 会被 gcc 折叠掉 (s16)i 的符号扩展; 拆 `int diff = g386 - i; tmp = (s16)diff >> 2;` 后出目标 `lsls r1,r3,#16; asrs; subs; lsls #16; asrs #18`;
  ⑤ **case2 条件翻转**: `if (g386 <= 0x10F) { 循环 } else { 置 0x4000 }` (循环在真支, 目标直落) vs 反写 `>0x10F` 出 `ble` 到循环 — 目标要 `bgt` 到 setflag;
  ⑥ **核心: 去 local 快照, 直接读全局 + 常量在前**: `u16 flags = gFlashFlags; if (flags & 0x1000)` 让 and 的操作数全是 REG → gcc 把**变量**当累加器 (`adds r0,r3;ands r0,r2` ✗); 改 `if (0x1000 & gFlashFlags)` (常量在源文本前 + 内存操作数) → 常量进累加器 (`adds r0,r2;ands r0,r3` ✓, 目标全 3 处 and/or 同此)。expab_binop 的 commutative swap 只在 op0 是 CONST_INT 或 REG 顺序下触发, 经 preserve_subexpressions_p(-O2) force_reg 后行为不同;
  ⑦ 原型/命名: code_0.h `void sub_8019784();` 保持; 全链改名 → `BattleFx_UpdateTable`。
- fncheck OK (604B @0x08019784, 0 池重定位); 全量 make+SHA1 绿。进度 718/1059 (67.8%)。

## 2026-09-06 glm-batch: code_8044394 批量攻坚 (2 匹配 + 3 深度候选)

任务: 匹配 code_8044394.c 下 <200 行全部未匹配函数 (~60 个)。本轮完成 2 个, 3 个推进到 90%+ 并留档。

### ✅ sub_8048C30 (43行, fncheck 80B)
- 结构 = 外层 `if (obj[0xBE] <= 0xA)` + 内层**无 default** switch (case 组 0..7 / 8..10 物理重复同体,
  GCC2 比较树 + jump.c 折叠体; 经验 69/72); default = `(u8)(obj[0xBE]-0xC) <= 0x64` 边界 + 查表。
- 表 0x0839D5BC 访问三坑: ①数组形式 `tbl[k*6+4]` 出 (idx+4)+base 序 ✗; ②struct 视图被 ARM ABI
  对齐到 8B (lsls#3) ✗; ③`*(tbl + 4 + k2*6)` 直用符号把 +4 折进池常量 ✗ —— 正解 = **指针局部**
  `const u8 *tbl = gUnk_0839D5BC;` + `*(tbl + 4 + idx)`/`*(tbl + 5 + idx)` (池放基址, 运行时 adds #4/#5,
  第二次访问复用 r2 in-place += 5)。idx=k2*6 须独立变量 (两次 `*(p + idx)` 共用)。
- 两个 flag 先读入变量再 `||` (目标两次 load 后才比较, 非短路形式)。
- permuter base score = 0 (compile.sh 加 `.equ gUnk_0839D5BC, 0x0839D5BC` 解决池重定位虚分, 经验 29)。

### ✅ sub_8048C80 (58行, fncheck 106B, 3 bl 忽略)
- **幽灵栈帧真因**: 未使用的 `u8 values[8];` 局部数组 (经验 174)。此前挂起记录猜"编译器版本差异"不成立。
- 阈值求和: `threshold = base + (sw + v1);` 两次独立 SET (经验 175)。
- switch case 序 = ROM 序 2,5,3+default (经验 124); Rng 需 `((u32 (*)(void))Rng_LcgNext)() % 100` + (u16) 截断。
- code_0.h 原型 `void sub_8048C80()` → `u8 sub_8048C80(u8 *)` (无调用点, 安全)。

### ✅ sub_804BD54 (67行, 2026-09-09 opencode 匹配)
- C10C byte-style 模板 + `(s8)entry[0] == -1` continue + mask 变量。唯一残差 = -1 常量物化点
  (见 EXPERIENCE 176)。候选 = permuter/sub_804BD54/base.c。
- **正解**: 孪生 `sub_804B8E8` (同 67 行同结构, 表 03000AE8 + 调 C3A4/C420) 已被 antigravity 于 09-09 攻克,
  直接套用其模板 (表→03000BE8, 调用→C5F8/C674)。破解 EXPERIENCE 176 的三件套:
  `int empty=-1` 函数作用域 + `for` 循环 + 块内 `base/mask` → -1 循环内物化 (`movs#1/negs`)、base 进 sl;
  0x20 测试复用 `u32 v` (`v=0x20; v&=flags`) → `ands r0,r1`。
- 原型 `u32 sub_804BD54(u8, u32)` → `void sub_804BD54(u8, u8)` (目标 arg1 入口 `lsrs#0x18` 截断须 u8 参;
  唯一调用点 scene_obj_fx.c:32 忽略返回值, 零影响)。
- fncheck OK 132B @0x0804bd54 (2 bl 槽忽略); 全量 make + SHA1 绿。进度 757/1059 (71.5%)。

### ⏸ sub_8049D58 (77行, 候选差 ~8B 指令)
- 状态机 + DMA (DmaCopy16(3, 0x02035AC0, 0x06007000, 0x800) + DmaWait —— cnt=0x80000400 反推 16bit/0x800B)。
- case0 的 `u8 *ptr` 局部防池折叠 (同 8C30 ③)。残差 = 入口截断 `adds r1,r0,#0` 保 r0 原始 arg0
  (case1/case2/default 经 copy-prop 直返 r0; 我方就地截断杀 r0 → 全路径重载 r3)。
- 候选 = permuter/sub_8049D58/base.c (v3)。

### ⏸ sub_804B3C0 (79行, 候选差 74B = 纯寄存器 home 级联)
- 控制流/掩码/循环全部对齐: 掩码须变量 `mask = ~0x10;` (常量被窄化成 movs#0xEF, 规则 76 推广);
  循环须显式 `if (count != 0) { do { entry[0xF] += 1; if (entry[0xF] > 7) break; count = (u8)(count >> 1); } while (count != 0); }`
  —— while 形式被 GCC2 旋转 + 首迭代剥离 (0-store 前传成 movs#1), do-while 不旋转逐指令命中;
  count 用 `max` 局部 (两分支各读一次 entry[2])。
- 残差 = flags 落 r5 (我 r2) / count 移位 tmp (我 in-place) / ldrsb temp home —— local-alloc 优先级。
- 候选 = permuter/sub_804B3C0/base.c (v5)。

### ⏸ sub_804D840 (91行, 候选差 ~4 指令)
- D1B4+DABC 混合: rng%0x65<=0x45 定 obj[0xBC] → `switch (*(s8 *)(obj + 0xBC))` case0 = D1B4 式 +2 读 /
  case1 = `obj[0xC2] = *(s8 *)(obj + 0xBC) & Rng()` → DABC 式 `((Unk_804DABC_Ptr *)*)->field_8[obj[0xC2]]`。
- 关键: (s8) 强转会出 ldrb+shifts; 必须 s8 指针解引用才出 ldrsb; switch-case 值会被 GCC2 常量传播进
  case 体 (v1 的 `& 3` 幻影), 用表达式 scrutinee + case 内重评同一表达式 (CSE 复用寄存器) 可避免。
- 残差 = 入口 scrutinee 区: 目标 `movs r4,#0; ldrsb r4,[r1,r4]` (复用 if/else 的 r1 地址, 零索引落 dest),
  我方多一条 `adds r4,r1,#0` 地址拷贝 + scrutinee 落 r0 —— local-alloc join 点 tiebreak。
- 候选 = permuter/sub_804D840/base.c (v3)。

### ✅ Op_AddPartyMember (原 sub_804F7F8, 117行, 2026-09-06, zcode-glm)
- 语义 = Op_RemovePartyMember 的镜像: 把 data[1] 升序插入 gPartyMemberIds[0..4] (已存在跳过;
  插入点后整体后移、末位溢出丢弃), Chara_ClearTempStatus + gPartyMemberIds[5]=0xFF,
  再做编队槽管理 (gPartyStats[id-1].field_unk[5] 记录槽号 ↔ gBattleFormationIds[0..4])。
- **part1 插入排序**: 目标是"头在顶不旋转"的 do-while —— C 必须 `i=0; if (ids[0]==m) goto after;
  do {...} while (ids[i]!=m);` 且出口用 **goto** 不能用 break (break 触发 GCC2 旋转+首迭代剥离,
  出现跳进循环头 + data[1] 三份拷贝, 差 2000 分级)。break→goto 一处改动词节全中。
- **双 home = 双源变量**: 目标 memberId 走 r4(早)→r7(晚) 两个 home = 源码里 scan2 的编队存储
  用的是**第二个变量** newId (`newId = memberId;` 在 scan1 入口), GCC2 无 live-range split,
  单变量不可能两 home。t1: else 块首 `newId = memberId;`。
- **scan1/scan2 循环**: 也必须 do-while (`i=0; do{...}while(i<=4);`), for 形态会多出顶部守卫。
- **r6/r7 互换 (残差最后 2 条)**: ptr 应落 r6、newId 落 r7, 我方反着 —— greg dump 量化:
  newId pri 0.0741 (2 refs/27 insns) vs ptr 0.0678 (4/118), newId 以 9% 优势先拿 r6。
  解法 = part1 循环头加死赋值 `newId = gPartyMemberIds[i];` (无读取, cse2 删除) 拉长 newId
  寿命 → pri 掉到 0.017 → ptr 回 r6。详见 EXPERIENCE 177。
  (do-while(0) 屏障也能翻 newId 但把 ptr 挤到 r8, 级联更大; u8/u16/s16 类型无效 —— PROMOTE_MODE 全 SImode。)
- 验证: permuter 25 (=6 个池字的渲染差地板, bytecmp/fncheck 为准); bytecmp OK (仅 bl 槽 4B,
  链接期解析); fncheck OK 224B; make+SHA1 绿; 已改名 Op_AddPartyMember (全链, SHA1 复绿)。
- 教训: 挂起 note 说"差 170B+"的前人卡点其实是整体结构 (for/break/单变量), 换到 do-while+goto+
  双变量后只差 2 条 home, 再用死赋值收尾。**先从同 C 文件已匹配的镜像函数抄结构**是最高效起手。

### ⏸ sub_805063C (114行, 语义全解, 差35B, 2026-09-06, void-main)
- 语义 = tile 动画帧写入(Op_LoadTileGfx 家族, 与 code_8044394.c 的 sub_804ABF8 同族但更复杂):
  `frame = *(u16 *)(gUnk_0862D574 + gUnk_03000F2A * 2 + arg1 * 18)` (动画 arg1 的当前帧值);
  `ptr = dest + (arg2 + arg3 * 32)` (32 列 tilemap 的 [行 arg3][列 arg2]);
  frame > 0xEFF → return 1 (动画越界); frame <= 0xDF → 直写帧号 `ptr[0]=(frame&0xFF)*2+0xB000,
  ptr[0x20]=(frame&0xFF)*2+0xB001`(帧值即 tile 对, 直写); 0xE0..0xEFF → 在 gUnk_03000EE8[0..gUnk_03000F24)
  线性搜 frame 取下标 i(未找到 = count), 写 `t=(u16)(i+0xE0)*2; ptr[0]=0xB000+t, ptr[0x20]=t+1+0xB000`
  (重定向到已分配块, 每块 2 tile); 成功路径 `gUnk_03000F2A++`。返回 u32 (0/1), 三个调用方
  (sub_8050720/sub_805144C/sub_8051BE4, 全未匹配)均忽略返回值, 实参 = (0x02005800, u8, u8, u8)。
- 结构要点(已验证): (1) 两个 if 必须写 `if (frame <= 0xEFF){ if (frame <= 0xDF){直写} else {搜索}
  counter++ } else {ret = 1}` 的嵌套 <= 形式 —— GCC2 的 if/else 布局是"then 贯穿/else 分支",
  直写形式才落出 bhi 到尾部 ret=1 块; (2) 表读取表达式必须**内联展开多次**(比较×2/直写×2/循环×1),
  不能缓存进局部 —— 读取是 u16 访问与 strh 可能别名, store 后必然重读, 缓存会让 store2 丢失重读;
  (3) 搜索循环必须 `for(i=0;i<count;i++){if(arr[i]==frame)break;}` 的 **break 形式**, && 条件形式
  会被 GCC2 旋转成 [incr, c1, c2] 体, 与目标 [c2, incr, c1] 不符; (4) 循环比较的操作数顺序要写
  `frame == arr[i]` (寄存器侧在前) 才出 `cmp r4, r0`。
- **常量物化三大坑 (本轮最大发现, 见 EXPERIENCE 167-169)**: ① 直接 `ptr[0] = x + 0xB000` 的
  strh 目标会把常量按 HImode 符号扩展成 -0x5000 进池 (值等价字节不同); 必须经 u32 中转
  `v = ... + 0xB000; ptr[0] = v;` 保持 SImode 正数 —— 可移位(0xB000=0xB0<<8)走 movs+lsls,
  不可移位(0xB001)走正数池。② `+1 + 0xB000` 会被 combine 折叠成 0xB001 池; 目标里不折叠是因为
  GCSE 把 0xB000 合并成共享伪寄存器(寄存器不可折叠) —— 多处使用的常量让 GCC 自己建寄存器即可,
  但 0xB000 的物化要落在两个 store 之前(循环头前的 preheader), `v = x + 1;` 与 `v += 0xB000;`
  拆两句可阻止重结合。③ 同一常量 0xB000 目标里出现两种物化形态: 直写分支 `movs 0xB0; lsls 8`
  (thumb.md define_split 扫描 i=0..24 低位优先的产物), 搜索分支 `movs 0xB; lsls 12` —— 后者是
  **物化被 GCSE/LICM 提升后重拆分 + combine 合并移位的双拆分痕迹**, 说明原 C 的 base 赋值在循环内
  被提升出循环; 本复刻(赋值在循环内/循环外)都只得到单拆分形态, 提升未复现。
- **兄弟函数结构移植失败教训**: sub_804ABF8 的 `base=表; off=counter*2+arg1*18; 读(base+off)` 局部
  变量风格在 head 上形状完美(表地址最先加载、off 单次计算), 但 off/tbl 的 pre-cse 存活范围跨循环
  (循环内的读取引用它们) → global-alloc 的 REG_LIVE_LENGTH 拉低 pri → 全部级联到高位寄存器
  (prologue 多存一个)。v19/v20(4565/2425) 均劣于全内联形态; 循环内读取必须保持全内联, tbl/off
  只出现在 head/store1 路径才可行(v20 2425, 仍劣)。最终最佳 = 全内联 + permuter 变体。
- **permuter 多轮研磨实录**: 10 轮 × 580s (-j 1), 每轮 promote 最优再跑。有效技巧:
  `i = arg3 * 32; ptr = dest + (arg2 + i);` (把乘法提升为具名变量, 1235 分) → `int new_var;
  new_var = 0xB001;` (把池常量变成具名 int 变量, 665 分) → base 赋值挪进循环 (885 分) →
  `tbl + (new_var2 = off)` 内联赋值 (595 分)。new_var/空转 if-else 屏障的本质 = 改变伪寄存器
  创建顺序与 PRI (floor_log2(nrefs)*nrefs/LL*size, global.c allocno_compare)。
- 残差 35B 明细: ptr 求和 dest(r2 vs r3, 6B)、直写分支 +tbl 地址 dest 与 store1 和的 dest
  (~14B)、搜索分支 i/count 寄存器互换 + ptr+0x40 落 r4 vs r7 + base 物化位置与形态 + E 值
  re-read(经 tbl+off 地址重读) vs 寄存器 copy (~15B)。全部是 GCC2 local-alloc 排列与
  拆分/提升 pass 历史痕迹, 表达式/语句形状已穷尽 (v1-v25 变体见 permuter/sub_805063C/ 历史)。
- 候选 = permuter/sub_805063C/base.c (bytecmp 35B/228B 差, 语义与目标逐条核对一致)。
  **未达分数 0, 按铁律 6 未合入 src**。下一步建议: 从 sub_8050720 (同文件未匹配, 大函数) 匹配后
  回看其对本函数的调用点上下文; 或用 -da 转储对比 allocno PRI 排布逐项拉齐 (EXPERIENCE 117/177)。

### ⭐ BD54 四孪生 (BD54/B7B0/B8E8/BE90) 的 -1 提升问题破解 (2026-09-06 第二轮)
挂起多轮的 `-1 提升问题` (EXPERIENCE 176 的失败存档) 本轮通过 **RTL 级取证 + 受控实验矩阵** 完整破解:
- `agbcc -dL` loop dump 直接打印 movable 的 savings/life/desirable —— 无需猜测;
- 直写 `(s8)x == -1` 的 -1 链 (QI化+扩展对, savings2/life4) 被 loop.c 合法提升 (104 ≥ 45);
- 正解 = 中间 u32 变量 (convert_move 出 ldrsb) + 无符号比较 (常量 INTVAL>0 走 cmpsi PATH A,
  force_reg 伪寄存器 life=1 不可提升) + flags 先读 + `index*16 + base` 操作数序防等价替换。
四孪生候选全部收敛到 **20B 差** (bl 槽 8B + scratch 旋转 12B), 详见 EXPERIENCE 181。
scratch 旋转 (flags/v/-1 的 r0/r1/r2 分配) 为最后一道 local-alloc tiebreak, 待后续攻克。

**⏸ sub_804621C (105 行, MOD-07 对象池找槽/过滤助手)** — 2026-09-06 opencode:
- 语义: `u8 f(u8 *obj, u8 *out, u8 mask)` 返回 count。找 pool 槽 5..0xB 中 sub_8045F10(..,0x1FF)==2
  的 (活动对象), 再过一遍 `(pool[slot][0xAC] & mask) == (obj[0xAC] & mask)` 过滤, 写 out[0..]。
- 结构里程碑: buf 必须 `[0xC]` (栈 12B, 清 7 项); filtered 须先算 (GetObjPool 之前); 3 参签名
  prototype 冲突→升 u8 原型后 callers 零字节差 (全部传 ptr,ptr,const)。
- 卡点 (已定性 global-alloc): 4 个跨 sub_8045F10 调用的长寿命值, 目标分配 arg1→r9(arg2→r10/filtered→r8/pool→r7
  (push 3 高), 我方 arg1→r8/arg2→r9/filtered→r7/pool→r6 (push 2 高, 整体低一位)。
  qtydump.sh: local-alloc 块块 0 qty → 决策在 global-alloc, C 写法层改不动。连同 `cmp r4,#0xc;bcc`
  (agbcc 对 u8<12 一律折成 cmp #0xb;bls) 一起归档 EXPERIENCE 失败存档。
- 已试 ~30 变体无效 (decl 序/buf 尺寸/pool 类型/循环形态/goto/别名/permuter×2)。候选存
  permuter/sub_804621C/base.c。释放 : 已转挂起。

## 2026-09-07 opencode: 数据区 blob 搬移批 (data_805769C.c, 0x0808A04C..0x080BAF54)

> 依据 `scripts/data.json` (name/address/size), 把 `data/data.s` 的 `rom_data` blob 头部连续区域
> 逐块搬到 `src/data_805769C.c` 并配语义命名 + `// 0x08xxxxxx` 地址注释。每块搬移后
> 更新 blob 起点 (`data/data.s` incbin 偏移), `make` + `sha1sum -c` 全绿 (744/1059)。

### 搬出的块 (按地址序)

| 地址 | 符号 | 大小 | 语义 | 依据 |
|---|---|---|---|---|
| 0x0808A04C | `gMenuEntDescGroups` | 0x1E8 | 菜单实体描述组 26 组, `gUnk_087EA138[26]` 指向; 每组 `{u8 count; count×{flags, animShift, palIdx, strlen, glyphs[]}}`; MenuEnt_ParseAll/Range→ParseDesc 消费, glyphs 是调色板动画序列 | data.json 指针表 + 逐组解码验证 |
| 0x0808A234 | `gMenuEntityPaletteTable` | 0xF80 | 124 个菜单实体 OBJ 调色板 (124×0x20), PaletteTransfer_Update 按序列字节查表 DMA | 已在 linker.ld 有绝对符号, 移除改由 C 定义 |
| 0x0808B1B4 | `gFlashFxPaletteTable` | 0x1C0 | 14×0x20 闪光特效调色板, sub_804FB24 (脚本 opcode) blend/闪光分支 DMA 16 色到 OBJ bank 4 | asm 引用 0x0808B1B4 |
| 0x0808B374 | `gMenuEntPaletteFramesTail` | 0x460 | 35×0x20 MenuEnt 调色板帧库后段 (大表项号 0x8A..0xAC, PaletteTransfer_Update 帧号寻址) | 见 progress.md 2026-09-07 追踪 |
| 0x0808B7D4 | `gSaveMenuUiPalettes` | 0x40 | 存档菜单 UI 2×0x20, sub_8011454/sub_80160F4 DMA 到 OBJ bank 14 (0x050001C0) | code_8010F10.c:729 |
| 0x0808B814 | `gData_0808B814` | 0x2F740 | 混合数据区: 精灵动画模型组 (gUnk_087EA1A0[248] 指向) + byte_8091948 NPC槽组表 + 成长曲线/经验表 + 技能道具表 + 消息文本 + 名字串 + 地图BG表 + 存档数据族; 均经 linker.ld 绝对符号访问 | 指针表扫描 + 68 个代码引用点 |

### 关键发现
- **data.json 的块切分与代码引用高度一致**: 26 个菜单实体组正好被 0x087EA138 指针表 (26 项) 一一指向,
  格式 `{count, records}` 逐组解码零误差 (计数值、strlen、palIdx 全吻合)。
- **0x0808B814 是混合区不是纯动画池**: 初始名 `gSpriteAnimModels` 会误导, 改名 `gData_0808B814`。
  内含 68 个被代码引用的独立符号 (经验表/道具表/NPC 槽组/存档签名 aLunar11209 等),
  全部通过 linker.ld 绝对符号 + 原字节保留, 未做进一步切分 (后续可逐个拆出具名)。
- **搬移方法**: 数据定义写在 `data_805769C.c` 末尾 (紧跟 gMapViewportBoundsTable 之后, 保持 ROM 顺序),
  再改 `data/data.s` 的 incbin 起点偏移, `arm-none-eabi-nm -n` 验证符号落址 == data.json 地址。
- 新 raw bin: `data/raw_data/gData_0808B814.bin` (由 baserom 0x0808B814..0x080BAF54 提出)。

## 2026-09-07 sensenova: sub_804FB24 473指令全量完全匹配 (0 字节差, 1264 B)

- **函数**: `sub_804FB24` (`0x0804FB24`, 模块 `src/code_804F0B8.c`, 660 行汇编, 473 条指令 / 1264 字节)
- **语义**: 脚本 VM 的视口控制 / 颜色混合 / 屏幕特效 opcode 解释器 (Op_ 核心分发器)。
- **核心突破与卡点根因**:
  1. **最后的 1 字节瓶颈**: 候选一直稳定在全函数 472/473 指令一致, 仅差偏移 `0x4dc` (Target: `8282 strh r2, [r0, #20]` vs 候选: `8284 strh r4, [r0, #20]`)。
  2. **深入 RTL 排查根因**:
     在 `else` 分支 (`data[2] == 0`) 中, 入口通过 `ldrb r2, [r3, #2]; cmp r2, #0` 将 `r2` 置为 0。整个 switch 分派树直达 `case 0xCA` 期间从未改写 `r2`。
     然而在 `case 3:` 中写 `gDrawCamEaseActive = data[2];` (QI mode) 使用了伪寄存器 `reg:QI 26` (映射到 `r2`); 而 `case 0xCA:` 写 `gViewportFlags[10] = 0;` (HI mode) 则使用了 CSE 识别出的常数 0 伪寄存器 `reg:SI 27`。
     因为两分支同在分派树下, `reg:QI 26` 与 `reg:SI 27` 并发存活, `global_alloc` 冲突图判定 `26 conflicts 27`, 无法分配同一物理寄存器, 导致 `reg:SI 27` 被迫提升到 `r4`, 并在入口生成 `adds r4, r2, #0`, 尾部产生 `strh r4, [r0, #20]`。
  3. **对称写 0 破解冲突**:
     注意到 `if` 分支内 `case 3:` 写入 `gDrawCamEaseActive = 1;`, `else` 分支逻辑上即为 `gDrawCamEaseActive = 0;`。将 `case 3:` 改写为 `gDrawCamEaseActive = 0;` 后, 两个 case 完全复用相同的 0 伪寄存器, 冲突彻底消除! 入口 `adds r4, r2, #0` 瞬间消失, 尾部自然生成 `strh r2, [r0, #20]`, permuter 分数直接归 0!
  4. **原型统一与直接调用**:
     在 `permuter/sub_804FB24_1/base.c` 中声明为 `extern void MapBg_LoadFull();` (K&R 无参), 因此原本即为无强转直接调用。合入工程后, 将 `include/code_0.h` 的原型声明同步修正为 `void MapBg_LoadFull();`, 并将 `src/code_8005020.c` 定义调整为兼容的 K&R 风格 `void MapBg_LoadFull(arg0) u8 arg0;`, 使 `src/code_804F0B8.c` 同样保持干净的直接调用 `MapBg_LoadFull(data[2] + 0x81);`, 两函数均 100% 字节精确匹配。
- **验证**: `python3 scripts/fncheck.py sub_804FB24 MapBg_LoadFull` 均验证通过 (0 字节差)。


## 2026-09-07 claude-opus-4-6: sub_804FB24 → Op_SysEffect 改名 (全链, fncheck/SHA1 绿)

- **触发**: 玩家实测触发该 opcode 后内存 dump 断点验证: r0=0x03000E6C=脚本 PC 槽, 槽值 0x02016EB2,
  脚本字节 `4D 64 00` = subop 0x64/arg 0 → `Bgm_Continue()` 分支; gUnk_0862D434 slot 0x4D → 0x0804FB25 确认。
- **语义定性**: 脚本 VM 万能系统/屏幕特效 op (opcode 0x4D, 3 字节 `4D <subop> <arg>`), arg!=0=执行/
  arg==0=复位, 按 insn[1] 分派: 抖动/白闪/BG 层开关/相机缓动/5 步调色板渐变/存档菜单/等 A 软复位/
  OBJ 调色板 10 帧渐显/满血/BGM 恢复/整图装载/白化淡入淡出。详见 MOD-08 新增条目。
- **改名**: `scripts/rename_fn.sh sub_804FB24 Op_SysEffect` (ll.cfg→code.s→4 引用文件→gen_asm→fncheck
  1264B OK→make+SHA1 绿)。code_0.h 原型仅换名不动签名。
- **ROM 数据建议名** (待登记): 0x08289B6E = gObjPalFadeInSteps (case7 的 10×0x20 OBJ 调色板渐显步进表),
  0x083936A8 = gObjPalFadeInFinal (末帧 16 色→OBJ bank10)。
- **误注修正**: docs/ATLAS.md / FAMILIES.md / remaining.md 仍列其为未匹配 — 系 sensenova 合入前的
  旧报表, gen_reports.py 可再生成, 未手改。

## 2026-09-07 claude-opus-4-6: Op_SysEffect 语义重命名深化 (枚举/符号化/下标枚举)

- **目标**: 在保字节精确前提下把Op_SysEffect 全部可读性命名落实。
- **改动**:
  1. 函数 `sub_804FB24` → `Op_SysEffect` (rename_fn.sh 全链); 参数 `ptr`→`ppScriptCursor`; 局部 `data`→`insn`,
     `step`→`fadeStep`, `clear`→`palDst`。
  2. 枚举 ×4 (code_804F0B8.c 内): `SysFxSubOp`(SYSFX_SHAKE..SYSFX_WHITEOUT, 14 个子命令)、
     `SysFxPalStep`(subop4 的 arg 1..5: PAL_BLACKIN/FLASH_SETUP/BG_PAL_CLEAR/FADE_2PH/FADE_LONG,
     case 标签用 `SYSFX_XXX - 1` 保持原 switch(insn[2]-1) 形状)、`SysFxShakeMask`(subop0 arg 档位)、
     `SysFxSaveUiStep`(subop8 的 4 步循环)。
  3. `iwram.h` 新增 `ViewportFlagIdx` 下标枚举 (VF_EFFECT_EN=0/VF_SHAKE1_OFF/VF_SHAKE2_OFF/VF_FLASH_CNT/
     VF_SHAKE_MASK=4/VF_WHITEOUT_CNT=10/VF_FADE_LONG_CNT=11/VF_SAVEUI_STEP=12/VF_BGMAP_FILL=13/
     VF_FADE_PHASE=14/VF_FADE_FRAME=15), 全 TU 引用点换名。
  4. 全局符号化: `gUnk_03000E6C`→`gScriptCursor`(8 引用), `gUnk_0862D434`→`gScriptOpcodeHandlers`,
     `gUnk_030047F4`→`gPaletteFxPhase` (iwram.h+linker.ld 同步)。
  5. ROM 数据符号: `gObjPalFadeInSteps = 0x08289B6E` (10×0x20 渐显步进表),
     `gObjPalFadeInFinal = 0x083936A8` (末帧 16 色, 亦为 MapScene_Load 场景调色板 0x08393688 表内 +0x20),
     gFlashFxPaletteTable/gCutsceneGfxBuf 直接引用已有符号。
- **字节坑 (新经验)**: `palDst = (u16 *)gCutsceneGfxBuf` 使 fncheck 差 +0x180..0x18b —
  `gCutsceneGfxBuf` 是**地址常量** (linker 绝对符号), GCC2 把它当循环不变量保存 r4
  (多 `adds r4,r1,#0`), DMA 源池重定位也不同; 必须写 `(u16 *)0x02020000` 字面量。
  与 sub_80526A0 的 `0x02016200` 字面量坑同根 (EXPERIENCE 已有同族记录)。
  而其余符号 (gFlashFxPaletteTable/gObjPalFadeInSteps/gObjPalFadeInFinal) 是**数据池重定位**,
  fncheck 施加后字节一致, 可安全符号化。
- **验证**: fncheck Op_SysEffect OK 1264B; code_804F0B8 TU 全部 13 个已匹配函数 fncheck 0 FAIL;
  make + SHA1 绿; audit 改名漂移 0。

## 2026-09-07 claude-opus-4-6: data.json/raw_data 与调色板符号名同步

- `scripts/data.json` 三项改名 (最小 diff, 仅动 name 字段): `unk_808B1B4`→`gFlashFxPaletteTable`、
  `unk_8289B6E`→`gObjPalFadeInSteps`、`unk_83936A8`→`gObjPalFadeInFinal` (与 linker.ld/代码符号一致)。
- `data/raw_data/*.bin` 三文件同步改名; `src/data_805769C.c` 的 INCBIN 路径更新为
  `gFlashFxPaletteTable.bin`; docs/PLAN_DATA.md 示例引用同步。
- 注意: gObjPalFadeInSteps(48318B 大 blob)/gObjPalFadeInFinal 未被 src INCBIN, 仅 dumpraw.py
  导出名变化, ROM 布局无影响。make + SHA1 绿, fncheck Op_SysEffect OK。

## 2026-09-07 claude-opus-4-6: gUnk_0808B374 引用追踪与重定性 (gMenuEntPaletteFramesTail)

- **旧注释有误**: "被 byte_80E79AC 指针引用 (0x080E93CC→0x0808B410)" 不成立 —
  byte_80E79AC 实为 Huff 压缩图形 blob (由 0x087E990C[] 指针表 HuffUnComp→0x02020000→VRAM 消费,
  code.s 12726), 0x080E93CC 的 u32 是压缩流巧合值; 全 ROM 对齐 u32 指针与 code.s 字面池均 0 引用。
- **真实消费路径**: PaletteTransfer_Update (code_8005020.c:1517) `off=(帧号<<5)+2` 自基址
  gMenuEntityPaletteTable (0x0808A234) 寻址, 帧号无掩码; MenuEnt 描述符组 (gUnk_087EA138[], 274 组)
  中 255 个组的帧号序列落在 0x8A..0xAC → 触达 0x0808B374..0x0808B7D4
  (实证组 0x0808A1F2: palSlot=0x0A, 帧序 8A..98 停帧 = OBJ bank10 逐帧填色动画)。
- **定性**: 0x0808A234(124 项) + 0x0808B1B4(14 项) + 0x0808B374(35 项) = 同一张 173 项
  0x20 项距调色板动画帧库 (0x0808A234..0x0808B7D4); 内容: 红/白逐格填入动画帧 + 3 张独立小表。
- **改名**: `gUnk_0808B374` → `gMenuEntPaletteFramesTail` (src/include/data_805769C.h/
  data/data.s/scripts/data.json/raw_data bin 同步)。make + SHA1 绿。

## 2026-09-07 claude-opus-4-6: 三段调色板帧库合并为 gMenuEntPaletteFrames (5536 B)

- 用户指正: gMenuEntPaletteFramesTail 无直接 C 引用, 不该单独 INCBIN。
- 合并: gMenuEntityPaletteTable(0x0808A234, 3968B) + gFlashFxPaletteTable(0x0808B1B4, 448B)
  + gMenuEntPaletteFramesTail(0x0808B374, 1120B) → 单一 `gMenuEntPaletteFrames`
  (0x0808A234, 5536 B = 173 × 0x20, 帧号 0x00..0xAC), raw_data 三 bin 合并为一个。
- 消费点: PaletteTransfer_Update (code_8005020.c) base 改引新符号 (u16 extern, 原 u8);
  Op_SysEffect 闪光 DMA 保持 `gFlashFxPaletteTable` — 改为 linker.ld 同地址别名
  (0x0808B1B4 = 项 0x7C 视图), 字节零变化。
- scripts/data.json: 三项合并为 gMenuEntPaletteFrames@0x0808A234 size=5536
  (json.dump indent=1 与原格式零噪声往返已验证)。
- 验证: fncheck PaletteTransfer_Update/Op_SysEffect OK; code_8005020 TU 全绿; make + SHA1 绿。

## 2026-09-07 claude-opus-4-6: gSaveMenuUiPalettes (0x0808B7D4) 引用清单与 raw_data 同步

- **使用点** (→ OBJ PLTT bank14, 0x050001C0):
  - `sub_8011454` (0x08011454 存档菜单主控, 未匹配/INCLUDE_ASM) 三个分支各 DMA 首 32 字节
    (code.s 34822/35584/35812, 0x80000020; 第三处带 r8/sb 泄漏形状)。
  - `sub_80160F4` (0x080160F4, ✅C code_8010F10.c) 整表 64B DmaCopy16。
- **内容定性**: [0] 蓝系渐变, [1] 绿系渐变 (同底色 25ad/同尾 018c..6bff) — 修正旧注"红色与灰度"。
- **同步**: data.json `unk_808B7D4`→`gSaveMenuUiPalettes`, raw_data bin 改名,
  src INCBIN 路径更新, sub_80160F4 字面量→符号 (fncheck OK 132B)。
- **验证**: code_8010F10 TU 全部已匹配函数 fncheck 0 FAIL; make + SHA1 绿。

## 2026-09-07 claude-opus-4-6: 0x0808B814 混合区引用分析 (结论: 保持 INCBIN, 不转 C 结构)

- **gData_0808B814 无任何直接引用** (code.s 字面池 0 处, src 无符号引用); 代码全部经
  linker.ld 绝对符号按地址访问其内部项 (gUnk_087EA1A0[] 表项/byte_8091948/unk_80921F0 等),
  INCBIN 字节与 ROM 逐字节一致 → 换 C 结构无收益且有链接风险。
- **精灵动画模型组定量**: gUnk_087EA1A0 248 组中 106 组可解析 (AnimSlot_Parse 变长记录链:
  u16 count + {8B 头 [b0&1 帧数, f1=b2, f8=b6, f9=b7], tile f8*f9*f1*2}), 合计仅 12.9KB /
  区内 194KB; 组链在 0x0808B988 有 2B 尾随异常 (F7 01, 恰为前组 tile 帧号序列末项) 且 3 处小间隙
  → 非均匀变长链, 不适合 C 结构数组。
- **区内构成**: 动画组 12.9KB + 精灵调色板 gUnk_080B9DFC(3296B, 16 色 BGR555 ×103) +
  NPC 槽组/成长曲线/经验表/技能道具表/消息文本/名字串表/地图 BG 表/存档签名族 +
  0x080A1314..0x080B9C68 连续 118 项资源块 (头 10 C0 06 00, 被 0x086B0000..0x086F0000
  事件/脚本区 83 处 u32 指针引用) — 高度异构, data.json 已按引用点切项。
- **定论**: 该区维持单 INCBIN 兜底 + linker.ld 绝对符号; 仅当某子表被匹配代码以
  "基址+定长下标"访问时才值得拆出 C 类型化声明 (如 gUnk_080B9DFC[][32] 先例)。

## 2026-09-07 claude-opus-4-6: gData_0808B814 按指针表/引用结构切分为 18 段独立 INCBIN

- **切分依据** (不按 data.json): 扫描全 ROM 对齐 u32 指针 + code.s 字面池, 聚类出指针表
  (连续间隔≤8B 的 u32 组, n≥2): 0x087EA1A0(248 项动画组)/0x087E8430(1075 项对话资源,
  区内 119 项, 每块头 10 C0 06 00)/0x087EB1F4(71 项)/0x08007F00+0x08008ADC({src,VRAM} 对表)
  /0x08007F2C+0x08011938(地图 BG 表) — 以表锚点+语义边界切 18 段, 无缝覆盖
  0x0808B814..0x080BAF54 (194368B, 已验证相邻段首尾相接)。
- **18 段**: gAnimModelGroups(24884B)/byte_8091948/unk_80921F0/unk_8092248/unk_80923D8/
  unk_80933DC/byte_8093418/byte_80936A0/stru_8095028/stru_8095828/gMapBgTables(7368B)/
  gSaveMetaArea/gSaveMiscTables(17708B)/gMapGfxLz77Blocks(17188B)/gSaveMenuGfxLz77(1980B)/
  gDialogDataBlocks(101096B)/gUnk_080B9DFC(3296B)/gDataTail_080BAADC(1144B)。
- **同步**: src/data_805769C.c + include/data_805769C.h 逐段注释+extern; data/data.s 注释更新;
  data.json 的 unk_808B814+区内容项替换为 18 段 (覆盖校验 OK); gData_0808B814.bin 删除。
- **验证**: make + SHA1 绿 (ROM 逐字节一致); audit 漂移 0。

## 2026-09-07 opencode: gData_0808B814 18 段切分独立复核 (通过) + 引用图补充

- **复核结论**: 18 段在 data.json 精确铺满 0x0808B814..0x080BAF54 (0 间隙 0 重叠); 18 个 raw bin
  逐个存在; src/data_805769C.c (行 1711-1781) 与 include/data_805769C.h (行 55-107) 18 extern 对齐;
  data/data.s incbin 起点已移 0xBAF54。make + SHA1 绿 (745/1059); audit 改名漂移 0, status=1 745/745。
- **gUnk_087EA1A0 引用图精确定量** (248 项, 222 个不同目标): 228 项 → gAnimModelGroups 区
  (0x0808B814..0x08091948, 组链 u16 count + count×[8B头 + f8*f9*f1*2 tile] 走查基本吻合);
  **22 项 (下标 [103..124]) 越出主区**: [114,116..122]×8→0x080BAADC, [115]→0x080BAB1C,
  [103,105..113]×10→0x080BABE0, [104]→0x080BAE10 (即 gDataTail_080BAADC 全部 1144B 实为
  动画组数据, 现"尾部杂项"注释低估), [123]→0x0808EA5C, [124]→0x0808EA76 (区内, 但
  u16-count 模型解析不通 → 疑为 AnimSlot_Parse/ParseLoop 之外的第三种记录布局, 待
  MapScene_LoadEventAnimations 匹配后定性)。gMapSceneDescriptors.spriteAnimSetId 仅用 100/102 两值。
- **勘误**: 上节 "gMapBgTables(7368B)" 应为 10068B (0x08095A1C+10068=0x08098170, 与 data.json/src 注释一致)。

## 2026-09-07 gpnux: sub_804F64C 完全匹配 (284B / 47 指令, 脚本 VM Opcode 0x1A 摄像机平移)

- **函数定位**: `sub_804F64C` @ `0x0804F64C`, 属于 `src/code_804F0B8.c`。
  属于《Lunar Legend》Script VM 中的 Opcode 0x1A 处理函数（`Op_CameraPan`）。
- **语义与工作机理**:
  - 输入参数 `u32 *ptr` 为当前脚本执行指针 `gScriptCursor`（0x03000E6C）。
  - 指令格式: `1A [duration:1] [targetX:2] [targetY:2]` (总长度 6 字节)。
  - `gUnk_030047B4 = data[1];` (记录摄像机平移总步数/帧数)。
  - `gUnk_03004844 = 0;` (平移当前步数清零)。
  - 根据 `gCameraDrawMode` 的模式分支（case 2/5/8/default），从当前摄像机坐标（`gCameraPosX`/`gCameraPosY` 或 `gDrawCamX`/`gDrawCamY`）采样作为平移起点，分别写入 `gUnk_030047DC` (StartX) 与 `gUnk_0300461C` (StartY)。
  - 解析 `data[2..3]` 为平移目标 X (`gUnk_03004630`)，解析 `data[4..5]` 为平移目标 Y (`gUnk_03004830`)。
  - `*ptr += 6; return 1;` 推进脚本流。
  - 在后续帧中，`Viewport_UpdateScroll` 消费上述变量执行逐帧坐标插值运算，直至步数满后将 `gUnk_030047B4` 复位为 0；Opcode 0x1B (`Op_WaitCameraPan`) 则轮询等待其归零以解除脚本阻塞。
- **匹配突破点**:
  - 条件反转: `case 5` 与 `case 8` 中 `if (!gDrawCamEaseActive)` 生成 `bne` 目标形状（原 C 草稿为 `if (gDrawCamEaseActive)` 生成了 `beq`）。
  - 16位拼装操作符: `data[2] + (data[3] << 8)` 生成 `adds` 指令（若写 `|` 则生成 `orrs`）。
- **验证**: `python3 scripts/fncheck.py sub_804F64C` 输出 `OK (284 bytes @0x0804f64c, 0 池重定位已施加, 0 bl 槽忽略)`，逐字节完全一致。

## 2026-09-07 claude-opus-4-6: gUnk_087EA1A0 指针表改 C 数组 (248 项)

- **定性修正**: gUnk_087EA1A0 非纯动画模型表 — 222 唯一目标中 104 个 (0x0808B814..0x0808EA80)
  是 AnimSlot_Parse 变长组, 114 个 (0x0808EAC8..0x08091938) 是 16B 定长记录组 (格式待定性),
  同表混合两种精灵资源配置; gAnimModelGroups 注释已更正。
- **C 数组化**: src/data_087EA1A0.c 定义 `const u8 *const gUnk_087EA1A0[248]`,
  每项 `&gAnimModelGroups[N]` 符号表达式 (agbcc 生成 .word 符号+偏移, 链接后与 ROM 逐字节一致)。
- **布局改造**: 表位于 data1.s 兜底 (0x7E9818..0x800000) 中段 — 拆 data1a.s(0x7E9818..0x7EA1A0 兜底)
  / data1b.s(0x7EA580..0x800000 兜底), data1.s 删除; linker.ld 加锚
  `. = ORIGIN(rom)+0x7EA1A0; src/data_087EA1A0.o(.rodata)` (Makefile wildcard 自动收 data1a/b 与新 .c)。
- **消费函数**: gUnk_087EA1A0[setId] → sub_8008BA4 (code_8005020.c:1091, AnimSlot_Parse 逐组解析)
  / MapScene_LoadEventAnimations (0x08007350, INCLUDE_ASM) / Op_StartScriptAnim (code_804F0B8.c:1684)。
  fncheck sub_8008BA4/MapScene_LoadEventAnimations/LoadSpriteSheetGfx OK; TU 全绿。
- **未整表化**: gUnk_087E8430 (1075 项, 区外 956 项指向 0x080B9DFC..0x083823F8 对话大表) 与
  gUnk_087EB1F4 (97 项混区内外字符串) — 表内区外目标无符号, 暂留 data 兜底 + linker 绝对符号。
- **验证**: make + SHA1 绿 (进度 746/1059, +1 因 data1 拆分新函数收编? 实为计数刷新)。

## 2026-09-07 gpnux: Op_CameraPan 变量语义化 + Op_WaitCameraPan 全链改名 + ROM Data 规范划分与语义命名

- **函数重构与改名**:
  - `Op_CameraPan` (`src/code_804F0B8.c` @ 0x0804F64C):
    - 局部变量与参数全面语义化: `ptr -> pScriptCursor`, `data -> pBytecode`。
    - 全局变量应用语义符号: `gCameraPanDuration`, `gCameraPanStep`, `gCameraPanStartX`, `gCameraPanStartY`, `gCameraPanTargetX`, `gCameraPanTargetY`。
    - 逐字节复验 `python3 scripts/fncheck.py Op_CameraPan` -> `OK (284B)`。
  - `Op_WaitCameraSnap` -> `Op_WaitCameraPan` (`0x08052E80`):
    - 该函数为 Opcode 0x1B，检查 `gCameraPanDuration != 0`，不为 0 时返回 0 阻塞脚本 VM，为 0 时推进 PC 返回 1。原名 Snap 有误，实为平移等待。
    - 经 `scripts/rename_fn.sh` 完成全链重命名 (`ll.cfg`, `code.s`, `include/code_0.h`, `functions.tsv`, `src/code_804F0B8.c`)。
    - 函数内更新为 `gCameraPanDuration`，`python3 scripts/fncheck.py Op_WaitCameraPan` -> `OK (28B)`。
- **ROM Data 划分与语义命名落地**:
  - 对 18 段切分数据及 `Op_SysEffect` / `Script VM` 关联资源建立清晰语义名并在 `include/data_805769C.h`, `src/data_805769C.c`, `linker.ld`, `scripts/data.json` 同步落地，提供双向兼容宏:
    1. `0x08091948` (2216 B): `byte_8091948` -> `gMapNpcSlotGroups` (地图 NPC 生成槽组表，`Sprites_LoadMapNPCs` 消费)。
    2. `0x080921F0` (88 B): `unk_80921F0` -> `gClassStatCurveTable` (职业x8维属性成长曲线索引表)。
    3. `0x08092248` (400 B): `unk_8092248` -> `gLevelUpExpTable` (100 级升级经验阈值表)。
    4. `0x080923D8` (4100 B): `unk_80923D8` -> `gStatGrowthCurveTables` (41 组成长曲线分段表)。
    5. `0x080933DC` (60 B): `unk_80933DC` -> `gStatGrowthTail` (成长表尾部)。
    6. `0x08093418` (648 B): `byte_8093418` -> `gSkillLearnTable` (技能/法术习得等级与组别定义表)。
    7. `0x080936A0` (6536 B): `byte_80936A0` -> `gMsgPoolMain` (主游戏消息文本池，0xFF 分隔)。
    8. `0x08095028` (2048 B): `stru_8095028` -> `gItemNames` (道具名称字符串表，256 项 x 8 字符)。
    9. `0x08095828` (500 B): `stru_8095828` -> `gCharacterNames` (角色名称字符串表，8 字符/项)。
    10. `0x080B9DFC` (3296 B): `gUnk_080B9DFC` -> `gSpriteObjPalettes` (103 项精灵 OBJ 调色板)。
    11. `0x0808B1B4` (32 B): `gFlashFxPaletteTable` (白闪 16 色调色板，`Op_SysEffect` DMA 到 OBJ bank 4)。
    12. `0x08289B6E` (320 B / 10 步): `gObjPalFadeInSteps` (OBJ 调色板 10 帧渐显步进表，`Op_SysEffect` 逐帧 DMA 到 OBJ bank 0..9)。
    13. `0x083936A8` (32 B): `gObjPalFadeInFinal` (渐显末帧 16 色调色板，`Op_SysEffect` DMA 到 OBJ bank 10)。
    14. `0x0862D434` (320 B / 80 指针): `gScriptOpcodeHandlers` (脚本 VM 80 项 Opcode 分发处理函数表)。
    15. `0x0862D574` (740 B): `gTileGfxSets` (`Op_LoadTileGfx` 调用的瓦片图形组配置表)。
- **全量终验**:
  - `make` 编译成功，`sha1sum -c ll.sha1` 通过 (ROM 逐字节一致)。
  - `python3 scripts/audit.py`: 746/746 status=1 函数全量字节核验通过，改名漂移 0。

## 2026-09-07 语义大更正: sub_80525E8 实为脚本集装载器 ScriptSet_Load — "LZ_BGM" 旧注证伪 (agent opencode-scriptset)

### 背景
用户质疑 NewGame_Init 分析报告中 `sub_80525E8 = LZ_BGM 装载` 的定性 (源自 EXPERIENCE #1155 与
progress 2026-09-04/05 两段挂起记录)。逐指令重读反汇编 + 全链追证据后, 证明旧注错误, 本函数是
**脚本引擎的脚本集 (script set) LZ 资源装载器**, 与 M4A 音乐毫无关系。

### 反汇编证据链 (asm/nonmatchings/ScriptSet_Load.s, 85 行)
```
gUnk_03000E68 = id                          ; u8 → 0x03000E68 (记挂集号)
r3 = gUnk_087ED6D4[id]                      ; ROM 指针表 (0x0862D8A4.., 363 项 LZ 块指针)
if (REG_DISPSTAT & 0x80)                    ; 正在 VBlank
    LZ_InitContext(0x02016000, r3); LZ_UncompressChunk()   ; 同步解压
else
    LZ_InitContext(0x02016000, r3); gUnk_03000E70 |= 0x200 ; 挂后台, VBlank 泵逐帧解压
switch (mode):
    1/其它: gScriptCursor = 0x02016200                    ; VM PC = 脚本区基址
    2:      gUnk_03000E69 = entry; gUnk_03000E70 |= 0x400  ; 解压完跳入口表标志
            gScriptCursor = 0x02016200 + gUnk_02016000[entry]
```
- 解压目标 0x02016000 = linker.ld EWRAM 脚本区 (entryTbl + 代码区 0x02016200)。
- `sub_805008C` (ScriptPump_ServiceFrame, VBlank 调用) 的 0x200/0x400 分支消费上述标志:
  `if (E70&0x200 && LZ_UncompressChunk()==0) { if (E70&0x400) gScriptCursor = base+entryTbl[E69]; }`。
- 脚本 opcode 处理器 Op_ScriptReturn/Op_ScriptStop 退场时调 `Script_SetEnvSet(gUnk_03000E68)`
  (原名 Bgm_Request, 仅 12B: `strb r0,[0x03004850]; bx lr`), 把装载时传入的集号写回
  gCurrentSongId(0x03004850) —— 该变量**全部 3 个消费者** (Scene_Reload:2867/
  Scene_RestoreAfterBattle:3143/MapScene_Load:13858) 都是 `ScriptSet_Load(gCurrentSongId,0,1)`
  重装环境脚本集。闭环: 它是"环境脚本集号", 不是"当前曲目号"。
- ROM 侧验证: 0x087ED6D4 表 363 项全部 ROM 指针; [140] 巨块解压后 0x78010B (~492KB) 显然是
  脚本资源池, 非 M4A 歌曲表。

### 初装链条 (NewGame_Init → 开场剧情)
`ScriptSet_Load(1, 0, 1)` 装脚本集 1 (0x0862E2A0, 0x73F1B) → 16×VBlankWaitExit_PumpSound 泵完解压
→ `ScriptPump_JumpToEntry(1, 2)`: gScriptCursor = 0x02016200 + entryTbl[1] → 脚本泵逐 opcode 跑开场。

### gUnk_03004634 同步定性 (gMapScriptSetId)
- MapScene_Load:604: `gUnk_03004634 = gMapSceneDescriptors[mapId].byte_0x0B` (字段注释 sceneFlag 有误);
- MapScene_Load:1300-1314: `if (gMainGameState==2 && !(脚本运行)) { ScriptSet_Load(*03004634,0,1); 03004850 = *03004634; }`
  → 进图后按它装载环境脚本集。NewGame_Init 里 `gUnk_03004634 = 1` 即"开场地图的环境脚本集 = 集 1"。

### 落地改动 (全部零字节: 改名/注释, fncheck+SHA1+audit 全绿 746/746)
1. 函数改名 (rename_fn.sh 全链, 不动签名):
   - Bgm_Request → **Script_SetEnvSet** (0x08008dcc)
   - sub_80526A0 → **ScriptPump_JumpToEntry** (0x080526a0)
   - sub_805008C → **ScriptPump_ServiceFrame** (0x0805008c)
   - sub_80525E8 → **ScriptSet_Load** (0x080525e8, --force 越过 claude-80525E8 挂起锁, 见 INCIDENTS)
2. RAM 符号改名 (iwram.h + linker.ld, asm 硬码地址不受影响):
   - gCurrentSongId → **gEnvScriptSetId** (0x03004850)
   - gUnk_03000E68 → **gScriptReturnSetId** (0x03000E68)
   - gUnk_03000E69 → **gScriptPendingEntry** (0x03000E69)
   - gUnk_03004634 → **gMapScriptSetId** (0x03004634)
3. 结构字段: MapSceneDescriptor.sceneFlag(+0x0B) → **scriptSetId**。
4. functions.tsv: 0x080525e8 note 前缀加证伪标记; 0x08008dcc 补 ✅ note。
5. EXPERIENCE: #1155 段首追加证伪横幅 (详见该文件), 新增 #186 (脚本装载三件套)。

### 经验教训
- "Bgm_Request 只写 gCurrentSongId" 不构成 "它是 BGM" 的证据 —— **必须看变量的消费者**而非赋值者;
  变量名/旧注 (gCurrentSongId/LZ_BGM) 都是前人猜名, 追到唯一消费点 (3 处全是 ScriptSet_Load) 才闭环。
- EXPERIENCE #1155 是"只看调用形状"的表面结论, 本次重写; 姊妹条目 sub_80513A0 的定性也需重审。

## 2026-09-07 antigravity: sub_8011454 (4314B 标题与存档界面大状态机实装入库)

### 任务与成果
- **函数**: `sub_8011454` (0x08011454, 1849 行汇编 / 4314 字节, 213 个字面池重定位, 111 个 BL 调用槽)。
- **位置**: `src/save_menu.c`。
- **状态**: 成功完成真 C 实装替换 `INCLUDE_ASM`，`fncheck.py sub_8011454` 100% 逐字节匹配。
- **全局终验**: `make` 0 报错，`sha1sum -c ll.sha1` 通过，匹配进度上升至 748/1059 (70.6%)。全库 `audit.py` status=1 字节核验 748/748 全绿。

### 核心攻坚与编译器行为总结
1. **Signed Switch 比较指令对齐**:
   - 原汇编状态机入口为 `cmp r0, #0xE; bgt ...`（有符号比较）；
   - 若状态变量 `gUnk_030025A8` 声明为 `u32`，GCC2.95 编译 switch 时会生成无符号比较 `bhi`；
   - 显式强转为 `switch ((s32)gUnk_030025A8)` 成功生成目标的 `bgt` 指令。
2. **DmaCopy16 宏大小与控制字反推**:
   - `DmaCopy16(channel, src, dest, size)` 在 GBA 宏展开中控制字为 `0x80000000 | (size / 2)`；
   - 原草稿注释中第 597/602 行的 `0x40` 与 `0x20` 颠倒，且 604/612 行将 0x40 误写为 0x20（汇编复用了 r6 中的控制字 `0x80000020`，对应半字数 0x20 = 0x40 字节）；修正后消除 DMA 控制字差异。
3. **Load-Delay Slot 与结构体字段访问优化**:
   - 原始草稿使用 `((u8 *)&gUnk_03001CB0)[0]` 进行字节强转，触发 GCC 别名分析（alias analysis）保守化，无法将 `movs r3, #0` 调度下沉至 load-delay 槽；
   - 替换为具名结构体 `gUnk_03001CB0.field_0` 访问后，指令调度槽与寄存器分配 100% 完美命中。
4. **子函数原型与参数/返回值截断一致性**:
   - `MapScene_Load` 原型在 `include/code_0.h` 中原声明为 `void MapScene_Load(u8);`，导致实参 `0x82 + ...` 被强制插入 `lsls/lsrs #24` 截断指令；修复为 K&R 风格 `void MapScene_Load();` 后消除截断。
   - `Save_Fsm` 声明返回值类型为 `s32`，但调用处目标汇编保留了字节截断 `lsls r0, r0, #0x18`；显式强转为 `(u8)Save_Fsm(...)` 完美复现截断。

### 全链改名与可读性重构
- **函数更名**: 通过 `scripts/rename_fn.sh sub_8011454 SaveMenu_ProcessFrame` 完成全链重命名。
- **状态机枚举化**:
  - `TitleScenePhase`: 39 个状态（`TITLE_PHASE_INIT` 至 `TITLE_PHASE_CARD_EXCHANGE_FINISH_FADE`）全量替换裸数字 case。
  - `CardExchangeStatus`: 通信子状态机（`CARD_EXCHANGE_IDLE`, `SENDING`, `TIMEOUT`, `FAILED`, `SUCCESS`, `CONNECTING`）。
- **全局变量语义化**:
  - `gUnk_03004D44` -> `gSaveFsmState`
  - `gUnk_03004D50` -> `gActiveSaveSlot`
  - `gUnk_03004DD0` -> `gSaveSramBlock`
  - `gUnk_03002608` -> `gTitleFadeTimer`
  - `gUnk_030025A8` -> `gCardExchangeStatus`
  - `gUnk_03000230` -> `gCardAlbumCursor`
  - `gUnk_03000232` -> `gCardAlbumPage`
  - `gUnk_03000233` -> `gCardCursorY`
  - `gUnk_03000234` -> `gTitleFadeStep`
  - `gUnk_03000236` -> `gCardRecvId`
  - `gUnk_03000238` -> `gCardSendId`
  - `gUnk_03001AD0` -> `gSioRecvPacket`
  - `gUnk_03001CB0` -> `gSioSendPacket`
- **终验**: `make` 编译成功，`sha1sum -c ll.sha1` 通过，全库 `audit.py` 748/748 通过，零改名漂移。

### 模块及函数命名规整 (2026-09-07)
- 用户指定模块业务更名为 `title_menu`（原文件 `src/save_menu.c` 实际承载标题画面、卡片图鉴、联机卡片交换与存档槽管理，定位为游戏标题总控与主界面）：
  1. **函数改名**:
     - `SaveMenu_ProcessFrame` (原 `sub_8011454`) -> `TitleMenu_ProcessFrame`
     - `Task_SaveMenuFrame` (原 `sub_80031E4`) -> `Task_TitleMenuFrame`
  2. **源文件与链接脚本重命名**:
     - `src/save_menu.c` -> `src/title_menu.c`
     - `linker.ld`: `src/save_menu.o(.text);` -> `src/title_menu.o(.text);`
  3. **函数清单同步**:
     - `functions.tsv`: 45 个属于原 `save_menu` 的函数 module 字段全部规范迁移为 `title_menu`。
  4. **终验**: `make` 0 报错，`sha1sum -c ll.sha1` 绿，`audit.py` 748/748 全绿，零改名漂移。

### 模块名 menu.c、公共头文件 menu.h 建立与宏消除重构 (2026-09-07)
- 用户指定将模块更名为 `menu.c`，创建独立头文件 `include/menu.h`，彻底消灭 `#define gUnk_03001CB0 gSioSendPacket` 这类宏桥接：
  1. **头文件构建 (`include/menu.h`)**:
     - 定义 `SioPacket` 数据包结构体；
     - 声明 `gSioSendPacket` 与 `gSioRecvPacket` 全局通讯变量；
     - 规范收拢 `TitleScenePhase` 与 `CardExchangeStatus` 状态枚举；
     - 暴露 `TitleMenu_ProcessFrame` 等核心接口声明。
  2. **消除宏 Hack 并直接全局替换**:
     - `src/menu.c`: 彻底移除开头的 `#define gUnk_03001CB0 ...` / `#undef` / `#define gUnk_03001CB0 gSioSendPacket` 桥接，直接使用真实具名变量与 `menu.h`；
     - `src/engine_core.c`: 将 `Sio_SetXferCtx(&gUnk_03001AD0, &gUnk_03001CB0, 0x10, 0);` 直接替换为 `Sio_SetXferCtx((u32 *)&gSioRecvPacket, (u32 *)&gSioSendPacket, 0x10, 0);`；
     - `include/iwram.h`: 包含 `menu.h`，移除重复枚举定义与旧 `gUnk_03001AD0`/`gUnk_03001CB0` 冗余声明；
     - `linker.ld`: 统一为 `src/menu.o(.text);`，并保留 `gSioRecvPacket`/`gSioSendPacket` 符号别名。
  3. **模块同步**:
     - `src/title_menu.c` -> `src/menu.c`；
     - `functions.tsv`: 45 个函数 module 列统一规范为 `menu`。
  4. **终验**: `make` 0 报错，`sha1sum -c ll.sha1` 绿，`audit.py` 748/748 全绿，零改名漂移。


## sub_80154E8 (0x080154E8, menu.c, 2026-09-08 codex-menu)
2 行文本绘制: `mode>>1` 选表 `D_87EB2A8[+3]/[+5]`, `mode&1` 选逐字绘制/空白计数路径, 高亮列 `(*0x03000187-4)==i ? 13 : 11`。fncheck OK 368B, 全 ROM 绿。

- 表项结构: `[x, y, 字符..., 0xFF]`; dest = buf + y<<1 + x<<6 (先 y 后 x, gcc2 先算的进 r1)。
- 空白路径: 先数非 0xFF 字符数, 再循环 `Text_PutGlyph(dest++, 0, 11)`; 逐字路径 `Text_PutGlyph(dest++, *str++, col)` — 两处都必须后置自增实参形态, 否则 adds r5,#2 位置错。
- ⚠ **gWindowBgBuf 引用形态** (新经验候选 118): 项目头里是 `extern u8 gWindowBgBuf[]`, 惯性写成 `(u16*)((u32)gWindowBgBuf + ...)` 没问题; 但我在 permuter base.c 用 `#define gWindowBgBuf ((u16*)0x02005800)` 字面量时 gcc2 给出 368B, 而 src 侧若写 `0x02005800 + ...` 字面量反而 360B — **符号引用 (extern u8[]) 与目标一致**。教训: base.c 的 extern/宏必须与项目头完全同型 (经验 96 的延伸, 类型不只是"能编过"而是决定分配)。
- `idx` 必须 u32 (i+1 存 r7 不做零扩展); color/count 复用一个变量 (同为 r6)。
- bytecmp 判定: 16 字节假差异 = 4 个 bl 槽 (mine.o 的 R_ARM_THM_CALL 未消解), 池 ABS32 已由 abs.ld 施加; skip = mine.o 重定位表的 bl+ABS32 槽后 **0 real diffs**。

## sub_8013934 (0x08013934, menu.c, ⏸ 2026-09-08 codex-menu)
4 行存档槽文字绘制: `i=0..3`, 高亮列 `(i == *(u8*)0x03000187) ? 13 : 11`。
`i<=1` 时按 `*(u16*)0x03004D48 & 1` 分: 存档存在 → 画锁定态文字 `gUnk_087EB278[i]` + 统计框清屏
(填 0xB001, 3 项/行 × 2 行, 行距 0x40) + `sub_800EAE4(0x02005936/0x020059F6, *(u8*)0x03000232 /*u16 0x03000230*/, col)`;
不存在 → 画 `gUnk_087EB278[i+10]`。`i>1` → 画 `gUnk_087EB278[i]`。结构已 100% 对齐 (分支/表/常量/调用全部命中)。

- 剩 24B = fill 循环的 cols/rows u16 shifted-home dance:
  目标 `[movs r0,#3; lsls r6,r0,#0x10; lsrs r0,r6,#0x10; mov ip,r0]` + `[movs r0,#0x80; lsls r0,#0xa; lsrs r7,r0,#0x10]`
  + 内层 `lsrs r1,r6,#0x10` 重载 + val `[ldr r1,=0xB001; adds r5,r1,#0]` (每块 ~20B ×2)。
  mine 为 `[movs r4,#3] [movs r6,#2] [ldr r5,=0xB001]` — 同等 u16 语义, agbcc 不出 dance。
  与 **sub_8049AD8 同族** (global-alloc 决策, 非 C 写法层)。
- 已穷举: 声明序 ×6, 类型 {u16/u32/int/s32} ×8, register, (u16) 强转, static 函数+内联重构
  (agbcc 不内联), 统一调用尾部 (`dst/arg` 变量使 bl 合并 ✓ 已实现), permuter ×4 轮。
- permuter 陷阱实录: ① `if (stat)` 未初始化读 (5675) — 偷改语义; ②
  `((0x02005800 & 0xFF) & 0xFF) & 0xFF` (6115) — **把 0x02005800 折叠成 0**, fndiff 分数假高 (8285),
  语义全错, 人工清洗后退回 12325。
- 最佳候选: `permuter/sub_8013934/base.c` (12325 分, 448B vs 472B)。
  下一步: agbcc global-alloc 转储分析 cols/rows 的 allocno 优先级 (EXPERIENCE §诊断)。

## InvUi_Main (0x08015C18, menu.c, ⏸ 2026-09-08 codex-menu)
道具菜单主绘制: ① 清屏 22×8 halfwords (0xB001) @0x020059CC 行距 0x40; ② 逐物品 (item=*gInvCursor2 起, 每迭代 +1, row 0..3):
count=gUnk_03004980[item]; count==0 → 仅记光标; 否则 icon=sub_800AB18(item)*10, 高亮列 (row==gMenuCursorStack[gMenuCursorGrp])?13:11,
icon==0 → col=12 + sub_8015E1C(0x15,row*2+7,12,0x08098748); icon!=0 → sub_800EAE4 画 icon @0x02005830+(row*2+7)*64
+ 6 tile 边框 (0x1B8..0x1BD + col<<12, 地址 0x02005832/0x02005872/5834/5874/5836/5876);
名字 ≤8 字符 @0x0200580C+(row*2+7)*64 (0x08095028+item*8, 0 终止); 数量 @0x020059DE+pos2*64 (col=12);
光标时 gUnk_03000229=item, gUnk_0300022C=icon (u32)。已登记 gUnk_03000229/0x0300022C (linker.ld+iwram.h)。
- 剩 ~130B: cols/rows/val 的 u16 shifted-home (r8=0xB0<<13 合成 + lsrs 重载; ldr r2,=0xB001; adds r5,r2,#0 纯拷贝) + rowi/count/col home 互换 — LRA live-range-split 家族, C 层不可达。
- 已试: 声明序 ×3、类型、register、tile 写址重构 (数组索引→独立地址计算 ✓ 该部分已对齐)、统一调用、permuter ×4 (最好 10350 但 fndiff 13561 反高)。
- 候选: permuter/InvUi_Main/base.c (12840 分)。

## sub_8010F10 (0x08010F10, menu.c, ⏸ 2026-09-08 codex-menu)
4 格选择框绘制 (菜单通用框): 框角/边框 tile (0xB190 左上, 0xB001×20 边, 0x1AE/0x1AF 角, attr<<12 调色板),
`tmp = idx + 0xA3` 3 分支 (idx 0..3 时两分支恒死, 但结构必须保留: `if (tmp != 0) { if ((tmp&0xFF)==0xFE) ... else ... } else ...`
— **嵌套结构使 gcc2 产生与目标一致的块布局**), 内容分数字串/图标名字两路径。
- 关键发现: 分支块布局由 **if/else 嵌套层级**决定 — 平铺 if/else if/else 产生 [A][B][C], 嵌套产生 [B][C][A] (与目标一致)。
- dest 表达式必须是字节级 u32 算术 `(u16*)(0x02005800 + (y<<6) + (x<<1))`, 不可写 `gWindowBgBuf + .../2` (会生成不同指令序)。
- 剩 ~220B: 寄存器 home 家族 (idx/y/attr home 互换 + pal shifted-home sb + val adds 纯拷贝) — LRA 家族同 sub_8013934/InvUi_Main。
- 候选: permuter/sub_8010F10/base.c (19065 分)。已登记 gUnk_03000204/0x03000208/0x03000210/0x080981A5/0x0830FC04。

## sub_8050434 (0x08050434, script_vm) — ⏸ 2026-09-08 claude-8050434 (456B/488B 差, 语义全解)

TileDma 请求收集器 (对话字模装载第一步)。两调用者: Op_LoadTileGfx (id=0x6F1E) 与
sub_804AC60 战斗侧 (id=0x4F1E)。**语义全解**:

1. **头部分派** (按 id 高位): `(id&0xF00)==0` 时 count=(u8)(id&0x6000?id:0) (条数上限);
   `(id&0x2000)==0` 时复位 `gUnk_03000F24=0` (注意: 与 count 判定是**两个独立 if**, 0x800 位不复位)。
2. **第一循环** (扫 entry 的 u16 tile 表): `v = tile & 0xFF00` (只比高字节!)。
   v>0x6E6: 0x800→(id==v 时 flags=0, 终止); 0xF00→终止; 其他→collect。
   v∈[0x6E3,0x6E6]→next (跳过, 特殊保留号); v<0x6E3→collect。
   collect: v16>0xDF 时去重搜索 gUnk_03000EE8[0..F24) 后追加, F24++。
   尾部: count!=0 时 counter++ 到限即停; F24>0x1D 硬停 (EE8 容量 30)。
3. **第二循环** (DMA 炸开): 每 tile 两块 32B 拷到 0x0203DE00:
   q=t-0xE0 (负则 t-0xC1), row=(修正q)>>5, **col=原始q-(row<<5)** (用原始 q!),
   src1 = tbl[1] + (row*64+col)*32; src2 = src1 + 0x400 (即 ((row*2+1)*32+col)*32);
   `DmaCopy16(3,...,32)` + `DmaWait(3)`, dest 每块 +32。tbl=gUnk_087ED904 (=gScriptSetTable 尾部视图,
   [1]=0x0861CC34 = Set141 字模 LZ 块, BgTiles_LoadSet 同款)。

**候选**: permuter/sub_8050434/base.c (语义精确版), bytecmp **456B 差/488B 总量** (头 232B 一致)。
permuter 7 轮 promote 研磨 8808→3275, 但**所有 score<4200 的变体都有同一语义错误**:
把 `v>=0x6E3 → next` 折叠成落穿 collect (`if (v<0x6E3) goto collect;` 后 collect 标签紧贴,
v∈[0x6E3,0x6E6] 误入搜索)。汇编里目标 0x84 `b.n 0x100` 是不可达死代码 (0x6E4 检查被
v<0x6E3 前提吸收), permuter 抓住这一点重排了 CFG 但改变了 0x6E5/0x6E6 的行为 —
**score 假高的活案例** (文本近似 ≠ 语义等价)。

**剩 456B 差异构成** (global-alloc home 布局, 同 804AB40 深层问题):
① 头部 `(id&0xF00)` 常量物化多一次 `adds r1,r2,#0` 拷贝 + id 拷贝用 r0/r3 两次 (我: r4 复用);
② count 槽位 [sp,#0] vs 我的 [sp,#4] (目标 count/n 两槽, sp+#8);
③ n=gUnk_03000F24 目标溢出到 [sp,#4], 我的进 r7;
④ 搜索循环/row-col 序列寄存器选择级偏差。
已穷尽: n 声明序/初始化、mask 变量、tbl 缓存与否、do-while 守卫、n=i*2 提取 (4120 招式)。
**接手建议**: 语义正确前提下攻 ②③ 栈槽位 — 让 GCC2 把 count 和 n 同时溢出 (增加第一循环
寄存器压力或把 counter 声明成 u32); 或查 EXPERIENCE 174 幽灵栈帧手法反向构造。

## sub_8012530 (0x08012530, menu.c, ⏸ 2026-09-08 codex-menu)
角色图块装载: `arg0*4` 查 `gUnk_0809888B[]` 表 (kind/animId lo+hi/flags); switch kind 选 tile 表+palette
(0,3: 0x08097DB0+0x8000; 1: 0x08097EF0+0x9000; 2: 0x08098030+0xA000); 双 16×16 循环写 0x02005058
(k>9 → 0; 否则 `*tiles & 0xC3F | palette | 0x200`; flags&0x80 路径: 行 13..14 span=1, 跳 1 tile 画 7 个)。
之后 `sub_800EAE4(0x020053A8 / 0x020053B4, *(u16*)0x03000238 & 0xF, ...)`; `gUnk_03004DC0 = 1`;
`Sprite_FreeChain(&gSpriteNodePool[gActors[0].statusFlags*3])`; `Chara_InitEffect(0)`;
kind 1/2 → `CutsceneAnim_Load(animId,0,5)` + `field_6 = gCameraPosX+0x88` + `field_8 = gCameraPosY+0x4C/0x58`;
else `CutsceneAnim_PlayFrame(animId, 0)`; 尾部 `gActors[0].field_13 = 0`。
- 已登记: gUnk_08097DB0/0x08097EF0/0x08098030/0x0809888B/0x02005058/0x020053A8/0x020053B4/0x03004DC0。
- 剩 ~150B: kind/flags 存储位置 ([sp+4] vs sl) + 双循环 home — LRA 家族同前。
- 候选: permuter/sub_8012530/base.c (15755 分)。

## sub_801417C (0x0801417C, menu.c, ⏸ 2026-09-08 codex-menu)
存档菜单状态机 (20+ case)。关键发现: **gcc2 的 switch 生成块序 = 源码 case 声明序**,
重排 case 声明序为目标的块地址序 (F5, 28, 1, 8, 2/4/6, 7, 5, 1E, 1F, 20, 21/22, 23, F0, F1-F4/F6-F8, F9, FA)
后 score 从 25975 降到 19765。剩余: case 7/2/4/6/0xF1-F8 共享 `strb r0,[r1]` 写
(r1 = &gUnk_03004D40 在 prologue 后立即建立, case 7 直接 `b` 进 2/4/6 的尾部) — C 层不可达。
- 已登记无新符号 (全部已有)。候选: permuter/sub_801417C/base.c (9010 分)。

## sub_8013C00 (0x08013C00, menu.c, ⏸ 2026-09-08 codex-menu)
存档菜单顶部绘制: 标题行 (gUnk_087EB278[4]) + 2 行选项 (高亮 = i == gMenuCursorStack[gMenuCursorGrp]) +
存档时间行 (SaveTimer_Get(gMenuCursorSel), col = 12/13) + 2×6 填 0xB001 框 @0x02005AC8 +
sub_800EAE4(0x02005ACC/0x02005AD2) + Text_TileAt(8,12)=0xB26D + 底部 2 行 (gMenuCursorSel-3 ≤ 1 →
高亮文字; 否则计数空白填充) + switch (gMenuCursorSel 0..4) — **gcc2 生成 jump table** (case 0..4 连续)。
- x/y 表达式求值序: `(val << 1) + (tmp << 6)` (x 项在前) 与目标一致。
- 剩 ~180B: cols shifted-home (r8 = 6<<16) + fill 循环 do-while 形态 + home 互换 — LRA 家族。
- 候选: permuter/sub_8013C00/output-12745-1/source.c (12745 分)。

## sub_80146A8 (0x080146A8, menu.c, ⏸ 2026-09-08 codex-menu)
菜单主状态机: `gUnk_03000048.field_0 |= 1; field_4 = field_8; field_6 = field_A` (须用结构体访问 —
裸地址强转会生成逐次重载基址的错误形状); switch (gMenuCursorGrp) 3 主路径 (case 1 短消息;
case 2: 3 张字符串表 `[x,y,pal,字符...,0xFF]` + fill + 边框 0xB000+0x1B8.. + sub_8015658;
case 3: 表 + InvUi_Main + 计数分支)。字符串表绘制 helper 必须内联展开 4 份 (agbcc 不内联 static)。
- 剩 ~140B: 高位寄存器 home (目标用 sl/r8 存 fill 循环变量) + LRA 家族。
- 候选: permuter/sub_80146A8/base.c (9745 分, 含 permuter 合法 new_var 冗余拷贝手法)。

## sub_80501B8 (0x080501B8, script_vm, ⏸ 2026-09-08 gpnux — 715 分候选, 语义全解)

**语义** (339 行, TextTileDma_QueueScan / FlushTileDma 的生产者):
- 前半: 扫描对话框字节流 (arg0+scan, 步长 arg2∈{8,0x10}):
  - 头部: 若 `(arg1 & 0xF00)==0` 则 `dups=(u8)arg1` (arg1=特殊调用模式+重复计数);
    若 `!(arg1 & 0x2000)` 清 `gUnk_03000F24=0` (0x2000=不清队列继续追加)。
  - 循环: `hi = p[1]<<8`; switch 比较树(0x100 步进):
    - `0x800 && arg1==0x800` → `more=0` (终止标记+模式匹配);
    - `0xF00` → `more=0`; `0x100..0xE00 (除 0x800/0xF00)` → 跳过;
    - default (含 `0x10` 标志的半字): `cur = ((p[1]<<8) & ~0x1000) | p[0]`,
      `>0xDF` 且不在 `gUnk_03000EE8[0..F24)` → 追加 (去重)。~0x1000 池值=0xFFFFEFFF。
    - `dups` 计满提前退出。
- 后半: 对队列每 id: `delta=id-0xE0`, 负则 `id-0xC1`; `row=delta>>5(asrs)`, `rem=delta-(row<<5)`;
  两次 DmaCopy16(3, gUnk_087ED904[1]+ofs, 0x0203DE00+64k, 0x20) — 从字库 blob 第 1 项
  (bank=row 的第 rem 字形 + odd-row 0x20 偏移) 各拷 0x20 字节, 控制字 `0x80000010`。
  尾: `gUnk_03000F26 = gUnk_03000F24 - 1` (IWRAM 0x03000F26, 未登记符号)。
- 调用方 6 处全传 `arg1=0x800`(r1=0x80<<4), `arg2=0x10`; 返回值未用。

**DMA 宏判定**: `str r0,[r3,#0/#4/#8]` 共基址 + 两次 `ldr r0,[r3,#8]` = `DmaSetUnchecked` 展开
(空读 `dmaRegs[2]`) + `DmaWait` (`ands #0x80000000` 循环, 经验55/FlushTileDma 同族)。
`0x80000010` = `(DMA_ENABLE|DMA_START_NOW|DMA_16BIT|DMA_SRC_INC|DMA_DEST_INC)<<16 | 0x20/2`
→ 必须用 **DmaCopy16(...,0x20)** 不能 DmaCopy32 (32 位版控制字是 0x80000008)。

**候选状态**: 715 分 (permuter), 距 0 卡 ~40 形状块, 全部集中在:
1. 头部 0xF00 掩码测试: 目标是"双拷贝"形状 `movs r2,#0xf0; lsls r2,#4; adds r1,r2,#0; adds r0,r5,#0; ands r0,r1`;
   候选是融合 `movs r0,#0xf0; lsls r0,#4; ands r0,r5`。需要掩码变量保活制造伪寄存器生死边界 (经验 87 同款),
   试过: bank/c/m1 变量+类型 u16/s16/s32/vu16、AND 结果存变量、if 反转+else、共享 0x2000 掩码变量、声明重排 — 全部融合或更差。
2. `scan`/队列指针 r6↔r7 级联翻转 (纯 home 争议, 经验 88: global-alloc 决定, 需 refs 翻盘)。
3. case 体寄存器翻转若干。

**工大于 17 万迭代 permuter (-j1, 8 轮固化重跑)** 未突破。次要目标 (score<1000) 达成。
资产: `permuter/sub_80501B8/` (target.o+compile.sh+settings.toml+base.c=715 候选);
备份 `.scratch/gpnux/80501B8/` (base_715.c + fndiff1..7.txt)。
后续人建议: ① 挖 agbcc `-dl` gccdump 看 0xF00 qty 的 refs/life, 验证经验 88 翻盘条件;
② 试把 `dups`/`more`/`j` 与 0xF00 掩码变量做生命周期交织 (经验 87);
③ 若仍无解, 考虑是否调用方语义允许 switch 外提 (源码重构空间未穷尽)。

## sub_8013870 (0x08013870, menu.c, ✅ 2026-09-08 antigravity)
标题菜单背景清屏与文字绘制：196 字节，完全逐字节匹配（通过 `fncheck.py sub_8013870`，`make` 及 `sha1sum -c ll.sha1` 绿）。

- **业务逻辑**:
  1. `ClearBuffer((u16 *)0x02005800, 30, 20);` 填 0xB001 清除菜单窗口背景；
  2. 遍历 `gTitleMenuDesc` (0x08098622, 格式: `x, y, palette, chars..., 0xFF, ..., 0xFF`，包含 0xFE 转义双字节字形)，逐字调用 `Text_PutGlyph` 渲染标题菜单选项；
  3. 尾部调用 `Text_WriteChars(Text_TileAt(0xC, 7), (u8 *)0x08098858, 0xB);`。
- **攻克历程与关键技术突破 (经验 191)**:
  - 此函数曾困扰 opencode 等多代 agent，长期挂起在 53B 平台期。
  - **核心症结**: 目标汇编 prologue 中把描述表基址进 r8 (`ldr r0, =0x08098622; mov r8, r0`)，清屏循环结束后以 `mov r4, r8` 拷入 r4 供字符串循环迭代。
  - 若在 C 中直接使用立即数字面量 `(const u8 *)0x08098622`，GCC2 的 local-alloc `update_equiv_regs` 识别到其为常量且仅被赋值/使用各 1 次，会触发常量传播折叠，强行将迭代器初始化替换为 `ldr r4, =0x08098622`，彻底抹去 r8 伪寄存器；若在清屏前赋给迭代器，又会导致其在 global-alloc 中因循环内高频引用抢先夺取 r4，把清屏循环的低位寄存器分配挤乱。
  - **突破口**: 声明外部数组符号 `extern const u8 gTitleMenuDesc[];`（并在 `linker.ld` 中定义绝对符号 `gTitleMenuDesc = 0x08098622;`），源码写为：
    ```c
    ClearBuffer((u16 *)0x02005800, 30, 20);
    table = gTitleMenuDesc;
    src = table;
    ```
    外部符号无法被 `update_equiv_regs` 作为已知整型常量折叠，GCC 的 loop/GCSE 自动将其识别为循环不变量提升至 prologue，由于清屏循环占满了 r4-r7，table 自然被分配到首选的高位寄存器 r8，清屏结束后 `src = table;` 顺理成章发射为目标的 `mov r4, r8`，逐指令 100% 完美命中！

## sub_8013934 (0x08013934, menu.c, ✅ 2026-09-08 antigravity)
卡片相册页面选项文字与数字框绘制：472 字节，完全逐字节匹配（通过 `fncheck.py sub_8013934`，`make` 及 `sha1sum -c ll.sha1` 全绿，`audit.py` 750/750 全过）。

- **业务逻辑**:
  1. 循环 `i = 0..3` 处理 4 个相册页面条目：
     - 若 `i == gMenuCursorSel`，文本高亮色 `color = 13`，否则普通色 `11`；
     - 若 `i <= 1`：
       - 测试 `(gUnk_03004D48 & 1) == 0`，若为 0 绘制 `gUnk_087EB278[i + 10]` 字符串；
       - 若非 0 绘制 `gUnk_087EB278[i]` 字符串，并在条目右侧清空 3×2 小框 (`ClearBuffer(buf, 3, 2)`)，调用 `sub_800EAE4` 绘制当前页码/游标数字（`i == 0` 时绘制 `gCardAlbumPage`，`i == 1` 时绘制 `gCardAlbumCursor`）；
     - 若 `i > 1`：直接绘制 `gUnk_087EB278[i]` 字符串。
- **攻克历程与关键技术突破 (破除假性 LRA 误区，验证经验 191 同构型)**:
  1. **破除前人错误诊断**: 前代 agent 在 `functions.tsv` 中误将本函数与 `sub_8049AD8` 归为 "u16 shifted-home + LRA live-range-split 不可达" 失败案例。经深入反编译比对发现：
     - 前人手写了 `do...while` 循环填 0xB001，导致未调用内联函数 `ClearBuffer`，分支结构也完全反转；
     - 目标代码中的 `ClearBuffer(buf, 3, 2)`（移位 `0x80 << 10` 即 2）与 `include/inline_funcs.h` 中的标准实现 100% 同构！
  2. **高位寄存器 `sl` 的归位**:
     - 目标汇编在清屏前 `ldr r2, =buf; movs r1, #0; ldr r0, =ptr; mov sl, r0;`，清屏后发射 `mov r2, sl; ldrb/ldrh r1, [r2]; bl sub_800EAE4`。
     - 若直接写立即数 `(u8 *)0x03000232`，GCC2 的 `update_equiv_regs` 会将其直接折叠并在清屏后发出 `ldr r0, =0x03000232`，清屏前不会把地址放进 `sl`，反而导致 `gUnk_087EB278` 被全局提升进 `sl`。
     - 正解完全契合**经验 191**: 使用外部符号 `extern u8 gCardAlbumPage;` 和 `extern u16 gCardAlbumCursor;`（并在 `linker.ld` 中登记 `gUnk_087EB278 = 0x087EB278;`）：
       ```c
       u8 *table;
       u8 *val_ptr;
       ClearBuffer((u16 *)0x02005932, 3, 2);
       table = &gCardAlbumPage;
       val_ptr = table;
       sub_800EAE4((u16 *)0x02005936, *val_ptr, color);
       ```
     - 外部符号阻止了常量传播折叠，GCC 将其作为寄存器变量保持；由于清屏循环占用低寄存器，GCC 精确将其分配入 `sl`，并在清屏后发射目标的 `mov r2, sl`，所有指令与文字池顺序 100% 逐字节对齐！


## 2026-09-08 InvUi_Main 挂起 (zcode-invui) — 结构全对, global-alloc home 差 1 链 C 不可达

**接手状态**: 前手 antigravity 挂起于 "GCSE PRE 提升 pos2<<6 进 r8 挤占寄存器分配", 候选 12840 分。
codex-menu 曾记录 "剩 ~130B = u16 shifted-home + adds 纯拷贝 (LRA live-range-split 家族)"。

**本轮突破 (12840 → 12090 fndiff 分, 残差从 ~170 条指令位差降到 ~121 条真实差异)**:

1. **破除经验 122 的 "shifted-home 不可达" 残余误判**: 目标 prologue 的 `movs r1,#0xB0; lsls r1,#0xD; mov r8,r1` (22<<16 编码清屏宽度) 已被经验 191 的 sub_8013870 完美匹配判例推翻 — 纯 C `ClearBuffer(buf, 22, 8)` 即可产出该形状, 本函数同样直接命中。
2. **next_row 必须 u32**: 目标 `adds r6,#1; mov sl,r6` (无截断) + 尾部 `mov r2,sl; lsls/lsrs #24` — u8 会在赋值前截断多 2 条指令, u32 尾部统一截断才对。
3. **sub_800EAE4 的 arg2 实为 u32**: 目标传 icon 无 `lsls/lsrs #0x10` 截断, 而 `sub_800AB18` 返回值后有一次截断 (u16 返回的 caller-side 截断)。code_0.h 现注册的 `u16` arg2 是错的, 但因会扰动其它已匹配调用点 (menu.c:558/954/964, menu_ui.c:735) 未敢本轮改, 留给后续验证。
4. **LIM 提升的真正破法**: 名字循环 `while ((ch = *strp++) != 0) {...}` 的 loop 头 = 测试块 → gcc loop.c 识别为正规循环 → `pos2<<6` 被判定循环不变量提升到循环前 (占 r8)。**改为 y2 内联 `(rowi*2+7)` 形态后循环块结构变体使 LIM 不再识别** (实验了 do-while(1)/do-while(cnt<8)/while+标志位均不奏效, 最终靠表达式内联改变基本块拓扑)。
5. **count/icon 落 r2 的触发器**: else 分支比较写成 `rowi == *(gMenuCursorStack + gMenuCursorGrp)` (指针算术而非数组下标!) — 改变伪结构后 count/icon 从 r1 挪到 r2, col 中转从 r3 变 r0, 与目标一致。数组下标形式 gcc 会先 ldrb grp 常量再 ldr base, 与目标的装载数序相反。
6. **汇合点 PRE phi 拷贝的出现**: 汇合后 `y2 = rowi*2+7` 与两分支内的 `rowi*2` 部分冗余 → gcse PRE 插入拷贝 (我的形态: `adds r4, r5, #0` 于汇合块头; 目标: `adds r3, r4, #0` 于 icon==0 尾 + `adds r3, r5, #0` 于 icon!=0 尾)。两者语义同构但 home 选择不同。

**最终卡点 (C 层不可达, 全部试遍)**: 目标把 y2 分配到 r2 (与 count/icon 的 r2 共色), PRE phi 伪分配到 r3; 我的 gcc 给 y2 = r3, PRE phi = r4 — 全链系统性差 1。已穷举:
- y2 类型: u8 / u32 / int+(u8) 强转 / icon 变量复用 (同一伪) — 全部无效或恶化
- 声明顺序: y2 前移/后移/移到最末 — 无效 (home 由冲突图决定非声明序)
- pos2 双表达式写法 (rowi*2 vs rowi+rowi vs rowi<<1) — gcc expand 阶段全部归一为 ashift, CSE 必然合并
- 两分支 pos2 赋值删一个 (icon==0 内联) — 恰好触发 PRE phi (本形态已采用)
- permuter 3 轮长跑 (累计 ~30000 iterations): 合法探索平台 ~8000 分, 3810/4060 分产出全部依赖 volatile unsigned long long / 改被调函数返回类型等作弊, 已拒绝 (经验 18/113)

**候选**: `permuter/InvUi_Main/best_20260908_zcode_12090.c` (fndiff 12090 分, base.c 同步), 备选 y2=u8 版 `best_u8_131.c`。逐指令对齐工具脚本思路: gbadisasm 输出与 objdump 输出做寄存器别名归一 (sb==r9!) + 池引用形式归一后逐条 diff — 本轮大量 "差异" 是 sb/r9 别名与池标签形式造成的伪差异, 排查时务必先归一。

**下一步建议**: 
- 查 gcc2 global-alloc 对 y2 的 prefer 寄存器来源 (gccdump.lreg 或重编 agbcc 加 dump), 确认 y2 的 preferred reg 为何是 r3 不是 r2;
- sub_800EAE4 arg2 u32 化对全项目调用点做字节影响评估后修正 code_0.h;
- 或尝试把本函数移入独立 C 文件 (menu.c 已 1000+ 行, register pressure 泄漏风险)。

**补充扫描 (2026-09-08 zcode-invui, 应用户要求扫描已匹配函数找相似形状)**:
- 全库扫描 751 个已匹配函数: 与 InvUi_Main 同族 (ClearBuffer 大块 + 游标循环 + gUnk_08095028 名字表 + sub_800EAE4) 的有 `MenuUi_DrawItemList` (✅)、`sub_8013870` (✅)、`sub_8013934` (✅)、`TitleMenu_ProcessFrame` (✅)、`InvUi_DrawCursors` (✅, 过小)。
- `MenuUi_DrawItemList` 的名字循环是 `for (i=0; i<8; i++) {ch=*src++; if(ch==0) break; ...}` 形式且匹配成功、无 LIM 提升 — 但它只有 1 个位移使用点 (无 PRE 介入), 迁移到 InvUi_Main (3 个使用点) 后分数不变 (135 vs 132), 因为 PRE 的介入与使用点数量绑定, 语义上无法绕开。
- **全库没有任何已匹配函数包含 InvUi_Main 目标的 "两分支各算 rowi*2 + 汇合 GCSE PRE phi 拷贝" 形状** (`adds rX,rA,#0` 与 `adds rX,rB,#0` 同目标双源拷贝在匹配函数中全部是普通参数拷贝/基址复用, 非PRE phi) — 该形状在本项目无已解决先例, home 差 1 链 (y2 r3 vs r2, phi r4 vs r3) 仍是 C 不可达。
- 同族未匹配函数 `sub_8011268` (技能菜单物品页, opencode 认领) 目标形状高度相似 (col 家 sb、pos2 溢出 [sp]、y2=u8(pos2+10) 截断), 但它的 pos2 在**两分支汇合后**才统一计算 (两分支只用 col), 无 PRE phi — 印证 InvUi_Main 的 phi 形态源于 icon==0 分支内 `rowi*2+7` 参数与汇合点 `rowi*2+7` 的跨块部分冗余。
- 结论: 本函数剩余卡点维持 "home 差 1 链" 判定, 候选 `best_u32_125.c` (125 diff / fndiff 12290) 为当前最优。

## ⏸ sub_805063C 大幅推进 (114行, 前人35B→本轮44B/228B真实残差, permuter 470分, 2026-09-08, zcode-805063c)

接手 void-main 2026-09-06 挂起候选 (bytecmp 35B/228B, permuter 585分)。本轮 30+ permuter 轮
(-j1, ~5min/轮, 逐轮 bytecmp 定性后 promote) + 20+ 人工定向变体 (exp/exp2 系列), 完整改写了残差结构:

### ✅ 破前人卡点 A (移位编码 #0x10/#0xf)
前人结论 "目标 `lsls #0x10; lsrs #0xf` (HImode) 编码当前 agbcc 无法复现, 疑 GCC2 版本差异" **不成立**。
正解: `u32 t = (u16)(i + 0xE0) * 2;` — t 必须是 **u32** 且加法结果先截断 u16 再乘 2:
combine 把 `(u16)(x)*2` 重结合成 `x<<17>>16` (SImode lsls#0x10/lsrs#0xf), 若 t 声明为 u16
则展开为 `x<<17>>17>>1` 出 #0x11/#0x10。`t = ((u16)(i+0xE0))*2` 与 `t=(u16)(i+0xE0)<<1` 字节不同 (乘 vs 移)。

### ✅ 新结构发现 (全部 bytecmp 逐条验证)
1. **ret 必须 u32 并兼作循环守卫**: `if (ret < n)` (恒真, ret=0) — 目标守卫是 `cmp r9(ret/sb), r3(n)`
   而非独立 i 比较。这解释了目标 prologue 用 sb 存 ret: 高寄存器 home 是守卫变量的。
2. **0xFF & v 常量前置**: 直写分支 `p[0] = 0xB000 + ((0xFF & v) << 1)` — BattleFx_UpdateTable 经验⑥
   (常量在源文本前) 的再证实, ands 才能落 `寄存器, #imm` 形状。
3. **w = v 双 web**: 比较链 (if) 用 w, 直写/循环用 v — 强制 GCC 生成两个 v 副本 web, 占满低寄存器,
   把计数器地址挤出低 8 寄存器 (目标 ip home)。
4. **p 求和链顺序**: `p = (y * 32 + x) + dest;` 才出 `adds r3,r3,r2; lsls r3,#1; adds r5,r0,r3`;
   `dest + (x + y*32)` 会出 r2 链 (adds r2,r2,r3)。
5. **重读地址加数序**: store2 的重读必须 `(base + charIdx * 18) + gUnk_03000F2A * 2` —
   池基址先加 charIdx*18 再加 counter*2, 出 `adds r0,r0,r6; adds r0,r0,r7`; 反序则 GCC 合并进不同 web。
6. **else 分支初始化序**: `n = gUnk_03000F24; i = 0; base2 = 0xB000;` (n 在最前) — 决定 0xB000
   物化在搜索头 (movs r2,#0xB0; lsls#8) 而非循环内。
7. **store2 的 u 提取**: `(u = t + 1) + base2` 内联赋值 (前人 VAF 招式复验有效, 免 orr/fold)。

### 残差 44B 明细 (目标 vs 本候选, 全部 local-alloc web 拆分痕迹)
- ① 直写分支: 目标 v 住 r4 且**不被 ands 破坏** (ands 走 r0 副本, 常量 0xFF 住 r1 复用两次);
  我的 v 住 r1 且被 `ands r1,r3` 原地破坏, 0xFF 住 r3。根因: 目标在 if 前有 `adds r2,r4,#0`
  (v→比较副本), 我方的双 web 拆分点在 w=v (if 用 w) 而非 v→r2 (if 用副本)。即目标的两个 web
  = [v:r4 原值 + r2 比较副本], 我的是 [v:r1 + w:r4] — 副本创建时刻不同。
- ② 搜索循环: 目标计数器 i 住 r1、上界 n 住 r2 (`adds r2,r3,#0` n 副本)、v 比较用 r4;
  我的 i 住 r3、n 住 r1、v 比较用 r4 — i/n 的 PRI 互换。尝试 i/n 声明顺序、初始化顺序、
  while/do-while、双变量 (v=w 后 w 做被比较值) 均未翻转。
- ③ 0xB000 物化: 形状已对 (movs+lsls#8, r8 home), 但我的在搜索头 0xB0 物化先于 n 读;
  目标 n 读先于物化 — preheader 排序差异。
- ④ 入口 ldrh 目标寄存器: 目标 `ldrh r4` (v 直接进 home), 我 `ldrh r1` 再拷贝 — 与①同根。

### 结论
表达式/语句形状已穷尽 (50+ 变体: exp*.c exp2*.c, 全部 bytecmp 记录), 剩余为 GCC2 local-alloc
web 拆分时刻与 allocno PRI 排布, C 源码级不可达 (同 Op_AddPartyMember r6/r7 案例需 -da RTL 级
死赋值技巧, 但本函数无合适的死赋值插入点: 循环变量 i 的 web 太短, 无引用可加)。
**最终候选 = permuter/sub_805063C/base.c** (permuter 470 分, bytecmp 44B/228B 差,
语义逐条核对与目标一致, 人类可读无作弊)。前人 35B 候选含 `(v^0)`/new_var 屏障技巧,
其 44B 候选语义等价但结构虚假 (比较复制不可由自然 C 产生), 已弃用; 本轮 44B 为诚实下界。
**未达分数 0, 按铁律 6 未合入 src。**
下一步建议: ①用 agbcc -da 转储对比 allocno PRI 排布 (EXPERIENCE 117/177), 定位 i/n PRI 互换
的量化差; ②尝试 -g 变体 flag (改变保活) 对①③的影响; ③匹配调用方 sub_8050720 后回看寄存器
传参压力是否解释 web 拆分时刻。

## 2026-09-08 `sub_8015E1C` (108B, 攻克 Ghost 寄存器 r6 全 ROM 唯二谜团, ✅匹配)

- **函数定位**: `src/menu.c` (0x08015E1C), 108 字节。UI 瓦片文字串输出器（仅被 `InvUi_Main` 调用一次）。
- **攻关历程与核心突破**:
  1. 前人 2026-09-02 (70B) 与 2026-09-04 (65B) 尝试均卡在寄存器置换。本轮初始通过重构分支与 dest 计算将体内 57 条指令 100% 对齐，仅剩首尾 2 字节差异：
     目标 prologue `push {r4, r5, r6, lr}` / epilogue `pop {r4, r5, r6}`，而体内完全未读写 `r6`（Ghost 寄存器）。
  2. 普查全 ROM 750+ 匹配函数与所有 nonmatchings，确认全 ROM 仅有两个函数存在 prologue push 了 callee-saved 寄存器但在体内完全未使用的现象：
     ① `MenuUi_HideAll` (0x0801667C, push {r4, lr}, 体内无 r4, 已 100% 匹配)；
     ② `sub_8015E1C` (0x08015E1C, push {r4, r5, r6, lr}, 体内无 r6)。
  3. 全链路溯源 GCC 2.95 编译机制：
     - 在 `Text_PutGlyph` 原型中，`tileId = ((charCode & 0xFF00) >> 7) + 0x280;`。
     - 当以 `static inline void PutGlyph(u16 *tilemap, u16 charCode, u8 palette)` 形式被内联进 `while (*arg3 != 0xFF)` 循环时，`charCode & 0xFF00` 的 16 位掩码常量 0xFF00 (insn: set reg:SI 68) 被 GCC `loop` pass 识别为循环不变量，并提升（hoist）至 loop preheader。
     - 随后 `combine` pass 发现传入的 `charCode = *arg3` 实际上为 8 位载荷，`(u8)x & 0xFF00` 恒为 0，因而将循环体内的整个移位表达式折叠消除，循环体内部不再需要 0xFF00。
     - 但在 `global_alloc` 阶段，preheader 中的 pseudo reg 68 依然存活且跨越整个循环，由于低寄存器 r0-r3 在循环体中频繁被使用、r4 被 `arg3` 占用、r5 被 `palette << 28` 占用，`global_alloc` 将其分配给了下一个可用的 callee-saved 寄存器 `r6`！
     - `reload` 期调用 `mark_home_live(6)` 将 `regs_ever_live[6] = 1`。
     - 最终在 reload 后的 `flow2` pass 中，GCC 发现 preheader 中的 0xFF00 指令结果无消费者，将其删除为 `NOTE_INSN_DELETED`。
     - 然而 `flow.c:1635` 从 reload 恢复了 `regs_ever_live`，导致 prologue 与 epilogue 输出了 `push {r4, r5, r6, lr}` / `pop {r4, r5, r6}`，而函数体内完全没有 `r6` 指令！
  4. 验证与定论：
     - 采用 `static inline void PutGlyph` 辅以内联调用，配合 K&R 风格形参声明（与 `include/code_0.h:305` 中的 `void sub_8015E1C();` 兼容，零副作用），立即达成 permuter `base score = 0`，`bytecmp.sh` 输出 `OK (108 bytes)`，`fncheck.py` 100% 字节定论，全 ROM SHA-1 校验通过！

## 2026-09-09 `sub_80512C4` (Op_ScriptStreamLZ, 220B, 脚本VM流式LZ解压操作码, ✅匹配)

- **函数定位**: `src/script_vm.c` (0x080512C4), 220 字节 (97 行汇编)。脚本虚拟机流式 LZ77 解压操作码（`Op_ScriptStreamLZ` / opcode 0x15）。
- **攻关历程与核心突破**:
  1. 初始前人状态：卡在末尾 else 分支寄存器轮换（Score 90 平台期），其余 90+ 行指令（包括堆栈帧、保存旧集入栈、读取 songId 与 entry、字面池、循环、DISPCNT 检查、解压初始化、跳转表等）已完全对齐。
  2. 差异瓶颈定位：
     - Target:
       ```arm
       ldr r0, =0x03000E70
       ldrh r1, [r0]
       movs r3, #128; lsls r3, r3, #2; adds r2, r3, #0
       orrs r1, r2
       strh r1, [r0]
       ```
     - 初始 C 产生：
       ```arm
       ldr r2, =0x03000E70
       ldrh r0, [r2]
       movs r3, #128; lsls r3, r3, #2; adds r1, r3, #0
       orrs r0, r1
       strh r0, [r2]
       ```
     - 经深入对比兄弟函数 `sub_80513A0` 与 GCC 2.95 `local-alloc` 的数量分配权重机制，发现目标在 else 块中将地址分配至 `r0` 的关键在于：`ioReg` 指针在 `if` 前用于读取 `0x04000000` (REG_DISPCNT)，并在 `else` 块复用为指向 `0x02016000` 传入 `LZ_InitContext`。
  3. 结构修正与定论：
     - 声明局部指针 `vu16 *ioReg;`，在 `if` 分支前执行 `ioReg = (vu16 *)0x04000000;`，并在 `else` 块执行 `ioReg = (vu16 *)0x02016000; LZ_InitContext((u8 *)ioReg, lzData, chunkSize);`。
     - 该局部指针精准重塑了 basic block 2 的 pseudo-register 生命周期与优先权，引导 GCC 将 `&gScriptVmFlags` (0x03000E70) 分配入 `r0`，旧值载入 `r1`，mask 载入 `r2`，达成 100% 逐指令完全对齐！
     - 登记符号：在 `include/iwram.h` 声明 `gUnk_03000EC8` / `gUnk_03000EC9`，在 `linker.ld` 按地址序补齐相应 section 条目。
     - `fncheck.py sub_80512C4`: 220 bytes OK, 全量 `make` 与 `sha1sum -c ll.sha1` 终验完全通过！

## 2026-09-09 `sub_80513A0` (Op_ScriptReturnChunk, 172B, 脚本VM恢复LZ块并弹栈操作码, ✅匹配)

- **函数定位**: `src/script_vm.c` (0x080513A0), 172 字节 (74 行汇编)。脚本虚拟机恢复上级 LZ 块并弹栈返回操作码（`Op_ScriptReturnChunk` / opcode 0x16）。
- **攻关历程与假象破除**:
  1. 初始前人状态：note 记录 `⏸ permuter score 60(不可达0); 差异全为GCC2 S-bit指令选择(adds/movs/lsls/ands/orrs vs add/mov/lsl/and/orr); 语义完全正确; 最佳C见permuter output-60-1`。
  2. 深入核查发现：
     - 前人误判了 permuter 的 60 分惩罚：汇编内部所有 S-bit 指令选择、寄存器分配、分支结构完全 100% 相同！
     - 60 分的真实来源是未施加重定位的 12 个字面池符号（12 × 5 = 60 分，典型经验 29 反向假象）。
     - 通过 `bytecmp.sh` 配合绝对符号地址比对，除 3 处函数调用外部 `bl` 槽外，172 字节二进制完全 100% 一致！
  3. 代码清理与合入：
     - 清理 permuter 伪宏与命名临时变量，采用与 `sub_80512C4` 统一的 `ioReg` 风格与 `gScriptReturnSetId` / `gScriptCursor` 规范命名。
     - 在 `linker.ld` 补齐遗漏的 `0x03000F30` 符号声明；在 `include/code_0.h` 更新原型为 `u32 sub_80513A0(u32 *);`。
     - 合入 `src/script_vm.c`，`fncheck.py sub_80513A0` 172 bytes OK，全量 `make` 与 `sha1sum -c ll.sha1` 校验完全通过！

## 2026-09-09 `sub_801A2AC` (64B, SIO/Blend 控制寄存器设置, ✅匹配)

- **函数定位**: `src/sio_link.c` (0x0801A2AC), 64 字节 (30 行汇编)。用于根据参数设置 `REG_BLDCNT`、`REG_BLDALPHA`，并在模式符合时写入 `REG_BLDY`。
- **攻关历程与核心突破**:
  1. 初始前人状态：note 记录 `⏸ 2026-09-05 zcode-ll: 原型实为(int,u8,u8)非(u16,u8,u8); 语义全解... 候选 bytecmp 剩 13B (v-shift 调度位置与链 home 分裂)`。
  2. 根因剖析：
     - 汇编入口前四条指令为：
       ```arm
       push {r4, lr}
       lsls r3, r0, #16
       lsls r1, r1, #24
       lsrs r4, r1, #24
       lsls r2, r2, #24
       ```
     - 在 GCC 2.95 中，若形参声明为 `u8 arg1, u8 arg2`，参数拓展与零扩展代码会被 `init_function_start` / `assign_parms` 恒定强制发射在函数体语句的最顶端，导致 `lsls r1, r1, #24` 总是跑在 `lsls r3, r0, #16` 之前。
     - 必须将形参统一声明为 32 位整型 `int arg0, int arg1, int arg2`（或 `u32`），并在函数体入口显式定义局部 `u8 b1 = arg1; u8 b2 = arg2;`，方能使 `u32 v = arg0 << 16;` 的 RTL 率先生成，彻底消灭前导 4 条指令的时序倒置问题！
  3. 表达式与语句流还原：
     - 随后对 `v` 实施就地移位与掩码运算：`v >>= 22; v &= 2; switch ((u16)v)`。
     - `switch ((u16)v)` 的 `(u16)` 截断精准引出目标汇编的 `lsls r3, r3, #16; lsrs r3, r3, #16; cmp r3, #3; bgt ...; cmp r3, #2; blt ...`。
     - 达成 `score = 0`，`bytecmp.sh` 64 字节 0 diff！
  4. 交付与终验：
     - 更新 `include/code_0.h:443` 为 `void sub_801A2AC(int, int, int);`，核验全部调用点（`sub_80348A8`, `sub_8031580` 等），调用点字节完全不受扰动。
     - 合入 `src/sio_link.c`，`python3 scripts/fncheck.py sub_801A2AC` 100% 通过（64 字节全等，0 重定位，0 bl 槽忽略）。
     - 全库 `make && sha1sum -c ll.sha1` 与 `python3 scripts/audit.py` (755/755) 全绿通过！

## 2026-09-09 `sub_804E7EC` (112B, 对象池属性检查与加入更新队列, ✅匹配)

- **函数定位**: `src/obj_pool.c` (0x0804E7EC), 112 字节 (57 行汇编)。检查对象的 `0x91` 与 `0x92` 字段是否通过 `sub_804DD90(val, 6)`，若命中则将对应 `(obj + slot)[0x90]` 清零，并将对象指针入队 `gUnk_03000DF0[gUnk_03000E04++]`。
- **攻关历程与核心突破**:
  1. 初始前人状态：note 记录 `⏸规则17类寄存器轮换: ... 卡点=callee-saved r4/r5/r6 分配 ... 2026-09-02 ll-agent 复核: v1-v5+goto+permuter(至525分)均未破; obj→r4/v92→r5 恒错位`。
  2. 破除 `r4/r5/r6` 寄存器分配墙与调度错位：
     - 参考同文件同一作者编写的兄弟函数 `sub_804F0B8`：作者在此类双槽检查逻辑中均采用 `ret = 0; a = obj[0x91]; b = obj[0x92]; do { if (a == 0 && b == 0) return 0; } while (0);` 模式。
     - 若将 `sub_804E7EC` 扁平写在一个函数内，`obj` 使用次数达 4 次，GCC 2.95 的 `global-alloc` 优先级公式导致 `obj` 抢占 `r4`，与目标（`r4 = v92`, `r5 = obj`, `r6 = ret`）错位；
     - 若提取独立 inline 函数且传入 `(v91, v92)`，则 `ret = 0` 会在形参求值后生成，导致 `movs r6, #0` 落在 `ldrb r4, [r1]` 之后（0xe 处，目标在 4 处）；
     - **终极破局**：将检查逻辑封装为 `static inline u8 CheckObj(u8 *obj)`，在内联函数第一行执行 `u8 ret = 0;`，随后读取 `u8 v91 = obj[0x91]; u8 v92 = obj[0x92];`。
     - 这一设计使 `ret = 0` 的 RTL 率先生成（紧随 `r5 = obj` 之后，生成 `movs r6, #0`），且对象指针在子作用域中被引用，使得 `v92` 的生命周期与使用频次完美匹配 `r4`，`obj` 归入 `r5`，`ret` 归入 `r6`！
     - 尾部写入 `(obj + slot)[0x90] = 0;` 完美消除操作数倒置，输出 `adds r0, r5, r0`。
  3. 终验定论：
     - permuter 跑出 `base score = 0`！
     - 替换 `src/obj_pool.c` 的 `INCLUDE_ASM`，`python3 scripts/fncheck.py sub_804E7EC` 112 字节 100% OK（0 重定位，2 bl 槽忽略）。
     - 全量 `make` 与 `sha1sum -c ll.sha1` 终验绿，匹配进度达 **756/1059 (71.4%)**！

## 2026-09-09 `sub_804BE90` (132B, 对象槽批量清除/失效, ✅匹配, BD54 逐字节孪生)

- 与已匹配的 `sub_804BD54` 归一化标签后**逐字节相同** (同表 gUnk_03000BE8 / 同调用 sub_804C5F8+C674,
  仅 bl 位置相关编码不同), 直接复用 BD54 真身 C (经验 201 家族模板: `int empty=-1` 函数作用域 +
  块内 base/mask + `u32 v=0x20; v&=flags`)。
- permuter 新建套件时踩到两个打分坑, 都与链接期字段有关, **不是代码问题**:
  1. `target.s` 池是字面量 `.4byte 0x03000BE8`, 候选侧池是 `R_ARM_ABS32 gUnk_03000BE8` 重定位,
     base score 恒 5; 把 target.s 池改成 `.4byte gUnk_03000BE8` (与候选同为符号重定位) 后 **base score = 0**。
     (与经验 compile.sh `.equ` 招式等价的反向做法, 目标字节值不变)
  2. bytecmp 给 bl 符号赋 ROM 地址 (0x0804C5F9) 会触发 ld 的 interworking **veneer** (多出 20~36B 尾部
     `ldr pc,[pc,#-4]` 蹦床 + bl 改指蹦床, 即前人"仅差 20B=bl槽"的来源); 赋近址 (下一条指令 0x4B/0x53)
     仍因符号无 thumb 类型标记生成 `__x_from_thumb`  veneer。带外部 bl 的函数定性直接用
     fncheck (2 bl 槽由全量链接保证), 别在 bytecmp 的 bl 上耗。
- 收尾: code_0.h:919 K&R `void sub_804BE90();` → `void sub_804BE90(u8, u8);` (照 BD54 先例,
  K&R 空参与真身定义冲突报 "can't match an empty parameter name list"); gen_asm → fncheck OK 132B
  → make 全量绿 → SHA1 绿, 进度 758/1059 (71.6%)。

## ⏸ sub_805063C-2 人工模式续攻 (131B/224B = 可读代码诚实下界, 2026-09-10, zcode-805063c)

### ⚠ 方法论修正 (用户裁定)
第一轮链路用 permuter new_var 穷举把 bytecmp 压到 50B, 但那些变形
(new_var5/9/13/14、do{}while(0) 双层、p[0&0xFF]、volatile) 语义等价却**不可读、不可合入** —
按铁律 6 人工修正后字节立刻回退。本轮起人工为主, permuter 只作结构参考。

### 本轮系统性实验 (全部 bytecmp 定性, h*/x*/z1 系列 ~25 变体)
1. **h1-h4 嵌套赋值矩阵**: `t = (w = load)` (675-1 学到的真结构) + 消费三角
   (if/直写/循环 各用 t/v/w) 全扫 → 156-164B, 劣于 exp1g。原因: t SImode home 抢占 r4,
   挤掉 v 的 u16 home。
2. **0xB000 物化 (残差③)**: 目标 then 分支是 `movs r2,#0xB0; lsls r2,#8` 内联;
   我的 exp1g 是 `ldr r3,=0xFFFFB000` 负池。试: 入口 hoist base2=0xB000 (186B, 但物化提前到入口
   且杀 charIdx home r6)、分支内 base2=0xB000 (190B, 被 LICM 提前) — 均劣化。
   目标的 then 分支内联物化来自 GCC preheader 调度把常量创建放在分支内, C 形状不可达
   (常量只有一次使用时 GCC 才内联; 但我方 GCSE 因 store2 的 B001 池加载而把 0xB000 并入池)。
3. **load 操作数序** (`off + base` vs `base + off`): 字节零差 (GCC 交换律折叠)。
4. **i/n 声明顺序互换** (残差②): 零差 — home 由冲突图着色决定, 与声明序无关 (呼应经验 188)。
5. **n 改 u32**: 零差 (PROMOTE_MODE 已是 SImode)。
6. **while 边界重读全局** `while (i < (u16)gUnk_03000F24)`: 182B 劣化 (多一次 ldrh)。
7. **搜循 store1 加数序** (t+base2 vs base2+t): 132B (+1), 劣化。

### 根因取证 (残差结构, 与目标逐条对照)
- 目标入口: base→r7 低寄存器, 计数器地址→ip **延迟**赋值 (mov ip,r3 在 ldrh 之后);
  我的: base→ip, r7 被计数器地址副本 (adds r7,r3,#0) 先占。
  根源 = 目标的 if 比较走**副本 web** (adds r2,r4,#0 后 cmp r2/r0), 副本占据 r2,
  使 base 的 home 升到 r7; 我的 if 比较直接用 v home, 无副本, base 被挤到 ip。
  而副本 web 的产生要求 GCC 把 if 链与 ands/循环判定为不同 web — w=v 拆分只能产生
  "if=w + ands=v" (我的现状, ands 破坏 v home r4), 无法产生 "if=w副本 + ands=v原值 + 循环=v原值"
  (目标形状) — 后者要求 if 的比较值与 ands 的输入值是不同 web 但与循环同 web,
  即 GCC 需要 w 只被 if 用、v 被 ands+循环用。已试所有 3 变量组合 (x1-x4), 均触发 r6 回退。
- 残差 ① + ④ 同根 (副本 web), ② 是 ① 的级联, ③ 独立 (GCSE 池合并), ⑤ (store2 地址寄存器序)
  是 ① 的级联。

### 结论
**exp1g 131B/224B = 可读 C 的诚实下界**。40+ 变体穷尽, 剩余 5 项残差全部是 GCC2
web 拆分时刻 / preheader 调度 / GCSE 池合并的内部痕迹, 无对应 C 源码形状。
候选 = permuter/sub_805063C/base.c (exp1g)。**未达分数 0, 按铁律 6 未合入 src。**
下一步 (若重启): 需 RTL 级 (-da) 对比 web 拆分时刻, 或找同 C 文件已匹配函数的
"if 副本 + ands 原值"先例借用其源码模式。

## 2026-09-10 `PaletteEffects_Update` (426B, 调色板混合特效逐帧驱动, ✅匹配, claude)

### 语义 (0x080091C4, player_stats.c)
- `gSceneBlendMode` (0x03004628, 新符号) 三值 switch: 唯一写点 = `MapScene_Load`
  (0x08006BB4 处从 `gMapSceneDescriptors[mapId].bgLoadMode` 装入), 唯一读点 = 本函数。
- **case 8**: `gPaletteFxPhase` 0..0x6F 循环; `value = phase>>3`, >7 时 `14-value`
  (三角波 7..14), 再 +7 → `gBlendCoefficients = (旧 & 0xFFE0) | value`。BLDY 明暗摆动。
- **case 11**: phase++; `(phase & 6)==0 ? 0x0D03 : 0x0F03` 整写 → 6 帧周期 BLDY 13/15 闪烁。
- **case 17**: 需 `gCurrentMapId == 0x6B` 且 `gViewportFlags[VF_FADE_PHASE]==1`
  (cmp #1/bne 字面比较); phase++ 后 bit4 区分波谷/波峰两相:
  bit4==0 → `0x1F08 | (0xF & ~((phase>>1)&7))`, bit4!=0 → `0x1F08 | ((phase>>1)&7)`
  (bics 实现 0xF&~x; 注意 0x1F08 低段含 BLDY=8, 高段 0x1F 是第二混合系数)。
- **尾部**: `gPaletteFxMode == 0` 时 4 项菜单条目调色板动画推进: flags 非零且无 bit2 →
  counter++ (gUnk_03000020[i]); `(counter >> gUnk_03000018[i]) >= gUnk_03000014[i]` 时
  (asrs/有符号 blt —— 变量右移= int 提升后算术移位, 同 PaletteTransfer_Update 先例)
  flags bit1 置位 → `flags = 0` (单次停播), 否则 `counter = 0` (回零循环)。
  目标 `strh r0,[r2]` 里 r0 是**早前测过为 0 的寄存器** = GCC 对 `x=0` 的 CSE
  (r6=0 先于 adds 生成), 源码直接写 `gUnk_03000020[i] = 0` 即命中。

### 流程要点
- mkpermuter 因 INCLUDE_ASM 占位跳过 C 种子, 但四件套齐全; base.c 按 fndiff 逐指令手写。
- **compile.sh 追加 `.equ` 绝对符号 11 个** (经验 29): 数据 10 个 + `PaletteFx_Transform`
  (⚠ 地址要带 Thumb 位 0x08009601, 否则 as 生成 blx veneer)。首试 **score=0**。
- bytecmp 剩 4 字节 = `bl PaletteFx_Transform` 的 R_ARM_THM_CALL 偏移 (经验 34 非真差异;
  target.o 是未解析占位 0xFFFFFE, 部分链接必然不同), fncheck 426B OK 定论。
- 新符号登记: `gSceneBlendMode` @linker.ld 0x03004628 (插在 gScreenIdleEventFlags 与
  gChoiceListPtr 之间) + iwram.h extern (0x030047B0=gCurrentMapId, 0x03004800=gViewportFlags
  均已有名, 直接复用; VF_FADE_PHASE=14 枚举正好对上)。

## 2026-09-10 CheckFacingEvent 匹配 (接管自 codex 2026-09-02 挂起, 420B 全等, SHA1 绿)

接管时 codex 留下的 permuter 最佳 1070 (后来实测 775/495/380)。最终破局靠四个叠加的结构发现,
每个单独都不够:

1. **表基址必须用 `gUnk_087E94F8`(= &gUnk_087E94FC[-1], 即 off_87E8D84 blob 末尾的指针字)**
   + `src = &gUnk_087E94F8[gUnk_03004618 * 4]`。
   语义上等价于 `&gUnk_087E94FC[n-1]`, 但写 `gUnk_087E94FC + n*4` 是 off-by-4 的错误语义
   (ROM 的池常量就是 0x087E94F8)。linker.ld SECTIONS 外注册 + data_87E83F0.h extern 同址别名。
2. **`const u16 *offs = gFacingEventOffsets;` 局部缓存表基址** (经验 15/109 族):
   x1 走 offs[rectIdx] 后, y1/x2/y2 走 gFacingEventOffsets[...] — 池加载顺序变成
   chara/tbl/pdir (与 ROM 一致), 且 rectIdx 的 d4(d=dir*4) 在 d8(d*8) 之前物化。
   没有这个变量时池序颠倒 + d4/d8 顺序反 (d8 先)。
3. **`u32 rectIdx = gPlayerMoveDir * 4`** (非 u16!): u16 会插入 lsls/lsrs 截断, 阻断
   combine 把 `rectIdx*2` 折叠成 `dir*8` 的代数通路 (目标 x1 = lsls r1,r1,#3 直接从 dir 字节)。
4. **两个循环共用一个 `u16 i`** (actorIndex 2..0x12, chestIndex 0..15 同名变量):
   这才是 chara 落 r6 的钥匙 — 合并后的计数器 allocno refs=8/live=24 pri≈10000 > chara≈8288,
   global-alloc 先给 r5, chara 被推到 r6; 随后 src 顺理成章复用 r5 (继承), pdir 不再被
   逐出 (无 mov r8,r4)。分离两个计数器时 chara 只能拿 r5, 全函数 home 系统性错位。

配套修复 (连带破案):
- **`ChestObject.x/y` 类型 u16 → s16** (iwram.h): 目标宝箱坐标读取是 `movs r0,#4;
  ldrsh r1,[r4,r0]` — Thumb 的 ldrsh 只有寄存器偏移形式, 无立即数形式; u16 字段只会生成
  `ldrh [r4,#4]`。改成 s16 后 CheckFacingEvent 420B 全等, 且 ChestObjects_LoadForMap
  的 fncheck 从 FAIL(+2B) 变 OK — 它写 strh 不受影响, 读侧受益。
- **`gFacingEventOffsets = 0x08059794` 登记 linker.ld**: fncheck 的符号地址解析里
  code.s 没有这个数据符号, 兜底用了 ll.map 的漂移地址 (0x08059790, 别人 in-progress
  的 +4 位移), 造成假 FAIL。登记绝对符号后解析稳定。
- permuter/CheckFacingEvent/target.s 的池字改为未定义全局符号引用 (gActors/gFacingEventOffsets/
  gPlayerMoveDir/...) — 让 target.o 与候选 .o 的 objdump -drz 行形状一致 (R_ARM_ABS32
  符号行), score 才能从 50 (纯池行显示差) 降到真 0。target.o 不再是字节精确的 ROM 拷贝,
  仅用于 permuter 行级打分; 字节定论仍以 bytecmp/fncheck 为准。

终验: permuter base score = 0 (1031 次迭代复验保持); fncheck OK 420B @0x08003F40
(10 池重定位施加, 6 bl 槽忽略); 全量 make + sha1sum -c ll.sha1 **绿**。

## sub_800A1B4 (0x0800A1B4, 258行) — 全量角色 stat 初始化, 已匹配 (2026-09-10, glm)

PlayerStats 全字段初始化 (gPartyStats[idx], idx=charId?charId-1:0):
- charId∈{9,10}: lv=98, exp=sum(gLevelUpExpTable[0..97]); 否则 lv=0, exp=0
- 清 field_unk[1]/[5], 从 gStatGrowthTail[idx*6..+5] 拷 6 字节到 equip_slot1..6
- next_exp = sum(gLevelUpExpTable[0..lv]) (带恒真 sum<=lv 守卫, 经验194形态)
- 8 维属性 (max_hp/max_mp u16 + base_atc..base_luc u8) 经 sub_8009F70(charId,lv,statIdx) 取
- 复制到 hp/mp/atc..luc 当前值; Stats_BuildSkillList(skills,lv,charId); field_unk[0]=Chara_GetFormGfx(charId)

**关键卡点 (经验192 "home差1链" 变体, 攻克)**:
第二循环 (next_exp) 的寄存器分配 target 是 i=r1/lv=r2, 但所有常规写法 (for/do-while/while, 内联/局部变量, u8/u16/cast) 都得到 i=r2/lv=r0或r1 — 经验192判定的"C不可达"形态。

**破法**: 让第二循环用**独立计数器变量 j** (不复用第一循环的 i), 并赋值 `i = st->lv` 让 lv **复用第一循环 i 的 home (r2)**。源码顺序 `i=st->lv; sum=0; j=0; if(sum<=i) do{...}while(j<=i)` → GCC 自然出 lv→r2 (复用i home), j→r1 (新伪), 与 target 完全吻合。

permuter 先找到 score=40 候选 (new_var=st->lv + while(i<=new_var)), 但那是 home差1 (lv→r1); 人工分析发现"独立计数器 j"才是正解, 改后 score=0 且 532B 逐字节匹配。

终验: fncheck OK (532B @0x0800a1b4, 0池重定位, 11bl槽忽略); 全量 make + sha1sum -c ll.sha1 绿 (761/1059)。

## sub_801E040 (2026-09-10, opencode-1)

**函数**: `u8 sub_801E040(void)` @ 0x0801E040, 模块 scene_obj_core, 202 asm_lines。
**语义**: 场景对象逐帧步进器。cursor=gUnk_03000715, count=gUnk_03000714。cursor<count 时处理
obj=gUnk_030006F8[cursor]，按 obj[0xBE] (槽号) 状态分派: ≤0xA→sub_801DC20(链表注册)+CBA4(kind3);
0xB..0x70 / >0x70 两段重复 inline helper (sub_801D12C(移动状态机)+inner-dispatch on reloaded obj[0xBE]:
≤0xA→CBA4(kind2), ≤0x70→CA08(kind1), (u8)(s-0x71)≤0x8D→CE80(kind2); 末尾 gUnk_03000630--, obj[0xB0]|=4,
sub_8045F94(obj,8)); cursor++。cursor≥count 时查 obj0=gUnk_030006F8[0][0xBE]: ≤0xA→ret=1, ≤0x70→ret=2,
else→sub_8044414()+ret=3; cursor=0; return ret (0..3)。

**关键卡点**: `*(u16*)(obj+0xB0) |= 4` 的 ORR 操作数顺序/寄存器分配。
- 目标: `ldrh r1,[r2]; movs r0,#4; orrs r0,r1` — CONST(4) 在 r0(dest/op1), loaded value 在 r1(op2)。
- agbcc `|=4` / `=4|x` / `=x|4` / 具名 m 局部变量 全部让 GCC2 combine 把 CONST 放第二操作数 →
  `ldrh r0; movs r1,#4; orrs r0,r1` (寄存器互换, 差 4B/copy)。
- 13 种拼法全卡在 28~32 diff (经验78 所述: 非结构体形式全停在 66~90 分的同族现象)。
- **破法** (经验5 直接应用): `u16 newval = 4 | *(u16*)(obj+0xB0); *(u16*)(obj+0xB0) = newval;`
  命名临时变量 + CONST 在左。GCC2 不做 combine-swap (命名变量阻断了 CSE 把 4 折叠进 IOR 的路径)，
  reload 分配 CONST→r0(dest), loaded→r1。一击全等。
- 同一问题的 volatile 变体也全等 (24 diff = 全 pool/bl 重定位), 但违反铁律4"禁止给普通RAM加volatile"。

**permuter**: score=40 (14 pool half-words + 26 bl half-words = 全重定位 artifacts, 指令级 0 差)。
permuter 无法到 0 因 .o 级比较不能解析 extern 符号重定位。以 raw byte cmp + fncheck 定论。

**终验**: fncheck OK (408B @0x0801e040, 0池重定位, 13bl槽忽略); 全量 make + sha1sum -c ll.sha1 绿 (762/1059)。

## 2026-09-10 zcode-main: E1D8 候选推进 (v4, 未合入)

**⏸ sub_801E1D8 (151行, scene_obj_core)** — 候选 v4 大幅推进但未到 0, 挂起待续:
- **target.s/o 重建**: permuter/sub_801E1D8/ 旧套件整体是 sub_801E040 错拷 (target.s 标 @0x0801E040),
  已用 asm/nonmatchings/sub_801E1D8.s 重建 target.s/o (arm-none-eabi-as 通过)。
- **v4 候选** (.scratch/main801E1D8/v4.c): `stride = 0xC8` 必须是**守卫内的局部变量** (经验 87 生死边界:
  目标 `movs r1,#0xc8; mov sb,r1` 在入口守卫之后、do 之前物化, 全函数零 remat) — 这一步让 pool 落 r7 +
  ret 落 sl, 头部 34B 逐指令全对 (v3 把 stride 放函数顶部物化过早, pool 被挤到 r8)。
- 四处乘加操作数序已破解: 倒计时/入队 `k*stride + pool` (adds r0,r0,r7 / adds r3,r3,r7), F90 两处
  `pool + k*stride` (adds r0,r7,r0); F90 段 ldr 4F90→r6 + ldr 730+i→r4 双缓存。
- **剩余差异** (fndiff): ① i/stride home 互换 (目标 i=r8/stride=sb, v4 是 i=r9/stride=r8) — i/stride
  的 pri (refs×live) 差异待 -da flow 量化; ② queue-flag 被 GCC 线程化: 目标保留
  `movs r0,#1`(零路径)/`movs r0,#0`(减路径) + 汇合单点 `cmp r0,#0; beq`, v4 被直接跳转穿透; 入队段需
  `obj2` 局部单乘积复用 (str r3 → adds r0,r3 → str r3,718 全复用), 但 v5 加回 obj2 局部引发 global-alloc
  全洗牌 (pool 掉 r9), 说明 obj2 形态与 home 格局耦合, 需同轮解。
- **工具发现**: `agbcc -da` 的 f.pp.flow 转储 (flow.c dump_flow_info) 可直接拿跨块 allocno 的
  (n_refs, REG_LIVE_LENGTH) — 经验 117 手法可批量用, 无需补丁。
- 套件 target.o 已重建, base.c=v1 (需换 v4 再跑 permuter)。score 参考: v1=3915 最优 (字面池虚高)。

## 2026-09-10 zcode: PaletteFx_Transform 候选推进 (接管自 claude 死锁, 590行, 挂起)

**⏸ PaletteFx_Transform (0x08009600, 590行, player_stats)** — permuter 从 claude 遗留 2750 压到 830
(fndiff 实测 1045), 形状除 amount/count 的 r4↔r6 互换外**逐指令全对**:

- 接管时发现 claude 认领后 50+ 分钟无活动 (锁 08:13, 最后文件改动 11:33), 事实接管。claude 遗留:
  base=2750 (m2c 修订), base_2340=2475, 27 个 output 最佳 1665。
- **还原目标的关键源语义** (相对 m2c/claude 版, 六步):
  1. 循环守卫 `if (count != 0)` + `do {...} while (--count)` 包裹块 (claude 版是 `for(;;)+break`, 编出 pat
     完全不同; 目标守卫跳向尾部 store 而非 return — claude 版此处理解错误, 见函数结构段);
  2. 尾部 `gUnk_03004914 = gUnk_03004910 + 1` **重读全局** (阻断 'mov r8,r3/adds r3,#1' 循环出口 hoist,
     -1195 分) + `effect` 局部变量保留 sw3 专用 (mode 不再全程存活, r8=&4910/r9=&4914 home 归位);
  3. mode5 块双判拆开 `effect = gUnk_03004910; if (effect == 5) { if (gUnk_03004918 > 0x20) {...} }`
     (目标 'mov r0,r8/ldrb r2/cmp #5' 在前, timer 重读在后);
  4. sw1/sw2 内 timer 全部改直读 `gUnk_03004918` (timer 局部删除, -635 分; sw2 由 'mode = gUnk & 3' 借道
     补出目标 e2 `adds r0,r3,#0` 复制);
  5. 乘法操作数交换: 源码写 `ch * amount` 而非 `amount * ch` (dark 体 7 处 subs/lsrs/ands 归位),
     像素组装 `(r + (g << 5)) + (b << 10)` (r 通道在左, -50 分);
  6. permuter 发现 **值借道** (经验 207): bright 体守卫 `amount = count != 0; if (amount)` +
     case7 `count = (gUnk>>1)+1; amount = count << 1` (885→830)。
- **剩余卡点** (fndiff 1045): amount home=r6/count home=r4 与目标互反, 连锁 ~33 处寄存器名差异
  (muls r0,r6↔r4, cmp r4/r6, ands, m6 体常量31 remat 位置跟随)。C 表层穷举无效
  (类型矩阵/声明序/new_var 位置/case 重排/指针化, 见经验 208 存档)。
- **工具坑**: asm-differ 对 NaN 池/填充行的并行对比含大量 'movs r0, r0' 噪声行,
  分析差异时用脚本剥离后真实差异只有 ~33 行; fndiff 目标侧 'subs r0,r3,r0' 等 operand 序
  在错位对齐时会被颜色标淹没, 需先修大块 (守卫/tail) 再看细节。
- 套件: base.c=830 winner, target.o 已重建, compile.sh 含 3 个 .equ。
- **最佳候选路径**: 经验 208 的"可能出路" (qtydump 引用计数干预) 或等 GCC2 flow 级新证据。

## sub_800BFF8 (0x0800BFF8) — ✅ 2026-09-10 glm, 攻克 gpnux ⏸ (2026-09-04 score=650)

数字→3位tiles转换: value 拆百/十/个位, 0 填空白 tile (0x25), base16=ROM基址低16位。
tiles 顺序 [dest_orig]=个位, [dest-1]=十位, [dest-2]=百位。

**三个叠加卡点 (gpnux 穷举 permuter 只到 650)**:
1. **hundreds/tens 必须共用单 `count` 变量** — 分声明让 GCC 把百位放 r2、base16 放 r4 (目标: count→r4, base16→r6)。单一变量让两个循环的计数器同 web, GCC 分到 r4 且 base16 自然落 r6。
2. **tiles[0] 必须写 `base16 + (0x25A + count)` 括号形** — 左结合形 GCC 重关联成 (count+0x25A)+base16, 括号形出 (base16+0x25A)+count 与目标一致。
3. **tiles[1]/[2] 必须写 `(u16)(base16 + 0x25A) + digit`** — u16 强转截断阻止 GCC combine 把两次加法重关联成 (digit+0x25A)+base16, 使 `base16+0x25A` 保留在 r0 跨两条 tiles 计算复用 (目标 0xae `adds r0,r6,r1` → 0xb6 `adds r0,r3,r0` 复用)。
   - 无强转: GCC CSE 只保留字面量 0x25A 在 r1, base16 每次单独加 → 多 2 条 adds (52 diff 卡点)。
   - 显式中间变量 (base25 = base16+0x25A) 会把 base16 挤到 r2、d 挤到 r6 — RA 全崩 (130/170 diff)。

**降 diff 轨迹**: gpnux 650 → v6 (单count) 54 → v11 (tiles[0]括号) 52 → v19 (u16强转) **0**。

关键教训: 三个问题各自独立修正才收敛 — 单修任何一个都不够。permuter 自身无法发现"强转截断"这类 combine 控制手段 (它只探索语句顺序/变量提升)。

终验: permuter base score=0 (2800+ 次迭代复验); fncheck OK (224B @0x0800bff8, 0池重定位, 0bl槽); 全量 make + sha1sum -c ll.sha1 绿 (768/1059)。

## 2026-09-10 claude-d840 — battle_rewards.c 剩余 8 函数全灭 (765→773/1059)

同族作战: D1B4 drop 家族的 8 个尾函数 (D4FC/D5B4/D8F4/D840/DA04/DB64/DC24/DCD8), 全部 permuter 5 分
(唯一池计分) + fncheck OK + ROM SHA1 绿。攻破两个前人挂起项 ((glm-batch 的 D840/DC24 同坑, opencode 的
DCD8 4B 差, sound-agent 的 D4FC scrutinee home)。

### 核心 idiom — "flag 指针 + v 局部 + 表指针进分支" 三件套 (适用于 ldrsb scrutinee 族)
sub_804D840 (D4FC/DC24 同构): ROM 入口 `movs r4,#0; ldrsb r4,[r1,r4]` (零索引落 dest) + case1 `ands r4,r0`
(scrutinee 值直接 AND) + 尾 `ldrb r0,[r2]` 重读。C 还原:
1. **flag 指针在 if/else 两分支内各自赋值** `flag = (s8*)(obj+0xBC);` — strb 后 r1 复活留给 ldrsb;
2. **v 用 unsigned int** (非 s8, 非 u8) — s8 出双 ldrb;
3. **case1 `v &= Rng(); obj[0xC2] = v;`** — 不能 RMW `*flag &=` (会展开成 ldrb+ands+strb 三步);
4. probe 别名表 `Unk_804DABC_Ptr.field_8[obj[0xC2]]` 保持 `ldrb [r2]+lsls #1+adds #8` 链。
key 变体: DB64 case1 = `kind=Rng%5`+`if((u32)obj[0xC2]==2) obj[0xC2]=v` (kind 独立局部);
DA04 (D1B4 家族 push{r4,r5,r6}) = 全部同上但 scrutinee `(s8)obj[0xBC]` 数组读归一化, 无 flag 指针。

### DCD8 4B 差的真相 — break 后调度屏障
opencode 2026-09-04 挂起 "global-alloc r1 跨块复用+地址拷 r2, agbcc 产不出"。解 = case0 `break;` 后
**`while (value) break;`** 一行 (两路皆 break, 零行为差, 比 permuter 的 `do{}while(0)` 可读) — 触发 agbcc
重排 region1 成 `movs r1,#0; movs r0,#1; strb r0,[r2](拷贝地址); strb r1,[r0xC2]`。zero 局部 (u8 zero=0)
+ flag 分支内赋值合用即 152B 全对。d5b4/DCD8/D840 三处证明: **"奇怪的单行屏障"胜过 30 个语句变体**。

### D8F4 幸运掉落 (gold 语义勘误)
ROM `lsls r0,#0x12; lsrs r6,#0x10` = `X/10<<2` (u16 gold 视角的归一化形状; 写 `/10*4` 会出 `<<16>>14` 折叠)。
gold 基址 = **obj+0x6E** (不是 arg1!) — `adds r0,#0x6e` 直接吃 r0 老值 (obj), 写 arg1 会多一条 copy。
lucky=victory 用 r7 低位; case1 else 的 `obj[0xC2] = v` 须复用 scrutinee 局部 (cse 阻断点同 D840)。

### D5B4 团队战奖励 (count=1 载体)
`(count = 1)` 出 `cmp r0,#1` 常量 + count home 保 r6 — 双赢。count 随即被 sub_80489E8 重算, 行为零差。
i 必须 u8 (归一化 lsls/lsrs); victory=unsigned int 声明在末位 (r8 高位); obj[0xAC]== 比较左值化
(`obj[0xAC] == arg1[...]`, == 反转就 130 分)。

### 验证纪律 (次目标完成度)
8/8 函数 permuter 分数 = 5 (唯一池项 0x08393B28 未重定位的固有计分, 经验 29 的 400 分池计分在 permuter
内部折半呈现), 即**逐指令逐字节 0 差异**; bytecmp 全部除 bl 槽逐字节一致; fncheck 全 OK; 每合入一个
make+sha1 全绿。无一依赖 >1000 分退路。

## sub_800F3AC (0x0800F3AC, menu_ui, 屏幕待机图标绘制) — ✅ 2026-09-10 glm
破除 gpnux 挂起 (TSV ⏸ score 240, "待解5条指令参数调度")。语义: ClearBuffer 清 0x02005986
(24x10, 0xB001) 后遍历 gScreenIdleIconIds[i+游标] (i=0..4), 调色板 11/13/12 三态 + 图标 8/0x18
的 EventFlags 分支, 写 0xB190/0xB191 框瓦片并 Msg_DrawPoolSegment+Text_PutGlyph。

### 逐项破关 (4 个独立卡点, 全部为"指令形状全对"级别)
1. **off 移位合并**: `(i << 7) + 0x180` → GCC 合并为 `((i<<23)+(0xC0<<17))>>16`。两步
   `off = i << 7; off += 0x180;` 解。⚠ 先踩了 `i << 7 + 0x180` = `i << 187` 的优先级坑
   (off 恒 0, 三条地址指令静默消失, mine 107 vs 目标 112 条指令才暴露)。
2. **ClearBuffer 展开的 home (w→r7 / 0xB001→r1)**: 由函数级 int 局部的声明序支配。
   终态 `u16 *ptr; int tileOff; u8 i; int iconId; u8 segIdx;` — 逐个试出 tileOff/iconId
   必须 int (u8 则 w→r3/0xB001→r7, 30 处差异)。该问题属 global-alloc (qty 表 0 项, 经验 88)。
3. **ORR 重写**: `base + 0x190` → `mov r1,r8; orrs r1,r2`。tileOff = 0x190 独立成句保住
   `add r1, r8`。
4. **0x02005812 的 CSE**: `subs r1, #62` (= 0x02005850-0x3E) 用 msgPtr 变量/拆语句均无法阻止。
   permuter 中奖 (330→60): `new_var = iconId;` 插在 tile2 存储与 pool3 之间。人工化为
   `segIdx = iconId;` (u8) — 同时买下 CSE 阻断与 `adds r1,r4,#0` 的调度位置。

### 流程
permuter base score = 0 (cleanX.c) → 人工代码 (无 perm 残留, 语义注释完整) → 复验 base score = 0
→ fndiff 仅剩 `...` vs `.short 0` 的 objdump 填充显示差 → 合入 src/menu_ui.c → fncheck OK (252B)
→ make + sha1 全绿。经验 214 已归档。

## sub_800FF10 (0x0800FF10, menu_ui, 装备更换属性预览) — ✅ 2026-09-10 claude_1

接管 `claude_1` 的进行中锁。306 asm 行 / 608B, 两个跳转表。语义: arg0=装备/道具 id (0xFF=不换),
arg1=装备槽 (0..5), arg2=队伍成员索引 (gPartyStats 下标, stride 0x40)。调用方
`SkillMenu_SaveCursor` (text_engine.c)。

### 结构 (四段)
1. `arg0 == 0xFF` → 早返回: 把 `gPartyStats[arg2]` 的 equip_atc..equip_luc (0x1D..0x23) 依次拷进
   `gEquipBonusAtkBase/Def2/Agl/Men/Res/Noa/Luc`。⚠ 写入顺序 (atc,def,agl,men,res,noa,luc) 与
   地址顺序 (0x4A90,0x4AA8,0x4AB0,0x4A98,0x4A94,0x4AB4,0x4AB8) 不同, 照反汇编逐条写。
2. 否则 **两组 `switch(arg1)`**: 第一组临时把 arg0 写进 `equip_slotN` (先存旧值 prev),
   然后 `Stats_RebuildEquipBonuses(gPartyMemberIds[(u8)(gMenuCursorStack[0] - 1)])`, 第二组把 prev 写回。
3. `gEquipBonusAtkBase += gEquipBonusAtk; gEquipBonusDef2 += gEquipBonusDef;`
4. 7 组 `if (ps->equip_X > gX) gStatArrowIds[k] = 0xC; else if (ps->equip_X < gX) gStatArrowIds[k] = 0xD;`
   (相等则保持调用方预置的 0xB)。

### 卡点/关键点
- **双跳转表**: 两组 switch 的 6 个 case 体是相同语句序列, GCC2 把它们 cross-jump 成**共享尾块**,
  每个 case 只剩 `adds r0,#0x24+k; b 共享尾块`。源码按"每个 case 各写两/一句"写即可;
  **不要**写成 `u8 *p = &ps->equip_slot1; p[arg1]` (会退化成单条 add, 跳转表消失)。经验 215。
- **sizeof(PlayerStats) 必须 = 0x40**: 首版 base.c 结构体写到 0x29 就收尾 (sizeof=0x2C), GCC 生成
  `movs r0,#0x2c; muls r1,r0`, 目标却是 `(u8)arg2 << 6` (`lsrs r2,#0x12`)。补 `u8 pad2[0x16]` 立刻对齐。
- **字面池假高**: base score 首测 135 (fndiff 10800) 全来自 13 个数据符号未重定位。按经验 117/§9
  给 `permuter/sub_800FF10/compile.sh` 追加 13 条 `.equ <sym>, 0x<addr>` → base score 立即 0。
- **bytecmp 多 16B**: mine.bin 624 vs 目标 608, 多出的 16B 是 ld 为超出 Thumb BL 范围的目标
  (0x0800A664) 生成的**长跳桩**; 608B 内只有 1 处 4B 差异 = 那条 `bl` 的重定位槽 (bytecmp 已豁免)。
- **luc/noa 顺序**: gStatArrowIds 索引序是 atk,def,agl,men,res,**luc(5),noa(6)** — 与
  `Stats_RebuildEquipBonuses` 内 case 5=Noa / case 6=Luc 相反; sub_800B374 的读取端证实了 luc→[5]。

### 新增/修正符号
- 新增 `gStatArrowIds` @ 0x030001BC (7×u8): 本函数填写, `sub_800B374` 读取后交给 `sub_800EAE4` 渲染;
  0xB=平 / 0xC=升 / 0xD=降。已登记 include/iwram.h + linker.ld (按地址序插在 0x1BA 与 0x1C4 之间)。
- include/code_0.h:214 `void sub_800A664(u8);` 是重命名遗留的旧名 (ll.cfg 与 asm 早已是
  `Stats_RebuildEquipBonuses`), 原位改成规范名。三个调用点实参均为 u8, 补原型后字节不变。

### ⚠ 流程坑 (本轮实踩)
- **Makefile 不跟踪头文件依赖**: 改 `include/*.h` 后必须 `touch src/*.c` 再 make, 否则旧 .o 复用 →
  sha1 假绿。第一次 make 仅 714ms 空转即为信号。已用 `find build -name '*.o' ! -newer include/code_0.h`
  确认 36 个 C 目标全量重编后才认绿。
- **functions.tsv 被并发写坏 (非本 agent)**: 0x0800f3ac/0x0800f4a8 与 0x08024618/0x080246e8 两组相邻行
  各自粘连成一行 (丢换行+note, name 变 `sub_800F3AC1` / `sub_80246180`), 导致 gen_asm 误删
  `asm/matchings/MenuUi_DrawItemList.s` 与 `asm/nonmatchings/sub_80246E8.s`, 全量 make 红在
  `Assembler: can't open asm/nonmatchings/sub_80246E8.s`。详见 INCIDENTS.md。

### 流程
mkpermuter (INCLUDE_ASM 失败)→ 手工搭套件 (copy compile.sh + 汇编 target.o) → m2c 草稿
→ 人工 base.c → fndiff 逐指令一致 (仅池注解差) → bytecmp 4B(=bl 槽) 差 → compile.sh 加 13 条 .equ
→ permuter base score 0 → 合入 src/menu_ui.c → gen_asm → fncheck OK 608B
→ touch src/*.c + make + sha1 绿 (776/1059) → audit 776/776 通过。经验 215 已归档。

## sub_804DFD8 (0x0804DFD8, obj_pool, 物件栏"名称+数量"绘制) — ✅ 2026-09-10 claude_1

138 asm 行 / 266B, 7 个参数 (3 个走栈) + 一次 `__udivsi3`。唯一调用方是 `sub_8024940`
(scene_obj_fx, 未匹配) 的物件列表循环。

### 签名与语义
```
void sub_804DFD8(u16 *arg0, u8 arg1, u8 arg2, u8 *arg3, u8 arg4, u8 arg5, u8 arg6)
```
- `arg0` = tilemap 基址 (调用方传 0x02035AC0); `arg1` = 列, 行 = `arg2 + arg4*2`;
- `arg3` = 4 字节物件条目 (0x030007C8 表): `arg3[0]` = 名称 id (索引 `gItemNames`, 256×8B),
  `arg3[1]` = 数量;
- `arg5` = 调色板号 (<<12 组成 tilemap 条目); `arg6 != 1` 时跳过数量绘制。
- 名称 8 格 × 2 瓦片 (`(arg5<<12) + ch*2` / `+1`), 空字符用空格瓦片 `(arg5<<12)+1`;
- 数量取两位, 个位/十位字形 = `(arg5<<12) + (d + 0xA2)*2 (+1)`; 十位为 0 时整格用空格。

### 三个卡点 (按发现顺序)
1. **字节域分组**: `dst = (u16*)arg0 + row*32 + col` 被 GCC 折成 `row*64 + col*2`
   (`lsls #6` + 复用 truncate 出的 `col*2`); 目标却是 `(row*32+col)*2`
   (`lsls #5; adds col; lsls #1`)。改成 `(u16 *)((u8 *)arg0 + ((row) * 32 + col) * 2)` 即对齐。
2. **抽变量 vs 内联 = 活跃值数量**: 目标把 `arg5` 经 `ip` 保留进循环、循环内重新物化 `(arg5<<12)`,
   因此活跃值多 1 个 → GCC 才把 name 放 sb(r9)、arg6 放 sl(r10), prologue 多压 3 个寄存器。
   我抽了 `int base = arg5 << 12;` → arg5 提前死亡、活跃值 -1、prologue 缩成 `push {r7}`、全函数
   寄存器整体错位 (3234 分, 但逐指令形状全对)。**把 `(arg5 << 12)` 在循环体内联写** → prologue 立即
   对齐, 3234 → 1000。经验 216。
3. **`%` → `__umodsi3`**: `digits[1] = arg3[1] % 10;` 会调 `__umodsi3`, 而目标只有一次
   `bl __udivsi3` + 手算余数 (`d1 = x - d0*10`, 且 d0 从栈重载)。改为 `d0 = (u32)x/10;
   d1 = (u32)x - d0*10;` 后到 0。注意 x 必须按 unsigned (否则 `__divsi3`)。

### 其余记录
- 剩余 1000 分里 dst 的 r5/r6 互换: 到 0 后自动消失 (修 #3 后一并解决)。
- back-edge 的 dead `lsls r6, ip, #0xc` 是正确分配的副产品 (经验 11 类), **不要消除**。
- 新增/修正: `include/code_0.h:985` 原型 `void sub_804DFD8();` → 补全 7 参;
  obj_pool.c 内新增局部 `extern const u8 gUnk_08095028[][8];` (与 menu_ui.c 同风格)。

### 流程
手工搭 permuter 套件 (mkpermuter 对 INCLUDE_ASM 失败) → compile.sh 预置 `.equ gUnk_08095028`
→ 人工 base.c 三轮 (3234 → 1000 → 0) → permuter base score 0 → 合入 src/obj_pool.c
→ gen_asm → fncheck OK 266B → touch src/*.c + make + sha1 绿 (777/1059) → audit 777/777。经验 216 已归档。

## sub_804E0E4 (0x0804E0E4, obj_pool, 物件使用演出状态机) — ✅ 2026-09-10 claude_1

209 asm 行 / 456B, 两个跳转表 + 7 个 bl。调用方 `sub_804F088` (arg0[0xA4] > 0xDC 时进入)。

### 结构
```
u8 sub_804E0E4(u8 *arg0, u32 arg1)
```
1. **前段** `switch (arg0[0xA4] - 0xDD)` (8 cases): 选出一对 (a→r3, b→r2), 只供 case 4 使用;
   注意 case 5/6/7 只赋 a, b 沿用未初始化值 (与目标一致, 不要"补"初值)。
2. **后段** `switch (gUnk_03000DDE)` (0..13 状态机, 返回 1 = 演出结束):
   - 0: `sub_8020DE4()`; state=1; `gUnk_03000DE6 = *(u16*)(arg0+0x2A)`; `gUnk_03000DE8 = arg0[0x35]`;
   - 1: `sub_801CBA4(arg0, 6, 0x1B4, 0xD, 0)`; state=2; `Sfx_Play(0x17,0,0)`;
   - 2: `if (*(u16*)(arg0+0x24) & 0x1000)` → 见下; state=3;
   - 3: `if (!(*(u16*)(arg0+0x24) & 0x800))` → `&= 0xFEFF`; state=4;
   - 4: `if ((u8)sub_801EEE4(arg0, arg1, 0, a, b) == 1)` → state=0xD;
   - 13: result=1; state=0。

### case 2 明细 (三个卡点都在这)
```c
v = arg0[0x35];
sub_804C3A4(v, (u8)sub_801B954((void **)(arg0 + 0xC)));
keys = *(u16 *)(arg0 + 0x24) & 0xEFFF;
zero = 0;
*(u16 *)(arg0 + 0x24) = keys;
sub_801CBA4(arg0, zero, gUnk_03000DE6, gUnk_03000DE8, zero);
keys = *(u16 *)(arg0 + 0x24) | 0x100;
*(u16 *)(arg0 + 0x24) = keys;
gUnk_03000DDE = 3;
```
- 第 2、5 参必须传**同一个 `u32 zero` 变量**(不是两个字面 0) → GCC 才只 `movs r1,#0` 一次并一职两用;
  直写字面 0 会多一条 `movs r0,#0` 且把 `movs r1,#0` 排到调用前 (fndiff 280)。
- 尾部 OR 必须用 `u16 keys` **临时承接再回存**, 不能写 `*(u16*)p |= 0x100;` —— 后者差 2 条指令
  (r0/r1 操作数互换, fndiff 10)。
- 两处写法**都是照抄同族已匹配函数**: `sub_8034440` (event_hub.c) 与 `event_actor.c:190`。经验 217。

### 新增符号
`gUnk_03000DE6` (u16, 保存 `obj+0x2A`) / `gUnk_03000DE8` (u8, 保存 `obj+0x35`), 均在 0x03000DDE
与 0x03000DF0 之间的空洞里, 按地址序登记 iwram.h + linker.ld。原型 `u8 sub_804E0E4(u8 *, u32)`
(code_0.h:986) 本就正确, 未改。

### 流程
手工搭 permuter 套件 + `.equ` 固化 3 个符号 → 人工 base.c 三轮 (280 → 10 → 0)
→ permuter base score 0 → 合入 src/obj_pool.c → gen_asm → fncheck OK 456B
→ touch src/*.c + make + sha1 绿 (778/1059) → audit 778/778。经验 217 已归档。

## sub_804E85C (0x0804E85C, obj_pool, 物件"演出"状态机) — ✅ 2026-09-10 claude_1

192 asm 行 / 384B, 返回 1 = 演出播完。无 C 调用方 (原型原为 `void`, 实为返回 u8)。

### 结构
```
u8 sub_804E85C(void)
```
`gUnk_03000DF0` = 对象指针表 (u32 每项), 第 0 项是**模板对象**; `gUnk_03000E04` = 对象数;
`gUnk_03000E05` = 状态。四个 case:
- **0** 起手: `sub_8020CC4(obj0, obj0[0xBF], obj0[0xC0], 0x2EA, 0xE, 0xA6, 0x104)`; `obj0[0x66] = 3`; → 1
- **1** 等 `*(u16*)(obj0+0x54)` 的 0x800 清零, 然后把模板对象 `+0x3C` 起 0x30 字节状态块
  复制到对象 1..count-1; → 2
- **2** 等 `*(u16*)(obj0+0x54)` 的 0x1000: `sub_804C3A4(obj0[0x65], (u8)sub_801B954(obj0+0x3C))`,
  再清所有对象 `*(u16*)(obji+0xB0)` 的 0x2000 位; → 4
- **4** 逐对象 `sub_804612C(obji, 0xA, 1)`; `result = 1`

### 唯一卡点: 表项必须"每次重读", 不能抽局部指针 (经验 218)
首版写成 `u8 *obj = (u8 *)gUnk_03000DF0[0];` 复用 → 1115 分, 全程逐指令形状几乎全对,
只是寄存器角色整体错位 (r5↔r6、r9↔r8、对象指针落 r5 而非 r0/r2)。
判据 (目标给出两条铁证):
1. **每个 case 开头都重新 `ldr rX, =0x03000DF0`** (没有复用寄存器);
2. **循环体内**还有一条 `ldr r0, [r2]` 重读表项 0 —— 只有源码每次都写表达式才不会被提升成循环不变量。

改成**全程直接下标** `((u8 *)gUnk_03000DF0[n])` → 一次到 0。为可读性收尾时用文件内宏
`#define ObjSlot(n) ((u8 *)gUnk_03000DF0[n])` (纯文本替换), 按铁律 6.6 复验 fndiff 仍 = 0。

### 附带
- `obj+0x3C` 起 0x30 字节整块拷贝: 写成 `*(struct { u32 w[12]; } *)dst = *(struct {...} *)src;`
  即得目标的 `4 × (ldmia r0!,{r5,r6,r7} / stmia r1!,{r5,r6,r7})` —— 不要手写循环或 memcpy。
- 新增文件内 `typedef struct { u32 w[12]; } ObjBlk;` 与 `ObjSlot` 宏 (均在 obj_pool.c 局部)。
- `code_0.h:991` 原型 `void sub_804E85C();` → `u8 sub_804E85C(void);` (反汇编有 r0 返回)。

### 流程
手工搭 permuter 套件 + `.equ` 固化 3 个符号 → 两版 base.c (1115 → 0) → 宏化后复验 0
→ permuter base score 0 → 合入 src/obj_pool.c → gen_asm → fncheck OK 384B
→ touch src/*.c + make + sha1 绿 (779/1059) → audit 779/779。经验 218 已归档。

## sub_804E9DC (0x0804E9DC, obj_pool, 战斗掉落结算) — ⏸ 2026-09-10 claude_1 挂起

271 asm 行。**结构已全部对齐, 只剩 global-alloc home** (终分 6030)。

### 已还原的语义 (结构 100% 对)
```
u8 sub_804E9DC(u32 *arg0)
```
- `gUnk_03000E30 = 0`; `pool = GetObjPool()`; `list = sub_8020E68()` (含 `list[0]`=条数,
  条目在 `list[i*4+1]`); `objs = GetObjPool()` (注意**两次** GetObjPool, 分存两个变量);
- `count = sub_80489E8(objs, values, 0, 0x1FF)` (`u8 values[8]`);
- **第一段**: 遍历 values 找第一个 `sub_804E76C(objs + values[i]*0xC8, 5, 4)` 返回 ≥0 的槽
  (`v = sub_804E76C(...); if ((s8)v >= 0) { found = v; break; }` — u8 临时 + (s8) 判定,
  这样 GCC 才会把 (u8) 归一化做在比较之前并复用 `lsls`); 命中则 `bonus = 0xF` 否则 0;
- **第二段**: `for (i = 0; i < list[0]; i++)`: 跳过 `pool[i*0xC8+0x493] == 7` 的槽;
  `v = list[i*4+1]`; `v <= 0x70` → `k = v-0xC`, 表 `gUnk_0839D9B8`; 否则 `k = v-0x71`,
  表 `gUnk_0839DBB1` (两表都是 **101 条 × 5 字节**, 0x0839DBB1-0x0839D9B8 = 0x1F9 = 101*5);
  `Rng_LcgNext() % 100` 与 0x3B 比较分两路, 再 `% 100` 与 `tbl[k][1]`(低路)/`tbl[k][3]`(高路)
  比较, 命中取 `tbl[k][0]` / `tbl[k][2]`, 加 `bonus` 后为 val;
- val 非 0 则并入 `gUnk_03000E08[]` (按 field_0 找同 id 累加 field_2, 否则新开一条并 `gUnk_03000E30++`);
- 末尾逐条 `sub_800AA60(field_0, field_2)` (= AddInventoryItem), 返回 `gUnk_03000E30`,
  并把 `gUnk_03000E08` 地址写入 `*arg0`。

### 卡点 (唯一, 且属 global-alloc 域)
寄存器角色整体错位一档: **目标 loop1 索引=r4、objs=r5、count=r6、loop2 索引=r6**,
我这边 **objs=r4、i=r5** (r4↔r5 互换), 以及 loop2 索引落不同寄存器。指令**形状逐条一致**。

### 已穷举无效的手段
- 局部声明序 (指针在前/在后、去掉未使用的 `s8 res` 声明) — **完全无变化** (恒 6030);
- 把两处 GetObjPool 合并成一个变量 — 更差 (6010→结构错);
- permuter (`-j 1`, 260s): 只把分降到 2930, 但**根因是把 `u16 Rng_LcgNext()` 改成 `int`**
  —— 改回 `u16` 立刻退回 6030, 即"给另一个程序打分"(经验 96)。**不可采纳**,
  `output-2930-1/` 等产物请勿直接合入。

### 判据留档
- `sub_804E9DC` 的 `%` 走 **`__modsi3`(有符号)**, 而其他已匹配调用点 (sub_802761C/sub_8034BFC/
  sub_803586C…) 走 **`__umodsi3`** → 说明别处源码显式转 unsigned, 本函数是纯 `int` 域。
- `Rng_LcgNext` 定义在 sio_link.c 返回 u16, 但 GCC2 在此处**不插零扩展**,
  所以 `u16` 原型 (code_0.h:397) 是正确的, 不要改成 int。

### 下一步建议
需要 qtydump/`-dl` 级分析 (经验 88/119/214/216/218 的 global-alloc 判定族),
或找到与 sub_804E9DC 同族的**已匹配**函数 (battle_rewards.c 里有多处 `values[8]` + 掉落循环)
抄其声明序/变量切分。悬赏见 `claim.sh --table`。

## sub_800F128 (0x0800F128, menu_ui, 技能菜单逐项绘制) — ⏸ 2026-09-10 glm (语义全解, 差纯寄存器漂移)
### 语义 (已完整还原)
arg0=memberId(gPartyMemberIds 值), arg1=cursorPos。
1. ClearBuffer(0x02005A44, 0x1C, 8) — 技能菜单 28x8 瓦片缓冲 (0xB001, 行距 0x40)。
2. gMenuCursorStack[0]==0 → 无角色分支: id=0x3E; cursorPos==0 → selVal=0x3E/palette=0xD else
   palette=0xB; !(gUnk_03002C44 & 0x80) → palette=0xC; id!=0xFF → 从 gCharacterNames[(id-1)*8]
   画角色名 8 字到 0x02005A86 (Text_PutGlyph, palette)。id==0xFF → 直达函数尾。
3. 主分支: id=memberId; id!=0 → id--; st=&gPartyStats[id]; 遍历 st->skills[idx] (idx=0..7):
   id==0xFF → break; idx++; !(gSkillLearnTable[(id-1)*5+2] & 0x80) (非双手武器) →
   skillRow==cursorPos → selVal=id/palette=0xD else palette=0xB; name=gCharacterNames+(id-1)*8;
   dest=0x02005800+((skillRow&0xFE)+0xA)*0x40+(0xD*(skillRow&1)+3)*2; 画 8 字名;
   sub_800EAE4(0x02005A80+(skillRow&0xFE)*0x40+(0xD*(skillRow&1)+0xD)*2,
   ItemGetUsePower(memberId,id), 0xE) — MP 消耗数字; skillRow++。
4. 尾: gUnk_030001C8 = selVal+0x128 (u16); *(u8*)0x030001C3 = selVal;
   gMenuCursorSel<=3 → 从 0x08098611 画描述文本 (页头 x,y,palette; 文本 0xFF 分隔, 0xFE 转义:
   Text_PutGlyph(dest, 0xFE00|next, palette); dest 逐项 +2)。

### 差异状态
fndiff 252 行差 (~126 条指令), 循环形状/池布局/sp 槽布局(5 槽同内容)全对, 剩余为纯 global-alloc
寄存器 home 漂移:
- id (0x3E/memberId/skillId 合并 web): 目标 r8 (每次赋值经 `mov r8, rX` 拷贝形态), 我侧 r3。
- palette (int→u8 均试): 目标 r9, 我侧 r8 (整体下移一位)。
- skillRow+1: 目标 sp10 (栈), 我侧 r9。idx: 目标 r4/sl 对, 我侧 ip/sp16。
- 无角色分支目标 `movs r2,#0x3e; mov r8,r2` 拷贝形不可达 (常量直落 r3)。

### 已试 (均不可达)
int/u8 类型矩阵 (id, palette), 声明序 ×3, idx++ 位置 (for 尾/0x80 检查前显式/while(1)+break),
三元 vs if-else, 合并 descId/skillId 为单变量 (关键结构 — 合并后形态更近但 home 仍 r3),
死赋值。permuter 两轮各 ~2000 迭代 (8375→7040, 7420→5940) — 分数被寄存器漂移锁死。

### 最佳候选
permuter/sub_800F128/base.c (=t6: int id 合并 + idx++ 显式前置 + u8 pagePal)。
经验线索: 经验 87/214 的"变量兼多职"已用于合并 id web, 但 global-alloc 的 callee-saved
优先级翻转仍差一档 — 可能需要增加 id web 的 refs (找源码里缺失的一次读) 或等同类案例。

## sub_800E8F8 (0x0800E8F8, menu_ui, 状态/选择框五位数计时绘制) — ⏸ 2026-09-10 gpnux (语义 100% 解, 剩 LRA 家族寄存器全局分配墙)
### 语义 (已完整还原, 结构 ~99% 对)
签名 `void sub_800E8F8(u16 *dst, s32 value, u8 flags)`; 调用点: sub_800C194(0x02005B18, *0x03001948, 0xB) 画状态窗计时,
sub_8010F10(gUnk_03000210[idx], flags) 画选择框数字。

1. **五位数 = 余数链除法**: `d0=Math_DivLoop(&value,21600000)` 然后 value%21600000 继续
   `d1=/2160000, d2=/216000, d3=/36000, d4=/3600` (Math_DivLoop 改 *ptr 为余数), 商各取 u8.
   [21600000,2160000,216000,36000,3600] = 100小时/时/分十/分 的 60fps 帧数。
2. **每位画 8×16 两格**: `w = p; v = d + 0xA2; p++` 后, 三分支 `if (v==0){} else if ((v&0xFF)==0xFE){} else {}`
   (v==0 体物理在最后 = 嵌套 if, 同族 Text_PutGlyph 证实); 每分支**各自内联** `base = flags << 12`,
   写 `*w = base + v; w += 0x20; *w = base | (v + 1)` (tile 连续上下两格, 行距 0x20 u16)。
3. **tile 判定**: 正常 `v <<= 1` (=2*(digit+0xA2)=2*digit+0x144); 0xFE 分支
   `v = ((u32)((v & 0xFF00) << 0x10) >> 0x17) | 0x280` (扩展字模基址 0x280, 死分支)。
4. **布局**: 五格写 dest+0,1,2,4,5 (u16), 第 3 位 (冒号:时:分) 不写; p 指针依次 +1+1+1,
   block4 前额外 `p++` (跳冒号格), block5 不递增。
5. 关键类型: **`int v`** (非 s16) — permuter 找到, 才让 `v <<= 1` 纯 `lsls rX,#1`、`v+1` 直接 adds、
   且差分对齐后 p→r5/flags→r6 归位 (s16 时 p/flags 互换)。`base` u16, `p`/`w` u16*。

### 卡点 (唯一已定位, 属 global-alloc 域, 与 sub_800EAE4/sub_8010F10 同族)
fndiff 1945 分 (~29 条系统性差异, 全是指令形状对而寄存器错位):
- **flags<<28 CSE 临时的 home**: 目标在块1三路各算 `lsls r6,#0x1c` 后**都**存进 r2
  (`adds r2,rX,#0`), 块2-5 全用 `lsrs r?,r2,#0x10`; 我侧只 normal 路存 r1 → 块2-5 用 r1。
  r1/r2 一差, 每块的 w(base/v/w) 三寄存器整族下移: 目标 {w:r1,base:r4,v:r3}, 我侧 {w:r2,base:r3,v:r4}。
- **p++ 经 w 计算**: 目标 `adds r5,#2` (原位), 我侧 `adds r5,rX,#2` (用 w 拷贝做基)。
- **0x280 物化**: 目标 `movs r6,#0xa0; lsls r6,r6,#2; adds r0,r6,#0; orrs r4,r0` (多一条 adds 拷贝, 经验 31),
  我侧 `movs r0,...; orrs r4,r0` (3 条, 少拷贝)。
- 序言 `str r1,[sp]` 先于 `adds r5,r0` (目标 p 先), 五个除法中间无差异。

### 已穷举 (均不达 0)
- 双指针 p/w: `w=p; p++` vs `p++; w=p` × 全 6 排列 (brute.py 全试, 序言 r1/r2 不变)。
- base 位置: 三分支内联 / 每块顶外提(w2, homes 对但块2为 reload 目标不 reload) / 块1 一次全局 (vbase)。
- v 类型: s16/u16/int; `v=d; v+=0xA2` 两语句 vs 单表达式。
- `flags<<12` 显式 `(u16)((u32)flags<<28>>16)` 移域形式 (2130 分)。
- 内联 store `(flags<<12)+v` 去 base 变量 (5875 分)。
- 嵌套 if vs if/else-if 平铺 (flux 翻转 905 规律同族)。
- `s32 *vp=&value` 先取地址 / 声明序 ×3。
- 0xFE 拆分 3 语句 (v&=0xFF00; v=...; v|=0x280)。
- permuter 两轮 ~14000 迭代 (10975→1945)。-g 变体无效。
- 关键取舍实测: "base 每块外提" 唯一买到 p=r5/flags=r6 正确 homes 但块2 重算 base (目标不复算);
  "base 分支内联" 结构全对但 p/flags 互换 (w2 vs n1 的 10495 vs 3130)。

### 下一步建议
flags<<28 临时落 r2 是全局 key (经验 87/214 的变量兼多职制造生死边界方向): 试令块1 base 赋值
显式为 `u32 h = (u32)flags << 28` 且 h 在块2-5 以 `h >> 16` 消费 (让 gcse 把临时固定到块1汇聚点
而非块2); 或接受 n1 结构 (3130) 用 permuter 长时间压寄存器梯度。同族 sub_800EAE4 停 1670 分,
此家族 (LRA) 为公认硬区, 建议转挂起留档。

### 最佳候选
`permuter/sub_800E8F8/base.c` (permuter output-1965-1, fndiff 1945 分, 语义等价、结构逼近目标)。

## sub_804473C (0x0804473C, battle_engine, 战斗物件状态服务分派) — ✅ 2026-09-10 wb (permuter score=0, fncheck OK 364B, ROM SHA1 绿)
### 语义 (已完整还原)
战斗物件 (obj) 的状态服务总入口, 被 MOD-06 hub `sub_803F5B4` 在遍历物件池时逐项调用
(参数2 = `pool + (slot_id & 0xF) * 0xC8`, 即 0xC8 字节的池 entry)。
- `arg0[0xBE] > 0xA` → 直接走 `sub_8044A40` (默认/兜底服务)。
- 否则按 `(s8)arg0[0xBC]` 三分派: `0`→A40 / `1`→按 `sub_8048764(arg0)` (技能/招式索引) 再细分 / `2`→`sub_8045098`;
  其余值 (负数或 >2) 落到"未初始化 result"返回 (源码 switch 无 default)。
- 内层 `switch (sub_8048764(arg0))` (u8, 表 0..59): `0,1,2,38,39,40,42,48,53,55,58,59`→A40,
  `17,18,19,20`→返回 0, 其余 (含 >0x3B)→F4C。
- 全部经共享 `u16 result` 汇聚, 函数末尾一次 `return result;` (函数本体返回 u32)。

### 结构判据 (一次到 0 的关键, 见 EXPERIENCE 经验 222)
1. **单一 result 变量 + 末尾一次 return**: 目标 `_0804489C` 是被 A40/F4C/5098 三个 call 块共享的
   `lsls r0,#0x10; lsrs r2,#0x10` (u16 截断) + `_080448A0` epilogue (`adds r0,r2,#0`)。
   写成"每分支自带 return"会出现多份转换块 (sen1 的 cand1..cand6 全部 4550 分即此因)。
2. **case 0 与外层 if 的相同 call 跨块合并**: `result = sub_8044A40(arg0,arg1)` 在 case 0 和
   `arg0[0xBE] > 0xA` 分支重复 → GCC cross-jumping 合并成末尾一块 (0x08044894), case 0 的
   dispatch 直接远跳过去; 源码顺序仍是 0,1,2, 无需手写 goto。
3. **内层不要画蛇添足写 `if (idx > 0x3B)`:** 目标里 `cmp #0x3B; bls 表; b F4C` 就是 switch 表驱动
   的边界检查 + default 合并, 手写外层 if 反而多出一个分支 (sen1 cand6 即此)。
4. **被调原型**: `sub_8044A40/sub_8044F4C/sub_8045098` 的赋值处有 `lsls#0x10` 截断 → 返回类型是
   32 位, code_0.h 由 `void` 改 `u32`。`sub_804473C` 本体保持 `u32` 返回 (改 u16 会让已匹配的
   `sub_803F5B4` 多出一次零扩展而破坏其匹配)。
5. **校验捷径**: permuter/bytecmp 的 target.o 其 bl 是未解析占位 (`f7ff fffe`), 候选 compile.sh
   **不要**给被调符号加 `.equ` —— 两边同为占位时, 直接 `objcopy -O binary --only-section=.text`
   后 `cmp` 即可 0 差异 (364B); 加了 `.equ` 反而 16B (4×bl) 差异 + 链接后长度 432≠364, 造成假 DIFF。

### 产物
- `src/battle_engine.c`: INCLUDE_ASM → 真 C (39 行 switch 结构)。
- `include/code_0.h`: 3 个被调原型 void→u32。
- `functions.tsv`: status 0→1。
- permuter 套件保留 `permuter/sub_804473C/base.c` (score=0 种子); sen1 遗留 cand1..cand6 已无价值。

## sub_80497B0 (0x080497B0, battle_engine) — ✅ 2026-09-10 zcode-8024618

### 语义
数字翻牌显示 (战斗奖励/HP 变化等数字滚动 UI 的底层):
把 `arg1` (u16) 拆成 5 个十进制位 `digits[0..4]` (万/千/百/十/个) 存栈上数组;
引导循环跳过前导零, i 指向首个非零位; `i` 加上全局翻牌计数器 `gUnk_0300094C`
(u8 回绕) 得到本次显示位, 写 `arg0[0]` / `arg0[0x20]` 两行 tilemap
(值 = `digit*2 - 0x4EBC / -0x4EBB`, 即 0xFFFFB144/5 + digit*2), 计数器 ++。
返回: val==0 → 1 (立即完成); i>3 → 1 (全部位播完); 否则 0 (逐位动画进行中)。
调用点: `sub_8049420` 的 kind 分发表 (战斗 UI 边框数字)。

### 关键匹配点 (一次 fndiff 回环即 3 字节收尾)
1. **参数 `u16 arg1`**: 入口产生 `lsls #0x10; lsrs #0x10` 截断 (u32 参数没有)。
2. **除法双通道**: 第 1 次 `(u32)val / 10000` → `__udivsi3`; 其余 3 次 `(int)(...)` →
   `__divsi3`。混用任何一个都差 2 字节 bl 编码。
3. **中间和括号全式**: `val - (d0*10000 + d1*1000 + d2*100)` 形式。链式
   `val - d0*10000 - d1*1000 - d2*100` 会被 GCC 拆成多次 `subs r0,r7,r0` 并 CSE 复用
   `val - d0*10000` 子树 (与目标逐项 muls 重算 + adds 累加 + 单次 subs 的形状冲突,
   差 88 字节)。(经验 209 姊妹形: 括号阻止重关联在这里是**必须**而非可选。)
4. **前导零循环计数器必须 u8**: `lsls #0x18; lsrs #0x18` 截断让 GCC 无法证明 i 单调,
   保留数组寻址 (`lsls r0,r4,#1; add r0,sp; ldrh`); u16/u32 计数器会被强度削减成
   指针步进 (`adds r1,#2`), 形状全变 (88→163 字节差的分水岭)。
5. **清零条件 `if (i > 4 && val == 0)`**: && 不是 ||。目标块流 = i>4 → cmp val →
   val==0 才清零; || 会把 i>4 直接 bhi 到清零 (跳过 cmp val), 差 11 字节。
6. **尾部 3 return 顺序**: `if (val == 0) return 1; if (i <= 3) return 0; return 1;`
   决定 ret1@10c (近) / ret0@120 (远) 的块布局; 写成 `val!=0 && i<=3` 合并形会把
   ret0/ret1 块位置互换 (差 11 字节)。
7. **tile 常量写 `x - 0x4EBC / -0x4EBB`** (非 `+0xFFFFB144/5`): 直接加会截断成
   0x0000B144 池条目, 目标池是 0xFFFFB144/5 (经验 209 姊妹形, 同 sub_8024618 的
   -0x4FFF 技巧)。
8. 原型: `code_0.h` 的 K&R `u32 sub_80497B0();` 必须改全原型
   `u32 sub_80497B0(u16 *arg0, u16 arg1);` — agbcc 对"空参数表 K&R 声明 + 带参定义"
   报 `can't match an empty parameter name list declaration`。

### 产物
- `src/battle_engine.c`: INCLUDE_ASM → 真 C (43 行)。
- `include/code_0.h`: sub_80497B0 原型 K&R → 全原型。
- `functions.tsv`: status 0→1 (780/1059)。
- permuter 套件 `permuter/sub_80497B0/` (手工搭建: mkpermuter 不支持 INCLUDE_ASM 占位)。
- fncheck OK 302B; make verify 780/780 OK; SHA1 通过。

## 2026-09-10 sub_80448A8 匹配 (战斗结果技能恢复量分发, battle_engine, 408B)

**语义**: `stats = *(obj+0x88)`; 默认 `kind = stats[0x2F]`, `(s8)obj[0xBC]==1` 时改取
`stats[0x30 + obj[0xC2]]`; `switch(kind)` 0..54 跳表分发到 `sub_8044A40(obj,arg1)`
(恢复量基准), case 12/34/38/41 结果 +0xF, case 14/37/40 结果 +0x1E, 其余原值;
u16 截断后写 `gUnk_03000908` (新登记 IWRAM) 并返回。恢复量与 skill id 联动 =
 HP 恢复类技能的效果量分发 (部分技能档位恢复 +15/+30)。

**过程**: 头部 `(s8)obj[0xBC]` 双 cmp + case1 取址用 permuter/exp-11 "结构体成员访问"
形状 (stats->unk30[obj->field_C2]) 直接命中; result/kind 的 r1/r4 home 互换穷举
10+ 声明/顺序/类型变体无效, permuter 从 1415 压到 5 分平台后靠 **`do{}while(0)`
整体包住 switch** 的屏障变体拿到正确 home (while(kind)break/置于 case 间/置于 switch
后全部无效 — 见 EXPERIENCE 223)。

**池计分伪差印证 (exp-29 permuter 侧)**: 人工修正版用 `extern u16 gUnk_03000908`
→ permuter base score = 5 (池字重定位计分); 换 `#define gUnk_03000908 (*(u16*)0x03000908)`
→ score = 0; 两版 bytecmp 字节完全一致 (bl 槽伪差外 0 差)。合入 src 用具名符号
(linker.ld+iwram.h 登记), fncheck OK 408B。

**产物**: src/battle_engine.c 真 C (99 行); code_0.h `void sub_80448A8()` → `u32`
(返回值 lsls#0x10 截断自证; 无已匹配调用方, 安全); linker.ld/iwram.h 登记
`gUnk_03000908`; functions.tsv status 0→1 + note。

## sub_80485A4 (0x080485A4, battle_engine, 战斗对象允许判定) — ⏸ 2026-09-10 zcode-main 挂起

**语义全解** (109 行, 3 实参 `(u8 *obj, u8 mode)` → `u8`; 调用方 sub_8044A40 传 mode=1/2):

三分支判定函数, `be = obj[0xBE]` (战斗对象类型字段):

1. `be <= 0xA` → 直接 `return 0` (基础职业不适用)
2. `(u8)(be - 0xC) <= 0x64` (0x0C..0x70 区间) → 查表 `gUnk_0839D5BC`:
   `(s8)obj[0xBC]==0` 取 `tbl[k2*6+4]`, `==1` 取 `tbl[k2*6+5]` (k2=be-0xC), 其余取常量 0x32;
   与 `mode` 相等 → ret=1。表布局 = 每条目 6 字节, +4/+5 是两个允许标志位
   (与已匹配姊妹 sub_8048C30 同表, 该函数判 `(flagA==1 || flagB==1)`, 本函数判 `flag == mode`)。
3. `be > 0x70` → 17 项跳转表 switch (0x71..0x81):
   0x7D/0x7E 直接置 1; 0x76/0x7B/0x7F 依赖内层 `switch((s8)obj[0xBC])`(0=空, 非0=置1);
   0x71-75/77-7A/80/81 显式 `ret=0` (jump table 有 11 项指向 `movs r3,#0`); 0x7C 缺席 = 落到比较末尾 ret 保持 0。

**关键代码生成发现**:

- 第三分支 0x7C 处理: 目标 `70: cmp r4,#0x70; bls 尾; 74: ldrb r0,[r1]; subs #0x71; cmp #0x10; bhi 尾; lsls #2; mov pc,r0`
  = GCC2 的 switch 双段守卫。跳转表 17 项里 11 项显式 `ret=0`、3 项内层 switch、2 项 `ret=1`,
  **源码必须显式写 ret=0 组** (写成无 default 的 switch + 跳转表外落入) 产生同样 jump table。
- 内层判定必须用 **switch 而非 if**: `if ((s8)obj[0xBC])` 形式 combine 会消掉
  `lsls #0x18; asrs #0x18` 符号扩展对 (目标尾部 0xd6-0xd8 有), `switch` 分发保留符号扩展 (b23/b39 实证, 差 32-39B)。
- 表地址读取形态: 每个 case 内 `const u8 *tbl = gUnk_0839D5BC; int k2 = ...; int idx = k2*6; const u8 *p = tbl+4;`
  (与 sub_8048C30 成功模式逐句对应) — GCC2 对 extern 符号不折叠 `+4` 进池, 生成运行时 `adds r2,#4`;
  任何把 `(be-0xC)*6` 与 `+4` 合并成一个表达式的写法都被 flatten 归到最左项 (经验 30), 产生错误池形态。
- `be` 值不能缓存为显式局部变量 (`u8 be = obj[0xBE]`): 会让第二次比较 `(u8)(obj[0xBE]-0xC)`
  复用缓存值 → 丢失目标里的重读 (`ldrb r0,[r1]` 第二次), 全函数移位 (v_b/v_c 实证, 差 128-150B)。

**卡点 (7 字节, 指令形状 100% 对齐)**:

mine 与目标仅入口/中段/尾部 7 字节差 = 两个伪寄存器 home 恰好互换:
```
mine:   8: movs r4,#0 (ret→r4)   10: adds r3,r0,#0 (be→r3)   70: cmp r3,#0x70   e0: movs r4,#0
target: 8: movs r3,#0 (ret→r3)   10: adds r4,r0,#0 (be→r4)   70: cmp r4,#0x70   e0: movs r3,#0
```
- agbcc `-da` greg 量化 (经验 117 方法): `be (allocno 33): 3 refs / live 10 → pri = 3000`;
  `ret (allocno 29): 6 refs / live 132 → pri = 909`。降序发号 → be 先挑, 拿 r3; ret 拿 r4。
- 目标要求 ret=r3/be=r4 → 需 pri(ret) > pri(be), 即 be 的 live > 33 或 ret 的 refs ≥ 14 (floor_log2 跳档 3)。
- **穷举 40 种人工变体全部无效**: be 提前读 (b13/b34, be_addr 形状破坏), be 显式缓存 (v_b/v_c, 丢重读),
  ret 提前 return (b21, CSE 吸收), ret 双重零赋值 (b35/b38, DCE 或 obj home 错位), ret|=1 (会产生 orrs 破坏 movs 形状),
  内层 case 0 显式 ret=0 (b37, GCC2 重写 switch 为 ldrsb 形态), do-while 屏障 (b22/b36, 持平无效),
  cast 组合 (b29-b31, CSE 吸收), 第二指针转发 (b16/b27, 持平), else-if 链 (b19, 持平) 等。
- be 的 live=10 由两个短活跃区间构成 (定义→第一次cmp 3 insns + CSE 复活→第二次cmp 7 insns),
  中间无任何合法语句可插入引用 (第二比较必须重读, switch 分发也重读) — live 无法合法拉长。
- 4 轮 permuter (3 轮无-g + 1 轮带-g, 13.5k 迭代) 平台期 score 45, 变异算子对 global-alloc 槽位无杠杆。

**给接手者的路径**:
1. 套件已建好 `permuter/sub_80485A4/` (compile.sh 已修为 -g 版, base.c = 最优候选 v_n 人工化)。
2. 最优候选 bytecmp = `DIFF: 7 bytes / 236` — 先复现: `scripts/bytecmp.sh sub_80485A4 permuter/sub_80485A4/base.c "gUnk_0839D5BC = 0x0839D5BC;"`。
3. 方向: 寻找使 ret 的 allocno pri 升到 3000 以上 / be 的 allocno live 拉到 33 以上的**人类可读**写法;
   或确认目标源码的 ret/be 声明顺序与逻辑结构与已试 40 种根本不同 (如 ret 非局部变量而是参数转发? 罕见)。
4. 若仍无解, 考虑与同表的 sub_8048C80 (已匹配, 同款三分支头) 对照其 ret/入口变量的写法。

## sub_8044F4C (0x08044F4C, battle_engine, 战斗伤害结算) — ✅ 2026-09-11 wb (permuter score=0, fncheck OK 332B, audit 784/784)
### 语义 (已完整还原)
战斗伤害结算。arg0 = 攻方对象, arg1 = 守方 (pool + entry*0xC8)。

| 阶段 | 行为 |
|---|---|
| 前置① | `*(u16*)(arg1+0xB0) & 0x1000` → 清该位 (&=0xEFFF) 并 return 0 |
| 前置② | `*(u16*)(arg1+0xB0) & 0x10` → 把 (hw&0x1000) 写 `arg1+0xB2` 并 return 0 |
| 随机项 | `mod = ((u32 (*)(void))Rng_LcgNext)() % (sub_8047024(arg0,8) / 10)` |
| 基础伤害 | `dmg = (sub_8047024(arg0,8) + v*arg0[0xAA] + mod - w9) / 2`, `v = sub_80472E8(arg0,(u8)sub_8048764(arg0),0)`, `w9 = sub_8047024(arg1,9)` (有符号除 2) |
| 属性克制 | `switch (sub_8047D28(arg1, sub_8047DC8(arg0)))`: 1→×2, 2→(s16)dmg/2, 0→noop |
| 暴击 | `arg0+0xB0 & 0x4000` → ×2 并清位 |
| 加成 | `sub_804E6DC(arg0,0xC) >= 0` → +10% |
| 钳位 | `<0 → 0`, `>999 → 999` |

### 收敛过程 (1865 → 850 → 630 → 340 → 245 → 0)
1. **Rng_LcgNext 强转调用** (850): 声明是 `u16`, 直接 `Rng_LcgNext() % x` 会多插零扩展;已匹配代码用
   `((u32 (*)(void))Rng_LcgNext)()` (battle_rewards.c:37) —— 照抄即消失。
2. **switch 要显式 `case 0:`** (245): 目标判定树多一条 `cmp r0,#1; ble`, probe 证明只有 case 1/2 时不出这条;
   加 `case 0: break;` 完全复现。
3. **sub_8047024 调用结果与变量同宽**: `u16 f()` 的结果参与算术要零扩展 (probe_f 证实);
   用 `u16 sub_8047024()` 声明 + `u16` 变量即得到目标的 `bl → lsls/lsrs → __udivsi3` 形状。
4. **`u32 bit`** (245): `u16 bit = *p & 0x1000` 出"先截断再搬";改 u32 即"截断直接进 home"。
5. **`v` 中间变量** (630→340): `dmg` 连赋两次会改变转换落点, 拆出 `v` 才对齐。
6. **+10% 必须 `s16 t` 中转** (245→0): 见经验 227③。

### 产物
- `src/battle_engine.c`: INCLUDE_ASM → 真 C。
- `include/code_0.h`: `sub_8047024` void→u16, `sub_80472E8` void→u32, `sub_8047DC8` void→u8 (均无其它 C 调用者)。
- `functions.tsv` (status 0→1 + note); 经验归档 **226/227**。
- 认领已释放, `permuter/sub_8044F4C/base.c` 保留 score=0 种子。

## sub_804AF60 (0x0804AF60, battle_anim, OAM 扫描线精灵显隐) — ⏸ 2026-09-11 claude_1 挂起 (best 940 忠实 / 800 带块嵌套技巧)
### 语义 (已完整还原)
按 VCOUNT 把一批精灵的 X 坐标在"表值"与"移出屏幕"之间切换 (HBlank 光栅效果), 由 `sub_801887C` 每帧调用。

| 阶段 | 行为 |
|---|---|
| 闸门① | `gUnk_03000ADE & 2` 为 0 → return |
| 闸门② | `(gUnk_03000ADE & 0xF0) != 0x10` → return |
| 窗口 | `vc = *(u8*)0x04000006`; `gUnk_03000ADB > vc` / `gUnk_03000ADC < vc` → return |
| 基准 | `base = (u16)(gUnk_03000ADC - gUnk_03000ADD)`; `base > vc` → return |
| 扫描线 | `vc` 命中 `base + {1,3,6,13,18,23,30}` 之一 **且** `gUnk_03000AD8 & 1` → 显示分支 |
| 显示 | `for (i=gAD9; i>gADA; i--)` 把 `oam[i].attr1` 低 9 位换成 `gUnk_030009D8[i] & 0x1FF`; 然后 return |
| 隐藏 | 同一循环写 `oam[i].attr1 = (attr1 & 0xFFFFFE00) | 0xF0` (X=240 移出屏幕) |

`oam` = 硬件 OAM `0x07000000` (`0xE0 << 19`), 条目 8 字节, `attr1` 在偏移 2。

### 收敛过程 (3290 → 2840 → 1775 → 1430 → 1310 → 1130 → 940 (800))
1. **标志判据不能合并** (3290→2840): `(x&2)!=0 && (x&0xF0)==0x10` 被 fold_range_test 合成 `(x&0xF2)==0x12`;
   写成两个独立早返回 if 才出目标的两段判据。(同族 `sub_804AE2C` 用 `&&` 没问题, 因为它是 `& 1`。)
2. **0xFFFFFE00 收窄** (2840→1775): 见经验 228① —— 必须先 `u32 v = oam[i].attr1; v &= 0xFFFFFE00; v |= ...; oam[i].attr1 = v;`。
3. **非 volatile 读 VCOUNT** (1775→1430): `*(u8 *)0x04000006` 让全局 CSE 把 3 处读合成 1 处。
4. **for 循环** (1430→1130), **表指针提到循环外** (1130→940), **循环体包一层 `do{}while(0)`** (→800, 伪改进)。

### 卡点 (剩 9~10 条归一化指令差)
- **头**: 第 3 次 VCOUNT 比较时我这边重新 `ldrb` 了一次 (目标把值一直留在 r1 复用);
  试过 `vcount` 局部变量 (1450)、非 volatile 变量 (1450)、`(char)/(u8)` 强转 (1130) 均无效。
- **show 循环**: 目标 hoist 顺序是 `ip=表基址 / r6=0x1FF / r7=掩码` 且**先载表后载 attr1**;
  我这边是 `r6=掩码 / r7=0x1FF` 且先载 attr1。"表先"的各种写法 (`v = p[i]&0x1FF; v |= attr1&M;`、
  双 u32 临时 `v/w`、`p` 在循环内外) 全部更差 (1970~2385)。
- **high-reg 对调**: 目标 `r8=OAM 基址, ip=表基址`; 加 `do{}while(0)` 后我这边会**反过来** (`ip=OAM, r8=表`)。

### 给接手者的路径
1. 套件已建好 `permuter/sub_804AF60/` (compile.sh 已加 8 条 `.equ`, base.c = 800 候选即 `dw1.c` 形态)。
2. 忠实最优候选 = `.scratch/claude_1/p_pBeforeLoop.c` (940, 无伪改进); 800 版 = `.scratch/claude_1/dw1.c` (含 do-while(0))。
3. 优先攻**头的重复 ldrb** (2 字节) —— 它还会连带消掉两处池填充 padding。
4. 再攻 show 循环的 0x1FF/掩码寄存器对调: 目标是 `ldr r0,=0x1FF; adds r6,r0,#0`(带一次搬寄存器) + `ldr r7,=掩码`(直达),
   说明 0x1FF 的 home 不是直接可载的低寄存器, 可能源里 0x1FF 来自某个表达式/变量而非常量。
5. 别信 permuter 的 `output-795-1` (空 `if(1){}`) 与 `output-715-1` (`new_var = oam[i]` 死结构体拷贝) —— 均为伪改进。

## 2026-09-11 sub_80480EC 挂起记录 (battle_engine, 104 asm 行, zcode)

**语义全解**: `ret = 0x32` (默认值 50); `pool = GetObjPool()`; `buf[5]` (sp+0..4) 清零;
收集循环: `for(i=0;i<5;i++)` 中 `sub_8045F10(pool+i*0xC8, 0x1FF)==2` 的槽下标填进
`buf[found++]`; 搜索循环: 找第一个 `pool[buf[j]*0xC8+0xBE] <= 1` 的槽, 返回其
`pool[buf[j]*0xC8+0xAA]` (等级字段), 找不到保持 0x32。语义 = 从活动战斗槽中
找首个等级≤1 的新单位, 返回其等级 (0/1), 无则 50 (表示无新单位)。

**卡点 — count/stride 双变量 r8↔r9 home 互换**:
- 目标寄存器域: pool→r7, &buf→r6(sp拷贝), found→r5, i→r4, ret→sl, **count(5)→r9, stride(0xC8)→r8**, 搜索下标→r2, found拷贝→r3 (`adds r3,r5,#0`)。
- 我方最优 (v29): 全部一致**除了 count→r8 / stride→r9 互换** + stride 早生于 GetObjPool 之前 (目标在零循环后才赋值)。fndiff 3105 / permuter 平台 2075。
- 已穷举: stride 类型 (u8/u16/s32), 声明顺序 7 种, 初始化位置 (ret 前/ret 后/GetObjPool 前/后/for-init/comma), 乘数顺序 (i*stride/stride*i), 独立搜索下标 j (有效: 3135→3105, r2/r3 形状归位), count int 化, buf 指针化 (有效: pool→r7/&buf→r6 命中), 防循环2 CSE (u32 cast 重算) — 均未翻转 r8/r9。
- stride 必须出生在 `GetObjPool()` 调用前才能保住高位寄存器 (v17 迟生 → 跌落 r2 + `str r2,[sp,#8]` spill); 但目标确实在 bl 后才赋值仍拿 r8 — 说明目标源码里 stride 的 QTY 排序高于 count, 而 GCC2 的 QTY 计数受 CSE 影响 (目标循环2无 CSE, 两次独立 muls; 我方 CSE 合并一次 → refs 减少 → QTY 降低)。u32 cast 防重算未成功。
- permuter 两轮 ~24k 迭代: 3655→2075 平台; 其最优解靠 `new_var/new_var2` 拷贝变量 hack (不忠实, 疑改数据流, 不可用 — 经验 18)。

**候选**: `permuter/sub_80480EC/candidates_v29.c` (j 独立搜索下标 + stride 早生版,
与目标仅差 count/stride 的 r8/r9 互换 6 条指令); `candidates_v22.c` (i 复用版, 3135)。
**待攻方向**: ① 找到阻止循环2 CSE 的写法 (目标两次独立 `movs #0xc8; muls` — 若复现可抬 stride QTY);
② GCC2 QTY 计算 (local_alloc.c) 对照 -dl RTL 转储定位 stride 伪寄存器的排序输入;
③ 若 stride QTY 抬过 count, 需要同时让 GCC 把 movs 指令调度到 bl 后 (agbcc 无调度器, expand 顺序即源序, 需 C 层面晚生 + 全局分配早占 — 可能本质矛盾, 或需第二个跨调用变量挤占 r2/r3 窗口)。

## 2026-09-11 sub_8032548 匹配 (NPC 对话状态机, event_hub, 484B)

**语义**: `switch(gUnk_03000820)` 10 态分发 (NPC 对话流程):
- case 0: 存 NPC 屏位 `gUnk_03000828/29 = arg0[0xBF]/[0xC0]`, `gUnk_03000825 = 0`,
  `sub_80444A4` + `sub_803F5B4` 初始化, 清 `arg0[0xB4]/[0xB6]` (u16 对),
  `gUnk_03000820 = 1`, `gUnk_0300086B = 0` (新登记 IWRAM 符号, 对话子状态)。
- case 1: `sub_803E58C(arg0,arg1,0) == 1` (对话对象到位) → 播开场动画
  `sub_8020974(arg0+0xC, b4, 0x1B4, 0xD, 2)`, `b4 = 0x368`, 若 `arg0[0xBE]==0` 则 `-=0xF`
  (=0x359, 两种 NPC 门样式); `arg1[0xBE] <= 0xA` 时 `arg0[0x24] |= 0x20`; 状态→2。
- case 2: 等 `[arg0+0x24] & 0x800` 清零 (动画完成) → 3。
- case 3: 按键分发: `[arg0+0x28]==0x21` (A 键?) → `Sfx_Play(0x31,1,1)` + `sub_8044514(0x5E)`;
  `==0x7C` → `Sfx_StopTrack(1)`; `==0x90` → `sub_8044514(0x28)` + 状态→5。
- case 5: `[arg0+0x24] & 0x1000` (选择确认) → `sub_804C3A4(arg0[0x35], sub_801B954(arg0+0xC))`
  (选项结果入队), `gUnk_0300086B = 0xC`, 状态→8。
- case 8: `sub_803E58C == 1` → 9; case 9: `gUnk_03000844/45/56 == 0` (无对话遮挡) 时
  `sub_8045B90(arg0, arg0[0xA1])` (对象退场) 并 `ret = 1`。尾部每态 `sub_803F658(arg0)`。

**匹配关键 (4 个非平凡点)**:
1. **case 0 双零变量**: 目标把 `B6/B4` 清零与 `086B=0` 用**两个不同零寄存器** (r1 stores / r2 to-086B)。
   单一 `zero` 变量被 GCC2 CSE 成一个 movs; 必须 `zero = 0; zero2 = 0;` 两个独立变量
   (zero2 供 stores, zero 供 086B), 且 `b4 = zero2` 中转 (permuter 发现, 借 b4 的 home 调度)。
2. **b6ptr 指针变量**: `b6ptr = (u16*)(arg0+0xB6); *b6ptr = zero2;` 与直接
   `*(u16*)(...)` 的池加载调度不同 — 指针变量让地址计算提前一条 (对齐目标 78 行)。
3. **case 1 参数序**: `sub_8020974(arg0+0xC, b4, 0x1B4, 0xD, 2)` — 参数 2=动画 id (b4),
   3=0x1B4 时长。最初写反 (0x1B4 在前) 导致 r1/r2 参数寄存器互换、连锁 home 漂移 (6260→5885)。
   同族已匹配 `sub_803586C` 的 `sub_8020974(arg+0xC, 0x386, 0x1B4, 0xD, 2)` 证实参数序。
4. **case 1 keys/b4 寄存器序**: `keys = arg0[0xBE]` (u8 读) + `b4 = 0x368` 紧随 +
   `if (keys == 0) b4 -= 0xF` — 顺序错则 movs 先于 ldrb (6260); keys 必须独立命名变量
   (内联读会让 GCC 先物化常量)。

**permuter 70 分 = 池计分伪差 (经验 29)**: 剩余 9 个池字 (0x03000820/25/28/29/6B/844/845/856
等地址值) 在 permuter 侧未重定位被计分; 指令级 fndiff 全同。合入后 fncheck OK 484B,
全量 make + SHA1 绿。battle_rewards 家族 (permuter 5 分) 先例一致。

**产物**: src/event_hub.c 真 C (110 行); code_0.h `sub_8032548()` → u32、
`sub_803E58C()` → u8 (两原型无已匹配调用方, 消费方 lsls#0x18 截断自证);
 linker.ld/iwram.h 登记 `gUnk_0300086B`; functions.tsv status 0→1。

---

## sub_804B080 (0x0804b080, battle_anim, 186行) — ⏸ 挂起 2026-09-11 (opencode)

**语义已全解, 卡点 = 纯寄存器分配 (permuter 到不了 0)**。候选 `permuter/sub_804B080/human_candidate.c`
(人类可读, 归一化指令 diff 169 行全是 callee-saved 寄存器换号, 语义/跳转表/访存形态全对)。

**函数原型**: `s32 sub_804B080(u8 *obj, u8 index, u16 flags)`。r2(flags) 入口 `lsls#0x10;lsrs#0x10`
→ u16; r1(index) 入口 `lsls#0x18;lsrs#0x18` → u8 (存进 sb/r9)。返回 `(index-1)&0xff`
(所有早退都 `return index`, 末尾 `index=index-1` 后统一尾返回, 供调用方 sub_8018500 递减循环)。

**逻辑**: 若 `(flags&0x100)==0` 或 `*(u16*)(obj+0xB0)&4` 或 `sub_8045F10(obj,0x6E)!=2` → return index。
否则 `cnt=obj+0xA8`: 若 `flagword&0x200` 则 `cnt++`, 进位过 0xF 清 0x200; 否则 `cnt--`, 归 0 置 0x200。
`sh = *cnt>>2`; `switch(obj[0xAB])` 选 tile (见下); 写 OAM 表项 `OAM_BUF[index]`:
`VPos = obj[0xC0]-(sh+0x22)`, byte1(AffineMode/ObjMode/Mosaic/ColorMode/Shape)=0,
`HPos = obj[0xBF]+4`, HFlip=VFlip=0, Size=1, `CharNo=tile`, Priority=1, Pltt=0xF。

**两个关键发现**:
1. **sub_8045F10 返回在此函数不截断**: 全 ROM 31 处 `bl sub_8045F10` 里, 唯此处在 `cmp r0,#2` 前
   **没有** `lsls#0x18;lsrs#0x18` (其余 30 处都有, 含同 battle_anim.c 的 0x0801C... 两处)。
   → 原 TU 在此调用点看到的是 **非 u8 返回原型**。合入时必须用
   `((s32 (*)(u8*,u16))sub_8045F10)(obj,0x6e)` 函数指针强转抑制零扩展, 否则多 4 字节 (待验证)。
2. **switch(obj[0xAB]) 是 6-case 跳转表, 不是 obj[0xAB]-1**: 直接 `switch(obj[0xAB])` 且必须
   **写出 case 2、case 4 到 default 同值 0x174** (6 个 case 标签才触发 agbcc 跳转表; 只写
   {1,3,5,6}+default = 4 标签 → agbcc 出决策树 bgt/cmp, 与目标 `subs#1;cmp#5;bhi;mov pc` 全不符)。
   映射: 1→0x180, 2→0x174, 3→0x184, 4→0x174, 5→0x178, 6→0x17c, default→0x174
   (0x180=0xC0<<1 等, agbcc 自动 movs+ lsl#1)。min-case=1 → `subs#1` 偏移, 表 6 槽, 索引=value-1。

**卡点 (为什么 permuter 到不了 0)**: best score 2350 (跑 ~15k 迭代, 稳定平台)。残差 100% 是
callee-saved 寄存器着色互换: 目标 `obj→r7 / index→sb(r9) / tile→r6`, 我这版 `obj→sb / index→r7 / tile→r7`;
且目标把 4 个位段掩码常量 (`-13=~0x0C`→r4, `-17=~0x10`→ip, `-33=~0x20`→r5, `0x3F`→r8) 提前物化进
callee-saved 复用, 我这版就地重算。语句重排 (permuter 唯一能力) 不改 liveness, 翻不动着色 →
经验 87 类"调度槽位"可解, 此属"结构/着色"permuter 救不了 (AGENTS §2b)。试过的翻车变体: index/obj
声明序互换、u8/u32 index、`fl` 提前物化、`&buf[i]` 取址写法 —— normalized diff 均 169~171 无改善。

**下一步候选路**: (a) 手改 `sub_8045F10` 强转后重新 fncheck 看池布局是否带动着色 (合入前禁改 src,
先在 permuter 内验证强转的字节); (b) 找同结构已匹配 OAM 写函数抄寄存器序; (c) 换目标。

## 2026-09-11 sub_803272C 匹配 (战斗型 NPC 对话状态机, event_hub, 540B)

**语义**: `switch(gUnk_03000820)` 十态, 与 sub_8032548 同族 (战斗触发的对话):
- case 0: 同 8032548 (存位/初始化/清 0825/086B)。
- case 1: `sub_803ED34(arg0,arg1,0)==1` (战斗对象到位, 原型 u8) → 动画 id `b4=0x36B`,
  `arg0[0xBE]==0` 时 `-=0xF` (0x35C); `sub_8020974(arg0+0xC, b4, 0x1B4, 0xD, 2)`;
  `[arg0+0xB6]=0x35E` (战斗对话 UI 序号), `[arg0+0xB4]=0`; `arg1[0xBE]<=0xA` 置 0x20 → 2。
- case 2: 等 0x800 → `Sfx_Play(0x4F,1,0)` + `sub_8044514(0x28)` +
  `sub_804BF14(0,3,7,0xE, 0x1C,4,4,-1,2)` (9 参窗口/对话框设置) + `0825=0` → 3。
- case 3: `gUnk_03000825 <= 3` → `sub_804C728(0,3,0x10)` (对象命令标记) → 5。
- case 5: 0x1000 确认 → `sub_804C3A4` + `086B=0xC` → 8; case 8/9 同 8032548。尾 `sub_803F658`。

**匹配关键 (4 个非平凡点, 均为 case1 的寄存器调度)**:
1. **b6val 独立载体**: `b6val = 0x35E; *b6ptr = b6val;` — 直接 `*b6ptr = 0x35E` 会让 GCC2 把
   SImode 池字伪寄存器分到 r3 再拷 r0 (多 2 条); u16 载体变量强制 HImode 池读直达 r0。
   (经验 227: SImode 常量经 u16 局部中转可消除 strh 前的拷贝)
2. **flagval 常量载体**: `keys = *(u16*)(arg0+0x24); flagval = 0x20; keys = keys|flagval;` —
   三段式让 GCC2 先评估 mem (进 keys home r0 = 结果寄存器), 常量后评估进 r1。
   任何单表达式/|= 形式都产出 mem→r1 (2 条寄存器互换, 4 字节差)。
   (经验 228: "先读后 OR" 的两段式 + 常量独立载体 = 控制 GCC2 双操作数求值序的手段)
3. **kind (u16) 与 keys (int) 分离**: 0xBE 读给 kind, ORR 给 keys — 共用会引发 home 级联漂移。
4. **sub_803ED34 原型 u8**: 消费点 `lsls#0x18/lsrs#0x18` 截断自证。

**permuter 85 分 = 池计分 floor (经验 29)**: 34 个 halfword 差全为未重定位池字 (0x03000820 等
地址值), 代码指令 0 差 (540B 两侧全同)。合入后 fncheck OK 540B, 全量 make + SHA1 绿。

**产物**: src/event_hub.c 真 C (约 120 行); code_0.h `sub_803272C()`/`sub_803ED34()` → u8/u32 修正;
functions.tsv status 0→1; functions.tsv note + 本条。

## 2026-09-11 sub_80368FC 匹配 (NPC 剧情对话状态机, event_hub, 564B)

**语义**: 单参数 (`u8 *arg0`), `switch(gUnk_03000820)` 0..0x14 二十一态 (战斗剧情对话):
- case 0: `0824 = arg0[0x35]` (NPC 槽), `0822 = arg0[0x2A]` (位置), 初始化, `[0xB6]=0x3A9`
  (对话 UI 序号), `[0xB4]=0` → 1。case 1: 动画 0x3A5 → 2。case 2: 等 0x800 → Sfx 0xA5 → 5。
- case 5: 0x1000 确认 → `sub_804C3A4(arg0[0x35], 801B954(arg0+0xC))`, 清 0xEFFF,
  动画 0x3A6 (第 5 参 0x102 = 0x81<<1) → 0x12。
- case 18: 等 0x800, 清 0xFEFF, `sub_8044514(0x32)` → 0x13。
- case 19: `sub_80471AC()==0` (打字机完成) → Sfx 0x64 → 0x14; 0825++。
- case 20: 无遮挡 (844/845/(v56=0856)==0) → `sub_801CBA4(arg0, 0, 0822, 0824, v56)` (NPC 归位) → 6; 0825++。
- case 6: 等 0x800 → 9; case 9: `sub_8045B90` 退场, ret 1。尾 `sub_803F658`。

**匹配要点** (全部复用 803272C 套路, 3 轮迭代到 0 代码差):
1. case0 store 块用 b6ptr/zero2/b6val 载体三件套 (同 803272C; 此处目标形状恰好相同: addr → zero → pool → store)。
2. case20 的 0856 读取: 单读保活载体 `(v56 = gUnk_03000856) == 0` + 调用传 v56 (同已匹配
   sub_803586C 的 v56 模式) — 直接传 gUnk_03000856 会读两次且寄存器互换。
3. case5 的 b4 (arg0[0x35] 槽号) 必须 u8 — u16 会让 case20 的 0856 地址/值寄存器对互换。
4. case9 `ret = 1` 不是 2 (与 sub_8032D74 的 2 不同, 此函数语义 = 普通结束)。

**permuter 70 = 池计分 floor (经验 29)**: 28 个 halfword 差全为池字, 代码 0 差。
fncheck OK 564B, 全量 make + SHA1 绿。

**产物**: src/event_hub.c 真 C; code_0.h `sub_80368FC()` → u32; functions.tsv 0→1。

## sub_80052F8 (0x080052F8, map_view, 2x2 分块贴图重建) — ⏸ 2026-09-11 claude_1 挂起, src 已恢复 INCLUDE_ASM (仓库绿 787/1059)
### 语义 (已完整还原)
四层循环 (y,x = 0..31; i,j = 0..1) 逐 8 字节块重建 2x2 分块贴图:
- a = gUnk_030047E8[y*32+x] (块属性, 每 x 重读); b = gUnk_03004690[a*4+i*2+j] (源块号);
- 从 gUnk_0300482C[b*4] 拷 4 个 u16 到 gUnk_03004620 的 (y*512 + x*4 + i*256 + j*2) (dst[128] = 下一行)。
- 调用方 src/anim_slot.c:597,602。agent1 的 5 天僵尸锁已接管; 其旧 base.c 有 volatile 洞察 (指针每块重读), 已吸收。

### 状态
- 归一化指令流 (idiff) 全对; **字节差 11 处、3 组**: 0x030047E8 / 0x03004690 / 0x03004620 的池常量载入被 GCC 合并成 `ldr rX,[pc] + ldr rX,[rX]`, 目标全部两步 (`ldr rA,[pc] + ldr rB,[rA]`, 地址/值分寄存器); 另 i 循环 preheader 中 `i*2` 的调度位 (目标 [j=0, i*256, i+1, i*2])。
- 已试并排除: ptrAt 助手 (修好 47E8/4690 但 4620 修不动且扰动 preheader)、显式 u32 临时、语句重排、k 形态 (int/u16/idxMul/shl/位置)、`-g` (本函数 ±g 输出相同, 排除)。
- **教训 (经验 230)**: 合入前 fncheck 校验的是 INCLUDE_ASM 产物 = 假绿; idiff 归一化寄存器掩盖分配差。字节判定用 `.scratch/claude_1/bytecheck.py <候选.c>`。
- 候选: `.scratch/claude_1/Va_k_i2.c` (最优, 剩 4620 组 + preheader 序)、`p1.c`、`X11_p47E8.c` (仅剩 4690+4620)。套件 permuter/sub_80052F8/ 完好 (compile.sh 曾加 -g 实验, 对结果无影响)。
- 给接手者: 三个池常量在目标里都两步, 但源码层面大概率没有显式临时 — 重点研究 482C 为何在所有候选里天然两步 (其值喂给跨语句存活的 src), 对 4620 复制该条件 (dst 也跨语句存活, 却合并了 — 差异在后面还有 4 次 store 使用); preheader 的 i*2 位置大概率由 LICM 扫描顺序决定, 试让 dst 语句先于 b/src 语句 (W8c 方向) 但保住强度削减累加器 (adds r4,#2)。

## sub_8038C84 (0x08038C84, event_hub, 201 行) — ✅ 2026-09-11 zcode, 全量绿 788/1059
### 语义
8032548 家族 NPC 对话状态机第十个变体 (gUnk_03000820 十态同 8032548), 差异点:
- case1: 无 kind 分支 (开场动画固定 0x3C7, 8032548 是 0x368/-0xF 按 arg0[0xBE]); arg1[0xBE]<=0xA 打 0x20。
- case3: 对话帧超时判定改为 `arg0[0x28] > 0x39` (8032548 是 ==0x21/0x7C/0x90 三分发音效);
  Sfx_Play(0x31,1,0) (8032548 第三参 1) → sub_8044514(0x28) → 5。
- case5/8/9 与 8032548 相同 (0x1000 确认 → sub_804C3A4 → 086B=0xC → 8 → sub_803E58C → 9 →
  无遮挡 (844/845/856==0) 时 sub_8045B90 退场, ret 1)。尾 sub_803F658。

### 匹配要点
1. **case1 的 `spr` 提载 + `b4 = 0x3C7` 是唯一非平凡卡点**: 直接 `sub_8020974(arg0+0xC, 0x3C7, ...)`
   时 r1 的 pool load 排在 `adds r0/r0,#0xC` 前 (目标相反); 加 u8 *spr 中转载体后顺序翻转。
   b4 必须在 case1 被真实赋值 (动画 id 走寄存器), 否则 r1/r2 物化顺序互换 (case0 07a/07c 两 halfword 差)。
   case0 的 zero/zero2/b6ptr/b4 四件套则照抄 8032548 即可。
2. **permuter 70 = 池计分 floor (经验 29)**: 8 个 IWRAM 全局池字全是 R_ARM_ABS32 重定位 vs target 硬码,
   代码 0 差也压不到 0。定性走部分链接 bytecmp (abs.ld 施加符号) → diff=0;
   自建只遮 R_ARM_THM_CALL 4 字节的归一化比较器比 mask 启发式可靠 (bl 检测 F000-F7FF+F800-FFFF 双字)。
3. 两条独立 permuter 运行 (主套件 + nat 种子) 都收敛到同款 spr 提载形状 — 交叉验证候选成立。
4. **code_0.h 原型是 `void sub_8038C84()`**: 合入后与 u32 定义冲突编译失败; 改 `u32 sub_8038C84();`
   (K&R 无原型形态, 不给调用点加截断 — 无 src 调用者, 零风险)。

**产物**: src/event_hub.c 真 C (带模块注释); code_0.h 返回类型修正; functions.tsv 0→1 + note。
**套件**: permuter/sub_8038C84/ (base.c 已 promote 固化最优形态)。

## sub_801B570 (0x0801B570, scene_obj_core, 143 行) — ✅ 2026-09-11 zcode-ll3, 全量绿 789/1059
### 语义 (MOD-04 kind6 对象动画帧 DMA 构建)
`Obj_FrameDispatch` (sub_801B8AC) case 6 每帧调用。f_18&0x200 直接返回; 否则逆序遍历
`*(u16*)f_04` 计数: 每轮 `sub_801B8E8(f_04+offset, f_1C)` 查跳转项 → `f_00 + f_08[value]` 帧表
→ 读 [n0, count] 后跳过 n0*4 个 u16 → 逐条目处理: 帧图块源 = EWRAM 0x0202B2C0 + (entry[2]<<22>>17)*2,
目的 = OBJ VRAM 0x0600C000 + tile*0x20 (tile 从 1 起), 传输半字数 = `gUnk_08393A30[(entry[3]>>6) + (entry[1]>>6)*2] << 4`
(高 2 位拼 4 行×4 列查尺寸表, 字节×32=DMA 字节量, 该查表值同时累加进 tile)。f_18&0x800 屏蔽 DMA 只累计 tile。

### 匹配要点
1. **接管过期锁**: opencode 2026-09-05 认领后 6 天未动, TSV note 显示 zcode-ll2 09-06 已接力留痕
   (前人 best score 255; 本次发现其后还有 output-5-1 = 5 分, 在 output-* 目录截断时易看漏 — ls 全目录再下结论)。
2. **分数 5 的残差全部是字面池重定位伪影 (补强经验 29/231)**: output-5-1 与目标逐指令形状已全对,
   剩 5 分来自独立编译下 4 个池常量 (0x040000D4/0x08393A30/0x0202B2C0/0x0600C000/0xFFFF0000) 的
   R_ARM_ABS32 vs target 硬码 + bl 槽位。**bytecmp (abs.ld 施加符号) = 唯一定性手段**, 本次以
   "字节一致 (bl 槽位除外)" 直接合入, 未强压分数到 0 — 已写进 AGENTS.md 铁律 6 新表述。
3. **abs.ld 符号要"近 Thumb 地址"复现 bl 占位编码**: target.o 里 `bl sub_801B8E8` 是 gas 占位
   `f7ff fffe`; bytecmp 若给真地址 0x0801B8E8, ld 会插 16 字节 ARM 互工作 veneer 且 bl 编码随之改变
   (296B vs 280B)。给 `sub_801B8E8 = 0x37` (自身地址+1, Thumb 位) 时 bl 槽位两侧字节同形, 差异只剩
   槽位本体的 4 字节 + 尾部 8 字节 veneer 伪影; veneer 在 .text 尾部不影响主体 cmp。
4. **DmaCopy16/DmaWait 标准宏 = 零汇编代价实证 (RULES_HARDWARE_IO §2.1)**: 裸 dmaRegs 块、
   `DmaCopy16(3, src, dst, size<<5)`、`DmaSet(3, ..., DMA_ENABLE|...|((size)<<4))` 三种写法
   bytecmp 输出逐字节相同 → 按规范选 DmaCopy16+DmaWait 合入。
   ⚠ 候选/新文件 include "gba/macro.h" 不含 DMA_* 常量 (在 io.h), 要 include "gba/gba.h"。
5. **`saved = entry` 指针复制是合法人类代码**: tile 累加用 DMA 前的 entry 指针读字节, 编译器
   寄存器复制后与 `entry += 3` 前的寻址完全一致 (r4 = r2 复制), 非经验 18/113 的偷改数据流。

### 产物
src/scene_obj_core.c 真 C (DmaCopy16+DmaWait 写法); functions.tsv 0→1+note; MOD-04 文档补条目;
EXPERIENCE 经验 233/234; AGENTS.md 铁律 6 改为"字节一致=唯一门槛, 分数只是参考"。
**套件**: permuter/sub_801B570/ (base.c 已是 output-5-1 形态); 候选存档 .scratch/zcode-ll3/。

## sub_801A3C4→ObjGfxLoad_Step / sub_801A5EC→ObjGfxLoad_Copy 改名 + Unk_801A5EC→ObjHead 结构体定名 (2026-09-11, gpnux-rename)

**背景**: 用户断点停在 0x0801A3C4 (r0=0x03000254), 提供 GBA 内存 dump, 指出该函数及邻域"只有战斗才触发",
现 C 文件命名 (scene_obj_dispatch/scene_obj_core) 有问题。

### 战斗语境证据链 (调用图全闭环)
- `AgbMain` 双主循环: `gMainLoopCallbacks[1] = BattleTask_Run` (战斗独立主循环, engine_core.c 注释+data_87E83F0.c 表)。
- `BattleTask_Run` (0x080177AC) → `sub_801B964` (0x0801A3C4 两处调用) 与 → `sub_80184A8` → `sub_801A3C4`
  (0x0801A3C4 内另一处), `sub_8018070` (战斗场景重建) 也走 80184A8。
- 对象系统头部 (`sub_801B878/B8AC` 命令分发) 均只被战斗链 (8018070/80184A8/8049D58) 调用;
  `Obj_Register`(0x08020B90) 调用点在 0x0804Axxx 战斗辅助区。MOD-04 旧注"地图 NPC/宝箱"不准确,
  该 0xC8 对象池实为战斗对象池 (BattleTask_Run 开头清 0x03000240 战斗状态块、 ListNode_Init 0x03000318 行动链)。

### 断点现场解码 (dump @0x03000254, 偏移 gUnk_03000240+0x14)
f_00..14=0x0856B4xx ROM 指针 (脚本头/命令流), f_18=0x0C03 (kind=3 → case3 LZ77→0x02020E00),
f_20=1 (装载总数), f_22=0 (当前片), f_24/26=0 (VRAM 槽/LZ77 基索引)。
脚本头 0x0856B440: w0=0x04/w1=0x44 → 801A684 重算 +0x0..0xC。
对象不在 0x02037028 池内 (池指针 0x03000250 就在实例前 4 字节), 是 IWRAM 独立实例 —
证明 Unk_801A5EC 布局 = 池对象 0xC8 的前 0x30 字节公共头部, 也单独作战斗全局装载器状态块 (0x03000918=第二实例)。

### 表大小修正
`gUnk_087EBE00` (LZ77 块指针表) 实测 **1382 项** (linker.ld 旧注 "[9]" 有误), battle_anim 调用者用到 0x540 索引。

### 改名落地 (rename_fn.sh 全链, 两次均 SHA1 绿)
- sub_801A3C4 → **ObjGfxLoad_Step**: 按 kindFlags&0xF 分步把 gUnk_087EBE00[gfxBaseIdx+gfxPos] LZ77 解压到
  VRAM OBJ 槽/WRAM 图形区, gfxPos++ 到 gfxTotal 后 sub_801A684 重载脚本并清 0x8000 (kind9 除外)。
- sub_801A5EC → **ObjGfxLoad_Copy**: ObjHead 22 个字段逐字段复制 (无 bl 调用者, 疑经指针表, 语义=状态块复制)。
- 文件: scene_obj_dispatch.c → **battle_gfx_load.c**, scene_obj_core.c → **battle_obj_core.c**
  (git mv + linker.ld + functions.tsv module 列 89 行 + gen_debug_ld.py)。
- 结构体: Unk_801A5EC → **ObjHead** (0x30 字节): cmdBase0/1(+0/4), jumpTable0/1(+8/C), scriptPtr(+0x10),
  palBitsPtr(+0x14), kindFlags(+0x18), frameIdx(+0x1C), gfxTotal(+0x20), gfxPos(+0x22), vramBank(+0x24),
  gfxBaseIdx(+0x26), palSlot(+0x29); f_1A/1E/28/2A..2F 未验证保留编号。
- scene_obj_core.c 的 sub_801B570/B81C 引用点同步改用 ObjHead 新字段名; fncheck 5 函数全 OK。

⚠ 未验证字段 (f_1A/1E/28/2A-2E) 与 ObjHead 高区 0x30..0xC7 的重叠视图需后续按 801A884/AD0C/B0B8 handler 逐个确认。

## BattleTask_Run (0x080177AC, sio_link, 933 行) — 结构全解, 卡 global-alloc 墙 (2026-09-11, zcode-main)

**结论: 未匹配 (挂起)**. 21-case 战斗主循环状态机, 结构 100% 还原 (助记符 97.4% = 674/692),
剩 967/2880 字节差异 = GCC2 跨 case 寄存器 home 分配, 非结构问题. 最佳候选 `permuter/BattleTask_Run/base.c`.

### 语义 (21 个 gUnk_03000240 状态)
战斗主循环由 gMainLoopCallbacks[1] 驱动, 每帧读 `gUnk_03000240` 分派:
- case0 初始化: DMA3 清对象池 0x02037028(0xC00 字节) → gObjPoolPtr=0x02037028; Sound_VSyncOn;
  sub_8020F4C(ctx 0x03000248) + ctx[0x37]=0xAF/ctx[0x38]=0x14; gGstate330[0..5]=-1;
  gUnk_080936A0 扫描 (数 0x128 个 0xFF) 存 gGstate340; state→1
- case1 等 FlashFlag 0x4000 → BattleFx_DispOff → 状态+1; 或 gGstate324&8 清各对象 +0x24 的 0x200 位
- case2 存 gRandCursor→gBattleRngSeed; sub_802151C 判定 → 状态3 或 5
- case3 sub_8021700()==1 → 状态4
- case4 对话阶段: DialogCtx field_C 6/7 忽略, 否则 sub_802192C → SetHead; 无对话则 Disp_Bg1Off → 状态5/0x14
- case5 sub_80207B4 分派 0..4 → 状态 0xD/6/8/0x13; 末尾按 gGstate324&0x4000 清位
- case6/7 战斗演出: sub_804AD60 → 7 → sub_8049C1C → 0x11
- case8/9 sub_804ADF8, gGstate32E==0x3C → 0x13 否则 9; 9 等 gUnk_0300032C>0x31 → 0xA
- case10 sub_8049DF8 返回 1→重置战斗(状态0), 2→System_ResetToLogo+回标题
- case13/14 sub_804A148 → 0xE → sub_804A368==1 → 0xF
- case15/16 sub_801FF40 取槽 → sub_802103C 应用; 16 读 ctx+0xB0 & 0xF0 ∈{0x40,0x50}
- case17/18 sub_8048DA4 → 0x12; sub_8048FB8 → 0x13
- case19 等 0x32 帧 → Bgm_FadeOut(0x14)+sub_8019AD0(0xA,0x110)+sub_80457AC → 0x15
- case20 sub_80401AC()==1 → 按 gGstate324&0x1000 状态5/0x13, 清 0x2000
- case21 收尾: Sound_GetFlags&4==0 → REG_DISPCNT|=0x80, 状态0, gGameState=6, gMainLoopMode=0,
  gVBlankPipelineMode=0, Bgm_Stop, Sound_VSyncOff
- 尾部(所有 case 共用): 遍历 12 个 0xC8 对象建 ListNode_InitKey/InsertSorted 到 0x03000318 行动链;
  `!(*ctx+0xB0 & 0x400)` 时把 ctx 也挂链; state=sub_801D984(state); sub_80184A8(链表头.next, state)

### 关键还原点 (可复用)
1. **DMA 是通道 3 不是 0**: 目标 `ldr =0x040000D4` = DMA3SAD (经验 149). 用 `DmaFill16(3,0,0x02037028,0xC00)`
   (control 0x81000600: 高16位 0x8100=DMA_ENABLE|DMA_SRC_FIXED, 低16位 0x0600=size/2=0xC00/2) + `DmaWait(3)`.
   错用 ch0 编出 0x040000B0 全错.
2. **-1 物化**: 目标 `movs r0,#1; negs r0,r0` (int -1). 用 `s16 tmp; tmp=-1; gGstate330[i]=tmp` 链式赋值
   比裸 `= -1` (会被折进字面池 0xFFFF) 和 `u16 tmp` (池载) 都好; 6 次复用同一寄存器.
3. **被调返回值按原型截断** (经验 90): `sub_80401AC` 目标 `bl; cmp r0,#1` 无 lsls 截断 →
   code_0.h 原型须 `u32` 不能 `u8`; 改成 u8 会多 2 条 `lsls/lsrs #24`.
4. **循环计数 u16 截断**: 目标 `adds r0,#1; lsls #0x10; lsrs r0,#0x10` → `i=(u16)(i+1)`, 且 `i/r5` 声明 u16.
5. **0x03000248 (+0xB0/+0x37/+0x38) 是基址+偏移访存**: 目标 `ldr r0,=0x03000248; adds r0,#176`,
   需经指针变量 (`u8 *p=(u8*)0x03000248; *(u16*)(p+0xB0)`) 而非裸 `0x030002F8` (会常量折叠成绝对地址).
6. **ctx 基址复用**: case0 `sub_8020F4C(p); p[0x37]=..; p[0x38]=..` 用块作用域 `u8 *p` 让 r4 复用.

### 新增符号 (linker.ld + iwram.h, 按地址序插)
- `gObjPoolPtr` @0x03000244 (u8*, 战斗对象池指针, case0 存 0x02037028)
- `gUnk_03000317` @0x03000317 (u8), `gUnk_0300032C` @0x0300032C (u16, 状态等待计数)

### 被调函数原型修正 (code_0.h, 这些函数未匹配且仅本函数调用, 不影响他人字节)
- `u8 sub_802151C(void*,void*)`, `u8 sub_802192C(void*,void*,u8*)`, `s8 sub_801FF40(u8)`,
  `u8 sub_8049C1C(void*)`, `u8 sub_8049DF8(void*,void*)`, `u8 sub_804A368(void*)`,
  `u8 sub_8048FB8(void)`, `u32 sub_80401AC(void)`

### gUnk_03000248 结构体分析 (2026-09-11, 用户要求先分析结构)
`0x03000248` = 单个 **0xC8 字节战斗对象** (池 0x02037028 的 IWRAM 独立实例, 与 0x03000310 恰差 0xC8)。
定义已加入 `include/code_0.h` 作 `BattleObj`; 关键发现是**对象内置两个 ObjHead**(各 0x30 字节,
+0x0C 与 +0x3C, kindFlags 分别在 +0x24/+0x54) — 证据 code.s 0x080180F8+ `sub_8018070` 对同一对象
先后 `sub_801B8AC(r4+0xC,r4[0x39])` 与 `sub_801B8AC(r4+0x3C,r4[0x69])`。字段映射:
- +0x00 12B UnkNode 头 (key 由 +0x38 值填充) → BattleTask_Run 挂 0x03000318 行动链
- +0x0C/+0x3C 两个 ObjHead (图形/脚本通道 A/B)
- +0x37/+0x38 = headA.f_2B/f_2C = 精灵 (X,Y) 滑动起点 (sub_801B81C arg1/arg2 写入, 上限 0xB4)
- +0xB0 state (bits0-3 kind; bits4-7 子态 0x10/0x20/0x60/0x400 不入链/0x2000 跳跃)
- +0xBD sub_802103C 写, +0xBE 槽号, +0xBB/BC 辅助, +0xBF/C0 朝向
**对匹配无帮助**: 用 `BattleObj*`/成员访问 (state/headA.f_2B) 替换裸偏移, bytecmp 仍 949 —
GCC2 把常量基址折叠, 不产生目标的 `ldr =0x03000248; adds #176` 形状 (该形状需基址来自运行时指针,
但函数级指针会让 prologue 劣化到 r9/r10, 实测 1623). 结构分析价值在语义文档, 不在字节。

### 卡点 (global-alloc 墙) — 最终 949/2880 字节
目标跨 case 的寄存器 home (r4/r5/r6 长存活承载 0x03000244/0x03000324/常量1 等) 由整个函数
692 条指令的伪寄存器生存期决定, 局部改 C 写法无效. 已试且**无效**: 命名符号 vs 裸字面量 (经验130),
ctx 函数级常驻 (r9/r10 prologue 劣化), flags 缓存/直读, case10 if-else/switch 变体, 循环拆分,
case5 tail flags 缓存, **声明顺序 5 种排列、tmp 类型 s16/u16/s32、-1 的 neg/paren/链式赋值、
case16 的 +0xB0 用指针/结构体/别名符号** —— 全部稳定在 949 (字节), 证明是 CFG 锁定的 global-alloc.

### 方法论: permuter 分数在本函数不迁移 (重要)
- permuter (自包含 base.c) base score = 4695, 与**已挂起套件 InvUi_Main 的 5870 同量级** —
  这个口径下几千分是常态 (被池重定位支配), 不是坏掉.
- 但 permuter 低分候选 (output-4220-1/4400-1/...) 用 **bytecmp 复核反而更差** (4400→1303 字节).
  根因: permuter 编译自包含 TU, 符号解析与真实构建 (include 头文件) 不同 → 寄存器分配不同,
  其梯度不指向我们的字节目标. 候选里唯一的实质手法 (`unsigned short new_var;` 插入) 在 include
  构建下 bytecmp 仍 949, 不迁移.
- 结论: **本函数压分须以 bytecmp 为唯一评分器** (脚本化: 对每个变体 `bytecmp.sh <fn> <cand> $(cat sym.ld)`),
  permuter 只能当"是否已在 949 平台期"的旁证. 与经验 29/233/234 (分数≠字节) 一致.
- 另: permuter 套件 compile.sh **不能带 `-g`** (本项目 Makefile 带 `-g`, 但 permuter 的 objdump 解析
  含 debug 段 .o 会错乱); target.o 用 asm 汇编即可.
判定手段: `scripts/bytecmp.sh BattleTask_Run permuter/BattleTask_Run/base.c <sym.ld 的 71 符号>`;
sym.ld 用 ll.cfg + linker.ld RAM 地址锚定外部符号 (否则链接失败).

### BattleTask_Run 最佳候选中继 (permuter/ 不入库, 候选全文内联)

> 字节差 967/2880, 助记符 97.4% (674/692). 重新验证:
> `mkdir -p permuter/BattleTask_Run && cp <下方>.c permuter/BattleTask_Run/base.c`
> 再准备 target.o (asm 汇编) 与 sym.ld (70 符号锚定), 然后
> `scripts/bytecmp.sh BattleTask_Run permuter/BattleTask_Run/base.c $(cat sym.ld)`.
> sym.ld 生成: ll.cfg 的 thumb_func 行 (地址正则须 `0x[0-9a-fA-F]+`) + linker.ld 的 RAM 地址,
> 覆盖: 全部被调函数符号 + gUnk_03000240/gObjPoolPtr/gGstate324/gBattleRngSeed/gUnk_0300032C/
> gGstate32E/gGstate330/gGstate340/gUnk_03000317/gUnk_03000318/gGstate314/gUnk_03000316/
> gRandCursor/gMainLoopMode/gGameState/gVBlankPipelineMode/gUnk_080936A0/gUnk_03000248.

```c
#include "gba/types.h"
#include "gba/defines.h"
#include "gba/macro.h"
#include "gba/io.h"
#include "iwram.h"
#include "sound.h"

extern u8 gUnk_080936A0[];
extern void sub_80188BC(void);
extern void sub_8017FA4(s8);
extern void sub_804B288(void);
extern void sub_8018A58(u8);
extern void sub_8019AD0(u8, u16);
extern void sub_801B964(void *);
extern void sub_8020F4C(void *);
extern void sub_801869C(void);
extern void sub_8021064(u8);
extern void sub_804ADE0(void);
extern void sub_804448C(void);
extern void sub_804DE20(void);
extern void sub_8020F08(void);
extern u8 sub_802151C(void *, void *);
extern u8 sub_8021700(void);
extern void sub_801933C(void);
extern u8 DialogCtx_GetField_C(u8);
extern u8 sub_802192C(void *, void *, u8 *);
extern void DialogCtx_SetHead(u8, u8, u8);
extern void Disp_Bg1Off(void);
extern void sub_804442C(u8);
extern u8 sub_80207B4(void *);
extern void sub_804AD60(void);
extern u8 sub_8049C1C(void *);
extern void sub_804ADF8(void);
extern u8 sub_8049DF8(void *, void *);
extern void System_ResetToLogo(void);
extern void sub_804A148(void);
extern u8 sub_804A368(void *);
extern s8 sub_801FF40(u8);
extern void sub_802103C(u8 *, u8, u16);
extern void sub_8048DA4(void);
extern u8 sub_8048FB8(void);
extern void sub_80457AC(void);
extern u32 sub_80401AC(void);
extern u16 Sound_GetFlags(void);
extern void Bgm_Stop(void);
extern void Sound_VSyncOff(void);
extern void sub_801CF90(void *, u8);
extern void sub_801FAB8(void *, void *);
extern u8 sub_801D984(u8);
extern void sub_80184A8(void *, u8);
extern u16 FlashFlag_Get(void);
extern void BattleFx_DispOff(void);
extern void BattleFx_Stop(void);
extern void DialogCtx_Clear3(void);
extern void sub_804EEC4(void);
extern void Bgm_FadeOut(u8);
extern void ListNode_Init(UnkNode *);
extern void ListNode_InitKey(UnkNode *, u8);
extern void ListNode_InsertSorted(UnkNode *, UnkNode *);

void BattleTask_Run(void)
{
    u8 state;
    u16 r5;
    u16 i;
    u16 flags;
    s16 tmp;
    u16 cnt;
    u8 ret;
    u8 *obj;
    u8 *ctxp;

    sub_80188BC();
    state = 0x7F;
    ListNode_Init((UnkNode *)&gUnk_03000318);
    switch (gUnk_03000240)
    {
    case 0:
        Sound_VSyncOn();
        DmaFill16(3, 0, (void *)0x02037028, 0xC00);
        DmaWait(3);
        gObjPoolPtr = (u8 *)0x02037028;
        if (gGstate324 & 1)
            sub_8017FA4((s8)gGstate32E);
        sub_804B288();
        BattleFx_Stop();
        gBattleRngSeed = gRandCursor;
        sub_8018A58(0);
        sub_8019AD0(0xAU, 0x120U);
        sub_801B964(gObjPoolPtr);
        ctxp = (u8 *)0x03000248;
        sub_8020F4C(ctxp);
        ctxp[0x37] = 0xAF;
        ctxp[0x38] = 0x14;
        gUnk_03000317 = 0;
        gGstate314 = 0;
        sub_801869C();
        sub_8021064(0U);
        DialogCtx_Clear3();
        sub_804ADE0();
        if (gGstate324 & 1)
            gGstate324 &= 0xFFFE;
        tmp = 0xFFFF;
        gGstate330[0] = tmp;
        gGstate330[1] = tmp;
        gGstate330[2] = tmp;
        gGstate330[3] = tmp;
        gGstate330[4] = tmp;
        gGstate330[5] = tmp;
        sub_804448C();
        sub_804DE20();
        i = 0;
        r5 = 0;
        while (r5 <= 0x128)
        {
            if (gUnk_080936A0[i] == 0xFF)
            {
                r5 = r5 + 1;
            }
            i = (u16)(i + 1);
        }
        gGstate340 = (u32)&gUnk_080936A0[i];
        gUnk_03000240++;
        gUnk_03000316 = 0xA;
        break;
    case 1:
        if (FlashFlag_Get() & 0x4000)
        {
            BattleFx_DispOff();
            sub_8020F08();
            gUnk_03000240++;
        }
        else if (gGstate324 & 8)
        {
            for (i = 0; i <= 0xB; i++)
            {
                obj = gObjPoolPtr + 0xC8 * i;
                if (obj[0xBE] != 0xFF)
                    *(u16 *)(obj + 0x24) &= 0xFDFF;
            }
            gGstate324 &= 0xFFF7;
            REG_DISPCNT &= 0xFF7F;
        }
        break;
    case 2:
        gBattleRngSeed = gRandCursor;
        DialogCtx_Clear3();
        if (gGstate324 & 2)
            gGstate324 &= 0xFFFD;
        if (sub_802151C(NULL, (void *)0x03000248) == 0)
        {
            gGstate324 |= 0x80;
            gUnk_03000240 = 3;
        }
        else
        {
            gUnk_03000240 = 5;
        }
        break;
    case 3:
        if (sub_8021700() == 1)
            gUnk_03000240 = 4;
        break;
    case 4:
        sub_801933C();
        if (DialogCtx_GetField_C(0) != 0 || DialogCtx_GetField_C(2) != 0)
        {
            if (DialogCtx_GetField_C(0) == 6)
            {
            }
            else if (DialogCtx_GetField_C(0) == 7)
            {
            }
            else if (sub_802192C(gObjPoolPtr, (void *)0x03000248, &state) != 0)
            {
                DialogCtx_SetHead(0, 4, 2);
            }
        }
        else
        {
            Disp_Bg1Off();
            if (!(gGstate324 & 0x2000))
            {
                gUnk_03000240 = 5;
            }
            else
            {
                sub_804442C(0);
                gUnk_03000240 = 0x14;
            }
            DialogCtx_Clear3();
            gGstate330[0] = tmp = -1;
            gGstate330[1] = tmp;
            gGstate330[2] = tmp;
            gGstate330[3] = tmp;
            gGstate330[4] = tmp;
            gGstate330[5] = tmp;
        }
        break;
    case 5:
        switch (sub_80207B4(gObjPoolPtr))
        {
        case 0:
            break;
        case 1:
            gUnk_03000240 = 0xD;
            gGstate324 |= 2;
            break;
        case 2:
            gUnk_03000240 = 6;
            gGstate324 |= 4;
            break;
        case 3:
            Bgm_FadeOut(0x14);
            gUnk_03000240 = 8;
            gGstate324 |= 4;
            break;
        case 4:
            gUnk_03000240 = 0x13;
            gUnk_0300032C = 0;
            break;
        }
        {
            u16 bit = 0x4000;
            if ((gGstate324 & bit) && (gGstate314 & 0xF000) == 0x2000)
                gGstate324 &= ~bit;
        }
        break;
    case 6:
        gGstate324 |= 0x40;
        sub_804AD60();
        gUnk_03000240 = 7;
        break;
    case 7:
        if (sub_8049C1C(&state) != 0)
        {
            sub_804ADE0();
            gUnk_03000240 = 0x11;
        }
        break;
    case 8:
        sub_804ADF8();
        if (gGstate32E == 0x3C)
        {
            gUnk_03000240 = 0x13;
            gUnk_0300032C = 0;
        }
        else
        {
            gUnk_03000240 = 9;
        }
        break;
    case 9:
        if (gUnk_0300032C <= 0x31)
            gUnk_0300032C++;
        else
        {
            gUnk_03000240 = 0xA;
        }
        break;
    case 10:
        switch (sub_8049DF8(gObjPoolPtr, (void *)0x03000248))
        {
        case 1:
            gGstate324 |= 1;
            REG_DISPCNT |= 0x80;
            sub_804EEC4();
            gUnk_03000240 = 0;
            break;
        case 2:
            System_ResetToLogo();
            gUnk_03000240 = 0;
            gMainLoopMode = 0;
            return;
        }
        break;
    case 13:
        sub_804A148();
        gUnk_03000240 = 0xE;
        break;
    case 14:
        if (sub_804A368(gObjPoolPtr) == 1)
            gUnk_03000240 = 0xF;
        break;
    case 15:
        if (gGstate324 & 2)
        {
            ret = sub_801FF40(0);
            if ((s8)ret >= 0)
            {
                sub_802103C((u8 *)0x03000248, ret, 0);
                gUnk_03000240 = 0x10;
            }
            else
            {
                gUnk_03000240 = 2;
            }
        }
        else if (gGstate324 & 4)
        {
            ret = sub_801FF40(1);
            if ((s8)ret >= 0)
            {
                sub_802103C((u8 *)0x03000248, ret, 0x200);
                gUnk_03000240 = 0x10;
            }
            else
            {
                gUnk_03000240 = 0x13;
                gUnk_0300032C = 0;
            }
        }
        break;
    case 16:
    {
        cnt = *(u16 *)(0x03000248 + 0xB0) & 0xF0;
        if (cnt == 0x40 || cnt == 0x50)
        {
            if (gGstate324 & 2)
            {
                gUnk_03000240 = 2;
            }
            else if (gGstate324 & 4)
            {
                gUnk_0300032C = 0;
                gUnk_03000240 = 0x13;
            }
        }
        break;
    }
    case 17:
        sub_8048DA4();
        gUnk_03000240 = 0x12;
        break;
    case 18:
        if (sub_8048FB8() != 0)
        {
            gUnk_0300032C = 0;
            gUnk_03000240 = 0x13;
        }
        break;
    case 19:
        if (gUnk_0300032C <= 0x31)
        {
            gUnk_0300032C++;
        }
        else
        {
            Bgm_FadeOut(0x14);
            gUnk_03000240 = 0x15;
            sub_8019AD0(0xAU, 0x110U);
            sub_80457AC();
        }
        break;
    case 20:
        if (sub_80401AC() == 1)
        {
            tmp = gGstate324 & 0x1000;
            if (tmp != 0)
                gUnk_03000240 = 5;
            else
            {
                gUnk_0300032C = tmp;
                sub_80457AC();
                gUnk_03000240 = 0x13;
            }
            gGstate324 &= ~0x2000;
        }
        break;
    case 21:
        if (!(Sound_GetFlags() & 4))
        {
            REG_DISPCNT |= 0x80;
            gUnk_03000240 = 0;
            gGameState = 6;
            gMainLoopMode = 0;
            gVBlankPipelineMode = 0;
            Bgm_Stop();
            Sound_VSyncOff();
        }
        break;
    }
    i = 0;
    do
    {
        obj = gObjPoolPtr + 0xC8 * i;
        if (obj[0xBE] != 0xFF)
        {
            if (i <= 4 && !(gGstate324 & 0x10))
                sub_801CF90(obj, i);
            tmp = 0xC8 * i;
            ListNode_InitKey((UnkNode *)(gObjPoolPtr + tmp), (gObjPoolPtr + tmp)[0x38]);
            ListNode_InsertSorted((UnkNode *)&gUnk_03000318, (UnkNode *)(gObjPoolPtr + tmp));
        }
        i = (u16)(i + 1);
    } while (i <= 0xB);
    obj = (u8 *)0x03000248;
    if (!(*(u16 *)(obj + 0xB0) & 0x400))
    {
        sub_801FAB8(obj, gObjPoolPtr);
        ListNode_InitKey((UnkNode *)obj, 0);
        ListNode_InsertSorted((UnkNode *)&gUnk_03000318, (UnkNode *)obj);
    }
    state = sub_801D984(state);
    sub_80184A8((void *)gUnk_03000318.next, state);
}
```

## 2026-09-11 claude-8018A58

### ✅ sub_8018A58 (416B, 接管自 zcode-ll 挂起候选)
战斗背景加载, 184 行 (见 TSV 语义全解)。三条关键修正把 53 diff 收敛到零:
1. **gUnk_087ED394 步长 = 12B (3 指针), 非 16B/4 指针**: 反汇编索引算术
   `lsls r0,#1; adds r4,r4,r0; lsls r4,#2` = idx×12, 与 ROM 数据交叉验证
   (entry0 = 08598EA4/08608648/08609368, 若 16B 步长则 entry1 错位)。C 形式:
   `gUnk_087ED394[idx].field_0/4/8` (struct 三字段), 比 `t[idx*3+n]` 下标式更贴目标
   (后者让 gcc 每次独立算 idx*3+n, +4B)。
2. **bldcnt 位组合实写 0x0400000C (GREENSWAP 地址), 不是 BLDCNT (0x04000050)**:
   目标 `movs r0,#0x80; lsls r0,#0x13; strh r6; adds r0,#0xC; strh r7` = DISPCNT+共享
   基址+0xC。用 `REG_BG2CNT = bldcnt` (io.h 恰为 REG_BASE+0xC) 达成同址共享基址形状;
   语义上是原始代码的死写 (GREENSWAP 只认 bit0), 怀疑原 SDK 头 BLDCNT 曾误定位 0xC。
   调试教训: 我最初"顺手改"成真 BLDCNT 语义 → 池里多出 0x0400000C… 反而错位。字节为准。
3. **sub_8018E34 原型 u8→u32**: 目标调用点 `bl sub_8018E34` 后**无** `lsls/lsrs #24`
   零扩展 → caller 按非 u8 接收。定义端 `ldrb r0,[r0]` 本身零扩展, 改 u32 返回后
   定义端 116B 不变 (fncheck 双绿)。若不改: src 版 fncheck 变 420B (+4B), 且
   sio_link .text +4B → linker.ld 0x61C784 钉扎点 "cannot move location counter
   backwards" —— **链接器钉扎点是最灵敏的 +4 字节探测器**。
其余: dispcnt/bldcnt = u32 未初始化局部 (落 r6/r7); 掩码链写自然常量 (~7,~0x10,~0x80;
~0xC,~0x30,~0x40,~0x80), agbcc 自动折叠成 movs+negs / subs 链 —— **不要手抄折叠后的
机器掩码** (我曾把 ~0xC/0x30/0x40 折成 ~0x10/0x34/0x10, 寄存器分配全歪); `s16 idx`
会引入符号扩展 (+4B), 用 u32; gUnk_03000500 用 8 条独立赋值语句 (目标序 2,0,6,4,A,8,E,C);
D8/E8 表用 REG_ADDR_BGxHOFS/VOFS 宏直接赋值 (gcc 自行组成 0x04000010+adds#4 链)。
bytecmp 判定注: 本函数目标 .o 未链接保留 bl 占位 (f7ff fffe), 候选部分链接会为跨 4MB 的
bl 生成 veneer (+80B 追加节) —— 排除 bl 对后主体 0x0..0x14C 零差异即算候选成立,
真门槛由 fncheck (全 ROM) 定论。

## 2026-09-11 gpnux — sub_800BFF8 可读性重构 (保字节)

HP/MP/Lv 三位数字 tilemap 写入器 (menu_ui.c @0x0800BFF8, 116 行 asm)。原匹配版 C 是"编译器形状"
(base16/count/d 等无语义命名, 减法除法无注释)。本次重构只动可读性: 逐参数块注释 + 减法除法/前导零
抑制语义标注 + 原始 ROM 缺陷标注; 表达式结构、求值序、类型宽度、变量角色全部未动 (改表达式需重过
permuter 回环, 收益为零——字节形状已锁定)。

**语义结论** (三参数): value=0–999 数值 (负值时商下溢出乱码瓦片, 调用方恒传非负); dest=tilemap
个位格地址, 向负方向写三格 (百/十/个); base=图块项基值 (调色板<<12|数字瓦片基号), 数字 d 表项 =
base+0x25A+d, 空白 = base+0x27F。调用方 menu.c: Num_Draw16 恒 0xB000; Hud_DrawHp/Mp 满值传
0xF000 (高亮调色板) 否则 0xB000。

**原始 ROM 缺陷** (字节级匹配保真, 勿"修复"): 十位空白判定硬编码 `tiles[0]==0xB27F` (0xB000 基的
空白瓦片) 而没跟 base 走 → base=0xF000 且数值形如 5/50 时百/十位空白判定失效, 渲染成 "005" 而非
"  5"。若日后给函数起语义名 (如 Num_Draw3Digits), 在模块文档保留此 quirk。

复验: fncheck OK (224B @0x0800BFF8), make+SHA1 全绿。

## 2026-09-11 claude-arg0: sub_801A684 参数改型 u8* → ObjHead*

应需求把 `sub_801A684` 第一参数从 `u8 *arg0` 改成 `ObjHead *arg0` (签名/头文件/调用点三处):
1. **定义** (battle_gfx_load.c): 签名 `ObjHead *arg0` + 首行 `u8 *obj = (u8 *)arg0;` 别名,
   函数体保留字节级验证过的裸偏移表达式 (只换变量名) —— 字节零风险, fncheck OK 110B。
   `sub_801A6F4(arg0)` 直传 ObjHead* (未匹配 INCLUDE_ASM, 无原型检查, 指针 ABI 同)。
2. **code_0.h** 原型 `void sub_801A684(u8 *)` → `(ObjHead *)`。
3. **调用点** (3 处 bl): ObjGfxLoad_Step 内直传 (消 `incompatible pointer` 警告);
   `sub_804AD60` (battle_anim.c, 已匹配) 调用点加 `(ObjHead *)obj` 强转 —— 该函数整体
   是 u8* 操纵 BattleObj 旧风格, 只动这一处; 第三调用方 `sub_803F658` (event_hub, 未匹配,
   code_0.h K&R 原型) 不受影响。
复验: fncheck sub_801A684/ObjGfxLoad_Step/sub_804AD60/sub_8018A58 四绿, make + SHA1 通过。

### 补充 (同日): sub_801A684 改为真·字段访问
初版改型保留了 `u8 *obj = (u8 *)arg0` 别名 + 裸偏移 (担心字节回归)。按 review 重写:
1. **函数体全字段访问**: `scriptPtr/cmdBase0/cmdBase1/jumpTable0/jumpTable1/f_1A/frameIdx/
   f_1E/vramBank/f_2F/palSlot/f_28/kindFlags/palBitsPtr` —— 一次过 fncheck 110B,
   字节与裸偏移版完全一致 (do-while(0) 与 zero8/zero16 顺序保持)。
2. **sub_804AD60 (battle_anim.c)**: `ObjHead *obj = (ObjHead *)gUnk_03000918;` 声明,
   `*(u16*)(obj+0x18)` → `obj->kindFlags`、`obj[0x2A]` → `obj->f_2A`,
   `sub_801B81C((u8 *)obj, ...)` 反向强转 (原型就是 u8*)。fncheck 128B 绿。
教训: "裸偏移保字节"是惯性 —— ObjHead 字段名与偏移一一对应且宽度正确时,
agbcc 对 `head->field` 与 `*(uN*)(p+off)` 生成相同代码; **先试字段版再退别名版**。

### sub_801B954/sub_801B95C 参数改型 void** → ObjHead* (同日)
两函数原为 `void **ptr` + `*(ptr[N])` 指针数组视图, 实际语义: 参数 = ObjHead*
(调用方传 `obj + 0xC` = headA / `obj + 0x3C` = headB), 取 `head->cmdBase0[2]` (u8,
脚本类型字节) / `head->cmdBase1[2]` (u16)。改型:
1. 定义改 `u8 sub_801B954(ObjHead *head) { return *(u8 *)(head->cmdBase0 + 2); }`,
   `u16 sub_801B95C(ObjHead *head) { return *(u16 *)(head->cmdBase1 + 2); }` —— fncheck 双 6B 绿。
2. 30 处调用点 `(void **)(X + 0xC/0x3C)` → `(ObjHead *)(X + 0xC/0x3C)` (event_hub 26,
   obj_pool 2, cutscene_mgr 1, battle_obj_core 内部 1), battle_gfx_load 直传 `head`。
   sed 批量时注意 `ObjSlot(0)` 含括号, 通用正则 `[^()]*` 漏配, 需单独补。
3. ROM 全量一致 (fncheck --blame 8388608 B)。调色板加载链: sub_801A684 →
   sub_801B954(head) 作为 sub_804C2FC 第三参数 (脚本类型字节决定装载方式)。

## 2026-09-11 claude-arg0: ObjHead 指针字段改型 + 脚本头实证 (sub_801A684 分析)

### 脚本头 (scriptPtr 指向) 的 ROM 实证
`scriptPtr` 来自 `gUnk_08393B28[idx].field_0` (0x14B 战斗对象资源表)。dump 0x0856D650 等
20 个脚本头:
- `u16[0]` 全部 = 4 → cmdBase0 = scriptPtr + 4 (固定)
- `u16[1]` (字节 +2) 可变 (0x14..0x224) → cmdBase1 = scriptPtr + u16[1]
- **注意是 u16[1] 不是 u16[2]**: 目标 asm `ldrh r1,[r0,#2]` = +2 字节。我曾误写 data[2]
  (u16* 第 2 项 = +4B), 编出 `ldrh [r0,#4]` → ROM 差 1 字节 (0x1A690: 81≠41)。
- 条目 = **u16 对 (值, 帧号)**, 非固定 2 字节脚本: sub_801B8E8 以 4B 步进扫 (值,帧号) 对;
  cmdBase1[0] = jumpTable1 项数 N; jumpTable0/1 紧随其后 (+4); 表值 = **字节偏移**。

### ObjHead 改型 (code_0.h)
| 字段 | 旧 | 新 | 依据 |
|---|---|---|---|
| scriptPtr | u32 | const u16 * | 半字头 [0]/[1] |
| cmdBase0/1 | u32 | u16 * | 半字命令流, 消费方全部 (u16*) cast |
| jumpTable0/1 | u32 | u16 * | u16 偏移表, 索引 ×2 |
| palBitsPtr | u32 | const u8 * | sub_804C2FC DMA 源 (调色板数据) |

### 两个语义坑 (都靠 fncheck/ROM 抓回)
1. **jumpTable 值是字节偏移**: 改型后 `obj->cmdBase1 + offset` (u16* + u16) 被编译成
   ×2 (lsls #1), 目标是直接加 → 写 `(u16 *)((u8 *)obj->cmdBase1 + offset)` 保字节语义。
   sub_801B570 因此多 4B (主体多 2 条指令)。
2. **u16* 索引的自动 ×2**: 同根因。

### fncheck 假绿事故 (重要!)
修完 data[1] 前, fncheck 报 sub_801A684 "OK 110B" 但 ROM 实际差 1 字节。根因:
`build/src/scene_obj_dispatch.o / scene_obj_core.o` 是**重命名前残留的陈旧 .o**
(源文件已改名 battle_gfx_load/battle_obj_core, Makefile 不再引用, 但 os.walk(BUILD)
先扫到它们, 里面的旧版函数字节"碰巧"与 ROM 一致 → 假绿)。已删除; 这两个文件属
编译产物, 非铁律 4 禁区。教训: fncheck 按文件名顺序取第一个含符号的 .o, 源文件
改名/搬迁后必须清 build 残留。

## 2026-09-11 claude-b0b8: sub_801B0B8 (600 行, 接管自 gpnux) — 转挂起

战斗对象帧构建/装载分发 (语义全解见 TSV note)。参数确认 ObjHead* (用户要求, code_0.h
原型 `u8 sub_801B0B8(u8*, u8)` 待合入时改)。语义链: 每 counter 逆序取 (cmdBase1 段 →
sub_801B8E8 按帧号查 (值,帧号) 对 → cmdBase0 段头 [n0,count] → 跳过 n0*4 字节属性表 →
count 轮 6 字节条目按 kindFlags&0xF 分 5 类 DMA 装载), slot = 0x030035C0 + arg1*8
每轮 -8 步进; 尾段 (kindFlags&0x80F)==0x809 且 f_22>=f_20 时 DMA 合成调色帧并清 0x800。

### 攻坚记录 (全部 bytecmp 定量, 符号锚定见 permuter/sub_801B0B8/compile.sh 环境)
| 变体 | 字节差/总长 | 说明 |
|---|---|---|
| gpnux base.c | 1122/1232 | u32 影子 struct + next* 影子对 |
| permuter 21735-15 | 1077/1240 | 分数最低, 结构未中 |
| v2 (claude) | **1062/1136** | ObjHead 字段 + 每 case 开头推进, 无 next* |
| v3 | 1113/1176 | u32 手动缩放视图 (方向错误, B570 语义已证 u16* 版本正确) |
| v4 | 1094/1152 | B570 声明序 + DmaCopy16 宏 |
| v5 | 1081/1160 | 位测试先行 + 显式 i+=1 (证伪"推进顺序"假设) |
| 目标 | 0/1208 | — |

### 结构性卡点 (需新洞察)
1. **变量→寄存器/栈全局分配不匹配**: 目标 obj 存 [sp] (入口 `str r0,[sp]`), 7 个栈槽
   (obj/count/unk8/keep/acc/slot/counter, sub sp,#0x1c); 候选全部把 obj 落寄存器
   (r8-r10), sub sp,#16。agbcc 的溢出触发条件未复现 — 与局部声明序/数量/使用密度相关。
2. **每 case 内联 9 条推进序列** (`adds r2,r4,#0; subs #8; str [sp,#0x14]; adds r3,#6;
   mov sl; subs r6,#1; mov sb; adds r7,#1; mov r8`): 目标在每个 case 的位测试后重复;
   r10/r9/r8 = nextCur/nextV6/nextI 三件套 → 目标 C **有 next* 变量** (与 gpnux base.c
   的影子对假设一致), 但其 base.c 的 next* 形状未触发相同分配。
3. 已证伪: v5 的"位测试先行 + 显式 i+=1"、v3 的 u32 手动缩放视图。

### 后续攻坚建议
- 对比 B570 (已绿) 的变量集合/声明序, 找 agbcc 溢出的判定边界 (B570 也是 8 高位寄存器
  全用但无 obj 落栈)。
- permuter 探索 next* 影子对的**声明位置** (函数顶部 vs case 内块) 与**类型** (u8 vs u32)。
- 100% 可信终验 = make+SHA1; bytecmp 符号锚: gUnk_08393A30=0x08393A30,
  gUnk_03000518=0x03000518, sub_801B8E8=0x0801B8E8, sub_801B790=0x0801B790,
  __umodsi3=0x08000e30。


## sub_8010770 (0x08010770, menu_ui, 道具/技能菜单"确认使用") — ✅ 2026-09-12 gpnux-10770 (520B 字节全等)
### 语义
`void sub_8010770(u8 arg0)`; 调用点: code.s 0x0800D47E 附近菜单确认逻辑 (传 1)。
1. `n = 0`; `sub_8010300(gUnk_030001C3)` 返回 0 → `Sfx_Play(3,0,0)` 收尾(冷块)。
2. `gUnk_030001C3 == 0x26`(传送) → 只置 `n = 1` 直接进尾部。
3. 否则 `gUnk_030001B0 = 0x10`;
   - `arg0 == 0`: `gUnk_030001C5 == 5`(全队回复类) → `i=0..4` 遍历 `gPartyMemberIds[i]`
     (0xFF 终止; id>0 则 id--), `gPartyStats[id].hp += gUnk_030001C6` 并以 `max_hp` 截顶,
     计数为 0 时直接 `hp = max_hp`; 逐项 `sub_8010624((u8)i, 2)`; `i>4` break; 之后 `Sfx_Play(0x17,1,0); n++`。
     非 5 → `gMenuCursorStack[gMenuCursorGrp] = gMenuCursorSel; gMenuCursorSel = gMenuCursorStack[15];
     if (gMenuCursorSel <= 3) gMenuCursorSel = 4;` + `sub_800E668(0xFF); Sfx_Play(1,0,0); return;`
   - `arg0 != 0`: `id = gPartyMemberIds[gMenuCursorSel - 4]`(无 u8 强转! 加了会多 lsls/lsrs),
     `hp < max_hp` 时同上加血 + `sub_8010624((u8)(gMenuCursorSel-4), 1)` + `Sfx_Play(0x17,1,0); n++`;
     否则 `gUnk_030001C8 = 0x24; Sfx_Play(3,0,0)`。
4. `if (n == 0) return;` → `gUnk_030001C3 == 0x3E` 清 `gPartyFollowFlags` bit7 并 `sub_800F128(0, gMenuCursorStack[gMenuCursorGrp])`;
   否则 `id = gPartyMemberIds[(u8)(gMenuCursorStack[0]-1)]`, `id>0 → id--`, `gPartyStats[id].mp -= gUnk_030001C4`。

### 命中路径 (结构 4 个硬杠杆, 少一个就差 2~10 字节)
1. **外层冷块**: 必须写 `if (sub_8010300(...) != 0) { 主体 } else { Sfx_Play(3,0,0); }`。
   写成 `if (... == 0) { Sfx_Play(3,0,0); return; }` 早退 → 冷块落函数头(0x16), ROM 是 `b 0x1f8` 落尾。
2. **0x26 判定取反**: `if (gUnk_030001C3 != 0x26) { 大段 } else { n = 1; }`。
   写 `== 0x26` 时 `n=1` 块被排到分支之前, 与 ROM 的 `_080108F0`(在 arg0 全部分支之后) 差 8 字节。
3. **全队循环用 `while` + `break`**, 不要 `if (id != 0xFF) { do {...} while (id != 0xFF); }`:
   后者 GCC2 生成"入口 b 到循环尾测试"的旋转形态(多 1 条 b + 测试块位置差), `while` 版逐字节一致。
4. **flag 检查写 `if (n == 0) return;`(跳过块在后), 且 c5 分支写 `n++` 而不是 `n = 1`** —— 见经验 244。
   c5 路径 n 已知为 0, `n++` 与 `n = 1` 都生成 `movs r6,#1`, 但 `n = 1` 会被 flow 判死删除,
   导致 `b.n` 后多 2 字节 padding + 池倾倒点前移, 连环差 8 字节。

### 差异排查工具 (本次自建, 放在 .scratch/gpnux-10770/)
`d.py <cand.c>`: 编候选 → 两侧 objdump → difflib 按指令流对齐, 忽略 `<标签名>`, 分支目标按绝对偏移比较。
比 asm-differ 强的地方: 目标 .o 的**内联字面池会被 objdump 反汇编成假指令 / 折叠成 `...`**,
asm-differ 与早期 fncheck diff 都会被这些假行淹没; d.py 只对齐助记符+操作数, 池差异一眼可辨。
`vtest.sh` 打印 `movs r6,#1` 出现次数与 .text 字节数作快速指纹 —— 本次靠它把 5 个结构变体筛到 1 个。
`gen*.py` 是配套的变体批量生成器 (只改 base 的一个片段批量生成/编译/打分)。

### 符号
新登记 `gUnk_030001C3`(linker.ld 0x1C3 + iwram.h); 补齐 iwram.h 里 gUnk_030001C4/C5/C6 的 extern
(linker.ld 早有、头文件漏登)。`gUnk_03002C44` 用已有别名 `gPartyFollowFlags`(同址, 语义一致: 清 bit7)。
`code_0.h`: `sub_8010300` 原型 `void()` → `u8(u8)`(调用点 `lsls r0,r0,#0x18; cmp r0,#0` = u8 返回;
唯一调用者就是本函数, 安全); `sub_8010770` 原型 `void()` → `void(u8)`。
`sub_8010624()` / `sub_800F128()` 保持无原型声明即可(显式 `(u8)` 强转使调用点字节不受影响, 实测两种写法全等)。

### 判定
`python3 scripts/fncheck.py sub_8010770` → OK (520 bytes @0x08010770, 0 池重定位, 10 bl 槽忽略);
`make` + `sha1sum -c ll.sha1` 全绿; permuter base score = 115 (仅池/bl 未重定位的假分)。

## sub_80053B4 (map_view, 274 行) — ⏸ 攻坚中, 候选差 59 字节 (2026-09-11, claude-53b4; 链式研磨最终收敛 675 分/59 字节差, 多轮平台期)

### 语义 (已完全解开)
地图视图重绘: `gObjGraphicsSetId > 0xFC` 早退; 按 `gCameraDrawMode`(1..8 + default) 确定
{srcX1, srcY1, srcX2, srcY2} 四个图块源坐标, 然后双层循环 (row 0..10 × col 0..15) 把两张地图的
16×11 tile 区换成 gfx: `src1 = 0x02006000[(srcY2+row)*0x80 + srcX1 + col]` → gfx 0x02000000[tile*4]
4 个 u16 写到 `0x02004000[row*64 + col*2 .. +33]`; `src2 = 0x0200E000[(srcY1+row)*0x80 + srcX2 + col]`
→ 0x02002000[tile*4] → 0x02004800 同构。调用点: sprite_engine.c `sub_80053B4(gCameraPosX, gCameraPosY)`。
符号全部已登记: gObjGraphicsSetId(0x0300467C)/gCameraDrawMode(0x0300460C)/gDrawCamEaseActive(0x03004680)/
gDrawCamY(0x03004684)/gDrawCamX(0x030047C0)。参数 (u16 x, u16 y)(入口 lsrs 零扩展)。

### 已破解的结构 (全部经 bytecmp 逐字节验证)
1. **dst 偏移 = row*64 + col*2**(非直觉的 row*32!)—— 目的内层步距 128 字节, 0x02004000/0x02004800
   各 2KB 缓冲, 每行只写前 64 字节。
2. **src 读取必须内联 `tile * 4`**(声明 `u16 tile` 后 `[tile * 4]`); 写 `tile *= 4;` 语句会给 u16 tile
   加截断对(lsls/lsrs), 直接崩形(480 字节)。dstOff 若声明为 u16 同理加截断; 必须内联索引或 s32。
3. **各 case 块是"人类不一致"书写**—— case 4 与 case 7 语义相同但 ROM 没合并成共享块:
   case 4 = `srcX2 = x>>4; srcY1 = ...; srcX1 = srcX2; srcY2 = y>>4;`(srcX2 先算, srcX1 拷贝),
   case 7 = `srcX1 = x>>4; ...; srcX2 = srcX1;`(反向)。case 1/3/6/8/default 同 case 4 型,
   case 2/5 同 case 7 型; case 5 的 srcY1 = srcY2 拷贝(`str r6,[sp,#4]`), case 2 顺序 srcX1,srcY2,srcX2,srcY1。
   全部统一成同序会被 GCC 合并 case 块, 立即不匹配。
4. **srcY1 必须溢出到 [sp+4]**(9 定义 + 1 使用, 帧槽 [sp]=rowNext/[sp+4]=srcY1)。
   我方自然写法把它分到 r7 → 内层循环地址被迫走栈搬运, 差 100+ 字节。
5. **default 块用 r4>>20/r1>>20**(入口 u16 参数规范化的 x<<16/y<<16 残值) —— 写 `x >> 4` 自然产生,
   无需特殊处理; 但只有 srcX1 是"拷贝目标"(各 case 块中第一个 def 最晚的变量)时 ip 才会分给 srcX1
   (REG_ALLOC_ORDER = 3,2,1,0,ip,lr,4,5,6,7,8,sl,sb; 全局分配按 QTY_CMP_PRI=floor_log2(refs)*refs/
   size/life 排序, 同分按 qty 号)。

### 当前最佳候选 (permuter/sub_80053B4/, 链式爬山 10325→1840→1330→1045→835→680)
`.scratch/claude-53b4/best680.c` = 564 字节(与目标等长!), 仅差 59 字节, 全部在内层循环的
{tile*8, dstoff, dst-addr} 三个临时量的 r1/r2/r3 角色分配 + 池常量组织(目标物化 0x02002002/04/06,
候选物化 0x02004040/4840)。候选含 permuter 非人类伪影(new_var/new_var2/new_var3/new_var4 指针 +
do-while(0) 屏障 + 死 dstOff 声明), **合入前必须人类化并复验**。指针类伪影反复出现暗示原代码
可能真的用了基址指针变量(如 `u16 *gfx = 0x02000000`), 但函数级 gfx 指针实测反而崩形(516 字节)。

### 工具与提示
- 套件: permuter/sub_80053B4/(compile.sh/target.o 已建), base.c = 当前最优种子。
- 链式爬山: 每轮 `cp output-<best>/source.c base.c && rm -rf output-* && permuter.py . -j 1`,
  每轮约 7 分钟, 分数仍在稳定下降。
- 逐指令对比: `scripts/fndiff.sh sub_80053B4 <候选.c>`; 字节定论: `scripts/bytecmp.sh sub_80053B4
  <候选.c> "gObjGraphicsSetId = 0x0300467C;" "gCameraDrawMode = 0x0300460C;" "gDrawCamEaseActive =
  0x03004680;" "gDrawCamY = 0x03004684;" "gDrawCamX = 0x030047C0;"`。
- 关键机制注记: local-alloc 按 QTY_CMP_PRI(寿命) 而非 refs 排序、death-reuse 让连发临时量共用
  同寄存器、regmove 的地址增量传播(+2/+0x3e bump vs 池物化)对寄存器角色敏感 —— 内层循环的
  r1/r2/r3 角色由 {tile*8 建议槽 r1, dstoff 寿命, dst-addr 寿命} 的优先级竞争决定。

## sub_8018070 (0x08018070, sio_link, 266 行切片) — ✅ 2026-09-12 nova (568B 字节全等, permuter base score 0)

### 语义: 战斗场景 VBlank 流水线
`VBlankIntr` 的 `gVBlankPipelineMode==2` 分支 (地图侧是 mode 1 `VBlank_UpdateGameScreen`)。
调用链: `BgScrolls_WriteAll` → `sub_801889C`(战斗 FX 表) → `sub_804C184` →
`!(gGstate324&0x10)` 时 `DmaCopy32(3, gUnk_020352C0, 0x06006800, 0x800)` →
`sub_804B224(&gGstate324)` → `DialogCtx_Flush` → `gUnk_03000344 = 0x7F` →
`ret = sub_8049D58(sub_8022458(0x7F))` → `sub_801B7B8`(清对象标志位图) →
**逐对象帧分发** → 未冻结时 `ret = sub_801D214(gObjPoolPtr, ret)` →
`sub_801B688(ret)` / `sub_801B920()` → `gUnk_03000344 = ret` →
`DmaCopy32(3, gOamBuffer, OAM, 0x400)` → bit10(0x400) 待传图块 `sub_80527AC()<0` 才清位 →
bit11(0x800) 图块装载 `BgTiles_LoadSet(0)` 后清位 →
`DmaCopy32(3, (void*)0x0861A7E4, 0x060125C0, 0x2C0)` → `sub_8018928`。

对象循环 (`gUnk_03000318` 行动链, `key<=0xFE` 且 `!(gGstate324&8)` 才续行):
```
kindA = obj->headA.kindFlags & 0xF;   kindB = obj->headB.kindFlags & 0xF;
kindA==6 || kindB==6 || kindA==7 || kindB==7  ->  DmaCopy32(3, 0x020362C0, 0x06007800, 0x800) + DmaWait
ret = sub_801B8AC(&obj->headA, obj->headA.f_2D);          // 恒调
if (obj->state & 0x2000)                                   // 跳跃态
    if (!(obj->headB.kindFlags & 0x800))                   // headB 未禁用 DMA
        ret = sub_801B8AC(&obj->headB, obj->headB.f_2D);
```
实证了 MOD-04 的「双 ObjHead」推断 —— 同一对象两条通道各自独立分发, headB 只在跳跃态且未禁用 DMA 时参与。

### 命中的 3 个硬杠杆 (逐个经 fndiff 定量)
1. **循环必须写成 `node`(UnkNode*) + `obj`(BattleObj*) 双变量**。
   单变量 `for (obj = ...; ...; obj = obj->node.next)` 版 fndiff **4880**;
   改成 `node = gUnk_03000318.next; while (...) { obj = (BattleObj *)node; ...; node = node->next; }`
   立刻降到 **2400**, 且循环体 233 条指令逐条对齐。
   原因: 目标在循环头有 `adds r5, r4, #0`(r4=迭代器, r5=对象副本), 全程 `obj` 走 r5 / `node->next` 走 r4,
   循环体里的 `adds r3,r5,#0xc` / `adds r2,r5,#0x39` / `adds r6,r5,#0xb0` 全以 r5 为基;
   单变量版 GCC 只用一个寄存器, 这些 base 复制全部退化成以 r4 为基, 与目标的双基形态不合。
2. **kind 判定必须内联成短路链, 不能预存 `kindA`/`kindB` 变量**。
   预存变量的写法下 GCC 会把两次 `kindFlags & 0xF` 都提到第一次比较之前 (先算完 kindB 再比 kindA==6);
   目标顺序是 `算 kindA → 比 6 → 算 kindB → 比 6 → kindA 比 7 → kindB 比 7`。
   写成 `if ((obj->headA.kindFlags & 0xF) == 6 || (obj->headB.kindFlags & 0xF) == 6 || ... )` 后,
   GCC 会在短路链内做 CSE (kindA 落 r2、kindB 落 r1, 后面两处 ==7 各自复用), 与目标逐条一致。
   (与 EXPERIENCE 72「fall-through 与显式 return 布局不同」同源: 语义相同但**求值顺序**不同的 C 出不同码。)
3. **4 处 DMA 全是 `DmaCopy32` + `DmaWait`** (不是 DmaCopy16!)。控制字 `0x8400_xxxx` =
   `(DMA_ENABLE|DMA_32BIT)<<16 | count`(本工程 `DMA_START_NOW=0`, 故高位是 0x8400 而非 0xC400)。
   形状 = `src/dst/ctl` 三次 `str` + **两次** `ldr` dummy 读 (一次来自 `DmaSetUnchecked` 的 `dmaRegs[2];`,
   一次是 `DmaWait` 的 `while` 被 loop-inversion 提到循环外) + `0x80000000` 等待环
   (`cmp r0,#0; bge` 前置测试 + 环内 `ands/cmp/bne`)。`sub_801B7B8` 里已有完全同形状样例可对照。

### 分数陷阱: 2400 是字面池重定位假分 (EXPERIENCE 29)
`extern` 全局符号在独立编译的候选 .o 里是未解析重定位, asm-differ 把池内容读成 0 → 与硬编码池值的
target.o 逐条 mismatch。把 base.c 里的 extern 换成硬编码地址
(`#define gGstate324 (*(u16 *)0x03000324)` 等) 后 **fndiff = 0**、permuter `base score = 0`。
两种写法字节完全一致 (bytecmp 掩码 bl 槽后 0 差异), 故 **src 保留符号写法, permuter base.c 用硬编码版**。
另注: `bytecmp.sh` 把 .text 链到 0 时, ROM 段函数符号距离 >4MB, ld 会在 .text 尾部插 interworking
veneer (本例 568→808 字节), 导致 `cmp` 报 64 字节差异 —— 全是 bl 槽, 需掩码后判定 (工具见
`.scratch/nova/bcmask.py`)。

### 新登记 / 修正
- `linker.ld` + `iwram.h`: 新增 `gUnk_03000344` (u8, 0x03000344, 位于 gGstate340 与 gDialogCtx 之间) —
  入口置 0x7F 哨兵、出口写本帧返回值; 全 ROM 目前仅本函数读写。
- `code_0.h` 原型修正 (四个函数**仅被本函数调用**, 已遍历全部 asm 确认无其它调用点):
  | 函数 | 旧 | 新 |
  |---|---|---|
  | sub_801B688 | `void ()` | `void (u8)` — asm 体 `lsls r0,r0,#0x18` 收 u8; 体内另有 `[sp,#4]` 读, 实参可能不止 1 个 |
  | sub_801D214 | `void ()` | `u8 (u8 *, u8)` — r0=gObjPoolPtr, r1=本帧结果 |
  | sub_8022458 | `void ()` | `u8 (u8)` — 入参 0x7F |
  | sub_8049D58 | `void ()` | `u8 (u8)` |

### 判定
`fncheck sub_8018070` OK (568 字节, 6 池重定位已施加, 16 bl 槽忽略); `touch src/*.c` 全量重编后
`make` rc=0 + `sha1sum -c ll.sha1` 绿 (794/1059); `permuter.py -j 1 --stop-on-zero` 报 `base score = 0`。

## 2026-09-11 zcode-engine: battle_engine 无卡点批次 (7 个认领, 5 匹配, 会话总结)

目标 = battle_engine.c 未匹配无卡点 <500 行共 29 个, 逐个不开 subagent。实际完成:

**✅ 匹配合入 (fncheck OK + 全量 SHA1 绿, 790→795):**
- sub_8046480 (110B): 行动槽处理; stride=0xC8 写循环体内(LICM 提升位), 指针加法第1参在前/0xAB 处 (u32)cast 形式三种地址写法; code_0.h sub_8045328 void→u8。
- sub_8046F0C (280B): 15 路 switch 取值器; case1-9 缩放 u16 指针 `*((u16*)obj+n)` 防 ldrh 位移折叠; 直接 return 使返回值落 r0; 无 default 直落 bx lr; 范围检查因 default 超条件分支射程(>254)自动变 bls+b 双分支。
- sub_8045BF4 (268B): switch(0xBE) 写 obj[0x8A]; 勿加 if-return (switch 自带范围检查, default=函数尾)。
- sub_8049C1C (316B): BGM 状态机; 0x03000918 用 ObjHead 视图 (数组+常量偏移会被树级折叠进池常量); case 自增用 <=(then臂); code_0.h void*→u8*。
- sub_8048DA4 (360B): 复位函数; DmaCopy16+DmaWait; 新登记 iwram 0x949/0x954/0x95A/0x974[]/0x979/0x97A; gUnk_03000970 用块级 u32 extern 避开文件后方 Unk_03000970* 视图冲突。

**⏸ 挂起 (语义/C 结构 100% 还原, 候选 base.c 已留 permuter/<fn>/):**
- sub_8048690 (差 1 字节!): GCC2 jump-threading 深度差 (x<=0 路径 b.n 目标), permuter base score=10 (归一化后视为相等, 该字节不可见)。
- sub_8046E18 / sub_8046558 / sub_8048310 / sub_80471AC / sub_8046CD4 / sub_8048458: 同族分配器三连坑 (store-flag 物化方向 / 参数 home 栈vs寄存器 / ref 序 home 级联), 详见 EXPERIENCE.md 经验 230。

**事故**: functions.tsv 更新脚本 split('\t',5) 吞换行导致 8046480/8046558 两行合并, 已按 git 基线重建 (见 INCIDENTS.md)。
**其余 <500 行无卡点目标** (80481B8/8049958/804753C/8047024/804519C/8047DC8/80472E8/8045D00/8047B1C/80492C0/804A148/8048FB8/8049DF8) 未及处理, 可用本文档同款流程。

### 2026-09-12 zcode-16d24 接管 sub_8016D24 (0x08016D24) — 第二轮, 仍未合入 ⏸
- 起点: TSV note 所述 61B 状态; 终态: **bytecmp 差 63B** (280B 全量) — 结构 100% 对齐, 剩纯寄存器 home/调度。
- 本轮新破 (61B→36B 级别的 5 项, 全部以字节判定回环验证):
  1. **0x4000 是错的, 目标常量 = 0x2000** (`movs r0,#0x80; lsls r0,r0,#6`): TSV 旧 note 的 0x4000 与调用方 sub_8017120 消费 `status & 0x2000` (联机会话错误标志) 互证。尾分支改回**单 return 三元** `? 0x2000|extra|status : status|extra`, 配 `siocnt.b.ID > 1u` 出 `bls`(低臂在前)。
  2. **(a) 已破**: ID 提取用 **u32 容器位域视图** (union { u32 full; struct { u32 BaudRate:2; SI:1; SD:1; ID:2; Error:1; Enable:1; ...}; };) 的 `siocnt.b.ID` → 正确产出 `lsls r6,#0x1a; lsrs r0,#0x1e`。该 union 已写入 base.c, 建议登记为 SIOCNT 的正式别名类型 (SioCntWord)。
  3. **si 形状**: 单语句 `si = siocnt.full & 4;` (掩码后一次 8 位截断), 不是两条语句。
  4. **modeBits u16 常量变量**: `u16 modeBits = 0x88;` (switch 前赋值) 买到 case0 的 `adds r4,r6,#0` 拷贝形状。
  5. **parentBit 提前装载**: `int parentBit = 0x80;` 在 counter++ 后、三元前装载, 臂内从变量取 → `movs rX,#0x80` 独立成条。
- 经验 87 双用尝试记录 (全部字节判定): `tmp` 兼职 case1 布尔 (`tmp = unk_2 != 0`) 有正收益 (-26B, 已留在 base.c); `status` 兼职 case0 常量 8 无效; `extra` 兼职 RMW 量 257B 恶化; `extra = modeBits` 通道只有 **long long extra** 才有效 (u32 同形状反而 210B)。
- ⚠ **long long 现象** (未采供, 用户禁止): `long long extra` + `extra = modeBits; mode &= extra;` = **36B**; 无 conduit = 38B; u32 同形 = 63/210B。8 字节宽度把 extra 的 qty pri ×2 (floor_log2(n_refs)*n_refs*size/life), 改变 block9/block16 的 local-alloc 排序 → mode 掩码/尾部 extra 的 home 全部归位。这是"改类型宽度买 home"的可复现案例, 但产码不可读, 弃用。
- **剩余 63B 全图** (mine→target, 逐字节): `2a-2d` case0 头 `adds r4,r6,#0` 与 `movs r0,#0x88` 调度对调 (掩码 home r0 vs r2/r4); `68/6a` RMW `ands r1,r0;strb r1` vs `ands r0,r1;strb r0` (tmp home r1 vs r0, 字节 home r0 vs r1); `cd-ea` 尾三元臂 dest r1 vs r0 (parentBit home r1 vs r0) + 目标多一条 `adds r2,r0,#0` (status 算完后拷进 bits 的 home r2, 再继续 OR); `ec-f7` extra: 目标直接 `lsls r1,r0,#0xf` (extra home = r1, 复用已死的 gUnk 基址 home), 我的落在 r0→`adds r2,r0,#0` (home r2); `f9-10b` 尾臂级联。
- 诊断: qtydump 的 block-9/16 表与目标逻辑一致, 差异发生在 global-alloc 层 (跨块 pseudo 165=gUnk 基址/87=status 等), qty 表看不到 (EXPERIENCE 88)。permuter 5 轮 (每轮 27k iter) 平台期 score 150-205。
- 下一步抓手: ① 找到让 gUnk 基址 (global pseudo) 在尾部提前死亡 (c6 `adds r7,r1,#0` 之后) 的源形状 → extra 能复用 r1; ② case0 掩码 home r0 需要"switch 前出生的 u32 首个常量 pseudo"落 r0 — 可试把 modeBits 也用于 case1/2 (加无害引用提高 refs); ③ 若项目某处已有 "8字节标量" 的合法人类用例可参照。
- 候选: permuter/sub_8016D24/base.c (63B, 人类可读, 结构全对); output-205-1 (38B, 含被禁 ll)。全量 280B 逐字节 diff 命令: bytecmp.sh sub_8016D24 base.c "gUnk_03004DF0 = 0x03004DF0;" "sub_8016E80 = 0x08016E80;"


## sub_80104F8 (0x080104F8, menu_ui, 传送/出生参数装载) — ⏸ 2026-09-12 gpnux-104f8
### 结论: 语义/结构 100% 还原, 卡在**一对寄存器 home 互换** (global-alloc 优先级墙)
候选: `.scratch/gpnux-104f8/T4.c` (自包含, permuter base 已更新为此文件)。
- 字节对比: `.text` 双方均 300B; `cmp -l` 52B 差异, **全部来自同一处**: 目标 `kind`→r6 / `tbl`→r5,
  我 `kind`→r5 / `tbl`→r6。指令流 **(助记符+操作数) 逐条一致**, 池布局/池倾倒点/分支距离全对。
- `fncheck`/字节门槛未过 → 未合入 src, 仓库保持绿。

### 语义 (与旧 note 一致, 已复核)
`kind = gScreenIdleIconIds[gScreenIdleIconCursor - 0xb + gMenuCursorSel];`
0 → msg 0x27; 8 且 `EventFlags_Test(0x10D)==0` → msg 0x1a; 0x18 且 `EventFlags_Test(0xFF)!=0` → msg 0x1a
(三条错误路共享 `strh + Sfx_Play(3,0,0) + return 0`)。
否则在 `gUnk_080981EE` 6B/项表里线性找首字节 == kind, 把 6 字节拆入
`gMapNpcSetId / gSpawnTileX / gSpawnTileY / gSpawnFacingDir / gMoveCmdSetId(u16) / gWarpAnimState=1`,
再 `gUnk_03004D4C = 0x34; SwitchFlags_ClearRange();` (无实参! 有实参会多一条 movs) `return 1`。

### 关键结构杠杆 (已全部命中, 记下来省后人时间)
1. **表基址必须是 u32 整数而不是指针**: `u32 tbl = (u32)gUnk_080981EE;` 且访问写成
   `*(u8 *)(i + tbl)` (索引在左)。用 `const u8 *tbl` + `tbl[i]` 时 C 前端会把 `i+tbl` 规范化成
   `tbl+i`, expand_binop 得到 `(plus base index)` → 目标却是 `(index, base)` (目标 6 处 `adds rX, rY, r5`)。
   **判据: 目标 `adds r0, r4, r5` (index 在前)** = 整数基址 + 索引写在左边的组合。
2. **p5 的第二个字节要 `(u16)` 强转**: `(*(u8 *)((u16)(i + 1) + tbl) << 8)`。直接写 `i + 1 + tbl`
   会被 fold 重新结合成 `((i+tbl)+1)` 并被 ARM 地址折叠成 `ldrb r0,[r0,#1]` (差 2 字节 + 连带池位移)。
   `(u16)(i+1)` 的 CONVERT_EXPR 让 split_tree 拆不动 → 保住 `adds r0, r4, #1; adds r0, r0, r5`。
3. **循环必须是 `do { if (*(u8*)(i+tbl) == kind) break; i = (u8)(i + 6); } while (1);`**。
   写成 `while (...)`/`for(;;)` 会被 GCC 旋转成"入口跳测试"形态 (26 处 diff); do-while 版逐字节对。
4. `SwitchFlags_ClearRange()` 无实参 (靠上一条 `movs r0,#0x34` 的残留 r0), 写 `(1)` 会多一条 `movs r0,#1`。

### 卡点: global-alloc 优先级 (已用 `-dg` 转储定量)
`tools/agbcc/bin/agbcc ... -dg` 会在 cwd 生成 `<name>.greg`, 内含:
```
Registers to be allocated in sorted order:
Register 23, refs = 17, live_length = 64   <- i,   pri 10625 -> r4
Register 22, refs =  6, live_length = 41   <- kind, pri 2926 -> r5
Register 24, refs =  9, live_length = 104  <- tbl,  pri 2596 -> r6
```
`global.c allocno_compare`: `pri = floor_log2(n_refs) * n_refs / live_length * 10000 * size`;
`flow.c:2457`: **`REG_N_REFS (regno) += loop_depth`** → 循环体内的引用按深度加权。
本例 kind refs=6 = def(1)+cmp0(1)+cmp8(1)+cmp24(1)+**循环 cmp(2)**;
tbl refs=9 = def(1)+**循环(2)**+p1..p4(4)+p5×2(2) ✓ 与 ROM 完全一致, 所以 refs 没有杠杆。
要让 tbl 先分配: 需 `LL_t/LL_k < 27/12 = 2.25`, 现为 104/41 = 2.54。
`live_length` = 寄存器活跃的指令条数, 只能靠"源结构改变 RTL 拷贝"来微调
(实测: 把赋值内联进 `if ((kind = ...) == 0)` 可把 LL_k 41→43, 但还不够)。

### 已穷举无效的杠杆 (全部保持 .text 300B/指令流一致后才算)
- 声明顺序 12 种排列、`kind` 类型 (u8/u32/int/s8/const/volatile)、`tbl` 类型 (u32/int/u16/const/指针)、
  死赋值 (`kind=kind`/`tbl=tbl`/`tbl|0`/`tbl+0`)、`if(!kind)`/`0==kind`、switch/else-if 链、语句顺序;
- p5 表达式 30+ 变体 (加数顺序/括号/`|`/`u16` 临时变量/`(u32)gUnk_080981EE` 各位置);
- 全队循环 4 种写法 × 表基址 2 种 (指针/整数) × p1..p5 直用 extern 与否 64 组合 (gen7 已扫);
- permuter 90s (score 130→65, 但 score 65 的产物 diff 反而更多 14 行; 评分与字节不对应)。
**判据: 任何"看起来能改 home"的写法都必须先过 `d.py` 的指令流 diff == 12 行才有效, 否则是改坏了代码。**

### 下一步建议
- 方向 A: 找一个能**延长 kind 活跃区间到 ~47 条** 或 **把 tbl 活跃区间压到 ≤92 条** 的源结构
  (两者都必须是"最终字节不变"的 RTL 级拷贝差异)。可考虑: 早期检查写成 `goto` 共享块
  (经验 107 的 flag 归约优先; goto 属铁律 4 禁项, 需谨慎)、或在指针 init 区插入会合并的拷贝。
- 方向 B: 打 `global.c` 转储补丁 (经验 216 路径 a), 看目标 .greg 是否可得 (需原版 C 才能复现, 不可行)。
- 方向 C: 换同类函数先做 (FAMILIES T6 组: sub_8010300 / sub_8010624 未匹配, 共享 gPartyStats 状态),
  用它们的 home 结论反推本函数的源结构。

### 工具 (本次自建, `.scratch/gpnux-104f8/`)
`d.py <cand.c>` 指令流对齐 diff (忽略标签名, 池差异一眼可辨);
`pri.sh <cand.c>` 编译 + `-dg` 转储 → 打印伪寄存器 refs/live_length + 寄存器 disposition;
`qty.sh` / `jj.sh` / `cnt.sh` / `rtl.sh` 转储探针; `gen*.py` 变体批量生成+打分。
⚠ `d.py` 的 diffs 计数在 **COMPILE FAIL 时会误报 0** (无 "!!" 行) —— 批量脚本必须显式检查 "COMPILE FAIL"
(本次就被这个坑骗过一次: `for (i=0; tbl[i]!=kind; ...)` 用 u32 基址编译不过却报了 diffs=0)。

## sub_8010300 (0x08010300, menu_ui, 157 行切片) — ⏸ 2026-09-12 nova 接管后推进到差 42 字节, 未匹配

### 背景
claude-300 于 2026-09-05 认领后停在「bytecmp 剩 80 字节」。2026-09-12 经确认其锁已 7 天无更新
(note 空), 由 nova 接管 (原锁备份在 `.scratch/nova/sub_8010300.lock.bak`)。

### 语义 (已 100% 确定, 与前人一致)
道具使用入口 / `VBlankIntr` 无关的普通函数, 返回 0/1/2:
- `itemId == 0` → `gUnk_030001C8 = 0x27`, 返回 0
- `itemId == 0x3E` → 若 `!(gUnk_03002C44 & 0x80)` 则 `gUnk_030001C8 = 0x24`, 返回 0; 否则落到公共段
- 其它 → `memberId = gPartyMemberIds[(u8)(gMenuCursorStack[0] - 1)]`,
  `charaId = memberId ? memberId - 1 : 0`, `power = ItemGetUsePower(memberId, itemId)`,
  MP 检查后写 `gUnk_030001C4`; 为 0 则 `gUnk_030001C8 = 0x1d`, 返回 0
- 公共段: `entry = &gSkillLearnTable[(itemId-1)*5]`, `gUnk_030001C5 = entry[1] & 0xF`,
  `gUnk_030001C6 = entry[3]`
- `itemId == 0x26` → `WarpTable_Check() != 0` 返回 1, 否则 `gUnk_030001C8 = 0x27` 返回 0
- 否则扫前 5 个队员, 统计 `hp < max_hp` 的人数; 非 0 返回 2, 否则 `gUnk_030001C8 = 0x1c` 返回 0

### 本轮推进 (掩码 bl 槽后的字节差: 101 → 66 → 42)
判定用 `.scratch/nova/score300b.py`(编译 + objcopy .text + 掩码 bl 槽后逐字节比, 并打印 prologue
push 列表) 与 `.scratch/nova/idiff.py`(指令流对齐 diff)。**不要用 fndiff 的分数做定量** ——
extern 符号版有字面池重定位假分 (经验 29), permuter 自己的分数又与字节差不线性。

1. **循环: hp 与 max_hp 必须各自取到局部 `u16`(关键突破, 101 → 66)**
   直接写 `if (gPartyStats[id].hp < gPartyStats[id].max_hp)` 时, agbcc 每轮都从字面池重载
   `0x03004AC0`; 目标却是把 stats 基址提到 r4、ids 基址提到 r5 复用。
   写成 `u16 hp = gPartyStats[id].hp; u16 mh = gPartyStats[id].max_hp; if (hp < mh) count++;`
   后基址即被提升。(permuter 独立跑出的 `new_var = 0x03004AC0` 是同一机制的另一种写法。)
   只取其一 (`u16 hp = ...; if (hp < gPartyStats[id].max_hp)`) 无效, 仍 101。
2. **MP 检查: 唯一剩余卡点**
   目标 (0x62-0x80):
   ```
   ldrh r4, [r4, #4]   ; mp 复用 charaInfo 的 r4
   cmp  r0, r4         ; 比的是 power, 且 power 留在 r0
   bls.n 78            ; -> adds r1, r0, #0   (result = power)
   movs r1, #0         ; result = 0
   b.n 7a
   [字面池 3 words]      ; 池正好在 b 之后断开
   78: adds r1, r0, #0
   7a: ldr r0, =gUnk_030001C4; strb r1; cmp r1, #0
   ```
   agbcc 对 `if (power > mp) result = 0; else result = power;` **一律**做 value-replacement,
   把复制提到比较前并改用 result 比较:
   `ldrh r1,[r4,#4]; adds r2,r0,#0; cmp r2,r1; bls; movs r2,#0`(少一条 `b`, 池也就不断开)。
   已排除的写法 (全部 66~193, 无一命中): if/else、三元、先 `result = power` 再清零、
   先 `result = 0` 再条件赋、`if (mp < power)`、`!(power <= mp)`、`u16 result`、
   4 种局部变量声明顺序、`PlayerStats *st`/`PlayerStats *c` 指针、goto 形式、
   分支内重复存储、`gUnk_030001C4 = result = (...) ? ... : ...`。
   **唯一能保住分支形状的是在 else 分支读一个尚未赋值的变量作屏障**:
   `else if (result) result = power; else result = power;` → 差 **42**, 且池在 `b` 后正确断开。
   GCC 能证明两条路径赋同一值, 生成码不依赖未初始化值 (语义安全), 但 `result` 因此
   「从函数入口即活跃」→ 被分配到 **r6** → `push {r4,r5,r6,lr}`, 而目标只 `push {r4,r5,lr}`。
   对 barrier 变量做全扫描 (result/power/memberId/charaId/count/id/i/entry/charaInfo):
   只有 `result` 能阻止 value-replacement, 其余全部回落到 67(即被提前复制的形状)。
3. **0x34 处两个池常量的载入顺序**(次要): 目标是先 `ldr r1, =gPartyMemberIds` 再
   `ldr r0, =gMenuCursorStack`(两条紧邻), 我的一律是先光标后基址(基址 `ldr` 被推到索引算完之后)。
   试过指针变量(顺序对了但占用 r6 破坏栈帧)、`*(base + idx)` 指针算术(无效)、`idx` 先算到变量(无效)。

### 产物
- `permuter/sub_8010300/base.c` — 66, 干净人类码, 可继续迭代
- `permuter/sub_8010300/base_barrier42_UB.c` — 42, **非人类码 (读未初始化 result), 禁止直接合入**,
  文件头已写警告
- 工具: `.scratch/nova/score300b.py`(批量打分 + push 列表)、`.scratch/nova/idiff.py`(指令流对齐 diff)、
  `.scratch/nova/diffsem.py`(忽略格式化/常量内联的 C 语义 diff, 用来看 permuter 到底改了什么)
- 前人候选备份: `permuter/sub_8010300/base_claude300.c.bak`

### 下一步建议
- 主攻 result 的寄存器归属: 需要一种既阻止 value-replacement、又不让 `result` 从入口活跃的写法;
  或者接受 r6 版本后, 再想办法把别处的压力降下来让 r6 变回 r1(当前 r4/r5 被 itemId、charaInfo 占死)。
- 可考虑 dump agbcc 的 RTL(`-dr`)看 value-replacement 具体发生在哪个 pass, 再决定用什么源码形状规避。
- 若放弃字节匹配, 66 版语义与结构完全正确, 可作为「等价但未匹配」候选留档。

### 补充 (2026-09-12 07:2x): RTL 级定位 —— 提升发生在 agbcc 的 `combine` pass, 且干净 C 无法绕开

用最小复现 + `agbcc -da` 全 pass dump 对比, 结论如下 (复现文件 `.scratch/nova/rtl_*.c`,
dump 在 `.scratch/nova/rtl/`, 追踪脚本 `.scratch/nova/trace_tA.py`):

1. **`if (c) result = 0; else result = power;` 必被提升。**
   初始 RTL (`h.i.rtl`) 还是自然展开: compare 在 insn 51, 两个分支赋值在 insn 55 / 61 (都在其后);
   到 `h.i.combine` 时第一个赋值已经跑到 compare 之前。后续 pass 只是维持。
   生成码形状 = `add r1,r0,#0; ldrh r4,[r4,#4]; cmp r1,r4; bls; mov r1,#0`(比较用的是 result)。
2. **三元式产生的是另一种形状, 也不是目标。**
   `result = (c) ? 0 : power;` / `gC4 = (c) ? 0 : power;` → `mov r1,#0; ldrh r4,[r4,#4]; cmp r0,r4;
   b?? .L; add r1,r0,#0; .L: strb`(true 值预先物化, 无 `b` 跳过 else 臂)。
   注意这一形状**比较用的确实是 power 且 mp 复用了 charaInfo 的 r4** —— 与目标一致的两点!
   但缺 `b.n` 与独立 else 臂, 所以池不会在 `b` 后断开, 字面池布局仍对不上。
3. **目标的形状 = 未被 combine 提升的自然 if/else 展开** (`cmp; bls else; then; b merge; else:; merge:`)。
   唯一能让 combine 不提升的写法是让 else 分支存在两条赋值路径, 而 GCC 只有在
   **条件读了尚未赋值的 result** 时才放弃折叠 (试了 memberId/charaId/count/id/i/entry/charaInfo/power
   作屏障条件, 全部回落到被提升的形状 67; 只有 `else if (result)` 有效 → 42/33)。
   代价是 `result` 因此「从函数入口即活跃」→ 被分到 r6/r7 → 多 push 一个寄存器, 目标只 push {r4,r5,lr}。
4. `register` 修饰、`do{}while(0)` 包裹、`result` 声明顺序、`result` 为 u8/u16/int 均无影响 (全 66/67)。

**结论**: 目标 MP 块的形状在 agbcc(GCC 2.9 thumb) 下无法由「干净 C」产生 ——
`if/else` 必被 combine 提升, `?:` 必是预物化形。要字节一致只有两条路:
(a) 找到一种让 combine 不提升、且不读未定义值的 C 形状 (尚未找到, 上面已穷举 ~40 种写法);
(b) 接受等价但非字节的 66 版 (`permuter/sub_8010300/base.c`), 语义与结构已 100% 正确。
后续若有人攻坚, 建议直接从 (a) 的「combine 为何传播这条拷贝」入手 (dump 见 `.scratch/nova/rtl/h.i.combine`),
或者考虑该函数是否真的来自 `if/else` —— 也可能源码用了别的结构 (如带副作用的宏) 产生了这个形状。

### 本轮最终状态
- 干净人类码候选: `permuter/sub_8010300/base.c`, 掩码 bl 槽后字节差 **66** (与目标 308 字节等长)
- 最接近候选: `base_barrier42_UB.c`(42) / permuter 爬到 33 —— 都依赖读未定义值, **禁止合入**
- 未改 `src/` (铁律 6: 未字节一致不得合入); `make` + `sha1sum -c ll.sha1` 保持绿


### sub_80104F8 二期 (2026-09-12 gpnux-104f8b): global-alloc 机制破案, 确认为真墙
一期卡在 kind/tbl 一对 home 互换后, 二期深挖 GCC2 pass 流水线, 把"墙"从玄学变成定量结论:

**机制破案 (详见经验 246)**
1. `REG_LIVE_LENGTH/REG_N_REFS` 在 **cse2 之后的 flow(life_analysis)** 计算 (toplev.c:3006, 唯一一次),
   之后被 regmove (move src/dst 间转移) 和 local-alloc 的 `update_equiv_regs` 修改
   (`REG_EQUIV` note 持有者 `LL *= 2`)。计算 LL 的 RTL 是 cse2 后、combine 前 —— 与 -dj 转储差 ~40 条指令。
2. **`.greg` dump 的 refs/LL 是 reload+life2 重算的装饰值**; global_alloc 的真实输入看 `.lreg` dump 头部。
   本次两处数值恰好一致 (kind 6/41, i 17/64, tbl 9/104), 但机制不同。
3. **池加载 → REG_EQUIV → LL×2**: CSE 给 `set reg (mem/u (symbol_ref/u "*.LCn"))` 加 REG_EQUIV(symbol),
   update_equiv_regs 翻倍其 LL。数组变址加载 (kind) 永远拿不到 → kind=41 奇数, p1..p6/tbl/池临时全偶数。
   tbl 的 raw=52, 翻倍=104 —— **指针/基址变量优先级天然腰斩**。
4. **LIM 把循环用到的池加载钉死在预头**: tbl 的 def 落在 `i=0` 之后、其余池加载之前, 与源码顺序无关
   (R1 重排实验: LL 不动)。
5. **QI round-trip 杠杆 (+2 LL, asm 不变)**: `if ((x = 内存加载) == 0)` 的赋值表达式值被条件使用时,
   GCC2 生成 QI 副本 + 零扩展两条指令 (x 活跃其间), 最终被 combine 删除。仅对内存加载 RHS 生效;
   `(f = call) == 0` 无效 (call 结果已在寄存器), `(x = x)` 被前端折叠, `(k = (x = load))` 链式反而被 k 吸收。
6. **指针变量是化妆品**: 删光 p1..p6 改直接符号存储 (V_NP.c), asm 逐字节不变。

**定量结论**: 翻转需 `f2(rk)·rk/LLk > f2(rt)·rt/LLt`, 即 kind 侧 `12·LL_t < 27·LL_k`。
最优形态 V_inline.c = (LL_k 43, LL_t 104): 1161 < 1248, 还差 8 分 —— 需 LL_k≥47 (再 +4) 或 LL_t≤96 (再 −8)。
穷尽无效的: i/kind 类型 (u8/u32/int/u16/s8, I32 改坏 asm: u8 截断舞是**真指令** lsls/lsrs)、
预头重排 (LIM 钉死)、p-def 前移、f/g flag 变量 (call RHS 无 round-trip)、自赋值、赋值链、
switch/else-if、循环 4 形态、p5 30+ 变体、permuter 61 轮 (base score=130=0 errors, 它也确认仅剩 home 互换)。

**结论**: asm 逐条钉死 → post-cse2 RTL 钉死 → LL 钉死, 每个自由度都验证过 —— 真墙。
permuter/base.c 已更新为 V_inline (43)。后续若要再攻: 需要找到"源结构改变 post-cse2 RTL 但最终 asm
不变"的**新**机制 (regmove 的 LL 迁移是唯一未深挖的 pass —— 若某源形态能触发 regmove 把 tbl 的 LL
转给短命 temp, 即可翻转)。

## sub_8045A10 (0x08045A10) — MATCH-45A10-20260912-a: 单块内被减数 home tiebreak (2026-09-12)

**目标**: `u8 sub_8045A10(u8 *obj, u8 index)`, 100 字节。语义 = 把 `obj[0x70]`(s16) 与"经两次
`sub_804E76C(obj,3,1/2)` 调整的技能值 amount"相减, 差 < 0 返 0 否则返 1。

**E2 锚点**: 前 32 条指令 (push..`lsrs r4,r4,#1`) 与**已匹配** `sub_8048934`(0x08048934) 逐条相同 ——
两者是同一 `gUnk_08093418`(行宽 5, 取 `[id*5+4]`) + 两次 `sub_804E76C` 调整的同构家族, 仅尾部消费不同:
- sub_8048934: 直接 `return amount;`
- sub_8045B90(已匹配): `current(obj+0x70) -= amount; if ((s16)current<0) current=0; *current=...`
- sub_8045A10(本函数, 未匹配): `if ((s16)obj[0x70] - amount < 0) return 0; return 1;`

**唯一卡点 (尾部 0x08045A56 起 4 条)**:
```
32: lsrs r4, r4, #1          (amount >>= 1)
33: adds r1, r4, #0          <- 把 amount 拷到第二个伪寄存器
34: lsls r0, r6, #0x10       (sext obj[0x70])
35: asrs r0, r0, #0x10
36: subs r0, r0, r1
37: lsls r0, r0, #0x10       (结果截回 s16)
38: cmp  r0, #0
39: blt  <ret0>
```
即: 相减时被减数(amount)在一个**独立 home** 里, 与 sext 后的 obj[0x70] 由一条 copy 相连。
自然 C (`(s16)original - amount`) 会让 amount 与 sext 结果直接同槽相减 → 第 33 条恒不出现。

**证据/方法**: 自建 `bc2.sh` 把候选与 target.o 施加**同一 abs.ld** 后再逐字节 cmp, 消除跨对象 BL
(到 0x0804E76C 超 ±4MB) 触发的 veneer 假差; 该方法用已匹配 sub_8048934 的抽取真 C 验证为 OK(96B)。
项目自带 `bytecmp.sh` 对 target.o 不做同环境链接, 本函数会报 8 字节假差 —— 判定时须用 bc2.sh。

**穷举 (>10k 变体, 全排除)**: 比较式/强转 40+ 形态; 中转变量 9 种类型 × 3 赋值时机 × 6 赋值形式;
`>>=` vs `= >> 1` vs `/2`; 双臂 if/else phi (实测**确实**产出 `adds r1,r4,#0`, 但移位落 r1 且多一条
`b`, 见 be4); 变量兼职(经验 87, 单汇合块不成立); register 存储类; 声明顺序; `-O2` vs `-O2 -g`;
permuter >4 万次迭代 (5 个不同 base)。

**结论**: 纯 global-alloc home tiebreak (经验 87/88 型)。全 ROM 中"`adds r1,rN,#0` 后紧接
`subs r0,r0,r1`"只出现 2 处 (本函数 + Chara_GetDrawX, 后者机制不同)。字节完全一致仅能靠**人工内存 home
载体**达成 (局部 `struct{u64 w;} s;` / union / `u64 limit` 取址), 属凑形, 按铁律 6.5 未合入。
自然最优候选只差那 1 条 copy (`cand_natural_nearest.c`) 或 46 条全对仅 4 条 home 差 (`cand_natural_m4.c`)。
交接: `docs/handoffs/MATCH-45A10-20260912-a.md`。

## 2026-09-12 workbuddy (GLM): sub_801ED40 匹配 ✅ (75.4%)

**✅ sub_801ED40 (300B)** — BattleObj 白色着色启动 (sub_801EE6C 清除的逆操作):
color 未初始化 RMW 白化 (三连 &0xFFFFFF00/&0xFFFF00FF/&0xFF00FFFF | 0x1F/0x1F00/0x1F0000,
高 8 位为栈残留) + gUnk_03000765=headA->palSlot+1 + 三分支 sub_804B654 调用族
(slot≤0xA 仅 mode2 登记 / ≤0x70 先 mode3 后回退 / >0x70 mode3) + (s8) 符号测试 +
共享 `kindFlags |= 0x8000` 尾。签名升为 `void sub_801ED40(BattleObj *, u8)`;
BattleObj.pad_6C 拆出 **f_AB** (+0xAB: ==4 时取 gUnk_03000744, 与 sub_801EE6C 共用证据)。

**卡点破解 (前次 09-06 挂起的 "3 高位寄存器 vs 目标 0")**: 根因不是寄存器分配本身, 而是
外层分支反写 (`> 0xA`) 导致 GCC 把 A 块外移 + 调度全变; 改 `<=` 正向写后 r6/r7 双 home 自然复现。
**四个形态级教训** (bytecmp 逐级 234→115→34→32B, 全为 bl 槽前只差尾块 2B):
① B 内选择必须 if/else 而非三目 (三目 beq 出跳, 目标是 bne 出跳);
② ret 必须 u8 类型 + `(s8)ret >= 0` 测试 (s32 少一条 lsrs 截断);
③ `mem |= CONST` / `mem = CONST|mem` 都打不中目标形状, 必须命名中间变量
   `flags = 0x8000 | obj->headA.kindFlags; obj->headA.kindFlags = flags;`
   ⇒ ORR 结果独立 home (adds r0,r2,#0 + orrs r0,r1, 常量拷贝进累加器);
④ 结构体成员访问与 u8* 字节访问字节级等价 (headA.palSlot/kindFlags/f_2F)。

验证: bytecmp 32B 差全落 8 个 bl 槽 (target.o 占位 vs veneer 解析, 方法论伪差);
`fncheck sub_801ED40` **OK** (298B); 全量 make + sha1 绿。交接: docs/handoffs/MATCH-ED40-20260912-a.md。
套件: permuter/sub_801ED40/ (candidates/struct.c = 最终形态)。

**追加**: sub_801EE6C 签名同步升级 `void sub_801EE6C(BattleObj *ptr)` (slot/f_AB/headA.palSlot/
kindFlags 结构体化; slot 两次读取保持独立, 目标即两次 ldrb)。bytecmp 12B=3bl 槽伪差,
fncheck OK 120B, sha1 绿, ED40 复验 OK。code_0.h 原型同步; 无 C 调用方零波及。

**追加 2**: sub_801EC3C / sub_801FA10 签名同步升级 `(BattleObj *, u8)`。
BattleObj.pad_6C 再拆出 **animPtr** (+0x88, 动画/图形数据块 u8* 指针, [0x2]/[0x1A]/[0x20]
u16 索引入口, 多处独立消费证据)。EC3C bytecmp **OK 260B 全等** (纯函数零 bl),
FA10 bytecmp 8B=2bl 槽伪差, 调用方 sub_8020F4C 加 cast 后 fncheck OK。
中途 sha1 红 = 并行 agent (claude-09/battle_engine.c) 中间态, 非本改动; 最终 sha1 绿 75.5%。

## 2026-09-12 gpnux: sub_801E4D4 ✅ (800/1059)

战斗对象效果函数 (232B, battle_obj_core.c)。**ROM 全扫描零引用 = 死代码** (姊妹 801E30C/E690 同;
活跃的是 801E1D8/E040)。语义: arg0[0xBC] 选表 gUnk_08393B28[idx].field_10 模式 0/1 ——
对 arg1 (单体) 或 0xC8 步长成员数组 (5/7 档) 做 [0x6C]-=[0xB2] 下溢清零扣减, 回绕经
sub_8020B90 语义入队 (gUnk_030006F8/714/718), 任一回绕则返回 1。

形状四要点 (详见 docs/handoffs/MATCH-801E4D4-20260912.md):
① v 用 `switch (加载表达式)` 而非 `v=…;switch(v)`, 后者寄存器分裂多 copy;
② case1 idx 拆 `s32 t = h+3; idx = t + b;` 两语句 —— 单语句被 CSE 重结合, u16 中间变量插截断;
③ `limit = (cond) ? 7 : 5` 三元式保字节加载先于 movs r7,#5 (两语句形式颠倒 → 上会话残留的
   ll.gba 10 字节差 0xd4..0xdd, 与本次 fncheck 差异完全吻合, 全量构建后消除);
④ 入队走 `static inline Inl_QueuePushObj` (= sub_8020B90 体), 手写内联打不中 muls 全链原位 r1。

教训: ll.gba 是共享构建产物可含陈旧中间态, 字节对比基准一律 baserom.gba。
fncheck OK 442B; 全量 make + sha1 **绿 75.5%**。code_0.h 原型同步 (无调用方零波及)。

**追加 3 (2026-09-12 gpnux): battle_obj_core.c 全量结构体化签名, sha1 绿**

BattleObj 补字段 f_6E/f_A2/f_B4/f_B6/f_C3 (布局不变, 全部沿 offset 命名;
f_6C/f_B2/f_C2 为前一会话已提升)。签名改造 30+ 函数:
- ObjHead 组: B570/B81C/B878/B8AC/B8FC/20974 (Unk_801B81C 视图退役 = ObjHead 别名);
- BattleObj 组: CE80/D12C/D19C/DB3C/DC20/DD04/DDB0/DEDC/DF90/E4D4/E040/
  20B90/20BC0/20BF0/20C2C(fnptr)/20C58/207DC/20840/208A4/20914/2093C/20A0C/20A7C/20974;
- 未匹配函数原型同步: CA08/CBA4 (BattleObj*,u8,u16,u8,u8), D568 (BattleObj*);
- iwram.h: gUnk_030006F8 → struct BattleObj *[7] (唯一消费者 battle_obj_core);
- 有意保留 u8*: 209C8 (+0x88 u16 半字写语义不明), 20B04 (arg0 语义不明), MyStruct 视图 (209EC);
- ldrsh 语义位用 *(s16 *)&obj->f_6C 形式 (u16 字段 + (s16) 别名读, 单指令 ldrsh);
- DC20 的 *(u16*)&animPtr=0 保持半字写 (animPtr=0 会变 u32 str);
- 外部调用点 cast 修整: battle_anim/sio_link 去 cast, scene_obj_fx/cutscene_mgr/
  event_hub/battle_engine 加 cast (指针传递零代码生成)。
教训: 批量替换漏网 `arg1[0xBE]` (u8* 下标 → BattleObj* 下标按 0xC8 缩放 → invalid operands),
编译器能抓到; 但 `ptr + off` 形式的静默缩放抓不到, 必须逐函数核对。
fncheck: battle_obj_core 57/57 + battle_engine 62 + scene_obj_fx 27 + cutscene 3 +
event_hub 27 + battle_anim 54 + sio_link 71 全 OK。

**追加 4 (gpnux): 结构体化的两处"静默缩放"陷阱 (血泪教训, 已修复)**

全局 gUnk_030006F8 由 `u8 *[]` 改为 `BattleObj *[]` 后, `gUnk_030006F8[0] + 0xBE`
不再是字节偏移: 指针算术按 sizeof(BattleObj)=0xC8 缩放, 生成 `muls #0xC8` 多 4 字节。
此 TU 全绿但**全量链接失败**: .rodata 段紧跟 .text, 绝对边界 0x61C784 被撑破 →
`ld: cannot move location counter backwards (0x61C788 → 0x61C784)`。
定位手段: 全项目 fncheck 找字节数异常者 (sub_8020C2C 48B vs 参考 44B 即命中), 修
`gUnk_030006F8[0]->slot` 后恢复 44B, 链接通过。
同类: `arg1[0xBE]` (u8* 下标) 改型后会按 0xC8 缩放 —— 编译器能报 "invalid operands",
但 `ptr + off` 形式**静默**生成错误取址, 编译器和 fncheck 单函数检查都抓不到, 只有
全量 sha1 能兜底。改型必须逐函数核对所有 `指针 + 偏移` / `指针[整数]` 表达式。

**追加 5 (gpnux): sub_801FEBC 签名 void* → BattleObj*** (无调用者)。

退役本地 Unk_8020F4C 视图 (与 BattleObj 精确同构: state/headA.kindFlags/headA.f_2B/f_2C),
改 `BattleObj *arg0`; 字节 132B OK。**更正**: 原保留的 `p=&state` 递减裸指针造型被注释
为"匹配必需", 实为错误——A/B/C 强制重编对照证明全字段访问 `arg0->headA.f_2B/f_2C`
产出完全相同字节, 已清理。battle_obj_core 57/57 全绿。

**追加 6 (agent-e690/gpnux, 2026-09-12): sub_801E690 匹配 (801E4D4 姊妹, 家族 T3 收官)**

同日第二个战果: sub_801E690 (230 asm 行/438B) bytecmp 一次通过 (440B 含字面池),
fncheck OK, 全量 make + sha1 通过。MATCH-801E4D4 handoff 的"套用形状经验"预言完全兑现。
- 方法: E4D4(主体/尾部字节同构) + DF90(dispatch 字节同构) 的现成 C 形状直接拼装;
  dispatch 差异仅查表入口: mode0=*(u16*)(animPtr+2), mode1=*(u16*)(animPtr+8+f_C2*2)
  (f_C2 即 animPtr+0x3A 处 u8 副索引, 与 DF90/DEDC 模式完全一致)。
- E4D4 两条形状规则再次生效: ①field_10 用 switch(*(u16*)((u8*)entry+0x10)) 直派发
  (r3 直落尾块) ②limit 三元式。case 1 无 E4D4 的 s32 t 两语句问题 (本函数 idx 无加法链)。
- 签名 void sub_801E690() → u32 sub_801E690(BattleObj*, BattleObj*) (code_0.h:577)。
- 剩余: 同族最后一个 sub_801E30C (237 行, status=0), 预计同套路。
