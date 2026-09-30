# 10. Deliverables, what could not be produced, open unknowns

## 10.1 Delivered

| Requested | Delivered |
|---|---|
| Mechanism comparison, selection, tradeoffs | `01-mechanism-selection.md` (wheel selected; v1 elevator-insertion and the two-well variant compared) |
| Honest cycle-time estimate | `01` §1.6: 40–45 s first build, 28–32 s tuned |
| Mathematics: proofs, RNG, fault analysis, riffle theory, validation plan | `02-mathematics.md` |
| Simulation code and results | `simulation/shuffle_sim.py`, `simulation/results/` (wheel, corrected/uncorrected faults, v1 faults, riffles, bins) |
| Dimensioned mechanical design, parametric CAD, printable files, exploded views, print settings, tolerances | `03-mechanical-design.md`, `cad/params.scad`, `cad/shuffler.scad`, 23 STL files in `cad/stl/`, views in `cad/png/`, `cad/print-settings.md` |
| Electronics, battery, wiring, BOM | `04-electronics.md`, `electronics/wiring-diagram.svg`, `05-bom.md`, `electronics/bom.csv` |
| Complete firmware | `firmware/shuffler/` (RNG core host-tested bit-exact; hardware modules syntax-checked against a mock) |
| Assembly, calibration, first-use checks | `06-assembly.md` |
| Operation, maintenance, troubleshooting | `07-…md` |
| Verification checklist, prototype plan | `08`, `09` |
| Previous design for comparison | `archive/v1-elevator-insertion/` (complete, self-contained) |

## 10.2 Not produced, or produced with limits

* **No physical build, no measurements.** Every [EST] value is an estimate.
  What can be claimed today: the algorithm is uniform (proof); the firmware
  RNG core is bit-exact with the reference and passes its tests; the CAD is
  geometrically consistent (rendered, clearances computed, all STLs on the
  bed); the fault models are simulated. What cannot: feeding reliability,
  slot-entry and slide-out reliability, the cycle time, battery life.
* **ESP32 compile.** The Espressif toolchain could not be downloaded in the
  authoring environment (the proxy blocks the PlatformIO registry, GitHub
  archives and dl.espressif.com). The hardware modules were syntax-checked
  with `g++` against a minimal Arduino mock (`firmware/test_host/mock/`,
  `firmware/test_host/syntax_check.sh`), which catches typos and type errors but not API
  mismatches with the real core. Budget an hour for first-compile fixes.
* **3MF, STEP, PCB files** not generated (OpenSCAD 2021 exports STL).
* **Supplier verification** partial: main parts checked against listings in
  September 2026 (ESP32, TMC2208, MG90S, N20, DRV8833, chargers, IR parts,
  lead-screw kits with couplers); the 3S pack, NEMA 17 single price, index
  module and small hardware are typical prices.
* **Renders** are OpenSCAD previews, not photographs.

## 10.3 Critical unknowns to retire in Stages 1–3

1. Single-card separation with a lip gate on a 50° hopper (the deck presses
   harder on the gate than in v1).
2. Card entry into the tapered slot at 50°: whether cards ever stop short or
   bounce back, and the symmetric/biased split of neighbour-slot errors.
3. Card slide-out at 28° below horizontal into the exit nip; cards clinging
   to the fin by static.
4. Card edges riding on the shroud in the lower half: wear over hundreds of
   decks.
5. Wheel index time and stall margin with the real wheel mass.
6. `bootloader_random_enable()` interaction with `analogRead` on the
   installed core (firmware re-initialises the ADC after seeding).
7. Shutter pin geometry through the tower slot (0.5 mm class clearances).

## 10.4 Environment record

* OpenSCAD 2021.01 rendered all 23 STL files with no warnings (`cad/render.log`).
* Simulation: `simulation/results/full_results.txt` and
  `simulation/results/corrected_runs.txt` (numpy 2.4, scipy 1.17).
* Host test: all pass. Syntax check: pass (two style warnings fixed).
* ESP32 compile: not possible here (see above).
