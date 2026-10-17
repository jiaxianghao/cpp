"""所有任务 t=0 到达、仅 CPU、零切换开销的调度模型。"""
import argparse
from collections import deque


def simulate(bursts, policy, quantum=2):
    if not bursts or any(b <= 0 for b in bursts) or quantum <= 0:
        raise ValueError("bursts and quantum must be positive")
    if policy not in ("fifo", "sjf", "rr"):
        raise ValueError("unknown policy")
    order = list(range(len(bursts)))
    if policy == "sjf":
        order.sort(key=lambda i: bursts[i])
    ready = deque(order)
    remaining = list(bursts)
    first, completion, timeline = {}, {}, []
    now = 0
    while ready:
        job = ready.popleft()
        first.setdefault(job, now)
        duration = min(quantum, remaining[job]) if policy == "rr" else remaining[job]
        timeline.append((job, now, now + duration))
        now += duration
        remaining[job] -= duration
        if remaining[job]:
            ready.append(job)
        else:
            completion[job] = now
    return timeline, first, completion


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--policy", choices=("fifo", "sjf", "rr"), default="rr")
    parser.add_argument("--bursts", type=int, nargs="+", default=[5, 2, 1])
    parser.add_argument("--quantum", type=int, default=2)
    args = parser.parse_args()
    try:
        timeline, first, completion = simulate(args.bursts, args.policy, args.quantum)
    except ValueError as error:
        parser.error(str(error))
    print("timeline:", " ".join(f"J{i}:{a}-{b}" for i, a, b in timeline))
    print("job burst response turnaround waiting")
    for i, burst in enumerate(args.bursts):
        print(f"J{i} {burst:5} {first[i]:8} {completion[i]:10} {completion[i] - burst:7}")
    print(f"mean response={sum(first.values()) / len(first):.2f} "
          f"turnaround={sum(completion.values()) / len(completion):.2f}")


if __name__ == "__main__":
    main()
