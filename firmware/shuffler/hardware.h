// hardware.h — stepper, servos, DC motors, beam sensors, battery, endstop, button, indicators.
#pragma once
#include <Arduino.h>
#include "config.h"

extern Calibration cal;

// ---- stepper (blocking trapezoidal moves; position tracked in steps) ----
void stepper_init();
void stepper_enable(bool on);
long stepper_pos();                       // steps from home
void stepper_set_pos(long steps);
// Moves `steps` (signed) with a trapezoidal profile. If `stop_if` is non-null it is polled every step
// and the move ends early when it returns true; the number of steps actually made is returned.
long stepper_move(long steps, float vmax_mm_s, float acc, bool (*stop_if)() = nullptr);

// ---- servos (blade bars) ----
enum BladeState { BLADES_RETRACTED, BLADES_EXTENDED };
void blades_init();
void blades_set(BladeState s, bool wait = true);
void blades_set_us(uint16_t left_us, uint16_t right_us);    // calibration
BladeState blades_state();
void blades_power(bool on);               // detach = go limp (idle)

// ---- DC motors (DRV8833) ----
enum Motor { M_FEED = 0, M_TRANS = 1 };
void motors_init();
void motor_run(Motor m, int pwm);         // -255..255, 0 = coast
void motor_brake(Motor m);
void motors_sleep(bool sleep);

// ---- sensors ----
enum Beam { BEAM_B = 0, BEAM_W = 1, BEAM_S = 2 };
void sensors_init();
void beams_poll();                         // one LED-on / LED-off cycle, updates all three channels
int  beam_signal(Beam b);                  // last (on - off) difference in ADC counts
bool beam_blocked(Beam b);                 // uses cal.beam_thresh
bool beam_blocked_now(Beam b);             // poll then test
bool endstop_hit();
bool button_down();
float battery_volts();

// ---- indicators ----
enum UiColor { C_OFF, C_IDLE, C_BUSY, C_DONE, C_WARN, C_ERROR, C_CAL, C_LOWBAT };
void ui_init();
void ui_color(UiColor c);
void ui_beep(int n, int ms = 80, int freq = 2400);
void ui_tick();                            // call from loop for blink patterns
void ui_blink(UiColor c, int period_ms);   // set a blink pattern (0 = steady)
