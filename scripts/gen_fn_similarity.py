#!/usr/bin/env python3
# 一键重算“未匹配函数相似参考”表（FN_MATCH_SIMILARITY_REFERENCE）。
#
# 数据源（天然反映当前匹配进度）：
#   asm/nonmatchings/*.s  -> status=0 未匹配函数（待分析目标）
#   asm/matchings/*.s     -> status=1 已匹配函数（模板）
#
# 相似度方法：
#   1) 指令序列归一化：寄存器->RN、局部标号->LBL、立即数->IMM、bl 目标->CALL、寄存器列表->{RN}
#   2) 主分数 = 归一化指令二文法(bigram)集合的 Jaccard
#   3) 叠加 bl 目标集合 Jaccard（目标解析到 ROM 地址，跨重命名稳定）
#   4) 叠加 .4byte 常量池集合 Jaccard
#   pct = round(100 * min(1, W_instr*jacc_instr + W_bl*jacc_bl + W_const*jacc_const))
import argparse, glob, os, re, time

REG = r'(?:r(?:[0-9]|1[0-5])|ip|sp|lr|pc|sb|sl|fp)'
LBL_DEF  = re.compile(r'^\s*([A-Za-z_][\w.$]*):\s*@ (0x[0-9a-fA-F]+)')
FOURBYTE = re.compile(r'\.4byte\s+(0x[0-9a-fA-F]+|\d+)')
BL       = re.compile(r'^\s*bl(?:\.\w+)?\s+(\S+)')
DIRECTIVE   = re.compile(r'^\s*\.')
FUNC_START  = re.compile(r'^\s*thumb_func')
PURE_LABEL  = re.compile(r'^[\w.$]+:\s*$')

def norm_instr(line):
    s = line.split('@')[0].strip()
    if not s or DIRECTIVE.match(s) or FUNC_START.match(s) or PURE_LABEL.match(s):
        return None
    mnem = s.split()[0].split(',')[0].lower()
    if mnem.endswith(':'):
        return None
    if mnem == 'bl':
        return 'bl CALL'
    t = s
    t = re.sub(r'\{[^}]*\}', '{RN}', t)
    t = re.sub(r'#\s*(?:0x[0-9a-fA-F]+|\d+)', '#IMM', t)
    t = re.sub(r'(?<![\w.])0x[0-9a-fA-F]+(?![\w])', 'IMM', t)
    t = re.sub(r'_[0-9A-Fa-f]{4,}', 'LBL', t)
    t = re.sub(r'\b' + REG + r'\b', 'RN', t)
    return re.sub(r'\s+', ' ', t).strip()

def bigrams(toks):
    return set(zip(toks, toks[1:])) if len(toks) > 1 else set()

class Fn:
    __slots__ = ('name', 'addr', 'bg', 'bl', 'const', 'ninstr')
    def __init__(self, name, addr, bg, bl, const, n):
        self.name, self.addr, self.bg, self.bl, self.const, self.ninstr = name, addr, bg, bl, const, n

def parse_file(path, label_map):
    lines = open(path, encoding='utf-8', errors='replace').read().splitlines()
    name, addr = os.path.basename(path)[:-2], None
    for ln in lines:
        m = LBL_DEF.match(ln)
        if m:
            name, addr = m.group(1), int(m.group(2), 16); break
    if addr is None:
        return None
    toks, consts, bls = [], set(), set()
    for ln in lines:
        fb = FOURBYTE.search(ln)
        if fb: consts.add(int(fb.group(1), 16))
        bm = BL.match(ln)
        if bm: bls.add(label_map.get(bm.group(1), bm.group(1)))
        ni = norm_instr(ln)
        if ni: toks.append(ni)
    return Fn(name, addr, bigrams(toks), bls, consts, len(toks))

def build_label_map(paths):
    mp = {}
    for p in paths:
        for ln in open(p, encoding='utf-8', errors='replace'):
            m = LBL_DEF.match(ln)
            if m: mp[m.group(1)] = int(m.group(2), 16)
    return mp

def jaccard(a, b):
    ua = len(a); ub = len(b)
    if not ua and not ub: return 0.0
    inter = len(a & b)
    return inter / (ua + ub - inter)

def score(u, m, wi, wb, wc):
    s = wi * jaccard(u.bg, m.bg)
    if u.bl or m.bl:   s += wb * jaccard(u.bl, m.bl)
    if u.const or m.const: s += wc * jaccard(u.const, m.const)
    return min(1.0, s)

def cell(t, is_top1, cell_min):
    if t is None: return ''
    nm, addr, pct = t
    if not is_top1 and pct < cell_min: return ''
    return '%s `0x%08x` %d%%' % (nm, addr, pct)

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap = argparse.ArgumentParser()
    ap.add_argument('--asm', default=os.path.join(root, 'asm'))
    ap.add_argument('--out', default=os.path.join(root, 'docs', 'FN_MATCH_SIMILARITY_REFERENCE_20260916.md'))
    ap.add_argument('--row-min', type=int, default=50, help='Top1 低于此值则整行删除')
    ap.add_argument('--cell-min', type=int, default=50, help='Top2~TopN 低于此值则单元格留空')
    ap.add_argument('--topn', type=int, default=5)
    ap.add_argument('--w-instr', type=float, default=0.75)
    ap.add_argument('--w-bl', type=float, default=0.15)
    ap.add_argument('--w-const', type=float, default=0.10)
    a = ap.parse_args()

    nm_dir = os.path.join(a.asm, 'nonmatchings'); mt_dir = os.path.join(a.asm, 'matchings')
    all_paths = glob.glob(os.path.join(nm_dir, '*.s')) + glob.glob(os.path.join(mt_dir, '*.s'))
    t0 = time.time()
    label_map = build_label_map(all_paths)
    unmatched = [f for f in (parse_file(p, label_map) for p in sorted(glob.glob(os.path.join(nm_dir, '*.s')))) if f]
    matched   = [f for f in (parse_file(p, label_map) for p in sorted(glob.glob(os.path.join(mt_dir, '*.s')))) if f]

    results = []  # (u_fn, [(name,addr,pct) topN])
    for u in unmatched:
        scored = sorted(((score(u, m, a.w_instr, a.w_bl, a.w_const), m) for m in matched),
                        key=lambda x: -x[0])[:a.topn]
        tops = [(m.name, m.addr, int(round(p * 100))) for p, m in scored]
        if not tops or tops[0][2] < a.row_min:
            continue
        results.append((u, tops))

    # 按相似度降序：Top1 优先，其次展示分之和，再次地址
    def shown(tops):
        return sum(t[2] for i, t in enumerate(tops) if i == 0 or t[2] >= a.cell_min)
    results.sort(key=lambda r: (-r[1][0][2], -shown(r[1]), r[0].addr))

    groups = {}
    for u, tops in results:
        groups.setdefault(tops[0][0], []).append(u.name)
    g1 = [(t, fs) for t, fs in groups.items() if len(fs) >= 2]
    best = {t: max(tt[0][2] for uu, tt in results if tt[0][0] == t) for t, fs in g1}
    g1.sort(key=lambda x: (-len(x[1]), -best[x[0]], x[0]))

    L = []
    L.append('# 未匹配函数相似参考（用于模板匹配）')
    L.append('')
    L.append('本表由 `scripts/gen_fn_similarity.py` 从 `asm/nonmatchings`（status=0）对 `asm/matchings`（status=1）自动重算。')
    L.append('（仅保留 Top1 相似度 >= %d%% 且当前仍未匹配的函数；Top2~Top%d 中 < %d%% 的项留空；按相似度降序。）'
             % (a.row_min, a.topn, a.cell_min))
    L.append('')
    L.append('方法：')
    L.append('')
    L.append('- 指令序列归一化：寄存器/局部标号/立即数折叠为 `RN/LBL/IMM`，`bl` 折叠为 `CALL`。')
    L.append('- 主分数取指令二文法 Jaccard 相似度（权重 %.2f）。' % a.w_instr)
    L.append('- 叠加 `bl` 调用目标集合重叠（权重 %.2f）、`.4byte` 常量池重叠（权重 %.2f）。' % (a.w_bl, a.w_const))
    L.append('- 最终百分比上限 100%，只作建模模板和结构参考，不替代逐函数字节 diff。')
    L.append('')
    L.append('## 1. 推荐模板组')
    L.append('')
    L.append('| 模板（已匹配） | 可参考未匹配数 | 未匹配函数 |')
    L.append('|---|---:|---|')
    for t, fs in g1:
        L.append('| %s | %d | %s |' % (t, len(fs), ', '.join(fs)))
    L.append('')
    L.append('## 2. 逐函数 Top %d' % a.topn)
    L.append('')
    hdr = '| 未匹配函数 | 地址 | 指令数 | ' + ' | '.join('Top%d' % (i + 1) for i in range(a.topn)) + ' |'
    L.append(hdr)
    L.append('|---|---|---:|' + '---|' * a.topn)
    for u, tops in results:
        cells = [u.name, '0x%08x' % u.addr, str(u.ninstr)]
        for i in range(a.topn):
            t = tops[i] if i < len(tops) else None
            cells.append(cell(t, i == 0, a.cell_min))
        L.append('| ' + ' | '.join(cells) + ' |')
    L.append('')
    L.append('## 3. 使用说明')
    L.append('')
    L.append('- Top1/Top2 只用于找“最值得先看源码/ASM 结构”的模板，不是最终匹配候选。')
    L.append('- 相似度超过 80% 的函数建议直接参考已匹配函数的控制流和声明序。')
    L.append('- 相似度 50%~80% 的函数适合参考同一族的状态机/循环结构，但局部 home/register 可能仍需单独调。')
    L.append('- 完成匹配后重跑本脚本，对应函数会自动从 `nonmatchings` 移出并消失。')
    L.append('')
    L.append('<!-- 生成时间: %s | 目标: %d 未匹配 / %d 模板 | 保留: %d -->'
             % (time.strftime('%Y-%m-%d %H:%M:%S'), len(unmatched), len(matched), len(results)))
    open(a.out, 'w', encoding='utf-8').write('\n'.join(L) + '\n')
    print('targets(unmatched)=%d templates(matched)=%d kept=%d groups=%d  %.1fs'
          % (len(unmatched), len(matched), len(results), len(g1), time.time() - t0))
    print('wrote', a.out)

if __name__ == '__main__':
    main()
