// sha256.h — small portable SHA-256 (FIPS 180-4). Used to condition entropy into the DRBG key.
#pragma once
#include <stdint.h>
#include <stddef.h>

struct Sha256 {
    uint32_t h[8];
    uint8_t  buf[64];
    uint64_t len;
    size_t   fill;
};
void sha256_init(Sha256* s);
void sha256_update(Sha256* s, const uint8_t* data, size_t n);
void sha256_final(Sha256* s, uint8_t out[32]);
void sha256(const uint8_t* data, size_t n, uint8_t out[32]);
