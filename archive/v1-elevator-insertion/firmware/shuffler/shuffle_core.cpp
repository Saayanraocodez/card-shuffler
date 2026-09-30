// shuffle_core.cpp
#include "shuffle_core.h"

void gaps_draw(Drbg* d, uint8_t n, uint8_t gaps[MAX_CARDS]) {
    for (uint8_t i = 0; i < n; i++) gaps[i] = (uint8_t)drbg_uniform(d, (uint32_t)i + 1);
}

void predict_order(uint8_t n, const uint8_t gaps[MAX_CARDS], uint8_t order[MAX_CARDS]) {
    // Build the stack bottom-up by insertion (O(n^2), n <= 60: trivial).
    uint8_t stack[MAX_CARDS]; uint8_t len = 0;
    for (uint8_t i = 0; i < n; i++) {
        uint8_t j = gaps[i]; if (j > len) j = len;
        for (int p = len; p > j; p--) stack[p] = stack[p - 1];
        stack[j] = i; len++;
    }
    for (uint8_t p = 0; p < n; p++) order[p] = stack[p];
}
