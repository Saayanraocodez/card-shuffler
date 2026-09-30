// hardware.cpp — see hardware.h
#include "hardware.h"
#include "platform.h"
#include "rng.h"
#include <math.h>

// ============================================================ stepper
static long s_pos = 0;
static bool s_enabled = false;

void stepper_init() {
    pinMode(PIN_STEP, OUTPUT); pinMode(PIN_DIR, OUTPUT); pinMode(PIN_STEP_EN, OUTPUT);
    digitalWrite(PIN_STEP, LOW); digitalWrite(PIN_DIR, LOW); digitalWrite(PIN_STEP_EN, HIGH);   // disabled
}
void stepper_enable(bool on) { digitalWrite(PIN_STEP_EN, on ? LOW : HIGH); if (on && !s_enabled) delay(5); s_enabled = on; }
long stepper_pos() { return s_pos; }
void stepper_set_pos(long steps) { s_pos = steps; }

long stepper_move(long steps, float vmax, float acc, bool (*stop_if)()) {
    if (steps == 0) return 0;
    stepper_enable(true);
    bool up = steps > 0;
    digitalWrite(PIN_DIR, up ? HIGH : LOW);
    delayMicroseconds(10);
    long n = labs(steps);
    const float spm = STEPS_PER_MM;
    long made = 0;
    for (long i = 0; i < n; i++) {
        if (stop_if && stop_if()) break;
        float s_mm  = (float)i / spm, r_mm = (float)(n - i) / spm;
        float v = vmax;
        float va = sqrtf(2.0f * acc * s_mm + ELEV_VMIN * ELEV_VMIN);
        float vd = sqrtf(2.0f * acc * r_mm + ELEV_VMIN * ELEV_VMIN);
        if (va < v) v = va; if (vd < v) v = vd; if (v < ELEV_VMIN) v = ELEV_VMIN;
        uint32_t period_us = (uint32_t)(1e6f / (v * spm));
        digitalWrite(PIN_STEP, HIGH); delayMicroseconds(STEP_PULSE_US); digitalWrite(PIN_STEP, LOW);
        if (period_us > STEP_PULSE_US) delayMicroseconds(period_us - STEP_PULSE_US);
        s_pos += up ? 1 : -1; made++;
    }
    return up ? made : -made;
}

// ============================================================ servos
static BladeState s_blades = BLADES_RETRACTED;
static bool s_servo_on = false;
static uint32_t us_to_duty(uint16_t us) { return (uint32_t)((uint64_t)us * 65535ULL / 20000ULL); }
void blades_init() { pwm_attach(PIN_SERVO_L, SERVO_FREQ_HZ, 16); pwm_attach(PIN_SERVO_R, SERVO_FREQ_HZ, 16); blades_power(false); }
void blades_set_us(uint16_t l, uint16_t r) {
    l = constrain(l, SERVO_US_MIN, SERVO_US_MAX); r = constrain(r, SERVO_US_MIN, SERVO_US_MAX);
    pwm_write(PIN_SERVO_L, us_to_duty(l)); pwm_write(PIN_SERVO_R, us_to_duty(r)); s_servo_on = true;
}
void blades_set(BladeState s, bool wait) {
    if (s == BLADES_EXTENDED) blades_set_us(cal.servo_l_extend, cal.servo_r_extend);
    else                      blades_set_us(cal.servo_l_retract, cal.servo_r_retract);
    if (wait) delay(SERVO_SETTLE_MS);
    s_blades = s;
}
BladeState blades_state() { return s_blades; }
void blades_power(bool on) {
    if (on) { blades_set(s_blades, false); }
    else { pwm_write(PIN_SERVO_L, 0); pwm_write(PIN_SERVO_R, 0); s_servo_on = false; }   // no pulses = limp; return springs hold them out
}

// ============================================================ DC motors
static const int IN1[2] = { PIN_FEED_IN1, PIN_TRANS_IN1 };
static const int IN2[2] = { PIN_FEED_IN2, PIN_TRANS_IN2 };
void motors_init() {
    pinMode(PIN_DRV_SLEEP, OUTPUT); digitalWrite(PIN_DRV_SLEEP, LOW);
    for (int m = 0; m < 2; m++) { pwm_attach(IN1[m], MOTOR_PWM_HZ, 8); pwm_attach(IN2[m], MOTOR_PWM_HZ, 8); pwm_write(IN1[m], 0); pwm_write(IN2[m], 0); }
}
void motor_run(Motor m, int pwm) {
    pwm = constrain(pwm, -255, 255);
    if (pwm >= 0) { pwm_write(IN2[m], 0); pwm_write(IN1[m], pwm); }
    else          { pwm_write(IN1[m], 0); pwm_write(IN2[m], -pwm); }
}
void motor_brake(Motor m) { pwm_write(IN1[m], 255); pwm_write(IN2[m], 255); }
void motors_sleep(bool sleep) { digitalWrite(PIN_DRV_SLEEP, sleep ? LOW : HIGH); if (!sleep) delay(2); }

// ============================================================ sensors
static int s_sig[3] = {0, 0, 0};
static const int BEAM_PIN[3] = { PIN_BEAM_B, PIN_BEAM_W, PIN_BEAM_S };
void sensors_init() {
    pinMode(PIN_IR_EN, OUTPUT); digitalWrite(PIN_IR_EN, LOW);
    pinMode(PIN_ENDSTOP, INPUT_PULLUP); pinMode(PIN_BUTTON, INPUT_PULLUP); pinMode(PIN_DECK, INPUT_PULLUP);
    adc_setup();
}
void beams_poll() {
    int on[3] = {0,0,0}, off[3] = {0,0,0};
    for (int s = 0; s < BEAM_SAMPLES; s++) {
        digitalWrite(PIN_IR_EN, HIGH); delayMicroseconds(BEAM_SETTLE_US);
        for (int b = 0; b < 3; b++) on[b] += analogRead(BEAM_PIN[b]);
        digitalWrite(PIN_IR_EN, LOW); delayMicroseconds(BEAM_SETTLE_US);
        for (int b = 0; b < 3; b++) off[b] += analogRead(BEAM_PIN[b]);
    }
    for (int b = 0; b < 3; b++) { s_sig[b] = (on[b] - off[b]) / BEAM_SAMPLES; jitter_add((uint32_t)(on[b] ^ (off[b] << 12) ^ micros())); }
}
int  beam_signal(Beam b) { return s_sig[b]; }
bool beam_blocked(Beam b) { return s_sig[b] < (int)cal.beam_thresh[b]; }
bool beam_blocked_now(Beam b) { beams_poll(); return beam_blocked(b); }
// NC contact wired to GND: pressed (lever pushed) opens the contact → pin reads HIGH via the pull-up.
bool endstop_hit() { return digitalRead(PIN_ENDSTOP) == HIGH; }
bool button_down() { return digitalRead(PIN_BUTTON) == LOW; }
float battery_volts() {
    uint32_t mv = 0; for (int i = 0; i < 8; i++) mv += analogReadMilliVolts(PIN_VBAT);
    return (mv / 8) * 0.001f * VBAT_DIVIDER * cal.vbat_gain;
}

// ============================================================ indicators
static UiColor s_col = C_OFF; static int s_blink = 0; static bool s_phase = true; static uint32_t s_last = 0;
static void rgb(UiColor c, uint8_t& r, uint8_t& g, uint8_t& b) {
    switch (c) { case C_IDLE: r=0; g=40; b=0; break;  case C_BUSY: r=0; g=0; b=60; break;  case C_DONE: r=0; g=80; b=10; break;
                 case C_WARN: r=80; g=40; b=0; break; case C_ERROR: r=90; g=0; b=0; break; case C_CAL: r=50; g=0; b=60; break;
                 case C_LOWBAT: r=90; g=20; b=0; break; default: r=g=b=0; }
}
static void show(UiColor c) { uint8_t r,g,b; rgb(c, r, g, b); pixel_write(PIN_PIXEL, r, g, b); }
void ui_init() { pinMode(PIN_BUZZER, OUTPUT); pwm_attach(PIN_BUZZER, 2000, 8); tone_stop(PIN_BUZZER); show(C_OFF); }
void ui_color(UiColor c) { s_col = c; s_blink = 0; show(c); }
void ui_blink(UiColor c, int period_ms) { s_col = c; s_blink = period_ms; s_phase = true; show(c); s_last = millis(); }
void ui_tick() {
    if (s_blink <= 0) return;
    if (millis() - s_last >= (uint32_t)s_blink / 2) { s_last = millis(); s_phase = !s_phase; show(s_phase ? s_col : C_OFF); }
}
void ui_beep(int n, int ms, int freq) {
    for (int i = 0; i < n; i++) { tone_start(PIN_BUZZER, freq); delay(ms); tone_stop(PIN_BUZZER); if (i + 1 < n) delay(ms); }
}
