// shuffler.ino — main state machine for the card shuffler (ESP32 DevKitC).
//
// One button: press = start shuffle / acknowledge; hold 3 s = lower platform & home (jam access);
// hold 8 s at power-on = enter calibration mode (also available over serial: type `help`).
//
// Files: config.h (pins/constants), hardware.* (actuators/sensors), motion.* (elevator/feeder),
// rng.* + sha256/chacha20 (randomness), shuffle_core.* (algorithm), storage.* (NVS), cli.cpp (serial).
#include <Arduino.h>
#include "config.h"
#include "platform.h"
#include "hardware.h"
#include "motion.h"
#include "rng.h"
#include "sha256.h"
#include "shuffle_core.h"
#include "storage.h"

Calibration cal;

enum State { ST_IDLE, ST_SHUFFLING, ST_DONE, ST_ERROR, ST_CAL };
static State state = ST_IDLE;
static int   error_code = 0;
static const char* error_text = "";
static uint32_t last_move_ms = 0;

// error codes (LED blinks red, buzzer beeps the code)
enum { E_NONE = 0, E_HOME = 1, E_NO_DECK = 2, E_JAM = 3, E_WELL_NOT_EMPTY = 4, E_RNG = 5, E_LOWBAT = 6, E_TOO_MANY = 7, E_STACK_MEASURE = 8 };

struct ShuffleLog {           // last shuffle, for the serial `last` command and the physical validation test
    uint8_t  n;
    uint8_t  gaps[MAX_CARDS];
    uint8_t  order[MAX_CARDS];
    uint8_t  retries[MAX_CARDS];
    uint8_t  double_suspects;
    float    t_card;
    uint32_t ms;
    uint8_t  key[32];
    uint64_t counter;
    bool     fixed_seed;
} last;

// provided by cli.cpp
void cli_init(); void cli_poll();
bool cli_fixed_seed_pending(uint32_t* seed);

static void set_error(int code, const char* text) {
    error_code = code; error_text = text; state = ST_ERROR;
    feeder_stop(); motors_sleep(true); blades_power(false); elevator_idle();
    ui_blink(C_ERROR, 500); ui_beep(3, 120, 1800);
    cal.total_jams += (code == E_JAM); cal_save(cal);
    Serial.printf("ERROR %d: %s\n", code, text);
}

// ---------------------------------------------------------------- seeding
static bool seed_drbg(Drbg* d, bool fixed, uint32_t fixed_seed) {
    cal.shuffle_counter++;
    if (fixed) {
        char buf[40]; int n = snprintf(buf, sizeof buf, "fixed-seed:%lu", (unsigned long)fixed_seed);
        uint8_t key[32]; sha256((const uint8_t*)buf, n, key);
        drbg_seed_key(d, key, cal.shuffle_counter);
        memcpy(last.key, key, 32); last.fixed_seed = true; last.counter = cal.shuffle_counter;
        return true;
    }
    uint8_t hw[64], jit[32];
    jitter_add(micros()); jitter_add((uint32_t)(battery_volts() * 100000.0f)); jitter_add(esp_random());
    hw_random_bytes(hw, sizeof hw);
    jitter_snapshot(jit);
    bool ok = drbg_seed(d, hw, jit, cal.boot_counter, cal.shuffle_counter);
    memcpy(last.key, d->key, 32); last.fixed_seed = false; last.counter = cal.shuffle_counter;
    return ok;
}

// ---------------------------------------------------------------- one shuffle
static bool feed_with_retries(uint8_t i, FeedResult* out) {
    FeedStats st; FeedResult r = FEED_OK;
    for (int attempt = 0; attempt <= FEED_RETRIES; attempt++) {
        r = feeder_feed_one(&st);
        if (r == FEED_OK) { if (st.double_suspect) last.double_suspects++; last.retries[i] = attempt; *out = r; return true; }
        if (r == FEED_NO_CARD) { *out = r; return false; }           // hopper empty (or pickup failure): caller decides
        Serial.printf("# card %u: %s, retry %d\n", i, feed_result_name(r), attempt + 1);
        feeder_reverse_pulse();
    }
    *out = r; return false;
}

static void run_shuffle(bool fixed, uint32_t fixed_seed) {
    uint32_t t0 = millis();
    state = ST_SHUFFLING; ui_color(C_BUSY);
    memset(&last, 0, sizeof last);

    float v = battery_volts();
    if (v < VBAT_REFUSE) { set_error(E_LOWBAT, "battery too low to start"); return; }

    Drbg d;
    if (!seed_drbg(&d, fixed, fixed_seed)) { set_error(E_RNG, "hardware RNG health test failed"); return; }
    if (fixed) {   // predicted-vs-actual test mode: print the plan before moving anything
        uint8_t g[MAX_CARDS], o[MAX_CARDS]; Drbg dd = d;
        uint8_t n = cal.expected_cards; gaps_draw(&dd, n, g); predict_order(n, g, o);
        Serial.printf("PLAN cards=%u seed=%lu\nPLAN gaps:", n, (unsigned long)fixed_seed);
        for (int i = 0; i < n; i++) Serial.printf(" %u", g[i]);
        Serial.print("\nPLAN order(bottom..top, input index):");
        for (int i = 0; i < n; i++) Serial.printf(" %u", o[i]);
        Serial.println();
    }

    // mechanics: home, verify the well is empty, verify a deck is loaded
    blades_power(true); blades_set(BLADES_RETRACTED);
    if (!elevator_home()) { set_error(E_HOME, "elevator endstop not found"); return; }
    elevator_move_to(Z_WELL_CHECK);
    if (beam_blocked_now(BEAM_S)) { set_error(E_WELL_NOT_EMPTY, "remove cards from the well first"); return; }
    if (!feeder_probe_hopper()) { set_error(E_NO_DECK, "no deck in the hopper"); return; }

    float t_card = cal.t_card;
    if (t_card < T_CARD_MIN || t_card > T_CARD_MAX) t_card = T_CARD_DEFAULT;
    uint8_t n = 0;
    bool lowbat_abort = false;

    for (uint8_t i = 0; i < MAX_CARDS; i++) {
        // periodic stack-height measurement: removes accumulated thickness error and lost steps
        if (i >= 8 && (i % STACK_MEASURE_EVERY) == 0) {
            float zp = elevator_find_beam_s(Z_BEAM_S_NOMINAL + 1.0f);
            if (isnan(zp)) { set_error(E_STACK_MEASURE, "stack-top beam not found"); return; }
            float h = Z_BEAM_S_NOMINAL - zp;
            float t = h / (float)i;
            if (t > T_CARD_MIN && t < T_CARD_MAX) t_card = t;
        }
        uint8_t j = (uint8_t)drbg_uniform(&d, (uint32_t)i + 1);
        last.gaps[i] = j;
        float z1 = gap_platform_z(knife_z(), j, t_card);
        elevator_move_to(z1);
        blades_set(BLADES_EXTENDED);
        elevator_move_to(z1 - GAP_DROP);
        FeedResult r;
        if (!feed_with_retries(i, &r)) {
            if (r == FEED_NO_CARD) {                     // hopper empty: close up and finish
                elevator_move_to(z1 - GAP_DROP + GAP_CLOSE); blades_set(BLADES_RETRACTED);
                break;
            }
            set_error(E_JAM, feed_result_name(r)); return;   // blades stay in, gap stays open for access
        }
        elevator_move_to(z1 - GAP_DROP + GAP_CLOSE);
        blades_set(BLADES_RETRACTED);
        n = i + 1;
        if (battery_volts() < VBAT_ABORT) { lowbat_abort = true; break; }
        if (n >= MAX_CARDS) { set_error(E_TOO_MANY, "more cards than MAX_CARDS"); return; }
        ui_tick();
    }
    blades_power(false); motors_sleep(true);

    // present the deck
    float h = n * t_card;
    float zp = Z_PRESENT_TOP - h; if (zp > Z_MAX) zp = Z_MAX;
    elevator_move_to(zp);
    elevator_idle();

    last.n = n; last.t_card = t_card; last.ms = millis() - t0;
    predict_order(n, last.gaps, last.order);
    cal.t_card = t_card; cal.total_cards += n; cal.last_shuffle_ms = last.ms; cal_save(cal);

    unsigned retries = 0; for (int i = 0; i < n; i++) retries += last.retries[i];
    Serial.printf("DONE cards=%u time=%lu ms t_card=%.4f retries=%u double_suspects=%u\n", n, (unsigned long)last.ms,
                  t_card, retries, last.double_suspects);
    if (lowbat_abort) { set_error(E_LOWBAT, "battery low: shuffle stopped early, deck presented"); return; }
    state = ST_DONE;
    if (n != cal.expected_cards || last.double_suspects) {
        ui_blink(C_WARN, 400); ui_beep(2, 200, 1500);
        Serial.printf("WARN expected %u cards, counted %u (double-feed suspects %u): consider re-running\n", cal.expected_cards, n, last.double_suspects);
    } else { ui_color(C_DONE); ui_beep(1, 250, 2600); }
}

void shuffle_print_last() {
    if (last.n == 0) { Serial.println("no shuffle yet"); return; }
    Serial.printf("LAST cards=%u time=%lu ms t_card=%.4f seed=%s counter=%llu key=", last.n, (unsigned long)last.ms, last.t_card,
                  last.fixed_seed ? "fixed" : "hardware", (unsigned long long)last.counter);
    for (int i = 0; i < 32; i++) Serial.printf("%02x", last.key[i]);
    Serial.print("\nLAST gaps:"); for (int i = 0; i < last.n; i++) Serial.printf(" %u", last.gaps[i]);
    Serial.print("\nLAST order(bottom..top, input index):"); for (int i = 0; i < last.n; i++) Serial.printf(" %u", last.order[i]);
    Serial.print("\nLAST retries:"); for (int i = 0; i < last.n; i++) Serial.printf(" %u", last.retries[i]);
    Serial.println();
}

// ---------------------------------------------------------------- jam access
static void lower_for_access() {
    ui_color(C_CAL);
    feeder_stop(); motors_sleep(true);
    blades_power(true); blades_set(BLADES_RETRACTED); blades_power(false);
    if (elevator_home()) elevator_move_to(Z_MIN + 1.0f);
    elevator_idle();
    state = ST_IDLE; ui_color(C_IDLE);
}

// ---------------------------------------------------------------- button
static void handle_button() {
    static uint32_t down_since = 0; static bool was_down = false;
    bool d = button_down();
    if (d && !was_down) { down_since = millis(); jitter_add(micros()); }
    if (!d && was_down) {
        uint32_t held = millis() - down_since;
        if (held > 30 && held < BUTTON_LONG_MS) {
            if (state == ST_IDLE || state == ST_DONE || state == ST_ERROR) {
                if (state == ST_DONE) { if (elevator_home()) elevator_move_to(Z_WELL_CHECK); elevator_idle(); state = ST_IDLE; ui_color(C_IDLE); }
                else if (state == ST_ERROR) { state = ST_IDLE; ui_color(C_IDLE); }
                else { uint32_t s; bool fixed = cli_fixed_seed_pending(&s); run_shuffle(fixed, s); }
            }
        } else if (held >= BUTTON_LONG_MS) lower_for_access();
    }
    was_down = d;
}

// ---------------------------------------------------------------- Arduino
void setup() {
    Serial.begin(115200);
    delay(200);
    cal_load(cal); cal.boot_counter++; cal_save(cal);
    stepper_init(); blades_init(); motors_init(); sensors_init(); ui_init(); cli_init();
    ui_color(C_IDLE);
    Serial.println("\n# card shuffler firmware — type `help` for the serial console");
    cal_print(cal);
    jitter_add(esp_random()); jitter_add(micros());
    float v = battery_volts();
    Serial.printf("# battery %.2f V\n", v);
    if (v < VBAT_WARN) ui_blink(C_LOWBAT, 1000);
}

void loop() {
    cli_poll();
    handle_button();
    ui_tick();
    static uint32_t last_bat = 0;
    if (millis() - last_bat > 5000) { last_bat = millis(); if (state == ST_IDLE) { float v = battery_volts(); if (v < VBAT_WARN) ui_blink(C_LOWBAT, 1000); else ui_color(C_IDLE); } }
#if KEEPALIVE_MS > 0
    static uint32_t last_ka = 0;
    if (millis() - last_ka > KEEPALIVE_MS) { last_ka = millis(); stepper_enable(true); delay(150); stepper_enable(false); }
#endif
    delay(5);
}
