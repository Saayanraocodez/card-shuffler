# 9. Staged prototype plan

The design's two risks are feeding exactly one card and placing a blade in
the intended gap. Each stage tests one of them before the next investment.

## Stage 0 — parts on the bench (1 evening, ~$25 of parts)

Buy the ESP32, the DRV8833, two N20 motors, the IR pairs and the O-rings.
Print the two rollers. Wire the DRV8833 and one beam on a breadboard; flash
the firmware; confirm `feed`-related commands drive the motors and the
beam reads. Purpose: firmware toolchain and sensor readings work.

## Stage 1 — feeder module only (1 weekend, ~$40 cumulative)

Print: feeder_deck, gate_block, motor_bracket, motor_retainer ×2,
bushing_bracket, idler_arm, idler_wheel ×2, lid. Assemble steps 1–13 of
`06-assembly.md`. Clamp the deck to a board tilted 10° with a box catching
cards.

Tests:
* `feed` × 100 with a standard deck: goal 0 double, 0 miss.
* Vary the gate: find the gap window where single feeds succeed (expect
  0.40–0.50 mm for 0.30 mm cards). Record it.
* Thin and thick decks; worn deck.
* Measure the card exit speed (B-block time and card length give it) and
  confirm the card would coast ≥ 18 mm on a 10° slope: place a card on the
  transport plate extension and see where it stops.

Exit criterion: 300 single feeds in a row across three deck types. If the
gate alone cannot separate cards for a deck type, add a cork strip to the
gate lip's front face (retard pad) before moving on.

## Stage 2 — well and elevator, no feeder (1 weekend, ~$75 cumulative)

Print: well_deck, well_sleeve, knife bars ×4, servo_bracket ×2, column,
carriage, platform. Buy the stepper, driver, lead screw, rods, bearings,
servos, brass.

Tests:
* Elevator travel, homing repeatability (10 homes → `z 0` → measure the
  platform height with a depth gauge: spread < 0.05 mm).
* `cal home`; beam-S repeatability (10 measurements of a 30-card stack:
  spread < 0.1 mm).
* Blade entry: `cal knifetest k` for k = 0, 5, 10, 20, 30 with 30 cards,
  10 times each. Count clean entries. Goal: 100 %. If not: polish the
  bevel; check the stack sits against the back wall; check bar height
  equality (both blades within 0.1 mm — measure with the platform as a
  reference).
* Hand-insertion test: with the gap open, push a card in by hand through
  the slot, close the gap. Repeat 50 times; the card must always end up
  exactly in the intended gap (check by numbering the cards).

Exit criterion: 50/50 clean blade entries at random k and 50/50 correct
hand insertions.

## Stage 3 — integration (1 weekend, ~$120 cumulative)

Join the modules, add the skirts, battery and panel. Run the predicted-
vs-actual test (§2.8 step 4) for 20 fixed-seed shuffles. Tune timeouts and
`SERVO_SETTLE_MS`. Record the shuffle time.

Exit criterion: the acceptance targets in `08-verification-checklist.md`
section E.

## Stage 4 — endurance and statistics

200 shuffles over a few weeks with real play; log `cal show` statistics;
run the physical output statistics; inspect cards for wear.

## What to do if a stage fails

| Failure | Fallback within this design |
|---|---|
| Double feeds persist | Cork retard strip on the gate; smaller lid ballast; second gate stage (print a second, higher lip 5 mm downstream) |
| Cards do not coast to the pad | Increase `TILT` to 12° in params.scad and reprint the skirts; raise transport PWM |
| Blades push cards | Thinner blades (0.4 mm feeler stock); sharper symmetric bevel; increase `GAP_DROP`; add 0.5 mm foam under the platform's card area so the lower stack yields |
| Blade heights unequal | Shim one bar with tape under the lower half; or print both bars again in the same job |
| Stack top measurement noisy | Move beam S to a 1.0 mm aperture (drill a printed plug) |
| Elevator too slow | 3S pack and 0.8 A Vref; `ELEV_VMAX` 40; T8×8 already the fastest common lead |
| Servo too slow | Digital MG90D-class servo (0.06 s/60°) |
