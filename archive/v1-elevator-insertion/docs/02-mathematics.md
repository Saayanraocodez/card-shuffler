# 2. Mathematics of the shuffle

This document separates four kinds of statements and labels each:

* **[PROOF]** — a mathematical statement that holds for the idealised model.
* **[SIM]** — a Monte-Carlo result from `simulation/shuffle_sim.py`
  (results in `simulation/results/full_results.txt`).
* **[EST]** — an engineering estimate.
* **[PHYS]** — a physical measurement. **There are none yet; the machine has
  not been built.** The validation plan in §2.8 says how to get them.

## 2.1 What "random" has to mean here

A shuffle produces an element π of S_n, the set of all n! orderings of the
deck (n = 52: 52! ≈ 8.07 × 10^67). The goal is:

> **Uniformity.** For every ordering π, P(output = π) = 1/n!, and the
> output is independent of the input order and of everything a player can
> observe (previous outputs, timing, sounds).

This needs three different things that are often conflated:

1. **Physical entropy.** Unpredictable bits from a physical process. This is
   what makes the outcome unknowable in advance. Bits needed per shuffle:
   log₂(52!) = **225.6 bits** minimum. We draw far more (≈1700 bits).
2. **Pseudorandom control.** A deterministic algorithm (here a ChaCha20-based
   generator) expands a seed into the gap indices the machine executes. A
   PRNG alone, without physical entropy, produces the *same* sequence every
   time it is seeded the same way; that is not random, however chaotic the
   motor timings look.
3. **A uniform output law.** A property of the *mechanism and algorithm*,
   not of the entropy. "Random motor timings" or "random-looking" behaviour
   does not give uniformity; only an algorithm whose output law is provably
   uniform, executed faithfully, does.

## 2.2 The algorithm the machine executes: inside-out Fisher–Yates

Cards are fed from the input deck bottom up; call them c₀ (input bottom)
… c₅₁ (input top). The output well starts empty. For i = 0 … n−1:

* the stack holds i cards and has i+1 gaps (below the bottom card, between
  any two cards, above the top card), numbered j = 0 … i from the bottom;
* draw j_i uniformly from {0, …, i}, independently of everything else;
* insert cᵢ into gap j_i.

This is the "inside-out" form of the Fisher–Yates shuffle (Fisher & Yates
1938; Durstenfeld 1964; Knuth TAOCP vol. 2, Algorithm P is the in-place
form). The insertion form is what casino elevator shufflers execute.

**[PROOF] Every permutation is reachable and equiprobable.** By induction
on i. After 0 cards the (single) empty arrangement has probability 1 = 1/0!.
Suppose that after i cards every one of the i! arrangements of {c₀…cᵢ₋₁}
has probability 1/i!. Any arrangement of i+1 cards is obtained from exactly
one arrangement of i cards (remove cᵢ) by exactly one gap choice (where cᵢ
sat), so its probability is (1/i!) × (1/(i+1)) = 1/(i+1)!. At i = n every
one of the n! orderings has probability 1/n!. ∎

Two conditions must hold for the proof to apply to the machine:

* **(C1)** each j_i is uniform on {0…i} and independent of the other j's
  and of the input (the RNG's job, §2.3);
* **(C2)** the mechanism inserts cᵢ into gap j_i, not into some other gap
  (the mechanism's job, §2.4).

Two useful corollaries:

* **[PROOF] Input independence.** The proof never uses the input order, so
  the output law is uniform for any input, including a sorted deck, a
  previously shuffled deck, or a deck disturbed by a jam. Therefore the safe
  response to any interrupted shuffle is simply to re-run it from the start.
* **[PROOF] No repeated riffles needed.** One pass suffices; there is no
  "number of passes" parameter and no mixing-time argument.

## 2.3 Random numbers: entropy, generator, sampling

Pipeline (firmware `firmware/shuffler/rng.cpp`, reference `simulation/rng_reference.py`):

| Stage | What | Why |
|---|---|---|
| Entropy source | ESP32 hardware RNG register, with `bootloader_random_enable()` called so the SAR-ADC noise source feeds it while Wi-Fi/BT are off. Espressif's documentation states the output is true-random only while the RF subsystem or this ADC source is enabled; otherwise it must be treated as pseudo-random. | The seed must be unpredictable. |
| Extra entropy | Microsecond timestamps of button presses and beam-sensor edges, ADC noise of the battery divider, boot counter, shuffle counter | Defence in depth: cheap independent noise mixed into the seed. |
| Health test | Repetition-count test on the raw hardware samples (any 32-bit value repeated 4× in a row aborts the shuffle with an error code) | Detects a stuck or disabled hardware source. This is a simplified NIST SP 800-90B style check, not a full one. |
| Conditioning | key = SHA-256(64 B hardware ‖ 32 B jitter pool ‖ boot ctr ‖ shuffle ctr) | Compresses ≥ 256 bits of raw material into a 256-bit key; small biases in the raw source do not survive hashing (assumes ≥ 256 bits of min-entropy in the material). |
| Generator | ChaCha20 keystream (RFC 8439), nonce = "SHF\x01" ‖ shuffle counter, block counter from 1 | Cryptographic-quality, fast, tiny code, identical output on host and firmware. Reseeded for every shuffle; nonce guarantees no keystream reuse even if the same key recurred. |
| Sampling | `uniform(m)`: draw a 32-bit word x; accept iff x < 2³² − (2³² mod m); return x mod m | **[PROOF]** The accepted x are uniform on a multiple of m values, so x mod m is exactly uniform. No modulo bias. Expected rejection probability ≤ 54/2³² ≈ 1.3 × 10⁻⁸ per draw. (Lemire 2019 gives a faster nearly-divisionless variant; not needed at 52 draws per shuffle.) |
| Gap sequence | j_i = uniform(i+1), i = 0…n−1 | Condition (C1). |

What this does **not** guarantee: if the ESP32's hardware source were
silently broken *and* the jitter sources were predictable, the seed would be
predictable and the sequence, though still uniform-looking, could be
reproduced by an attacker who knows the firmware. That is why the health
test exists and why the entropy is mixed from several sources. For home
poker the practical adversary is a player trying to learn something about
the next deck from the previous one; the reseeding and nonce scheme mean no
two shuffles share any generator state.

## 2.4 What the physical mechanism can get wrong (condition C2)

Each card's intended gap j_i is realised by (a) positioning the elevator so
the boundary below card j_i is level with the blades, (b) driving the
blades in, (c) dropping the lower stack 3 mm, (d) feeding the card,
(e) closing. Deviations:

| Fault | Physical cause | Effect on the gap actually used | Detectable by |
|---|---|---|---|
| **F1 blade one gap off, symmetric** | ±0.15 mm positioning noise; the bevelled blade tip self-centres into the nearest boundary, equally likely above or below | j' = j ± 1 with equal probabilities p, clipped to [0, i] | Not directly; see below (harmless) |
| **F2 blade one gap off, systematic** | Calibration offset: blade plane consistently above/below the commanded boundary | j' = j + 1 with probability p (one direction) | Calibration routine; predicted-vs-actual test (§2.8) |
| **F3 thickness-estimate error** | Stack height measured wrongly, or card thickness not uniform; error grows with j | j' = round(j · (1 + d)) | Stack-top beam re-measure every 4 cards; predicted-vs-actual test |
| **F4 blades miss** | Blade hits a card edge and pushes it instead of entering; lower stack drops, card lands on top | j' = i with probability p | Front-slot beam jam timeout catches most; a clean miss is silent → predicted-vs-actual test |
| **F5 double feed** | Two cards pass the gate together | cᵢ₊₁ enters the same gap directly above cᵢ | Card count at end of shuffle (51 counted, hopper empty) → firmware flags it |
| **F6 missed feed / retry** | Roller slips; card does not arrive; firmware retries | Same card into the same still-open gap: **no effect on the law** [PROOF: the gap index is unchanged, or a fresh uniform draw is used] | — |
| **F7 jam with manual intervention** | User clears cards | Order disturbed; firmware requires a full restart, which is uniform by input independence | — |
| **F8 lost stepper steps** | Elevator stalls | Systematic offset from then on (like F2/F3) | Re-homing each shuffle; stack-top re-measure detects a drift > 0.3 mm |

**[PROOF] Symmetric independent ±1 errors preserve exact uniformity.**
Let the realised gap be j' = clip(j + ε, 0, i) with ε ∈ {−1, 0, +1},
P(ε = +1) = P(ε = −1) = p, independent of j and of other cards. For an
interior gap m (0 < m < i): P(j' = m) = P(j = m)(1 − 2p) + P(j = m−1)p +
P(j = m+1)p = 1/(i+1). For m = 0: P(j' = 0) = P(j = 0)(1 − p) +
P(j = 1)p = 1/(i+1) (the ε = −1 case at j = 0 is clipped back to 0 and the
ε = +1 case leaves; they cancel). Symmetrically for m = i. So j' is exactly
uniform and, being independent across cards, the Fisher–Yates proof applies
unchanged. The same holds for any symmetric error distribution with
independent errors (the transition kernel is doubly stochastic). ∎

The consequence for the design: **positioning noise is harmless as long as
it is unbiased and independent of the commanded gap**. What matters is
eliminating *systematic* offsets (F2, F3, F8) and *silent* failures (F4, F5).
This is why the machine re-measures the stack height with a beam sensor
every few cards (removes accumulated thickness error), homes before every
shuffle (removes lost steps), has a calibration routine for the blade
plane (removes the constant offset), counts cards (catches F5) and has a
jam timeout on the entry beam (catches most F4).

**[SIM] Effect sizes.** 200 000 shuffles per model (20 000 for riffles),
seed 12345. Columns: chi-square p-value of the 52×52 card-position table;
largest cell deviation in σ; probability the input bottom card ends at the
output bottom (uniform: 0.0192); pairwise-precedence pairs beyond 3σ
(uniform: ≈3.6 of 1326); mean number of original successors directly
above their predecessor (uniform: 0.981); mean rising sequences (uniform:
26.5); informed next-card guessing score, correct cards per deck (uniform:
4.54).

| Model | pos-freq p | max σ | P(bot→bot) | pairs > 3σ | adjacency | rising | guess |
|---|---|---|---|---|---|---|---|
| **Exact Fisher–Yates** | 0.42 | 3.9 | 0.0189 | 1 | 0.979 | 26.51 | 4.56 |
| F1 symmetric ±1, p = 5 % each | 0.17 | 4.1 | 0.0199 | 7 | 0.980 | 26.50 | 4.52 |
| F1 symmetric ±1, p = 25 % each | 0.18 | 3.8 | 0.0192 | 4 | 0.984 | 26.49 | 4.54 |
| F2 systematic +1, p = 10 % | < 10⁻³⁰⁰ | 35.7 | **0.0302** | 248 | 0.991 | 26.41 | 4.55 |
| F2 systematic +1, p = 100 % | < 10⁻³⁰⁰ | 3194 | **1.000** | 778 | 1.135 | 25.52 | 5.64 |
| F3 thickness scale d = 3 % | < 10⁻³⁰⁰ | 62.6 | 0.0193 | 974 | 1.018 | 26.47 | 4.58 |
| F4 blade miss p = 2 % | < 10⁻³⁰⁰ | 63.0 | 0.0207 | 1020 | 1.011 | 26.42 | 4.58 |
| F5 double feed p = 2 % (undetected) | 1.3 × 10⁻⁶ | 5.4 | 0.0209 | 97 | **1.515** | 26.01 | 4.99 |
| 8-bin shelf shuffler, 1 pass | < 10⁻³⁰⁰ | 344 | 0.125 | 1326 | 6.38 | 23.3 | **17.2** |
| 8-bin shelf shuffler, 4 passes | 7.5 × 10⁻⁴ | 3.6 | 0.0189 | 1 | 0.988 | 26.50 | 4.58 |
| GSR hand riffle × 1 | < 10⁻³⁰⁰ | 503 | 0.495 | 1315 | 25.5 | 2.0 | 26.7 |
| GSR hand riffle × 7 | 3.7 × 10⁻¹² | 5.2 | 0.0242 | 150 | 1.185 | 24.7 | 4.62 |
| GSR hand riffle × 12 | 8 × 10⁻³ | 4.0 | 0.0184 | 4 | 0.976 | 26.44 | 4.55 |
| Two-hopper riffle machine × 1 | < 10⁻³⁰⁰ | 496 | 0.500 | 1304 | 10.0 | 2.0 | 12.3 |
| Two-hopper riffle machine × 7 | 2.7 × 10⁻¹⁰ | 7.1 | 0.0226 | 105 | 1.042 | 26.29 | 4.59 |

**[SIM] Total-variation distance for n = 6** (720 permutations,
10⁶ samples; the "floor" is what a perfectly uniform source shows with the
same sample size, ≈ 0.011):

| Model | TV | Floor |
|---|---|---|
| Exact Fisher–Yates | 0.0108 | 0.0110 |
| F1 symmetric ±1, 5 % | 0.0102 | 0.0109 |
| F2 systematic +1, 10 % | 0.070 | 0.011 |
| F2 systematic +1, 100 % | 0.833 | 0.011 |
| F4 blade miss 2 % | 0.033 | 0.011 |
| F5 double feed 2 % | 0.032 | 0.011 |
| 8-bin shelf, 1 pass | 0.240 | 0.011 |
| 8-bin shelf, 3 passes | 0.0116 | 0.010 |

Reading: symmetric blade noise is invisible (as proved). A 2 % rate of
silent misses or double feeds is roughly as bad as an 8-bin shelf machine
run three times, and would be caught by the card count (F5) and by the
predicted-vs-actual test (F4). A 10 % systematic offset is very bad and is
exactly what calibration removes.

## 2.5 Riffle shuffles, for comparison

The Gilbert–Shannon–Reeds model of a hand riffle (Gilbert 1955; Reeds 1981;
Aldous & Diaconis 1986) cuts the deck at Binomial(n, ½) and interleaves with
probability proportional to packet size. Bayer & Diaconis (1992, Theorem 1)
give the exact law after k riffles: P(π) = C(2ᵏ + n − r, n)/2ᵏⁿ where r is
the number of rising sequences of π, and hence the exact total-variation
distance (computed by `simulation/shuffle_sim.py --bd-table`):

| k riffles | 1–4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 |
|---|---|---|---|---|---|---|---|---|---|
| TV distance (n = 52) | 1.000 | 0.924 | 0.614 | **0.334** | 0.167 | 0.085 | 0.043 | 0.021 | 0.011 |

An N-bin "shelf" pass (each card to a uniform bin, bins stacked in order)
is the inverse of an N-shuffle, and k passes compose to an Nᵏ-shuffle
(Bayer–Diaconis, Lemma 1 on composition of a- and b-shuffles), so the same
formula gives e.g. 8 bins × 4 passes → TV 0.011, 10 bins × 2 passes →
0.42. Diaconis, Fulman & Holmes (2013) analysed a real 10-shelf casino
machine, showed one pass leaves enough structure for a player to guess the
next card far above chance (their guessing strategy is the model for the
"guess" column above), and the manufacturer adopted a second pass.

**How real machines differ from the models.** A consumer two-hopper riffle
machine does not follow GSR: its cut is always close to 26 and the two
rollers alternate almost perfectly (run lengths mostly 1). After one pass
the deck has exactly 2 rising sequences, and the mechanical model above is
detectably non-uniform even after 7 passes. Real hand riffles also differ
from GSR (people tend toward perfect interleaving in the middle and
clumps at the ends). The insertion mechanism chosen here has no
"mixing model" to argue about: its ideal output is uniform after one pass,
and its errors are the concrete faults in §2.4.

## 2.6 What the statistical tests can and cannot establish

* A test on B shuffles can only detect deviations larger than roughly
  1/√B per cell. With 200 000 shuffles the position-frequency test sees
  cell probabilities to ±0.0005; it cannot see a deviation of 10⁻⁴.
* No finite battery of tests proves uniformity over 8 × 10⁶⁷ outcomes.
  Passing tests establishes "no bias of the tested kinds larger than the
  resolution". That is why the argument for this machine is layered:
  (1) the algorithm is uniform **[PROOF]**; (2) the RNG and sampler can be
  checked bit-for-bit against the reference and statistically **[SIM/PHYS
  via serial dump]**; (3) each physical step is confirmed by a sensor, and
  the residual physical error can be *measured directly* by comparing the
  predicted permutation with the actual one (§2.8), which is far more
  powerful than testing output statistics.
* Small-n exact TV distance (n = 6) *is* a complete measure, but only for
  the model with 6 cards; effects that scale with n (F3) are
  under-represented there.

## 2.7 Simulation code

`simulation/shuffle_sim.py` — models and tests (see the file header).
`simulation/rng_reference.py` — bit-exact reference of the firmware RNG.
`simulation/analyze_physical.py` — runs the same tests on recorded physical
outputs, and does the predicted-vs-actual comparison.

Run `python3 shuffle_sim.py --quick` (≈1 min) or without `--quick`
(≈10 min). The tests implemented: card-position frequency (chi-square,
df 2601), bottom-card and top-card identity, pairwise precedence (1326
pairs), original-neighbour adjacency, rising sequences (Eulerian
expectation (n+1)/2 = 26.5, sd √((n+1)/12) = 2.10), serial correlation of
consecutive cards' output positions (expected −1/(n−1)), Spearman
correlation between input and output position, and the informed next-card
guessing score.

## 2.8 Validation plan

Ordered from strongest to weakest evidence; do them in this order.

1. **RNG + sampler, bit-exact [SIM/PHYS].** Firmware TEST mode prints the
   derived key, shuffle counter and the 52 gap indices. `rng_reference.py
   --key … --counter …` must print the identical sequence. This checks the
   ChaCha20 and rejection-sampling implementation on the real chip.
2. **RNG statistics on the chip [PHYS].** Firmware TEST mode streams
   100 000 gap sequences over USB serial (a few minutes). Feed them to
   `shuffle_sim.py` through `build_order`/`run_all_stats` (the
   `analyze_physical.py --gaps` option). Expected: every statistic within
   the uniform reference ranges above. This validates (C1) on hardware.
3. **Entropy health [PHYS].** Firmware TEST mode dumps 1 MB of raw hardware
   RNG bytes; run `ent` or NIST SP 800-22 on it. Expected ≈ 8.0 bits/byte.
   Also confirm `bootloader_random_enable()` is active (the firmware checks
   the register and refuses to shuffle otherwise).
4. **Mechanism accuracy, predicted vs actual [PHYS].** Use a deck with
   visible indices. TEST mode "fixed seed" runs a shuffle with a known gap
   sequence and prints the *predicted* output order. Record the actual
   output order (`analyze_physical.py --enter`). Every discrepancy is a
   mechanical error; the script classifies it (off-by-one up/down, miss to
   top, double feed, other). 20 shuffles = 1040 insertions, giving the error
   rate to about ±0.3 % and, importantly, its *direction*. Acceptance
   target **[EST]**: ≤ 1 % off-by-one with no directional bias beyond
   sampling noise (two-sided binomial test, p > 0.05), 0 misses, 0 double
   feeds in 1040 insertions. Directional bias → recalibrate blade offset.
5. **Output statistics on physical shuffles [PHYS].** 200–500 shuffles of
   a real deck, entered by hand (or photographed). At this sample size only
   gross faults are visible (e.g. P(bottom→bottom) = 0.03 vs 0.019 needs
   ~600 shuffles for 2σ). Use it as a sanity check, not as the proof.
6. **Different starting orders [PHYS].** Repeat steps 4–5 from a sorted
   deck, a reversed deck and a previously shuffled deck. Input
   independence is proved for the algorithm; this checks that no mechanical
   effect depends on input order (e.g. new decks feed differently from
   worn ones).
7. **Fault injection [PHYS].** Deliberately set the gate to 0.8 mm to
   provoke double feeds and confirm the count check flags them; loosen the
   blade calibration by +0.3 mm and confirm the predicted-vs-actual test
   reports a directional bias.

**Approximation target [EST].** With the acceptance criteria of step 4, and
with symmetric residual errors, the output law is within total-variation
distance ≈ 0.01 of uniform (comparable to 12 hand riffles), dominated by
undetected asymmetry at the 1 % level. This is a target to be validated,
not a claim.

## References

* R. A. Fisher, F. Yates, *Statistical Tables for Biological, Agricultural
  and Medical Research*, 1938 (example 12).
* R. Durstenfeld, "Algorithm 235: Random permutation", *CACM* 7(7), 1964.
* D. E. Knuth, *The Art of Computer Programming* vol. 2, §3.4.2, Algorithm P.
* E. N. Gilbert, "Theory of shuffling", Bell Labs technical memorandum, 1955.
* D. Aldous, P. Diaconis, "Shuffling cards and stopping times", *American
  Mathematical Monthly* 93(5), 1986, pp. 333–348.
* D. Bayer, P. Diaconis, "Trailing the dovetail shuffle to its lair",
  *Annals of Applied Probability* 2(2), 1992, pp. 294–313.
* P. Diaconis, J. Fulman, S. Holmes, "Analysis of casino shelf shuffling
  machines", *Annals of Applied Probability* 23(4), 2013, pp. 1692–1720.
* L. N. Trefethen, L. M. Trefethen, "How many shuffles to randomize a deck
  of cards?", *Proc. R. Soc. A* 456, 2000, pp. 2561–2568.
* D. Lemire, "Fast random integer generation in an interval", *ACM TOMACS*
  29(1), 2019.
* Y. Nir, A. Langley, "ChaCha20 and Poly1305 for IETF Protocols", RFC 8439,
  2018.
* Espressif, ESP-IDF Programming Guide, "Random Number Generation"
  (`esp_random`, `bootloader_random_enable`).
* NIST SP 800-90B, "Recommendation for the Entropy Sources Used for Random
  Bit Generation", 2018 (health tests, §4.4).
