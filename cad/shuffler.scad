// shuffler.scad — WHEEL card shuffler: all printed parts + assembly.
//
//   openscad -D 'part="wheel"' -o stl/wheel.stl shuffler.scad
//   openscad -D 'part="assembly"' [-D explode=30] [-D section=1] -o png/... shuffler.scad
//
// Every "part=" selection is emitted in its print orientation resting on Z = 0.
// See params.scad for the frame and every dimension.
//
// Clearance rule used throughout: seated cards sweep every point with r <= r_seat_out (85.5) and
// |Y| <= 46 (card half-length + cage play) whenever the wheel turns, and the discs sweep r <= 77.5 at
// |Y| 46..49.  Nothing fixed may enter those volumes.

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
// extrude a 2D shape drawn in the XZ plane (x right, z up) along Y from y0 to y1
module xz_extrude(y0, y1) { translate([0,y1,0]) rotate([90,0,0]) linear_extrude(y1-y0) children(); }
module arc_y(r0, r1, a0, a1, y0, y1) { xz_extrude(y0, y1) arc2d(r0, r1, a0, a1); }

// derived feeder / exit positions (local fin frames)
side_in = hop_w/2; side_out = side_in + wall;        // 45.5 / 48.5
feed_z  = feed_protr - feed_roller_d/2;              // -12.8
nip_z   = nip_protr - nip_roller_d/2;                // -7.5
piv_x0  = 8;  piv_x1 = 15.5;  piv_c = (piv_x0 + piv_x1)/2;   // idler pivot block, relative to the nip axis
mot_y0  = roller_len/2 + 0.5;                       // N20 gearbox face (motor body runs to mot_y0 + 24 < 58)
bush_y  = [-roller_len/2 - 10, -roller_len/2 - 2];  // stub-axle bushing block, -Y side

// ================================================================== WHEEL
module fin() {   // one fin in its local frame: CCW face on local Z = 0, body toward -Z
    difference() {
        box([r_hub - hub_wall + 0.5, -cage_w/2, -fin_t], [r_tip, cage_w/2, 0]);
        // lightening windows; pointed toward +Y (up when printed) so their tops print without bridging
        for (x=[[30, 47], [52, 71]], y=[[-38, -8], [8, 38]]) let(w = x[1] - x[0]) translate([0, 0, -fin_t - 1]) linear_extrude(fin_t + 2)
            polygon([[x[0], y[0]], [x[1], y[0]], [x[1], y[1] - w/2], [(x[0] + x[1])/2, y[1]], [x[0], y[1] - w/2]]);
        // lead-in: both faces chamfered over the last 2.5 mm so an incoming card rides onto the fin
        xz_extrude(-cage_w/2 - 1, cage_w/2 + 1) polygon([[r_tip - 2.5, 0.01], [r_tip + 0.1, 0.01], [r_tip + 0.1, -fin_t/2]]);
        xz_extrude(-cage_w/2 - 1, cage_w/2 + 1) polygon([[r_tip - 2.5, -fin_t - 0.01], [r_tip + 0.1, -fin_t - 0.01], [r_tip + 0.1, -fin_t/2]]);
    }
}
module disc(y0, y1, with_tab=false, solid=false) {   // solid: the -Y disc, which is the first layers on the bed
    difference() {
        union() {
            if (solid) cyl_y(r_tip, y0, y1);
            else {
                arc_y(disc_ring_in, r_tip, 0, 360, y0, y1);
                cyl_y(hub_boss_r, y0, y1);
                for (i=[0:n_spokes-1]) rotate([0, -i*360/n_spokes, 0]) box([0, y0, -4], [r_tip - 2, y1, 4]);
            }
            if (with_tab) arc_y(index_r0, index_r1, index_ang - 3, index_ang + 3, y0, y1 + index_w);   // tab inside the rim
        }
        cyl_y(bore_d/2, y0 - 1, y1 + index_w + 1);
    }
}
module wheel() {
    difference() {
        union() {
            difference() { cyl_y(r_hub, -cage_w/2, cage_w/2); cyl_y(r_hub - hub_wall, -cage_w/2 - 1, cage_w/2 + 1); }   // hub tube
            cyl_y(7, -cage_w/2, cage_w/2);                                                         // bore tube and ribs inside the hub:
            for (i=[0:5]) rotate([0, -(30 + i*60), 0]) box([5, -cage_w/2, -0.8], [r_hub - hub_wall + 0.5, cage_w/2, 0.8]);   // the +Y disc bridges between them
            for (k=[0:n_slots-1]) rotate([0, -k*pitch_deg, 0]) fin();
            disc(-cage_w/2 - disc_t, -cage_w/2, solid=true);
            disc(cage_w/2, cage_w/2 + disc_t, true);
            cyl_y(hub_boss_r, cage_w/2, cage_w/2 + disc_t + hub_boss_len);        // +Y hub boss (the -Y one is the separate hub_spacer)
        }
        cyl_y(bore_d/2, -100, 100);
    }
}

// Spacer between the -Y disc and its clamp nut (a separate part so the wheel prints with its flat disc on the bed).
module hub_spacer() { difference() { cylinder(r=hub_boss_r, h=hub_boss_len); translate([0,0,-1]) cylinder(d=bore_d, h=hub_boss_len + 2); } }

// ================================================================== SHROUD + SHUTTER
// The arc runs from tower to tower (|Y| <= ye) so it prints standing on a full edge (+Y down).  Beyond |Y| 46 it is
// outside every moving part except on the -Y side over the shutter drive tab's travel, where that skirt is left off;
// across the window the +Y skirt is a strap that ties the two sides of the window together.
module shroud_segment(a0, a1, window=false) {
    ye = tower_y_in;
    ri = r_shroud_in; ro = r_shroud_in + shroud_t;
    difference() {
        union() {
            arc_y(ri, ro, a0, a1, -shroud_w/2, ye);
            if (window) { arc_y(ri, ro, a0, win_a0 - 1, -ye, -shroud_w/2 + 0.01); arc_y(ri, ro, shut_skirt_a1, a1, -ye, -shroud_w/2 + 0.01); }
            else arc_y(ri, ro, a0, a1, -ye, -shroud_w/2 + 0.01);
            for (a=[a0 + 4, a1 - 4], s=[-1,1]) rotate([0,-a,0]) hull() {                                                       // tabs to the towers,
                box([ri, s*(shroud_w/2 - 9), -6], [ro, s*tower_y_in, 6]); box([ri, s*(shroud_w/2 - 2), -6], [ri + 10, s*tower_y_in, 6]); }   // 45° underneath
        }
        if (window) arc_y(ri - 1, ro + 1, win_a0, win_a1, -shroud_w/2 - 1, shroud_w/2 + 1);
        for (a=[a0 + 4, a1 - 4], s=[-1,1]) rotate([0,-a,0]) hole_y(r_shroud_in + 5, 0, hole_m3_free, s*(tower_y_in - 8), s*(tower_y_in + 1));
    }
}
// Blade profile (XZ): outer radius constant, inner radius ramps from 87.8 at the tips to 87.0.
function shut_rin(a) = a < shut_a0 + shut_ramp ? 87.8 - 0.8*(a - shut_a0)/shut_ramp
                     : a > shut_a1 - shut_ramp ? 87.0 + 0.8*(a - (shut_a1 - shut_ramp))/shut_ramp : shut_r_in;
module shutter() {   // closed position; the servo slides it shutter_open_deg counter-clockwise to open
    n = 60;
    difference() {
        union() {
            xz_extrude(-shut_hw, shut_hw) polygon(concat(
                [for (i=[0:n]) let(a = shut_a0 + (shut_a1 - shut_a0)*i/n) [shut_r_out*cos(a), shut_r_out*sin(a)]],
                [for (i=[n:-1:0]) let(a = shut_a0 + (shut_a1 - shut_a0)*i/n) [shut_rin(a)*cos(a), shut_rin(a)*sin(a)]]));
            arc_y(shut_tab_r0, shut_tab_r1, shut_tab_a - 3, shut_tab_a + 3, -shut_hw, -shroud_w/2 - 0.5);   // drive tab beside the shroud end
        }
        // radial pin slot: the horn pin moves between r = shut_pin_r (ends of travel) and servo_ax_r + horn_r (mid travel)
        rotate([0, -shut_tab_a, 0]) hull() for (r=[shut_pin_r - 0.6, servo_ax_r + horn_r + 0.6]) hole_y(r, 0, 2.4, -shut_hw - 1, -shroud_w/2);
    }
}

// ================================================================== TOWERS
module beam_boss(side, xs) {   // inner-face boss on a tower carrying the oblique entry beams (E, S); entry fin frame
    yi = side > 0 ? tower_y_in : -tower_y_in;
    at_fin(entry_fin_ang, 1) difference() {
        box([78, yi, -6], [88.0, yi - side*4, 6]);
        for (x=xs) {
            hole_y(x, side*beam_dz, 3.2, side > 0 ? yi - 3 : yi - 4, side > 0 ? yi + 4 : yi + 3);          // LED / PT pocket
            hole_y(x, side*beam_dz, 1.2, side > 0 ? yi - 4.1 : yi + 2.9, side > 0 ? yi - 2.9 : yi + 4.1);   // aperture
        }
    }
}
module tower(side=1) {   // side=+1: motor side (+Y). Plate in the XZ plane, thickness tower_t, inner face at |Y| = tower_y_in
    y0 = side > 0 ? tower_y_in : -tower_y_in - tower_t; y1 = y0 + tower_t;
    yi = side > 0 ? y0 : y1;   // inner face
    yo = side > 0 ? y1 : y0;   // outer face
    difference() {
        union() {
            hull() { cyl_y(30, y0, y1); box([tower_x0, y0, tower_z_bot], [tower_x1, y1, tower_z_bot + 8]); }   // upright
            box([-102, y0, -8], [102, y1, 8]);                                                                  // crossbar (shroud ends)
            at_fin(entry_fin_ang, 1)  box([40, y0, -14], [gate_r + 20, y1, -plate_t]);                          // feeder arm (plate sits on it)
            at_fin(exit_fin_ang, -1)  box([40, y0, -chute_drop - plate_t - 11], [chute_r0 + 16, y1, -6]);      // chute cheek (chute screws in from the side)
            beam_boss(side, [beamE_r, beamS_r]);
            at_fin(entry_fin_ang, 1) box([76, y0, -14], [deck_tip_r - 0.1, y1, 7]);                             // plate behind the beam boss
            if (side < 0) rotate([0, -shut_mid_a, 0]) box([servo_ax_r - 12, y0, -16], [servo_ax_r + 25, y1, 16]);  // servo boss
            if (side > 0) box([-12, y0, 20], [12, y1, 92]);                                                   // post for the index bracket
        }
        cyl_y(bearing_d/2, side > 0 ? yi - 1 : yi - bearing_t, side > 0 ? yi + bearing_t : yi + 1);              // 608 pocket from the inner face
        cyl_y(4.5, y0 - 1, y1 + 1);                                                                               // rod clearance
        at_fin(entry_fin_ang, 1) for (x=[beamE_r, beamS_r]) hole_y(x, side*beam_dz, 2.2, y0 - 1, y1 + 1);        // beam leads to the outside
        if (side > 0) {
            // NEMA 17 through-bolts (M3 x 40): heads in counterbores on the inner face, through the tower and the motor_mount
            for (dx=[-nema_bolt/2, nema_bolt/2], dz=[-nema_bolt/2, nema_bolt/2]) translate([dx,0,dz]) { cyl_y(hole_m3_free/2, y0 - 1, y1 + 1); cyl_y(3.1, y0 - 1, y0 + 4); }
            for (dx=[-9, 9]) hole_y(dx, 71, hole_m3, yi - 1, yi + 9);                                             // index bracket screws
        } else {
            // shutter servo (MG90S, shaft along Y) in a through-pocket; horn recess on the inner face
            rotate([0, -shut_mid_a, 0]) {
                box([servo_ax_r - 5.9, y0 - 1, -6.4], [servo_ax_r - 5.9 + 23.2, y1 + 1, 6.4]);
                for (dr=[-14, 14]) translate([servo_ax_r - 5.9 + 11.6 + dr, 0, 0]) cyl_y(hole_m2/2, y0 - 1, y0 + 8);
            }
            translate([servo_ax_r*cos(shut_mid_a), 0, servo_ax_r*sin(shut_mid_a)]) cyl_y(horn_r + 3, yi - 3, yi + 1);
        }
        at_fin(entry_fin_ang, 1) for (r=[nipE_r + 6, gate_r + 12]) hole_z(r, (y0 + y1)/2, hole_m3, -15, -plate_t + 1);   // feeder deck screws (M3 x 12 down through the plate into the arm)
        at_fin(exit_fin_ang, -1) for (z=chute_screw_z) hole_y(chute_screw_x, z, hole_m3_free, y0 - 1, y1 + 1);                  // chute screws (M3 x 20 from outside)
        for (a=[shroud_a0 + 4, 270 - 4, 270 + 4, shroud_a1 - 4]) rotate([0,-a,0]) translate([r_shroud_in + 5, 0, 0]) cyl_y(hole_m3/2, y0 - 1, y1 + 1);   // shroud tabs
        for (x=[-92, -62, -30, 10, 42, 82]) hole_z(x, (y0 + y1)/2, hole_m3_free, tower_z_bot - 1, tower_z_bot + 9);   // foot screws
        for (p=[[-60, -70], [-30, -45], [30, -45], [60, -70], [0, -75]]) hole_y(p[0], p[1], 18, y0 - 1, y1 + 1);        // weight reduction
    }
}
// Motor standoff between the +Y tower's outer face and the NEMA 17 (the coupler lives inside it).  Separate from the
// tower so the tower prints flat; the four M3 x 40 motor bolts pass through both.
module motor_mount() {
    y0 = tower_y_in + tower_t; y1 = y0 + motor_standoff;
    difference() {
        cyl_y(nema_w/2 + 4, y0, y1);
        cyl_y(13, y0 - 1, y1 + 1);                                                                                // coupler bore
        for (dx=[-nema_bolt/2, nema_bolt/2], dz=[-nema_bolt/2, nema_bolt/2]) translate([dx,0,dz]) cyl_y(hole_m3_free/2, y0 - 1, y1 + 1);
        for (a=[0, 90, 180, 270]) rotate([0,-a,0]) box([12, y0 + 5, -5], [nema_w/2 + 5, y1 - 6, 5]);           // coupler access windows
    }
}
// Index sensor (wheel frame, 12 o'clock = +Z): IR LED at r 59 shines outward across the tab's path (r 66..76,
// Y 49..53) to a phototransistor at r 83.  Screws to the post on the +Y tower's inner face.
module index_bracket() {
    difference() {
        union() {
            box([-12, 55, 52], [12, tower_y_in, 88]);                                       // back plate
            box([-5, 50, 52], [5, tower_y_in, index_led_r + 3]);                           // LED arm (1 mm from the disc face)
            box([-5, 50, index_pt_r - 3], [5, tower_y_in, 88]);                            // PT arm
        }
        hole_z(0, index_beam_y, 3.2, 51, index_led_r + 1.9);  hole_z(0, index_beam_y, 1.5, index_led_r + 1.8, index_led_r + 3.1);
        hole_z(0, index_beam_y, 3.2, index_pt_r - 1.9, 89);   hole_z(0, index_beam_y, 1.5, index_pt_r - 3.1, index_pt_r - 1.8);
        for (dx=[-9, 9]) hole_y(dx, 71, hole_m3_free, 54, tower_y_in + 1);
    }
}

// ================================================================== FEEDER (entry fin frame; card plane local Z = 0, cards on +Z)
module feeder_deck(print_ribs=false) {   // print_ribs: the STL carries cut-away supports in the gate window
    r1 = gate_r + hop_len + wall;                     // hopper back wall outer (186)
    difference() {
        union() {
            box([deck_tip_r, -(tower_y_in + tower_t + 1), -plate_t], [r1, tower_y_in + tower_t + 1, 0]);        // plate (rests on the tower arms)
            box([gate_r + hop_len, -side_out, 0], [r1, side_out, hop_h]);                                       // back wall
            for (s=[-1,1]) box([gate_r, s*side_in, 0], [r1, s*side_out, hop_h]);                               // side walls
            box([gate_r - wall, -side_out, gate_win_h], [gate_r, side_out, hop_h]);                            // gate wall above the window
            for (s=[-1,1]) box([gate_r - wall, s*side_in, 0], [gate_r, s*side_out, gate_win_h]);             // window jambs (the wall's ends reach the plate)
            for (s=[-1,1]) box([gate_r - wall, s*side_out, 0], [gate_r + 6, s*(side_out + 4.5), 12]);          // gate clamp bosses
            for (s=[-1,1]) box([nipE_r + 7.5, s*side_in, 0], [gate_r - wall, s*side_out, rail_h]);             // rails between nip and gate
            for (s=[-1,1]) box([nipE_r + piv_x0, s*(side_out + 0.5), 0], [nipE_r + piv_x1, s*(side_out + 6.5), post_h]);   // idler pivot posts (spring_bar on top)
            for (y=[-24, 24]) hull() { translate([r1 - 1, y - 4, hop_h]) cube([9, 8, 8]); translate([r1 - 1, y - 4, hop_h - 9]) cube([1, 8, 1]); }   // hinge knuckles, 45° below
        }
        hole_y(r1 + 4, hop_h + 4, 3.3, -40, 40);                                                                 // hinge pin
        box([feed_r - 10, -roller_len/2 - 1, -plate_t - 1], [feed_r + 10, roller_len/2 + 1, 1]);                // feed roller slot
        box([deck_tip_r - 1, -roller_len/2 - 1, -plate_t - 1], [nipE_r + 7.5, mot_y0 + 25, 1]);                 // nip roller + motor relief
        box([gate_r - wall - 1, -side_in, -1], [gate_r + 1, side_in, gate_win_h]);                               // gate window
        for (s=[-1,1]) hole_x(s*(side_out + 2.25), 7, hole_m3, gate_r - wall - 1, gate_r + 7);                  // gate clamp screws (along r)
        hole_z(beamB_r, 0, 3.3, -plate_t - 1, -1.0); hole_z(beamB_r, 0, 2.0, -plate_t - 1, 1);                   // beam B emitter
        for (s=[-1,1]) hole_y(nipE_r + piv_c, 10, hole_m3_free, s*(side_out - 1), s*(side_out + 8));             // idler pivot
        for (s=[-1,1]) hole_z(nipE_r + piv_c, s*spring_bar_y, hole_m3, post_h - 8, post_h + 1);                  // spring_bar screws
        for (x=[nipE_r + 16, feed_r + 13.5], y=[44, -38]) hole_z(x, y, hole_m3, -plate_t - 1, -0.8);             // bracket screws (blind)
        for (r=[nipE_r + 6, gate_r + 12], s=[-1,1]) hole_z(r, s*(tower_y_in + tower_t/2), hole_m3_free, -plate_t - 1, 1);            // tower arm screws
        translate([r1 - 1, 0, hop_h + 1]) rotate([0,90,0]) cylinder(d=24, h=wall + 2);                          // finger recess
    }
    // Print supports under the 91 mm gate-wall bridge: three 1.2 mm ribs across the gate window.
    // CUT THEM AWAY (flush cutters, then a file) before fitting the gate block.
    if (print_ribs) for (y=[-1, 0, 1]*(side_in/2)) box([gate_r - wall, y - 0.6, 0], [gate_r, y + 0.6, gate_win_h + 0.01]);
}
module gate_block() {
    difference() {
        union() {
            hull() { box([gate_r - wall, -side_in + 0.3, 0], [gate_r, side_in - 0.3, 12]);                      // upper part tapers toward the
                     box([gate_r - 0.4, -side_in + 0.3, 0], [gate_r, side_in - 0.3, gate_win_h - 0.5]); }      // hopper face so it prints unsupported
            box([gate_r - wall - 3, -(side_out + 5), 0], [gate_r - wall, side_out + 5, 12]);
        }
        for (s=[-1,1]) translate([gate_r - wall - 4, s*(side_out + 2.25), 6]) rotate([0,90,0]) linear_extrude(6) hull() { translate([-3,0]) circle(d=3.4); translate([3,0]) circle(d=3.4); }
        translate([gate_r + 0.5, -side_in - 1, -0.5]) rotate([-90,0,0]) cylinder(r=1.0, h=2*side_in + 2);   // rounded lip on the hopper side
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
// +Y side under the plate: push-fit pockets (open downward) for the feed and nip N20 motors, inside the towers (|Y| < 58)
module feeder_motor_bracket() {
    difference() {
        box([deck_tip_r + 0.5, mot_y0 - 2.5, -22], [feed_r + 16, tower_y_in - 0.5, -plate_t]);   // starts past the tower's beam boss
        for (p=[[feed_r, feed_z], [nipE_r, nip_z]]) {
            box([p[0] - n20_w/2 - 0.2, mot_y0, -23], [p[0] + n20_w/2 + 0.2, mot_y0 + n20_len + 0.3, p[1] + n20_h/2 + 0.2]);
            hole_y(p[0], p[1], 5, mot_y0 - 3, mot_y0 + 1);                                                       // shaft boss clearance
        }
        for (x=[nipE_r + 16, feed_r + 13.5]) hole_z(x, 44, hole_m3_free, -23, -plate_t + 1);
    }
}
module feeder_bushing_bracket() {  // -Y side: U-slots (open downward) for the roller stub axles, closed by cross screws
    difference() {
        box([deck_tip_r - 1, bush_y[0], -22], [feed_r + 16, bush_y[1], -plate_t]);
        for (p=[[feed_r, feed_z], [nipE_r, nip_z]]) { hole_y(p[0], p[1], 4.4, bush_y[0] - 1, bush_y[1] + 1);
            box([p[0] - 2.2, bush_y[0] - 1, -23], [p[0] + 2.2, bush_y[1] + 1, p[1]]); hole_x((bush_y[0] + bush_y[1])/2, p[1] - 6, hole_m3, p[0] - 10, p[0] + 10); }
        for (x=[nipE_r + 16, feed_r + 13.5]) hole_z(x, -38, hole_m3_free, -23, -plate_t + 1);
    }
}
// Idler arm for a Ø16 nip roller at radius rn (entry and exit use the same part). Pivot at rn + piv_c;
// two idler wheels sit over the outer O-rings; the boss carries the beam B / beam X phototransistor.
module idler_arm(rn) {
    difference() {
        union() {
            box([rn - 8, -34, 14], [rn + piv_x1, 34, 20]);
            for (s=[-1,1]) box([rn + piv_x0, s*34, 7], [rn + piv_x1, s*(side_out - 1.2), 20]);   // ears clear the rails (z <= 6) by 1 mm
            for (y=idler_y) box([rn - 8, y - 6, 4], [rn + 8, y + 6, 20]);
            box([rn + 10.5 - 4, -4, 6], [rn + 10.5 + 4, 4, 20]);                    // sensor boss
        }
        for (s=[-1,1]) hole_y(rn + piv_c, 10, hole_m3_free, s*33, s*(side_out + 1));
        for (y=idler_y) box([rn - 9, y - 3.6, 3], [rn + 9, y + 3.6, 15]);
        hole_y(rn, 6.85, 3.3, -40, 40);
        for (y=[-14, 14]) hole_z(rn + 3, y, 5.5, 17, 21);                           // spring pockets (springs bear on the crossbar)
        hole_z(rn + 10.5, 0, 3.3, 8, 21); hole_z(rn + 10.5, 0, 2.0, 5, 9);          // phototransistor + aperture
    }
}
module idler_wheel() { difference() { cylinder(d=idler_d, h=idler_w); translate([0,0,-1]) cylinder(d=3.4, h=idler_w + 2);
    translate([0,0,idler_w/2]) rotate_extrude() translate([idler_d/2, 0]) circle(d=2.2); } }
// Spring bar across the tops of an idler's pivot posts (entry and exit use the same part): the idler springs push on it.
// Separate from the deck / chute because, printed in place, it would be a 110 mm bridge.
module spring_bar(rn) {
    difference() {
        box([rn - 1, -(side_out + 6.5), post_h], [rn + piv_x1, side_out + 6.5, post_h + 4]);
        hole_z(rn + 10.5, 0, 5, post_h - 1, post_h + 5);                                     // beam B / X phototransistor leads
        for (s=[-1,1]) hole_z(rn + piv_c, s*spring_bar_y, hole_m3_free, post_h - 1, post_h + 5);
    }
}
module lid() {
    r1 = gate_r + hop_len + wall; L = hop_len + 2*wall + 6; W = 2*side_out; z0 = hop_h + 8;
    difference() {
        union() {
            translate([r1 - L + 2, -W/2, z0]) cube([L, W, 4]);
            box([gate_r + 4, -30, 0.5], [gate_r + 28, 30, z0]);                                    // pressure boss reaching the floor
            for (y=[-14, 14]) translate([r1, y - 4, hop_h]) cube([8, 8, z0 + 4 - hop_h]);         // knuckles run up to the plate top (print on the bed)
        }
        hole_y(r1 + 4, hop_h + 4, 3.3, -40, 40);
        for (y=[-14, 14]) hole_z(gate_r + 16, y, 15.2, 6, z0 + 5);                                // ballast: two AA cells drop into the boss from the top
    }
}

// ================================================================== EXIT MODULE (exit fin frame, side = -1: card on local +Z)
// Everything here starts at local x >= 95 so the shutter's drive tab (r <= 93.5) can travel past it.
module chute() {
    r1 = chute_r0 + chute_len; xs = nipX_r - 5;           // 95.5
    difference() {
        union() {
            box([xs, -(tower_y_in - 0.5), -chute_drop - plate_t], [r1 + wall, tower_y_in - 0.5, -chute_drop]);        // floor (between the towers)
            box([xs, -(tower_y_in - 0.5), -chute_drop], [nipX_r + piv_x1, tower_y_in - 0.5, -plate_t]);              // nip housing
            box([xs, -side_in, -plate_t], [nipX_r + piv_x0, -side_out, 6]);            // short rail at the nip (-Y only: the +Y one would hang over the motor pocket)
            for (s=[-1,1]) box([nipX_r + piv_x1, s*side_in, -chute_drop], [r1 + wall, s*side_out, chute_wall_h]);        // side walls (after the pivot)
            box([r1, -side_out, -chute_drop], [r1 + wall, side_out, chute_wall_h - 6]);                                // end wall
            for (s=[-1,1]) box([nipX_r + piv_x0, s*(side_out + 0.5), -plate_t], [nipX_r + piv_x1, s*(side_out + 6.5), post_h]);   // idler pivot posts (spring_bar on top)
        }
        box([xs - 1, -roller_len/2 - 1, -chute_drop - 1], [nipX_r + 7.5, roller_len/2 + 1, 1]);                    // nip roller slot
        hole_y(nipX_r, nip_z, 4.4, bush_y[0] - 1, bush_y[1] + 1);                                                    // -Y bushing
        box([nipX_r - 2.2, bush_y[0] - 1, -chute_drop - 1], [nipX_r + 2.2, bush_y[1] + 1, nip_z]);
        hole_x((bush_y[0] + bush_y[1])/2, nip_z - 6, hole_m3, nipX_r - 4, nipX_r + 10);                            // cross screw
        box([nipX_r - n20_w/2 - 0.2, mot_y0, -chute_drop - 1], [nipX_r + n20_w/2 + 0.2, mot_y0 + n20_len + 0.3, nip_z + n20_h/2 + 0.2]);   // +Y motor pocket
        hole_y(nipX_r, nip_z, 5, mot_y0 - 3, mot_y0 + 1);
        for (s=[-1,1]) hole_y(nipX_r + piv_c, 10, hole_m3_free, s*(side_out - 1), s*(side_out + 8));                // idler pivot
        hole_z(beamX_r, 0, 3.3, -chute_drop - plate_t - 1, -chute_drop - 1.0); hole_z(beamX_r, 0, 2.0, -chute_drop - 1, 1);   // beam X emitter
        for (s=[-1,1]) hole_z(nipX_r + piv_c, s*spring_bar_y, hole_m3, post_h - 8, post_h + 1);                    // spring_bar screws
        for (z=chute_screw_z, s=[-1,1]) hole_y(chute_screw_x, z, hole_m3, s > 0 ? tower_y_in - 11 : -tower_y_in, s > 0 ? tower_y_in : -tower_y_in + 11);   // tower screws (from outside)
        box([chute_r0 + 20, -side_in + 12, -chute_drop - plate_t - 1], [r1 - 6, side_in - 12, -chute_drop + 1]);    // finger opening in the floor
    }
}

// ================================================================== BASE (world frame, two halves; the wheel axis is 100 mm above the base top)
module base_half(x0, x1) {
    difference() {
        translate([x0, -base_hw, 0]) cube([x1 - x0, 2*base_hw, base_h]);
        translate([x0 + 3, -base_hw + 3, -1]) cube([x1 - x0 - 6, 2*base_hw - 6, base_h - 2]);     // hollow, open bottom (cover plate)
        for (x=[x0 + 8, x1 - 8], y=[-base_hw + 8, base_hw - 8]) hole_z(x, y, hole_m3, -1, base_h + 1);   // cover screws
        for (x=[-92, -62, -30, 10, 42, 82], y=[-(tower_y_in + tower_t/2), tower_y_in + tower_t/2]) if (x > x0 && x < x1) hole_z(x, y, hole_m3, base_h - 10, base_h + 1);   // tower feet
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
module mg90s_y() { color("dimgray") translate([-5.9, -22.5, -6.1]) cube([22.8, 22.5, 12.2]); color("white") translate([0, 0, 0]) cyl_y(3, 0, 10); }
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
        translate([feed_r, mot_y0, feed_z - e*0.8]) n20_y(); translate([nipE_r, mot_y0, nip_z - e*0.8]) n20_y();
        color("mediumseagreen") translate([0,0,e*1.0]) idler_arm(nipE_r);
        color("lightsteelblue") translate([0,0,e*1.4]) spring_bar(nipE_r);
        for (y=idler_y) color("lightgray") translate([nipE_r, y - idler_w/2, 6.85 + e*1.0]) rotate([-90,0,0]) idler_wheel();
        color("khaki", 0.85) translate([0,0,e*1.6]) lid();
        color("ivory") translate([gate_r + 1, -card_l/2, 0.8]) cube([card_w, card_l, 52*0.3]);
        // the two oblique beams (E, S), drawn thin for reference
        for (x=[beamE_r, beamS_r]) color("red") hull() { translate([x, tower_y_in - 4, beam_dz]) sphere(0.3); translate([x, -tower_y_in + 4, -beam_dz]) sphere(0.3); }
    }
}
module exit_module(e=0) {
    at_fin(exit_fin_ang, -1) {
        color("lightsteelblue") translate([e*0.8, 0, -e*0.6]) chute();
        translate([nipX_r, -roller_len/2, nip_z - e*1.2]) rotate([-90,0,0]) { color("lightgray") roller_nip(); for (y=oring_y) translate([0,0,y + roller_len/2]) oring_ring(nip_oring_root, nip_oring_cs); }
        translate([nipX_r, mot_y0, nip_z - e*0.8]) n20_y();
        color("mediumseagreen") translate([0,0,e*1.0]) idler_arm(nipX_r);
        color("lightsteelblue") translate([0,0,e*1.4]) spring_bar(nipX_r);
        for (y=idler_y) color("lightgray") translate([nipX_r, y - idler_w/2, 6.85 + e*1.0]) rotate([-90,0,0]) idler_wheel();
        color("ivory") translate([chute_r0 + chute_len - card_w, -card_l/2, -chute_drop]) cube([card_w, card_l, 20*0.3]);   // partial output pile against the end wall
    }
}
module wheel_module(e=0) {
    color("wheat") wheel();
    color("wheat") translate([0, -e*0.6, 0]) cyl_y(hub_boss_r, -cage_w/2 - disc_t - hub_boss_len, -cage_w/2 - disc_t);   // hub_spacer
    for (k=[0:2:53]) if (k != 0) card_in_slot(k, 0);
    color("lightsteelblue", 0.7) translate([0, 0, -e*1.2]) shroud_segment(shroud_a0, 270, true);
    color("lightsteelblue", 0.7) translate([0, 0, -e*1.2]) shroud_segment(270, shroud_a1, false);
    color("orange") translate([-e*0.5, 0, -e*0.5]) shutter();
    color("gainsboro") translate([0, e*1.5, 0]) tower(1);
    color("gainsboro") translate([0, -e*1.5, 0]) tower(-1);
    color("silver") cyl_y(4, -tower_y_in - tower_t - 8, tower_y_in + tower_t + motor_standoff + 5);   // M8 rod
    color("gainsboro") translate([0, e*1.8, 0]) motor_mount();
    translate([0, tower_y_in + tower_t + motor_standoff + e*2, 0]) nema17_y();
    color("silver") cyl_y(coupler_d/2, tower_y_in + tower_t + 1, tower_y_in + tower_t + coupler_len);
    translate([servo_ax_r*cos(shut_mid_a), -tower_y_in - tower_t - e*2.5, servo_ax_r*sin(shut_mid_a)]) rotate([0, -shut_mid_a, 0]) mg90s_y();
    color("gainsboro") translate([0, e*0.8, 0]) index_bracket();
}
module assembly(e=0) {
    translate([0, 0, base_h - tower_z_bot]) { wheel_module(e); feeder_module(e); exit_module(e); }   // tower feet on the base top; axis 145 mm above the table
    color("gainsboro", 0.5) translate([0,0,-e*1.5]) { base_front(); base_rear(); }
}

// ================================================================== PART SELECTOR (print orientation, on Z = 0)
if (part == "assembly") { if (section) intersection() { assembly(explode); translate([-300, 0, -10]) cube([600, 300, 400]); } else assembly(explode); }
else if (part == "wheel") translate([0, 0, cage_w/2 + disc_t]) rotate([90,0,0]) wheel();       // axis vertical, solid -Y disc on the bed
else if (part == "hub_spacer") hub_spacer();
else if (part == "tower_R") translate([0, 0, tower_y_in + tower_t]) rotate([-90,0,0]) tower(1);   // outer face down, inner-face bosses up
else if (part == "tower_L") translate([0, 0, tower_y_in + tower_t]) rotate([90,0,0]) tower(-1);
else if (part == "motor_mount") translate([0, 0, -(tower_y_in + tower_t)]) rotate([90,0,0]) motor_mount();   // tower face down
else if (part == "shroud_A") translate([0, 0, tower_y_in]) rotate([-90,0,0]) shroud_segment(shroud_a0, 270, true);   // +Y edge down
else if (part == "shroud_B") translate([0, 0, tower_y_in]) rotate([-90,0,0]) shroud_segment(270, shroud_a1, false);
else if (part == "shutter") translate([0, 0, shut_hw]) rotate([90,0,0]) shutter();
else if (part == "index_bracket") translate([0, 0, tower_y_in]) rotate([-90,0,0]) index_bracket();   // back plate (tower face) down, arms up
else if (part == "feeder_deck") translate([0,0,plate_t]) feeder_deck(print_ribs=true);
else if (part == "gate_block") translate([0,0,-(gate_r - wall - 3)]) rotate([0,-90,0]) gate_block();   // flange face down
else if (part == "roller_feed") translate([0,0,roller_len]) rotate([180,0,0]) roller_feed();   // D-bore end down, stub up
else if (part == "roller_nip") translate([0,0,roller_len]) rotate([180,0,0]) roller_nip();
else if (part == "feeder_motor_bracket") translate([0,0,22]) feeder_motor_bracket();
else if (part == "feeder_bushing_bracket") translate([0,0,22]) feeder_bushing_bracket();
else if (part == "idler_arm") translate([0,0,20]) rotate([180,0,0]) idler_arm(nipE_r);
else if (part == "idler_wheel") idler_wheel();
else if (part == "spring_bar") translate([0,0,-post_h]) spring_bar(nipE_r);
else if (part == "lid") translate([0,0,hop_h + 8 + 4]) rotate([180,0,0]) lid();             // plate top down, boss up
else if (part == "chute") translate([0,0,chute_drop + plate_t]) chute();
else if (part == "base_front") translate([0,0,base_h]) rotate([180,0,0]) base_front();
else if (part == "base_rear") translate([0,0,base_h]) rotate([180,0,0]) base_rear();
else if (part == "base_cover_front") base_cover(-60, base_x1);
else if (part == "base_cover_rear") base_cover(base_x0, -60);
else if (part == "wheel_module") wheel_module(explode);
else if (part == "feeder_module") feeder_module(explode);
else if (part == "exit_module") exit_module(explode);
else echo(str("unknown part: ", part));
