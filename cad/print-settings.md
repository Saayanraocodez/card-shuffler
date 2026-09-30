# Print settings and orientation

Every STL in `stl/` is exported in its print orientation resting on Z = 0.
Regenerate with `./render.sh stl` after editing `params.scad` / `shuffler.scad`.

| Part | Qty | Material | Layer | Infill | Supports | Notes |
|---|---|---|---|---|---|---|
| wheel | 1 | PETG | 0.2 | 30 % | none | axis vertical; ~24 h; check every slot with a card |
| tower_R / tower_L | 1 + 1 | PETG | 0.2 | 40 % | none | flat, 219 × 192 mm: needs a 220 mm bed (rotate 45° on smaller beds is not possible; scale is not allowed) |
| shroud_A / shroud_B | 1 + 1 | PETG | 0.2 | 30 % | none | standing on an end face (116 mm tall thin arcs): brim |
| shutter | 1 | PETG | 0.2 | 50 % | none | standing; sand the sliding faces |
| shutter_servo_bracket, index_bracket | 1 + 1 | PETG | 0.2 | 40 % | none | |
| feeder_deck | 1 | PETG | 0.2 | 25 % | none | plate on the bed; brim |
| gate_block | 1 | PETG | 0.15 | 50 % | none | flange down; round the lip |
| roller_feed, roller_nip ×2 | 3 | PLA | 0.15 | 30 % | none | axis vertical |
| feeder brackets, retainers | | PETG | 0.2 | 30 % | none | |
| idler_arm ×2, idler_wheel ×4 | | PETG / PLA | 0.2 / 0.15 | 30 / 50 % | none | arms printed upside down |
| lid | 1 | PLA | 0.2 | 20 % | none | upside down; troughs bridge 15 mm |
| chute | 1 | PETG | 0.2 | 25 % | none | floor on the bed; the nip housing bridges 20 mm |
| base_front, base_rear | 1 + 1 | PLA | 0.28 | 15 % | none | open side up |
| base_cover_front / rear | 1 + 1 | PLA | 0.2 | 20 % | none | |

Nozzle 0.4 mm, 3 perimeters, 4 top/bottom layers. Total ≈ 900 g.

Post-processing: run a 1.5 mm feeler blade down every slot of the wheel;
ream the wheel bore to 8.6 mm and the bearing pockets to a press fit; M3
screw through every 2.6 mm hole; deburr the shutter guides; check the
feeder plate tip is flat (it faces the wheel).
