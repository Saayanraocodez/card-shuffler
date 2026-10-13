// Poker chip test: steel washer for weight + NFC tag, sealed in by one pause.
// Three identical chips on one plate (A, B, C on the underside); load them differently:
//   A  washer + on-metal (ferrite-backed) NFC tag   the real chip
//   B  washer only                                  weight / balance check
//   C  NFC tag only                                 read-range control
// Stepped pocket: the washer sits on the floor around a centring boss, the tag lies on
// top of it (or on the ledge and boss when there is no washer). The tag faces the top.
// Measure your washer and tag and edit the numbers below; keep pocket_top on a layer line.
// Rendered with OpenSCAD 2021.01:  openscad -o poker_chip_test.stl poker_chip_test.scad
$fn = 96;

chip_d   = 39;     // casino size
layer    = 0.2;
floor_t  = 0.6;    // 3 layers under the pocket
roof_t   = 0.6;    // 3 layers over it

washer_od = 24;  washer_id = 8.4;  washer_t = 2.0;   // M8 fender washer, DIN 9021 (≈ 6 g)
tag_d     = 25;  tag_t     = 1.0;                    // Ø25 on-metal NTAG215
fit       = 0.3;   // diameter allowance (printed holes come out small)
headroom  = 0.2;   // tag top below the first roof layer, so the nozzle clears it

ledge_z    = floor_t + washer_t;                                      // 2.6
pocket_top = ceil((ledge_z + tag_t + headroom) / layer) * layer;      // 3.8: PAUSE before the layer above this
chip_t     = pocket_top + roof_t;                                     // 4.4
chamfer    = 0.4;

module chip(label) {
    difference() {
        hull() {
            translate([0, 0, chamfer]) cylinder(d=chip_d, h=chip_t - 2 * chamfer);
            cylinder(d=chip_d - 2 * chamfer, h=chip_t);
        }
        difference() {
            union() {
                translate([0, 0, floor_t]) cylinder(d=washer_od + fit, h=washer_t + 0.01);
                translate([0, 0, ledge_z]) cylinder(d=tag_d + fit + 0.3, h=pocket_top - ledge_z);
            }
            cylinder(d=washer_id - fit, h=ledge_z);                    // centring boss, flush with the washer top
        }
        translate([0, 16, -0.01]) mirror([1, 0, 0]) linear_extrude(0.4)   // reads correctly from below
            text(label, size=3.5, halign="center", valign="center");
    }
}

module chip_set() {
    for (i=[0:2]) translate([i * (chip_d + 6), 0, 0]) chip(["A", "B", "C"][i]);
}

chip_set();
echo(chip_t=chip_t, pause_after_z=pocket_top);
