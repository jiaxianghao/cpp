"""核对手算结果与进程/线程实验不变量，不依赖执行速度。"""
import importlib.util
from pathlib import Path
import re
import subprocess
import sys
import unittest

ROOT = Path(__file__).resolve().parents[1]


def load(name):
    spec = importlib.util.spec_from_file_location(name, ROOT / "labs" / f"{name}.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


schedule = load("schedule")
paging = load("paging")


class Models(unittest.TestCase):
    def test_scheduling_hand_calculation(self):
        expected = {
            "fifo": ([0, 5, 7], [5, 7, 8]),
            "sjf": ([3, 1, 0], [8, 3, 1]),
            "rr": ([0, 2, 4], [8, 4, 5]),
        }
        for policy, (responses, completions) in expected.items():
            with self.subTest(policy=policy):
                trace, first, done = schedule.simulate([5, 2, 1], policy)
                self.assertEqual([first[i] for i in range(3)], responses)
                self.assertEqual([done[i] for i in range(3)], completions)
                self.assertEqual(sum(b - a for _, a, b in trace), 8)

    def test_large_quantum_matches_fifo(self):
        self.assertEqual(schedule.simulate([5, 2, 1], "rr", 10),
                         schedule.simulate([5, 2, 1], "fifo"))

    def test_belady(self):
        refs = [1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5]
        self.assertEqual(paging.simulate(refs, 3, "fifo")[1], 9)
        self.assertEqual(paging.simulate(refs, 4, "fifo")[1], 10)

    def test_hits_change_lru_order_only(self):
        refs = [1, 2, 1, 3, 1]
        self.assertEqual(paging.simulate(refs, 2, "fifo")[1], 4)
        self.assertEqual(paging.simulate(refs, 2, "lru")[1], 3)

    def test_invalid_parameters(self):
        with self.assertRaises(ValueError):
            schedule.simulate([0], "rr")
        with self.assertRaises(ValueError):
            schedule.simulate([1], "rr", 0)
        with self.assertRaises(ValueError):
            paging.simulate([1], 0, "fifo")


class Programs(unittest.TestCase):
    def run_program(self, *args):
        return subprocess.run(args, cwd=ROOT, text=True, capture_output=True,
                              check=True, timeout=15).stdout

    def test_process(self):
        output = self.run_program(str(ROOT / "build/process"))
        self.assertIn("parent: value=10", output)
        self.assertIn("child_exit=7", output)
        self.assertEqual(re.search(r"child: pid=(\d+)", output).group(1),
                         re.search(r"exec image: pid=(\d+)", output).group(1))
        addresses = re.findall(r"address=(0x[0-9a-f]+)", output)
        self.assertEqual(len(addresses), 3)
        self.assertEqual(len(set(addresses)), 1)

    def test_pipe_eof(self):
        self.assertIn("bytes=11 EOF reached", self.run_program(str(ROOT / "build/pipe")))

    def test_counter(self):
        for mode, result in (("split", 10000), ("mutex", 20000), ("atomic", 20000)):
            with self.subTest(mode=mode):
                self.assertIn(f"actual={result}",
                              self.run_program(str(ROOT / "build/counter"), mode))

    def test_queue(self):
        self.assertIn("sum=50005000 expected=50005000 ordered=1",
                      self.run_program(str(ROOT / "build/queue")))

    def test_replace_and_old_fd(self):
        output = self.run_program(sys.executable, str(ROOT / "labs/durable.py"))
        self.assertIn("old fd: version=old", output)
        self.assertIn("new open: version=new", output)


if __name__ == "__main__":
    unittest.main()
