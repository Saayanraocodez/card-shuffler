// storage.cpp
#include <Arduino.h>
#include <Preferences.h>
#include "storage.h"
static Preferences prefs;
void cal_defaults(Calibration& c) {
    memset(&c, 0, sizeof c); c.magic = CAL_MAGIC;
    c.home_offset_deg = INDEX_TAB_DEG; c.entry_trim_deg = 0; c.exit_trim_deg = 0;
    c.shutter_closed_us = SHUTTER_CLOSED_US_DEFAULT; c.shutter_open_us = SHUTTER_OPEN_US_DEFAULT;
    for (int i = 0; i < 3; i++) c.beam_thresh[i] = BEAM_THRESH_DEFAULT;
    c.feed_pwm = FEED_PWM_DEFAULT; c.nipe_pwm = NIPE_PWM_DEFAULT; c.nipx_pwm = NIPX_PWM_DEFAULT;
    c.expected_cards = EXPECTED_CARDS_DEFAULT; c.vbat_gain = 1.0f;
}
void cal_load(Calibration& c) { prefs.begin("shuffler", true); size_t n = prefs.getBytes("cal", &c, sizeof c); prefs.end();
    if (n != sizeof c || c.magic != CAL_MAGIC) { cal_defaults(c); Serial.println("# no calibration stored: defaults loaded"); } }
void cal_save(const Calibration& c) { prefs.begin("shuffler", false); prefs.putBytes("cal", &c, sizeof c); prefs.end(); }
void cal_print(const Calibration& c) {
    Serial.printf("home_offset=%.2f entry_trim=%.2f exit_trim=%.2f deg\n", c.home_offset_deg, c.entry_trim_deg, c.exit_trim_deg);
    Serial.printf("shutter closed/open=%u/%u us  beam thresholds B/E/X=%u/%u/%u  pwm feed/nipE/nipX=%u/%u/%u  cards=%u  vbat_gain=%.3f\n",
                  c.shutter_closed_us, c.shutter_open_us, c.beam_thresh[0], c.beam_thresh[1], c.beam_thresh[2], c.feed_pwm, c.nipe_pwm, c.nipx_pwm, c.expected_cards, c.vbat_gain);
    Serial.printf("boots=%lu shuffles=%llu cards=%lu jams=%lu corrections=%lu last_shuffle=%lu ms\n", (unsigned long)c.boot_counter,
                  (unsigned long long)c.shuffle_counter, (unsigned long)c.total_cards, (unsigned long)c.total_jams, (unsigned long)c.total_corrections, (unsigned long)c.last_shuffle_ms);
}
