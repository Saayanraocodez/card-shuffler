// dealer.cpp — see dealer.h
#include "dealer.h"
#if DEALER
#include <Preferences.h>
#include <math.h>
#include "hardware.h"
#include "motion.h"

void app_yield();                         // shuffler.ino: LED, phone page (only Stop and status are served while busy)

DealConfig dcfg;
static Preferences s_prefs;
static const float TT_SPD = TT_STEPS_PER_REV / 360.0f;   // steps per degree
static bool s_homed = false;
static volatile bool s_abort = false;
static bool s_occ[N_SLOTS_HW];            // cards still in the wheel
static int  s_left = 0;
static HandStage s_stage = HAND_NONE;
static uint32_t s_median_x = 0;           // beam X block time, for the double-card check (as eject_one does)

// ============================================================ settings
void deal_spread_seats(DealConfig& c, float first_deg, float last_deg) {
    for (int k = 0; k < c.n_seats; k++) c.seat_deg[k] = c.n_seats > 1 ? first_deg + (last_deg - first_deg) * k / (c.n_seats - 1) : first_deg;
}
void deal_defaults(DealConfig& c) {
    memset(&c, 0, sizeof c); c.magic = DEAL_MAGIC;
    c.n_seats = 6; c.seat_on = 0x3F; c.cards_each = 2; c.button = 0; c.auto_button = 1; c.board_mode = 1;
    c.dealer_deg = 0;                     // point the foot's magnet mark at the dealer
    deal_spread_seats(c, 360.0f / 7, 6 * 360.0f / 7);    // dealer + six players evenly round the table
    for (int k = 0; k < MAX_SEATS; k++) c.seat_pwm[k] = THROW_PWM_DEFAULT;
    strncpy(c.ap_pass, WIFI_AP_PASS_DEFAULT, sizeof c.ap_pass - 1);
}
void deal_load(DealConfig& c) {
    s_prefs.begin("dealer", true); size_t n = s_prefs.getBytes("cfg", &c, sizeof c); s_prefs.end();
    if (n != sizeof c || c.magic != DEAL_MAGIC || c.n_seats < 2 || c.n_seats > MAX_SEATS) { deal_defaults(c); Serial.println("# no dealer settings stored: defaults loaded"); }
}
void deal_save(const DealConfig& c) { s_prefs.begin("dealer", false); s_prefs.putBytes("cfg", &c, sizeof c); s_prefs.end(); }
int deal_players(const DealConfig& c) { int p = 0; for (int k = 0; k < c.n_seats; k++) if (c.seat_on & (1u << k)) p++; return p; }
int deal_cards_needed(const DealConfig& c) { return deal_players(c) * c.cards_each + (c.board_mode ? 8 : 0); }

// ============================================================ turntable
static float wrap360(float a) { while (a < 0) a += 360; while (a >= 360) a -= 360; return a; }
static bool stop_on_index() { return index_active(); }
static void tt_move_deg(float d, float v, bool (*stop_if)() = nullptr) { tt_stepper_move(lroundf(d * TT_SPD), v, TT_ACC_DPS2, stop_if); }
bool  tt_homed() { return s_homed; }
float tt_angle() { return wrap360(tt_stepper_pos() / TT_SPD); }

bool tt_home() {
    // The index input is shared with the wheel: LOW means the wheel's tab OR the turntable's magnet.
    if (index_active()) {
        tt_move_deg(2 * TT_INDEX_KEEPOUT, TT_V_HOME_DPS);                       // if it was the magnet, this leaves it
        if (index_active() && !wheel_goto(wheel_angle() + 15)) return false;    // else it is the wheel's tab: move the wheel
        if (index_active()) { Serial.println("# index stays active: check the index beam and the hall switch"); return false; }
    }
    s_homed = false;
    tt_move_deg(370, TT_V_HOME_DPS, stop_on_index);
    if (!index_active()) { Serial.println("# turntable magnet not found in one turn"); return false; }
    tt_move_deg(-4, TT_V_HOME_DPS);
    tt_move_deg(8, 15.0f, stop_on_index);                                        // slow final approach from one side
    if (!index_active()) return false;
    tt_stepper_set_pos(0); s_homed = true;
    tt_move_deg(TT_INDEX_KEEPOUT, TT_V_HOME_DPS);                                // off the magnet so the wheel index works
    return true;
}
bool tt_goto(float deg) {
    if (!s_homed && !tt_home()) return false;
    float d = wrap360(deg) - tt_angle();
    if (d > 180) d -= 360;
    if (d < -180) d += 360;
    tt_move_deg(d, TT_VMAX_DPS);
    return true;
}
bool tt_jog(float ddeg) { tt_move_deg(ddeg, TT_VMAX_DPS); return true; }
bool tt_clear_of_index() {
    if (s_homed) { float a = tt_angle(); if (a < TT_INDEX_KEEPOUT || a > 360 - TT_INDEX_KEEPOUT) return tt_goto(TT_INDEX_KEEPOUT); return true; }
    if (index_active()) tt_move_deg(2 * TT_INDEX_KEEPOUT, TT_V_HOME_DPS);      // unhomed: step off a magnet we may be on
    return true;
}

// ============================================================ hand state
const char* hand_stage_name(HandStage s) {
    switch (s) { case HAND_NONE: return "none"; case HAND_HOLE: return "hole cards dealt"; case HAND_FLOP: return "flop out";
                 case HAND_TURN: return "turn out"; case HAND_RIVER: return "river out"; }
    return "?";
}
HandStage hand_stage() { return s_stage; }
int hand_cards_left() { return s_left; }
void deal_request_abort() { s_abort = true; }
bool launcher_clear() { beams_poll(); return !beam_blocked(BEAM_L); }
const char* deal_result_name(DealResult r) {
    switch (r) { case DEAL_OK: return "ok"; case DEAL_ABORTED: return "stopped"; case DEAL_TT_FAIL: return "turntable home not found";
        case DEAL_WHEEL_FAIL: return "wheel move refused (a card bridges the wheel)"; case DEAL_NO_CARD: return "no card came out of the exit window";
        case DEAL_JAM_EXIT: return "card stuck in the exit nip"; case DEAL_JAM_LAUNCH: return "card stuck at the throw roller";
        case DEAL_NOT_REACHED: return "card did not reach the throw roller"; case DEAL_CHUTE_FULL: return "cards in the chute: take them first";
        case DEAL_FEW_CARDS: return "not enough cards for this many players"; case DEAL_BAD_STAGE: return "nothing to deal at this stage"; }
    return "?";
}

// ============================================================ one card out of the wheel
static int next_slot() { for (int s = 0; s < N_SLOTS_HW; s++) if (s_occ[s]) return s; return -1; }

// Eject the card in slot s. throw_pwm > 0: the roller spins and must throw it (beam L blocks, then clears);
// throw_pwm == 0: it piles against the stopped roller. Beams X and L are watched in one loop because the card's
// head reaches beam L (r 169) before its tail leaves beam X (r 111).
static DealResult eject_slot(int s, int throw_pwm) {
    if (!wheel_fin_to_exit((uint8_t)s)) return DEAL_WHEEL_FAIL;
    motors_sleep(false);
    if (throw_pwm > 0) { throw_run(throw_pwm); delay(THROW_SPINUP_MS); }
    DealResult res = DEAL_OK;
    for (int attempt = 0; attempt < 3; attempt++) {
        bool x_seen = false, x_clear = false, l_seen = false, l_was_blocked = launcher_clear() == false;
        uint32_t t0 = millis(), tx = 0, tclr = 0;
        res = DEAL_OK;
        motor_run(M_NIPX, cal.nipx_pwm);
        for (;;) {
            beams_poll();
            bool x = beam_blocked(BEAM_X), l = beam_blocked(BEAM_L); uint32_t now = millis();
            if (l) l_seen = true;
            if (!x_seen) { if (x) { x_seen = true; tx = now; } else if (now - t0 > T_X_BLOCK_MS) { res = DEAL_NO_CARD; break; } }
            else if (!x_clear) { if (!x) { x_clear = true; tclr = now; } else if (now - tx > T_X_CLEAR_MS) { res = DEAL_JAM_EXIT; break; } }
            if (x_clear && now - tclr > 60) motor_run(M_NIPX, 0);
            if (!x_clear) continue;
            if (throw_pwm > 0) {
                if (l_seen && !l) break;                                                   // thrown
                if (!l_seen && now - tclr > T_L_BLOCK_MS) { res = DEAL_NOT_REACHED; break; }
                if (l_seen && now - tclr > T_L_CLEAR_MS + 200) { res = DEAL_JAM_LAUNCH; break; }
            } else {
                if (now - tclr > 60 && (l_seen || l_was_blocked || now - tclr > T_L_BLOCK_MS)) break;   // resting on the pile
            }
        }
        motor_run(M_NIPX, 0);
        if (res == DEAL_JAM_EXIT) { motor_run(M_NIPX, -cal.nipx_pwm); delay(T_REVERSE_MS); motor_run(M_NIPX, 0); delay(60); continue; }
        if (res == DEAL_NO_CARD && attempt < 2) continue;
        if (res == DEAL_OK && x_seen) {
            uint32_t ms = tclr - tx;                                                       // double-card heuristic (eject_one's)
            if (s_median_x && ms > (uint32_t)(s_median_x * DOUBLE_FEED_RATIO)) Serial.printf("WARN slot %d: two cards may have left together (misdeal?)\n", s);
            else s_median_x = s_median_x ? (s_median_x * 7 + ms) / 8 : ms;
        }
        break;
    }
    throw_run(0);
    if (res == DEAL_OK) { s_occ[s] = false; s_left--; }
    return res;
}
static DealResult deliver_to_chute(int count) {   // count < 0: everything left
    if (!tt_goto(dcfg.dealer_deg)) return DEAL_TT_FAIL;
    shutter_power(true); shutter_set(SHUTTER_OPEN);
    for (int i = 0; count < 0 || i < count; i++) {
        int s = next_slot(); if (s < 0) break;
        if (s_abort) return DEAL_ABORTED;
        DealResult r = eject_slot(s, 0); if (r != DEAL_OK) return r;
        app_yield();
    }
    shutter_set(SHUTTER_CLOSED);
    return DEAL_OK;
}
static void park() { motors_sleep(true); shutter_power(false); wheel_idle(); }

// ============================================================ hands
DealResult deal_from_wheel(const bool occupied[N_SLOTS_HW], uint8_t n) {
    memcpy(s_occ, occupied, sizeof s_occ); s_left = n; s_abort = false; s_stage = HAND_NONE;
    if (n < deal_cards_needed(dcfg)) { deliver_to_chute(-1); park(); return DEAL_FEW_CARDS; }
    uint8_t order[MAX_SEATS]; int m = 0;                                      // clockwise from the seat after the button
    for (int i = 1; i <= dcfg.n_seats; i++) { int k = (dcfg.button + i) % dcfg.n_seats; if (dcfg.seat_on & (1u << k)) order[m++] = (uint8_t)k; }
    shutter_power(true); shutter_set(SHUTTER_OPEN);
    DealResult r = DEAL_OK;
    for (int round = 0; round < dcfg.cards_each && r == DEAL_OK; round++)
        for (int j = 0; j < m && r == DEAL_OK; j++) {
            if (s_abort) { r = DEAL_ABORTED; break; }
            int s = next_slot(); if (s < 0) { r = DEAL_FEW_CARDS; break; }
            if (!tt_goto(dcfg.seat_deg[order[j]])) { r = DEAL_TT_FAIL; break; }
            r = eject_slot(s, dcfg.seat_pwm[order[j]]);
            if (r != DEAL_OK) Serial.printf("# seat %d, round %d: %s\n", order[j] + 1, round + 1, deal_result_name(r));
            app_yield();
        }
    if (r != DEAL_OK) { shutter_set(SHUTTER_CLOSED); park(); s_stage = s_left ? HAND_RIVER : HAND_NONE; return r; }   // misdeal: Finish empties the wheel
    if (dcfg.auto_button && m > 0) { dcfg.button = order[0]; deal_save(dcfg); }   // the button moves one player clockwise
    if (dcfg.board_mode) { shutter_set(SHUTTER_CLOSED); tt_goto(dcfg.dealer_deg); s_stage = HAND_HOLE; }
    else { r = deliver_to_chute(-1); s_stage = r == DEAL_OK ? HAND_NONE : HAND_RIVER; }
    park();
    return r;
}
DealResult deal_next_street() {
    if (s_stage == HAND_RIVER) return deal_finish_hand();
    if (s_stage == HAND_NONE) return DEAL_BAD_STAGE;
    if (!launcher_clear()) return DEAL_CHUTE_FULL;          // the previous street must be taken, or the piles mix
    s_abort = false;
    int count = s_stage == HAND_HOLE ? 4 : 2;               // burn + 3, burn + 1, burn + 1: the burn is the bottom card of the pile
    DealResult r = deliver_to_chute(count);
    if (r == DEAL_OK) s_stage = (HandStage)(s_stage + 1);
    park();
    return r;
}
DealResult deal_finish_hand() {
    if (s_left > 0 && !launcher_clear()) return DEAL_CHUTE_FULL;
    s_abort = false;
    DealResult r = s_left > 0 ? deliver_to_chute(-1) : DEAL_OK;
    if (r == DEAL_OK) s_stage = HAND_NONE;
    park();
    return r;
}
DealResult deal_test_throw(uint8_t seat) {
    if (s_stage != HAND_NONE) return DEAL_BAD_STAGE;
    if (!launcher_clear()) return DEAL_CHUTE_FULL;
    if (!tt_homed() && !tt_home()) return DEAL_TT_FAIL;
    if (!wheel_home() || !wheel_fin_to_entry(0)) return DEAL_WHEEL_FAIL;
    if (beam_blocked_now(BEAM_E)) { Serial.println("# test throw: slot 0 already holds a card (unload the wheel first)"); return DEAL_WHEEL_FAIL; }
    shutter_power(true); shutter_set(SHUTTER_CLOSED);
    FeedStats st; FeedResult f = feeder_feed_one(&st); feeder_stop();
    if (f != FEED_OK) { Serial.printf("# test throw: feed %s\n", feed_result_name(f)); park(); return DEAL_NO_CARD; }
    memset(s_occ, 0, sizeof s_occ); s_occ[0] = true; s_left = 1;
    shutter_set(SHUTTER_OPEN);
    DealResult r = tt_goto(dcfg.seat_deg[seat % MAX_SEATS]) ? eject_slot(0, dcfg.seat_pwm[seat % MAX_SEATS]) : DEAL_TT_FAIL;
    shutter_set(SHUTTER_CLOSED); park();
    if (r != DEAL_OK) s_stage = s_left ? HAND_RIVER : HAND_NONE;
    return r;
}
void hand_reset() { memset(s_occ, 0, sizeof s_occ); s_left = 0; s_stage = HAND_NONE; }
#endif
