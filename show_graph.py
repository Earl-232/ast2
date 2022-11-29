import random
import shutil
import os
from filter_graph import info

range_x = 100 
range_y = 50 
N_PERCENT = .5
H_PERCENT = .2
T_PERCENT = .2
B_PERCENT = .1
GRAPH_DIR = "./graphs/"

info_distr = {info.N: N_PERCENT, info.H: H_PERCENT,
        info.T: T_PERCENT, info.B: B_PERCENT}

def set_attr(nodes, info_type, thresh):
    percent = float(0)
    count = 0
    while percent < thresh:
        randx = random.randrange(range_x)
        randy = random.randrange(range_y)
        if nodes.get((randx, randy), None) is None:
            nodes[(randx, randy)] = info_type
            count += 1
            percent = float(count) / float(range_x * range_y)

def gen_cells(nodes):
    for x in range(range_x):
        for y in range(range_y):
            nodes[(x,y)] = None

    for info_type, percent in info_distr.items():
        set_attr(nodes, info_type, percent)


def new_world():
    nodes = dict()
    gen_cells(nodes)
    return nodes

cleaned = False
def make_world(file_name, nodes):
    global cleaned
    graphs_path = os.path.join(os.getcwd(), GRAPH_DIR)
    if not cleaned:
        if os.path.exists(graphs_path):
            try:
                shutil.rmtree(graphs_path)
            except OSError as e:
                print("Error removing (%s): %s" % (graphs_path, e.strerror))
#create a new graph directory
        try:
            os.mkdir(graphs_path)
        except OSError as e:
            if not os.path.exists(graphs_path):
                print("not created")
                print("Error: %s" % (e.strerror))
                exit(1)
        cleaned = True

#make world files
    graph_fname = graphs_path + file_name + ".txt"
    with open(graph_fname, 'w') as f:
        f.write(str(range_x) + " " + str(range_y) + "\n")
        for x in range(range_x):
            for y in range(range_y):
                if nodes[(x,y)] == info.N:
                    letter = 'N'
                elif nodes[(x,y)] == info.H:
                    letter = 'H'
                elif nodes[(x,y)] == info.T:
                    letter = 'T'
                else:
                    letter = 'B'

                f.write(str(x + 1) + " " + str(y + 1) + " " + str(letter) + "\n")
