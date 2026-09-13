#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Lunar Legend (Japan) - 脚本集反汇编与解码工具
用于对 ROM 中 LZ77 压缩的脚本数据包进行解压并反汇编为人类可读的结构化脚本代码流。
"""

import sys
import os
import struct

# ---------------------------------------------------------------------------
# 1. 字符与汉字解码映射
# ---------------------------------------------------------------------------

CHARMAP = {}
CHARMAP_FILE = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), 'charmap.txt')

if os.path.exists(CHARMAP_FILE):
    with open(CHARMAP_FILE, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            if '=' in line:
                parts = line.split('=')
                c = parts[0].strip().strip("'")
                v = int(parts[1].strip(), 16)
                CHARMAP[v] = c

# 常用汉字字模映射表 (由游戏内剧情上下文与字模序号严格对齐)
KANJI_MAP = {
    0x000D: '声',
    0x0012: '家',
    0x00E4: '迎',
    0x00EA: '様',
    0x00EF: '長',
    0x00F2: '雲',
    0x00F3: '上',
    0x00F5: '見',
    0x00F7: '行',
    0x00FB: '目',
    0x0104: '好',
    0x0106: '知',
    0x0114: '魔',
    0x0115: '法',
    0x0116: '使',
    0x0117: '来',
    0x0118: '修',
    0x011A: '聞',
    0x011E: '思',
    0x011F: '村',
    0x0123: '元',
    0x012C: '光',
    0x013A: '生',
    0x013B: '気',
    0x014A: '平',
    0x0151: '雄',
    0x0159: '違',
    0x015D: '雄',
    0x015E: '談',
    0x0162: '今',
    0x0164: '話',
    0x0169: '方',
    0x016E: '儀',
    0x016F: '式',
    0x0170: '準',
    0x0171: '備',
    0x0172: '終',
    0x0173: '英',
    0x0174: '墓',
    0x0175: '参',
    0x0176: '行',
    0x0177: '目',
    0x0178: '的',
    0x0179: '間',
    0x017B: '敷',
    0x017C: '感',
    0x017D: '謝',
    0x017E: '肉',
    0x017F: '四',
    0x0182: '栄',
    0x0183: '雲',
    0x0184: '上',
    0x0185: '気',
    0x0186: '意',
    0x0187: '奴',
    0x018B: '待',
    0x018C: '助',
    0x018D: '勇',
    0x018E: '礼',
    0x018F: '泉',
    0x0190: '丘',
    0x0191: '疲',
    0x0192: '情',
    0x0193: '中',
    0x0195: '意',
    0x0206: '考',
    0x0208: '屋',
    0x023E: '臭',
}

def decode_token(t):
    """解码单个 16 位文本 Token"""
    hi = t >> 8
    lo = t & 0xFF
    if hi == 0:
        if lo in CHARMAP:
            return CHARMAP[lo]
        if lo == 0:
            return ''
        return f'<{lo:02X}>'
    elif hi == 0x09:
        return '\n'
    elif hi == 0x07:
        return '[WAIT]'
    elif hi == 0x0F:
        return '[CLEAR]'
    elif hi == 0x0D:
        return f'[PORTRAIT:{lo}]'
    elif hi == 0x01:
        return f'[CMD1:{lo}]'
    elif hi == 0x02:
        return f'[CMD2:{lo}]'
    elif (t & 0xF000) == 0x1000:
        code = t & 0x0FFF
        return KANJI_MAP.get(code, f'[{code:03X}]')
    return f'[{hi:02X}:{lo:02X}]'

def decode_text(tokens):
    """将 Token 序列解码为完整字符串"""
    return ''.join(decode_token(t) for t in tokens)

# ---------------------------------------------------------------------------
# 2. LZ77 解压缩核心算法 (严格等价于 src/engine_core.c: LZ_UncompressChunk)
# ---------------------------------------------------------------------------

def lz_uncompress(raw_bytes):
    """
    解压 GBA 专用的 LzHeader 数据块。
    LzHeader:
      uncompressedSize (4 字节)
      size (4 字节，数据负载长度)
      data[size] (字面字节流与 LZ Token 流)
      flags[...] (位标志流，紧跟在 data 之后)
    """
    if len(raw_bytes) < 8:
        raise ValueError("数据长度不足 8 字节")
    uncomp_size = struct.unpack('<I', raw_bytes[:4])[0]
    comp_size = struct.unpack('<I', raw_bytes[4:8])[0]
    src = 8
    flags = 8 + comp_size
    bit_index = 0
    dest = bytearray()

    while len(dest) < uncomp_size:
        flag_byte = raw_bytes[flags + (bit_index >> 3)]
        bit_offset = bit_index & 7
        is_token = ((flag_byte >> bit_offset) & 1) != 0

        if is_token:
            token = struct.unpack('<H', raw_bytes[src:src+2])[0]
            src += 2
            match_offset = (token & 0x0FFF) + 1
            match_length = (token >> 12) + 3
            for _ in range(match_length):
                if len(dest) >= uncomp_size:
                    break
                dest.append(dest[-match_offset])
        else:
            dest.append(raw_bytes[src])
            src += 1

        bit_index += 1

    return bytes(dest)

# ---------------------------------------------------------------------------
# 3. 脚本反汇编引擎 (80 个 Opcode 全覆盖)
# ---------------------------------------------------------------------------

def parse_instruction(data, pc, entries):
    """
    解析位于 pc 处的单条虚拟机指令。
    返回 (next_pc, mnemonic_string, branch_targets)
    """
    if pc >= len(data):
        return pc, "EOF", []
    
    op = data[pc]

    if op == 0x00:
        l = struct.unpack('<H', data[pc+2:pc+4])[0]
        sty = struct.unpack('<H', data[pc+4:pc+6])[0]
        toks = struct.unpack(f'<{l}H', data[pc+6:pc+6+l*2])
        text = decode_text(toks)
        # 将多行换行文本整齐输出
        text_disp = text.replace('\n', '\\n\n' + ' ' * 16)
        return pc + 6 + l * 2, f'DialogMessage style={sty}, len={l}:\n{" " * 16}"{text_disp}"', []

    elif op == 0x01:
        e = data[pc+1]
        target = 0x200 + entries[e]
        return pc + 2, f'Op_ScriptJump Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x02:
        e = data[pc+1]
        target = 0x200 + entries[e]
        return pc + 2, f'Script_Call Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x03:
        return pc + 1, 'Op_ScriptReturn', []

    elif op == 0x04:
        l = data[pc+1]
        subdata = data[pc+2:pc+2+l]
        if len(subdata) >= 5 and subdata[0] == 0x4C:
            chara = subdata[1]
            x, y, d = subdata[2], subdata[3], subdata[4]
            return pc + 2 + l, f'CharaControl SetPosDir chara={chara}, pos=({x}, {y}), dir={d}', []
        return pc + 2 + l, f'CharaControl len={l}: {subdata.hex(" ")}', []

    elif op == 0x05:
        return pc + 1, 'Op_Nop', []

    elif op == 0x06:
        mode = data[pc+1]
        return pc + 2, f'Op_ScriptStop mode={mode}', []

    elif op == 0x07:
        return pc + 1, 'Op_WaitCharsStop', []

    elif op == 0x08:
        return pc + 4, f'Op_LoadCharaGfx chara={data[pc+1]}, gfx1={data[pc+2]}, gfx2={data[pc+3]}', []

    elif op == 0x09:
        return pc + 4, f'Op_LoadCharaPal chara={data[pc+1]}, pal1={data[pc+2]}, pal2={data[pc+3]}', []

    elif op == 0x0A:
        return pc + 1, 'Op_WaitSpriteLoad', []

    elif op == 0x0B:
        return pc + 2, f'Op_SceneChangeFade mode={data[pc+1]}', []

    elif op == 0x0C:
        return pc + 2, f'Op_SceneChangePlain mode={data[pc+1]}', []

    elif op == 0x0D:
        return pc + 1, 'Op_WaitSceneIdle', []

    elif op == 0x0E:
        m = data[pc+1]
        mov = struct.unpack('<H', data[pc+2:pc+4])[0]
        x, y, d = data[pc+4], data[pc+5], data[pc+6]
        return pc + 7, f'Op_LoadMap mapNpcSet={m}, moveCmdSet={mov}, spawn=({x}, {y}), dir={d}', []

    elif op == 0x0F:
        flg = struct.unpack('<H', data[pc+1:pc+3])[0]
        e = data[pc+3]
        target = 0x200 + entries[e]
        return pc + 4, f'Op_IfEventFlagJump flag=0x{flg:04X} -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x10:
        flg = struct.unpack('<H', data[pc+1:pc+3])[0]
        return pc + 3, f'Op_SetEventFlag flag=0x{flg:04X}', []

    elif op == 0x11:
        flg = struct.unpack('<H', data[pc+1:pc+3])[0]
        return pc + 3, f'Op_ClearEventFlag flag=0x{flg:04X}', []

    elif op == 0x12:
        sw = struct.unpack('<H', data[pc+1:pc+3])[0]
        e = data[pc+3]
        target = 0x200 + entries[e]
        return pc + 4, f'Op_IfSwitchJump switch=0x{sw:04X} -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x13:
        sw = struct.unpack('<H', data[pc+1:pc+3])[0]
        return pc + 3, f'Op_SetSwitch switch=0x{sw:04X}', []

    elif op == 0x14:
        sw = struct.unpack('<H', data[pc+1:pc+3])[0]
        return pc + 3, f'Op_ClearSwitch switch=0x{sw:04X}', []

    elif op == 0x15:
        return pc + 3, f'Op_ScriptStreamLZ setId={data[pc+1]}, entry={data[pc+2]}', []

    elif op == 0x16:
        return pc + 1, 'Op_ScriptReturnChunk', []

    elif op == 0x17:
        l = struct.unpack('<H', data[pc+4:pc+6])[0]
        toks = struct.unpack(f'<{l}H', data[pc+6:pc+6+l*2])
        text = decode_text(toks)
        return pc + 6 + l * 2, f'Op_DialogText x={data[pc+2]}, y={data[pc+3]}, len={l}, text="{text}"', []

    elif op == 0x18:
        return pc + 1, 'Op_CameraSnap', []

    elif op == 0x19:
        return pc + 1, 'Op_CameraFollow', []

    elif op == 0x1A:
        dur = data[pc+1]
        x, y = struct.unpack('<HH', data[pc+2:pc+6])
        return pc + 6, f'Op_CameraPan duration={dur}, target=({x}, {y})', []

    elif op == 0x1B:
        return pc + 1, 'Op_WaitCameraPan', []

    elif op == 0x1C:
        return pc + 2, f'Op_RemovePartyMember memberId={data[pc+1]}', []

    elif op == 0x1D:
        return pc + 2, f'Op_AddPartyMember memberId={data[pc+1]}', []

    elif op == 0x1E:
        return pc + 5, f'Op_LoadCutsceneAnim {data[pc+1:pc+5].hex(" ")}', []

    elif op == 0x1F:
        return pc + 3, f'Op_RestartCharaAnim chara={data[pc+1]}, anim={data[pc+2]}', []

    elif op == 0x20:
        return pc + 2, f'Op_WaitCharaAnim chara={data[pc+1]}', []

    elif op == 0x21:
        e = data[pc+3]
        target = 0x200 + entries[e]
        return pc + 4, f'Op_IfPartyMemberJump member={data[pc+1]}, count={data[pc+2]} -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x22:
        e = data[pc+3]
        target = 0x200 + entries[e]
        return pc + 4, f'Op_ScriptBattle b1={data[pc+1]}, b2={data[pc+2]} -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x23:
        return pc + 7, f'Op_DialogSetup {data[pc+1:pc+7].hex(" ")}', []

    elif op == 0x24:
        return pc + 1, 'Op_OpenWindow', []

    elif op == 0x25:
        return pc + 1, 'Op_CloseWindow', []

    elif op == 0x26:
        return pc + 2, f'Op_WaitFrames count={data[pc+1]}', []

    elif op == 0x27:
        return pc + 3, f'Op_LoadAnimSet {data[pc+1]}, {data[pc+2]}', []

    elif op == 0x28:
        return pc + 2, f'Op_AnimSlotResume slot={data[pc+1]}', []

    elif op == 0x29:
        return pc + 2, f'Op_AnimSlotPause slot={data[pc+1]}', []

    elif op == 0x2A:
        return pc + 2, f'Op_WaitAnimSlotIdle slot={data[pc+1]}', []

    elif op == 0x2B:
        return pc + 3, f'Op_MenuLoadAnims {data[pc+1]}, {data[pc+2]}', []

    elif op == 0x2C:
        return pc + 2, f'Op_MenuUnlock flag={data[pc+1]}', []

    elif op == 0x2D:
        return pc + 2, f'Op_MenuLock flag={data[pc+1]}', []

    elif op == 0x2E:
        return pc + 2, f'Op_WaitMenuReady flag={data[pc+1]}', []

    elif op == 0x2F:
        return pc + 1, 'Op_FullHealParty', []

    elif op == 0x30:
        return pc + 4, f'Op_EquipItem chara={data[pc+1]}, slot={data[pc+2]}, item={data[pc+3]}', []

    elif op == 0x31:
        return pc + 3, f'Op_GiveTakeItem item={data[pc+1]}, count={data[pc+2]}', []

    elif op == 0x32:
        amt = struct.unpack('<H', data[pc+2:pc+4])[0]
        return pc + 4, f'Op_SilverAddSub mode={data[pc+1]}, amount={amt}', []

    elif op == 0x33:
        l = struct.unpack('<H', data[pc+6:pc+8])[0]
        toks = struct.unpack(f'<{l}H', data[pc+8:pc+8+l*2])
        e0, e1 = data[pc+3], data[pc+4]
        t0, t1 = 0x200 + entries[e0], 0x200 + entries[e1]
        text = decode_text(toks)
        return pc + 8 + l * 2, f'Op_DialogChoice e0=Entry_{e0:02X} (0x{t0:04X}), e1=Entry_{e1:02X} (0x{t1:04X}), text="{text}"', [t0, t1]

    elif op == 0x34:
        song = struct.unpack('<H', data[pc+1:pc+3])[0]
        return pc + 4, f'Op_BgmPlay song={song}, arg={data[pc+3]}', []

    elif op == 0x35:
        return pc + 1, 'Op_BgmStop', []

    elif op == 0x36:
        vol = struct.unpack('<H', data[pc+2:pc+4])[0]
        return pc + 4, f'Op_BgmVolume volume={vol}', []

    elif op == 0x37:
        return pc + 2, f'Op_BgmFadeIn speed={data[pc+1]}', []

    elif op == 0x38:
        return pc + 2, f'Op_BgmFadeOut speed={data[pc+1]}', []

    elif op == 0x39:
        sfx = struct.unpack('<H', data[pc+1:pc+3])[0]
        return pc + 4, f'Op_SfxPlay sfx={sfx}, arg={data[pc+3]}', []

    elif op == 0x3A:
        return pc + 2, f'Op_SfxStop sfx={data[pc+1]}', []

    elif op == 0x3B:
        e = data[pc+2]
        target = 0x200 + entries[e]
        return pc + 3, f'Op_IfItemQtyJump item={data[pc+1]} -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x3C:
        return pc + 1, 'Op_ChestOpen', []

    elif op == 0x3D:
        return pc + 2, f'Op_SaveUiTrigger mode={data[pc+1]}', []

    elif op == 0x3E:
        e = data[pc+1]
        target = 0x200 + entries[e]
        return pc + 2, f'Op_IfSaveLoadedJump -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x3F:
        return pc + 2, f'Op_SaveTimerA {data[pc+1]}', []

    elif op == 0x40:
        return pc + 2, f'Op_SaveTimerB {data[pc+1]}', []

    elif op == 0x41:
        e = data[pc+2]
        target = 0x200 + entries[e]
        return pc + 3, f'Op_IfSaveFlagJump flag={data[pc+1]} -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x42:
        return pc + 2, f'Op_SaveOp {data[pc+1]}', []

    elif op == 0x43:
        l = data[pc+1]
        n = l // 2
        flags = struct.unpack(f'<{n}H', data[pc+2:pc+2+l])
        flg_s = ', '.join(f'0x{x:04X}' for x in flags)
        return pc + 2 + l, f'Op_SetFlagsList [{flg_s}]', []

    elif op == 0x44:
        l = data[pc+1]
        n = l // 2
        flags = struct.unpack(f'<{n}H', data[pc+2:pc+2+l])
        flg_s = ', '.join(f'0x{x:04X}' for x in flags)
        return pc + 2 + l, f'Op_ClearFlagsList [{flg_s}]', []

    elif op == 0x45:
        l = data[pc+1]
        e = data[pc+2]
        n = l // 2
        flags = struct.unpack(f'<{n}H', data[pc+3:pc+3+l])
        flg_s = ', '.join(f'0x{x:04X}' for x in flags)
        target = 0x200 + entries[e]
        return pc + 3 + l, f'Op_IfAllFlagsJump [{flg_s}] -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x46:
        l = data[pc+1]
        e = data[pc+2]
        n = l // 2
        flags = struct.unpack(f'<{n}H', data[pc+3:pc+3+l])
        flg_s = ', '.join(f'0x{x:04X}' for x in flags)
        target = 0x200 + entries[e]
        return pc + 3 + l, f'Op_IfAllFlagsClearJump [{flg_s}] -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x47:
        l = data[pc+1]
        e = data[pc+2]
        n = l // 2
        flags = struct.unpack(f'<{n}H', data[pc+3:pc+3+l])
        flg_s = ', '.join(f'0x{x:04X}' for x in flags)
        target = 0x200 + entries[e]
        return pc + 3 + l, f'Op_IfAnyFlagJump [{flg_s}] -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x48:
        return pc + 1, 'Op_ClearSwitchTail', []

    elif op == 0x49:
        e = data[pc+1]
        money = struct.unpack('<H', data[pc+2:pc+4])[0]
        target = 0x200 + entries[e]
        return pc + 4, f'Op_IfMoneyJump threshold={money} -> Entry_{e:02X} (0x{target:04X})', [target]

    elif op == 0x4A:
        return pc + 1, 'Op_StartLogoFade', []

    elif op == 0x4B:
        return pc + 1, 'Op_WaitLogoFade', []

    elif op == 0x4C:
        return pc + 3, f'Op_SetCharacterLevel chara={data[pc+1]}, level={data[pc+2]}', []

    elif op == 0x4D:
        return pc + 3, f'Op_SysEffect subOp=0x{data[pc+1]:02X}, arg=0x{data[pc+2]:02X}', []

    elif op == 0x4E:
        return pc + 3, f'Op_RandomJump [{data[pc+1]}..{data[pc+2]}]', []

    elif op == 0x4F:
        e = data[pc+1]
        target = 0x200 + entries[e]
        return pc + 2, f'Op_ScriptCallAlt Entry_{e:02X} (0x{target:04X})', [target]

    else:
        return pc + 1, f'UNKNOWN_OP_0x{op:02X}', []

# ---------------------------------------------------------------------------
# 4. 主反汇编流程
# ---------------------------------------------------------------------------

def disassemble_script_set(decomp_data):
    """反汇编完整的解压脚本数据集"""
    entries = struct.unpack('<256H', decomp_data[:512])

    entry_map = {}
    for i, off in enumerate(entries):
        addr = 0x200 + off
        if addr not in entry_map:
            entry_map[addr] = []
        entry_map[addr].append(i)

    disassembly = {}
    entry_targets = sorted(entry_map.keys())

    for start_pc in entry_targets:
        cur = start_pc
        while cur < len(decomp_data):
            if cur in disassembly:
                break
            op = decomp_data[cur]
            if op > 0x4F:
                break
            next_pc, dis, branches = parse_instruction(decomp_data, cur, entries)
            disassembly[cur] = (next_pc, dis)
            cur = next_pc
            if op in (0x01, 0x03, 0x06, 0x16):
                break

    return entries, entry_map, disassembly

def format_script_output(entries, entry_map, disassembly, decomp_len):
    """格式化反汇编输出文本"""
    lines = []
    lines.append("=" * 80)
    lines.append(" Lunar Legend (GBA) - Script VM Disassembly")
    lines.append(f" Total Decompressed Size: {decomp_len} bytes (0x{decomp_len:X})")
    lines.append(f" Total Entries: 256 (Unique Entry Points: {len(entry_map)})")
    lines.append("=" * 80)
    lines.append("")
    lines.append("--- [Entry Point Table (0x0000 .. 0x01FF)] ---")
    for i in range(0, 256, 8):
        row = [f"E_{j:02X}: 0x{entries[j]+0x200:04X}" for j in range(i, min(i+8, 256))]
        lines.append("  " + " | ".join(row))
    lines.append("")
    lines.append("=" * 80)
    lines.append("--- [Script Bytecode Disassembly Stream] ---")
    lines.append("=" * 80)

    all_pcs = sorted(disassembly.keys())
    for pc in all_pcs:
        if pc in entry_map:
            labels = ", ".join(f"Entry_{e:02X}" for e in entry_map[pc])
            lines.append("")
            lines.append(f"; ----------------------------------------------------------------------------")
            lines.append(f"; {labels}  @ Offset 0x{pc:04X} (Buffer 0x{0x02016000+pc:08X})")
            lines.append(f"; ----------------------------------------------------------------------------")

        next_pc, dis = disassembly[pc]
        lines.append(f"  [0x{pc:04X}] {dis}")

    return "\n".join(lines)

def main():
    set_id = 0
    target_path = "data/raw_data/unk_862D8A4.bin"
    out_file = "docs/scripts/script_set_000.txt"
    if len(sys.argv) > 1:
        arg = sys.argv[1]
        if arg.isdigit():
            set_id = int(arg)
            set_paths = {
                0: "data/raw_data/unk_862D8A4.bin",
                1: "data/raw_data/unk_862E2A0.bin",
            }
            if set_id not in set_paths:
                print(f"SetId {set_id} not mapped yet.")
                sys.exit(1)
            target_path = set_paths[set_id]
            out_file = f"docs/scripts/script_set_{set_id:03d}.txt"
        else:
            target_path = arg
            stem = os.path.splitext(os.path.basename(target_path))[0]
            out_file = f"docs/scripts/{stem}.txt"

    if not os.path.exists(target_path):
        print(f"Error: File not found: {target_path}")
        sys.exit(1)

    print(f"[*] Reading compressed script: {target_path}")
    with open(target_path, "rb") as f:
        raw = f.read()

    print(f"[*] Raw compressed size: {len(raw)} bytes")
    decomp = lz_uncompress(raw)
    print(f"[*] Decompressed size: {len(decomp)} bytes")

    entries, entry_map, disassembly = disassemble_script_set(decomp)
    print(f"[*] Disassembled {len(disassembly)} instructions across {len(entry_map)} unique entries.")

    output_text = format_script_output(entries, entry_map, disassembly, len(decomp))

    os.makedirs(os.path.dirname(out_file), exist_ok=True)
    with open(out_file, "w", encoding="utf-8") as f:
        f.write(output_text)
    print(f"[+] Successfully wrote disassembly to: {out_file}")

if __name__ == "__main__":
    main()
