import glob
import os
import re

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np


class Bench:
    def __init__(self, compiler, size, threads):
        self.compiler = compiler
        self.size = size
        self.threads = threads
        self.init = list()
        self.calc = list()
        self.repetitions = None
        self.bind = None
        self.schedule = None
        self.init_mean = None
        self.calc_mean = None

    def fetch_data(self, lines):
        init_counter = 0
        calc_counter = 0
        for line in lines:
            line = line.lstrip(' ')
            if re.match('.*OMP_SCHEDULE', line):
                line_split = line.split("=")
                line_split = line_split[1].lstrip(' ')
                self.schedule = line_split.replace('\'', '')
            elif re.match('.*OMP_PROC_BIND', line):
                line_split = line.split("=")
                line_split = line_split[1].lstrip(' ')
                self.bind = line_split.replace('\'', '')
            elif line.startswith("Field initializer took"):
                if init_counter:
                    line_split = line.split(' ')
                    self.init.append(float(line_split[3]))
                else:
                    init_counter = True
            elif line.startswith("Calculation took"):
                if calc_counter:
                    line_split = line.split(' ')
                    self.calc.append(float(line_split[2]))
                else:
                    calc_counter = True
            elif line.startswith('We are doing'):
                line_split = line.split(' ')
                self.repetitions = line_split[3]
            elif line.startswith("Done"):
                init_counter = False
                calc_counter = False
        self.init_mean = np.mean(self.init)
        self.calc_mean = np.mean(self.calc)


# Processing .txt files
print("Processing .txt files")
gcc_list = list()
icc_list = list()
for file in glob.glob('./results/*cc_S*_T*.txt'):
    with open(file, "r") as input_file:
        lines = input_file.readlines()
        lines = [line.rstrip() for line in lines]
        file_name = os.path.basename(input_file.name)
        file_name = file_name.split(".", 1)[0]
        file_name = file_name.split("_")
        file_name[1] = file_name[1][1:]
        file_name[2] = file_name[2][1:]
        bench = Bench(file_name[0], int(file_name[1]), int(file_name[2]))
        if file_name[0] == "gcc":
            gcc_list.append(bench)
        else:
            icc_list.append(bench)
        bench.fetch_data(lines)

# create beautiful graphs
print("Creating beautiful graphs")
gcc_list.sort(key=lambda x: x.size, reverse=False)
sub_list = [gcc_list[i:i + 6] for i in range(0, len(gcc_list), 6)]
plt.style.use('ggplot')

# gcc calc time
for s in sub_list:
    x_axes = list()
    y_axes = list()
    s.sort(key=lambda x: x.threads, reverse=False)
    for b in s:
        x_axes.append(b.threads)
        y_axes.append(b.calc_mean)
    plt.plot(x_axes, y_axes, label=b.size)
plt.rcParams['pgf.texsystem'] = "pdflatex"
plt.legend(loc='upper right', title="Gameboard size")
plt.ylabel('time in s')
plt.xlabel('number of threads')
# plt.xticks([1, 2, 4, 8, 16, 32])
plt.yscale("log")
plt.xscale("log", base=2)
plt.gca().xaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
plt.gca().xaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
plt.gca().yaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_minor_locator(mpl.ticker.LogLocator(base=10, subs='all'))
plt.gca().grid(True, which="both")
plt.title("calc time in dependence of size and threads")
plt.savefig("pics/gcc_all.png", dpi=300)
plt.savefig("pics/gcc_all.svg")
plt.savefig('pics/gcc_all.pgf', format='pgf')

print("Done.")
