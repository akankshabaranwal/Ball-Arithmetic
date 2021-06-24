from itertools import groupby

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np
import sys

########################################################
# Cost functions for different versions of operations, #
#  commented has radius, uncommented is midpoint only. #
########################################################

def barith_add_cost_rad(limbs):
    # return 5 * limbs
    return 2 * limbs


def barith_add_cost_norad(limbs):
    # return 2 * limbs + 28
    return 2 * limbs


def barith2_add_cost(limbs):
    # return 2 * limbs + 8
    return 2 * limbs


def barith_simd_cost(limbs):
    # return 8 * limbs + 32
    return 8 * limbs


def barith_mul_cost_rad(limbs):
    # return 4 * (limbs ** 2) + 8 * (limbs ** 2) + 4 * limbs
    return 4 * limbs ** 2


def barith_mul_cost_norad(limbs):
    # return 4 * (limbs ** 2) + 2 * limbs + 36
    return 4 * limbs ** 2


def barith2_mul_cost(limbs):
    # return 4 * (limbs ** 2) + 12
    return 4 * limbs ** 2


cost_functions = {
    "barith_add_portable": barith_add_cost_rad,
    "barith_add_intrinsics": barith_add_cost_rad,
    "barith_add_shiftr": barith_add_cost_rad,
    "barith_add_shiftl": barith_add_cost_rad,
    "barith_add_detect1": barith_add_cost_rad,
    "barith_add_merged": barith_add_cost_rad,
    "barith_add_midptrep": barith_add_cost_rad,
    "barith_add_nestedbranch": barith_add_cost_rad,
    "barith_add_norad_noexp": barith_add_cost_norad,
    "barith2_add": barith2_add_cost,
    "barith2_add_optim1": barith2_add_cost,
    "barith2_add_O0": barith2_add_cost,
    "barith2_add_O2": barith2_add_cost,
    "barith2_add_fastest": barith2_add_cost,
    "barith2_add4_scalar": barith_simd_cost,
    "barith2_add4_simd": barith_simd_cost,
    "barith_sub_portable": barith_add_cost_rad,
    "barith_sub_intrinsics": barith_add_cost_rad,
    "barith_sub_shiftr": barith_add_cost_rad,
    "barith_sub_shiftl": barith_add_cost_rad,
    "barith_sub_detect1": barith_add_cost_rad,
    "barith_sub_merged": barith_add_cost_rad,
    "barith_sub_midptrep": barith_add_cost_rad,
    "barith_sub_nestedbranch": barith_add_cost_rad,
    "barith_sub_norad_noexp": barith_add_cost_norad,
    "barith2_sub": barith_add_cost_norad,
    "barith2_sub_O0": barith_add_cost_norad,
    "barith2_sub_O2": barith_add_cost_norad,
    "barith2_sub_fastest": barith_add_cost_norad,
    "barith2_sub_optim1": barith_add_cost_norad,
    "barith_mul_portable": barith_mul_cost_rad,
    "barith_mul_intrinsic": barith_mul_cost_rad,
    "barith_mul_rad_opt": barith_mul_cost_norad,
    "barith_mul_unroll": barith_mul_cost_norad,
    "barith2_mul": barith2_mul_cost,
    "barith2_mul_O0": barith2_mul_cost,
    "barith2_mul_O2": barith2_mul_cost,
    "barith2_mul_fastest": barith2_mul_cost,
}


def get_iops(name, prec):
    n = prec / 64
    # return n ** 2 + n / 2 + 28
    # return 3 * (n ** 2) + 10 * n # This includes all the error bound calculations in fp
    return cost_functions[name](n)


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
    # plt.plot(x, y, 'o-', label=name)
    z = get_iops(name, x) / y
    plt.plot(x, z, 'o-', label=name)

plt.xscale('log', base=2)
plt.xticks(list(x_ticks), list(map(sizeof_fmt, x_ticks)), rotation=45)
plt.xlabel('Precision [bits]')
plt.ylabel('Performance [IOPS/cyc]')
# plt.ylabel('Throughput [bytes/cyc]')
plt.grid()
# plt.legend()
plt.tight_layout()


# Shrink current axis's height on the bottom
ax = plt.gca()
box = ax.get_position()
ax.set_position([box.x0, box.y0 - box.height * 0.10, box.width, box.height * 0.8])

ax.legend(loc='upper center', bbox_to_anchor=(0.465, 1.4), ncol=3, fancybox=True, shadow=True)

fig = mpl.pyplot.gcf()
print(fig.get_size_inches())
fig.set_size_inches(7, 4)

plt.savefig(in_file + '.png')
