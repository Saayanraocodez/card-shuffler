// motion.h — elevator (Z in mm, card frame) and feeder procedures built on hardware.h.
#pragma once
#include <Arduino.h>
#include "config.h"

// ---- elevator ----
bool  elevator_home();                       // false if the endstop never trips
float elevator_z();
void  elevator_move_to(float z, float vmax = ELEV_VMAX);
// Raises the platform slowly until beam S blocks. Returns the platform Z at the trigger, or NAN if the
// beam did not block before z_limit.
float elevator_find_beam_s(float z_limit);
float knife_z();                             // Z_KNIFE_NOMINAL + cal.knife_offset
void  elevator_idle();                       // disable the driver (lead screw is self-locking)

// ---- feeder ----
enum FeedResult { FEED_OK = 0, FEED_NO_CARD, FEED_JAM_PICK, FEED_JAM_GATE, FEED_JAM_WELL, FEED_JAM_CLEAR };
struct FeedStats { uint32_t b_block_ms; bool double_suspect; };
bool       feeder_probe_hopper();            // true if a card is present (pre-stages it at the gate)
FeedResult feeder_feed_one(FeedStats* st);   // full sequence with sensor timeouts (no retries)
void       feeder_reverse_pulse();           // both motors backwards briefly (jam clearing)
void       feeder_stop();
const char* feed_result_name(FeedResult r);
