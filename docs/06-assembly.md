# 6. Assembly instructions

Build order is chosen so that the feeder can be tested on its own before the
well module exists (see `09-prototype-plan.md`). Part names refer to
`cad/stl/`; dimensions to `cad/params.scad`; views to `cad/png/` and
`docs/img/`.

## 6.1 Tools

* 3D printer (0.4 mm nozzle), calipers, feeler gauge set (0.30–0.60 mm)
* Hex/Phillips drivers for M3 and M2, small needle-nose pliers, side cutters
* Soldering iron, wire strippers, JST-XH crimper (or pre-crimped leads)
* Files (flat, needle), 400/600-grit paper, tin snips or a hacksaw for brass
* Multimeter, USB cable for the ESP32
* Optional: M3 tap, 8 mm reamer or a spare 8 mm rod, heat-set insert tip

## 6.2 Fasteners

| Use | Fastener | Qty |
|---|---|---|
| Brackets, sleeve, servo brackets, column flange, bar halves, platform | M3 × 8 self-tapping into 2.6 mm holes | ≈ 36 |
| Gate clamp | M3 × 12 | 2 |
| Deck-to-deck flange | M3 × 12 + nut | 4 |
| Column flange | M3 × 10 + nut | 4 |
| NEMA 17 | M3 × 8 (through the 4 mm top plate) | 4 |
| T8 nut | M3 × 8 | 4 |
| Rod grub screws | M3 × 6 grub (or M3 × 8) | 4 |
| Idler axle | M3 × 50 + nyloc | 1 |
| Idler pivot | M3 × 10 | 2 |
| Bushing bracket cross screws | M3 × 20 | 2 |
| Lid hinge | M3 × 60 (or 3 mm rod 70 mm) | 1 |
| Roller set screws | M3 × 5 grub | 2 |
| Skirt mounting | M3 × 10 | 8 |
| Servo tabs, endstop | M2 × 8 | 6 |

## 6.3 Stage A — feeder module (steps 1–13)

1. **Post-process the feeder deck.** Run an M3 screw through every 2.6 mm
   hole. Sand the hopper floor and the guide plate between gate and flange
   smooth (cards slide on them). Check the gate window is clear.
2. **Gate block.** Round the lower inner edge of the gate block with 600-grit
   until it is a smooth R1 radius (this edge touches the second card).
   Fit it into the window from the outside, insert the two M3 × 12 clamp
   screws through the slots into the bosses, finger-tight.
3. **Rollers.** Fit three O-rings (ID 20 × 3.5) into each roller's grooves.
   Screw an M3 × 5 grub screw into the radial hole of the D-bore end so its
   tip is just below the bore. Check the stub axle is straight.
4. **Bushing bracket (−Y side).** Drop the two rollers' stub axles into the
   U-slots, then close each U with its M3 × 20 cross screw (the screw passes
   under the axle). Do not tighten the bracket to the deck yet.
5. **Motor bracket (+Y side).** Slide each N20 motor into its pocket from
   the outside with the shaft toward the rollers, wires outward. Hold the
   bracket against the plate's underside so the shafts enter the rollers'
   D-bores (rotate the rollers to align the flats), then screw both brackets
   to the plate's underside (4 × M3 × 8 each). Tighten the roller grub
   screws onto the shaft flats. Fit the motor retainer plates (2 × M3 × 8
   each).
6. **Check:** each roller turns freely by hand with the motor's gearbox
   resistance only, no wobble > 0.3 mm at the crown, crowns protrude
   0.7 (feed) and 0.5 mm (transport) above the plate — measure with a
   straight edge and feeler gauge across the slot.
7. **Beam B emitter.** Push a 3 mm IR LED into the vertical pocket at
   X = 19 from below (anode/cathode leads down), dome up, until it seats
   against the 2 mm aperture. Secure with a drop of hot glue on the leads
   under the plate. Solder 150 mm leads.
8. **Idler arm.** Fit an O-ring (9 × 2) on each idler wheel. Place the
   wheels in the forks with the M3 × 50 axle through the arm and wheels,
   nyloc on the end. Push the 3 mm phototransistor into the sensor boss from
   above (leads up), seating on the 2 mm aperture; hot-glue the leads.
   Mount the arm on the pivot blocks with 2 × M3 × 10 (arm must pivot
   freely). Drop a pen spring into each spring pocket; the springs bear
   against the underside of the lid bridge (added in step 12) — for now the
   arm rests by its own weight.
9. **Gate setting.** Loosen the clamp screws, lay a 0.45 mm feeler blade
   on the floor under the gate, press the block down onto it, tighten,
   remove the blade. Check with 0.40 (passes) and 0.55 (does not pass).
10. **Hinge and lid.** Line up the lid's two knuckles between the deck's
    three, push the M3 × 60 through (or a 3 mm rod). Stick a felt pad
    (30 × 50 mm) under the lid's boss. Put two dead AA cells in the troughs
    and tape them.
11. **Skirt (front).** Install the panel parts in the front skirt: start
    button (12 mm hole), status LED (6 mm), power switch (6.5 mm), charge
    jack (8.5 mm). Do not mount the deck on the skirt yet — the feeder is
    tested on the bench first.
12. **Idler spring bridge.** With the lid closed, the lid bridge sits over
    the idler arm; the springs should be compressed ~3 mm. If not, add a
    washer under each spring.
13. **Bench test of the feeder** (Stage 1 of the prototype plan): with the
    electronics on a breadboard (§6.5), run `feed` from the serial console
    with a deck loaded and a box catching the cards. Tune `cal pwm feed`,
    `cal pwm trans` and the gate until 100 consecutive single feeds succeed.

## 6.4 Stage B — well module (steps 14–27)

14. **Blades.** Cut two 40 × 22 mm pieces of 0.5 mm brass. File a symmetric
    bevel ≈ 1.5 mm long on both faces of one 40 mm edge (the entry edge),
    round the two corners R1, polish the bevel and both faces to 600 grit.
    Drill/punch two 3.2 mm holes 17.5 mm from the entry edge, 30 mm apart
    (the bar halves are the template: clamp a half to the brass and mark).
    Wipe with alcohol.
15. **Bars.** Lay a blade on a lower bar half with the entry edge toward
    the well side (the side with the pin slot away from you), 10.5 mm of
    blade overhanging the bar's inner face, screw the upper half on
    (2 × M3 × 10). Repeat for the second side (mirror). Check: blade flat,
    no rock, overhang 10.5 ± 0.3 mm on both.
16. **Well deck.** Deburr the two blade slots with a 1 mm feeler blade.
    Ream the beam-S apertures with a 1.6 mm drill. Slide each bar through
    its two guide loops from the outside; it must slide 8 mm freely and the
    blade tip must stop 1 mm inside the wall's inner face when the bar is
    fully out (adjust by loosening the clamp screws and sliding the blade).
    Loop a small rubber band from the bar's hook hole to the post.
17. **Servos.** Set each MG90S to its centre (1500 µs) using the console
    (`cal servo L ret 1500`, etc.) before fitting horns. Press a single-arm
    horn on pointing along +X (toward the column) and screw it. Fit an
    M2 × 8 screw through the horn's hole at r ≈ 8 mm from below so ~4 mm
    protrudes upward (this is the pin). Drop each servo into its bracket
    pocket (shaft up), secure with 2 × M2 tab screws.
18. **Servo brackets.** Slide the pin into the bar's underside slot through
    the plate's horn window while screwing the bracket to the plate's
    underside (4 × M3 × 8). Command `blades in` / `blades out`; adjust the
    endpoints with `cal servo` so that "in" gives 7 mm protrusion and "out"
    gives 1 mm inside the wall. Save.
19. **Beam S.** IR LED into one tube, phototransistor into the other
    (leads outward), hot-glue. **Beam W.** Phototransistor into the vertical
    hole above the slot from the top (leads up). The emitter goes into the
    sleeve (next step).
20. **Well sleeve.** Push the beam-W IR LED up the vertical hole in the
    sleeve's front wall until it seats under the 2 mm aperture; hot-glue.
    Stick 1.5 mm felt strips on both sides of the arm slot on the sleeve's
    and deck's inner back wall (recess provided). Screw the sleeve to the
    plate's underside (4 × M3 × 8), aligning the interior with the deck's.
21. **Column.** Push the two 8 mm rods up through the bottom plate holes
    into the top plate's blind holes; fix with grub screws top and bottom.
    Screw the microswitch to the inside of the back plate at the bottom
    (2 × M2), lever pointing up and toward the front.
22. **Carriage.** Press two LM8UU into the rear beam. Fit the T8 nut into
    the flange recess (flange up), 4 × M3 × 8. Screw the platform to the
    arm's spine from above (2 × M3 × 8, countersunk holes).
23. **Marry carriage and column.** Slide the carriage's bearings onto the
    rods from the top before the motor is fitted. Thread the lead screw
    down through the nut until it passes through the bottom clearance
    hole. Fit the coupler on the screw's top, then the NEMA 17 on the top
    plate (4 × M3 × 8) with its shaft in the coupler; tighten both coupler
    clamps.
24. **Column to deck.** From below the well deck, guide the carriage arm
    through the back wall's slot (deck + sleeve) while lowering the column
    flange onto the plate; 4 × M3 × 10 + nuts. Turn the coupler by hand:
    the platform must travel the full range without touching the walls.
25. **Bench test of the elevator and blades** (Stage 2): `home`, `z 0`,
    `z 20`, `cal home`, `blades in/out`. Then `cal knifetest` with a deck
    in the well (see §6.9).
26. **Join the modules.** Bolt the feeder deck's flange tabs to the well
    deck's (4 × M3 × 12 + nuts). The guide plate and the well slot bottom
    must be flush ±0.2 mm (shim with tape under a tab if not).
27. **Skirts.** Mount the rear skirt (4 × M3 × 10 into the bosses), then
    the front skirt; the column bottom must clear the rear skirt floor by
    ≥ 2 mm.

## 6.5 Stage C — electronics (steps 28–36)

28. **Perfboard.** Solder female headers for the ESP32, TMC2208 and DRV8833;
    the buck module on standoffs; the NPN with its 1 kΩ base resistor and
    three 150 Ω LED resistors; three 10 kΩ pull-downs; the 100 kΩ/33 kΩ
    divider with 100 nF; 100 µF across TMC2208 VM/GND and across DRV8833
    VM/GND; 470 µF on the 5 V rail; JST-XH headers J1–J16 (wiring diagram).
29. **Set the buck to 5.00 V** with nothing but the meter connected.
30. **Set TMC2208 Vref** to 0.9 V (≈ 0.65 A RMS) with the motor unplugged;
    tie MS1 and MS2 to 3V3 for 1/16 microstepping (or leave the module's
    default and set `STEPS_PER_MM` accordingly).
31. **Wire the harness**: stepper (4-pin JST), N20 motors (2-pin each),
    servos (3-pin each, 5 V + GND + signal), beams (4-pin each: LED+, LED−,
    PT collector 3V3, PT emitter → GPIO/10 kΩ), endstop (NC + COM), button,
    pixel (5 V via 1N4001, GND, data via 330 Ω), buzzer. Label every cable.
32. **Battery**: fuse holder in the positive lead, then the switch, then
    the perfboard's VBAT input; the charge jack directly across the pack's
    protected terminals (upstream of the switch). Velcro the pack to the
    front skirt's floor on a foam pad.
33. **Flash the firmware** (`pio run -t upload` or the Arduino IDE) with
    the battery disconnected; the ESP32 runs from USB.
34. **First power-up on battery**, USB connected, motors' JSTs unplugged:
    check `bat` reads within 0.2 V of the meter, `cal vbat <volts>`.
35. Plug in the sensors; `beams`; `cal beams`. Plug in the servos, N20s,
    stepper (power off when plugging the stepper).
36. Route cables away from the rollers and the elevator; zip-tie to the
    skirt walls; close the skirts (the decks are the lids).

## 6.6 Wiring diagram

`electronics/wiring-diagram.svg` — pinout table in `04-electronics.md` §4.3.

## 6.7 Calibration (first use)

Do these in order, with the serial console open.

| Step | Command | What it does / target |
|---|---|---|
| C1 | `cal vbat 12.30` (your meter reading) | battery gain |
| C2 | `cal beams` (machine empty) | thresholds = half of the clear signal; "WEAK" means realign the LED/PT |
| C3 | `home` | endstop found; platform at Z ≈ −26 |
| C4 | `cal home` (well empty) | platform finds beam S → Z origin defined |
| C5 | `blades in` / `blades out`, `cal servo …` | 7 mm in / 1 mm out, both sides equal ±0.3 |
| C6 | load a deck in the hopper; `probe`, `feed` ×20 | every feed "ok", B-block time steady ±10 %; adjust gate/PWM |
| C7 | put 30 cards in the well, `cal knifetest 15` | look through the finger notch: both blades in a card boundary, no card pushed in; if the blade lands consistently above the intended boundary use `cal knife -0.05`, below → `+0.05`; repeat with `cal knifetest 5` and `25` |
| C8 | `blades out`, `home`; remove the cards | |
| C9 | `test fixed 1`, deck in the hopper, press the button | machine prints PLAN; after the shuffle compare with `analyze_physical.py --compare` |

## 6.8 First-use checks

* Blades never move while the elevator moves (watch one full shuffle).
* Each card seats against the felt pad (look through the finger notch).
* No card is visible through the entry slot when the machine is idle.
* The stepper is cold to the touch after a shuffle; the servos are warm at
  most.
* Battery voltage after 5 shuffles has dropped by less than 0.15 V.
* `last` shows zero retries and zero double-feed suspects for a good deck.

## 6.9 Adjustments summary

| Symptom | Adjust |
|---|---|
| Two cards fed together | gate down 0.05 mm; less lid ballast; check the lip is smooth |
| Card not picked (FEED no card with a deck loaded) | gate up 0.05 mm; more ballast; clean O-rings with alcohol; raise `cal pwm feed` |
| Card stops before the pad (jam: well entry) | raise `cal pwm trans`; check the felt pad thickness; check the idler springs |
| Blade pushes a card instead of entering | `cal knife` by ±0.05; re-polish the bevel; check the well's stack is against the back wall (tilt, felt) |
| Cards hang on the blades after retract | more `GAP_CLOSE` (2.2 → 2.4) in `firmware/shuffler/config.h`; check bar travel returns fully |
| Stack measure fails | beam S LED/PT alignment, `cal beams` |
