# 1. Mechanism selection

This document compares the feasible ways to shuffle one 52–54 card poker
deck with a machine a hobbyist can print and wire, and explains why this
project uses a **54-slot wheel with random slot assignment** (a horizontal-
axis "rolodex" into which each card is pushed at a uniformly random empty
slot, then emptied in slot order). The first iteration of this project used
elevator-insertion with separator blades; it is kept in
`archive/v1-elevator-insertion/` and is compared below.

Everything here is a design analysis. Nothing has been built; the speed
and reliability numbers are engineering estimates and are labelled as such.

## 1.1 Requirements

| Requirement | Target |
|---|---|
| Deck | 52 cards, 54 if practical (poker size 63.5 × 88.9 mm) |
| Card thickness | 0.25–0.35 mm standard; 0.22–0.42 mm accepted (custom decks) |
| Operation | Load, one button, collect |
| Cycle time | < 30 s target; honest estimate required |
| Randomness | Measurably close to uniform over all 52! orders |
| Card safety | No scratches, bent corners, exposed faces |
| Power | Rechargeable protected battery |
| Cost | < $130 excluding printing |
| Size | Footprint under 12 × 12 in (305 × 305 mm); noise not a concern |

## 1.2 Candidates

**A. Two-hopper riffle machine** (the $20 consumer shuffler). Fast per pass
(3–5 s) but a machine riffle is far from the Gilbert–Shannon–Reeds model:
the cut is always near 26/26 and the rollers alternate almost perfectly, so
one pass leaves the deck with exactly 2 rising sequences. Seven or more
passes with manual re-splitting are needed; automating the re-split is the
hard part of any shuffler. Fails "one button".

**B. Random-bin (shelf) routing.** One feeder, a diverter, N bins stacked in
order. One pass is the inverse of an N-riffle; with N = 8 it takes 4 passes
to reach total-variation distance 0.01, and recombining the bins needs
either the user or a complex mechanism (Diaconis, Fulman & Holmes 2013
analysed a real 10-shelf casino machine and showed one pass is exploitable).

**C. Single-card random-gap insertion (elevator + blades)** — the archived
v1 design. Exact Fisher–Yates; every card needs its own serial
elevator/blade cycle of 1.0–1.4 s, and the blade must enter a chosen card
boundary to ±0.15 mm. Estimated 70–80 s per deck, 45–50 s tuned.

**D. Two parallel insertion wells** — v1 with a diverter and a second well.
Exactly uniform when the A/B assignment is exchangeable (proof in
`02-mathematics.md` §2.6). About 35–40 s, +$45, and the user stacks two half
decks.

**E. 54-slot wheel (random slot assignment) — selected.** A horizontal-axis
cage of 54 radial fins. Each card is pushed radially into a uniformly
chosen empty slot at the upper-right entry position (it slides down the fin
face under gravity and seats against the hub); when the hopper is empty a
shutter in the lower shroud opens and, as the wheel steps through the slots
in order, each card slides out under gravity into a roller nip that pulls
it into a tilted output chute. Assigning card i to a uniformly random empty
slot is the Fisher–Yates "draw from the remaining set" step, so the output
order (slot order) is exactly uniform (proof in `02-mathematics.md`).

| Criterion | A. Riffle | B. Bins | C. v1 insertion | D. Dual well | **E. Wheel** |
|---|---|---|---|---|---|
| Randomness per press | 1 | 2 | 5 | 5 | **5** |
| Speed (est.) | 5 (5 s ×7 manual) | 3 | 2 (70 s) | 3 (38 s) | **4 (35–45 s; ~30 s tuned)** |
| Precision demanded of the build | 4 | 4 | 1 (±0.15 mm) | 1 | **4 (±1 mm)** |
| Cost | 5 | 3 | 3 | 2 | **3** |
| Footprint | 4 | 3 | 4 | 2 | **3 (284 × 213 mm)** |
| Mechanical complexity | 4 | 2 | 3 | 2 | **4** |
| Card wear | 2 | 4 | 5 | 5 | **4** |
| One-button | 1 | 2 | 5 | 4 | **5** |
| **Weighted (randomness ×3, precision ×2, one-button ×2)** | 31 | 33 | 42 | 41 | **52** |

## 1.3 Why the wheel wins

* **No fine tolerance.** A slot mouth is 8 mm wide at the rim and tapers
  to 1.2 mm at the hub, so the wheel only has to be positioned to about
  ±1 mm (±0.7°). A direct-drive NEMA 17 at 1/16 microstepping resolves
  0.11°. The v1 design needed ±0.15 mm on a blade edge.
* **Exact algorithm, no gap opening.** Every card has its own slot; nothing
  is inserted into a stack.
* **Fewer actuators.** One stepper, three small DC motors (feed, entry nip,
  exit nip), one servo (shutter). v1 needed two servos, blades, a lead
  screw and guide rods.
* **Speed.** Loading is limited by the wheel index (average 90°, ≈0.25 s)
  plus the card push (≈0.25 s); unloading is a step-and-pull of 6.7° per
  card. Estimates in §1.6.
* **Self-checking.** A beam at the entry sees whether the card is in the
  slot just loaded; a card that lands one slot off is found by looking at
  the neighbours and the firmware corrects its map (§2.4 shows this keeps
  the output exactly uniform when the error is symmetric).

## 1.4 Architecture

```
                         hopper (deck face down, 50° slope, lid)
                        /  feed roller → gate → entry nip → beam E
                       /
      +Y tower ──── WHEEL: 54 radial fins, cards lie flat on a fin ──── −Y tower
      NEMA 17 ◄── M8 rod, 608 bearings ──►  index tab / sensor at 12 o'clock
                       \
   shroud (lower half)  \ exit window at 8 o'clock, servo shutter
                         \ exit nip → beam X → tilted chute (output deck)
```

* **Wheel module**: cage (Ø155 mm, 98 mm wide, hub tube Ø44, fins from
  r 22 to 77.5), two towers with 608 bearings on an M8 rod, NEMA 17 on the
  +Y tower through a flexible coupler, two shroud segments (lower half),
  sliding shutter, index bracket.
* **Feeder module** (at 50°): the v1 hopper turned so cards travel along
  their 63.5 mm edge; feed roller (Ø27), adjustable gate, entry nip (Ø16)
  with spring idlers, gate beam B, entry beam E, weighted lid.
* **Exit module** (at 205°): shutter window in the shroud, exit nip
  (Ø16) with idlers, exit beam X, chute inclined 28° with a felt-lined end
  wall; the deck is lifted out of the open chute.
* **Base**: two boxes (front/rear) holding the battery and the perfboard,
  with the panel (button, LED, switch, charge jack).
* **Controller**: ESP32 DevKitC; TMC2208 for the wheel, two DRV8833 for the
  three N20 motors; ChaCha20 DRBG seeded from the hardware RNG.

## 1.5 Card handling in the wheel

Cards lie flat on a fin at the entry (fin at 46.7° above horizontal) and
slide down the fin face into the slot under gravity; the entry nip pushes
until 9 mm before the hub, gravity does the rest. In the upper half of the
wheel gravity keeps each card seated against the hub; in the lower half the
card slides outward by at most 1.5 mm onto the fixed shroud. Fins are 1.4 mm
thick and lightened with windows; the card overhangs the fin tips by 8 mm
so the two beams see the card edge outside the discs. At the exit (fin at
208.3°, i.e. 28° below horizontal) the shutter opens, the card slides 7 mm
out under gravity into the running nip and is pulled into the chute.

## 1.6 Cycle time: honest estimate

| Phase | Per card | Basis |
|---|---|---|
| Wheel index to a random empty slot | 0.25 s | mean 90° shortest path; NEMA 17 direct drive, 900°/s², 360°/s max; wheel + cards I ≈ 1.2 × 10⁻³ kg·m² → 19 N·cm peak |
| Card push from the pre-staged gate into the slot (85 mm at 0.3 m/s) | 0.30 s | overlaps ~0.05 s with the index |
| **Load, 52 cards** | **≈ 26 s** | |
| Unload: step 6.7° + pull 63.5 mm at 0.5 m/s + settle | 0.22 s | 52 occupied slots |
| **Unload** | **≈ 12 s** | |
| Homing, start-of-run scan, shutter | ≈ 6 s | the scan (one revolution reading beam E) can be disabled |
| **Total, first build** | **≈ 40–45 s** | |
| Tuned (faster stepper current, 0.15 s index, 0.15 s pull, no scan) | ≈ 28–32 s | |

The 30 s target is within reach after tuning but should not be promised
before a build. The v1 design could not get below ~45 s.

## 1.7 Tradeoffs accepted

* The wheel is the largest printed part (155 × 162 × 110 mm, ~24 h print).
* Cards' edges touch the shroud in the lower half (smooth PETG) and their
  faces slide on the fins; both are gentler than a riffle.
* Two extra sensors and one extra DC motor compared with v1, in exchange
  for removing the blades, the lead screw, the rods and one servo.
* Height ≈ 290 mm (the hopper stands at 50°).

## References

* D. Bayer, P. Diaconis, "Trailing the dovetail shuffle to its lair",
  *Annals of Applied Probability* 2(2), 1992, pp. 294–313.
* P. Diaconis, J. Fulman, S. Holmes, "Analysis of casino shelf shuffling
  machines", *Annals of Applied Probability* 23(4), 2013, pp. 1692–1720.
* Shuffle Master elevator-insertion patents (US 8,538,155 and family) for
  the v1 comparison; Shuffle Tech ST-1000 user reports (80–100 s per deck).
