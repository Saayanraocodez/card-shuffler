# Simulation and validation tools

Requirements: Python 3.9+, numpy (scipy optional, for exact chi-square p-values).

| File | Purpose |
|---|---|
| `shuffle_sim.py` | Monte-Carlo models (exact Fisher–Yates, blade/feed fault models, GSR riffles, mechanical riffle machine, bin/shelf shuffler), the statistical test battery, exact Bayer–Diaconis total-variation table, small-n exact TV. `--quick` for a 1-minute run; `--bd-table` for the theory table only. |
| `rng_reference.py` | Bit-exact reference of the firmware's RNG pipeline (SHA-256 seed → ChaCha20 → rejection sampling). `--selftest` checks RFC 8439 vectors and prints the test-key gap sequence that the firmware's `test ref` must reproduce. |
| `analyze_physical.py` | Predicted-vs-actual comparison for physical shuffles (`--compare`), output statistics on recorded decks (`--decks`), statistics on gap sequences dumped by the firmware (`--gaps`). |
| `results/full_results.txt`, `results/full_results.json` | Output of the full run used in `docs/02-mathematics.md`. |

The firmware host test (`firmware/test_host`) reproduces the same 52-gap
sequence and the same predicted order as these scripts.
