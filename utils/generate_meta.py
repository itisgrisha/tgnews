from subprocess import call
from multiprocessing import Pool
import os



prefix = '/eee/tgnews/data'
dst = '/eee/tgnews/meta/all_lang_lists'

def process(path):
    if os.path.isdir(os.path.join(prefix, path)):
        print("RUNNING", path)
        call(
            ['./build/tgnews', 'dump_languages', os.path.join(prefix, path), os.path.join(dst, path) + '.tsv'],
        )


tasks = [x for x in os.listdir(prefix)]


with Pool(1) as p:
    p.map(process, tasks)
