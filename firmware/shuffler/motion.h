// motion.h — wheel positioning, feeding into a slot, unloading from a slot, occupancy scanning.
#pragma once
#include <Arduino.h>
#include "config.h"

// ---- wheel ----
// Interlock: every wheel move first checks that no card bridges the wheel and a fixed part
// (beam S at the entry, beam X at the exit).  If one does, the move is refused and returns false.
bool  wheel_path_clear();
bool  wheel_home();                          // find the index tab; false if blocked or not found in one revolution
float wheel_angle();                         // current wheel angle (deg, CCW, fin 0 reference)
bool  wheel_goto(float angle_deg, float vmax = WHEEL_VMAX_DPS);   // shortest path, wrap-around
bool  wheel_fin_to_entry(uint8_t fin);       // fin k → entry plane (card enters slot k)
bool  wheel_fin_to_exit(uint8_t slot);       // fin k+1 → exit plane (card leaves slot k)
void  wheel_idle();
// Step through all slots reading beam E; occupied[s] = card seen in slot s. Returns the count, or -1 if a move was refused.
int   wheel_scan(bool occupied[N_SLOTS_HW]);

// ---- feeder ----
enum FeedResult { FEED_OK = 0, FEED_NO_CARD, FEED_JAM_PICK, FEED_JAM_ENTRY, FEED_JAM_CLEAR, FEED_NOT_SEATED, FEED_STUCK_MOUTH };
struct FeedStats { uint32_t b_block_ms; bool double_suspect; };
bool        feeder_probe_hopper();
// Wheel must already present the (empty) target slot at the entry.
//   FEED_OK          card seated in the slot at the entry (beam E blocked, beam S clear)
//   FEED_NOT_SEATED  card left the feeder and the mouth is clear, but the slot at the entry is empty
//                    (it went to a neighbour slot: the caller searches)
//   FEED_STUCK_MOUTH card still bridges the plate and the slot after nudging: the wheel must not move
FeedResult  feeder_feed_one(FeedStats* st);
void        feeder_reverse_pulse();
void        feeder_stop();
const char* feed_result_name(FeedResult r);

// ---- unload ----
enum EjectResult { EJECT_OK = 0, EJECT_NO_CARD, EJECT_JAM };
struct EjectStats { uint32_t x_block_ms; bool double_suspect; };
EjectResult eject_one(EjectStats* st);       // wheel must already present the slot at the exit; shutter open
void        eject_stop();
