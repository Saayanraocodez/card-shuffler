# Print list and settings

Every STL in `stl/` is exported in its print orientation, resting on Z = 0:
load it, do not rotate it, do not add supports. Regenerate with
`./render.sh stl` after editing `params.scad` / `shuffler.scad`, then run
`python3 check_printability.py` (mesh, floating pieces, bed size and
support-free overhangs, slice by slice) and `./check_clearance.sh`.

**24 files, 30 pieces.** Printer: 0.4 mm nozzle, bed ≥ 220 × 220 mm (the
towers are 213 × 198 mm).

| # | File | Qty | Material | Layer | Infill | Size (mm) | Notes |
|---|---|---|---|---|---|---|---|
| 1 | wheel | 1 | PETG | 0.2 | 30 % | 155 × 155 × 104 | solid disc on the bed; ≈ 24 h, ≈ 375 g (the 54 fins print solid). Print it first and test it (below) |
| 2 | hub_spacer | 1 | PETG | 0.2 | 40 % | Ø30 × 6 | goes on the rod between the −Y disc and its nut |
| 3 | tower_R | 1 | PETG | 0.2 | 40 % | 213 × 198 × 16 | outer face down. Skirt off or ≤ 3 mm (it nearly fills a 220 bed) |
| 4 | tower_L | 1 | PETG | 0.2 | 40 % | 213 × 198 × 16 | as tower_R |
| 5 | motor_mount | 1 | PETG | 0.2 | 40 % | Ø50 × 28 | tower face down |
| 6 | shroud_A | 1 | PETG | 0.2 | 30 % | 99 × 112 × 116 | standing on its +Y edge; 5 mm brim (tall and thin) |
| 7 | shroud_B | 1 | PETG | 0.2 | 30 % | 99 × 112 × 116 | as shroud_A |
| 8 | shutter | 1 | PETG | 0.2 | 100 % | 17 × 25 × 97 | standing on its end; 5 mm brim; sand the outer face smooth |
| 9 | index_bracket | 1 | PETG | 0.2 | 40 % | 24 × 36 × 8 | back plate down |
| 10 | feeder_deck | 1 | PETG | 0.2 | 25 % | 106 × 142 × 61 | plate down. Has 3 thin ribs in the gate window: **cut them off** after printing |
| 11 | gate_block | 1 | PETG | 0.15 | 50 % | 16 × 107 × 6 | flange down |
| 12 | roller_feed | 1 | PLA | 0.15 | 30 % | Ø24 × 72 | D-bore end down, stub axle up |
| 13 | roller_nip | **2** | PLA | 0.15 | 30 % | Ø13 × 72 | as roller_feed. One for the entry nip, one for the exit nip |
| 14 | feeder_motor_bracket | 1 | PETG | 0.2 | 30 % | 56 × 28 × 19 | motor pockets open at the bed |
| 15 | feeder_bushing_bracket | 1 | PETG | 0.2 | 30 % | 58 × 8 × 19 | |
| 16 | idler_arm | **2** | PETG | 0.2 | 30 % | 24 × 95 × 16 | upside down. Entry and exit use the same part |
| 17 | idler_wheel | **4** | PLA | 0.15 | 50 % | Ø12 × 6 | two per idler arm; print 1–2 spares |
| 18 | spring_bar | **2** | PETG | 0.2 | 40 % | 16 × 110 × 4 | one per idler arm; screws onto the pivot posts |
| 19 | lid | 1 | PLA | 0.2 | 20 % | 83 × 97 × 62 | top face down, pressure boss up |
| 20 | chute | 1 | PETG | 0.2 | 25 % | 90 × 115 × 54 | floor down |
| 21 | base_front | 1 | PLA | 0.28 | 15 % | 193 × 210 × 45 | top face down (open side up) |
| 22 | base_rear | 1 | PLA | 0.28 | 15 % | 40 × 210 × 45 | as base_front |
| 23 | base_cover_front | 1 | PLA | 0.2 | 20 % | 186 × 203 × 2.5 | |
| 24 | base_cover_rear | 1 | PLA | 0.2 | 20 % | 33 × 203 × 2.5 | |

Only the feed roller drives without an idler: the hopper's weight and the
lid press the bottom card onto it. The two nips (entry and exit) each have
one `roller_nip` below the card and one `idler_arm` with two
`idler_wheel`s above it.

**All parts:** 3 perimeters, 4 top / 4 bottom layers, no supports. PETG
for everything that touches cards or carries load; PLA is fine for the
rollers (they carry O-rings), idler wheels, lid and base.

**Filament [EST]:** ≈ 0.9 kg PETG + ≈ 0.35 kg PLA (≈ 1.25 kg in total).
The wheel alone is ≈ 375 g. One 1 kg PETG spool is just enough if nothing
fails; buy a second if the wheel might need a reprint.

## Bearing fit test (optional, before the towers)

`stl/bearing_fit_test.stl` (from `bearing_fit_test.scad`): five copies of the
tower's 608ZZ pocket, Ø22.0 to 22.4 in 0.1 steps (22.2 is what the towers
use). PETG, 0.2 mm, 3 perimeters, 40 % infill, pocket side up, like the
towers. About 146 × 34 × 10 mm, ≈ 25 g [EST].

Press a 608ZZ into each pocket. Use the smallest pocket it goes into by hand
or with a small clamp without cracking the rim. The bearing must stay put when
the coupon is turned upside down and shaken; push it out through the Ø9 hole.
If that pocket is not 22.2, set `bearing_d` in `params.scad` to its size and
run `./render.sh stl` before printing the towers.

## Suggested order

1. **wheel**, then check it before printing anything else: slide a real
   card into all 54 slots. It must drop to the hub and come back out
   freely. If cards stick, widen the slots (`fin_t` in `params.scad`) and
   reprint before going on.
2. tower_R, tower_L, motor_mount, hub_spacer, shroud_A, shroud_B, shutter,
   index_bracket. Check the wheel turns freely on its rod inside the
   towers and shrouds (assembly Stage B).
3. feeder_deck, gate_block, rollers, brackets, idler arms and wheels,
   spring bars, lid, chute.
4. base parts last: they do not affect function.

## After printing

* **feeder_deck:** cut the three 1.2 mm ribs out of the gate window with
  flush cutters and file the window edges flat. The gate block must slide
  in without binding. Check the plate tip (the edge facing the wheel) is
  flat.
* **wheel:** run a 1.5 mm feeler blade down every slot. Ream the bore to
  8.6 mm if the rod does not pass.
* **towers:** press-fit test a 608 bearing; ream or sand the pocket if it
  is too tight.
* **every 2.6 mm hole:** run an M3 screw through it once (they are tapped
  by the screw).
* **shutter:** deburr the edges and sand the outer face; it slides on the
  shroud's inner face.
* **gate_block:** round the hopper-side lower edge (the lip) to R1 with
  600-grit.
* **rollers:** check the stub axle is straight. Reprint if it bent while
  printing (slow down or add a cooling pause for the last 8 mm).

## Dealer build (docs/11-dealer.md)

Print these instead of / as well as the parts above. `base_front` and `base_rear` (rows 21–22)
now carry corner bosses for the cover screws; in this branch `base_front`'s bosses stop 5 mm up for
the thicker turntable cover (`dealer = false` in `params.scad` restores the 2.5 mm version).

| File | Qty | Material | Layer | Infill | Size (mm) | Notes |
|---|---|---|---|---|---|---|
| tt_foot | 1 | PETG or PLA | 0.2 | 30 % | 180 × 182 × 21 | plate down; gear, race ring and magnet post up. 4 perimeters |
| tt_cover_front | 1 | PETG | 0.2 | 40 % | 186 × 203 × 11 | **replaces base_cover_front**; top face down, race ring up. 4 perimeters |
| tt_pinion | 1 | PETG | 0.12 | 100 % | Ø 20 × 11 | teeth down |
| chute_dealer | 1 | PETG | 0.2 | 25 % | 119 × 135 × 54 | **replaces chute**; floor down; sand the ramp smooth (600 grit) |
| throw_arm | 1 | PETG | 0.2 | 40 % | 46 × 122 × 26 | bridge down; pocket and U-slot up |
| roller_feed | +1 | PLA | 0.15 | 30 % | Ø 24 × 72 | the throw roller (as row 12) |

Filament for the dealer parts: ≈ 0.4 kg PETG [EST]. Check collisions with `./dealer_check.sh`.
