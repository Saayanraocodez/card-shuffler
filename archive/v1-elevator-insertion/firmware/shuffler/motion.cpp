// motion.cpp — elevator and feeder procedures.
#include "motion.h"
#include "hardware.h"
#include <math.h>

// ============================================================ elevator
static float s_z = Z_HOME_DEFAULT;     // current platform Z (mm) — tracked from steps

float knife_z() { return Z_KNIFE_NOMINAL + cal.knife_offset; }
float elevator_z() { return s_z; }
void elevator_idle() { stepper_enable(false); }

static bool stop_on_endstop() { return endstop_hit(); }
static bool stop_on_beam_s()  { return beam_blocked_now(BEAM_S); }

bool elevator_home() {
    // 1. if already on the switch, back off up 3 mm
    if (endstop_hit()) { stepper_move((long)(3.0f * STEPS_PER_MM), ELEV_V_HOME, ELEV_ACC); if (endstop_hit()) return false; }
    // 2. travel down until the switch trips (limit: full travel + margin)
    long maxdown = (long)((Z_MAX - Z_MIN + 12.0f) * STEPS_PER_MM);
    stepper_move(-maxdown, ELEV_V_HOME, ELEV_ACC, stop_on_endstop);
    if (!endstop_hit()) return false;
    // 3. back off 1.5 mm and re-approach slowly for repeatability
    stepper_move((long)(1.5f * STEPS_PER_MM), ELEV_V_HOME, ELEV_ACC);
    stepper_move(-(long)(3.0f * STEPS_PER_MM), 1.5f, ELEV_ACC, stop_on_endstop);
    if (!endstop_hit()) return false;
    stepper_set_pos(0);
    s_z = cal.z_home;
    return true;
}

void elevator_move_to(float z, float vmax) {
    if (z > Z_MAX) z = Z_MAX; if (z < Z_MIN) z = Z_MIN;
    long target = lroundf((z - cal.z_home) * STEPS_PER_MM);
    long delta = target - stepper_pos();
    stepper_move(delta, vmax, ELEV_ACC);
    s_z = cal.z_home + stepper_pos() / STEPS_PER_MM;
}

float elevator_find_beam_s(float z_limit) {
    if (beam_blocked_now(BEAM_S)) {
        // already blocked: go down until clear, then approach upward
        long down = (long)((s_z - Z_MIN) * STEPS_PER_MM);
        stepper_move(-down, ELEV_V_HOME, ELEV_ACC, [](){ return !beam_blocked_now(BEAM_S); });
        s_z = cal.z_home + stepper_pos() / STEPS_PER_MM;
        stepper_move(-(long)(1.0f * STEPS_PER_MM), ELEV_V_HOME, ELEV_ACC);
        s_z = cal.z_home + stepper_pos() / STEPS_PER_MM;
    }
    long up = (long)((z_limit - s_z) * STEPS_PER_MM);
    if (up <= 0) return NAN;
    stepper_move(up, ELEV_V_MEASURE, ELEV_ACC, stop_on_beam_s);
    s_z = cal.z_home + stepper_pos() / STEPS_PER_MM;
    return beam_blocked_now(BEAM_S) ? s_z : NAN;
}

// ============================================================ feeder
static uint32_t s_median_ms = 0;   // running estimate of the beam-B block time for a single card

const char* feed_result_name(FeedResult r) {
    switch (r) { case FEED_OK: return "ok"; case FEED_NO_CARD: return "no card"; case FEED_JAM_PICK: return "jam: pickup";
                 case FEED_JAM_GATE: return "jam: gate/nip"; case FEED_JAM_WELL: return "jam: well entry"; case FEED_JAM_CLEAR: return "jam: not clearing"; }
    return "?";
}
void feeder_stop() { motor_run(M_FEED, 0); motor_run(M_TRANS, 0); }

static bool wait_beam(Beam b, bool want_blocked, uint32_t timeout_ms) {
    uint32_t t0 = millis();
    while (millis() - t0 < timeout_ms) { beams_poll(); if (beam_blocked(b) == want_blocked) return true; }
    return false;
}

bool feeder_probe_hopper() {
    motors_sleep(false);
    beams_poll();
    if (beam_blocked(BEAM_B)) return true;                 // a card is already pre-staged over the gate beam
    motor_run(M_FEED, cal.feed_pwm);
    bool got = wait_beam(BEAM_B, true, PROBE_MS);
    motor_run(M_FEED, 0);
    return got;
}

FeedResult feeder_feed_one(FeedStats* st) {
    st->b_block_ms = 0; st->double_suspect = false;
    motors_sleep(false);
    motor_run(M_TRANS, cal.trans_pwm);
    motor_run(M_FEED, cal.feed_pwm);
    // 1. pickup: beam B must be (or become) blocked
    if (!wait_beam(BEAM_B, true, T_PICK_MS)) { feeder_stop(); return FEED_NO_CARD; }
    uint32_t t_block = millis();
    // 2. leading edge reaches the well slot
    if (!wait_beam(BEAM_W, true, T_W_BLOCK_MS)) { feeder_stop(); return FEED_JAM_GATE; }
    // 3. trailing edge passes the gate beam → stop the feed roller (next card stays pre-staged)
    if (!wait_beam(BEAM_B, false, T_B_CLEAR_MS)) { feeder_stop(); return FEED_JAM_CLEAR; }
    motor_run(M_FEED, 0);
    st->b_block_ms = millis() - t_block;
    // 4. trailing edge enters the well
    if (!wait_beam(BEAM_W, false, T_W_CLEAR_MS)) { feeder_stop(); return FEED_JAM_WELL; }
    delay(T_COAST_MS);
    motor_run(M_TRANS, 0);
    // double-feed heuristic on the beam-B block duration (a second card riding along lengthens it)
    if (s_median_ms == 0) s_median_ms = st->b_block_ms;
    else {
        if (st->b_block_ms > (uint32_t)(s_median_ms * DOUBLE_FEED_RATIO)) st->double_suspect = true;
        s_median_ms = (s_median_ms * 7 + st->b_block_ms) / 8;
    }
    return FEED_OK;
}

void feeder_reverse_pulse() {
    motors_sleep(false);
    motor_run(M_FEED, -cal.feed_pwm); motor_run(M_TRANS, -cal.trans_pwm);
    delay(T_REVERSE_MS);
    feeder_stop(); delay(60);
}
