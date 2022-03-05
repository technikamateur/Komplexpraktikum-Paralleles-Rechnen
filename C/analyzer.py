import glob
import os
import re
from enum import Enum
from textwrap import wrap

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np


class Type(Enum):
    ASERIAL = "without SIMD"
    BSIMD = "SIMD"
    CEXTREM = "SIMDxorshift"


class Bench:
    def __init__(self, compiler, size, simd):
        self.compiler = compiler
        self.size = size
        self.init = list()
        self.calc = list()
        self.repetitions = 100
        self.init_mean = None
        self.calc_mean = None
        self.type = simd

    def fetch_data(self, lines):
        for line in lines:
            line = line.lstrip(' ')
            if line.startswith("Field initializer took"):
                line_split = line.split(' ')
                self.init.append(float(line_split[3]))

            elif line.startswith("Calculation took"):
                line_split = line.split(' ')
                self.calc.append(float(line_split[2]))

        self.init_mean = np.mean(self.init)
        self.calc_mean = np.mean(self.calc)


# Processing .txt files
print("Processing .txt files")
gcc_list = list()
icc_list = list()
simd = None

# gcc and icc
for file in glob.glob('./results/*cc_S*_*.txt'):
    with open(file, "r") as input_file:
        lines = input_file.readlines()
        lines = [line.rstrip() for line in lines]
        file_name = os.path.basename(input_file.name)
        file_name = file_name.split(".", 1)[0]
        file_name = file_name.split("_")
        file_name[1] = file_name[1][1:]
        try:
            file_name[3]
            simd = Type['CEXTREM']
        except IndexError:
            if file_name[2] == 'serial':
                simd = Type['ASERIAL']
            else:
                simd = Type['BSIMD']
        bench = Bench(file_name[0], int(file_name[1]), simd)
        if file_name[0] == "gcc":
            gcc_list.append(bench)
        else:
            icc_list.append(bench)
        bench.fetch_data(lines)

# create beautiful graphs
print("Creating beautiful graphs")
for measurement in [gcc_list, icc_list]:
    measurement.sort(key=lambda x: x.type.name, reverse=False)
    sub_list = [measurement[i:i + 5] for i in range(0, len(measurement), 5)]

    # calc time plots
    plt.style.use('ggplot')
    # sub list contains all sizes of one Type
    for s in sub_list:
        if s[0].type.value != Type.CEXTREM.value:
            x_axes = list()
            y_axes = list()
            s.sort(key=lambda x: x.size, reverse=False)
            for b in s:
                if b.calc_mean != 0:
                    x_axes.append(b.size)
                    y_axes.append(b.calc_mean)
                    compiler = b.compiler
            plt.plot(x_axes, y_axes, 'o-', label=b.type.value)

    # generate calc plot
    plt.rcParams['pgf.texsystem'] = "pdflatex"
    plt.legend(loc='upper left')
    plt.ylabel('time in s')
    plt.xlabel('gameboard size')
    # axes
    plt.yscale("log")
    plt.xscale("log", base=2)
    plt.gca().xaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
    plt.gca().yaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
    plt.gca().yaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
    plt.gca().yaxis.set_minor_locator(mpl.ticker.LogLocator(base=10, subs='all'))
    plt.xticks([128, 512, 2048, 8192, 32768])
    plt.gca().grid(True, which="both")
    plt.title("\n".join(wrap("calculate_next_gen() mean run time in dependence of size", 60)))
    plt.savefig("pics/png/{}_all_calc.png".format(compiler), dpi=300)
    plt.savefig("pics/svg/{}_all_calc.svg".format(compiler))
    plt.savefig('Bericht/{}_all_calc.pgf'.format(compiler), format='pgf')
    plt.close()

    # now init plot
    plt.style.use('ggplot')
    for s in sub_list:
        if s[0].type.value != Type.BSIMD.value:
            x_axes = list()
            y_axes = list()
            s.sort(key=lambda x: x.size, reverse=False)
            for b in s:
                if b.init_mean != 0:
                    x_axes.append(b.size)
                    y_axes.append(b.init_mean)
                    compiler = b.compiler
            plt.plot(x_axes, y_axes, 'o-', label=b.type.value)

    # generate init plot
    plt.rcParams['pgf.texsystem'] = "pdflatex"
    plt.legend(loc='upper left')
    plt.ylabel('time in s')
    plt.xlabel('gameboard size')
    # axes
    plt.yscale("log")
    plt.xscale("log", base=2)
    plt.gca().xaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
    plt.gca().yaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
    plt.gca().yaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
    plt.gca().yaxis.set_minor_locator(mpl.ticker.LogLocator(base=10, subs='all'))
    plt.xticks([128, 512, 2048, 8192, 32768])
    plt.gca().grid(True, which="both")
    plt.title("\n".join(wrap("field_initializer mean run time in dependence of size", 60)))
    plt.savefig("pics/png/{}_all_init.png".format(compiler), dpi=300)
    plt.savefig("pics/svg/{}_all_init.svg".format(compiler))
    plt.savefig('Bericht/{}_all_init.pgf'.format(compiler), format='pgf')
    plt.close()

print("Output for tables\n")
print("Init time\n")
for measurement in [gcc_list, icc_list]:
    measurement.sort(key=lambda x: x.size, reverse=False)
    for m in measurement:
        if m.type.value != Type.BSIMD.value:
            print("{} & {} & {} & {} & {} \\\ \hline".format(m.compiler, m.size, m.type.value, 100, round(m.init_mean, 4)))

print("\n")
print("Calc time\n")
for measurement in [gcc_list, icc_list]:
    measurement.sort(key=lambda x: x.size, reverse=False)
    for m in measurement:
        if m.type.value != Type.CEXTREM.value:
            print("{} & {} & {} & {} & {} \\\ \hline".format(m.compiler, m.size, m.type.value, 100, round(m.init_mean, 4)))
