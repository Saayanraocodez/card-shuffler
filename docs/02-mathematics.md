# 2. Mathematics of the shuffle

Statements are labelled **[PROOF]** (holds for the idealised model), **[SIM]**
(Monte-Carlo, `simulation/shuffle_sim.py`, results in `simulation/results/`),
**[EST]** (engineering estimate) or **[PHYS]** (measured on hardware —
**none yet; the machine has not been built**).

## 2.1 What "random" has to mean here

The output is an ordering π of the deck, one of n! (52! ≈ 8.07 × 10⁶⁷).
The goal is **uniformity**: P(output = π) = 1/n! for every π, independent of
the input order and of anything a player can observe. Three different
things are needed and must not be confused:

1. **Physical entropy** — unpredictable bits (≥ log₂ 52! = 225.6 per deck).
2. **Pseudorandom control** — a deterministic generator expanding a seed
   into the slot choices. Without a fresh physical seed it repeats.
3. **A uniform output law** — a property of algorithm + mechanism. Random
   motor timings do not create it; only an algorithm whose output law is
   provably uniform, executed faithfully, does.

## 2.2 The algorithm the wheel executes: random empty-slot assignment

The wheel has N = 54 slots. Cards are fed in input order c₀ (bottom) …
c₅₁. For card i the firmware draws a uniformly random slot among the slots
it knows to be empty and places the card there. At the end the slots are
emptied in increasing slot number into the output chute, first out at the
bottom.

**[PROOF] Every ordering is reachable and equiprobable.** The assignment
is an injective map from the n cards to the N slots. At step i there are
N − i empty slots, each chosen with probability 1/(N − i), so every
injective map has probability 1/(N(N−1)…(N−n+1)) = (N−n)!/N!. Each output
ordering π corresponds to exactly C(N, n) injective maps (choose which n
slots are used; the ordering then fixes which card goes to which of them).
Hence P(π) = C(N, n) · (N−n)!/N! = 1/n!. ∎ (For N = n this is Fisher–Yates
in its original "draw from the remaining set" form; Fisher & Yates 1938,
Durstenfeld 1964, Knuth TAOCP vol. 2 §3.4.2.)

Conditions the machine must satisfy:

* **(C1)** each draw is uniform on the currently empty set and independent
  of everything else (the RNG's job, §2.3);
* **(C2)** the card ends up in the chosen slot, and the unload delivers the
  slots in order (the mechanism's job, §2.4).

Corollaries: **[PROOF]** the law is uniform for *any* input order, so a
restart after a jam is as good as a fresh deck; one pass suffices; the two
unused slots may be anywhere.

## 2.3 Random numbers

Pipeline (`firmware/shuffler/rng.cpp`; bit-exact reference in
`simulation/rng_reference.py`; host test in `firmware/test_host`):

| Stage | What |
|---|---|
| Entropy | ESP32 hardware RNG with `bootloader_random_enable()` (SAR-ADC noise source) enabled during seeding; Espressif documents the output as true-random only while RF or this source is enabled. Plus timing jitter of sensor edges and button presses, ADC noise, boot and shuffle counters. |
| Health test | Repetition-count test on the raw 32-bit samples (4 identical in a row → abort with ERROR 5). Simplified NIST SP 800-90B §4.4. |
| Conditioning | key = SHA-256(64 B hardware ‖ 32 B jitter ‖ boot counter ‖ shuffle counter). |
| Generator | ChaCha20 keystream (RFC 8439); nonce = "SHF\x01" ‖ shuffle counter; block counter from 1. Reseeded per shuffle; no keystream reuse. |
| Sampling | `uniform(m)`: draw a 32-bit word x, accept iff x < 2³² − (2³² mod m), return x mod m. **[PROOF]** exact, no modulo bias; rejection probability ≤ 54/2³². (Lemire 2019 gives a faster variant, unnecessary here.) |
| Assignment | `slots_assign`: empty = [0..53]; for each card idx = uniform(len), slot = empty[idx], swap-remove. The swap changes the list order, not the uniformity of the choice. |

`test ref` on the ESP32 must print the same 52 slots as
`rng_reference.py --selftest`; `firmware/test_host` verifies the same
sequence and the predicted output order on the host.

## 2.4 What the mechanism can get wrong (condition C2)

| Fault | Cause | Effect on the realised assignment | Detection |
|---|---|---|---|
| **W1 neighbour slot, symmetric** | card enters the slot one above or below the target, equally likely (wheel index noise ±1 mm at the mouth, card tilt) | realised slot = intended ± 1 when that slot is empty | Beam E at the entry: after each feed the card must be seen in the target slot; if not, the firmware looks at both neighbours, finds the card and **corrects its occupancy map** (`VERIFY_AFTER_INSERT`), logging the direction |
| **W2 neighbour slot, biased** | entry plane trim wrong (always the same side) | realised = intended + 1 always when empty | As W1; the *direction statistics* of the corrections reveal it; `cal entry` trim |
| **W3 uncorrected W1/W2** | firmware not verifying | later card sent to an actually occupied slot: collision (jam) or deflection to a free neighbour | Only by the final count or a jam |
| **W4 double feed** | two cards through the gate | both cards in one slot, original order kept → adjacent in the output | Beam-B block-time heuristic; end count (52 loaded); the slot holds two cards, the exit pulls them together |
| **W5 card lost / not seated** | card stops in the mouth, falls back | beam E clear after the feed and not found in the neighbours → ERROR 8 | Sensor |
| **W6 eject failure** | card does not come out of a slot | beam X never blocks → retry, then ERROR 9; the deck is short | Sensor + count |
| **W7 missed feed / retry** | roller slips | same card into the same (still empty, still chosen) slot: **no effect on the law** [PROOF] | — |
| **W8 lost steps / wrong home** | stepper stall, index misread | all subsequent slots offset by a constant → still an injective assignment: the *output order* is unchanged (slot k+1 for all cards is the same order) unless the offset crosses the two empty slots or the wheel end; unload uses the same offset | Homing before every shuffle; the start-of-run scan |

**[PROOF] Symmetric, independent neighbour errors with map correction
preserve uniformity.** Let E be the set of truly empty slots when card i is
placed, s uniform on E, and the realised slot s' = s ± 1 with equal
probability p if that neighbour is in E, else s' = s. The kernel K(s → s')
on E is symmetric (K(a→b) = K(b→a)) and hence doubly stochastic, so s' is
uniform on E. With the map corrected, the next draw is uniform on the true
remaining set, and the argument of §2.2 applies to the realised assignment.
∎ Without correction (W3) the firmware's belief and the truth diverge and
uniformity is lost (see the table below). A biased kernel is not doubly
stochastic and produces a bias whether or not the map is corrected.

**[SIM] Effect sizes**, 200 000 shuffles per model (20 000 for riffles).
Columns: chi-square p of the 52 × 52 card-position table; largest cell in
σ; P(input bottom card ends at the bottom) (uniform 0.0192); pairwise
precedence pairs beyond 3σ (uniform ≈ 3.6 of 1326); mean original
successors directly above (uniform 0.981); Spearman z of input vs output
position; informed next-card guessing score (uniform 4.54).

| Model | pos-freq p | max σ | P(bot→bot) | pairs > 3σ | adjacency | Spearman z | guess |
|---|---|---|---|---|---|---|---|
| **Wheel, exact** | 0.72 | 3.4 | 0.0189 | 2 | 0.980 | 0.5 | 4.55 |
| W3 neighbour 5 % symmetric, **uncorrected** | 1.3 × 10⁻⁶ | 3.6 | 0.0189 | 48 | 1.005 | −6.4 | 4.57 |
| W3 neighbour 5 % biased, uncorrected | 2.6 × 10⁻⁴⁰ | 6.5 | 0.0181 | 308 | 0.980 | −17.3 | 4.53 |
| W3 neighbour 30 % symmetric, uncorrected | < 10⁻³⁰⁰ | 14.2 | 0.0191 | 613 | 1.136 | −39.2 | 4.81 |
| W1 neighbour 5 % symmetric, **corrected** (firmware default) | 0.16 | 3.6 | 0.0188 | 5 | 0.982 | 0.4 | 4.54 |
| W2 neighbour 5 % biased, corrected | 2.8 × 10⁻⁷ | 4.4 | 0.0179 | 34 | 0.935 | −5.5 | 4.60 |
| W2 neighbour 30 % biased, corrected | < 10⁻³⁰⁰ | 20.9 | 0.0133 | 1278 | 0.713 | −35.1 | 5.29 |
| W4 double feed 2 % (v1 model, same effect) | 5.8 × 10⁻⁷ | 5.9 | 0.0210 | 85 | **1.515** | 1.4 | 5.01 |
| 8-bin shelf shuffler, 1 pass | < 10⁻³⁰⁰ | 347 | 0.126 | 1326 | 6.39 | 403 | **17.1** |
| 8-bin shelf shuffler, 4 passes | 0.40 | 3.3 | 0.0196 | 3 | 0.989 | −0.6 | 4.55 |
| GSR hand riffle × 7 | 8.6 × 10⁻¹³ | 5.9 | 0.0242 | 151 | 1.197 | 7.5 | 4.64 |
| GSR hand riffle × 12 | 0.96 | 3.2 | 0.0197 | 5 | 0.981 | 1.3 | 4.54 |
| Two-hopper riffle machine × 7 | 1.6 × 10⁻⁹ | 7.2 | 0.0240 | 116 | 1.049 | 6.1 | 4.63 |

**[SIM] Exact total-variation distance for 6 cards** (wheel models with 8
slots, 10⁶ samples; the floor is a perfect source at the same sample size):

| Model | TV | floor |
|---|---|---|
| Wheel, exact | 0.0103 | 0.0110 |
| W3 neighbour 5 % symmetric, uncorrected | 0.0134 | 0.0109 |
| W3 neighbour 5 % biased, uncorrected | 0.0166 | 0.0111 |
| W1 neighbour 5 % symmetric, corrected | 0.0108 | 0.0110 |
| W2 neighbour 5 % biased, corrected | 0.0118 | 0.0109 |
| 8-bin shelf, 1 pass | 0.240 | 0.011 |

Reading: the wheel's only silent failure mode is a misplaced card the
firmware does not notice, which is exactly what beam E and the correction
routine are for. Every correction is logged with its direction, so a biased
entry plane shows up in the machine's own statistics long before it could
be seen in output statistics.

## 2.5 Riffle shuffles, for comparison

The Gilbert–Shannon–Reeds model (Gilbert 1955; Reeds 1981; Aldous &
Diaconis 1986) and its exact analysis by Bayer & Diaconis (1992, Theorem 1:
P(π) = C(2ᵏ + n − r, n)/2ᵏⁿ with r the number of rising sequences) give the
total-variation distance after k riffles of 52 cards
(`shuffle_sim.py --bd-table`):

| k | 1–4 | 5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 |
|---|---|---|---|---|---|---|---|---|---|
| TV | 1.000 | 0.924 | 0.614 | **0.334** | 0.167 | 0.085 | 0.043 | 0.021 | 0.011 |

An N-bin shelf pass is the inverse of an N-riffle and k passes compose to
an Nᵏ-shuffle, so 8 bins × 4 passes ≈ 12 riffles. Diaconis, Fulman & Holmes
(2013) showed a real 10-shelf casino machine is exploitable after one pass.
Consumer two-hopper riffle machines are worse than GSR per pass (near-
perfect alternation, cut near 26). The wheel has no mixing-time question:
its ideal output is uniform after one pass, and its faults are the concrete
items in §2.4.

## 2.6 A second exact architecture (kept for reference): two wells

If cards are diverted to two insertion wells A and B, each performing
uniform random-gap insertion, and stack A is placed on stack B at the end,
the result is exactly uniform whenever the label vector L ∈ {A,B}ⁿ is
*exchangeable* (P(L) depends only on the number of A's), e.g. i.i.d. coin
flips of any bias or a uniformly random 26/26 split. **[PROOF]** For a
target π and each k, the event "top k of π = the A cards in π's order" has
probability f(k)/(k!(n−k)!); summing over k gives
(1/n!) Σₖ f(k) C(n, k) = (1/n!) Σ_L P(L) = 1/n!. ∎ This was the alternative
to the wheel; it is slower and needs the user to combine two stacks.

## 2.7 What the tests can and cannot establish

* B shuffles resolve deviations of about 1/√B per cell; 200 000 shuffles
  see cell probabilities to ±0.0005. No finite test proves uniformity over
  10⁶⁸ outcomes.
* The argument for this machine is therefore layered: (1) the algorithm is
  uniform **[PROOF]**; (2) the RNG and sampler are checked bit-for-bit
  against the reference and statistically on the chip; (3) every insertion
  is verified by a sensor and every deviation is logged with its direction,
  so the residual physical error is *measured*, not assumed; (4) the
  predicted-vs-actual test checks the whole chain including the unload.

## 2.8 Validation plan (strongest evidence first)

1. **RNG bit-exact** — `test ref` = `rng_reference.py --selftest` ★
2. **RNG statistics on the chip** — `test rng 100000` →
   `analyze_physical.py --gaps` (slot sequences; every statistic in the
   uniform ranges) ★
3. **Entropy health** — `test raw 1000000` → `ent` / NIST STS ≈ 8.0 bits/byte ★
4. **Insertion accuracy from the machine's own log** — after every shuffle
   `last` prints intended and realised slots. Over 20 shuffles (1040
   insertions) the acceptance target **[EST]** is ≤ 1 % corrections with no
   directional excess beyond the binomial test (p > 0.05), 0 lost cards,
   0 double-feed suspects. `analyze_physical.py --log` tallies this. ★
5. **Predicted-vs-actual** — `test fixed <seed>`, press the button, then
   read the output deck (`analyze_physical.py --compare`). Every
   discrepancy is a mechanical error not caught by the sensors (eject
   order, unnoticed misplacement, double card). 20 decks. ★
6. **Output statistics on physical shuffles** — 200–500 decks entered by
   hand; only gross faults are visible at this size; sanity check.
7. **Different starting orders** — sorted, reversed, previously shuffled:
   5 decks each of step 5. Input independence is proved for the algorithm;
   this checks the mechanics do not depend on order (new vs worn decks).
8. **Fault injection** — misalign `cal entry` by +1.5° and confirm the
   correction log shows a directional excess; set the gate to 0.8 mm and
   confirm the double-feed warning.

**Approximation target [EST].** Under the step-4 acceptance criteria, with
symmetric residual errors and map correction, the output law is exactly
uniform by §2.4; the residual risk is an undetected directional bias at
the 1 % level, which corresponds to a total-variation distance of order
0.01 (comparable to 12 hand riffles). To be validated, not claimed.

## References

* R. A. Fisher, F. Yates, *Statistical Tables*, 1938, example 12.
* R. Durstenfeld, "Algorithm 235: Random permutation", *CACM* 7(7), 1964.
* D. E. Knuth, *TAOCP* vol. 2, §3.4.2.
* E. N. Gilbert, "Theory of shuffling", Bell Labs memo, 1955.
* D. Aldous, P. Diaconis, "Shuffling cards and stopping times", *Amer.
  Math. Monthly* 93(5), 1986.
* D. Bayer, P. Diaconis, "Trailing the dovetail shuffle to its lair",
  *Ann. Appl. Probab.* 2(2), 1992.
* P. Diaconis, J. Fulman, S. Holmes, "Analysis of casino shelf shuffling
  machines", *Ann. Appl. Probab.* 23(4), 2013.
* L. N. Trefethen, L. M. Trefethen, "How many shuffles to randomize a deck
  of cards?", *Proc. R. Soc. A* 456, 2000.
* D. Lemire, "Fast random integer generation in an interval", *ACM TOMACS*
  29(1), 2019.
* RFC 8439 (ChaCha20); Espressif ESP-IDF "Random Number Generation";
  NIST SP 800-90B §4.4.
