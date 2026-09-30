// motion.h — wheel positioning, feeding into a slot, unloading from a slot, occupancy scanning.
#pragma once
#include <Arduino.h>
#include "config.h"

// ---- wheel ----
bool  wheel_home();                          // find the index tab; false if not found in one revolution
float wheel_angle();                         // current wheel angle (deg, CCW, fin 0 reference)
void  wheel_goto(float angle_deg, float vmax = WHEEL_VMAX_DPS);   // shortest path, wrap-around
void  wheel_fin_to_entry(uint8_t fin);       // fin k → entry plane (card enters slot k)
void  wheel_fin_to_exit(uint8_t fin);        // fin k+1 → exit plane (card leaves slot k): pass k
void  wheel_idle();
// Rotate one revolution at scan speed sampling beam E; occupied[s] = card seen in slot s.
// Returns the number of occupied slots.
int   wheel_scan(bool occupied[N_SLOTS_HW]);

// ---- feeder ----
enum FeedResult { FEED_OK = 0, FEED_NO_CARD, FEED_JAM_PICK, FEED_JAM_ENTRY, FEED_JAM_CLEAR, FEED_NOT_SEATED };
struct FeedStats { uint32_t b_block_ms; bool double_suspect; };
bool        feeder_probe_hopper();
FeedResult  feeder_feed_one(FeedStats* st);  // wheel must already present the target slot at the entry
void        feeder_reverse_pulse();
void        feeder_stop();
const char* feed_result_name(FeedResult r);

// ---- unload ----
enum EjectResult { EJECT_OK = 0, EJECT_NO_CARD, EJECT_JAM };
EjectResult eject_one();                     // wheel must already present the slot at the exit; shutter open
void        eject_stop();
