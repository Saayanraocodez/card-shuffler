// rng.cpp — DRBG and sampler (portable; no Arduino dependencies).
#include "rng.h"
#include "sha256.h"
#include "chacha20.h"
#include <string.h>

static Sha256 g_jitter;          // running hash of everything fed to jitter_add()
static bool   g_jitter_init = false;

void jitter_add(uint32_t v) {
    if (!g_jitter_init) { sha256_init(&g_jitter); g_jitter_init = true; }
    uint8_t b[4] = { (uint8_t)v, (uint8_t)(v >> 8), (uint8_t)(v >> 16), (uint8_t)(v >> 24) };
    sha256_update(&g_jitter, b, 4);
}
void jitter_snapshot(uint8_t out[32]) {
    if (!g_jitter_init) { sha256_init(&g_jitter); g_jitter_init = true; }
    Sha256 copy = g_jitter;          // snapshot without resetting the running hash
    sha256_final(&copy, out);
    sha256_update(&g_jitter, out, 32);   // fold the snapshot back in so successive snapshots differ
}

bool rng_health_ok(const uint8_t hw[64]) {
    uint32_t prev = 0; int run = 0;
    for (int i = 0; i < 16; i++) {
        uint32_t v; memcpy(&v, hw + 4*i, 4);
        if (i > 0 && v == prev) { if (++run >= 3) return false; } else run = 0;
        prev = v;
    }
    // also reject an all-zero or all-0xFF buffer
    int z = 0, f = 0; for (int i = 0; i < 64; i++) { z += hw[i] == 0; f += hw[i] == 0xFF; }
    return z < 32 && f < 32;
}

static void set_nonce(Drbg* d, uint64_t ctr) {
    d->nonce[0] = 'S'; d->nonce[1] = 'H'; d->nonce[2] = 'F'; d->nonce[3] = 0x01;
    for (int i = 0; i < 8; i++) d->nonce[4 + i] = (uint8_t)(ctr >> (8*i));
    d->block_counter = 1; d->buf_pos = 16; d->words_drawn = 0; d->rejections = 0;
}

bool drbg_seed(Drbg* d, const uint8_t hw[64], const uint8_t jitter[32], uint32_t boot_counter, uint64_t shuffle_counter) {
    if (!rng_health_ok(hw)) return false;
    Sha256 s; sha256_init(&s);
    sha256_update(&s, hw, 64);
    sha256_update(&s, jitter, 32);
    uint8_t b[8];
    for (int i = 0; i < 4; i++) b[i] = (uint8_t)(boot_counter >> (8*i));
    sha256_update(&s, b, 4);
    for (int i = 0; i < 8; i++) b[i] = (uint8_t)(shuffle_counter >> (8*i));
    sha256_update(&s, b, 8);
    sha256_final(&s, d->key);
    set_nonce(d, shuffle_counter);
    return true;
}
void drbg_seed_key(Drbg* d, const uint8_t key[32], uint64_t shuffle_counter) { memcpy(d->key, key, 32); set_nonce(d, shuffle_counter); }

uint32_t drbg_next_u32(Drbg* d) {
    if (d->buf_pos >= 16) { chacha20_block(d->key, d->block_counter++, d->nonce, d->buf); d->buf_pos = 0; }
    d->words_drawn++;
    return d->buf[d->buf_pos++];
}
uint32_t drbg_uniform(Drbg* d, uint32_t n) {
    if (n == 0) return 0;
    // NB: n == 1 still consumes one keystream word (t == 0, x % 1 == 0) to stay bit-exact with the reference.
    // Accept x iff x < 2^32 - t where t = 2^32 mod n (same acceptance region as rng_reference.py).
    // t is computed as (2^32 - n) mod n in 32-bit arithmetic; when n divides 2^32 (t == 0) every x is
    // accepted, so the comparison must never form 2^32 - t in 32 bits (it would overflow to 0).
    uint32_t t = (uint32_t)(0u - n) % n;
    for (;;) { uint32_t x = drbg_next_u32(d); if (t == 0 || x <= 0xFFFFFFFFu - t) return x % n; d->rejections++; }
}
