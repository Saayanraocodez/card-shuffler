// chacha20.cpp — RFC 8439 ChaCha20 block function.
#include "chacha20.h"

static inline uint32_t rotl(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }
static inline uint32_t le32(const uint8_t* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
#define QR(a,b,c,d) do { a += b; d ^= a; d = rotl(d,16); c += d; b ^= c; b = rotl(b,12); \
                         a += b; d ^= a; d = rotl(d, 8); c += d; b ^= c; b = rotl(b, 7); } while (0)

void chacha20_block(const uint8_t key[32], uint32_t counter, const uint8_t nonce[12], uint32_t out[16]) {
    uint32_t s[16] = { 0x61707865, 0x3320646e, 0x79622d32, 0x6b206574 };
    for (int i = 0; i < 8; i++) s[4 + i] = le32(key + 4*i);
    s[12] = counter;
    for (int i = 0; i < 3; i++) s[13 + i] = le32(nonce + 4*i);
    uint32_t w[16]; for (int i = 0; i < 16; i++) w[i] = s[i];
    for (int r = 0; r < 10; r++) {
        QR(w[0], w[4], w[8],  w[12]); QR(w[1], w[5], w[9],  w[13]); QR(w[2], w[6], w[10], w[14]); QR(w[3], w[7], w[11], w[15]);
        QR(w[0], w[5], w[10], w[15]); QR(w[1], w[6], w[11], w[12]); QR(w[2], w[7], w[8],  w[13]); QR(w[3], w[4], w[9],  w[14]);
    }
    for (int i = 0; i < 16; i++) out[i] = w[i] + s[i];
}
