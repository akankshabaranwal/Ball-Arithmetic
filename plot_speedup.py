from itertools import groupby

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import sys


def sizeof_fmt(num, suffix='b'):
    for unit in ['','K','M','G','T','P','E','Z']:
        if abs(num) < 1024:
            return "%3d %s%s" % (num, unit, suffix)
        num //= 1024
    return "%d %s%s" % (num, 'Y', suffix)


plt.rcParams['figure.figsize'] = [7.0, 4.0]
plt.rcParams['figure.dpi'] = 140

if len(sys.argv) > 1:
    in_file = sys.argv[1]
else:
    in_file = 'benchmark'

if len(sys.argv) > 2:
    base = sys.argv[2]
else:
    base = 'portable'

with open(in_file + '.log', 'r') as f:
    bench_out = f.read()

benchmarks = [line for line in bench_out.splitlines() if line != '']
benchmarks = [line.split(' : ') for line in benchmarks]
benchmarks = [(tuple(name.split()[:2]), cycles.split()[0]) for name, cycles in benchmarks]
benchmarks = [((name, int(bits)), float(cycles)) for (name, bits), cycles in benchmarks]

key = lambda b: b[0]
benchmarks = [(key, [float(c) for k, c in value]) for key, value in groupby(sorted(benchmarks, key=key), key=key)]
benchmarks = [(name, (bits, min(cycles))) for (name, bits), cycles in benchmarks]
benchmarks = [(key, [v for k, v in value]) for key, value in groupby(benchmarks, lambda b: b[0])]
benchmarks = [(key, [list(v) for v in zip(*value)]) for key, value in benchmarks]
benchmarks = dict(benchmarks)

x_ticks = set()

for name, (x, y) in benchmarks.items():
    if name != base:
        x, y = np.array(x), np.array(y)
        x_ticks.update(x)
        y = np.array(benchmarks[base][1]) / y
        plt.plot(x, y, 'o-', label=name)

plt.xscale('log', base=2)
plt.xticks(list(x_ticks), list(map(sizeof_fmt, x_ticks)), rotation=45)
plt.xlabel('Precision [bits]')
plt.ylabel('Speedup over ' + base)
plt.grid()
# plt.legend()
plt.tight_layout()

# Shrink current axis's height by 10% on the bottom
ax = plt.gca()
box = ax.get_position()
ax.set_position([box.x0, box.y0 - box.height * 0.05, box.width, box.height * 0.85])
#
# # Put a legend below current axis
# ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.05), fancybox=True, shadow=True, ncol=5)
ax.legend(loc='upper center', bbox_to_anchor=(0.5, 1.25), ncol=2, fancybox=True, shadow=True)

plt.savefig(in_file + '_speedup' + '.png')
