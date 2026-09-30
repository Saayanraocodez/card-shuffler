// shuffle_core.h — the pure algorithm: gap sequence generation and permutation prediction.
// No hardware dependencies, so it is unit-tested on the host (firmware/test_host).
#pragma once
#include <stdint.h>
#include "rng.h"

#define MAX_CARDS 60

// Draw the inside-out Fisher–Yates gap sequence: gaps[i] uniform in {0..i}.
void gaps_draw(Drbg* d, uint8_t n, uint8_t gaps[MAX_CARDS]);

// Predict the output order for a gap sequence: order[p] = index of the input card (0 = input bottom)
// that ends at output position p (0 = output bottom).
void predict_order(uint8_t n, const uint8_t gaps[MAX_CARDS], uint8_t order[MAX_CARDS]);

// Insertion geometry: platform Z (mm, card frame) that puts gap k of an n-card stack of mean
// thickness t_mean (mm) at the blade mid-plane knife_z.
static inline float gap_platform_z(float knife_z, uint8_t k, float t_mean) { return knife_z - (float)k * t_mean; }
