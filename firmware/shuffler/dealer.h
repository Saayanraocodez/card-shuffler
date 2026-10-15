// dealer.h — turntable, launcher and hand sequencing for the DEALER build (docs/11-dealer.md).
//
// The whole machine turns on a turntable so the chute points at each seat. A spinning top roller at the chute end
// throws each card off a ramp; stopped, it is the wall the rest of the deck piles against for the dealer.
// Cards are dealt in slot order (the shuffle's output order), one at a time round the table, so every player's hand
// is uniformly random exactly as the plain shuffle's output order is.
#pragma once
#include <Arduino.h>
#include "config.h"
#if DEALER

// Settings kept in NVS and edited on the phone page. Seats are numbered clockwise seen from above.
struct DealConfig {
    uint32_t magic;
    uint8_t  n_seats;               // seats around the table, 2..MAX_SEATS
    uint16_t seat_on;               // bit k: seat k has a player this hand
    float    seat_deg[MAX_SEATS];   // turntable angle (clockwise from the home magnet) that points the launcher at seat k
    uint8_t  seat_pwm[MAX_SEATS];   // throw power for seat k (0..255): farther seats need more
    float    dealer_deg;            // turntable angle that points the chute at the dealer (rest of the deck, board cards, parking)
    uint8_t  cards_each;            // hole cards per player (2 hold'em, 4 Omaha, 5 PLO5 / draw)
    uint8_t  button;                // dealer-button seat; the first card goes to the next occupied seat clockwise
    uint8_t  auto_button;           // move the button to the next occupied seat after each hand
    uint8_t  board_mode;            // 0: rest of the deck to the chute after the hole cards
                                    // 1: keep it in the wheel; deliver burn + flop, burn + turn, burn + river on request
    char     ap_pass[24];           // Wi-Fi password (8..23 characters)
};
#define DEAL_MAGIC 0xDEA10002u
extern DealConfig dcfg;
void deal_defaults(DealConfig& c);
void deal_load(DealConfig& c);
void deal_save(const DealConfig& c);
void deal_spread_seats(DealConfig& c, float first_deg, float last_deg);   // occupied-or-not, seats evenly over an arc
int  deal_players(const DealConfig& c);
int  deal_cards_needed(const DealConfig& c);   // hole cards (+ 8 board cards incl. burns in board mode)

// ---- turntable ----
bool  tt_home();                    // find the magnet; leaves the turntable TT_INDEX_KEEPOUT past it
bool  tt_homed();
float tt_angle();                   // deg, clockwise from the magnet, 0..360
bool  tt_goto(float deg);           // shortest way round (nothing is cabled to the table, so it can turn forever)
bool  tt_jog(float ddeg);
// Called by wheel_home(): the index input is shared, so make sure the turntable is not sitting on its magnet.
bool  tt_clear_of_index();

// ---- hand state (the cards still in the wheel after the hole cards, in board mode) ----
enum HandStage : uint8_t { HAND_NONE = 0, HAND_HOLE, HAND_FLOP, HAND_TURN, HAND_RIVER };
const char* hand_stage_name(HandStage s);
HandStage hand_stage();
int  hand_cards_left();

enum DealResult { DEAL_OK = 0, DEAL_ABORTED, DEAL_TT_FAIL, DEAL_WHEEL_FAIL, DEAL_NO_CARD, DEAL_JAM_EXIT, DEAL_JAM_LAUNCH,
                  DEAL_NOT_REACHED, DEAL_CHUTE_FULL, DEAL_FEW_CARDS, DEAL_BAD_STAGE };
const char* deal_result_name(DealResult r);

// After a shuffle has loaded the wheel (occupied[] = slots holding a card, n cards): deal the hole cards round the
// table, then either deliver the rest to the chute or keep it for the streets (board mode).
DealResult deal_from_wheel(const bool occupied[N_SLOTS_HW], uint8_t n);
DealResult deal_next_street();      // board mode: burn + flop / burn + turn / burn + river into the chute
DealResult deal_finish_hand();      // whatever is left goes to the chute; the wheel is empty afterwards
DealResult deal_test_throw(uint8_t seat);   // feed one card from the hopper and throw it at a seat (calibration)
bool launcher_clear();              // beam L (and B) clear: nothing resting at the throw roller
void deal_request_abort();          // checked between cards (phone Stop button, long button press)
void hand_reset();                  // forget the hand (after "unload all" has emptied the wheel)
#endif
