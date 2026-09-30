// shuffle_core.h — the pure algorithm for the WHEEL: random empty-slot assignment and output prediction.
// No hardware dependencies; unit-tested on the host (firmware/test_host) against simulation/rng_reference.py.
#pragma once
#include <stdint.h>
#include "rng.h"

#define MAX_CARDS 60
#define N_SLOTS   54

// Assign card i (i = 0 .. n_cards-1, in feed order, 0 = bottom of the input deck) to a uniformly random
// EMPTY slot.  Exactly the Fisher–Yates "draw from the remaining set" step:
//   empty = [0..n_slots-1]; for i: idx = uniform(len); slot = empty[idx]; empty[idx] = empty[len-1]; len--.
// Returns false if n_cards > n_slots.
bool slots_assign(Drbg* d, uint8_t n_cards, uint8_t n_slots, uint8_t slot_of_card[MAX_CARDS]);

// Output order after unloading slots 0,1,2,... in sequence onto the output pile (first out = bottom):
// order[p] = input index of the card at output position p (0 = bottom).
void predict_order_from_slots(uint8_t n_cards, const uint8_t slot_of_card[MAX_CARDS], uint8_t order[MAX_CARDS]);

// Wheel angle (degrees, CCW from +X) that puts fin `fin` on the plane at `fin_angle_deg`.
static inline float wheel_angle_for_fin(uint8_t fin, float fin_angle_deg, float pitch_deg) { return fin_angle_deg - (float)fin * pitch_deg; }
