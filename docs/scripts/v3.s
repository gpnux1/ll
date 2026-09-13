; ============================================================================
; Lunar Legend - ScriptSet 001  (v3 authoring 案例)
;
; 数据源: docs/scripts/script_set_001.txt (反汇编)
; 语法依据: docs/game/GAME_SCRIPT_AUTHORING_V3.md
; 说明:
;   - 本文件用 v3 语法重述 ROM 反汇编, 命令名从 src/script_vm.c 里
;     每个 opcode handler 的源码语义独立推导, 不继承 v1/v2 命名。
;   - 所有命令统一用 name(arg0, arg1, ...) 函数式调用。
;   - 参数宽度严格按 handler 源码反推 (见 V3.md §4 注册表)。
;   - 256 项入口表: Entry_00/01 有实际内容, Entry_02..0xF9 全部指向
;     0x13EF (stop 1 fallback), Entry_FA..0xFF 指向 0x13F1 (stop 1)。
; ============================================================================

mode = recover
baseline = "data/raw_data/unk_862E2A0.bin"

; ---- 本文件别名 ----
alias map_title   = 0x82    ; mapNpcSet=130 (Title 阶段)
alias map_birth   = 0x83    ; mapNpcSet=131 (主角出生剧情)
alias map_burg    = 0x0B    ; mapNpcSet=11  (主城)
alias move_00     = 0x00
alias dir_right   = 0x06

; ============================================================================
; Entry_00  @ offset 0x0200  (Title / Opening intro)
;
; ROM 反汇编:
;   [0x0200] Op_ClearSwitchTail
;   [0x0201] Op_LoadMap mapNpcSet=130, moveCmdSet=0, spawn=(0,0), dir=0
;   [0x0208] Op_OpenWindow
;   [0x0209] Op_DialogSetup 01 00 00 1f 1f 00
;   [0x0210] Op_SceneChangePlain mode=0
;   [0x0212] Op_WaitSceneIdle
;   [0x0213] Op_BgmPlay song=65339, arg=0
;   [0x0217] Op_DialogText x=6 y=7 ...
; ============================================================================

entry 0x00 title_intro:
    switch.clear_all()                ; 0x48 1B
    map.load(0x82, 0x00, 0x00, 0x00, 0x00)   ; 0x1B 7B  npc,move,x,y,dir
    window.open()                     ; 0x24 1B
    dialog.setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)  ; 0x23 7B
    scene.plain(0)                    ; 0x18 2B
    wait.scene()                      ; 0x19 1B
    bgm.start(0xFB, 0x0000)           ; 0x07 4B  track,loop(u16)
    ; [0x0217] 以下是对白序列, 用 text 引用
    dialog.text(0x06, 0x07, 0x35, opening_line_00)  ; 0x17 4+tokens
    dialog.setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    dialog.text(0x07, 0x08, 0x23, opening_line_01)
    dialog.setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    dialog.text(0x07, 0x08, 0x25, opening_line_02)
    dialog.setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    dialog.text(0x07, 0x08, 0x24, opening_line_03)
    dialog.setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    dialog.text(0x09, 0x09, 0x0F, opening_line_04)
    dialog.setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    ; [0x03B8] 切到主城场景, 装载角色过场动画
    map.load(0x0B, 0x00, 0x0F, 0x2E, 0x06)   ; npc=0x0B, spawn=(0x0F,0x2E), dir=6
    cutscene.load(0x0000, 0x00, 0x00)  ; 0x2C 5B  u16 anim, u8 x, u8 y
    cutscene.load(0x0004, 0x01, 0x01)
    cutscene.load(0x0018, 0x04, 0x00)
    cutscene.load(0x0029, 0x05, 0x00)
    cutscene.load(0x002A, 0x06, 0x00)
    cutscene.load(0x0003, 0x07, 0x04)
    cutscene.load(0x000D, 0x08, 0x00)
    cutscene.load(0x000E, 0x09, 0x00)
    cutscene.load(0x0000, 0x0B, 0x64)
    cutscene.load(0x0004, 0x0C, 0x65)
    ; ... (后续 cutscene.load 与角色 exec 序列省略)
    wait.chars()                      ; 0x10 1B
    ; [0x0DA0] 保存标志, 结束 title 阶段
    save.mark(0x06)                   ; 0x42 2B
    stream.return()                   ; 0x16 1B  (回到 ScriptSet 000)

; ============================================================================
; Entry_01  @ offset 0x0CE1  (New Game flow - 主角出生剧情)
;
; ROM 反汇编 (摘取):
;   [0x0CE1] Op_SceneChangeFade mode=2
;   [0x0CE3] Op_WaitSceneIdle
;   [0x0CE4] Op_LoadMap mapNpcSet=131, moveCmdSet=0, spawn=(0,0), dir=0
;   [0x0CEB] Op_BgmPlay song=65343, arg=0
;   [0x0CEF] Op_SceneChangePlain mode=0
;   [0x0CF1] Op_WaitSceneIdle
;   [0x0CF2] Op_OpenWindow
;   ... (17 段 Op_DialogText 主角出生故事)
;   [0x0F0F] Op_SysEffect subOp=0xC8, arg=0x03
;   [0x0F12] Op_WaitFrames count=30
;   ... (更多 dialog + sys.effect 交替)
;   [0x13E4] Op_CloseWindow
;   [0x13E5] Op_SceneChangeFade mode=0
;   [0x13E7] Op_BgmFadeOut speed=30
;   [0x13E9] Op_WaitFrames count=30
;   [0x13EB] Op_WaitSceneIdle
;   [0x13EC] Op_BgmStop
;   [0x13ED] Op_ScriptStop mode=1
; ============================================================================

entry 0x01 new_game:
    scene.fade(2)                     ; 0x14 2B  mode=2 (fade out)
    wait.scene()                      ; 0x19 1B
    map.load(0x83, 0x00, 0x00, 0x00, 0x00)   ; npc=0x83
    bgm.start(0xFB, 0x0011)           ; 0x07 4B  (song 65343 = 0x00FB loop 0x0011)
    scene.plain(0)                    ; 0x18 2B
    wait.scene()                      ; 0x19 1B
    window.open()                     ; 0x24 1B
    ; --- 主角出生故事 (17 段对白) ---
    dialog.text(0x01, 0x10, 0x45, birth_story_00)   ; x=1,y=16,len=69
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x0D, birth_story_01)   ; len=13
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x30, birth_story_02)   ; len=48
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x1E, birth_story_03)   ; len=30
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x44, birth_story_04)   ; len=68
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x03, birth_story_05)   ; len=3 (空格+wait)
    dialog.setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    sys.effect(0xC8, 0x03)            ; 0x4D 3B  MapBg_LoadFull(0x84)
    wait.frames(30)                   ; 0x26 2B
    dialog.setup(0x00, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    dialog.text(0x01, 0x10, 0x0D, birth_story_06)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x36, birth_story_07)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x3C, birth_story_08)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x3F, birth_story_09)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x03, birth_story_10)
    dialog.setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    sys.effect(0xC8, 0x04)            ; MapBg_LoadFull(0x85)
    wait.frames(30)
    dialog.setup(0x00, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    dialog.text(0x01, 0x10, 0x22, birth_story_11)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x3A, birth_story_12)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x3B, birth_story_13)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x33, birth_story_14)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x28, birth_story_15)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x03, birth_story_16)
    dialog.setup(0x01, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    sys.effect(0xC8, 0x05)            ; MapBg_LoadFull(0x86)
    wait.frames(30)
    dialog.setup(0x00, 0x00, 0x00, 0x1F, 0x1F, 0x00)
    dialog.text(0x01, 0x10, 0x35, birth_story_17)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x13, birth_story_18)
    dialog.setup(0x01, 0x00, 0x10, 0x1F, 0x04, 0x00)
    dialog.text(0x01, 0x10, 0x03, birth_story_19)
    ; --- 结束 sequence ---
    window.close()                    ; 0x25 1B
    scene.fade(0)                     ; 0x14 2B  mode=0 (fade in)
    bgm.fade_out(30)                  ; 0x0B 2B
    wait.frames(30)                   ; 0x26 2B
    wait.scene()                      ; 0x19 1B
    bgm.stop()                        ; 0x08 1B
    stop(1)                           ; 0x06 2B

; ============================================================================
; Entry_02..0xF9  @ offset 0x13EF  (fallback: 单行 stop)
;
; 反汇编显示: 这 238 个 entry 都指向同一偏移 0x13EF,
; 那里是一条 Op_ScriptStop mode=1。这是"未使用的入口"的占位。
; ============================================================================

entry 0x02 fallback_stop:
    stop(1)                           ; 0x06 2B

; Entry_03..0xF9 全部 alias 到 fallback_stop (ROM 中共享偏移)
alias_entry 0x03 = fallback_stop
alias_entry 0x04 = fallback_stop
; ... (省略 0x05..0xF9, 共 237 条, 全部 alias 到 fallback_stop)

; ============================================================================
; Entry_FA..0xFF  @ offset 0x13F1  (另一段 stop fallback)
;
; 反汇编显示: 0xFA..0xFF 指向 0x13F1, 也是一条 stop 1。
; 与 0x02..0xF9 只是偏移相差 2 字节, 效果相同。
; ============================================================================

entry 0xFA tail_stop:
    stop(1)                           ; 0x06 2B

alias_entry 0xFB = tail_stop
alias_entry 0xFC = tail_stop
alias_entry 0xFD = tail_stop
alias_entry 0xFE = tail_stop
alias_entry 0xFF = tail_stop

; ============================================================================
; 对白 token 定义 (部分示例)
;
; 实际文本内容从反汇编 [0x0217] 等行的 text="..." 字段提取。
; 这里展示 v3 的 text 块语法。
; ============================================================================

text opening_line_00:
    face(0x00)                        ; 无前导 face (ROM 无 face token)
    0x125 0x0A08                      ; いつの[0A:08]からだろう
    newline()
    0x0E1 0x0A08 0x0E2 0x0A08 0x0E3 0x0A08  ; 『[0E1][0E2]の[0E3]』という
    newline()
    0x0E6 0x0A08 0x0AFA               ; とらえて ... の[WAIT]
    clear()

text birth_story_00:
    0x125 0x0A08 0x126 0x0A08 0x0A08 0x127 0x0A08
    newline()
    0x12A 0x0A08 0x12B 0x0A08 0x0A08
    clear()

text birth_story_05:
    0x0A14                            ; 空格 + wait token
    clear()

; ============================================================================
; raw 段示例 (未解析的字节保留原样)
;
; 反汇编中 sub_804F280 (chara.exec) 的子命令尚未完整反编译,
; 角色控制序列保留为 raw。
; ============================================================================

; 例: [0x03B1] 之后若出现未解析的角色控制指令:
raw(0x04, "4c 03 3e 5a 02")
raw(0x04, "4c 04 41 59 00")

; 例: 整段 legacy entry (未解析)
legacy(0x30, "raw/script_001_entry_30.bin")

; ============================================================================
; 文件结束
; ============================================================================
