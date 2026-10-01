# Simulation and validation tools

Requirements: Python 3.9+, numpy (scipy optional for exact chi-square p-values).

| File | Purpose |
|---|---|
| `shuffle_sim.py` | Models: exact wheel assignment, wheel neighbour-slot faults (uncorrected and sensor-corrected, symmetric and biased), the archived v1 insertion model and its faults, GSR riffles, a two-hopper riffle machine, bin/shelf shufflers; the statistical test battery; exact Bayer–Diaconis table; small-n total variation. `--quick` ≈ 1 min; `--bd-table` theory only. |
| `rng_reference.py` | Bit-exact reference of the firmware RNG pipeline; `--selftest` prints the test-key slot assignment the firmware's `test ref` must reproduce. |
| `analyze_physical.py` | `--log` (correction tally from the firmware's `last` lines), `--compare` (predicted vs actual deck), `--slots` (statistics on dumped assignments), `--decks` (statistics on recorded physical shuffles). |
| `wheel_sim.html` | Animated, timed simulation of one shuffle on the CAD geometry: homing, scan, load into random empty slots, unload in slot order. Uses the firmware's RNG (bit-exact, self-tested against `rng_reference.py` on load), stepper ramp, waits and beam checks; `test fixed N` seeds reproduce the firmware's `PLAN` lines. Options for off-slot landings and the unload order; runs 1,000 shuffles for cycle-time and exit-window statistics. Open it in a browser; no install. |
| `results/` | Output of the runs quoted in `docs/02-mathematics.md`. |
