#!/usr/bin/env python3
# 一键重算“未匹配函数之间的相似度”表（FN_UNMATCHED_SIMILARITY_REFERENCE）。
#
# 与 gen_fn_similarity.py 的区别：
#   gen_fn_similarity.py          : nonmatchings(目标)  × matchings(模板)
#   本脚本                        : nonmatchings       × nonmatchings（排除自身）
#
# 用途：把彼此同构/同族的未匹配函数聚成批量分析单元
#   （同族先做一个当内部模板，其余复用控制流/声明序/状态机建模）。
#
# 打分逻辑完全复用 gen_fn_similarity.py：
#   指令 bigram Jaccard + bl 目标集合 + .4byte 常量池，权重可调。
#
# 输出：
#   1. 相似族：按 >= --edge-min 的边做并查集聚类，列出成员数 >= 2 的族
#   2. 逐函数 Top N：每个未匹配函数最像的其他未匹配函数
import argparse, glob, os, sys, time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import gen_fn_similarity as G

def main():
    root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    ap = argparse.ArgumentParser()
    ap.add_argument('--asm', default=os.path.join(root, 'asm'))
    ap.add_argument('--out', default=os.path.join(
        root, 'docs', 'FN_UNMATCHED_SIMILARITY_REFERENCE_%s.md' % time.strftime('%Y%m%d')))
    ap.add_argument('--row-min', type=int, default=50, help='Top1 低于此值则整行删除')
    ap.add_argument('--cell-min', type=int, default=50, help='Top2~TopN 低于此值则单元格留空')
    ap.add_argument('--edge-min', type=int, default=60, help='族聚类的成边阈值（%%）')
    ap.add_argument('--topn', type=int, default=5)
    ap.add_argument('--w-instr', type=float, default=0.75)
    ap.add_argument('--w-bl', type=float, default=0.15)
    ap.add_argument('--w-const', type=float, default=0.10)
    a = ap.parse_args()

    nm_dir = os.path.join(a.asm, 'nonmatchings'); mt_dir = os.path.join(a.asm, 'matchings')
    nm_paths = sorted(glob.glob(os.path.join(nm_dir, '*.s')))
    all_paths = nm_paths + glob.glob(os.path.join(mt_dir, '*.s'))
    t0 = time.time()

    label_map = G.build_label_map(all_paths)  # bl 目标解析到 ROM 地址，跨重命名稳定
    fns = [f for f in (G.parse_file(p, label_map) for p in nm_paths) if f]
    n = len(fns)

    # 成对打分矩阵（排除自身）
    pair = [[0.0] * n for _ in range(n)]
    for i in range(n):
        for j in range(n):
            if i != j:
                pair[i][j] = G.score(fns[i], fns[j], a.w_instr, a.w_bl, a.w_const)

    results = []  # (i, [(name, addr, pct) topN])
    for i in range(n):
        scored = sorted(((pair[i][j], j) for j in range(n) if j != i),
                        key=lambda x: -x[0])[:a.topn]
        tops = [(fns[j].name, fns[j].addr, int(round(p * 100))) for p, j in scored]
        if not tops or tops[0][2] < a.row_min:
            continue
        results.append((i, tops))
    results.sort(key=lambda r: (-r[1][0][2], fns[r[0]].addr))

    # 并查集：score >= edge-min 的边聚成“相似族”
    parent = list(range(n))
    def find(x):
        while parent[x] != x:
            parent[x] = parent[parent[x]]
            x = parent[x]
        return x
    def union(x, y):
        rx, ry = find(x), find(y)
        if rx != ry: parent[rx] = ry
    for i in range(n):
        for j in range(i + 1, n):
            if pair[i][j] * 100 >= a.edge_min:
                union(i, j)
    clusters = {}
    for i in range(n):
        clusters.setdefault(find(i), []).append(i)
    # 保留成员数 >= 2 且存在一条 >= edge-min 边的族（过滤仅靠传递性串起来的孤立对）
    groups = []
    for members in clusters.values():
        if len(members) < 2: continue
        if any(pair[i][j] * 100 >= a.edge_min for ii, i in enumerate(members) for j in members[ii + 1:]):
            members.sort(key=lambda i: fns[i].addr)
            best = max(pair[i][j] for ii, i in enumerate(members) for j in members[ii + 1:])
            groups.append((members, int(round(best * 100))))
    groups.sort(key=lambda g: (-len(g[0]), -g[1], fns[g[0][0]].addr))

    L = []
    L.append('# 未匹配函数互相相似度（同族批量分析参考）')
    L.append('')
    L.append('本表由 `scripts/gen_fn_unmatched_similarity.py` 从 `asm/nonmatchings` 内部两两对比自动重算')
    L.append('（与 `FN_MATCH_SIMILARITY_REFERENCE`（未匹配 vs 已匹配模板）互补）。')
    L.append('（仅保留 Top1 相似度 >= %d%% 的行；Top2~Top%d 中 < %d%% 的项留空；族成边阈值 >= %d%%。）'
             % (a.row_min, a.topn, a.cell_min, a.edge_min))
    L.append('')
    L.append('打分方法与 `gen_fn_similarity.py` 完全一致：指令 bigram Jaccard（权重 %.2f）'
             '+ `bl` 目标集合重叠（%.2f）+ `.4byte` 常量池重叠（%.2f），上限 100%%。' % (a.w_instr, a.w_bl, a.w_const))
    L.append('')
    L.append('## 1. 相似族（建议同批认领/顺序分析：先做族内一个当内部模板）')
    L.append('')
    L.append('| # | 成员数 | 族内最高分 | 成员 |')
    L.append('|---:|---:|---:|---|')
    for gi, (members, best) in enumerate(groups, 1):
        cells = ', '.join('%s `0x%08x`' % (fns[i].name, fns[i].addr) for i in members)
        L.append('| %d | %d | %d%% | %s |' % (gi, len(members), best, cells))
    L.append('')
    L.append('## 2. 逐函数 Top %d' % a.topn)
    L.append('')
    hdr = '| 未匹配函数 | 地址 | 指令数 | ' + ' | '.join('Top%d' % (i + 1) for i in range(a.topn)) + ' |'
    L.append(hdr)
    L.append('|---|---|---:|' + '---|' * a.topn)
    for i, tops in results:
        cells = [fns[i].name, '0x%08x' % fns[i].addr, str(fns[i].ninstr)]
        for k in range(a.topn):
            t = tops[k] if k < len(tops) else None
            cells.append(G.cell(t, k == 0, a.cell_min))
        L.append('| ' + ' | '.join(cells) + ' |')
    L.append('')
    L.append('## 3. 使用说明')
    L.append('')
    L.append('- 同族函数适合一次性认领“明确的同构族”（AGENTS.md §2.2），先匹配族内最简单的一个，')
    L.append('  其余复用其结构建模（循环形态、状态机、声明序、home/register 习惯）。')
    L.append('- 族是按阈值边聚类得到的，成员间未必两两高度相似（存在传递性），以 Top N 矩阵为准。')
    L.append('- 相似度只作建模参考，不能替代逐函数字节 diff（fndiff/bytecmp/fncheck）。')
    L.append('- 完成匹配后重跑本脚本与 gen_fn_similarity.py，对应函数会从 nonmatchings 移出。')
    L.append('')
    L.append('<!-- 生成时间: %s | 未匹配: %d | 保留行: %d | 族: %d -->'
             % (time.strftime('%Y-%m-%d %H:%M:%S'), n, len(results), len(groups)))
    open(a.out, 'w', encoding='utf-8').write('\n'.join(L) + '\n')
    print('unmatched=%d kept=%d groups=%d  %.1fs' % (n, len(results), len(groups), time.time() - t0))
    print('wrote', a.out)

if __name__ == '__main__':
    main()
