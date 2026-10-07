// shuffler.ino — main state machine for the WHEEL card shuffler (ESP32 DevKitC).
//
// One button: press = start / acknowledge; hold 3 s = unload all (home, scan, empty every occupied slot).
// Serial console at 115200: type `help`.
//
// Shuffle = LOAD phase (each card from the hopper into a uniformly random empty slot of the 54-slot
// wheel) + UNLOAD phase (slots emptied in slot order into the chute).  See docs/02-mathematics.md.
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

enum State { ST_IDLE, ST_LOADING, ST_UNLOADING, ST_DONE, ST_ERROR };
static State state = ST_IDLE;
static int error_code = 0;
enum { E_NONE = 0, E_HOME = 1, E_NO_DECK = 2, E_JAM_FEED = 3, E_WHEEL_NOT_EMPTY = 4, E_RNG = 5, E_LOWBAT = 6, E_TOO_MANY = 7, E_LOST_CARD = 8, E_JAM_EJECT = 9, E_COUNT = 10, E_MAP = 11 };

struct ShuffleLog {
    uint8_t  n;                        // cards loaded
    uint8_t  slot_of_card[MAX_CARDS];  // realised slot (after any correction)
    uint8_t  intended[MAX_CARDS];      // slot drawn by the RNG
    uint8_t  order[MAX_CARDS];         // predicted output order (bottom..top, input index)
    uint8_t  retries[MAX_CARDS];
    uint8_t  corrections, double_suspects, eject_double_suspects, ejected;
    uint32_t ms_load, ms_unload;
    uint8_t  key[32]; uint64_t counter; bool fixed_seed;
} last;

void cli_init(); void cli_poll(); bool cli_fixed_seed_pending(uint32_t* seed);

static void set_error(int code, const char* text) {
    error_code = code; state = ST_ERROR;
    feeder_stop(); eject_stop(); motors_sleep(true); wheel_idle();
    ui_blink(C_ERROR, 500); ui_beep(3, 120, 1800);
    if (code == E_JAM_FEED || code == E_JAM_EJECT) cal.total_jams++;
    cal_save(cal);
    Serial.printf("ERROR %d: %s\n", code, text);
}

// ---------------------------------------------------------------- seeding
static bool seed_drbg(Drbg* d, bool fixed, uint32_t fixed_seed) {
    cal.shuffle_counter++;
    if (fixed) { char buf[40]; int n = snprintf(buf, sizeof buf, "fixed-seed:%lu", (unsigned long)fixed_seed);
        uint8_t key[32]; sha256((const uint8_t*)buf, n, key); drbg_seed_key(d, key, cal.shuffle_counter);
        memcpy(last.key, key, 32); last.fixed_seed = true; last.counter = cal.shuffle_counter; return true; }
    uint8_t hw[64], jit[32];
    jitter_add(micros()); jitter_add((uint32_t)(battery_volts() * 100000.0f)); jitter_add(esp_random());
    hw_random_bytes(hw, sizeof hw); jitter_snapshot(jit);
    bool ok = drbg_seed(d, hw, jit, cal.boot_counter, cal.shuffle_counter);
    memcpy(last.key, d->key, 32); last.fixed_seed = false; last.counter = cal.shuffle_counter;
    return ok;
}

// ---------------------------------------------------------------- one shuffle
static bool feed_with_retries(uint8_t i, FeedResult* out) {
    FeedStats st; FeedResult r = FEED_OK;
    for (int attempt = 0; attempt <= FEED_RETRIES; attempt++) {
        last.retries[i] = attempt;
        r = feeder_feed_one(&st);
        if (r == FEED_OK) { if (st.double_suspect) last.double_suspects++; *out = r; return true; }
        if (r == FEED_NO_CARD || r == FEED_NOT_SEATED) { *out = r; return false; }   // hopper empty / card in a neighbour slot
        // jam or card stuck in the mouth: pull it back with a reverse pulse and try the same slot again.
        // (Retrying into the same, still empty slot does not change the output distribution.)
        Serial.printf("# card %u: %s, retry %d\n", i, feed_result_name(r), attempt + 1);
        feeder_reverse_pulse();
    }
    *out = r; return false;
}

// After a feed, the card should be in slot s (beam E blocked at the entry).  If not, look one slot each
// side; return the slot where it was found or -1.
static int locate_card(uint8_t s, bool occupied[N_SLOTS_HW]) {
    if (beam_blocked_now(BEAM_E)) return s;
    int cands[2] = { (s + 1) % N_SLOTS_HW, (s + N_SLOTS_HW - 1) % N_SLOTS_HW };
    for (int c = 0; c < 2; c++) {
        if (occupied[cands[c]]) continue;                    // was already occupied: a card there is not the new one
        if (!wheel_fin_to_entry((uint8_t)cands[c])) return -1;
        delay(30);
        if (beam_blocked_now(BEAM_E)) return cands[c];
    }
    return -1;
}

// Open only with the first occupied slot at the exit: opened elsewhere, cards over the window slide out and the next move drags them.
static bool unload_open(const bool occupied[N_SLOTS_HW]) {
    for (uint8_t s = 0; s < N_SLOTS_HW; s++)
        if (occupied[s]) { if (!wheel_fin_to_exit(s)) return false; break; }
    shutter_set(SHUTTER_OPEN);
    return true;
}

static void run_shuffle(bool fixed, uint32_t fixed_seed) {
    uint32_t t0 = millis();
    state = ST_LOADING; ui_color(C_BUSY);
    memset(&last, 0, sizeof last);
    if (battery_volts() < VBAT_REFUSE) { set_error(E_LOWBAT, "battery too low to start"); return; }

    Drbg d;
    if (!seed_drbg(&d, fixed, fixed_seed)) { set_error(E_RNG, "hardware RNG health test failed"); return; }
    if (fixed) {   // predicted-vs-actual test: print the plan before moving
        uint8_t sl[MAX_CARDS], o[MAX_CARDS]; Drbg dd = d; uint8_t n = cal.expected_cards;
        slots_assign(&dd, n, N_SLOTS_HW, sl); predict_order_from_slots(n, sl, o);
        Serial.printf("PLAN cards=%u seed=%lu\nPLAN slots:", n, (unsigned long)fixed_seed);
        for (int i = 0; i < n; i++) Serial.printf(" %u", sl[i]);
        Serial.print("\nPLAN order(bottom..top, input index):"); for (int i = 0; i < n; i++) Serial.printf(" %u", o[i]); Serial.println();
    }

    shutter_power(true); shutter_set(SHUTTER_CLOSED);
    if (!wheel_path_clear()) { set_error(E_JAM_FEED, "a card bridges the wheel and the feeder or chute: remove it"); return; }
    if (!wheel_home()) { set_error(E_HOME, "wheel index not found"); return; }
    bool occupied[N_SLOTS_HW] = {false};
#if SCAN_AT_START
    int found_cards = wheel_scan(occupied);
    if (found_cards < 0) { set_error(E_JAM_FEED, "a card bridges the wheel and the feeder or chute"); return; }
    if (found_cards != 0) { set_error(E_WHEEL_NOT_EMPTY, "cards left in the wheel: hold the button to unload"); return; }
#endif
    if (!feeder_probe_hopper()) { set_error(E_NO_DECK, "no deck in the hopper"); return; }

    // ---- LOAD: card i -> uniform among the slots the firmware knows to be empty (exact Fisher–Yates)
    uint8_t empty[N_SLOTS_HW]; uint8_t n_empty = N_SLOTS_HW; for (uint8_t s = 0; s < N_SLOTS_HW; s++) empty[s] = s;
    uint8_t n = 0; bool lowbat = false;
    for (uint8_t i = 0; i < N_SLOTS_HW; i++) {
        uint8_t idx = (uint8_t)drbg_uniform(&d, n_empty);
        uint8_t s = empty[idx];
        last.intended[i] = s;
        if (!wheel_fin_to_entry(s)) { set_error(E_JAM_FEED, "a card bridges the wheel and a fixed part"); return; }
        // The target must be empty. If not, the map is wrong (lost steps or an unnoticed misplaced card):
        // re-home once (fixes lost steps); if it is still occupied, stop rather than guess.
        if (beam_blocked_now(BEAM_E)) {
            Serial.printf("# slot %u should be empty but beam E is blocked: re-homing\n", s);
            if (!wheel_home() || !wheel_fin_to_entry(s)) { set_error(E_HOME, "re-homing failed"); return; }
            if (beam_blocked_now(BEAM_E)) { set_error(E_MAP, "unexpected card in a slot the map says is empty"); return; }
        }
        FeedResult r;
        if (!feed_with_retries(i, &r)) {
            if (r == FEED_NO_CARD) break;                                      // hopper empty
            if (r == FEED_NOT_SEATED) {
#if VERIFY_AFTER_INSERT
                int found = locate_card(s, occupied);
                if (found < 0) { set_error(E_LOST_CARD, "card left the feeder but is not in the slot or its neighbours"); return; }
                if (found != s) { last.corrections++; cal.total_corrections++; Serial.printf("# card %u landed in slot %d (intended %u): map corrected\n", i, found, s); }
                s = (uint8_t)found;
#else
                set_error(E_LOST_CARD, "card not seen in the slot"); return;
#endif
            } else { set_error(E_JAM_FEED, feed_result_name(r)); return; }   // wheel stays put; the interlock blocks moves while beam S is blocked
        }
        // remove the realised slot from the empty list (swap-remove; order irrelevant for uniformity)
        occupied[s] = true; last.slot_of_card[i] = s; n = i + 1;
        uint8_t k = 0; while (k < n_empty && empty[k] != s) k++;
        if (k < n_empty) { empty[k] = empty[n_empty - 1]; n_empty--; }
        if (n_empty == 0) break;
        if (battery_volts() < VBAT_ABORT) { lowbat = true; break; }
        ui_tick();
    }
    feeder_stop();
    last.n = n; last.ms_load = millis() - t0;
    if (n == 0) { set_error(E_NO_DECK, "no card could be fed"); return; }

    // ---- UNLOAD: slots in increasing order into the chute
    state = ST_UNLOADING; ui_color(C_UNLOAD);
    uint32_t t1 = millis();
    if (!unload_open(occupied)) { set_error(E_JAM_EJECT, "a card is still in the exit nip"); return; }
    uint8_t ejected = 0;
    for (uint8_t s = 0; s < N_SLOTS_HW; s++) {
        if (!occupied[s]) continue;
        if (!wheel_fin_to_exit(s)) { set_error(E_JAM_EJECT, "a card is still in the exit nip"); return; }
        EjectResult er = EJECT_OK; EjectStats es;
        for (int attempt = 0; attempt < 3; attempt++) {
            er = eject_one(&es);
            if (er == EJECT_OK) break;
            Serial.printf("# slot %u: eject %s, retry %d\n", s, er == EJECT_NO_CARD ? "no card" : "jam", attempt + 1);
            if (er == EJECT_JAM) { motor_run(M_NIPX, -cal.nipx_pwm); delay(T_REVERSE_MS); eject_stop(); delay(60); }
        }
        if (er == EJECT_JAM) { set_error(E_JAM_EJECT, "card stuck in the exit nip"); return; }
        // No card reached beam X. It may be hanging half out of the window, where turning the wheel would
        // tear it, so stop and let the user look rather than rotate on.
        if (er == EJECT_NO_CARD) { set_error(E_JAM_EJECT, "a card did not come out at the exit: check the exit window"); return; }
        ejected++; if (es.double_suspect) last.eject_double_suspects++;
        ui_tick();
    }
    shutter_set(SHUTTER_CLOSED); shutter_power(false); motors_sleep(true); wheel_idle();
    last.ms_unload = millis() - t1; last.ejected = ejected;
    predict_order_from_slots(n, last.slot_of_card, last.order);
    cal.total_cards += n; cal.last_shuffle_ms = millis() - t0; cal_save(cal);

    unsigned retries = 0; for (int i = 0; i < n; i++) retries += last.retries[i];
    Serial.printf("DONE loaded=%u ejected=%u load=%lu ms unload=%lu ms retries=%u corrections=%u double_suspects in/out=%u/%u\n",
                  n, ejected, (unsigned long)last.ms_load, (unsigned long)last.ms_unload, retries, last.corrections, last.double_suspects, last.eject_double_suspects);
    if (lowbat) { set_error(E_LOWBAT, "battery low: stopped early; loaded cards are in the chute, the rest are still in the hopper"); return; }
    state = ST_DONE;
    if (ejected != n || n != cal.expected_cards || last.double_suspects || last.eject_double_suspects) {
        ui_blink(C_WARN, 400); ui_beep(2, 200, 1500);
        Serial.printf("WARN expected %u, loaded %u, ejected %u (double-card suspects in/out %u/%u): consider re-running\n",
                      cal.expected_cards, n, ejected, last.double_suspects, last.eject_double_suspects);
    } else { ui_color(C_DONE); ui_beep(1, 250, 2600); }
}

void shuffle_print_last() {
    if (last.n == 0) { Serial.println("no shuffle yet"); return; }
    Serial.printf("LAST loaded=%u ejected=%u load=%lu ms unload=%lu ms seed=%s counter=%llu key=", last.n, last.ejected, (unsigned long)last.ms_load,
                  (unsigned long)last.ms_unload, last.fixed_seed ? "fixed" : "hardware", (unsigned long long)last.counter);
    for (int i = 0; i < 32; i++) Serial.printf("%02x", last.key[i]);
    Serial.print("\nLAST intended slots:"); for (int i = 0; i < last.n; i++) Serial.printf(" %u", last.intended[i]);
    Serial.print("\nLAST realised slots:"); for (int i = 0; i < last.n; i++) Serial.printf(" %u", last.slot_of_card[i]);
    Serial.print("\nLAST order(bottom..top, input index):"); for (int i = 0; i < last.n; i++) Serial.printf(" %u", last.order[i]);
    Serial.print("\nLAST retries:"); for (int i = 0; i < last.n; i++) Serial.printf(" %u", last.retries[i]);
    Serial.println();
}

// ---------------------------------------------------------------- recovery: unload whatever is in the wheel
static void unload_all() {
    ui_color(C_CAL); feeder_stop();
    shutter_power(true); shutter_set(SHUTTER_CLOSED);   // may still be open after ERROR 9 or `shutter open`
    if (!wheel_path_clear()) { set_error(E_JAM_FEED, "a card bridges the wheel and a fixed part: remove it by hand first"); return; }
    if (!wheel_home()) { set_error(E_HOME, "wheel index not found"); return; }
    bool occ[N_SLOTS_HW]; int n = wheel_scan(occ);
    if (n < 0) { set_error(E_JAM_FEED, "a card bridges the wheel and a fixed part: remove it by hand"); return; }
    if (!unload_open(occ)) { set_error(E_JAM_EJECT, "unload stopped: check the exit window and nip"); return; }
    for (uint8_t s = 0; s < N_SLOTS_HW; s++) if (occ[s]) {
        EjectStats es;
        if (!wheel_fin_to_exit(s) || eject_one(&es) != EJECT_OK) { set_error(E_JAM_EJECT, "unload stopped: check the exit window and nip"); return; }
    }
    shutter_set(SHUTTER_CLOSED); shutter_power(false); motors_sleep(true); wheel_idle();
    Serial.printf("# unloaded %d cards\n", n);
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
            if (state == ST_DONE || state == ST_ERROR) { state = ST_IDLE; ui_color(C_IDLE); }
            else if (state == ST_IDLE) { uint32_t s; bool fixed = cli_fixed_seed_pending(&s); run_shuffle(fixed, s); }
        } else if (held >= BUTTON_LONG_MS) unload_all();
    }
    was_down = d;
}

// ---------------------------------------------------------------- Arduino
void setup() {
    Serial.begin(115200); delay(200);
    cal_load(cal); cal.boot_counter++; cal_save(cal);
    stepper_init(); shutter_init(); motors_init(); sensors_init(); ui_init(); cli_init();
    ui_color(C_IDLE);
    Serial.println("\n# wheel card shuffler firmware — type `help` for the serial console");
    cal_print(cal);
    jitter_add(esp_random()); jitter_add(micros());
    float v = battery_volts(); Serial.printf("# battery %.2f V\n", v);
    if (v < VBAT_WARN) ui_blink(C_LOWBAT, 1000);
}
void loop() {
    cli_poll(); handle_button(); ui_tick();
    static uint32_t last_bat = 0;
    if (millis() - last_bat > 5000) { last_bat = millis(); if (state == ST_IDLE) { float v = battery_volts(); if (v < VBAT_WARN) ui_blink(C_LOWBAT, 1000); else ui_color(C_IDLE); } }
#if KEEPALIVE_MS > 0
    static uint32_t last_ka = 0; if (millis() - last_ka > KEEPALIVE_MS) { last_ka = millis(); stepper_enable(true); delay(150); stepper_enable(false); }
#endif
    delay(5);
}
