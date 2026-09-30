# 1. Mechanism selection

This document compares the feasible ways to shuffle one 52–54 card poker deck
with a machine a hobbyist can print and wire, and explains why this project
uses **single-card random-gap insertion** (an elevator well with separator
blades, executing the inside-out Fisher–Yates algorithm physically).

Everything here is a design analysis. Nothing in this repository has been
built and measured yet; the speed and reliability numbers are engineering
estimates and are labelled as such.

## 1.1 Requirements recap

| Requirement | Target | Notes |
|---|---|---|
| Deck | 52 cards, 54 if practical | Poker size 63.5 × 88.9 mm |
| Card thickness | 0.25–0.35 mm standard; 0.22–0.42 mm accepted | Custom printed decks tend to be thicker |
| Operation | Load, one button, collect | Minimal effort |
| Cycle time | < 30 s target, honest estimate required | See §1.6 |
| Randomness | Measurably close to uniform on all 52! orders | See `02-mathematics.md` |
| Card safety | No scratches, bent corners, exposed faces | |
| Power | Battery, rechargeable, protected pack | |
| Cost | < $130 excluding printing | |
| Build | Consumer FDM printer, hand tools, no machining | |

## 1.2 Candidate mechanisms

### A. Automated riffle (two hoppers, alternating feed)

This is the $20 consumer shuffler. The deck is split into two hoppers; a
roller on each hopper flicks cards into a centre tray, alternating between
the two sides.

* **Randomness.** The mathematical model for a hand riffle is the
  Gilbert–Shannon–Reeds (GSR) shuffle, which needs about 7 riffles for a
  52-card deck (Bayer & Diaconis 1992). Consumer riffle machines are far
  *less* random than GSR per pass, because the two rollers alternate almost
  perfectly (run lengths of 1–2 cards) and the cut is always near 26/26. A
  one-pass machine riffle leaves the deck with at most 2 rising sequences;
  the number of reachable permutations after one pass is tiny (≈2^52 out of
  8×10^67, and the actual distribution is much narrower than that).
* **To fix it** you must re-split and re-riffle 8–12 times. Automating the
  re-split requires a second mechanism that cuts a stack at a random depth
  and moves half of it, which is the hard part of any shuffler. Without it,
  the user has to split the deck by hand every pass, which fails the
  "one button" requirement.
* **Speed** is excellent per pass (3–5 s). **Card wear** is moderate: cards
  are bent as they are flicked. **Noise** is high. **Cost** is low.

### B. Random-bin routing ("shelf shuffler")

One feeder takes cards off the deck one at a time and a diverter drops each
card into one of N bins chosen at random. The bins are then stacked in a
fixed order. This is the casino "shelf shuffler" analysed by
Diaconis, Fulman & Holmes (2013).

* **Randomness.** One pass with N bins, each card going to a uniformly
  chosen bin and each bin keeping arrival order, is the inverse of an
  N-riffle (an "a-shuffle" with a = N in Bayer–Diaconis terms). Its distance
  from uniform is exactly that of an N-riffle: with N = 8 one pass is
  roughly as good as 3 hand riffles, which is *not* enough; four passes
  (8^4 = 4096-shuffle) reach total-variation distance ≈ 0.01. Diaconis,
  Fulman & Holmes showed that a real 10-shelf machine after one pass lets a
  player guess the next card far better than chance, and the manufacturer
  changed the machine to run two passes.
* **Mechanics.** A moving diverter or a moving bin stack, plus a way to
  recombine the bins. Recombining automatically means every bin needs its
  own feeder, or the bins must be re-fed through the machine (user
  intervention). Multi-pass makes a slow machine.
* Speed per pass ~15–20 s; 3–4 passes plus re-feeding: 60–80 s with user
  intervention.

### C. Random-slot carousel (one slot per card)

A rotating drum with 54 slots. Each card is fed into a random empty slot,
then the slots are ejected in order. Assigning each card to a uniformly
random free slot is a uniform permutation (it *is* Fisher–Yates).

* **Randomness.** Exact, in principle.
* **Size.** 54 cards standing radially need a drum of about Ø190–240 mm and
  95 mm height, plus a feeder, plus an output tray with a spring follower.
  This is the least compact option by far.
* **Mechanics.** Two positioning moves and two card pushes per card
  (in and out), so ~100 s per deck. Thin radial fins are printable but
  fragile. Ejecting a card standing on edge into a neat stack needs a
  spring-loaded output shoe.

### D. Single-card random-gap insertion (elevator well) — **selected**

One feeder takes the bottom card off the deck. The card is pushed into a
vertical well whose stack rests on an elevator. Before each card, the
elevator moves the stack so that a randomly chosen gap (between card k and
k+1, or below the bottom card, or above the top card) is level with a pair
of thin separator blades. The blades slide in from the sides, the elevator
drops the lower part of the stack by 3 mm, the new card slides into the gap,
the elevator closes the gap and the blades withdraw.

This is the architecture of the casino single-deck shufflers described in
Shuffle Master's elevator-insertion patents (see references) and of the
Shuffle Tech ST-1000 home unit (~$500), scaled down to hobby parts.

* **Randomness.** Inserting card i (i = 0..n−1) into a uniformly random one
  of the i+1 gaps of the current stack is the inside-out Fisher–Yates
  algorithm. Every permutation is reachable and, if each gap index is
  uniform and independent and the mechanism places the card where told,
  every permutation has probability exactly 1/52!. The physical error modes
  (a blade landing one gap off, a double feed) perturb this and are
  analysed and simulated in `02-mathematics.md`.
* **Mechanics.** One friction feeder, one transport roller, one lead-screw
  elevator (stepper), one micro servo for the blades. No card is ever bent
  or flicked. Cards are pushed edge-first on a flat guide.
* **Speed.** ~1.0–1.4 s per card, so 55–75 s per deck (estimate, §1.6).
* **Card wear.** Lowest of the options: rolling contact only, no bending,
  no flicking. The blades touch card edges only.
* **Noise.** Low (stepper hum, small DC motors, servo clicks).
* **Cost.** ~$100–115 in parts (see `05-bom.md`).
* **Complexity.** Medium. The two precision requirements are (1) feeding
  exactly one card and (2) positioning the elevator to ±0.15 mm so a blade
  enters the intended gap. Both are addressed with sensors and calibration
  (see `03-mechanical-design.md` §3.7 and `06-assembly.md` §6.9).

### E. Variants considered and rejected

* **Insertion without blades** (push the card into the stack edge and let
  it wedge its own gap): a 0.3 mm card buckles at ~0.5 N; lifting 40 cards
  plus edge friction needs more than that, and card edges are not aligned
  to 0.3 mm anyway. Not reliable.
* **Clamp the upper stack from the sides and drop the lower stack**: works
  in principle but the boundary between "held" and "dropped" cards is fuzzy
  (recessed cards are not clamped), so a card can be held on one side and
  dropped on the other and jam. Blades give a defined support surface.
* **Passive one-way blades** (cards ride up over them): cards can never
  come back down, so the gap position cannot be chosen freely. Active
  (servo) blades are required.
* **Two steppers or a belt elevator**: a lead screw is self-locking, so the
  stepper can be de-energised between moves, which saves battery.

## 1.3 Scoring

Scores 1 (poor) to 5 (excellent). These are judgement scores, not
measurements.

| Criterion | A. Riffle machine | B. Bin routing | C. Carousel | **D. Gap insertion** |
|---|---|---|---|---|
| Randomness per button press | 1 | 2 (3 with 4 auto passes) | 5 | **5** (exact algorithm; physical error modes bounded) |
| Speed | 5 | 3 | 2 | **3** |
| Cost | 5 | 3 | 2 | **3** |
| Size | 4 | 3 | 1 | **4** |
| Mechanical complexity | 4 | 2 | 2 | **3** |
| Card wear | 2 | 4 | 3 | **5** |
| Noise | 1 | 3 | 3 | **4** |
| Reliability (estimated) | 3 | 3 | 2 | **3** |
| One-button operation | 1 (needs re-splits) | 2 | 5 | **5** |
| **Weighted total** (randomness ×3, one-button ×2, others ×1) | 27 | 32 | 40 | **50** |

## 1.4 Selected architecture

```
                 lid + follower weight
     ┌────────────────────┐
     │  HOPPER (deck,     │  gate (adjustable 0.3–0.6 mm slot)
     │  face down)        │  │       idler
     │                    │  ▼        ▼    ┌──────────────────┐
     └──────┬──────┬──────┘ ═══▶ ═══════▶ │  WELL            │
        feed roller  transport roller     │  ┌────────────┐  │
        (N20 motor)  (N20 motor)          │  │ upper stack│◀─┼─ separator blades (servo)
                                          │  ├────────────┤  │   3 mm gap ← new card
   whole card path tilted 12°             │  │ lower stack│  │
   (cards slide to the back wall)         │  └────────────┘  │
                                          │   elevator platform (T8 lead screw + NEMA 17)
                                          └──────────────────┘
```

* **Feeder module**: hopper with weighted lid, bottom-feed friction roller,
  adjustable separation gate, transport roller with spring idler, gate
  beam sensor, hopper-empty beam sensor. Two N20 gear motors.
* **Well module**: card well, elevator (T8×8 lead screw, two 8 mm guide
  rods, NEMA 17), two separator-blade bars driven by one MG90S servo through
  a Scotch-yoke disc, well-slot beam sensor (card fully inserted), stack-top
  beam sensor (stack height measurement), bottom endstop switch.
* **Controller**: ESP32 DevKitC. Hardware RNG (ADC-noise entropy source
  enabled explicitly) → ChaCha20 DRBG → rejection sampling for unbiased
  gap indices. TMC2208 stepper driver, DRV8833 dual DC driver.
* **Power**: 3S Li-ion pack with BMS, 12.6 V barrel charger (USB-C PD
  option documented).

## 1.5 Why the 12° tilt

The whole card path (hopper floor, guide plate, well axis) is tilted 12°
so that the well is the low end. Three benefits:

1. The transport roller releases the card about 13 mm before its leading
   edge reaches the back wall. On a level guide (μ ≈ 0.3 card-on-card)
   a card leaving at 0.25 m/s coasts only ~10 mm. At 12° the net
   deceleration drops to about 0.85 m/s² and the card coasts > 35 mm, so
   every card seats fully against the padded back wall.
2. Cards in the well settle against the back wall under gravity, which
   keeps the stack's edges aligned. Aligned edges are what lets a 0.5 mm
   blade enter a chosen gap reliably.
3. The deck in the hopper slides against the gate, so feeding does not
   depend on the user pushing the deck forward.

## 1.6 Cycle time: honest estimate

Per card, sequential operations (estimates from actuator specs):

| Step | Actuator | Estimate |
|---|---|---|
| Elevator to random gap (avg |Δz| ≈ 5 mm at 25–40 mm/s plus accel) | NEMA 17, T8×8 | 0.25 s |
| Blades in (≈120° of servo travel) | MG90S (0.1 s/60°) | 0.20 s |
| Open gap (3 mm) | stepper | 0.10 s |
| Feed + transport (≈105 mm at 0.2–0.25 m/s, includes pickup) | 2 × N20 | 0.50 s |
| Close gap (2.2 mm), blades out, settle (0.5 mm) | stepper + servo | 0.35 s |
| **Total per card** | | **≈1.4 s** |
| **52 cards** | | **≈ 73 s** |
| Plus homing, stack measurements (every 4 cards), present deck | | ≈ 6 s |

Expected first-build result: **70–80 s per deck**. With tuning (faster servo,
40 mm/s elevator, overlapping the next card's pickup with the blade retract)
the realistic floor for this architecture with these parts is about
**45–50 s**. **The 30 s target is not achievable with a single-insertion
architecture and hobby actuators**; reaching it would need either two
parallel insertion wells or a fundamentally faster (and less random per
pass) riffle mechanism. This is stated up front rather than promised.

For comparison, the commercial single-deck insertion shufflers run
30–40 s per deck with industrial actuators, and the Shuffle Tech ST-1000
takes 80–100 s (it performs 7 riffles and a strip).

## 1.7 Tradeoffs accepted

* Slower than a riffle machine, in exchange for an output distribution that
  is provably uniform when the mechanism does what it is told, and whose
  failure modes are individually detectable by sensors.
* Two precision requirements (single-card feed, ±0.15 mm elevator
  positioning) that need calibration on each build. Both have sensor-based
  auto-calibration routines in the firmware, plus manual fallbacks.
* A stepper, a servo and two DC motors instead of one motor: more wiring,
  but each motion is independently controllable and testable, which makes
  the staged prototype plan (`09-prototype-plan.md`) possible.

## References

* D. Bayer, P. Diaconis, "Trailing the dovetail shuffle to its lair",
  *Annals of Applied Probability* 2(2), 1992, pp. 294–313.
  https://projecteuclid.org/euclid.aoap/1177005705
* P. Diaconis, J. Fulman, S. Holmes, "Analysis of casino shelf shuffling
  machines", *Annals of Applied Probability* 23(4), 2013, pp. 1692–1720.
  https://projecteuclid.org/euclid.aoap/1371834042
* Shuffle Master / SG Gaming elevator-insertion patents, e.g. US 8,538,155,
  US 9,387,390, US 10,576,363 ("Card shuffling apparatus and card handling
  device": elevator positions the stack so a card can be inserted at a
  randomly selected position).
* Shuffle Tech ST-1000 timing: user reports of 80–100 s per deck
  (pokerchipforum.com threads on home shufflers).
