// config.h — pins, mechanical constants, defaults for the WHEEL shuffler.  Edit here, not in the modules.
#pragma once
#include <stdint.h>

// ---------------- pins (ESP32 DevKitC, see docs/04-electronics.md §4.3) ----------------
#define PIN_STEP       25
#define PIN_DIR        26
#define PIN_STEP_EN    27      // TMC2208 EN, active low
#define PIN_FEED_IN1   16      // DRV8833 #1 AIN1 (PWM)   feed roller
#define PIN_FEED_IN2   17      // DRV8833 #1 AIN2
#define PIN_NIPE_IN1   4       // DRV8833 #1 BIN1 (PWM)   entry nip roller
#define PIN_NIPE_IN2   5       // DRV8833 #1 BIN2
#define PIN_NIPX_IN1   15      // DRV8833 #2 AIN1 (PWM)   exit nip roller
#define PIN_NIPX_IN2   2       // DRV8833 #2 AIN2
#define PIN_DRV_SLEEP  23      // both DRV8833 nSLEEP
#define PIN_SERVO      18      // shutter servo
#define PIN_IR_EN      13      // NPN base: switches the three IR emitters
#define PIN_PIXEL      14      // WS2812B
#define PIN_BUZZER     33
#define PIN_BEAM_B     34      // ADC1  gate beam (feeder)
#define PIN_BEAM_E     35      // ADC1  entry/occupancy beam (card in the slot at the entry)
#define PIN_BEAM_X     36      // ADC1  exit beam (card leaving through the chute)
#define PIN_VBAT       39      // ADC1  battery divider
#define PIN_INDEX      32      // slotted optical module, digital (LOW when the disc tab is in the slot)
#define PIN_BUTTON     21      // momentary to GND
#define PIN_DECK       22      // optional TCRT5000 DO (unused by default)

// ---------------- wheel geometry (see cad/params.scad) ----------------
#define N_SLOTS_HW        54
#define PITCH_DEG         (360.0f / N_SLOTS_HW)   // 6.6667
#define ENTRY_FIN_DEG     46.667f   // fin k's CCW face here → card enters slot k
#define EXIT_FIN_DEG      208.333f  // fin k+1's CW face here → card leaves slot k
#define INDEX_TAB_DEG     90.0f     // angle of the disc tab when the wheel reference (fin 0) is at 0°
#define STEPS_PER_REV     3200.0f   // 200 full steps × 1/16 microstepping, direct drive
#define WHEEL_VMAX_DPS    360.0f    // deg/s
#define WHEEL_VMIN_DPS    20.0f
#define WHEEL_ACC_DPS2    900.0f    // deg/s²
#define WHEEL_V_HOME_DPS  90.0f
#define WHEEL_V_SCAN_DPS  120.0f
#define STEP_PULSE_US     3

// ---------------- shutter servo ----------------
#define SERVO_FREQ_HZ     50
#define SERVO_US_MIN      500
#define SERVO_US_MAX      2500
#define SHUTTER_CLOSED_US_DEFAULT 1150
#define SHUTTER_OPEN_US_DEFAULT   1850
#define SHUTTER_SETTLE_MS 250

// ---------------- feeder / nips ----------------
#define FEED_PWM_DEFAULT   160     // 0..255 on the 5 V rail
#define NIPE_PWM_DEFAULT   210
#define NIPX_PWM_DEFAULT   230
#define MOTOR_PWM_HZ       20000
#define T_PICK_MS          800     // beam B must block (card picked) within this
#define T_E_BLOCK_MS       900     // beam E must block (card reached the slot mouth)
#define T_B_CLEAR_MS       900     // beam B must clear (trailing edge past the gate)
#define T_SEAT_MS          250     // extra nip run after B clears (card slides to the hub)
#define T_X_BLOCK_MS       700     // unload: beam X must block after the nip starts
#define T_X_CLEAR_MS       700     // unload: beam X must clear (card fully out)
#define T_REVERSE_MS       150
#define FEED_RETRIES       2
#define PROBE_MS           600
#define DOUBLE_FEED_RATIO  1.30f

// ---------------- sensors ----------------
#define BEAM_THRESH_DEFAULT 400
#define BEAM_SETTLE_US      250
#define BEAM_SAMPLES        2
#define VBAT_DIVIDER        4.03f
#define VBAT_WARN           9.9f
#define VBAT_REFUSE         9.6f
#define VBAT_ABORT          9.3f

// ---------------- behaviour ----------------
#define EXPECTED_CARDS_DEFAULT 52
#define BUTTON_LONG_MS     3000
#define IDLE_DISABLE_MS    2000
#define KEEPALIVE_MS       0
#define USE_BOOTLOADER_RANDOM 1
#define SCAN_AT_START      1       // rotate once and verify the wheel is empty before loading
#define VERIFY_AFTER_INSERT 1      // read beam E after each insertion; search ±1 slot if not found

// ---------------- calibration record (NVS) ----------------
struct Calibration {
    uint32_t magic;            // 0xCA11B0A8
    float    home_offset_deg;  // wheel angle (fin 0 reference) when the index trips; nominal INDEX_TAB_DEG
    float    entry_trim_deg;   // added to ENTRY_FIN_DEG
    float    exit_trim_deg;    // added to EXIT_FIN_DEG
    uint16_t shutter_closed_us, shutter_open_us;
    uint16_t beam_thresh[3];   // B, E, X
    uint8_t  feed_pwm, nipe_pwm, nipx_pwm;
    uint8_t  expected_cards;
    float    vbat_gain;
    uint32_t boot_counter;
    uint64_t shuffle_counter;
    uint32_t total_cards, total_jams, total_corrections;
    uint32_t last_shuffle_ms;
};
#define CAL_MAGIC 0xCA11B0A8u
