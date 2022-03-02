import glob
import os
from textwrap import wrap

import matplotlib as mpl
import matplotlib.pyplot as plt
import numpy as np

bench_list = list()


def find_bench(size, ranks):
    for b in bench_list:
        if b.ranks == ranks and b.size == size:
            return b
    print("Fatal: Bench not found for: size,ranks {},{}".format(size, ranks))
    exit(1)


class Bench:
    def __init__(self):
        self.size = self.ranks = self.repetitions = None
        self.init, self.calc, self.total = (list() for _ in range(3))
        self.init_mean = self.calc_mean = self.total_mean = None

    def fetch_data(self, lines):
        for line in lines:
            if line.startswith("Field initializer took"):
                line_split = line.split(' ')
                self.init.append(float(line_split[3]))
            elif line.startswith("Calculation took"):
                line_split = line.split(' ')
                self.calc.append(float(line_split[2]))
            elif line.startswith('We are doing'):
                line_split = line.split(' ')
                self.repetitions = line_split[3]
            elif line.startswith('Running with'):
                line_split = line.split(' ')
                self.ranks = int(line_split[2])
            elif line.startswith('Game size: Columns:'):
                line_split = line.split(' ')
                self.size = int(line_split[3][:-1])

    def mean(self):
        self.init_mean = np.mean(self.init)
        self.calc_mean = np.mean(self.calc)
        self.total_mean = np.mean(self.total)


# Processing .txt files
print("Processing .txt files")

# grep output files
for file in glob.glob('./results/Tasks*.txt'):
    with open(file, "r") as input_file:
        lines = input_file.readlines()
        lines = [line.rstrip() for line in lines]
        lines = [line.lstrip() for line in lines]
        bench = Bench()
        bench_list.append(bench)
        bench.fetch_data(lines)

# combine with total execution time
for file in glob.glob('./results/err*.txt'):
    with open(file, "r") as total_time:
        lines = total_time.readlines()[6:]
        lines = [line.rstrip() for line in lines]
        lines = [line.lstrip() for line in lines]

        file_name = os.path.basename(total_time.name)
        size = 2048
        i = 0
        ranks = int(file_name.split(".")[0][3:])

        current_bench = find_bench(size, ranks)
        for line in lines:
            if i == 20 and line.startswith("real"):
                i = 0
                size = size * 4
                current_bench = find_bench(size, ranks)
            if line.startswith("real"):
                i += 1
                line_split = line.split('\t')
                mins = int(line_split[1].split("m")[0])
                secs = float(line_split[1].split("m")[1][:-1])
                tot = mins * 60 + secs
                current_bench.total.append(tot)

for b in bench_list:
    b.mean()
bench_list.sort(key=lambda x: x.size, reverse=False)
sub_bench_list = list()
size = 2048
tmp_list = list()
for b in bench_list:
    if b.size == size:
        tmp_list.append(b)
    else:
        size = b.size
        tmp_list.sort(key=lambda x: x.ranks, reverse=False)
        sub_bench_list.append(tmp_list)
        tmp_list = list()
        tmp_list.append(b)
tmp_list.sort(key=lambda x: x.ranks, reverse=False)
sub_bench_list.append(tmp_list)

# create beautiful graphs
print("Creating beautiful graphs")
plt.style.use('ggplot')
for sub in sub_bench_list:
    # calc time plots
    x_axes = list()
    y_axes = list()
    for b in sub:
        x_axes.append(b.ranks)
        y_axes.append(b.calc_mean)
    plt.plot(x_axes, y_axes, 'o-', label=b.size)

# generate calc plot
plt.rcParams['pgf.texsystem'] = "pdflatex"
plt.legend(loc='upper right', title="Gameboard size")
plt.ylabel('time in s')
plt.xlabel('number of ranks')
# axes
plt.yscale("log")
plt.xscale("log", base=2)
plt.gca().xaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
# plt.gca().xaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
plt.gca().yaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_minor_locator(mpl.ticker.LogLocator(base=10, subs='all'))
plt.xticks([1, 4, 16, 64, 128, 256])
plt.gca().grid(True, which="both")
plt.title("\n".join(wrap("calculate_next_gen mean run time in dependence of size and ranks", 60)))
#    plt.savefig("pics/{}_all_calc.png".format(compiler), dpi=300)
plt.savefig("pics/all_calc.svg")
plt.savefig('Bericht/all_calc.pgf', format='pgf')
plt.close()

# init plot
plt.style.use('ggplot')
for sub in sub_bench_list:
    # calc time plots
    x_axes = list()
    y_axes = list()
    for b in sub:
        x_axes.append(b.ranks)
        y_axes.append(b.init_mean)
    plt.plot(x_axes, y_axes, 'o-', label=b.size)

plt.rcParams['pgf.texsystem'] = "pdflatex"
plt.legend(loc='upper right', title="Gameboard size")
plt.ylabel('time in s')
plt.xlabel('number of ranks')
# axes
plt.yscale("log")
plt.xscale("log", base=2)
plt.gca().xaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
# plt.gca().xaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
plt.gca().yaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_minor_locator(mpl.ticker.LogLocator(base=10, subs='all'))
plt.xticks([1, 4, 16, 64, 128, 256])
plt.gca().grid(True, which="both")
plt.title("\n".join(wrap("field_initializer mean run time in dependence of size and ranks", 60)))
#    plt.savefig("pics/{}_all_calc.png".format(compiler), dpi=300)
plt.savefig("pics/all_init.svg")
plt.savefig('Bericht/all_init.pgf', format='pgf')
plt.close()

# total plot
plt.style.use('ggplot')
for sub in sub_bench_list:
    # calc time plots
    x_axes = list()
    y_axes = list()
    for b in sub:
        x_axes.append(b.ranks)
        y_axes.append(b.total_mean)
    plt.plot(x_axes, y_axes, 'o-', label=b.size)

plt.rcParams['pgf.texsystem'] = "pdflatex"
plt.legend(loc='upper right', title="Gameboard size")
plt.ylabel('time in s')
plt.xlabel('number of ranks')
# axes
plt.yscale("log")
plt.xscale("log", base=2)
plt.gca().xaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
# plt.gca().xaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
plt.gca().yaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_minor_locator(mpl.ticker.LogLocator(base=10, subs='all'))
plt.xticks([1, 4, 16, 64, 128, 256])
plt.gca().grid(True, which="both")
plt.title("\n".join(wrap("total mean run time in dependence of size and ranks", 60)))
#    plt.savefig("pics/{}_all_calc.png".format(compiler), dpi=300)
plt.savefig("pics/all_total.svg")
plt.savefig('Bericht/all_total.pgf', format='pgf')
plt.close()

# overhead plot
plt.style.use('ggplot')
for sub in sub_bench_list:
    # calc time plots
    x_axes = list()
    y_axes = list()
    for b in sub:
        x_axes.append(b.ranks)
        y_axes.append(b.total_mean - (b.calc_mean + b.init_mean))
    plt.plot(x_axes, y_axes, 'o-', label=b.size)

plt.rcParams['pgf.texsystem'] = "pdflatex"
plt.legend(loc='upper right', title="Gameboard size")
plt.ylabel('time in s')
plt.xlabel('number of ranks')
# axes
plt.yscale("log")
plt.xscale("log", base=2)
plt.gca().xaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
# plt.gca().xaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
plt.gca().yaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_minor_locator(mpl.ticker.LogLocator(base=10, subs='all'))
plt.xticks([1, 4, 16, 64, 128, 256])
plt.gca().grid(True, which="both")
plt.title("\n".join(wrap("overhead in dependence of size and ranks", 60)))
#    plt.savefig("pics/{}_all_calc.png".format(compiler), dpi=300)
plt.savefig("pics/all_overhead.svg")
plt.savefig('Bericht/all_overhead.pgf', format='pgf')
plt.close()

# total - overhead plot
plt.style.use('ggplot')
for sub in sub_bench_list:
    # calc time plots
    x_axes = list()
    y_axes = list()
    for b in sub:
        x_axes.append(b.ranks)
        y_axes.append(b.calc_mean + b.init_mean)
    plt.plot(x_axes, y_axes, 'o-', label=b.size)

plt.rcParams['pgf.texsystem'] = "pdflatex"
plt.legend(loc='upper right', title="Gameboard size")
plt.ylabel('time in s')
plt.xlabel('number of ranks')
# axes
plt.yscale("log")
plt.xscale("log", base=2)
plt.gca().xaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
# plt.gca().xaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_major_formatter(mpl.ticker.ScalarFormatter())
plt.gca().yaxis.set_major_formatter(mpl.ticker.EngFormatter(places=0))
plt.gca().yaxis.set_minor_locator(mpl.ticker.LogLocator(base=10, subs='all'))
plt.xticks([1, 4, 16, 64, 128, 256])
plt.gca().grid(True, which="both")
plt.title("\n".join(wrap("init + calc mean run time in dependence of size and ranks without overhead", 60)))
#    plt.savefig("pics/{}_all_calc.png".format(compiler), dpi=300)
plt.savefig("pics/all_init+calc.svg")
plt.savefig('Bericht/all_init+calc.pgf', format='pgf')
plt.close()

print("Done.")
