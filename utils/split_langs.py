import os
import csv
from glob import glob
import pandas as pd
from argparse import ArgumentParser

def parse_args():
    parser = ArgumentParser()
    parser.add_argument('--input_folder', default='/eee/tgnews/meta/all')
    parser.add_argument('--output_folder', default='/eee/tgnews/meta/lang')
    parser.add_argument('--langs', nargs='+', default=['ru', 'en'])
    return parser.parse_args()


if __name__ == '__main__':
    args = parse_args()

    table = pd.DataFrame()
    for path in glob(os.path.join(args.input_folder, "**/*.tsv"), recursive=True):
        print(path)
        table = table.append(pd.read_csv(path, sep='\t', quoting=csv.QUOTE_NONE), sort=True, ignore_index=True)

    table.reset_index(drop=True, inplace=True)


    for lang in args.langs:
        table[table['lang']==lang].to_csv(
            os.path.join(args.output_folder, lang+'.tsv'),
            quoting=csv.QUOTE_NONE,
            index=False,
            sep='\t')
