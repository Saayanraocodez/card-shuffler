# 8. Verification checklist

Tick every item before trusting the machine for a game. Items marked ★ are
the ones that establish randomness; the others establish reliability.

## A. Printed parts and mechanics

- [ ] All 19 parts printed; no warping on the two decks (lay on glass: < 0.3 mm gap).
- [ ] Rollers turn freely; crowns 0.7 / 0.5 mm above the plate.
- [ ] Gate set with a feeler gauge; 0.40 mm passes, 0.55 mm does not (standard cards).
- [ ] Idler arm pivots freely, wheels touch the transport roller, springs compressed ~3 mm with the lid closed.
- [ ] Both blade bars slide 8 mm freely; blades 1 mm inside the wall when out, 7 mm in when in; blade edges polished, corners rounded.
- [ ] Platform travels −25 … +22 mm without touching the walls; no play in the T8 nut (< 0.05 mm).
- [ ] Endstop trips at Z ≈ −26 (`status` while jogging).
- [ ] Well and sleeve interiors aligned (run a card up and down the well by hand: no catch).
- [ ] Feeder plate and well slot bottom flush (±0.2 mm).
- [ ] Felt pads fitted; cards seat square against the back wall.

## B. Electronics

- [ ] Buck output 5.00 ± 0.05 V under a 1 A load.
- [ ] TMC2208 Vref 0.9 V; MS1/MS2 configured; motor wires never hot-plugged.
- [ ] `bat` matches the multimeter after `cal vbat` (± 0.05 V).
- [ ] `beams`: all three clear signals > 300 counts, blocked (card inserted by hand) < 50 counts.
- [ ] Endstop reads "hit" only when pressed (NC wiring verified by unplugging: must read hit → safe).
- [ ] Servos reach both endpoints without buzzing at the stops (endpoints are inside the mechanical range).
- [ ] Fuse in the positive lead; charge jack wired upstream of the switch; charger LED indicates charging.

## C. Firmware

- [ ] `firmware/test_host` `make run` passes (RNG core bit-exact with the Python reference).
- [ ] `test ref` on the ESP32 prints the same 52 gaps as `rng_reference.py --selftest`. ★
- [ ] `test raw 65536` → `ent` (or NIST STS) on the dump: ≈ 8.0 bits/byte, no test failing. ★
- [ ] `test seed 20`: 20 different keys (no repeats). ★
- [ ] `test rng 100000` → `analyze_physical.py --gaps`: all statistics within the uniform reference ranges. ★
- [ ] Low-battery behaviour tested with a bench supply: warn at 9.9 V, refuse at 9.6 V, abort-and-present at 9.3 V.
- [ ] Error paths tested: unplug the endstop (ERROR 1), empty hopper (ERROR 2), card left in the well (ERROR 4).

## D. Feeding reliability (Stage 1 of the prototype plan)

- [ ] 100 consecutive single feeds, standard deck: 0 double feeds, 0 misses, B-block time spread < ±10 %.
- [ ] Same with a thin deck (0.25 mm) and a thick custom deck (0.40 mm) after re-setting the gate.
- [ ] 100 feeds with a worn (used) deck.

## E. Insertion accuracy (Stage 2–3) ★

- [ ] `cal knifetest` at gaps 0, 5, 15, 25, 40 (deck of 52 in the well): blades enter a boundary cleanly every time, no card pushed.
- [ ] Predicted-vs-actual test, 20 fixed-seed shuffles (1040 insertions): ≤ 1 % off-by-one, direction-bias p > 0.05, 0 landed-on-top, 0 double feeds (`analyze_physical.py --compare`).
- [ ] Repeat the predicted-vs-actual test from a sorted deck, a reversed deck and a shuffled deck (5 each).
- [ ] Fault injection: gate at 0.8 mm → count warning appears; `cal knife +0.3` → bias test reports "+" direction, then restore.

## F. Whole-machine

- [ ] 20 consecutive shuffles without an error; time per shuffle recorded (`stats` via `cal show`).
- [ ] Card inspection after 50 shuffles: no scratches, no marked edges, no bent corners (compare with 5 unused cards from the same deck under a lamp).
- [ ] 200+ physical shuffles logged for `analyze_physical.py --decks` (sanity statistics).
- [ ] Battery: shuffles per charge measured and written into `04-electronics.md` in place of the estimate.

## G. Documentation honesty

- [ ] Every number in the docs that was measured on the build has replaced its [EST] with [PHYS] and the value.
- [ ] Items still marked [EST] are listed in `10-deliverables-and-gaps.md`.
