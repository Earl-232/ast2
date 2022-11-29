import random
import shutil
import os
from show_graph import new_world, make_world
from filter_graph import info
from show_act import gen_sequence

RESULT_DIR = "./results/"
WORLD_COUNT = 10
ACTIONS_PER_WORLD = 10
SENSOR_FAIL_PERCENT = .1
ACTION_FAIL_PERCENT = .1

def do_action(cells, src, action):
    if random.random() <= ACTION_FAIL_PERCENT:
        return src

    deltax = 0
    deltay = 0
    if action == 'D':
        deltay = 1
    elif action == 'U':
        deltay = -1
    elif action == 'R':
        deltax = 1
    elif action == 'L':
        deltax = -1

    res_coord = (src[0] + deltay, src[1] + deltax)
    dst = cells.get(res_coord, None)
    if dst is None or dst == info.B:
        return src
    return res_coord

def gen_models(cells, src, actions):
    path = []
    observation = ""
    for action in actions:
        result = do_action(cells, src, action)
        path.append(result)
        src = result
        sensor = cells[result]
        if random.random() <= SENSOR_FAIL_PERCENT:
            choices = list(info)
            choices.remove(sensor)
            choices.remove(info.B)
            sensor = random.choice(choices)
        observation += str(sensor)
    return (path, observation)

def create_file(src, t_model, actions, o_model, i, j):
    fname = os.path.join(os.getcwd(), RESULT_DIR)
    fname += "world" + str(i) + "_action" + str(j) + "_result.txt"
    with open(fname, 'w') as f:
        f.write(str(src[0]) + ", " + str(src[1]) + "\n")
        for t in t_model:
            f.write(str(t[0]) + ", " + str(t[1]) + "\n")
        f.write(actions + "\n")
        f.write(o_model + "\n")

def main():
    graphs_path = os.path.join(os.getcwd(), RESULT_DIR)
    if os.path.exists(graphs_path):
        try:
            shutil.rmtree(graphs_path)
        except OSError as e:
            print("Error removing (%s): %s" % (graphs_path, e.strerror))

    try:
        os.mkdir(graphs_path)
    except OSError as e:
        if not os.path.exists(graphs_path):
            print("Error creating graph directory!")
            print("Error: %s" % (e.strerror))
            exit(1)

    for i in range(WORLD_COUNT):
        cells = new_world()
        make_world("world" + str(i), cells)
        for j in range(ACTIONS_PER_WORLD):
            actions = gen_sequence(cells, 100, 50)
            src = actions[0]
            action_str = actions[1]
            models = gen_models(cells, src, action_str)
            transition_model = models[0]
            observation_model = models[1]
            create_file(src, transition_model, action_str, observation_model, i, j)


if __name__ == '__main__':
    main()
