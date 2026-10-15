// hardware.h — stepper (wheel), shutter servo, three DC motors, beam sensors, index, battery, button, indicators.
#pragma once
#include <Arduino.h>
#include "config.h"

extern Calibration cal;

// ---- stepper (wheel): blocking trapezoidal moves in microsteps; position tracked in steps ----
void stepper_init();
void stepper_enable(bool on);
long stepper_pos();
void stepper_set_pos(long steps);
// Moves `steps` (signed) with a trapezoid (deg/s units converted internally). If stop_if is non-null it is
// polled every step and the move ends early when it returns true. Returns the signed steps made.
long stepper_move(long steps, float vmax_dps, float acc_dps2, bool (*stop_if)() = nullptr);
#if DEALER
// Turntable stepper: own STEP pin, DIR and EN shared with the wheel driver (only one moves at a time).
// Positive = clockwise seen from above. Same profile and stop_if semantics as stepper_move.
long tt_stepper_move(long steps, float vmax_dps, float acc_dps2, bool (*stop_if)() = nullptr);
long tt_stepper_pos();
void tt_stepper_set_pos(long steps);
#endif

// ---- shutter servo ----
enum ShutterState { SHUTTER_CLOSED, SHUTTER_OPEN };
void shutter_init();
void shutter_set(ShutterState s, bool wait = true);
void shutter_set_us(uint16_t us);
void shutter_power(bool on);
ShutterState shutter_state();

// ---- DC motors ----
enum Motor { M_FEED = 0, M_NIPE = 1, M_NIPX = 2 };
void motors_init();
void motor_run(Motor m, int pwm);          // -255..255, 0 = coast
void motors_sleep(bool sleep);
#if DEALER
// The throw roller's DRV8833 channel is driven by the entry-nip signals: it spins whenever the entry nip does.
inline void throw_run(int pwm) { motor_run(M_NIPE, pwm); }
#endif

// ---- sensors ----
enum Beam { BEAM_B = 0, BEAM_E = 1, BEAM_S = 2, BEAM_X = 3 };
#if DEALER
// Beam L (card at the launcher) is in series with beam B: either blocked reads as BEAM_B blocked. While dealing the
// feeder is idle and B is clear, so BEAM_B means L; before loading, both must be clear (chute and gate empty).
#define BEAM_L BEAM_B
#endif
#define N_BEAMS 4
void sensors_init();
void beams_poll();
int  beam_signal(Beam b);
bool beam_blocked(Beam b);
bool beam_blocked_now(Beam b);
bool index_active();                       // disc tab blocks the index beam
bool button_down();
float battery_volts();

// ---- avalanche noise board (USE_AVALANCHE) ----
struct AvalStats { uint16_t min, max, distinct; };
// Reads n ADC samples from PIN_AVAL, mixes each into the jitter pool and returns true if they pass the
// health test (span and distinct-value count). `out`, if given, receives the raw samples.
bool aval_collect(int n, AvalStats* st, uint16_t* out = nullptr);

// ---- indicators ----
enum UiColor { C_OFF, C_IDLE, C_BUSY, C_UNLOAD, C_DONE, C_WARN, C_ERROR, C_CAL, C_LOWBAT };
void ui_init();
void ui_color(UiColor c);
void ui_blink(UiColor c, int period_ms);
void ui_tick();
void ui_beep(int n, int ms = 80, int freq = 2400);
