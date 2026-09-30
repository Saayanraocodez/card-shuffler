// storage.cpp
#include <Arduino.h>
#include <Preferences.h>
#include "storage.h"

static Preferences prefs;

void cal_defaults(Calibration& c) {
    memset(&c, 0, sizeof c);
    c.magic = CAL_MAGIC;
    c.z_home = Z_HOME_DEFAULT; c.knife_offset = 0.0f; c.t_card = T_CARD_DEFAULT;
    c.servo_l_retract = SERVO_L_RETRACT_US_DEFAULT; c.servo_l_extend = SERVO_L_EXTEND_US_DEFAULT;
    c.servo_r_retract = SERVO_R_RETRACT_US_DEFAULT; c.servo_r_extend = SERVO_R_EXTEND_US_DEFAULT;
    for (int i = 0; i < 3; i++) c.beam_thresh[i] = BEAM_THRESH_DEFAULT;
    c.feed_pwm = FEED_PWM_DEFAULT; c.trans_pwm = TRANS_PWM_DEFAULT;
    c.expected_cards = EXPECTED_CARDS_DEFAULT; c.vbat_gain = 1.0f;
}
void cal_load(Calibration& c) {
    prefs.begin("shuffler", true);
    size_t n = prefs.getBytes("cal", &c, sizeof c);
    prefs.end();
    if (n != sizeof c || c.magic != CAL_MAGIC) { cal_defaults(c); Serial.println("# no calibration stored: defaults loaded"); }
}
void cal_save(const Calibration& c) { prefs.begin("shuffler", false); prefs.putBytes("cal", &c, sizeof c); prefs.end(); }
void cal_print(const Calibration& c) {
    Serial.printf("z_home=%.3f knife_offset=%.3f t_card=%.4f\n", c.z_home, c.knife_offset, c.t_card);
    Serial.printf("servo L ret/ext=%u/%u  R ret/ext=%u/%u us\n", c.servo_l_retract, c.servo_l_extend, c.servo_r_retract, c.servo_r_extend);
    Serial.printf("beam thresholds B/W/S=%u/%u/%u  feed_pwm=%u trans_pwm=%u  cards=%u  vbat_gain=%.3f\n",
                  c.beam_thresh[0], c.beam_thresh[1], c.beam_thresh[2], c.feed_pwm, c.trans_pwm, c.expected_cards, c.vbat_gain);
    Serial.printf("boots=%lu shuffles=%llu cards=%lu jams=%lu last_shuffle=%lu ms\n",
                  (unsigned long)c.boot_counter, (unsigned long long)c.shuffle_counter, (unsigned long)c.total_cards, (unsigned long)c.total_jams, (unsigned long)c.last_shuffle_ms);
}
