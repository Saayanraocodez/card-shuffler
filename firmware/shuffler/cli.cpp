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
void shuffle_print_last();   // shuffler.ino

static char line[96]; static int len = 0;
static bool s_fixed_pending = false; static uint32_t s_fixed_seed = 0;

bool cli_fixed_seed_pending(uint32_t* seed) { bool p = s_fixed_pending; s_fixed_pending = false; *seed = s_fixed_seed; return p; }
void cli_init() {}

static void help() {
    Serial.println(
        "commands:\n"
        "  status | last | cal show | cal save | cal default\n"
        "  home | z <mm> | jog <mm> | blades in|out|off | feed | probe | rev\n"
        "  beams            print beam signals for 5 s (LED-on minus LED-off)\n"
        "  bat              battery voltage\n"
        "  cal beams        auto-set beam thresholds (well empty, hopper empty, nothing in the slot)\n"
        "  cal home         auto-calibrate Z from beam S with an EMPTY well (platform finds the beam)\n"
        "  cal knife <mm>   set blade-plane offset (+ raises the commanded boundary)\n"
        "  cal knifetest <k> put gap k of the current stack at the blades and extend them (inspect)\n"
        "  cal servo L|R ret|ext <us>   set a servo endpoint (900..2100) and move to it\n"
        "  cal pwm feed|trans <0-255>\n"
        "  cal cards <52|54>\n"
        "  cal vbat <measured volts>   set the battery gain from a multimeter reading\n"
        "  test rng <n>     dump n gap sequences (52 gaps each) from one hardware-seeded DRBG\n"
        "  test raw <n>     dump n raw hardware-random bytes (hex)\n"
        "  test seed <n>    print n keys derived from fresh hardware seeds\n"
        "  test fixed <seed> next button press runs a fixed-seed shuffle and prints the plan first\n"
        "  test ref         print the reference test vector (must match rng_reference.py)");
}

static void cmd_beams() {
    uint32_t t0 = millis();
    while (millis() - t0 < 5000) { beams_poll(); Serial.printf("B=%4d W=%4d S=%4d   blocked: %c%c%c\n", beam_signal(BEAM_B), beam_signal(BEAM_W), beam_signal(BEAM_S),
        beam_blocked(BEAM_B) ? 'B' : '-', beam_blocked(BEAM_W) ? 'W' : '-', beam_blocked(BEAM_S) ? 'S' : '-'); delay(200); }
}
static void cal_beams() {
    long acc[3] = {0,0,0};
    for (int i = 0; i < 20; i++) { beams_poll(); for (int b = 0; b < 3; b++) acc[b] += beam_signal((Beam)b); delay(20); }
    for (int b = 0; b < 3; b++) { int clear = acc[b] / 20; cal.beam_thresh[b] = (uint16_t)max(60, clear / 2);
        Serial.printf("beam %c clear=%d threshold=%u %s\n", "BWS"[b], clear, cal.beam_thresh[b], clear < 150 ? "(WEAK: check LED/PT alignment)" : ""); }
}
static void cal_home_auto() {
    Serial.println("# well must be EMPTY. homing...");
    if (!elevator_home()) { Serial.println("homing failed"); return; }
    float z = elevator_find_beam_s(Z_MAX);
    if (isnan(z)) { Serial.println("beam S never blocked: check the sensor"); return; }
    // the platform top is now at the beam: redefine z_home so that this position reads Z_BEAM_S_NOMINAL
    float steps_here = stepper_pos() / STEPS_PER_MM;
    cal.z_home = Z_BEAM_S_NOMINAL - steps_here;
    Serial.printf("beam S found %.3f mm above home → z_home=%.3f (saved)\n", steps_here, cal.z_home);
    cal_save(cal);
    elevator_move_to(Z_WELL_CHECK); elevator_idle();
}
static void cmd_test_rng(int n) {
    Drbg d; uint8_t hw[64], jit[32]; hw_random_bytes(hw, 64); jitter_snapshot(jit);
    if (!drbg_seed(&d, hw, jit, cal.boot_counter, cal.shuffle_counter + 1)) { Serial.println("health test failed"); return; }
    uint8_t g[MAX_CARDS];
    for (int s = 0; s < n; s++) { gaps_draw(&d, 52, g); for (int i = 0; i < 52; i++) { Serial.print(g[i]); Serial.print(i < 51 ? ',' : '\n'); } }
    Serial.printf("# words=%lu rejections=%lu\n", (unsigned long)d.words_drawn, (unsigned long)d.rejections);
}
static void cmd_test_raw(int n) { uint8_t b[64]; while (n > 0) { int k = min(n, 64); hw_random_bytes(b, k); for (int i = 0; i < k; i++) Serial.printf("%02x", b[i]); Serial.println(); n -= k; } }
static void cmd_test_seed(int n) { for (int s = 0; s < n; s++) { Drbg d; uint8_t hw[64], jit[32]; hw_random_bytes(hw, 64); jitter_snapshot(jit);
    drbg_seed(&d, hw, jit, cal.boot_counter, cal.shuffle_counter + 1 + s); for (int i = 0; i < 32; i++) Serial.printf("%02x", d.key[i]); Serial.println(); } }
static void cmd_test_ref() {
    uint8_t key[32]; sha256((const uint8_t*)"card-shuffler-test-key", 22, key);
    Drbg d; drbg_seed_key(&d, key, 1); uint8_t g[MAX_CARDS]; gaps_draw(&d, 52, g);
    Serial.print("ref gaps:"); for (int i = 0; i < 52; i++) Serial.printf(" %u", g[i]); Serial.println();
    Serial.println("expect : 0 0 0 0 4 0 1 7 0 5 6 5 9 8 0 7 13 13 5 0 19 8 18 5 1 25 25 24 15 29 12 24 17 6 23 31 3 35 8 28 29 22 15 36 1 38 12 32 29 30 21 42");
}
static void knifetest(int k) {
    // place gap k of the stack currently in the well at the blades and extend them for inspection
    blades_power(true); blades_set(BLADES_RETRACTED);
    if (!elevator_home()) { Serial.println("homing failed"); return; }
    float zp = elevator_find_beam_s(Z_BEAM_S_NOMINAL + 1.0f);
    if (isnan(zp)) { Serial.println("no stack found (beam S)"); return; }
    float h = Z_BEAM_S_NOMINAL - zp;
    int n = cal.expected_cards; float t = h / n;
    Serial.printf("stack height %.2f mm, assuming %d cards → t=%.4f\n", h, n, t);
    float z1 = gap_platform_z(knife_z(), k, t);
    elevator_move_to(z1); blades_set(BLADES_EXTENDED); elevator_move_to(z1 - GAP_DROP);
    Serial.println("gap open at the blades: inspect through the well/slot. `blades out` then `home` when done.");
    elevator_idle();
}

static void exec(char* s) {
    char* a[6] = {0}; int na = 0; for (char* t = strtok(s, " "); t && na < 6; t = strtok(NULL, " ")) a[na++] = t;
    if (na == 0) return;
    if (!strcmp(a[0], "help")) help();
    else if (!strcmp(a[0], "status")) { Serial.printf("z=%.3f blades=%s bat=%.2fV endstop=%d button=%d\n", elevator_z(), blades_state() == BLADES_EXTENDED ? "in" : "out", battery_volts(), endstop_hit(), button_down()); }
    else if (!strcmp(a[0], "last")) shuffle_print_last();
    else if (!strcmp(a[0], "bat")) Serial.printf("%.3f V (raw pin %lu mV)\n", battery_volts(), (unsigned long)analogReadMilliVolts(PIN_VBAT));
    else if (!strcmp(a[0], "home")) Serial.println(elevator_home() ? "homed" : "homing FAILED");
    else if (!strcmp(a[0], "z") && na > 1) { elevator_move_to(atof(a[1])); Serial.printf("z=%.3f\n", elevator_z()); }
    else if (!strcmp(a[0], "jog") && na > 1) { elevator_move_to(elevator_z() + atof(a[1])); Serial.printf("z=%.3f\n", elevator_z()); }
    else if (!strcmp(a[0], "blades") && na > 1) { if (!strcmp(a[1], "in")) { blades_power(true); blades_set(BLADES_EXTENDED); } else if (!strcmp(a[1], "out")) { blades_power(true); blades_set(BLADES_RETRACTED); } else blades_power(false); }
    else if (!strcmp(a[0], "feed")) { FeedStats st; FeedResult r = feeder_feed_one(&st); Serial.printf("%s  B-block=%lu ms%s\n", feed_result_name(r), (unsigned long)st.b_block_ms, st.double_suspect ? " DOUBLE?" : ""); motors_sleep(true); }
    else if (!strcmp(a[0], "probe")) { Serial.println(feeder_probe_hopper() ? "card present" : "hopper empty"); motors_sleep(true); }
    else if (!strcmp(a[0], "rev")) { feeder_reverse_pulse(); motors_sleep(true); }
    else if (!strcmp(a[0], "beams")) cmd_beams();
    else if (!strcmp(a[0], "cal") && na > 1) {
        if (!strcmp(a[1], "show")) cal_print(cal);
        else if (!strcmp(a[1], "save")) { cal_save(cal); Serial.println("saved"); }
        else if (!strcmp(a[1], "default")) { uint32_t b = cal.boot_counter; uint64_t sc = cal.shuffle_counter; cal_defaults(cal); cal.boot_counter = b; cal.shuffle_counter = sc; cal_save(cal); Serial.println("defaults restored"); }
        else if (!strcmp(a[1], "beams")) { cal_beams(); cal_save(cal); }
        else if (!strcmp(a[1], "home")) cal_home_auto();
        else if (!strcmp(a[1], "knife") && na > 2) { cal.knife_offset = atof(a[2]); cal_save(cal); Serial.printf("knife_offset=%.3f\n", cal.knife_offset); }
        else if (!strcmp(a[1], "knifetest") && na > 2) knifetest(atoi(a[2]));
        else if (!strcmp(a[1], "servo") && na > 4) {
            uint16_t us = (uint16_t)constrain(atoi(a[4]), 900, 2100); bool L = a[2][0] == 'L' || a[2][0] == 'l'; bool ret = !strcmp(a[3], "ret");
            if (L) { if (ret) cal.servo_l_retract = us; else cal.servo_l_extend = us; } else { if (ret) cal.servo_r_retract = us; else cal.servo_r_extend = us; }
            blades_power(true); blades_set(ret ? BLADES_RETRACTED : BLADES_EXTENDED); cal_save(cal); Serial.println("set");
        }
        else if (!strcmp(a[1], "pwm") && na > 3) { uint8_t v = (uint8_t)constrain(atoi(a[3]), 0, 255); if (!strcmp(a[2], "feed")) cal.feed_pwm = v; else cal.trans_pwm = v; cal_save(cal); Serial.println("set"); }
        else if (!strcmp(a[1], "cards") && na > 2) { cal.expected_cards = (uint8_t)constrain(atoi(a[2]), 10, MAX_CARDS - 1); cal_save(cal); Serial.println("set"); }
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

void cli_poll() {
    while (Serial.available()) {
        char c = Serial.read(); jitter_add(micros());
        if (c == '\n' || c == '\r') { line[len] = 0; if (len) exec(line); len = 0; }
        else if (len < (int)sizeof(line) - 1) line[len++] = c;
    }
}
