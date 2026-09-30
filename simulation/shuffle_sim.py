#!/usr/bin/env python3
"""
shuffle_sim.py — Monte-Carlo models and statistical tests for the card shuffler.

Models
  fy            exact inside-out Fisher–Yates (what the machine executes when every
                gap index is uniform and the mechanism does what it is told)
  blade_rand    blade lands one gap above/below the intended gap with prob p+/p-
  blade_sys     blade lands one gap ABOVE the intended gap with prob p (calibration bias)
  thick_scale   accumulated thickness-estimate error: gap k is realised as round(k*(1+d))
  blade_miss    blades miss (card lands on TOP of the stack) with prob p, undetected
  double_feed   cards i and i+1 enter the same gap together with prob p, undetected
  gsr           k Gilbert–Shannon–Reeds riffles (hand riffle model)
  mech_riffle   k passes of a two-hopper consumer riffle machine (near-perfect alternation)
  bins          `passes` passes through an N-bin shelf shuffler (inverse N-riffle per pass)

Statistics (all computed on the OUTPUT order; position 0 = bottom of the output deck;
card c = card that was at input position c, 0 = bottom of the input deck):
  position frequency (chi-square), bottom/top card identity, pairwise precedence,
  original-neighbour adjacency, rising sequences, serial correlation of consecutive
  cards' positions, Spearman correlation input vs output position, and an informed
  next-card guessing score (Diaconis–Fulman–Holmes style, learned conditional table).

Also: exact Bayer–Diaconis total-variation distance for a-shuffles (k riffles = 2^k-shuffle),
and empirical total-variation distance for small n where n! is enumerable.

Usage
  python3 shuffle_sim.py                 # full run (a few minutes)
  python3 shuffle_sim.py --quick         # smaller sample sizes
  python3 shuffle_sim.py --model fy --B 200000
  python3 shuffle_sim.py --bd-table      # only the Bayer–Diaconis table
"""
import argparse
import json
import math
import sys
import time
from fractions import Fraction

import numpy as np

N_DEFAULT = 52


# ----------------------------------------------------------------------------------
# Permutation construction from gap indices (vectorised over B shuffles)
# ----------------------------------------------------------------------------------
def draw_gaps(B, n, rng):
    """Uniform gap index j_i in {0..i} for card i (inside-out Fisher–Yates)."""
    j = np.empty((B, n), dtype=np.int64)
    for i in range(n):
        j[:, i] = rng.integers(0, i + 1, size=B)
    return j


def build_order(j):
    """Given gap indices j (B×n, j[:,i] in [0,i]), return order (B×n): order[b,p] = card at
    position p, p = 0 is the bottom of the output stack."""
    B, n = j.shape
    j = np.clip(j, 0, np.arange(n)[None, :])
    pos = np.empty((B, n), dtype=np.int64)
    for i in range(n):
        ji = j[:, i:i + 1]
        if i > 0:
            prev = pos[:, :i]
            prev += (prev >= ji)
        pos[:, i] = j[:, i]
    order = np.empty_like(pos)
    rows = np.arange(B)[:, None]
    order[rows, pos] = np.arange(n)[None, :]
    return order


def inverse(order):
    """pos[b, c] = position of card c."""
    B, n = order.shape
    pos = np.empty_like(order)
    rows = np.arange(B)[:, None]
    pos[rows, order] = np.arange(n)[None, :]
    return pos


# ----------------------------------------------------------------------------------
# Models
# ----------------------------------------------------------------------------------
def model_fy(B, n, rng, **kw):
    return build_order(draw_gaps(B, n, rng))


def model_blade_rand(B, n, rng, p_plus=0.05, p_minus=0.05, **kw):
    j = draw_gaps(B, n, rng)
    u = rng.random((B, n))
    j = j + (u < p_plus).astype(np.int64) - ((u >= p_plus) & (u < p_plus + p_minus)).astype(np.int64)
    return build_order(j)


def model_blade_sys(B, n, rng, p=0.10, **kw):
    j = draw_gaps(B, n, rng)
    j = j + (rng.random((B, n)) < p).astype(np.int64)
    return build_order(j)


def model_thick_scale(B, n, rng, d=0.03, **kw):
    j = draw_gaps(B, n, rng)
    j = np.rint(j * (1.0 + d)).astype(np.int64)
    return build_order(j)


def model_blade_miss(B, n, rng, p=0.02, **kw):
    j = draw_gaps(B, n, rng)
    miss = rng.random((B, n)) < p
    top = np.broadcast_to(np.arange(n)[None, :], (B, n))
    j = np.where(miss, top, j)
    return build_order(j)


def model_double_feed(B, n, rng, p=0.02, **kw):
    j = draw_gaps(B, n, rng)
    for i in range(n - 1):
        m = rng.random(B) < p
        j[m, i + 1] = j[m, i] + 1          # card i+1 rides on top of card i into the same gap
    return build_order(j)


def gsr_riffle_once(deck, rng):
    n = len(deck)
    c = rng.binomial(n, 0.5)
    a, b = list(deck[:c]), list(deck[c:])
    out = []
    while a or b:
        na, nb = len(a), len(b)
        if rng.random() < na / (na + nb):
            out.append(a.pop(0))
        else:
            out.append(b.pop(0))
    return out


def model_gsr(B, n, rng, k=7, **kw):
    orders = np.empty((B, n), dtype=np.int64)
    for b in range(B):
        deck = list(range(n))
        for _ in range(k):
            deck = gsr_riffle_once(deck, rng)
        orders[b] = deck
    return orders


def mech_riffle_once(deck, rng, run_p=(0.85, 0.12, 0.03), cut_sd=1.5):
    """Two-hopper machine: cut near the middle, then alternate packets with short runs."""
    n = len(deck)
    c = int(np.clip(round(n / 2 + rng.normal(0, cut_sd)), 1, n - 1))
    a, b = list(deck[:c]), list(deck[c:])
    out = []
    side = rng.integers(0, 2)
    runs = np.array([1, 2, 3])
    while a or b:
        src = a if side == 0 else b
        if not src:
            src = a if a else b
        r = int(rng.choice(runs, p=run_p))
        for _ in range(min(r, len(src))):
            out.append(src.pop(0))
        side ^= 1
    return out


def model_mech_riffle(B, n, rng, k=7, **kw):
    orders = np.empty((B, n), dtype=np.int64)
    for b in range(B):
        deck = list(range(n))
        for _ in range(k):
            deck = mech_riffle_once(deck, rng)
        orders[b] = deck
    return orders


def model_bins(B, n, rng, nbins=8, passes=1, **kw):
    """Each card to a uniform bin; bins stacked in fixed order; repeat. One pass is an
    inverse nbins-riffle."""
    order = np.broadcast_to(np.arange(n)[None, :], (B, n)).copy()
    for _ in range(passes):
        bins = rng.integers(0, nbins, size=(B, n))
        # stable sort by bin keeps arrival order inside each bin
        idx = np.argsort(bins, axis=1, kind="stable")
        order = np.take_along_axis(order, idx, axis=1)
    return order


MODELS = {
    "fy": model_fy,
    "blade_rand": model_blade_rand,
    "blade_sys": model_blade_sys,
    "thick_scale": model_thick_scale,
    "blade_miss": model_blade_miss,
    "double_feed": model_double_feed,
    "gsr": model_gsr,
    "mech_riffle": model_mech_riffle,
    "bins": model_bins,
}


# ----------------------------------------------------------------------------------
# Statistics
# ----------------------------------------------------------------------------------
def chi2_sf(x, df):
    """Survival function of chi-square (upper tail p-value)."""
    try:
        from scipy.stats import chi2
        return float(chi2.sf(x, df))
    except Exception:  # pragma: no cover
        # Wilson–Hilferty normal approximation
        z = ((x / df) ** (1 / 3) - (1 - 2 / (9 * df))) / math.sqrt(2 / (9 * df))
        return 0.5 * math.erfc(z / math.sqrt(2))


def stat_position_frequency(order):
    B, n = order.shape
    pos = inverse(order)
    counts = np.zeros((n, n), dtype=np.int64)          # counts[c, p]
    for c in range(n):
        counts[c] = np.bincount(pos[:, c], minlength=n)
    expected = B / n
    chi2 = ((counts - expected) ** 2 / expected).sum()
    df = (n - 1) ** 2
    sigma = math.sqrt(expected * (1 - 1 / n))
    zmax = float(np.abs(counts - expected).max() / sigma)
    return {
        "chi2": float(chi2), "df": df, "p_value": chi2_sf(chi2, df),
        "max_abs_z": zmax,
        "bottom_card_chi2_p": chi2_sf(((np.bincount(order[:, 0], minlength=n) - expected) ** 2 / expected).sum(), n - 1),
        "top_card_chi2_p": chi2_sf(((np.bincount(order[:, -1], minlength=n) - expected) ** 2 / expected).sum(), n - 1),
        "P(bottom output = bottom input)": float(np.mean(order[:, 0] == 0)),
        "P(top output = top input)": float(np.mean(order[:, -1] == n - 1)),
        "uniform value": 1.0 / n,
    }, counts


def stat_pairwise(order):
    B, n = order.shape
    pos = inverse(order).astype(np.float64)
    # P(card a below card b) for a<b; vectorised via mean over B of (pos[:,a] < pos[:,b])
    # Compute with broadcasting in chunks to bound memory.
    prob = np.zeros((n, n))
    for a in range(n):
        prob[a] = np.mean(pos[:, a:a + 1] < pos, axis=0)
    iu = np.triu_indices(n, 1)
    dev = prob[iu] - 0.5
    sigma = 0.5 / math.sqrt(B)
    z = dev / sigma
    return {
        "pairs": int(len(dev)),
        "max_abs_dev": float(np.abs(dev).max()),
        "max_abs_z": float(np.abs(z).max()),
        "pairs_beyond_3sigma": int(np.sum(np.abs(z) > 3)),
        "expected_beyond_3sigma_if_uniform": float(len(dev) * 0.0027),
        "mean_P(earlier input card below later input card)": float(prob[iu].mean()),
    }


def stat_adjacency(order):
    B, n = order.shape
    succ = np.mean(np.sum(order[:, 1:] == order[:, :-1] + 1, axis=1))
    both = np.mean(np.sum(np.abs(order[:, 1:] - order[:, :-1]) == 1, axis=1))
    return {
        "mean_original_successor_directly_above": float(succ),
        "expected_if_uniform": (n - 1) / n,
        "mean_original_neighbours_adjacent_either_way": float(both),
        "expected_if_uniform_either_way": 2 * (n - 1) / n,
        "std_err": float(math.sqrt(((n - 1) / n) / B)),
    }


def rising_sequences(order):
    """Number of rising sequences of the permutation read bottom→top: 1 + #{v: pos[v+1] < pos[v]}."""
    pos = inverse(order)
    return 1 + np.sum(pos[:, 1:] < pos[:, :-1], axis=1)


def eulerian_mean_var(n):
    # number of descents of a uniform permutation of n: mean (n-1)/2, var (n+1)/12
    return (n - 1) / 2 + 1, (n + 1) / 12


def stat_rising(order):
    B, n = order.shape
    r = rising_sequences(order)
    mean, var = eulerian_mean_var(n)
    hist = np.bincount(r, minlength=n + 1)[1:n + 1]
    return {
        "mean": float(r.mean()), "expected_if_uniform": mean,
        "std": float(r.std()), "expected_std_if_uniform": math.sqrt(var),
        "z_of_mean": float((r.mean() - mean) / math.sqrt(var / B)),
        "min": int(r.min()), "max": int(r.max()),
        "hist_1_to_n": hist.tolist(),
    }


def stat_serial(order):
    B, n = order.shape
    pos = inverse(order).astype(np.float64)
    # correlation between positions of input-consecutive cards, per shuffle
    x, y = pos[:, :-1], pos[:, 1:]
    xm, ym = x - x.mean(1, keepdims=True), y - y.mean(1, keepdims=True)
    r = (xm * ym).sum(1) / np.sqrt((xm ** 2).sum(1) * (ym ** 2).sum(1))
    # Spearman between input position c and output position pos[c]
    c = np.arange(n, dtype=np.float64)
    cm = c - c.mean()
    pm = pos - pos.mean(1, keepdims=True)
    rho = (pm * cm).sum(1) / np.sqrt((pm ** 2).sum(1) * (cm ** 2).sum())
    return {
        "serial_corr_mean": float(r.mean()),
        "serial_corr_expected_if_uniform": -1.0 / (n - 1),
        "serial_corr_z": float((r.mean() + 1.0 / (n - 1)) / (r.std() / math.sqrt(B))),
        "spearman_mean": float(rho.mean()),
        "spearman_std": float(rho.std()),
        "spearman_expected_std_if_uniform": 1 / math.sqrt(n - 1),
        "spearman_z": float(rho.mean() / (rho.std() / math.sqrt(B))),
    }


def stat_guessing(order, rng, train_frac=0.5):
    """Informed guesser: learns P(next card | current card) and P(first card) from a training
    half, then on the test half guesses, at each position, the most likely UNSEEN card.
    Expected correct guesses per deck for a uniform shuffle (best possible strategy is any
    unseen card): H_n = sum 1/k = 4.54 for n = 52."""
    B, n = order.shape
    Bt = int(B * train_frac)
    train, test = order[:Bt], order[Bt:]
    first = np.bincount(train[:, 0], minlength=n).astype(np.float64) + 1e-9
    trans = np.zeros((n, n)) + 1e-9
    np.add.at(trans, (train[:, :-1].ravel(), train[:, 1:].ravel()), 1)
    correct = 0
    for b in range(len(test)):
        seen = np.zeros(n, dtype=bool)
        prev = None
        for p in range(n):
            w = first if prev is None else trans[prev]
            w = np.where(seen, -1.0, w)
            guess = int(np.argmax(w))
            actual = int(test[b, p])
            correct += guess == actual
            seen[actual] = True
            prev = actual
    Hn = sum(1.0 / k for k in range(1, n + 1))
    return {"correct_per_deck": correct / len(test), "expected_if_uniform": Hn,
            "test_decks": int(len(test))}


def run_all_stats(order, rng, do_guess=True):
    out = {}
    t = time.time()
    out["position_frequency"], _ = stat_position_frequency(order)
    out["pairwise_precedence"] = stat_pairwise(order)
    out["adjacency"] = stat_adjacency(order)
    out["rising_sequences"] = stat_rising(order)
    out["serial_and_rank_correlation"] = stat_serial(order)
    if do_guess:
        sub = order[: min(len(order), 20000)]
        out["next_card_guessing"] = stat_guessing(sub, rng)
    out["_stats_seconds"] = round(time.time() - t, 1)
    return out


# ----------------------------------------------------------------------------------
# Exact theory: Bayer–Diaconis a-shuffle distance, and small-n empirical TV
# ----------------------------------------------------------------------------------
def eulerian_numbers(n):
    """A(n, r) = number of permutations of n with exactly r rising sequences, r = 1..n."""
    A = [[0] * (n + 2) for _ in range(n + 2)]
    A[1][1] = 1
    for m in range(2, n + 1):
        for r in range(1, m + 1):
            A[m][r] = r * A[m - 1][r] + (m - r + 1) * A[m - 1][r - 1]
    return [A[n][r] for r in range(1, n + 1)]


def bayer_diaconis_tv(n, a):
    """Total variation distance between an a-shuffle of n cards and uniform.
    P(π) = C(a + n - r, n) / a^n where r = rising sequences of π (Bayer–Diaconis 1992, Thm 1)."""
    A = eulerian_numbers(n)
    fact = math.factorial(n)
    total = Fraction(0)
    for r in range(1, n + 1):
        p = Fraction(math.comb(a + n - r, n), a ** n)
        total += A[r - 1] * abs(p - Fraction(1, fact))
    return float(total / 2)


def empirical_tv_small_n(model, n, B, rng, **kw):
    """Empirical total-variation distance for small n (needs n! bins). Also returns the
    sampling floor: the TV a truly uniform source shows with the same B."""
    import itertools
    perms = {p: i for i, p in enumerate(itertools.permutations(range(n)))}
    fact = math.factorial(n)

    def tv(order):
        keys = [perms[tuple(row)] for row in order.tolist()]
        counts = np.bincount(keys, minlength=fact) / len(keys)
        return float(np.abs(counts - 1 / fact).sum() / 2)

    return tv(model(B, n, rng, **kw)), tv(model_fy(B, n, rng))


# ----------------------------------------------------------------------------------
def fmt(d, indent=0):
    lines = []
    for k, v in d.items():
        if isinstance(v, dict):
            lines.append(" " * indent + f"{k}:")
            lines.append(fmt(v, indent + 2))
        elif isinstance(v, float):
            lines.append(" " * indent + f"{k}: {v:.6g}")
        elif isinstance(v, list) and len(v) > 12:
            lines.append(" " * indent + f"{k}: [{', '.join(str(x) for x in v[:12])}, ...]")
        else:
            lines.append(" " * indent + f"{k}: {v}")
    return "\n".join(lines)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--model", default=None, help="one model name, default: the standard suite")
    ap.add_argument("--B", type=int, default=None)
    ap.add_argument("--n", type=int, default=N_DEFAULT)
    ap.add_argument("--seed", type=int, default=12345)
    ap.add_argument("--quick", action="store_true")
    ap.add_argument("--bd-table", action="store_true")
    ap.add_argument("--json", default=None, help="write results to this JSON file")
    ap.add_argument("--kw", default="{}", help="model kwargs as JSON, e.g. '{\"p\":0.05}'")
    args = ap.parse_args()
    rng = np.random.default_rng(args.seed)
    n = args.n

    if args.bd_table:
        print("Bayer–Diaconis exact total-variation distance from uniform, n=52")
        print("k riffles (2^k-shuffle):")
        for k in range(1, 13):
            print(f"  k={k:2d}  a={2**k:5d}  TV={bayer_diaconis_tv(52, 2**k):.4f}")
        print("shelf/bin shuffler with N bins, `passes` passes (a = N^passes):")
        for nb in (2, 4, 8, 10, 16):
            for passes in (1, 2, 3, 4):
                print(f"  N={nb:2d} passes={passes}  a={nb**passes:6d}  TV={bayer_diaconis_tv(52, nb**passes):.4f}")
        return

    results = {"n": n, "seed": args.seed, "bayer_diaconis_52": {f"k={k}": bayer_diaconis_tv(52, 2 ** k) for k in range(1, 13)}}

    if args.model:
        B = args.B or (20000 if args.model in ("gsr", "mech_riffle") else 100000)
        suite = [(args.model, json.loads(args.kw), B)]
    else:
        Bfast = 20000 if args.quick else 200000
        Bslow = 3000 if args.quick else 20000
        suite = [
            ("fy", {}, Bfast),
            ("blade_rand", {"p_plus": 0.05, "p_minus": 0.05}, Bfast),
            ("blade_rand", {"p_plus": 0.25, "p_minus": 0.25}, Bfast),
            ("blade_sys", {"p": 0.10}, Bfast),
            ("blade_sys", {"p": 1.00}, Bfast),
            ("thick_scale", {"d": 0.03}, Bfast),
            ("blade_miss", {"p": 0.02}, Bfast),
            ("double_feed", {"p": 0.02}, Bfast),
            ("bins", {"nbins": 8, "passes": 1}, Bfast),
            ("bins", {"nbins": 8, "passes": 4}, Bfast),
            ("gsr", {"k": 1}, Bslow),
            ("gsr", {"k": 7}, Bslow),
            ("gsr", {"k": 12}, Bslow),
            ("mech_riffle", {"k": 1}, Bslow),
            ("mech_riffle", {"k": 7}, Bslow),
        ]

    for name, kw, B in suite:
        label = name + ("" if not kw else " " + json.dumps(kw))
        t = time.time()
        order = MODELS[name](B, n, rng, **kw)
        gen_s = time.time() - t
        stats = run_all_stats(order, rng, do_guess=True)
        stats["_B"] = B
        stats["_generate_seconds"] = round(gen_s, 1)
        results[label] = stats
        print("=" * 78)
        print(f"MODEL {label}   B={B}   n={n}")
        print(fmt(stats))
        sys.stdout.flush()

    # small-n exact TV for the physical fault models
    print("=" * 78)
    print("Empirical total-variation distance, n=6 (720 permutations), B=1,000,000")
    Bs = 200000 if args.quick else 1000000
    tvres = {}
    for name, kw in [("fy", {}), ("blade_rand", {"p_plus": 0.05, "p_minus": 0.05}),
                     ("blade_sys", {"p": 0.10}), ("blade_sys", {"p": 1.0}), ("blade_miss", {"p": 0.02}),
                     ("double_feed", {"p": 0.02}), ("bins", {"nbins": 8, "passes": 1}),
                     ("bins", {"nbins": 8, "passes": 3})]:
        tv, floor = empirical_tv_small_n(MODELS[name], 6, Bs, rng, **kw)
        label = name + ("" if not kw else " " + json.dumps(kw))
        tvres[label] = {"tv": tv, "sampling_floor_uniform": floor}
        print(f"  {label:45s} TV={tv:.4f}   (uniform source with same B: {floor:.4f})")
    results["small_n_tv"] = tvres

    if args.json:
        with open(args.json, "w") as f:
            json.dump(results, f, indent=1)
        print("wrote", args.json)


if __name__ == "__main__":
    main()
