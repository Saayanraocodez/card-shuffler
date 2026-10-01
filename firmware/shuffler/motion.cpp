// motion.cpp
#include "motion.h"
#include "hardware.h"
#include <math.h>

static const float SPD = STEPS_PER_REV / 360.0f;   // steps per degree
static float s_angle = 0;                          // deg, CCW positive, fin-0 reference

static float wrap360(float a) { while (a < 0) a += 360; while (a >= 360) a -= 360; return a; }
float wheel_angle() { return s_angle; }
void  wheel_idle() { stepper_enable(false); }
static bool stop_on_index() { return index_active(); }

bool wheel_path_clear() {
    beams_poll();
    if (beam_blocked(BEAM_S)) { Serial.println("# wheel move refused: a card bridges the entry (beam S)"); return false; }
    if (beam_blocked(BEAM_X)) { Serial.println("# wheel move refused: a card is in the exit nip (beam X)"); return false; }
    return true;
}

bool wheel_home() {
    if (!wheel_path_clear()) return false;
    // leave the tab if we are on it, then rotate CCW until the tab blocks the index beam; approach slowly for repeatability
    if (index_active()) stepper_move((long)(15 * SPD), WHEEL_V_HOME_DPS, WHEEL_ACC_DPS2);
    stepper_move((long)(370 * SPD), WHEEL_V_HOME_DPS, WHEEL_ACC_DPS2, stop_on_index);
    if (!index_active()) return false;
    stepper_move(-(long)(4 * SPD), WHEEL_V_HOME_DPS, WHEEL_ACC_DPS2);
    stepper_move((long)(8 * SPD), 15.0f, WHEEL_ACC_DPS2, stop_on_index);
    if (!index_active()) return false;
    stepper_set_pos(0);
    s_angle = wrap360(cal.home_offset_deg);
    return true;
}

bool wheel_goto(float target, float vmax) {
    target = wrap360(target);
    float d = target - s_angle;
    if (d > 180) { d -= 360; }
    if (d < -180) { d += 360; }          // shortest path
    long steps = lroundf(d * SPD);
    if (steps == 0) return true;
    if (!wheel_path_clear()) return false;
    stepper_move(steps, vmax, WHEEL_ACC_DPS2);
    s_angle = wrap360(s_angle + steps / SPD);
    return true;
}
bool wheel_fin_to_entry(uint8_t fin) { return wheel_goto(ENTRY_FIN_DEG + cal.entry_trim_deg - fin * PITCH_DEG); }
bool wheel_fin_to_exit(uint8_t slot) { return wheel_goto(EXIT_FIN_DEG + cal.exit_trim_deg - (slot + 1) * PITCH_DEG); }

int wheel_scan(bool occupied[N_SLOTS_HW]) {
    int n = 0;
    for (int s = 0; s < N_SLOTS_HW; s++) {
        if (!wheel_fin_to_entry((uint8_t)s)) return -1;
        delay(30);
        occupied[s] = beam_blocked_now(BEAM_E);
        n += occupied[s];
    }
    return n;
}

// ============================================================ feeder
static uint32_t s_median_b = 0, s_median_x = 0;
// Running-median-ish double-card heuristic shared by the entry (beam B) and exit (beam X).
static bool double_check(uint32_t* median, uint32_t ms) {
    if (*median == 0) { *median = ms; return false; }
    bool suspect = ms > (uint32_t)(*median * DOUBLE_FEED_RATIO);
    if (!suspect) *median = (*median * 7 + ms) / 8;    // suspects do not drag the median up
    return suspect;
}
const char* feed_result_name(FeedResult r) {
    switch (r) { case FEED_OK: return "ok"; case FEED_NO_CARD: return "no card"; case FEED_JAM_PICK: return "jam: pickup";
        case FEED_JAM_ENTRY: return "jam: gate to wheel"; case FEED_JAM_CLEAR: return "jam: not clearing gate";
        case FEED_NOT_SEATED: return "card not in the target slot"; case FEED_STUCK_MOUTH: return "card stuck in the slot mouth"; }
    return "?";
}
void feeder_stop() { motor_run(M_FEED, 0); motor_run(M_NIPE, 0); }
static bool wait_beam(Beam b, bool want_blocked, uint32_t timeout_ms) {
    uint32_t t0 = millis(); while (millis() - t0 < timeout_ms) { beams_poll(); if (beam_blocked(b) == want_blocked) return true; } return false;
}
bool feeder_probe_hopper() {
    motors_sleep(false); beams_poll();
    if (beam_blocked(BEAM_B)) return true;
    motor_run(M_FEED, cal.feed_pwm); bool got = wait_beam(BEAM_B, true, PROBE_MS); motor_run(M_FEED, 0);
    return got;
}
FeedResult feeder_feed_one(FeedStats* st) {
    st->b_block_ms = 0; st->double_suspect = false;
    motors_sleep(false);
    motor_run(M_NIPE, cal.nipe_pwm); motor_run(M_FEED, cal.feed_pwm);
    if (!wait_beam(BEAM_B, true, T_PICK_MS)) { feeder_stop(); return FEED_NO_CARD; }
    uint32_t t_block = millis();
    if (!wait_beam(BEAM_E, true, T_E_BLOCK_MS)) { feeder_stop(); return beam_blocked_now(BEAM_S) ? FEED_STUCK_MOUTH : FEED_JAM_ENTRY; }   // leading edge inside the slot
    if (!wait_beam(BEAM_B, false, T_B_CLEAR_MS)) { feeder_stop(); return FEED_JAM_CLEAR; }      // trailing edge past the gate
    motor_run(M_FEED, 0);
    st->b_block_ms = millis() - t_block;
    delay(T_SEAT_MS);                                                                          // nip pushes the rest; card slides to the hub
    motor_run(M_NIPE, 0);
    st->double_suspect = double_check(&s_median_b, st->b_block_ms);
    // the trailing edge must leave the mouth (beam S clear); nudge with the nip if it stopped there
    bool seated = wait_beam(BEAM_S, false, T_SEAT_WAIT_MS);
    for (int k = 0; !seated && k < 2; k++) {
        motor_run(M_NIPE, cal.nipe_pwm); delay(T_NUDGE_MS); motor_run(M_NIPE, 0);
        seated = wait_beam(BEAM_S, false, T_SEAT_WAIT_MS);
    }
    if (!seated) return FEED_STUCK_MOUTH;
    delay(40);
    if (!beam_blocked_now(BEAM_E)) return FEED_NOT_SEATED;                                     // card should now sit in the slot at the entry
    return FEED_OK;
}
void feeder_reverse_pulse() { motors_sleep(false); motor_run(M_FEED, -cal.feed_pwm); motor_run(M_NIPE, -cal.nipe_pwm); delay(T_REVERSE_MS); feeder_stop(); delay(60); }

// ============================================================ unload
void eject_stop() { motor_run(M_NIPX, 0); }
EjectResult eject_one(EjectStats* st) {
    st->x_block_ms = 0; st->double_suspect = false;
    motors_sleep(false);
    motor_run(M_NIPX, cal.nipx_pwm);
    if (!wait_beam(BEAM_X, true, T_X_BLOCK_MS)) { eject_stop(); return EJECT_NO_CARD; }
    uint32_t t0 = millis();
    if (!wait_beam(BEAM_X, false, T_X_CLEAR_MS)) { eject_stop(); return EJECT_JAM; }
    st->x_block_ms = millis() - t0;
    // Two cards that left one slot together are usually offset by a few mm and block beam X for longer.
    // Two perfectly stacked cards are NOT detectable this way; the end-of-shuffle count catches those.
    st->double_suspect = double_check(&s_median_x, st->x_block_ms);
    delay(60); eject_stop();
    return EJECT_OK;
}
