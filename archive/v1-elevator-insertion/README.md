# Open card shuffler — single-card random-gap insertion, 3D-printed, battery powered

A one-button shuffler for one 52–54 card poker deck. Cards are fed one at a
time from a hopper into a vertical well whose stack rests on an elevator;
before each card, two thin blades open a gap at a uniformly random position
and the card is pushed in. That is the inside-out Fisher–Yates algorithm
executed physically: every ordering of the deck is equally likely when the
mechanism does what it is told, and the design is arranged so that the ways
it can fail to do so are detectable.

**Status: complete design, unbuilt.** Nothing in this repository has been
printed, wired or tested on hardware. Every performance number is an
estimate and is labelled as such. Do not describe this machine as proven or
as guaranteed uniformly random; the validation plan says how to earn those
words.

![assembly](cad/png/assembly.png)

## What is here

| Deliverable | Where |
|---|---|
| Mechanism comparison and selection (riffle, bin routing, carousel, gap insertion) | `docs/01-mechanism-selection.md` |
| Mathematics: uniformity proof, RNG pipeline, fault analysis, riffle theory, validation plan | `docs/02-mathematics.md` |
| Simulation code and results | `simulation/shuffle_sim.py`, `simulation/results/` |
| Bit-exact RNG reference and physical-test analysis | `simulation/rng_reference.py`, `simulation/analyze_physical.py` |
| Parametric CAD (OpenSCAD), 19 printable STLs, assembly / exploded / section views | `cad/params.scad`, `cad/shuffler.scad`, `cad/stl/`, `cad/png/` |
| Mechanical design document (dimensions, tolerances, adjustments, card protection) | `docs/03-mechanical-design.md`, `cad/print-settings.md` |
| Electronics, battery, power budget, pin table | `docs/04-electronics.md`, `electronics/wiring-diagram.svg` |
| Bill of materials | `docs/05-bom.md`, `electronics/bom.csv` |
| Firmware (ESP32, Arduino framework) with host-tested RNG core | `firmware/` |
| Assembly instructions, calibration, first-use checks | `docs/06-assembly.md` |
| Operation, maintenance, troubleshooting | `docs/07-operation-maintenance-troubleshooting.md` |
| Verification checklist | `docs/08-verification-checklist.md` |
| Staged prototype plan | `docs/09-prototype-plan.md` |
| What could not be produced here, and open unknowns | `docs/10-deliverables-and-gaps.md` |
| Diagrams | `docs/img/mechanism-section.svg`, `docs/img/insertion-sequence.svg`, `docs/img/firmware-states.svg` |

## Headline numbers (all estimates unless stated)

| | |
|---|---|
| Parts cost, excluding printing | ≈ $121 new; ≈ $104 with a normal parts drawer; ≈ $95 minimum configuration |
| Cycle time, 52 cards | 70–80 s first build; ~45–50 s tuned. **The 30 s target is not reachable with this architecture.** |
| Randomness | Exact uniform in the ideal model (proof in §2.2); symmetric ±1 blade errors provably harmless; systematic errors and silent faults are measurable with the predicted-vs-actual test |
| Size | 261 × 126 mm footprint, ≈ 160 mm tall |
| Battery | 3S Li-ion 2200 mAh with BMS; ≈ 0.085 Wh per shuffle → about 200 shuffles or 3 game evenings per charge |
| Cards | 52–54, thickness 0.22–0.42 mm (gate adjustable) |

## Quick start for a builder

1. Read `docs/01` and `docs/03` to understand the mechanism.
2. Follow `docs/09-prototype-plan.md`: build and test the feeder alone
   first, then the well, then integrate.
3. Print with `cad/print-settings.md`; buy from `docs/05-bom.md`; wire from
   `electronics/wiring-diagram.svg`; flash `firmware/`; calibrate with
   `docs/06-assembly.md` §6.7; verify with `docs/08`.

## Reproducing the analysis

```
pip install numpy scipy
python3 simulation/shuffle_sim.py --quick        # ~1 min
python3 simulation/rng_reference.py --selftest
cd firmware/test_host && make run                # host test of the firmware RNG core
cd cad && ./render.sh                            # needs OpenSCAD 2021.01 and xvfb-run for PNGs
```

## Licence

Everything here is released under CC BY 4.0 (documentation, CAD) and the
MIT licence (code). Attribution appreciated; no warranty of any kind — see
the safety notes in `docs/07`.
