import sys

dict_path = sys.argv[1]

with open(dict_path, 'r') as f:
    d = [x.split(',')[:3] for x in f.read().split('\n')[2:]]

output = sys.argv[2]
with open(output, 'w') as f:
    for di in d:
        if len(di) != 3:
            print("KEK")
            continue
        x = di[0].strip()
        t = di[1].strip().split("_")[0]
        if not x:
            x = 'krakakpopka'
        if t == 'позвонить':
            print('позвонить')
            x = ','.join(di[:2]).strip()
            t = di[2].strip().split("_")[0]

        f.write(f'{x} {t}\n')
