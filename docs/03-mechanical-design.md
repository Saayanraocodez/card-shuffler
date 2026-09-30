# 3. Mechanical design and printable parts

Source of truth: `cad/params.scad` (every dimension) and `cad/shuffler.scad`
(every part, the assembly, exploded and section views). STL files rendered
from them are in `cad/stl/`, views in `cad/png/`. `cad/render.sh` regenerates
everything with OpenSCAD 2021.01.

| View | File |
|---|---|
| Assembly | `cad/png/assembly.png` |
| Exploded assembly | `cad/png/exploded.png` |
| Section at the centreline | `cad/png/section.png`, annotated: `docs/img/mechanism-section.svg` |
| Feeder module exploded | `cad/png/feeder_exploded.png` |
| Well module exploded | `cad/png/well_exploded.png` |
| Insertion sequence | `docs/img/insertion-sequence.svg` |

## 3.1 Coordinate frame and overall layout

The "card frame" has X along the card travel (hopper → well), Y across the
card, Z normal to the cards; the card path plane is Z = 0. The whole card
frame is tilted 10° (well end low) and sits on two wedge-shaped skirts whose
bottoms are flat on the table.

| | mm |
|---|---|
| Footprint (skirts) | 261 × 126 |
| Height, hopper end (lid closed) | ≈ 155 |
| Height, column end (stepper on top) | ≈ 160 |
| Card path plane above the table at X = 0 | 90 |
| Mass (printed parts ≈ 650 g + hardware) | ≈ 1.4 kg [EST] |

Two modules bolt together at a vertical flange at X = 47 mm:

* **Feeder module** (X −87 … +47): hopper, gate, feed and transport rollers,
  idler arm, gate beam, lid. Electronics and battery in the front skirt below.
* **Well module** (X 47 … 176): well, blade bars, servos, elevator column.

## 3.2 Card path geometry

| Feature | X (mm) | Z (mm) | Note |
|---|---|---|---|
| Hopper interior | −81 … +10 | 0 … 50 | 65.0 wide (card 63.5 + 1.5), 91.0 long (card 88.9 + 2.1) |
| Feed roller axis | −4 | −12.8 | Ø27 crown, protrudes 0.7 above the floor |
| Gate lip (inner face of gate wall) | 10 | gap 0.30–0.60 | set by feeler gauge, nominal 0.45 |
| Gate beam B (vertical) | 19 | — | emitter in plate, phototransistor in idler arm |
| Transport roller axis | 32 | −13.0 | Ø27 crown, protrudes 0.5 |
| Idler wheels | 32 | axle 6.85 | Ø12 wheels with 9×2 O-rings, two at Y = ±20 |
| Well front wall (outer/inner face) | 47 / 50 | | entry slot Z −1.0 … +3.5 |
| Well slot beam W (vertical) | 48.5 | through the slot | Y = 12 |
| Well interior | 50 … 141 | −30 … +38 | 65 wide; felt pad 1.5 mm on the back wall |
| Blade mid-plane | 75.5 … 115.5 (blade width) | +2.5 | enters 7 mm from each side wall |
| Stack-top beam S (horizontal) | 117.5 | +14 | 1.6 mm apertures in both side walls |
| Platform | 52 … 138 | travel −25 … +22 | 45 wide (blades pass beside it), 4 thick |
| Lead screw axis / rods | 158 | | rods at Y = ±22, 8 × 120 mm |

Why these numbers:

* **Coasting.** The transport nip releases a card when its trailing edge
  passes X = 32; the leading edge is then at X ≈ 121, 18.5 mm short of the
  pad at 139.5. At 10° tilt and 0.2–0.25 m/s the card coasts 17–26 mm
  (μ ≈ 0.3 card-on-card) — enough, with margin, to seat on the felt pad
  every time. On a level path it would stop short.
* **Gap height.** Lower stack top at Z = −0.5, blade underside at +2.25:
  the incoming card (on the Z = 0 plane) has 0.5 mm below and 1.9 mm above.
* **Platform width 45.** Cards overhang the platform by 9.25 mm per side; the
  blades enter 7 mm, so they can slide under the bottom card (gap index 0)
  without touching the platform.
* **Slot height 4.5 mm (−1.0 … +3.5).** Tall enough for a curled card,
  short enough that no card of the lower stack (top at −0.5) can escape.

## 3.3 Feeder: single-card separation

The deck sits face down on the hopper floor. The bottom card rests on the
feed roller's three nitrile O-rings (μ ≈ 0.7 on card stock) and on the
floor. The 10° tilt slides the deck against the gate wall. The gate block's
lower edge is set 1.3–1.5 card thicknesses above the floor with a feeler
gauge; only the bottom card fits under it, and the block's rounded lip
holds back the second card. This is the separation method of every desktop
card dealer and most printers ("gate" or "retard lip").

Normal force on the bottom card = deck weight (≤ 90 g) + lid pressure. The
lid's hanging boss (with a felt pad) reaches the floor, so pressure does not
vanish as the deck empties; two AA cells in the lid's troughs give ≈ 70 g at
the boss through the lever ratio (lid CG 45 mm from the hinge, boss 70 mm).
Add washers to the troughs if the last cards feed unreliably; remove ballast
if cards feed double (too much force can push the second card under the
lip on very thin decks).

Feed force ≈ 0.7 × (0.7 … 1.6 N) ≈ 0.5 … 1.1 N. Card-to-card drag on the
second card ≈ 0.3 × N, which is what the lip resists.

After the gate, the card enters the transport nip: transport roller
(O-rings) below, two spring-loaded idler wheels above (two pen springs,
≈ 2 N total). Both rollers run from separate N20 motors so the feed roller
can stop as soon as beam B clears (trailing edge past the gate) while the
transport roller finishes the card; the next card is then already
"pre-staged" 10 mm past the gate, singulated, waiting.

**Thickness range.** The gate is adjustable from 0.30 to 0.60 mm (slots
8 mm long give more than needed). Standard cards 0.25–0.35 mm → set
0.42–0.45. Thick custom decks 0.40 mm → set 0.55. The rule is
t_max < gap < 2 × t_min.

## 3.4 Well, blades, elevator

* **Blades.** Two 40 × 22 × 0.5 mm brass blades (K&S sheet) or two pairs of
  0.45–0.55 mm feeler-gauge blades, each sandwiched between a printed lower
  and upper bar half (M3 × 10 × 2). Entry edge: file a symmetric bevel
  about 1.5 mm long on both faces (tip ≈ 0.1 mm), round the two corners,
  polish to 600 grit. The symmetric bevel makes the tip self-centre into
  the nearest card boundary when the elevator is up to ±0.15 mm off.
* **Bars.** 70 × 12 × 6 mm, sliding in two printed guide loops per side,
  8 mm travel. Driven by an MG90S below the plate through a 2 mm pin on the
  horn at r = 8 mm (±30° = 8 mm). A rubber band or light spring between the
  bar's hook hole and the post on the plate returns the bar outward when
  the servo is unpowered (blades out = safe default).
* **Well walls** are printed in two parts: the upper walls are part of the
  well deck (plate on the bed, walls up); the lower sleeve (Z −30 … −3)
  screws to the plate's underside. Both have the 7 mm arm slot in the back
  wall and the felt-pad recess.
* **Elevator.** T8×8 4-start lead screw (8 mm per turn; 400 µsteps/mm at
  1/16), flanged brass nut on the carriage's rear beam, two 8 mm rods with
  LM8UU bearings 44 mm apart, NEMA 17 on top of the column driving through
  a flexible coupler; the screw's lower end runs free in a clearance hole
  (Ender-3 style). The carriage arm passes through the 7 mm slot in the
  well's back wall; the platform screws to the arm's spine from above.
  Resolution 0.0025 mm; positioning repeatability is limited by the print,
  not the drive.
* **Endstop** at the column bottom, tripped by the rear beam at Z ≈ −26.
  The Z origin is then set by the beam-S auto-calibration (§3.7).
* **Stack-top measurement.** With blades out, the platform rises slowly
  until beam S blocks; the platform top is defined to be at Z = 14.0 at
  that instant (empty well, calibration) or the stack top is (with cards).
  Because the same threshold crossing is used for both, the beam's exact
  height and threshold cancel out of the thickness estimate.

## 3.5 Printed parts list

All parts print without supports in the orientation the STL is exported in
(the selector in `cad/shuffler.scad` applies the rotation; every STL rests on
Z = 0).

| # | Part | Qty | Size (mm) | Material | Notes |
|---|---|---|---|---|---|
| 1 | feeder_deck | 1 | 134 × 90 × 83 | PETG/PLA | hopper + plate + rails; plate on the bed |
| 2 | gate_block | 1 | 81 × 15.5 × 6 | PETG | flange face down; round the lip after printing |
| 3 | roller | 2 | Ø24 × 72 | PLA | axis vertical, stub up; 0.15 mm layers |
| 4 | motor_bracket | 1 | 68 × 27 × 19 | PETG | mating face on the bed |
| 5 | motor_retainer | 2 | 22 × 12 × 2.5 | any | |
| 6 | bushing_bracket | 1 | 68 × 7 × 19 | PETG | |
| 7 | idler_arm | 1 | 28 × 69 × 16 | PETG | printed upside down |
| 8 | idler_wheel | 2 | Ø12 × 6 | PLA | |
| 9 | lid | 1 | 103 × 71 × 79 | PLA | printed upside down (troughs bridge 15 mm) |
| 10 | well_deck | 1 | 129 × 131 × 63 | PETG | plate on the bed; guide loops bridge 22 mm |
| 11 | well_sleeve | 1 | 99 × 82 × 27 | PETG | |
| 12 | knife_bar_lower | 2 | 70 × 12 × 2.25 | PETG | 0.15 mm layers (blade seat) |
| 13 | knife_bar_upper | 2 | 70 × 12 × 3.25 | PETG | |
| 14 | servo_bracket | 2 | 48 × 18 × 29 | PETG | |
| 15 | column | 1 | 30 × 90 × 119 | PETG | upright; top plate bridges 66 mm — add a 0.4 mm sacrificial layer or enable supports for that bridge only |
| 16 | carriage | 1 | 24 × 62 × 74 | PETG | on its rear face |
| 17 | platform | 1 | 86 × 45 × 4 | PLA | flat; sand the top smooth |
| 18 | skirt_front | 1 | 134 × 126 × 102 | PLA | open top up |
| 19 | skirt_rear | 1 | 128 × 126 × 79 | PLA | open top up |

Print settings: 0.2 mm layers (0.15 for rollers and bar halves), 0.4 mm
nozzle, 3 perimeters, 4 top/bottom layers, 25 % infill (40 % for the
carriage and column), PETG at 235–245 °C for the parts that see load or
warmth (decks, column, carriage, brackets), PLA is acceptable everywhere for
a first prototype. Print the two decks with a brim.

## 3.6 Tolerances, clearances, adjustment points

| Fit | Nominal | Adjustment |
|---|---|---|
| Card in hopper / well width | 65.0 (card 63.5) | none needed; if cards bind, scale the print 100.3 % in Y |
| Gate gap | 0.45 | slotted clamp, feeler gauge (the critical adjustment) |
| Blade height vs boundary | ±0.15 | firmware `cal knife` (±0.025 mm steps), plus the print's ±0.15; `cal knifetest` to inspect |
| Bar in guide loop | 12.0 in 12.3 (Y), 6.0 in 6.4 (Z) | sand the bar if tight; the bar must slide freely under its own return spring |
| Blade slot in wall | 1.6 tall for a 0.5 blade | none (the bar defines the height) |
| LM8UU in carriage | 15.0 in 15.2 | press fit; drop of CA if loose |
| T8 nut flange recess | 22.5 | 4 × M3 |
| N20 in bracket pocket | 12.4 × 10.4 | retainer plate |
| Servo horn pin in bar slot | 2.0 in 2.8 (Y) × 4.8 (X) | the slot's X length absorbs the horn arc |
| Idler pressure | 2 pen springs | swap springs or add a washer under them |
| Idler axle height | 6.85 → 0.15 interference with the roller crown | none (arm floats) |
| Stack-top beam | 1.6 mm apertures | firmware threshold auto-set (`cal beams`) |
| Card entry slot | 4.5 tall | none |
| Skirt / deck alignment | 4 × M3 per module into bosses | oversize holes in the plates |

Print variation: the design uses ±0.3 mm sliding clearances everywhere a
printed part slides on a printed part, and every dimension that matters for
randomness (blade height, stack height) is calibrated by sensor, not trusted
from the print.

## 3.7 Calibration features designed into the parts

1. **Beam S defines Z.** `cal home` raises the empty platform until beam S
   blocks and declares that height Z = 14.0. The blade slot is 11.5 mm below
   the beam in the same print, so the blade plane is at 2.5 ± 0.15; the
   residual is trimmed with `cal knife`.
2. **`cal knifetest k`** puts gap k of a loaded stack at the blades and
   extends them so you can see (through the finger notches) whether the
   blade entered a boundary cleanly.
3. **Gate by feeler gauge**: no printed dimension is trusted for the gap.
4. **Beam thresholds** are auto-set from the clear readings.

## 3.8 Card protection

| Risk | Design response |
|---|---|
| Scratches | Rolling contact only (O-rings, idler O-rings); felt pad on the back wall; felt under the lid boss; all card-contact edges rounded (gate lip R1, slot edges chamfered); blades polished with rounded corners |
| Bent corners | Straight path; the well interior is only 1.5 mm wider than the card so a card cannot skew far enough to catch a corner; 10° tilt seats cards square on the pad; jam detection stops the motors within one poll (~1 ms) of a timeout |
| Excessive pressure | Idler ≈ 2 N; lid ≈ 0.7 N; blades carry the upper stack on 2 × 7 × 40 mm — pressure < 1 kPa |
| Exposed faces | Deck loaded face down under a lid; cards travel face down; the well is enclosed except the top (only the top card's back is visible) |
| Double feed | Gate lip at 1.3–1.5 t; lid pressure moderate; beam-B block-time heuristic; end-of-shuffle count check |
| Inconsistent feed | Positive lid pressure to the last card; pre-staged next card; sensor-timed feed (not open-loop timing); retries with reverse pulse |
| Print variation | Calibrated by sensors (§3.7); adjustable gate; sliding clearances 0.3 mm |

## 3.9 Things a builder may want to change

* `TILT` (10°): more tilt = more coasting margin, taller machine.
* `roller_d` and O-ring size: any crown Ø24–28 works if `feed_z`/`trans_z`
  follow (they do, automatically).
* `plat_w`: narrower gives more blade clearance but less card support.
* `col_top_z`, `rod_len`: for a pancake stepper or different rods.
* `n_cards_max`: 54 by default; the well and travel allow up to about 60.
