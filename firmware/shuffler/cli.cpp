// cli.cpp — serial console for bring-up, calibration, tests and validation dumps (115200 baud).
#include <Arduino.h>
#include "config.h"
#include "platform.h"
#include "hardware.h"
#include "motion.h"
#include "rng.h"
#include "sha256.h"
#include "shuffle_core.h"
#include "storage.h"

extern Calibration cal;
void shuffle_print_last();
static char line[96]; static int len = 0;
static bool s_fixed_pending = false; static uint32_t s_fixed_seed = 0;
bool cli_fixed_seed_pending(uint32_t* seed) { bool p = s_fixed_pending; s_fixed_pending = false; *seed = s_fixed_seed; return p; }
void cli_init() {}

static void help() {
    Serial.println(
        "commands:\n"
        "  status | last | cal show | cal save | cal default | bat\n"
        "  home | goto <deg> | jog <deg> | entry <slot> | exit <slot> | scan\n"
        "  shutter open|close|off | feed | probe | rev | eject   (wheel moves are refused while beam S or X is blocked)\n"
        "  beams            print beam signals for 5 s (LED-on minus LED-off)\n"
        "  cal beams        auto-set thresholds (nothing in the beams)\n"
        "  cal home <deg>   wheel angle at the index (adjust until `entry 0` puts fin 0 on the feeder plate plane;\n"
        "                   the default entry trim then lowers it 0.3 deg so cards meet the fin lead-in)\n"
        "  cal entry <deg>  trim for the entry plane (+ = CCW)   cal exit <deg>  trim for the exit plane\n"
        "  cal shutter closed|open <us>   set and move\n"
        "  cal pwm feed|nipe|nipx <0-255> | cal cards <52|54> | cal vbat <measured volts>\n"
        "  test rng <n>     dump n slot assignments (52 slots each) from one hardware-seeded DRBG\n"
        "  test raw <n>     dump n raw hardware-random bytes (hex)\n"
        "  test seed <n>    print n keys from fresh hardware seeds\n"
        "  test fixed <seed> next button press: fixed-seed shuffle, plan printed first\n"
        "  test ref         reference vector (must match rng_reference.py --selftest)");
}
static void cmd_beams() { uint32_t t0 = millis(); while (millis() - t0 < 5000) { beams_poll();
    Serial.printf("B=%4d E=%4d S=%4d X=%4d  blocked: %c%c%c%c  index=%s\n", beam_signal(BEAM_B), beam_signal(BEAM_E), beam_signal(BEAM_S), beam_signal(BEAM_X),
        beam_blocked(BEAM_B) ? 'B' : '-', beam_blocked(BEAM_E) ? 'E' : '-', beam_blocked(BEAM_S) ? 'S' : '-', beam_blocked(BEAM_X) ? 'X' : '-',
        index_active() ? "TAB" : "clear"); delay(200); } }
static void cal_beams() { long acc[N_BEAMS] = {0,0,0,0};
    for (int i = 0; i < 20; i++) { beams_poll(); for (int b = 0; b < N_BEAMS; b++) acc[b] += beam_signal((Beam)b); delay(20); }
    for (int b = 0; b < N_BEAMS; b++) { int clear = acc[b] / 20; cal.beam_thresh[b] = (uint16_t)max(60, clear / 2);
        Serial.printf("beam %c clear=%d threshold=%u %s\n", "BESX"[b], clear, cal.beam_thresh[b], clear < 150 ? "(WEAK: check LED/PT alignment)" : ""); } }
static void cmd_test_rng(int n) { Drbg d; uint8_t hw[64], jit[32]; hw_random_bytes(hw, 64); jitter_snapshot(jit);
    if (!drbg_seed(&d, hw, jit, cal.boot_counter, cal.shuffle_counter + 1)) { Serial.println("health test failed"); return; }
    uint8_t sl[MAX_CARDS];
    for (int s = 0; s < n; s++) { slots_assign(&d, 52, N_SLOTS_HW, sl); for (int i = 0; i < 52; i++) { Serial.print(sl[i]); Serial.print(i < 51 ? ',' : '\n'); } }
    Serial.printf("# words=%lu rejections=%lu\n", (unsigned long)d.words_drawn, (unsigned long)d.rejections); }
static void cmd_test_raw(int n) { uint8_t b[64]; while (n > 0) { int k = min(n, 64); hw_random_bytes(b, k); for (int i = 0; i < k; i++) Serial.printf("%02x", b[i]); Serial.println(); n -= k; } }
static void cmd_test_seed(int n) { for (int s = 0; s < n; s++) { Drbg d; uint8_t hw[64], jit[32]; hw_random_bytes(hw, 64); jitter_snapshot(jit);
    drbg_seed(&d, hw, jit, cal.boot_counter, cal.shuffle_counter + 1 + s); for (int i = 0; i < 32; i++) Serial.printf("%02x", d.key[i]); Serial.println(); } }
static void cmd_test_ref() { uint8_t key[32]; sha256((const uint8_t*)"card-shuffler-test-key", 22, key);
    Drbg d; drbg_seed_key(&d, key, 1); uint8_t sl[MAX_CARDS]; slots_assign(&d, 52, N_SLOTS_HW, sl);
    Serial.print("ref slots:"); for (int i = 0; i < 52; i++) Serial.printf(" %u", sl[i]); Serial.println();
    Serial.println("expect   : 3 23 40 50 29 13 47 52 22 5 39 43 36 25 20 2 15 53 51 37 11 8 26 16 6 34 1 48 42 4 9 0 49 12 32 17 44 14 45 31 10 38 41 33 27 19 35 46 30 28 21 24"); }

static void exec(char* s) {
    char* a[6] = {0}; int na = 0; for (char* t = strtok(s, " "); t && na < 6; t = strtok(NULL, " ")) a[na++] = t;
    if (na == 0) return;
    if (!strcmp(a[0], "help")) help();
    else if (!strcmp(a[0], "status")) Serial.printf("wheel=%.2f deg shutter=%s bat=%.2fV index=%d button=%d\n", wheel_angle(), shutter_state() == SHUTTER_OPEN ? "open" : "closed", battery_volts(), index_active(), button_down());
    else if (!strcmp(a[0], "last")) shuffle_print_last();
    else if (!strcmp(a[0], "bat")) Serial.printf("%.3f V (pin %lu mV)\n", battery_volts(), (unsigned long)analogReadMilliVolts(PIN_VBAT));
    else if (!strcmp(a[0], "home")) Serial.println(wheel_home() ? "homed" : "index NOT found");
    else if (!strcmp(a[0], "goto") && na > 1) { if (!wheel_goto(atof(a[1]))) return; Serial.printf("wheel=%.2f\n", wheel_angle()); }
    else if (!strcmp(a[0], "jog") && na > 1) { if (!wheel_goto(wheel_angle() + atof(a[1]))) return; Serial.printf("wheel=%.2f\n", wheel_angle()); }
    else if (!strcmp(a[0], "entry") && na > 1) { if (!wheel_fin_to_entry((uint8_t)atoi(a[1]))) return; Serial.printf("slot %d at the entry, wheel=%.2f, beam E %s\n", atoi(a[1]), wheel_angle(), beam_blocked_now(BEAM_E) ? "BLOCKED (card present)" : "clear"); }
    else if (!strcmp(a[0], "exit") && na > 1) { if (!wheel_fin_to_exit((uint8_t)atoi(a[1]))) return; Serial.printf("slot %d at the exit, wheel=%.2f\n", atoi(a[1]), wheel_angle()); }
    else if (!strcmp(a[0], "scan")) { bool occ[N_SLOTS_HW]; int n = wheel_scan(occ); Serial.printf("%d occupied:", n); for (int i = 0; i < N_SLOTS_HW; i++) if (occ[i]) Serial.printf(" %d", i); Serial.println(); }
    else if (!strcmp(a[0], "shutter") && na > 1) { if (!strcmp(a[1], "open")) { shutter_power(true); shutter_set(SHUTTER_OPEN); } else if (!strcmp(a[1], "close")) { shutter_power(true); shutter_set(SHUTTER_CLOSED); } else shutter_power(false); }
    else if (!strcmp(a[0], "feed")) { FeedStats st; FeedResult r = feeder_feed_one(&st); Serial.printf("%s  B-block=%lu ms%s\n", feed_result_name(r), (unsigned long)st.b_block_ms, st.double_suspect ? " DOUBLE?" : ""); motors_sleep(true); }
    else if (!strcmp(a[0], "probe")) { Serial.println(feeder_probe_hopper() ? "card present" : "hopper empty"); motors_sleep(true); }
    else if (!strcmp(a[0], "rev")) { feeder_reverse_pulse(); motors_sleep(true); }
    else if (!strcmp(a[0], "eject")) { EjectStats es; EjectResult r = eject_one(&es); Serial.printf("%s  X-block=%lu ms%s\n", r == EJECT_OK ? "ejected" : r == EJECT_NO_CARD ? "no card" : "jam", (unsigned long)es.x_block_ms, es.double_suspect ? " DOUBLE?" : ""); motors_sleep(true); }
    else if (!strcmp(a[0], "beams")) cmd_beams();
    else if (!strcmp(a[0], "cal") && na > 1) {
        if (!strcmp(a[1], "show")) cal_print(cal);
        else if (!strcmp(a[1], "save")) { cal_save(cal); Serial.println("saved"); }
        else if (!strcmp(a[1], "default")) { uint32_t b = cal.boot_counter; uint64_t sc = cal.shuffle_counter; cal_defaults(cal); cal.boot_counter = b; cal.shuffle_counter = sc; cal_save(cal); Serial.println("defaults restored"); }
        else if (!strcmp(a[1], "beams")) { cal_beams(); cal_save(cal); }
        else if (!strcmp(a[1], "home") && na > 2) { cal.home_offset_deg = atof(a[2]); cal_save(cal); Serial.println("set; run `home`"); }
        else if (!strcmp(a[1], "entry") && na > 2) { cal.entry_trim_deg = atof(a[2]); cal_save(cal); Serial.println("set"); }
        else if (!strcmp(a[1], "exit") && na > 2) { cal.exit_trim_deg = atof(a[2]); cal_save(cal); Serial.println("set"); }
        else if (!strcmp(a[1], "shutter") && na > 3) { uint16_t us = (uint16_t)constrain(atoi(a[3]), 900, 2100); if (!strcmp(a[2], "open")) cal.shutter_open_us = us; else cal.shutter_closed_us = us;
            shutter_power(true); shutter_set(!strcmp(a[2], "open") ? SHUTTER_OPEN : SHUTTER_CLOSED); cal_save(cal); Serial.println("set"); }
        else if (!strcmp(a[1], "pwm") && na > 3) { uint8_t v = (uint8_t)constrain(atoi(a[3]), 0, 255); if (!strcmp(a[2], "feed")) cal.feed_pwm = v; else if (!strcmp(a[2], "nipe")) cal.nipe_pwm = v; else cal.nipx_pwm = v; cal_save(cal); Serial.println("set"); }
        else if (!strcmp(a[1], "cards") && na > 2) { cal.expected_cards = (uint8_t)constrain(atoi(a[2]), 10, N_SLOTS_HW); cal_save(cal); Serial.println("set"); }
        else if (!strcmp(a[1], "vbat") && na > 2) { float meas = atof(a[2]); float est = battery_volts() / cal.vbat_gain; cal.vbat_gain = meas / est; cal_save(cal); Serial.printf("vbat_gain=%.4f\n", cal.vbat_gain); }
        else help();
    }
    else if (!strcmp(a[0], "test") && na > 1) {
        if (!strcmp(a[1], "rng")) cmd_test_rng(na > 2 ? atoi(a[2]) : 10);
        else if (!strcmp(a[1], "raw")) cmd_test_raw(na > 2 ? atoi(a[2]) : 256);
        else if (!strcmp(a[1], "seed")) cmd_test_seed(na > 2 ? atoi(a[2]) : 5);
        else if (!strcmp(a[1], "fixed")) { s_fixed_seed = na > 2 ? strtoul(a[2], NULL, 10) : 1; s_fixed_pending = true; Serial.printf("next button press: fixed-seed shuffle, seed %lu\n", (unsigned long)s_fixed_seed); }
        else if (!strcmp(a[1], "ref")) cmd_test_ref();
        else help();
    }
    else help();
}
void cli_poll() { while (Serial.available()) { char c = Serial.read(); jitter_add(micros());
    if (c == '\n' || c == '\r') { line[len] = 0; if (len) exec(line); len = 0; } else if (len < (int)sizeof(line) - 1) line[len++] = c; } }
