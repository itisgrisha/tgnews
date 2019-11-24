from subprocess import call
from multiprocessing import Pool
import os


def process(path):
    dst = '/eee/tgnews/meta/all'
    prefix = '/eee/tgnews/data'
    print("RUNNING", path)
    call(
        ['./build/tgnews', 'dump', os.path.join(prefix, path), os.path.join(dst, path) + '.tsv'],
    )


prefix = '/eee/tgnews/data'
dst = '/eee/tgnews/meta'

tasks = [x for x in os.listdir(prefix) if os.path.isdir(os.path.join(prefix, x))]


with Pool(3) as p:
    p.map(process, tasks)
