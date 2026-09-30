# 9. Staged prototype plan

The wheel's risks, in order: single-card feeding, card entry into a slot,
card slide-out at the exit, and unload throughput. Each stage tests one
before the next print job.

## Stage 0 — electronics on the bench (1 evening, ≈ $30)

ESP32, two DRV8833, three N20, IR pairs, index module on a breadboard.
Flash; `beams`, `feed` (motors spinning free), `test ref`. Purpose: the
toolchain and sensors work; `test ref` matches the Python reference.

## Stage 1 — feeder module (1 weekend, ≈ $45 cumulative)

Print feeder_deck, gate_block, roller_feed, one roller_nip, the two feeder
brackets, retainers, one idler_arm, two idler_wheels, lid. Assemble steps
1–12. Clamp the deck at 50° with a box under the tip.

Tests: 100 single feeds; gate window (expect 0.40–0.50 mm for 0.30 mm
cards); thin/thick/worn decks; card exit speed from the B-block time;
confirm a card leaving the nip at 50° slides ≥ 20 mm on a PETG surface.
Exit criterion: 300 single feeds in a row across three deck types.
Fallback: cork retard strip on the gate lip.

## Stage 2 — wheel + towers, hand feeding (1 weekend, ≈ $75 cumulative)

Print wheel, both towers, shroud_A/B, shutter, index_bracket, servo
bracket; buy the stepper, driver, rod, bearings, coupler, servo. Assemble
steps 13–22 on the joined base boxes (print them now or use a plywood
board with the tower foot holes).

Tests: homing repeatability (10 homes, `entry 0`, mark the rim: spread
< 0.5 mm); `entry k` for 10 random k then a hand-pushed card: beam E
blocked 50/50; rotate the loaded wheel 20 revolutions each way: no card
lost, none touching the shroud in the upper half; the shutter opens and a
card at `exit k` slides out into a hand-held box 50/50. Measure the index
time for 90° (target ≤ 0.3 s) and the stall margin (increase
`WHEEL_ACC_DPS2` until it skips, then back off 30 %).
Exit criterion: 50/50 entries, 50/50 slide-outs, no lost cards.

## Stage 3 — feeder on the wheel (1 evening)

Mount the feeder deck (step 23). `feed` with `entry k`: 100 cards, 0
corrections, 0 lost; `scan` confirms the map. Tune `cal entry`.
Exit criterion: 200 feeds, ≤ 1 correction, all in the intended slot after
correction.

## Stage 4 — exit module and full cycle (1 weekend, ≈ $115 cumulative)

Print chute, second idler_arm, wheels; buy the rest. Steps 25–37. 20
fixed-seed shuffles with predicted-vs-actual comparison; tune timeouts,
PWMs, `cal exit`. Record the cycle time.
Exit criterion: section E of `08-verification-checklist.md`.

## Stage 5 — endurance and statistics

200 shuffles over weeks of play; `cal show` statistics; output statistics;
card wear inspection; battery life measurement.

## Fallbacks

| Failure | Fallback within this design |
|---|---|
| Cards stop in the slot mouth | polish the fin faces (600 grit); raise `cal pwm nipe`; steepen `entry_slot_ang` to 55° (reprint feeder arms via params) |
| Cards catch on the shroud | shim the shroud outward 0.5 mm; felt strip inside the shroud; smaller `r_shroud_in` clearance not needed |
| Cards do not slide out at the exit | steepen `exit_slot_ang` to 210°; shorten the shutter/nip distance; raise `cal pwm nipx` |
| Wheel too heavy for the index time | bigger fin windows in params; 0.8 A Vref; NEMA 17 high-torque (60 N·cm) |
| Double feeds | cork retard on the gate; lighter lid |
| Index unreliable | bare TCST2103 interrupter in the index bracket instead of the module |
