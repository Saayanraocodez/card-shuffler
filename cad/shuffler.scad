// shuffler.scad — WHEEL card shuffler: all printed parts + assembly.
//
//   openscad -D 'part="wheel"' -o stl/wheel.stl shuffler.scad
//   openscad -D 'part="assembly"' [-D explode=30] [-D section=1] -o png/... shuffler.scad
//
// Every "part=" selection is emitted in its print orientation resting on Z = 0.
// See params.scad for the frame and every dimension.

include <params.scad>

part = "assembly"; explode = 0; section = 0;

// ------------------------------------------------------------------ helpers
module box(p0, p1) { translate([min(p0[0],p1[0]), min(p0[1],p1[1]), min(p0[2],p1[2])])
    cube([abs(p1[0]-p0[0]), abs(p1[1]-p0[1]), abs(p1[2]-p0[2])]); }
module hole_z(x, y, d, z0, z1) { translate([x,y,z0]) cylinder(d=d, h=z1-z0); }
module hole_y(x, z, d, y0, y1) { translate([x,y0,z]) rotate([-90,0,0]) cylinder(d=d, h=y1-y0); }
module hole_x(y, z, d, x0, x1) { translate([x0,y,z]) rotate([0,90,0]) cylinder(d=d, h=x1-x0); }
// Fin-local frame at angle a: local X = radial outward, local Y = wheel axis,
// local Z = normal to the fin face on the card side (side=+1: CCW face, side=-1: CW face).
module at_fin(a, side=1) { rotate([0,-a,0]) mirror([0,0, side < 0 ? 1 : 0]) children(); }
module cyl_y(r, y0, y1) { translate([0,y0,0]) rotate([-90,0,0]) cylinder(r=r, h=y1-y0); }
module arc2d(r0, r1, a0, a1, n=48) { polygon(concat([for (i=[0:n]) let(a=a0+(a1-a0)*i/n) [r0*cos(a), r0*sin(a)]],
                                                  [for (i=[n:-1:0]) let(a=a0+(a1-a0)*i/n) [r1*cos(a), r1*sin(a)]])); }
module arc_y(r0, r1, a0, a1, y0, y1) { translate([0,y1,0]) rotate([90,0,0]) linear_extrude(y1-y0) arc2d(r0, r1, a0, a1); }

// ================================================================== WHEEL
module fin() {   // one fin in its local frame: CCW face on local Z = 0, body toward -Z
    difference() {
        box([r_hub - hub_wall + 0.5, -cage_w/2, -fin_t], [r_tip, cage_w/2, 0]);
        for (x=[[30, 47], [52, 71]], y=[[-38, -8], [8, 38]]) box([x[0], y[0], -fin_t - 1], [x[1], y[1], 1]);   // lightening windows
    }
}
module disc(y0, y1, with_tab=false) {
    difference() {
        union() {
            arc_y(disc_ring_in, r_tip, 0, 360, y0, y1);
            cyl_y(hub_boss_r, y0, y1);
            for (i=[0:n_spokes-1]) rotate([0, -i*360/n_spokes, 0]) box([0, y0, -4], [r_tip - 2, y1, 4]);
            if (with_tab) arc_y(index_r0, index_r1, index_ang - 3, index_ang + 3, y0, y1 + index_w);
        }
        cyl_y(bore_d/2, y0 - 1, y1 + index_w + 1);
    }
}
module wheel() {
    difference() {
        union() {
            difference() { cyl_y(r_hub, -cage_w/2, cage_w/2); cyl_y(r_hub - hub_wall, -cage_w/2 - 1, cage_w/2 + 1); }   // hub tube
            for (k=[0:n_slots-1]) rotate([0, -k*pitch_deg, 0]) fin();
            disc(-cage_w/2 - disc_t, -cage_w/2);
            disc(cage_w/2, cage_w/2 + disc_t, true);
            cyl_y(hub_boss_r, -cage_w/2 - disc_t - hub_boss_len, -cage_w/2);       // hub bosses
            cyl_y(hub_boss_r, cage_w/2, cage_w/2 + disc_t + hub_boss_len);
        }
        cyl_y(bore_d/2, -100, 100);
    }
}

// ================================================================== SHROUD + SHUTTER
module shroud_segment(a0, a1, window=false) {
    difference() {
        union() {
            arc_y(r_shroud_in, r_shroud_in + shroud_t, a0, a1, -shroud_w/2, shroud_w/2);
            // end tabs to the towers (both ends, both sides)
            for (a=[a0 + 4, a1 - 4], s=[-1,1]) rotate([0,-a,0]) box([r_shroud_in, s*(shroud_w/2), -6], [r_shroud_in + 10, s*(tower_y_in), 6]);
            if (window) {   // shutter guide flanges on both sides of the window
                for (s=[-1,1]) arc_y(r_shroud_in + shroud_t, shutter_r0 + shutter_t + 2, win_a0 - shutter_open_deg - 3, win_a1 + 3, s*(shroud_w/2 - 6), s*(shroud_w/2));
                for (s=[-1,1]) arc_y(shutter_r0 + shutter_t, shutter_r0 + shutter_t + 2, win_a0 - shutter_open_deg - 3, win_a1 + 3, s*(shroud_w/2 - 12), s*(shroud_w/2 - 6));
            }
        }
        if (window) arc_y(r_shroud_in - 1, r_shroud_in + shroud_t + 1, win_a0, win_a1, -shroud_w/2 + 6, shroud_w/2 - 6);
        for (a=[a0 + 4, a1 - 4], s=[-1,1]) rotate([0,-a,0]) hole_y(r_shroud_in + 5, 0, hole_m3_free, s*(tower_y_in - 8), s*(tower_y_in + 1));
    }
}
module shutter() {   // curved plate sliding on the shroud; a tongue at -Y takes the servo horn pin
    difference() {
        union() {
            arc_y(shutter_r0, shutter_r0 + shutter_t, win_a0 - 1.5, win_a0 + shutter_span, -shroud_w/2 + 7, shroud_w/2 - 7);
            rotate([0, -(win_a0 + shutter_span/2), 0]) box([shutter_r0, -tower_y_in + 3, -6], [shutter_r0 + shutter_t, -shroud_w/2 + 8, 6]);   // tongue
        }
        rotate([0, -(win_a0 + shutter_span/2), 0]) hull() for (dz=[-4, 4]) translate([0, 0, dz]) hole_x(-tower_y_in + 9, 0, 2.4, shutter_r0 - 1, shutter_r0 + shutter_t + 1);   // pin slot (tangential play for the horn arc)
    }
}

// ================================================================== TOWERS
module tower(side=1) {   // side=+1: motor side (+Y). Plate in the XZ plane, thickness tower_t, inner face at |Y| = tower_y_in
    y0 = side > 0 ? tower_y_in : -tower_y_in - tower_t; y1 = y0 + tower_t;
    yi = side > 0 ? y0 : y1;   // inner face
    yo = side > 0 ? y1 : y0;   // outer face
    difference() {
        union() {
            hull() { cyl_y(30, y0, y1); box([tower_x0, y0, tower_z_bot], [tower_x1, y1, tower_z_bot + 8]); }   // upright
            box([-102, y0, -8], [102, y1, 8]);                                                                  // crossbar (shroud ends)
            at_fin(entry_fin_ang, 1)  box([40, y0, -14], [gate_r + 20, y1, -plate_t]);                          // feeder arm (plate sits on it)
            at_fin(exit_fin_ang, -1)  box([40, y0, -chute_drop - plate_t - 11], [chute_r0 + 16, y1, -chute_drop - plate_t]);   // chute arm
            if (side > 0) cyl_y(nema_w/2 + 4, y1, y1 + motor_standoff);                                          // motor standoff
        }
        cyl_y(bearing_d/2, side > 0 ? yi - 1 : yi - bearing_t, side > 0 ? yi + bearing_t : yi + 1);              // 608 pocket from the inner face
        cyl_y(4.5, y0 - 1, y1 + motor_standoff + 1);                                                              // rod clearance
        if (side > 0) {
            cyl_y(13, y1 - 1, y1 + motor_standoff + 1);                                                           // coupler bore
            cyl_y(nema_boss_d/2 + 0.5, y1 + motor_standoff - 3, y1 + motor_standoff + 1);
            for (dx=[-nema_bolt/2, nema_bolt/2], dz=[-nema_bolt/2, nema_bolt/2]) translate([dx,0,dz]) cyl_y(hole_m3_free/2, y1 + motor_standoff - 10, y1 + motor_standoff + 1);
            for (a=[45, 135, 225, 315]) rotate([0,-a,0]) box([12, y1 + 5, -5], [nema_w/2 + 5, y1 + motor_standoff - 6, 5]);   // coupler access windows
            for (s=[-1,1]) rotate([0, -index_ang, 0]) translate([index_r1 + 4 + s*8, 0, 0]) cyl_y(hole_m3/2, yi - 1, yi + 8);   // index bracket screws
        } else {
            arc_y(shutter_r0 + 0.5, shutter_r0 + 3.5, win_a0 - 3, win_a0 + shutter_span + shutter_open_deg + 3, y0 - 1, y1 + 1);   // pin slot for the shutter servo
            rotate([0, -shutter_servo_ang, 0]) for (dx=[-18, 18]) translate([shutter_servo_r, 0, dx]) cyl_y(hole_m3/2, yo - 8, yo + 1);   // servo bracket screws
        }
        at_fin(entry_fin_ang, 1) for (r=[nipE_r + 6, gate_r + 12]) translate([r, 0, 0]) cyl_y(hole_m3/2, y0 - 1, y1 + 1);           // feeder deck screws
        at_fin(exit_fin_ang, -1) for (r=[nipX_r + 4, chute_r0 + 14]) translate([r, 0, 0]) cyl_y(hole_m3/2, y0 - 1, y1 + 1);        // chute screws
        for (a=[shroud_a0 + 4, 270 - 4, 270 + 4, shroud_a1 - 4]) rotate([0,-a,0]) translate([r_shroud_in + 5, 0, 0]) cyl_y(hole_m3/2, y0 - 1, y1 + 1);   // shroud tabs
        for (x=[-100, -62, -30, 10, 42, 82]) hole_z(x, (y0 + y1)/2, hole_m3_free, tower_z_bot - 1, tower_z_bot + 9);   // foot screws
        for (p=[[-60, -70], [-30, -45], [30, -45], [60, -70], [0, -75]]) hole_y(p[0], p[1], 18, y0 - 1, y1 + 1);        // weight reduction
    }
}
module index_bracket() {   // holds an IR LED / phototransistor pair straddling the disc's home tab; screws to the +Y tower's inner face
    difference() {
        box([-14, 0, -6], [14, 11, 6]);
        box([-4, -1, -7], [4, 5.5, 7]);                        // tab passes here (tab at Y 49..52 → bracket Y 47..58)
        hole_y(0, 0, 3.3, -1, 12);                             // LED / PT pockets on either side of the tab slot (drill through)
        for (x=[-10, 10]) hole_y(x, 0, hole_m3_free, -1, 12);
    }
}

// ================================================================== FEEDER (in the entry fin frame; card plane local Z = 0, cards on +Z)
side_in = hop_w/2; side_out = side_in + wall;        // 45.5 / 48.5
feed_z  = feed_protr - feed_roller_d/2;              // -12.8
nip_z   = nip_protr - nip_roller_d/2;                // -7.5

module feeder_deck() {
    r1 = gate_r + hop_len + wall;                     // hopper back wall outer (177.5)
    difference() {
        union() {
            box([deck_tip_r, -(tower_y_in + tower_t + 1), -plate_t], [r1, tower_y_in + tower_t + 1, 0]);        // plate (rests on the tower arms)
            box([gate_r + hop_len, -side_out, 0], [r1, side_out, hop_h]);                                       // back wall
            for (s=[-1,1]) box([gate_r, s*side_in, 0], [r1, s*side_out, hop_h]);                               // side walls
            box([gate_r - wall, -side_out, gate_win_h], [gate_r, side_out, hop_h]);                            // gate wall above the window
            for (s=[-1,1]) box([gate_r - wall, s*side_out, 0], [gate_r + 6, s*(side_out + 4.5), 12]);          // gate clamp bosses
            for (s=[-1,1]) box([deck_tip_r, s*side_in, 0], [gate_r - wall, s*side_out, rail_h]);               // rails to the wheel
            for (s=[-1,1]) box([nipE_r + 8, s*(side_out + 0.5), 0], [nipE_r + 16, s*(side_out + 6.5), 16]);    // idler pivot blocks
            for (y=[-24, 0, 24]) translate([r1, y - 4, hop_h]) cube([8, 8, 8]);                                 // hinge knuckles
            box([beamE_r - 5, -side_out, rail_h], [beamE_r + 5, side_out, rail_h + 8]);                      // beam E bridge across the rails
        }
        hole_y(r1 + 4, hop_h + 4, 3.3, -40, 40);                                                                 // hinge pin
        box([feed_r - 10, -side_in - 0.1, -plate_t - 1], [feed_r + 10, side_in + 0.1, 1]);                       // feed roller slot
        box([nipE_r - 6, -side_in - 0.1, -plate_t - 1], [nipE_r + 6, side_in + 0.1, 1]);                         // nip roller slot
        box([gate_r - wall - 1, -side_in, -1], [gate_r + 1, side_in, gate_win_h]);                               // gate window
        for (s=[-1,1]) hole_x(s*(side_out + 2.25), 7, hole_m3, gate_r - wall - 1, gate_r + 7);                  // gate clamp screws (along r)
        hole_z(beamB_r, 0, 3.3, -plate_t - 1, -1.0); hole_z(beamB_r, 0, 2.0, -plate_t - 1, 1);                   // beam B emitter
        hole_z(beamE_r, 0, 3.3, -plate_t - 1, -1.0); hole_z(beamE_r, 0, 2.0, -plate_t - 1, 1);                   // beam E emitter (below)
        hole_z(beamE_r, 0, 3.3, rail_h + 3, rail_h + 9); hole_z(beamE_r, 0, 2.0, rail_h - 1, rail_h + 4);        // beam E detector (in the bridge)
        for (s=[-1,1]) hole_y(nipE_r + 12, 10, hole_m3_free, s*(side_out - 1), s*(side_out + 8));                // idler pivot
        for (x=[feed_r - 12, feed_r + 12, nipE_r - 10, nipE_r + 10], s=[-1,1]) hole_z(x, s*(side_out + 3), hole_m3, -plate_t - 1, 1);   // bracket screws
        for (r=[nipE_r + 6, gate_r + 12], s=[-1,1]) hole_z(r, s*(tower_y_in + tower_t/2), hole_m3_free, -plate_t - 1, 1);            // tower arm screws
        translate([r1 - 1, 0, hop_h + 1]) rotate([0,90,0]) cylinder(d=24, h=wall + 2);                          // finger recess
    }
}
module gate_block() {
    difference() {
        union() {
            box([gate_r - wall, -side_in + 0.3, 0], [gate_r, side_in - 0.3, gate_win_h - 0.5]);
            box([gate_r - wall - 3, -(side_out + 5), 0], [gate_r - wall, side_out + 5, 12]);
        }
        for (s=[-1,1]) translate([gate_r - wall - 4, s*(side_out + 2.25), 6]) rotate([0,90,0]) linear_extrude(6) hull() { translate([-3,0]) circle(d=3.4); translate([3,0]) circle(d=3.4); }
        translate([gate_r + 0.5, -side_in - 1, -0.5]) rotate([-90,0,0]) cylinder(r=1.0, h=2*side_in + 2);   // rounded lip on the wheel side
    }
}
module roller(hub_d, root_d, cs, ys, len=roller_len) {
    difference() {
        union() { cylinder(d=hub_d, h=len); translate([0,0,-roller_stub]) cylinder(d=4, h=roller_stub + 0.2); }
        for (y=ys) translate([0,0,y + len/2]) difference() { cylinder(d=hub_d + 2, h=cs + 0.2, center=true); cylinder(d=root_d, h=cs + 0.4, center=true); }
        translate([0,0,len - 10]) intersection() { cylinder(d=3.1, h=11); translate([-2, -1.25, 0]) cube([4, 4, 11]); }
        translate([0,0,len - 5]) rotate([0,90,0]) cylinder(d=hole_m3, h=hub_d);
    }
}
module roller_feed() { roller(feed_hub_d, oring_root_d, oring_cs, oring_y); }
module roller_nip()  { roller(nip_hub_d, nip_oring_root, nip_oring_cs, oring_y); }
module feeder_motor_bracket() {   // +Y side: pockets for the feed N20 and the nip N20
    difference() {
        box([nipE_r - 14, side_out, -22], [feed_r + 16, side_out + 27, -plate_t]);
        for (p=[[feed_r, feed_z], [nipE_r, nip_z]]) { box([p[0] - n20_w/2 - 0.2, side_out - 1, p[1] - n20_h/2 - 0.2], [p[0] + n20_w/2 + 0.2, side_out + 28, p[1] + n20_h/2 + 0.2]);
            for (dx=[-9, 9]) hole_y(p[0] + dx, p[1], hole_m3, side_out + 20, side_out + 28); }
        for (x=[feed_r - 12, feed_r + 12, nipE_r - 10, nipE_r + 10]) hole_z(x, side_out + 3, hole_m3_free, -23, -plate_t + 1);
    }
}
module feeder_bushing_bracket() {  // -Y side
    difference() {
        box([nipE_r - 14, -side_out - 7, -22], [feed_r + 16, -side_out, -plate_t]);
        for (p=[[feed_r, feed_z], [nipE_r, nip_z]]) { hole_y(p[0], p[1], 4.4, -side_out - 8, -side_out + 1);
            box([p[0] - 2.2, -side_out - 8, p[1] - 12], [p[0] + 2.2, -side_out + 1, p[1]]); hole_x(-side_out - 3.5, p[1] - 8, hole_m3, p[0] - 10, p[0] + 10); }
        for (x=[feed_r - 12, feed_r + 12, nipE_r - 10, nipE_r + 10]) hole_z(x, -side_out - 3, hole_m3_free, -23, -plate_t + 1);
    }
}
module motor_retainer() { difference() { box([-11, 0, -6], [11, 2.5, 6]); for (x=[-9,9]) hole_y(x, 0, hole_m3_free, -1, 3); } }
// Idler arm for a Ø16 nip roller at radius rn, pivot at rn+12 (used at the entry and, mirrored, at the exit)
module idler_arm(rn) {
    difference() {
        union() {
            box([rn - 10, -34, 14], [rn + 16, 34, 20]);
            for (s=[-1,1]) box([rn + 8, s*34, 6], [rn + 16, s*(side_out - 1.2), 20]);
            for (y=idler_y) box([rn - 8, y - 6, 4], [rn + 8, y + 6, 20]);
        }
        for (s=[-1,1]) hole_y(rn + 12, 10, hole_m3_free, s*33, s*(side_out + 1));
        for (y=idler_y) box([rn - 9, y - 3.6, 3], [rn + 9, y + 3.6, 15]);
        hole_y(rn, 6.85, 3.3, -40, 40);
        for (y=[-14, 14]) hole_z(rn - 4, y, 5.5, 17, 21);
    }
}
module idler_wheel() { difference() { cylinder(d=idler_d, h=idler_w); translate([0,0,-1]) cylinder(d=3.4, h=idler_w + 2);
    translate([0,0,idler_w/2]) rotate_extrude() translate([idler_d/2, 0]) circle(d=2.2); } }
module lid() {
    r1 = gate_r + hop_len + wall; L = hop_len + 2*wall + 6; W = 2*side_out; z0 = hop_h + 8;
    difference() {
        union() {
            translate([r1 - L + 2, -W/2, z0]) cube([L, W, 4]);
            box([gate_r + 4, -30, 0.5], [gate_r + 28, 30, z0]);                                    // pressure boss reaching the floor
            for (y=[-14, 14]) translate([r1, y - 4, hop_h]) cube([8, 8, 8]);
            box([gate_r + 30, -19, z0 + 4], [gate_r + hop_len - 2, 19, z0 + 21]);                  // ballast box
        }
        hole_y(r1 + 4, hop_h + 4, 3.3, -40, 40);
        for (y=[-9, 9]) box([gate_r + 32, y - 7.5, z0 + 6], [gate_r + hop_len - 4, y + 7.5, z0 + 22]);
        box([gate_r + 6, -28, 2.5], [gate_r + 26, 28, z0 - 2]);
    }
}

// ================================================================== EXIT MODULE (exit fin frame, side = -1: card on local +Z)
module chute() {
    r1 = chute_r0 + chute_len;
    difference() {
        union() {
            box([nipX_r - 10, -(tower_y_in + tower_t + 1), -chute_drop - plate_t], [r1 + wall, tower_y_in + tower_t + 1, -chute_drop]);   // floor (rests on the tower arms)
            for (s=[-1,1]) box([nipX_r - 10, s*side_in, -chute_drop], [r1 + wall, s*side_out, chute_wall_h]);          // side walls
            box([r1, -side_out, -chute_drop], [r1 + wall, side_out, chute_wall_h]);                                    // end wall
            // nip housing: bridge under the exit plane carrying the roller bushings and beam X
            box([nipX_r - 10, -side_out - 6, -chute_drop], [nipX_r + 16, side_out + 6, -plate_t]);
            for (s=[-1,1]) box([nipX_r + 8, s*(side_out + 0.5), -plate_t], [nipX_r + 16, s*(side_out + 6.5), 16]);     // idler pivot blocks
            for (s=[-1,1]) box([nipX_r - 10, s*side_in, -plate_t], [beamX_r + 6, s*side_out, 6]);                       // short rails at the nip
            box([beamX_r - 5, -side_out, 6], [beamX_r + 5, side_out, 14]);                                              // beam X bridge
        }
        box([nipX_r - 6, -side_in - 0.1, -chute_drop - 1], [nipX_r + 6, side_in + 0.1, 1]);                             // nip roller slot
        hole_y(nipX_r, nip_z, 4.4, -side_out - 7, -side_out + 1);                                                        // -Y bushing
        box([nipX_r - 2.2, -side_out - 7, nip_z - 14], [nipX_r + 2.2, -side_out + 1, nip_z]);
        box([nipX_r - n20_w/2 - 0.2, side_out - 1, nip_z - n20_h/2 - 0.2], [nipX_r + n20_w/2 + 0.2, side_out + 7, nip_z + n20_h/2 + 0.2]);   // +Y motor pocket
        for (s=[-1,1]) hole_y(nipX_r + 12, 10, hole_m3_free, s*(side_out - 1), s*(side_out + 8));                       // idler pivot
        hole_z(beamX_r, 0, 3.3, -chute_drop - plate_t - 1, -chute_drop - 1.0); hole_z(beamX_r, 0, 2.0, -chute_drop - 1, 1);
        hole_z(beamX_r, 0, 3.3, 9, 15); hole_z(beamX_r, 0, 2.0, 5, 10);
        for (r=[nipX_r + 4, chute_r0 + 14], s=[-1,1]) hole_z(r, s*(tower_y_in + tower_t/2), hole_m3_free, -chute_drop - plate_t - 1, -chute_drop + 1);   // tower arm screws
        box([chute_r0 + 20, -side_in + 12, -chute_drop - plate_t - 1], [r1 - 6, side_in - 12, -chute_drop + 1]);        // finger opening in the floor
    }
}
module shutter_servo_bracket() {   // MG90S with its shaft along Y; screws to the -Y tower's outer face; horn pin (M2) reaches the shutter tongue through the tower slot
    difference() {
        box([-24, -14, -12], [24, 0, 12]);
        box([-servo_body[0]/2 - 0.3, -13, -servo_body[1]/2 - 0.3], [servo_body[0]/2 + 0.3, 1, servo_body[1]/2 + 0.3]);   // body pocket (open toward the tower)
        for (dx=[-servo_hole_sp/2, servo_hole_sp/2]) hole_y(dx, 0, hole_m2, -15, 1);
        for (x=[-18, 18]) hole_y(x, 0, hole_m3_free, -15, 1);
    }
}

// ================================================================== BASE (world frame, two halves; the wheel axis is at Z = 100)
module base_half(x0, x1) {
    difference() {
        translate([x0, -base_hw, 0]) cube([x1 - x0, 2*base_hw, base_h]);
        translate([x0 + 3, -base_hw + 3, -1]) cube([x1 - x0 - 6, 2*base_hw - 6, base_h - 2]);     // hollow, open bottom (cover plate)
        for (x=[x0 + 8, x1 - 8], y=[-base_hw + 8, base_hw - 8]) hole_z(x, y, hole_m3, -1, base_h + 1);   // cover screws
        for (x=[-100, -62, -30, 10, 42, 82], y=[-(tower_y_in + tower_t/2), tower_y_in + tower_t/2]) if (x > x0 && x < x1) hole_z(x, y, hole_m3, base_h - 10, base_h + 1);   // tower feet
    }
}
module base_front() { difference() { base_half(-60, base_x1);
    // panel: button, LED, switch, jack, USB
    translate([base_x1, 0, 25]) rotate([0,90,0]) cylinder(d=12.5, h=10, center=true);
    translate([base_x1, -30, 25]) rotate([0,90,0]) cylinder(d=6, h=10, center=true);
    translate([base_x1, 30, 25]) rotate([0,90,0]) cylinder(d=6.5, h=10, center=true);
    translate([base_x1, 60, 22]) rotate([0,90,0]) cylinder(d=8.5, h=10, center=true);
    translate([60, -base_hw - 2, 12]) cube([14, 6, 9]); } }
module base_rear() { base_half(base_x0, -60); }
module base_cover(x0, x1) { difference() { translate([x0 + 3.5, -base_hw + 3.5, 0]) cube([x1 - x0 - 7, 2*base_hw - 7, 2.5]);
    for (x=[x0 + 8, x1 - 8], y=[-base_hw + 8, base_hw - 8]) hole_z(x, y, hole_m3_free, -1, 4); } }

// ================================================================== DUMMIES
module nema17_y() { color("black") translate([-nema_w/2, 0, -nema_w/2]) cube([nema_w, nema_len, nema_w]); }
module n20_y(len=n20_len) { color("silver") translate([-n20_w/2, 0, -n20_h/2]) cube([n20_w, len, n20_h]); }
module oring_ring(d_root, cs) { color("black") rotate_extrude() translate([d_root/2 + cs/2, 0]) circle(d=cs); }
module card_in_slot(k, ang0) { color("ivory") rotate([0, -(ang0 + k*pitch_deg), 0]) box([r_hub, -card_l/2, 0.05], [r_seat_out, card_l/2, 0.35]); }

// ================================================================== ASSEMBLY
module feeder_module(e=0) {
    at_fin(entry_fin_ang, 1) {
        color("lightsteelblue") feeder_deck();
        color("tomato") translate([-e*0.5, 0, e*0.6]) gate_block();
        color("lightsteelblue") translate([0,0,-e*0.8]) { feeder_motor_bracket(); feeder_bushing_bracket(); }
        translate([feed_r, -roller_len/2, feed_z - e*1.2]) rotate([-90,0,0]) { color("lightgray") roller_feed(); for (y=oring_y) translate([0,0,y + roller_len/2]) oring_ring(oring_root_d, oring_cs); }
        translate([nipE_r, -roller_len/2, nip_z - e*1.2]) rotate([-90,0,0]) { color("lightgray") roller_nip(); for (y=oring_y) translate([0,0,y + roller_len/2]) oring_ring(nip_oring_root, nip_oring_cs); }
        translate([feed_r, side_out + 1, feed_z - e*0.8]) n20_y(); translate([nipE_r, side_out + 1, nip_z - e*0.8]) n20_y();
        color("mediumseagreen") translate([0,0,e*1.0]) idler_arm(nipE_r);
        for (y=idler_y) color("lightgray") translate([nipE_r, y - idler_w/2, 6.85 + e*1.0]) rotate([-90,0,0]) idler_wheel();
        color("khaki", 0.85) translate([0,0,e*1.6]) lid();
        color("ivory") translate([gate_r + 1, -card_l/2, 0.8]) cube([card_w, card_l, 52*0.3]);
    }
}
module exit_module(e=0) {
    at_fin(exit_fin_ang, -1) {
        color("lightsteelblue") translate([e*0.8, 0, -e*0.6]) chute();
        translate([nipX_r, -roller_len/2, nip_z - e*1.2]) rotate([-90,0,0]) { color("lightgray") roller_nip(); for (y=oring_y) translate([0,0,y + roller_len/2]) oring_ring(nip_oring_root, nip_oring_cs); }
        translate([nipX_r, side_out + 1, nip_z - e*0.8]) n20_y();
        color("mediumseagreen") translate([0,0,e*1.0]) idler_arm(nipX_r);
        for (y=idler_y) color("lightgray") translate([nipX_r, y - idler_w/2, 6.85 + e*1.0]) rotate([-90,0,0]) idler_wheel();
        color("ivory") translate([chute_r0 + 8, -card_l/2, -chute_drop]) cube([card_w, card_l, 20*0.3]);   // partial output pile
    }
}
module wheel_module(e=0) {
    color("wheat") wheel();
    for (k=[0:2:53]) if (k != 0) card_in_slot(k, 0);
    color("lightsteelblue", 0.7) translate([0, 0, -e*1.2]) shroud_segment(shroud_a0, 270, true);
    color("lightsteelblue", 0.7) translate([0, 0, -e*1.2]) shroud_segment(270, shroud_a1, false);
    color("orange") translate([-e*1.5, 0, -e*1.5]) shutter();
    color("gainsboro") translate([0, e*1.5, 0]) tower(1);
    color("gainsboro") translate([0, -e*1.5, 0]) tower(-1);
    color("silver") cyl_y(4, -tower_y_in - tower_t - 8, tower_y_in + tower_t + motor_standoff + 5);   // M8 rod
    translate([0, tower_y_in + tower_t + motor_standoff + e*2, 0]) nema17_y();
    color("silver") cyl_y(coupler_d/2, tower_y_in + tower_t + 1, tower_y_in + tower_t + coupler_len);
    color("gainsboro") rotate([0, -shutter_servo_ang, 0]) translate([shutter_servo_r, -tower_y_in - tower_t - e, 0]) shutter_servo_bracket();
    color("gainsboro") rotate([0, -index_ang, 0]) translate([index_r1 + 4, tower_y_in - 11 + e, 0]) index_bracket();
}
module assembly(e=0) {
    translate([0, 0, 100]) { wheel_module(e); feeder_module(e); exit_module(e); }
    color("gainsboro", 0.5) translate([0,0,-e*1.5]) { base_front(); base_rear(); }
}

// ================================================================== PART SELECTOR (print orientation, on Z = 0)
if (part == "assembly") { if (section) intersection() { assembly(explode); translate([-300, 0, -10]) cube([600, 300, 400]); } else assembly(explode); }
else if (part == "wheel") translate([0, 0, cage_w/2 + disc_t + hub_boss_len]) rotate([90,0,0]) wheel();       // axis vertical, one boss on the bed
else if (part == "tower_R") translate([0, 0, -tower_y_in]) rotate([90,0,0]) tower(1);        // inner face on the bed, standoff up
else if (part == "tower_L") translate([0, 0, -tower_y_in]) rotate([-90,0,0]) tower(-1);       // inner face on the bed
else if (part == "shroud_A") translate([0, 0, tower_y_in]) rotate([90,0,0]) shroud_segment(shroud_a0, 270, true);
else if (part == "shroud_B") translate([0, 0, tower_y_in]) rotate([90,0,0]) shroud_segment(270, shroud_a1, false);
else if (part == "shutter") translate([0, 0, tower_y_in - 3]) rotate([90,0,0]) shutter();
else if (part == "feeder_deck") translate([0,0,plate_t]) feeder_deck();
else if (part == "gate_block") translate([0,0,-(gate_r - wall - 3)]) rotate([0,-90,0]) gate_block();   // flange face down
else if (part == "roller_feed") translate([0,0,roller_stub]) roller_feed();
else if (part == "roller_nip") translate([0,0,roller_stub]) roller_nip();
else if (part == "feeder_motor_bracket") translate([0,0,22]) feeder_motor_bracket();
else if (part == "feeder_bushing_bracket") translate([0,0,22]) feeder_bushing_bracket();
else if (part == "motor_retainer") translate([0,0,2.5]) rotate([-90,0,0]) motor_retainer();
else if (part == "idler_arm") translate([0,0,20]) rotate([180,0,0]) idler_arm(nipE_r);
else if (part == "idler_wheel") idler_wheel();
else if (part == "lid") translate([0,0,hop_h + 8 + 21]) rotate([180,0,0]) lid();
else if (part == "chute") translate([0,0,chute_drop + plate_t]) chute();
else if (part == "shutter_servo_bracket") translate([0,0,0]) rotate([-90,0,0]) shutter_servo_bracket();
else if (part == "index_bracket") translate([0,0,0]) rotate([90,0,0]) translate([0,0,0]) index_bracket();
else if (part == "base_front") translate([0,0,base_h]) rotate([180,0,0]) base_front();
else if (part == "base_rear") translate([0,0,base_h]) rotate([180,0,0]) base_rear();
else if (part == "base_cover_front") base_cover(-60, base_x1);
else if (part == "base_cover_rear") base_cover(base_x0, -60);
else if (part == "wheel_module") wheel_module(explode);
else if (part == "feeder_module") feeder_module(explode);
else if (part == "exit_module") exit_module(explode);
else echo(str("unknown part: ", part));
