import glob
import os


class Bench:
    def __init__(self, compiler, size, threads):
        self.compiler =compiler
        self.size = size
        self.threads = threads
        self.init = list()
        self.calc = list()
        self.repetitions = None
        self.bind = None
        self.schedule = None

    def fetch_data(self, lines):
        init_counter = 0
        calc_counter = 0
        for line in lines:
            line = line.lstrip(' ')
            if line.startswith("OMP_SCHEDULE"):
                line_split = line.split("=")
                line_split = line_split[1].lstrip(' ')
                self.schedule = line_split.replace('\'', '')
            elif line.startswith("OMP_PROC_BIND"):
                line_split = line.split("=")
                line_split = line_split[1].lstrip(' ')
                self.bind = line_split.replace('\'', '')
            elif line.startswith("Field initializer took"):
                if init_counter:
                    line_split = line.split(' ')
                    self.init.append(line_split[3])
                else:
                    init_counter = True
            elif line.startswith("Calculation took"):
                if calc_counter:
                    line_split = line.split(' ')
                    self.calc.append(line_split[2])
                else:
                    calc_counter = True
            elif line.startswith('We are doing'):
                line_split = line.split(' ')
                self.repetitions = line_split[3]
            elif line.startswith("Done"):
                init_counter = False
                calc_counter = False


all_data = list()

for file in glob.glob('./results/*cc_S*_T*.txt'):
    with open(file, "r") as input_file:
        lines = input_file.readlines()
        lines = [line.rstrip() for line in lines]
        file_name = os.path.basename(input_file.name)
        file_name = file_name.split(".", 1)[0]
        file_name = file_name.split("_")
        file_name[1] = file_name[1][1:]
        file_name[2] = file_name[2][1:]
        bench = Bench(file_name[0], file_name[1], file_name[2])
        all_data.append(bench)
        bench.fetch_data(lines)

for ele in all_data:
    from pprint import pprint
    pprint(vars(ele))
