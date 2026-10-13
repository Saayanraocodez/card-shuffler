// Bearing pocket fit test: one 608ZZ pocket per diameter, nominal (bearing_d) in the middle.
// Same pocket as the towers (7 mm deep, Ø9 through hole), printed the same way:
// pocket opening up, PETG, 0.2 mm, 3 perimeters, 40 % infill. See print-settings.md.
include <params.scad>

offsets = [-0.2, -0.1, 0, 0.1, 0.2];   // added to bearing_d (22.2): 22.0 … 22.4
labels  = ["22.0", "22.1", "22.2", "22.3", "22.4"];
pitch   = 30;
floor_t = 3;
h       = floor_t + bearing_t;          // 10

difference() {
    hull() for (x=[-9, (len(offsets) - 1) * pitch + 9]) for (y=[-13, 13]) translate([x, y, 0]) cylinder(r=4, h=h);
    for (i=[0:len(offsets) - 1]) translate([i * pitch, 0, 0]) {
        translate([0, 0, floor_t]) cylinder(d=bearing_d + offsets[i], h=bearing_t + 1);
        translate([0, 0, -1]) cylinder(r=4.5, h=h + 2);                                   // rod clearance, push-out hole
        translate([0, -14.5, h - 0.6]) linear_extrude(1)
            text(labels[i], size=3, halign="center", valign="center");
    }
}
