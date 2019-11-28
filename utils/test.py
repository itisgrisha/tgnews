import json
import unittest
from subprocess import run, PIPE, STDOUT


class CheckEqualScores(unittest.TestCase):
    def test_news_det(self):
        with open("tests/news_det/gt.json", 'r') as f:
            gt_dict = json.load(f)
        proc = run(["./build/tgnews", "news_test", "/eee/tgnews/data/20191101/00/"], stdout=PIPE, text=True)
        prod_dict = json.loads(proc.stdout)
        for k in prod_dict:
            if (prod_dict[k] != 1):
                self.assertAlmostEqual(prod_dict[k], gt_dict[k], delta=1e-6, msg=f"path={k}")



if __name__ == '__main__':
    unittest.main()
