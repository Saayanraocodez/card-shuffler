#!/usr/bin/env python3
"""
analyze_physical.py — analyse real shuffles from the machine.

Three uses:

1. Predicted-vs-actual (the strongest physical test; see docs/02-mathematics.md §2.8 step 4)
     python3 analyze_physical.py --compare predicted.txt actual.txt
   predicted.txt: the `PLAN order(...)` or `LAST order(...)` line from the serial console
                  (input indices bottom→top, 0 = bottom card of the input deck).
   actual.txt:    the order you read off the output deck, bottom→top, as input indices.
   Reports every discrepancy and classifies it (off-by-one up/down, landed on top = blade miss,
   pair moved together = double feed, other).

2. Output statistics on recorded physical shuffles (weak test, needs hundreds of decks)
     python3 analyze_physical.py --decks decks.csv
   decks.csv: one line per shuffle, 52 comma-separated input indices in output order bottom→top.

3. RNG statistics on gap sequences dumped by the firmware (`test rng 100000 > gaps.csv`)
     python3 analyze_physical.py --gaps gaps.csv

Helper to type in a deck: --enter  (prompts for the 52 cards; use a marked deck or write the input
index on each card's face with a pencil before the test).
"""
import argparse
import sys

import numpy as np

sys.path.insert(0, __file__.rsplit("/", 1)[0] if "/" in __file__ else ".")
from shuffle_sim import build_order, run_all_stats, fmt  # noqa: E402


def read_order_line(path):
    txt = open(path).read()
    if ":" in txt and "order" in txt:
        txt = txt.split(":", 1)[1]
    vals = [int(v) for v in txt.replace(",", " ").split()]
    return vals


def compare(pred, act):
    n = len(pred)
    if sorted(pred) != list(range(n)) or sorted(act) != sorted(pred):
        print("ERROR: the two orders are not permutations of the same cards (typo?)")
        return
    pos_p = {c: i for i, c in enumerate(pred)}
    pos_a = {c: i for i, c in enumerate(act)}
    bad = [c for c in range(n) if pos_p[c] != pos_a[c]]
    if not bad:
        print(f"IDENTICAL: all {n} cards where predicted. 0 mechanical errors in {n} insertions.")
        return
    print(f"{len(bad)} cards not where predicted. Reconstructing per-card insertion errors:")
    # Replay the insertion process against the actual deck: for card i (in feed order), find the gap
    # it actually occupies relative to the cards fed before it.
    errors = []
    for i in range(n):
        below = sum(1 for c in range(i) if pos_a[c] < pos_a[i])       # cards fed earlier that are below i
        below_pred = sum(1 for c in range(i) if pos_p[c] < pos_p[i])
        if below != below_pred:
            d = below - below_pred
            kind = "off-by-one UP" if d == 1 else "off-by-one DOWN" if d == -1 else \
                   "landed on TOP (blade miss?)" if below == i else f"off by {d:+d}"
            errors.append((i, below_pred, below, kind))
    dbl = 0
    for (i, jp, ja, kind) in errors:
        # double feed signature: card i+1 sits directly above card i and card i+1 is also "wrong"
        if i + 1 < n and pos_a[i + 1] == pos_a[i] + 1 and any(e[0] == i + 1 for e in errors):
            kind += "  [pair with next card → double feed?]"; dbl += 1
        print(f"  card {i:2d}: intended gap {jp:2d}, actual gap {ja:2d}  → {kind}")
    ups = sum(1 for e in errors if e[3].startswith("off-by-one UP"))
    downs = sum(1 for e in errors if e[3].startswith("off-by-one DOWN"))
    print(f"\nsummary: {len(errors)} erroneous insertions of {n} ({100*len(errors)/n:.1f} %); "
          f"up={ups} down={downs} top={sum(1 for e in errors if 'TOP' in e[3])} double-feed suspects={dbl}")
    if ups + downs > 0:
        from math import comb
        k, m = max(ups, downs), ups + downs
        p = sum(comb(m, x) for x in range(k, m + 1)) / 2 ** m * 2   # two-sided binomial test
        print(f"direction bias test (two-sided binomial): p = {min(1.0, p):.3f}  "
              f"{'→ BIAS: recalibrate knife offset (' + ('+' if ups > downs else '-') + ')' if p < 0.05 else '→ no significant direction bias'}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--compare", nargs=2, metavar=("PREDICTED", "ACTUAL"))
    ap.add_argument("--decks")
    ap.add_argument("--gaps")
    ap.add_argument("--enter", action="store_true")
    ap.add_argument("--n", type=int, default=52)
    a = ap.parse_args()
    rng = np.random.default_rng(1)

    if a.enter:
        print(f"Type the {a.n} input indices of the output deck from BOTTOM to TOP, separated by spaces:")
        vals = [int(v) for v in sys.stdin.read().split()]
        print(",".join(map(str, vals)))
        return
    if a.compare:
        compare(read_order_line(a.compare[0]), read_order_line(a.compare[1]))
        return
    if a.decks:
        rows = [[int(v) for v in line.replace(",", " ").split()] for line in open(a.decks) if line.strip()]
        order = np.array(rows, dtype=np.int64)
        B, n = order.shape
        print(f"{B} physical shuffles of {n} cards")
        stats = run_all_stats(order, rng, do_guess=B >= 200)
        print(fmt(stats))
        # reference: what a uniform source shows at this sample size
        from shuffle_sim import model_fy
        ref = run_all_stats(model_fy(B, n, rng), rng, do_guess=B >= 200)
        print("\n--- uniform reference at the same sample size ---")
        print(fmt(ref))
        print("\nNote: with a few hundred decks only gross faults are detectable; the predicted-vs-actual test is far stronger.")
        return
    if a.gaps:
        rows = [[int(v) for v in line.replace(",", " ").split()] for line in open(a.gaps) if line.strip() and not line.startswith("#")]
        j = np.array(rows, dtype=np.int64)
        n = j.shape[1]
        bad = np.sum(j > np.arange(n)[None, :])
        print(f"{j.shape[0]} gap sequences of {n}; out-of-range gaps: {bad}")
        order = build_order(j)
        print(fmt(run_all_stats(order, rng, do_guess=True)))
        return
    ap.print_help()


if __name__ == "__main__":
    main()
