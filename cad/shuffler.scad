// shuffler.scad — parametric card shuffler, all printed parts + assembly.
//
//   openscad -D 'part="feeder_deck"' -o stl/feeder_deck.stl shuffler.scad
//   openscad -D 'part="assembly"' -D explode=0  -o png/assembly.png  shuffler.scad
//   openscad -D 'part="assembly"' -D explode=40 -o png/exploded.png  shuffler.scad
//   openscad -D 'part="assembly"' -D section=1  ...   (cut at Y = 0)
//
// See params.scad for every dimension and the coordinate frame.  All parts are modelled
// in the card frame (card path plane = Z 0) except the two skirts, which are modelled in
// the world frame (table = Z 0).  The assembly places the card frame with card_to_world().
// Every "part=" selection is emitted in its print orientation, resting on Z = 0.

include <params.scad>

part    = "assembly";   // which part to emit
explode = 0;            // exploded-view offset (mm)
section = 0;            // 1 = cut the assembly at Y = 0

// ------------------------------------------------------------------ derived
feed_z    = feed_protr  - roller_d/2;      // -12.8 feed roller axis Z
trans_z   = trans_protr - roller_d/2;      // -13.0 transport roller axis Z
plate_x0  = hop_back_x - wall - 3;         // -87
plate_x1  = well_front_x;                  //  47
plate_hw  = 45;                            // feeder plate half width
wplate_x1 = col_x0 + col_depth + 4;        // 176 well plate end
wplate_hw = 62;
side_in   = hop_in_w/2;                    // 32.5
side_out  = side_in + wall;                // 35.5
bar_ret_y = side_out + 8.5;                // 44.0 bar inner face, retracted
blade_tip_ret = side_in + 1.0;             // 33.5 blade tip when retracted
guide_x   = [knife_cx - 32, knife_cx + 25];// bar guide loops (7 wide in X)
loop_y1   = bar_ret_y + bar_len_y + 4;     // 60 outer face of the guide loops
servo_x   = knife_cx + 13;                 // 108.5 servo shaft X (pin lands outside the blade span)
servo_y   = bar_ret_y + 6;                 // 50   servo shaft Y
pin_x     = servo_x + horn_r;              // 116.5 pin X at mid swing
pin_dy    = 10.0;                          // pin Y offset from the bar's inner face
flange_ys = [-43, -38.5, 38.5, 43];        // deck-to-deck flange bolts
sleeve_fl = 5.5;                           // sleeve flange width outside the well walls

module card_to_world() { translate([0,0,H0]) rotate([0,TILT,0]) children(); }

// ------------------------------------------------------------------ helpers
module box(p0, p1) { translate([min(p0[0],p1[0]), min(p0[1],p1[1]), min(p0[2],p1[2])])
    cube([abs(p1[0]-p0[0]), abs(p1[1]-p0[1]), abs(p1[2]-p0[2])]); }
module hole_z(x, y, d, z0, z1) { translate([x,y,z0]) cylinder(d=d, h=z1-z0); }
module hole_x(y, z, d, x0, x1) { translate([x0,y,z]) rotate([0,90,0]) cylinder(d=d, h=x1-x0); }
module hole_y(x, z, d, y0, y1) { translate([x,y0,z]) rotate([-90,0,0]) cylinder(d=d, h=y1-y0); }

// ================================================================== FEEDER DECK
// Hopper floor + walls + gate bosses + guide rails + flange tabs.  Prints upright, plate on bed.
module feeder_deck() {
    difference() {
        union() {
            box([plate_x0, -plate_hw, -plate_t], [plate_x1, plate_hw, 0]);                          // plate
            box([hop_back_x - wall, -side_out, 0], [hop_back_x, side_out, hop_h]);                  // back wall
            for (s=[-1,1]) box([hop_back_x - wall, s*side_in, 0], [gate_x + wall, s*side_out, hop_h]); // side walls
            box([gate_x, -side_out, gate_win_h], [gate_x + wall, side_out, hop_h]);                  // gate wall above window
            for (s=[-1,1]) box([gate_x - 6, s*side_out, 0], [gate_x + wall, s*(side_out + 4.5), 12]);  // gate clamp bosses
            for (s=[-1,1]) box([gate_x + wall, s*side_in, 0], [plate_x1, s*side_out, rail_h]);      // side rails
            for (s=[-1,1]) box([idler_pivot_x - 4, s*(side_out + 0.5), 0], [idler_pivot_x + 4, s*(side_out + 6.5), 16]); // pivot blocks
            for (s=[-1,1]) box([plate_x1 - wall, s*36, -25], [plate_x1, s*plate_hw, 0]);            // flange tabs
            for (y=[-24, 0, 24]) translate([hop_back_x - wall - 2, y - 4, hop_h]) cube([8, 8, 8]);   // hinge knuckles
        }
        hole_y(hop_back_x - wall + 2, hop_h + 4, 3.3, -40, 40);                                      // hinge pin
        box([feed_x - 10, -side_in - 0.1, -plate_t - 1], [feed_x + 10, side_in + 0.1, 1]);          // feed roller slot
        box([trans_x - 9.5, -side_in - 0.1, -plate_t - 1], [trans_x + 9.5, side_in + 0.1, 1]);      // transport roller slot
        box([gate_x - 1, -side_in, -1], [gate_x + wall + 1, side_in, gate_win_h]);                   // gate window
        for (s=[-1,1]) hole_x(s*(side_out + 2.25), 7, hole_m3, gate_x - 7, gate_x + wall + 1);       // gate clamp screws
        hole_z(beamB_x, 0, 3.3, -plate_t - 1, -1.0);                                                 // beam B LED pocket
        hole_z(beamB_x, 0, 2.0, -plate_t - 1, 1);                                                    // beam B aperture
        for (s=[-1,1]) hole_y(idler_pivot_x, 10, hole_m3_free, s*(side_out - 1), s*(side_out + 8));  // idler pivot (ears only)
        for (x=[feed_x - 12, feed_x + 12, trans_x - 12, trans_x + 12], s=[-1,1]) hole_z(x, s*40, hole_m3, -plate_t - 1, 1); // bracket screws
        for (x=[plate_x0 + 6, plate_x1 - 8], s=[-1,1]) hole_z(x, s*(plate_hw - 5), hole_m3_free, -plate_t - 1, 1);       // skirt screws
        for (y=flange_ys) hole_x(y, -12, hole_m3_free, plate_x1 - wall - 1, plate_x1 + 1);           // flange bolts
        box([-48, -5, -plate_t - 1], [-42, 5, 1]);                                                   // optional TCRT5000 window
        translate([hop_back_x - wall - 1, 0, hop_h + 1]) rotate([0,90,0]) cylinder(d=24, h=wall + 2); // finger recess
    }
}

// Gate block: inner plate fills the window, outer flange with vertical slots clamps to the bosses.
module gate_block() {
    difference() {
        union() {
            box([gate_x, -side_in + 0.3, 0], [gate_x + gate_block_t, side_in - 0.3, gate_win_h - 0.5]);
            box([gate_x + gate_block_t, -(side_out + 5), 0], [gate_x + gate_block_t + 3, side_out + 5, 12]);
        }
        for (s=[-1,1]) translate([gate_x + gate_block_t - 1, s*(side_out + 2.25), 6]) rotate([0,90,0])
            linear_extrude(6) hull() { translate([-3,0]) circle(d=3.4); translate([3,0]) circle(d=3.4); }   // vertical slots
        // round the lip (card side, lower inner edge)
        translate([gate_x - 0.5, -side_in - 1, -0.5]) rotate([-90,0,0]) cylinder(r=1.0, h=2*side_in + 2);
    }
}

// Roller: printed hub, three O-ring grooves, D-bore for the N20 shaft (+Y end), stub axle (-Y end).
module roller() {
    difference() {
        union() { cylinder(d=roller_hub_d, h=roller_len); translate([0,0,-roller_stub]) cylinder(d=4, h=roller_stub + 0.2); }
        for (y=oring_y) translate([0,0,y + roller_len/2]) difference() {
            cylinder(d=roller_hub_d + 2, h=oring_cs + 0.2, center=true); cylinder(d=oring_root_d, h=oring_cs + 0.4, center=true); }
        translate([0,0,roller_len - 10]) intersection() { cylinder(d=3.1, h=11); translate([-2, -1.25, 0]) cube([4, 4, 11]); }  // D-bore
        translate([0,0,roller_len - 5]) rotate([0,90,0]) cylinder(d=hole_m3, h=roller_hub_d);                                   // set screw
    }
}

// Motor bracket (+Y side): both N20 motors slide in from the outside; retainer plates close the pockets.
module motor_bracket() {
    difference() {
        box([feed_x - 16, side_out, -22], [trans_x + 16, side_out + 27, -plate_t]);
        for (x=[feed_x, trans_x]) { z = (x == feed_x) ? feed_z : trans_z;
            box([x - n20_w/2 - 0.2, side_out - 1, z - n20_h/2 - 0.2], [x + n20_w/2 + 0.2, side_out + 28, z + n20_h/2 + 0.2]);
            for (dx=[-9, 9]) hole_y(x + dx, z, hole_m3, side_out + 20, side_out + 28); }             // retainer screws
        for (x=[feed_x - 12, feed_x + 12, trans_x - 12, trans_x + 12]) hole_z(x, 40, hole_m3_free, -23, -plate_t + 1);
    }
}
module motor_retainer() { difference() { box([-11, 0, -6], [11, 2.5, 6]); for (x=[-9,9]) hole_y(x, 0, hole_m3_free, -1, 3); } }

// Bushing bracket (-Y side): U-slots (open downward) for the roller stub axles; a cross screw closes each U.
module bushing_bracket() {
    difference() {
        box([feed_x - 16, -side_out - 7, -22], [trans_x + 16, -side_out, -plate_t]);
        for (x=[feed_x, trans_x]) { z = (x == feed_x) ? feed_z : trans_z;
            hole_y(x, z, 4.4, -side_out - 8, -side_out + 1);
            box([x - 2.2, -side_out - 8, z - 12], [x + 2.2, -side_out + 1, z]);
            hole_x(-side_out - 3.5, z - 8, hole_m3, x - 10, x + 10); }
        for (x=[feed_x - 12, feed_x + 12, trans_x - 12, trans_x + 12]) hole_z(x, -40, hole_m3_free, -23, -plate_t + 1);
    }
}

// Idler arm: pivots on M3 screws in the deck's pivot blocks, two idler wheels above the transport roller,
// carries the beam-B phototransistor above the plate's emitter.  Prints upside down.
module idler_arm() {
    difference() {
        union() {
            box([idler_pivot_x - 8, -30, 14], [idler_x + 10, 30, 20]);                                   // bridge
            for (s=[-1,1]) box([idler_pivot_x - 4, s*30, 6], [idler_pivot_x + 4, s*(side_out - 1.2), 20]);  // pivot ears
            for (y=idler_y) box([idler_x - 8, y - 6, 4], [idler_x + 8, y + 6, 20]);                        // wheel forks
            box([beamB_x - 4, -4, 6], [beamB_x + 4, 4, 20]);                                               // sensor boss
        }
        for (s=[-1,1]) hole_y(idler_pivot_x, 10, hole_m3_free, s*29, s*(side_out + 1));               // pivot
        for (y=idler_y) box([idler_x - 9, y - 3.6, 3], [idler_x + 9, y + 3.6, 15]);                    // wheel clearance
        hole_y(idler_x, 6.85, 3.3, -40, 40);                                                            // wheel axle (M3 x 50)
        hole_z(beamB_x, 0, 3.3, 8, 21);  hole_z(beamB_x, 0, 2.0, 5, 9);                                  // phototransistor + aperture
        for (y=[-12, 12]) hole_z(trans_x - 2, y, 5.5, 17, 21);                                           // spring pockets (pen springs)
    }
}
module idler_wheel() { difference() { cylinder(d=idler_d, h=idler_w); translate([0,0,-1]) cylinder(d=3.4, h=idler_w + 2);
    translate([0,0,idler_w/2]) rotate_extrude() translate([idler_d/2, 0]) circle(d=2.2); } }   // O-ring groove 9x2

// Lid: hinged plate, hanging pressure boss (felt pad under it), ballast troughs for two AA cells on top.
module lid() {
    L = hop_in_l + 2*wall + 6; W = 2*side_out; z0 = hop_h + 8;
    difference() {
        union() {
            translate([hop_back_x - wall - 2, -W/2, z0]) cube([L, W, 4]);                 // plate
            box([-24, -28, 0.5], [0, 28, z0]);                                            // pressure boss (reaches the floor)
            for (y=[-14, 14]) translate([hop_back_x - wall - 2, y - 4, hop_h]) cube([8, 8, 8]);   // hinge knuckles
            box([-72, -19, z0 + 4], [-12, 19, z0 + 21]);                                  // ballast box
        }
        hole_y(hop_back_x - wall + 2, hop_h + 4, 3.3, -40, 40);
        for (y=[-9, 9]) box([-70, y - 7.5, z0 + 6], [-14, y + 7.5, z0 + 22]);              // two AA troughs (open top)
        box([-22, -26, 2.5], [-2, 26, z0 - 2]);                                            // hollow boss (2 mm skin)
    }
}

// ================================================================== WELL DECK
// Plate + well walls above the plate + blade-bar guide loops + sensor holes.  Prints upright, plate on bed.
module well_deck() {
    difference() {
        union() {
            box([well_front_x, -wplate_hw, -plate_t], [wplate_x1, wplate_hw, 0]);                                  // plate
            difference() { box([well_front_x, -side_out, 0], [well_back_x, side_out, well_h]);
                           box([well_x0, -side_in, -1], [well_x1, side_in, well_h + 1]); }                        // well walls
            for (s=[-1,1], gx=guide_x) box([gx, s*(side_out + 0.3), 0], [gx + 7, s*loop_y1, bar_h + 3]);         // guide loops
            for (s=[-1,1]) box([well_front_x, s*36, -25], [well_front_x + wall, s*plate_hw, 0]);                 // flange tabs
            for (s=[-1,1]) translate([knife_cx - 40, s*(loop_y1 + 3), 0]) cylinder(d=5, h=10);                    // return-spring posts
            for (s=[-1,1]) translate([beamS_x, s*side_out, beamS_z]) rotate([s*-90,0,0]) cylinder(d=8, h=7);      // beam S tubes
        }
        for (s=[-1,1], gx=guide_x) box([gx - 1, s*(side_out), 0], [gx + 8, s*(loop_y1 - 2.5), bar_h + 0.4]);    // bar channels
        for (s=[-1,1]) box([knife_cx - knife_len_x/2 - 1, s*(side_in - 1), knife_z - knife_slot_h/2],
                           [knife_cx + knife_len_x/2 + 1, s*(side_out + 1), knife_z + knife_slot_h/2]);           // blade slots
        box([well_front_x - 1, -side_in, slot_z0], [well_x0 + 1, side_in, slot_z1]);                             // entry slot
        box([well_x0, -side_in, -plate_t - 1], [well_x1, side_in, 1]);                                           // well through plate
        box([well_x1 - 1, -arm_slot_w/2, -plate_t - 1], [well_back_x + 1, arm_slot_w/2, z_travel_max + 6]);      // arm slot
        for (s=[-1,1]) box([knife_cx - finger_w/2, s*(side_in - 1), well_h - finger_d], [knife_cx + finger_w/2, s*(side_out + 1), well_h + 1]);
        hole_y(beamS_x, beamS_z, 1.6, -side_out - 8, side_out + 8);                                              // beam S aperture
        for (s=[-1,1]) hole_y(beamS_x, beamS_z, 3.3, s*(side_out + 1.5), s*(side_out + 9));                       // beam S LED pockets
        hole_z(well_front_x + well_wall/2, beamW_y, 2.0, -plate_t - 1, slot_z1 + 1);                             // beam W aperture (from sleeve LED)
        hole_z(well_front_x + well_wall/2, beamW_y, 3.3, slot_z1 + 1.5, well_h + 1);                             // beam W detector pocket
        for (s=[-1,1]) {                                                                                        // servo shaft + horn window
            hole_z(servo_x, s*servo_y, 9, -plate_t - 1, 1);
            box([servo_x - 12, s*(bar_ret_y - 1), -plate_t - 1], [servo_x + 11, s*(bar_ret_y + bar_len_y + 1), 1]);
            for (dx=[-20, 20], yy=[45, 58]) hole_z(servo_x + dx, s*yy, hole_m3_free, -plate_t - 1, 1);           // servo bracket screws
        }
        box([col_x0 - 0.3, -col_hw - 4.3, -plate_t - 1], [col_x0 + col_depth + 0.3, col_hw + 4.3, 1]);            // column opening
        for (x=[col_x0 + 4, col_x0 + col_depth - 4], s=[-1,1]) hole_z(x, s*(col_hw + 8), hole_m3_free, -plate_t - 1, 1);  // column flange bolts
        for (y=flange_ys) hole_x(y, -12, hole_m3_free, well_front_x - 1, well_front_x + wall + 1);               // flange bolts
        for (x=[well_x0 + 10, well_x1 - 10], s=[-1,1]) hole_z(x, s*(side_out + sleeve_fl/2), hole_m3, -plate_t - 1, 1);   // sleeve screws
        for (x=[well_front_x + 8, wplate_x1 - 8], s=[-1,1]) hole_z(x, s*(wplate_hw - 5), hole_m3_free, -plate_t - 1, 1);  // skirt screws
        box([well_x1 - back_pad_t, -side_in - 1, 0], [well_x1 + 0.01, side_in + 1, well_h + 1]);                 // back pad recess
    }
}

// Lower well sleeve: guides the stack below the plate (Z -30 .. -3); carries the beam-W emitter.
module well_sleeve() {
    difference() {
        union() {
            box([well_front_x, -side_out, well_floor_z], [well_back_x, side_out, -plate_t]);
            box([well_x0 + 0.5, -side_out - sleeve_fl, -plate_t - 4], [well_back_x + 2, side_out + sleeve_fl, -plate_t]);   // flange
        }
        box([well_x0, -side_in, well_floor_z - 1], [well_x1, side_in, 0]);
        box([well_x1 - 1, -arm_slot_w/2, well_floor_z - 1], [well_back_x + 1, arm_slot_w/2, 0]);                  // arm slot
        for (x=[well_x0 + 10, well_x1 - 10], s=[-1,1]) hole_z(x, s*(side_out + sleeve_fl/2), hole_m3_free, -plate_t - 5, 0);
        box([well_x1 - back_pad_t, -side_in - 1, well_floor_z - 1], [well_x1 + 0.01, side_in + 1, 0]);            // pad recess
        hole_z(well_front_x + well_wall/2, beamW_y, 3.3, well_floor_z - 1, -plate_t - 1.0);                         // beam W LED (from below)
        hole_z(well_front_x + well_wall/2, beamW_y, 2.0, well_floor_z - 1, 0);                                     // aperture
    }
}

// Knife bar halves.  The blade (0.5 mm brass, 40 x 22 mm) is sandwiched between them.
module knife_bar_lower() {
    difference() {
        box([knife_cx - bar_len/2, bar_ret_y, 0], [knife_cx + bar_len/2, bar_ret_y + bar_len_y, knife_z - knife_t/2]);
        for (dx=[-15, 15]) hole_z(knife_cx + dx, bar_ret_y + 7, hole_m3, -1, 10);                                  // clamp screws
        box([pin_x - 2.4, bar_ret_y + pin_dy - 1.4, -1], [pin_x + 2.4, bar_ret_y + pin_dy + 1.4, 10]);              // servo pin slot (along X)
        hole_z(knife_cx - 30, bar_ret_y + 9, 2.5, -1, 10);                                                          // return spring hook
    }
}
module knife_bar_upper() {
    difference() {
        box([knife_cx - bar_len/2, bar_ret_y, knife_z + knife_t/2], [knife_cx + bar_len/2, bar_ret_y + bar_len_y, bar_h]);
        for (dx=[-15, 15]) hole_z(knife_cx + dx, bar_ret_y + 7, hole_m3_free, 0, 10);
    }
}
module blade() { color("goldenrod") difference() {
    box([knife_cx - knife_len_x/2, blade_tip_ret, knife_z - knife_t/2], [knife_cx + knife_len_x/2, blade_tip_ret + blade_w_y, knife_z + knife_t/2]);
    for (dx=[-15, 15]) hole_z(knife_cx + dx, bar_ret_y + 7, 3.2, 0, 5); } }

// Servo bracket: hangs under the plate; MG90S sits on a ledge at Z -8 with its shaft up through the plate.
module servo_bracket() {
    difference() {
        box([servo_x - 24, bar_ret_y - 2, -32], [servo_x + 24, bar_ret_y + 16, -plate_t]);
        box([servo_x - servo_body[0]/2 - 0.3, servo_y - servo_body[1]/2 - 0.3, -33],
            [servo_x + servo_body[0]/2 + 0.3, servo_y + servo_body[1]/2 + 0.3, -8]);                                 // body pocket
        box([servo_x - servo_flange[0]/2 - 0.3, servo_y - servo_body[1]/2 - 0.3, -8],
            [servo_x + servo_flange[0]/2 + 0.3, servo_y + servo_body[1]/2 + 0.3, -plate_t + 1]);                     // flange pocket
        for (dx=[-servo_hole_sp/2, servo_hole_sp/2]) hole_z(servo_x + dx, servo_y, hole_m2, -16, -7);              // servo tab screws
        for (dx=[-20, 20], yy=[45, 58]) hole_z(servo_x + dx, yy, hole_m3, -12, -plate_t + 1);                       // screws from the plate
        box([servo_x - 20, bar_ret_y - 3, -31], [servo_x + 20, bar_ret_y + 17, -18]);                               // open sides for wires
    }
}
module servo_dummy() {   // MG90S for the assembly view; origin = shaft axis at the flange plane
    color("dimgray") { translate([-servo_body[0]/2, -servo_body[1]/2, -22.5]) cube(servo_body);
        translate([-servo_flange[0]/2, -servo_flange[1]/2, 0]) cube(servo_flange); cylinder(d=6, h=4.5); }
    color("white") translate([0,0,4.5]) hull() { cylinder(d=7, h=1.5); translate([horn_r, 0, 0]) cylinder(d=4, h=1.5); }
    color("silver") translate([horn_r, 0, 6]) cylinder(d=2, h=4);
}

// ================================================================== ELEVATOR
// Column: U-channel behind the well, NEMA 17 on top, rods and lead screw inside, endstop at the bottom.
module column() {
    x1 = col_x0 + col_depth;
    difference() {
        union() {
            box([x1 - 4, -col_hw - 4, col_bot_z], [x1, col_hw + 4, col_top_z]);                     // back plate
            for (s=[-1,1]) box([col_x0, s*col_hw, col_bot_z], [x1, s*(col_hw + 4), col_top_z]);      // side plates
            box([col_x0, -col_hw - 4, col_bot_z], [x1, col_hw + 4, col_bot_z + 4]);                 // bottom plate
            box([col_x0, -col_hw - 4, col_top_z - 4], [x1, col_hw + 4, col_top_z]);                 // top plate (motor mount)
            box([col_x0, -col_hw - 12, 0], [x1 + 4, col_hw + 12, 3]);                               // flange on top of the deck plate
        }
        hole_z(screw_x, screw_y, 12, col_bot_z - 1, col_top_z + 1);                                 // screw / coupler clearance
        hole_z(screw_x, screw_y, nema_boss_d + 0.5, col_top_z - 5, col_top_z + 1);
        for (dx=[-nema_bolt/2, nema_bolt/2], dy=[-nema_bolt/2, nema_bolt/2]) hole_z(screw_x + dx, screw_y + dy, hole_m3_free, col_top_z - 5, col_top_z + 1);
        for (y=rod_y) { hole_z(screw_x, y, rod_d + 0.15, col_bot_z - 1, col_bot_z + 5); hole_z(screw_x, y, rod_d + 0.15, col_top_z - 5, col_top_z - 1); }
        for (y=rod_y) { hole_y(screw_x, col_bot_z + 2, hole_m3, y > 0 ? col_hw - 2 : -col_hw - 5, y > 0 ? col_hw + 5 : -col_hw + 2);
                        hole_y(screw_x, col_top_z - 2.5, hole_m3, y > 0 ? col_hw - 2 : -col_hw - 5, y > 0 ? col_hw + 5 : -col_hw + 2); }  // rod grub screws
        for (x=[col_x0 + 4, x1 - 4], s=[-1,1]) hole_z(x, s*(col_hw + 8), hole_m3_free, -1, 4);      // flange bolts
        box([col_x0 - 1, -col_hw, -1], [x1 - 4, col_hw, 4]);                                        // keep the channel open through the flange
        for (dy=[-4.75, 4.75]) hole_x(12 + dy, endstop_z, hole_m2, x1 - 5, x1 + 1);                  // endstop screws (KW12, 9.5 mm pitch)
        for (s=[-1,1]) box([col_x0 + 4, s*(col_hw - 1), -40], [x1 - 8, s*(col_hw + 5), 45]);        // side windows
    }
}

// Carriage: rear beam (nut + 2 bushings) + arm through the back wall + spine under the platform.
// Modelled with the platform top at Z = 0.  Prints on its back (X = x1 face on the bed).
module carriage() {
    x0 = col_x0 + 4; x1 = col_x0 + 20;   // 150..166 (back plate starts at 168)
    difference() {
        union() {
            box([x0, -col_hw + 2, -14], [x1, col_hw - 2, 10]);                                       // rear beam
            box([well_x1 - 4, -arm_w/2, -plat_t], [x0 + 1, arm_w/2, 2]);                             // arm through the wall slot
            box([plat_x0 + 40, -arm_w/2, -plat_t - 6], [well_x1 - 3, arm_w/2, -plat_t + 0.01]);       // spine under the platform
        }
        hole_z(screw_x, screw_y, t8nut_body_d + 0.4, -15, 11);
        translate([screw_x, screw_y, 10 - 3.6]) cylinder(d=t8nut_flange_d + 0.5, h=4);
        for (a=[45, 135, 225, 315]) hole_z(screw_x + t8nut_pcd/2*cos(a), screw_y + t8nut_pcd/2*sin(a), hole_m3, -15, 11);
        for (y=rod_y) hole_z(screw_x, y, bush_d + 0.2, -15, 11);                                     // LM8UU
        for (x=[plat_x0 + 48, well_x1 - 10]) hole_z(x, 0, hole_m3, -plat_t - 7, 1);                  // platform screws
    }
}
module platform() {
    difference() {
        box([plat_x0, -plat_w/2, -plat_t], [plat_x0 + plat_l, plat_w/2, 0]);
        for (x=[plat_x0 + 48, well_x1 - 10]) { hole_z(x, 0, hole_m3_free, -plat_t - 1, 1); hole_z(x, 0, 6, -1.2, 1); }   // countersunk
    }
}

// ================================================================== SKIRTS (world frame)
// Wedge box under each deck plate: flat bottom on the table, open top following the tilted plate.
function pw(x, y, dz=0) = [x*cos(TILT) - (plate_t - dz)*sin(TILT), y, H0 - x*sin(TILT) - (plate_t - dz)*cos(TILT)];
module skirt(x0, x1, hw, mount=[]) {
    module rim(inset, dz) { hull() for (x=[x0, x1], y=[-hw, hw])
        translate(pw(x + (x == x0 ? inset : -inset), y > 0 ? y - inset : y + inset, dz)) cube(0.05, center=true); }
    module base(inset, z) { hull() for (x=[x0, x1], y=[-hw, hw])
        translate([(x + (x == x0 ? inset : -inset))*cos(TILT), y > 0 ? y - inset : y + inset, z]) cube(0.05, center=true); }
    difference() {
        hull() { rim(0, 0); base(0, 0); }
        hull() { rim(wall, 6); base(wall, wall); }          // hollow, open top (inner hull pokes 6 mm above the rim)
    }
    // screw bosses hanging under the rim at the plate's mounting holes
    for (m=mount) translate(pw(m[0], m[1])) rotate([0, TILT, 0]) difference() {
        translate([-5, -5, -12]) cube([10, 10, 12]); translate([0, 0, -13]) cylinder(d=hole_m3, h=14); }
}
module skirt_front() {
    xf = plate_x0*cos(TILT);
    difference() {
        skirt(plate_x0, plate_x1, skirt_hw, [[plate_x0 + 6, -40], [plate_x0 + 6, 40], [plate_x1 - 8, -40], [plate_x1 - 8, 40]]);
        translate([xf, 0, 60])   rotate([0,90,0]) cylinder(d=12.5, h=12, center=true);   // start button
        translate([xf, -28, 60]) rotate([0,90,0]) cylinder(d=6, h=12, center=true);      // status LED
        translate([xf, 28, 60])  rotate([0,90,0]) cylinder(d=6.5, h=12, center=true);    // power toggle
        translate([xf, 28, 35])  rotate([0,90,0]) cylinder(d=8.5, h=12, center=true);    // charge jack
        translate([xf + 12, -skirt_hw - 2, 22]) cube([14, 6, 9]);                         // USB access (side)
        translate([xf + 30, skirt_hw - 4, 10]) cube([30, 8, 4]);                          // Velcro strap slot
        translate([xf + 30, -skirt_hw - 4, 10]) cube([30, 8, 4]);
    }
}
module skirt_rear() { skirt(plate_x1, wplate_x1, skirt_hw, [[well_front_x + 8, -57], [well_front_x + 8, 57], [wplate_x1 - 8, -57], [wplate_x1 - 8, 57]]); }

// ================================================================== DUMMIES (non-printed parts, for views)
module nema17() { color("black") translate([-nema_w/2, -nema_w/2, 0]) cube([nema_w, nema_w, nema_len]); color("silver") translate([0,0,-22]) cylinder(d=5, h=24); }
module lead_screw() { color("silver") cylinder(d=8, h=100); }
module rod() { color("silver") cylinder(d=rod_d, h=rod_len); }
module n20() { color("silver") { translate([-n20_w/2, 0, -n20_h/2]) cube([n20_w, n20_len, n20_h]); translate([0,-n20_shaft_l,0]) rotate([-90,0,0]) cylinder(d=3, h=n20_shaft_l); } }
module oring_ring(d_root, cs) { color("black") rotate_extrude() translate([d_root/2 + cs/2, 0]) circle(d=cs); }
module deck_dummy(n=52, t=card_t_nom) { color("ivory", 0.9) box([well_x0 + 1, -card_w/2, 0], [well_x0 + 1 + card_l, card_w/2, n*t]); }

// ================================================================== ASSEMBLY
module feeder_module(e=0) {
    color("lightsteelblue") feeder_deck();
    color("tomato") translate([e*0.5,0,e*0.6]) gate_block();
    color("lightsteelblue") translate([0,0,-e*0.8]) motor_bracket();
    color("lightsteelblue") translate([0,0,-e*0.8]) bushing_bracket();
    for (x=[feed_x, trans_x]) { z = (x==feed_x) ? feed_z : trans_z;
        translate([x, -roller_len/2, z - e*1.2]) rotate([-90,0,0]) { color("lightgray") roller(); for (y=oring_y) translate([0,0,y + roller_len/2]) oring_ring(oring_root_d, oring_cs); }
        translate([x, side_out + 1, z - e*0.8]) n20();
    }
    color("mediumseagreen") translate([0,0,e*1.0]) idler_arm();
    for (y=idler_y) color("lightgray") translate([idler_x, y - idler_w/2, 6.85 + e*1.0]) rotate([-90,0,0]) idler_wheel();
    color("khaki", 0.85) translate([0,0,e*1.6]) lid();
    color("ivory") translate([hop_back_x + 1, -card_w/2, 0.8]) cube([card_l, card_w, 52*card_t_nom]);   // deck in hopper
}
module well_module(e=0, z_plat=-8) {
    color("lightsteelblue") well_deck();
    color("lightsteelblue") translate([0,0,-e*1.0]) well_sleeve();
    for (s=[-1,1]) mirror([0, s<0 ? 1 : 0, 0]) {
        color("orange") translate([0,0,e*0.9]) knife_bar_lower();
        color("orange") translate([0,0,e*1.3]) knife_bar_upper();
        translate([0,0,e*1.1]) blade();
        color("lightsteelblue") translate([0,0,-e*1.2]) servo_bracket();
        translate([servo_x, servo_y, -8 - e*1.2]) servo_dummy();
    }
    color("lightsteelblue") translate([e*1.0, 0, 0]) column();
    translate([screw_x, screw_y, col_top_z + e*1.6]) nema17();
    translate([screw_x, screw_y, -54 + e*1.0]) lead_screw();
    for (y=rod_y) translate([screw_x, y, col_bot_z + 1 + e*1.0]) rod();
    color("silver") translate([screw_x, screw_y, 34 + e*1.3]) cylinder(d=coupler_d, h=coupler_len);
    translate([0, 0, z_plat + e*0.5]) { color("plum") carriage(); color("plum") platform(); }
    translate([0,0,z_plat]) deck_dummy(30);
}
module assembly(e=0) {
    card_to_world() { feeder_module(e); well_module(e); }
    color("gainsboro", 0.55) translate([0,0,-e*1.5]) skirt_front();
    color("gainsboro", 0.55) translate([0,0,-e*1.5]) skirt_rear();
}

// ================================================================== PART SELECTOR (print orientation, on Z = 0)
if (part == "assembly") { if (section) intersection() { assembly(explode); translate([-200,0,-10]) cube([500,200,400]); } else assembly(explode); }
else if (part == "feeder_deck") translate([0,0,25]) feeder_deck();          // flange tabs reach Z -25
else if (part == "gate_block") translate([0,0,gate_x + gate_block_t + 3]) rotate([0,90,0]) gate_block();    // flange face down
else if (part == "roller") translate([0,0,roller_stub]) roller();
else if (part == "motor_bracket") translate([0,0,22]) motor_bracket();
else if (part == "motor_retainer") translate([0,0,2.5]) rotate([-90,0,0]) motor_retainer();
else if (part == "bushing_bracket") translate([0,0,22]) bushing_bracket();
else if (part == "idler_arm") translate([0,0,20]) rotate([180,0,0]) idler_arm();
else if (part == "idler_wheel") idler_wheel();
else if (part == "lid") translate([0,0,hop_h + 8 + 21]) rotate([180,0,0]) lid();
else if (part == "well_deck") translate([0,0,25]) well_deck();
else if (part == "well_sleeve") translate([0,0,-well_floor_z]) well_sleeve();
else if (part == "knife_bar_lower") knife_bar_lower();
else if (part == "knife_bar_upper") translate([0,0,bar_h]) rotate([180,0,0]) knife_bar_upper();
else if (part == "servo_bracket") translate([0,0,32]) servo_bracket();
else if (part == "column") translate([0,0,-col_bot_z]) column();
else if (part == "carriage") translate([0,0,col_x0 + 20]) rotate([0,90,0]) carriage();   // rear face on the bed, platform end up
else if (part == "platform") translate([0,0,plat_t]) platform();
else if (part == "skirt_front") skirt_front();
else if (part == "skirt_rear") skirt_rear();
else if (part == "feeder_module") feeder_module(explode);
else if (part == "well_module") well_module(explode);
else echo(str("unknown part: ", part));
