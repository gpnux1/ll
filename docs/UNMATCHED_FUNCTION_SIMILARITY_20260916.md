# 未匹配函数按代码功能聚类

本分析基于 `functions.tsv` 中 `status=0` 的 164 个函数，结合 `asm/nonmatchings/*.s` 的指令二文法相似度、常量池、调用函数和访问的 IWRAM/硬件地址分类。函数名不作为判断依据。

## 1. 聚类总览

| 类别 | 数量 | 代码证据 |
|---|---:|---|
| `stage.dialogue_handler` | 22 | gObjActStep, 0x03000820/825/822/824, textbox sub_801A2AC, keys, Sfx, palette release, sub_8020974. |
| `stage.effect_handler` | 17 | gObjActStep, target pool sub_80462E4/sub_80489E8, Rng, damage/effect helpers sub_801CA08/sub_801CE80/sub_8020CC4, gUnk_08393B28. |
| `menu.inventory_equip` | 10 | inventory/equip/skill menu pages and item operations. |
| `core.skill_effect` | 9 | RNG, equipment/skill tables, stat/effect checks, float/stat-mod code. |
| `menu.hud_text` | 8 | HUD/text glyph/timer/numeric drawing. |
| `menu.window_list_state` | 8 | gUnk_03000768/769/76A/76B/76C/76E, menu list/window phase, dialog ctx. |
| `stage.actor_handler` | 8 | gObjActStep + ObjHead animation command services; several current module names mislead because they sit in menu/actor files. |
| `menu.title_save_ui` | 7 | title/save/option menu state and drawing. |
| `battle_object.effect_queue` | 6 | effect queue, damage popup, recovery/hit display, scene timers. |
| `core.result_dialog` | 6 | battle result screen, EXP/level/items dialog state, command table cursors. |
| `core.target_filter` | 6 | GetObjPool + sub_8045F10, filters 0x1FF/0xFF/0x7F, collects slot buffers. |
| `battle_object.spawn_animation` | 5 | object spawn, formation, enemy/special object setup, animation load switching. |
| `battle_object.head_command_loader` | 4 | ObjHead command stream parsing, jump table, DMA to OBJ VRAM/WRAM. |
| `battle_object.ui_drawing` | 4 | 3-digit/box drawing, dialog context, battle UI tilemap helpers. |
| `map.actor_move` | 4 | character movement, collision/followers. |
| `map.viewport_camera` | 4 | viewport bounds, VBlank pipeline, tile cache, camera draw mode. |
| `menu.dialog_tile_render` | 4 | battle dialog tilemap 0x02035AC0, tile DMA table 0x0839B462/0x0839CFBA, tile attr 0xB001. |
| `palette.fade` | 4 | OBJ/BG palette animation entries, DMA palette backup/restore. |
| `core.damage_calc` | 3 | damage numbers, hit/float/counter, HP/MP clamps. |
| `core.turn_sort` | 3 | quicksort partition/action ordering, agility evaluation. |
| `map.scene_load` | 3 | MapScene descriptor load, NPC IDs, event animations. |
| `script.dialog_text` | 3 | script dialog message/text window handlers. |
| `script.tile_dma` | 3 | script tile DMA and window/dialog refresh handlers. |
| `item.drops` | 2 | post-battle drop RNG/table and inventory write. |
| `sio.link` | 2 | SIO serial communication session/state. |
| `stage.service_dispatcher` | 2 | Stage handler dispatcher and shared tail service; very high fan-in. |
| `battle_object.anim_dma` | 1 | animation DMA sequence driven by menu/obj state. |
| `battle_object.loop` | 1 | BattleTask_Run main battle loop. |
| `item.use_fx` | 1 | item use stage machine for weapon/consumable. |
| `menu.choice_input` | 1 | choice menu input and event branch. |
| `palette.wipe` | 1 | OAM HPos/VPos capture and wipe progress. |
| `script.chara_control` | 1 | script character walk/party control. |
| `stage.service_helper` | 1 | Small helper shared by stage/dialogue tile rendering. |

## 2. 建议优先处理顺序

1. `stage.service_dispatcher` + `stage.dialogue_handler`：共享状态机模式最多，先命名/拆解 `gObjActStep` 族，收益最大。
2. `core.target_filter` + `battle_object.effect_queue`：大量目标槽筛选和伤害数字队列是重复家族，先做相似函数统一建模。
3. `menu.dialog_tile_render` + `core.result_dialog`：战斗结果/对话框 tile DMA 是同一渲染族，适合一起反推 helper。
4. `battle_object.head_command_loader`：`sub_801A884`/`sub_801AD0C`/`sub_801B0B8` 都是 ObjHead 命令流/DMA loader，函数体长且影响面广。
5. `script.dialog_text`/`script.tile_dma`：脚本 opcode handler 内部互相调用，先处理小 helper 再攻大 handler。

## 3. 模块归属修正线索

- `battle_menu_windows.c` 不只是菜单窗口：里面有 stage actor handler、dialog tile renderer、target filter。
- `battle_stage_dialogue.c` / `battle_stage_effects.c` / `battle_stage_actor.c` 实际是同一 stage handler 家族的不同 handler 段；应继续按 handler 类型而不是旧 `event_*` 名组织。
- `battle_task_services.c` 中大量未匹配函数是 battle UI 绘制（数字/边框/dialog tilemap），不是 link service。
- `menu_ui.c` 中多个大函数是 inventory/equip 状态机，不是普通 UI helper。
- `battle_flow_rules.c` 应至少拆成 target filter、damage calc、turn sort、result dialog 四族。
- `battle_object_engine.c` 应至少拆成 head command loader、spawn animation、effect queue、UI drawing 四族。

## 4. 全部分类表

| 类别 | 地址 | 当前模块 | 当前名 | 指令数 | 相似聚类 | 现有分析线索 |
|---|---|---|---|---:|---:|---|
| `stage.dialogue_handler` | 0x08032ea0 | battle_stage_dialogue | sub_8032EA0 | 607 | cluster0 |  |
| `stage.dialogue_handler` | 0x080334b8 | battle_stage_dialogue | sub_80334B8 | 458 | cluster0 |  |
| `stage.dialogue_handler` | 0x08033988 | battle_stage_dialogue | sub_8033988 | 442 | cluster0 |  |
| `stage.dialogue_handler` | 0x08033e2c | battle_stage_dialogue | sub_8033E2C | 595 | cluster0 |  |
| `stage.dialogue_handler` | 0x08035360 | battle_stage_dialogue | sub_8035360 | 511 | cluster0 |  |
| `stage.dialogue_handler` | 0x08037388 | battle_stage_dialogue | sub_8037388 | 491 | cluster0 |  |
| `stage.dialogue_handler` | 0x08038920 | battle_stage_dialogue | sub_8038920 | 341 | cluster0 |  |
| `stage.dialogue_handler` | 0x08038e44 | battle_stage_dialogue | sub_8038E44 | 188 | cluster52 |  |
| `stage.dialogue_handler` | 0x08039724 | battle_stage_dialogue | sub_8039724 | 493 | cluster0 |  |
| `stage.dialogue_handler` | 0x08039c38 | battle_stage_dialogue | sub_8039C38 | 806 | cluster0 | ⏸ .s被跳转表截断, 需先补全反汇编 |
| `stage.dialogue_handler` | 0x0803a478 | battle_stage_dialogue | sub_803A478 | 412 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803a8d0 | battle_stage_dialogue | sub_803A8D0 | 642 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803af60 | battle_stage_dialogue | sub_803AF60 | 506 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803b484 | battle_stage_dialogue | sub_803B484 | 746 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803bbec | battle_stage_dialogue | sub_803BBEC | 708 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803c328 | battle_stage_dialogue | sub_803C328 | 1109 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803ce0c | battle_stage_dialogue | sub_803CE0C | 377 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803d20c | battle_stage_dialogue | sub_803D20C | 376 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803d60c | battle_stage_dialogue | sub_803D60C | 870 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803decc | battle_stage_dialogue | sub_803DECC | 654 | cluster0 |  |
| `stage.dialogue_handler` | 0x0803e58c | battle_stage_dialogue | sub_803E58C | 802 | cluster12 |  |
| `stage.dialogue_handler` | 0x0803ed34 | battle_stage_dialogue | sub_803ED34 | 521 | cluster12 |  |
| `stage.effect_handler` | 0x080264c0 | battle_stage_actor | sub_80264C0 | 516 | cluster1 | ⏸ 2026-09-13 agent-80264c0: 语义完整还原 (战斗对象"合击/连锁演出"状态机, 双参 obj+伙伴arg1, g |
| `stage.effect_handler` | 0x0803ff54 | battle_stage_effects | sub_803FF54 | 207 | cluster57 |  |
| `stage.effect_handler` | 0x080401ac | battle_stage_effects | sub_80401AC | 369 | cluster83 |  |
| `stage.effect_handler` | 0x08040690 | battle_stage_effects | sub_8040690 | 901 | cluster1 |  |
| `stage.effect_handler` | 0x08040ee8 | battle_stage_effects | sub_8040EE8 | 421 | cluster1 |  |
| `stage.effect_handler` | 0x08041308 | battle_stage_effects | sub_8041308 | 390 | cluster1 |  |
| `stage.effect_handler` | 0x080416f0 | battle_stage_effects | sub_80416F0 | 297 | cluster1 |  |
| `stage.effect_handler` | 0x080419e0 | battle_stage_effects | sub_80419E0 | 531 | cluster1 |  |
| `stage.effect_handler` | 0x08041edc | battle_stage_effects | sub_8041EDC | 322 | cluster76 |  |
| `stage.effect_handler` | 0x080422b8 | battle_stage_effects | sub_80422B8 | 496 | cluster97 |  |
| `stage.effect_handler` | 0x08042784 | battle_stage_effects | sub_8042784 | 311 | cluster1 |  |
| `stage.effect_handler` | 0x08042b90 | battle_stage_effects | sub_8042B90 | 278 | cluster1 |  |
| `stage.effect_handler` | 0x08042e70 | battle_stage_effects | sub_8042E70 | 750 | cluster1 |  |
| `stage.effect_handler` | 0x08043554 | battle_stage_effects | sub_8043554 | 383 | cluster1 |  |
| `stage.effect_handler` | 0x08043938 | battle_stage_effects | sub_8043938 | 199 | cluster1 |  |
| `stage.effect_handler` | 0x08043b5c | battle_stage_effects | sub_8043B5C | 412 | cluster1 |  |
| `stage.effect_handler` | 0x08043f90 | battle_stage_effects | sub_8043F90 | 397 | cluster1 |  |
| `menu.inventory_equip` | 0x0800c2f8 | menu_ui | sub_800C2F8 | 3239 | cluster120 |  |
| `menu.inventory_equip` | 0x0800e244 | menu_ui | sub_800E244 | 411 | cluster87 |  |
| `menu.inventory_equip` | 0x0800ec54 | menu_ui | sub_800EC54 | 522 | cluster100 |  |
| `menu.inventory_equip` | 0x0800f128 | menu_ui | sub_800F128 | 289 | cluster71 |  |
| `menu.inventory_equip` | 0x0800f70c | menu_ui | sub_800F70C | 277 | cluster70 |  |
| `menu.inventory_equip` | 0x08010170 | menu_ui | sub_8010170 | 101 | cluster23 | ⏸ 装备更换: 语义100%解(草稿核对+符号全对齐); permuter发现死赋值技巧 oldEquip=0;if(item!=oldEq |
| `menu.inventory_equip` | 0x08010300 | menu_ui | sub_8010300 | 128 | cluster32 | ⏸ 2026-09-12 nova 接管(claude-300 锁 7 天无更新): 掩码bl槽字节差 101→66(干净C)/42(非人类 |
| `menu.inventory_equip` | 0x080104f8 | menu_ui | sub_80104F8 | 121 | cluster31 | ⏸ 2026-09-12 gpnux-104f8b 二期: 语义/指令流 100% 还原 (最优候选 .scratch/gpnux-104f |
| `menu.inventory_equip` | 0x08011268 | menu | sub_8011268 | 158 | cluster5 | ⏸ 2026-09-02 opencode: 技能菜单物品页绘制=ClearBuffer(0x02005AA0,8,6)+(0x02005A |
| `menu.inventory_equip` | 0x08015c18 | menu | InvUi_Main | 207 | cluster5 | ⏸ 2026-09-08 zcode-invui: LIM提升已破(名字循环do-while化不奏效,真解=y2内联rowi*2+7使loo |
| `core.skill_effect` | 0x0804519c | battle_flow_rules | sub_804519C | 175 | cluster49 |  |
| `core.skill_effect` | 0x080466f0 | battle_flow_rules | sub_80466F0 | 651 | cluster106 |  |
| `core.skill_effect` | 0x080472e8 | battle_flow_rules | sub_80472E8 | 205 | cluster56 |  |
| `core.skill_effect` | 0x0804753c | battle_flow_rules | sub_804753C | 172 | cluster48 | ⏸ 2026-09-15 qwen-analyze-804753c: 语义全解(kind 0..4 选 BattleObj.statMods |
| `core.skill_effect` | 0x080476dc | battle_flow_rules | sub_80476DC | 458 | cluster95 |  |
| `core.skill_effect` | 0x08047dc8 | battle_flow_rules | sub_8047DC8 | 188 | cluster53 |  |
| `core.skill_effect` | 0x08048310 | battle_flow_rules | sub_8048310 | 158 | cluster43 | ⏸ 2026-09-11 zcode-engine: 语义全解(逃跑率: bufA[5]/bufB[7]两轮收sub_8045F10(poo |
| `core.skill_effect` | 0x08048458 | battle_flow_rules | sub_8048458 | 159 | cluster44 | ⏸ 2026-09-11 zcode-engine: 语义全解(逃跑率变体: obj[0xBE]<=10时 rate=(0x91/0x92= |
| `core.skill_effect` | 0x080485a4 | battle_flow_rules | sub_80485A4 | 96 | cluster19 | ⏸ 2026-09-10 zcode-main: 语义/结构全解(三分支判定: be<=0xA早退0; (u8)(be-0xC)<=0x64 |
| `menu.hud_text` | 0x0800b374 | menu_ui | sub_800B374 | 1172 | cluster115 |  |
| `menu.hud_text` | 0x0800c194 | menu_ui | sub_800C194 | 154 | cluster39 |  |
| `menu.hud_text` | 0x0800e8f8 | menu_ui | sub_800E8F8 | 234 | cluster61 | ⏸ 播放时间/倒计时五位数绘制(余数链除法: /21600000,2160000,216000,36000,3600各取u8; 每位=8×1 |
| `menu.hud_text` | 0x0800eae4 | menu_ui | sub_800EAE4 | 87 | cluster17 |  |
| `menu.hud_text` | 0x0800fb2c | menu_ui | sub_800FB2C | 310 | cluster74 |  |
| `menu.hud_text` | 0x08010624 | menu_ui | sub_8010624 | 143 | cluster37 |  |
| `menu.hud_text` | 0x08010f10 | menu | sub_8010F10 | 269 | cluster69 | ⏸ 4格框绘制(选择框: 框角0xB190/边框0xB001x20/标题tile 0x1AE-0x1BD attr<<12/图标或数字内容) |
| `menu.hud_text` | 0x08012530 | menu | sub_8012530 | 262 | cluster65 | ⏸ 角色图块装载(arg0*4查gUnk_0809888B表: kind/animId/flags); switch kind选tile表+ |
| `menu.window_list_state` | 0x080212b4 | battle_menu_windows | sub_80212B4 | 265 | cluster66 |  |
| `menu.window_list_state` | 0x0802192c | battle_menu_windows | sub_802192C | 1208 | cluster117 |  |
| `menu.window_list_state` | 0x080230bc | battle_menu_windows | sub_80230BC | 267 | cluster67 |  |
| `menu.window_list_state` | 0x08023320 | battle_menu_windows | sub_8023320 | 108 | cluster25 | ⏸ 2026-09-05 zcode-ll: 语义全解: 状态机(switch((s8)0x03000769){case1: 0x03000 |
| `menu.window_list_state` | 0x08023414 | battle_menu_windows | sub_8023414 | 455 | cluster94 |  |
| `menu.window_list_state` | 0x08023820 | battle_menu_windows | sub_8023820 | 1296 | cluster118 |  |
| `menu.window_list_state` | 0x080246e8 | battle_menu_windows | sub_80246E8 | 150 | cluster38 |  |
| `menu.window_list_state` | 0x08024940 | battle_menu_windows | sub_8024940 | 1202 | cluster116 |  |
| `stage.actor_handler` | 0x080257d8 | battle_menu_windows | sub_80257D8 | 178 | cluster2 |  |
| `stage.actor_handler` | 0x08025994 | battle_menu_windows | sub_8025994 | 444 | cluster92 |  |
| `stage.actor_handler` | 0x08025da8 | battle_menu_windows | sub_8025DA8 | 307 | cluster2 |  |
| `stage.actor_handler` | 0x080260bc | battle_menu_windows | sub_80260BC | 404 | cluster2 |  |
| `stage.actor_handler` | 0x08028ad8 | battle_stage_actor | sub_8028AD8 | 747 | cluster108 | ⏸ mode=ALLOC_WALL; 2026-09-13 gpnux: 语义/结构已完整建模 (全 18 态多角色编排状态机, 见 han |
| `stage.actor_handler` | 0x0802c0ec | battle_stage_actor | sub_802C0EC | 357 | cluster80 |  |
| `stage.actor_handler` | 0x0802de04 | battle_stage_actor | sub_802DE04 | 177 | cluster7 |  |
| `stage.actor_handler` | 0x0802eac4 | battle_stage_actor | sub_802EAC4 | 306 | cluster7 |  |
| `menu.title_save_ui` | 0x0800acc8 | menu_ui | sub_800ACC8 | 368 | cluster82 |  |
| `menu.title_save_ui` | 0x08012790 | menu | TitleMenu_UpdateUi | 1727 | cluster119 | ⏸ 2026-09-02 gpnux: 存档/场景选择 UI 主状态机；扫描 gSaveMapUnlockFlags 并以 SaveFlag |
| `menu.title_save_ui` | 0x08013c00 | menu | sub_8013C00 | 349 | cluster77 | ⏸ 存档菜单顶部绘制(标题行+2行选项列(高亮=i==cursorStack)+存档时间行+2x6填0xB001框+sub_800EAE4画 |
| `menu.title_save_ui` | 0x0801417c | menu | sub_801417C | 306 | cluster73 | ⏸ 存档菜单状态机(20+ case: 0x28存档UI初始化DISPCNT/BLDCNT/定时器+PartyUi_InitEntities |
| `menu.title_save_ui` | 0x080146a8 | menu | sub_80146A8 | 404 | cluster85 | ⏸ 菜单主状态机(switch gMenuCursorGrp: case1→Msg_ShowById(sel+0xE/sel==2?0x13 |
| `menu.title_save_ui` | 0x08014a68 | menu | sub_8014A68 | 1034 | cluster113 |  |
| `menu.title_save_ui` | 0x08015658 | menu | sub_8015658 | 476 | cluster96 |  |
| `battle_object.effect_queue` | 0x0801e1d8 | battle_object_engine | sub_801E1D8 | 135 | cluster35 | ⏸ 2026-09-10 main801E1D8: v4 候选已过 permuter/score=5175 且 fndiff 前 34B 逐 |
| `battle_object.effect_queue` | 0x0801e30c | battle_object_engine | sub_801E30C | 214 | cluster58 |  |
| `battle_object.effect_queue` | 0x0801e848 | battle_object_engine | sub_801E848 | 243 | cluster62 | ⏸ 2026-09-13 gpnux(接管): 语义 100% 还原(战斗效果队列 030006F8 的 3 态状态机 0→1→2→0: c |
| `battle_object.effect_queue` | 0x0801eee4 | battle_object_engine | sub_801EEE4 | 545 | cluster10 |  |
| `battle_object.effect_queue` | 0x0801f3fc | battle_object_engine | sub_801F3FC | 357 | cluster10 |  |
| `battle_object.effect_queue` | 0x0801fab8 | battle_object_engine | sub_801FAB8 | 428 | cluster88 |  |
| `core.result_dialog` | 0x08048fb8 | battle_flow_rules | sub_8048FB8 | 268 | cluster68 |  |
| `core.result_dialog` | 0x080492c0 | battle_flow_rules | sub_80492C0 | 232 | cluster59 |  |
| `core.result_dialog` | 0x080494f0 | battle_flow_rules | sub_80494F0 | 256 | cluster64 |  |
| `core.result_dialog` | 0x08049b70 | battle_flow_rules | sub_8049B70 | 77 | cluster14 | ⏸ 2026-09-06 glm-batch: 语义全解(tile DMA入口查找: gUnk_0300096C=u16*ctx指针, ti |
| `core.result_dialog` | 0x08049df8 | battle_flow_rules | sub_8049DF8 | 352 | cluster79 |  |
| `core.result_dialog` | 0x0804ab40 | battle_flow_rules | sub_804AB40 | 60 | cluster13 | ⏸ 2026-09-14 gpnux-ab40 接管(原锁 claude1 9-03 起停滞)。do-while+守卫+链式94B=(94C |
| `core.target_filter` | 0x08022710 | battle_menu_windows | sub_8022710 | 1001 | cluster112 |  |
| `core.target_filter` | 0x08046558 | battle_flow_rules | sub_8046558 | 137 | cluster36 | ⏸ 2026-09-11 zcode-engine: 语义全解(8046480姊妹: big=arg0[0xBE]>10; 5/7两组buf |
| `core.target_filter` | 0x08046cd4 | battle_flow_rules | sub_8046CD4 | 160 | cluster45 | ⏸ 2026-09-11 zcode-engine 二轮攻坚: 语义全解(arg0[0xBE]<=10: buf非空清7字节+local[7 |
| `core.target_filter` | 0x080471ac | battle_flow_rules | sub_80471AC | 154 | cluster40 | ⏸ 2026-09-11 zcode-engine: 语义全解(8048310同族: bufA[5]/bufB[7]收0x1FF==2槽号, |
| `core.target_filter` | 0x080480ec | battle_flow_rules | sub_80480EC | 99 | cluster21 | ⏸ 2026-09-11 zcode: 语义全解(收集sub_8045F10(slot,0x1FF)==2的活动槽入buf[5], 找首个f |
| `core.target_filter` | 0x0804cee0 | battle_palette_wipe | sub_804CEE0 | 254 | cluster63 |  |
| `battle_object.spawn_animation` | 0x0801b964 | battle_object_engine | sub_801B964 | 545 | cluster103 |  |
| `battle_object.spawn_animation` | 0x0801be34 | battle_object_engine | sub_801BE34 | 621 | cluster11 |  |
| `battle_object.spawn_animation` | 0x0801c484 | battle_object_engine | sub_801C484 | 555 | cluster11 |  |
| `battle_object.spawn_animation` | 0x0802031c | battle_object_engine | sub_802031C | 364 | cluster81 |  |
| `battle_object.spawn_animation` | 0x08020648 | battle_object_engine | sub_8020648 | 158 | cluster42 |  |
| `battle_object.head_command_loader` | 0x0801a884 | battle_object_engine | sub_801A884 | 530 | cluster101 |  |
| `battle_object.head_command_loader` | 0x0801ad0c | battle_object_engine | sub_801AD0C | 452 | cluster93 |  |
| `battle_object.head_command_loader` | 0x0801b0b8 | battle_object_engine | sub_801B0B8 | 537 | cluster102 | ⏸ 2026-09-11 claude-b0b8: 语义全解 (帧构建+DMA装载 5 路switch; 参数 ObjHead*, kind |
| `battle_object.head_command_loader` | 0x0801cba4 | battle_object_engine | sub_801CBA4 | 320 | cluster75 | ⏸ r5/r6互换(obj=r6/anim=r5 vs mine r5/r6, 89r×5≈445pts)+24处结构差异(case0 ab |
| `battle_object.ui_drawing` | 0x08018ea8 | battle_task_services | sub_8018EA8 | 132 | cluster34 | ⏸ 2026-09-05 zcode-ll 接管(opencode→zcode-ll): 语义全解: 3位数图块显示(u16 arg0值,  |
| `battle_object.ui_drawing` | 0x08018fc0 | battle_task_services | sub_8018FC0 | 186 | cluster51 |  |
| `battle_object.ui_drawing` | 0x080191cc | battle_task_services | sub_80191CC | 155 | cluster41 | ⏸ 2026-09-05 zcode-ll: 语义全解: 32宽u16图块矩形边框绘制(u16* dest, u16 pal基址, u8 x |
| `battle_object.ui_drawing` | 0x0801933c | battle_task_services | sub_801933C | 437 | cluster91 |  |
| `map.actor_move` | 0x08003958 | sprite_engine | Chara_SetWalkPath | 203 | cluster55 |  |
| `map.actor_move` | 0x08003c54 | sprite_engine | Chara_StepMove | 352 | cluster78 | semantic audit 2026-09-15: pure inline step/collision; returns 0/1/2/3 |
| `map.actor_move` | 0x080040e4 | sprite_engine | Party_FollowAnim | 290 | cluster72 |  |
| `map.actor_move` | 0x080055e8 | map_view | sub_80055E8 | 626 | cluster104 | ⏸ 进行中(nova2): 语义已全解(MapZone子系统+贴墙滑动); fndiff最优10865(v1000); 剩38条差异: 对角 |
| `map.viewport_camera` | 0x08005020 | map_view | sub_8005020 | 178 | cluster50 |  |
| `map.viewport_camera` | 0x080052f8 | map_view | sub_80052F8 | 87 | cluster16 | ⏸ 2026-09-13 opencode_1 接管: 语义/结构 100% 还原 (四层循环 2x2 分块贴图); permuter 搜索 |
| `map.viewport_camera` | 0x080053b4 | map_view | sub_80053B4 | 233 | cluster60 | ⏸ 语义/结构已全解(见 progress.md sub_80053B4); 候选差59字节: dst偏移=row*64+col*2, ca |
| `map.viewport_camera` | 0x08005c70 | map_view | Viewport_UpdateScroll | 755 | cluster109 |  |
| `menu.dialog_tile_render` | 0x0802181c | battle_menu_windows | sub_802181C | 128 | cluster33 |  |
| `menu.dialog_tile_render` | 0x08022f2c | battle_menu_windows | sub_8022F2C | 191 | cluster54 |  |
| `menu.dialog_tile_render` | 0x080244bc | battle_menu_windows | sub_80244BC | 166 | cluster4 |  |
| `menu.dialog_tile_render` | 0x08024618 | battle_menu_windows | sub_8024618 | 95 | cluster18 | ⏸ 2026-09-10 zcode-8024618: permuter 分 520→25→10 (字节 208 中仅剩 2 差, fndi |
| `palette.fade` | 0x08009600 | player_stats | PaletteFx_Transform | 509 | cluster99 | ⏸ 2026-09-10 zcode: 接管claude死锁; permuter 2750→830; 形状全对仅剩 amount/count |
| `palette.fade` | 0x0804b56c | battle_palette_wipe | sub_804B56C | 117 | cluster28 |  |
| `palette.fade` | 0x0804bbdc | battle_palette_wipe | sub_804BBDC | 181 | cluster6 |  |
| `palette.fade` | 0x0804bf14 | battle_palette_wipe | sub_804BF14 | 242 | cluster6 |  |
| `core.damage_calc` | 0x08044a40 | battle_flow_rules | sub_8044A40 | 499 | cluster98 |  |
| `core.damage_calc` | 0x08045098 | battle_flow_rules | sub_8045098 | 99 | cluster20 | ⏸ 2026-09-14 zcode: 战斗伤害浮动计算 (fxKind==2 时由 sub_804473C 调用)。25项跳转表+Rng浮 |
| `core.damage_calc` | 0x08048690 | battle_flow_rules | sub_8048690 | 99 | cluster22 | ⏸ 2026-09-11 zcode-engine: 语义全解(命中判定: entry=gUnk_083988A8[arg0[0xBE]]  |
| `core.turn_sort` | 0x08046e18 | battle_flow_rules | sub_8046E18 | 117 | cluster27 | ⏸ 2026-09-15 antigravity: 语义与控制流100%全解(Hoare partition快速排序分区函数: 首参为u8* |
| `core.turn_sort` | 0x080481b8 | battle_flow_rules | sub_80481B8 | 164 | cluster46 |  |
| `core.turn_sort` | 0x0804a368 | battle_flow_rules | sub_804A368 | 686 | cluster107 |  |
| `map.scene_load` | 0x0800661c | map_view | MapScene_Load | 1044 | cluster114 | ⏸ 3024B asm; 场景描述符表已结构化为 gMapSceneDescriptors[180] @0x08088D80; fnchec |
| `map.scene_load` | 0x080071ec | map_view | MapScene_LoadNpcSlotIds | 80 | cluster15 | ⏸ 176B asm; 按场景 npcSlotGroupId 装载 8 个 NPC 槽的图形/调色板 ID; fncheck OK (202 |
| `map.scene_load` | 0x08007350 | map_view | MapScene_LoadEventAnimations | 649 | cluster105 | ⏸ 2026-09-02 codex: 按场景动画组加载并依据地图事件旗标播放一次性动画; 语义已确认, 保留 asm |
| `script.dialog_text` | 0x08050720 | script_vm | sub_8050720 | 1071 | cluster3 |  |
| `script.dialog_text` | 0x0805144c | script_vm | sub_805144C | 605 | cluster3 | ⏸ 2026-09-14 gpnux: 语义100%解 (13态状态机: 态0初始化/BG0CNT/双清屏→态5 token高字节分派 (0 |
| `script.dialog_text` | 0x08051be4 | script_vm | sub_8051BE4 | 996 | cluster3 |  |
| `script.tile_dma` | 0x080501b8 | script_vm | sub_80501B8 | 299 | cluster9 | ⏸ 候选715分: 语义全解(队列插入+DMA3字库拷贝, 0x80000010=DmaCopy16 0x20B); 卡点=头部0xF00掩 |
| `script.tile_dma` | 0x08050434 | script_vm | sub_8050434 | 246 | cluster9 | ⏸ 语义全解+候选已到 456B/488B 差(232B 一致); permuter 8808→3275 但所有最优变体把 0x6E3-0x |
| `script.tile_dma` | 0x0805063c | script_vm | sub_805063C | 104 | cluster24 | ⏸ 2026-09-10 zcode-805063c: 人工模式续攻(h系列矩阵+根因取证); 确认 exp1g 131B/224B = 可 |
| `item.drops` | 0x0804e9dc | battle_itemuse_rewards | BattleDrops_Roll | 239 | cluster8 | ⏸ 2026-09-10 claude_1: 结构已全对齐(双 GetObjPool 变量/5B×101 掉落表 0x0839D9B8+0x |
| `item.drops` | 0x0804ec04 | battle_itemuse_rewards | sub_804EC04 | 305 | cluster8 |  |
| `sio.link` | 0x08016d24 | text_engine | sub_8016D24 | 118 | cluster29 | ⏸ 语义全解 + 结构 100% 对齐 (bytecmp 差 63B/280B, 全为 home/调度)。SIO 联机主循环: siocnt |
| `sio.link` | 0x080171e4 | battle_task_services | sub_80171E4 | 434 | cluster90 |  |
| `stage.service_dispatcher` | 0x0803f444 | battle_stage_dialogue | sub_803F444 | 169 | cluster47 | ⏸ 2026-09-13 gpt5: 战斗对象演出/技能分派器 (args: r0=BattleObj*, r1=同池对象; ret=u8) |
| `stage.service_dispatcher` | 0x0803f658 | battle_stage_dialogue | sub_803F658 | 963 | cluster111 | ⏸ mode=SEMANTIC_ANALYSIS; handoff=ENGINE-HUB-20260912-a; addr=0x0803f6 |
| `battle_object.anim_dma` | 0x08022458 | battle_menu_windows | sub_8022458 | 111 | cluster26 | ⏸ 2026-09-05 zcode-ll: 语义全解: 动画DMA序列驱动(u8 arg0→返回递减值): 若(s8)gUnk_03000 |
| `battle_object.loop` | 0x080177ac | battle_task_services | BattleTask_Run | 757 | cluster110 | ⏸ 2026-09-11 zcode-main: 战斗主循环状态机 21-case 全解, 结构 100% 对 (助记符 97.4%); 剩 |
| `item.use_fx` | 0x0804e2ac | battle_itemuse_rewards | ItemUseFx_RunWeapon | 430 | cluster89 | 改名 2026-09-14 zcode (原 sub_804E2AC): 物件使用演出·武器攻击道具分支 (ids 0x19..0x31=ハ |
| `menu.choice_input` | 0x08008254 | option_scene_loader | ChoiceMenu_HandleInput | 404 | cluster84 | ⏸ 2026-09-11 zcode-engine: 语义全解+完整C候选已写(见 docs/progress.md 2026-09-01  |
| `palette.wipe` | 0x0804af60 | battle_palette_wipe | sub_804AF60 | 118 | cluster30 | ⏸ 2026-09-14 claude804AF60: 前任940→现20分. 关键突破: ①OAM写用位域形态 oam[i].fields |
| `script.chara_control` | 0x0804f280 | script_vm | sub_804F280 | 404 | cluster86 |  |
| `stage.service_helper` | 0x0803f21c | battle_stage_dialogue | sub_803F21C | 130 | cluster4 |  |
