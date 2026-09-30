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
index_ang   = 90;  index_r0 = 72; index_r1 = 84; index_w = 3;   // home tab on the +Y disc

// ---- shroud (retains cards in the lower half) ------------------------------------
r_shroud_in = r_seat_out + 3;          // 88.5
shroud_t    = 3;
shroud_w    = 100;                     // axial width (covers card edges and disc rims)
shroud_a0   = 172;  shroud_a1 = 368;   // arc covered (two segments split at 270°)
win_a0      = 202;  win_a1 = 214;      // exit window
shutter_r0  = r_shroud_in + shroud_t + 0.5;   // 92.0 shutter slides over the shroud
shutter_t   = 3;   shutter_span = 15;  // degrees
shutter_open_deg = 14;                 // servo moves it this far counter-clockwise to open (≈23 mm)
shutter_servo_r  = 108;  shutter_servo_ang = win_a0 + shutter_span/2 + shutter_open_deg/2;   // 216.5°, on the -Y tower
shutter_horn_r   = 15;                 // horn pin radius; ±50° swing = 23 mm

// ---- entry (feeder) --------------------------------------------------------------
entry_slot_ang = 50;
entry_fin_ang  = entry_slot_ang - pitch_deg/2;   // 46.667° fin k's CCW face = feeder plate plane
deck_tip_r     = r_tip + 4.5;          // 82 inner end of the feeder plate
beamE_r        = 84.5;                 // occupancy beam (perpendicular through the plate)
nipE_r         = r_seat_out + 1 + 8;   // 94.5 entry nip roller axis (Ø16 rollers)
beamB_r        = nipE_r + 10.5;        // 105 gate beam
gate_r         = nipE_r + 15;          // 109.5 gate lip (inner face of gate wall)
feed_r         = gate_r + 11.5;        // 121 feed roller axis (Ø27)
hop_len        = card_w + 1.5;         // 65 hopper interior along the radius
hop_w          = card_l + 2.1;         // 91 hopper interior across (Y)
hop_h          = 50;
wall           = 3;  plate_t = 3;
gate_gap_nom   = 0.45;  gate_win_h = 16;  gate_block_t = 3;
feed_roller_d  = 27;  feed_hub_d = 24;  oring_root_d = 20;  oring_cs = 3.5;
nip_roller_d   = 16;  nip_hub_d = 13;   nip_oring_root = 10;  nip_oring_cs = 3.0;
roller_len     = 90;  roller_stub = 8;
oring_y        = [-33, -11, 11, 33];
feed_protr     = 0.7;  nip_protr = 0.5;
n20_w = 12; n20_h = 10; n20_len = 24;
idler_d = 12; idler_w = 6; idler_y = [-28, 28];
rail_h = 6;

// ---- exit (chute) ----------------------------------------------------------------
exit_slot_ang = 205;
exit_fin_ang  = exit_slot_ang + pitch_deg/2;    // 208.333° fin k+1; card lies on its CW face
nipX_r        = shutter_r0 + shutter_t + 1 + 8;  // 104 exit nip roller axis
beamX_r       = nipX_r + 10;                     // 114
chute_r0      = nipX_r + 9;                      // 113 chute floor starts
chute_len     = card_w + 16;                     // 79.5
chute_drop    = 20;                              // floor below the exit plane
chute_wall_h  = 30;

// ---- towers / frame --------------------------------------------------------------
tower_y_in    = 58;     // inner faces of the two towers
tower_t       = 12;
tower_x0      = -117; tower_x1 = 100;   // foot extent (plate fits a 220 mm bed)
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
base_x0 = -185; base_x1 = 135; base_hw = 105; base_h = 45;
