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

bool wheel_home() {
    // leave the tab if we are on it, then rotate CCW until the tab enters the slot; approach slowly for repeatability
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

void wheel_goto(float target, float vmax) {
    target = wrap360(target);
    float d = target - s_angle;
    if (d > 180) { d -= 360; }
    if (d < -180) { d += 360; }          // shortest path
    long steps = lroundf(d * SPD);
    stepper_move(steps, vmax, WHEEL_ACC_DPS2);
    s_angle = wrap360(s_angle + steps / SPD);
}
void wheel_fin_to_entry(uint8_t fin) { wheel_goto(ENTRY_FIN_DEG + cal.entry_trim_deg - fin * PITCH_DEG); }
void wheel_fin_to_exit(uint8_t slot)  { wheel_goto(EXIT_FIN_DEG + cal.exit_trim_deg - (slot + 1) * PITCH_DEG); }

int wheel_scan(bool occupied[N_SLOTS_HW]) {
    // Put slot 0 at the entry, then step through all slots reading beam E at each.
    int n = 0;
    for (int s = 0; s < N_SLOTS_HW; s++) {
        wheel_fin_to_entry((uint8_t)s);
        delay(30);
        occupied[s] = beam_blocked_now(BEAM_E);
        n += occupied[s];
    }
    return n;
}

// ============================================================ feeder
static uint32_t s_median_ms = 0;
const char* feed_result_name(FeedResult r) {
    switch (r) { case FEED_OK: return "ok"; case FEED_NO_CARD: return "no card"; case FEED_JAM_PICK: return "jam: pickup";
        case FEED_JAM_ENTRY: return "jam: gate to wheel"; case FEED_JAM_CLEAR: return "jam: not clearing gate"; case FEED_NOT_SEATED: return "card not seen in slot"; }
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
    if (!wait_beam(BEAM_E, true, T_E_BLOCK_MS)) { feeder_stop(); return FEED_JAM_ENTRY; }        // leading edge at the slot mouth
    if (!wait_beam(BEAM_B, false, T_B_CLEAR_MS)) { feeder_stop(); return FEED_JAM_CLEAR; }      // trailing edge past the gate
    motor_run(M_FEED, 0);
    st->b_block_ms = millis() - t_block;
    delay(T_SEAT_MS);                                                                          // nip pushes the rest; card slides to the hub
    motor_run(M_NIPE, 0);
    if (s_median_ms == 0) s_median_ms = st->b_block_ms;
    else { if (st->b_block_ms > (uint32_t)(s_median_ms * DOUBLE_FEED_RATIO)) st->double_suspect = true; s_median_ms = (s_median_ms * 7 + st->b_block_ms) / 8; }
    delay(40);
    if (!beam_blocked_now(BEAM_E)) return FEED_NOT_SEATED;                                     // card should now sit in the slot at the entry
    return FEED_OK;
}
void feeder_reverse_pulse() { motors_sleep(false); motor_run(M_FEED, -cal.feed_pwm); motor_run(M_NIPE, -cal.nipe_pwm); delay(T_REVERSE_MS); feeder_stop(); delay(60); }

// ============================================================ unload
void eject_stop() { motor_run(M_NIPX, 0); }
EjectResult eject_one() {
    motors_sleep(false);
    motor_run(M_NIPX, cal.nipx_pwm);
    if (!wait_beam(BEAM_X, true, T_X_BLOCK_MS)) { eject_stop(); return EJECT_NO_CARD; }
    if (!wait_beam(BEAM_X, false, T_X_CLEAR_MS)) { eject_stop(); return EJECT_JAM; }
    delay(60); eject_stop();
    return EJECT_OK;
}
