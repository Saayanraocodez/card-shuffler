# 6. Assembly instructions

Build order lets the feeder be tested on the bench first, then the wheel,
then the rest (`09-prototype-plan.md`). Part names refer to `cad/stl/`;
dimensions to `cad/params.scad`; views to `cad/png/` and `docs/img/`.

## 6.1 Tools

3D printer (0.4 mm nozzle), calipers, feeler gauge set, hex/Phillips
drivers (M3, M2), pliers, side cutters, soldering iron, JST-XH crimper or
pre-crimped leads, files and 600-grit paper, hacksaw (M8 rod), multimeter,
USB cable. Optional: M3 tap, 8 mm drill for reaming, heat-set insert tip.

## 6.2 Fasteners

| Use | Fastener | Qty |
|---|---|---|
| Brackets, chute, deck, shroud tabs, base covers | M3 × 8 self-tapping (2.6 mm holes) | ≈ 50 |
| Tower feet to base | M3 × 10 | 12 |
| Feeder deck / chute to tower arms | M3 × 10 | 8 |
| NEMA 17 | M3 × 8 | 4 |
| Gate clamp | M3 × 12 | 2 |
| Idler axles | M3 × 50 + nyloc | 2 |
| Idler pivots | M3 × 10 | 4 |
| Bushing cross screws | M3 × 20 | 3 |
| Lid hinge | M3 × 60 (or 3 mm rod) | 1 |
| Roller set screws | M3 × 5 grub | 3 |
| Wheel on the rod | M8 nyloc nuts + washers | 4 + 6 |
| Servo tabs, index module, horn pin | M2 × 8 | 6 |

## 6.3 Stage A — feeder module (steps 1–12)

1. Post-process `feeder_deck`: run an M3 through every 2.6 mm hole; sand
   the hopper floor and the plate between the gate and the tip; check the
   gate window and the beam-E bridge aperture are clear.
2. `gate_block`: round the lip (the lower edge on the hopper side) to R1
   with 600-grit; fit into the window from the wheel side; two M3 × 12
   through the slots into the bosses, finger-tight.
3. Rollers: 4 O-rings 20 × 3.5 on `roller_feed`, 4 O-rings 10 × 3 on each
   `roller_nip`. Grub screw in each D-bore end.
4. `feeder_bushing_bracket` (−Y): drop the feed roller's and the entry
   nip's stub axles into the U-slots; close each with its M3 × 20.
5. `feeder_motor_bracket` (+Y): slide two N20 motors into the pockets from
   the outside, shafts toward the rollers; hold the bracket under the plate
   so the shafts enter the D-bores, screw both brackets to the plate's
   underside (4 × M3 × 8 each); tighten the roller grub screws onto the
   flats; fit the retainer plates.
6. Check: rollers turn freely, crowns 0.7 (feed) and 0.5 mm (nip) above
   the plate, no wobble > 0.3 mm.
7. Beam B: 3 mm IR LED into the pocket at r 105 from below (dome up to the
   aperture), hot-glue. Beam E: LED into the pocket at r 84.5 from below;
   phototransistor into the bridge from above (leads up); hot-glue both.
8. `idler_arm` (entry): O-rings on two `idler_wheel`s; M3 × 50 axle with a
   nyloc; phototransistor for beam B into the arm's sensor boss (leads
   up); mount on the pivot blocks (2 × M3 × 10); pen springs in the
   pockets.
9. Gate: 0.45 mm feeler under the block, press down, tighten; verify 0.40
   passes and 0.55 does not.
10. `lid`: hinge with the M3 × 60; felt pad under the boss; two dead AA
    cells in the troughs, taped.
11. Bench test (Stage 1): with the electronics on a breadboard (§6.6),
    clamp the deck at 50° with a box below the tip; `feed` ×100 with a
    deck loaded; tune `cal pwm feed`, `cal pwm nipe` and the gate until
    100 consecutive single feeds succeed. (Beam E will not see a card on
    the bench; expect "card not seen in slot" — the B-beam timing is what
    you are tuning.)
12. Hot-glue the beam leads to the plate's underside and route them toward
    the tower side.

## 6.4 Stage B — wheel module (steps 13–24)

13. `wheel`: clean the slot mouths of stringing with a 1.5 mm feeler blade
    run down each slot; check every slot with a card: it must slide to the
    hub and back out freely in all 54 slots. Ream the bore to 8.6 mm if
    the rod does not pass.
14. Cut the M8 rod to 165 mm; deburr. Slide a washer and nyloc onto one
    end 8 mm from the end; slide the rod through the wheel; washer + nyloc
    on the other side; tighten both nuts against the hub bosses so the
    wheel is centred with the +Y disc (the one with the index tab) on the
    long end of the rod (the end that gets the coupler).
15. Towers: press a 608ZZ into each bearing pocket (inner face). Run M3s
    through the arm holes. Screw `index_bracket` to the +Y tower's inner
    face at 12 o'clock with its slot at r ≈ 82; fit an IR LED and
    phototransistor in its pockets facing each other across the slot
    (or the slotted module, if its slot fits the tab: 3 mm thick,
    r 72–84).
16. `shroud_A` (window) and `shroud_B`: fit the `shutter` between the
    guide flanges of shroud_A; it must slide 15° freely.
17. Base boxes: join `base_front` and `base_rear` on a flat surface. Stand
    the −Y tower on its foot holes (6 × M3 × 10). Stand the +Y tower
    loosely.
18. Slide the wheel's rod through the −Y bearing (short end), then bring
    the +Y tower onto the long end and screw its foot down. The wheel must
    spin freely by hand with no axial play > 0.5 mm (adjust the nuts).
19. Fit the shroud segments between the towers (4 tabs each, M3 × 8 into
    the tower crossbar/seats). Rotate the wheel: no card edge may touch the
    shroud with cards seated (load a few cards by hand at 12 o'clock and
    rotate through the bottom: they should slide out ≤ 1.5 mm and ride
    on the shroud).
20. Coupler on the rod's long end; NEMA 17 on the standoff (4 × M3 × 8)
    with its shaft in the coupler; tighten both coupler clamps through the
    access windows.
21. `shutter_servo_bracket` with the MG90S on the −Y tower's outer face at
    the exit; M2 × 8 pin through the horn's outer hole at r ≈ 15 pointing
    inward through the tower's slot into the shutter's tongue slot.
    Command `shutter open` / `shutter close`; set `cal shutter` endpoints
    so "closed" is flush with the shroud and "open" clears the window.
22. Bench test (Stage 2): `home`; `entry 0` → look from above: fin 0's
    upper face should be coplanar with where the feeder plate will sit
    (use a straight edge against the tower arm); adjust `cal home`.
    `entry 10`, `entry 40`: the wheel goes the short way. `scan` with
    a few cards placed by hand: reported slots match.
23. Feeder deck onto the tower arms (4 × M3 × 10 from below through the
    arms). Push a card by hand through the nip into the slot at the entry:
    it must slide down and stop on the hub with beam E blocked
    (`beams`). Fine-tune `cal entry` (±0.2°) until a card enters cleanly
    in slots 0, 13, 27, 40.
24. Load 20 cards with `feed` (wheel indexed by `entry k` each time), then
    `scan`: 20 occupied, all at the intended slots.

## 6.5 Stage C — exit module (steps 25–29)

25. `chute`: exit `roller_nip` with O-rings into the nip housing (stub in
    the −Y U-slot, M3 × 20 cross screw; N20 into the +Y pocket, retainer).
    Beam X LED into the floor pocket from below, phototransistor into the
    bridge from above. Second `idler_arm` on the pivot blocks with its
    wheels, springs, axle.
26. Felt strip on the chute end wall (inside face).
27. Chute onto the tower arms (4 × M3 × 10 from below).
28. Bench test (Stage 3): with cards in the wheel, `shutter open`,
    `exit 5`, `eject`: the card slides out, is pulled through, beam X
    blocks then clears, the card lands in the chute. Adjust `cal exit`
    and `cal pwm nipx`. Repeat for slots 0, 27, 53.
29. Run `test fixed 1`, press the button: full shuffle. Compare with
    `analyze_physical.py --compare`.

## 6.6 Stage D — electronics (steps 30–37)

30. Perfboard per `electronics/wiring-diagram.svg`: ESP32, TMC2208, two
    DRV8833 on female headers; buck; NPN + 1 kΩ; three 150 Ω; three 10 kΩ
    pull-downs; 100 kΩ/33 kΩ + 100 nF; 100 µF ×3, 470 µF; JST-XH J1–J16.
31. Buck to 5.00 V (nothing connected). TMC Vref 1.0 V (motor unplugged);
    MS1/MS2 to 3V3.
32. Harness: stepper (4-pin), three N20 (2-pin each), servo (3-pin), beams
    B/E/X (4-pin each), index module (3-pin), button, pixel, buzzer. Label.
33. Battery: fuse in the positive lead, then the switch, then VBAT in; the
    charge jack across the pack's protected terminals. Velcro the pack in
    the front base box on a foam pad.
34. Flash the firmware with the battery disconnected (USB power).
35. First power-up on battery with the motor connectors unplugged:
    `bat` → `cal vbat <meter>`; `beams` → `cal beams`.
36. Plug in the sensors, servo, N20s, stepper (power off for the stepper).
37. Route cables along the towers; close the base covers.

## 6.7 Calibration (first use)

| Step | Command | Target |
|---|---|---|
| C1 | `cal vbat 12.30` (meter) | battery gain |
| C2 | `cal beams` (machine empty) | thresholds; "WEAK" → realign |
| C3 | `home` | index found |
| C4 | `entry 0` + straight edge; `cal home <deg>` | fin 0 face coplanar with the feeder plate |
| C5 | `cal shutter closed/open` | flush / clear |
| C6 | deck in hopper; `feed` ×20 with `entry k` | 20 × "ok", no double-feed suspects |
| C7 | `cal entry ±0.2` if beam E is not blocked after feeds | card seen in the slot every time |
| C8 | cards in the wheel; `shutter open`; `exit k`; `eject` | pulled out cleanly; `cal exit` |
| C9 | `test fixed 1`, button | `last` shows 0 corrections; compare the deck |

## 6.8 First-use checks

* Wheel never moves while a card is in the nip (watch a shuffle).
* No card is visible outside the shroud during loading.
* Every card lands flat in the chute and slides to the end wall.
* Stepper cool after a shuffle; N20s warm at most.
* `last`: zero corrections, zero retries, zero double-feed suspects on a
  good deck.
* Battery drop after 5 shuffles < 0.1 V.

## 6.9 Adjustments summary

| Symptom | Adjust |
|---|---|
| Two cards fed together | gate −0.05; less lid ballast; smooth the lip |
| Card not picked | gate +0.05; more ballast; clean O-rings; `cal pwm feed` up |
| Card stops in the mouth (not seated) | `cal entry`; check the fin face is clean; `cal pwm nipe` up |
| Corrections logged, all in one direction | `cal entry` by ±0.1° in that direction |
| Card does not slide out at the exit | `cal exit`; check the shutter opens fully; wipe the shroud |
| Eject jams | `cal pwm nipx`; idler springs |
| Index not found | sensor alignment with the tab; `cal home` |
