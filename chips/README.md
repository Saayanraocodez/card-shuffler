# Poker chip test (washer + NFC)

**Status: unprinted.** Weights and fits are estimates.

| File | What |
|---|---|
| `poker_chip_test.scad` / `.stl` | three Ø39 × 4.4 mm chips, labelled A, B, C on the underside |
| `test_plate.scad` / `.stl` | the three chips plus `cad/stl/bearing_fit_test.stl` on one plate (146 × 87 mm) |

## Parts

* **Washer:** M8 fender washer, DIN 9021 (Ø24 × Ø8.4 × 2.0 mm, ≈ 6 g).
* **NFC tag:** Ø25 mm NTAG215, **on-metal** (ferrite backed), ≈ 1 mm thick.
  An ordinary sticker tag lying on the washer will not read: the steel
  kills the field. The ferrite layer goes on the washer side.

Measure both with calipers. If they differ, change `washer_*` / `tag_*` in
`poker_chip_test.scad`, re-export, and use the new pause height it echoes.

## Print

PETG or PLA, 0.2 mm layers, 0.2 mm first layer, 100 % infill for the chips
(for weight; the bearing coupon keeps 40 %). No supports.

**One pause, after the layer that ends at 3.8 mm.** In Bambu Studio's
preview, drag the layer slider to the layer at **4.0 mm** (the first one
printed over the pockets), right-click, *Add Pause*. When it pauses, drop in:

| Chip | Insert | Tests |
|---|---|---|
| A | washer, then the tag on top, label side up | the real chip |
| B | washer only | weight and balance |
| C | tag only (it rests on the ledge and centre boss) | read range without steel |

Everything must sit below the rim; resume. The bearing coupon just waits.

## Check

* Weight [EST]: A ≈ 11–12 g (chip ≈ 4.8 g, washer ≈ 6.2 g, tag ≈ 0.5–1 g);
  casino clay chips are ≈ 9–11.5 g.
* NFC: write and read A and C with a phone (NXP TagInfo / NFC Tools) from
  the top face. A should read from the top; from the bottom it will not
  (steel in the way). If A does not read at all, the tag is not an on-metal
  type.
* Nothing rattles when shaken; the top over B (washer only) bridges a
  1.2 mm gap and may sag slightly.
