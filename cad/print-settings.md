# Print settings and orientation

Every STL in `stl/` is already in its print orientation and rests on Z = 0.
Regenerate with `./render.sh stl` after editing `params.scad` / `shuffler.scad`.

| Part | Qty | Material | Layer | Infill | Supports | Notes |
|---|---|---|---|---|---|---|
| feeder_deck | 1 | PETG (PLA ok) | 0.2 | 25 % | none | brim; plate on the bed, walls up |
| well_deck | 1 | PETG | 0.2 | 25 % | none | brim; guide loops bridge 22 mm — fine at 0.2 mm |
| well_sleeve | 1 | PETG | 0.2 | 25 % | none | |
| column | 1 | PETG | 0.2 | 40 % | top plate only (66 mm bridge) or accept a sagging bridge and drill the holes | 119 mm tall; brim |
| carriage | 1 | PETG | 0.2 | 40 % | none | prints on its rear face |
| platform | 1 | PLA | 0.2 | 30 % | none | sand the top face smooth |
| roller | 2 | PLA | 0.15 | 30 % | none | axis vertical; clean the O-ring grooves |
| gate_block | 1 | PETG | 0.15 | 50 % | none | round the lip with 600-grit |
| knife_bar_lower / upper | 2 + 2 | PETG | 0.15 | 50 % | none | flat; the mating faces are the bed faces |
| servo_bracket | 2 | PETG | 0.2 | 30 % | none | |
| motor_bracket, bushing_bracket | 1 + 1 | PETG | 0.2 | 30 % | none | |
| motor_retainer | 2 | any | 0.2 | 50 % | none | |
| idler_arm | 1 | PETG | 0.2 | 30 % | none | printed upside down |
| idler_wheel | 2 | PLA | 0.15 | 50 % | none | |
| lid | 1 | PLA | 0.2 | 20 % | none | printed upside down; AA troughs bridge 15 mm |
| skirt_front, skirt_rear | 1 + 1 | PLA | 0.2–0.28 | 15 % | none | 2 perimeters are enough; brim |

Nozzle 0.4 mm, 3 perimeters unless noted, 4 top/bottom layers. Total ≈ 650 g.

Post-processing: ream the 8.15 mm rod holes in the column with an 8 mm rod;
run an M3 tap or a screw through every 2.6 mm hole once before assembly;
deburr the blade slots in the well side walls with a 1 mm feeler blade.
