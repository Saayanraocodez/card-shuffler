# 8. Verification checklist

★ = establishes randomness; the rest establishes reliability.

## A. Parts and mechanics

- [ ] `cad/check_clearance.sh` reports no COLLISION in any of its 24 checks after any CAD change.
- [ ] `cad/check_printability.py` reports ALL PARTS OK, `cad/check_clearance.sh` reports no COLLISION, and (on an Ultimaker 3) `cad/check_um3_fit.py` reports ALL PARTS FIT after any CAD change.
- [ ] All 24 STL files printed (30 pieces with duplicates, `cad/print-settings.md`); the feeder deck and the chute flat on glass (< 0.3 mm).
- [ ] The gate-window print ribs are cut off and filed flush; the gate block slides in freely.
- [ ] Every one of the 54 slots accepts a card by hand to the hub and releases it.
- [ ] Wheel spins freely on the rod, axial play < 0.5 mm, no wobble > 0.5 mm at the rim.
- [ ] With cards seated, no card touches the shroud in the upper half; in the lower half cards slide out ≤ 1.5 mm and ride the shroud without catching.
- [ ] Feeder mounted, wheel full: `jog 170` six times (counter-clockwise at full speed). Cards ride up the `shroud_B` ramp and pass the entry with no card touching the entry nip roller or the deck tip.
- [ ] A card laid on a printed PETG surface starts to slide at a tilt of ≤ 30° (friction ≤ 0.6, the range `simulation/card_slip.js` covers with the ramp).
- [ ] Index tab passes the sensor with ≥ 1 mm clearance each side.
- [ ] Rollers turn freely; crowns 0.7 / 0.5 mm above their plates.
- [ ] Gate set by feeler gauge (0.40 passes, 0.55 blocked).
- [ ] The shutter blade slides 17.5° freely inside the shroud; closed it covers the window, open the window is clear; cards ride over its ramps without catching.
- [ ] At `entry k` the fin face is 0.4 mm below the plate plane at its tip (straight edge).
- [ ] With a card held halfway into the mouth, `jog 10` is refused (beam S interlock). Same for a card in the exit nip (beam X).
- [ ] Chute end wall felt fitted; finger opening usable.

## B. Electronics

- [ ] Buck 5.00 ± 0.05 V at 1 A; TMC Vref ≈ 1.4 V (≈ 1.0 A RMS); MS1/MS2 set; motor never hot-plugged.
- [ ] `bat` = multimeter ± 0.05 V after `cal vbat`.
- [ ] `beams`: B, E, S, X clear > 300 counts, blocked < 50 with a card; `index` shows TAB once per revolution.
- [ ] Fuse in the positive lead; charge jack upstream of the switch; charger LED indicates charging.

## C. Firmware

- [ ] `firmware/test_host` `make run` passes; `firmware/test_host/syntax_check.sh` passes.
- [ ] `test ref` on the ESP32 = `rng_reference.py --selftest` slot list. ★
- [ ] `test raw 65536` → `ent`: ≈ 8.0 bits/byte. ★
- [ ] `test seed 20`: 20 different keys. ★
- [ ] `test rng 100000` → `analyze_physical.py --slots`: all statistics within the uniform reference. ★
- [ ] Low-battery behaviour with a bench supply: warn 9.9 V, refuse 9.6 V, abort 9.3 V then unload-all recovers the cards.
- [ ] Error paths: index unplugged (E1), empty hopper (E2), cards left in the wheel (E4), a card placed by hand in an "empty" slot during loading (E11).

## D. Feeding (Stage 1)

- [ ] 100 consecutive single feeds, standard deck; B-block time spread < ±10 %.
- [ ] Same with a 0.25 mm deck and a 0.40 mm deck after re-setting the gate; with a worn deck.

## E. Wheel loading and unloading (Stages 2–3) ★

- [ ] `entry k` for k = 0, 13, 27, 40, 53 then a hand-fed card: beam E blocked and beam S clear every time (20 trials each).
- [ ] Mean index time ≤ 0.3 s (time 50 random `entry k` moves) with no skipped steps (`home` afterwards finds the index within ±0.2°).
- [ ] 20 fixed-seed shuffles: `last` shows ≤ 1 % corrections, direction-bias binomial p > 0.05, 0 lost cards, 0 double-feed suspects, ejected = loaded = 52 (`analyze_physical.py --log`).
- [ ] Predicted-vs-actual on the same 20 decks (`--compare`): identical.
- [ ] Repeat 5 decks each from a sorted, reversed and shuffled deck.
- [ ] Fault injection: `cal entry +1.5` → corrections show a directional excess; gate 0.8 mm → count warning. Restore.

## F. Whole machine

- [ ] 20 consecutive shuffles without error; time recorded (`cal show`).
- [ ] Card inspection after 50 shuffles under a lamp against 5 unused cards: no scratches, edge marks, corners.
- [ ] 200+ physical shuffles for `analyze_physical.py --decks`.
- [ ] Shuffles per charge measured and written into `04-electronics.md`.

## G. Documentation honesty

- [ ] Every measured number replaces its [EST] with [PHYS].
- [ ] Remaining [EST] items listed in `10-deliverables-and-gaps.md`.
