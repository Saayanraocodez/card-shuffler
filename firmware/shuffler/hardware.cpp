// hardware.cpp — see hardware.h
#include "hardware.h"
#include "platform.h"
#include "rng.h"
#include <math.h>
#include <string.h>

// ============================================================ stepper
static long s_pos = 0; static bool s_enabled = false;
void stepper_init() { pinMode(PIN_STEP, OUTPUT); pinMode(PIN_DIR, OUTPUT); pinMode(PIN_STEP_EN, OUTPUT);
    digitalWrite(PIN_STEP, LOW); digitalWrite(PIN_DIR, LOW); digitalWrite(PIN_STEP_EN, HIGH);
#if DEALER
    pinMode(PIN_TT_STEP, OUTPUT); digitalWrite(PIN_TT_STEP, LOW);
#endif
}
void stepper_enable(bool on) { digitalWrite(PIN_STEP_EN, on ? LOW : HIGH); if (on && !s_enabled) delay(5); s_enabled = on; }
long stepper_pos() { return s_pos; }
void stepper_set_pos(long steps) { s_pos = steps; }

// Trapezoidal step train on one STEP pin (DIR already set). Returns the steps made (unsigned).
static long step_train(int pin, float spd, long n, float vmax_dps, float vmin_dps, float acc_dps2, bool (*stop_if)()) {
    long made = 0;
    for (long i = 0; i < n; i++) {
        if (stop_if && stop_if()) break;
        float s_deg = (float)i / spd, r_deg = (float)(n - i) / spd;
        float v = vmax_dps;
        float va = sqrtf(2.0f * acc_dps2 * s_deg + vmin_dps * vmin_dps);
        float vd = sqrtf(2.0f * acc_dps2 * r_deg + vmin_dps * vmin_dps);
        if (va < v) { v = va; }
        if (vd < v) { v = vd; }
        if (v < vmin_dps) { v = vmin_dps; }
        uint32_t period_us = (uint32_t)(1e6f / (v * spd));
        digitalWrite(pin, HIGH); delayMicroseconds(STEP_PULSE_US); digitalWrite(pin, LOW);
        if (period_us > STEP_PULSE_US) delayMicroseconds(period_us - STEP_PULSE_US);
        made++;
    }
    return made;
}
long stepper_move(long steps, float vmax_dps, float acc_dps2, bool (*stop_if)()) {
    if (steps == 0) return 0;
    stepper_enable(true);
    bool fwd = steps > 0;
    digitalWrite(PIN_DIR, fwd ? HIGH : LOW); delayMicroseconds(10);
    long made = step_train(PIN_STEP, STEPS_PER_REV / 360.0f, labs(steps), vmax_dps, WHEEL_VMIN_DPS, acc_dps2, stop_if);
    s_pos += fwd ? made : -made;
    return fwd ? made : -made;
}
#if DEALER
static long s_tt_pos = 0;
long tt_stepper_pos() { return s_tt_pos; }
void tt_stepper_set_pos(long steps) { s_tt_pos = steps; }
long tt_stepper_move(long steps, float vmax_dps, float acc_dps2, bool (*stop_if)()) {
    if (steps == 0) return 0;
    stepper_enable(true);
    bool fwd = steps > 0;
    digitalWrite(PIN_DIR, (fwd ^ (TT_DIR_INVERT != 0)) ? HIGH : LOW); delayMicroseconds(10);
    long made = step_train(PIN_TT_STEP, TT_STEPS_PER_REV / 360.0f, labs(steps), vmax_dps, TT_VMIN_DPS, acc_dps2, stop_if);
    s_tt_pos += fwd ? made : -made;
    return fwd ? made : -made;
}
#endif

// ============================================================ shutter servo
static ShutterState s_shutter = SHUTTER_CLOSED;
static uint32_t us_to_duty(uint16_t us) { return (uint32_t)((uint64_t)us * 65535ULL / 20000ULL); }
void shutter_init() { pwm_attach(PIN_SERVO, SERVO_FREQ_HZ, 16); shutter_power(false); }
void shutter_set_us(uint16_t us) { pwm_write(PIN_SERVO, us_to_duty(constrain(us, SERVO_US_MIN, SERVO_US_MAX))); }
void shutter_set(ShutterState s, bool wait) { shutter_set_us(s == SHUTTER_OPEN ? cal.shutter_open_us : cal.shutter_closed_us); if (wait) delay(SHUTTER_SETTLE_MS); s_shutter = s; }
void shutter_power(bool on) { if (on) shutter_set(s_shutter, false); else pwm_write(PIN_SERVO, 0); }
ShutterState shutter_state() { return s_shutter; }

// ============================================================ DC motors
static const int IN1[3] = { PIN_FEED_IN1, PIN_NIPE_IN1, PIN_NIPX_IN1 };
static const int IN2[3] = { PIN_FEED_IN2, PIN_NIPE_IN2, PIN_NIPX_IN2 };
void motors_init() { pinMode(PIN_DRV_SLEEP, OUTPUT); digitalWrite(PIN_DRV_SLEEP, LOW);
    for (int m = 0; m < 3; m++) { pwm_attach(IN1[m], MOTOR_PWM_HZ, 8); pwm_attach(IN2[m], MOTOR_PWM_HZ, 8); pwm_write(IN1[m], 0); pwm_write(IN2[m], 0); } }
void motor_run(Motor m, int pwm) { pwm = constrain(pwm, -255, 255);
    if (pwm >= 0) { pwm_write(IN2[m], 0); pwm_write(IN1[m], pwm); } else { pwm_write(IN1[m], 0); pwm_write(IN2[m], -pwm); } }
void motors_sleep(bool sleep) { digitalWrite(PIN_DRV_SLEEP, sleep ? LOW : HIGH); if (!sleep) delay(2); }

// ============================================================ sensors
static int s_sig[N_BEAMS] = {0,0,0,0};
static const int BEAM_PIN[N_BEAMS] = { PIN_BEAM_B, PIN_BEAM_E, PIN_BEAM_S, PIN_BEAM_X };
void sensors_init() { pinMode(PIN_IR_EN, OUTPUT); digitalWrite(PIN_IR_EN, LOW);
    pinMode(PIN_INDEX, INPUT);          // external 10 k pull-down; the index LED is always on
    pinMode(PIN_BUTTON, INPUT_PULLUP);
#if !DEALER
    pinMode(PIN_DECK, INPUT_PULLUP);
#endif
    adc_setup(); }
void beams_poll() {
    int on[N_BEAMS] = {0,0,0,0}, off[N_BEAMS] = {0,0,0,0};
    for (int s = 0; s < BEAM_SAMPLES; s++) {
        digitalWrite(PIN_IR_EN, HIGH); delayMicroseconds(BEAM_SETTLE_US);
        for (int b = 0; b < N_BEAMS; b++) on[b] += analogRead(BEAM_PIN[b]);
        digitalWrite(PIN_IR_EN, LOW); delayMicroseconds(BEAM_SETTLE_US);
        for (int b = 0; b < N_BEAMS; b++) off[b] += analogRead(BEAM_PIN[b]);
    }
    for (int b = 0; b < N_BEAMS; b++) { s_sig[b] = (on[b] - off[b]) / BEAM_SAMPLES; jitter_add((uint32_t)(on[b] ^ (off[b] << 12) ^ micros())); }
}
int  beam_signal(Beam b) { return s_sig[b]; }
bool beam_blocked(Beam b) { return s_sig[b] < (int)cal.beam_thresh[b]; }
bool beam_blocked_now(Beam b) { beams_poll(); return beam_blocked(b); }
bool index_active() { return digitalRead(PIN_INDEX) == LOW; }
bool button_down() { return digitalRead(PIN_BUTTON) == LOW; }
bool aval_collect(int n, AvalStats* st, uint16_t* out) {
    static uint8_t seen[4096 / 8]; memset(seen, 0, sizeof seen);
    uint16_t lo = 4095, hi = 0, distinct = 0;
    for (int i = 0; i < n; i++) {
        uint16_t v = (uint16_t)analogRead(PIN_AVAL) & 0x0FFF;
        jitter_add(((uint32_t)v << 16) ^ micros());
        if (!(seen[v >> 3] & (1 << (v & 7)))) { seen[v >> 3] |= 1 << (v & 7); distinct++; }
        if (v < lo) lo = v;
        if (v > hi) hi = v;
        if (out) out[i] = v;
    }
    st->min = lo; st->max = hi; st->distinct = distinct;
    return n > 0 && hi - lo >= AVAL_MIN_SPAN && distinct >= AVAL_MIN_DISTINCT;
}
float battery_volts() { uint32_t mv = 0; for (int i = 0; i < 8; i++) mv += analogReadMilliVolts(PIN_VBAT); return (mv / 8) * 0.001f * VBAT_DIVIDER * cal.vbat_gain; }

// ============================================================ indicators
static UiColor s_col = C_OFF; static int s_blink = 0; static bool s_phase = true; static uint32_t s_last = 0;
static void rgb(UiColor c, uint8_t& r, uint8_t& g, uint8_t& b) {
    switch (c) { case C_IDLE: r=0; g=40; b=0; break; case C_BUSY: r=0; g=0; b=60; break; case C_UNLOAD: r=0; g=40; b=60; break;
                 case C_DONE: r=0; g=80; b=10; break; case C_WARN: r=80; g=40; b=0; break; case C_ERROR: r=90; g=0; b=0; break;
                 case C_CAL: r=50; g=0; b=60; break; case C_LOWBAT: r=90; g=20; b=0; break; default: r=g=b=0; }
}
static void show(UiColor c) { uint8_t r,g,b; rgb(c, r, g, b); pixel_write(PIN_PIXEL, r, g, b); }
void ui_init() { pinMode(PIN_BUZZER, OUTPUT); pwm_attach(PIN_BUZZER, 2000, 8); tone_stop(PIN_BUZZER); show(C_OFF); }
void ui_color(UiColor c) { s_col = c; s_blink = 0; show(c); }
void ui_blink(UiColor c, int period_ms) { s_col = c; s_blink = period_ms; s_phase = true; show(c); s_last = millis(); }
void ui_tick() { if (s_blink <= 0) return; if (millis() - s_last >= (uint32_t)s_blink / 2) { s_last = millis(); s_phase = !s_phase; show(s_phase ? s_col : C_OFF); } }
void ui_beep(int n, int ms, int freq) { for (int i = 0; i < n; i++) { tone_start(PIN_BUZZER, freq); delay(ms); tone_stop(PIN_BUZZER); if (i + 1 < n) delay(ms); } }
