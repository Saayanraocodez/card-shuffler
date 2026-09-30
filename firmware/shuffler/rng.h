// rng.h — entropy collection, DRBG, unbiased integer sampling.
//
// Pipeline (matches simulation/rng_reference.py bit for bit):
//   key   = SHA-256( 64 B hardware random || 32 B jitter pool || uint32 boot counter || uint64 shuffle counter )
//   nonce = "SHF\x01" || uint64 LE shuffle counter
//   keystream = ChaCha20(key, nonce, block counter from 1); next_u32() = successive LE words
//   uniform(n) = rejection sampling: x = next_u32(); accept iff x < 2^32 - (2^32 mod n); return x % n
//
// Platform hooks (implemented in rng_platform_*.cpp):  hardware random bytes, jitter pool, boot counter.
#pragma once
#include <stdint.h>
#include <stddef.h>

struct Drbg {
    uint8_t  key[32];
    uint8_t  nonce[12];
    uint32_t block_counter;
    uint32_t buf[16];
    int      buf_pos;          // 16 = empty
    uint32_t words_drawn;
    uint32_t rejections;
};

// Seeds the DRBG from the given material (the platform layer collects it). Returns false if the
// hardware entropy health test fails.
bool drbg_seed(Drbg* d, const uint8_t hw[64], const uint8_t jitter[32], uint32_t boot_counter, uint64_t shuffle_counter);
// Seeds with an explicit key (test mode / reference comparison).
void drbg_seed_key(Drbg* d, const uint8_t key[32], uint64_t shuffle_counter);
uint32_t drbg_next_u32(Drbg* d);
uint32_t drbg_uniform(Drbg* d, uint32_t n);   // unbiased integer in [0, n)

// Repetition-count health test on raw 32-bit hardware samples (NIST SP 800-90B §4.4.1, simplified):
// fails if any value repeats 4 times in a row.
bool rng_health_ok(const uint8_t hw[64]);

// Jitter pool: mix timestamps / ADC noise from events as they happen (call from anywhere, cheap).
void jitter_add(uint32_t v);
void jitter_snapshot(uint8_t out[32]);
