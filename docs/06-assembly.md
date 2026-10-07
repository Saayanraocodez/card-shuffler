# 6. Assembly instructions

The build order lets you test the feeder on the bench first, then the wheel,
then everything else (`09-prototype-plan.md`). Part names refer to
`cad/stl/`, dimensions to `cad/params.scad`, and views to `cad/png/` and
`docs/img/`.

## 6.1 Tools

* **Required:** 3D printer (0.4 mm nozzle), calipers, feeler gauge set,
  hex and Phillips drivers (M3, M2), pliers, side cutters, soldering iron,
  JST-XH crimper or pre-crimped leads, files and 600-grit paper, hacksaw
  (for the M8 rod), multimeter, USB cable.
* **Optional:** M3 tap, 8 mm drill for reaming, heat-set insert tip.

## 6.2 Fasteners

| Use | Fastener | Qty |
|---|---|---|
| Brackets to the deck, shroud tabs, base covers, index bracket | M3 × 8 self-tapping (2.6 mm holes) | ≈ 40 |
| Tower feet to base | M3 × 10 | 12 |
| Feeder deck to the tower arms (down through the plate into the arms) | M3 × 12 | 4 |
| Chute to the tower cheeks (horizontal, from the towers' outer faces) | M3 × 20 | 4 |
| NEMA 17, through the +Y tower and the motor_mount | M3 × 40 | 4 |
| Spring bars to the idler pivot posts | M3 × 10 | 4 |
| Gate clamp | M3 × 12 | 2 |
| Idler axles | M3 × 50 + nyloc | 2 |
| Idler pivots | M3 × 10 | 4 |
| Bushing cross screws | M3 × 20 | 3 |
| Lid hinge | M3 × 60 (or 3 mm rod) | 1 |
| Roller set screws | M3 × 5 grub | 3 |
| Wheel on the rod | M8 nyloc nuts + washers | 4 + 6 |
| Servo flange | M2 × 8 | 2 |
| Shutter drive pin (through the horn) | M2 × 12 | 1 |

## 6.3 Stage A: feeder module (steps 1–12)

1. **Prepare the feeder deck.** Cut the three thin print ribs out of the
   gate window with flush cutters and file the window edges flat. Run an
   M3 screw through every 2.6 mm hole. Sand the hopper floor and the plate
   between the gate and the nip.
2. **Gate block.** Round the lip (the lower edge on the hopper side) to R1
   with 600-grit. Fit the block into the window from the wheel side. Put two
   M3 × 12 screws through the slots into the bosses, finger-tight.
3. **Rollers.** Fit 4 O-rings 20 × 3.5 on `roller_feed` and 4 O-rings
   10 × 3 on each `roller_nip`. Put a grub screw in each D-bore end.
4. **Bushing bracket (−Y side).** Drop the feed roller's and entry nip's
   stub axles into the U-slots, then close each with its M3 × 20.
5. **Motor bracket (+Y side).**
   1. Push the two N20 motors up into the pockets from below, shafts
      toward the rollers. The pockets are a push fit, so add a dab of hot
      glue.
   2. Hold both brackets under the plate so the shafts enter the rollers'
      D-bores.
   3. Screw each bracket to the plate's underside with 2 × M3 × 8.
   4. Tighten the roller grub screws onto the flats.
6. **Check** that the rollers turn freely and their crowns sit 0.7 mm
   (feed) and 0.5 mm (nip) above the plate.
7. **Beam B emitter.** Push a 3 mm IR LED into the pocket at r 106.5 from
   below, dome up against the aperture, and hot-glue it.
8. **Entry idler arm.**
   1. Fit O-rings on two `idler_wheel`s, then fit the wheels on an
      M3 × 50 axle with a nyloc. They sit over the roller's outer O-rings.
   2. Push the beam-B phototransistor into the arm's boss from above,
      leads up.
   3. Mount the arm on the pivot posts (2 × M3 × 10). It must pivot
      freely.
   4. Drop a pen spring into each spring pocket. Thread the
      phototransistor leads up through the hole in a `spring_bar`, then
      screw the bar across the tops of the pivot posts (2 × M3 × 10). The
      springs bear on it.
9. **Gate.** Put a 0.45 mm feeler under the block, press the block down
   and tighten. Check that 0.40 passes and 0.55 does not.
10. **Lid.** Fit the hinge with the M3 × 60. Put a felt pad under the boss.
    Drop two dead AA cells into the pockets in the top of the lid and tape
    over them.
11. **Bench test (Stage 1).** Put the electronics on a breadboard (§6.6).
    Clamp the deck at 50° with a box below the nip. Run `feed` 100 times
    with a deck loaded, tuning `cal pwm feed`, `cal pwm nipe` and the gate
    until 100 consecutive single feeds succeed. Beams E and S are on the
    towers, so on the bench `feed` reports a jam after beam B. Beam B's
    timing is what you are tuning here.
12. Hot-glue the beam leads to the plate's underside.

## 6.4 Stage B: wheel module (steps 13–24)

13. **Wheel.** Clean the slot mouths with a 1.5 mm feeler blade run down
    each slot. A card must slide to the hub and back out freely in all 54
    slots. Ream the bore to 8.6 mm if the rod does not pass.
14. **Rod.**
    1. Cut the M8 rod to 165 mm and deburr it.
    2. Put a washer and nyloc on one end, 8 mm in.
    3. Slide the `hub_spacer` onto the rod, then the wheel with its plain
       (−Y) disc against the spacer. Add a washer and nyloc on the other
       side.
    4. Clamp the wheel centred, with the index-tab disc toward the long end
       of the rod.
    5. Wrap one layer of tape on the thread where each bearing will sit.
15. **Towers.**
    1. Press a 608ZZ into each bearing pocket.
    2. Fit the beam E and beam S parts into the 4 mm bosses on the towers'
       inner faces: IR LEDs in the +Y tower, phototransistors in the −Y
       tower. Leads go out through the holes to the outer face. Hot-glue
       them.
    3. Fit the index LED and phototransistor in `index_bracket`, facing
       each other across the gap, and screw the bracket to the post on the
       +Y tower (2 × M3 × 8).
    4. **Motor, before the wheel goes in** (the bolts go in from the
       tower's inner face, which the wheel later covers). Fit the coupler
       on the NEMA 17 shaft. Hold `motor_mount` on the +Y tower's outer
       face and the motor on the mount. Push 4 × M3 × 40 from the inner
       face (heads into the counterbores) through the tower and the mount
       into the motor, and tighten them evenly.
16. **Shroud and shutter.** Lay the `shutter` blade inside `shroud_A`
    against the window, with its drive tab at the −Y end beside the shroud.
    It must slide 17.5° freely. Sand its outer face if it drags.
17. **Base.** Join `base_front` and `base_rear` on a flat surface. Screw
    the −Y tower's foot down (6 × M3 × 10). Stand the +Y tower loosely.
18. **Wheel into the towers.** Slide the rod through the −Y bearing (short
    end first). Bring the +Y tower, with its motor, onto the long end so
    the rod passes through the bearing into the coupler, and screw its foot
    down. The wheel must spin freely with axial play under 0.5 mm.
19. **Shroud segments.** Fit both segments between the towers (two tabs
    each end, M3 × 8). Load a few cards by hand at 12 o'clock and rotate
    them through the bottom. They should ride on the shroud and on the
    shutter's ramps without catching.
20. **Coupler.** Turn the wheel by hand to check the rod is centred in the
    coupler, then tighten the coupler's rod-side screws through the
    `motor_mount` access windows.
21. **Shutter servo.** Centre the MG90S at 1500 µs. Drop it into the
    −Y tower's pocket from the outside, flange on the outer face, and
    secure it with 2 × M2 × 8. Fit a single-arm horn inside the recess,
    pointing at mid-travel. Screw an M2 × 12 through the horn's hole at
    r ≈ 18 into the shutter tab's slot. Then set `cal shutter closed` (blade
    centred on the window) and `cal shutter open` (window clear).
22. **Bench test (Stage 2).** Run `home`, then `entry 0`. Check with a
    straight edge that fin 0's upper face lies 0.3° (0.4 mm at the tip)
    below where the feeder plate will sit, and adjust `cal home`. Run
    `entry 10` and `entry 40`: the wheel goes the short way. Run `scan` with
    a few cards placed by hand: the reported slots must match.
23. **Feeder on the wheel.** Screw the feeder deck to the tower arms
    (4 × M3 × 12, down through the plate into the arms). Push a card by hand through
    the nip into the slot at the entry. It must slide down to the hub, with
    beam E blocked and beam S clear (`beams`). Fine-tune `cal entry`
    (±0.1°) until cards enter cleanly in slots 0, 13, 27 and 40.
24. **Feed test.** Load 20 cards with `feed`, indexing the wheel with
    `entry k` each time, then run `scan`: 20 occupied, all at the intended
    slots.

## 6.5 Stage C: exit module (steps 25–29)

25. **Chute.**
    1. Fit the exit `roller_nip` with O-rings: stub in the −Y U-slot,
       closed with an M3 × 20 cross screw.
    2. Push the N20 into the +Y pocket and hot-glue it.
    3. Put the beam X LED into the floor pocket from below.
    4. Fit the second `idler_arm` (its boss takes the beam X
       phototransistor) with wheels and springs, then its `spring_bar`
       (2 × M3 × 10), as in step 8.
26. **Felt.** Stick a felt strip on the inside face of the chute end wall.
27. **Mount.** Slide the chute in between the towers from outside (it
    clears each tower face by 0.5 mm) and screw it to the tower cheeks:
    4 × M3 × 20, horizontal, from the towers' outer faces into the nip
    housing.
28. **Bench test (Stage 3).** With cards in the wheel, run `exit 5`,
    then `shutter open`, then `eject`. The card slides out and is
    pulled through, beam X blocks then clears, and the card lands in the
    chute. Adjust `cal exit` and `cal pwm nipx`. Repeat for slots 0, 27
    and 53, with `shutter close` before each `exit k`. Never turn the
    wheel with the shutter open: the cards over the window slide out
    and the move drags them across its edge.
29. **Full shuffle.** Run `test fixed 1` and press the button. Compare the
    result with `analyze_physical.py --compare`.

## 6.6 Stage D: electronics (steps 30–37)

30. **Perfboard.** Build it to `electronics/wiring-diagram.svg`:
    * ESP32, TMC2208 and two DRV8833 on female headers, plus the buck
      module;
    * NPN with a 470 Ω base resistor;
    * LED resistors: 150 Ω for B and X, 68 Ω for E and S, 330 Ω for the
      index LED, which is always on;
    * five 10 kΩ pull-downs;
    * 100 kΩ/33 kΩ divider with 100 nF;
    * capacitors: 100 µF × 3 and 470 µF;
    * JST-XH headers J1–J17.
31. **Set the voltages.** Set the buck to 5.00 V with nothing connected.
    Set the TMC2208 Vref to ≈ 1.4 V (≈ 1.0 A RMS on 0.11 Ω-sense modules)
    with the motor unplugged. Tie MS1 and MS2 to 3V3.
32. **Harness.** Make leads for:
    * stepper (4-pin) and three N20 motors (2-pin each);
    * servo (3-pin);
    * beams B, E, S and X (4-pin each);
    * index (4-pin);
    * button, pixel and buzzer.

    Label every lead.
33. **Battery.** Put the fuse in the positive lead, then the switch, then
    the VBAT input. Wire the charge jack across the pack's protected
    terminals. Velcro the pack in the front base box on a foam pad.
34. **Flash.** Flash the firmware with the battery disconnected (USB
    power).
35. **First power-up** on battery, motor connectors unplugged. Run `bat`,
    then `cal vbat <meter>`, then `beams`, then `cal beams`.
36. **Connect the rest.** Plug in the sensors, servo and N20s. Power off
    before plugging in the stepper.
37. **Finish.** Route cables along the towers and close the base covers.

## 6.7 Calibration (first use)

| Step | Command | Target |
|---|---|---|
| C1 | `cal vbat 12.30` (your meter reading) | battery gain |
| C2 | `cal beams` (machine empty) | thresholds; "WEAK" means realign that pair |
| C3 | `home` | index found |
| C4 | `entry 0` with a straight edge, then `cal home <deg>` | fin 0 face 0.4 mm below the plate plane at its tip |
| C5 | `cal shutter closed` / `cal shutter open` | blade over the window / window clear |
| C6 | deck in the hopper; `feed` ×20 with `entry k` | 20 × "ok", no double-feed suspects |
| C7 | `cal entry ±0.1` if cards stop in the mouth or land in a neighbour | beam E blocked and beam S clear after every feed |
| C8 | cards in the wheel; `exit k`; `shutter open`; `eject`; `shutter close` | pulled out cleanly; adjust `cal exit` |
| C9 | `test fixed 1`, then press the button | `last` shows 0 corrections; compare the deck |

## 6.8 First-use checks

* The wheel never moves while beam S or beam X is blocked. Push a card
  halfway into the mouth and try `jog 10`: it must be refused.
* No card is visible outside the shroud during loading.
* Every card lands flat in the chute and slides to the end wall.
* The stepper is warm, not hot, after 10 shuffles at 1 A.
* `last` shows zero corrections, retries and double-card suspects for a
  good deck.
* The battery drops less than 0.1 V over 5 shuffles.

## 6.9 Adjustments

| Symptom | Adjust |
|---|---|
| Two cards fed together | gate −0.05; less lid ballast; smooth the lip |
| Card not picked | gate +0.05; more ballast; clean the O-rings; raise `cal pwm feed` |
| Card stops in the mouth (ERROR 3, "stuck in the slot mouth") | `cal entry` −0.1 (fin lower); polish the fin lead-ins; raise `cal pwm nipe` |
| Corrections logged, all in one direction | `cal entry` by ±0.1° in that direction |
| Card does not slide out at the exit | `cal exit`; check the shutter opens fully; wipe the shroud |
| Eject jams | `cal pwm nipx`; idler springs |
| Index not found | index LED and phototransistor alignment; `cal home` |
| Stepper skips on long moves | Vref up to 1.6 V (1.1 A); lower `WHEEL_ACC_DPS2` to 3500 |
