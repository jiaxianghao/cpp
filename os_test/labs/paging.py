"""FIFO / LRU 页框模拟；列表左端是下一次淘汰候选。"""
import argparse


def simulate(references, capacity, policy):
    if capacity <= 0:
        raise ValueError("frames must be positive")
    if policy not in ("fifo", "lru"):
        raise ValueError("unknown policy")
    frames, trace = [], []
    faults = 0
    for page in references:
        hit = page in frames
        if hit:
            if policy == "lru":
                frames.remove(page)
                frames.append(page)
        else:
            faults += 1
            if len(frames) == capacity:
                frames.pop(0)
            frames.append(page)
        trace.append((page, hit, list(frames)))
    return trace, faults


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--policy", choices=("fifo", "lru"), default="fifo")
    parser.add_argument("--frames", type=int, default=3)
    parser.add_argument("--refs", type=int, nargs="+", default=[1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5])
    args = parser.parse_args()
    try:
        trace, faults = simulate(args.refs, args.frames, args.policy)
    except ValueError as error:
        parser.error(str(error))
    for page, hit, frames in trace:
        print(f"page={page} {'HIT ' if hit else 'MISS'} frames={frames}")
    print(f"faults={faults} hits={len(trace) - faults}")


if __name__ == "__main__":
    main()
