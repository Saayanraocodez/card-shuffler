# Simulation and validation tools

Requirements: Python 3.9+, numpy (scipy optional for exact chi-square p-values).

| File | Purpose |
|---|---|
| `shuffle_sim.py` | Models: exact wheel assignment, wheel neighbour-slot faults (uncorrected and sensor-corrected, symmetric and biased), the archived v1 insertion model and its faults, GSR riffles, a two-hopper riffle machine, bin/shelf shufflers; the statistical test battery; exact Bayer–Diaconis table; small-n total variation. `--quick` ≈ 1 min; `--bd-table` theory only. |
| `rng_reference.py` | Bit-exact reference of the firmware RNG pipeline; `--selftest` prints the test-key slot assignment the firmware's `test ref` must reproduce. |
| `analyze_physical.py` | `--log` (correction tally from the firmware's `last` lines), `--compare` (predicted vs actual deck), `--slots` (statistics on dumped assignments), `--decks` (statistics on recorded physical shuffles). |
| `results/` | Output of the runs quoted in `docs/02-mathematics.md`. |
