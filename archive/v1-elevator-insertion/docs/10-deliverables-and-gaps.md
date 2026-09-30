# 10. Deliverables, what could not be produced, and open unknowns

## 10.1 Delivered

| Requested | Delivered | Notes |
|---|---|---|
| Mechanism comparison (≥ 3, incl. riffle and random insertion) | `01-mechanism-selection.md` | 4 candidates scored; gap insertion selected |
| Honest cycle-time estimate | `01` §1.6 | 70–80 s first build; 30 s target declared unreachable for this architecture |
| Mathematical analysis, proofs vs simulation vs estimates separated | `02-mathematics.md` | Fisher–Yates uniformity proof; symmetric-error invariance proof; Bayer–Diaconis exact table; fault-model simulations |
| Entropy, RNG, seeding, bias avoidance | `02` §2.3, `firmware/shuffler/rng.cpp`, `simulation/rng_reference.py` | ChaCha20 DRBG, rejection sampling, health test |
| Simulation code + validation plan | `simulation/`, `02` §2.7–2.8 | Position frequency, pairwise, adjacency, rising sequences, serial correlation, guessing score, small-n TV |
| Dimensioned mechanical design | `03-mechanical-design.md`, `cad/params.scad` | Every dimension parametric |
| Editable parametric CAD + printable files | `cad/shuffler.scad`, 19 STL files | OpenSCAD 2021.01; STLs rendered here |
| Exploded views, labels, print orientation, materials, tolerances | `cad/png/*.png`, `cad/print-settings.md`, `03` §3.5–3.6 | |
| Electronics: motors, drivers, controller, sensors, jam detection, safe shutdown | `04-electronics.md` | |
| Battery: protected pack, charger, energy per shuffle, shuffles per charge, stall currents | `04` §4.4–4.6 | All labelled [EST] |
| Wiring diagram, BOM with quantities/specs/alternatives/costs/availability | `electronics/wiring-diagram.svg`, `05-bom.md`, `electronics/bom.csv` | Prices partly verified (see status column) |
| Complete firmware: randomisation, motor control, sensors, counting, jams, low battery, calibration, indicators | `firmware/shuffler/` | See 10.2 for compile status |
| Numbered assembly instructions, tools, fasteners, calibration, first-use checks | `06-assembly.md` | |
| Operating, maintenance, troubleshooting | `07-…md` | |
| Staged prototype plan validating feeding first | `09-prototype-plan.md` | |
| Verification checklist | `08-verification-checklist.md` | |

## 10.2 Not produced, or produced with limits

* **No physical build, no measurements.** All [EST] values are estimates.
  The claims that can be made today: the algorithm is uniform (proof), the
  firmware's RNG core is bit-exact with the reference and passes its host
  tests, the CAD is geometrically consistent (rendered STLs, checked
  clearances), the fault models are simulated. The claims that cannot be
  made: that feeding is reliable, that blade entry works at the stated
  rate, that the cycle time is what is estimated, that the battery life is
  what is estimated.
* **ESP32 compile.** The platform-independent core compiles and passes
  tests on the host (`firmware/test_host`). Compiling the full firmware for
  the ESP32 requires the Espressif toolchain; the authoring environment's
  attempt is recorded at the end of this file. Expect the usual first-compile
  fixes (an include path, a renamed core API) if it was not compiled.
* **3MF files** were not generated (OpenSCAD 2021.01 exports STL; 3MF export
  exists in newer builds: `openscad --export-format 3mf`).
* **PCB.** Perfboard layout guidance only; no PCB design files.
* **STEP/Fusion files.** OpenSCAD only. Editable, but not a feature-tree CAD.
* **Supplier verification.** Prices were checked against search results for
  the main parts (ESP32, TMC2208, lead screw kits, MG90S, N20, DRV8833,
  chargers, IR parts, feeler gauges) in September 2026; the 3S pack, NEMA 17
  single-unit price, PD trigger boards and small hardware were not verified
  item by item. Availability changes; treat the BOM as a shopping guide.
* **Photographs / rendered "attractive" images.** OpenSCAD previews only.
* **The 30 s cycle target** is not met by design (see `01` §1.6).

## 10.3 Critical unknowns to resolve in Stage 1–2 of the prototype plan

1. **Single-card separation reliability** with a simple lip gate across
   card types and wear states. Fallback: cork retard strip; two-stage gate.
2. **Blade entry into a chosen boundary** with the stack aligned only by
   gravity (10° tilt) and a felt pad: the success rate and the symmetry of
   the residual errors. This is the number the predicted-vs-actual test
   measures, and the one that decides whether the ≈ 0.01 TV target is met.
3. **Card cling** on the blades when the lower stack drops (plastic cards).
   Mitigation exists (jam detection, larger `GAP_DROP`) but the rate is
   unknown.
4. **`bootloader_random_enable()` interaction with `analogRead`** on the
   installed Arduino core version; the firmware re-initialises the ADC after
   seeding, but this must be checked once on the real board (`beams` after
   `test raw`).
5. **Servo pin geometry**: the M2 screw pin from the horn through the plate
   window into the bar slot has 0.5 mm of vertical margin to the blade;
   print tolerance may require a 3.5 mm rather than 4 mm pin protrusion.
6. **Lid ballast** needed for the last few cards vs. the double-feed limit.
7. **Actual cycle time** — depends on servo settle and feed retries.

## 10.4 Environment record

* OpenSCAD 2021.01 rendered all 19 STLs with no warnings; render log in
  `cad/render.log`.
* Simulation run: `simulation/results/full_results.txt` (numpy 2.4, scipy
  1.17).
* Host test: `firmware/test_host` — all tests pass.
* ESP32 compile attempt (PlatformIO, `pio run -e esp32dev`): see the line
  appended below by the build step.

**ESP32 compile status (authoring environment):** not compiled. Two
PlatformIO attempts (`pio run -e esp32dev`) failed at "Platform Manager:
Installing espressif32 — HTTPClientError" because the environment's network
proxy blocks the PlatformIO registry, GitHub archive downloads and
dl.espressif.com (HTTP 000/403 on direct probes). The firmware was written
against the documented Arduino-ESP32 2.x/3.x APIs and its
platform-independent core is host-tested, but the hardware modules
(`platform.cpp`, `hardware.cpp`, `motion.cpp`, `cli.cpp`, `shuffler.ino`)
have not been through a compiler. Budget an hour for first-compile fixes.
