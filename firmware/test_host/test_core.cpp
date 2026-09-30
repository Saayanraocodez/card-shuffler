// test_core.cpp — host-side unit test for the platform-independent firmware core.
//   make run   (or: g++ -std=c++17 -O2 -I../shuffler test_core.cpp ../shuffler/{sha256,chacha20,rng,shuffle_core}.cpp -o test_core)
// Checks: SHA-256 test vector, ChaCha20 RFC 8439 block vector, the DRBG gap sequence against the
// Python reference (simulation/rng_reference.py --selftest), the predicted order against
// simulation/shuffle_sim.py, rejection-sampling uniformity, and the entropy health test.
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>
#include "sha256.h"
#include "chacha20.h"
#include "rng.h"
#include "shuffle_core.h"

static int fails = 0;
#define CHECK(c, msg) do { if (!(c)) { printf("FAIL: %s\n", msg); fails++; } else printf("ok:   %s\n", msg); } while (0)

static std::string hex(const uint8_t* p, size_t n) { char b[3]; std::string s; for (size_t i = 0; i < n; i++) { snprintf(b, 3, "%02x", p[i]); s += b; } return s; }

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    // SHA-256("abc")
    uint8_t h[32]; sha256((const uint8_t*)"abc", 3, h);
    CHECK(hex(h, 32) == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad", "SHA-256 test vector");

    // ChaCha20 block function, RFC 8439 §2.3.2
    uint8_t key[32]; for (int i = 0; i < 32; i++) key[i] = i;
    uint8_t nonce[12] = {0,0,0,9,0,0,0,0x4a,0,0,0,0};
    uint32_t blk[16]; chacha20_block(key, 1, nonce, blk);
    CHECK(blk[0] == 0xe4e7f110 && blk[1] == 0x15593bd1 && blk[15] == 0x4e3c50a2, "ChaCha20 RFC 8439 block vector");

    // DRBG gap sequence vs Python reference (key = SHA-256("card-shuffler-test-key"), counter 1)
    uint8_t tkey[32]; sha256((const uint8_t*)"card-shuffler-test-key", 22, tkey);
    CHECK(hex(tkey, 32) == "1b5093f405094119166c8d4b01f0d753525f4939e192e07e88c6a3993a126870", "test key derivation");
    Drbg d; drbg_seed_key(&d, tkey, 1);
    uint8_t gaps[MAX_CARDS]; gaps_draw(&d, 52, gaps);
    const int ref_gaps[52] = {0,0,0,0,4,0,1,7,0,5,6,5,9,8,0,7,13,13,5,0,19,8,18,5,1,25,25,24,15,29,12,24,17,6,23,31,3,35,8,28,29,22,15,36,1,38,12,32,29,30,21,42};
    bool same = true; for (int i = 0; i < 52; i++) same &= gaps[i] == ref_gaps[i];
    CHECK(same, "gap sequence matches simulation/rng_reference.py");
    CHECK(d.words_drawn == 52 && d.rejections == 0, "52 words drawn, 0 rejections");

    // Predicted order vs Python build_order
    uint8_t order[MAX_CARDS]; predict_order(52, gaps, order);
    const int ref_order[52] = {19,44,24,14,36,8,5,6,33,38,23,3,46,18,2,21,11,42,30,15,9,50,10,28,32,13,41,1,12,17,48,49,34,16,22,47,39,40,0,31,4,20,51,43,45,27,35,7,26,37,25,29};
    same = true; for (int i = 0; i < 52; i++) same &= order[i] == ref_order[i];
    CHECK(same, "predicted order matches simulation/shuffle_sim.py build_order");

    // Every gap in range, always
    Drbg d2; drbg_seed_key(&d2, tkey, 7); bool inrange = true;
    for (int r = 0; r < 2000; r++) { gaps_draw(&d2, 54, gaps); for (int i = 0; i < 54; i++) inrange &= gaps[i] <= i; }
    CHECK(inrange, "gaps always within [0, i] for 54 cards over 2000 draws");

    // Sampler uniformity: chi-square on uniform(52) over 5.2 million draws (expect ~df=51)
    Drbg d3; drbg_seed_key(&d3, tkey, 99); long cnt[52] = {0}; const long N = 5200000;
    for (long i = 0; i < N; i++) cnt[drbg_uniform(&d3, 52)]++;
    double chi = 0, e = (double)N / 52; for (int i = 0; i < 52; i++) chi += (cnt[i] - e) * (cnt[i] - e) / e;
    printf("      chi2(51 df) = %.1f  (expect ~51 ± 10; >88 would be p<0.001)\n", chi);
    CHECK(chi < 88.0, "uniform(52) chi-square within bounds");

    // Rejection sampling exercises the reject path for a large n
    Drbg d4; drbg_seed_key(&d4, tkey, 5); uint32_t n = 0xC0000000u; long rej = 0;
    for (int i = 0; i < 100000; i++) { uint32_t x = drbg_uniform(&d4, n); if (x >= n) { printf("out of range\n"); fails++; } }
    rej = d4.rejections; printf("      rejections for n=0xC0000000: %ld of 100000 accepted (expect ~33333: p_reject = 1/4 per draw)\n", rej);
    CHECK(rej > 30500 && rej < 36500, "rejection path behaves");

    // Entropy health test
    uint8_t hw[64]; memset(hw, 0x5a, 64);
    CHECK(!rng_health_ok(hw), "health test rejects a stuck source");
    for (int i = 0; i < 64; i++) hw[i] = (uint8_t)(i * 37 + 11);
    CHECK(rng_health_ok(hw), "health test accepts a varying source");

    // Seeding is deterministic given the same material, and differs when the shuffle counter changes
    uint8_t jit[32] = {0}; Drbg a, b, c;
    drbg_seed(&a, hw, jit, 1, 10); drbg_seed(&b, hw, jit, 1, 10); drbg_seed(&c, hw, jit, 1, 11);
    CHECK(memcmp(a.key, b.key, 32) == 0 && memcmp(a.key, c.key, 32) != 0, "seed derivation deterministic and counter-sensitive");

    printf("%s (%d failures)\n", fails ? "SOME TESTS FAILED" : "ALL TESTS PASSED", fails);
    return fails ? 1 : 0;
}
