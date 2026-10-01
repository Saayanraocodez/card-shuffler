// params.scad — every dimension of the WHEEL card shuffler (mm).
//
// Frame: the wheel axis is the Y axis, horizontal.  X is horizontal, Z is up, origin on the axis.
// Angles are measured in the XZ plane from +X (3 o'clock) counter-clockwise: 90° = 12 o'clock,
// 180° = 9 o'clock, 270° = 6 o'clock.  "Fin k" is a radial plate whose counter-clockwise face lies
// exactly on the radial plane at angle (wheel_angle + k*pitch_deg); slot k is the space between
// fin k and fin k+1.  A card in slot k lies on fin k at the entry (upper right) and on fin k+1 at
// the exit (lower left).  Cards enter and leave radially, lying flat on a fin.

// ---- cards -------------------------------------------------------------------
card_w      = 63.5;   // radial extent in the wheel (short edge)
card_l      = 88.9;   // axial extent (long edge, along Y)
card_t_max  = 0.42;
n_slots     = 54;
pitch_deg   = 360 / n_slots;    // 6.667°

// ---- wheel (cage) ----------------------------------------------------------------
r_hub       = 22;      // outer radius of the hub tube = card inner-edge stop
hub_wall    = 2;
r_seat_out  = r_hub + card_w;          // 85.5 outer edge of a seated card
r_tip       = r_seat_out - 8;          // 77.5 fin tips and disc radius (cards overhang 8 mm)
fin_t       = 1.4;                     // constant fin thickness (slot at hub = 2.56 - 1.4 = 1.16)
cage_w      = card_l + 3.1;            // 92 clear width between the discs
disc_t      = 3;
disc_ring_in= 62;                      // spoked disc: ring from here to r_tip
n_spokes    = 6;
hub_boss_r  = 15;  hub_boss_len = 6;   // bosses outside the discs (nuts clamp against them)
bore_d      = 8.6;                     // M8 threaded rod
index_ang   = 90;  index_r0 = 66; index_r1 = 76; index_w = 4;   // home tab on the +Y disc face (inside the disc rim, clear of beam E)
index_led_r = 59;  index_pt_r = 83;  index_beam_y = 51;             // radial index beam across the tab's path (+Y side, 12 o'clock)

// ---- shroud (retains cards in the lower half) ------------------------------------
r_shroud_in = r_seat_out + 3;          // 88.5
shroud_t    = 3;
shroud_w    = 92;                      // axial width: covers the card edges (|Y| <= 46 incl. play); discs are inside r 77.5
shroud_a0   = 172;  shroud_a1 = 368;   // arc covered (two segments split at 270°)
win_a0      = 202;  win_a1 = 214;      // exit window
// Shutter: a thin curved blade riding on the shroud's INNER surface.  Closed, it covers the window so a
// card edge rides over it on ramps (no edge to catch); open, it slides counter-clockwise past the window.
shut_r_in   = 87.0;  shut_r_out = r_shroud_in - 0.3;     // blade 87.0 .. 88.2
shut_ramp   = 2.6;                     // degrees of taper at each end (thickness 1.2 -> 0.4)
shut_a0     = win_a0 - shut_ramp;      // 199.4  closed: blade 199.4 .. 216.6
shut_a1     = win_a1 + shut_ramp;      // 216.6
shutter_open_deg = 17.5;               // opening slide, counter-clockwise
shut_hw     = 48.5;                    // blade half width (shroud is ±46; the blade overhangs it beside the discs' rims)
shut_tab_a  = 213;  shut_tab_r0 = 82; shut_tab_r1 = 91.5;   // drive tab at the -Y end (closed position)
shut_pin_r  = 86.75;                   // nominal pin radius on the tab
horn_r      = 18;                      // servo horn pin radius
// servo axis: on the perpendicular bisector of the pin's two end positions, inside the pin arc
shut_mid_a  = shut_tab_a + shutter_open_deg/2;                                 // 221.75°
shut_skirt_a1 = shut_tab_a + shutter_open_deg + 3 + 3;                        // -Y shroud skirt left off from the window to here (drive tab travel)
shut_half_chord = shut_pin_r * sin(shutter_open_deg/2);
servo_ax_r  = shut_pin_r * cos(shutter_open_deg/2) - sqrt(horn_r*horn_r - shut_half_chord*shut_half_chord);   // ≈ 73.4

// ---- entry (feeder) --------------------------------------------------------------
entry_slot_ang = 50;
entry_fin_ang  = entry_slot_ang - pitch_deg/2;   // 46.667° fin k's CCW face = feeder plate plane
// Nothing at the entry may reach inside r_seat_out (85.5): seated cards sweep that radius when the wheel turns.
nipE_r         = r_seat_out + 10.5;    // 96 entry nip roller axis (Ø16 rollers; nearest point r 88.3)
deck_tip_r     = nipE_r - 7.5;         // 88.5 the plate starts at the nip; the card bridges 11 mm to the fin tips
beamE_r        = 81.5;                 // "card in slot" beam: crosses the card plane obliquely between the towers
beamS_r        = 86.8;                 // "card seated" beam: blocked while a card still bridges plate and slot
beam_dz        = 4;                    // the oblique beams run from local z +4 (+Y tower) to -4 (-Y tower)
beamB_r        = nipE_r + 10.5;        // 106.5 gate beam
gate_r         = nipE_r + 22;          // 118 gate lip (inner face of gate wall); leaves room for the idler pivot
feed_r         = gate_r + 11.5;        // 129.5 feed roller axis (Ø27)
hop_len        = card_w + 1.5;         // 65 hopper interior along the radius
hop_w          = card_l + 2.1;         // 91 hopper interior across (Y)
hop_h          = 50;
wall           = 3;  plate_t = 3;
gate_gap_nom   = 0.45;  gate_win_h = 16;  gate_block_t = 3;
feed_roller_d  = 27;  feed_hub_d = 24;  oring_root_d = 20;  oring_cs = 3.5;
nip_roller_d   = 16;  nip_hub_d = 13;   nip_oring_root = 10;  nip_oring_cs = 3.0;
roller_len     = 64;  roller_stub = 8;   // short enough that the N20 motor fits inside the towers (|Y| < 58)
oring_y        = [-27, -10, 10, 27];
feed_protr     = 0.7;  nip_protr = 0.5;
n20_w = 12; n20_h = 10; n20_len = 24;
idler_d = 12; idler_w = 6; idler_y = [-27, 27];   // directly over the outer O-rings of the driven roller
rail_h = 6;
post_h = 23;            // idler pivot posts: top face, where the separate spring_bar screws on
spring_bar_y = 52;      // spring_bar screw positions (|Y|), in the middle of the 49..55 posts

// ---- exit (chute) ----------------------------------------------------------------
exit_slot_ang = 205;
exit_fin_ang  = exit_slot_ang + pitch_deg/2;    // 208.333° fin k+1; card lies on its CW face
nipX_r        = r_shroud_in + shroud_t + 1 + 8;  // 100.5 exit nip roller axis
beamX_r       = nipX_r + 10.5;                   // 111 exit beam (detector in the exit idler arm, like beam B)
chute_r0      = nipX_r + 9;                      // 109.5 chute floor starts
chute_len     = card_w + 9.5;                    // 73: the pile (against the end wall) starts 8 mm past beam X
chute_drop    = 28;                              // floor below the exit plane: a 54-card thick deck stays below the plane
chute_wall_h  = 4;                               // side walls only guide; low so the footprint stays under 12 in
chute_screw_x = nipX_r + 10.5;                   // 111: horizontal M3 x 20 screws through the tower cheeks into the nip housing
chute_screw_z = [-10, -22];                      //       (clear of the +Y motor pocket, which ends at x 106.7)

// ---- towers / frame --------------------------------------------------------------
tower_y_in    = 58;     // inner faces of the two towers
tower_t       = 12;
tower_x0      = -100; tower_x1 = 100;   // foot extent (plate fits a 220 mm bed; the chute passes beyond x -100)
tower_z_bot   = -100;   // base top (world Z of the wheel axis above the base = 100)
bearing_d     = 22.2;  bearing_t = 7;           // 608ZZ
nema_w = 42.3; nema_bolt = 31; nema_boss_d = 22.5; nema_len = 40;
coupler_d = 19; coupler_len = 25;
motor_standoff = 28;    // tower outer face → motor mounting face (coupler lives inside the standoff)
rod_len = 200;          // M8 threaded rod
servo_body = [22.8, 12.2, 22.5]; servo_flange = [32.5, 12.2, 2.5]; servo_hole_sp = 28;
hole_m3 = 2.6; hole_m3_free = 3.4; hole_m2 = 1.7; hole_m4_free = 4.4;
$fn = 64;

// ---- base ------------------------------------------------------------------------
base_x0 = -100; base_x1 = 133; base_hw = 105; base_h = 45;   // the chute end overhangs the rear base and clears the table by ~30 mm
