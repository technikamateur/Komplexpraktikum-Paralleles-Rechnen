import glob
import os


class Bench:
    def __init__(self, compiler, size, threads):
        self.compiler =compiler
        self.size = size
        self.threads = threads
        self.init = list()
        self.calc = list()

    def fetch_data(self, lines):
        pass


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
