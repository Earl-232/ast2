import math
from enum import Enum

class info(Enum):
    N = 'N'
    H = 'H'
    T = 'T'
    B = 'B'

    def __str__(self):
        return "{0}".format(self.value)
#default normal
class Node():
    def __init__(self, x, y, i = info.N):
        self.x = x
        self.y = y
        self.info = i;

    def set_info(self, i):
        self.info = i;
#general graph generation, 
class FilterGraph():

    def __init__(self, f):
        self.nodes = []
        self.width = 10
        self.height = 10
        self.src = None
        self.read_file(f)

    def init_graph(self, cells):
        for x in range(self.width):
            row = []
            for y in range(self.height):
                row.append(Node(x, y))
            self.nodes.append(row)

    def read_file(self, fi):
        with open(fi, 'r') as f:
            line = f.readline().strip().split()
            self.src = (int(line[0]) - 1, int(line[1]) - 1) 
            line = f.readline().strip().split()
            self.dst = (int(line[0]) - 1, int(line[1]) - 1) 
            line = f.readline().strip().split()
            self.width = int(line[0])
            self.height = int(line[1])

            cells = dict()
            for line in f.readlines():
                split = line.strip().split()
                cells[(int(split[0]) - 1, int(split[1]) - 1)] = int(split[2])

            self.init_graph(cells)
