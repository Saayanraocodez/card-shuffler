// params.scad — every dimension of the card shuffler in one place (mm).
// Coordinate frame ("card frame"): X = card travel (+X toward the well), Y = across the card
// (0 = machine centreline), Z = normal to the cards (+Z = up). The card path plane is Z = 0.
// The whole card frame is tilted TILT degrees (well end low) when mounted on the skirts.

// ---- cards -----------------------------------------------------------------
card_w       = 63.5;   // poker width
card_l       = 88.9;   // poker length
card_t_nom   = 0.30;
card_t_max   = 0.42;
n_cards_max  = 54;
stack_max    = n_cards_max * card_t_max;   // 22.7

// ---- global ----------------------------------------------------------------
TILT         = 10;     // degrees, card path descends toward the well
wall         = 3;      // default wall thickness
plate_t      = 3;      // deck plate thickness (plate occupies Z = -plate_t .. 0)
clr          = 0.30;   // sliding clearance for printed-on-printed fits
hole_m3      = 2.6;    // M3 self-tapping in PLA/PETG
hole_m3_free = 3.4;    // M3 clearance
hole_m2      = 1.7;    // M2 self-tapping (servo tabs, endstop)
$fn          = 48;
H0           = 90;     // world height (mm above the table) of the card plane at X = 0

// ---- hopper / feeder -------------------------------------------------------
hop_in_w     = card_w + 1.5;      // 65.0 interior width
hop_in_l     = card_l + 2.1;      // 91.0 interior length
gate_x       = 10;                // inner face of the gate (front) wall
hop_back_x   = gate_x - hop_in_l; // -81 inner face of back wall
hop_h        = 50;                // hopper wall height above the floor
gate_gap_nom = 0.45;              // set with a feeler gauge; 0.30 .. 0.60
gate_win_h   = 16;                // window in front wall for the gate block
gate_block_t = 3;

roller_d     = 27;     // O-ring crown diameter (ID20 x CS3.5 O-rings on a hub)
roller_hub_d = 24;
oring_root_d = 20;
oring_cs     = 3.5;
oring_y      = [-18, 0, 18];
roller_len   = 64;     // hub length (Y -32..+32)
roller_stub  = 8;      // stub axle length on the -Y end
feed_x       = -4;     // feed roller axis X
feed_protr   = 0.7;    // crown above hopper floor
trans_x      = 32;     // transport roller axis X
trans_protr  = 0.5;
n20_w        = 12; n20_h = 10; n20_len = 24; n20_shaft_d = 3; n20_shaft_l = 9;

rail_h       = 6;      // side rails guiding the card between gate and well
beamB_x      = 19;     // vertical gate beam (emitter in plate, detector in idler arm)
idler_pivot_x= 22;
idler_x      = trans_x;
idler_d      = 12; idler_w = 6; idler_y = [-20, 20];

// ---- well ------------------------------------------------------------------
well_front_x = trans_x + roller_d/2 + 1.5;   // 47.0 outer face of well front wall
well_wall    = 3;
well_in_w    = card_w + 1.5;                 // 65
well_in_l    = card_l + 2.1;                 // 91.0 (incl. 1.5 mm back pad)
well_x0      = well_front_x + well_wall;     // 50 inner front face
well_x1      = well_x0 + well_in_l;          // 141 inner back face
well_back_x  = well_x1 + well_wall;          // 144 outer back face
well_h       = 38;                           // rim height above Z=0
well_floor_z = -30;                          // sleeve bottom
slot_z0      = -1.0; slot_z1 = 3.5;          // card entry slot in the front wall
knife_z      = 2.5;                          // blade mid-plane
knife_t      = 0.5;
knife_slot_h = 1.6;
knife_len_x  = 40;                           // blade width along X
knife_cx     = (well_x0 + well_x1)/2;        // 95.5
knife_entry  = 7;                            // protrusion into the well when extended
knife_travel = 8;
bar_len_y    = 12; bar_h = 6;                // knife bar section (Y x Z)
bar_len      = 70;                           // along X
blade_w_y    = 22;                           // blade extent along Y
finger_w     = 22; finger_d = 22;            // finger notches at the top of the well side walls
beamS_z      = 14; beamS_x = knife_cx + 22;  // stack-top beam (horizontal, through side walls)
beamW_y      = 12;                           // vertical beam through the entry slot, at this Y
back_pad_t   = 1.5;
arm_slot_w   = 7;                            // slot in back wall for carriage arm
arm_w        = 6;

// ---- elevator column (behind the well) -------------------------------------
col_x0       = well_back_x + 2;   // 146 column front face
col_depth    = 26;                // X extent (146..172)
col_hw       = 33;                // inner half width
screw_x      = col_x0 + 12;       // 158 lead screw axis
screw_y      = 0;
rod_y        = [-22, 22];
rod_d        = 8; rod_len = 120;
col_bot_z    = -58;               // bottom plate lower face
col_top_z    = 61;                // top plate upper face (motor sits here)
bush_d       = 15; bush_len = 24; // LM8UU
nema_w       = 42.3; nema_bolt = 31; nema_boss_d = 22.5; nema_len = 40;
coupler_d    = 19; coupler_len = 25;
t8nut_flange_d = 22; t8nut_body_d = 10.2; t8nut_pcd = 16; t8nut_h = 15;
endstop_z    = -47;               // microswitch mounting height (lever tip ≈ -40)

// platform (inside the well, on the carriage)
plat_w       = 45;  plat_l = 86; plat_t = 4;
plat_x0      = well_x0 + 2;       // 52
z_travel_min = -25; z_travel_max = 22;

// ---- servos (one per side, MG90S, shaft up, body under the plate) -----------
servo_body   = [22.8, 12.2, 22.5];   // X, Y, Z
servo_flange = [32.5, 12.2, 2.5];
servo_hole_sp= 28;
horn_r       = 8;      // pin radius on the servo horn; ±30° swing gives 8 mm travel

// ---- skirts (world frame) --------------------------------------------------
skirt_hw     = 63;     // half width of both skirts
