# 10. Deliverables, what could not be produced, open unknowns

## 10.1 Delivered

| Requested | Delivered |
|---|---|
| Mechanism comparison, selection, tradeoffs | `01-mechanism-selection.md` (wheel selected; v1 elevator-insertion and the two-well variant compared) |
| Honest cycle-time estimate | `01` §1.6: 40–45 s first build, 28–32 s tuned |
| Mathematics: proofs, RNG, fault analysis, riffle theory, validation plan | `02-mathematics.md` |
| Simulation code and results | `simulation/shuffle_sim.py`, `simulation/results/` (wheel, corrected/uncorrected faults, v1 faults, riffles, bins) |
| Dimensioned mechanical design, parametric CAD, printable files, exploded views, print settings, tolerances | `03-mechanical-design.md`, `cad/params.scad`, `cad/shuffler.scad`, 24 STL files in `cad/stl/`, views in `cad/png/`, print list `cad/print-settings.md`, collision checks `cad/check_clearance.sh`, print checks `cad/check_printability.py` |
| Electronics, battery, wiring, BOM | `04-electronics.md`, `electronics/wiring-diagram.svg`, `05-bom.md`, `electronics/bom.csv` |
| Complete firmware | `firmware/shuffler/` (RNG core host-tested bit-exact; hardware modules syntax-checked against a mock) |
| Assembly, calibration, first-use checks | `06-assembly.md` |
| Operation, maintenance, troubleshooting | `07-…md` |
| Verification checklist, prototype plan | `08`, `09` |
| Previous design for comparison | `archive/v1-elevator-insertion/` (complete, self-contained) |

## 10.2 Not produced, or produced with limits

* **No physical build, no measurements.** Every [EST] value is an estimate.
  What can be claimed today: the algorithm is uniform (proof); the firmware
  RNG core is bit-exact with the reference and passes its tests; the CAD
  passes 17 automated collision checks against the wheel's swept volume
  and between neighbouring parts; the fault models are simulated. What cannot: feeding reliability,
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
7. The inner shutter blade (1.2 mm PETG) sliding on the shroud: friction,
   and whether card edges ride its 0.8 mm ramps smoothly.
8. The oblique beams E and S: signal margin over 116 mm with a 1.2 mm
   aperture, and how cleanly beam S separates "seated" (edge at r 85.5)
   from "bridging" (card across r 86.8).

## 10.4 Review log (design review after the wheel redesign)

Mistakes found and fixed in the review:

* **Speed:** firmware acceleration 900°/s² gave 0.6 s per index, not the
  0.25 s the docs claimed. It is now 5000°/s² at about 1 A RMS. The quoted
  torque had also been 10× too high.
* **Entry:** seated cards overlapped the feeder plate and the beam-E
  bridge by 3.5 mm. Every wheel move would have dragged cards into them.
  The plate now starts at the nip, and beams E and S run obliquely between
  the towers.
* **Exit:** the outside sliding shutter let cards drop 3.5 mm into the
  window and snag its edge. It is now a ramped blade on the inside of the
  shroud.
* **Feeder motors:** the entry N20 motors passed through the motor-side
  tower. The rollers are now 64 mm long and the motors fit inside.
* **Idlers:** the idler wheels missed the roller O-rings. They now sit over
  the outer O-rings.
* **Missing mounts:** beam B's detector had no mount; it is now in the
  idler arm's boss. The idler springs had nothing to push on; spring
  crossbars were added. The index bracket screws went into empty space
  above the tower; a post was added. The slotted index module could not
  straddle the tab; it is now a radial LED and phototransistor pair.
* **Pivot conflicts:** the gate block flange collided with the idler pivot.
  The gate moved 7 mm outward. The idler ears sat on the rails and could
  not pivot.
* **Mismatched holes:** the tower-to-deck and tower-to-chute screw holes
  were perpendicular to their mating holes.
* **Placement:** the mechanism was placed 45 mm too low, sinking into the
  base. The chute passed through the rear base box.
* **Footprint:** it was 320 mm, over the 12 in (305 mm) limit. It is now
  about 298 × 231 mm.
* **Firmware safety:** nothing stopped the wheel turning with a card half
  in a slot. There is now a beam S / beam X interlock on every move.
  There was also no check that the target slot was empty before feeding.
  The firmware now re-homes, then stops with ERROR 11. A failed eject
  carried on rotating; it now stops. The exit has a block-time double-card
  check.
* **Docs:** the low-battery abort description did not match the firmware.
* **Unload order:** the firmware opened the shutter before the first unload
  move, with the wheel still at the last entry position. Two cards rested
  over the window there and slid out against the stopped nip, unseen by
  beam X, and the move to slot 0 (up to 180°) swept 11–14 more cards across
  the open window, the snag the inner blade was meant to prevent. The
  wheel now brings the first occupied slot to the exit with the shutter
  closed, then opens it. With a full deck every later move carries only
  empty slots past the window. "Unload all" also closes the shutter before
  homing, since it can follow an ERROR 9 with the shutter still open. The
  bench test (6.5) and C8 (6.7) now move first and open second. Still
  open: on a wheel holding a few cards, a run of more than 27 empty slots
  makes the move to the next card take the short way back over the
  window, carrying any later card across it.

## 10.4.1 Print-readiness sweep

A slicer-style check (`cad/check_printability.py`: slices every STL at
0.2 mm and flags anything printed over air) and new mating-part collision
checks found:

* **Towers** printed standing on their 4 mm beam bosses, with the whole
  plate 4 mm in the air. They now print outer face down; the motor
  standoff became the separate `motor_mount`, bolted through with 4 × M3 ×
  40. A plate was added behind the beam boss, which overhung the tower edge.
* **Chute vs towers (assembly clash):** the chute floor ran through both
  tower plates (≈ 1.9 cm³ overlap), and one chute screw hole was sealed
  inside the tower. The floor now sits between the towers and screws to
  new tower cheeks from the side.
* **Wheel** printed on a hub boss, with the 155 mm disc 6 mm in the air.
  The −Y boss is now the separate `hub_spacer`, the −Y disc is solid, the
  hub has internal ribs, and the fin windows are pointed so their tops
  need no bridging. The wheel is heavier (≈ 375 g; inertia ≈ 1.45 × 10⁻³
  kg·m² with cards, ≈ 13 N·cm peak at 5000°/s²).
* **Shroud A** was cut in two by the exit window, and both shrouds stood
  on their tab feet only, with the arc 12 mm in the air. The arc now runs
  tower to tower, with a strap across the window and 45° tab gussets.
* **Feeder deck:** the hinge knuckles were loose, unattached bodies, and
  the middle one sat over the finger recess. The rails touched the gate
  bosses along a single edge, so the mesh was not watertight. The gate
  wall was a 91 mm bridge. Now there are two gusseted knuckles, window
  jambs, and three cut-away print ribs.
* **Spring crossbars** (deck and chute) were 110 mm bridges. They are now
  a separate `spring_bar`, ×2.
* **Lid** printed on its ballast box, with the plate overhanging it, and
  had a sealed cavity. The AA ballast now drops into pockets in the
  pressure boss, and the lid prints top face down.
* **Rollers** stood on their 4 mm stub axles; they now stand on the D-bore
  end. The **index bracket** printed with 7 mm ledges; it now prints back
  plate down. The **gate block's** top overhung its flange; it is tapered.
  The exit +Y rail hung over the motor pocket and was removed.
* **Counts:** 24 files, 30 pieces. The two nips use 2 × `roller_nip`, and
  the 2 × `idler_arm` carry 4 × `idler_wheel`. Filament is ≈ 1.25 kg, not
  900 g.

## 10.5 Environment record

* OpenSCAD 2021.01 rendered all 24 STL files with no warnings; `cad/check_clearance.sh`: 19 OK + 4 face contacts, no collisions (`cad/clearance.log`); `cad/check_printability.py`: all 24 parts OK (`cad/printability.log`).
* Simulation: `simulation/results/full_results.txt` and
  `simulation/results/corrected_runs.txt` (numpy 2.4, scipy 1.17).
* Host test: all pass. Syntax check: pass (two style warnings fixed).
* ESP32 compile: not possible here (see above).
