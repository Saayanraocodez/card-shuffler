// dealer_check.scad — collision checks for the dealer additions (openscad -D 'check="..."'; empty result = clear)
use <shuffler.scad>
include <params.scad>
mot_y0 = roller_len/2 + 0.5;   // as in shuffler.scad
check = "arm_chute";
module launch_frame() { at_fin(exit_fin_ang, -1) translate([0, 0, -chute_drop]) children(); }
module throw_roller() { translate([throw_r, -roller_len/2, feed_roller_d/2 + 0.1]) rotate([-90,0,0]) { roller_feed(); for (y=oring_y) translate([0,0,y + roller_len/2]) oring_ring(oring_root_d, oring_cs); } }
module throw_motor()  { translate([throw_r, mot_y0, feed_roller_d/2 + 0.1]) n20_y(); }
if (check == "arm_chute")      intersection() { launch_frame() translate([0,0,0.1]) throw_arm(); at_fin(exit_fin_ang, -1) chute(true); }
if (check == "roller_chute")   intersection() { launch_frame() throw_roller(); at_fin(exit_fin_ang, -1) chute(true); }
if (check == "roller_arm")     intersection() { launch_frame() throw_roller(); launch_frame() throw_arm(); }
if (check == "motor_chute")    intersection() { launch_frame() throw_motor(); at_fin(exit_fin_ang, -1) chute(true); }
if (check == "arm_bases")      intersection() { launch_frame() throw_arm(); translate([0,0,tower_z_bot - base_h]) { base_front(); base_rear(); } }
if (check == "chute_bases")    intersection() { at_fin(exit_fin_ang, -1) chute(true); translate([0,0,tower_z_bot - base_h]) { base_front(); base_rear(); } }
// turntable (machine frame): the rotating side must clear the static side at every angle
if (check == "tt_rot_foot")    for (a=[0:30:330]) intersection() { translate([tt_c[0], tt_c[1], 0]) rotate([0,0,a]) translate([-tt_c[0], -tt_c[1], 0]) tt_cover_front(); tt_foot(); }
// pinion meshing with the static gear at rest (phased half a tooth); it rolls round the gear, so one position is representative
if (check == "tt_mesh")        intersection() { translate([tt_motor_c()[0], tt_motor_c()[1], 0]) rotate([0,0,tt_motor_dir + 180/pinion_z]) tt_pinion(); tt_foot(); }
if (check == "tt_motor_base")  intersection() { translate([tt_motor_c()[0] - nema_w/2, tt_motor_c()[1] - nema_w/2, tt_cover_t]) cube([nema_w, nema_w, tt_motor_len]); base_front(); }
if (check == "tt_cover_base")  intersection() { tt_cover_front(); base_front(); }
