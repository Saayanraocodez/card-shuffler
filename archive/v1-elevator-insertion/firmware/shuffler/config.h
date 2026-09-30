// config.h — pins, mechanical constants, defaults.  Edit here, not in the modules.
#pragma once
#include <stdint.h>

// ---------------- pins (ESP32 DevKitC, see docs/04-electronics.md §4.3) ----------------
#define PIN_STEP       25
#define PIN_DIR        26
#define PIN_STEP_EN    27      // TMC2208 EN, active low
#define PIN_FEED_IN1   16      // DRV8833 AIN1 (PWM)
#define PIN_FEED_IN2   17      // DRV8833 AIN2
#define PIN_TRANS_IN1  4       // DRV8833 BIN1 (PWM)
#define PIN_TRANS_IN2  5       // DRV8833 BIN2
#define PIN_DRV_SLEEP  23      // DRV8833 nSLEEP
#define PIN_SERVO_L    18
#define PIN_SERVO_R    19
#define PIN_IR_EN      13      // NPN base: switches all three IR emitters
#define PIN_PIXEL      14      // WS2812B
#define PIN_BUZZER     33
#define PIN_BEAM_B     34      // ADC1_CH6  gate beam
#define PIN_BEAM_W     35      // ADC1_CH7  well-slot beam
#define PIN_BEAM_S     36      // ADC1_CH0  stack-top beam
#define PIN_VBAT       39      // ADC1_CH3  battery divider
#define PIN_ENDSTOP    32      // NC microswitch to GND (LOW = not pressed... see sensors.cpp)
#define PIN_BUTTON     21      // momentary to GND
#define PIN_DECK       22      // optional TCRT5000 DO (unused by default)

// ---------------- elevator geometry (mm, card frame; see cad/params.scad) ----------------
#define STEPS_PER_MM      400.0f   // 200 full steps * 16 microsteps / 8 mm lead (T8x8). T8x2: 1600.
#define Z_KNIFE_NOMINAL   2.5f     // blade mid-plane
#define Z_BEAM_S_NOMINAL  14.0f    // stack-top beam centre (defines the Z origin after calibration)
#define Z_MIN            -25.0f
#define Z_MAX             22.0f
#define Z_PRESENT_TOP     41.0f    // wanted stack-top height when presenting the deck (rim = 38)
#define Z_HOME_DEFAULT   -26.0f    // platform Z when the endstop trips (before calibration)
#define Z_WELL_CHECK      11.5f    // platform Z used to verify the well is empty (beam S must be clear)
#define GAP_DROP          3.0f     // lower stack drop to open the gap
#define GAP_CLOSE         2.2f     // raise after the card is in (blade still in)
#define STACK_MEASURE_EVERY 4      // re-measure stack height every N cards (from card 8 on)
#define T_CARD_DEFAULT    0.30f    // mm, mean card thickness until measured
#define T_CARD_MIN        0.18f
#define T_CARD_MAX        0.50f

#define ELEV_VMAX         30.0f    // mm/s
#define ELEV_VMIN         2.0f     // mm/s (start/stop speed)
#define ELEV_ACC          400.0f   // mm/s^2
#define ELEV_V_HOME       6.0f
#define ELEV_V_MEASURE    4.0f
#define STEP_PULSE_US     3

// ---------------- servos ----------------
#define SERVO_FREQ_HZ     50
#define SERVO_US_MIN      500
#define SERVO_US_MAX      2500
#define SERVO_L_RETRACT_US_DEFAULT 1170   // ≈ -30° from centre (mirror of R)
#define SERVO_L_EXTEND_US_DEFAULT  1830
#define SERVO_R_RETRACT_US_DEFAULT 1830
#define SERVO_R_EXTEND_US_DEFAULT  1170
#define SERVO_SETTLE_MS   180

// ---------------- feeder ----------------
#define FEED_PWM_DEFAULT   160     // 0..255 on a 5 V rail
#define TRANS_PWM_DEFAULT  210
#define MOTOR_PWM_HZ       20000
#define T_PICK_MS          800     // beam B must block (card picked) within this
#define T_W_BLOCK_MS       900     // beam W must block (card reached the well) within this
#define T_B_CLEAR_MS       900     // beam B must clear (trailing edge past the gate)
#define T_W_CLEAR_MS       900     // beam W must clear (card fully inside)
#define T_COAST_MS         80      // transport keeps running after W clears
#define T_REVERSE_MS       150     // jam-clearing reverse pulse
#define FEED_RETRIES       2
#define PROBE_MS           600     // hopper probe: feed motor time to see beam B block
#define DOUBLE_FEED_RATIO  1.30f   // beam-B block time ratio vs running median → suspect

// ---------------- sensors ----------------
#define BEAM_THRESH_DEFAULT 400    // ADC counts (LED on minus LED off); blocked if below
#define BEAM_SETTLE_US      250
#define BEAM_SAMPLES        2
#define VBAT_DIVIDER        4.03f  // (100k + 33k) / 33k
#define VBAT_WARN           9.9f
#define VBAT_REFUSE         9.6f
#define VBAT_ABORT          9.3f

// ---------------- behaviour ----------------
#define EXPECTED_CARDS_DEFAULT 52
#define BUTTON_LONG_MS     3000
#define BUTTON_CAL_MS      8000
#define IDLE_DISABLE_MS    2000    // stepper driver off this long after the last move
#define KEEPALIVE_MS       0       // >0: pulse the stepper coils every N ms (USB power banks)
#define USE_BOOTLOADER_RANDOM 1    // 1: enable the SAR-ADC entropy source while seeding (recommended)

// ---------------- calibration record (stored in NVS) ----------------
struct Calibration {
    uint32_t magic;            // 0xCA11B0A7
    float    z_home;           // platform Z at the endstop (derived from the beam-S auto-calibration)
    float    knife_offset;     // added to Z_KNIFE_NOMINAL
    float    t_card;           // last measured mean card thickness
    uint16_t servo_l_retract, servo_l_extend, servo_r_retract, servo_r_extend;
    uint16_t beam_thresh[3];   // B, W, S
    uint8_t  feed_pwm, trans_pwm;
    uint8_t  expected_cards;
    float    vbat_gain;        // multiplies the divider estimate
    uint32_t boot_counter;
    uint64_t shuffle_counter;
    uint32_t total_cards;      // lifetime statistics
    uint32_t total_jams;
    uint32_t last_shuffle_ms;
};
#define CAL_MAGIC 0xCA11B0A7u
