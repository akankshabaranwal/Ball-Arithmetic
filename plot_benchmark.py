from itertools import groupby

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import sys


def get_iops_mul(prec):
    n = prec / 64
    # return n ** 2 + n / 2 + 28
    # return 3 * (n ** 2) + 10 * n # This includes all the error bound calculations in fp
    return 4 * n


plt.rcParams['figure.figsize'] = [6.0, 3.0]
plt.rcParams['figure.dpi'] = 140

if len(sys.argv) > 1:
    in_file = sys.argv[1]
else:
    in_file = 'benchmark'

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

def sizeof_fmt(num, suffix='b'):
    for unit in ['','K','M','G','T','P','E','Z']:
        if abs(num) < 1024:
            return "%3d %s%s" % (num, unit, suffix)
        num //= 1024
    return "%d %s%s" % (num, 'Y', suffix)

x_ticks = set()

for name, (x, y) in benchmarks.items():
    x, y = np.array(x), np.array(y)
    x_ticks.update(x)
    # y = x / (8 * y)
    z = get_iops_mul(x) / y
    # plt.plot(x, y, 'o-', label=name)
    plt.plot(x, z, 'o-', label=name)

plt.xscale('log', base=2)
plt.xticks(list(x_ticks), list(map(sizeof_fmt, x_ticks)), rotation=45)
plt.xlabel('Precision [bits]')
plt.ylabel('Performance [IOPS/cyc]')
plt.grid()
plt.legend()
plt.tight_layout()
plt.savefig(in_file + '.png')
