// check_clearance.scad — collision checks for the wheel shuffler (wheel frame, axis on Y, no base offset).
// Each `check` value renders the INTERSECTION of two volumes; an empty result means "no collision".
//   ./check_clearance.sh      (runs every check and reports OK / COLLISION)
use <shuffler.scad>
include <params.scad>

check = "list";

// ---- volumes swept by moving parts --------------------------------------------------------------
clear = 0.4;   // required clearance (mm)
module sweep_cards() { cyl_y(r_seat_out + clear, -46 - clear, 46 + clear); }                       // seated cards + cage play
module sweep_discs() { for (s=[-1,1]) translate([0, s > 0 ? 46 : -49 - clear, 0]) cyl_y(r_tip + clear, 0, 3 + clear); }
module sweep_tab()   { difference() { cyl_y(index_r1 + clear, 48.5, 53 + clear); cyl_y(index_r0 - clear, 48, 54); } }
module sweep_hub()   { for (s=[-1,1]) translate([0, s > 0 ? 49 : -55 - clear, 0]) cyl_y(hub_boss_r + clear, 0, 6 + clear); }
module sweep_all()   { sweep_cards(); sweep_discs(); sweep_tab(); sweep_hub(); }
// the hub and spacer turn with the wheel but must clear the towers' inner faces and the bearing (checked in sweep_towers via sweep_hub)

// ---- fixed parts in their assembled positions ---------------------------------------------------
module shutter_at(open=false) { rotate([0, open ? -shutter_open_deg : 0, 0]) shutter(); }
module feeder_parts() { at_fin(entry_fin_ang, 1) { feeder_deck(); gate_block(); feeder_motor_bracket(); feeder_bushing_bracket();
    idler_arm(nipE_r); spring_bar(nipE_r);
    translate([feed_r, -roller_len/2, 0.7 - 13.5]) rotate([-90,0,0]) roller_feed();
    translate([nipE_r, -roller_len/2, 0.5 - 8]) rotate([-90,0,0]) roller_nip(); } }
module feeder_motors() { at_fin(entry_fin_ang, 1) { for (p=[[feed_r, 0.7 - 13.5], [nipE_r, 0.5 - 8]]) translate([p[0] - 6, roller_len/2 + 0.5, p[1] - 5]) cube([12, 24, 10]); } }
module exit_parts() { at_fin(exit_fin_ang, -1) { chute(); idler_arm(nipX_r); spring_bar(nipX_r); translate([nipX_r, -roller_len/2, 0.5 - 8]) rotate([-90,0,0]) roller_nip(); } }
module exit_motor() { at_fin(exit_fin_ang, -1) translate([nipX_r - 6, roller_len/2 + 0.5, 0.5 - 8 - 5]) cube([12, 24, 10]); }
module towers() { tower(1); tower(-1); }
module wheel_extras() { cyl_y(hub_boss_r, -cage_w/2 - disc_t - hub_boss_len, -cage_w/2 - disc_t); }   // hub_spacer in place
module shrouds() { shroud_segment(shroud_a0, 270, true); shroud_segment(270, shroud_a1, false); }

module pair(a, b) { intersection() { children(0); children(1); } }

if (check == "sweep_feeder")   intersection() { sweep_all(); feeder_parts(); }
if (check == "sweep_motors")   intersection() { sweep_all(); union() { feeder_motors(); exit_motor(); } }
if (check == "sweep_exit")     intersection() { sweep_all(); exit_parts(); }
if (check == "sweep_towers")   intersection() { sweep_all(); towers(); }
if (check == "sweep_shroud")   intersection() { sweep_all(); shrouds(); }
if (check == "sweep_shutter_closed") intersection() { sweep_all(); shutter_at(false); }
if (check == "sweep_shutter_open")   intersection() { sweep_all(); shutter_at(true); }
if (check == "sweep_index")    intersection() { sweep_all(); index_bracket(); }
if (check == "motors_towers")  intersection() { union() { feeder_motors(); exit_motor(); } towers(); }
if (check == "brackets_towers") intersection() { at_fin(entry_fin_ang, 1) { feeder_motor_bracket(); feeder_bushing_bracket(); } towers(); }
if (check == "idler_deck")     intersection() { at_fin(entry_fin_ang, 1) idler_arm(nipE_r); at_fin(entry_fin_ang, 1) { feeder_deck(); gate_block(); spring_bar(nipE_r); } }
if (check == "idler_chute")    intersection() { at_fin(exit_fin_ang, -1) idler_arm(nipX_r); at_fin(exit_fin_ang, -1) { chute(); spring_bar(nipX_r); } }
if (check == "shutter_open_vs_fixed")   intersection() { shutter_at(true);  union() { exit_parts(); towers(); shrouds(); } }
if (check == "shutter_closed_vs_fixed") intersection() { shutter_at(false); union() { exit_parts(); towers(); shrouds(); } }
if (check == "shutter_mid_vs_fixed")    intersection() { rotate([0, -shutter_open_deg/2, 0]) shutter(); union() { exit_parts(); towers(); shrouds(); } }
if (check == "chute_shroud")   intersection() { at_fin(exit_fin_ang, -1) chute(); shrouds(); }
if (check == "deck_shroud")    intersection() { at_fin(entry_fin_ang, 1) feeder_deck(); shrouds(); }
if (check == "list") echo("set -D check=...");
// ---- mating parts that must touch only on their mounting faces (added in the print-readiness sweep)
if (check == "chute_towers")   intersection() { at_fin(exit_fin_ang, -1) chute(); towers(); }
if (check == "deck_towers")    intersection() { at_fin(entry_fin_ang, 1) { feeder_deck(); gate_block(); } towers(); }
if (check == "shroud_towers")  intersection() { shrouds(); towers(); }
if (check == "index_towers")   intersection() { index_bracket(); towers(); }
if (check == "motor_mount_tower") intersection() { motor_mount(); tower(1); }
if (check == "lid_deck")       intersection() { at_fin(entry_fin_ang, 1) lid(); at_fin(entry_fin_ang, 1) feeder_deck(); }
