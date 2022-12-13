import display_world
import sys

if len(sys.argv) != 1:
    print("Usage: pythn main.py")
    exit(1)

win = display_world.Window(1600, 900)
win.run()